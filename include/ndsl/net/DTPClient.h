/*
 * @file DTPClient.h
 * @brief
 *  用来解析MUL发上来的消息，判断其内容，并分别调用Entity不同的CallBack
 * @author gyz
 * @email mni_gyz@163.com
 */
#ifndef __NDSL_NET_DTPClient_H__
#define __NDSL_NET_DTPClient_H__
#include "Entity.h"
#include "Multiplexer.h"
#include "DTP.h"
#include "src/ndsl/ControlMsg.pb.h"
#include <unordered_map>

namespace ndsl {
namespace net {

class Entity;
class Multiplexer;
class Asynclo;

class DTPClient
{
  private:
    Entity *entity_;
    std::unordered_map<std::string, mnet::ControlMsg *> mp_;

    using entitycallback = void (*)(int, void *, void *);

  public:
    Multiplexer *mul_;
    // 用法同Entity.h
    entitycallback noticeCallback_, dataCallback_;
    void *noticeParam_, *dataParam_;
    // 保存buf和dataid
    std::unordered_map<std::string, std::string> dataIdMp_;

  public:
    DTPClient(
        Entity *e,
        Multiplexer *m,
        entitycallback noticecb,
        entitycallback datacb,
        void *noticeParam,
        void *dataParam);
    ~DTPClient();

    // 解析传过来的protobuf，并调用相应的callback
    void analysis(char *protobuf);

    void analysisProtobuf(mnet::ControlMsg *msg);

    void setCallBack();

    void agreeRecvFile(const char *fileName);
    void agreeRecvData(const char *dataId);

    void agreeSendFile(const char *fileName);

    void sendAck(int peerId, Asynclo *conn, int type, char *dataId);

    Asynclo *getEntityConnection();

    // Entity 通过 DTPClient 调用 mul 的 sendFile
    // void sendFile(TcpConnection *conn, int peerId, char *filePath, int
    // length);
};

} // namespace net
} // namespace ndsl

#endif // __NDSL_NET_DTPClient_H__
