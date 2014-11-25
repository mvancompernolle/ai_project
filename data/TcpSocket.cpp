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


// TcpSocket.cpp -- implementation for the TcpSocket class

#include "TcpSocket.h"


/////////////////////////////////////////////////////////////////////////////
// class TcpSocket

// Default constructor
TcpSocket::TcpSocket(TcpBinding binding) : ConnectableSocket(SOCK_STREAM, AF_INET)
{
	if ( binding == bound )
		Socket::bind(SocketAddress());
}

// Construct from the OS native descriptor
TcpSocket::TcpSocket(SOCKET socketHandle) : ConnectableSocket(socketHandle)
{
}

// Construct from the given SocketAddress
TcpSocket::TcpSocket(const SocketAddress& address) : ConnectableSocket(SOCK_STREAM,address.getAddressFamily(),0)
{
	Socket::bind(address);
}


// Close and reopen the TcpSocket (all active connections will be lost)
void TcpSocket::reset()
{
	Socket::reset(SOCK_STREAM,AF_INET);
}


// Whether keep-alives are being sent
bool TcpSocket::keepAlive() const
{
	return getBoolOption(SO_KEEPALIVE);
}


// Set the keep-alive socket option
void TcpSocket::setKeepAlives( bool flag )
{
	setBoolOption(SO_KEEPALIVE, flag);
}
