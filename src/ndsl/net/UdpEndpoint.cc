////
// @file  UdpEndpoint_.cc
// @brief
//
//
// @author lanry
// @email luckylanry@163.com
//

#include <sys/socket.h>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <cstdio>
#include <errno.h>
#include <unistd.h>
#include <sys/un.h>
#include "ndsl/config.h"
#include "ndsl/net/UdpEndpoint.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/utils/Log.h"
#include "ndsl/utils/Error.h"
#include "ndsl/net/UdpChannel.h"

namespace ndsl {
namespace net {

UdpEndpoint::UdpEndpoint(EventLoop *pLoop)
    : sfd_(-1)
    , pLoop_(pLoop)
    , pUdpChannel_(NULL)
{
    cb_ = NULL;
    p_ = NULL;
    RecvInfo_.inUse_ = false;
}

UdpEndpoint::~UdpEndpoint() {}

// 注册sfd
int UdpEndpoint::createChannel(int sfd)
{
    pUdpChannel_ = new UdpChannel(sfd, pLoop_);
    pUdpChannel_->setCallBack(handleRead, handleWrite, this);
    pUdpChannel_->enroll(true);
    return S_OK;
}

int UdpEndpoint::start(struct SocketAddress4 servaddr)
{
    int n = createAndBind(servaddr);
    if (n < 0) {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_UDPCHANEEL, "createAndBind fail\n");
        return S_FALSE;
    }

    pUdpChannel_ = new UdpChannel(sfd_, pLoop_);
    if (pUdpChannel_ == NULL) {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_UDPCHANEEL, "new udp fail\n");
        return S_FALSE;
    } else {
        pUdpChannel_->setCallBack(handleRead, NULL, this);
        pUdpChannel_->enroll(false);
    }
    return S_OK;
}

int UdpEndpoint::createAndBind(struct SocketAddress4 servaddr)
{
    // udp套接口类型SOCK_DGRAM
    sfd_ = socket(AF_INET, SOCK_DGRAM, 0);

    if (sfd_ < 0) {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_UDPCHANEEL, "socket fail\n");
        return S_FALSE;
    }
    // 设置非阻塞
    fcntl(sfd_, F_SETFL, O_NONBLOCK);

    if (-1 == bind(sfd_, (struct sockaddr *) &servaddr, sizeof(servaddr))) {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_UDPCHANEEL, "UdpEndpoint bind error");
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_UDPCHANEEL, strerror(errno));
        printf("UdpEndpoint bind error\n");
        return S_FALSE;
    }
    return S_OK;
}

// 注册起回调作用
int UdpEndpoint::onSend(
    void *buf,
    size_t len,
    int flags,
    struct sockaddr *dest_addr,
    socklen_t addrlen,
    Callback cb,
    void *p)
{
    int sockfd = pUdpChannel_->getFd();
    size_t n =
        sendto(sockfd, buf, len, flags | MSG_NOSIGNAL, dest_addr, addrlen);
    if (n == len) {
        // 写完，通知用户
        if (cb != NULL) cb(p);
        return S_OK;
    } else if (n < 0) {
        // error occurs, tell user
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UDPENDPOINT, "sendto error occurs");
        return S_FALSE;
    }
    // 保存发送数据的用户信息
    pInfo tsi = new Info;
    tsi->len_ = new size_t;
    tsi->offset_ = n;
    tsi->sendBuf_ = (void *) buf;
    tsi->recvBuf_ = NULL;
    *(tsi->len_) = len;
    tsi->flags_ = flags | MSG_NOSIGNAL;

    tsi->dest_addr_ = dest_addr;
    tsi->addrlen_ = addrlen;
    tsi->src_addr_ = NULL;
    tsi->cb_ = cb;
    tsi->p_ = p;
    SendInfo_.push(tsi);
    return S_OK;
}
// udp只要有数据就发，不保证传送到目的地
int UdpEndpoint::handleWrite(void *pthis)
{
    UdpEndpoint *pThis = static_cast<UdpEndpoint *>(pthis);
    if (pThis->SendInfo_.size() > 0) {
        int sockfd = pThis->pUdpChannel_->getFd();
        if (sockfd < 0) { return -1; }

        ssize_t n;
        // 如果发送的数据报大小大于发送缓冲区，则发送失败
        pInfo tsi = pThis->SendInfo_.front();

        if ((n = sendto(
                 sockfd,
                 (char *) tsi->sendBuf_ + tsi->offset_,
                 *(tsi->len_) - tsi->offset_,
                 tsi->flags_,
                 tsi->dest_addr_,
                 tsi->addrlen_)) > 0) {
            tsi->offset_ += n;

            if (tsi->offset_ == *(tsi->len_)) {
                if (tsi->cb_ != NULL) tsi->cb_(tsi->p_);
                pThis->SendInfo_.pop();
                delete tsi; // 删除申请的内存
            } else if (n == 0) {
                return S_OK;
            } else if (n < 0) {
                // 将事件从队列中移除
                pThis->SendInfo_.pop();
                delete tsi;
                return S_FALSE;
            }
        }
    }
    return S_OK;
}

