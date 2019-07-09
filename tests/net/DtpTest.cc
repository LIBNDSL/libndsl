/*
 * @file DtpTest.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#include "../catch.hpp"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/Epoll.h"
#include "ndsl/net/TcpChannel.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/TcpClient.h"
#include "ndsl/utils/Log.h"
#include "ndsl/config.h"
#include "ndsl/utils/Error.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/net/Entity.h"
#include "ndsl/net/Multiplexer.h"
#include "ndsl/net/DTPClient.h"

#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 9877

using namespace ndsl;
using namespace net;

Entity *entitySer, *entityCli;
TcpConnection *servConn;
TcpClient *pCli;
int port = PORT;
char *recvbuf = NULL;
const char *sendbuf = "hello cient, i am data from server";

static void noticecb(int type, void *point, void *param)
{
    if (type == SENDFILEFLAG) {
        // 同意接收文件
        // printf("收到接收文件请求\n");
        char *filename = static_cast<char *>(point);
        entityCli->client_->agreeRecvFile(filename);
    } else if (type == SENDDATAFLAG) {
        // 同意接收数据
        // printf("收到接收数据请求\n");
        char *dataId = static_cast<char *>(point);
        entityCli->client_->agreeRecvData(dataId);
    } else if (type == SENDDATAONUDP) {
        // printf("收到接收数UDP据请求\n");
        char *dataId = static_cast<char *>(point);
        entityCli->client_->agreeRecvData(dataId);

    } else if (type == RECVFILEFLAG) {
        //
        char *filepath = (char *) calloc(64, sizeof(char));
        strcpy(filepath, "../tests/recvFileTest.txt");
        int fd = open(filepath, O_RDONLY);
        if (fd < 0) {
            // printf("文件不存在！\n");
        } else {
            entitySer->multiplexer_->sendFile(
                servConn, entityCli->id_, filepath, 0);
        }
    } else if (type == RECVFILESUCC) {
        // 接收文件成功
        // printf("接收文件成功\n");
    } else if (type == RECVFILEFAIL) {
        // 接收文件失败
        // printf("接收文件失败\n");
    } else if (type == RECVDATASUCC || type == RECVDATAFAIL) {
        //接收数据成功或者失败都告诉对方
        char *dataId = static_cast<char *>(point);
        entityCli->client_->sendAck(
            entityCli->id_, entityCli->conn_, type, dataId);
    } else if (type == RECVUDPDATAFAIL) {
        // 通过udp接收数据失败
        // printf("recv udp data failed");
    } else if (type == OPPENDRECVFAIL || type == OPPENDRECVRDMAFAIL) {
        // 对端接收数据失败，重发
        // printf("the opposite end recvs data not succeessully\n");
        char *dataId = static_cast<char *>(point);
        int length = *(static_cast<int *>(param));
        if (type == OPPENDRECVFAIL)
            entitySer->multiplexer_->sendData(
                servConn, entitySer->id_, NULL, length, dataId, TCP);
        else
            entitySer->multiplexer_->sendData(
                servConn, entitySer->id_, NULL, length, dataId, RDMA);
    } else if (type == OPPENDRECVSUCC) {
        // printf("the opposite end recvs data succeessully\n");
        char *dataId = static_cast<char *>(point);
        entitySer->client_->dataIdMp_.erase(dataId);
    } else if (type == SENDFILESUCC) {
        // 发送文件成功
        // printf("文件发送成功\n");
    } else if (type == SENDDATASUCC) {
        // printf(" send data successfully\n");
    }
}

// 用户在这里处理收到的数据
static void datacb(int type, void *point, void *param)
{
    // printf("in datacb: %s:%ld\n", (char *) point, strlen((char *) point));
    recvbuf = (char *) point;
}

static void startCbOfRecvfile(void *a)
{
    // cli向ser拉文件 文件在ndsl目录下
    char *filename = (char *) calloc(64, sizeof(char));
    strcpy(filename, "recvFileTest.txt");
    entityCli->multiplexer_->recvFile(pCli, entitySer->id_, filename);
}

static void stcbofsendtcpdata(void *a)
{
    // ser向cli发送数据
    entitySer->multiplexer_->sendData(
        servConn, entitySer->id_, sendbuf, strlen(sendbuf) + 1, NULL, TCP);
}

static void stcbofsendudpdata(void *a)
{
    // ser向cli发送数据
    entitySer->multiplexer_->sendData(
        servConn, entitySer->id_, sendbuf, strlen(sendbuf) + 1, NULL, UDP);
}

// static void stcbofsendrdmadata(void *a)
// {
//     // ser向cli发送数据
//     entitySer->multiplexer_->sendData(
//         servConn, entitySer->id_, sendbuf, strlen(sendbuf), NULL, RDMA);
// }

static void startCbOfSendFile(void *a)
{
    // ser向cli发送文件 文件在ndsl目录下
    char *path = (char *) calloc(64, sizeof(char));
    strcpy(path, "../tests/sendFileTest.txt");
    entitySer->multiplexer_->sendFile(servConn, entitySer->id_, path, 0);
}

TEST_CASE("net/DtpTest")
{
    // 启动服务
    // 初始化EPOLL
    EventLoop loop;
    REQUIRE(loop.init() == S_OK);

    struct SocketAddress4 servaddr("0.0.0.0", port);

    TcpAcceptor *tAc = new TcpAcceptor(&loop);
    tAc->start(servaddr);

    servConn = new TcpConnection();
    tAc->onAccept(servConn, NULL, NULL, NULL, NULL, &loop);

    // 启动一个客户端
    struct SocketAddress4 clientservaddr("127.0.0.1", port);
    port++;
    pCli = new TcpClient();
    REQUIRE(pCli->onConnect(&loop, true, &clientservaddr) == S_OK);
    // 添加中断
    loop.quit();
    REQUIRE(loop.loop(&loop) == S_OK);

    // 启动两边mul
    Multiplexer *mulSer = new Multiplexer(&loop);
    Multiplexer *mulCli = new Multiplexer(&loop);

    SECTION("sendFile")
    {
        // 启动两边Entity
        entitySer =
            new Entity(servConn, 1, mulSer, noticecb, datacb, NULL, NULL);
        entityCli = new Entity(pCli, 1, mulCli, noticecb, datacb, NULL, NULL);
        entitySer->start(startCbOfSendFile, NULL);
        entityCli->start();

        loop.runAfter(3, loop.quit, &loop);
        REQUIRE(loop.loop(&loop) == S_OK);
    }

    SECTION("recvFile")
    {
        // 启动两边Entity
        entitySer =
            new Entity(servConn, 2, mulSer, noticecb, datacb, NULL, NULL);
        entityCli = new Entity(pCli, 2, mulCli, noticecb, datacb, NULL, NULL);
        entityCli->start(startCbOfRecvfile, NULL);
        entitySer->start();

        loop.runAfter(3, loop.quit, &loop);
        REQUIRE(loop.loop(&loop) == S_OK);
    }

    SECTION("sendDataOnTcp")
    {
        // 启动两边Entity
        entitySer =
            new Entity(servConn, 3, mulSer, noticecb, datacb, NULL, NULL);
        entityCli = new Entity(pCli, 3, mulCli, noticecb, datacb, NULL, NULL);
        entitySer->start(stcbofsendtcpdata, NULL);
        entityCli->start();

        // 添加定时器，退出
        loop.runAfter(3, loop.quit, &loop);
        REQUIRE(loop.loop(&loop) == S_OK);
        REQUIRE(strcmp(sendbuf, recvbuf) == 0);
    }

    SECTION("sendDataOnUdp")
    {
        // 启动两边Entity
        entitySer =
            new Entity(servConn, 4, mulSer, noticecb, datacb, NULL, NULL);
        entityCli = new Entity(pCli, 4, mulCli, noticecb, datacb, NULL, NULL);
        entitySer->start(stcbofsendudpdata, NULL);
        entityCli->start();

        // 添加定时器，退出
        loop.runAfter(3, loop.quit, &loop);
        REQUIRE(loop.loop(&loop) == S_OK);
        REQUIRE(strcmp(sendbuf, recvbuf) == 0);
    }

    // SECTION("sendDataOnRdma")
    // {
    //     // 启动两边Entity
    //     entitySer =
    //         new Entity(servConn, 5, mulSer, noticecb, datacb, NULL, NULL);
    //     entityCli = new Entity(pCli, 5, mulCli, noticecb, datacb, NULL,
    //     NULL); entitySer->start(stcbofsendrdmadata, NULL);
    //     entityCli->start();

    //     // 添加定时器，退出
    //     loop.runAfter(3, loop.quit, &loop);
    //     REQUIRE(loop.loop(&loop) == S_OK);
    //     REQUIRE(strcmp(sendbuf, recvbuf) == 0);
    // }
}
