#ifndef __FTPREPLY_H__
#define __FTPREPLY_H__

#include "FtpConnection.h"

class FtpReply {
    private:
	  /* data */
	  
    public:
	  FtpReply ();
	  virtual ~FtpReply ();
	 
	public:
	  static void reply(FtpConnection* conn, int num, const char* text);
	  static void lreply(FtpConnection* conn, int num, const char* text);
	  static void sreply(FtpConnection* conn, const char* text);
	  static void replyList(FtpConnection* conn, const char* text);
	  
};

#endif // __FTPREPLY_H__