// 如果执行成功，返回值就为 S_OK；如果出现错误，返回值就为 S_FAIL，并设置 errno
// 的值。
int UdpEndpoint::onRecv(
    char *buf,
    size_t *len,
    int flags,
    struct sockaddr *src_addr,
    socklen_t addrlen,
    Callback cb,
    void *p)
{
    int sockfd = pUdpChannel_->getFd();
    int n;
    if ((n = recvfrom(sockfd, buf, *len, flags, src_addr, &addrlen)) < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 保存用户信息
            LOG(LOG_INFO_LEVEL, LOG_SOURCE_UDPCHANEEL, "receive meet agagin");
            RecvInfo_.recvBuf_ = buf;
            RecvInfo_.sendBuf_ = NULL;
            RecvInfo_.flags_ = flags | MSG_NOSIGNAL;
            RecvInfo_.len_ = len;
            RecvInfo_.src_addr_ = src_addr;
            RecvInfo_.addrlen_ = addrlen;
            RecvInfo_.dest_addr_ = NULL; // 接收数据只需知道发送源
            RecvInfo_.cb_ = cb;
            RecvInfo_.p_ = p;
            RecvInfo_.inUse_ = true;
            return S_OK;
        } else {
            // error occurs,callback user
            LOG(LOG_INFO_LEVEL, LOG_SOURCE_UDPCHANEEL, "receive other error");
            return S_FALSE;
        }
    }

    *len = n;
    // tell user after reading successfully in one time
    if (cb != NULL) cb(p);
    // 先返回，最终的处理在onRecv()里面
    return S_OK;
}

int UdpEndpoint::handleRead(void *pthis)
{
    UdpEndpoint *pThis = static_cast<UdpEndpoint *>(pthis);

    if (pThis->RecvInfo_.inUse_) {
        int sockfd = pThis->pUdpChannel_->getFd();
        if (sockfd < 0) { return S_FALSE; }

        size_t n;
        if ((n = recvfrom(
                 sockfd,
                 pThis->RecvInfo_.recvBuf_,
                 MAXLINE,
                 pThis->RecvInfo_.flags_,
                 pThis->RecvInfo_.src_addr_,
                 (&(pThis->RecvInfo_.addrlen_)))) < 0) {
            // 出错
            *(pThis->RecvInfo_.len_) = n; // ?
            return S_FALSE;
        }

        *(pThis->RecvInfo_.len_) = n;

        // 完成数据读取之后通知mul
        if (pThis->RecvInfo_.cb_ != NULL)
            pThis->RecvInfo_.cb_(pThis->RecvInfo_.p_);

        return S_OK;
    } else
        return S_FALSE;
}
} // namespace net
} // namespace ndsl
