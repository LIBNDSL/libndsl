/**
 * @file Clientplugin.h
 * @brief
 *
 * @author zzt
 * @emial 429834658@qq.com
 **/

#include "ndsl/utils/Plugin.h"
#include "ndsl/net/Multiplexer.h"
#include "ndsl/net/Clientplugin.h"

void Client::add(void *)
{
    printf("result = \n");
    mymul->hellom();
}
