////
// @file Ftpcliet.h
// @brief
// 
//
// @author lanry
// @email luckylanry@163.com
//
#ifndef __NDSL_NET_FTPCLIENT_H__
#define __NDSL_NET_FTPCLIENT_H__

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/Epoll.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/TcpChannel.h"
#include "ndsl/net/SocketAddress.h"

using namespace ndsl;
using namespace net;

class  FtpClient
{
  private:
    int sockfd;
    char cmd[256];
  public:
    FtpClient();
    ~FtpClient();
    int sendRequest(TcpConnection *conn);
    const char *recvReply(TcpConnection *conn,struct SocketAddress4 ftpservaddr);
    int dataConn(TcpClient *pCli);
     //显示当前目录 
    int ftp_pwd(char* buff); 
    //更改目录 
    int ftp_cd(TcpConnection *con,char* dir); 
    //返回上层目录 
    int ftp_cdup(); 
    //创建目录 
    int ftp_mkdir(char* dir); 
    //删除目录 
    int ftp_rmdir(char* dir); 
    int ftp_upload(TcpConnection *con,char* localfile,char* remotepath,char* remotefilename);
    int ftp_download(TcpConnection *con,char *localfile,char* remotefile);

};

#endif // __NDSL_NET_FTPCLIENT_H__