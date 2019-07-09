#define CATCH_CONFIG_MAIN
#include <sys/socket.h>
#include "catch.hpp"
#include "SocketAddressUn.h"

using namespace ndsl::net;
using namespace std;

TEST_CASE( "rxj", "test_unixsocket")
{
	SocketAddressUn a;
	REQUIRE(a.sun_family == AF_LOCAL);
	REQUIRE( a.sun_path[0] == a.sun_path[5] );
	cout<< "path" << a.sun_path <<endl;
	REQUIRE( a.sun_path[5] == 0 );
	string ss = "/ranxiangjun/mnt/xxx";
	SocketAddressUn b(ss);
	REQUIRE( strcmp( b.sun_path, ss.c_str() ) == 0 );
	SocketAddressUn c( b );
	cout<< c.sun_path <<endl;
	REQUIRE( strcmp( c.sun_path, ss.c_str() ) == 0 );
	a = b;
	REQUIRE( strcmp( a.sun_path, ss.c_str() ) == 0 );
	string sss = "/usr/ranxianshen/home";
	a.setAddress( sss );
	REQUIRE( strcmp( a.sun_path, sss.c_str() ) == 0 );
	REQUIRE( strcmp( ( a.getStrPath ).c_str(), sss.c_str() ) == 0 );



}
