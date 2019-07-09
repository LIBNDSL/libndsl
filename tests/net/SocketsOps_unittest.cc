#include <ndsl/net/SocketsOps.h>
// #include "catch.hpp"

#include <ndsl/net/InetAddress.h>

#include <iostream>

using namespace ndsl;
using namespace ndsl::net;

// //InetAddress直接使用的muduo的库
// using namespace muduo;
// using namespace muduo::net;

// int main(){
// 	std::cout<<"aaa..."<<std::endl;
// 	int port = 8992;
// 	muduo::net::InetAddress addr(port, false, false);
// 	//int fd = ::createNonblockingOrDie(addr.family());
// 	// std::cout<<fd<<std::endl;

// 	return 0;
// }

TEST_CASE("SocketsOps_UnitTest"){

	SECTION("createNonblockingOrDie"){
		//
		REQUIRE(sockets::createNonblockingOrDie(AF_INET) > 0);
		InetAddress addr("127.0.0.1", 9999);
		REQUIRE(sockets::createNonblockingOrDie(addr.family()) > 0);
	}
	
}