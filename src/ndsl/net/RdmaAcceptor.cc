/*
 * @file RdmaAcceptor.cc
 * @brief
 * 管理RDMA连接
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include <fcntl.h>
#include <sys/socket.h>
#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>

#include "ndsl/net/RdmaAcceptor.h"
#include "ndsl/net/RdmaConnection.h"

namespace ndsl {
namespace net {

RdmaAcceptor::RdmaAcceptor(EventLoop *loop, RdmaQpConfig conf)
    : loop_(loop)
    , conf_(conf)
    , cmChannel_(NULL)
    , id_(NULL)
{
    info_.valid = false;
    info_.isAccepted = false;
}

RdmaAcceptor::~RdmaAcceptor()
{
    if (id_ != NULL) {
        if (cmChannel_ != NULL) delete cmChannel_;

        // 释放event_channel
        rdma_destroy_event_channel(id_->channel);
        // 释放cm id
        rdma_destroy_id(id_);
    }
}

int RdmaAcceptor::start(const SocketAddress4 *addr)
{
    int ret;
    struct rdma_event_channel *channel;

    // 创建event通道,用于导引rdma_cm_id事件
    channel = rdma_create_event_channel();
    if (channel == NULL) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMAACCEPTOR,
            "rdma create event channel error.");
        goto CREATE_CHANNEL_FAILED;
    }

    // 设置非阻塞
    setNonblocking(channel->fd);

    // 建立基于可靠连接的通信,这是cm的socket的语义抽象
    ret = rdma_create_id(channel, &id_, NULL, RDMA_PS_TCP);
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMAACCEPTOR, "rdma create id error.");
        goto CREATE_ID_FAILED;
    }

    // 绑定设备地址
    ret = rdma_bind_addr(id_, (struct sockaddr *) addr);
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMAACCEPTOR, "rdma create id error.");
        goto CREATE_ADDR_FAILED;
    }

    id_->context = this;

    ret = listen();
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMAACCEPTOR, "rdma listen error.");
        goto CREATE_LISTEN_FAILED;
    }

    // 实例化cmChannel
    cmChannel_ = new RdmaCmChannel(id_->channel->fd, loop_);
    if (cmChannel_ == NULL) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMAACCEPTOR, "rdma create qp error.");
        goto CREATE_CMCHANNEL_FAILED;
    }

    // 设置回调函数
    cmChannel_->setCallBack(handleRead, NULL, this);
    cmChannel_->enrollIn(false);

    return S_OK;

CREATE_CMCHANNEL_FAILED:
CREATE_LISTEN_FAILED:
CREATE_ADDR_FAILED:
    rdma_destroy_id(id_);
CREATE_ID_FAILED:
    rdma_destroy_event_channel(channel);
CREATE_CHANNEL_FAILED:
    return S_FALSE;
}

void RdmaAcceptor::onAccept(RdmaConnection *rc, 
         AcceptorCallback aCb, void *acceptorParam)
{
    info_.rc = rc;
    info_.aCb = aCb;
    info_.acceptorParam = acceptorParam;

    info_.valid = true;
    info_.isAccepted = false;
}

int RdmaAcceptor::getCmEvent(struct rdma_cm_event **ev)
{
    int ret;
    do {
        ret = rdma_get_cm_event(id_->channel, ev);
    } while (ret == -1 && errno == EINTR);

    return ret;
}

int RdmaAcceptor::handleRead(void *pThis)
{
    int ret;
    RdmaAcceptor *rdmaAcc = static_cast<RdmaAcceptor *>(pThis);
    struct rdma_cm_event *ev;
    RdmaConnection *rdmaCon;

    // 获取event
    ret = rdmaAcc->getCmEvent(&ev);
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMAACCEPTOR,
            "rdma get cm events error.");
        return S_FALSE;
    }

    // 根据事件类型来选择
    switch (ev->event) {
    case RDMA_CM_EVENT_CONNECT_REQUEST:
        // 建立连接
        ret = rdmaAcc->establishConection(ev);
        if (ret < 0) {
            LOG(LOG_ERROR_LEVEL,
                LOG_SOURCE_RDMAACCEPTOR,
                "eatablish a connection error.");
            // 没有成功建连则不ack
            return S_FALSE;
        }

        rdmaAcc->info_.isAccepted = true;
        LOG(LOG_DEBUG_LEVEL,
            LOG_SOURCE_RDMAACCEPTOR,
            "new connection incoming.");

        // 执行建连后的回调
        if (rdmaAcc->info_.aCb != NULL)
            rdmaAcc->info_.aCb(rdmaAcc->info_.acceptorParam);
        break;

        // 建连后重新置位valid
        rdmaAcc->info_.valid = false;
    case RDMA_CM_EVENT_ESTABLISHED:
        // 连接建立完成
        LOG(LOG_DEBUG_LEVEL,
            LOG_SOURCE_RDMAACCEPTOR,
            "new connection accepted.");
        break;
    case RDMA_CM_EVENT_DISCONNECTED:
        // 断开连接
        rdmaCon = static_cast<RdmaConnection *>(ev->id->context);
        rdmaCon->shutdown();

        LOG(LOG_DEBUG_LEVEL,
            LOG_SOURCE_RDMAACCEPTOR,
            "connection disconnected.");
        break;
    default:
        LOG(LOG_DEBUG_LEVEL, LOG_SOURCE_RDMAACCEPTOR, "unknown event.");
        break;
    }

    // 每个event都必须ack
    rdma_ack_cm_event(ev);
    return S_OK;
}

int RdmaAcceptor::establishConection(rdma_cm_event *ev)
{
    if (!info_.valid) return S_FALSE;

    info_.rc->createPassiveConnection(ev);

    int ret = info_.rc->accept();
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMAACCEPTOR,
            "accept connection error.");
        return S_FALSE;
    }

    return S_OK;
}

RdmaConnection *RdmaAcceptor::getAcceptedConnection()
{
    return (info_.valid == true && info_.isAccepted == true) ? info_.rc : NULL;
}

void RdmaAcceptor::setNonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int RdmaAcceptor::listen(int backlog) { return rdma_listen(id_, backlog); }

} // namespace net
} // namespace ndsl
