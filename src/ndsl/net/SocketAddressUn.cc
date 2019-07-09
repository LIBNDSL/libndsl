////
// @file SocketAddressUn.cc
// @brief
// unixsocketaddress'shixain
//
// @author ranxiangjun
// @email ranxianshen@gmail.com
//
#include <string.h>
#include <cstring>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "ndsl/net/SocketAddressUn.h"

using namespace std;

namespace ndsl{
namespace net{

SocketAddressUn::SocketAddressUn()
{
	memset( this, 0, sizeof(sockaddr_un) );
	sun_family = AF_LOCAL;
}

SocketAddressUn::SocketAddressUn( const string& path)
{
	string p = path;
	memset( this, 0, sizeof(sockaddr_un) );
	sun_family = AF_LOCAL;
	memcpy( sun_path, p.c_str(), p.length() + 1 );
}

SocketAddressUn::SocketAddressUn( const SocketAddressUn& addr)
{
	sun_family = addr.sun_family;
	memcpy( sun_path, addr.sun_path, strlen(addr.sun_path) );
}

SocketAddressUn::~SocketAddressUn(){}

SocketAddressUn& SocketAddressUn::operator=( const SocketAddressUn& addr )
{
	sun_family = addr.sun_family;
	memcpy( sun_path, addr.sun_path, strlen(addr.sun_path) );

	return *this;
}

void SocketAddressUn::setAddress( const string& path )
{
	string p = path;
	memset( sun_path, 0, sizeof(sun_path) );
	memcpy( sun_path, p.c_str(), p.length() + 1 );
}

void SocketAddressUn::setVirtualAddress( const std::string& path)
{
	string p = path;
	memset( sun_path, 0, sizeof(sun_path) );
	sun_path[0] = '\0';
	memcpy( sun_path + 1, p.c_str(), p.length() + 1 );
}
void SocketAddressUn::setAddress( const SocketAddressUn& addr )
{
	sun_family = addr.sun_family;
	memcpy( sun_path, addr.sun_path, sizeof(addr.sun_path) );
}

string SocketAddressUn::getStrPath()
{
	return sun_path;
}

}	// namespace net
}	// namespace ndsl

