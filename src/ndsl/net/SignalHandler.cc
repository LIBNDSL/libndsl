#include <string.h>
#include <sys/signalfd.h>
#include <signal.h>

#include "ndsl/net/SignalHandler.h"
#include "ndsl/config.h"
#include "ndsl/utils/Log.h"

#include <iostream>

namespace ndsl {
namespace net {

SignalHandler::SignalHandler(EventLoop *pLoop) {
	pLoop_ = pLoop; 
	memset(blockSignals_, 0, sizeof(blockSignals_));
}

SignalHandler::~SignalHandler() {}

int SignalHandler::blockSignals_[64] = {0};

int SignalHandler::registSignalfd(unsigned int signums[], int num, Callback handleFunc, void *p){
	
	handleFunc_ = handleFunc;
	p_ = p;
	
	sigset_t mask;  
    sigemptyset(&mask);  
    for(int i = 0; i < num; i++){
		signums_[i] = signums[i];
		sigaddset(&mask, signums[i]);
	}
    
    // 阻塞信号，不以默认的方式处理该信号
    sigprocmask(SIG_BLOCK, &mask, NULL);
    
    int sfd; 

    sfd = signalfd(-1, &mask, 0);

    pSignalChannel_ = new SignalChannel(sfd, pLoop_);
    pSignalChannel_->setCallBack(handleRead, handleWrite, this);
    pSignalChannel_->enrollIn(true);
    return S_OK;
}

int SignalHandler::handleRead(void *pthis)
{
    struct signalfd_siginfo fdsi;
    memset(&fdsi, 0, sizeof(struct signalfd_siginfo));

    SignalHandler *pSignalHandler = (SignalHandler *) (pthis);
    int sfd = pSignalHandler->pSignalChannel_->getFd();
    int s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));

    if (s != sizeof(struct signalfd_siginfo)) {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_SIGNALFDCHANNEL, "SignalHandler::handleRead: s != sizeof(struct signalfd_siginfo)!");
        return S_FALSE;
    }

    pSignalHandler->handleFunc_(pSignalHandler->p_);
    return S_OK;
}

int SignalHandler::blockAllSignals(){
	sigset_t set;
	sigfillset(&set);
	sigdelset(&set, SIGINT);
	sigdelset(&set, SIGTERM);
	sigprocmask(SIG_BLOCK, &set, NULL);
	return S_OK;
}
	  
int SignalHandler::unBlockAllSignals(){

	sigset_t set;	
	sigpending(&set);
	for(int i = 1; i <= 64; i++){
		if(sigismember(&set, i)){
			blockSignals_[i-1] = i;
			signal(i, handleSignalEnd);
		}
	}
	
	sigemptyset(&set);
	sigprocmask(SIG_SETMASK, &set, NULL);
	
	return S_OK;
}

void SignalHandler::handleSignalEnd(int p){
	signal(p, SIG_DFL);
}

int SignalHandler::handleWrite(void *pthis){
	return 0;
}

int SignalHandler::remove(){
	pSignalChannel_ -> erase();
	return S_OK;
}

} // namespace net
} // namespace ndsl
