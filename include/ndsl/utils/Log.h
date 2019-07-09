////
// @file Log.h
// @brief
// log头文件
//
// @author zhangsiqi
// @email 1575033031@qq.com
//
#ifndef __NDSL_UTILS_LOG_H__
#define __NDSL_UTILS_LOG_H__

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define LOG(level, source, format, ...)                                        \
    ndsl_log_into_sink(                                                        \
        level, source, __FILE__, __FUNCTION__, format, ##__VA_ARGS__)

////
// @brief
// 日志级别
//
enum
{
    LOG_DEBUG_LEVEL = 0,
    LOG_INFO_LEVEL = 1,
    LOG_WARN_LEVEL = 2,
    LOG_ERROR_LEVEL = 3
};
////
// @brief
// 日志源
//

const uint64_t LOG_SOURCE_EPOLL = 1;
const uint64_t LOG_SOURCE_WQTHREADPOOL = 1UL << 1;
const uint64_t LOG_SOURCE_EVENTLOOP = 1UL << 2;
const uint64_t LOG_SOURCE_THREAD = 1UL << 3;
const uint64_t LOG_SOURCE_V8ENGINE = 1UL << 4;
const uint64_t LOG_SOURCE_CHANNEL = 1UL << 5;
const uint64_t LOG_SOURCE_UDPCHANEEL = 1UL << 6;
const uint64_t LOG_SOURCE_TCPCHANNEL = 1UL << 7;
const uint64_t LOG_SOURCE_UNIXCHANNEL = 1UL << 8;
const uint64_t LOG_SOURCE_RDMACLIENT = 1UL << 9;
const uint64_t LOG_SOURCE_TIMEFDCHANNEL = 1UL << 10;
const uint64_t LOG_SOURCE_SIGNALFDCHANNEL = 1UL << 11;
const uint64_t LOG_SOURCE_DNSCHANNEL = 1UL << 12;
const uint64_t LOG_SOURCE_FILECHANNEL = 1UL << 13;
const uint64_t LOG_SOURCE_TCPACCETPOR = 1UL << 14;
const uint64_t LOG_SOURCE_TCPCONNECTION = 1UL << 15;
const uint64_t LOG_SOURCE_UNIXCONNECTION = 1UL << 16;
const uint64_t LOG_SOURCE_TIMEWHEEL = 1UL << 17;
const uint64_t LOG_SOURCE_SEMAPHORE = 1UL << 18;
const uint64_t LOG_SOURCE_NAMESERVICE = 1UL << 19;
const uint64_t LOG_SOURCE_CONNECTION = 1UL << 20;
const uint64_t LOG_SOURCE_MULTIPLEXER = 1UL << 21;
const uint64_t LOG_SOURCE_ENTITY = 1UL << 22;
const uint64_t LOG_SOURCE_SOCKETOP = 1UL << 23;
const uint64_t LOG_SOURCE_SPLITE3 = 1UL << 24;
const uint64_t LOG_SOURCE_XML = 1UL << 25;
const uint64_t LOG_SOURCE_ENDIAN = 1UL << 26;
const uint64_t LOG_SOURCE_TIMESTAMP = 1UL << 27;
const uint64_t LOG_SOURCE_ERROR = 1UL << 28;
const uint64_t LOG_SOURCE_PLUGIN = 1UL << 29;
const uint64_t LOG_SOURCE_ADDRESS4 = 1UL << 30;
const uint64_t LOG_SOURCE_GUID = 1UL << 31;
const uint64_t LOG_SOURCE_ELTHREADPOOL = 1UL << 32;
const uint64_t LOG_SOURCE_TCPCLIENT = 1UL << 33;
const uint64_t LOG_SOURCE_DNS = 1UL << 34;
const uint64_t LOG_SOURCE_DTP = 1UL << 35;
const uint64_t LOG_SOURCE_DTPCLIENT = 1UL << 36;
const uint64_t LOG_SOURCE_RDMAACCEPTOR = 1UL << 37;
const uint64_t LOG_SOURCE_RDMACONNECTION = 1UL << 38;
const uint64_t LOG_SOURCE_UDPENDPOINT = 1UL << 39;

const uint64_t LOG_SOURCE_ALL = ~0;

////
// @brief
// 添加日志源
//

uint64_t add_source();

////
// @brief
// 输出选项
//
enum
{
    LOG_OUTPUT_TER = 0,
    LOG_OUTPUT_FILE = 1
};

void set_ndsl_log_sinks(uint64_t sinks, int file_or_ter);
void ndsl_log_into_sink(
    int level,
    uint64_t source,
    const char *file_name,
    const char *func_name,
    const char *format,
    ...);

#if defined(__cplusplus)
}
#endif

#endif // __NDSL_UTILS_LOG_H__