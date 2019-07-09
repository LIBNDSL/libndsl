/*
 * @file DTPClient.cc
 * @brief
 *
 * @author gyz
 * @email mni_gyz@163.com
 */
#include "ndsl/net/DTPClient.h"
#include "ndsl/net/DTP.h"
#include "ndsl/net/Asynclo.h"
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include "ndsl/net/Asynclo.h"

namespace ndsl {
namespace net {
using namespace std;

DTPClient::DTPClient(
    Entity *e,
    Multiplexer *m,
    entitycallback noticecb,
    entitycallback datacb,
    void *noticeParam,
    void *dataParam)
    : entity_(e)
    , mul_(m)
    , noticeCallback_(noticecb)
    , dataCallback_(datacb)
    , noticeParam_(noticeParam)
    , dataParam_(dataParam)
{}

DTPClient::~DTPClient() {}

void DTPClient::setCallBack() {}

Asynclo *DTPClient::getEntityConnection() { return entity_->conn_; }

// 暂时没用到这个函数，不知道以后会不会有用 传过来的就一定是protobuf类型的数据
void DTPClient::analysis(char *protobuf)
{
    // isDTP, flag, id,  ip，      dataport, checksum,          filename, length
    // bool,  int,  int, char[15], int,      unsigned char[16], char 64,  int
    // 解析下报头，看是不是DTP协议，不是的话处理之后传给用户

    mnet::ControlMsg *msg = new mnet::ControlMsg;
    std::string s = protobuf;
    msg->ParseFromString(s);
    analysisProtobuf(msg);
}

void DTPClient::analysisProtobuf(mnet::ControlMsg *msg)
{
    int flag = msg->flag();
    if (flag == SENDDATAFLAG) {
        int length = msg->length();
        noticeParam_ = (void *) (&length);
    }
    if (flag == RECVDATAONUDP) { // opposite end agree to recv data on udp
        SocketAddress4 addr(msg->ip().c_str(), msg->dataport());

        // find the udpinfo with dataid
        DTP::UdpInfo *info;
        std::unordered_map<string, DTP::UdpInfo *>::iterator iter =
            mul_->dtp_->udpDataMp_.find(msg->dataid());
        if (iter != mul_->dtp_->udpDataMp_.end()) { info = iter->second; }

        mul_->dtp_->sendDataOnUdp(addr, info);
        return; // don't need user to judge,
    }

    // 接收文件的话 传给用户的只有type, fileName
    // 接受数据的话  也复用filename这一参数，传一个guid进来
    char *filename = NULL;
    if (noticeCallback_ != NULL) {
        if (flag == SENDFILEFLAG) {
            mp_.insert(std::make_pair(msg->filename(), msg));
            filename = (char *) calloc(FILEPATHSIZE, sizeof(char));
            strcpy(filename, msg->filename().c_str());
        }
        if (flag == SENDDATAFLAG || flag == RECVDATASUCC ||
            flag == RECVDATAFAIL || flag == OPPENDRECVSUCC ||
            flag == OPPENDRECVFAIL || flag == SENDDATAONUDP) {
            mp_.insert(std::make_pair(msg->dataid(), msg));
            filename = (char *) calloc(FILEPATHSIZE, sizeof(char));
            strcpy(filename, msg->dataid().c_str());
        }
        noticeCallback_(msg->flag(), filename, noticeParam_);

        free(filename);
    } else
        printf("noticeCallback == NULL\n");
}

void DTPClient::agreeRecvFile(const char *fileName)
{
    std::string s = fileName;
    std::unordered_map<std::string, mnet::ControlMsg *>::iterator iter =
        mp_.find(s);
    if (iter != mp_.end()) {
        // 找到
        mnet::ControlMsg msg = *(iter->second);
        mul_->agreeRecvFile(iter->second, this);

        // 把这条记录删掉
        mp_.erase(s);
    } else {
        // 没找到
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DTPCLIENT, "没找到对应的protobuf");
    }
}

void DTPClient::agreeRecvData(const char *dataId)
{
    std::string s = dataId;
    std::unordered_map<std::string, mnet::ControlMsg *>::iterator iter =
        mp_.find(s);
    if (iter != mp_.end()) {
        // 找到
        mul_->agreeRecvData(iter->second, this);

        // 把这条记录删掉
        mp_.erase(s);
    } else {
        // 没找到
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_DTPCLIENT, "find no matched protobuf");
    }
}

void DTPClient::sendAck(int peerId, Asynclo *conn, int type, char *dataId)
{
    if (type == RECVDATASUCC) { type = OPPENDRECVSUCC; }
    if (type == RECVDATAFAIL) { type = OPPENDRECVFAIL; }
    if (type == RECVRDMADATAFAIL) { type = OPPENDRECVRDMAFAIL; }

    mnet::ControlMsg *msg =
        mul_->dtp_->makeDataControlMsg(type, peerId, NULL, 0, 0, 0, dataId);
    std::string *s = new std::string;
    (*msg).SerializeToString(s);

    delete msg;
    mul_->sendMessage(conn, 0, s);
}
} // namespace net
} // namespace ndsl
