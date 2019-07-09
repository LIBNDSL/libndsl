#include <string.h>
#include <iostream>

#include "FtpConnection.h"
#include "FtpReply.h"
#include "Session.h"

using namespace std;

FtpConnection::FtpConnection (){
//	sess = new Session();
}
FtpConnection::~FtpConnection (){}

void FtpConnection::storDataCallback(void* p){
	Session* sess = static_cast<Session*>(p);

	cout << "---storDataCallback errno =  " << errno << "len = " << sess->len << endl;


	
	write(sess->localFd, sess->recvBuf, strlen(sess->recvBuf));
	sess->dataConn->onRecv(sess->recvBuf, &(sess->len), 0, FtpConnection::storDataCallback, sess);	
	
	if(errno == 11 && sess->len == 0){
		write(sess->localFd, sess->recvBuf, strlen(sess->recvBuf));
		FtpReply::reply(sess->ctrlConn, 226, "Directory send OK");
		sess->dataConn->pTcpChannel_->erase();
		close(sess->dataConn->pTcpChannel_->getFd());
		sess->dataConn = NULL;
		close(sess->localFd);
		free(sess->recvBuf);
		sess->recvBuf = NULL;
		sess->len = -1;
	}
}

void FtpConnection::errorHandle(void* p, int err){
	Session* sess = static_cast<Session*>(p);
	if(err == 11){
		return;	
	}else{
		cout << "this is errorHandle." << endl;
		FtpReply::reply(sess->ctrlConn, 451, "Failed reading from network stream");
	}
}

void FtpConnection::dataConnection(Session* sess){
	FtpConnection *con = new FtpConnection();
	con->sess = sess;
    sess->dataTac->onAccept(con, NULL, NULL, FtpServer::onDataConnection, con);
}

