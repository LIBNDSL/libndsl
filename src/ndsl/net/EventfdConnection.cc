/*
 * @file: 
 * @description: file content
 * @Date: 2019-01-12 22:22:04
 * @author: peng jun
 * @email: 785733871@qq.com
 */

#include <sys/eventfd.h>
#include <errno.h>

#include "ndsl/net/EventfdConnection.h"
#include "ndsl/net/EventfdChannel.h"
#include "ndsl/net/EventLoop.h"

namespace ndsl{
namespace net{
    EventfdConnection::EventfdConnection(){}
    EventfdConnection::~EventfdConnection(){}

    int EventfdConnection::createEventfd(int &ev_fd){
        ev_fd = eventfd(0,EFD_NONBLOCK);//非阻塞
        if(ev_fd != -1)
            return S_OK;
        else
            return S_FALSE;
    }

    int EventfdConnection::createChannel(int ev_fd,EventLoop *pLoop){
        //创建一个channel，设置channel的loop/fd
        pEventfdChannel_ = new EventfdChannel(ev_fd,pLoop);
        //设置channel的回调函数
        if(pEventfdChannel_ -> setCallBack(handleRead,handleWrite,this) == S_OK){
            //将channel注册到loop中
            if(pEventfdChannel_ -> enroll(true) == S_OK){
                return S_OK;
            }else {
                return S_FALSE;
            }
        }
        return S_FALSE;
    }

    int EventfdConnection::onWrite(uint64_t &count,int flags,Callback cb,void *param){
        int eventfd = pEventfdChannel_ -> getFd();
        int ret = write(eventfd,&count,sizeof(count));
        if(ret == sizeof(count)){
            return S_OK;                       
        }else if(ret < 0){
            // printf("error occur...\n");
            errorHandle_(errno,pEventfdChannel_ -> getFd());
            return S_FALSE;
        }else {
            pInfo tsi = new Info;
            // tsi->offset_ = 0;
            tsi->sendBuf_ = count;
            // tsi->readBuf_ = NULL;
            // tsi->len_ = sizeof(count);
            // tsi->flags_ = flags | MSG_NOSIGNAL;
            tsi->cb_ = cb;
            tsi->param_ = param;
            qSendInfo_.push(tsi);
            return S_OK;
        }
        return S_FALSE;
    }

    int EventfdConnection::handleWrite(void *pthis){
        EventfdConnection *pThis = static_cast<EventfdConnection *>(pthis);

        int ev_fd = pThis->pEventfdChannel_->getFd();

        if(ev_fd < 0)return S_FALSE;

        if(pThis->qSendInfo_.size() > 0){
            pInfo tsi = pThis -> qSendInfo_.front();
            int len = sizeof(tsi->sendBuf_);
            int ret = write(ev_fd,&(tsi->sendBuf_),len);
            if(ret == len){
                pThis-> qSendInfo_.pop();
                delete tsi;
                return S_OK;
            }else if(ret < 0){
                // printf("error occur...\n");
                pThis->errorHandle_(errno,pThis->pEventfdChannel_ -> getFd());
                return S_FALSE;
            }else if(ret == 0){
                return S_OK;
            }
        }
        return S_OK;
    }

    int EventfdConnection::onRead(uint64_t &count,int flags,Callback cb,void *param){
// printf("onRead start.\n");
        int eventfd = pEventfdChannel_ -> getFd();
        int ret = read(eventfd,&count,sizeof(count));
// printf("read ends.\n");
        if(ret == sizeof(count)){
// printf("read success.\n");
            return S_OK;
        }else {
// printf("read error!\n");
            if(errno == EAGAIN || errno == EWOULDBLOCK){
// printf("error:EAGAIN or EWOULDBLOCK.errno:%d\n",errno);
                //加入回调，等待再读
                RecvInfo_.sendBuf_ = 0;
                RecvInfo_.cb_ = cb;
                RecvInfo_.param_ = param;
                return S_FALSE;
            }
// printf("other errno.then end.\n");
            return S_FALSE;
        }  
    }

    int EventfdConnection::handleRead(void *pthis){
        EventfdConnection *pThis = static_cast<EventfdConnection *>(pthis);
// printf("handleRead start.\n");
        int ev_fd = pThis->pEventfdChannel_->getFd();

        if(ev_fd < 0)return S_FALSE;


        // pInfo tsi = pThis -> RecvInfo_;
        int len = sizeof(pThis-> RecvInfo_.readBuf_);
        int ret = read(ev_fd,&(pThis-> RecvInfo_.readBuf_),len);
        if(ret == len){
            
            // delete tsi;
            return S_OK;
        }else if(ret < 0){
            // printf("error occur...\n");
            pThis->errorHandle_(errno,pThis->pEventfdChannel_ -> getFd());
            return S_FALSE;
        }else if(ret == 0){
            return S_OK;
        }

        return S_OK;
    }
} // namespace net
} // namespace ndsl