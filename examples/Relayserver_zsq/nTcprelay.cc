/**
 * @file nTcprelay.cc
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
#include "ndsl/net/TcpClient.h"
#include "ndsl/net/Multiplexer.h"
#include "ndsl/net/Entity.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/utils/Log.h"
#include "Protbload.pb.h"
#include "ndsl/net/ELThreadpool.h"
#include <map>
#include <iostream>
#include <string>
using namespace ndsl;
using namespace net;
using namespace utils;
using namespace Protbload;


ELThreadpool *threadPool;
TcpAcceptor *tAc;
Multiplexer *clientmulti;
int serverid = 16;
int clientid = 14;
int relayclientid = 18;
int relayserverid = 20;
int sockfd;
std::map<int ,Multiplexer *> mapclient;
std::map<int ,Multiplexer *>::iterator iter;
std::map<int,int> mapsock;
std::map<int,int>::iterator itersock;

//static void mError(int a,Channel*) {printf("there is a error!\n");}

static void clientcallback(Multiplexer *Multiplexer, char *data, int len, int ero)
{
    //printf("clientcallback!\n");
    Protbload::RESULT *resultmessage = new Protbload::RESULT;
    resultmessage->ParseFromString(data);

    //printf("result == %d \n", resultmessage->answer());

    //printf("connfd %d \n",resultmessage->connfd());
 
    //printf("id == %d \n", resultmessage->id());

    for(iter = mapclient.begin(); iter != mapclient.end();++iter)
    {
        //std::cout << "fd = " << iter->first << std::endl;
    }
    itersock = mapsock.find(resultmessage->id());
    iter = mapclient.find(itersock->second);
    iter->second->sendMessage(resultmessage->id(), len, data);

    //servermulti->sendMessage(resultmessage->id(), len, data);


}

static void servercallback(Multiplexer *Multiplexer, char *data, int len, int ero)
{
    //printf("servercallback!\n");
    LOG(LOG_DEBUG_LEVEL,LOG_SOURCE_TCPCONNECTION,"servercallback");
    Protbload::ADD *addmessage = new Protbload::ADD;
    addmessage->ParseFromString(data);

    std::string fstr;
  
    printf("argv1 and argv2 of ADD: %d  %d\n",addmessage->agv1(),addmessage->agv2());
    //printf("id == %d \n", addmessage->id());


    std::string pstr;
    Protbload::ADD *addmessage1 = new Protbload::ADD;
    addmessage1->set_agv1(addmessage->agv1());
    addmessage1->set_agv2(addmessage->agv2());
    addmessage1->set_id(addmessage->id());
    itersock = mapsock.find(addmessage->id());
    if(itersock == mapsock.end())
    {
        mapsock.insert(std::pair<int,int>(addmessage->id(),sockfd));
        addmessage1->set_connfd(sockfd);

    }else{
        addmessage1->set_connfd(itersock->second);
    }
    //printf("connfd == %d \n", addmessage1->connfd());

    addmessage1->SerializeToString(&pstr);
    int mlen = pstr.size();

    clientmulti->sendMessage(serverid, mlen, pstr.c_str());
}

static void onServerConnection(void * para)
{
    //printf("onServerConnection!\n");
    TcpConnection *conn = (TcpConnection *) para;
    sockfd = conn->pTcpChannel_->getFd();
    //printf("sockfd = %d\n",sockfd);

    Multiplexer *servermulti = new Multiplexer(conn);

    mapclient.insert(std::pair<int,Multiplexer *>(sockfd,servermulti));

    Entity *server = new Entity(relayserverid, servercallback, servermulti);

    server->start();

    TcpConnection *con1 = new TcpConnection();
    tAc->onAccept(con1, NULL, NULL, onServerConnection, con1,threadPool->getNextEventLoop());
}



int main(int argc,char **argv)
{
    set_ndsl_log_sinks(LOG_SOURCE_ALL, LOG_OUTPUT_TER);
    if(argc < 4)
    {
        fprintf(stderr,"Usage: %s <host_ip> <port> <listen_port>\n",argv[0]);
    }else{
        const char *ip = argv[1];
        const char *ip1 = "0.0.0.0";
        //中继服务器作为客户端
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        struct SocketAddress4 servaddr(ip,port);

        //中继服务器作为服务器
        uint16_t  acceptPort = static_cast<uint16_t>(atoi(argv[3]));
        struct SocketAddress4 listenaddr(ip1,acceptPort);

        // //
        EventLoop *loop = new EventLoop;
        loop->init();
        ELThread thread(loop);

        // 线程还没加入 后面加
        int threadCount = atoi(argv[4]);

        // 初始化线程池
        threadPool = new ELThreadpool();
        threadPool->setMaxThreads(threadCount);

        threadPool->start();

        //初始化EPOLL
        EventLoop loop1;
        int s1 = loop1.init();
        if(s1 < 0) printf("loop init fail!\n");


        //作为服务器(socket -> bind -> listen)
        tAc = new TcpAcceptor(loop);
        int s = tAc->start(listenaddr);
        if(s < 0) printf("tAc->start fail!\n");

        struct sockaddr_in rservaddr;
        bzero(&rservaddr,sizeof(rservaddr));
        socklen_t addrlen;

        TcpConnection *serverconn_ = new TcpConnection();
        //serverconn_-> onError(mError);
        tAc->onAccept(
            serverconn_,(struct sockaddr *)&rservaddr,&addrlen,onServerConnection,serverconn_,NULL);

        // //作为客户端
        TcpClient client_;
        TcpConnection *clientconn_ = client_.onConnect(&loop1,false,&servaddr);
        if(clientconn_ == NULL) {printf("start onConnect fail!\n");}

        clientmulti = new Multiplexer(clientconn_ );

        // printf("after new multiplexer!\n");


        Entity *client = new Entity(relayclientid, clientcallback, clientmulti);

        client->start();

        thread.run();
        //运行
        printf("loop1 start!\n");
        EventLoop::loop(&loop1);

    }
    return 0;
}