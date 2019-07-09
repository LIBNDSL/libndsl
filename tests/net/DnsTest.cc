////
// @file DnsTest.cc
// @brife
// Dns的测试
// @author zjc
// @email 2364464666@qq.com
//
// win10子系统和ubuntu的编译环境还是有些不一样

#include "../catch.hpp"
#include <ndsl/net/Dns.h>
#include <ndsl/net/ELThreadpool.h>

static const unsigned short DEFAULT_QUERY_FLAGS = 0x0100; // 缺省查询标志
static const unsigned short DEFAULT_QUERY_TYPE = 0x0001; // 缺省查询ip地址
static const unsigned short DEFAULT_INET_CLASS = 0x0001; // 缺省返回

void dns_doit(void *param) { // TOOD 参数出了问题  workitem 的多次释放
  ndsl::net::DnsTask *task = reinterpret_cast<ndsl::net::DnsTask *>(param);
  char buf[64];
  int count = task->answer_address.size();

  for (int i = 0; i < count; i++) {
    task->answer_address[i].getIP(buf);
    // buf += 16;
  }
  LOG(LOG_DEBUG_LEVEL, LOG_SOURCE_DNS, "www.com-- %s\n", buf);
  // LOG(LOG_DEBUG_LEVEL, LOG_SOURCE_DNS, "www.comFASADFF-- ");
}

TEST_CASE("net/Dns") {
  SECTION("set") {

    ndsl::net::Dns dns;
    ndsl::net::ELThreadpool pool;
    pool.start();

    ndsl::net::EventLoop *loop = pool.getNextEventLoop();

    int ret = dns.init(loop);
    REQUIRE(ret == S_OK);

    ndsl::net::SocketAddress4 addr("8.8.8.8", 53);

    ret = dns.setNameServer(addr);

    REQUIRE(ret == EAGAIN);

    ndsl::net::DnsTask *task = new ndsl::net::DnsTask;
    task->domain_name = "www.baidu.com";
    task->domain_size = ::strlen(task->domain_name);
    task->doit = dns_doit;
    ret = dns.request(*task);

    sleep(5);
    dns.stop();
    //  sleep(1000);
    REQUIRE(ret == S_OK);
  }
}