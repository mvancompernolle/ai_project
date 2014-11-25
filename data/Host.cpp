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


// Host.cpp -- implementation of the Host class

#include "Host.h"
#include "NetworkEx.h"


/////////////////////////////////////////////////////////////////////////////
// class Host


// static variable for local host
std::auto_ptr<Host> Host::ipLocalhost;

// standard c++ iostream insertor
std::ostream& Host::operator<<( std::ostream& stream ) const
{
	stream << "Host( ";
	if ( isDefined() ) {
		stream << getName().c_str() << ",";

		for ( std::vector<IpAddress>::const_iterator i = ipAddresses.begin(); i != ipAddresses.end(); ++i ) {
			stream << " ";
			(*i).operator<<(stream);
		}
	}
	else {
		stream << "network host is not defined";
    }

	stream << " )";
	return stream;
}

// Construct myself to be the host with name `name`.
Host::Host( const std::string& name )
{
	set(name);
}

// Construct myself to be the host with IP address `address`.
Host::Host( const IpAddress& address )
{
	set(address);
}

// Collection of IpAddresses for the local host
const std::vector<IpAddress>& Host::getIpAddresses() const
{
	return ipAddresses;
}

// Comparsion of host names
bool Host::operator==( const Host& host ) const
{
	return getName() == host.getName();
}

// Comparsion of host names
bool Host::operator<( const Host& host ) const
{
	return getName() < host.getName();
}

// Default constructor
Host::Host() : hostName(), ipAliases(), ipAddresses()
{
}

// Use the host
Host::Host( const hostent& host )
{
	set(host);
}

// Copy constructor
Host::Host( const Host& host ) : hostName( host.hostName ), ipAliases( host.ipAliases ), ipAddresses( host.ipAddresses )
{
}

// Verfication that the host has a name
bool Host::isDefined() const
{
	return !(getName().empty());
}


// Return a collection host aliases
const std::vector<std::string>& Host::getAliases() const
{
	return ipAliases;
}

// Accessor for name
std::string Host::getName() const
{
	return hostName;
}

// Assignment operator
Host& Host::operator=(const Host& host)
{
	if ( this != &host ) {
		hostName = host.hostName;
		ipAliases = host.ipAliases;
		ipAddresses = host.ipAddresses;
	}

	return *this;
}

// Static function to retrieve the localhost
const Host& Host::getLocalhost()
{
	if (ipLocalhost.get() == 0 ) {
		try {
			char name[ 255 ];
			::gethostname(name,sizeof(name));
			ipLocalhost = std::auto_ptr<Host>(new Host(name));
		}
		catch ( std::exception& ) { //os_network_toolkit_error& ) {
			ipLocalhost = std::auto_ptr<Host>(new Host());
			ipLocalhost->hostName = "Could not find ip address for the localhost";
		}
	}

	return *(ipLocalhost.get());
}


// Accessor for setting host name
void Host::set( const std::string& name )
{
	struct hostent entry;
	if ( Host::find( name, &entry) )
		set( entry );
	else
		NetworkException::throwNetworkException(NetworkException::invalid_host,name.c_str());
}

// Accessor for setting host name
void Host::set( const IpAddress& address )
{
	struct hostent entry;
	if ( Host::find( address, &entry) )
		set( entry );
	else {
		std::stringstream stream;
		address.operator <<(stream);
		NetworkException::throwNetworkException(NetworkException::invalid_host,stream.str().c_str());
	}
}

// Accessor for setting host name
void Host::set( const hostent& host )
{
	hostName = host.h_name;
	char** ptr = host.h_aliases;

	while (*ptr)
		ipAliases.push_back(std::string( *(ptr++) ));
 
	ptr = host.h_addr_list;
	while (*ptr)
		ipAddresses.push_back(IpAddress( *(in_addr*) *(ptr++) ));
}

// Gather information on the named host
bool Host::find(const std::string& name,struct hostent* data) throw()
{
	struct hostent* result;
	if ( (result = ::gethostbyname( name.c_str() )) != 0 )
		*data = *result;
	return result != 0;
}

// Gather host information for the given IpAddress
bool Host::find(const IpAddress& addr,struct hostent* data) throw()
{
	struct hostent* result;
    if((result = ::gethostbyaddr(reinterpret_cast<const char*>(&(in_addr)addr),sizeof(in_addr),AF_INET) ) != 0)
		*data = *result;
	return result != 0;
}
