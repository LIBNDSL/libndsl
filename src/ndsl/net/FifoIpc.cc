////
// @file FifoIpc.cc
// @brief
// FifoIpc的实现
//
// @author ranxiangjun
// @email ranxianshen@gmail.com
//
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>	// open needs
// #include "FifoIpc.h"
#include "../../../include/ndsl/net/FifoIpc.h"
using namespace std;
  
namespace ndsl{
namespace net{

FifoIpc::FifoIpc(){}
FifoIpc::~FifoIpc(){}

int FifoIpc::createAndOpen(const char *pathname1, const char *pathname2, int fd[2], mode_t mode)
{
	int readfd, writefd;
	
	if (((mkfifo(pathname1, mode)) < 0) && (errno != EEXIST))
	{	
		cout<< "can not create:"<< pathname1<<endl;
		return 0;
	}
	if (((mkfifo(pathname2, mode) < 0)) && (errno != EEXIST))
	{	
		cout<< "can not create:"<< pathname2<<endl;
		unlink(pathname1);
		return 0;
	}
	cout<<"create ok"<<endl;
	if ((readfd = open(pathname1, O_RDONLY, 0)) < 0)
	{
		cout<< "open :"<<pathname1<< "error   "<<strerror(errno)<<endl;
		return 0;
	}
	fd[0] = readfd;
	if ((writefd = open(pathname2, O_WRONLY, 0)) < 0)
	{
		cout<< "open :"<<pathname2<< "error   "<<strerror(errno)<<endl;
		return 0;
	}
	fd[1] = writefd;
	return 1;
}

int FifoIpc::createFifo(const char *pathname1, const char *pathname2, mode_t mode) 
{
	if (((mkfifo(pathname1, mode)) < 0) && (errno != EEXIST))
	{	
		cout<< "can not create:"<< pathname1<<endl;
		return 0;
	}
	if (((mkfifo(pathname2, mode) < 0)) && (errno != EEXIST))
	{	
		cout<< "can not create:"<< pathname2<<endl;
		unlink(pathname1);
		return 0;
	}
	return 1;	
}
int FifoIpc::openChildFifo(const char *pathname1, const char *pathname2, int fd[2])
{
	int readfd, writefd;
	if ((writefd = open(pathname1, O_WRONLY, 0)) < 0)
	{
		cout<< "open :"<<pathname1<< "error   "<<strerror(errno)<<endl;
		return 0;
	}
	fd[1] = writefd;
	if ((readfd = open(pathname2, O_RDONLY, 0)) < 0)
	{
		cout<< "open :"<<pathname2<< "error   "<<strerror(errno)<<endl;
		return 0;
	}
	fd[0] = readfd;
	return 1;
	
}

int FifoIpc::openFatherFifo(const char *pathname1, const char *pathname2, int fd[2])
{
	int readfd, writefd;
	if ((readfd = open(pathname1, O_RDONLY, 0)) < 0)
	{
		cout<< "open :"<<pathname1<< "error   "<<strerror(errno)<<endl;
		return 0;
	}
	fd[0] = readfd;
	if ((writefd = open(pathname2, O_WRONLY, 0)) < 0)
	{
		cout<< "open :"<<pathname2<< "error   "<<strerror(errno)<<endl;
		return 0;
	}
	fd[1] = writefd;
	return 1;
	
}
void FifoIpc::closeAndUnlink(const char *pathname1, const char *pathname2, int fd[2])
{
	close(fd[0]);
	close(fd[1]);
	
	unlink(pathname1);
	unlink(pathname2);
}

}	// namespace net
}	// namespace ndsl
