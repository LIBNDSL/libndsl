/**
 * @file Multiplexer.cc
 * @brief
 * 实现功能：
 *  1.绑定connection
 *  2.插入通信实体
 *  3.删除通信实体
 *  4.发送消息
 *  5.分发消息给实体
 *
 * @author zzt
 * @emial 429834658@qq.com
 **/

#include "ndsl/net/Multiplexer.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/TcpChannel.h"
#include "ndsl/utils/Log.h"
#include "ndsl/utils/Endian.h"
#include "ndsl/net/DTPClient.h"
#include "ndsl/net/DTP.h"
#include "ndsl/config.h"
#include "ndsl/utils/Guid.h"

#include <stdio.h>
using namespace std;
namespace ndsl {
namespace net {

Multiplexer::Multiplexer(EventLoop *loop)
    : loop_(loop)
{
    dtp_ = new DTP(this);
}

Multiplexer::~Multiplexer()
{
    // 清理内存
    map<Asynclo *, std::map<int, DTPClient *> *>::iterator it;
    for (it = connMp_.begin(); it != connMp_.end(); it++) {
        map<int, DTPClient *> *mp = it->second;
        delete mp;
    }
}

// 在map中插入<id,DTPClient*>
void Multiplexer::insert(void *pa)
{
    struct para *p = static_cast<struct para *>(pa);
    Multiplexer *mul = p->mul;

    map<Asynclo *, map<int, DTPClient *> *>::iterator it =
        mul->connMp_.find(p->conn);
    if (it == mul->connMp_.end()) {
        // 添加conn和cb 一个conn一个buf
        map<int, DTPClient *> *mp = new map<int, DTPClient *>;
        mp->insert(make_pair(p->id, p->dtpClient));
        mul->connMp_.insert(make_pair(p->conn, mp));

        // 新conn添加进来，new buf
        // TODO: 这个理论上是在conn断开的时候进行释放 但是还没实现
        struct bufInfo *bufinfo = new struct bufInfo(p->mul, p->conn);

        //  conn第一次来的时候 调用onRecv
        p->conn->onRecv(bufinfo->buf, &(bufinfo->len), 0, dispatch, bufinfo);
        p->conn->onError(handleErro, NULL);
    } else {
        // 只添加cb
        map<int, DTPClient *> *mp = it->second;
        mp->insert(make_pair(p->id, p->dtpClient));
    }

    // 调用用户注册的startcb
    if (p->entity->scb_ != NULL) { p->entity->scb_(p->entity->startcbparam_); }

    if (p != NULL) // 释放para
    {
        delete p;
        p = NULL;
    }
}

// 在loop工作队列中加入insert任务
// 实体调用这个函数将自身注册进mul
void Multiplexer::registEntity(
    Asynclo *conn,
    int id,
    Entity *entity,
    DTPClient *client)
{
    struct para *p = new para; // 在insert()中释放
    p->id = id;
    p->entity = entity; // 加入的意义在于需要调其中的scb_
    p->mul = this;
    p->conn = conn;
    p->dtpClient = client;

    // 在eventloop中释放
    utils::WorkItem *w1 = new utils::WorkItem;
    w1->doit = insert;
    w1->param = static_cast<void *>(p);
    // if (conn->getChannel() == NULL)
    //     LOG(LOG_ERROR_LEVEL,
    //         LOG_SOURCE_MULTIPLEXER,
    //         "MULTIPLEXER::ADDINSERTWORK tcpchannel==NULL,cant find loop\n");
    // else
    conn->getLoop()->addWork(w1);
}

// 在map中删除<id,callback>对
void Multiplexer::remove(void *pa)
{
    struct para *p = static_cast<struct para *>(pa);

    std::map<Asynclo *, std::map<int, DTPClient *> *>::iterator it =
        p->mul->connMp_.find(p->conn);

    if (it != p->mul->connMp_.end()) {
        map<int, DTPClient *> *mp = it->second;
        map<int, DTPClient *>::iterator iter = mp->find(p->id);
        if (iter == mp->end()) {
            // 理论上不可能
        } else {
            DTPClient *client = iter->second;
            delete client;
        }

        mp->erase(p->id);
    } else {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_MULTIPLEXER,
            "MULTIPLEXER::REMOVE cant remove the entity, not in the map\n");
    }

