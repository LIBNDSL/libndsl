/* @file Epoll.cc
 * @brief
 * Epoll的实现
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include "ndsl/net/Epoll.h"
#include "ndsl/net/Channel.h"
#include "ndsl/utils/Log.h"
#include "ndsl/utils/Error.h"
#include "ndsl/config.h"

namespace ndsl {
namespace net {

Epoll::Epoll()
    : epfd_(-1)
{}

Epoll::~Epoll()
{
    if (epfd_ != -1) ::close(epfd_);
}

int Epoll::init()
{
    // 设置CLOEXEC
    epfd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epfd_ < 0) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_EPOLL,
            "epoll_create errno = %d:%s\n",
            errno,
            strerror(errno));
        return errno;
    }

    return S_OK;
}

int Epoll::enroll(Channel *pCh)
{
    struct epoll_event ev;

    ev.data.ptr = pCh;
    ev.events = pCh->events_;

    int ret = ::epoll_ctl(epfd_, EPOLL_CTL_ADD, pCh->getFd(), &ev);

    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_EPOLL,
            "epoll_ctl errno = %d:%s\n",
            errno,
            strerror(errno));
        return errno;
    }

    // printf("Epoll::enroll fd = %d\n", pCh->getFd());
    return S_OK;
}

int Epoll::modify(Channel *pCh)
{
    struct epoll_event ev;

    ev.data.ptr = pCh;
    ev.events = pCh->events_;

    int ret = ::epoll_ctl(epfd_, EPOLL_CTL_MOD, pCh->getFd(), &ev);

    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_EPOLL,
            "epoll_ctl errno = %d:%s\n",
            errno,
            strerror(errno));
        return errno;
    }

    return S_OK;
}

int Epoll::erase(Channel *pCh)
{
    struct epoll_event ev;

    // printf("Epoll::erase fd = %d\n", pCh->getFd());

    int ret = ::epoll_ctl(epfd_, EPOLL_CTL_DEL, pCh->getFd(), &ev);

    if (ret < 0) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_EPOLL,
            "epoll_ctl errno = %d:%s\n",
            errno,
            strerror(errno));
        return errno;
    }

    return S_OK;
}

// nEvents返回响应事件数,timeoutMs默认为-1
int Epoll::wait(Channel *channels[], int &nEvents, int timeoutMs)
{
    struct epoll_event events[MAX_EVENTS];

retry:
    int ret = ::epoll_wait(epfd_, events, MAX_EVENTS, timeoutMs);

    if (ret < 0) {
        // 若系统中断退出wait,则继续等待
        // FIXME:是否会造成事件丢失?
        if (errno == EINTR) goto retry;

        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_EPOLL,
            "epoll_wait errno = %d:%s\n",
            errno,
            strerror(errno));
        return errno;
    }

    // 记录响应事件数
    nEvents = ret;

    // 依次读取事件，并返回事件
    for (int i = 0; i < ret; i++) {
        Channel *channel = static_cast<Channel *>(events[i].data.ptr);
        channel->revents_ = events[i].events;
        channels[i] = channel;
    }

    return S_OK;
}

} // namespace net
} // namespace ndsl
