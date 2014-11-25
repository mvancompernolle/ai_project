/*******************************************************************
// Copyright (c) 2000, Robert Umbehant
// mailto:rumbehant@wheresjames.com
//
// This library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either 
// version 2.1 of the License, or (at your option) any later 
// version.
//
// This library is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public 
// License along with this library; if not, write to the Free 
// Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
// MA 02111-1307 USA 
//
*******************************************************************/
// WinSocket.cpp: implementation of the CWinSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef ENABLE_SOCKETS2

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Initialization 
int CWinSocket::m_nInit	= -1;
WSADATA CWinSocket::m_wd;

CWinSocket::CWinSocket()
{_STT();

	m_hSocket = INVALID_SOCKET;

	m_uLastError = 0;

	m_uPort = 0;

	m_hEvent = WSA_INVALID_EVENT;
}

CWinSocket::~CWinSocket()
{_STT();
	
	/// Lose the current socket
	Destroy();
}

BOOL CWinSocket::InitSockets(WORD ver)
{_STT();

	// Quit if already initialized
	if ( m_nInit == 0 ) return TRUE;

	// Attempt to initialize the Socket library
	m_nInit = WSAStartup( ver, &m_wd );

	return IsInitialized();
}

void CWinSocket::UninitSockets()
{_STT();

	// Punt if not initialized
	if ( !IsInitialized() ) return;

	// Not initialized
	m_nInit	= -1;

	// Clean up socket lib
	WSACleanup();
}

void CWinSocket::Destroy()
{_STT();

	// Punt if not initialized
	if ( !IsInitialized() ) { TRACE( "CWinSocket: Error - Call InitSockets()" ); return; }

	// Ditch the event handle
	CloseEventHandle();

	// Save socket pointer
	SOCKET hSocket = m_hSocket;
	m_hSocket = INVALID_SOCKET;

	// Close the socket
	if ( INVALID_SOCKET == hSocket ) return;

	m_uPort = 0;
	m_sDotAddress = "";

	// Close the socket
	closesocket( hSocket );

	// Save the last error code
	m_uLastError = WSAGetLastError();
}

BOOL CWinSocket::Create( int af, int type, int protocol )
{_STT();

	// Punt if not initialized
	if ( !IsInitialized() ) { TRACE( "CWinSocket: Error - Call InitSockets()" ); return FALSE; }

	// Close any open socket
	Destroy();

	// Create a scocket
	m_hSocket = socket( af, type, protocol );
	
	// Save the last error code
	m_uLastError = WSAGetLastError();

	// Create the event handle
	CreateEventHandle();

	// Capture all events
	EventSelect();

	return IsSocket();
}

BOOL CWinSocket::Bind(UINT uPort)
{_STT();

	// Punt if not initialized
	if ( !IsInitialized() ) { TRACE( "CWinSocket: Error - Call InitSockets()" ); return FALSE; }

	// Create socket if there is none
	if ( !IsSocket() && !Create() ) 
	{	Destroy(); return FALSE; }

	sockaddr_in sai;
	ZeroMemory( &sai, sizeof( sai ) );

	sai.sin_family = PF_INET;
	sai.sin_port = htons( (WORD)uPort );

	// Attempt to bind the socket
	int nRet = bind( m_hSocket, (sockaddr*)&sai, sizeof( sockaddr_in ) );

	// Save the last error code
	m_uLastError = WSAGetLastError();

	return !nRet;
}

BOOL CWinSocket::Listen(UINT uMaxConnections)
{_STT();

	// Punt if not initialized
	if ( !IsInitialized() ) { TRACE( "CWinSocket: Error - Call InitSockets()" ); return FALSE; }

	// Must have socket
	if ( !IsSocket() ) return FALSE;

	// Valid number of connections?
	if ( uMaxConnections == 0 ) return FALSE;

	// Start the socket listening
	int nRet = listen( m_hSocket, (int)uMaxConnections );

	// Save the last error code
	m_uLastError = WSAGetLastError();

	return nRet;
}

