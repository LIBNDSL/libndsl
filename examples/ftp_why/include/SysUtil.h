#ifndef __SYSUTIL_H__
#define __SYSUTIL_H__

class SysUtil{

  public:
	SysUtil();
	virtual ~SysUtil();

  public:
	int tcpServer(const char* host, unsigned short port);  
	int tcpClient(unsigned short port);
	void getLocalIp(const char* ip);
	static int lockFileRead(int fd);
	static int lockFileWrite(int fd);
};


#endif // __SYSUTIL_H__

