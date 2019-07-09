#include "ndsl/net/SocketOpt.h"

// #define CATCH_CONFIG_MAIN
#include "../catch.hpp"

#include <sys/socket.h> //socket()
#include <netinet/tcp.h> //tcp_info
#include <netinet/in.h> //IPPROTO_TCP
#include <regex> //regex

using namespace ndsl;
using namespace net;

TEST_CASE("test socket option:getTcpInfo"){
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    REQUIRE(sockfd >= 0);

    struct SocketOpt sock;
    struct tcp_info tcpi;
    REQUIRE(sock.getTcpInfo(sockfd,&tcpi) == 0);
}

TEST_CASE("test socket option:getTcpInfoString"){
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    REQUIRE(sockfd >= 0);

    struct SocketOpt sock;

    char buf[2048];
    int ok = sock.getTcpInfoString(sockfd,buf,2048);
    REQUIRE(ok == 0);

    REQUIRE(std::regex_match(buf,std::regex("^unrecovered=[0-9]+ rto=[0-9]+ ato=[0-9]+ snd_mss=[0-9]+ rcv_mss=[0-9]+ lost=[0-9]+ retrans=[0-9]+ rtt=[0-9]+ rttvar=[0-9]+ sshthresh=[0-9]+ cwnd=[0-9]+ total_retrans=[0-9]+")));
    
}

TEST_CASE("test socket option:setTcpNoDelay"){
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    REQUIRE(sockfd >= 0);

    struct SocketOpt sock;

    bool val;
    socklen_t len = sizeof(val);
    REQUIRE(getsockopt(sockfd,IPPROTO_TCP, TCP_NODELAY,&val,&len) == 0);
    REQUIRE(val == false);
    sock.setTcpNoDelay(sockfd,true);
    REQUIRE(getsockopt(sockfd,IPPROTO_TCP, TCP_NODELAY,&val,&len) == 0);
    REQUIRE(val == true);
    sock.setTcpNoDelay(sockfd,false);
    REQUIRE(getsockopt(sockfd,IPPROTO_TCP, TCP_NODELAY,&val,&len) == 0);
    REQUIRE(val == false);
}

TEST_CASE("test socket option:setReuseAddr"){
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    REQUIRE(sockfd >= 0);

    struct SocketOpt sock;
 
    bool val;
    socklen_t len = sizeof(val);
    REQUIRE(getsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&val,&len) == 0);
    REQUIRE(val == false);
    sock.setReuseAddr(sockfd,true);
    REQUIRE(getsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&val,&len) == 0);
    REQUIRE(val == true);
    sock.setReuseAddr(sockfd,false);
    REQUIRE(getsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&val,&len) == 0);
    REQUIRE(val == false);
}

TEST_CASE("test socket option:setReusePort"){
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    REQUIRE(sockfd >= 0);

    struct SocketOpt sock;
 
    bool val;
    socklen_t len = sizeof(val);
    REQUIRE(getsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,&val,&len) == 0);
    REQUIRE(val == false);
    sock.setReusePort(sockfd,true);
    REQUIRE(getsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,&val,&len) == 0);
    REQUIRE(val == true);
    sock.setReusePort(sockfd,false);
    REQUIRE(getsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,&val,&len) == 0);
    REQUIRE(val == false);
}

TEST_CASE("test socket option:setKeepAlive"){
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    REQUIRE(sockfd >= 0);

    struct SocketOpt sock;
 
    bool val;
    socklen_t len = sizeof(val);
    REQUIRE(getsockopt(sockfd,SOL_SOCKET,SO_KEEPALIVE,&val,&len) == 0);
    REQUIRE(val == false);
    sock.setKeepAlive(sockfd,true);
    REQUIRE(getsockopt(sockfd,SOL_SOCKET,SO_KEEPALIVE,&val,&len) == 0);
    REQUIRE(val == true);
    sock.setKeepAlive(sockfd,false);
    REQUIRE(getsockopt(sockfd,SOL_SOCKET,SO_KEEPALIVE,&val,&len) == 0);
    REQUIRE(val == false);
}