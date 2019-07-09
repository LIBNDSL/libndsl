/*
 * @file WQThreadpool.cc
 * @brief
 * WorkQueue的线程池
 * 可用于耗时的工作,比如:磁盘IO
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include "ndsl/net/WQThreadpool.h"
#include "ndsl/utils/Log.h"

namespace ndsl {
namespace net {

WQThreadpool::WQThreadpool()
    : maxThreads_(
          utils::Thread::getNumberOfProcessors() *
          2) // 设置最大线程数为CPU核心数的两倍
    , isRunning_(false)
{
    // 初始化互斥量和条件变量
    ::pthread_mutex_init(&mutex_, NULL);
    ::pthread_cond_init(&cond_, NULL);
}
WQThreadpool::~WQThreadpool()
{
    // 释放互斥量和条件变量
    ::pthread_mutex_destroy(&mutex_);
    ::pthread_cond_destroy(&cond_);

    // 退出
    if (isRunning_) quit();
}

int WQThreadpool::setMaxThreads(const int maxThreads)
{
    // 若线程池已经启动,则不能更改线程池大小
    if (isRunning_) return S_FALSE;
    // 超过最大值,不能设置成功
    if (maxThreads > maxThreads_) return S_FALSE;

    maxThreads_ = maxThreads;
    return S_OK;
}

int WQThreadpool::getMaxThreads() const { return maxThreads_; }

// 启动线程池(开启所有线程)
int WQThreadpool::start()
{
    if (isRunning_) return S_FALSE;

    for (int i = 0; i < maxThreads_; i++) {
        utils::Thread *pThread = new utils::Thread(doWork, this);
        pThread->run();
        vThreads.push_back(pThread);
    }

    isRunning_ = true;
    return S_OK;
}

// 退出线程池
int WQThreadpool::quit()
{
    if (!isRunning_) return S_FALSE;

    for (int i = 0; i < maxThreads_; i++) {
        utils::WorkItem *item = new utils::WorkItem;
        item->doit = quitFunc;
        item->param = NULL;
        addWork(item);
    }

    isRunning_ = false; // 设置退出标志

    for (auto it = vThreads.begin(); it != vThreads.end(); ++it) {
        if ((*it)->join() != S_OK) {
            LOG(LOG_ERROR_LEVEL, LOG_SOURCE_WQTHREADPOOL, "quit error.");
            return S_FALSE;
        }
        delete (*it);
    }
    return S_OK;
}

// 线程函数
void *WQThreadpool::doWork(void *param)
{
    WQThreadpool *WQPool = static_cast<WQThreadpool *>(param);
    utils::WorkItem *item; // 接收WorkQueue中的任务

    while (true) {
        ::pthread_mutex_lock(&WQPool->mutex_);

        while (WQPool->workqueue_.size() <= 0)
            ::pthread_cond_wait(&WQPool->cond_, &WQPool->mutex_);

        // 取出任务
        item = WQPool->workqueue_.dequeue();
        // 若任务队列还有任务,继续发送信号
        if (WQPool->workqueue_.size() > 0)
            ::pthread_cond_signal(&WQPool->cond_);

        ::pthread_mutex_unlock(&WQPool->mutex_);

        // 如果是退出函数,则退出
        if (item->doit == WQPool->quitFunc) {
            delete item;
            break;
        }

        item->doit(item->param);
        // 释放内存
        delete item;
    }
    return NULL;
}

// 增加任务
int WQThreadpool::addWork(utils::WorkItem *item)
{
    if (item == NULL || isRunning_ == false) return S_FALSE;

    ::pthread_mutex_lock(&mutex_);

    workqueue_.enqueue(item);

    if (workqueue_.size() > 0) ::pthread_cond_signal(&cond_);

    ::pthread_mutex_unlock(&mutex_);

    return S_OK;
}

// NOTE:该函数不会被执行
void WQThreadpool::quitFunc(void *param)
{
    LOG(LOG_ERROR_LEVEL, LOG_SOURCE_WQTHREADPOOL, "This should not be called.");
}

} // namespace net
} // namespace ndsl