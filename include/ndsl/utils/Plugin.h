////
// @file Plugin.h
// @brief
// plugin头文件
//
// @author zhangsiqi
// @email 1575033031@qq.com
//
#ifndef __NDSL_UTILS_PLUGIN_H__
#define __NDSL_UTILS_PLUGIN_H__

#include "ndsl/config.h"
#include "ndsl/utils/Error.h"
#if defined(__cplusplus)
extern "C" {
#endif

struct Plugin
{
    using functype = void (*)(void *);
    int doit(functype func, void *para);
    int tag;
};

struct Plugin1 : Plugin
{
    int doit(functype func, void *para)
    {
        if (func) {
            (*func)(para);
            return S_OK;
        }
        return S_FALSE;
    }
};

struct Plugin2 : Plugin
{
    int doit(functype func, void *para)
    {
        if (func) {
            (*func)(para);
            return S_OK;
        }
        return S_FALSE;
    }
};
Plugin *CreatPlugin(int tag);

#if defined(__cplusplus)
}
#endif

#endif // __NDSL_UTILS_PLUGIN_H__