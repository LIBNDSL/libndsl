/**
 * @file Session.cc
 * @brief
 * FTP会话
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include <pwd.h>
#include <fcntl.h>
#include <shadow.h>
#include <crypt.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/time.h>
#include <errno.h>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>

#include "ndsl/net/Channel.h"
#include "ndsl/net/SocketAddress.h"
#include "ndsl/utils/Log.h"

#include "Session.h"
#include "FTPServer.h"
#include "FTPCodes.h"

// 测试
#include <iostream>

using namespace std;
using namespace ndsl::net;

Session::Session(
    ndsl::net::EventLoop *loop,
    ndsl::net::TcpConnection *cmdConn,
    FTPServer *ftp)
    : type_(-1)
    , recvlen_(0)
    , fileFd_(-1)
    , cmdLen_(0)
    , remoteDataAddr_(NULL)
    , dataConn_(NULL)
    , cmdConn_(cmdConn)
    , loop_(loop)
    , ftp_(ftp)
    , tlog_(ftp->tlog_)
{
    cmdConn_->onRecv(cmdBuf_, &cmdLen_, 0, onCmdMessage, this);
    cmdConn->onError(ftp_->onCmdError, this);

    ftpReply(FTP_GREET, "(ftp_lgr 0.1)");
}

Session::~Session()
{
    if (remoteDataAddr_ != NULL) delete remoteDataAddr_;
    if (dataConn_ != NULL) delete dataConn_;
    if (cmdConn_ != NULL) {
        // 从EventLoop中删除channel.FIXME:这样太麻烦,需要更友好的方式
        cmdConn_->pTcpChannel_->erase();
        // 关闭文件描述符
        close(cmdConn_->pTcpChannel_->getFd());
        delete cmdConn_;
    }
}

// 收到命令的回调函数
void Session::onCmdMessage(void *pThis)
{
    Session *sess = reinterpret_cast<Session *>(pThis);
    sess->trimCrlf(sess->cmdBuf_);
    // 清空参数
    memset(sess->cmd_, 0, sizeof(sess->cmd_));
    memset(sess->arg_, 0, sizeof(sess->arg_));

    LOG(LOG_INFO_LEVEL, sess->ftp_->tlog_, "cmdLine = [%s]", sess->cmdBuf_);

    sess->parseMessage(sess->cmdBuf_, sess->cmd_, sess->arg_, ' ');

    LOG(LOG_INFO_LEVEL,
        sess->ftp_->tlog_,
        "cmd = [%s] arg = [%s]",
        sess->cmd_,
        sess->arg_);

    // 根据命令字执行相应函数
    {
        auto it = sess->ctrlCmds.begin();
        for (; it != sess->ctrlCmds.end(); ++it) {
            if (strcmp(it->cmd, sess->cmd_) == 0) {
                LOG(LOG_INFO_LEVEL, sess->tlog_, "%s called...", it->cmd);
                (sess->*it->handler)(); // 调用相应的函数
                break;
            }
        }
        // 若没有找到,则输出提示信息
        if (it == sess->ctrlCmds.end()) {
            cout << sess->cmd_ << ":command not found!" << endl;
            sess->ftpReply(FTP_BADCMD, "Unknown command.");
        }
    }

    // 清空缓冲区数据
    memset(sess->cmdBuf_, 0, sizeof(sess->cmdBuf_));

    // 循环注册onRecv
    sess->cmdConn_->onRecv(
        sess->cmdBuf_, &sess->cmdLen_, 0, onCmdMessage, sess);
}

void Session::ftpReply(int status, const char *text)
{
    if (cmdConn_ == NULL) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "cmdConn_ == NULL");
        return;
    }

    char buf[1024];
    sprintf(buf, "%d %s\r\n", status, text);

    cmdConn_->onSend(buf, strlen(buf), 0, NULL, NULL);
}

// 验证用户名
void Session::doUser()
{
    struct passwd *pw = getpwnam(arg_);

    if (pw == NULL) {
        ftpReply(FTP_LOGINERR, "Login incorrect.");
        return;
    }

    uid_ = pw->pw_uid;
    ftpReply(FTP_GIVEPWORD, "Please specify the password.");
}

// 验证密码
void Session::doPass()
{
    struct passwd *pw = getpwuid(uid_);
    if (pw == NULL) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "uid(%u):uid not found!", uid_);
        ftpReply(FTP_LOGINERR, "Login incorrect.");
        return;
    }

    struct spwd *sp = getspnam(pw->pw_name);
    if (sp == NULL) {
        LOG(LOG_ERROR_LEVEL,
            ftp_->tlog_,
            "name(%s):name not found!",
            pw->pw_name);
        ftpReply(FTP_LOGINERR, "Login incorrect.");
        return;
    }

    // 验证密码
    char *encryptedPass = crypt(arg_, sp->sp_pwdp);
    if (strcmp(encryptedPass, sp->sp_pwdp) != 0) {
        ftpReply(FTP_LOGINERR, "Login incorrect.");
        return;
    }

    // FIXME:sigurg??

    // 设置权限
    unsigned int localMask = ftp_->confUintMap_["local_mask"];
    umask(localMask);
    setegid(pw->pw_gid);
    seteuid(pw->pw_uid);
    chdir(pw->pw_dir);

    ftpReply(FTP_LOGINOK, "Login successful.");
}

// 返回系统类型
void Session::doSyst() { ftpReply(FTP_SYSTOK, "UNIX Type: L8"); }

// 退出
void Session::doQuit() { ftpReply(FTP_GOODBYE, "Goodbye."); }

// 记录IP地址和端口信息
void Session::doPort()
{
    // PORT 127,0,0,1,123,233
    if (strlen(arg_) < 11) // 0,0,0,0,0,0
    {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "error: argument too short!");
        return;
    }

    char ip[20];
    unsigned int port;

    istringstream is(arg_);
    string token;
    unsigned int utoken;
    stringstream ss;
    unsigned int vec[6];

    int i = 0;
    for (; getline(is, token, ',') && i < 6; i++) {
        // string转uint
        ss.clear();
        ss << token;
        ss >> utoken;

        vec[i] = utoken;
    }
    if (i != 6) {
        LOG(LOG_ERROR_LEVEL,
            ftp_->tlog_,
            "error: incorrect number of arugments!");
        return;
    }

    ss.clear();
    // 设置IP地址
    ss << vec[0] << "." << vec[1] << "." << vec[2] << "." << vec[3];
    ss >> ip;

    port = vec[4] * 256 + vec[5];

    LOG(LOG_INFO_LEVEL, ftp_->tlog_, "ip = %s, port = %d", ip, port);

    remoteDataAddr_ = new SocketAddress4(ip, port);

    ftpReply(FTP_PORTOK, "PORT command successful. Consider using PASV.");
}

// 返回当前目录
void Session::doList()
{
    if (createDataConnection() != S_OK) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Create data connection error.");
        return;
    }
    ftpReply(FTP_DATACONN, "Here comes the directory listing.");

    listDir(true);

    if (destroyDataConnection() != S_OK) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Destroy data connection error.");
        return;
    }

    ftpReply(FTP_TRANSFEROK, "Dierectory send OK.");
}

// 设置文件格式
void Session::doType()
{
    if (strcmp(arg_, "A") == 0) {
        type_ = FTP_TYPE_ASCII;
        ftpReply(FTP_TYPEOK, "Switching to ASCII mode.");
    } else if (strcmp(arg_, "I") == 0) {
        type_ = FTP_TYPE_BIN;
        ftpReply(FTP_TYPEOK, "Switching to Binary mode.");
    } else {
        ftpReply(FTP_BADCMD, "Unrecognised TYPE command.");
    }
}

// 下载文件
void Session::doRetr()
{
    // TODO:断点续传

    // 创建数据连接
    if (createDataConnection() != S_OK) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Create data connection error.");
        return;
    }

    // TODO:文件操作是否需要改为C++版本?
    // 打开文件
    fileFd_ = open(arg_, O_RDONLY);
    if (fileFd_ == -1) {
        ftpReply(FTP_FILEFAIL, "Failed to open file.");
        return;
    }

    // 对文件上锁
    if (lockFile(fileFd_, F_RDLCK) == S_FALSE) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "File read lock failed.");
        ftpReply(FTP_FILEFAIL, "Failed to open file.");
        return;
    }

    struct stat st;
    fstat(fileFd_, &st);
    // 若不是常规文件,则直接返回
    if (!S_ISREG(st.st_mode)) {
        LOG(LOG_WARN_LEVEL, ftp_->tlog_, "File is not an regular file.");
        ftpReply(FTP_FILEFAIL, "Failed to open file.");
        return;
    }

    stringstream ss;
    if (type_ == FTP_TYPE_ASCII) {
        ss << "Opening ASCII mode data connection for " << arg_ << " ("
           << st.st_size << " bytes).";
    } else {
        ss << "Opening BINARY mode data connection for " << arg_ << " ("
           << st.st_size << " bytes).";
    }
    ftpReply(FTP_DATACONN, ss.str().c_str());

    int ret;
    int nbytes;
    // FIXME:若发送失败,缓冲区将被覆盖?  如何解决?
    // 发送数据
    if ((nbytes = read(fileFd_, sendBuf_, sizeof(sendBuf_))) > 0) {
        ret = dataConn_->onSend(sendBuf_, nbytes, 0, downloadFile, this);
        if (ret < 0) { LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Send error."); }
    } else
        downloadFileComplete(nbytes);
}

void Session::doStor()
{
    // FIXME:限速

    // 创建数据连接
    if (createDataConnection() != S_OK) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Create data connection error.");
        return;
    }

    // 创建文件
    fileFd_ = open(arg_, O_CREAT | O_WRONLY, 0666);
    if (fileFd_ == -1) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Create file failed.");
        ftpReply(FTP_UPLOADFAIL, "Could not create file.");
        return;
    }

    // 给文件加写锁
    if (lockFile(fileFd_, F_WRLCK) != S_OK) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "File write lock failed.");
        ftpReply(FTP_UPLOADFAIL, "Could not create file.");
        return;
    }

    // 清空文件并设置偏移量
    ftruncate(fileFd_, 0);
    if (lseek(fileFd_, 0, SEEK_SET) < 0) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Lseek failed.");
        ftpReply(FTP_UPLOADFAIL, "Could not create file.");
        return;
    }

    struct stat st;
    fstat(fileFd_, &st);
    if (!S_ISREG(st.st_mode)) {
        LOG(LOG_WARN_LEVEL, ftp_->tlog_, "File is not an regular file.");
        ftpReply(FTP_UPLOADFAIL, "The file is not an regular file.");
        return;
    }

    ftpReply(FTP_DATACONN, "Ok to send data.");

    dataConn_->onRecv(recvBuf_, &recvlen_, 0, uploadFile, this);
}

void Session::trimCrlf(char *cmd)
{
    if (cmd == NULL) return;
    char *p = &cmd[strlen(cmd) - 1];

    while (*p == '\r' || *p == '\n')
        *p-- = '\0';
}

int Session::parseMessage(char *cmdLine, char *cmd, char *arg, char c)
{
    char *p = strchr(cmdLine, c);
    if (p == NULL) {
        strcpy(cmd, cmdLine);
    } else {
        strncpy(cmd, cmdLine, p - cmdLine);
        strcpy(arg, p + 1);
    }
    return S_OK;
}

// 建立数据连接
int Session::createDataConnection()
{
    if (dataConn_ != NULL) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Data connection in use.");
        return S_FALSE;
    }

    if (remoteDataAddr_ == NULL) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Remote address not set.");
        return S_FALSE;
    }

    // 建立连接
    // 在disconnect中释放
    dataConn_ = remoteData_.onConnect(loop_, false, remoteDataAddr_);
    if (dataConn_ == NULL) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Connect error");
        return S_FALSE;
    }

    dataConn_->onError(onDataError, this);

    return S_OK;
}

// 销毁数据连接
int Session::destroyDataConnection()
{
    if (remoteDataAddr_ == NULL || dataConn_ == NULL) {
        LOG(LOG_ERROR_LEVEL,
            ftp_->tlog_,
            "Remote address or data connection is not exist.");
        return S_FALSE;
    }

    dataConn_->pTcpChannel_->erase();

    remoteData_.disConnect();
    delete remoteDataAddr_;
    remoteDataAddr_ = NULL;

    dataConn_ = NULL;

    return S_OK;
}

void Session::listDir(bool details)
{
    DIR *dir = opendir(".");
    if (dir == NULL) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "opendir error.");
        return;
    }

    if (dataConn_ == NULL) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Data connection not exist.");
    }

    struct dirent *dt;
    struct stat st;

    stringstream ss;

    while ((dt = readdir(dir)) != NULL) {
        if (lstat(dt->d_name, &st) < 0) continue;

        if (dt->d_name[0] == '.') continue;

        memset(sendBuf_, 0, sizeof(sendBuf_));
        ss.str("");
        ss.clear();

        // 文件详细信息
        if (details) {
            char permits[20];
            char date[32];

            // 获取权限
            if (getFilePermits(&st, permits) != S_OK) return;
            // 获取时间
            if (getFileDate(&st, date) != S_OK) return;

            ss << permits << " ";
            ss << setw(3) << st.st_nlink << " ";
            ss << left << setw(8) << st.st_uid << " ";
            ss << left << setw(8) << st.st_gid << " ";
            ss << setw(8) << st.st_size << " ";
            ss << date << " ";

            if (S_ISLNK(st.st_mode)) {
                char tmpbuf[1024] = {0};
                readlink(dt->d_name, tmpbuf, sizeof(tmpbuf));
                ss << dt->d_name << " -> " << tmpbuf << "\r\n";
            } else {
                ss << dt->d_name << "\r\n";
            }

            strcpy(sendBuf_, ss.str().c_str());
        } else {
            ss << dt->d_name << "\r\n";
            strcpy(sendBuf_, ss.str().c_str());
        }

        dataConn_->onSend(sendBuf_, strlen(sendBuf_), 0, NULL, NULL);
    }

    closedir(dir);
}

// 获取文件权限
int Session::getFilePermits(struct stat *st, char *buf)
{
    // 注意传入数组大小
    // if (sizeof(buf) < 11) {
    //     LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Size of buffer is not enough.");
    //     return S_FALSE;
    // }
    char permits[] = "----------";

    mode_t mode = st->st_mode;

    switch (mode & S_IFMT) {
    case S_IFREG:
        break;
    case S_IFDIR:
        permits[0] = 'd';
        break;
    case S_IFLNK:
        permits[0] = 'l';
        break;
    case S_IFIFO:
        permits[0] = 'p';
        break;
    case S_IFSOCK:
        permits[0] = 's';
        break;
    case S_IFCHR:
        permits[0] = 'c';
        break;
    case S_IFBLK:
        permits[0] = 'b';
        break;

    default:
        permits[0] = '?';
        break;
    }

    if (mode & S_IRUSR) permits[1] = 'r';
    if (mode & S_IWUSR) permits[2] = 'w';
    if (mode & S_IXUSR) permits[3] = 'x';
    if (mode & S_IRGRP) permits[4] = 'r';
    if (mode & S_IWGRP) permits[5] = 'w';
    if (mode & S_IXGRP) permits[6] = 'x';
    if (mode & S_IROTH) permits[7] = 'r';
    if (mode & S_IWOTH) permits[8] = 'w';
    if (mode & S_IXOTH) permits[9] = 'x';

    if (mode & S_ISUID) permits[3] = ((permits[3] == 'x') ? 's' : 'S');
    if (mode & S_ISGID) permits[6] = ((permits[6] == 'x') ? 's' : 'S');
    if (mode & S_ISVTX) permits[9] = ((permits[9] == 'x') ? 't' : 'T');

    strcpy(buf, permits);
    return S_OK;
}

// 获取文件时间
int Session::getFileDate(struct stat *st, char *buf)
{
    // 注意传入数组大小
    // if (sizeof(buf) < 13) {
    //     LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Size of buffer is not enough.");
    //     return S_FALSE;
    // }

    const char *dateFormat = "%b %e %H:%M";
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t localTime = tv.tv_sec;
    if (st->st_mtime > localTime ||
        (localTime - st->st_mtime) > 60 * 60 * 24 * 182) {
        dateFormat = "%b %e %Y";
    }
    struct tm *pt = localtime(&st->st_mtime);
    strftime(buf, sizeof(buf), dateFormat, pt);

    return S_OK;
}

// 对文件加锁
int Session::lockFile(int fd, int type)
{
    int ret;
    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = type;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;

    do {
        ret = fcntl(fd, F_SETLKW, &fl);
    } while (ret < 0 && errno == EINTR);

    return (ret < 0) ? S_FALSE : S_OK;
}

// 下载文件
void Session::downloadFile(void *pThis)
{
    Session *sess = static_cast<Session *>(pThis);

    int nbytes = read(sess->fileFd_, sess->sendBuf_, sizeof(sess->sendBuf_));
    if (nbytes > 0) {
        // 循环注册
        int ret = sess->dataConn_->onSend(
            sess->sendBuf_, nbytes, 0, downloadFile, sess);
        if (ret < 0) LOG(LOG_ERROR_LEVEL, sess->ftp_->tlog_, "Send error.");
        return;
    } else
        sess->downloadFileComplete(nbytes);
}

// 下载文件完成
void Session::downloadFileComplete(int nbytes)
{
    // 关闭文件描述符
    close(fileFd_);

    // 断开连接
    if (S_OK != destroyDataConnection()) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Destroy data connection error.");
    }

    // 文件发送完成
    if (nbytes == 0)
        ftpReply(FTP_TRANSFEROK, "Transfer complete.");
    else { // read文件失败
        ftpReply(FTP_BADSENDFILE, "Failure reading from local file.");
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Read file Error.");
    }
}

// 填充文件
void Session::uploadFile(void *psess)
{
    Session *sess = static_cast<Session *>(psess);
    LOG(LOG_DEBUG_LEVEL, sess->ftp_->tlog_, "uploadFile called...");

    int nwritten;
    LOG(LOG_DEBUG_LEVEL, sess->ftp_->tlog_, "recvlen = %d", sess->recvlen_);
    while (sess->recvlen_ > 0) {
        if ((nwritten = write(sess->fileFd_, sess->recvBuf_, sess->recvlen_)) <
            0) {
            if (errno == EINTR) continue;
            LOG(LOG_INFO_LEVEL, sess->tlog_, "write error.");
            return;
        } else if (nwritten == 0)
            continue;

        sess->recvlen_ -= nwritten;
    }

    // 循环注册onRecv
    sess->dataConn_->onRecv(
        sess->recvBuf_, &sess->recvlen_, 0, uploadFile, sess);
}

void Session::uploadFileComplete()
{
    // 断开连接
    if (S_OK != destroyDataConnection()) {
        LOG(LOG_ERROR_LEVEL, ftp_->tlog_, "Destroy data connection error.");
        return;
    }

    // 关闭文件描述符
    close(fileFd_);

    ftpReply(FTP_TRANSFEROK, "Transfer complete.");
}

// 数据连接错误的回调函数
void Session::onDataError(void *param, int errnum)
{
    Session *sess = static_cast<Session *>(param);
    LOG(LOG_ERROR_LEVEL,
        sess->ftp_->tlog_,
        "There is an error! errno = %s",
        strerror(errnum));

    // 文件传输完毕
    if (errnum == EAGAIN && sess->recvlen_ == 0) {
        LOG(LOG_DEBUG_LEVEL, sess->ftp_->tlog_, "Upload file complete.");
        sess->uploadFileComplete();
    }
}