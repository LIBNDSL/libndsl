////
// @file SocketAddressUn.h
// @brief
// fengzhuang unixsocketaddress
//
// @author ranxiangjun
// @email ranxianshen@gmail.com
//
#ifndef _SOCKETADDRESSUN_H_
#define _SOCKETADDRESSUN_H_

#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/un.h>

namespace ndsl{
namespace net{
struct SocketAddressUn:sockaddr_un
{
  public:
	SocketAddressUn();
	explicit SocketAddressUn(const std::string& );
	SocketAddressUn(const SocketAddressUn& );
	~SocketAddressUn();
	
	SocketAddressUn& operator=( const SocketAddressUn& );
	void setAddress( const std::string& );
	void setVirtualAddress( const std::string&);
	void setAddress( const SocketAddressUn& );
	std::string getStrPath();
};
}	// namespace net
}	// namespace ndsl

#endif	// _SOCKETADDRESSUN_H_
