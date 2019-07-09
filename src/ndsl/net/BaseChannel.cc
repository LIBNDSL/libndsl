/**
 * @file BaseChannel.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#include "ndsl/net/BaseChannel.h"
#include "ndsl/config.h"
#include "ndsl/utils/Log.h"
#include "ndsl/net/EventLoop.h"
#include <sys/epoll.h>

namespace ndsl {
namespace net {

BaseChannel::BaseChannel(int fd, EventLoop *loop)
    : Channel(loop)
    , fd_(fd)
    , pUp_(NULL)
{}

int BaseChannel::getFd() { return fd_; }

int BaseChannel::handleEvent()
{
    // EPOLLHUP EPOLLRDHUP 好像老版本的Linux内核才会有EPOLLHUP这个提示
    // Stream socket peer closed connection, or shut down writing half of
    // connection.  (This flag is especially useful for writing simple code  to
    // detect peer shutdown when using Edge Triggered monitoring.)
    // EPOLLIN | EPOLLRDHUP 对端关闭

    // EPOLLERR 表示相关联的fd发生了错误
    // Error condition happened  on  the  associated  file  descriptor.
    // epoll_wait(2)  will always wait for this event; it is not
    // necessary to set it in events.

    // FIXME: 这个fd在哪close还得协商 是不是没必要实现
    if (revents_ & EPOLLIN) {
        // printf("BaseChannel::handleEvent  EPOLLIN\n");
        if (handleRead_) handleRead_(pUp_);
    } else if (revents_ & EPOLLOUT) {
        // printf("BaseChannel::handleEvent  EPOLLOUT\n");
        if (handleWrite_) handleWrite_(pUp_);
    }

    if ((revents_ & EPOLLIN) && (revents_ & EPOLLHUP)) {
        // printf("BaseChannel::handleEvent receive EPOLLHUP\n");
        if (errorHandle_) errorHandle_(errParam_, errno);
        // close(fd_);
    } else if ((revents_ & EPOLLIN) && (revents_ & EPOLLRDHUP)) {
        // printf("BaseChannel::handleEvent receive EPOLLRDHUP\n");
        if (errorHandle_) errorHandle_(errParam_, errno);
        // close(fd_);
    } else if ((revents_ & EPOLLIN) && (revents_ & EPOLLERR)) {
        // printf("BaseChannel::handleEvent receive EPOLLERR\n");
        if (errorHandle_) errorHandle_(errParam_, errno);
        // close(fd_);
    }

    return S_OK;
}

int BaseChannel::setCallBack(
    ChannelCallBack handleRead,
    ChannelCallBack handleWrite,
    void *thi)
{
    handleRead_ = handleRead;
    handleWrite_ = handleWrite;
    pUp_ = thi;

    return S_OK;
}

int BaseChannel::enroll(bool isET)
{
    if (isET) events_ |= EPOLLET;

    // 同时注册输入输出
    events_ |= EPOLLIN;
    events_ |= EPOLLOUT;

    // 注册出错
    // events_ |= EPOLLHUP; // 不需要注册,系统自动注册
    events_ |= EPOLLRDHUP;
    events_ |= EPOLLERR;

    return pLoop_->enroll(this);
}

int BaseChannel::enrollIn(bool isET)
{
    if (isET) events_ |= EPOLLET;

    events_ |= EPOLLIN;

    events_ |= EPOLLRDHUP;
    events_ |= EPOLLERR;
    return pLoop_->enroll(this);
}

int BaseChannel::erase() { return pLoop_->erase(this); }

int BaseChannel::onError(ErrorHandle cb, void *param)
{
    errorHandle_ = cb;
    errParam_ = param;

    return S_OK;
}

} // namespace net
} // namespace ndsl
