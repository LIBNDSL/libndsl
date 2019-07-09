////
// @file UnixConnection.h
// @brief
// fengzhuang unixconnetion
//
// @author ranxiangjun
// @email ranxianshen@gmail.com
//
#ifndef __NDSL_NET_UNIXCONNECTION_H__
#define __NDSL_NET_UNIXCONNECTION_H__
#include <queue>
#include <sys/socket.h>
#include "ndsl/config.h"

namespace ndsl {
namespace net {

class UnixChannel;
class EventLoop;
class UnixAcceptor;

class UnixConnection
{
  public:
    using Callback = void (*)(void *);      // Callback 函数指针原型
    using ErrorHandle = void (*)(int, int); // error callbak function

  private:
    // 用户主动调用onRecv/onSend函数的参数存在这
    typedef struct SInfo
    {
        const void *sendBuf_; // 用户给的写地址
        void *readBuf_;       // 用户给的读地址
        ssize_t *len_;        // buf长度
        int flags_;           // send()的参数
        Callback cb_;         // 存储用户传过来的回调函数
        void *param_;         // 回调函数的参数
        ssize_t offset_;      // 一次没发送完的发送偏移
    } Info, *pInfo;

    std::queue<pInfo> qSendInfo_; // 等待发送的队列
    Info RecvInfo_;

    // svae Acceptor UnixChannel
    UnixAcceptor *pUnixAcceptor_;

    UnixChannel *pUnixChannel_;

    // calback function to deal with error
    ErrorHandle errorHandle_;

  public:
    UnixConnection();
    UnixConnection(UnixAcceptor *unixAcceptor);
    ~UnixConnection();

    static int handleRead(void *);
    static int handleWrite(void *);

    // create a channel
    int createChannel(int sockfd_, EventLoop *pLoop);

    // error summary, register callback function of error
    int onError(ErrorHandle cb);

    // onSend onRecv 的语义是异步通知
    int onRecv(char *buffer, ssize_t *len, int flags, Callback cb, void *param);

    // 会有好多人同时调用这个进行send，需要一个队列
    int
    onSend(const void *buf, ssize_t len, int flags, Callback cb, void *param);

    // 正常执行accept的流程
    int onAccept(
        UnixConnection *pCon,
        struct sockaddr *addr,
        socklen_t *addrlen,
        Callback cb,
        void *param);
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_UNIXCONNECTION_H__
