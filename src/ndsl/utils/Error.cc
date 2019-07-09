////
// @file Error.cc
// @brief
// 封装错误处理函数
//
// @author lanyue
// @email luckylanry@163.com
//

#include <stdio.h>
#include <arpa/inet.h>
#include "ndsl/utils/Error.h"

namespace ndsl {
namespace utils {

Error::Error() {}

int Error::Handler(int err, char *s)
{
    int error = err;
    if (error == S_FALSE) {
        char *n = strerror_r(error, errmsg, sizeof(errmsg));
        if (n == NULL) {}
        printf("%s\n", errmsg);
    } else {
        errCode = S_OK;
    }
    return 0;
}
void Error::perr_exit(const char *s)
{
    perror(s);
    exit(S_FALSE);
}
const char *Error::getError(int error)
{
    switch (error) {
    case EINTR: {
        return "Interrupted system call";
    }
    case ETIMEDOUT: {
        return "Connection timedout";
    }
    case EAGAIN: {
        return "Try again";
    }
    case EPIPE: {
        return "Broken pipe";
    }
    case EBADF: {
        return "Bad file number";
    }
    case ECONNREFUSED: {
        return "Connection refused";
    }
    case ECONNRESET: {
        return "Connection reset by peer";
    }
    case EINVAL: {
        return "Invalid argument";
    }
    case EMFILE: {
        return "Too many open files";
    }
    case ENOTCONN: {
        return "Transport endpoint is not connected";
    }
    case ECONNABORTED: {
        return "Software caused connection abort";
    }
    case ENETUNREACH: {
        return "Network is unreachable";
    }
    case ENETRESET: {
        return "Network dropped connection because of reset ";
    }
    case EINPROGRESS: {
        return " Operation now in progress";
    }
    case ENOTSOCK: {
        return "Socket operation on non-socket";
    }
    case EDESTADDRREQ: {
        return "Destination address required";
    }
    case EMSGSIZE: {
        return "Message too long";
    }
    case EPROTOTYPE: {
        return "Protocol wrong type for socket";
    }
    case ENOPROTOOPT: {
        return "Protocol not available ";
    }
    case EPROTONOSUPPORT: {
        return "Protocol not supported";
    }
    case ESOCKTNOSUPPORT: {
        return "Socket type not supported";
    }
    case EOPNOTSUPP: {
        return "Operation not supported on transport endpoint";
    }
    case EPFNOSUPPORT: {
        return "Protocol family not supported";
    }
    case EAFNOSUPPORT: {
        return "Address family not supported by protocol ";
    }
    case EADDRINUSE: {
        return "Address already in use";
    }
    case EADDRNOTAVAIL: {
        return "Cannot assign requested address";
    }
    case ENETDOWN: {
        return "Network is down";
    }
    case ENOBUFS: {
        return "No buffer space available";
    }
    case EISCONN: {
        return "Transport endpoint is already connected";
    }
    }
    return 0;
}

} // namespace utils
} // namespace ndsl