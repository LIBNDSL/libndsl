////
// @file TimeStamp.h
// @brief
// 封装std::chrono
//
// @author zhangsiqi
// @email 1575033031@qq.com
//

#include <stdio.h>
#include <strings.h>
#include <ctime>
#include <string>
#include <chrono>
#include "ndsl/utils/TimeStamp.h"

bool ndsl::utils::TimeStamp::to_string(char *buffer, size_t size) const
{
    struct tm tm;
    std::time_t tmt;
    int ms = (int)
            (std::chrono::duration_cast<std::chrono::microseconds>(stamp_.time_since_epoch()).count() % 1000000);
    tmt = std::chrono::system_clock::to_time_t(stamp_);
    localtime_r(&tmt, &tm);
    int ret = snprintf(
        buffer,
        size,
        "%4d_%02d_%02d_%02d_%02d_%02d_%06d",
        tm.tm_year + 1900,
        tm.tm_mon + 1, // 0 - 11
        tm.tm_mday,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec,
        ms
        );
    return ret > 0;
}
void ndsl::utils::TimeStamp::from_string(const char *time)
{
    struct tm tm;
    ::bzero(&tm, sizeof(tm));
    tm.tm_year = std::stoi(time) - 1900;
    tm.tm_mon = std::stoi(time + 5) - 1;
    tm.tm_mday = std::stoi(time + 8);
    tm.tm_hour = std::stoi(time + 11);
    tm.tm_min = std::stoi(time + 14);
    tm.tm_sec = std::stoi(time + 17);
    std::chrono::microseconds micro(std::stoi(time + 20));

    std::time_t tmt;
    tmt = mktime(&tm);
    stamp_ = std::chrono::system_clock::from_time_t(tmt);
    stamp_ += micro;
}
