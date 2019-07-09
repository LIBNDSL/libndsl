#include <string.h>

#include "Session.h"
#include "FtpConnection.h"
#include "FtpProto.h"

#include <iostream>
using namespace std;

Session::Session (){
	// control connect
	ctrlConn = NULL;
	uid = 0;
	memset(cmdline, 0, sizeof(cmdline));
	memset(cmd, 0, sizeof(cmd));
	memset(arg, 0, sizeof(arg));
	
	// data connection
	dataConn = NULL;
	isPasv = -1;
	localFd = -1;
	recvBuf = NULL;
	len = -1;
	flag = -1;

	// ipc
	parentFd = -1;
	childFd = -1;
	
	// status
	isASCII = 0;
	// loop 
	loop = NULL;
}
Session::~Session (){}

void Session::beginSession(void* p){
	Session *sess = static_cast<Session *>(p);
	FtpProto ftpProto;
	ftpProto.handle(sess);
}
