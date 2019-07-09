////
// @file PipeAndFifo.cc
// @brief
//
// @auther ranxiangjun
// @email ranxianshen@gmail.compare
//
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "ndsl/config.h"
#include "ndsl/utils/Error.h"
#include "ndsl/utils/Log.h"
#include "ndsl/net/PipeAndFifo.h"
using namespace std;

namespace ndsl{
namespace net{

PipeChannel::PipeChannel(int pipefd, EventLoop *loop)
    :BaseChannel(pipefd, loop){}

PipeChannel::~PipeChannel()
{
    erase();
}

PipeAndFifo::PipeAndFifo(EventLoop *pLoop)
{
	pLoop_ = pLoop;
}

PipeAndFifo::~PipeAndFifo(){}

int PipeAndFifo::createPipe(int fd[2])
{	
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0)
	{	
		// cout << "create pipe failed: "<< strerror(errno)<<endl;
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "create pipe failed\n");
		return S_FALSE;	
	}
	int flags = fcntl(fd[0], F_GETFL, 0); //without it,other states may be cleaned
	if (flags < 0)
	{
		// cout<<"fget failed: "<<strerror(errno)<<endl;
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "fcntl get state error\n");
		return S_FALSE;
	}
	flags |= O_NONBLOCK;
	if( fcntl(fd[0], F_SETFL, flags) < 0)
	{
		// cout<<"fset failed: "<<strerror(errno)<<endl;
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "fcntl set state error\n");
		return S_FALSE;	
	}
	
	pPipeChannel_ = new PipeChannel(fd[0], pLoop_);
	pPipeChannel_ -> setCallBack(handleRead, handleWrite, this);
	pPipeChannel_ -> enroll(true);
	return S_OK;
	
}
 
 int PipeAndFifo::onSend(void *buf, size_t len, int flags, Callback cb, void *param)
 {
	int pipefd = pPipeChannel_->getFd();
	size_t n = send(pipefd, buf, len, flags | MSG_NOSIGNAL);
	if(n == len)
	{
		if (cb != NULL) cb(param);
		return S_OK;
	}else if(n < 0)
	{
		// cout << "send error : "<< strerror(errno)<<endl;
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "send error\n");
		errorHandle_(errno, pPipeChannel_->getFd());
		return S_FALSE;
	}
	pInfo tsi = new Info;
	tsi->offset_ = n;
	tsi->sendBuf_ = (void *)buf;
	tsi->readBuf_ = NULL;
	tsi->len_ = len;
	tsi->flags_ = flags | MSG_NOSIGNAL;
	tsi->cb_ = cb;
	tsi->param_ = param;
	
	qSendInfo_.push(tsi);
	return S_OK;
	
 }
 
 int PipeAndFifo::handleWrite(void *pthis)
{
	PipeAndFifo *pThis = static_cast<PipeAndFifo *>(pthis);
	int pipefd = pThis->pPipeChannel_->getFd();
	if (pipefd < 0) return S_FALSE;
	size_t n;
	
	if (pThis->qSendInfo_.size() > 0)
	{
		pInfo tsi = pThis->qSendInfo_.front();
		if ((n = send(pipefd, (char *)tsi->sendBuf_ + tsi->offset_
			, tsi->len_ - tsi->offset_, tsi->flags_) > 0))
		{
			tsi->offset_ += n;
			if (tsi->len_ == tsi->offset_)
			{
				if (tsi->cb_ != NULL) tsi->cb_(tsi->param_);
				pThis->qSendInfo_.pop();
				delete tsi;
				LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "PipeAndFifo::send error\n");
			}else
			{
				// cout<<"only send part of info"<<endl;
				LOG(LOG_INFO_LEVEL, LOG_SOURCE_UNIXCONNECTION, "send part info\n");
				return S_OK;
			}
			
		}else if (n < 0)
		{
			LOG(LOG_INFO_LEVEL, LOG_SOURCE_UNIXCONNECTION, "PipeAndFifo::send error\n");
			pThis->errorHandle_(errno, pThis->pPipeChannel_->getFd());
			pThis->qSendInfo_.pop();
			delete tsi;
			
			return S_FALSE;
		}
	}
	return S_OK;
}

int PipeAndFifo::onRecv(char *buf, size_t len, int flags, Callback cb, void *param)
{
	int n;
	int pipefd = pPipeChannel_->getFd();
	if ((n = recv(pipefd, buf, len, flags | MSG_NOSIGNAL)) < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			// cout <<"receive meet EGAIN"<<endl;
			LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "receive meet EAGAIN\n");
			
			RecvInfo_.readBuf_ = buf;
			RecvInfo_.sendBuf_ = NULL;
			RecvInfo_.flags_ = flags | MSG_NOSIGNAL;
			RecvInfo_.len_ = len;
			RecvInfo_.cb_ = cb;
			RecvInfo_.param_ = param;
			
			return S_OK;
		}else
		{
			// cout << "receive other error"<<endl;
			LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "receive other error\n");
			errorHandle_(errno, pPipeChannel_->getFd());
			return S_FALSE;
		}
	}
	if (cb != NULL) cb(param);
	return S_OK;
}

