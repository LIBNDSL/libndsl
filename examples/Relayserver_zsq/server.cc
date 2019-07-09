/**
 * @file server.cc
 * @brief
 *
 * @author zsq
 * @email 1575033031@qq.com
 */

#include "ndsl/net/EventLoop.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/config.h"
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

using namespace ndsl;
using namespace net;

#define BUFSIZE 16384

TcpConnection *conn;
char sbuf[BUFSIZE];
ssize_t len;

//static void mError(int a,Channel*) {printf("there is a error!\n");}

static void onMessage(void * a)
{
    printf("onMessage!\n");
    printf("sbuf = %s\n",sbuf);
    if(len > 0) conn->onSend(sbuf ,len,0,NULL,NULL);
    memset(sbuf, 0, sizeof(sbuf));
}
static void onConnection(void * a)
{
    printf("onConnection!\n");
    memset(sbuf, 0, sizeof(sbuf));
    len = 0;
    conn->onRecv(sbuf, &len, 0, onMessage, NULL);

}
int main(int argc,char**argv)
{
    if(argc < 3){
        fprintf(stderr,"Usage: server <address> <port>\n");
    }else{

        //初始化EPOLL
        EventLoop loop;
        int s = loop.init();
        if(s < 0) printf("loop init fail!\n");

        struct SocketAddress4 servaddr(
            argv[1],static_cast<unsigned short>(atoi(argv[2])));

        TcpAcceptor *tAc = new TcpAcceptor(&loop);
        s = tAc->start(servaddr);
        if(s < 0) printf("tAc->start fail!\n");

        struct sockaddr_in rservaddr;
        bzero(&rservaddr,sizeof(rservaddr));
        socklen_t addrlen;

        conn = new TcpConnection();
        //conn->onError(mError);
        tAc->onAccept(
            conn,(struct sockaddr *)&rservaddr,&addrlen,onConnection,NULL);

        
        //运行
        printf("loop start!\n");
        EventLoop::loop(&loop);
    }
    return 0;
}
