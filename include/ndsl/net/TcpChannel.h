/**
 * @file TcpChannel.h
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#ifndef __NDSL_NET_TCPCHANNEL_H__
#define __NDSL_NET_TCPCHANNEL_H__
#include "BaseChannel.h"

namespace ndsl {
namespace net {

class TcpAcceptor;
class TcpConnection;

class TcpChannel : public BaseChannel
{
  public:
    TcpChannel(int sockfd, EventLoop *loop);
    ~TcpChannel();
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_TCPCHANNEL_H__
