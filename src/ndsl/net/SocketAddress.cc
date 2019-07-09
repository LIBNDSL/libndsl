////
// @file SocketAddress.cc
// @brief
// socketaddress' shixian
//
// @author ranxiangjun
// @email ranxianshen@gmail.com
//
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ndsl/net/SocketAddress.h"
#include "ndsl/utils/Error.h"

using namespace std;

namespace ndsl{
namespace net{

const size_t IPV4LEN = 15;   
const size_t IPV6LEN = 39;   
const size_t MAXSOCKADDRLEN = 100;
const char* ANYADDR = "0";
const char* IPV4_ANYADDR = "0.0.0.0"; // 原基类是 "0.0.0"
const char* IPV6_ANYADDR = "::"; // 原基类是 "::1"

SocketAddress4::SocketAddress4()
{
	sin_family = AF_INET;
	sin_port = htons(0);
	sin_addr.s_addr = htonl(INADDR_ANY); 
}

SocketAddress6::SocketAddress6()
{
	sin6_family = AF_INET6;
	sin6_port = htons(0);
	sin6_addr = in6addr_any; 
}

SocketAddress4::SocketAddress4( const char* buf, unsigned short p )
{
    
	sin_family = AF_INET;
	sin_port = htons(p);
    if ( strlen( buf ) > IPV4LEN || strcmp( buf, ANYADDR ) == 0 ) // ip要是大于255了需要处理吗？
    {
        sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        inet_pton(AF_INET, buf, &sin_addr);
    }   
}

SocketAddress6::SocketAddress6( const char* buf, unsigned short p )
{
    
	sin6_family = AF_INET6;
	sin6_port = htons(p);   
	if ( strlen( buf ) > IPV6LEN || strcmp( buf, ANYADDR ) == 0 )    // ip要是大于ffff了需要处理吗？ 
	{
		sin6_addr = in6addr_any; 
	}
	else
	{
		inet_pton(AF_INET6, buf, &sin6_addr);
	}
}

SocketAddress4::~SocketAddress4()
{
}

SocketAddress6::~SocketAddress6()
{
}

void SocketAddress4::setPort( unsigned short p )
{
    sin_port = htons(p);
}

void SocketAddress6::setPort( unsigned short p )
{
    sin6_port = htons(p);
}

void SocketAddress4::setAddress( const char* buf, unsigned short p )
{
    
	sin_family = AF_INET;
	sin_port = htons(p);
    {
        if ( strlen( buf ) <=  IPV4LEN && strcmp( buf, ANYADDR ) != 0 )   
        {
            inet_pton(AF_INET, buf, &sin_addr);
        }
    }
}

void SocketAddress6::setAddress( const char* buf, unsigned short p )
{
    
	sin6_family = AF_INET6;
	sin6_port = htons(p);
	if ( strlen( buf ) <= IPV6LEN && strcmp( buf, ANYADDR ) != 0 )   
	{
		inet_pton(AF_INET6, buf, &sin6_addr);
	}
}

void SocketAddress4::getIP( char* strbuf4 )const
{
	inet_ntop(AF_INET, (void *)&sin_addr, strbuf4, 16); 
}

void SocketAddress6::getIP( char* strbuf4 )const
{
	inet_ntop(AF_INET6, (void *)&sin6_addr, strbuf4, 40); 
}

unsigned short SocketAddress4::getPort( void )const
{
	return ntohs(sin_port);
}

unsigned short SocketAddress6::getPort( void )const
{
	return ntohs(sin6_port);
}
	
void SocketAddress4::convertToString( string& str ) 
{
    char buf[ MAXSOCKADDRLEN ] = { 0 };
    sprintf( buf, "%s %u", inet_ntop(AF_INET, &sin_addr, buf, 
	sizeof(buf)), ntohs(sin_port) );
	str = buf;
}

void SocketAddress6::convertToString( string& str ) 
{
    char buf[ MAXSOCKADDRLEN ] = { 0 };
    sprintf( buf, "%s %u", inet_ntop(AF_INET6, &sin6_addr, buf, 
	sizeof(buf)), ntohs(sin6_port) );
	str = buf;
}

bool SocketAddress4::operator == ( const SocketAddress4& h )const
{
    if ( sin_addr.s_addr == h.sin_addr.s_addr && sin_port == h.sin_port )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool SocketAddress6::operator == ( const SocketAddress6& h )const
{
	char buf1[50];
	char buf2[50];
	getIP(buf1);
	getIP(buf2);
    if (strcmp(buf1, buf2) == 0 && sin6_port == h.sin6_port )
    {
        return true;
    }
    else
    {
        return false;
    }
}

SocketAddress4& SocketAddress4::operator=( const SocketAddress4& h )             
{
    sin_family = h.sin_family;
	sin_addr.s_addr = h.sin_addr.s_addr;
    sin_port = h.sin_port;
    return *this;  
}  
                    
SocketAddress6& SocketAddress6::operator=( const SocketAddress6& h )             
{
    sin6_family = h.sin6_family;
	memcpy(sin6_addr.s6_addr, h.sin6_addr.s6_addr, 16);
    sin6_port = h.sin6_port;
    return *this;  
}
            
sockaddr_in SocketAddress4::getAddr()
{
    struct sockaddr_in addr;
    memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = sin_addr.s_addr;
    addr.sin_port = sin_port;
    return addr;
}

sockaddr_in6 SocketAddress6::getAddr()
{
    struct sockaddr_in6 addr;
    memset( &addr, 0, sizeof( addr ) );
    addr.sin6_family = AF_INET6;
    memcpy(addr.sin6_addr.s6_addr,  sin6_addr.s6_addr, 16);
    addr.sin6_port = sin6_port;
    return addr;
}

int SocketAddress4::getAddr( sockaddr_in& addr )
{
    if (sin_port > 0)
	{
		memset( &addr, 0, sizeof( addr ) );
		addr.sin_family = AF_INET;
		addr.sin_port = htons( sin_port );
		addr.sin_addr.s_addr = sin_addr.s_addr;
		return S_OK;
	}
	else return S_FALSE;
}

int SocketAddress6::getAddr( sockaddr_in6& addr )
{
    if (sin6_port > 0)
	{
		memset( &addr, 0, sizeof( addr ) );
		addr.sin6_family = AF_INET6;
		addr.sin6_port = htons( sin6_port );
		memcpy(addr.sin6_addr.s6_addr, sin6_addr.s6_addr, 16);
		return S_OK;
	}
	else return S_FALSE;
}

}	// namespace net
}	// namespace ndsl
