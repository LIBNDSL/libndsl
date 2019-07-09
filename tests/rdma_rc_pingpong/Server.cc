/*
 * @file Server.cc
 * @brief
 * Rdma ping-pong测试
 * Rdma Server
 *
 * 说明:
 * 1.服务端端口可以任意设置，客户端设置的ip和端口需与服务端一致
 *  注:此ip与以太网ip不同，是RDMA的HCA配置的IP
 * 2.本次ping-pong测试时长为一分钟
 * 3.经过实际测试，每次onSend/onRecv都去重新注册内存对性能影响较大
 *  所以，只注册一次内存
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */

#include "ndsl/net/RdmaAcceptor.h"
#include "ndsl/net/RdmaClient.h"
#include "ndsl/net/RdmaConnection.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/net/TimeWheel.h"

using namespace ndsl::net;

int recvCount = 0;

RdmaQpConfig conf(1, 500);

const size_t MAX_STR_LEN = 16384;
size_t max_str_len;

char serverRecvBuf[MAX_STR_LEN];
char serverSendBuf[MAX_STR_LEN] = "this is a rdma message from server.";

void serverSendMessage(void *pRdmaConn)
{
    RdmaConnection *rdmaCon = static_cast<RdmaConnection *>(pRdmaConn);

    // 发送但不设置回调
    rdmaCon->onSend(serverSendBuf, MAX_STR_LEN);

    // 连续发送500个接收请求
    // if (++recvCount == 500) {
    // max_str_len = MAX_STR_LEN;
    rdmaCon->onRecv(serverRecvBuf, &max_str_len, 0, serverSendMessage, rdmaCon);
    // recvCount = 0;
    // }
}

void serverRecvMessage(void *pserver)
{
    RdmaAcceptor *server = static_cast<RdmaAcceptor *>(pserver);
    RdmaConnection *acceptedConn = server->getAcceptedConnection();

    // 注册内存
    acceptedConn->registerMemory(serverRecvBuf, MAX_STR_LEN);
    acceptedConn->registerMemory(serverSendBuf, MAX_STR_LEN);

    max_str_len = MAX_STR_LEN;
    // 接收数据并设置回调
    acceptedConn->onRecv(
        serverRecvBuf, &max_str_len, 0, serverSendMessage, acceptedConn);

    // 为下次连接注册回调，本例可不用
    server->onAccept(
        new RdmaConnection(acceptedConn->getLoop(), conf),
        serverRecvMessage,
        server);
}

int main()
{
    set_ndsl_log_sinks(LOG_SOURCE_ALL, LOG_OUTPUT_TER);
    int ret;
    EventLoop loop;
    ret = loop.init();
    assert(ret == S_OK);

    // 服务器
    RdmaAcceptor server(&loop, conf);

    SocketAddress4 serverAddr("0.0.0.0", 20079);
    // 开始监听
    ret = server.start(&serverAddr);
    assert(ret == S_OK);

    RdmaConnection *rdmaCon = new RdmaConnection(&loop, conf);
    server.onAccept(rdmaCon, serverRecvMessage, &server);

    loop.loop(&loop);
}
