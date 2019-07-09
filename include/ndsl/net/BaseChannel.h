/**
 * @file BaseChannel.h
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#ifndef __NDSL_NET_BASECHANNEL_H__
#define __NDSL_NET_BASECHANNEL_H__

#include <cstdlib>
#include "Channel.h"

namespace ndsl {
namespace net {

class EventLoop;

class BaseChannel : public Channel
{
  private:
    int fd_; // sockfd

    void *pUp_; // 存储上层的this Connecion Acceptor

    // 定义handleRead handleWrite函数指针原型
    using ChannelCallBack = int (*)(void *);

  public:
    BaseChannel(int fd, EventLoop *loop);
    // ~BaseChannel(); // TODO: 要不要去做fd的释放

    using ErrorHandle = void (*)(
        void *,
        int); // error回调函数 自定义参数, int errno

    ErrorHandle errorHandle_; // 用户注册的出错回调函数
    void *errParam_;          // errHandle函数 用户传递过来的参数

    // 指向被调用的函数
    ChannelCallBack handleRead_, handleWrite_;

    int handleEvent();
    int getFd();

    // epoll 事件管理
    int erase();
    int enroll(bool isET);

    // 内部使用 ps.留给EVentLoop
    int enrollIn(bool isET);

    // 设置回调函数和上层指针
    int setCallBack(
        ChannelCallBack handleRead,
        ChannelCallBack handleWrite,
        void *thi);

    int onError(ErrorHandle cb, void *param = NULL);
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_BASECHANNEL_H__
