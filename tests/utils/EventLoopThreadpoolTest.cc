/**
 * @file EventLoopThreadpoolTest.cc
 * @brief
 * EventLoopThreadpool的单元测试
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include "../catch.hpp"
#include "ndsl/utils/EventLoopThreadpool.h"
#include "ndsl/utils/Error.h"

using namespace ndsl::net;
using namespace ndsl::utils;

TEST_CASE("EventLoopThreadpool")
{
    EventLoopThreadpool eltp;
    SECTION("setMaxThreads")
    {
        int ret = eltp.setMaxThreads(4);
        REQUIRE(ret == S_OK);

        REQUIRE(eltp.getMaxThreads() == 4);

        eltp.setMaxThreads(1024);
        REQUIRE(ret == S_OK);
        REQUIRE(eltp.getMaxThreads() == (2 * Thread::getNumberOfProcessors()));
    }
}