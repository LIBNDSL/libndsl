/*
 * @file Client.cc
 * @brief
 * Rdma ping-pong测试
 * Rdma Client
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

bool connected = true;

const size_t MAX_STR_LEN = 16384;
size_t max_str_len;

char sendBuf[MAX_STR_LEN] = "This is a rdma message.";
char recvBuf[MAX_STR_LEN];

long long count = 0;
// int recvCount = 0;

// 退出loop的函数
void stopLoop(void *ploop)
{
    EventLoop *loop = static_cast<EventLoop *>(ploop);

    printf(
        "total count: %lld, total data size: %lldMbit/s\n",
        count,
        (count * 16 * (MAX_STR_LEN / 1024) / 1024) / 60);

    loop->quit();
}

void sendMessage(void *pclient)
{
    RdmaClient *client = static_cast<RdmaClient *>(pclient);
    count++;

    // printf("client recviced message %lld: %s\n", count++, recvBuf);

    // 发送后就准备接收数据
    client->onSend(sendBuf, MAX_STR_LEN /*, 0, recvMessage, pclient*/);

    // 连续发送500个接收请求
    // if (++recvCount == 500) {
    // for (int i = 0; i < 500; i++)
    // max_str_len = MAX_STR_LEN;
    client->onRecv(recvBuf, &max_str_len, 0, sendMessage, pclient);
    // recvCount = 0;
    // }
}

void sendMessageWithInit(void *pclient)
{
    RdmaClient *client = static_cast<RdmaClient *>(pclient);

    // 注册内存
    client->registerMemory(sendBuf, MAX_STR_LEN);
    client->registerMemory(recvBuf, MAX_STR_LEN);

    max_str_len = MAX_STR_LEN;
    // 发送后就准备接收数据
    client->onSend(sendBuf, MAX_STR_LEN);
    client->onRecv(recvBuf, &max_str_len, 0, sendMessage, pclient);

    connected = true;
}

int main()
{
    set_ndsl_log_sinks(LOG_SOURCE_ALL, LOG_OUTPUT_TER);
    int ret;
    EventLoop loop;
    ret = loop.init();
    assert(ret == S_OK);

    RdmaQpConfig conf(1, 500);

    SocketAddress4 serverAddr("10.0.1.1", 20079);

    // 客户端
    RdmaClient client(&loop, conf);
    ret = client.initCmChannel(&serverAddr);
    assert(ret == S_OK);

    // 请求连接并设置回调函数
    ret = client.onConnect(sendMessageWithInit, &client);
    assert(ret == S_OK);

    // 时间轮,定时退出loop
    TimeWheel tw(&loop);
    tw.start();

    TimeWheel::Task *stop = new TimeWheel::Task;

    stop->doit = stopLoop;
    stop->param = &loop;
    stop->setInterval = 60;
    stop->times = 1;

    assert(tw.addTask(stop) == S_OK);

    loop.loop(&loop);
}