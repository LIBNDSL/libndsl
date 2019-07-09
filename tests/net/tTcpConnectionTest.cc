/**
 * @file TcpConnectionTest.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#define CATCH_CONFIG_MAIN
#include "../catch.hpp"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/Epoll.h"
#include "ndsl/net/TcpChannel.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/utils/temp_define.h"
#include "ndsl/net/TcpAcceptor.h"
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace ndsl;
using namespace net;

// char *sayHello() { return "Hello"; }

bool rrecv = false;

static void fun1(void *param) { rrecv = true; }

TEST_CASE("net/TcpConnection(onRecv)")
{
    SECTION("send write")
    {
        // 启动服务
        // 初始化EPOLL
        EventLoop loop;
        REQUIRE(loop.init() == S_OK);

        // 初始化tcpAcceptor
        TcpAcceptor *pAc = new TcpAcceptor(fun1, &loop);
        REQUIRE(pAc->start() == S_OK);

        // 添加中断
        loop.quit();
        REQUIRE(loop.loop() == S_OK);

        // // 启动一个客户端
        // int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        // struct sockaddr_in servaddr;
        // bzero(&servaddr, sizeof(servaddr));
        // servaddr.sin_family = AF_INET;
        // servaddr.sin_port = htons(SERV_PORT);
        // inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
        // connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

        // 测试是否接收到了客户的连接
        REQUIRE(rrecv == true);
    }
}