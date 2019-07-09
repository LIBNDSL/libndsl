/**
 * @file TcpConnection.h
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#ifndef __NDSL_NET_TCPCONNECTION_H__
#define __NDSL_NET_TCPCONNECTION_H__
#include <queue>
#include <stdlib.h>
#include <sys/socket.h>
#include "ndsl/net/Asynclo.h"
#include "ndsl/config.h"

namespace ndsl {
namespace net {

class TcpChannel;
class EventLoop;
class TcpAcceptor;
class Channel;

class TcpConnection : public Asynclo
{
  private:
    // 用于和mul头对其
    struct Message
    {
        TcpConnection *conn; // 传输到对面的时候，对面的conn把这个填上
        int id;              // 通信实体的id
        int len;             // 负载长度
    };

    // 用户主动调用onRecv/onSend函数的参数存在这
    struct SInfo
    {
        SInfo()
            : sendBuf_(NULL)
            , readBuf_(NULL)
            , len_(NULL)
            , flags_(0)
            , cb_(NULL)
            , param_(NULL)
            , offset_(0)
            , recvInUse_(false)
        {}
        SInfo(
            char *sbuf,
            char *rbuf,
            size_t len,
            int flags,
            Callback cb,
            void *p,
            size_t off,
            bool inUse)
            : sendBuf_(sbuf)
            , readBuf_(rbuf)
            , flags_(flags)
            , cb_(cb)
            , param_(p)
            , offset_(off)
            , recvInUse_(inUse)
        {
            len_ = new size_t;
            (*len_) = len;
        }
        char *sendBuf_; // 用户给的写地址
        char *readBuf_; // 用户给的读地址
        size_t *len_;   // buf长度
        int flags_;     // send()的参数在
        Callback cb_;   // 存储用户传过来的回调函数
        void *param_;   // 回调函数的参数
        size_t offset_; // 一次没发送完的发送偏移
        bool recvInUse_; // 判断是否已经使用过一次 Proactor要求每次注册
    };

    typedef struct SInfo Info, *pInfo;

    std::queue<pInfo> qSendInfo_; // 等待发送的队列
    Info RecvInfo_;

  public:
    TcpConnection();
    ~TcpConnection();

    // BaseChannel *getChannel();

    static int handleRead(void *pthis);
    static int handleWrite(void *pthis);

    // TcpChannel的指针 方便Mulpipleter拿
    TcpChannel *pTcpChannel_;

    // 方便上层拿到对应的loop
    EventLoop *getLoop();

    // 新建一个Channel
    int createChannel(int sockfd, EventLoop *pLoop);

    // error汇总 注册error回调函数
    int onError(ErrorHandle cb, void *param);

    // onSend onRecv 的语义是异步通知
    // 需要调用一次 就是起到一个注册buf和回调的作用
    int onRecv(char *buffer, size_t *len, int flags, Callback cb, void *param);

    // 允许多用户同时调用这个进行send，需要一个队列
    int onSend(char *buf, size_t len, int flags, Callback cb, void *param);

    // // 进程之间相互通信
    // int sendMsg(
    //     int sockfd, // 要不要加?
    //     struct msghdr *msg,
    //     int flags,
    //     Callback cb,
    //     void *param);

    // 获取本端地址
    SocketAddress4 *getLocalAddr();
    // 获取对端地址
    SocketAddress4 *getPeerAddr();
    // 获取本端的port
    uint16_t getSrcPort();
    // 获取对端的port
    uint16_t getDstPort();

    // Sendfile相关函数
  private:
    typedef struct SendfileInfo
    {
        int inFd_;
        off_t offset_;
        size_t length_;
        Callback cb_;
        void *param_;
        SendfileInfo(int fd, off_t off, size_t len, Callback cb, void *param)
            : inFd_(fd)
            , offset_(off)
            , length_(len)
            , cb_(cb)
            , param_(param)
        {}
    } sendfileInfo, *pSendfileInfo;

    std::queue<pSendfileInfo> qSendfileInfo_;

  public:
    int sendFile(
        const char *filePath,
        off_t offset,
        size_t length,
        Callback cb,
        void *param);

    int sendFile(int fd, off_t offset, size_t length, Callback cb, void *param);

    // loop 回调函数
    static int sendInLoop(void *pthis);
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_TCPCONNECTION_H__