////
// @file Log.cc
// @brief
// 实现日志
//
// @author zhangsiqi
// @email 1575033031@qq.com
//
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <stdarg.h>
#include <ndsl/utils/Log.h>
#include <ndsl/utils/TimeStamp.h>

////
// @biref
// 文件
//

class Filelog
{
  private:
    int m_file;

  public:
    Filelog()
        : m_file(-1)
    {
        // init();
    }
    ~Filelog()
    {
        if (m_file != -1) ::close(m_file);
    }
    void init()
    {
        char path[256];
        ndsl::utils::TimeStamp ts;
        ts.now();
        ts.to_string(path, 256);
        int len = ::strlen(path);
        len = len - 7;
        sprintf(path + len, ".log");

        m_file = ::open(
            path,                        // 路径
            O_RDWR | O_APPEND | O_CREAT, // 追加数据
            0666);                       // 其它进程可读
        if (m_file == -1)
            printf("unable to open %s, error = %d\n", path, errno);
    }
    void log(const char *data, size_t size)
    {
        int n = ::write(m_file, data, size);
        // printf("write = %d\n", n);
        if (n < 0) { printf("write error\n"); }
    }
};

////
// @brief
// 全局logger
//
static Filelog file_log;
static int log_source_tag = 35;
static int tag = -1;
static uint64_t _sinks = 0;

void set_ndsl_log_sinks(uint64_t sinks, int file_or_ter) // file = 1, ter = 0
{
    _sinks = sinks;
    if (file_or_ter == 1) {
        tag = 1;
        file_log.init();
    } else {
        tag = 0;
    }
}

uint64_t add_source()
{
    uint64_t module = 1UL << log_source_tag;
    // printf("module = %lu\n",module);
    log_source_tag++;
    return module;
}

void ndsl_log_into_sink(
    int level,
    uint64_t source,
    const char *file_name,
    const char *func_name,
    const char *format,
    ...)
{
    if (source & _sinks) {
        ndsl::utils::TimeStamp ts;
        char buffer[4096] = {0};

        ts.now();
        buffer[0] = '[';
        ts.to_string(buffer + 1, 4096);
        int ret1 = ::strlen(buffer);

        buffer[ret1] = ']';

        const char *ptr = strrchr(file_name, '/');

        int ret2 = sprintf(
            buffer + ret1 + 1,
            " lv=%d pid=%d tid=%lx  %s  %s ",
            level,
            ::getpid(),
            (long) ::pthread_self(),
            // file_name,
            ptr + 1,
            func_name); // 毫秒

        // 复制数据
        va_list ap;
        va_start(ap, format);
        int ret3 = vsnprintf(
            buffer + ret1 + ret2 + 1, 512 - ret1 - ret2 - 1, format, ap);
        if (ret3 < 0) return;

        if ((source & _sinks) && (tag == 0)) printf("%s\n", buffer);

        if ((source & _sinks) && (tag == 1)) {
            buffer[ret1 + ret2 + ret3 + 1] = '\n';
            file_log.log(buffer, ret1 + ret2 + ret3 + 2);
        }
    }
}