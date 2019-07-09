/**
* @file SignalChannel.h
* @brief
* SignalChannel头文件
*
* @author why
* @email 136046355@qq.com
*/
#ifndef __SIGNAL_CHANNEL_H__
#define __SIGNAL_CHANNEL_H__

#include "BaseChannel.h"
#include "EventLoop.h"
#include "SignalHandler.h"

namespace ndsl{
namespace net{


class SignalChannel : public BaseChannel{
	public:
	  SignalChannel (int signalfd, EventLoop *loop);
	  ~SignalChannel ();
};

} // namespace net
} // namespace ndsl




#endif // __SIGNAL_CHANNEL_H__

