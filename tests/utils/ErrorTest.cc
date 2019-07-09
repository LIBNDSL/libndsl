////
// @file ErrorTest.cc
// @brief
// 测试Error
//
// @author lanyue
// @email luckylanry@163.com
//
#include "../catch.hpp"
#include "ndsl/utils/Error.h"

TEST_CASE("utils/Error")
{
    ndsl::utils::Error err;
    SECTION("geterror")
    {
        int ln = 4;
        char str[32] = "Interrupted system call";
        err.getError(ln);
        REQUIRE(strcmp((err.getError(ln)), str) == 0);
        // char str[32] = "0D1A1E";
        // err.perr_exit(str);
        // err.lan();
    }
    // SECTION("GET")
    // {
    //     int m=110;
    //     char strl[32]="Connection timedout";
    //     REQUIRE((err.getError(m)) == strl);
    // }
    // SECTION("perr_exit"){
    //    err.perr_exit(errmsg);
    //    std::cout << errmsg << std::endl;
    // }
    // SECTION("getError")
    // {
    //     char errmsg1_[64];
    //     errmsg1_ = err.getError(error_);
    //     std::cout << errmsg1_ << std::endl;
    // }
}
