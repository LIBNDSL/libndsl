////
// @file  UdpClient.cc
// @brief
//
//
// @author lanry
// @email luckylanry@163.com
//
#include <arpa/inet.h>
#include <fcntl.h>
#include "ndsl/net/SocketAddress.h"
#include "ndsl/net/UdpClient.h"
#include "ndsl/net/UdpChannel.h"
#include "ndsl/config.h"
#include "ndsl/utils/Error.h"

namespace ndsl {
namespace net {

UdpClient::UdpClient() {}
UdpClient::~UdpClient() {}

UdpEndpoint *UdpClient::begin(EventLoop *loop,struct SocketAddress4 servaddr)
{
    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    fcntl(sfd,F_SETFL,O_NONBLOCK);

    int n;

    UdpEndpoint *ue=new UdpEndpoint(loop);

    if((n=ue->createChannel(sfd))< 0)
    {
        return NULL;
    }

    return ue;
}

int UdpClient::end()
{
    close(sfd);
    return S_OK;
}
} // namespace net
} // namespace ndsl
