/**
 * @file SignalfdTest.cc
 * @brief
 * signalfd测试
 *
 * @author why
 * @email 136046355@qq.com
 */
//#define CATCH_CONFIG_MAIN
#include "../catch.hpp"
#include <iostream>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

#include "ndsl/net/EventLoop.h"
#include "ndsl/net/SignalHandler.h"
#include "ndsl/utils/Log.h"

void aaa(void *p)
{
    LOG(LOG_INFO_LEVEL, LOG_SOURCE_SIGNALFDCHANNEL, "This is SIGCHLD handler!");
}

TEST_CASE("signalfd")
{
    // 启动服务
    // 初始化EPOLL
    ndsl::net::EventLoop loop;
    loop.init();

    // 初始化signalHandler
    ndsl::net::SignalHandler sh(&loop);

    unsigned int arr[1] = {SIGCHLD};
    SECTION("regist")
    {
        // 注册SIGCHLD信号
        REQUIRE(sh.registSignalfd(arr, 1, aaa, NULL) == 0);

        if (fork() == 0) { exit(0); }

        sleep(1);

        // 添加中断
        loop.quit();
        // 开始loop
        loop.loop(&loop);
    }

    SECTION("remove")
    {
        REQUIRE(sh.registSignalfd(arr, 1, aaa, NULL) == 0);

        if (fork() == 0) { exit(0); }

        sleep(1);

        REQUIRE(sh.remove() == 0);

        // 添加中断
        loop.quit();
        // 开始loop
        loop.loop(&loop);
    }

    SECTION("blockAllSignals && unBlockAllSignals")
    {
        REQUIRE(ndsl::net::SignalHandler::blockAllSignals() == 0);

        if (fork() == 0) { exit(0); }

        sleep(1);

        REQUIRE(ndsl::net::SignalHandler::unBlockAllSignals() == 0);
        for (int i = 0; i < 64; i++) {
            if (sh.blockSignals_[i] != 0) {
                LOG(LOG_INFO_LEVEL,
                    LOG_SOURCE_SIGNALFDCHANNEL,
                    "this is blockAllSignalsTest! blockSignals_[i] = %d",
                    sh.blockSignals_[i]);
            }
        }
    }
}
