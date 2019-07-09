/**
 * @file client1.cc
 * @brief
 *
 * @author zsq
 * @email 1575033031@qq.com
 */

#include "ndsl/net/Entity.h"
#include "ndsl/net/Multiplexer.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/TcpChannel.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/TcpClient.h"
#include "ndsl/net/Epoll.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/utils/Log.h"
#include "Protbload.pb.h"

using namespace ndsl;
using namespace net;
using namespace utils;
using namespace Protbload;

int clientid = 22;
int serverid = 16;
int relayclientid = 18;
int relayserverid = 20;

static void clientcallback(Multiplexer *Multiplexer, char *data, int len, int ero)
{
    printf("clientcallback!\n ");
    Protbload::RESULT *resultmessage = new Protbload::RESULT;
    resultmessage->ParseFromString(data);
    printf("result==%d \n", resultmessage->answer());
    int a, b;
    printf("\ninput the agv1 and agv2 of ADD: \n");
    scanf("%d %d", &a, &b);
    std::string pstr;
    Protbload::ADD *addmessage = new Protbload::ADD;
    addmessage->set_agv1(a);
    addmessage->set_agv2(b);
    addmessage->set_id(22);

    addmessage->SerializeToString(&pstr);
    int mlen = pstr.size();

    Multiplexer->sendMessage(relayserverid, mlen, pstr.c_str());

}



int main(int argc,char **argv)
{
    set_ndsl_log_sinks(LOG_SOURCE_ALL, LOG_OUTPUT_TER);
    if(argc < 2){
        fprintf(stderr,"Usage: %s <address> <port> \n",argv[0]);
    }else{  

        //初始化EPOLL
        EventLoop loop;
        loop.init();
        
        //地址结构
        struct SocketAddress4 servaddr(
            argv[1],static_cast<unsigned short>(atoi(argv[2])));


        TcpConnection *conn;
        TcpClient* pCli = new TcpClient();
        conn = pCli->onConnect(&loop, false, &servaddr);

        if(conn == NULL)
        {
            printf("conn == NULL!\n");
        }  

        Multiplexer *clientmulti = new Multiplexer(conn);

        printf("after new multiplexer!\n");

        Entity *client = new Entity(clientid, clientcallback, clientmulti);
        client->start();

        //client->pri();

        //sleep(10);
        int a , b;
        printf("input the agv1 and agv2 of ADD: \n");
        scanf("%d %d", &a, &b);
        std::string pstr;
        Protbload::ADD *addmessage = new Protbload::ADD;
        addmessage->set_agv1(a);
        addmessage->set_agv2(b);
        addmessage->set_id(22);

        addmessage->SerializeToString(&pstr);
        int mlen = pstr.size();
           
        client->multiplexer_->sendMessage(relayserverid, mlen, pstr.c_str());



        printf("after sendmessage!\n");

        printf("loop start!\n");
        loop.loop(&loop);

    }
    return 0;
}