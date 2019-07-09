/*
 * @file Thread.h
 * @brief
 * 封装线程
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#ifndef __NDSL_UTILS_THREAD_H__
#define __NDSL_UTILS_THREAD_H__
#include <pthread.h>

namespace ndsl {
namespace utils {
// 线程类
class Thread
{
  public:
    using ThreadFunc = void *(*) (void *); // 线程函数

  private:
    ThreadFunc func_; // 线程函数
    void *param_;     // 函数参数
    pthread_t id_;    // 线程id
    void *ret_;       // 线程返回值

  public:
    Thread(ThreadFunc threadfunc, void *param);
    ~Thread();

    // 开启线程
    int run();
    // 等待线程结束
    int join(void **retval = NULL);
    // CPU核心数
    static int getNumberOfProcessors();

  private:
    static void *runInThread(void *);
};
} // namespace utils

} // namespace ndsl
#endif // __NDSL_UTILS_THREAD_H__
