/*
 * @file DTP.h
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#ifndef __NDSL_NET_DTP_H__
#define __NDSL_NET_DTP_H__
#include "TcpConnection.h"
#include "Multiplexer.h"
#include "DTPClient.h"
#include "Acceptor.h"
#include "ndsl/config.h"
#include "TimeWheel.h"

namespace ndsl {
namespace net {

class DTPClient;
class TcpAcceptor;
class Sendfile;
class TcpClient;
// class TimeWheel;
class SocketAddress4;

class DTP
{
  private:
    Multiplexer *mul_;
    // DTPClient *dtpClient_;

    // 动态侦听管理 TODO: 超时还没加
    typedef struct ListenMgr
    {
        ListenMgr(
            DTP *dtp,
            char *fp,
            Asynclo *p,
            DTPClient *client,
            TcpAcceptor *ac,
            int length
            // ,TimeWheel *time
            )
            : dtp_(dtp)
            , conn_(p)
            , client_(client)
            , acceptor_(ac)
        // , time_(time)
        {
            filePath_ = (char *) calloc(FILEPATHSIZE, sizeof(char));
            strcpy(filePath_, fp);
        }
        ~ListenMgr() { free(filePath_); }
        DTP *dtp_;
        char *filePath_;
        Asynclo *conn_; // 在sendFile语义里面是TcpConnection指针
        DTPClient *client_;

        TcpAcceptor *acceptor_; // 保存，用于最后的销毁
        // TimeWheel *time_;
    } listenMgr, *pListenMgr;

    typedef struct DataListenMgr
    {
        DataListenMgr(
            DTP *dtp,
            const char *buf,
            Asynclo *conn,
            DTPClient *client,
            Acceptor *ac,
            int length,
            TimeWheel *time,
            char *dataid)
            : dtp_(dtp)
            , buffer_(buf)
            , conn_(conn)
            , client_(client)
            , acceptor_(ac)
            , length_(length)
            , time_(time)
            , dataId_(dataid)
        {}
        ~DataListenMgr() {}
        DTP *dtp_;
        const char *buffer_; //  buffer address in senddata
        Asynclo *conn_;      
        DTPClient *client_;

        Acceptor *acceptor_; // 保存，用于最后的销毁
        int length_;
        TimeWheel *time_;
        char *dataId_;
    } dataListenMgr, *pDataListenMgr;
    typedef struct RecvFileInfo
    {
        RecvFileInfo(
            DTP *dtp,
            DTPClient *client,
            const char *fn,
            int length,
            const char *checkSum,
            char *buf,
            size_t *len,
            // TcpConnection *conn,
            TcpClient *tcpClient
            // ,TimeWheel *time
            )
            : dtp_(dtp)
            , client_(client)
            , fileName_(fn)
            , length_(length)
            , buf_(buf)
            , len_(len)
            // , conn_(conn)
            , tcpClient_(tcpClient)
        // , time_(time)
        {
            if (checkSum) {
                checkSum_ = (char *) calloc(MD5LENTH * 2 + 1, sizeof(char));
                strcpy(checkSum_, checkSum);
            }
        }
        ~RecvFileInfo() { free(checkSum_); }
        DTP *dtp_;
        DTPClient *client_;
        const char *fileName_;
        int length_;
        char *checkSum_;
        char *buf_;
        size_t *len_;
        // TcpConnection *conn_;
        TcpClient *tcpClient_;
        // TimeWheel *time_;
    } recvFileInfo, *pRecvFileInfo;

    typedef struct RecvDataInfo
    {
        RecvDataInfo(
            DTP *dtp,
            DTPClient *client,
            char *recvBuf,
            const int dataSize, // need when check the crc
            int *dataLength,
            const unsigned short checkSum,
            size_t *bufLen,
            TcpClient *tcpClient,
            const char *dataId)
            : dtp_(dtp)
            , client_(client)
            , recvBuf_(recvBuf)
            , dataSize_(dataSize)
            , dataLength_(dataLength)
            , checkSum_(checkSum)
            , bufLen_(bufLen)
            , tcpClient_(tcpClient)
            , dataId_(dataId)
        {
            temp_ = NULL;
        }
        ~RecvDataInfo() {}
        DTP *dtp_;
        DTPClient *client_;
        char *recvBuf_;
        const int dataSize_;
        int *dataLength_;
        const unsigned short checkSum_;
        size_t *bufLen_;
        TcpClient *tcpClient_;
        const char *dataId_;
        void *temp_; // save the rdmaclient, or save udpendpoint for free
    } recvDataInfo, *pRecvDataInfo;

  public:
    // TODO: 可进行传输协议的选择 tcp udp rdma ?
    DTP(Multiplexer *m);
    // 保存udpinfo和dataid
    struct UdpInfo
    {
        UdpInfo(char *sendbuf, int length, DTPClient *dtpclient)
            : sendBuf_(sendbuf)
            , length_(length)
            , dtpClient_(dtpclient)
        {}
        ~UdpInfo() {}
        char *sendBuf_;
        int length_;
        DTPClient *dtpClient_;
    };
    std::unordered_map<std::string, UdpInfo *> udpDataMp_;
    ~DTP() {}

    // sendFile标准接口
    int sendFile(
        TcpConnection *conn,
        int peerId,
        char *filePath,
        int length,
        DTPClient *client);

    // sendData标准接口
    int sendData(
        TcpConnection *conn,
        int peerId,
        const char *sendBuf,
        int length,
        DTPClient *client,
        char *dataId,
        int flag);

    // recvFile标准接口
    int recvFile(
        TcpConnection *conn,
        int peerId,
        char *filename,
        DTPClient *client);

    //保存定时器和DTPClient的对应，主要是recvFile时，
    //通过定时处理文件不存在情况
    std::map<DTPClient *, TimeWheel::Task *> timeMp_;

    // 对面连接过来了，执行相应动作
    static void acceptorCallback(void *point);
    // sendData
    static void acceptorCallbackOfSendData(void *point);

    // 文件发送完通知用户
    static void sendFileCallback(void *msg);
    //数据发送完 通知用户
    static void sendDataCallback(void *point);

    // 收到文件之后的处理 对比checksum，写回硬盘，通知用户
    static void recvFileCallback(void *pthis);
    // 收到数据之后的处理
    static void recvDataCallback(void *pthis);
    // rdma connect后的处理
    static void recvMessage(void *point);

    // 获取文件md5
    int getMd5(char *filePath, char *result);
    // 获取数据crc
    void InvertUint8(unsigned char *dBuf, unsigned char *srcBuf);
    void InvertUint16(unsigned short *dBuf, unsigned short *srcBuf);
    unsigned short
    getCRC16_CCITT(unsigned char *puchMsg, unsigned int usDataLen);

    // 从文件路径中过滤出文件名
    char *getFileName(char *filePath);

    // 获取第一个非lo的第一个ip
    int getLocalIp(char *addressBuffer);

    // 得到本机存储文件的绝对路径
    char *getFileRecvAbsolutePath(const char *fileName, char *filePath);

    // Entity同意接收文件
    void agreeRecvFile(mnet::ControlMsg *msg, DTPClient *client);
    // Entity同意接收数据
    void agreeRecvData(mnet::ControlMsg *msg, DTPClient *client);

    // step one of sending udpdata
    int sendDataOnUdp(
        TcpConnection *conn,
        int peerId,
        const char *sendBuf,
        int length,
        DTPClient *client,
        char *dataId);
    // step two of sending udpdata
    int sendDataOnUdp(SocketAddress4 addr, UdpInfo *info);
    // the action after recving data
    static void recvUdpDataCallback(void *pthis);
    // Entity agree with recving data
    void agreeRecvUdpData(mnet::ControlMsg *msg, DTPClient *client);

    // 创建通知报文
    mnet::ControlMsg *makeNoticeMsg(int type, const char *fileName);
    // 创建数据的通知报文
    mnet::ControlMsg *makeDataNoticeMsg(int type, const char *dataId);

    // 创建控制报文
    mnet::ControlMsg *makeControlMsg(
        int type,
        int id,
        char *ip,
        int dataport,
        char *md5,
        char *filePath,
        int length);

    // 创建数据的控制报文
    mnet::ControlMsg *makeDataControlMsg(
        int type,
        int id,
        char *ip,
        int dataport,
        int crc,
        int length,
        char *dataId);

    // 创建udp的控制报文
    mnet::ControlMsg *
    makeUdpControlMsg(int type, int id, int crc, int length, char *dataId);

    // 发文件完成后清理内存
    static void senfFileClearMemory(void *manager);

    // 错误处理函数 也被动断开链接的时候清理内存
    static void handleErro(void *rInfo, int b);
    // sendData的错误处理函数
    static void handleSdErro(void *rInfo, int b);

    // loop执行的停止监听函数
    static void stopListen(void *manager);
    // sendData相关部分
    static void stopListenOfData(void *manager);
    // sendDataOnUdp相关部分
    static void stopListenOfUdp(void *manager);

    //对端没找到文件的处理函数
    static void handleRecvfileFail(void *para);
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_DTP_H__