BOOL CWinSocket::Connect(sockaddr_in *pSa, UINT uSize)
{_STT();

	// Punt if not initialized
	if ( !IsInitialized() ) { TRACE( "CWinSocket: Error - Call InitSockets()" ); return FALSE; }

	// Sanity check
	if ( !pSa || !uSize ) return FALSE;

	// Create socket if there is none
	if ( !IsSocket() && !Create() ) 
	{	Destroy(); return FALSE; }

	// Save the port
	m_uPort = pSa->sin_port;

	// Create dot address if needed
	m_sDotAddress = inet_ntoa( pSa->sin_addr );

	// Use dot address as address if not specified
	if ( !m_sAddress.length() ) m_sAddress = m_sDotAddress;
	
	// Attempt a connection
	int nRet = connect( m_hSocket, (PSOCKADDR)pSa, uSize );

	// Save the last error code
	m_uLastError = WSAGetLastError();

	// Return the error code
	return ( !nRet || WSAEWOULDBLOCK == m_uLastError );
}

BOOL CWinSocket::Connect(LPCTSTR pAddress, UINT uPort)
{_STT();

	// Punt if not initialized
	if ( !IsInitialized() ) { TRACE( "CWinSocket: Error - Call InitSockets()" ); return FALSE; }

	UINT port = 0;
	LPHOSTENT pHe = NULL;
	
	// Interpret the host address
	if ( !GetHostByName( pAddress, &pHe, uPort ? NULL : &port ) ) 
		return FALSE;

	// Ensure we get the correct port
	if ( !uPort ) uPort = port;

	// Save the address
	m_sAddress = pAddress;

	// Save address information
	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons( (WORD)uPort );
	sa.sin_addr = *( (LPIN_ADDR)*pHe->h_addr_list );

	// Attempt to connect
	return Connect( &sa, sizeof( sa ) );
}

BOOL CWinSocket::GetHostByName(LPCTSTR pHost, LPHOSTENT *pHe, UINT *puPort)
{_STT();

	// Sanity check
	if ( pHost == NULL || *pHost == 0x0 ) return FALSE;

	// Does caller want the port?  
	// Assume it is the number following ':' character
	if ( puPort )
	{	UINT i = 0; while ( pHost[ i ] && ':' != pHost[ i ] ) i++;
		*puPort = pHost[ i ] ? strtoul( &pHost[ i + 1 ], NULL, 10 ) : 0;
	} // end if

	// Does the caller want the address?
	if ( pHe )
	{
		// First try to interpret as dot address
		ULONG uAddr = inet_addr( pHost );
		if ( INADDR_NONE != uAddr )
		{
			// Initialize HOSTENT structure
			*pHe = &m_he;
			ZeroMemory( &m_he, sizeof( m_he ) );

			// Set address information
			ZeroMemory( &m_paddr, sizeof( m_paddr ) );
			m_paddr[ 0 ].S_un.S_addr = uAddr;

			// Save address
			m_ppaddr = m_paddr;
			m_he.h_addr_list = (char**)&m_ppaddr;

			// Quit without calling any functions
			return TRUE;

		} // end if

		// Attempt to lookup the name
		else *pHe = gethostbyname( pHost );	 

	} // end if

	// Get the last error code
	m_uLastError = WSAGetLastError();

	// How did it go?
	return ( !pHe || *pHe );	
}

BOOL CWinSocket::WaitEvent( WSAEVENT hEvent, long lEventId, LPWSANETWORKEVENTS pWne, UINT uTimeout )
{
	// Must have a socket handle
	if ( !IsSocket() ) return FALSE;

	// Use local structure if one was not provided
	WSANETWORKEVENTS wne;
	if ( pWne == NULL ) pWne = &wne;

	// Save start time
	UINT uEnd = GetTickCount() + uTimeout;
	for( ; ; )
	{
		// Wait for event
		UINT uRet = WaitForSingleObject( hEvent, uTimeout );

		// Check for timeout or error
		if ( uRet != WAIT_OBJECT_0 ) return FALSE;

		// Reset the network event
		ResetEvent( hEvent );

		// Get network events
		if ( 0 != WSAEnumNetworkEvents( m_hSocket, hEvent, pWne ) )
			return FALSE;

		// Was it an event we're looking for?
		if ( 0 != ( pWne->lNetworkEvents & lEventId ) )
		{
			// Save the error code
			m_uLastError = pWne->iErrorCode[ GetEventBit( lEventId & pWne->lNetworkEvents ) ];

			// We received the event
			return TRUE;

		} // end if

		// Have we timed out?
		UINT uTick = GetTickCount();
		if ( uEnd < uTick ) return FALSE;

		// Adjust timeout
		uTimeout -= ( uEnd - uTick );

	} // end if

	// Can't get here...
	return FALSE;
}

