/**
 * @file FtpServer.cc
 * @brief
 * FTP服务器
 *
 * @author Liu GuangRui
 * @email 675040625@qq.com
 */
#include <iostream>
#include <unistd.h>
#include "ndsl/net/EventLoop.h"
#include "ndsl/utils/Log.h"
#include "ndsl/utils/Error.h"
#include "ndsl/net/SignalHandler.h"
#include "FTPServer.h"

using namespace ndsl::net;
using namespace std;

int main(int argc, char const *argv[])
{
    SignalHandler::blockAllSignals();

    set_ndsl_log_sinks(LOG_SOURCE_ALL, LOG_OUTPUT_FILE);
    uint64_t mlog = add_source();

    EventLoop loop;
    loop.init();

    FTPServer ftp("ftp.conf", &loop);

    ftp.start();

    if (getuid() != 0) {
        LOG(LOG_ERROR_LEVEL, mlog, "FTPServer must be started sa root!");
        return S_FALSE;
    }

    loop.loop(&loop);

    SignalHandler::unBlockAllSignals();

    return 0;
}
