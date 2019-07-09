////
// @file UdpChannel.h
// @brief
//
//
// @author lanry
// @email luckylanry@163.com
//

#ifndef __NDSL_NET_UDPCHANNEL_H__
#define __NDSL_NET_UDPCHANNEL_H__

#include "BaseChannel.h"

namespace ndsl{
namespace net{

class UdpEndpoint;

class UdpChannel : public BaseChannel
{
  public:
    UdpChannel(int sockfd,EventLoop *loop);
    ~UdpChannel();
        
};

} // namespace net
} //namespace ndsl

#endif // __NDSL_NET_UDPCHANNEL_H__