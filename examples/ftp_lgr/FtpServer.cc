/**
 * @file FtpServer.cc
 * @brief
 * FTP服务器
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include "ndsl/utils/Log.h"
#include "ndsl/utils/Error.h"

int main(int argc, char const *argv[])
{
    set_ndsl_log_sinks(LOG_SOURCE_ALL, LOG_OUTPUT_TER);
    LOG(LOG_INFO_LEVEL, LOG_SOURCE_THREADPOOL, "This is FTP server!");
    return 0;
}
