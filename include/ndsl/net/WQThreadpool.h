/*
 * @file WQThreadpool.h
 * @brief
 * WorkQueue的线程池
 * 可用于耗时的工作,比如:磁盘IO
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#ifndef __NDSL_NET_WORKQUEUETHREADPOOL_H__
#define __NDSL_NET_WORKQUEUETHREADPOOL_H__
#include <vector>
#include <pthread.h>
#include "ndsl/utils/Thread.h"
#include "ndsl/utils/WorkQueue.h"
#include "ndsl/utils/Noncopyable.h"

namespace ndsl {
namespace net {

// WorkQueue线程池
class WQThreadpool
{
  private:
    utils::WorkQueue workqueue_; // 任务队列,所有线程从这里获取任务
    std::vector<utils::Thread *> vThreads; // 记录所有创建的线程
    int maxThreads_; // 最大线程数,默认为CPU核心数的两倍
    bool isRunning_; // 标志线程池是否正在工作/可用于线程退出
    pthread_mutex_t mutex_; // WorkQueue的互斥量
    pthread_cond_t cond_;   // WorkQueue的条件变量

  public:
    WQThreadpool();
    ~WQThreadpool();

    // 设置线程池的大小
    int setMaxThreads(const int maxThreads);
    // 获取最大线程数
    int getMaxThreads() const;

    // 启动线程池(开启所有线程)
    int start();
    // 退出
    int quit();

    // 线程函数
    static void *doWork(void *param);

    // 增加任务
    int addWork(utils::WorkItem *item);

  private:
    // 用于退出的函数
    static void quitFunc(void *param);
};

} // namespace net
} // namespace ndsl
#endif // __NDSL_NET_WORKQUEUETHREADPOOL_H__