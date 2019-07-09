/**
 * @file Channel.h
 * @brief
 * Channel 基类
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#ifndef __NDSL_NET_CHANNEL_H__
#define __NDSL_NET_CHANNEL_H__
// #include <stdint.h>
#include "ndsl/config.h"
#include "ndsl/utils/Log.h"

namespace ndsl {
namespace net {

class EventLoop;

struct Channel
{
  public:
    using Callback = void (*)(void *); // Callback 函数指针原型

    // protected:
  public:
    uint32_t events_;  // 注册事件
    uint32_t revents_; // 发生事件
    EventLoop *pLoop_; // 指向EventLoop

  public:
    Channel(EventLoop *loop)
        : events_(0)
        , revents_(0)
        , pLoop_(loop)
    {}
    Channel() {}
    virtual ~Channel() {} // 虚析构函数

    virtual int handleEvent() = 0; // loop的事件处理函数
    virtual int getFd() = 0;       // rdma的fd在结构内部
};

} // namespace net
} // namespace ndsl

#endif // __CHANNEL_H__
