////
// @file Endian.cc
// @brief
// 实现大小端的转换
//
// @author lanyue
// @email luckylanry@163.com
//
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ndsl/utils/Endian.h>

namespace ndsl {
namespace utils {
// // 类型强转，implict_cast只能执行up-cast,派生类->基类
// template<typename To, typename From>
// inline To implicit_cast(From const &f)
// {
//   return f;
// }
Endian::Endian() {}

uint64_t Endian::hToN64(uint64_t host64) { return htobe64(host64); }
uint32_t Endian::hToN32(uint32_t host32) { return htobe32(host32); }

uint16_t Endian::hToN16(uint16_t host16) { return htobe16(host16); }

// uint64_t 的整形数字由网络字节序转化为主句字节序
uint64_t Endian::nToH64(uint64_t net64) { return be64toh(net64); }

uint32_t Endian::nToH32(uint32_t net32) { return be32toh(net32); }

uint16_t Endian::nToH16(uint16_t net16) { return be16toh(net16); }
// /* 提取出套接字地址的 ip 点分十进制 */
// void Endian::toIp(char* buf, size_t size,const struct sockaddr* addr)
// {
//     if (addr->sa_family == AF_INET)
//     {
//         assert(size >= INET_ADDRSTRLEN);
//         const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
//         /* inet_ntop 将二进制整数转化为点分十进制 */
//         ::inet_ntop(AF_INET, &addr4->sin_addr, buf,
//         static_cast<socklen_t>(size));
//      }
//      else if (addr->sa_family == AF_INET6)
//      {
//          assert(size >= INET6_ADDRSTRLEN);
//          const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
//          ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf,
//          static_cast<socklen_t>(size));
//       }
//  }
} // namespace utils
} // namespace ndsl