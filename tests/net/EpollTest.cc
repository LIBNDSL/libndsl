/**
 * @file EpollTest.cc
 * @brief
 * Epoll的单元测试
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */

#include "../catch.hpp"
#include "ndsl/net/Epoll.h"
#include "ndsl/utils/Log.h"
#include "ndsl/utils/Error.h"

using namespace ndsl;
using namespace net;

TEST_CASE("net/Epoll")
{
    SECTION("init")
    {
        Epoll ep;
        REQUIRE(ep.init() == S_OK);
    }
}