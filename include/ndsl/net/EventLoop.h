/**
 * @file EventLoop.h
 * @brief
 * 事件循环的封装
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#ifndef __NDSL_NET_EVENTLOOP_H__
#define __NDSL_NET_EVENTLOOP_H__

#include <unistd.h>
#include "ndsl/utils/Log.h"
#include "ndsl/utils/Error.h"
#include "ndsl/net/Epoll.h"
#include "ndsl/net/BaseChannel.h"
#include "ndsl/utils/WorkQueue.h"
#include "ndsl/net/TimeWheel.h"

namespace ndsl {
namespace net {

/**
 * @class EventLoop
 * @brief
 * 事件循环: 包含一个QueueChannel和一个InterruptChannel
 */
class EventLoop
{
  private:
    /**
     * @class QueueChannel
     * @brief
     * 用于维护任务队列
     */
    class QueueChannel : public BaseChannel
    {
      private:
        utils::WorkQueue workqueue_; // 任务队列

      public:
        QueueChannel(int fd, EventLoop *loop)
            : BaseChannel(fd, loop)
        {}
        ~QueueChannel()
        {
            // 关闭描述符
            if (getFd() >= 0) {
                erase(); // 从EventLoop中取消注册
                ::close(getFd());
            }
        }

        // 增加任务
        int addWork(utils::WorkItem *item) { return workqueue_.enqueue(item); }

        // 删除任务
        int removeWork(utils::WorkItem *item)
        {
            return workqueue_.dequeue(item);
        }

        // 发送中断信号
        int onWrite()
        {
            uint64_t data = 1;
            int ret = ::write(getFd(), &data, sizeof(data));

            if (ret == -1) {
                LOG(LOG_ERROR_LEVEL,
                    LOG_SOURCE_EVENTLOOP,
                    "QueueChannel::onWrite write errno = %d:%s\n",
                    errno,
                    strerror(errno));
                return errno;
            }

            return S_OK;
        }
        // 处理队列中的任务
        static int onQueue(void *pThis)
        {
            QueueChannel *pQc = (QueueChannel *) pThis;
            while (!pQc->workqueue_.empty()) {
                pQc->workqueue_.doit();
            }
            return S_OK;
        }
    };
    /**
     * @class InterruptChannel
     * @brief
     * 中断EventLoop,退出循环
     */
    class InterruptChannel : public BaseChannel
    {
      private:
        EventLoop *loop_;

      public:
        InterruptChannel(int fd, EventLoop *loop)
            : BaseChannel(fd, loop)
        {}
        ~InterruptChannel()
        {
            if (getFd() >= 0) {
                erase(); // 从EventLoop中取消注册
                ::close(getFd());
            }
        }

        // 发送中断信号
        int onWrite()
        {
            uint64_t data;
            data = 1;

            int ret = ::write(getFd(), &data, sizeof(data));

            if (ret == -1) {
                LOG(LOG_ERROR_LEVEL,
                    LOG_SOURCE_EVENTLOOP,
                    "InterruptChannel::onWrite write errno = %d:%s\n",
                    errno,
                    strerror(errno));
                return errno;
            }

            return S_OK;
        }
    };

  private:
    Epoll epoll_;               // 用于监听事件
    QueueChannel *pQueCh_;      // 用于维护任务队列的Channel
    InterruptChannel *pIntrCh_; // 用于退出的channel
    TimeWheel timer_;           // 用于完成定时任务

  public:
    EventLoop();
    ~EventLoop();

    // 初始化eventloop，默认不打开timer
    // 但在调用runAfter时会自动打开
    int init(bool timerOn = false);

    // 开始循环
    static void *loop(void *pThis);

    // 添加任务
    int addWork(utils::WorkItem *item);
    int removeWork(utils::WorkItem *item);

    // 注册、更新、删除事件
    int enroll(Channel *);
    int modify(Channel *);
    int erase(Channel *);

    // 退出
    void quit();
    // 用于回调的退出函数
    static void quit(void *pThis);

    // 定时任务
    int runAfter(TimeWheel::Task *task);
    TimeWheel::Task *
    runAfter(int ns, TimeWheel::Task::Taskfunc func, void *param);

    // 取消定时任务
    int cancelRunAfter(TimeWheel::Task *task);
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_EVENTLOOP_H__