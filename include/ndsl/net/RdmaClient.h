/*
 * @file RdmaClient.h
 * @brief
 * RDMA client可主动建连
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#ifndef __NDSL_NET_RDMACLIENT_H__
#define __NDSL_NET_RDMACLIENT_H__
#include <rdma/rdma_cma.h>

#include "ndsl/net/RdmaAcceptor.h"
#include "ndsl/net/RdmaConnection.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/net/EventLoop.h"

namespace ndsl {
namespace net {

class RdmaClient : public RdmaConnection
{
  public:
    using ConnectionCallback = void (*)(void *);
    using SendCallback = void (*)(void *);
    using RecvCallback = void (*)(void *);
    using ErrorHandler = void (*)(void *, int);

  private:
    RdmaCmChannel *cmChannel_; // Client中需要一个cm channel管理连接
    ConnectionCallback cb_;    // 完成连接的回调函数
    void *cbParam_;            // cb_的参数
    bool isConnected;          // 是否连接上

  public:
    RdmaClient(EventLoop *loop, RdmaQpConfig &conf);
    ~RdmaClient();

    // 初始化cmChannel
    int initCmChannel(SocketAddress4 *peerAddr);

    // 发起连接,并设置完成连接的回调函数
    int onConnect(
        ConnectionCallback cb,
        void *param,
        struct rdma_conn_param *conn_param = NULL);

  private:
    // 处理Cm事件
    static int handleCmEvent(void *param);

    // 获取Cm事件
    int getCmEvent(struct rdma_cm_event **ev);

    // 设置非阻塞
    void setNonblock(int fd);
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_RDMACLIENT_H__