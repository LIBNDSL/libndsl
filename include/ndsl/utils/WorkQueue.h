/*
 * @file WorkQueue.h
 * @brief
 * 任务队列类
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#ifndef __NDSL_UTILS_WORKQUEUE_H__
#define __NDSL_UTILS_WORKQUEUE_H__

#include <list>
#include <mutex>
#include "ndsl/utils/Error.h"

namespace ndsl {
namespace utils {

// 定义work结构体
// TODO:堆上分配内存,最终会在WorkQueue中释放
struct WorkItem
{
    // 任务回调函数
    using Callback = void (*)(void *);
    Callback doit; // 回调函数
    void *param;   // 回调函数的参数指针
};

/**
 * @brief
 * 任务队列
 */
class WorkQueue
{
  private:
    std::list<WorkItem *> queue_; // 任务队列,用list实现
    std::mutex queueMutex_;       // 任务队列的锁

  public:
    // 队列是否为空
    bool empty()
    {
        queueMutex_.lock();
        bool ret = queue_.empty();
        queueMutex_.unlock();
        return ret;
    }
    // 执行队头任务
    void doit()
    {
        WorkItem *curWork;

        std::list<struct WorkItem *> getQueue;

        queueMutex_.lock();
        getQueue.swap(queue_);
        queueMutex_.unlock();

        while (!getQueue.empty()) {
            // 取队首任务
            curWork = getQueue.front(); // 若非空，则取出
            getQueue.pop_front();       // 删除第一个元素

            // 执行任务
            curWork->doit(curWork->param);

            // TODO:在此处释放WorkItem
            delete curWork;
        }
    }
    // 返回队列长度
    size_t size() { return queue_.size(); }
    // 队尾增加任务
    int enqueue(WorkItem *item)
    {
        queueMutex_.lock();
        queue_.push_back(item);
        queueMutex_.unlock();
        return S_OK;
    }
    // 删除队中任务
    int dequeue(WorkItem *item)
    {
        queueMutex_.lock();
        for (auto it = queue_.begin(); it != queue_.end(); ++it) {
            if ((*it) == item) {
                queue_.erase(it);
                queueMutex_.unlock();
                // 此处释放WorkItem
                delete item;
                return S_OK;
            }
        }
        queueMutex_.unlock();
        return S_FALSE;
    }

    // 返回队头任务
    WorkItem *dequeue()
    {
        queueMutex_.lock();
        // 若queue为空,则返回NULL
        if (queue_.empty()) {
            queueMutex_.unlock();
            return NULL;
        }

        WorkItem *item = queue_.front();
        queue_.pop_front();
        queueMutex_.unlock();
        return item;
    }
};
} // namespace utils
} // namespace ndsl

#endif // __NDSL_UTILS_WORKQUEUE_H__