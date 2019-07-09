#include "ndsl/utils/Log.h"
#include "Connection.h" 

using namespace ndsl::ftp;

int main()
{
    set_ndsl_log_sinks(LOG_SOURCE_ALL, LOG_OUTPUT_FILE);
    Connection ftpconn;
    ftpconn.ftpServerInit();
    ftpconn.contrloop_.loop(&(ftpconn.contrloop_));
} 
