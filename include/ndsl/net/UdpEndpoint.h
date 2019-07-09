////
// @file UdpEndpoint_.h
// @brief
// 
//
// @author lanry
// @email luckylanry@163.com
//
#ifndef __NDSL_NET_UDPENDPOINT_H__
#define __NDSL_NET_UDPENDPOINT_H__
#include <queue>
#include <sys/socket.h>
#include "../utils/Error.h"
#include "UdpChannel.h"
#include "EventLoop.h"

class UdpChannel;

namespace ndsl{
namespace net{

using Callback = void (*)(void *); // Callback 函数指针原型

class  UdpEndpoint
{
  private:
    int sfd_;
    EventLoop *pLoop_;
    Callback cb_;
    void *p_;

  public:
    UdpEndpoint(EventLoop *pLoop);
    ~ UdpEndpoint();
    UdpChannel *pUdpChannel_;   // need the fd 
 
  private:
    // 用户调用sendto/recvfrom函数的参数
    typedef struct SInfo
    {
        void *sendBuf_;               // 用户的发送缓冲区
        void *recvBuf_;                     // 用户的接收缓冲区
        size_t *len_;                        // 缓冲区的大小
        int flags_;                         // sendto和recvfrom的参数
        struct sockaddr *dest_addr_;        // 接收数据的用户主机地址
        struct sockaddr *src_addr_;         // 发送数据的用户主机地址
        socklen_t addrlen_; 
        size_t offset_;                // 地址结构的长度
        Callback cb_;                       // 存储用户传来的回调函数
        void *p_;                    
        bool inUse_;
    
    }Info,*pInfo;
    int createAndBind(struct SocketAddress4 servaddr);

    std::queue<pInfo> SendInfo_; // 发送的数据信息
    Info RecvInfo_;

  public:
 
    // 创建UDP

    int createChannel(int sfd); 

    int start(struct SocketAddress4 servaddr);

    // 事件发生后的处理
    static int handleRead(void *pthis);
    static int handleWrite(void *pthis);
    
    // 注册buffer,起回调作用

    int onRecv(char *buf, size_t *len, int flags,struct sockaddr *src_addr,socklen_t addrlen,Callback cb, void *param); // recv 接收函数
    int onSend(void *buf, size_t len, int flags,struct sockaddr *dest_addr,socklen_t addrlen,Callback cb, void *param); // 用户调用send发送数据
};

} // namespace net
} //namespace ndsl

#endif // __NDSL_NET_UDPENDPOINT_H__
