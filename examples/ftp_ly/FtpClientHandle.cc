#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include "Protbload.pb.h"
#include "ndsl/net/Multiplexer.h"
#include "ndsl/net/Entity.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/Epoll.h"
#include "ndsl/net/TcpChannel.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/TcpClient.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/config.h"
#include "FtpClient.h"

using namespace ndsl;
using namespace net;
using namespace Protbload;

TcpConnection *conn;
FtpClient *ftp; 

int main(int argc,char **argv)
{
    if(argc < 2){
        fprintf(stderr,"Usage: <address> <port> \n");
    }else{  
        //初始化EPOLL
        EventLoop loop;
        loop.init();
        //地址结构
        struct SocketAddress4 servaddr(
            argv[1],static_cast<unsigned short>(atoi(argv[2])));      // connect server port 21
        // start a client
        TcpClient* pCli = new TcpClient();
        conn = pCli->onConnect(&loop, true, &servaddr);
        if(conn == NULL)
        {
            printf("conn == NULL!\n");
        }else
        {
            printf("connect success\n");
            printf("connect fd:%d\n",conn->pTcpChannel_->getFd());
        }
        // command request
        //ftp->sendRequest(conn);
       char recvBuf[15];
        ssize_t recvLen;
        memset(recvBuf, 0, sizeof(recvBuf));
        conn->onRecv(
            recvBuf, &recvLen, 0, NULL,NULL);

        printf("lalalalalala\n");
        // const char *output = ftp->recvReply(conn,servaddr);
        // printf("%s\n",output);
        
        // send datd connection;;


        // ftp->dataConn(pCli);

        loop.loop(&loop);
    }
    return 0;
}
