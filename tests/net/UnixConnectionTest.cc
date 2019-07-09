/**
 * @file UnixConnectionTest.cc
 * @brief
 *
 * @author ranxiangjun
 * @email ranxianshen@gmail.com
 */
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/un.h>
#include "../catch.hpp"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/Epoll.h"
#include "ndsl/net/UnixChannel.h"
#include "ndsl/net/UnixConnection.h"
#include "ndsl/net/UnixAcceptor.h"
#include "ndsl/net/SocketAddressUn.h"

using namespace ndsl;
using namespace net;

bool unixflag = false;

void unixfun1(void *a) { unixflag = true; }

bool unixflagsend = false;
static void unixsendTest(void *a) { unixflagsend = true; }

bool unixflagerror = false;
static void unixiserror(int a, int b) { unixflagerror = true; }

bool unixflagrecv = false;
static void unixrecvTest(void *a) { unixflagrecv = true; }

TEST_CASE("UnixConnection(onRecv)")
{
    SECTION("onAccept/onSend/onRecv")
    {
        // 启动服务
        // 初始化EPOLL
        EventLoop loop;
        REQUIRE(loop.init() == S_OK);

        UnixAcceptor *tAc = new UnixAcceptor(&loop);
        tAc->start("unix.socket");

        // 准备接收的数据结构
        // struct sockaddr_un rservaddr;
        // bzero(&rservaddr, sizeof(rservaddr));
        SocketAddressUn rservaddr;
        socklen_t addrlen;

        UnixConnection *Conn = new UnixConnection(tAc);
        Conn->onAccept(Conn, (SA *) &rservaddr, &addrlen, unixfun1, NULL);

        // 启动一个客户端
        int sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
        SocketAddressUn servaddr("unix.socket");
        servaddr.setVirtualAddress("unix.socket");
        connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

        // 添加中断
        loop.quit();
        REQUIRE(loop.loop(&loop) == S_OK);

        // 测试是否接收到了客户的连接
        REQUIRE(unixflag == true);

        // 测试onSend
        Conn->onError(unixiserror);
        Conn->onSend(
            "hello world", sizeof("hello world"), 0, unixsendTest, NULL);

        char recvBuf[15];
        memset(recvBuf, 0, sizeof(recvBuf));
        read(sockfd, recvBuf, MAXLINE);

        REQUIRE(strcmp("hello world", recvBuf) == 0);
        REQUIRE(unixflagsend == true);

        // 测试onRecv
        memset(recvBuf, 0, sizeof(recvBuf));
        ssize_t len;
        write(sockfd, "hello world", sizeof("hello world"));

        REQUIRE(Conn->onRecv(recvBuf, &len, 0, unixrecvTest, NULL) == S_OK);
        REQUIRE(unixflagrecv == true);

        // 第二次需要添加中断
        loop.quit();
        REQUIRE(loop.loop(&loop) == S_OK);
    }

    // TODO: handleRead handleWrite 好像没法测
    // SECTION("onRecv onSecd handleRead hanleWrite")
    // {
    //     // 启动服务
    //     // 初始化EPOLL
    //     EventLoop loop;
    //     REQUIRE(loop.init() == S_OK);
    // }
}
