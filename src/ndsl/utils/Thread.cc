/*
 * @file Thread.cc
 * @brief
 * 封装线程
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
// #include <pthread.h>
#include "ndsl/utils/Thread.h"
#include "ndsl/utils/Error.h"
#include "ndsl/utils/Log.h"

namespace ndsl {
namespace utils {

Thread::Thread(ThreadFunc threadfunc, void *param)
    : func_(threadfunc)
    , param_(param)
    , id_(0)
    , ret_(NULL)
{}

Thread::~Thread() {}

int Thread::run()
{
    int ret = ::pthread_create(&id_, NULL, runInThread, (void *) this);
    if (ret != S_OK) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_THREAD,
            "pthread_create errno = %d:%s\n",
            errno,
            strerror(errno));
        return S_FALSE;
    }
    return S_OK;
}

int Thread::join(void **retval)
{
    int ret = ::pthread_join(id_, retval);
    if (ret != S_OK) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_THREAD,
            "pthread_join errno = %d:%s\n",
            errno,
            strerror(errno));
        return S_FALSE;
    }
    return S_OK;
}

void *Thread::runInThread(void *arg)
{
    LOG(LOG_INFO_LEVEL, LOG_SOURCE_THREAD, "one thread is running!\n");

    Thread *th = static_cast<Thread *>(arg);
    th->ret_ = th->func_(th->param_);
    return (void *) th->ret_;
}

int Thread::getNumberOfProcessors()
{
    int ret = ::sysconf(_SC_NPROCESSORS_ONLN);
    if (ret == -1) {
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_THREAD,
            "sysconf errno = %d:%s\n",
            errno,
            strerror(errno));
        return S_FALSE;
    }

    return ret;
}
} // namespace utils

} // namespace ndsl