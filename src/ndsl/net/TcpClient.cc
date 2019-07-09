/**
 * @file TcpClient.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#include <sys/socket.h>
#include <fcntl.h>

#include "ndsl/net/SocketAddress.h"
#include "ndsl/net/TcpClient.h"
#include "ndsl/net/TcpChannel.h"
#include "ndsl/config.h"
#include "ndsl/utils/Error.h"
#include "ndsl/utils/Log.h"

#include <cstdio>

namespace ndsl {
namespace net {

TcpClient::TcpClient()
    : sockfd_(0)
// , conn_(NULL)
{}
TcpClient::~TcpClient() {}

int TcpClient::onConnect(
    EventLoop *loop,
    bool isConnNoBlock,
    struct SocketAddress4 *servaddr)
{
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);

    // 设成非阻塞
    if (isConnNoBlock) fcntl(sockfd_, F_SETFL, O_NONBLOCK);

    int n;
    if ((n = connect(sockfd_, (SA *) servaddr, sizeof(struct SocketAddress4))) <
        0) {
        if (errno != EINPROGRESS) {
            // connect出错 返回
            LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "connect fail");
            // return NULL;
            return S_FALSE;
        }
    }

    // 设成非阻塞
    if (!isConnNoBlock) fcntl(sockfd_, F_SETFL, O_NONBLOCK);

    // 创建一个TcpConnection
    // conn_ = new TcpConnection();
    // if (NULL == conn_) {
    //     LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "new TcpConnection fail");
    //     // return NULL;
    //     return S_FALSE;
    // }

    if ((n = createChannel(sockfd_, loop)) < 0) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "createChannel fail");
        // return NULL;
        return S_FALSE;
    }

    // return conn_;
    return S_OK;
}

int TcpClient::disConnect()
{
    // 释放内存
    // delete conn_;
    return S_OK;
}

} // namespace net
} // namespace ndsl
