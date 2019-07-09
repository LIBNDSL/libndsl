/*
 * @file: Socket.cc
 * @author: peng jun
 * @date: 2018-12-25 14:00:43
 * @description: Socket option
 */
#include "ndsl/net/SocketOpt.h"
#include "ndsl/utils/Log.h"

#include <sys/socket.h> //getSockopt
#include <sys/types.h> // getSockopt
#include <netinet/tcp.h> //tcp_info
#include <netinet/in.h> //IPPROTO_TCP

#include <strings.h> //bzero
#include <stdio.h> // snprintf


namespace ndsl{
namespace net{
  int SocketOpt::getTcpInfo(int sockfd,struct tcp_info *tcpi) const{
      socklen_t len = sizeof(struct tcp_info);
      bzero(tcpi,len);
      //成功返回0，失败返回-1
      return getsockopt(sockfd,SOL_TCP,TCP_INFO,tcpi,&len);
  }

  int SocketOpt::getTcpInfoString(int sockfd,char* buf,int len) const{
      struct tcp_info tcpi;
      int ok = getTcpInfo(sockfd,&tcpi);
      if(ok == 0 ){
          snprintf(buf, len, "unrecovered=%u "
                  "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                  "lost=%u retrans=%u rtt=%u rttvar=%u "
                  "sshthresh=%u cwnd=%u total_retrans=%u",
                  tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
                  tcpi.tcpi_rto,          // Retransmit timeout in usec
                  tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
                  tcpi.tcpi_snd_mss,
                  tcpi.tcpi_rcv_mss,
                  tcpi.tcpi_lost,         // Lost packets
                  tcpi.tcpi_retrans,      // Retransmitted packets out
                  tcpi.tcpi_rtt,          // Smoothed round trip time in usec
                  tcpi.tcpi_rttvar,       // Medium deviation
                  tcpi.tcpi_snd_ssthresh,
                  tcpi.tcpi_snd_cwnd,
                  tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
          
      }
      return ok;
  }

  int SocketOpt::setTcpNoDelay(int sockfd,bool on)
  {
    int optval = on ? 1 : 0;
    return setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
                &optval, static_cast<socklen_t>(sizeof optval));
    
  }

  int SocketOpt::setReuseAddr(int sockfd,bool on)
  {
    int optval = on ? 1 : 0;
    return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                &optval, static_cast<socklen_t>(sizeof optval));
    
  }

  int SocketOpt::setReusePort(int sockfd,bool on)
  {
  #ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,
                          &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on)
    {
      LOG(LOG_ERROR_LEVEL, LOG_SOURCE_SOCKETOP, "SO_REUSEPORT failed.");
    }
  #else
    if (on)
    {
      LOG(LOG_ERROR_LEVEL, LOG_SOURCE_SOCKETOP, "SO_REUSEPORT is not supported.");
    }
  #endif
    return ret;
  }

  int SocketOpt::setKeepAlive(int sockfd,bool on)
  {
    int optval = on ? 1 : 0;
    return setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE,
                &optval, static_cast<socklen_t>(sizeof optval));
    
  }

} // namespace net
} // namespace ndsl