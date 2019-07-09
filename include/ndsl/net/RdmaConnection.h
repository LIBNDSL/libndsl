/*
 * @file RdmaConnection.h
 * @brief
 * RdmaTransferChannel:响应QP中的事件的channel
 * RdmaConnection:管理QP中的事件,传输数据
 *
 * rdma_cm和ibverbs分别会创建一个fd，这两个fd的分工不同。
 * rdma_cm fd主要用于通知建连相关的事件，verbs fd则主要通知有新的cqe发生。
 * 当直接对rdma_cm fd进行poll/epoll监听时，此时只能监听到POLLIN事件，
 * 这意味着有rdma_cm事件发生。当直接对verbs fd
 * 进行poll/epoll监听时，同样只能监听到POLLIN事件，这意味着有新的cqe。
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#ifndef __NDSL_NET_RDMACONNECTION_H__
#define __NDSL_NET_RDMACONNECTION_H__

#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>
#include <map>
#include <list>
#include "ndsl/net/Channel.h"
#include "ndsl/net/BaseChannel.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/Asynclo.h"
#include "ndsl/net/RdmaAcceptor.h"

// 若把两个fd放在一个channel中,是否可行?
// 不可行,epoll注册的时候 per channel per fd

namespace ndsl {
namespace net {

class RdmaTransferChannel : public BaseChannel
{
  private:
  public:
    RdmaTransferChannel(int fd, EventLoop *loop)
        : BaseChannel(fd, loop)
    {}
    ~RdmaTransferChannel() { erase(); }
};

// RDMA Connection 用于管理队列对中的事件,用于传输数据
class RdmaConnection : public Asynclo
{
  public:
    // 用于记录内存区域的相关信息
    struct MemoryReigon
    {
        struct ibv_mr *mr;
        void *addr;
        uint64_t size;
    };

  private:
    // 内存地址与内存区域信息的映射
    using MemoryReigonMap = std::map<void *, MemoryReigon *>;
    // 插入键值的pair
    using MemoryReigonMapKV = std::pair<void *, MemoryReigon *>;

    // 记录send和recv的信息
    struct saveInfo
    {
        Callback cb;    // 回调函数
        void *param;    // 回调函数的参数
        uint64_t wr_id; // post发送或接收请求的wr_id
        void *buffer;   // 注册的buffer
        size_t len;     // 注册buffer的长度
    };

    // 完成事件处理队列
    using wcHandlerQueue = std::list<saveInfo *>;
    // 拉取cq的数据类型
    using wcLists = std::list<struct ibv_wc>;

    // client继承
  protected:
    EventLoop *loop_;       // 绑定eventloop
    RdmaQpConfig conf_;     // 记录QP配置
    struct rdma_cm_id *id_; // cm的socket语义抽象

  private:
    RdmaTransferChannel *sendCqChannel_; // send完成队列的channel
    RdmaTransferChannel *recvCqChannel_; // recv完成队列的channel
    MemoryReigonMap mrMap_; // 内存地址与内存区域信息的映射

    // send需要一个队列,在多次调用onSend的时候,需要把buf和callback保存起来
    // 在send事件完成时,从队列中取一个元素,执行callback
    // 1.多次调用onSend之后，若完成顺序不同怎么办？
    //  使用wr_id,和wc的wr_id对比
    wcHandlerQueue sendHandlerQueue;

    // recv和send类似
    // 若事件在用户调用onRecv之前到达，则不提取事件，直到用户调用onRecv的时候
    // 再把事件拉上来。
    // 1.但多次调用onRecv，如何保存？不能保证事件到达顺序
    //  使用wr_id,执行回调之前需要对比完成任务wc的wr_id和发送的wr_id
    wcHandlerQueue recvHandlerQueue;

  public:
    // 被动连接
    RdmaConnection(EventLoop *loop, RdmaQpConfig &conf);
    // 主动连接
    RdmaConnection(EventLoop *loop, RdmaQpConfig &conf, struct rdma_cm_id *id);
    ~RdmaConnection();

    // 注册内存区域
    ibv_mr *registerMemory(void *addr, uint64_t len, bool atomic = false);
    // 取消注册内存区域
    int deregisterMemory(void *addr);

    // 获取communication manager相关channel的fd
    int getCmChannelFd() const
    {
        return id_ == NULL ? S_FALSE : id_->channel->fd;
    }

    // 获取数据传输相关的fd
    int getRecvChannelFd() const
    {
        return id_ == NULL ? S_FALSE : id_->recv_cq_channel->fd;
    }

    // 获取数据传输相关的fd
    int getSendChannelFd() const
    {
        return id_ == NULL ? S_FALSE : id_->send_cq_channel->fd;
    }

    // NOTE:被动连接,根据event建立QPchannel
    int createPassiveConnection(struct rdma_cm_event *event);
    // 主动建立连接
    int createActiveConnection();

    // 获取本端地址
    SocketAddress4 *getLocalAddr()
    {
        return (SocketAddress4 *) rdma_get_local_addr(id_);
    }
    // 获取对端地址
    SocketAddress4 *getPeerAddr()
    {
        return (SocketAddress4 *) rdma_get_peer_addr(id_);
    }

    // 获取本端的port
    uint16_t getSrcPort() { return rdma_get_src_port(id_); }
    // 获取对端的port
    uint16_t getDstPort() { return rdma_get_dst_port(id_); }

    EventLoop *getLoop() { return loop_; }

    // 注册错误回调函数
    int onError(ErrorHandle cb, void *param = NULL);

    // 异步接收数据,完成事件在handleRead响应
    int onRecv(
        char *buf,
        size_t *len,
        int flags = 0,
        Callback cb = NULL,
        void *param = NULL);

    // 异步发送数据,完成事件不响应
    int onSend(
        char *buf,
        size_t len,
        int flags = 0,
        Callback cb = NULL,
        void *param = NULL);

    // 原子操作
    // 1.检查设备支持的原子操作等级：device_attr.atomic_cap
    // 2.远端地址是8位对齐的
    // 3.修改qp，使其支持原子操作: ibv_modify_qp()/
    //  ibv_qp_attr.qp_access_flags = IBV_ACCESS_REMOTE_ATOMIC(注册内存时设置)
    // 4.使用RC的QP(只有RC的QP才支持原子操作)

    // client call
    int FetchAndAdd(uint64_t *buf, struct ibv_mr *remote_mr, uint64_t add);

    // client call
    int CmpAndSwp(
        uint64_t *buf,
        struct ibv_mr *remote_mr,
        uint64_t expected,
        uint64_t swap);

    // 被动端接受连接请求
    int accept(struct rdma_conn_param *param = NULL);
    // 被动关闭连接
    void shutdown();

    // 主动关闭连接
    int disconnect();

  private:
    // 注册给Channel的回调函数
    static int handleSend(void *pThis);
    static int handleRecv(void *pThis);

    // 获取wc,最多max个
    int pollIncomingData(struct ibv_comp_channel *compChannel, wcLists &wcList);

    // 设置非阻塞
    void setNonblocking(int fd);

    // 用于添加到eventLoop的函数
    static void destroyConnection(void *pThis);
}; // namespace net

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_RDMACONNECTION_H__