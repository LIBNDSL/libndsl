////
// @file Endian.h
// @brief
// 转换主机与网络字节序
//
// @author lanry
// @email luckylanry@163.com
//
#ifndef __NDSL_UTILS_ENDIAN_H__
#define __NDSL_UTILS_ENDIAN_H__

#include <stdint.h>
#include <endian.h>
#include <arpa/inet.h>

namespace ndsl {
namespace utils {
class Endian
{
  public:
    Endian();

  public:
    // uint64_t 的整形数字由机器字节序转化为网络字节序
    uint64_t hToN64(uint64_t host64);

    uint32_t hToN32(uint32_t host32);

    uint16_t hToN16(uint16_t host16);

    // uint64_t 的整形数字由网络字节序转化为主句字节序
    uint64_t nToH64(uint64_t net64);

    uint32_t nToH32(uint32_t net32);

    uint16_t nToH16(uint16_t net16);

  private:
    uint64_t host64;
    uint32_t host32;
    uint16_t host16;
    //   // 返回套接字地址ip和port点分十进制
    //  void toIpPort(char* buf, size_t size,const struct sockaddr* addr);
    //  void toIp(char* buf, size_t size,const struct sockaddr* addr);
    //   // 从ip和port转换到sockaddr_in类型
    //  void fromIpPort(const char* ip, uint16_t port,struct sockaddr_in* addr);
    //  const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
    //  const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr*
    //  addr);
};
} // namespace utils
} // namespace ndsl

#endif // __NDSL_UTILS_ENDIAN_H__