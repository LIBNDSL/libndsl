////
// @file FifoIpc.h
// @brief
// 封装有名管道
//
// @author ranxiangjun
// @email ranxianshen@gmail.com
//
#ifndef _NDSL_NET_FIFOIPC_H_
#define _NDSL_NET_FIFOIPC_H_
#include <sys/stat.h>	// mode访问权限

// usr reads, usr write, group menbers read only, others read only
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

namespace ndsl{
namespace net{
	
//create two pipes with name
class FifoIpc
{
  public:
	FifoIpc();
	~FifoIpc();
	
	// create and open two pipes, usually used by father proccessor
	// fd[0] only can be read, fd[1] only can be writed
	int createAndOpen(const char *pathname1, const char *pathname2, int fd[2], mode_t mode);
	// father-child proccess may use
	int createFifo(const char *pathname1, const char *pathname2, mode_t mode); 
	
	//only open. fd[0] only can be read, fd[1] only can be writed
	int openChildFifo(const char *pathname1, const char *pathname2, int fd[2]);
	int openFatherFifo(const char *pathname1, const char *pathname2, int fd[2]);
	
	// close the fd and unlink the path
	void closeAndUnlink(const char *pathname1, const char *pathname2, int fd[2]);
};

}	// namespace net
}	// namespace ndsl

#endif	// _NDSL_NET_FIFOIPC_H_