    if (p != NULL) // 释放para
    {
        delete p;
        p = NULL;
    }
}

// 在loop工作队列中加入remove任务
void Multiplexer::addRemoveWork(Asynclo *conn, int id)
{
    struct para *p = new para; // 在remove()中释放
    p->conn = conn;
    p->id = id;
    p->mul = this;

    // 在eventloop中释放
    utils::WorkItem *w2 = new utils::WorkItem;
    w2->doit = remove;
    w2->param = static_cast<void *>(p);
    conn->getLoop()->addWork(w2);
}

// 向上层提供发送消息接口
void Multiplexer::sendMessage(Asynclo *conn, int id, string *sprotobuf)
{
    int length = sprotobuf->size();
    const char *data = sprotobuf->c_str();

    int len = sizeof(char) * length + sizeof(struct Message);
    // 在tcpconnection中释放
    char *buffer = (char *) calloc(len, sizeof(char));

    Message *message = reinterpret_cast<struct Message *>(buffer);
    // 消息头中的len为负载长度
    message->id = htobe32(id);
    message->len = htobe32(length);

    memcpy(buffer + sizeof(struct Message), data, length);

    conn->onSend(
        buffer, length + sizeof(struct Message), 0, sendMsgCallback, buffer);

    delete sprotobuf;
}
void Multiplexer::sendMsgCallback(void *buffer)
{
    char *buf = static_cast<char *>(buffer);
    free(buf);
    LOG(LOG_INFO_LEVEL, LOG_SOURCE_MULTIPLEXER, "send control msg success");
}

void Multiplexer::dispatch(void *pthis)
{
    struct bufInfo *info = static_cast<struct bufInfo *>(pthis);
    Multiplexer *mul = info->mul;
    struct incompleteBuf *icbuf = info->incompletebuf;

    // 因为buf里面有一个Message头，打印不出来，可以尝试打印buf+sizeof(struct
    // Message)
    char *buf = info->buf;

    int id;                               // Entity id
    int len;                              // 负载的长度
    int recvLen = info->len;              // 收到的长度
    int offset = 0;                       // buf的偏移量
    int msgSize = sizeof(struct Message); // 正常头的大小
    struct Message *message;

    while (offset < recvLen) {
        // 不管怎样，先填满头
        if (recvLen < (msgSize - icbuf->headSize)) {
            memcpy(icbuf->head + icbuf->headSize, buf + offset, recvLen);
            icbuf->headSize += recvLen;
            offset += recvLen;
        } else {
            memcpy(
                icbuf->head + icbuf->headSize,
                buf + offset,
                (msgSize - icbuf->headSize));
            offset += (msgSize - icbuf->headSize);
            icbuf->headSize = msgSize;
        }

        // 如果头满了，则填充数据
        if (icbuf->headSize == msgSize) {
            message = reinterpret_cast<struct Message *>(icbuf->head);
            id = be32toh(message->id);
            if (id != 0) {
                // 非dtp数据 直接传给用户
                // 通过conn和id找到对应的 dtpClient，然后直接传给用户
                LOG(LOG_INFO_LEVEL,
                    LOG_SOURCE_MULTIPLEXER,
                    "直接传给用户的消息");

                map<Asynclo *, map<int, DTPClient *> *>::iterator it =
                    mul->connMp_.find(info->conn);
                map<int, DTPClient *> *mp = it->second;
                map<int, DTPClient *>::iterator iter = mp->find(id);
                DTPClient *client = iter->second;
                if (NULL != client->dataCallback_)
                    client->dataCallback_(
                        RECVDATAFLAG, info->buf + msgSize, client->dataParam_);

            } else {
                len = be32toh(message->len);
                if (icbuf->buf == NULL) {
                    icbuf->buf = (char *) calloc(len, sizeof(char));
                }

                memcpy(
                    icbuf->buf + icbuf->bufSize,
                    buf + offset,
                    len - icbuf->bufSize);
                offset += len - icbuf->bufSize;
                icbuf->bufSize = strlen(icbuf->buf);
            }
        }

        // 如果数据也满了，则进行处理
        if (icbuf->bufSize == len) {
            mul->solve(info->mul, info->conn, icbuf->buf);

            // 清除所有状态
            memset(icbuf->head, 0, sizeof(char) * icbuf->headSize);
            icbuf->headSize = 0;
            free(icbuf->buf);
            icbuf->buf = NULL;
            icbuf->bufSize = 0;
        }
    }

    // 再次调用onRecv 只要conn在，就不会停
    memset(info->buf, 0, sizeof(char) * MAXLINE);
    info->conn->onRecv(info->buf, &(info->len), 0, dispatch, info);
}

// 这里的buf只是数据部分，没有头
void Multiplexer::solve(Multiplexer *mul, Asynclo *conn, char *buf)
{
    // 拆包找到对应DtpClient的cb，分发
    map<Asynclo *, map<int, DTPClient *> *>::iterator it =
        mul->connMp_.find(conn);
    // 拿到Entity的id
    mnet::ControlMsg *msg = new mnet::ControlMsg;
    std::string s = buf;
    msg->ParseFromString(s);
    int id = msg->id();
    if (it == mul->connMp_.end()) {
        // 理论上不会找不到
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_MULTIPLEXER,
            "找不到与当前conn对应的map");

    } else {
        map<int, DTPClient *> *mp = it->second;
        map<int, DTPClient *>::iterator iter = mp->find(id);

        if (iter == mp->end()) {
            // 理论上不会找不到
            LOG(LOG_ERROR_LEVEL,
                LOG_SOURCE_MULTIPLEXER,
                "找不到对应的DTPClient");
        } else {
            DTPClient *client = iter->second;
            client->analysisProtobuf(msg);
        }
    }
}

