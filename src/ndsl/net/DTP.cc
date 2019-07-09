/*
 * @file DTP.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#include "ndsl/net/SocketAddress.h"
#include "ndsl/net/TcpAcceptor.h"

#include "ControlMsg.pb.h"
#include "ndsl/net/DTPClient.h"
#include "ndsl/net/TcpClient.h"
#include "ndsl/net/DTP.h"
#include "ndsl/config.h"
#include "ndsl/net/RdmaAcceptor.h"
#include "ndsl/net/UdpEndpoint.h"
#include "ndsl/net/UdpClient.h"
#include "ndsl/net/RdmaClient.h"
#include "ndsl/net/RdmaConnection.h"

#include <ifaddrs.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <openssl/md5.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace ndsl;
using namespace net;
using namespace std;

DTP::DTP(Multiplexer *m)
    : mul_(m)
{}

int DTP::sendFile(
    TcpConnection *conn,
    int peerId,
    char *filePath,
    int length,
    DTPClient *client)
{
    int realLength = length;
    if (length == 0) {
        // 手动计算文件长度
        struct stat filestat;
        if (stat(filePath, &filestat) < 0) {
            LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DTP, "Failed to get file length");
        }
        realLength = filestat.st_size;
    }

    // 创建动态侦听端口
    // 启动tcoAcceptor 准备接收链接
    struct SocketAddress4 servaddr("0.0.0.0", 0);
    TcpAcceptor *tAc = new TcpAcceptor(conn->getLoop());
    tAc->start(servaddr);

    int fd = tAc->listenfd_;
    struct sockaddr_in localaddr;
    socklen_t len = sizeof(localaddr);
    int ret = getsockname(fd, (struct sockaddr *) &localaddr, &len);
    if (ret != 0) { LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DTP, "getsockname error"); }

    TcpConnection *Conn = new TcpConnection();

    // 相关信息保存在一个结构体中
    pListenMgr mgr = new struct ListenMgr(this, filePath, Conn, client, tAc, 0);

    // 定时任务
    mul_->loop_->runAfter(WAITTIMEOUT, stopListen, mgr);

    // 进行监听
    tAc->onAccept(Conn, NULL, NULL, acceptorCallback, mgr);

    // 计算文件checksum CRC MD5 需要把文件读上来
    char *md5 = (char *) calloc(MD5LENTH * 2 + 1, sizeof(char));
    if (getMd5(filePath, md5) != S_OK) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DTP, "getMd5 error");
    }

    char *ip = (char *) calloc(INET_ADDRSTRLEN, sizeof(char));
    // if (getLocalIp(ip) != S_OK) {
    //     LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DTP, "getLocalIp error");
    // }
    // FIXME: 这里伪装一个ip方便测试 正式代码用上面注释部分的代码
    strcpy(ip, "127.0.0.1");

    // 创建控制报文
    mnet::ControlMsg *msg = makeControlMsg(
        SENDFILEFLAG,
        peerId,
        ip,
        ntohs(localaddr.sin_port),
        md5,
        filePath,
        realLength);

    // 通过Mul发送buf
    std::string *s = new std::string;
    (*msg).SerializeToString(s);

    free(md5);
    free(ip);

    // c_str() 返回的const char*的声明周期同string的生命周期
    // 并且改变string的值，当前返回值也会改变，即使不再调用
    mul_->sendMessage(conn, 0, s);

    return S_OK;
}
int DTP::recvFile(
    TcpConnection *conn,
    int peerId,
    char *filename,
    DTPClient *client)
{
    //控制报文
    mnet::ControlMsg *msg = new mnet::ControlMsg;
    // msg->set_isdtp(true);
    msg->set_flag(RECVFILEFLAG);
    msg->set_id(peerId);
    msg->set_filename(filename);

    // 通过Mul发送buf
    std::string *s = new std::string;
    msg->SerializeToString(s);

    // 启动定时器 用于处理对端文件不存在的情况
    // TimeWheel *time_ = new TimeWheel(conn->getLoop());
    // time_->start();

    // 相关信息保存在一个结构体中
    pRecvFileInfo info =
        new RecvFileInfo(this, client, filename, -1, NULL, NULL, NULL, NULL);

    // mul_->loop_->runAfter(WAITTIMEOUT, handleRecvfileFail, info);

    // 初始化定时器任务
    TimeWheel::Task *t = new TimeWheel::Task;
    t->setInterval = WAITTIMEOUT; // 中断
    t->doit = handleRecvfileFail;
    t->param = info;

    // 开始任务
    mul_->loop_->runAfter(t);
    timeMp_.insert(make_pair(client, t));

    mul_->sendMessage(conn, 0, s);

    return S_OK;
}

void DTP::handleRecvfileFail(void *para)
{
    LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DTP, "RecvFile fail!\n");
    pRecvFileInfo info = static_cast<pRecvFileInfo>(para);

    DTP *dtp = info->dtp_;
    DTPClient *client = info->client_;

    mnet::ControlMsg *msg = NULL;
    msg = dtp->makeNoticeMsg(RECVFILEFAIL, info->fileName_);

    client->analysisProtobuf(msg);

    // 停止计时器 释放掉计时器资源
    // info->time_->stop();
    // delete info->time_;
}

void DTP::stopListen(void *manager)
{
    LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DTP, "Listening timeout");
    pListenMgr mgr = static_cast<pListenMgr>(manager);

    // 防止conn已经建立连接误删的情况发生
    if (mgr->acceptor_ != NULL) {
        delete mgr->acceptor_;
        delete mgr->conn_;
    }
}

void DTP::acceptorCallback(void *point)
{
    // 拿到相应指针
    pListenMgr mgr = static_cast<pListenMgr>(point);
    TcpConnection *conn = static_cast<TcpConnection *>(mgr->conn_);

    delete mgr->acceptor_;
    mgr->acceptor_ = NULL;

    // 停止超时计时器，并回收内存
    // mgr->time_->stop();
    // delete mgr->time_;

    // 发送数据
    conn->sendFile(mgr->filePath_, 0, 0, sendFileCallback, mgr);
}

void DTP::sendFileCallback(void *point)
{
    pListenMgr mgr = static_cast<pListenMgr>(point);

    // 准备回传的数据
    mnet::ControlMsg *msg = mgr->dtp_->makeNoticeMsg(
        SENDFILESUCC, mgr->dtp_->getFileName(mgr->filePath_));

    // 通知用户 做protobuf，然后交给DTPClient
    mgr->client_->analysisProtobuf(msg);

    delete msg;

    // 清理内存
    utils::WorkItem *w1 = new utils::WorkItem;
    w1->doit = senfFileClearMemory;
    w1->param = static_cast<void *>(mgr);
    mgr->dtp_->mul_->loop_->addWork(w1);
}

void DTP::senfFileClearMemory(void *manager)
{
    pListenMgr mgr = static_cast<pListenMgr>(manager);
    // delete mgr->sendfile_;
    // delete mgr->acceptor_;
    delete mgr->conn_;
    delete mgr;
}

int DTP::getMd5(char *filePath, char *result)
{
    std::ifstream ifile(filePath, std::ios::in | std::ios::binary);
    //打开文件
    unsigned char MD5result[MD5LENTH] = {0};

    if (ifile.fail()) //打开失败不做文件MD5
    {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DTP, "open file fail");
        return S_FALSE;
    }
    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);

    char DataBuff[MAXLINE];
    while (!ifile.eof()) {
        ifile.read(DataBuff, MAXLINE); //读文件
        int length = ifile.gcount();
        if (length) {
            MD5_Update(&md5_ctx, DataBuff, length);
            //将当前文件块加入并更新MD5
        }
    }
    MD5_Final(MD5result, &md5_ctx); //获取MD5

    // 转换格式
    for (int i = 0; i < MD5LENTH; i++) {
        sprintf(result + (i * 2), "%02X", MD5result[i]);
    }
    return S_OK;
}

mnet::ControlMsg *DTP::makeNoticeMsg(int type, const char *fileName)
{
    mnet::ControlMsg *msg = new mnet::ControlMsg;
    msg->set_flag(type);
    msg->set_filename(fileName);

    return msg;
}

mnet::ControlMsg *DTP::makeControlMsg(
    int type,
    int id,
    char *ip,
    int dataport,
    char *md5,
    char *filePath,
    int length)
{
    // flag ,id,  ip，      dataport, checksum,          filename, length
    // int  ,int, char[15], int,      unsigned char[16], char 64,  int

    mnet::ControlMsg *msg = new mnet::ControlMsg;
    msg->set_flag(type); // 区分是什么消息，sendfile senddata
                         // recvfile ercvdata
    msg->set_id(id);
    msg->set_ip(ip);
    msg->set_dataport(dataport);
    msg->set_checksum(md5);
    msg->set_filename(getFileName(filePath));
    msg->set_length(length);

    return msg;
}

char *DTP::getFileName(char *filePath)
{
    int st = 0;
    for (int i = 0; filePath[i]; i++) {
        if (filePath[i] == '/') { st = i; }
    }
    return filePath + st + 1;
}

void DTP::agreeRecvFile(mnet::ControlMsg *msg, DTPClient *client)
{
    //关闭定时器
    map<DTPClient *, TimeWheel::Task *>::iterator it = timeMp_.find(client);
    if (it != timeMp_.end()) {
        // 取消定时器
        client->mul_->loop_->cancelRunAfter(it->second);
        delete it->second;
    }
    // 动作：建立连接，接收文件，对比checksum，写回硬盘，通知用户文件接收完成

    // 根据protobuf 找到对应的 ip，port，checksum
    string sip = msg->ip();
    int port = msg->dataport();
    // 建立连接
    struct SocketAddress4 addr(sip.c_str(), port);

    TcpClient *pCli = new TcpClient();
    pCli->onConnect(mul_->loop_, true, &addr);

    char *buf = (char *) calloc(MAXLINE, sizeof(char));
    size_t *len = new size_t;

    string sCheckSum = msg->checksum();

    pRecvFileInfo info = new recvFileInfo(
        this,
        client,
        msg->filename().c_str(),
        msg->length(),
        sCheckSum.c_str(),
        buf,
        len,
        pCli);

    pCli->onRecv(buf, len, 0, recvFileCallback, info);
    pCli->onError(handleErro, info);
}

void DTP::handleErro(void *rInfo, int b)
{
    LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DTP, "An error occurred");
    pRecvFileInfo info = static_cast<pRecvFileInfo>(rInfo);

    info->tcpClient_->disConnect();
    delete info->tcpClient_;
    free(info->buf_);
    delete info->len_;
    delete info;
}

char *DTP::getFileRecvAbsolutePath(const char *fileName, char *filePath)
{
    if (fileName == NULL) return NULL;
    strcat(filePath, FILERECVROOT);
    strcat(filePath, fileName);

    return filePath;
}

void DTP::recvFileCallback(void *pthis)
{
    pRecvFileInfo info = static_cast<pRecvFileInfo>(pthis);

    size_t *len = info->len_;
    int length = info->length_;
    char *buf = info->buf_;
    DTP *dtp = info->dtp_;
    DTPClient *client = info->client_;

    char *filePath = (char *) calloc(FILEPATHSIZE, sizeof(char));
    dtp->getFileRecvAbsolutePath(info->fileName_, filePath);

    // 即使没传完也要写进去，因为是append模式，没的问题
    FILE *fp = fopen(filePath, "ab+,css=<UTF-8>");
    if (fp == NULL) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DTP, "fail to open the file");
    }
    free(filePath);

    fwrite(buf, sizeof(char), length, fp);
    fclose(fp);

    if ((*len) == (size_t) length) {
        // 计算md5
        char *newCheckSum = (char *) calloc(MD5LENTH * 2 + 1, sizeof(char));
        char *filePath = (char *) calloc(FILEPATHSIZE, sizeof(char));
        dtp->getFileRecvAbsolutePath(info->fileName_, filePath);
        dtp->getMd5(filePath, newCheckSum);

        // 对比md5
        const char *oldCheckSum = info->checkSum_;
        mnet::ControlMsg *msg = NULL;

        if (strcmp(newCheckSum, oldCheckSum) != 0) {
            // 接收文件出错，由DTPClient通知用户
            // 文件已经写回硬盘，才发现md5不符，需要删掉这个文件
            LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DTP, "Md5 does not match");
            msg = dtp->makeNoticeMsg(RECVFILEFAIL, info->fileName_);

            // 从磁盘上删除这个文件
            unlink(filePath);
        } else {
            msg = dtp->makeNoticeMsg(RECVFILESUCC, info->fileName_);
        }

        free(newCheckSum);

        client->analysisProtobuf(msg);

        delete msg;
    } else {
        // 文件没接收完，要继续收
        memset(buf, 0, sizeof(char) * MAXLINE);
        info->length_ -= (int) (*len);
        info->tcpClient_->onRecv(buf, len, 0, recvFileCallback, info);
    }
}

// sendData 部分
int DTP::sendData(
    TcpConnection *conn,
    int peerId,
    const char *sendBuf,
    int length,
    DTPClient *client,
    char *dataId,
    int flag)
{
    // 启动tcoAcceptor 准备接收链接
    int port;
    char *ip;
    Acceptor *acp = NULL;
    Asynclo *asconn = NULL; // data connection
    // 相关信息保存在一个结构体中 过程结束后释放需要
    pDataListenMgr mgr = new DataListenMgr(
        this, sendBuf, asconn, client, acp, length, NULL, dataId);

    if (flag == TCP) {
        // tcp addr
        struct SocketAddress4 servaddr("0.0.0.0", 0);
        TcpAcceptor *tac = new TcpAcceptor(conn->getLoop());
        tac->start(servaddr);

        struct sockaddr_in localaddr;
        socklen_t len = sizeof(localaddr);
        int ret =
            getsockname(tac->listenfd_, (struct sockaddr *) &localaddr, &len);
        if (ret != 0) { printf("getsockname error\n"); }
        port = ntohs(localaddr.sin_port);
        acp = tac;
        TcpConnection *conn = new TcpConnection();
        tac->onAccept(conn, NULL, NULL, acceptorCallbackOfSendData, mgr);
        asconn = conn;
    } else if (flag == RDMA) {
        // rdma addr ;fix port
        // qp配置
        static RdmaQpConfig conf(8, 8);
        SocketAddress4 servaddr("0.0.0.0", SERV_PORT);
        RdmaAcceptor *rac = new RdmaAcceptor(mul_->loop_, conf);
        int ret = rac->start(&servaddr);
        if (ret != S_OK) printf("rdma starts failed");

        acp = rac;
        RdmaConnection *rconn = new RdmaConnection(mul_->loop_, conf);
        // register the memory
        rconn->registerMemory((void *) sendBuf, length);
        rac->onAccept(rconn, acceptorCallbackOfSendData, mgr);
        asconn = rconn;
        port = SERV_PORT;
    }

    mgr->conn_ = asconn;
    mgr->acceptor_ = acp;

    // 启动定时器
    mul_->loop_->runAfter(WAITTIMEOUT, stopListenOfData, mgr);

    // 计算数据 的checksum CRC
    unsigned short crc = 0;
    crc = getCRC16_CCITT(
        reinterpret_cast<unsigned char *>((char *) sendBuf), length);

    ip = (char *) calloc(INET_ADDRSTRLEN, sizeof(char));

    // FIXME: 这里伪装一个rdma ip, 还需要进一步处理
    strcpy(ip, RDMA_IP);

    if (flag == RDMA)
        flag = SENDDATAONRDMA;
    else if (flag == TCP)
        flag = SENDDATAFLAG;
    // 创建控制报文 fix ipaddress
    // TCP的ip不用给，但是rdma的必须给
    mnet::ControlMsg *msg =
        makeDataControlMsg(flag, peerId, ip, port, crc, length, dataId);

    // 通过Mul发送buf
    std::string *s = new std::string;
    (*msg).SerializeToString(s);

    mul_->sendMessage(conn, 0, s);

    delete msg;
    delete ip;

    return S_OK;
}

void DTP::stopListenOfData(void *manager)
{
    LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DTP, "time out, stop listen");
    pDataListenMgr mgr = static_cast<pDataListenMgr>(manager);

    if (mgr->acceptor_ != NULL) {
        delete mgr->acceptor_;
        delete mgr->conn_;
    }
}

int DTP::sendDataOnUdp(
    TcpConnection *conn,
    int peerId,
    const char *sendBuf,
    int length,
    DTPClient *client,
    char *dataId)
{
    // step one: save the related info
    UdpInfo *param = new UdpInfo((char *) sendBuf, length, client);
    // save for the step two
    udpDataMp_.insert(std::make_pair(dataId, param));

    // 计算数据 的checksum CRC
    unsigned short crc = 0;
    crc = getCRC16_CCITT(
        reinterpret_cast<unsigned char *>((char *) sendBuf), length);

    // 创建控制报文 fix ipaddress
    mnet::ControlMsg *msg =
        makeUdpControlMsg(SENDDATAONUDP, peerId, crc, length, dataId);

    // 通过Mul发送buf
    std::string *s = new std::string;
    (*msg).SerializeToString(s);

    delete msg;

    mul_->sendMessage(conn, 0, s);

    return S_OK;
}

int DTP::sendDataOnUdp(SocketAddress4 addr, UdpInfo *info)
{
    EventLoop *loop = info->dtpClient_->getEntityConnection()->getLoop();

    // 初始化一个用于发送数据的udpendpoint
    UdpEndpoint *pClient;
    UdpClient *pCli = new UdpClient();

    pClient = pCli->begin(loop, addr);
    pClient->onSend(
        info->sendBuf_,
        info->length_,
        0,
        (struct sockaddr *) &addr,
        sizeof(addr),
        NULL,
        NULL);
    return S_OK;
}

void DTP::acceptorCallbackOfSendData(void *point)
{
    // 拿到相应指针
    pDataListenMgr mgr = static_cast<pDataListenMgr>(point);
    // TcpConnection *conn = static_cast<TcpConnection *>(mgr->conn_);

    // 启动onSend
    mgr->conn_->onSend(
        (char *) (mgr->buffer_), mgr->length_, 0, sendDataCallback, mgr);
}

void DTP::sendDataCallback(void *point)
{
    pDataListenMgr mgr = static_cast<pDataListenMgr>(point);

    // 准备回传的数据
    mnet::ControlMsg *msg =
        mgr->dtp_->makeDataNoticeMsg(SENDDATASUCC, mgr->dataId_);

    // 通知用户 做protobuf，然后交给DTPClient
    mgr->client_->analysisProtobuf(msg);

    // free memory,
    delete (mgr->conn_);
    // 方便超时判断
    mgr->conn_ = NULL;
    delete mgr->acceptor_; // 要是用户需要连续发多个buf的数据咋办？
    mgr->acceptor_ = NULL;
    delete msg;
}

void DTP::InvertUint8(unsigned char *dBuf, unsigned char *srcBuf)
{
    int i;
    unsigned char tmp[4];
    tmp[0] = 0;
    for (i = 0; i < 8; i++) {
        if (srcBuf[0] & (1 << i)) tmp[0] |= 1 << (7 - i);
    }

    dBuf[0] = tmp[0];
}

void DTP::InvertUint16(unsigned short *dBuf, unsigned short *srcBuf)
{
    int i;
    unsigned short tmp[4];
    tmp[0] = 0;

    for (i = 0; i < 16; i++) {
        if (srcBuf[0] & (1 << i)) tmp[0] |= 1 << (15 - i);
    }

    dBuf[0] = tmp[0];
}

// still some job to do
// poly: 0x1021
unsigned short
DTP::getCRC16_CCITT(unsigned char *puchMsg, unsigned int usDataLen)
{
    unsigned short wCRCin = 0x0000;
    unsigned short wCPoly = 0x1021;
    unsigned char wChar = 0;

    while (usDataLen--) {
        wChar = *(puchMsg++);
        InvertUint8(&wChar, &wChar);
        wCRCin ^= (wChar << 8);
        for (int i = 0; i < 8; i++) {
            if (wCRCin & 0x8000)
                wCRCin = (wCRCin << 1) ^ wCPoly;
            else
                wCRCin = wCRCin << 1;
        }
    }
    InvertUint16(&wCRCin, &wCRCin);
    return (wCRCin);
}

// 是不是也可以把用户给的buffer再给他， 方便他处理
mnet::ControlMsg *DTP::makeDataNoticeMsg(int type, const char *dataId)
{
    mnet::ControlMsg *msg = new mnet::ControlMsg;
    // msg->set_isdtp(true);
    msg->set_flag(type);
    msg->set_dataid(dataId);
    // msg->set_filename(fileName);

    return msg;
}

mnet::ControlMsg *DTP::makeDataControlMsg(
    int type,
    int id,
    char *ip,
    int dataport,
    int crc,
    int length,
    char *dataId)
{
    // flag ,id,  ip，      dataport, checksum,           length
    // int  ,int, char[15], int,      unsigned char[16],   int

    mnet::ControlMsg *msg = new mnet::ControlMsg;
    // msg->set_isdtp(true);
    msg->set_flag(type); // 区分是什么消息，sendfile senddata
                         // recvfile ercvdata
    msg->set_id(id);
    if (ip != NULL) msg->set_ip(ip);
    msg->set_dataport(dataport);
    msg->set_crcchecksum(crc);
    msg->set_length(length);
    msg->set_dataid(dataId);

    return msg;
}

mnet::ControlMsg *
DTP::makeUdpControlMsg(int type, int id, int crc, int length, char *dataId)
{
    mnet::ControlMsg *msg = new mnet::ControlMsg;
    msg->set_flag(type); // 区分是什么消息，sendfile senddata
                         // recvfile ercvdata, UDP reuse
    msg->set_id(id);
    msg->set_crcchecksum(crc);
    msg->set_length(length);
    msg->set_dataid(dataId);

    return msg;
}

// 要是用户给的buf不够大怎么办？感觉需要告诉用户这个数据的大小呢？假定用户给的
// buffer大小和数据的长度一样
void DTP::agreeRecvData(mnet::ControlMsg *msg, DTPClient *client)
{
    // 动作：建立连接，接收数据，对比checksum，通知用户数据接收完成

    // 根据protobuf 找到对应的port，checksum
    // get ip with tcpconnection
    Asynclo *conn = client->getEntityConnection();
    SocketAddress4 *peeraddr;
    peeraddr = conn->getPeerAddr();
    int port = msg->dataport();

    int length = msg->length();
    char *recvbuf = new char[length];
    memset(recvbuf, 0, length);
    size_t *buflen = new size_t((size_t) length);

    // 建立连接
    RdmaClient *rdmaclient;
    TcpClient *pCli = NULL;
    peeraddr->setPort(port);
    if (msg->flag() == SENDDATAONRDMA) {
        RdmaQpConfig conf(8, 8);
        rdmaclient = new RdmaClient(mul_->loop_, conf);
        rdmaclient->initCmChannel(peeraddr);
    } else {
        pCli = new TcpClient();
        pCli->onConnect(mul_->loop_, true, peeraddr);
    }

    int *dataLength = new int(msg->length());
    unsigned short crcCheckSum = msg->crcchecksum();

    // 回调处理需要的信息
    pRecvDataInfo info = new recvDataInfo(
        this,
        client,
        recvbuf,
        msg->length(),
        dataLength,
        crcCheckSum,
        buflen,
        pCli,
        msg->dataid().c_str());

    // 有可能只收到一部分数据，就不好处理了
    if (msg->flag() == SENDDATAONRDMA) {
        info->temp_ = rdmaclient;
        rdmaclient->onConnect(recvMessage, info);
        rdmaclient->onError(handleSdErro, info);
    } else {
        // Asynclo *conn = client -> getEntityConnection();
        pCli->onRecv(recvbuf, buflen, 0, recvDataCallback, info);
        pCli->onError(handleSdErro, info);
    }
}

void DTP::recvMessage(void *point)
{
    pRecvDataInfo info = static_cast<pRecvDataInfo>(point);
    RdmaClient *client = static_cast<RdmaClient *>(info->temp_);

    client->registerMemory(info->recvBuf_, info->dataSize_);

    client->onRecv(info->recvBuf_, info->bufLen_, 0, recvDataCallback, point);
}

void DTP::agreeRecvUdpData(mnet::ControlMsg *msg, DTPClient *client)
{
    // bind addr, onrecv data, send control message
    Asynclo *conn = client->getEntityConnection();

    UdpEndpoint *point = new UdpEndpoint(mul_->loop_);

    SocketAddress4 *peeraddr = new SocketAddress4();
    SocketAddress4 localaddr("0.0.0.0", 0);
    point->start(localaddr);
    socklen_t len = sizeof(localaddr);
    // get the bind port
    int ret = getsockname(
        point->pUdpChannel_->getFd(), (struct sockaddr *) &localaddr, &len);
    if (ret != 0) { printf("getsockname error\n"); }

    int length = msg->length();
    size_t *recvlen = new size_t(length);
    char *recvbuf = new char[length];

    // 回调处理需要的信息
    pRecvDataInfo info = new recvDataInfo(
        this,
        client,
        recvbuf,
        length,
        NULL,
        msg->crcchecksum(),
        recvlen,
        NULL,
        msg->dataid().c_str());
    // need to tell user the data address with cb
    point->onRecv(
        recvbuf,
        recvlen,
        0,
        (struct sockaddr *) peeraddr,
        len,
        recvUdpDataCallback,
        info);

    info->temp_ = point;
    mul_->loop_->runAfter(WAITTIMEOUT, stopListenOfUdp, info);

    // get the bind port
    int port = ntohs(localaddr.sin_port);
    // get the local ip of tcp
    peeraddr = conn->getLocalAddr();
    char ipstr[16];
    peeraddr->getIP(ipstr);

    mnet::ControlMsg *umsg = makeDataControlMsg(
        RECVDATAONUDP,
        msg->id(),
        ipstr,
        port,
        0,
        0,
        (char *) ((msg->dataid()).c_str()));

    // 通过Mul发送buf
    std::string *s = new std::string;
    (*umsg).SerializeToString(s);
    mul_->sendMessage(conn, 0, s);

    delete umsg;
}

void DTP::stopListenOfUdp(void *manager)
{
    LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DTP, "time out, stop listen");
    pRecvDataInfo mgr = static_cast<pRecvDataInfo>(manager);

    if (mgr->temp_ != NULL) {
        // 表明还没有数据发送过来
        delete (UdpEndpoint *) (mgr->temp_);
        delete mgr->recvBuf_;
        mgr->recvBuf_ = NULL;
        delete mgr->bufLen_;
    }
}
// 如果rdma的连接被关掉  没有epoll响应的话 就会出现内存泄露
void DTP::handleSdErro(void *rInfo, int b)
{
    LOG(LOG_ERROR_LEVEL,
        LOG_SOURCE_DTP,
        "error or the opposite closed the conn");
    pRecvDataInfo info = static_cast<pRecvDataInfo>(rInfo);

    if (info->tcpClient_ != NULL) {
        info->tcpClient_->disConnect();
        delete info->tcpClient_;
    }
    delete info->dataLength_;
    delete info->bufLen_;
    delete info;
}

void DTP::recvDataCallback(void *pthis)
{
    //
    RdmaClient *rdmaclient;
    int flag;
    pRecvDataInfo info = static_cast<pRecvDataInfo>(pthis);
    if (info->temp_ != NULL)
        rdmaclient = static_cast<RdmaClient *>(info->temp_);

    size_t *buflen = info->bufLen_;    // 实际接受到的数据大小
    int length = *(info->dataLength_); //	需要接受的数据的大小

    if ((*buflen) == (size_t) length) {
        //
        unsigned short newCheckSum = info->dtp_->getCRC16_CCITT(
            reinterpret_cast<unsigned char *>(
                info->recvBuf_ + length - info->dataSize_),
            info->dataSize_);

        // 对比crc
        unsigned short oldCheckSum = info->checkSum_;
        mnet::ControlMsg *msg;
        if (newCheckSum != oldCheckSum) {
            // TODO: 接收数据出错，由DTPClient通知用户具体哪个消息接受失败
            // 感觉数据需要个id呀  要不然用户也不知道哪个数据没收到
            // 接受失败  就不告诉用户了
            if (info->temp_ != NULL)
                flag = RECVRDMADATAFAIL;
            else
                flag = RECVDATAFAIL;
            msg = info->dtp_->makeDataNoticeMsg(flag, info->dataId_);
            delete info->recvBuf_;
            info->recvBuf_ = NULL;
        } else {
            // 通过datacallback告诉用户数据地址，需要用户释放
            char *buf = info->recvBuf_ + length - info->dataSize_;
            info->client_->dataCallback_(
                RECVDATASUCC, buf, info->client_->dataParam_);
            msg = info->dtp_->makeDataNoticeMsg(RECVDATASUCC, info->dataId_);
        }

        info->client_->analysisProtobuf(msg);

        // free memory 感觉rdma需要在这里面释放内存
        // handlesderro do it
        // info->tcpClient_->disConnect();
        // delete info -> tcpClient_;
        // delete info -> dataLength_;
        // delete info;

    } else {
        // 文件没接收完，要继续收
        // 记录未收到的数据长度
        info->dataLength_ -= *buflen;
        info->recvBuf_ += *buflen;
        if (info->temp_ != NULL)
            rdmaclient->onRecv(
                info->recvBuf_, info->bufLen_, 0, recvDataCallback, info);
        else
            info->client_->getEntityConnection()->onRecv(
                info->recvBuf_, info->bufLen_, 0, recvDataCallback, info);
    }
}

void DTP::recvUdpDataCallback(void *pthis)
{
    //
    DTP::pRecvDataInfo info = static_cast<DTP::pRecvDataInfo>(pthis);

    size_t *buflen = info->bufLen_;       // 实际接受到的数据大小
    int length = (int) (info->dataSize_); //	需要接受的数据的大小
    mnet::ControlMsg *msg;

    if ((*buflen) == (size_t) length) {
        //
        unsigned short newCheckSum = info->dtp_->getCRC16_CCITT(
            reinterpret_cast<unsigned char *>(info->recvBuf_), length);

        // 对比crc
        unsigned short oldCheckSum = info->checkSum_;
        if (newCheckSum != oldCheckSum) {
            // TODO: 接收数据出错，由DTPClient通知用户具体哪个消息接受失败
            msg = info->dtp_->makeDataNoticeMsg(RECVUDPDATAFAIL, info->dataId_);
        } else {
            // 通过datacallback告诉用户数据地址，需要用户释放
            char *buf = info->recvBuf_;
            info->client_->dataCallback_(
                RECVDATASUCC, buf, info->client_->dataParam_);
            msg = info->dtp_->makeDataNoticeMsg(RECVDATASUCC, info->dataId_);
        }

    } else {
        // 文件没接收完，表示失败
        msg = info->dtp_->makeDataNoticeMsg(RECVUDPDATAFAIL, info->dataId_);
        // 接收失败，释放buf
        delete info->recvBuf_;
        info->recvBuf_ = NULL;
    }
    info->client_->analysisProtobuf(msg);

    // free memory
    delete info->bufLen_;
    delete (UdpEndpoint *) (info->temp_);
    info->temp_ = NULL;
    delete info;
    delete msg;
}

int DTP::getLocalIp(char *addressBuffer)
{
    struct ifaddrs *ifAddrStruct = NULL;
    void *tmpAddrPtr = NULL;
    getifaddrs(&ifAddrStruct);
    bool find = false;
    while (ifAddrStruct != NULL && !find) {
        if (strcmp(ifAddrStruct->ifa_name, "lo") == 0) {
            // 跳过本地地址
        } else if (ifAddrStruct->ifa_addr->sa_family == AF_INET) {
            // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr =
                &((struct sockaddr_in *) ifAddrStruct->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            find = true;
        }
        // 下面是ipv6地址，现在用不到
        // else if (ifAddrStruct->ifa_addr->sa_family == AF_INET6) {
        // check it is IP6
        //     // is a valid IP6 Address
        //     tmpAddrPtr =
        //         &((struct sockaddr_in
        //         *)ifAddrStruct->ifa_addr)->sin_addr;
        //     char addressBuffer[INET6_ADDRSTRLEN];
        //     inet_ntop(AF_INET6, tmpAddrPtr,
        //     addressBuffer,INET6_ADDRSTRLEN); printf("%s IP Address %s\n",
        //     ifAddrStruct->ifa_name, addressBuffer);
        // }
        ifAddrStruct = ifAddrStruct->ifa_next;
    }

    return S_OK;
}
