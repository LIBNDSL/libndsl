#include <iostream>

#include "FtpServer.h"
#include "FtpConnection.h"
#include "Session.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/SocketAddress.h"

using namespace std;

FtpServer::FtpServer (){
	
}
FtpServer::~FtpServer (){}

TcpAcceptor* FtpServer::tac = NULL;
ELThreadpool* FtpServer::threadPool = NULL;
EventLoop* FtpServer::loop = NULL;
TcpAcceptor* FtpServer::dataTac = NULL;

void FtpServer::start(){
	loop = new EventLoop();
	if (loop->init() < 0) 
		cout << "loop.init() failed" << endl;

	// 初始化线程池
    threadPool = new ELThreadpool();
    threadPool->setMaxThreads(5);
    threadPool->start();
    
    tac = new TcpAcceptor(loop);	
    
	SocketAddress4 *servaddr = new SocketAddress4;
    servaddr -> setPort(5188);
    
    if(tac -> start(*servaddr) < 0) {
    	cout << "tac -> start(*servaddr) failed" << endl;
    	return;
    }
    
    // start dataAcceptor listen
	SocketAddress4 *servaddr2 = new SocketAddress4;
	servaddr2->setPort(5189);
    dataTac = new TcpAcceptor(loop);
    if(dataTac -> start(*servaddr2) < 0) {
    	cout << "dataTac -> start(*servaddr) failed" << endl;
    	return;
    } 
        
    FtpConnection *conn = new FtpConnection();
    tac->onAccept(conn, NULL, NULL, onConnection, conn, threadPool->getNextEventLoop());

	cout << "waiting for connection..." << endl;
    // 运行
    EventLoop::loop(loop);
}

void FtpServer::onConnection(void *p){
	FtpConnection *conn = static_cast<FtpConnection *>(p);
	Session *sess = new Session();
	sess->ctrlConn = conn;
	sess->loop = loop;
	sess->dataTac = dataTac;
	conn->sess = sess;
	// 向loop里面添加任务
    EventLoop::WorkItem *w1 = new EventLoop::WorkItem;
    w1->doit = Session::beginSession;
    w1->param = sess;
    conn->pTcpChannel_->pLoop_->addWork(w1);
    
    FtpConnection *con1 = new FtpConnection();
    tac->onAccept(con1, NULL, NULL, onConnection, con1, threadPool->getNextEventLoop());
}

void FtpServer::onDataConnection(void *p){
	cout << "this is onDataConnection" << endl;
	FtpConnection *conn = static_cast<FtpConnection *>(p);
	conn->sess->dataConn = conn;
}



