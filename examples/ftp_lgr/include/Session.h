/**
 * @file Session.h
 * @brief
 * FTP会话
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#ifndef __SESSION_H__
#define __SESSION_H__

#include <string>
#include <map>
#include <vector>
#include "ndsl/net/Channel.h"
#include "ndsl/net/EventLoop.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/net/TcpClient.h"
#include "ndsl/net/TcpConnection.h"

#include "FTPServer.h"
#define MAXLINE 16384

// session
class Session
{
  public:
    Session(
        ndsl::net::EventLoop *loop,
        ndsl::net::TcpConnection *cmdConn,
        FTPServer *ftp);
    ~Session();

  private:
    char cmd_[32];   // 命令字
    char arg_[1024]; // 命令参数
    uid_t uid_;      // uid
    int type_;       // 文件格式

    char sendBuf_[MAXLINE]; // 发送缓冲区
    char recvBuf_[MAXLINE]; // 接收缓冲区
    ssize_t recvlen_;       // 接受数据大小
    int fileFd_;            // 打开文件fd

    char cmdBuf_[MAXLINE]; // 命令的缓冲
    ssize_t cmdLen_;       // 接收缓冲长度

    // 主动模式下,记录客户端IP地址和端口,以便建立数据连接
    ndsl::net::SocketAddress4 *remoteDataAddr_;
    ndsl::net::TcpClient remoteData_;    // 用于建立连接
    ndsl::net::TcpConnection *dataConn_; // 记录建立完成的连接
    ndsl::net::TcpConnection *cmdConn_;  // 命令连接

    ndsl::net::EventLoop *loop_; // Session所在的loop

    FTPServer *ftp_; // 访问ftp内部的配置参数
    uint64_t tlog_;  // 日志输出源

  private:
    // 文件类型
    static const int FTP_TYPE_ASCII = 0;
    static const int FTP_TYPE_BIN = 1;

    struct ftpCmd_t
    {
        // 设置handler
        using cmdHandler = void (Session::*)();
        const char *cmd;    // 命令字
        cmdHandler handler; // 命令执行函数指针
    };

    std::vector<ftpCmd_t> ctrlCmds = {{"USER", &Session::doUser},
                                      {"PASS", &Session::doPass},
                                      {"SYST", &Session::doSyst},
                                      {"QUIT", &Session::doQuit},
                                      {"PORT", &Session::doPort},
                                      {"LIST", &Session::doList},
                                      {"TYPE", &Session::doType},
                                      {"RETR", &Session::doRetr},
                                      {"STOR", &Session::doStor}};

  public:
    uint64_t getLogsource() { return tlog_; }

  private:
    // 收到命令的回调函数
    static void onCmdMessage(void *pThis);
    // 给客户端发送消息
    void ftpReply(int status, const char *text);

  private:
    // 验证用户名
    void doUser();
    // 验证密码
    void doPass();
    // 返回系统类型
    void doSyst();
    // 退出
    void doQuit();
    // 记录IP地址和端口信息
    void doPort();
    // 返回当前目录
    void doList();
    // 设置传输文件的格式
    void doType();
    // 下载文件
    void doRetr();
    // 上传文件
    void doStor();

  private:
    void trimCrlf(char *cmdLine); // 删除字符串中的'\r','\n'
    int parseMessage(char *cmdLine, char *cmd, char *arg, char c); // 分离命令

    int createDataConnection();  // 创建数据连接
    int destroyDataConnection(); // 销毁数据连接

    // 发送当前目录
    void listDir(bool details);
    int getFilePermits(struct stat *st, char *buf); // 获取文件权限
    int getFileDate(struct stat *st, char *buf);    // 获取文件时间

    // 对文件加锁,可选择锁的类型
    int lockFile(int fd, int type);

    // 下载文件
    static void downloadFile(void *pThis);
    // 下载文件完成
    void downloadFileComplete(int nbytes);

    // 上传文件
    static void uploadFile(void *pThis);
    void uploadFileComplete(); // 上传文件完成

    // 命令连接错误回调函数
    static void onCmdError(void *pThis, int errnum);
    // 数据连接错误回调函数
    static void onDataError(void *pThis, int errnum);
};

#endif