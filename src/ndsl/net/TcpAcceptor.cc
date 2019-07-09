/**
 * @file TcpAccepter.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>

#include "ndsl/config.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/utils/Log.h"

#include <ndsl/net/SocketOpt.h>
#include <stdio.h>
namespace ndsl {
namespace net {

TcpAcceptor::TcpAcceptor(EventLoop *pLoop)
    : pLoop_(pLoop)
    , pTcpChannel_(NULL)
    , listenfd_(-1)
{
    info.inUse_ = false;

    // 将测试用回调函数置为空
    cb_ = NULL;
}

// 测试专用构造函数
TcpAcceptor::TcpAcceptor(Callback cb, EventLoop *pLoop)
    : pLoop_(pLoop)
    , pTcpChannel_(NULL)
    , cb_(cb)
    , listenfd_(-1)
{
    info.inUse_ = false;
}

TcpAcceptor::~TcpAcceptor() { delete pTcpChannel_; }

int TcpAcceptor::setInfo(
    TcpConnection *pCon,
    struct sockaddr_in *addr,
    socklen_t *addrlen,
    Callback cb,
    void *param,
    EventLoop *loop)
{
    info.pCon_ = pCon;
    info.addr_ = addr;
    info.addrlen_ = addrlen;
    info.cb_ = cb;
    info.param_ = param;
    info.loop_ = loop;
    info.inUse_ = true;

    return S_OK;
}

int TcpAcceptor::start(struct SocketAddress4 servaddr)
{
    int n = createAndListen(servaddr);
    if (n < 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPACCETPOR, "createAndListen fail");
        return S_FALSE;
    }

    pTcpChannel_ = new TcpChannel(listenfd_, pLoop_);
    if (pTcpChannel_ == NULL) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPACCETPOR, "new TcpChannel fail");
        return S_FALSE;
    } else {
        pTcpChannel_->setCallBack(handleRead, NULL, this);
        pTcpChannel_->enrollIn(false);
    }

    return S_OK;
}

int TcpAcceptor::createAndListen(struct SocketAddress4 servaddr)
{
    // 端口复用
    listenfd_ = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);

    if (listenfd_ < 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPACCETPOR, "socket fail");
        return S_FALSE;
    }

    // 设置非阻塞
    fcntl(listenfd_, F_SETFL, O_NONBLOCK);
    // setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // 增加REUSEADDR
    SocketOpt sock;
    sock.setReuseAddr(listenfd_, true);

    if (-1 ==
        bind(listenfd_, (struct sockaddr *) &servaddr, sizeof(servaddr))) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPACCETPOR, "bind error");
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPACCETPOR, strerror(errno));
        return S_FALSE;
    }

    if (-1 == listen(listenfd_, LISTENQ)) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPACCETPOR, "listen error");
        return S_FALSE;
    }

    return S_OK;
}

int TcpAcceptor::handleRead(void *pthis)
{
    TcpAcceptor *pThis = static_cast<TcpAcceptor *>(pthis);
    if (pThis->info.inUse_) {
        int connfd;
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(struct sockaddr_in);
        connfd = accept(
            pThis->listenfd_,
            (struct sockaddr *) &cliaddr,
            (socklen_t *) &clilen);

        if (connfd > 0) {
            //连接成功
            // LOG(LOG_INFO_LEVEL,
            //     LOG_SOURCE_TCPACCETPOR,
            //     "TcpAcceptor::connect succ\n");
        } else {
            // 连接失败
            LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPACCETPOR, "connect fail");
            return S_FALSE;
        }

        // 设置非阻塞io
        fcntl(connfd, F_SETFL, O_NONBLOCK);

        if (NULL != pThis->info.loop_) {
            ((pThis->info).pCon_)->createChannel(connfd, pThis->info.loop_);
        } else {
            ((pThis->info).pCon_)
                ->createChannel(connfd, pThis->pTcpChannel_->pLoop_);
        }

        if (NULL != pThis->info.addr_) {
            memcpy(pThis->info.addr_, &cliaddr, sizeof(struct sockaddr_in));
        }
        // pThis->info.addr_ = (struct sockaddr *) &cliaddr;
        if (NULL != pThis->info.addrlen_) { *(pThis->info.addrlen_) = clilen; }
        // pThis->info.addrlen_ = (socklen_t *) &clilen;

        // proactor模式，需要循环注册
        pThis->info.inUse_ = false;
        if (pThis->info.cb_ != NULL) pThis->info.cb_(pThis->info.param_);

        // 测试专用
        if (pThis->cb_ != NULL) pThis->cb_(NULL);
    }

    return S_OK;
}

int TcpAcceptor::onAccept(
    TcpConnection *pCon,
    struct sockaddr_in *addr,
    socklen_t *addrlen,
    Callback cb,
    void *param,
    EventLoop *loop)
{
    return setInfo(pCon, addr, addrlen, cb, param, loop);
}

} // namespace net
} // namespace ndsl
