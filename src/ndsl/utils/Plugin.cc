////
// @file Plugin.cc
// @brief
// plugin 实现
//
// @author zhangsiqi
// @email 1575033031@qq.com
//

#include <stdio.h>
#include "ndsl/config.h"
#include "ndsl/utils/Plugin.h"

#if defined(__cplusplus)
extern "C" {
#endif

Plugin *CreatPlugin(int tag)
{
    // client
    if (tag == 1) {
        return new Plugin1;
    } else
        return NULL;
    // server
    if (tag == 2) {
        return new Plugin2;
    } else
        return NULL;
}

#if defined(__cplusplus)
}
#endif
