////
// @file Error.h
// @brief
// 封装网络库中的错误
//
// @author lanry
// @email luckylanry@163.com
//
#ifndef __NDSL_UTILS_ERROR_H__
#define __NDSL_UTILS_ERROR_H__

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <string>
#define S_OK 0
#define S_FALSE -1
namespace ndsl {
namespace utils {

class Error
{
  public:
    Error();
    int Handler(int error, char *s);
    void perr_exit(const char *s);
    const char *getError(int error);

  private:
    int errCode;
    char errmsg[64];
};

} // namespace  utils
} // namespace ndsl

#endif // __NDSL_UTILS_ERROR_H__