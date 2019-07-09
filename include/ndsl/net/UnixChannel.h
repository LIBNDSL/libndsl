////
// @file UnixChannel
// @brief
// fengzhuang channle class
//
// @author ranxiangjun
// @email ranxianshen@gmail.com
//
#ifndef __NDSL_NET_UNIXCHANNEL_H__
#define __NDSL_NET_UNIXCHANNEL_H__
#include "ndsl/net/BaseChannel.h"

namespace ndsl {
namespace net {

class UnixConnection;
class TcpAcceptor;

class UnixChannel : public BaseChannel
{
  public:
    UnixChannel(int sockfd, EventLoop *loop);
    ~UnixChannel();

	// not decided
    // UnixConnection *newConnection(int connfd);
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_UNIXCHANNEL_H__
