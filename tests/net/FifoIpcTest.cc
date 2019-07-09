#define CATCH_CONFIG_MAIN
#include "../catch.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <iostream>
#include "FifoIpc.h"
#define namepth1 "/mnt/unixsocket/pipe1"
#define namepth2 "/mnt/unixsocket/pipe2"

using namespace ndsl::net;
using namespace std;

TEST_CASE("FifoIpc")
{
	SECTION("fifo")
	{
		FifoIpc fi;
		int fd[2];
		char buf[1024];
		int childpid;

		cout<<"create and open begin"<<endl;
	 // 	REQUIRE(fi.createAndOpen(namepth1, namepth2, fd, FILE_MODE) == 1);
		REQUIRE(fi.createFifo(namepth1, namepth2, FILE_MODE) == 1);
		cout<<"create ok"<<endl;
		if ( (childpid = fork()) == 0)
		{
			REQUIRE(fi.openChildFifo(namepth1, namepth2, fd) == 1);
			cout<<"child"<<endl;
			REQUIRE(read(fd[0], buf, 1024) > 0);
			cout<<buf<<endl;
			REQUIRE(strcmp(buf, "I am you father"));
			write(fd[1], "I am your son", sizeof("I am your son"));
			fi.closeAndUnlink(namepth1, namepth2, fd);
			// exit(0);
		}else{
		cout<<"father"<<endl;
		REQUIRE(fi.openFatherFifo(namepth1, namepth2, fd) == 1);
		write(fd[1], "I am your father", sizeof("I am your father"));
		REQUIRE(read(fd[0], buf, 1024) > 0);
		cout<<buf<<endl;
		REQUIRE(strcmp(buf, "I am you son"));
		fi.closeAndUnlink(namepth1, namepth2, fd);
		waitpid(childpid, NULL, 0);
		// exit(0);
		}
	}
}