int PipeAndFifo:: handleRead(void *pthis)
{
	PipeAndFifo *pThis = static_cast<PipeAndFifo *>(pthis);
	int pipefd = pThis->pPipeChannel_->getFd();
	if (pipefd < 0) return S_FALSE;
	
	int n;
	if ((n= recv(pipefd, pThis->RecvInfo_.readBuf_, pThis->RecvInfo_.len_
		,pThis->RecvInfo_.flags_)) < 0)
	{
		pThis->errorHandle_(errno, pThis->pPipeChannel_->getFd());
		return S_FALSE;
	}
	if (pThis->RecvInfo_.cb_ != NULL)
		pThis->RecvInfo_.cb_(pThis->RecvInfo_.param_);
	return S_OK;
}

int PipeAndFifo::onError(ErrorHandle cb)
{
	errorHandle_ = cb;
	return S_OK;
}


/* 
* fifo
*/
int PipeAndFifo::createAndOpenFifo(const char *pathname1, const char *pathname2, int fd[2], mode_t mode)
{
	int readfd, writefd;
	
	if ((mkfifo(pathname1, mode) < 0) && (errno != EEXIST))
	{	
		// cout<< "can not create:"<< pathname1<<endl;
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "can not create\n");
		return S_OK;
	}
	if ((mkfifo(pathname2, mode) < 0) && (errno != EEXIST))
	{	
		// cout<< "can not create:"<< pathname2<<endl;
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "can not create\n");
		unlink(pathname1);
		return S_OK;
	}
	if ((readfd = open(pathname1, O_RDONLY, 0)) < 0)
	{
		LOG(LOG_INFO_LEVEL, LOG_SOURCE_UNIXCONNECTION, "PipeAndFifo::open pathname1 error: %s\n", strerror(errno));
		return S_OK;
	}
	fd[0] = readfd;
	int flags = fcntl(fd[0], F_GETFL, 0); //without it,other states may be cleaned
	if (flags < 0)
	{
		// cout<<"fget failed: "<<strerror(errno)<<endl;
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "fcntl get error\n");
		return S_FALSE;
	}
	flags |= O_NONBLOCK;
	if( fcntl(fd[0], F_SETFL, flags) < 0)
	{
		// cout<<"fset failed: "<<strerror(errno)<<endl;
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "fcntl set error\n");
		return S_FALSE;	
	}
	
	// fIFO essentially is pipe, no need to create a FifoChannel
	pPipeChannel_ = new PipeChannel(fd[0], pLoop_);		
	pPipeChannel_ -> setCallBack(handleRead, NULL, this);	// only read
	pPipeChannel_ -> enroll(true);
	
	if ((writefd = open(pathname2, O_WRONLY, 0)) < 0)
	{
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "open pathname2 error: %s\n", strerror(errno));
		return S_OK;
	}
	fd[1] = writefd;
	flags = fcntl(fd[1], F_GETFL, 0); //without it,other states may be cleaned
	if (flags < 0)
	{
		// cout<<"fget failed: "<<strerror(errno)<<endl;
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "fcntl get error\n");
		return S_FALSE;
	}
	flags |= O_NONBLOCK;
	if( fcntl(fd[1], F_SETFL, flags) < 0)
	{
		// cout<<"fset failed: "<<strerror(errno)<<endl;
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "fcntl set error\n");
		return S_FALSE;	
	}
	
	// fIFO essentially is pipe, no need to create a FifoChannel
	pPipeChannel_ = new PipeChannel(fd[1], pLoop_);
	pPipeChannel_ -> setCallBack(NULL, handleWrite, this);	// only write
	pPipeChannel_ -> enroll(true);
	
	return 1;
}

int PipeAndFifo::openAndMatchFifo(const char *pathname1, const char *pathname2, int fd[2])
{
	int readfd, writefd;
	if ((writefd = open(pathname1, O_RDONLY, 0)) < 0)
	{
		// cout<< "open :"<<pathname1<< "error   "<<strerror(errno)<<endl;
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "open path1 error\n");
		return S_OK;
	}
	fd[1] = writefd;
	if ((readfd = open(pathname2, O_WRONLY, 0)) < 0)
	{
		// cout<< "open :"<<pathname2<< "error   "<<strerror(errno)<<endl;
		LOG(LOG_ERROR_LEVEL, LOG_SOURCE_UNIXCONNECTION, "open path2 error\n");
		return S_OK;
	}
	fd[0] = readfd;
	
	return 1;
	
}

void PipeAndFifo::closeAndUnlink(const char *pathname1, const char *pathname2, int fd[2])
{
	close(fd[0]);
	close(fd[1]);
	
	unlink(pathname1);
	unlink(pathname2);
}
}	// namespace net
}	// namespace ndsl
