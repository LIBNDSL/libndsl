/*
 * @file: Socket.h
 * @author: peng jun
 * @date: 2018-12-22 20:42:13
 * @description: Socket option
 */

#include <netinet/tcp.h> //tcp_info

#ifndef NDSL_NET_SOCKETOPT_H
#define NDSL_NET_SOCKETOPT_H

namespace ndsl{
namespace net{

struct SocketOpt{

public:
    //获得tcp信息
    //成功返回0/S_OK，失败返回-1/S_FALSE
    int getTcpInfo(int sockfd,struct tcp_info*) const;
    int getTcpInfoString(int sockfd,char* buf,int len) const;

    int setTcpNoDelay(int sockfd,bool on);
    int setReuseAddr(int sockfd,bool on);
    int setReusePort(int sockfd,bool on);
    int setKeepAlive(int sockfd,bool on);

};

} // namespace net
} // namespace ndsl


#endif
