/**
 * @file client.cc
 * @brief
 *
 * @author zzt
 * @emial 429834658@qq.com
 **/
#include "ndsl/net/Entity.h"
#include "ndsl/net/Multiplexer.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/net/TcpClient.h"
#include "Protbload.pb.h"

using namespace ndsl;
using namespace net;
using namespace Protbload;

void clientcallbak(Multiplexer *Multiplexer, char *data, int len, int ero)
{
    Protbload::RESULT *resultmessage = new Protbload::RESULT;
    resultmessage->ParseFromString(data);
    printf("result = %d \n", resultmessage->answer());
    int a, b;
    printf("\ninput the agv1 and agv2 of ADD equation: \n");
    scanf("%d %d", &a, &b);
    std::string pstr;
    Protbload::ADD *addmessage = new Protbload::ADD;
    addmessage->set_agv1(a);
    addmessage->set_agv2(b);
    addmessage->SerializeToString(&pstr);
    int mlen = pstr.size();

    Multiplexer->sendMessage(1, mlen, pstr.c_str());
}

int main()
{
    // 启动服务
    EventLoop loop;
    loop.init();

    struct SocketAddress4 clientservaddr("127.0.0.1", 8080);
    TcpConnection *conn2s;
    TcpClient *pCli = new TcpClient();
    conn2s = pCli->onConnect(&loop, true, &clientservaddr);

    Multiplexer *multi = new Multiplexer(conn2s);

    Entity *client = new Entity(1, clientcallbak, multi);

    int a, b;
    printf("input the agv1 and agv2 of ADD equation: \n");
    scanf("%d %d", &a, &b);
    std::string pstr;
    Protbload::ADD *addmessage = new Protbload::ADD;
    addmessage->set_agv1(a);
    addmessage->set_agv2(b);
    addmessage->SerializeToString(&pstr);
    int mlen = pstr.size();

    client->multiplexer_->sendMessage(1, mlen, pstr.c_str());

    loop.loop(&loop);
    return 0;
}