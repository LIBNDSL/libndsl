/**
 * @file client.cc
 * @brief
 *
 * @author lanry
 * @email luckylanry@163.com
 */

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

#define BUFSIZE 2048
ssize_t len;

struct SocketAddress4 ftpservaddr;

FtpClient::FtpClient() { }
FtpClient::~FtpClient() { }

// client send a command request
int FtpClient::sendRequest(TcpConnection *con)
{   
    int length;
    // char sendbuf[BUFSIZE];
    // ssize_t len ;
    // char cmd[20];
    // scanf("%s\n",cmd);
    // len = sizeof(cmd);
    char *sendbuf = (char *) malloc(sizeof(char) * 5);
    strcpy(sendbuf, "PASV\0");
    length = con->onSend(sendbuf,strlen("PASV"),0,NULL,NULL);
    // length = send(sockfd, cmd, strlen(cmd), 0);
    // length = con->onSend(sendbuf,len,0,NULL,NULL);
    // client request's fd
    printf("%d\n",con->pTcpChannel_->getFd());
    printf("length:%d\n",length);

    //send函数通过sockfd接口发送cmd，发送成功返回发送的字节数  
    if (length <= 0)
    {
        printf("send cmd error ! \n");
        return S_FALSE; 
    }
    return S_OK;  
}
// receive server's responce
const char *FtpClient::recvReply(TcpConnection *conn,struct SocketAddress4 ftpservaddr)
{   
    int status =0;
    char recvbuf[BUFSIZE];
    ssize_t len1 = sizeof(recvbuf);
    // status= recv(sockfd,recvbuf,BUFSIZE,0);
    status = conn->onRecv(recvbuf,&len1,0,NULL,NULL);
    printf("%d\n",sockfd);
    // cmd concerning interpretion
    switch(status)
    {
	    case 220: { return "server ready.\n"; }
        case 200: { return "dispose commmand\n" ;}
        case 221: { return "exit\n";}
        case 502: { return "cmd send fail,not legally cmd \n " ;}
		case 226: { return "request file success\n"; }
        case 227: {
              // get server IP and address
             ftpservaddr.getIP(recvbuf);
             unsigned short port = ftpservaddr.getPort();
             printf("server's port is:%d",port);
             return "send PASV mode success\n"; 
             }
        default: { return "S_FALSE" ;}
    }
    return 0;
}

// send data connection
int FtpClient::dataConn(TcpClient *pCli)
{
    EventLoop loop;
    int dataConnfd;
    TcpConnection *conn1 = new TcpConnection();
    conn1= pCli->onConnect(&loop,true,&ftpservaddr);
    dataConnfd = pCli->sockfd_ ;
    if(conn1 == NULL)
    {
        printf("conn1 = NUll \n");
    }
    printf("%d,%d\n",pCli->sockfd_,dataConnfd);
	int ack = 1;
	//给服务器发送一个确认，告诉服务器客户端创建好了一条数据链路
	if(send(dataConnfd,(char*)&ack,sizeof(ack),0)<0) 
	{
		printf("client:ack write error:%d\n",errno);
		return S_FALSE;
	}
	return dataConnfd;     
}
int FtpClient::ftp_cd(TcpConnection *con,char* dir) 
{ 
    sprintf(cmd,"%s\r\n",dir); 
    int err = sendRequest(con); 
    if(err)return -1;  
    return 0; 
} 
 
int FtpClient::ftp_upload(TcpConnection *con,char* localfile,char* remotepath,char* remotefilename) 
{ 
    // remote path
    //ftp_mkdir(remotepath); 
    int err = ftp_cd(con,remotepath); 
    if(err)return -1;  
    sprintf(cmd,"STOR %s\r\n",remotefilename); 
    // send data request
    err = sendRequest(con); 
    if(err)return -1; 
    FILE* pf = fopen(localfile,"r"); 
    if(NULL==pf)return -1; 
    char sendbuf[256]; 
    size_t len = 0; 
    while((len = fread(sendbuf,1,255,pf))>0) 
    { 
        err = send(sockfd,sendbuf,len,0); 
        if(err<0)return -1; 
    } 
    close(sockfd); 
    fclose(pf); 
    return 0; 
} 
int FtpClient::ftp_download(TcpConnection *con,char* localfile,char* remotefile) 
{ 
    sprintf(cmd,"%s\r\n",remotefile); 
    int err = sendRequest(con); 
    if(err)return -1; 
  
    FILE* pf = fopen(localfile,"w"); 
    if(NULL==pf)return -1; 
    char recvbuf[256]; 
    int len = 0; 
    while((len = recv(sockfd,recvbuf,256,0))>0) 
    { 
        err = fwrite(recvbuf,len,1,pf); 
        if(len<0)return -1; 
    } 
    close(sockfd); 
    fclose(pf); 
   
    return 0; 
}
// int ftp_upload(char* localfile,char* remotepath,char* remotefilename)
// {
//     char send[BUFSIZE];
    
//     return 0; 
// }

// static int ftp_download(char *localfile,char* remotefile)
// {

// }