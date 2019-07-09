////
// @file TimeStampTest.cc
// @brief
// 测试TimeStamp
//
// @author zhnagsiqi
// @email 1575033031@qq.com
//
#include "../catch.hpp"
#include <string.h>
#include "ndsl/utils/TimeStamp.h"

using namespace ndsl;
using namespace utils;

TEST_CASE("utils/TimeStamp")
{
    SECTION("to_from")
    {
        ndsl::utils::TimeStamp ts;

        ts.now();
        char buf[512];
        char *time;
        REQUIRE(ts.to_string(buf, 512));
        time = (char *) buf;
        REQUIRE(::strncmp(buf, time, ::strlen(time)) == 0);

        const char *time1 = (const char *) buf;
        ts.from_string(time1);
        REQUIRE(ts.to_string(buf, 512));
        REQUIRE(::strncmp(buf, time1, ::strlen(time1)) == 0);
    }

    SECTION("test_equal")
    {
        ndsl::utils::TimeStamp ts;
        ndsl::utils::TimeStamp ts1;
        ndsl::utils::TimeStamp ts2;
        ndsl::utils::TimeStamp ts3;
        ndsl::utils::TimeStamp ts4;

        ts.now();
        const char *time1 = "2018-12-02 14:40:08.521990";
        const char *time2 = "2018-03-02 12:47:08.245690";
        const char *time3 = "2019-12-02 14:40:08.521990";
        const char *time4 = "2019-12-02 14:40:08.521990";

        ts1.from_string(time1);
        ts2.from_string(time2);
        ts3.from_string(time3);
        ts4.from_string(time4);

        REQUIRE(ts2 < ts1);
        REQUIRE(ts2 < ts3);
        REQUIRE(ts1 < ts3);
        REQUIRE(ts3 == ts4);
    }
}