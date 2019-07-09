/**
 * @file EntityTest.cc
 * @brief
 * 实体的测试
 *
 * @author zzt
 * @emial 429834658@qq.com
 **/
#include "ndsl/net/Entity.h"
#include "ndsl/net/Multiplexer.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/TcpChannel.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/TcpClient.h"
#include "ndsl/net/Epoll.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/utils/Log.h"
#include <sys/socket.h>
#include "ndsl/config.h"
#include <cstring>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include "Protbload.pb.h"

using namespace ndsl;
using namespace net;
using namespace Protbload;

void servercallbak(Multiplexer *Multiplexer, char *data, int len, int ero)
{
    Protbload::ADD *addmessage = new Protbload::ADD;
    addmessage->ParseFromString(data);
    printf("agv1:%d   agv2:%d \n", addmessage->agv1(), addmessage->agv2());

    std::string fstr;
    Protbload::RESULT *resultmessage = new Protbload::RESULT;
    resultmessage->set_answer(addmessage->agv1() + addmessage->agv2());
    resultmessage->SerializeToString(&fstr);
    int flen = fstr.size();
    Multiplexer->sendMessage(10, flen, fstr.c_str());
}

void clientcallbak(Multiplexer *Multiplexer, char *data, int len, int ero)
{
    Protbload::RESULT *resultmessage = new Protbload::RESULT;
    resultmessage->ParseFromString(data);
    printf("result==%d \n", resultmessage->answer());
}

void startcallback(void *)
{
    LOG(LOG_INFO_LEVEL, LOG_SOURCE_MULTIPLEXER, "entity success start\n");
}

bool flag2 = false;

void fun2(void *a) { flag2 = true; }
int main()
{
    set_ndsl_log_sinks(LOG_SOURCE_ALL, LOG_OUTPUT_FILE);
    // 启动服务
    // 初始化EPOLL
    EventLoop loop;
    loop.init();

    // unsigned short p = 8888;
    struct SocketAddress4 servaddr("0.0.0.0", 8456);

    TcpAcceptor *tAc = new TcpAcceptor(&loop);
    tAc->start(servaddr);

    // 准备接收的数据结构
    struct sockaddr_in rservaddr;
    bzero(&rservaddr, sizeof(rservaddr));
    socklen_t addrlen;

    TcpConnection *Conn = new TcpConnection();
    tAc->onAccept(Conn, (SA *) &rservaddr, &addrlen, fun2, NULL, NULL);
    printf("hereeeeeeeeeeeeee\n");
    loop.quit();
    loop.loop(&loop);
    // 启动一个客户端
    struct SocketAddress4 clientservaddr("127.0.0.1", 8456);
    TcpConnection *serverconn;

    TcpClient *pCli = new TcpClient();
    serverconn = pCli->onConnect(&loop, true, &clientservaddr);
    Multiplexer *clientmulti = new Multiplexer(Conn);
    Multiplexer *servermulti = new Multiplexer(serverconn);
    // 添加中断
    loop.quit();
    loop.loop(&loop);

    /******
     * addInsertwork()测试
     *****/
    int clientid = 10;
    int serverid = 12;
    Entity *client = new Entity(clientid, clientcallbak, clientmulti);
    Entity *server = new Entity(serverid, servercallbak, servermulti);
    server->start(startcallback, NULL);
    client->start(startcallback, NULL);
    loop.quit();
    loop.loop(&loop);
    /*********************************
     * 客户端服务器实体测试：
     ********************************/
    int a, b;
    printf("input the agv1 and agv2 of ADD equation: \n");
    scanf("%d %d", &a, &b);
    std::string pstr;
    Protbload::ADD *addmessage = new Protbload::ADD;
    addmessage->set_agv1(a);
    addmessage->set_agv2(b);
    addmessage->SerializeToString(&pstr);
    int mlen = pstr.size();

    client->multiplexer_->sendMessage(serverid, mlen, pstr.c_str());

    loop.quit();
    loop.loop(&loop);
    loop.quit();
    loop.loop(&loop);
    return 0;
}