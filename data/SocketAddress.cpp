/**
*	TcpServer Socket library
*
*	Copyright (c) 2000 Jasen Plietz
*	All rights reserved.
*
*	No warranties are extended. Use at your own risk.
*
*	To contact the author with suggestions or comments, use Jasen@plietz.net
*
*/


// SocketAddress.cpp -- implementation of the SocketAddress class

#include <string.h>

#include "SocketAddress.h"
#include "IpAddress.h"

/////////////////////////////////////////////////////////////////////////////
// class SocketAddress

std::ostream& SocketAddress::operator<<(std::ostream& stream) const
{
	stream << std::string("SocketAddress( ");
	getIpAddress().operator<<(stream);
	stream << std::string(", port ");
	// stream << getPort(); TODO: get this to wwork under GNU
	stream << std::string(" )");
	return stream;
}

// Set IP address to `address`.
void SocketAddress::setIpAddress( const IpAddress& address )
{
	setIpAddress( address.toLong() );
}


// Assign myself from `address`.
SocketAddress& SocketAddress::operator=( const sockaddr_in& address )
{
	socketAddress = address;
	return *this;
}


// Assign myself from `address`.
SocketAddress& SocketAddress::operator=( const SocketAddress& address )
{
	socketAddress = address.socketAddress;
	return *this;
}


// Return my IP address.
IpAddress SocketAddress::getIpAddress() const
{
	return getIpAddressAsLong();
}


// Return the size, in bytes, of my encoded address.
int SocketAddress::length() const
{
	return sizeof( socketAddress );
}


// Return myself as a `sockaddr_in` structure.
SocketAddress::operator sockaddr_in() const
{
	return socketAddress;
}


// Return `true` if my port is non-zero.
bool SocketAddress::isDefined() const
{
	return getPort() != 0;
}


// Return `true` if my address is IP based.
bool SocketAddress::isIpAddress() const
{
	return socketAddress.sin_family == AF_INET;
}


// Return `true` if my address is path based.
bool SocketAddress::isPathAddress() const
{
	return socketAddress.sin_family == AF_UNIX;
}


// Return my domain.
short SocketAddress::getAddressFamily() const
{
	return socketAddress.sin_family;
}


// Return `true` if my address and port are the same as `address`'s.
bool SocketAddress::operator==( const SocketAddress& address ) const
{
	return ( getPort() == address.getPort() ) && ( getIpAddressAsLong() == address.getIpAddressAsLong() );
}


// Return `true` if I'm considered to be less than `address`.
bool SocketAddress::operator<( const SocketAddress& address ) const
{
	return ( getPort() < address.getPort() ) || ( ( getPort() == address.getPort() ) &&
		( getIpAddressAsLong() < address.getIpAddressAsLong() ));
}


// Construct myself to represent the port `port` on my host.
SocketAddress::SocketAddress( int port )
{
	initialize();
	setPort(port);
	setIpAddress( getIpAddress() );
}


// Construct myself from the encoded IP socket address `address`.
SocketAddress::SocketAddress( const sockaddr_in& address )
{
	socketAddress = address;
}


// Construct myself to reference the port `port=0`) at IP address`address`.
SocketAddress::SocketAddress( const IpAddress& address, int port )
{
	initialize();
	setPort( port );
	setIpAddress( address );
}


// Construct myself to be a copy of `address`.
SocketAddress::SocketAddress( const SocketAddress& address ) : socketAddress( address.socketAddress )
{
}

// Initialize my address by zeroing its data and setting its family
// to `AF_INET`.
void SocketAddress::initialize() // throw()
{
	::memset( &socketAddress, 0, sizeof(socketAddress) );
	socketAddress.sin_family = AF_INET;
}


// Set my port to `port`.
void SocketAddress::setPort( int port ) // throw()
{
	socketAddress.sin_port = htons( (unsigned short) port );
}


// Return my port number.
int SocketAddress::getPort() const // throw()
{
	return ntohs( socketAddress.sin_port );
}


// Return my IP address in encoded form.
long SocketAddress::getIpAddressAsLong() const // throw()
{
	return ntohl( socketAddress.sin_addr.s_addr );
}


// Set my IP address to the IP address encoded by `code`.
void SocketAddress::setIpAddress( long code ) // throw()
{
	socketAddress.sin_addr.s_addr = htonl( code );
}
