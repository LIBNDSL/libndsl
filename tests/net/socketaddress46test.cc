#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "SocketAddress.h"

using namespace ndsl::net;
using namespace std;

TEST_CASE("ebebe", "test socketaddress54"){
	SocketAddress4 a;
	REQUIRE(a.sin_port == 0);
	SocketAddress4 b("192.168.1.121", 555);
	static char buf[20];
	b.getIP(buf);
	a.getIP(buf);
	string str;
	a.convertToString(str);
	REQUIRE(strcmp(str.c_str(), "0.0.0.0 0") == 0);
	REQUIRE(b.sin_port == ntohs(555));
	b.setPort(1111);
	REQUIRE(b.sin_port == ntohs(1111));
	a.setAddress("192.168.1.110", 1111);
	a.getIP(buf);
	REQUIRE(strcmp(buf, "192.168.1.110") == 0);
	REQUIRE(b.getPort() == 1111);
	SocketAddress4 e, f;
	e = f = b;
	REQUIRE(e == b);
	REQUIRE(e.sin_port == ntohs(1111));
	REQUIRE(e.sin_port == f.sin_port);
	REQUIRE((b.getAddr()).sin_port == b.sin_port);
	REQUIRE(b.getAddr(e));
}

TEST_CASE("sdas", "SocketAddress6")
{
	char buf6[50];
	string str6;
	SocketAddress6 c;
	SocketAddress6 d("1111:1111:1111:2222:2222:2222:abcd:abcd", 6666);
	REQUIRE(c.sin6_port == 0);
	REQUIRE(d.sin6_port == ntohs(6666));
	d.setPort(1111);
	REQUIRE(d.sin6_port == ntohs(1111));
	c.setAddress("1:1:1:1:1:1:aaaa:bbbb", 1111);
	c.getIP(buf6);
	REQUIRE(strcmp(buf6, "1:1:1:1:1:1:aaaa:bbbb") == 0);
	REQUIRE(c.getPort() == 1111);
	c.convertToString(str6);
	REQUIRE(strcmp(str6.c_str(), "1:1:1:1:1:1:aaaa:bbbb 1111") == 0);
	SocketAddress6 g, h;
	g = h = c;
	g.getIP(buf6);
	REQUIRE(strcmp(buf6, "1:1:1:1:1:1:aaaa:bbbb") == 0);
	REQUIRE(c == h); // fix it
	REQUIRE(g.sin6_port == ntohs(1111));
	REQUIRE(h.sin6_port == ntohs(1111));
	REQUIRE((d.getAddr()).sin6_port == d.sin6_port);
	REQUIRE(d.getAddr(g));
}
