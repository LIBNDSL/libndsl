#include <fcntl.h>
#include <iostream>
#include <error.h>
#include <sys/socket.h>
#include "Connection.h"
#include "CommandHandler.h"
#include "config_ftp.h"
#include "ndsl/utils/Error.h"
#include "ndsl/utils/Log.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/config.h"

using namespace ndsl::ftp;
using namespace std;

Connection::Connection(){}

Connection::~Connection(){}

void commRecvCb(void *pparam)
{
    struct Param *param = static_cast<struct Param *>(pparam);
    param -> cher -> parseCommand(param -> buffer, param);
    memset(param -> buffer, 0, 128);
    *(param -> buflen) = 128;
    param -> pconn -> onRecv(param -> buffer, param -> buflen, 
            0, commRecvCb, param);
}

void acceptCb(void *pparam)
{
    struct Param *cbparams = static_cast<struct Param *>(pparam); 

    struct Param *param = new Param;
    // cbparams -> tac = param -> tac;
    // cbparams -> loop = param -> loop;

    char *buffer = new char[128];
    SocketAddress4 *recvaddr = new SocketAddress4;
    param -> paddr = recvaddr;
    param -> buffer = buffer;
    CommandHandler *cher = new CommandHandler;
    // param -> pdataconn = new TcpConnection();
    param -> cher = cher;
    ssize_t *bufferlen = new ssize_t[128];
    param -> buflen = bufferlen;
    param -> pconn = cbparams -> pconn;
    param -> loop = cbparams -> loop;
    CommandHandler::writeState(param -> pconn,(string)"220 need account\r\n");
    if (param -> pconn -> onRecv(buffer,bufferlen,0, commRecvCb, 
                (void *)param) != S_OK) 
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "onrecv fail");

    // recv accept
    delete(cbparams -> paddr);
    SocketAddress4 *clientaddr = new SocketAddress4;
    TcpConnection *conn = new TcpConnection();
    cbparams -> pconn = conn;
    cbparams -> paddr = clientaddr;
    socklen_t addrlen = sizeof(sockaddr_in);
    (cbparams -> tac) -> onAccept(conn, (SA *)clientaddr, &addrlen, 
            acceptCb, (void *)cbparams);
}
    
int Connection::ftpServerInit()
{
    if (contrloop_.init() != S_OK) return S_FALSE;
    SocketAddress4 *servaddr = new SocketAddress4;
    servaddr -> setPort(21);
    TcpAcceptor *tac = new TcpAcceptor(&contrloop_);
    if(tac -> start(*servaddr) != S_OK) 
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "accept start fail");
    struct Param *cbparam = new Param;
    cbparam -> tac = tac;
    cbparam -> loop = &contrloop_;
    TcpConnection *conn = new TcpConnection();
    cbparam -> pconn = conn;
    cbparam -> paddr = servaddr;
    socklen_t len = sizeof(sockaddr_in);
    tac -> onAccept(conn, (SA *)servaddr, &len, acceptCb, (void *)cbparam);
    return S_OK;
}

// server connect to client with port 20
int Connection::portConn(TcpConnection *conn, Param *connparam)
{
    SocketAddress4 *clientaddr = connparam -> paddr;
    int datafd = socket(AF_INET, SOCK_STREAM, 0);
    setNonBlock(datafd);
    SocketAddress4 localaddr;
    localaddr.setPort(20);
    int opt = 1;
    setsockopt(datafd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
    if (bind(datafd, (SA *)&localaddr, sizeof(localaddr)) < 0)
    {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "bind local port fail");
        cout<<"bind 20 port error"<<endl;
        CommandHandler::writeState(connparam -> pconn, (string)"425 can not connet client\r\n");
        return S_FALSE;
    }
    char buf[20];
    clientaddr -> getIP(buf);
    // cout<<buf<<htons(clientaddr -> sin_port)<<endl;
    if ( connect(datafd, (SA*)clientaddr, sizeof(*clientaddr)) < 0)
    {
        if (errno != EINPROGRESS)
        {
            LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "connect fail");
            cout<<"connect error"<<strerror(errno)<<endl;
            CommandHandler::writeState(connparam -> pconn, (string)"425 can not connet client\r\n");
            return S_FALSE;
        }
    }
    CommandHandler::writeState(connparam -> pconn, (string)"150 connect client successfully\r\n");

    // create a tcpconnection
    // conn = new TcpConnection();
    if (conn == NULL)
    {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "create tcpconnection fail");
        CommandHandler::writeState(connparam -> pconn, 
                (string)"425 can not connet client\r\n");
        return S_FALSE;
    }
    if ( conn -> createChannel(datafd, connparam -> loop) < 0)
    {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "create channel fail");
        CommandHandler::writeState(connparam -> pconn, 
                (string)"425 can not connet client\r\n");
        return S_FALSE;
    }
    return S_OK;

}

int Connection::pasvConn(TcpConnection *conn, Param *param)
{
    SocketAddress4 server(IPADDRESS, PORT);
    TcpAcceptor *tAc = new TcpAcceptor(param -> loop);
    tAc -> start(server);

    // receive addr
    struct sockaddr_in rserveraddr;
    bzero(&rserveraddr, sizeof(rserveraddr));
    socklen_t addrlen = sizeof(rserveraddr);

    tAc -> onAccept(conn, (SA *)&rserveraddr, &addrlen, NULL, NULL);
    CommandHandler::writeState(param -> pconn, 
            (string)"200 entering passive mode(192,168,1,117,"+
            to_string( PORT/256)+","+to_string( PORT%256)+")\r\n");

    LOG(LOG_DEBUG_LEVEL, LOG_SOURCE_TCPCLIENT,"s%",
            (string)"200 entering passive mode(192,168,1,117,"+
            to_string( PORT/256)+","+to_string( PORT%256)+")\r\n");
    return S_OK;

}

int Connection::setNonBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0); //without it,other states may be cleaned
    if (flags < 0)
    {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "fcntl get state error\n");
        return S_FALSE;
    }
    flags |= O_NONBLOCK;
    if( fcntl(fd, F_SETFL, flags) < 0)
    {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPCLIENT, "fcntl set state error\n");
        return S_FALSE;             
    }
    return S_OK;
}
