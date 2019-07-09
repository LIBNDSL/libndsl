/**
 * @file Channel.h
 * @brief
 * 各种 Connection 基类
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#ifndef __NDSL_NET_ASYNCLO_H__
#define __NDSL_NET_ASYNCLO_H__
#include "ndsl/config.h"
#include "ndsl/utils/Log.h"
#include "ndsl/net/BaseChannel.h"
#include "ndsl/utils/Error.h"
#include "SocketAddress.h"
#include <sys/types.h>

namespace ndsl {
namespace net {

class Asynclo
{
    // Channel *channel_;
  public:
    // Channel *channel_;
    using Callback = void (*)(void *); // Callback 函数指针原型
    using ErrorHandle = void (*)(
        void *,
        int); // error回调函数 自定义参数, int errno

    Asynclo() {}
    virtual ~Asynclo(){};

    virtual int onError(ErrorHandle cb, void *param) = 0;

    virtual EventLoop *getLoop() = 0;

    virtual int
    onRecv(char *buffer, size_t *len, int flags, Callback cb, void *param)
    {
        return S_OK;
    }

    virtual int
    onSend(char *buf, size_t len, int flags, Callback cb, void *param)
    {
        return S_OK;
    }

    // TODO: unix域不知道啥情况，依据具体情况这里可改为虚函数
    // 获取本端地址
    virtual SocketAddress4 *getLocalAddr() = 0;
    // 获取对端地址
    virtual SocketAddress4 *getPeerAddr() = 0;
    // 获取本端的port
    virtual uint16_t getSrcPort() = 0;
    // 获取对端的port
    virtual uint16_t getDstPort() = 0;
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_ASYNCLO_H__