UINT CWinSocket::GetEventBit(long lEventMask)
{
	// return correct event bit 
	if ( 0 != ( FD_READ & lEventMask ) ) return FD_READ_BIT;	
	if ( 0 != ( FD_WRITE & lEventMask ) ) return FD_WRITE_BIT;	
	if ( 0 != ( FD_CONNECT & lEventMask ) ) return FD_CONNECT_BIT;
	if ( 0 != ( FD_CLOSE & lEventMask ) ) return FD_CLOSE_BIT;
	if ( 0 != ( FD_ACCEPT & lEventMask ) ) return FD_ACCEPT_BIT;	
	if ( 0 != ( FD_OOB & lEventMask ) ) return FD_OOB_BIT;	
	if ( 0 != ( FD_QOS & lEventMask ) ) return FD_QOS_BIT;
	if ( 0 != ( FD_GROUP_QOS & lEventMask ) ) return FD_GROUP_QOS_BIT;	
	if ( 0 != ( FD_ROUTING_INTERFACE_CHANGE & lEventMask ) ) return FD_ROUTING_INTERFACE_CHANGE_BIT;

	return 0;
}


UINT CWinSocket::Send(const LPVOID pData, UINT uSize, UINT *puSent, UINT uFlags)
{
	// Initialize bytes sent
	if ( puSent ) *puSent = 0;

	// Must have a socket handle
	if ( !IsSocket() ) return FALSE;

	// Attempt to send the data
	int nRet = send( m_hSocket, (LPCTSTR)pData, (int)uSize, (int)uFlags );

	// Get the last error code
	m_uLastError = WSAGetLastError();

	// Check for error
	if ( SOCKET_ERROR == nRet )
	{
		// Is the socket blocking?
		if ( WSAEWOULDBLOCK != m_uLastError )
			return SOCKET_ERROR;

		// Number of bytes sent
		if ( puSent ) *puSent = uSize;
			
		// Error
		return uSize;

	} // end if

	// Save the number of bytes sent
	if ( puSent ) *puSent = nRet;

	return nRet;
}

UINT CWinSocket::Recv(LPVOID pData, UINT uSize, UINT *puRead, UINT uFlags)
{
	// Initialize bytes read
	if ( puRead ) *puRead = 0;

	// Must have a socket handle
	if ( !IsSocket() ) return FALSE;

	// Receive data from socket
	int nRet = recv( m_hSocket, (LPSTR)pData, (int)uSize, (int)uFlags );

	// Check for closed socket
	if ( !nRet ) return SOCKET_ERROR;

	// Check for socket error
	if ( SOCKET_ERROR == nRet )
	{
		// Is the socket blocking?
		if ( WSAEWOULDBLOCK != m_uLastError )
			return SOCKET_ERROR;

		// All bytes read
		if ( puRead ) *puRead = uSize;

		return uSize;

	} // end if

	// Save the number of bytes read
	if ( puRead ) *puRead = nRet;

	return nRet;
}

BOOL CWinSocket::GetPeerName(LPSTR pName, LPDWORD pdwPort)
{_STT();
	SOCKADDR_IN sai; ZeroMemory( &sai, sizeof( sai ) );

	// Initialize input params
	if ( pName ) *pName = 0;
	if ( pdwPort ) *pdwPort = 0;

	// Get the socket info
	int len = sizeof( sai );
	if ( getpeername( m_hSocket, (sockaddr*)&sai, &len ) )
		return FALSE;

	if ( pdwPort ) *pdwPort = ntohs( sai.sin_port );
	if ( pName ) strcpy( pName, inet_ntoa( sai.sin_addr ) );

	return TRUE;
}

BOOL CWinSocket::GetSocketName( LPSTR pName, LPDWORD pdwPort )
{_STT();
	SOCKADDR_IN sai; ZeroMemory( &sai, sizeof( sai ) );
	
	// Initialize input params
	if ( pName ) *pName = 0;
	if ( pdwPort ) *pdwPort = 0;

	// Get the socket info
	int len = sizeof( sai );
	if ( getsockname( m_hSocket, (sockaddr*)&sai, &len ) )
		return FALSE;

	if ( pdwPort ) *pdwPort = ntohs( sai.sin_port );
	if ( pName ) strcpy( pName, inet_ntoa( sai.sin_addr ) );

	return TRUE;
}


#endif