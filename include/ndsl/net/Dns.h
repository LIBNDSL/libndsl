////
// @file Dns.h
// @brife
// 异步，带缓存Dns
//
// @author hideinbed
// @email 2364464666@qq.com
//
#ifndef __NDSL_NET_DNS_H_
#define __NDSL_NET_DNS_H_
#include "../utils/WorkQueue.h"
#include "./BaseChannel.h"
#include "./SocketAddress.h"
#include "EventLoop.h"
#include <unistd.h>
#include <vector>

// ndsl\include\ndsl\utils\WorkQueue.h

namespace ndsl {
namespace net {
////
// @brief
// dns任务
//
using TaskCallback = void (*)(void *);
using EventCallback = void (*)(unsigned int &events, void *);

struct DnsTask : ndsl::utils::WorkItem {
  const char *domain_name; // 域名字符串
  // SocketAddress4 answer_address;
  std::vector<SocketAddress4> answer_address; // 查询结果地址，只有一个？

  int domain_size;         // 字符串长度
  unsigned short query_id; // 查询id
  DnsTask() : domain_name(NULL), domain_size(0), query_id(0) { param = this; }
};

struct UdpChannle : public BaseChannel {
  /* data */
  enum {
    OTHER = 0,   //其他
    NETWORK = 1, //网络
  };
  // channel 文件描述符
  int error;           //错误码
  void *app;           //定制指针
  unsigned short type; //类型
  UdpChannle(int fd, EventLoop *loop)
      : BaseChannel(fd, loop), error(0), app(NULL), type(NETWORK) {}

  ~UdpChannle() { close(); }
  void close() {
    if (BaseChannel::getFd()) {
      ::close(BaseChannel::getFd());
    }
  }
};

// Dns
class Dns {
  struct ListEntry {
    /* data */
    ListEntry *next;
    ListEntry *prev;

    ListEntry() : next(this), prev(this) {}
    inline bool empty() { return next == this && prev == this; }
    inline void insert_after(ListEntry &x) {
      next->prev = &x;
      x.next = next;
      x.prev = this;
      next = &x;
    }
    inline void erase() {
      next->prev = prev;
      prev->next = next;
    }
  };
  ////
  // @brief
  // dns请求实体
  //
  struct BucketEntry : ListEntry {
    /* data */
    DnsTask *task;
    BucketEntry() : task(NULL) {}
  };
  ////
  // @brief
  // 请求桶头
  //
  struct BucketHead : ListEntry

  {};
  static const int DNS_BUCKET_SIZE = 64; // 64 轮
  static const int BUFFER_SIZE = 4096;   // buffer大小

private:
  EventLoop *loop_; //指定的loop，一个dns对应一个loop
  UdpChannle *udp_; // udp 套接字

  size_t count_;                       // 请求个数
  BucketHead bucket_[DNS_BUCKET_SIZE]; // 查询bucket
  SocketAddress4 address_;             // dns服务器地址
  void *buffer_;                       // 接收buffer指针
  unsigned int current_;               //当前id
                                       // 全局lock   TD 没写

public:
  Dns() : loop_(NULL), udp_(NULL), count_(0), buffer_(NULL), current_(0) {
    address_.sin_family = AF_UNSPEC;

    // udp_.type = UdpChannle::NETWORK; //网络
    // udp_.app = this;
    // udp_.handleRead_ = handle_read; //读处理函数
    buffer_ = ::malloc(BUFFER_SIZE);
  }
  ~Dns() {
    ::free(buffer_);
    // delete udp_;
  }

  // 初始化
  int init(EventLoop *evloop);
  // 停止服务
  void stop();
  // 要求没有dns请求，并且设定后必须返回EAFGAIN
  int setNameServer(SocketAddress4 &srv);
  //请求dns
  int request(DnsTask &task);

private:
  // 编码query name ,dot 替代为字节数，尾部为0
  bool coding(DnsTask &task, char *buffer);
  // channel 事件回调函数
  static int handle_read(void *param);
  // 再bucket中查找query
  DnsTask *find_query(unsigned short id);
};
} // namespace net
} // namespace ndsl
#endif // __NDSL_NET_DNS_H_