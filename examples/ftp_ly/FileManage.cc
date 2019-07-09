////
// @file FileManage.cc
// @brief
// 
//
// @author lanry
// @email luckylanry@163.com
//
#include "FileManage.h"
#include "ndsl/net/SocketAddress.h"

const string SERVER_RETR_COMMAND = "RETR";
const string SERVER_STOR_COMMAND = "STOR";
const string SERVER_PWD_COMMAND = "PWD";
const string SERVER_LIST_COMMAND = "LIST";
const string SERVER_CWD_COMMAND = "CWD";
const string SERVER_HELP_COMMAND = "HELP";
const string SERVER_QUIT_COMMAND = "QUIT";

//创建socket
int FileManage::createSocket(){

	return socket(AF_INET, SOCK_STREAM, 0);
}

//设置地址

// void FileManage::setAddr(string IPv4_addr, short port, struct sockaddr_in &sin){

// 	sin.sin_family = AF_INET;
// 	sin.sin_port = htons(port);
// 	inet_pton(AF_INET, IPv4_addr.c_str(), &sin.sin_addr);
// }

//默认填入本机地址
void FileManage::setAddr(int port, struct sockaddr_in &sin){
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = htons(INADDR_ANY);

}
//默认填入本机地址与任意可用端口号
void FileManage::setAddr(struct sockaddr_in &sin){
	sin.sin_family = AF_INET;
	sin.sin_port = htons(0);
	sin.sin_addr.s_addr = htons(INADDR_ANY);
}

// //连接至地址为sin的主机
// int FileManage::connect2Target(int sockfd, struct sockaddr_in sin){
// 	return connect(sockfd, (struct sockaddr*)&sin, sizeof(sin));
// }

//去掉首尾的c字符
string FileManage::trim(string s, char c){
	int length = s.length();
	int begin = 0, end = length - 1;
	while(begin < length && s[begin] == c) ++begin;
	while(end >= 0 && s[end] == c) --end;
	if (begin <= end)
		return s.substr(begin, end - begin + 1);
	else
		return string("");

}
//切割字符串
void FileManage::split(string s, char sp, vector<string> &ret){
	s = trim(s, sp);
	int length = s.length();
	int begin = 0, end = 0;
	ret.clear();
	while(end < length){
		if (s[end] != sp){
			++end;
		}
       else{
			ret.push_back(s.substr(begin, end - begin));
			while(s[end] == sp) ++end;
			begin = end;
		}
	}
	if(begin < end)
		ret.push_back(s.substr(begin, end - begin));
}

//发送文件
void FileManage::send_file(int sockfd, string file_name, char* buffer, int buffer_size){
    int filefd = open(file_name.c_str(), O_RDWR);
    int size ;
    while(true){
        size = read(filefd, buffer, buffer_size);
        if (size <= 0) break;
        write(sockfd, buffer, size);
    }
    close(filefd);
}
//接收文件
void FileManage::recv_file(int sockfd, string file_name, char* buffer, int buffer_size){
    int filefd = open(file_name.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0644);
    int size;
    while(true){
        size = read(sockfd, buffer, buffer_size);
        if(size <= 0) break;
        write(filefd, buffer, size);
    }
    close(filefd);
}

// 把当前路径下相关文件信息保存在buffer
void FileManage::saveDir(string current_path, char* buffer, int buffer_size){
    memset(buffer, 0, buffer_size);
    struct dirent *ptr;
    // 打开目录
	DIR *dir = opendir(current_path.c_str());
    while ((ptr = readdir(dir)) != NULL) {
        if (string(ptr->d_name) == "." || string(ptr->d_name) == "..") continue;
        strcat(buffer, ptr->d_name);
        strcat(buffer, "\n");
	}
}

// 根据用户发来的命令进行路径改变，若路劲不存在，则返回原路径
string FileManage::cwd(string current_path, string cwd_path, bool &path_exist){
    vector<string> path_split;
    split(cwd_path, '/', path_split);
    string temp_path = current_path;
    bool exist_flag;
    struct dirent *ptr;
    DIR *dir;
    for(vector<string>::iterator it = path_split.begin(); it != path_split.end(); ++it){

        //后退到上一层目录
        if((*it) == ".."){
            vector<string> temp_path_split;
            split(temp_path, '/', temp_path_split);
            if(temp_path_split.size() == 1){//已是根目录

                path_exist = false;
                return current_path;

            }
            temp_path = temp_path_split[0];
            int m =temp_path_split.size();
            for (int i = 1; i < m-1; ++i)
                temp_path = temp_path + "/" + temp_path_split[i];
            continue;
        }
        //进入下一层目录
        dir = opendir(temp_path.c_str());
        exist_flag = false;
        while((ptr = readdir(dir)) != NULL){
            if (string(ptr->d_name) == "." || string(ptr->d_name) == "..") continue;

            if ((*it) == string(ptr->d_name)){
                temp_path = temp_path + "/" + ptr->d_name;
                exist_flag = true;
                break;
            }
        }
        if(!exist_flag){

            path_exist = false;
            return current_path;
        }

    }
    path_exist = true;
    return temp_path;
}
//判断在当前路径current_path，是否存在文件file_name

bool FileManage::file_exist(string current_path, string file_name){

    struct dirent *ptr;
    DIR *dir = opendir(current_path.c_str());
    while((ptr = readdir(dir)) != NULL){
        if (string(ptr->d_name) == "." || string(ptr->d_name) == "..") continue;
        if (file_name == string(ptr->d_name)){
            return true;
        }
    }
    return false;
}