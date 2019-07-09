/**
 * @file FTPServer.cc
 * @brief
 * FTP服务器
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include "FTPServer.h"
#include "FTPCodes.h"
#include "ndsl/net/Channel.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/TcpConnection.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/utils/Log.h"
#include "ndsl/utils/Error.h"

#include "Session.h"

using namespace std;
using namespace ndsl::net;

FTPServer::FTPServer(const char *path, ndsl::net::EventLoop *loop)
    : path_(path)
    , loop_(loop)
    , tcpAcceptor_(NULL)
{
    // 设置默认配置
    confBoolMap_.insert(pair<string, bool>("pasv_enable", DEFAULT_pasv_enable));
    confBoolMap_.insert(pair<string, bool>("port_enable", DEFAULT_port_enable));

    confUintMap_.insert(
        pair<string, unsigned int>("listen_port", DEFAULT_listen_port));
    confUintMap_.insert(
        pair<string, unsigned int>("max_clients", DEFAULT_max_clients));
    confUintMap_.insert(
        pair<string, unsigned int>("max_per_ip", DEFAULT_max_per_ip));
    confUintMap_.insert(
        pair<string, unsigned int>("accept_timeout", DEFAULT_accept_timeout));
    confUintMap_.insert(
        pair<string, unsigned int>("connect_timeout", DEFAULT_connect_timeout));
    confUintMap_.insert(pair<string, unsigned int>(
        "idle_session_timeout", DEFAULT_idle_session_timeout));
    confUintMap_.insert(pair<string, unsigned int>(
        "data_connection_timeout", DEFAULT_connect_timeout));
    confUintMap_.insert(
        pair<string, unsigned int>("local_umask", DEFAULT_local_umask));
    confUintMap_.insert(
        pair<string, unsigned int>("upload_max_rate", DEFAULT_upload_max_rate));
    confUintMap_.insert(pair<string, unsigned int>(
        "download_max_rate", DEFAULT_download_max_rate));

    confStrMap_.insert(
        pair<string, string>("listen_address", DEFAULT_listen_address));

    // 创建tcpAcceptor
    tcpAcceptor_ = new TcpAcceptor(loop_);

    // 增加log输出源
    tlog_ = add_source();
}

FTPServer::~FTPServer()
{
    if (NULL != tcpAcceptor_) delete tcpAcceptor_;
}

int FTPServer::start()
{
    // 解析配置文件
    parseConfFile();

    // 设置监听套接字
    {
        string listenAddr = confStrMap_.at("listen_address");
        unsigned int listenPort = confUintMap_.at("listen_port");

        LOG(LOG_INFO_LEVEL, tlog_, "listen address = %s", listenAddr.c_str());
        LOG(LOG_INFO_LEVEL, tlog_, "listen port = %u", listenPort);
        struct SocketAddress4 servaddr(listenAddr.c_str(), listenPort);

        tcpAcceptor_->start(servaddr);
    }

    // 启动线程池
    pool_.start();
    // 获取下一个Eventloop
    nextLoop_ = pool_.getNextEventLoop();

    {
        // 在onCmdError中delete
        conn_ = new TcpConnection();
        tcpAcceptor_->onAccept(
            conn_, NULL, NULL, onCmdConnection, this, nextLoop_);
    }

    LOG(LOG_INFO_LEVEL, tlog_, "start end!");

    return S_OK;
}

// 建立命令连接
void FTPServer::onCmdConnection(void *pThis)
{
    FTPServer *ftp = static_cast<FTPServer *>(pThis);

    // 使用多线程,但存在问题.
    // FIXME:进程改变euid,egid在整个进程中生效,多用户使用存在问题
    new Session(ftp->nextLoop_, ftp->conn_, ftp);

    // 更新nextLoop_
    ftp->nextLoop_ = ftp->pool_.getNextEventLoop();
    {
        // 在onCmdError中delete
        ftp->conn_ = new TcpConnection();
        ftp->tcpAcceptor_->onAccept(
            ftp->conn_, NULL, NULL, onCmdConnection, ftp, ftp->nextLoop_);
    }
}

// 命令错误的回调函数
void FTPServer::onCmdError(void *Sess, int errnum)
{
    Session *sess = static_cast<Session *>(Sess);
    if (errnum == EAGAIN) {
        LOG(LOG_INFO_LEVEL, sess->getLogsource(), "Connection closed by peer.");
        delete sess;
    } else {
        LOG(LOG_ERROR_LEVEL, sess->getLogsource(), "Unknown error.");
    }
}

// 删除字符串中的空格
void FTPServer::trimSpace(string &str)
{
    for (auto it = str.begin(); it != str.end();) {
        if ((*it) == ' ')
            str.erase(it);
        else
            ++it;
    }
}

// 分离配置文件中的key和value,并进行设置
int FTPServer::parseSetting(string setting)
{
    string key, value;
    size_t pos = setting.find('=');
    key = setting.substr(0, pos);
    value = setting.substr(pos + 1, setting.size());

    if (confBoolMap_.count(key)) {
        bool bv = value == "YES" ? 1 : 0;
        confBoolMap_[key] = bv;
        return S_OK;
    }

    if (confUintMap_.count(key)) {
        unsigned int uv = (unsigned int) stoi(value);
        confUintMap_[key] = uv;
        return S_OK;
    }

    if (confStrMap_.count(key)) {
        confStrMap_[key] = value;
        return S_OK;
    }

    return S_FALSE;
}

// 解析配置文件
void FTPServer::parseConfFile()
{
    ifstream conf(path_);

    // 若文件打开失败,则直接返回s
    if (!conf.is_open()) {
        cout << "there is no configure file." << endl;
        return;
    }

    string oneline;

    while (getline(conf, oneline)) {
        // 删除空格
        trimSpace(oneline);
        if (oneline[0] == '#') continue;
        parseSetting(oneline);
        // cout << oneline << endl;
    }

    // 打印输出map的值,测试用
    // showMap();

    conf.close();
}

void FTPServer::showMap()
{
    for (auto it = confBoolMap_.begin(); it != confBoolMap_.end(); ++it) {
        cout << it->first << " = " << it->second << endl;
    }

    for (auto it = confUintMap_.begin(); it != confUintMap_.end(); ++it) {
        cout << it->first << " = " << it->second << endl;
    }

    for (auto it = confStrMap_.begin(); it != confStrMap_.end(); ++it) {
        cout << it->first << " = " << it->second << endl;
    }
}
