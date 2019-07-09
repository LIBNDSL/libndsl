/**
 * @file Httphandler.cc
 * @brief
 * 处理HTTP报文
 * @author zzt
 * @emial 429834658@qq.com
 **/
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "malloc.h"
#include <arpa/inet.h>
#include "Httphandler.h"
#include "ndsl/net/Multiplexer.h"
#include "ndsl/net/Entity.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/TcpChannel.h"
#include "ndsl/net/TcpClient.h"
#include "Protbload.pb.h"

namespace ndsl {
namespace net {
using namespace Protbload;
// 开始代理，作为onAccept函数的回调
void Httphandler::beginProxy(void *para)
{
    struct hpara *p = (struct hpara *) para;
    char *recvbuf =
        (char *) malloc(sizeof(char) * MAXLINE); // 在disposeClientMsg函数中释放

    p->clientbuf = recvbuf;
    p->readlen = 0;
    p->con2c->onRecv(recvbuf, &(p->readlen), 0, disposeClientMsg, (void *) p);
    p->con2c->onError(handleErro, NULL);

    struct hpara *hp = new struct hpara(); // 在disposeClientMsg函数中释放
    hp->multi2s = p->multi2s;
    struct sockaddr_in rservaddr;
    bzero(&rservaddr, sizeof(rservaddr));
    socklen_t addrlen;
    TcpConnection *conn2c = new TcpConnection(); // TODO: 内存释放
    hp->con2c = conn2c;
    // printf("con2c = %p\n", conn2c);
    hp->map = Httphandler::getMap();
    hp->tAc = p->tAc;
    printf("1111111111111111\n");

    hp->tAc->onAccept(
        conn2c,
        (SA *) &rservaddr,
        &addrlen,
        Httphandler::beginProxy,
        (void *) hp);

    printf("@222222222222222222222\n");
}

// 解析从浏览器端发来的http报文
void Httphandler::disposeClientMsg(void *para)
{
    // printf("int the disposeclientMsg fun\n");
    struct hpara *hp = reinterpret_cast<struct hpara *>(para);
    LOG(LOG_INFO_LEVEL, LOG_SOURCE_ENTITY, "\nClientMsg = \n%s", hp->clientbuf);
    LOG(LOG_INFO_LEVEL, LOG_SOURCE_ENTITY, "rlen = %lu\n", hp->readlen);

    if (strstr(hp->clientbuf, "Referer") != NULL) {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_ENTITY, "丢弃referer消息\n");
        return;
    }
    if (strstr(hp->clientbuf, "num1=") == NULL) {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_ENTITY, "丢弃无法处理消息\n");
        return;
    }
    // 得到第一个参数
    char *FirstLocation = strstr(hp->clientbuf, "num1=") + strlen("num1=");
    char *LastLocation = strstr(FirstLocation, "&");
    char num[4];
    memset(num, 0, 4);
    memcpy(num, FirstLocation, LastLocation - FirstLocation);
    int a = atoi(num);
    LOG(LOG_INFO_LEVEL, LOG_SOURCE_ENTITY, "num1 = %d\n", a);

    // 得到第二个参数
    FirstLocation = strstr(FirstLocation, "num2=") + strlen("num2=");
    LastLocation = strstr(LastLocation, " ");
    memset(num, 0, 4);
    memcpy(num, FirstLocation, LastLocation - FirstLocation);
    int b = atoi(num);
    LOG(LOG_INFO_LEVEL, LOG_SOURCE_ENTITY, "num2 = %d\n", b);

    if (hp->clientbuf != NULL) {
        delete hp->clientbuf;
        LOG(LOG_INFO_LEVEL,
            LOG_SOURCE_ENTITY,
            "finish read,delete clientbuf\n");
    }

    static int id = 1;

    Protbload::ADD *addmessage =
        new Protbload::ADD; // 在sendMsg2server函数中释放
    addmessage->set_agv1(a);
    addmessage->set_agv2(b);
    addmessage->set_id(id);

