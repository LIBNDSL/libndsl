/**
* @file SignalHandler.h
* @brief
* SignalHandler头文件
*
* @author why
* @email 136046355@qq.com
*/
#ifndef __SIGNALHANDLER_H__
#define __SIGNALHANDLER_H__

#include "SignalChannel.h"
#include "EventLoop.h"

class SignalChannel;

namespace ndsl{
namespace net{

using Callback = void (*)(void *); 

class SignalHandler {
	public:
	  SignalHandler (EventLoop *pLoop);
	  ~SignalHandler ();

    public:
	  SignalChannel *pSignalChannel_;
	  EventLoop *pLoop_;
	  unsigned int signums_[64]; // 信号编号
	  Callback handleFunc_; // 用于信号发生后的用户处理函数
	  void *p_; // 信号发生后用户回调函数的参数
	  static int blockSignals_[64]; // 阻塞过程中注册的信号
	   
	public:
	  // signums：需要注册的信号数组; num：信号个数; handleFunc：用户的回调函数; p：回调函数参数
	  // 将信号生成文件描述符fd，并发送给channel, p为用户回调函数参数
	  int registSignalfd(unsigned int signums[], int num, Callback handleFunc, void *p);
	  // 移除注册
	  int remove(); 
	  
	  // 阻塞除SIGINT，SIGKILL之外的信号
	  static int blockAllSignals();
	  // 解除阻塞，并将阻塞时发生过的信号存入blockSignals_
	  static int unBlockAllSignals();
	  
	  
	protected:
	  // 事件发生后的处理
	  static int handleRead(void *pthis); 
      static int handleWrite(void *pthis);
      
      // 忽略一次解除阻塞之前注册过的信号
      static void handleSignalEnd(int p);
	  
};
	
} // namespace net
} // namespace ndsl


#endif // __SIGNALHANDLER_H__

