#include <iostream>

#include "FtpServer.h"

int main (int argc, char const* argv[]){
	set_ndsl_log_sinks(LOG_SOURCE_ALL,LOG_OUTPUT_FILE);
	FtpServer fs;
	fs.start();
	return 0;
}
