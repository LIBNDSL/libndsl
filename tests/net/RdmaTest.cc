/*
 * @file RdmaTest.cc
 * @brief
 * RDMA onSend/onRecv的单元测试
 * RDMA 原子操作的单元测试（Fetch&Add Compare&Swap）
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */

#include "../catch.hpp"
#include "ndsl/net/RdmaAcceptor.h"
#include "ndsl/net/RdmaClient.h"
#include "ndsl/net/RdmaConnection.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/SocketAddress.h"

using namespace ndsl::net;

// qp配置
static RdmaQpConfig conf(8, 8);

const size_t MAX_STR_LEN = 128;
size_t max_str_len;

char sendBuf[MAX_STR_LEN] = "This is a rdma message.";
char recvBuf[MAX_STR_LEN] = "";

char serverRecvBuf[MAX_STR_LEN] = "";
char serverSendBuf[MAX_STR_LEN] = "this is a rdma message from server.";

// 原子操作变量
uint64_t *pserverAtomic;
uint64_t *pclientAtomic;

// client获取server的mr，以便进行原子操作
// 实际应用中，需要将remote_mr传输给client
struct ibv_mr *remote_mr;

// 退出loop的函数
void stopLoop(void *ploop)
{
    EventLoop *loop = static_cast<EventLoop *>(ploop);

    loop->quit();
}

// 打印输出收到的数据,并且关闭连接
void disconnect(void *pclient)
{
    RdmaClient *client = static_cast<RdmaClient *>(pclient);

    client->disconnect();
}

void sendMessage(void *pclient)
{
    RdmaClient *client = static_cast<RdmaClient *>(pclient);

    client->registerMemory(sendBuf, MAX_STR_LEN);
    client->registerMemory(recvBuf, MAX_STR_LEN);

    max_str_len = MAX_STR_LEN;
    // 发送后就准备接收数据
    client->onSend(sendBuf, MAX_STR_LEN, 0, NULL, NULL);
    client->onRecv(recvBuf, &max_str_len, 0, disconnect, pclient);
}

void serverSendMessage(void *pserver)
{
    RdmaConnection *server = static_cast<RdmaConnection *>(pserver);

    server->onSend(serverSendBuf, MAX_STR_LEN);
}

void serverRecvMessage(void *pserver)
{
    RdmaAcceptor *server = static_cast<RdmaAcceptor *>(pserver);
    RdmaConnection *acceptedConn = server->getAcceptedConnection();

    max_str_len = MAX_STR_LEN;

    acceptedConn->registerMemory(serverSendBuf, MAX_STR_LEN);
    acceptedConn->registerMemory(serverRecvBuf, MAX_STR_LEN);

    acceptedConn->onRecv(
        serverRecvBuf, &max_str_len, 0, serverSendMessage, acceptedConn);

    // 准备下次accept
    server->onAccept(
        new RdmaConnection(acceptedConn->getLoop(), conf),
        serverRecvMessage,
        server);
}

void serverPrepareAtomic(void *pserver)
{
    pserverAtomic = new uint64_t(0);
    RdmaAcceptor *server = static_cast<RdmaAcceptor *>(pserver);
    RdmaConnection *acceptedCon = server->getAcceptedConnection();

    remote_mr =
        acceptedCon->registerMemory(pserverAtomic, sizeof(uint64_t), true);
}

void clientPostFetchAddOperation(void *pclient)
{
    RdmaClient *client = static_cast<RdmaClient *>(pclient);

    pclientAtomic = new uint64_t;
    client->registerMemory(pclientAtomic, sizeof(pclientAtomic));

    client->FetchAndAdd(pclientAtomic, remote_mr, 1);
}

void clientPostCmpSwapOperation(void *pclient)
{
    RdmaClient *client = static_cast<RdmaClient *>(pclient);

    pclientAtomic = new uint64_t;
    client->registerMemory(pclientAtomic, sizeof pclientAtomic);

    client->CmpAndSwp(pclientAtomic, remote_mr, 0, 2);
}

TEST_CASE("net/RdmaTest")
{
    int ret;
    EventLoop loop;
    ret = loop.init(true);
    REQUIRE(ret == S_OK);

    // 服务器
    RdmaAcceptor server(&loop, conf);
    RdmaClient client(&loop, conf);

    // 根据rdma网卡的IP
    SocketAddress4 serverAddr("10.0.1.1", 20079);
    // 开始监听
    ret = server.start(&serverAddr);
    REQUIRE(ret == S_OK);

    // 客户端
    ret = client.initCmChannel(&serverAddr);
    REQUIRE(ret == S_OK);

    TimeWheel::Task *stop;

    SECTION("onSend/onRecv")
    {
        RdmaConnection *rdmaCon = new RdmaConnection(&loop, conf);
        server.onAccept(rdmaCon, serverRecvMessage, &server);

        // 请求连接并设置回调函数
        ret = client.onConnect(sendMessage, &client);
        REQUIRE(ret == S_OK);

        // 定时退出loop
        stop = new TimeWheel::Task;
        stop->doit = stopLoop;
        stop->param = &loop;
        stop->setInterval = 2;
        stop->times = 1;

        REQUIRE(loop.runAfter(stop) == S_OK);

        loop.loop(&loop);

        REQUIRE(strcmp(serverRecvBuf, sendBuf) == 0);
        REQUIRE(strcmp(recvBuf, serverSendBuf) == 0);
    }

    SECTION("Fetch and Add")
    {
        RdmaConnection *rdmaCon = new RdmaConnection(&loop, conf);
        server.onAccept(rdmaCon, serverPrepareAtomic, &server);

        // 请求连接并设置回调函数
        ret = client.onConnect(clientPostFetchAddOperation, &client);
        REQUIRE(ret == S_OK);

        // 定时退出loop
        stop = new TimeWheel::Task;
        stop->doit = stopLoop;
        stop->param = &loop;
        stop->setInterval = 2;
        stop->times = 1;

        REQUIRE(loop.runAfter(stop) == S_OK);

        loop.loop(&loop);
        REQUIRE(*pserverAtomic == 1);

        delete pserverAtomic;
        delete pclientAtomic;
    }

    SECTION("Compare and swap")
    {
        RdmaConnection *rdmaCon = new RdmaConnection(&loop, conf);
        server.onAccept(rdmaCon, serverPrepareAtomic, &server);

        // 请求连接并设置回调函数
        ret = client.onConnect(clientPostCmpSwapOperation, &client);
        REQUIRE(ret == S_OK);

        // 定时退出loop
        stop = new TimeWheel::Task;
        stop->doit = stopLoop;
        stop->param = &loop;
        stop->setInterval = 2;
        stop->times = 1;

        REQUIRE(loop.runAfter(stop) == S_OK);

        loop.loop(&loop);
        REQUIRE(*pserverAtomic == 2);

        delete pserverAtomic;
        delete pclientAtomic;
    }
}
