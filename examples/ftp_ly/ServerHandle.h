////
// @file ServerHandle.h
// @brief
// 
//
// @author lanry
// @email luckylanry@163.com
//
#ifndef __SERVERHANDLE_H__
#define __SERVERHANDLE_H__

#include "FileManage.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/ELThreadpool.h"

using namespace std;

//class FileManage;
class ServerHandle{
public:
	ServerHandle(int command_sockfd);
	~ServerHandle();
	FileManage f;
	void dispose();
private:
    // 命令链接参数
	char command_buffer[COMMAND_BUF_SIZE];
	int command_sockfd; 
    vector<string> command;

	// 数据连接参数 
	char data_buffer[DATA_BUF_SIZE];
	int data_sockfd;

	struct sockaddr_in dataSock;

	// 当前目录路径
	string current_path;
public:
    // 命令相关操作
	int sendCommand();
	int recvCommand();
	bool handleCommand();

    // 数据连接操作处理
	int sendData();
	int recvData();
    bool createDataSocket();

	void RETR_command(string file_name);
	void STOR_command(string file_name);
	void PWD_command(); // 打印工作目录
	void LIST_command();
	void CWD_command(string path); 
	void QUIT_command();
};
#endif // __SERVERHANDLE_H__










