////
// @file PipeAndFifo.h
// @brief
// father-child proccesses use pipe
// proccess without relationship use fifo  
//
// @auther ranxiangjun
// @email ranxianshen@gmail.compare
//
#ifndef _NDSL_NET_PIPEANDFIFO_H_
#define _NDSL_NET_PIPEANDFIFO_H_
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#include <sys/stat.h>
#include <queue>
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/BaseChannel.h"

namespace ndsl{
namespace net{
	
using Callback = void (*)(void *);	// callbank函数指针原型
using ErrorHandle = void (*)(int, int);	// error回调函数

class PipeChannel:public BaseChannel
{
  public:
    PipeChannel(int pipefd, EventLoop *loop);
    ~PipeChannel();
};

class PipeAndFifo{
  private:
    typedef struct SInfo
	{
		void *sendBuf_;	// users write the info of the address to fd
		void *readBuf_; // users read info from fd to this address
		size_t len_;	// buf length
		int flags_;		// the paramater of send()
		Callback cb_;	// save the users' callback function 
		void *param_;	// paramater of callback
		size_t offset_;	// the mounts of send successfully
	}Info, *pInfo;
	std::queue<pInfo> qSendInfo_;	// the queue need to send
	Info RecvInfo_;
	ErrorHandle errorHandle_;	  // the function to address error
	
  public:
    PipeAndFifo(EventLoop *pLoop);
	~PipeAndFifo();
	
    PipeChannel *pPipeChannel_;
	EventLoop *pLoop_;
	
	// 生成管道的fd，并发送给channel,p为用户回调函数参数
	int createPipe(int fd[2]);
	
	//移除注册
	int remove;
	
	//事件发生后的处理函数
	static int handleRead(void *pthis);
	static int handleWrite(void *pthis);
	
	// registe the callback fuction of error
	int onError(ErrorHandle cb);
	
	// receive messege asynchronously
	int onRecv(char *buf, size_t len, int flags, Callback cb, void *param);
	
	// send messege asynchronously
	int onSend(void *buf, size_t len, int flags, Callback cb, void *param);
	
	/* 
	* create two pipes with name
	* father-child proccesses use pipe
	* proccess without relationship use fifo  
	*/
	
	// create and open two pipes, usually used by father proccessor
	// fd[0] only can be read, fd[1] only can be writed
	int createAndOpenFifo(const char *pathname1, const char *pathname2, int fd[2], mode_t mode);
	
	//only open. fd[0] only can be read, fd[1] only can be writed
	int openAndMatchFifo(const char *pathname1, const char *pathname2, int fd[2]);
	
	// close the fd and unlink the path
	void closeAndUnlink(const char *pathname1, const char *pathname2, int fd[2]);
};

}	// namespace net
}	// namespace ndsl

#endif	// _NDSL_NET_PIPEANDFIFO_H_
