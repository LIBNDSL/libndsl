/**
 * @file TcpConnectionTest.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#include "../catch.hpp"
#include "ndsl/net/EventLoop.h"
#include "ndsl/config.h"
#include "ndsl/utils/Error.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/TcpClient.h"
#include "ndsl/net/SocketAddress.h"

using namespace ndsl;
using namespace net;

bool rrecv = false;

static void fun1(void *param) { rrecv = true; }

TEST_CASE("net/TcpAcceptor")
{
    SECTION("start handleRead")
    {
        // 初始化EPOLL 服务器 客户端共用一个EPOLL
        EventLoop loop;
        REQUIRE(loop.init() == S_OK);

        // 准备客户端的接受参数 默认全ip接受 端口6666
        struct SocketAddress4 servaddr("0.0.0.0", 6667);

        TcpAcceptor *tAc = new TcpAcceptor(&loop);
        REQUIRE(tAc->start(servaddr) == S_OK);

        // 准备接收的数据结构
        struct sockaddr_in rservaddr;
        bzero(&rservaddr, sizeof(rservaddr));
        socklen_t addrlen;

        TcpConnection *Conn = new TcpConnection();
        tAc->onAccept(Conn, &rservaddr, &addrlen, fun1, NULL);

        // 启动一个客户端
        struct SocketAddress4 clientservaddr("127.0.0.1", 6667);
        // TcpConnection *pClientConn;
        TcpClient *pCli = new TcpClient();
        REQUIRE((pCli->onConnect(&loop, true, &clientservaddr)) == S_OK);

        // 添加中断
        loop.quit();
        REQUIRE(loop.loop(&loop) == S_OK);

        // 测试是否接收到了客户的连接
        REQUIRE(rrecv == true);

        delete tAc;
        delete Conn;
        pCli->disConnect();
        delete pCli;
    }
}