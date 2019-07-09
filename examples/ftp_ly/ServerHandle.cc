////
// @file ServerHandle.cc
// @brief
// 
//
// @author lanry
// @email luckylanry@163.com
//

#include "ServerHandle.h"
#include "ndsl/utils/Log.h"

ServerHandle::ServerHandle(int command_sockfd){
	this->command_sockfd = command_sockfd;
	current_path = ".";
}

ServerHandle::~ServerHandle(){
}

//收客户端传来的命令
inline int ServerHandle::sendCommand(){
	return write(command_sockfd, command_buffer, COMMAND_BUF_SIZE);
}

inline int ServerHandle::recvCommand(){
	return read(command_sockfd, command_buffer, COMMAND_BUF_SIZE);
}

// 处理客户端发来的参数 
void ServerHandle::dispose(){
    bool flag = true;
	//处理客户端发来的命令
	while(flag){
		if(recvCommand() < 0){
			LOG(LOG_ERROR_LEVEL,LOG_SOURCE_TCPCONNECTION,"recv error");
            close(command_sockfd);
			break;
		}
		cout << "recv command: " << command_buffer << endl;
		flag = handleCommand();
	}
}

//建立数据连接
bool ServerHandle::createDataSocket(){
	int listen_data_sockfd = f.createSocket();
	if (listen_data_sockfd < 0){
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPACCETPOR, "socket fail");
		return false;
	}
	f.setAddr(dataSock);//设置本机地址与任意可用端口号
	if(bind(listen_data_sockfd, (struct sockaddr*)&dataSock, sizeof(dataSock)) != 0){
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPACCETPOR, "bind error");
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPACCETPOR, strerror(errno));
		return false;
	}

	//获得系统给的随机可用端口号
	socklen_t data_sin_len = sizeof(dataSock);
	if(getsockname(listen_data_sockfd, (sockaddr*)&dataSock, &data_sin_len)){
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPACCETPOR, "getsockname error");
		return false;
	}

	if(listen(listen_data_sockfd, 1) != 0){
		LOG(LOG_ERROR_LEVEL,LOG_SOURCE_TCPACCETPOR,"LISTEN ERROR");
		return false;
	}

	//发送端口号

    *((short*)command_buffer) = dataSock.sin_port;
	sendCommand();

    //cout  << "server port: " << ntohs(data_sin.sin_port) << ", ";
	//数据连接建立

	data_sockfd = accept(listen_data_sockfd, (struct sockaddr*)&dataSock, &data_sin_len);
	if(data_sockfd < 0){
		LOG(LOG_ERROR_LEVEL,LOG_SOURCE_TCPACCETPOR,"accept error");
		return false;
	}
    //cout << "build data connection successfully!" << endl;
    return true;
}


//解析命令并处理命令
bool ServerHandle::handleCommand(){
	f.split(command_buffer, ' ', command);
	if (command.empty())
		return true;
	if(command[0] == SERVER_RETR_COMMAND){
		RETR_command(command[1]);
	}
	else if(command[0] == SERVER_STOR_COMMAND){
		STOR_command(command[1]);
	}
	else if(command[0] == SERVER_PWD_COMMAND){
		PWD_command();
	}
	else if(command[0] == SERVER_LIST_COMMAND){
		LIST_command();
	}
	else if(command[0] == SERVER_CWD_COMMAND){
		CWD_command(command[1]);
	}
	else if(command[0] == SERVER_QUIT_COMMAND){
		QUIT_command();
        return false;
	}
	else{
		cout << "unknown command" << endl;
	}
    return true;
}

//处理RETR命令
void ServerHandle::RETR_command(string file_name){
    if(f.file_exist(current_path, file_name)){
        command_buffer[0] = FILE_EXIST;

        sendCommand();
        if(!createDataSocket()) return;
        f.send_file(data_sockfd, current_path + "/" + file_name, data_buffer, DATA_BUF_SIZE);
        close(data_sockfd);
    }
    else{
        command_buffer[0] = FILE_NOT_EXIST;
        sendCommand();
    }
}
//处理STOR命令，将文件存储在路径上
void ServerHandle::STOR_command(string file_name){
    if(!createDataSocket()) return;
	f.recv_file(data_sockfd, current_path + "/" + file_name, data_buffer, DATA_BUF_SIZE);
    close(data_sockfd);
}

//处理PWD命令
void ServerHandle::PWD_command(){
	strcpy(command_buffer, current_path.c_str());
    sendCommand();
}

//处理LIST命令
void ServerHandle::LIST_command(){
    f.saveDir(current_path, command_buffer, COMMAND_BUF_SIZE);
    sendCommand();
}

//处理CWD命令
void ServerHandle::CWD_command(string path){ 
    bool path_exist;
    current_path = f.cwd(current_path, path, path_exist);
	if(!path_exist){
        strcpy(command_buffer, "file doesn't exist!\n");
    }
    else{
        strcpy(command_buffer, "");
    }
    sendCommand();
}
//处理QUIT命令
void ServerHandle::QUIT_command(){
	close(command_sockfd);
    cout << "client disconnect" << endl;
}

//收发数据
inline int ServerHandle::sendData(){
	return write(data_sockfd, data_buffer, DATA_BUF_SIZE);
}
inline int ServerHandle::recvData(){
	return read(data_sockfd, data_buffer, DATA_BUF_SIZE);
}

