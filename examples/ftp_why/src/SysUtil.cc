#include "SysUtil.h"
#include "Common.h"

#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <string.h>

using namespace std;

SysUtil::SysUtil(){}
SysUtil::~SysUtil(){}

int SysUtil::tcpClient(unsigned short port){
	int sock;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		ERR_EXIT("tcpClient");
	}
	if(port > 0){
		int on = 1;
		if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) < 0)
			ERR_EXIT("setsockopt");
		
		char ip[16] = {0};
		getLocalIp(ip);
		struct sockaddr_in loaclAddr;
		memset(&loaclAddr, 0, sizeof(loaclAddr));
		loaclAddr.sin_family = AF_INET;
		loaclAddr.sin_port = htons(port);
		loaclAddr.sin_addr.s_addr = inet_addr(ip);
		if(bind(sock, (struct sockaddr*)&loaclAddr, sizeof(loaclAddr)) < 0)
			ERR_EXIT("bind");
	}
	return sock;
}

int SysUtil::tcpServer(const char* host, unsigned short port){
	int listenfd;
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		ERR_EXIT("tcpServer");
		
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	if(host != NULL){
		inet_aton(host, &servaddr.sin_addr);
	}else{
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	
	servaddr.sin_port = htons(port);
	
	int on = 1;
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) < 0)
		ERR_EXIT("setsockopt");
		
	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("bind");
		
	if(listen(listenfd, SOMAXCONN) < 0)
		ERR_EXIT("listen");
		
	return listenfd;
}

void SysUtil::getLocalIp(const char* ip){
//	char name[256];
//	gethostname(name, sizeof(name));
//	cout << "hostname = " << name << endl;
//	
//	struct hostent* host = gethostbyname(name);
//	char ipStr[32];
//	inet_ntop(host->h_addrtype, host->h_addr_list[0], ipStr, sizeof(ipStr));
//	strcpy(ip, ipStr);
//	
//	cout << "ip = " << ip << endl;
	ip = "192.168.153.128";
}

int SysUtil::lockFileRead(int fd){
	int ret;
	struct flock theLock;
	memset(&theLock, 0, sizeof(theLock));
	theLock.l_type = F_RDLCK;
	theLock.l_whence = SEEK_SET;
	theLock.l_start = 0;
	theLock.l_len = 0;
	do{
		ret = fcntl(fd, F_SETLKW, &theLock);
	} while (ret < 0 && errno == EINTR);
	return ret;
}
int SysUtil::lockFileWrite(int fd){
	int ret;
	struct flock theLock;
	memset(&theLock, 0, sizeof(theLock));
	theLock.l_type = F_WRLCK;
	theLock.l_whence = SEEK_SET;
	theLock.l_start = 0;
	theLock.l_len = 0;
	do{
		ret = fcntl(fd, F_SETLKW, &theLock);
	} while (ret < 0 && errno == EINTR);
	return ret;
}





