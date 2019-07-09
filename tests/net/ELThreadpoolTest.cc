/**
 * @file ELThreadpoolTest.cc
 * @brief
 * ELThreadpool的单元测试
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include "../catch.hpp"
#include "ndsl/net/ELThreadpool.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/utils/Error.h"
#include "ndsl/utils/WorkQueue.h"

using namespace ndsl::net;
using namespace ndsl::utils;

void ELTP_func(void *) {}

TEST_CASE("ELThreadpool")
{
    ELThreadpool eltp;
    SECTION("setMaxThreads")
    {
        int ret = eltp.setMaxThreads(2);
        REQUIRE(ret == S_OK);

        REQUIRE(eltp.getMaxThreads() == 2);

        eltp.setMaxThreads(1024);
        REQUIRE(ret == S_OK);
        REQUIRE(eltp.getMaxThreads() == (2 * Thread::getNumberOfProcessors()));
    }

    SECTION("quit")
    {
        eltp.start();

        // start只有一个线程
        REQUIRE(eltp.getLoopsNum() == 1);

        EventLoop *loop = eltp.getNextEventLoop();

        loop = eltp.getNextEventLoop();

        WorkItem *wi = new WorkItem;
        wi->doit = ELTP_func;
        wi->param = NULL;

        loop->addWork(wi);

        // 以后没getNextEventLoop再起线程
        REQUIRE(eltp.getLoopsNum() == 2);

        REQUIRE(eltp.quit() == S_OK);

        REQUIRE(eltp.getLoopsNum() == 0);
    }
}