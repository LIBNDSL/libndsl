/**
 * @file Multiplexer.h
 * @brief
 *
 * @author zzt
 * @emial 429834658@qq.com
 **/
#ifndef __NDSL_UTILS_V8WORK_H__
#define __NDSL_UTILS_V8WORK_H__
#include <queue>
#include "ndsl/utils/WorkQueue.h"
#include "ndsl/utils/Error.h"

namespace ndsl {
namespace net {

struct JsQueue
{
    struct JsItem // 放入queue的结构体
    {
        int length;
        char *address;
    };

    std::queue<struct JsItem *> jsQueue_; // js队列,元素为结构体jsItem指针
    std::mutex queueMutex_;               // 任务队列的锁

    bool empty()
    {
        queueMutex_.lock();
        bool ret = queue_.empty();
        jsQueue_.unlock();
        return ret;
    }
    int push(char *script, int length) // 入队
    {
        queueMutex_.lock();
        JsItem *item = (JsItem * item) malloc(sizeof(JsItem));
        item->length = length;
        item->address = script;
        jsQueue_.push(item);
        queueMutex_.unlock();
        return S_OK;
    }
    int pop(char *script, int &length) // 出队,传入的参数是要带数据出去的
    {
        queueMutex_.lock();
        JsItem *item = jsQueue_.front();
        jsQueue_.pop();
        if (item != NULL) {
            script = item->address;
            length = item->length;
            free(item);
        }
        queueMutex_.unlock();
        return S_OK;
    }
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_UTILS_V8WORK_H__