////
// @file FtpServer.h
// @brief
// 
//
// @author lanry
// @email luckylanry@163.com
//
#ifndef __FTPSERVER_H__
#define __FTPSERVER_H__

#include <pthread.h>
#include <unistd.h>
#include "ServerHandle.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/ELThreadpool.h"
#include "ndsl/net/SocketAddress.h"

using namespace std;
using namespace ndsl;
using namespace net;
void* service_thread(void* arg);
class FileManage;

class FtpServer{
public:
	FtpServer();
	~FtpServer();
	void start();
	ELThreadpool* threadPool;
private:
	int fd;
	SocketAddress4 servaddr;
};
#endif // __FTPSERVER_H__