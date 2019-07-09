/*
 * @file WQThreadpoolTest.cc
 * @brief
 * WorkQueue线程池的单元测试
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include "../catch.hpp"
#include "ndsl/net/WQThreadpool.h"
#include "ndsl/utils/WorkQueue.h"
#include "ndsl/utils/Log.h"

using namespace ndsl::net;
using namespace ndsl::utils;

void WQThreadpoolTestFunc(void *param)
{
    long times = reinterpret_cast<long>(param);

    LOG(LOG_INFO_LEVEL, LOG_SOURCE_WQTHREADPOOL, "sleep %d seconds.", times);

    sleep(times);

    LOG(LOG_INFO_LEVEL,
        LOG_SOURCE_WQTHREADPOOL,
        "sleep %d seconds over.",
        times);
}

TEST_CASE("WQThreadpool")
{
    WQThreadpool WQpool;
    SECTION("setMaxThreads")
    {
        REQUIRE(WQpool.setMaxThreads(2) == S_OK);
        REQUIRE(WQpool.getMaxThreads() == 2);
    }

    SECTION("run")
    {
        WQpool.setMaxThreads(4);
        REQUIRE(WQpool.start() == S_OK);
        WorkItem *item[4];
        for (long i = 0; i < 4; ++i) {
            item[i] = new WorkItem();
            item[i]->doit = WQThreadpoolTestFunc;
            item[i]->param = (void *) (i + 1);
            WQpool.addWork(item[i]);
        }
        sleep(5);
        WQpool.quit();
    }

    SECTION("quit")
    {
        REQUIRE(WQpool.start() == S_OK);

        REQUIRE(WQpool.quit() == S_OK);

        WorkItem *item = new WorkItem;
        item->doit = WQThreadpoolTestFunc;
        item->param = (void *) 0;

        // 退出后,再加入任务无效
        REQUIRE(WQpool.addWork(item) == S_FALSE);
        delete item;

        // 退出后,再次退出无效
        REQUIRE(WQpool.quit() == S_FALSE);
    }
}