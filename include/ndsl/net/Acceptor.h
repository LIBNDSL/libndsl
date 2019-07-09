/**
 * @file Acceptor.h
 * @brief
 * 各种 accetor 基类
 *
 * @author ranxiangjun
 * @email ranxianshen@gmail.com
 */
#ifndef __NDSL_NET_ACCEPTOR_H__
#define __NDSL_NET_ACCEPTOR_H__
#include "ndsl/config.h"
#include "ndsl/net/Asynclo.h"
#include "ndsl/utils/Log.h"
#include "ndsl/utils/Error.h"
#include <sys/types.h>

namespace ndsl {
namespace net {

class RdmaConnection;
class TcpConnection;

class Acceptor
{
  public:
    using Callback = void (*)(void *); // Callback 函数指针原型
    
    Acceptor() {}
    virtual ~Acceptor(){};

	virtual int onAccept(
        TcpConnection *pCon,
        struct sockaddr *addr,
        socklen_t *addrlen,
        Callback cb,
        void *param,
        EventLoop *loop = NULL)
    { 
        return S_OK;
    }

    virtual void OnAccept(
            RdmaConnection *rc
            ,Callback aCb
            ,void *acceptorParam)
    {
        return;
    }
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_ACCEPTOR_H__
