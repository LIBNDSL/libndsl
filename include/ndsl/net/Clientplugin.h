/**
 * @file
 * @brief
 *
 * @author zzt
 * @emial 429834658@qq.com
 **/

#ifndef __NDSL_NET_CLIENTPLUGIN_H__
#define __NDSL_NET_CLIENTPLUGIN_H__
#include "ndsl/utils/Plugin.h"
#include "ndsl/net/Multiplexer.h"
#include "ndsl/net/Protbload.pb.h"

struct Client : Plugin1
{
    ndsl::net::Multiplexer *mymul;
    void add(void *);
};

#endif // __NDSL_NET_CLIENTPLUGIN_H__