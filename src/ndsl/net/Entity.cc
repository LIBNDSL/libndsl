/*
 * @file Entity.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#include "ndsl/net/Entity.h"
#include "ndsl/net/Multiplexer.h"
#include "ndsl/net/DTPClient.h"

namespace ndsl {
namespace net {

// 开始的时候需要指定一块存数据buf的区域，暂定为通过noticeParam传入
Entity::Entity(
    Asynclo *conn,
    int id,
    Multiplexer *Multiplexer,
    entitycallback noticecb,
    entitycallback datacb,
    void *noticeParam,
    void *dataParam)
    : conn_(conn)
    , id_(id)
    , multiplexer_(Multiplexer)
    , noticeCallback_(noticecb)
    , dataCallback_(datacb)
    , noticeParam_(noticeParam)
    , dataParam_(dataParam)
{}

Entity::~Entity() { multiplexer_->addRemoveWork(conn_, id_); }

void Entity::start(startcallback cb, void *startcbparam)
{
    scb_ = cb;
    startcbparam_ = startcbparam;
    start1();
}

void Entity::start()
{
    scb_ = NULL;
    start1();
}

void Entity::start1()
{
    // 注册DTPClient
    client_ = new DTPClient(
        this,
        multiplexer_,
        noticeCallback_,
        dataCallback_,
        noticeParam_,
        dataParam_);
    multiplexer_->registEntity(conn_, id_, this, client_);
}

} // namespace net
} // namespace ndsl