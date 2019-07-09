#ifndef __SESSION_H__
#define __SESSION_H__

#include "FtpConnection.h"
#include "Common.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/TcpAcceptor.h"

using namespace ndsl;
using namespace net;
class FtpConnection;

class Session {
    public:
	  // control connect
	  FtpConnection* ctrlConn;
	  int uid;
	  char cmdline[MAX_COMMAND_LINE];
	  char cmd[MAX_COMMAND];
	  char arg[MAX_ARG];
	  
	  // data connection
	  FtpConnection* dataConn;
	  int isPasv;
	  int localFd;
	  char* recvBuf;
	  ssize_t len;
	  int flag;
	  
	  // ipc
	  int parentFd;
	  int childFd;
	  
	  // status
	  int isASCII;
	  // loop
	  EventLoop* loop;
	  // dataAcceptor
	  TcpAcceptor* dataTac;
	  
    public:
	  Session ();
	  virtual ~Session ();
	 
	public:
	  static void beginSession(void* p);
	  static void dataTrans(void* p);
	  
};

#endif // __SESSION_H__

