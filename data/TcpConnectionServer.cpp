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


// TcpConnectionServer.cpp -- implementation for the TcpConnectionServer class

#include <errno.h>
#include "TcpConnectionServer.h"
#include "TcpSocket.h"
#include "SystemCall.h"


/////////////////////////////////////////////////////////////////////////////
// TcpConnectionServer


// Construct from native OS socket descriptor
TcpConnectionServer::TcpConnectionServer(SOCKET descriptor) : Socket(descriptor)
{
}

// Construct and bind to the SocketAddress
// backlog is the maximum length of the queue of pending connections
TcpConnectionServer::TcpConnectionServer(const SocketAddress& address,int backlog) : Socket(SOCK_STREAM, address.getAddressFamily())
{
	if (address.isIpAddress())
		setReuseLocalAddresses(true);

	bind( address );
	listen( backlog );
}

// Permits an incoming connection attempt on a socket
bool TcpConnectionServer::accept( TcpSocket& socket )
{
	char buffer[ 200 ];
#ifdef _MSC_VER
	int size = sizeof(buffer);
#else
	socklen_t size = sizeof( sockaddr );
#endif

	int result;
	SOCKET_CALL_3(result=(int),::accept,getDescriptor(),(sockaddr*)buffer,&size)
	if ( result >= 0 ) {
		socket.close();
		socket.setDescriptor(result);
		return true;
	}
	else {
		return false;
	}
}

// Places socket state to listen for incoming connections.
// backlog is the maximum length of the queue of pending connections
void TcpConnectionServer::listen( int backlog )
{
	int dummy;
	SOCKET_CALL_2( dummy =, ::listen, getDescriptor(), backlog )
}
