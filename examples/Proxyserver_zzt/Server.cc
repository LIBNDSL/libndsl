/**
 * @file server.cc
 * @brief
 *
 * @author zzt
 * @emial 429834658@qq.com
 **/
#include "ndsl/net/Entity.h"
#include "ndsl/net/Multiplexer.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/SocketAddress.h"
#include "Protbload.pb.h"
#include "ndsl/net/TcpClient.h"

using namespace ndsl;
using namespace net;
using namespace Protbload;

void servercallbak(Multiplexer *Multiplexer, char *data, int len, int ero)
{
    LOG(LOG_INFO_LEVEL, LOG_SOURCE_ENTITY, "in the servercallback\n");
    Protbload::ADD *addmessage = new Protbload::ADD;
    addmessage->ParseFromString(data);

    std::string fstr;
    Protbload::RESULT *resultmessage = new Protbload::RESULT;
    resultmessage->set_answer(addmessage->agv1() + addmessage->agv2());
    // printf("the answer = %d\n", resultmessage->answer());
    resultmessage->set_id(addmessage->id());
    resultmessage->SerializeToString(&fstr);
    int flen = fstr.size();
    Multiplexer->sendMessage(1, flen, fstr.c_str());
}

void AcceptCallback(void *para)
{
    TcpConnection *conn2c = (TcpConnection *) para;
    Multiplexer *smulti = new Multiplexer(conn2c);
    Entity *server = new Entity(1, servercallbak, smulti);
    server->start();
}

int main()
{
    set_ndsl_log_sinks(
        LOG_SOURCE_MULTIPLEXER | LOG_SOURCE_EVENTLOOP |
            LOG_SOURCE_TCPCONNECTION | LOG_SOURCE_TCPCHANNEL |
            LOG_SOURCE_TCPACCETPOR,
        LOG_OUTPUT_TER);
    // 启动服务
    EventLoop loop;
    loop.init();

    struct SocketAddress4 servaddr("0.0.0.0", 9999);
    TcpAcceptor *tAc = new TcpAcceptor(&loop);
    tAc->start(servaddr);

    // 准备接收的数据结构
    struct sockaddr_in rservaddr;
    bzero(&rservaddr, sizeof(rservaddr));
    socklen_t addrlen;

    TcpConnection *conn2c = new TcpConnection();
    tAc->onAccept(conn2c, (SA *) &rservaddr, &addrlen, AcceptCallback, conn2c);
    loop.loop(&loop);

    return 0;
}