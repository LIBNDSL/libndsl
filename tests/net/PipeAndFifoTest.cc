#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include "../catch.hpp"
#include "ndsl/net/PipeAndFifo.h"

using namespace ndsl::net;
using namespace std;

bool pfflag = false;
void pffun(void *a) {pfflag = true;}

bool pfflagsend = false;
static void pfsendTest(void *a) {pfflagsend = true;}

bool pfflagerror = false;
static void pfisError(int a, int b){pfflagerror = true;}

bool pfflagrecv = false;
static void pfrecvTest(void *a){pfflagrecv = true;}

TEST_CASE("PipeIpc")
{
	SECTION("pipe")
	{
		EventLoop loop;
		REQUIRE(loop.init() == S_OK);
		
		int fd[2];
		PipeAndFifo *pipe = new PipeAndFifo(&loop);
		REQUIRE(pipe->createPipe(fd) == S_OK);
		
		pipe->onError(pfisError);
		char *sendbuf = (char *)malloc(sizeof(char) * 10);
		strcpy(sendbuf,"hello son\0");
		REQUIRE(pipe->onSend(sendbuf, strlen("hello son"), 0, pfsendTest, NULL) == S_OK);
		
	//		int childpid;
	//	if ((childpid = fork()) == 0)
	//	{
			char buf[20];
			memset(buf, 0, sizeof(buf));
			read(fd[1], buf, 20);
			REQUIRE(strcmp("hello son", buf) == 0);
			// cout<<"in child"<<endl;
			write(fd[1], buf, 20);
	//	}else
	//	{
			sleep(2);
			// loop.quit();
			// REQUIRE(loop.loop(&loop) == S_OK);
			char buf1[20];
			memset(buf1, 0, sizeof(buf));
			if (pipe->onRecv(buf1,20, 0, pfrecvTest, NULL) != S_OK)
			{
			// 	cout<<"recv error"<<strerror(errno)<<endl;
			}else
			//	cout<<"in father recv"<<buf1<<endl;
		// 	printf("%s", buf1);
			REQUIRE(strcmp("hello son", buf1) == 0);
		// cout<<"in father"<<endl;
	//	}
	}
	
}

