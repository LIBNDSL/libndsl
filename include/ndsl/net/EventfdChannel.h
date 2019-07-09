/*
 * @file: eventfdChannel.h
 * @description: file content
 * @Date: 2019-01-11 17:15:45
 * @author: peng jun
 * @email: 785733871@qq.com
 */

#ifndef __EVENTFD_CHANNEL_H__
#define __EVENTFD_CHANNEL_H__

#include "ndsl/net/BaseChannel.h"
#include "ndsl/net/EventLoop.h"

using namespace ndsl;
using namespace net;

namespace ndsl{
namespace net{

class EventfdChannel : public BaseChannel{
    public:
        EventfdChannel(int eventfd,EventLoop *loop);
        ~EventfdChannel();

        
};

}//namespace net
}//namespace ndsl

#endif //__EVENTFD_CHANNEL_H__
