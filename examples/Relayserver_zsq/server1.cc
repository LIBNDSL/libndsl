/**
 * @file server1.cc
 * @brief
 *
 * @author zsq
 * @email 1575033031@qq.com
 */

#include "ndsl/net/Epoll.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/TcpChannel.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/Multiplexer.h"
#include "ndsl/net/Entity.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/utils/Log.h"
#include "Protbload.pb.h"

using namespace ndsl;
using namespace net;
using namespace utils;
using namespace Protbload;



int serverid = 16;
int clientid = 14;
int relayclientid = 18;
int relayserverid = 20;

TcpAcceptor *tAc;

//static void mError(int a,Channel*) {printf("there is a error!\n");}

static void servercallback(Multiplexer *Multiplexer, char *data, int len, int ero)
{
    printf("servercallback!\n");
    Protbload::ADD *addmessage = new Protbload::ADD;
    addmessage->ParseFromString(data);

    std::string fstr;
    Protbload::RESULT *resultmessage = new Protbload::RESULT;
    resultmessage->set_answer(addmessage->agv1() + addmessage->agv2());
    resultmessage->set_id(addmessage->id());
    resultmessage->set_connfd(addmessage->connfd());

    printf("argv1 and argv2 of ADD: %d  %d\n",addmessage->agv1(),addmessage->agv2());
    //printf("id %d \n",addmessage->id());
    //printf("connfd %d \n",addmessage->connfd());

    printf("result = %d \n", resultmessage->answer());
    resultmessage->SerializeToString(&fstr);
    int flen = fstr.size();
    Multiplexer->sendMessage(relayclientid, flen, fstr.c_str());

}
static void onConnection(void * para)
{
    printf("onConnection!\n");
    TcpConnection *conn = (TcpConnection *) para;
    Multiplexer *servermulti = new Multiplexer(conn);
    Entity *server = new Entity(serverid, servercallback, servermulti);
    server->start();

    //TcpConnection *con1 = new TcpConnection();
    //tAc->onAccept(con1, NULL, NULL, onConnection, con1);
}
int main(int argc,char **argv)
{
    set_ndsl_log_sinks(LOG_SOURCE_ALL, LOG_OUTPUT_TER);
    if(argc < 3)
    {
        fprintf(stderr, "Usage: server <address> <port>\n");
    }else{

        //初始化EPOLL
        EventLoop loop;
        int s = loop.init();
        if(s < 0) {printf("loop init fail!\n"); }

        //地址结构
        struct SocketAddress4 servaddr(
            argv[1],static_cast<unsigned short>(atoi(argv[2])));

        tAc = new TcpAcceptor(&loop);
        s = tAc->start(servaddr);
        if(s < 0) {printf("TcpAcceptor start fail!\n");} 

        struct sockaddr_in rservaddr;
        bzero(&rservaddr,sizeof(rservaddr));
        socklen_t addrlen;
      
        TcpConnection* conn = new TcpConnection();
        //conn->onError(mError);
        tAc->onAccept(
            conn,(struct sockaddr *)&rservaddr,&addrlen,onConnection,conn,NULL);

        //运行
        printf("loop start!\n");
        EventLoop::loop(&loop);

    }
    return 0;
}