/**
 * @file EventLoopTest.cc
 * @brief
 * EventLoop的单元测试，包含WorkQueue的单元测试
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include <sys/eventfd.h>
#include "../catch.hpp"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/Epoll.h"
#include "ndsl/utils/Log.h"

using namespace ndsl;
using namespace net;

bool ev = false;

void func1(void *para) { ev = true; }

void func2(void *para) { ev = false; }

TEST_CASE("net/EventLoop(WorkQueue)")
{
    SECTION("init")
    {
        EventLoop loop;
        REQUIRE(loop.init() == S_OK);
    }

    SECTION("addwork/removeWork and runAfter/quit")
    {
        EventLoop loop;
        REQUIRE(loop.init() == S_OK);

        utils::WorkItem *w1 = new utils::WorkItem;
        w1->doit = func1;
        w1->param = NULL;
        loop.addWork(w1);

        loop.quit();
        REQUIRE(loop.loop(&loop) == S_OK);

        REQUIRE(ev == true);

        utils::WorkItem *w2 = new utils::WorkItem;
        w2->doit = func2;
        w2->param = NULL;
        loop.addWork(w2);
        REQUIRE(loop.removeWork(w2) == S_OK);

        loop.runAfter(1, loop.quit, &loop);
        REQUIRE(loop.loop(&loop) == S_OK);

        REQUIRE(ev == true);
    }
}