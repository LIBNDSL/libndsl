#ifndef __FTPSERVER_H__
#define __FTPSERVER_H__

#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/ELThreadpool.h"
#include "ndsl/net/EventLoop.h"

using namespace ndsl;
using namespace net;

class FtpServer {
    public:
	  static TcpAcceptor* tac;
	  static TcpAcceptor* dataTac;
	  static ELThreadpool* threadPool;
	  static EventLoop* loop;
	  
    public:
	  FtpServer ();
	  virtual ~FtpServer ();
	 
	public:
	  void start();
	  static void onConnection(void *p);
	  static void onDataConnection(void *p);
	  
};

#endif // __FTPSERVER_H__

