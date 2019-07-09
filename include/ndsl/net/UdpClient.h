/**
 * @file UdpClient.h
 * @brief
 *
 * @author lanry
 * @email  luckylanry@163.com
 */
#ifndef __UDPCLIENT_H__
#define __UDPCLIENT_H__
#include "EventLoop.h"
#include "ndsl/net/UdpEndpoint.h"

namespace ndsl {
namespace net {

class UdpClient
{
  public:
    UdpClient();
    ~UdpClient();

    int sfd; //用来保存链接fd
    UdpEndpoint *begin(EventLoop *loop,struct SocketAddress4 servaddr);

    int end();
};

} // namespace net
} // namespace ndsl

#endif // __UDPCLIENT_H__