////
// @file PluginTest.cc
// @brief
// 测试Plugin
//
// @author zhangsiqi
// @email 1575033031@qq.com
//
#include "../catch.hpp"
#include "ndsl/config.h"
#include "ndsl/utils/Plugin.h"



void func(void* para)
{
    //cdprintf("func!\n");
}

TEST_CASE("Plugin")
{
    SECTION("doit")
    {      
        Plugin1* plugin;
        plugin = (Plugin1 *)CreatPlugin(1);
        REQUIRE(plugin->doit(func,NULL) == S_OK);
    }
}