    if (hp->multi2s != NULL) {
        Entity *myent =
            new Entity(id, Httphandler::disposeServerMsg, hp->multi2s);
        std::pair<Protbload::ADD *, struct hpara *> Pair;
        myent->start(Httphandler::sendMsg2server, (void *) addmessage);

        std::map<int, TcpConnection *>::iterator iter = hp->map->find(id);
        // printf("hp->enMap = %p\n", hp->map);
        if (iter == hp->map->end()) {
            std::pair<std::map<int, TcpConnection *>::iterator, bool>
                Insert_Pair;
            Insert_Pair = hp->map->insert(std::make_pair(id, hp->con2c));
            if (Insert_Pair.second == 1)
                LOG(LOG_INFO_LEVEL,
                    LOG_SOURCE_ENTITY,
                    "success insert entityMap, id= %d, con2c=%p\n",
                    id,
                    Insert_Pair.first->second);
            else
                LOG(LOG_INFO_LEVEL,
                    LOG_SOURCE_ENTITY,
                    "Failure : insert entityMap\n");
        }
        if (hp != NULL) {
            // delete hp;
            // LOG(LOG_INFO_LEVEL, LOG_SOURCE_ENTITY, "delete hpara\n");
        }
        id++;
    }
}

// 将加法消息发送给服务器
void Httphandler::sendMsg2server(void *para)
{
    printf("in the sendMsg2server function\n");
    Protbload::ADD *addmessage = reinterpret_cast<Protbload::ADD *>(para);
    std::string pstr;
    addmessage->SerializeToString(&pstr);
    int mlen = pstr.size();
    Multiplexer *mul = Httphandler::getMultiplexer();
    printf("before sendMsg to server\n");
    mul->sendMessage(1, mlen, pstr.c_str());
    if (addmessage != NULL) {
        delete addmessage;
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_ENTITY, "delete addmessage\n");
    }
}

// 解析从服务器端发来的消息，然后发送给浏览器
void Httphandler::disposeServerMsg(
    Multiplexer *Multiplexer,
    char *data,
    int len,
    int error)
{
    TcpConnection *conec2c = NULL;
    Protbload::RESULT *resultmessage =
        new Protbload::RESULT; // 在函数末尾释放内存
    resultmessage->ParseFromString(data);
    entityMap *map = Httphandler::getMap();
    std::map<int, TcpConnection *>::iterator iter =
        map->find(resultmessage->id());
    // printf("id = %d\n", resultmessage->id());
    if (iter != map->end()) {
        conec2c = iter->second;
        LOG(LOG_INFO_LEVEL,
            LOG_SOURCE_ENTITY,
            "disposeServerMsg即将发送到con2c = %p\n",
            iter->second);
    } else {
        LOG(LOG_INFO_LEVEL,
            LOG_SOURCE_ENTITY,
            "cannot find the connection of id=%d\n",
            resultmessage->id());
        return;
    }

    std::string s = std::to_string(resultmessage->answer());
    // printf("result==%d \n", resultmessage->answer());
    // conec2c->onSend((void *) (s.c_str()), s.size(), 0, NULL, NULL);
    std::string str = "HTTP/1.0 200 "
                      "OK\r\nContent-type:text/"
                      "plain\r\n";
    s = "the result is=" + s;
    str += "Content-Length: " + std::to_string(s.size()) + "\r\n\r\n";
    str += s;
    // printf("%s\n", str.c_str());
    conec2c->onSend((void *) (str.c_str()), str.size(), 0, NULL, NULL);

    map->erase(iter);
    if (resultmessage != NULL) {
        LOG(LOG_INFO_LEVEL,
            LOG_SOURCE_ENTITY,
            "erase id=%d\n",
            resultmessage->id());
        LOG(LOG_INFO_LEVEL,
            LOG_SOURCE_ENTITY,
            "the size of map is %d\n",
            map->size());
        delete resultmessage;
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_ENTITY, "delete resultmessage\n");
    }
}

// 获取<id，Tcpconnection>类型的map
entityMap *Httphandler::getMap()
{
    static entityMap *map = new entityMap;
    return map;
}

