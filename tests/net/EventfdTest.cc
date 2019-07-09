#include "../catch.hpp"

#include <ndsl/net/EventLoop.h>
#include <ndsl/net/EventfdConnection.h>

#include <sys/eventfd.h>
#include <errno.h>
#include <pthread.h>//线程


using namespace ndsl;
using namespace net;

int flagFunc1 = 1;
int fdfunc1(void *param){

printf("fdfunc1...\n");
    return (++flagFunc1);
}

TEST_CASE("eventfd test in one thread"){
    // 启动服务
    // 初始化EPOLL
    ndsl::net::EventLoop loop;
    REQUIRE(loop.init() == S_OK);

    uint64_t count = 2;
    uint64_t count2 = 1;
    int efd;

    EventfdConnection *conn = new EventfdConnection();
    int ret = conn->createEventfd(efd);//创建eventfd
    REQUIRE(ret == S_OK);
    ret = conn->createChannel(efd,&loop);//创建channel
    REQUIRE(ret == S_OK);

    //写之前读，设置为非阻塞，会产生EAGAIN的错误
    // ret = read(conn->pEventfdChannel_->getFd(),&count,sizeof(count));
    ret = conn->onRead(count,0,fdfunc1,NULL);
    REQUIRE(count == 2);//count不变
    REQUIRE(ret == S_FALSE);
    REQUIRE(errno == EAGAIN);
    errno = 0;

    //写+写+读
    ret = conn->onWrite(count,0,fdfunc1,NULL);
    REQUIRE(ret == S_OK);
    ret = conn->onWrite(count,0,fdfunc1,NULL);
    REQUIRE(ret == S_OK);
    ret = conn->onRead(count2,0,fdfunc1,NULL);
    REQUIRE(ret == S_OK);
    REQUIRE(count2 == 4);

    //再读，会产生EAGAIN的错误,count2不变
    count2 = 3;
    ret = conn->onRead(count2,0,fdfunc1,NULL);
    REQUIRE(ret == S_FALSE);
    REQUIRE(errno == EAGAIN);
    errno = 0;
    REQUIRE(count2 == 3);//count2不变

    //再写+读，能成功读
    ret = conn->onWrite(count,0,fdfunc1,NULL);
    REQUIRE(ret == S_OK);
    ret = conn->onRead(count2,0,fdfunc1,NULL);
    REQUIRE(ret == S_OK);
    REQUIRE(count2 == 2);
    REQUIRE(errno == 0);
}

//子线程，用作 先读，5秒后连续读两次
void *fun2(void *param){
    EventfdChannel *channel = (EventfdChannel *)param;
    uint64_t count = 3;
    int ret;

    EventfdConnection *conn = new EventfdConnection();
    conn -> pEventfdChannel_ = channel;

    //读
    ret = conn->onRead(count,0,fdfunc1,NULL);
    REQUIRE(ret == -1);
    REQUIRE(count == 3);
    REQUIRE(errno == EAGAIN);
    errno = 0;

    sleep(5);

    // 写两次
    count = 3;
    ret = conn->onRead(count,0,fdfunc1,NULL);
    REQUIRE(ret == 0);
    REQUIRE(count == 101*2);
    REQUIRE(errno == 0);

    count =  3;
    ret = conn->onRead(count,0,fdfunc1,NULL);
    REQUIRE(ret == S_FALSE);
    REQUIRE(count == 3);
    REQUIRE(errno == EAGAIN);
    return NULL;
}
//读写写读读
TEST_CASE("multi thread,read before write"){
    // 启动服务
    // 初始化EPOLL
    ndsl::net::EventLoop loop;
    REQUIRE(loop.init() == S_OK);
  
    uint64_t count;
    pthread_t pid;

    EventfdConnection *conn = new EventfdConnection();
    int efd;
    int ret = conn->createEventfd(efd);//创建eventfd
    REQUIRE(ret == S_OK);
    ret = conn->createChannel(efd,&loop);//创建channel
    REQUIRE(ret == S_OK);

    //子线程
    pthread_create(&pid,NULL,fun2,(void *)conn->pEventfdChannel_);

    sleep(2);
    
    // 连续写两次
    count = 101;
    ret = conn->onWrite(count,0,fdfunc1,NULL);
    REQUIRE(ret == S_OK);
    ret = conn->onWrite(count,0,fdfunc1,NULL);
    REQUIRE(ret == S_OK);

    //等待子线程结束
    pthread_join(pid,NULL);
}


