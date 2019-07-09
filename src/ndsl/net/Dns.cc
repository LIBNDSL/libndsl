////
// @file Dns.cc
// @brief
// 实现异步dns
//
// @author zjc
// @email zjc@qq.com

#include <arpa/inet.h>
#include <ndsl/net/Dns.h>

static const unsigned short DEFAULT_QUERY_FLAGS = 0x0100; //缺省查询标志
static const unsigned short DEFAULT_QUERY_TYPE = 0x0001; // 缺省查询ip地址
static const unsigned short DEFAULT_INET_CLASS = 0x0001; // 缺省返回internet
static const unsigned short QUERY_RESPONSE_FLAGS = 0x8000; // 查询响应

// dns报文按1字节对齐
#pragma pack(push, 1)
////
// @biref
// dns 报文头部
//
struct PacketHead {
  /* data */
  unsigned short id;        // dns报文 id
  unsigned short flags;     // 标志位
  unsigned short quests;    // 问题数目
  unsigned short answers;   // 应答数目
  unsigned short authors;   // 认证应答个数
  unsigned short additions; //未认证应答个数
  PacketHead()
      : id(0), flags(::htons(DEFAULT_QUERY_FLAGS)), quests(::htons(1)),
        answers(0), authors(0), additions(0) {}
};
////
// @biref
// dns报文尾部
//
struct PacketTail {
  /* data */
  unsigned short type;
  unsigned short classes;
};

#pragma pack(pop)
namespace ndsl {
namespace net {
int Dns::init(EventLoop *evloop) {
  if (evloop == NULL) {
    LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DNS,
        "Dns::init,eventloop pointer is null\n");
    return EINVAL;
  } else {
    loop_ = evloop;
  }
  //创建套接字

  int dnsfd = ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
  // TOOD 套接字赋值 非组赛// SOCK_CLOEXEC,
  udp_ = new UdpChannle(dnsfd, loop_); //创建udpchannle   del在哪

  // udp_->erase()
  // udp_->handle
  udp_->setCallBack(handle_read, NULL, udp_);
  udp_->app = this;
  // udp_->enrollIn(true);

  // udp_()
  if (udp_->getFd() == -1) {
    LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DNS,
        "Dns::init,fail to creat the udp socket,ret=%d\n", errno);
    return errno;
  }
  //注册事件-

  int ret = udp_->enrollIn(true);
  if (ret)
    return ret;
  return S_OK;
}

void Dns::stop() {
  // 注销并关闭udp
  loop_->erase(udp_);

  ::close(udp_->getFd());
  // udp_.fd = -1; // TOOD ::更改fd值
  delete udp_; //释放udpchannel
  if (count_ != 0)
    LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DNS,
        "Dns::stop ,there are %ld request left while stop\n", count_);

  //释放内存
  for (int i = 0; i < DNS_BUCKET_SIZE; ++i) {
    while (!bucket_[i].empty()) {
      BucketEntry *be = reinterpret_cast<BucketEntry *>(bucket_[i].next);
      be->erase();
      delete be;
      /* code */
    }
  }
}

int Dns::setNameServer(SocketAddress4 &srv) {
  // 如果有dns请求，则失效
  if (count_) {
    LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DNS,
        "Dns::setNameServer, there are still %ld requests are left\n", count_);
    return EINVAL;
  } else
    address_ = srv;

  // disconnect
  // SocketAddress4 addr;
  // //
  // AF_UNSPEC则意味着函数返回的是适用于指定主机名和服务名且适合任何协议族的地址。如果某个主机既有AAAA记录(IPV6)地址，同时又有A记录(IPV4)地址，那么AAAA记录将作为sockaddr_in6结构返回，而A记录则作为sockaddr_in结构返回
  // addr.sin_family = AF_UNSPEC;
  // ::connect(udp_->getFd(), (const struct sockaddr *)&addr, sizeof(addr));

  // connect
  int ret = ::connect(udp_->getFd(), (const struct sockaddr *)&address_,
                      sizeof(address_)); //
  if (ret == -1)
    return errno;

  //启动接收
  ret = ::recv(udp_->getFd(), buffer_, BUFFER_SIZE, 0);
  // TOOD 出错点  recv函数中出了问题
  if (ret != -1 && errno != EAGAIN) {
    LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DNS,
        "Dns::setNameServer, fail to start receive, ret=%d errno=%d\n", ret,
        errno);
    return errno;
  } else
    return EAGAIN; // 必须是EAGAIN
}

