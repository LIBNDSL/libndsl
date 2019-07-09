/**
 * @file Entity.h
 * @brief
 * Entity实体类
 *
 * @author zzt
 * @emial 429834658@qq.com
 **/
#ifndef __NDSL_NET_ENTITY_H__
#define __NDSL_NET_ENTITY_H__

#include "Asynclo.h"

namespace ndsl {
namespace net {

class Multiplexer;
class DTPClient;

class Entity
{
  private:
    using startcallback = void (*)(void *);

    // 内部函数，不向外暴露
    void start1();

  public:
    Asynclo *conn_; // 暂时这样
    using entitycallback = void (*)(int, void *, void *);
    int id_; // 实体自身id
    DTPClient *client_;

    startcallback
        scb_; // entity想mul注册成功之后回调的函数 因为是异步，所以需要
    void *startcbparam_; // 回调参数
    Multiplexer *multiplexer_;

    // noticecb 负责处理那8中成功/失败状态 datacb负责处理非dtp数据
    entitycallback noticeCallback_, dataCallback_;
    void *noticeParam_, *dataParam_;

    Entity(
        Asynclo *conn,
        int id,
        Multiplexer *Multiplexer,
        entitycallback noticecb,
        entitycallback datacb,
        void *noticeParam,
        void *dataParam);
    ~Entity();

    // entity成功插入map后执行cb函数，param为参数
    void start(startcallback cb, void *startcbparam);

    // 无参接口
    void start();

};

} // namespace net
} // namespace ndsl
#endif // __NDSL_NET_ENTITY_H__
