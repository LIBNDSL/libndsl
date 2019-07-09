#include "ndsl/net/SignalChannel.h"

namespace ndsl{
namespace net{

SignalChannel::SignalChannel(int signalfd, EventLoop *loop) : BaseChannel(signalfd, loop){}
SignalChannel::~SignalChannel(){}
//int SignalChannel::handleEvent(){

//	if (revents_ & EPOLLIN) {
//		pSignalHander_->handleRead(); 
//	}else if (revents_ & EPOLLOUT) {
//		pSignalHander_->handleWrite(); 
//	}else{
//		return S_FALSE;
//	}
//    return S_OK;
//}

//int SignalChannel::setCallBack(SignalHandler *pSignalHander){
//	pSignalHander_ = pSignalHander;
//	return S_OK; 
//}

//int SignalChannel::enableReading(){
//    events_ |= EPOLLIN;
//    regist();
//    return S_OK;
//}

//int SignalChannel::enableWriting(){
//    events_ |= EPOLLOUT;
//    update();
//    return S_OK;
//}

//int SignalChannel::disableWriting(){
//    events_ &= ~EPOLLOUT;
//    update();
//    return S_OK;
//}

//int SignalChannel::isWriting() { 
//	return events_ & EPOLLOUT; 
//}

//int SignalChannel::update(){
//	ploop_ -> update(this);
//	return S_OK;
//}

//int SignalChannel::regist(){
//	ploop_ -> regist(this);
//	return S_OK;
//}

//int SignalChannel::del(){
//	ploop_ -> del(this);
//	return S_OK;
//}

//int SignalChannel::getEvents() { 
//	return events_; 
//}

//int SignalChannel::setRevents(int revents){
//    revents_ = revents;
//    return S_OK;
//}
		
	
} // namespace net
} // namespace ndsl