int Dns::request(DnsTask &task) {
  if (task.domain_size >= 255)
    return EINVAL;

  // 分配entry
  BucketEntry *be = new BucketEntry;
  be->task = &task;
  count_++;

  // 加锁访问bucket
  task.query_id = current_++;
  unsigned short index = task.query_id % DNS_BUCKET_SIZE;

  // 入队
  bucket_[index].insert_after(*be);

  // 报头

  char packet[512];
  PacketHead *head = new (packet) PacketHead;
  head->id = task.query_id;
  size_t len = sizeof(PacketHead);
  // 报体
  if (!coding(task, packet + len)) {
    be->erase();
    delete be;
    --count_;
    LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DNS,
        "Dns::request, invalid query because the label is greater than 64 "
        "bytes\n");
    return EINVAL;
  }
  len += task.domain_size + 2; // 一头一尾
  // 报尾
  PacketTail *tail = new (packet + len) PacketTail;
  tail->type = htons(DEFAULT_QUERY_TYPE);
  tail->classes = htons(DEFAULT_INET_CLASS);
  len += sizeof(PacketTail);

  // 发送packet，udp为非阻塞
retry:
  int ret = ::send(udp_->getFd(), packet, len, 0);
  // delete head;
  // delete tail;

  if (ret == -1) {
    int error = errno;
    if (error == EAGAIN)
      goto retry;
    else {
      // delete be;      //TOOD del
      return error;
    }
  }

  return S_OK;
}

bool Dns::coding(DnsTask &task, char *buffer) {
  bcopy(task.domain_name, buffer + 1, task.domain_size);
  buffer[task.domain_size + 1] = 0;

  char *p = buffer + 1;
  size_t i = 0;
  while (p - buffer < task.domain_size + 1) {
    if (*p == '.') {
      if (i > 64)
        return false;
      *(p - i - 1) = (char)i;
      i = 0;
    } else {
      ++i;
    }
    ++p;
  }
  if (i > 64)
    return false;
  *(p - i - 1) = (char)i;
  return true;
}

int Dns::handle_read(void *param) {

  UdpChannle *udp = reinterpret_cast<UdpChannle *>(param);
  Dns *pThis = reinterpret_cast<Dns *>(udp->app);
  LOG(LOG_DEBUG_LEVEL, LOG_SOURCE_DNS, "dns read response");
  // 读数据
retry:
  int ret =
      ::recv(pThis->udp_->getFd(), pThis->buffer_, BUFFER_SIZE, 0); // ret 大小
  if (ret == -1)
    return -1;

  // 检查头部
  PacketHead *head = (PacketHead *)pThis->buffer_;
  if (!(ntohs(head->flags) & QUERY_RESPONSE_FLAGS))
    goto retry;

  int answers = ntohs(head->answers);
  int quests = ntohs(head->quests);
  unsigned short id = head->id;

  // 问题
  char *p = (char *)pThis->buffer_ + sizeof(PacketHead);
  for (int i = 0; i < quests; ++i) {

    if ((*p) & 0xc0) { // 指针
      *p = 0x3f & *p;  // 清掉指针bit位11 有两位表示字符字符数的
      // short offset = ntohs(*((short *)p));
      p += 2;
    } else {
      while (*p != 0)
        ++p;
      ++p; // 尾部0
    }
    p += sizeof(PacketTail);
  }

  // 答案
  for (int i = 0; i < answers; ++i) {

    if ((*p) & 0xc0) { // 指针
      *p = 0x3f & *p;  // 清掉指针bit位11

      p += 2;
    } else {
      while (*p != 0)
        ++p;
      ++p;
    }
    short type = ntohs(*((short *)p)); // type
    p += 2;

    p += 2;

    p += 4;
    short dlen = ntohs(*((short *)p)); // data length  15
    p += 2;
    char *data = p;
    if (type == DEFAULT_QUERY_TYPE) {         // A
      DnsTask *dtask = pThis->find_query(id); //返回查找的问题task

      if (dtask) {
        // ::bcopy(data, &dtask->answer_address.sin_addr.s_addr, 4); //对的
        // //此后4个字节是答案
        // dtask->answer_address.sin_family = AF_INET;
        // // dtask->param = dtask;
        // pThis->loop_->addWork(dtask);
        // goto retry; // @todo 只选第1个答案    判断buf的

        // 1 增加到vector，并增加p的
        SocketAddress4 *it = new SocketAddress4;
        ::bcopy(data, &(it->sin_addr.s_addr), 4);
        it->sin_family = AF_INET;
        dtask->answer_address.push_back(*it);

        // 2 如果还有数据执行loop
        if (answers - 1 == i) {
          pThis->loop_->addWork(dtask);
          goto retry;
          /* code */
        }

        //没有插入loop并retry
      }
    }
    p += dlen;
  }
  return 0;
}

DnsTask *Dns::find_query(unsigned short id) {
  int index = id % DNS_BUCKET_SIZE;
  DnsTask *task = NULL;
  BucketEntry *be = NULL;

  for (be = reinterpret_cast<BucketEntry *>(bucket_[index].next);
       reinterpret_cast<ListEntry *>(be) != bucket_ + index;
       be = reinterpret_cast<BucketEntry *>(be->next)) {
    if (be->task->query_id == id) { // 从时间轮上摘下定时器
      be->erase();
      task = be->task;
      delete be; // 删除BucketEntry TOOD double free
      --count_;
      break;
    }
  }

  return task;
}

} // namespace net
} // namespace ndsl