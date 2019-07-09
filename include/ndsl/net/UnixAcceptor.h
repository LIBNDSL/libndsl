////
// @file UnixAcceptor.h
// brief fengzhuang unixacceptor
//
// @author ranxiangjun
// @email ranxianshen@gmail.com
//
#ifndef __NDSL_NET_UNIXACCEPTOR_H__
#define __NDSL_NET_UNXIACCEPTOR_H__
#include <cstring>
#include "ndsl/net/UnixChannel.h"
#include "ndsl/net/EventLoop.h"

namespace ndsl {
namespace net {

class UnixConnection;
class UnixAcceptor
{
  private:
    int listenfd_;
    EventLoop *pLoop_;
    UnixChannel *pUnixChannel_;
	using Callback = void (*)(void *); //callbak function pointer type
    Callback cb_;

    // 用于保存用户回调信息
    struct Info
    {
        UnixConnection *pCon_;
        struct sockaddr *addr_;
        socklen_t *addrlen_;
        Callback cb_;
        void *param_;
        bool inUse_; // 判断当前结构体是否有数据
    } info;

  public:
    UnixAcceptor(EventLoop *pLoop);
    ~UnixAcceptor();

	// UnixConnection uses 
	UnixChannel *getUnixChannel();
	// test use
	UnixAcceptor(Callback cb, EventLoop *pLoop);

    // 为用户主动调用onAcceptor()而生
    int setInfo(
        UnixConnection *pCon,			//should be unixconnection?
        struct sockaddr *addr,
        socklen_t *addrlen,
        Callback cb,
        void *param);

	static int handleRead(void *);
    // int handleWrite();
    int start(const std::string& );

  private:
    int createAndListen(const std::string&);
};

} // namespace net
} // namespace ndsl

#endif // _NDSL_NET__UNIXACCEPTOR_H__
