#ifndef __FTPCONNECTION_H__
#define __FTPCONNECTION_H__

#include "Session.h"
#include "FtpServer.h"
#include "ndsl/net/TcpConnection.h"

using namespace ndsl;
using namespace net;

class Session;

class FtpConnection : public TcpConnection{
    public:
	  Session* sess;
	  
    public:
	  FtpConnection ();
	  virtual ~FtpConnection ();
	 
	public:
	  static void storDataCallback(void* p);
	  static void errorHandle(void* p, int err);
	  static void dataConnection(Session* sess);
	  
};

#endif // __FTPCONNECTION_H__

