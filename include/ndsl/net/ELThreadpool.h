/**
 * @file ELThreadpool.h
 * @brief
 * 封装EventLoop的线程和线程池
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#ifndef __NDSL_UTILS_EVENTLOOPTHREAD_H__
#define __NDSL_UTILS_EVENTLOOPTHREAD_H__
#include <vector>
#include "ndsl/utils/Noncopyable.h"
#include "ndsl/utils/Thread.h"

namespace ndsl {

namespace net {
class EventLoop;

// EventLoop线程类
class ELThread
    : public utils::Thread
    , public utils::noncopyable
    , public utils::nonmoveable
{
  private:
    EventLoop *loop_; // 记录线程中的loop

  public:
    ELThread(EventLoop *loop);
    ~ELThread();
};

// EventLoop线程池
class ELThreadpool
    : public utils::noncopyable
    , public utils::nonmoveable
{
  private:
    std::vector<ELThread *> loopThreads_; // 线程池中的所有线程
    std::vector<EventLoop *> loops_;      // 线程池中所有的EventLoop

    // 线程池容量与CPU物理核心的倍数
    static const int Redundancy = 2;
    // 最大线程数 默认为两倍的CPU核心数
    unsigned int MaxThreads_;
    // 当前线程指向
    unsigned int nextThread_;

  public:
    ELThreadpool();
    ~ELThreadpool();

    // 初始化线程池
    int start();
    // 获取EventLoop
    EventLoop *getNextEventLoop();

    // 设置最大线程数
    int setMaxThreads(unsigned int maxThreads);
    // 获取最大线程数
    unsigned int getMaxThreads() const;

    // 关闭线程池中所有线程
    int quit();
    // 已有EventLoop数量
    int getLoopsNum();

  private:
    int capacity(); // 默认最大线程数
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_UTILS_EVENTLOOPTHREAD_H__