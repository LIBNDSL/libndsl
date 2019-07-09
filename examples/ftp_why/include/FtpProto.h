#ifndef __FTPPROTO_H__
#define __FTPPROTO_H__

#include <iostream>
#include "Session.h"
#include "FtpConnection.h"

using namespace std;
using namespace ndsl;
using namespace net;
//struct FtpCmd{
//    const char* cmd;
//    void (*cmdHandle)(Session*);
//};

class FtpProto {
    public:
//	  FtpCmd* ftpCmds_;
	  
    public:
	  FtpProto ();
	  virtual ~FtpProto ();
	
	public:
	  static void doUser(Session* sess);
	  static void doPass(Session* sess);
	  static void doSyst(Session* sess);
	  static void doFeat(Session* sess);
	  static void doPwd(Session* sess);
	  static void doType(Session* sess);
	  static void doPasv(Session* sess);
//	  static void doPort(Session* sess);
	  static void doList(Session* sess);
	  static void doCwd(Session* sess);
	  static void doRetr(Session* sess);
	  static void doStor(Session* session);
	public:
	  void handle(Session* sess);
	  static void revcCallback(void* p);
	  
	  static int listCommon(Session* sess);
//	  int getTransferFd(Session* session);
//	  int portActive(Session* session);
//	  int pasvActive(Session* session);
//	  int getPortFd(Session* session);
//	  int getPasvFd(Session* session);
	  
};

#endif // __FTPPROTO_H__

