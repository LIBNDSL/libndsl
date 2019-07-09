/**
 * @file Semaphore.h
 * @brief
 * 信号量
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#ifndef __NDSL_UTILS_SEMAPHORE_H__
#define __NDSL_UTILS_SEMAPHORE_H__
#include <pthread.h>

namespace ndsl {
namespace utils {

class Semaphore
{
  private:
    unsigned int resources_;
    pthread_mutex_t mutex_;
    pthread_cond_t cond_;

  public:
    Semaphore(int resources = 0);
    ~Semaphore();

    void give(unsigned int r = 1);
    void take(unsigned int r = 1);

    int available() const;
};

} // namespace utils
} // namespace ndsl

#endif // __NDSL_UTILS_SEMAPHORE_H__