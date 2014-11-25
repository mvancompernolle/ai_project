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


// NetworkEx.cpp -- implementation of NetworkException class

#include "NetworkEx.h"


/////////////////////////////////////////////////////////////////////////////
// Quick and dirty way to do error resources.  The message_catalog
// from the C++ standard should be used, but I can't find any information
// on how it is used

struct ErrorMap
{
	const char* symbol;
	const char* description;
}
NetworkExceptions[] =
{
	"invalid_address","An unknown or invalid address was supplied.",
	"invalid_host","Unknown or invalid host name was supplied.",
	"invalid_network","An invalid network name was supplied.",
	"invalid_service","An invalid service name was supplied.",
	"read_failed","A read operation failed.",
	"write_failed","A write operation failed.",
	"socket_path_exceeded","File path exceeded the system limit.",
	"eof_encountered","An end of file marker was detected while reading from a stream.",
	"system_call_failure","The operating system reported a failure during a system call.",
	"system_call_timout","The call made to the operating system has timed out."
};


/////////////////////////////////////////////////////////////////////////////
// class NetworkException

// Constructor.
NetworkException::NetworkException(ExceptionCode code,const char* note,long native) : std::exception( ),
	exceptionCode( code ),nativeCode( native )
{
}

// Return error value from enumeration
NetworkException::ExceptionCode NetworkException::getErrorCode() const throw ()
{
	return exceptionCode;
}

// Return the native operating system error code.

long NetworkException::getNativeErrorCode() const throw ()
{
	return nativeCode;
}

const char *NetworkException::what() const throw()
{
    return getErrorDescription( getErrorCode() );
}

// Throws a network exception
void NetworkException::throwNetworkException(ExceptionCode code, const char* message) //throw(NetworkException)
{
	std::ostringstream stream;
	stream << code << ": " << message << std::ends;
	throw NetworkException( code,stream.str().c_str());
}

// Throws a network exception with the given native error
void NetworkException::throwNetworkException(ExceptionCode code, long nativeError) //throw(NetworkException)
{
	std::ostringstream stream;
	stream << getErrorSymbol(code) << ": " << "OS Error code is " << nativeError << std::ends;
	throw NetworkException(code,stream.str().c_str(),nativeError);
}

const char* NetworkException::getErrorSymbol( long code )
{
	if (code < 0 ||code >= sizeof( NetworkExceptions ) / sizeof( ErrorMap ))
		return 0;

	return NetworkExceptions[ code ].symbol;
}

const char* NetworkException::getErrorDescription( long code )
{
	if (code < 0 || code >= sizeof( NetworkExceptions ) / sizeof( ErrorMap ))
		return 0;
	return NetworkExceptions[ code ].description;
}
