/**
 * @file server.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/config.h"
#include "ndsl/net/ELThreadpool.h"

using namespace ndsl;
using namespace net;
using namespace utils;

#define BUFSIZE 16384

TcpAcceptor *tAc;
uint64_t mlog;
ELThreadpool *threadPool;

static void mError(void *se, int eno);

class Session
{
  private:
    static void onMessage(void *pthis)
    {
        // LOG(LOG_ERROR_LEVEL, mlog, "");
        Session *session = static_cast<Session *>(pthis);
        ssize_t n;

        if (session->len_ > 0) {
            if ((n = session->conn_->onSend(
                     session->buf_,
                     session->len_,
                     0,
                     onSendMessage,
                     session)) == S_FALSE) {
                LOG(LOG_ERROR_LEVEL, mlog, "send error");
            }
        }
    }

  public:
    static void onSendMessage(void *pthis)
    {
        // LOG(LOG_ERROR_LEVEL, mlog, "");
        Session *session = static_cast<Session *>(pthis);

        int n;
        if (!session->isStop_) {
            // LOG(LOG_ERROR_LEVEL, mlog, "send a message");
            // 循环注册onRecv
            if ((n = session->conn_->onRecv(
                     session->buf_, &(session->len_), 0, onMessage, session)) ==
                S_FALSE) {
                LOG(LOG_ERROR_LEVEL, mlog, "recv error");
            }
        }
    }

    Session(TcpConnection *conn)
        : conn_(conn)
        , len_(0)
        , isStop_(false)
        , bytesWritten_(0)
    {}
    ~Session() {}

    static void start(void *pthis)
    {
        Session *session = static_cast<Session *>(pthis);
        session->conn_->onError(mError, session);
        int n = session->conn_->onRecv(
            session->buf_, &(session->len_), 0, onMessage, session);
        if (n == S_FALSE) { LOG(LOG_ERROR_LEVEL, mlog, "recv error"); }
    }

    char buf_[BUFSIZE];
    TcpConnection *conn_;
    size_t len_;
    bool isStop_;
    ssize_t bytesWritten_;
};

static void stop(void *pthis)
{
    Session *session = static_cast<Session *>(pthis);
    delete session->conn_;
    delete session;
}

static void mError(void *se, int eno)
{
    Session *session = static_cast<Session *>(se);
    if (session->len_ == 0 && !(session->isStop_) && eno == 11) {
        LOG(LOG_ERROR_LEVEL, mlog, "bad error");
        session->isStop_ = true;

        // 添加到addWord
        WorkItem *w1 = new WorkItem;
        w1->doit = stop;
        w1->param = session;
        session->conn_->pTcpChannel_->pLoop_->addWork(w1);

        // LOG(LOG_ERROR_LEVEL, mlog, "byteWrite = %zd",
        // session->bytesWritten_);
    }
}

static void onConnection(void *conn)
{
    TcpConnection *Con = static_cast<TcpConnection *>(conn);
    Session *session = new Session(Con);
    // session->start();

    // 向loop里面添加任务
    WorkItem *w1 = new WorkItem;
    w1->doit = Session::start;
    w1->param = session;
    Con->pTcpChannel_->pLoop_->addWork(w1);

    TcpConnection *con1 = new TcpConnection();
    tAc->onAccept(
        con1, NULL, NULL, onConnection, con1, threadPool->getNextEventLoop());
}

int main(int argc, char *argv[])
{
    // 设置log
    // mlog = add_source();
    // set_ndsl_log_sinks(mlog, LOG_OUTPUT_FILE);

    if (argc < 4) {
        LOG(LOG_ERROR_LEVEL, mlog, "Usage: server <address> <port> <threads>");
    } else {
        int s;

        // 初始化EPOLL
        EventLoop loop;
        s = loop.init();
        if (s < 0) LOG(LOG_ERROR_LEVEL, mlog, "loop init fail");

        struct SocketAddress4 servaddr(
            argv[1], static_cast<unsigned short>(atoi(argv[2])));

        // 设定线程数量
        int threadCount = atoi(argv[3]);

        // 初始化线程池
        threadPool = new ELThreadpool();
        threadPool->setMaxThreads(threadCount);

        threadPool->start();

        tAc = new TcpAcceptor(&loop);
        s = tAc->start(servaddr);
        if (s < 0) LOG(LOG_ERROR_LEVEL, mlog, "tAc->start fail");

        TcpConnection *Conn = new TcpConnection();

        tAc->onAccept(
            Conn,
            NULL,
            NULL,
            onConnection,
            Conn,
            threadPool->getNextEventLoop());

        // 运行
        EventLoop::loop(&loop);

        delete tAc;
        delete threadPool;
    }
}
