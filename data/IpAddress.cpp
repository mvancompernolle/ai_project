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


// IpAddress.cpp -- implementation for the IpAddress class

#include "IpAddress.h"
#include "Host.h"
#include "NetworkEx.h"


/////////////////////////////////////////////////////////////////////////////
// class IpAddress

// static variable for local IpAddress
std::auto_ptr<IpAddress> IpAddress::localIpAddress;

// Constructor for address assignment
IpAddress::IpAddress( const std::string& address )
{
	char first = address[ 0 ];
	if ( first >= '0' && first <= '9' )
		set( address );
	else
		setHostName( address );
}

// Construct from encoded IpAddress
IpAddress::IpAddress( unsigned long code )
{
	set( code );
}


// Construct from the Berkley Sockets in_addr structure
IpAddress::IpAddress( const in_addr& address ) : ipAddress( address )
{
}


// Copy constructor
IpAddress::IpAddress( const IpAddress& address ) : ipAddress( address.ipAddress )
{
}

// Standard destructor.
IpAddress::~IpAddress()
{
}

// Standard c++ iostream inserter
std::ostream& IpAddress::operator<<( std::ostream& stream) const
{
	stream << "IpAddress( ";
    stream << ( toLong() == INADDR_ANY ? "ANY" : ::inet_ntoa( ipAddress ) );
	stream << " )";
	return stream;
}

// Comparison of IpAddress
bool IpAddress::operator==( const IpAddress& address ) const
{
	return toLong() == address.toLong();
}

// Comparison of IpAddress
bool IpAddress::operator<( const IpAddress& address ) const
{
	return toLong() < address.toLong();
}

// Assignment operator from Berkley Socket structure
IpAddress& IpAddress::operator=(const in_addr& address)
{
	ipAddress = address;
	return *this;
}

// Assignment operator
IpAddress& IpAddress::operator=( const IpAddress& address )
{
	ipAddress = address.ipAddress;
	return *this;
}

// Verify whether IpAddress is the local host
bool IpAddress::isLocal() const
{
	return *this == getLocalIpAddress();
}

// Conversion to the Berkely Sockets in_addr structure
IpAddress::operator in_addr() const
{
	return ipAddress;
}

// Validity check
bool IpAddress::isDefined() const
{
	return ipAddress.s_addr != 0;
}

// Accessor to set the host name
void IpAddress::setHostName( const std::string& hostName )
{
	Host host( hostName );
	if ( host.isDefined() )
		*this = host.getIpAddresses().front();
}

// Accessor to set the IpAddress
void IpAddress::set( const std::string& string )
{
	long addr = ::inet_addr(string.c_str());
	if ( addr == -1 )
		NetworkException::throwNetworkException(NetworkException::invalid_address,string.c_str());

	ipAddress.s_addr = addr;
}

// Static function to return IpAddress for the local host
const IpAddress& IpAddress::getLocalIpAddress()
{
	if ( localIpAddress.get() == 0 ) {
		try {
			localIpAddress = std::auto_ptr<IpAddress>(new IpAddress());
			localIpAddress->set( Host::getLocalhost().getName() );
		}
		catch ( std::exception& )
		{
			// Only occurs with a bad machine configuration
		}
	}

	return *(localIpAddress.get());
}

// Accessor to set the IpAddress
void IpAddress::set( unsigned long code ) throw()
{
	ipAddress.s_addr = htonl( code );
}


// Conversion of IpAddress to a long
unsigned long IpAddress::toLong() const throw()
{
	return ntohl( ipAddress.s_addr );
}