// 获取代理服务器与加法服务器的连接的multiplexer
Multiplexer *Httphandler::getMultiplexer()
{
    static Multiplexer *multi2s = NULL;
    if (multi2s != NULL) {
        return multi2s;
    } else {
        entityMap *map = Httphandler::getMap();
        std::map<int, TcpConnection *>::iterator iter = map->find(0);
        if (iter != map->end()) {
            multi2s = new Multiplexer(iter->second);
            if (multi2s != NULL)
                LOG(LOG_INFO_LEVEL,
                    LOG_SOURCE_MULTIPLEXER,
                    "already build multiplexer to server\n");
            return multi2s;
        } else {
            LOG(LOG_ERROR_LEVEL,
                LOG_SOURCE_MULTIPLEXER,
                "no con2s in the entitymap\n");
            return NULL;
        }
    }
}

/********** FIXME: 现在建立了到服务器的长连接，不需要这样处理了

// 解析从客户端发来的http报文
void Httphandler::disposeClientMsg(void *para)
{
    struct hpara *pa = reinterpret_cast<struct hpara *>(para);
    LOG(LOG_INFO_LEVEL,
        LOG_SOURCE_ENTITY,
        "\nClientMsg = \n%s\n",
        pa->clientbuf);
    LOG(LOG_INFO_LEVEL, LOG_SOURCE_ENTITY, "rlen = %lu\n", pa->readlen);

    // 得到目标服务器的名称
    char *FirstLocation = strstr(pa->clientbuf, HttpHead) +
strlen(HttpHead); char *LastLocation = strstr(FirstLocation, "/"); char
Servername[50]; memset(Servername, 0, 50); memcpy(Servername, FirstLocation,
LastLocation - FirstLocation); LOG(LOG_INFO_LEVEL, LOG_SOURCE_ENTITY,
"Servername = %s\n", Servername);

    // DNS域名转换
    struct hostent *hp = gethostbyname(Servername);

    if (hp == NULL)
        LOG(LOG_ERROR_LEVEL,
            LOG_SOURCE_ENTITY,
            "gethostname error for host%s\n",
            Servername);

    // 将IP地址放入pa
    switch (hp->h_addrtype) {
    case AF_INET: {
        char **iptr = hp->h_addr_list;
        if (*iptr != NULL) {
            LOG(LOG_INFO_LEVEL,
                LOG_SOURCE_ENTITY,
                "\taddress:%s\n",
                inet_ntop(
                    AF_INET,
                    hp->h_addr_list[0],
                    pa->hostip,
                    sizeof(pa->hostip)));
            //将获取的IP地址转为点分十进制后存入pa->hostip
        }
        break;
    }
    default:
        LOG(LOG_ERROR_LEVEL, LOG_SOURCE_ENTITY, "unknown addrtype\n");
        break;
    }

    connectGoalserver(reinterpret_cast<void *>(pa));
}

// 连接目标服务器
// void Httphandler::connectGoalserver(void *para)
// {
//     struct hpara *pa = reinterpret_cast<struct hpara *>(para);

//     struct SocketAddress4 clientservaddr(pa->hostip, 9999);
//     TcpConnection *Conn2s;
//     TcpClient *pCli = new TcpClient();

//     Conn2s = pCli->onConnect(
//         pa->conn2c->pTcpChannel_->pLoop_, false, &clientservaddr);
//     if (Conn2s != NULL) {
//         LOG(LOG_INFO_LEVEL,
//             LOG_SOURCE_ENTITY,
//             "success connect to Goalserver\n");
//         pa->conn2s = Conn2s;
//     } else {
//         LOG(LOG_ERROR_LEVEL, LOG_SOURCE_ENTITY, "cant connect to
Goalserver\n");
//     }

//     Conn2s->onSend(pa->clientbuf, pa->readlen, 0, NULL, NULL);

//     if (pa->serverbuf == NULL) {
//         char *buffer =
//             (char *) malloc(sizeof(char) * MAXLINE); // TODO: 内存释放
//         LOG(LOG_INFO_LEVEL, LOG_SOURCE_ENTITY, "malloc new
serverbuffer\n");
//         pa->serverbuf = buffer;
//     }

//     Conn2s->onRecv(
//         pa->serverbuf,
//         &(pa->readlen),
//         0,
//         disposeServerMsg,
//         reinterpret_cast<void *>(pa));
// }
**********/

} // namespace net
} // namespace ndsl
