////
// @file TimeStamp.h
// @brief
// 封装std::chrono
//
// @author zhangsiqi
// @email 1575033031@qq.com
//

#ifndef __NDSL_UTILS_TIMESTAMP_H__
#define __NDSL_UTILS_TIMESTAMP_H__

#include <stdio.h>
#include <strings.h>
#include <ctime>
#include <string>
#include <chrono>

namespace ndsl {
namespace utils {

struct TimeStamp
{
    std::chrono::system_clock::time_point stamp_;
    void now() { stamp_ = std::chrono::system_clock::now(); }
    bool to_string(char *buffer, size_t size) const;
    void from_string(const char *time);
};
inline bool operator<(const TimeStamp &lhs, const TimeStamp &rhs)
{
    char buf_lhs[512];
    char buf_rhs[512];
    lhs.to_string(buf_lhs, 512);
    rhs.to_string(buf_rhs, 512);
    int i = 0;
    while (buf_lhs[i]) {
        if (buf_lhs[i] > buf_rhs[i]) {
            return false;
        } else if (buf_lhs[i] < buf_rhs[i]) {
            return true;
        } else {
            i++;
        }
    }
    return false;
}
inline bool operator==(const TimeStamp &lhs, const TimeStamp &rhs)
{
    char buf_lhs[512];
    char buf_rhs[512];
    lhs.to_string(buf_lhs, 512);
    rhs.to_string(buf_rhs, 512);
    int i = 0;
    while (buf_lhs[i]) {
        if (buf_lhs[i] == buf_rhs[i]) {
            i++;
        } else {
            return false;
        }
    }
    return true;
}
} // namespace utils
} // namespace ndsl

#endif //__NDSL_UTILS_TIMESTAMP_H__