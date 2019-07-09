/**
 * @file Semaphore.cc
 * @brief
 * 信号量
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include "ndsl/utils/Semaphore.h"
#include "ndsl/utils/Log.h"
#include "ndsl/utils/Error.h"

namespace ndsl {
namespace utils {

Semaphore::Semaphore(int resources)
    : resources_(resources)
{
    ::pthread_mutex_init(&mutex_, NULL);
    ::pthread_cond_init(&cond_, NULL);
}

Semaphore::~Semaphore()
{
    ::pthread_mutex_destroy(&mutex_);
    ::pthread_cond_destroy(&cond_);
}

void Semaphore::take(unsigned int r)
{
    pthread_mutex_lock(&mutex_);
    while (resources_ < r) {
        pthread_cond_wait(&cond_, &mutex_);
    }

    resources_ -= r;
    if (resources_ > 0) pthread_cond_signal(&cond_);

    pthread_mutex_unlock(&mutex_);
}

void Semaphore::give(unsigned int r)
{
    pthread_mutex_lock(&mutex_);
    resources_ += r;

    // 若资源数大于0,则通知其他线程
    if (resources_ > 0) pthread_cond_signal(&cond_);

    pthread_mutex_unlock(&mutex_);
}

int Semaphore::available() const { return resources_; }

} // namespace utils
} // namespace ndsl