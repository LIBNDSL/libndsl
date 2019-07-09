/*
 * @file ThreadTest.cc
 * @brief
 * 线程类的测试
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include "../catch.hpp"
#include "ndsl/utils/Thread.h"
#include "ndsl/utils/Log.h"
#include "ndsl/utils/Error.h"

using namespace ndsl::utils;

void *test1(void *arg)
{
    LOG(LOG_INFO_LEVEL,
        LOG_SOURCE_ELTHREADPOOL,
        "This is thread %ld\n",
        (long) arg);
    return (void *) 0;
}

TEST_CASE("Thread")
{
    SECTION("run and join")
    {
        Thread t1(test1, (void *) 1);
        REQUIRE(t1.run() == S_OK);

        REQUIRE(t1.join() == S_OK);
    }
}