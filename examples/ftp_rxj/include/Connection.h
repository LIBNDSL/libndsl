////
// @file Connection.h
// @brief package connection
//
// @author ranxiangjun
// @email ranxianshen@gmail.com
//
#ifndef NDSL_EXAMPLE_CONNECTION_H
#define NDSL_EXAMPLE_CONNECTION_H

#include "../../../include/ndsl/net/EventLoop.h"
#include "../../../include/ndsl/net/TcpAcceptor.h"
#include "../../../include/ndsl/net/TcpConnection.h"
#include "../../../include/ndsl/net/SocketAddress.h"

using namespace ndsl::net;

namespace ndsl{
namespace ftp{
class CommandHandler;

struct Param
{
    TcpAcceptor *tac;
    TcpAcceptor *datatac;
    EventLoop *loop;
    SocketAddress4 *paddr;
    TcpConnection *pconn;
    TcpConnection *pdataconn;
    char *buffer;
    ssize_t *buflen;
    CommandHandler *cher;
    int port;
};

class Connection
{

  public:
    Connection();
    ~Connection();
    int setNonBlock(int );
    int ftpServerInit();
    int portConn(TcpConnection *conn, Param *);
    int pasvConn(TcpConnection *conn, Param *);

  private:
    Param loopandtac_;
  
  public:
    EventLoop contrloop_;
};

}   // ftp
}   // ndsl

#endif   // NDSL_EXAMPLE_CONNECTION_H
