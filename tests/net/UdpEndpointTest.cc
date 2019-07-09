////
// @file UdpEndpointTest.cc
// @brief
// 测试UdpEndpoint
//
// @author lanyue
// @email luckylanry@163.com 
//

#include "../catch.hpp"
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/UdpChannel.h"
#include "ndsl/net/UdpEndpoint.h"
#include "ndsl/net/Epoll.h"
#include "ndsl/net/UdpClient.h"
#include "ndsl/net/SocketAddress.h"
bool flag_ = false;

void TestFun1(void *a) { flag_ = true; }

bool udpTestFlagSend = false;

static void udpSendTest(void *aa) {
    
    udpTestFlagSend = true; }

bool udpClientRecv = false;
static void ClientudpRecvTest(void *aa)
{
    udpClientRecv = true;
}

using namespace ndsl::net;

TEST_CASE("net/UdpEndpoint")
{
	// 初始化EPOLL 服务器 客户端共用一个EPOLL
	EventLoop loop;
	REQUIRE(loop.init() == S_OK);

	UdpEndpoint *t = new UdpEndpoint(&loop);

	SECTION("udp")
	{
		// 准备客户端的接受参数 默认全ip接受 
		struct SocketAddress4 servaddr("0.0.0.0", 6666);
        // bzero(&servaddr, sizeof(servaddr));
		REQUIRE((t->start(servaddr))== 0);

        UdpEndpoint *pClient;
		struct SocketAddress4 cliaddr("127.0.0.1", 6666);
		UdpClient *pCli = new UdpClient();

	    REQUIRE((pClient = pCli->begin(&loop,cliaddr))!= NULL);

		// 测试Send
        char *sendbuf = (char *) malloc(sizeof(char) * 12);
        strcpy(sendbuf, "hello world");
		struct SocketAddress4 servaddr1("127.0.0.1", 6666);

        t->onSend(
        sendbuf,strlen("hello world"),0,(struct sockaddr*)&servaddr1,sizeof(servaddr1),udpSendTest, sendbuf);
        REQUIRE(udpTestFlagSend== true);

        char recvBuf[25];
        socklen_t struct_len;
        struct_len = sizeof(cliaddr);

        // 测试onRecv
        memset(recvBuf, 0, sizeof(recvBuf));
        size_t len=sizeof(recvBuf);
        t -> onSend((char*)"hello ndsl", strlen("hello ndsl"),0,(struct sockaddr*)&servaddr1,sizeof(servaddr1), udpSendTest, (char*)"hello ndsl");

        REQUIRE(
        t->onRecv(recvBuf,&len,0,(struct sockaddr*)&cliaddr,struct_len,ClientudpRecvTest, recvBuf) ==
            S_OK);
        REQUIRE(
        t->onRecv(recvBuf,&len,0,(struct sockaddr*)&cliaddr,struct_len,ClientudpRecvTest, recvBuf) ==
            S_OK);
        loop.quit();
        loop.loop(&loop);
        REQUIRE(udpClientRecv == true);
	}
}
