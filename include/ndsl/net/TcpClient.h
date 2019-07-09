/**
 * @file TcpClient.h
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#ifndef __NDSL_NET_TCPCLIENT_H__
#define __NDSL_NET_TCPCLIENT_H__
#include "EventLoop.h"
#include "Asynclo.h"
#include "TcpConnection.h"

namespace ndsl {
namespace net {

class TcpClient : public TcpConnection
{
  public:
    TcpClient();
    ~TcpClient();

    int sockfd_;          // 保存sockfd
    // TcpConnection *conn_; // 保存用于释放内存

    // 与服务器建立连接
    int onConnect(
        EventLoop *loop,
        bool isConnNoBlock,
        struct SocketAddress4 *servaddr);

    // 与服务器断开链接
    int disConnect();

    // // 重载基类函数 全部交给conn处理
    // int onError(ErrorHandle cb, void *param)
    // {
    //     return conn_->onError(cb, param);
    // }

    // int onRecv(char *buffer, size_t *len, int flags, Callback cb, void
    // *param)
    // {
    //     return conn_->onRecv(buffer, len, flags, cb, param);
    // }

    // int onSend(char *buf, size_t len, int flags, Callback cb, void *param)
    // {
    //     return conn_->onSend(buf, len, flags, cb, param);
    // }

    // EventLoop *getLoop() { return conn_->getLoop(); }
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_TCPCLIENT_H__
