/**
 * @file Multiplexer.h
 * @brief
 *
 * 消息分发器，实现不同通信实体对tcp长连接的复用，每个通信实体拥有一个id，
 * 将通信实体的id与回调函数作为键值对存入map方便进行检索，回调相应的回调函数。
 * 分发器绑定在connection上，即 one multiplexer per connection.
 * 实现功能如下：
 *  1.绑定connection
 *  2.插入通信实体
 *  3.删除通信实体
 *  4.发送消息
 *  5.分发消息给实体
 *
 * @author zzt
 * @emial 429834658@qq.com
 **/

#ifndef __NDSL_NET_MULTIPLEXER_H__
#define __NDSL_NET_MULTIPLEXER_H__

#include <map>
#include <iostream>
#include "Asynclo.h" // connection抽象类
#include "TcpConnection.h"
//#include "ControlMsg.pb.h"
#include "src/ndsl/ControlMsg.pb.h"
#include "ndsl/config.h"

namespace ndsl {
namespace net {

// 自定义消息结构体
#pragma pack(push)
#pragma pack(1) // 一字节对齐
struct Message
{
    int id;  // 通信实体的id
    int len; // 负载长度
};
#pragma pack(pop)
class Entity;
class DTPClient;
class DTP;

/**
 * @class: Multiplexer
 * @brief:
 * 多路复用类
 **/
class Multiplexer
{
  public:
    // 定义消息回调函数
    // using Callback =
    //     void (*)(Multiplexer *Multiplexer, char *buffer, int len, int error);

    DTP *dtp_;        // 保存DTP对象
    EventLoop *loop_; // loop对象

  private:
    // 自定义addwork传入的参数结构体
    struct para
    {
        Multiplexer *mul;
        ndsl::net::Entity *entity;
        int id;
        Asynclo *conn;
        DTPClient *dtpClient;
    };

    // 用来保存一次没读完的数据
    struct incompleteBuf
    {
        incompleteBuf()
            : headSize(0)
            , buf(NULL)
            , bufSize(0)
        {
            // TODO: 可以优化一下
            head = (char *) calloc(sizeof(struct Message), sizeof(char));
        }

        ~incompleteBuf() { free(head); }
        char *head;   // 暂存残缺的头
        int headSize; // 头的大小
        char *buf;    // 暂存残缺的数据 看情况动态申请
                   // 感觉大多数情况用不到，所有就不提前申请内存了
        int bufSize; // 数据大小
    };

    // 一个conn有一个 用于管理buf
    struct bufInfo
    {
        bufInfo(Multiplexer *m, Asynclo *c)
            : mul(m)
            , conn(c)
        {
            buf = (char *) calloc(MAXLINE, sizeof(char));
            incompletebuf = new struct incompleteBuf();
        }
        ~bufInfo()
        {
            free(buf);
            delete incompletebuf;
        }

        Multiplexer *mul;
        Asynclo *conn;
        char *buf;
        size_t len;
        struct incompleteBuf
            *incompletebuf; // 暂存未完整接收的数据 用完之后记得清空
    };

    // 里面存的是conn和id和DTPClient*的对应关系
    std::map<Asynclo *, std::map<int, DTPClient *> *> connMp_;

  public:
    Multiplexer(EventLoop *loop);
    ~Multiplexer();

    // 在loop工作队列中加入insert任务
    void registEntity(Asynclo *conn, int id, Entity *entity, DTPClient *client);

    // 在loop工作队列中加入remove任务
    void addRemoveWork(Asynclo *conn, int id);

    // 插入id对应的回调函数
    static void insert(void *pa);

    // 删除id对应的回调函数
    static void remove(void *pa);

    // // 根据id查找回调函数
    // Callback findById(int id);

    // 向上层提供发送消息接口
    void sendMessage(Asynclo *conn, int id, std::string *sprotobuf);

    // 发送完消息的回调
    static void sendMsgCallback(void *buffer);

    // 分发消息给通信实体
    static void dispatch(void *pthis);

    // 错误处理函数
    static void handleErro(void *a, int b)
    {
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_MULTIPLEXER, "ERROR!!!\n");
    }

    // 封装一层agreeRecvFile
    void agreeRecvFile(mnet::ControlMsg *msg, DTPClient *client);
	void agreeRecvData(mnet::ControlMsg *msg, DTPClient *client);

	// 封装senddata， flag表示基于TCP,UDP,RDMA发送数据
	void sendData(TcpConnection *conn, int peerId, const char *sendBuf, int length, char *dataId, int flag);

    // 封装一层sendfile
    void sendFile(TcpConnection *conn, int peerId, char *filePath, int length);

    //
    void recvFile(TcpConnection *conn,int peerId,char *filename);
    
    // Message id = 0 时，处理的函数
    void solve(Multiplexer *mul, Asynclo *conn, char *buf);
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_MULTIPLEXER_H__
