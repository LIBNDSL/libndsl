/**
 * @file client.cc
 * @brief
 *  TODO: 客户端是否也要用到网络库
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#include "ndsl/net/TcpClient.h"
#include "ndsl/net/EventLoop.h"

#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <sys/time.h>

#define BUFSIZE 16384

using namespace ndsl;
using namespace net;

size_t bytesRead;
char cbuf[BUFSIZE];
char recvbuf[BUFSIZE];
int64_t mtime;

void onDisconnect()
{
    printf(
        "%lf MiB/s throughput\n",
        (static_cast<double>(bytesRead) * 1000000.0) / (mtime * 1024 * 1024));
}

void makeMessage()
{
    for (int i = 0; i < BUFSIZE; ++i) {
        //cbuf[i] = static_cast<char>(1);
        cbuf[i] = '1';
    }
}

int main(int argc, char *argv[])
{
    // 启动服务
    // 初始化EPOLL
    EventLoop loop;
    loop.init();

    // 启动一个客户端
    TcpClient *pCli = new TcpClient();
    pCli->onConnect();

    // 构造传输数据
    makeMessage();

    int count = 0; // 统计发送次数
    memset(recvbuf, 0, sizeof(recvbuf));
    bytesRead = 0;
    bool isEnd = false;
    size_t n1, n2;
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);

    while (!isEnd) {
        if ((n1 = write(pCli->sockfd_, cbuf, strlen(cbuf))) < 0) {
            printf("write error\n");
            isEnd = true;
        } else {
            if ((n2 = read(pCli->sockfd_, recvbuf, BUFSIZE)) < 0) {
                printf("read error\n");
                isEnd = true;
            } else {
                memset(recvbuf, 0, sizeof(recvbuf));
                bytesRead += n2;
                count++;
            }
        }
        if (count > 15000) isEnd = true;
    }
    gettimeofday(&t2, NULL);
    mtime = (t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec;
    printf("time = %ld\n", mtime);

    // 计算
    onDisconnect();

    return 0;
}
