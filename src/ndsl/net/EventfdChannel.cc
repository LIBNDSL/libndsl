/*
 * @file: 
 * @description: file content
 * @Date: 2019-01-11 18:16:26
 * @author: peng jun
 * @email: 785733871@qq.com
 */

#include "ndsl/net/EventfdChannel.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/BaseChannel.h"

namespace ndsl{
namespace net{
    
    EventfdChannel::EventfdChannel(int eventfd,EventLoop *loop) : BaseChannel(eventfd,loop){}
    EventfdChannel::~EventfdChannel(){
        erase();
        //close(fd_);
    }

}//namespace net
}//namespace ndsl
