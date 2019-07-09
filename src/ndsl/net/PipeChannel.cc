////
// @file PipeChannel.cc
// @brief
//
// @auther ranxiangjun
// @email ranxianshen@gmail.compare
//
#include "ndsl/net/PipeChannel.h"

namespace ndsl{
namespace net{

PipeChannel::PipeChannel(int pipefd, EventLoop *loop)
	:BaseChannel(pipefd, loop){}
PipeChannel::~PipeChannel()
{
	// delete itself from the eventloop
	erase();
}

} // namespace net
} // namespace ndsl
