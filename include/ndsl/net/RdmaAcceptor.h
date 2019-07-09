/*
 * @file RdmaAcceptor.h
 * @brief
 * 包含RdmaCmChannel和RdmaAcceptor
 *
 * RdmaCmChannel:RDMA管理连接的channel
 * RdmaAcceptor:管理RDMA连接
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#ifndef __NDSL_NET_RDMAACCEPTOR_H__
#define __NDSL_NET_RDMAACCEPTOR_H__
#include <unistd.h>
#include <rdma/rdma_cma.h>

#include "ndsl/net/SocketAddress.h"
#include "ndsl/net/BaseChannel.h"
#include "ndsl/net/Acceptor.h"
#include "ndsl/net/Asynclo.h"
#include "ndsl/utils/Error.h"
#include "ndsl/utils/Log.h"

namespace ndsl {
namespace net {

class RdmaConnection;

// RDMA管理建立连接的channel
class RdmaCmChannel : public BaseChannel
{
  public:
    RdmaCmChannel(int fd, EventLoop *loop)
        : BaseChannel(fd, loop)
    {}
    ~RdmaCmChannel() { erase(); }
};

// 用于设置队列对(queue pair)的最大工作请求(work request)数
struct RdmaQpConfig
{
    size_t max_pending_send_req;
    size_t max_pending_recv_req;

    // 默认构造函数
    RdmaQpConfig() {}

    RdmaQpConfig(size_t maxSend, size_t maxRecv)
        : max_pending_send_req(maxSend)
        , max_pending_recv_req(maxRecv)
    {}
};

// 管理RDMA连接
class RdmaAcceptor:public Acceptor
{
  public:
    using AcceptorCallback = void (*)(void *);
    using RecvCallback = void (*)(void *);

    struct Info
    {
        bool valid;           // 标记此info是否有效
        bool isAccepted;      // 是否已经建连
        RdmaConnection *rc;   // accept的connection
        AcceptorCallback aCb; // 完成acceptor之后的回调函数
        void *acceptorParam;  // acceptor回调函数的参数
    };

  private:
    EventLoop *loop_;          // 记录eventloop
    RdmaQpConfig conf_;        // 创建新的队列对的设置
    RdmaCmChannel *cmChannel_; // 管理连接的channel
    struct rdma_cm_id *id_;    // cm的socket语义抽象
    Info info_; // 为完成onAcceptor异步语义,存储RdmaConnecion的相关信息

  public:
    RdmaAcceptor(EventLoop *loop, RdmaQpConfig conf);
    ~RdmaAcceptor();

    // 启动
    int start(const SocketAddress4 *addr);

    // 注册accept需要的RdmaConnection相关信息
	// param two and three should be set as NULL
    void
    onAccept(RdmaConnection *rc,
         AcceptorCallback aCb, void *acceptorParam);

    // 响应回调函数
    static int handleRead(void *pThis);

    // 获取建立好的连接,以便在callback中获取connection
    RdmaConnection *getAcceptedConnection();

    // 获取绑定的port
    uint16_t getSrcPort() { return rdma_get_src_port(id_); }

  private:
    // 设置非阻塞
    void setNonblocking(int fd);
    // 获取事件
    int getCmEvent(struct rdma_cm_event **ev);

    // 监听
    int listen(int backlog = 64);

    // 建立新的连接
    int establishConection(struct rdma_cm_event *ev);
};

} // namespace net
} // namespace ndsl

#endif
