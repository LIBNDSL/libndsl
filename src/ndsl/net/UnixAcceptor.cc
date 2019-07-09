////
// @file UnixAcceptor.cc
// @brief
// unixacceptor shixian
//
// @author ranxaingjun
// @email ranxianshen@gmail.com
//
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <cstdio>
#include <iostream>
#include <errno.h>
#include "ndsl/config.h"
#include "ndsl/utils/Error.h"
#include "ndsl/utils/Log.h"
#include "ndsl/net/UnixAcceptor.h"
#include "ndsl/net/SocketAddressUn.h"
#include "ndsl/net/UnixConnection.h"

using namespace std;

namespace ndsl {
namespace net {

UnixAcceptor::UnixAcceptor(EventLoop *pLoop)
    : listenfd_(-1)
    , pLoop_(pLoop)
    , pUnixChannel_(NULL)
{
    info.inUse_ = false;
    cb_ = NULL;
}

UnixAcceptor::~UnixAcceptor() { delete pUnixChannel_; }

UnixAcceptor::UnixAcceptor(Callback cb, EventLoop *pLoop)
    : listenfd_(-1)
    , pLoop_(pLoop)
    , pUnixChannel_(NULL)
    , cb_(cb)
{
    info.inUse_ = false;
}

int UnixAcceptor::setInfo(
    UnixConnection *pCon,
    struct sockaddr *addr,
    socklen_t *addrlen,
    Callback cb,
    void *param)
{
    memset(&info, 0, sizeof(struct Info));

    info.pCon_ = pCon;
    info.addr_ = addr;
    info.addrlen_ = addrlen;
    info.cb_ = cb;
    info.param_ = param;
    info.inUse_ = true;

    return S_OK;
}

UnixChannel *UnixAcceptor::getUnixChannel() { return pUnixChannel_; }

int UnixAcceptor::start(const string &path)
{
    createAndListen(path);
    pUnixChannel_ = new UnixChannel(listenfd_, pLoop_);
    pUnixChannel_->setCallBack(handleRead, NULL, this);
    pUnixChannel_->enroll(false);
    // pUnixChannel_->enableReading();

    return S_OK;
}

int UnixAcceptor::createAndListen(const string &path)
{
    listenfd_ = socket(AF_LOCAL, SOCK_STREAM, 0);
    struct SocketAddressUn servaddr(path);
    //  
    servaddr.setVirtualAddress(path);

    // in case the address was used
    unlink(path.c_str());

    // 设置非阻塞
    int flags =
        fcntl(listenfd_, F_GETFL, 0); // without it,other states may be cleaned
    if (flags < 0) {
        // cout<<"fget failed: "<<strerror(errno)<<endl;
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_UNIXCONNECTION,
            "fcntl get state error\n");
        return S_FALSE;
    }
    flags |= O_NONBLOCK;
    if (fcntl(listenfd_, F_SETFL, flags) < 0) {
        // cout<<"fset failed: "<<strerror(errno)<<endl;
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_UNIXCONNECTION,
            "fcntl set state error\n");
        return S_FALSE;
    }

    if (-1 ==
        bind(listenfd_, (struct sockaddr *) &servaddr, sizeof(servaddr))) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_UNIXCONNECTION,
            "bind error %d:%s\n",
            errno,
            strerror(errno));
        return S_FALSE;
    }

    if (-1 == listen(listenfd_, LISTENQ)) {}

    return S_OK;
}

int UnixAcceptor::handleRead(void *pthis)
{
    UnixAcceptor *pThis = static_cast<UnixAcceptor *>(pthis);

    int connfd;
    struct sockaddr_un cliaddr;
    socklen_t clilen = sizeof(struct sockaddr_un);
    connfd = accept(
        pThis->listenfd_, (struct sockaddr *) &cliaddr, (socklen_t *) &clilen);
    if (connfd > 0) {
        // 连接成功
    } else {
        // 连接失败
    }

    // 设置非阻塞io
    int flags =
        fcntl(connfd, F_GETFL, 0); // without it,other states may be cleaned
    if (flags < 0) {
        // cout<<"fget failed: "<<strerror(errno)<<endl;
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_UNIXCONNECTION,
            "fcntl get state error\n");
        return S_FALSE;
    }
    flags |= O_NONBLOCK;
    if (fcntl(connfd, F_SETFL, flags) < 0) {
        // cout<<"fset failed: "<<strerror(errno)<<endl;
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_UNIXCONNECTION,
            "fcntl set state error\n");
        return S_FALSE;
    }

    if (pThis->info.inUse_) {
        ((pThis->info).pCon_)
            ->createChannel(connfd, pThis->pUnixChannel_->pLoop_);
        pThis->info.addr_ = (struct sockaddr *) &cliaddr;
        pThis->info.addrlen_ = (socklen_t *) &clilen;
        if (pThis->info.cb_ != NULL) pThis->info.cb_(pThis->info.param_);
        // pThis->pUnixChannel_->disableReading();
        pThis->info.inUse_ = false;
    }

    // test use
    if (pThis->cb_ != NULL) pThis->cb_(NULL);
    return S_OK;
}

} // namespace net
} // namespace ndsl
