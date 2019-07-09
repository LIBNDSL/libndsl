/*
 * @file RdmaClient.cc
 * @brief
 * RDMA client用于主动建连
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>
#include <fcntl.h>

#include "ndsl/net/RdmaClient.h"
#include "ndsl/utils/Error.h"
#include "ndsl/utils/Log.h"

namespace ndsl {
namespace net {

RdmaClient::RdmaClient(EventLoop *loop, RdmaQpConfig &conf)
    : RdmaConnection(loop, conf)
    , cmChannel_(NULL)
    , cb_(NULL)
    , cbParam_(NULL)
{
    isConnected = false;
}

RdmaClient::~RdmaClient()
{
    if (cmChannel_ != NULL) delete cmChannel_;
}

int RdmaClient::initCmChannel(SocketAddress4 *peerAddr)
{
    int ret;
    struct rdma_event_channel *channel; // 和id_->channel相同
    struct rdma_cm_event *ev;
    struct ibv_qp_init_attr qp_init_attr;

    channel = rdma_create_event_channel();
    if (channel == NULL) return S_FALSE;

    ret = rdma_create_id(channel, &id_, NULL, RDMA_PS_TCP);
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMACLIENT, "rdma create id error.");
        goto CREATE_ID_FAILED;
    }

    // 解析对端地址
    ret = rdma_resolve_addr(id_, NULL, (struct sockaddr *) peerAddr, 1000);
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMACLIENT, "rdma resolve addr error.");
        goto RESOLVE_ADDR_FAILED;
    }

    ret = rdma_get_cm_event(channel, &ev);
    if (ret < 0) goto RESOLVE_ADDR_FAILED;
    if (ev->event != RDMA_CM_EVENT_ADDR_RESOLVED) goto RESOLVE_ADDR_FAILED;

    rdma_ack_cm_event(ev);

    // 解析路由信息
    ret = rdma_resolve_route(id_, 1000);
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMACLIENT,
            "rdma resolve route error.");
        goto RESOLVE_ROUTE_FAILED;
    }

    ret = rdma_get_cm_event(channel, &ev);
    if (ret < 0) goto RESOLVE_ROUTE_FAILED;
    if (ev->event != RDMA_CM_EVENT_ROUTE_RESOLVED) goto RESOLVE_ROUTE_FAILED;

    rdma_ack_cm_event(ev);

    id_->pd = ibv_alloc_pd(id_->verbs);
    if (id_->pd == NULL) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMACLIENT, "ibv alloc pd error.");
        goto CREATE_PD_FAILED;
    }

    // 准备qp
    memset(&qp_init_attr, 0, sizeof qp_init_attr);
    qp_init_attr.qp_type = IBV_QPT_RC;
    qp_init_attr.cap.max_recv_wr = conf_.max_pending_recv_req;
    qp_init_attr.cap.max_send_wr = conf_.max_pending_send_req;
    qp_init_attr.cap.max_recv_sge = 1;
    qp_init_attr.cap.max_send_sge = 1;

    ret = rdma_create_qp(id_, id_->pd, &qp_init_attr);
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMACLIENT, "rdma create qp error.");
        goto CREATE_QP_FAILED;
    }

    // 设置非阻塞
    setNonblock(id_->channel->fd);

    cmChannel_ = new RdmaCmChannel(id_->channel->fd, loop_);
    cmChannel_->setCallBack(handleCmEvent, NULL, this);
    cmChannel_->enrollIn(false);

    return S_OK;

CREATE_QP_FAILED:
    ibv_dealloc_pd(id_->pd);
RESOLVE_ROUTE_FAILED:
CREATE_PD_FAILED:
RESOLVE_ADDR_FAILED:
    rdma_destroy_id(id_);
CREATE_ID_FAILED:
    rdma_destroy_event_channel(channel);
    return S_FALSE;
}

int RdmaClient::onConnect(
    ConnectionCallback cb,
    void *param,
    struct rdma_conn_param *conn_param)
{
    cb_ = cb;
    cbParam_ = param;

    return rdma_connect(id_, NULL);
}

int RdmaClient::handleCmEvent(void *param)
{
    int ret;
    RdmaClient *rdmaClient = static_cast<RdmaClient *>(param);
    struct rdma_cm_event *ev;

    ret = rdmaClient->getCmEvent(&ev);
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMACLIENT, "rdma get cm event error.");
        return S_FALSE;
    }

    switch (ev->event) {
    case RDMA_CM_EVENT_ESTABLISHED:
        // 成功建立connetion
        ret = rdmaClient->createActiveConnection();
        if (ret != S_OK) {
            LOG(LOG_WARN_LEVEL,
                LOG_SOURCE_RDMACLIENT,
                "create active connection error.");
            return S_FALSE;
        }
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_RDMACLIENT, "connection established.");

        rdmaClient->isConnected = true;
        // 成功连接的回调函数
        if (rdmaClient->cb_ != NULL) rdmaClient->cb_(rdmaClient->cbParam_);

        break;
    case RDMA_CM_EVENT_DISCONNECTED:
        if (!rdmaClient->isConnected) {
            LOG(LOG_ERROR_LEVEL,
                LOG_SOURCE_RDMACLIENT,
                "disconnected error: connection is not exists.");
            return S_FALSE;
        }

        // 断开连接
        rdmaClient->shutdown();
        // 标记连接已经断开
        rdmaClient->isConnected = false;

        LOG(LOG_INFO_LEVEL, LOG_SOURCE_RDMACLIENT, "connection disconnected.");

        break;
    default:
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMACLIENT,
            "unknown error: unknown event %d.",
            ev->event);
        break;
    }

    rdma_ack_cm_event(ev);

    return S_OK;
}

int RdmaClient::getCmEvent(struct rdma_cm_event **ev)
{
    int ret;
    do {
        ret = rdma_get_cm_event(id_->channel, ev);
    } while (ret == -1 && errno == EINTR);

    return ret;
}

void RdmaClient::setNonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

} // namespace net
} // namespace ndsl