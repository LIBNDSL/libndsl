/**
 * @file FTPServer.h
 * @brief
 * FTP服务器
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#ifndef __FTPSERVER_H__
#define __FTPSERVER_H__

#include <string>
#include <vector>
#include <map>
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/TcpAcceptor.h"
#include "ndsl/net/ELThreadpool.h"

class Session;
class FTPServer
{
    friend class Session;

  private:
    const bool DEFAULT_pasv_enable = true;
    const bool DEFAULT_port_enable = true;
    const unsigned int DEFAULT_listen_port = 21;
    const unsigned int DEFAULT_max_clients = 2000;
    const unsigned int DEFAULT_max_per_ip = 50;
    const unsigned int DEFAULT_accept_timeout = 60;
    const unsigned int DEFAULT_connect_timeout = 60;
    const unsigned int DEFAULT_idle_session_timeout = 300;
    const unsigned int DEFAULT_data_connection_timeout = 300;
    const unsigned int DEFAULT_local_umask = 077;
    const unsigned int DEFAULT_upload_max_rate = 102400;
    const unsigned int DEFAULT_download_max_rate = 102400;
    const std::string DEFAULT_listen_address = "0.0.0.0";

  private:
    std::map<std::string, bool> confBoolMap_;         // 配置bool的map
    std::map<std::string, unsigned int> confUintMap_; // 配置uint的map
    std::map<std::string, std::string> confStrMap_;   // 配置string的map

    // 临时日志源
    uint64_t tlog_;

    const char *path_; // 配置文件路径
    ndsl::net::EventLoop *loop_;
    ndsl::net::TcpAcceptor *tcpAcceptor_;
    ndsl::net::TcpConnection *conn_; // 记录连接

    ndsl::net::ELThreadpool pool_;

    ndsl::net::EventLoop *nextLoop_; // 记录下一个连接绑定的EventLoop

  public:
    FTPServer(const char *path, ndsl::net::EventLoop *loop);
    ~FTPServer();

    // 开始运行
    int start();

  private:
    // 解析配置文件
    void parseConfFile();
    // 建立命令连接
    static void onCmdConnection(void *pThis);

    // 命令错误的回调函数
    static void onCmdError(void *Sess, int errnum);

  private:
    void trimSpace(std::string &str);      // 删除字符串中的空格
    int parseSetting(std::string setting); // 设置选项

    void showMap(); // 输出map的值,测试用
};

#endif // __FTPSERVER_H__