/**
 * @file EventLoopThreadpool.h
 * @brief
 * 封装EventLoop的线程和线程池
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#ifndef __NDSL_UTILS_EVENTLOOPTHREAD_H__
#define __NDSL_UTILS_EVENTLOOPTHREAD_H__
#include <pthread.h>
#include <vector>
#include "ndsl/utils/Noncopyable.h"
#include "ndsl/utils/Thread.h"

namespace ndsl {

namespace net {
class EventLoop;
}

namespace utils {

class EventLoopThread
    : public noncopyable
    , public nonmoveable
{
  private:
    net::EventLoop *loop_; // 记录线程中的loop
    Thread thread_;        // 线程
    bool running;          // 标志线程是否在使用

  public:
    EventLoopThread(net::EventLoop *loop);
    ~EventLoopThread();

    // 启动线程
    int run();
    // 等待线程结束
    int join();
    // 线程是否正在运行
    bool isRunning() const;
};

class EventLoopThreadpool
    : public noncopyable
    , public nonmoveable
{
  private:
    std::vector<EventLoopThread *> loopThreads_; // 线程池中的所有线程
    std::vector<net::EventLoop *> loops_; // 线程池中所有的EventLoop

    // 线程池容量与CPU物理核心的倍数
    static const int Redundancy = 2;
    // 最大线程数 默认为两倍的CPU核心数
    unsigned int MaxThreads_;
    // 当前线程指向
    unsigned int nextThread_;

  public:
    EventLoopThreadpool();
    ~EventLoopThreadpool();

    // 初始化线程池
    int start();
    // 获取EventLoop
    net::EventLoop *getNextEventLoop();

    // 设置最大线程数
    int setMaxThreads(unsigned int maxThreads);
    // 获取最大线程数
    unsigned int getMaxThreads();
    // 关闭线程池中所有线程
    void quit();
};

} // namespace utils
} // namespace ndsl

#endif // __NDSL_UTILS_EVENTLOOPTHREAD_H__