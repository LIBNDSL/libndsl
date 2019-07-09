////
// @file PipeChannel.h
// @brief
//
// @auther ranxiangjun
// @email ranxianshen@gmail.compare
//
#ifndef _NDSL_NET_PIPECHANNEL_H_
#define _NDSL_NET_PIPECHANNEL_H_
#include "ndsl/net/BaseChannel.h"

namespace ndsl{
namespace net{
	
class PipeChannel : public BaseChannel
{
  public:
    PipeChannel(int pipefd, EventLoop *loop);
	~PipeChannel();
};

} // namespace net
} // namespace ndsl

#endif // _NDSL_NET_PIPECHANNEL_H_
