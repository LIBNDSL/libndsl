/**
 * @file TimeWheel.h
 * @brief
 * TimeWheel方便事件管理
 * TimerfdChannel用于与EventLoop通信
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#ifndef __NDSL_NET_TIMEWHEEL_H__
#define __NDSL_NET_TIMEWHEEL_H__

#include <list>
#include "ndsl/net/BaseChannel.h"

namespace ndsl {
namespace net {

class EventLoop;

class TimeWheel
{
  private:
    // TimefdChanel与EventLoop之间通信
    class TimerfdChannel : public BaseChannel
    {
      public:
        TimerfdChannel(int fd, EventLoop *loop)
            : BaseChannel(fd, loop)
        {}
        ~TimerfdChannel()
        {
            if (getFd() > 0) {
                erase(); // 从EventLoop中取消注册
                ::close(getFd());
            }
        }
    };

  public:
    struct Task
    {
        using Taskfunc = void (*)(void *);
        int setInterval = -1;  // 设置的时间间隔
        int times = 1;         // -1:无限制响应   1:响应次数
        Taskfunc doit;         // 函数指针
        void *param;           // 函数参数
        int restInterval = -1; // 剩余时间间隔(不由用户设置)
        int setTick = -1; // 记录放在哪个slot位置上(不由用户设置)
    };

    // 转一圈需要1min,一共有60个槽,两个槽之间间隙为1s
    static const int SLOTNUM = 60; // 一共有60个时间槽
    static const int INTERVAL = 1; // 每两个槽之间的间隔为1s

  private:
    int curTick_;                     // 指示当前刻度
    EventLoop *pLoop_;                // TimerfdChannel注册的eventloop
    TimerfdChannel *ptimerfdChannel_; // 注册在epoll上的channel
    std::list<Task *> slot_[SLOTNUM]; // 每个刻度上的任务队列(双向链表)
    bool isWorking_;                  // 标记定时器是否工作

  public:
    TimeWheel(EventLoop *loop);
    ~TimeWheel();

    int start(); // 开始运行时间轮
    int stop();  // 停止运行时间轮

    int addTask(Task *Task);    // 增加新任务
    int removeTask(Task *task); // 删除任务

    static int onTick(void *pThis); // 处理事件

    // 定时器是否正在工作
    bool isWork() { return isWorking_; }

  private:
    int initChannel(); // 初始化TimerfdChannel和slot
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_TIMEWHEEL_H__