void Multiplexer::agreeRecvFile(mnet::ControlMsg *msg, DTPClient *client)
{
    dtp_->agreeRecvFile(msg, client);
}

void Multiplexer::agreeRecvData(mnet::ControlMsg *msg, DTPClient *client)
{
    // size_t *length = new size_t((size_t)bufLen);
    if (msg->flag() == SENDDATAONUDP)
        dtp_->agreeRecvUdpData(msg, client);
    else
        dtp_->agreeRecvData(msg, client);
}

void Multiplexer::sendFile(
    TcpConnection *conn,
    int peerId,
    char *filePath,
    int length)
{
    // 找到对应的dtpclient
    DTPClient *client;
    map<Asynclo *, map<int, DTPClient *> *>::iterator it = connMp_.find(conn);

    if (it != connMp_.end()) {
        map<int, DTPClient *> *mp = it->second;
        map<int, DTPClient *>::iterator iter = mp->find(peerId);
        if (iter != mp->end()) { client = iter->second; }
    } else {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_MULTIPLEXER, "没找到对应DTPclient");
    }

    dtp_->sendFile(conn, peerId, filePath, length, client);
}
void Multiplexer::recvFile(TcpConnection *conn, int peerId, char *filename)
{
    // 找到对应的dtpclient
    DTPClient *client;
    map<Asynclo *, map<int, DTPClient *> *>::iterator it = connMp_.find(conn);

    if (it != connMp_.end()) {
        map<int, DTPClient *> *mp = it->second;
        map<int, DTPClient *>::iterator iter = mp->find(peerId);
        if (iter != mp->end()) { client = iter->second; }
    } else {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_MULTIPLEXER, "没找到对应DTPclient");
    }

    dtp_->recvFile(conn, peerId, filename, client);
}

// 由于有重传的可能，需要用户保留buf和dataid
void Multiplexer::sendData(
    TcpConnection *conn,
    int peerId,
    const char *sendBuf,
    int length,
    char *dataId,
    int flag)
{
    // 找到对应的dtpclient
    DTPClient *client;
    map<Asynclo *, map<int, DTPClient *> *>::iterator it = connMp_.find(conn);

    if (it != connMp_.end()) {
        map<int, DTPClient *> *mp = it->second;
        map<int, DTPClient *>::iterator iter = mp->find(peerId);
        if (iter != mp->end()) { client = iter->second; }
    } else {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_MULTIPLEXER, "find no matched DTPclient");
    }

    // 避免用户分配id
    if (dataId == NULL) {
        ndsl::utils::Guid g;
        char *str = new char[32];
        g.generate();
        g.toString(str);
        dataId = str;
    }
    if (flag == TCP || flag == RDMA) {
        if (sendBuf == NULL) {
            // 重发请求
            // 根据dataid找到buffer
            std::unordered_map<string, string>::iterator iter =
                client->dataIdMp_.find(dataId);
            if (iter != client->dataIdMp_.end()) {
                sendBuf = iter->second.c_str();
            }
        }
        client->dataIdMp_.insert(std::make_pair(dataId, sendBuf));
        dtp_->sendData(conn, peerId, sendBuf, length, client, dataId, flag);
    } else
        dtp_->sendDataOnUdp(conn, peerId, sendBuf, length, client, dataId);
}

} // namespace net
} // namespace ndsl
