////
// @file FtpServer.cc
// @brief
// 
//
// @author lanry
// @email luckylanry@163.com
//

#include "FtpServer.h"
#include "FileManage.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/net/TcpAcceptor.h"

using namespace std;

FtpServer::FtpServer(){

}

FtpServer::~FtpServer(){
}

//运行服务器
void FtpServer::start(){
	EventLoop loop;
	int l = loop.init();
	if(l < 0)  
	{
		printf("loop fail\n");
	}
    //监听指定端口
    threadPool->setMaxThreads(NUMBER_OF_CLIENT);
	threadPool->start();

	TcpAcceptor *tAc = new TcpAcceptor(&loop);
    servaddr.setPort(6001);
    tAc->start(servaddr);
	
    //处理连接服务器请求
	while(true){
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		int data_sockfd = accept(fd, (struct sockaddr*)&client_addr, &client_addr_len);
        cout << "client connect" << endl;
		if(data_sockfd < 0){
			LOG(LOG_ERROR_LEVEL, LOG_SOURCE_TCPACCETPOR, "accept error");
			LOG(LOG_ERROR_LEVEL,LOG_SOURCE_TCPACCETPOR,"ACCEPTOR fail");
			break;
		}
        //为每个新请求创建一个线程进行处理
		pthread_t pthread_temp;
		if(pthread_create(&pthread_temp, NULL, service_thread, (void*)&data_sockfd) != 0){
			 LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_THREAD,
            "pthread_create errno = %d:%s\n",
            errno,
            strerror(errno));
		}
	}
	EventLoop::loop(&loop);
	close(fd);

}
//新线程
void* service_thread(void* arg) {
	int data_sockfd = *(int*)arg;
	ServerHandle service(data_sockfd);
	service.dispose();
	return arg;
}
