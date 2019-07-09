////
// @file test.cc
// @brief
// 采用catch作为单元测试方案，需要一个main函数，这里定义。
//
// @author niexw
// @email xiaowen.nie.cn@gmail.com
//
#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <ndsl/utils/Log.h>

int main(int argc, char *argv[])
{
    set_ndsl_log_sinks(LOG_SOURCE_ALL, LOG_OUTPUT_TER);
    int result = Catch::Session().run(argc, argv);
    return result;
}
