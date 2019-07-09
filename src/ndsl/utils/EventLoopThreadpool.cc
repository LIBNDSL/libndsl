/**
 * @file EventLoopThreadpool.cc
 * @brief
 * 封装EventLoop的线程和线程池
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include "ndsl/utils/EventLoopThreadpool.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/utils/Error.h"
#include "ndsl/utils/Thread.h"
#include "ndsl/utils/Log.h"

namespace ndsl {

using namespace net;
namespace utils {

EventLoopThread::EventLoopThread(EventLoop *loop)
    : loop_(loop)
    , thread_(loop_->loop, loop)
    , running(false)
{}

EventLoopThread::~EventLoopThread()
{
    // 若线程正在运行,则退出循环,等待线程结束
    if (running) {
        loop_->quit();
        join();
    }
}

int EventLoopThread::run()
{
    int ret = thread_.run();
    running = true;
    return ret;
}

int EventLoopThread::join()
{
    int ret = thread_.join();
    running = false;
    return ret;
}

bool EventLoopThread::isRunning() const { return running; }

EventLoopThreadpool::EventLoopThreadpool()
    : MaxThreads_(0)
    , nextThread_(0)
{}

EventLoopThreadpool::~EventLoopThreadpool() {}

int EventLoopThreadpool::start()
{
    // 若没有设置最大线程数,则设为默认值
    if (MaxThreads_ == 0)
        MaxThreads_ = Redundancy * Thread::getNumberOfProcessors();

    // 暂时打开一个线程
    EventLoop *el = new EventLoop();
    EventLoopThread *elt = new EventLoopThread(el);

    loops_.push_back(el);
    loopThreads_.push_back(elt);
    // 开启线程
    elt->run();

    return S_OK;
}

EventLoop *EventLoopThreadpool::getNextEventLoop()
{
    nextThread_ %= MaxThreads_;
    // 若已有足够多的线程,则直接返回
    if (nextThread_ < loops_.size())
        return loops_[nextThread_++];
    else if (nextThread_ == loops_.size()) {
        EventLoop *el = new EventLoop();

        // TODO: 在这里对EventLoop进行初始化
        el->init();

        EventLoopThread *elt = new EventLoopThread(el);

        loops_.push_back(el);
        loopThreads_.push_back(elt);

        // 开启线程
        elt->run();

        return loops_[nextThread_++];
    } else {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_EVENTLOOPTHREADPOOL,
            "EventLoopThreadpool::getNextEventLoop");
        return NULL;
    }
}

int EventLoopThreadpool::setMaxThreads(unsigned int maxThreads)
{
    // 若设置的线程数小于当前已有的线程数,则退出
    if (maxThreads < loopThreads_.size()) return S_FALSE;

    MaxThreads_ = Redundancy * Thread::getNumberOfProcessors();
    // 若超过默认的最大值,则设置为最大值
    if (maxThreads > MaxThreads_) return S_OK;

    MaxThreads_ = maxThreads;
    return S_OK;
}

unsigned int EventLoopThreadpool::getMaxThreads() { return MaxThreads_; }

void EventLoopThreadpool::quit()
{
    if (loopThreads_.size() != loops_.size()) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_EVENTLOOPTHREADPOOL,
            "EventLoopThreadpool::quit");
        return;
    }

    for (unsigned int i = 0; i < loops_.size(); i++) {
        delete loopThreads_[i];
        delete loops_[i];
    }
}
} // namespace utils
} // namespace ndsl