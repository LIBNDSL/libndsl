/**
 * @file TimeWheel.cc
 * @brief
 * 时间轮,方便事件管理
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include <algorithm>
#include <sys/timerfd.h>
#include "ndsl/utils/Log.h"
#include "ndsl/utils/Error.h"
#include "ndsl/net/TimeWheel.h"

namespace ndsl {
namespace net {

TimeWheel::TimeWheel(EventLoop *loop)
    : curTick_(0)
    , pLoop_(loop)
    , ptimerfdChannel_(NULL)
    , isWorking_(false)
{}

TimeWheel::~TimeWheel()
{
    if (ptimerfdChannel_) delete ptimerfdChannel_;
}

// 初始化,创建TimerfdChannel
int TimeWheel::initChannel()
{
    // 生成timerfd
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd == -1) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_TIMEWHEEL,
            "timerfd_create errno = %d:%s\n",
            errno,
            strerror(errno));
        return errno;
    }

    // 初始化TimerfdChannel
    ptimerfdChannel_ = new TimerfdChannel(fd, pLoop_);
    ptimerfdChannel_->setCallBack(onTick, NULL, this);

    for (int i = 0; i < SLOTNUM; i++)
        slot_[i].clear();

    return S_OK;
}

int TimeWheel::start()
{
    int ret = initChannel();
    if (ret != S_OK) return ret;

    // 设置时间轮的时间间隔
    struct itimerspec new_value;

    new_value.it_value.tv_sec = INTERVAL;
    new_value.it_value.tv_nsec = 0;

    new_value.it_interval.tv_sec = INTERVAL;
    new_value.it_interval.tv_nsec = 0;

    // 注册时间间隔
    if (timerfd_settime(ptimerfdChannel_->getFd(), 0, &new_value, NULL) == -1) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_TIMEWHEEL,
            "timerfd_settime errno = %d:%s\n",
            errno,
            strerror(errno));
        return errno;
    }

    // 注册ptimerChannel
    ret = ptimerfdChannel_->enrollIn(false);
    if (ret != S_OK) return ret;

    isWorking_ = true;
    return S_OK;
}

// 停止时间轮
int TimeWheel::stop()
{
    struct itimerspec stop_value;

    stop_value.it_interval.tv_nsec = 0;
    stop_value.it_interval.tv_sec = 0;

    stop_value.it_value.tv_sec = 0;
    stop_value.it_value.tv_nsec = 0;

    if (timerfd_settime(ptimerfdChannel_->getFd(), 0, &stop_value, NULL) ==
        -1) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_TIMEWHEEL,
            "timerfd_settime errno = %d:%s\n",
            errno,
            strerror(errno));
        return errno;
    }

    isWorking_ = false;
    return S_OK;
}

int TimeWheel::addTask(Task *task)
{
    // 若task为空,直接返回
    if (task->setInterval < 0 || task->doit == NULL) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_TIMEWHEEL,
            "invalid task! task->setInterval = %d\n",
            task->setInterval);
        return S_FALSE;
    }
    int setTick;

    // 若setInterval为0,则下一时刻立即响应
    if (task->setInterval == 0)
        setTick = (curTick_ + 1) % SLOTNUM;
    else
        setTick = (curTick_ + task->setInterval) % SLOTNUM;

    task->setTick = setTick;
    // 设置restInterval
    task->restInterval = task->setInterval;
    slot_[setTick].push_back(task);

    return S_OK;
}

int TimeWheel::removeTask(Task *task)
{
    // task为空,直接返回
    if (task->setInterval == -1 || task->setTick == -1 || task->doit == NULL) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_TIMEWHEEL,
            "invalid task! task->setInterval = %d, task->setTick = %d\n",
            task->setInterval,
            task->setTick);
        return S_FALSE;
    }

    // 标准模板库提供的find函数
    auto iter = std::find(
        slot_[task->setTick].begin(), slot_[task->setTick].end(), task);

    // 若没有找到,则直接返回
    if (iter == slot_[task->setTick].end()) {
        LOG(LOG_INFO_LEVEL,
            LOG_SOURCE_TIMEWHEEL,
            "erased failed: not found task!\n");
        return S_FALSE;
    }

    slot_[task->setTick].erase(iter);
    LOG(LOG_INFO_LEVEL, LOG_SOURCE_TIMEWHEEL, "erased!\n");
    return S_OK;
}

// 每个时钟周期的响应函数
int TimeWheel::onTick(void *pThis)
{
    TimeWheel *ptw = (TimeWheel *) pThis;
    uint64_t exp;

    int ret = read(ptw->ptimerfdChannel_->getFd(), &exp, sizeof(uint64_t));
    if (ret == -1) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_TIMEWHEEL,
            "read errno = %d:%s\n",
            errno,
            strerror(errno));
        return errno;
    }

    // 刻度自增
    ++ptw->curTick_;
    if (ptw->curTick_ >= 60) ptw->curTick_ %= ptw->SLOTNUM;

    // 若队列为空,则直接返回
    if (ptw->slot_[ptw->curTick_].empty()) return S_OK;

    std::list<TimeWheel::Task *> curSlot;

    // 将对应的list拉下来一一处理
    curSlot.swap(ptw->slot_[ptw->curTick_]);

    for (auto it = curSlot.begin(); it != curSlot.end();) {
        // 若不在本轮,则减少轮数后再加入相应的list
        if ((*it)->restInterval > ptw->SLOTNUM) {
            (*it)->restInterval -= ptw->SLOTNUM;
            ptw->slot_[ptw->curTick_].push_back(*it);
        } else {
            (*it)->doit((*it)->param);
            if ((*it)->times > 0) (*it)->times--;

            // 若times减到0,由用户自己释放所分配的内存
            // But,用户如何知道何时可以释放内存
            if ((*it)->times == 0) {
                Task *delTask = *it;
                // 一定要以这种形式(it++)
                curSlot.erase(it++);
                // 此处释放Task
                delete delTask;
                continue;
            }

            // 若任务是周期性执行,即(*it)->times == -1 或 (*it)->times > 0
            // ,则再次插入,并重新选择时间槽和设置restInterval值
            (*it)->setTick =
                (ptw->curTick_ + (*it)->setInterval) % ptw->SLOTNUM;
            (*it)->restInterval = (*it)->setInterval;
            ptw->slot_[(*it)->setTick].push_back((*it));
            // 指向下一个地址
            ++it;
        }
    }

    return S_OK;
}
} // namespace net
} // namespace ndsl