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


// ConnectableSocket.cpp -- implementation for the ConnectableSocket class

#include <errno.h>
#include "ConnectableSocket.h"
#include "SystemCall.h"


/////////////////////////////////////////////////////////////////////////////
// class Host

// standard c++ iostream inserter
std::ostream& ConnectableSocket::operator<<( std::ostream& stream) const
{
	stream << "ConnectableSocket( ";
	if ( isOpen() ) {
		stream << getDescriptor() << ", ";
		if ( isConnected() )
			peerAddress().operator <<(stream);
		else
			stream << "connection is not defined";
	}
	else {
		stream << "connection is not currently opened";
	}

	stream << " )";
	return stream;
}


// Return true if the end of stream has been reached
bool ConnectableSocket::isEof() const
{
	return ( flags_ & state_eof ) != 0;
}


// Returns whether the end of stream will raise an event
bool ConnectableSocket::eventOnEof() const
{
	return ( flags_ & state_raise_event ) != 0;
}


// Sets the end of stream event
void ConnectableSocket::eventOnEof( bool flag )
{
	if ( flag )
		flags_ |= state_raise_event;
	else
		flags_ &= ~state_raise_event;
}

// Return success if the socket hasn't failed
bool ConnectableSocket::isOk() const
{
	return isOpen() && ( ( flags_ & state_failed ) == 0 );
}


// Return `true` if I'm in a good state, and not at eof.
bool ConnectableSocket::isGood() const
{
	return isOk() && !isEof();
}


// I'm if not broken, reset my eof and failed flags.
void ConnectableSocket::clear()
{
	flags_ &= state_raise_event;
}


// Return `true` if every flag in `mask` is in my I/O state.
bool ConnectableSocket::getFlags( short mask ) const
{
	return ( flags_ & mask ) == mask;
}


// Construct myself to reference the socket with descriptor
ConnectableSocket::ConnectableSocket( SOCKET descriptor ) : Socket( descriptor )
{
}


// Construct myself to be a new socket with type, domain, and protocol
ConnectableSocket::ConnectableSocket(int type,int domain,int protocol) : Socket(type,domain,protocol)
{
}


// Connect to the socket with IP socket address
void ConnectableSocket::connectTo( const SocketAddress& address )
{
	sockaddr_in in = address.operator sockaddr_in();
	connectTo(reinterpret_cast<sockaddr*>(&in),sizeof(sockaddr));
}


// Connect to the socket with address the given address
void ConnectableSocket::connectTo( sockaddr* address, int size )
{
	int result;
	SOCKET_CALL_3( result=(int),::connect,getDescriptor(),address,size)
	if ( result >= 0 )
		clear();
}


// Receive up to bytes and place them into supplied buffer.
// Return the number of bytes that were successfully read, and set address to the IP socket
// address of the socket that the bytes were read from (`AF_INET` only).
int ConnectableSocket::receiveFrom(SocketAddress& address,void* buffer,int bytes)
{
	int result = -1 ;
    // not really ::accept(), but same idea applies
#ifdef _MSC_VER
    int length = sizeof( sockaddr );
#else
	socklen_t length = sizeof( sockaddr );
#endif

	if ( isOk() ) {
		SOCKET_CALL_6(result = (int),::recvfrom,getDescriptor(),(char*) buffer,
			bytes,0,(sockaddr*) &(address.operator sockaddr_in()),&length)

	if ( result < 0 )
		flags_ |= state_failed;
	}
	return result;
}


// Return `true` if I'm connected to another socket.
bool ConnectableSocket::isConnected() const
{
	bool connected = true;
	try {
		peerAddress();
	}
	catch ( NetworkException& ) {
		connected = false;
	}
	return connected;
}


// Write buffer (number given by parameter) to socket stream
int ConnectableSocket::write( const void* buffer, int bytes )
{
	int result = -1;
	if ( !( flags_ & state_failed ) ) {
		result = ::send(socketHandle,reinterpret_cast<const char*>(buffer),bytes,0);
		if (result < 0) {
			flags_ |= state_failed;
			NetworkException::throwNetworkException(NetworkException::write_failed,socket_error);
		}
		else {
			flags_ &= ~state_eof;
		}
	}

	return result;
}



// Read from the socket stream into the buffer
int ConnectableSocket::read( void* buffer, int bytes )
{
	int result = -1;
	if ( !( flags_ & state_failed ) ) {
		result = ::recv(socketHandle,reinterpret_cast<char*>(buffer),bytes,0);
		if (result < 0) {
			flags_ |= state_failed;
			NetworkException::throwNetworkException(NetworkException::read_failed,socket_error);
		}
		else if ( !result && !(flags_ & state_eof) ) {
			flags_ |= state_eof;
			if (( flags_ & state_raise_event) == state_raise_event)
				NetworkException::throwNetworkException(NetworkException::eof_encountered);
		}
	}
	return result;
}



// Return an SocketAddress for the connected peer
SocketAddress ConnectableSocket::peerAddress() const
{
	SocketAddress address;
#ifdef _MSC_VER
    int length = sizeof( sockaddr );
#else
	socklen_t length = sizeof( sockaddr );
#endif
    int dummy = 0;
	SOCKET_CALL_3(dummy =,::getpeername,getDescriptor(),
		reinterpret_cast<sockaddr*>(&(address.operator sockaddr_in())),&length)
	return address;
}
