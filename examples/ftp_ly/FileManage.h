////
// @file FileManage.h
// @brief
// 
//
// @author lanry
// @email luckylanry@163.com
//
#ifndef __FILEMANAGE_H__
#define __FILEMANAGE_H__

#include <iostream>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <vector>
#include <fstream>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <dirent.h>  // 包含文件夹操作函数
#include <unistd.h>

using namespace std;

// 分别对应命令连接和数据连接
#define COMMAND_BUF_SIZE 4096
#define DATA_BUF_SIZE 4096

// 设定可连接的客户端数目上限
#define NUMBER_OF_CLIENT 40
#define FILE_NOT_EXIST 0
#define FILE_EXIST 1


extern const string CLIENT_RETR_COMMAND;
extern const string CLIENT_STOR_COMMAND;
extern const string CLIENT_PWD_COMMAND;
extern const string CLIENT_LIST_COMMAND;
extern const string CLIENT_CWD_COMMAND;
extern const string CLIENT_HELP_COMMAND;
extern const string CLIENT_QUIT_COMMAND;
extern const string SERVER_RETR_COMMAND;
extern const string SERVER_STOR_COMMAND;
extern const string SERVER_PWD_COMMAND;
extern const string SERVER_LIST_COMMAND;
extern const string SERVER_CWD_COMMAND;
extern const string SERVER_HELP_COMMAND;
extern const string SERVER_QUIT_COMMAND;

class FileManage{
public:
    FileManage()    {}
    ~FileManage() {}
public:    
    int createSocket();
    //void setAddr(string IPv4_addr, short port, struct sockaddr_in &sin);
    void setAddr(int port, struct sockaddr_in &sin);
    void setAddr(struct sockaddr_in &sin);

    // int connect2Target(int sockfd, struct sockaddr_in sin);
    // 去掉字符串首尾空格
    string trim(string s, char c);
    // 切割字符串
    void split(string s, char sp, vector<string> &ret);
 
    // 收发文件
    void send_file(int sockfd, string file_name, char* buffer, int buffer_size);
    void recv_file(int sockfd, string file_name, char* buffer, int buffer_size);

    void saveDir(string current_path, char* buffer, int buffer_size);
    bool file_exist(string current_path, string file_name);

    // 根据用户输入命令更改寻找文件的路径
    string cwd(string current_path, string cwd_path, bool &path_exist);
};


#endif // __FILEMANAGE_H__