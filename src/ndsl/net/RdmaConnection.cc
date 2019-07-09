/*
 * @file RdmaConnection.cc
 * @brief
 * 响应QP中的事件,传输数据
 *
 * int send_flags，描述了WR的属性，其值为0或者一个或多个flags的按位异或。
 *
 *  IBV_SEND_FENCE - 为此WR设置围栏指示器。这意味着这个WR的处理将被阻止，
 *  直到所有之前发布的RDMA Read和Atomic WR都将被完成。
 *  仅对运输服务类型为IBV_QPT_RC的QP有效
 *
 *  IBV_SEND_SIGNALED - 设置此WR的完成通知指示符。这意味着如果QP是
 *  使用sq_sig_all = 0创建的，那么当WR的处理结束时，将会产生一个工作完成。
 *  如果QP是使用sq_sig_all = 1创建的，则不会对此标志产生任何影响
 *
 *  IBV_SEND_SOLICITED - 为此WR设置请求事件指示器。这意味着，
 *  当这个WR中的消息将在远程QP中结束时，将会创建一个请求事件，
 *  如果在远程侧，用户正在等待请求事件，它将被唤醒。
 *  与仅用于发送和RDMA写入的立即操作码相关
 *
 *  IBV_SEND_INLINE - sg_list中指定的内存缓冲区将内联放置在发送请求中。
 *  这意味着低级驱动程序（即CPU）将读取数据而不是RDMA设备。
 *  这意味着L_Key将不会被检查，实际上这些内存缓冲区甚至不需要被注册，
 *  并且在ibv_post_send（）结束之后可以立即重用。仅对发送和RDMA写操作码有效。
 *  由于在该代码中没有涉及到key的交换，所以也无法使用RDMA传输，
 *  所以还是使用了CPU读取数据，既然是CPU读取，那么也就不需要注册内存缓冲区了，
 *  这个标记只能用于发送和写操作。
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include <fcntl.h>
#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>
#include "ndsl/net/RdmaConnection.h"
#include "ndsl/utils/Error.h"
#include "ndsl/utils/Log.h"

namespace ndsl {
namespace net {

RdmaConnection::RdmaConnection(EventLoop *loop, RdmaQpConfig &conf)
    : loop_(loop)
    , conf_(conf)
    , id_(NULL)
    , sendCqChannel_(NULL)
    , recvCqChannel_(NULL)
{}

RdmaConnection::~RdmaConnection()
{
    if (sendCqChannel_ != NULL) delete sendCqChannel_;
    if (recvCqChannel_ != NULL) delete recvCqChannel_;

    // dereg内存区域并释放内存
    if (mrMap_.size() != 0) {
        for (auto iter = mrMap_.begin(); iter != mrMap_.end();) {
            ibv_dereg_mr(iter->second->mr);
            delete iter->second;
            mrMap_.erase(iter++);
        }
    }

    if (id_->qp) ibv_destroy_qp(id_->qp);

    ibv_dealloc_pd(id_->pd);
    rdma_destroy_id(id_);
}

int RdmaConnection::handleSend(void *pThis)
{
    RdmaConnection *rdmaCon = static_cast<RdmaConnection *>(pThis);

    // sendHandlerQueue中没有元素，则直接返回
    // 等待事件注册后，再读取cq
    if (rdmaCon->sendHandlerQueue.size() == 0) return S_FALSE;

    wcHandlerQueue &shq = rdmaCon->sendHandlerQueue;
    wcLists dataList;
    saveInfo *sendComp; // 发送完成事件

    rdmaCon->pollIncomingData(rdmaCon->id_->send_cq_channel, dataList);

    for (auto it = dataList.begin(); it != dataList.end(); it++) {
        // 取出头部元素
        sendComp = shq.front();

        // 若不是队头元素，则需要遍历整个队列
        if (sendComp->wr_id != it->wr_id) {
            for (auto qit = shq.begin(); qit != shq.end(); qit++) {
                // 找到则出队
                if ((*qit)->wr_id == it->wr_id) {
                    sendComp = *qit;
                    shq.erase(qit);
                    break;
                }
            }

            // 没有找到，丢弃这个wc
            if (sendComp->wr_id == (shq.front())->wr_id) {
                LOG(LOG_WARN_LEVEL,
                    LOG_SOURCE_RDMACONNECTION,
                    "unknown wc error.");
                continue;
            }
        } else // 是队头元素则出队
            shq.pop_front();

        // 执行回调函数
        if (sendComp->cb != NULL) sendComp->cb(sendComp->param);

        // 释放内存
        delete sendComp;
    }

    return S_OK;
}

// 最大完成事件最好设为1
int RdmaConnection::handleRecv(void *pThis)
{
    RdmaConnection *rdmaCon = static_cast<RdmaConnection *>(pThis);

    // recvHandlerQueue中没有元素，直接返回
    // 等待事件注册后在读取cq
    if (rdmaCon->recvHandlerQueue.size() == 0) return S_FALSE;

    wcHandlerQueue &rhq = rdmaCon->recvHandlerQueue;
    wcLists dataList;
    saveInfo *recvComp; // 接收完成事件

    rdmaCon->pollIncomingData(rdmaCon->id_->recv_cq_channel, dataList);

    for (auto it = dataList.begin(); it != dataList.end(); it++) {
        // 取出头部元素
        recvComp = rhq.front();

        // 若不是头部元素,则需便利整个队列
        // 注意：遍历整个队列也有可能没有找到，则丢弃
        if (recvComp->wr_id != it->wr_id) {
            for (auto qit = rhq.begin(); qit != rhq.end(); ++qit) {
                // 找到则出队
                if ((*qit)->wr_id == it->wr_id) {
                    recvComp = *qit;
                    rhq.erase(qit);
                    break;
                }
            }

            // 没有找到，丢弃这个wc
            if (recvComp->wr_id == (rhq.front())->wr_id) {
                LOG(LOG_WARN_LEVEL,
                    LOG_SOURCE_RDMACONNECTION,
                    "unknown wc error.");
                continue;
            }
        } else // 是对头元素则出队
            rhq.pop_front();

        // 执行回调函数
        if (recvComp->cb != NULL) recvComp->cb(recvComp->param);

        // 释放内存
        delete recvComp;
    }

    return S_OK;
}

int RdmaConnection::pollIncomingData(
    struct ibv_comp_channel *compChannel,
    wcLists &wcList)
{
    int ret;
    void *cq_ctx;
    struct ibv_cq *cq;
    struct ibv_wc wc;

    wcList.clear();

    ret = ibv_get_cq_event(compChannel, &cq, &cq_ctx);
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMACONNECTION,
            "ibv get cq event error.");
        return S_FALSE;
    }

    // 重新设置提醒
    ret = ibv_req_notify_cq(cq, 0);
    if (ret != 0) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMACONNECTION,
            "ibv req notify cq error.");
        return S_FALSE;
    }

    while (1) {
        ret = ibv_poll_cq(cq, 1, &wc);
        if (ret <= 0) {
            if (ret == 0) break;
            if (errno == EINTR) continue;

            LOG(LOG_ERROR_LEVEL,
                LOG_SOURCE_RDMACONNECTION,
                "ibv poll cq error.");
            break;
        }

        if (wc.status != IBV_WC_SUCCESS) {
            LOG(LOG_ERROR_LEVEL,
                LOG_SOURCE_RDMACONNECTION,
                "completion with status 0x%x:%s was found.\t wc.opcode = %d",
                wc.status,
                ibv_wc_status_str(wc.status),
                wc.opcode);

            continue;
        }

        wcList.push_back(wc);
    }

    // 一次get一次ack
    ibv_ack_cq_events(cq, 1);

    return S_OK;
}

ibv_mr *RdmaConnection::registerMemory(void *addr, uint64_t len, bool atomic)
{
    // 用于判断是否插入成功
    std::pair<MemoryReigonMap::iterator, bool> result;
    MemoryReigon *mr;
    struct ibv_mr *region;
    int access; // 接入属性

    if (addr == NULL) return NULL;

    // 若存在,则返回mr
    auto iter = mrMap_.find(addr);
    if (iter != mrMap_.end()) {
        LOG(LOG_WARN_LEVEL,
            LOG_SOURCE_RDMACONNECTION,
            "memory region already exists.");
        return iter->second->mr;
    }

    access = IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
             IBV_ACCESS_REMOTE_READ;

    // 若需要原子操作，则添加原子选项
    if (atomic) access |= IBV_ACCESS_REMOTE_ATOMIC;

    region = ibv_reg_mr(id_->pd, addr, len, access);
    if (region == NULL) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMACONNECTION,
            "ibv register mr error.");
        return NULL;
    }

    mr = new MemoryReigon;
    if (mr == NULL) goto REG_FAILED;
    mr->addr = addr;
    mr->mr = region;
    mr->size = len;

    result = mrMap_.insert(MemoryReigonMapKV(addr, mr));
    if (result.second != true) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMACONNECTION, "mrMap insert error.");
        goto INSERT_FAILED;
    }

    return region;

INSERT_FAILED:
    delete mr;
REG_FAILED:
    ibv_dereg_mr(region);
    return NULL;
}

int RdmaConnection::deregisterMemory(void *addr)
{
    auto iter = mrMap_.find(addr);

    // addr没有注册
    if (iter == mrMap_.end()) return S_FALSE;

    int ret = ibv_dereg_mr(iter->second->mr);
    if (ret != 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMACONNECTION, "ibv dereg mr error.");
        return S_FALSE;
    }

    delete iter->second; // 释放内存
    mrMap_.erase(iter);  // 从map中删除

    return S_OK;
}

// 被动建连
int RdmaConnection::createPassiveConnection(struct rdma_cm_event *event)
{
    if (sendCqChannel_ != NULL || recvCqChannel_ != NULL || event == NULL)
        return S_FALSE;

    int ret;
    struct ibv_qp_init_attr qp_init_attr;

    // 给id赋值
    id_ = event->id;

    // 分配pd
    id_->pd = ibv_alloc_pd(id_->verbs);
    if (id_->pd == NULL) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMACONNECTION, "ibv alloc pd error.");
        goto ALLOC_PD_FAILED;
    }

    // 创建新的qp,无需指定send_cq和recv_cq，rdma_cm会自动为qp分配cq
    memset(&qp_init_attr, 0, sizeof qp_init_attr);
    qp_init_attr.qp_type = IBV_QPT_RC;
    qp_init_attr.cap.max_recv_wr = conf_.max_pending_recv_req;
    qp_init_attr.cap.max_send_wr = conf_.max_pending_send_req;
    qp_init_attr.cap.max_recv_sge = 1;
    qp_init_attr.cap.max_send_sge = 1;
    ret = rdma_create_qp(id_, id_->pd, &qp_init_attr);
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMACONNECTION, "rdma alloc qp error.");
        goto CREATE_QP_FAILED;
    }

    // 设置非阻塞
    setNonblocking(getRecvChannelFd());
    setNonblocking(getSendChannelFd());

    // 建立cq通知机制
    ret = ibv_req_notify_cq(id_->recv_cq, 0);
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMACONNECTION,
            "ibv req notify recv cq error.");
        goto NOTIFY_CQ_FAILED;
    }

    ret = ibv_req_notify_cq(id_->send_cq, 0);
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMACONNECTION,
            "ibv req notify send cq error.");
        goto NOTIFY_CQ_FAILED;
    }

    // 记录connection,以便后面事件响应后使用
    id_->context = this;

    // 创建sendCqChannel
    sendCqChannel_ = new RdmaTransferChannel(getSendChannelFd(), loop_);
    // 给channel设置回调并注册进epoll
    sendCqChannel_->setCallBack(handleSend, NULL, this);
    sendCqChannel_->enrollIn(false);

    // 创建recvCqChannel
    recvCqChannel_ = new RdmaTransferChannel(getRecvChannelFd(), loop_);
    // 给channel设置回调并注册进epoll
    recvCqChannel_->setCallBack(handleRecv, NULL, this);
    recvCqChannel_->enrollIn(false);

    return S_OK;

NOTIFY_CQ_FAILED:
    rdma_destroy_qp(id_);
CREATE_QP_FAILED:
    ibv_dealloc_pd(id_->pd);
ALLOC_PD_FAILED:
    rdma_reject(id_, NULL, 0);
    rdma_destroy_id(id_);
    return S_FALSE;
}

// 主动建立连接
int RdmaConnection::createActiveConnection()
{
    if (sendCqChannel_ != NULL || recvCqChannel_ != NULL) return S_FALSE;
    int ret;

    setNonblocking(id_->recv_cq_channel->fd);
    setNonblocking(id_->send_cq_channel->fd);

    // 设置通知机制
    ret = ibv_req_notify_cq(id_->recv_cq, 0);
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMACONNECTION,
            "ibv req notify cq error.");
        return S_FALSE;
    }

    // 设置通知机制
    ret = ibv_req_notify_cq(id_->send_cq, 0);
    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMACONNECTION,
            "ibv req notify cq error.");
        return S_FALSE;
    }

    // 分配cq channel
    sendCqChannel_ = new RdmaTransferChannel(getSendChannelFd(), loop_);
    recvCqChannel_ = new RdmaTransferChannel(getRecvChannelFd(), loop_);

    // 设置回调函数并注册进epoll
    sendCqChannel_->setCallBack(handleSend, NULL, this);
    sendCqChannel_->enrollIn(false);

    recvCqChannel_->setCallBack(handleRecv, NULL, this);
    recvCqChannel_->enrollIn(false);

    return S_OK;
}

// 是否有EPOLLERR事件?
int RdmaConnection::onError(ErrorHandle cb, void *param)
{
    if (cb == NULL || sendCqChannel_ == NULL || recvCqChannel_ == NULL)
        return S_FALSE;

    sendCqChannel_->onError(cb, param);
    recvCqChannel_->onError(cb, param);

    return S_OK;
}

// 发送send请求,并且设置完成send的回调函数
int RdmaConnection::onSend(
    char *buf,
    size_t len,
    int flags,
    Callback cb,
    void *param)
{
    struct ibv_send_wr swr, *bad;
    struct ibv_sge sge;
    struct ibv_mr *mr;
    int ret;

    auto it = mrMap_.find(buf);
    if (it == mrMap_.end()) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMACONNECTION,
            "Error: buffer not registed.");
        return S_FALSE;
    }
    mr = it->second->mr;

    memset(&swr, 0, sizeof swr);
    memset(&sge, 0, sizeof sge);

    swr.num_sge = 1;
    swr.sg_list = &sge;
    swr.opcode = IBV_WR_SEND;
    swr.send_flags = IBV_SEND_SIGNALED;
    swr.wr_id = (uint64_t) buf;

    sge.addr = (uint64_t) buf;
    sge.length = (uint32_t) len;
    sge.lkey = mr->lkey;

    ret = ibv_post_send(id_->qp, &swr, &bad);
    if (ret != 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMACONNECTION, "ibv post send error.");
        return S_FALSE;
    }

    // 创建发送完成的处理信息
    saveInfo *sendinfo = new saveInfo;
    sendinfo->cb = cb;
    sendinfo->param = param;
    sendinfo->buffer = buf;
    sendinfo->len = len;
    sendinfo->wr_id = swr.wr_id;

    // 添加到sendHandlerQueue
    sendHandlerQueue.push_back(sendinfo);

    return S_OK;
}

// 发送recv请求,并设置回调函数
int RdmaConnection::onRecv(
    char *buf,
    size_t *len,
    int flags,
    Callback cb,
    void *param)
{
    struct ibv_mr *mr;
    struct ibv_recv_wr rwr, *bad;
    struct ibv_sge sge;
    int ret;

    auto it = mrMap_.find(buf);
    if (it == mrMap_.end()) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMACONNECTION,
            "Error: buffer not registed.");
        return S_FALSE;
    }
    mr = it->second->mr;

    memset(&rwr, 0, sizeof rwr);
    memset(&sge, 0, sizeof sge);

    rwr.num_sge = 1;
    rwr.sg_list = &sge;
    rwr.wr_id = (uint64_t) buf;

    sge.addr = (uint64_t) buf;
    sge.length = *len;
    sge.lkey = mr->lkey;

    // for (int i = 0; i < conf_.max_pending_recv_req; i++) {
    ret = ibv_post_recv(id_->qp, &rwr, &bad);
    if (ret != 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMACONNECTION, "ibv post recv error.");
        return S_FALSE;
    }

    // 创建接收完成的处理信息
    saveInfo *recvinfo = new saveInfo;
    recvinfo->cb = cb;
    recvinfo->param = param;
    recvinfo->buffer = buf;
    recvinfo->len = *len;
    recvinfo->wr_id = rwr.wr_id;

    // 添加到recvHandlerQueue
    recvHandlerQueue.push_back(recvinfo);
    // }
    return S_OK;
}

int RdmaConnection::FetchAndAdd(
    uint64_t *buf,
    struct ibv_mr *remote_mr,
    uint64_t add)
{
    int ret;
    struct ibv_send_wr swr, *bad;
    struct ibv_sge sge;
    struct ibv_mr *mr;

    auto it = mrMap_.find(buf);
    if (it == mrMap_.end()) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMACONNECTION,
            "Error: buffer not registed.");
        return S_FALSE;
    }
    mr = it->second->mr;

    // 设置sge
    memset(&sge, 0, sizeof sge);
    sge.addr = (uint64_t) buf;
    sge.length = sizeof(uint64_t);
    sge.lkey = mr->lkey;

    // 设置send wr
    memset(&swr, 0, sizeof swr);
    swr.wr_id = (uint64_t) buf;
    swr.opcode = IBV_WR_ATOMIC_FETCH_AND_ADD;
    swr.num_sge = 1;
    swr.sg_list = &sge;
    swr.send_flags = IBV_SEND_SIGNALED;

    // 设置atomic相关
    swr.wr.atomic.remote_addr = (uint64_t) remote_mr->addr;
    swr.wr.atomic.rkey = remote_mr->rkey;
    swr.wr.atomic.compare_add = add; // 远端地址的值加上该数

    ret = ibv_post_send(id_->qp, &swr, &bad);
    if (ret != 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMACONNECTION, "ibv post send error.");
        return S_FALSE;
    }

    return S_OK;
}

int RdmaConnection::CmpAndSwp(
    uint64_t *buf,
    struct ibv_mr *remote_mr,
    uint64_t expected,
    uint64_t swap)
{
    int ret;
    struct ibv_send_wr swr, *bad;
    struct ibv_sge sge;
    struct ibv_mr *mr;

    auto it = mrMap_.find(buf);
    if (it == mrMap_.end()) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_RDMACONNECTION,
            "Error: buffer not registed.");
        return S_FALSE;
    }
    mr = it->second->mr;

    // 设置sge
    memset(&sge, 0, sizeof sge);
    sge.addr = (uint64_t) buf;
    sge.length = sizeof buf;
    sge.lkey = mr->lkey;

    // 设置send wr
    memset(&swr, 0, sizeof swr);
    swr.wr_id = (uint64_t) buf;
    swr.sg_list = &sge;
    swr.num_sge = 1;
    swr.opcode = IBV_WR_ATOMIC_CMP_AND_SWP;
    swr.send_flags = IBV_SEND_SIGNALED;

    // 设置atomic相关
    swr.wr.atomic.remote_addr = (uint64_t) remote_mr->addr;
    swr.wr.atomic.rkey = remote_mr->rkey;
    swr.wr.atomic.compare_add = expected; // 期待值
    swr.wr.atomic.swap = swap; // 与若期待值相同，则将远端的值设为swap

    ret = ibv_post_send(id_->qp, &swr, &bad);
    if (ret != 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_RDMACONNECTION, "ibv post send error.");
        return S_FALSE;
    }

    return S_OK;
}

void RdmaConnection::setNonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int RdmaConnection::accept(struct rdma_conn_param *param)
{
    return rdma_accept(id_, param);
}

int RdmaConnection::disconnect() { return rdma_disconnect(id_); }

void RdmaConnection::shutdown()
{
    // 添加断连操作
    utils::WorkItem *w = new utils::WorkItem;
    w->doit = destroyConnection;
    w->param = this;

    loop_->addWork(w);
}

void RdmaConnection::destroyConnection(void *pThis)
{
    RdmaConnection *rdmaCon = static_cast<RdmaConnection *>(pThis);

    delete rdmaCon;
}

} // namespace net
} // namespace ndsl