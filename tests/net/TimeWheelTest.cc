/*
 * @file TimeWheelTest.cc
 * @brief
 * TimeWheel的单元测试
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include "../catch.hpp"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/TimeWheel.h"

using namespace ndsl;
using namespace net;

void func3(void *para)
{
    printf("Call func2 and para is %s!\n", (char *) para);
}

void TwStopLoop(void *ploop)
{
    EventLoop *loop = static_cast<EventLoop *>(ploop);

    loop->quit();
}

TEST_CASE("net/TimeWheel")
{
    // 初始化EventLoop
    EventLoop eloop;
    REQUIRE(eloop.init() == S_OK);
    TimeWheel tw(&eloop);

    SECTION("start and stop")
    {
        REQUIRE(tw.start() == S_OK);
        REQUIRE(tw.stop() == S_OK);
    }

    // 如何测?
    SECTION("addTask")
    {
        tw.start();
        // TimeWheel中释放
        TimeWheel::Task *task = new TimeWheel::Task;

        task->doit = func3;
        task->param = (void *) "Hello World";
        task->setInterval = 1;

        REQUIRE(tw.addTask(task) == S_OK);

        // 设置loop退出
        TimeWheel::Task *stop = new TimeWheel::Task;

        stop->doit = TwStopLoop;
        stop->param = &eloop;
        stop->setInterval = 2;

        REQUIRE(tw.addTask(stop) == S_OK);

        eloop.loop(&eloop);
    }

    // 如何测?
    SECTION("removeTask")
    {
        tw.start();
        // TimeWheel中释放
        TimeWheel::Task *task = new TimeWheel::Task;

        task->doit = func3;
        task->param = (void *) "Goodbye World";
        task->setInterval = 61;

        REQUIRE(tw.addTask(task) == S_OK);

        REQUIRE(tw.removeTask(task) == S_OK);

        TimeWheel::Task *stop = new TimeWheel::Task;

        stop->doit = TwStopLoop;
        stop->param = &eloop;
        stop->setInterval = 2;

        REQUIRE(tw.addTask(stop) == S_OK);

        eloop.loop(&eloop);
    }
}