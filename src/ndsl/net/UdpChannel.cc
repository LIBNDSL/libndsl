////
// @file UdpChannel.cc
// @brief
//
//
// @author lanry
// @email luckylanry@163.com
//

#include "ndsl/net/UdpChannel.h"
#include "ndsl/net/UdpEndpoint.h"

namespace ndsl {
namespace net {

UdpChannel::UdpChannel(int sockfd, EventLoop *loop)
    : BaseChannel(sockfd, loop)
{}

UdpChannel::~UdpChannel()
{
    // 将自身从eventloop上面删掉
    erase();
}

} // namespace net
} // namespace ndsl