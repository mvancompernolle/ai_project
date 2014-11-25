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
// AsyncWinSock.cpp: implementation of the CAsyncWinSock class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#ifdef ENABLE_SOCKETS

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAsyncWinSock::CAsyncWinSock()
{_STT();
	m_nInit	= -1;

	m_hSocket = INVALID_SOCKET;

	m_hEvent = NULL;
	m_hShutDown = NULL;

	// Init values
	*m_szScheme = NULL;
	m_dwScheme = 0;
	*m_szHostName = NULL;
	m_dwPort = 0;
	*m_szUserName = NULL;
	*m_szPassword = NULL;
	*m_szUrlPath = NULL;
	*m_szExtraInfo = NULL;

	m_hWnd = NULL;
	m_uMsg = 0;

	m_iErr = 0;
	*m_szErrorMsg = 0;

	m_dwStatus = CWINSOCK_STATUS_NOSOCKET;

	m_lEnableCallbacks = 0;

	m_pCallback = NULL;

	// Initialize sockets
	InitSockets();
}

CAsyncWinSock::~CAsyncWinSock()
{_STT();
	Destroy();

	// Uninit sockets
	UninitSockets();
}

void CAsyncWinSock::Destroy()
{_STT();
	// Abortive close any open socket
	CloseSocket( FALSE, 0 );
}

BOOL CAsyncWinSock::InitSockets(WORD ver)
{_STT();
	// Quit if already initialized
	if ( m_nInit == 0 ) return TRUE;

	// Attempt to initialize the Socket library
	m_nInit = WSAStartup( ver, &m_wd );

	return IsInitialized();
}

void CAsyncWinSock::UninitSockets()
{_STT();
	// Punt if not initialized
	if ( !IsInitialized() ) return;

	m_nInit	= -1;

	// Clean up socket lib
	WSACleanup();
}

BOOL CAsyncWinSock::OpenSocket( int af, int type, int protocol )
{_STT();
	// We must be initialized
	if ( !IsInitialized() ) return FALSE;

	// Close any open socket
	CloseSocket( TRUE, 0 );

	// Create a scocket
	m_hSocket = socket( af, type, protocol );

	// Is there a socket
	if ( IsSocket() ) m_dwStatus = CWINSOCK_STATUS_IDLE;

	// Start callbacks if needed
	if ( m_lEnableCallbacks != 0 ) StartCallbacks( m_lEnableCallbacks );

	return IsSocket();
}

BOOL CAsyncWinSock::CloseSocket( BOOL bBlocking, DWORD dwTimeout )
{_STT();
	DWORD status = m_dwStatus;

	// We're disconnecting
	m_dwStatus = CWINSOCK_STATUS_DISCONNECTING;

	// if connected and there is a thread, let thread do the work
	if (	( !bBlocking || dwTimeout != 0 ) &&
			status == CWINSOCK_STATUS_CONNECTED && 
			m_hThread != NULL )
	{

		// Signal the thread to shutdown
		ThreadShutdown();

		// Clean up
		StopCallbacks();

		return TRUE;
	} // end if

	// Save socket pointer
	SOCKET s = m_hSocket;
	m_hSocket = INVALID_SOCKET;

	// Stop callbacks
	StopCallbacks( dwTimeout );

	// Close the socket
	BOOL ret = TRUE;
	if ( s != INVALID_SOCKET ) 
	{
		// Shut down the socket
		shutdown( s, SD_BOTH );

		// Close the socket
		ret = ( closesocket( s ) == 0 );

		// We're outta' here
		m_dwStatus = CWINSOCK_STATUS_NOSOCKET;

	} // end if

	return ret;
}

BOOL CAsyncWinSock::ThreadCloseSocket()
{_STT();
	// Do we have a valid socket?
	if ( m_hSocket == INVALID_SOCKET )
	{	m_dwStatus = CWINSOCK_STATUS_NOSOCKET;
		return TRUE;
	} // end if

	// Close the socket
	BOOL ret = ( closesocket( m_hSocket ) == 0 );
	m_hSocket = INVALID_SOCKET;

	// We're outta' here
	m_dwStatus = CWINSOCK_STATUS_NOSOCKET;

	return ret;
}

BOOL CAsyncWinSock::Listen(DWORD dwMaxConnections)
{_STT();
	if ( !IsSocket() ) return FALSE;
	if ( dwMaxConnections == 0 ) return FALSE;

	// Start the socket listening
	BOOL success = ( listen( m_hSocket, (int)dwMaxConnections ) == 0 );

	// Save status
	if ( success ) m_dwStatus = CWINSOCK_STATUS_LISTENING;

	return success;
}

BOOL CAsyncWinSock::Accept(CAsyncWinSock *pSocket, SOCKADDR *pSa, int *nAddr)
{_STT();
	if ( pSocket == NULL ) return FALSE;
	if ( !IsSocket() ) return FALSE;

	// Accept and encapsulate the socket
	BOOL success = pSocket->Attach( accept( m_hSocket, pSa, nAddr ) );

	// Update status
	if ( success ) 
	{	m_dwStatus = CWINSOCK_STATUS_CONNECTED;
		pSocket->m_dwStatus = CWINSOCK_STATUS_CONNECTED;
	} // end if

	// Start callbacks if needed
	if ( pSocket->m_lEnableCallbacks != 0 ) 
		pSocket->StartCallbacks( m_lEnableCallbacks );

	return success;
}

DWORD CAsyncWinSock::Recv(LPVOID pData, DWORD len, LPDWORD read, DWORD flags)
{_STT();
	DWORD bytes = len;

	// Init read variable
	if ( read != NULL ) *read = SOCKET_ERROR;

	// Is there a socket?
	if ( !IsSocket() ) return SOCKET_ERROR;

	// Attempt to read some data
	int r = recv( m_hSocket, (LPSTR)pData, (int)bytes, (int)flags );

	// Check for closed socket
	if ( r == 0 ) return SOCKET_ERROR;

	// Check for error
	if ( r == SOCKET_ERROR )
	{
		// Check for non-blocking read (no bytes)
		if ( WSAGetLastError() == WSAEWOULDBLOCK ) return r;

		return SOCKET_ERROR;

	} // end if

	// Save number of bytes read
	if ( read != NULL ) *read = r;

	return r;
}

DWORD CAsyncWinSock::Send( const LPVOID pData, DWORD len, LPDWORD sent, DWORD flags)
{_STT();
	if ( !IsSocket() ) return SOCKET_ERROR;

	// Zero length means NULL terminated string
	if ( len == 0 ) while ( ( (char*)pData )[ len ] != 0x0 ) len++;

	// Send the data
	DWORD s = send( m_hSocket, (LPCTSTR)pData, (int)len, (int)flags );	

	// Check for error
	if ( s == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK ) 
		return SOCKET_ERROR;

	// exit if error
	else if ( s == SOCKET_ERROR ) 
	{	if ( sent != NULL ) *sent = 0;
		return SOCKET_ERROR;
	} // end else if

	// Save number of bytes sent
	if ( sent != NULL ) *sent = s;

	return s;
}

BOOL CAsyncWinSock::IoCtlSocket(long cmd, LPDWORD pdw)
{_STT();
	if ( !IsSocket() ) return SOCKET_ERROR;
	
	// Relay the command
	return ( ioctlsocket( m_hSocket, cmd, pdw ) == 0 ); 
}

BOOL CAsyncWinSock::IoCtl(DWORD code, LPVOID pInbuf, DWORD dwInbuf, LPVOID pOutbuf, DWORD dwOutbuf, LPDWORD pBytesReturned)
{_STT();
	if ( !IsSocket() ) return FALSE;

	// Execute the command
	return ( WSAIoctl(	m_hSocket, code, pInbuf, dwInbuf, 
						pOutbuf, dwOutbuf, pBytesReturned, NULL, NULL ) == 0 );
}

BOOL CAsyncWinSock::Connect(PSOCKADDR psai, DWORD size)
{_STT();
	// Fill in whatever the user left out
	if ( psai == NULL ) psai = (PSOCKADDR)&m_sai;
	if ( size == 0 ) size = sizeof( m_sai );

	// Create socket if there is none
	if ( !IsSocket() ) 
	{	if ( !IsInitialized() || !OpenSocket() ) 
		{ 
			Destroy(); 
			return FALSE; 
		} // end if
	} // end if

	// Attempt a connection
	int ret = connect( m_hSocket, psai, size );
	int err = WSAGetLastError();
	
	// Update status
	if ( ret != SOCKET_ERROR ) m_dwStatus = CWINSOCK_STATUS_CONNECTED;
	else m_dwStatus = CWINSOCK_STATUS_CONNECTING;

	return ( ret != SOCKET_ERROR || err == WSAEWOULDBLOCK );
}

BOOL CAsyncWinSock::Connect(LPCTSTR pAddress, DWORD dwPort)
{_STT();
	DWORD port = 0;
	
	// Look up the host address
	if ( !GetHostByName( pAddress, &port ) ) return FALSE;

	// Save connection information
	strcpy( m_szHostName, pAddress );	
	if ( dwPort != 0 ) m_dwPort = dwPort;
	else m_dwPort = port;

	// Save address information
	m_sai.sin_family = AF_INET;
	m_sai.sin_port = htons( (WORD)dwPort );
	m_sai.sin_addr = *( (LPIN_ADDR)*m_phe->h_addr_list );

	// Attempt to connect
	return Connect();
}

BOOL CAsyncWinSock::Connect(DWORD ip, DWORD port)
{_STT();
	// Save connection information
	*m_szHostName = 0;
	m_dwPort = port;

	// Save address information
	m_sai.sin_family = AF_INET;
	m_sai.sin_port = htons( (WORD)port );
	m_sai.sin_addr = *( (LPIN_ADDR)&ip );

	// Attempt to connect
	return Connect();
}

BOOL CAsyncWinSock::Bind(DWORD dwPort)
{_STT();
	sockaddr_in		sai;

	ZeroMemory( &sai, sizeof( sai ) );

	sai.sin_family = PF_INET;
	sai.sin_port = htons( (WORD)dwPort );

	// Attempt to bind the socket
	int ret = bind( m_hSocket, (sockaddr*)&sai, sizeof( sockaddr_in ) );

	return ( ret != SOCKET_ERROR );
}

BOOL CAsyncWinSock::GetHostByName(LPCTSTR pHost, LPDWORD pPort, LPHOSTENT *pHE)
{_STT();
	LPHOSTENT	phe;

	if ( pHost == NULL || *pHost == 0x0 ) return FALSE;

	// Check for dot number
	DWORD port = 0;
	if ( GetDotAddress( pHost, &m_paddr[ 0 ].S_un.S_addr, &port ) )
	{
		// Dot address could specify a port
		if ( port != 0 && pPort != NULL ) *pPort = port;

		// Fill in address information
		m_he.h_name = NULL;
		m_he.h_aliases = NULL;
		m_he.h_addrtype = 0;
		m_he.h_length = 0;
		m_ppaddr = m_paddr;
		m_he.h_addr_list = (char**)&m_ppaddr;
		m_phe = &m_he;

		return TRUE;
	} // end if

	// Look up the host
	phe = gethostbyname( pHost );
	if ( phe == NULL )
	{
		ULONG addr = inet_addr( pHost );
		phe = gethostbyaddr( (char*)&addr, sizeof( ULONG ), PF_INET );
		if ( phe == NULL ) return FALSE;
	} // end if

	// does the user want a copy of the data?
	if ( pHE != NULL ) *pHE = phe;

	// else assign it to our member variable
	else m_phe = phe;

	return TRUE;	
}

BOOL CAsyncWinSock::GetDotAddress(LPCTSTR pHost, LPDWORD pdwIP, LPDWORD pdwPort )
{_STT();
	if ( pHost == NULL ) return FALSE;
	
	DWORD	i, dots = 0;
	DWORD	num = 0;
	DWORD	len = 0;
	char	buf[ MAX_PATH ];

	for ( i = 0; i < MAX_PATH && pHost[ i ] != 0x0 && pHost[ i ] != ':' && dots < 4; i++ )
	{
		// Check for invalid characters
		if (	( pHost[ i ] < '0' || pHost[ i ] > '9' ) &&
				pHost[ i ] != '.' && pHost[ i ] > ' ' && 
				pHost[ i ] <= '~' && pHost[ i ] != ':' ) return FALSE;

		// Save the character
		else if (	( pHost[ i ] >= '0' && pHost[ i ] <= '9' ) ||
					pHost[ i ] == '.' ) buf[ len++ ] = pHost[ i ];

		// Count the dots
		if ( pHost[ i ] >= '0' && pHost[ i ] <= '9' ) num++;
		if ( num > 3 ) return FALSE;
		if ( num && pHost[ i ] == '.' ) dots++, num = 0;
	} // end for 
	buf[ len++ ] = 0;

	// Must have three dots
	if ( dots != 3 ) return FALSE;

	char	*ptr = buf;
	DWORD	ip;

	// First num
	if ( *ptr == 0x0 ) return FALSE;
	ip =  ( strtoul( ptr, &ptr, 10 ) & 0xff );
	// Sec num
	while ( ptr != 0x0 && ( *ptr < '0' || *ptr > '9' ) ) ptr++;
	if ( *ptr == 0x0 ) return FALSE;
	ip |= ( strtoul( ptr, &ptr, 10 ) & 0xff ) << 8;
	// Third
	while ( ptr != 0x0 && ( *ptr < '0' || *ptr > '9' ) ) ptr++;
	if ( *ptr == 0x0 ) return FALSE;
	ip |= ( strtoul( ptr, &ptr, 10 ) & 0xff ) << 16;
	// Fourth
	while ( ptr != 0x0 && ( *ptr < '0' || *ptr > '9' ) ) ptr++;
	if ( *ptr == 0x0 ) return FALSE;
	ip |= ( strtoul( ptr, &ptr, 10 ) & 0xff ) << 24;

	// Do they want to know the address?
	if ( pdwIP != NULL ) *pdwIP = ip;

	// Does the user want a port number?
	if ( pdwPort != NULL )
	{
		DWORD	i = 0; 
		char	*end;

		// Search for port separator
		while( i < MAX_PATH && pHost[ i ] != 0 && pHost[ i ] != ':' ) i++;

		// Get the user a number
		if ( i >= MAX_PATH || pHost[ i ] != ':' ) *pdwPort = 0;
		else *pdwPort = strtoul( &pHost[ i + 1 ], &end, 10 );

	} // end if

	return TRUE;
}

BOOL CAsyncWinSock::GetServeByName(LPCTSTR pName, LPCTSTR pProto, LPSERVENT *pServent)
{_STT();
	LPSERVENT	pServe;

	// Attempt to look up the service
	pServe = getservbyname( pName, pProto );
	if ( pServent == NULL ) return FALSE;

	// Is the user interested int the return value?
	if ( pServent != NULL ) *pServent = pServe;

	// Just use our member variable
	else m_ps = pServe;

	return TRUE;
}

BOOL CAsyncWinSock::GetErrorMsg(LPSTR pMsg, DWORD err, BOOL bLineBreaks)
{_STT();
	if ( pMsg == NULL ) return FALSE;
	*pMsg = 0x0;

	// Write out error code
	if ( bLineBreaks ) wsprintf( pMsg, "Error Code: %lu\r\n\r\n", err );
	else wsprintf( pMsg, "Error Code: %lu  -  ", err );
	pMsg = &pMsg[ strlen( pMsg ) ];

	// Use last error if zero
	if ( err == 0 ) err = WSAGetLastError(); 

	// What does Windows say this error is?
	if ( !FormatMessage(	FORMAT_MESSAGE_FROM_SYSTEM,
							NULL, err, 0, pMsg, MAX_PATH, NULL ) )
		CWin32::GetWSAErrorMsg( pMsg, err );

	return TRUE;
}


BOOL CAsyncWinSock::Connect(LPCTSTR pUrl)
{_STT();
	// Crack the url
	if ( !CrackUrl( pUrl ) ) return FALSE;

	// Is the port specified?
	DWORD colon = FALSE;
	for ( DWORD i = 0; m_szHostName[ i ] != ':' && m_szHostName[ i ] != 0; i++ );
	if ( m_szHostName[ i ] == ':' )
	{	char *end;
		m_szHostName[ i++ ] = 0;
		m_dwPort = strtoul( &m_szHostName[ i ], &end, 10 );
	} // end if

	// Attempt to connect
	return Connect( m_szHostName, m_dwPort );
}

BOOL CAsyncWinSock::CrackUrl(LPCTSTR pUrl, LPCRACKURLINFO pcui)
{_STT();
	// Sanity check
	if ( pUrl == NULL || pcui == NULL ) return FALSE;

	// Init values
	*pcui->szScheme = NULL;
	pcui->dwScheme = 0;
	*pcui->szHostName = NULL;
	pcui->dwPort = 0;
	*pcui->szUserName = NULL;
	*pcui->szPassword = NULL;
	*pcui->szUrlPath = NULL;
	*pcui->szExtraInfo = NULL;

	pcui->urlc.dwStructSize = sizeof( URL_COMPONENTS );
	pcui->urlc.lpszScheme = pcui->szScheme;
	pcui->urlc.dwSchemeLength = sizeof( pcui->szScheme );
	pcui->urlc.lpszHostName = pcui->szHostName;
	pcui->urlc.dwHostNameLength = sizeof( pcui->szHostName );
	pcui->urlc.lpszUserName = pcui->szUserName;
	pcui->urlc.dwUserNameLength = sizeof( pcui->szUserName );
	pcui->urlc.lpszPassword = pcui->szPassword;
	pcui->urlc.dwPasswordLength = sizeof( pcui->szPassword );
	pcui->urlc.lpszUrlPath = pcui->szUrlPath;
	pcui->urlc.dwUrlPathLength = sizeof( pcui->szUrlPath );
	pcui->urlc.lpszExtraInfo = pcui->szExtraInfo;
	pcui->urlc.dwExtraInfoLength = sizeof( pcui->szExtraInfo );

	// Let windows crack the url
	if ( !InternetCrackUrl( pUrl, strlen( pUrl ), ICU_DECODE | ICU_ESCAPE, &pcui->urlc ) )
		return FALSE;

	// Move a little data
	pcui->dwScheme = (DWORD)pcui->urlc.nScheme;
	pcui->dwPort = (DWORD)pcui->urlc.nPort;

	return TRUE;
}

BOOL CAsyncWinSock::CrackUrl( LPCTSTR pUrl )
{_STT();
	ZeroMemory( &m_urlc, sizeof( m_urlc ) );

	LPURL_COMPONENTS	puc;

	puc = &m_urlc;

	// Init values
	*m_szScheme = NULL;
	m_dwScheme = 0;
	*m_szHostName = NULL;
	m_dwPort = 0;
	*m_szUserName = NULL;
	*m_szPassword = NULL;
	*m_szUrlPath = NULL;
	*m_szExtraInfo = NULL;

	// Fill in the structure	
	m_urlc.dwStructSize = sizeof( m_urlc );
	m_urlc.lpszScheme = m_szScheme;
	m_urlc.dwSchemeLength = sizeof( m_szScheme );
	m_urlc.lpszHostName = m_szHostName;
	m_urlc.dwHostNameLength = sizeof( m_szHostName );
	m_urlc.lpszUserName = m_szUserName;
	m_urlc.dwUserNameLength = sizeof( m_szUserName );
	m_urlc.lpszPassword = m_szPassword;
	m_urlc.dwPasswordLength = sizeof( m_szPassword );
	m_urlc.lpszUrlPath = m_szUrlPath;
	m_urlc.dwUrlPathLength = sizeof( m_szUrlPath );
	m_urlc.lpszExtraInfo = m_szExtraInfo;
	m_urlc.dwExtraInfoLength = sizeof( m_szExtraInfo );

	// Let windows crack the url
	if ( !InternetCrackUrl( pUrl, strlen( pUrl ), ICU_DECODE | ICU_ESCAPE, puc ) )
		return FALSE;

	// Grab some more values
	m_dwScheme = (DWORD)m_urlc.nScheme;
	m_dwPort = (DWORD)m_urlc.nPort;

	return TRUE;
}

BOOL CAsyncWinSock::StartCallbacks( long lEvents, BOOL bThread )
{_STT();
	// Save the enabled callbacks
	m_lEnableCallbacks = lEvents;

	// Halt current callbacks
	StopCallbacks();

	// Create the event
	m_hEvent = WSACreateEvent();

	// Select events
	EventSelect( m_hEvent, lEvents );

	// Create a shutdown event
	m_hShutDown = CreateEvent( NULL, TRUE, FALSE, NULL );
	ResetEvent( m_hShutDown );

	// Stop here if we don't want a thread
	if ( !bThread ) return TRUE;
	
	// Start a callback thread
	StartThread();

	return TRUE;
}

BOOL CAsyncWinSock::StopCallbacks( DWORD dwTimeout )
{_STT();
	// Stop the callback thread
	StopThread( dwTimeout );

	// Close event event
	if ( m_hEvent != NULL )
	{
		// Turn callbacks off
		WSAEventSelect( m_hSocket, m_hEvent, 0 );

		// Close the event
		WSACloseEvent( m_hEvent );
		m_hEvent = NULL;
	} // end if

	// Lose shutdown event
	if ( m_hShutDown != NULL )
	{	CloseHandle( m_hShutDown );
		m_hShutDown = NULL;
	} // end if

	return TRUE;
}

//------------------------------------------------------------------
// CAsyncWinSock::ThreadRun ()
//------------------------------------------------------------------
//
// This static function is the actual thread loop
// 
//------------------------------------------------------------------
BOOL CAsyncWinSock::DoThread( LPVOID pData )
{_STT();
	HANDLE				phEvents[ 4 ];

	// Get events
	phEvents[ 0 ] = m_hStop;
	phEvents[ 1 ] = m_hEvent;
	phEvents[ 2 ] = m_hShutDown;

	// Wait for an event
	DWORD ret;

	// Wait forever
	ret = WaitForMultipleObjects( 3, phEvents, FALSE, INFINITE );

	// Verify range
	if ( ret <= WAIT_OBJECT_0 + 2 )
	{
		// Get event index
		ret -= WAIT_OBJECT_0;

		// Reset event
		if ( ret > 0 && ret < 3 ) ResetEvent( phEvents[ ret ] );

		// Check for close request
		if ( ret == 0 ) return FALSE;

		// Handle socket event
		else if ( ret == 1 ) OnNetworkEvent( phEvents[ 1 ], &m_ne );

		// Look for shutdown command
		else if ( ret == 2 ) 
		{
			// Handle shutdown
			OnShutdownEvent();

			// End thread
			return FALSE;

		} // end else if
		
	} // end if	
	
	return TRUE;
}

BOOL CAsyncWinSock::OnNetworkEvent(HANDLE hEvent, LPWSANETWORKEVENTS pne)
{_STT();
	// Get network events
	if ( WSAEnumNetworkEvents( m_hSocket, hEvent, pne ) != 0 )
		return FALSE;

	if ( ( pne->lNetworkEvents & FD_READ ) != 0 )
	{	m_dwStatus = CWINSOCK_STATUS_CONNECTED;
		OnRead( pne->iErrorCode[ FD_READ_BIT ] );
	} // end if

	if ( ( pne->lNetworkEvents & FD_WRITE ) != 0 )
	{	m_dwStatus = CWINSOCK_STATUS_CONNECTED;
		OnWrite( pne->iErrorCode[ FD_WRITE_BIT ] );
	} // end if

	if ( ( pne->lNetworkEvents & FD_OOB ) != 0 )
	{	m_dwStatus = CWINSOCK_STATUS_CONNECTED;
		OnOOB( pne->iErrorCode[ FD_OOB_BIT ] );
	} // end if

	if ( ( pne->lNetworkEvents & FD_ACCEPT ) != 0 )
	{	m_dwStatus = CWINSOCK_STATUS_CONNECTED;
		OnAccept( pne->iErrorCode[ FD_ACCEPT_BIT ] );
	} // end if
	
	if ( ( pne->lNetworkEvents & FD_CONNECT ) != 0 )
	{	if ( pne->iErrorCode[ FD_CONNECT_BIT ] == 0 )
			m_dwStatus = CWINSOCK_STATUS_CONNECTED;
		else m_dwStatus = CWINSOCK_STATUS_ERROR;
		OnConnect( pne->iErrorCode[ FD_CONNECT_BIT ] );
	} // end if

	if ( ( pne->lNetworkEvents & FD_CLOSE ) != 0 )
	{	m_dwStatus = CWINSOCK_STATUS_IDLE;
		OnClose( pne->iErrorCode[ FD_CLOSE_BIT ] );
	} // end if

	if ( ( pne->lNetworkEvents & FD_QOS ) != 0 )
		OnQOS( pne->iErrorCode[ FD_QOS_BIT ] );

	if ( ( pne->lNetworkEvents & FD_GROUP_QOS ) != 0 )
		OnGroupQOS( pne->iErrorCode[ FD_GROUP_QOS_BIT ] );

	if ( ( pne->lNetworkEvents & FD_ROUTING_INTERFACE_CHANGE ) != 0 )
		OnRoutingInterfaceChange( pne->iErrorCode[ FD_ROUTING_INTERFACE_CHANGE_BIT ] );

	// Send window message if needed
	SendMessage(	(WPARAM)pne->lNetworkEvents,
					(LPARAM)pne->iErrorCode );
	
	return TRUE;
}

BOOL CAsyncWinSock::OnShutdownEvent()
{_STT();
	// Ensure we get the close command
	int err = 0;
	BOOL e = EventSelect( m_hEvent, FD_CLOSE ) != 0;
	BOOL sd = shutdown( m_hSocket, SD_SEND ) == 0;

	// Check for error
	if ( !e || !sd )
	{
		err = WSAGetLastError();
		char str[ MAX_PATH ];
		GetErrorMsg( str );

		// Let the user know we're aborting
		OnClose( 0 );
		if ( m_hWnd != NULL )
			PostMessage(	m_hWnd, m_uMsg, 
							(WPARAM)FD_CLOSE,
							(LPARAM)NULL );
	} // end if

	else
	{
		DWORD bytes = 0;

		// Is there anymore data?
		// Windows does not seem to want to send
		// the FD_CLOSE message if there are no more bytes
		if ( !Recv( NULL, 0, &bytes ) || bytes == 0 )
		{
			// No errors
			m_internalerr[ FD_CLOSE_BIT ] = 0;

			OnClose( 0 );
			if ( m_hWnd != NULL )
				PostMessage(	m_hWnd, m_uMsg, 
								(WPARAM)FD_CLOSE,
								(LPARAM)&m_internalerr );

		} // end if

		// Ensure data is read
		else
		{
			// No errors
			m_internalerr[ FD_READ_BIT ] = 0;

			OnRead( 0 );
			SendMessage(	(WPARAM)FD_READ,
							(LPARAM)&m_internalerr );

		} // end else

	} // end else

	ThreadCloseSocket();
	SignalStop();

	return TRUE;
}


BOOL CAsyncWinSock::GetSocketName( LPSTR pName, LPDWORD pdwPort )
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

BOOL CAsyncWinSock::GetPeerName(LPSTR pName, LPDWORD pdwPort)
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

BOOL CAsyncWinSock::IsRecvData()
{_STT();
	if ( !IsSocket() ) return FALSE;

	char buf[ 2 ];

	// Attempt to read some data
	int r = recv( m_hSocket, buf, 1, MSG_PEEK );

	// Check for error
	if ( r == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK )
		return FALSE;

	return TRUE;
}


BOOL CAsyncWinSock::GetConnectionInfo(LPDWORD pdwAddr, LPDWORD pdwPort, LPSTR pAddr)
{_STT();
	SOCKADDR_IN		sai;
	int				size = sizeof( sai );

	// Get socket address
	if ( ::getsockname( m_hSocket, (SOCKADDR*)&sai, &size ) != 0 )
		return FALSE;

	// Loopback = 0x0100007f
	if ( pdwAddr != NULL ) *pdwAddr = *( (LPDWORD) &sai.sin_addr );

	if ( pdwAddr != NULL || pAddr != NULL )
	{
		// Get real address if loop-back
		char name[ MAX_PATH ];
		strcpy( name, "0.0.0.0" );
		if ( !gethostname( name, MAX_PATH - 1 ) )
		{	LPHOSTENT he = gethostbyname( name );
			if ( he != NULL )
			{	
				if ( *pdwAddr == 0x0100007f ) *pdwAddr = *( (LPDWORD)he->h_addr_list[ 0 ] );
				if ( pAddr != NULL )
				{	char *paddr = inet_ntoa( *(LPIN_ADDR) he->h_addr_list );
					if ( paddr != NULL ) strcpy( pAddr, paddr );
				} // end if

			} // end if

		} // end if

	} // end if
	if ( pdwPort != NULL ) *pdwPort = *( (UINT*) &sai.sin_port );

	return TRUE;
}

static const char *g_notallowed[] =
{
	"www.", "ftp.", 
	"http://www.", "ftp://ftp.",
	"ftp://www.", "http://ftp.",
	"http://", "ftp://", 0
};


BOOL CAsyncWinSock::GetDomainName(LPCTSTR pLink, LPSTR pDomain)
{_STT();
	char domain[ CWF_STRSIZE ];

	// Sanity check
	if ( pLink == NULL || pDomain == NULL ) return FALSE;

	// Nothing yet
	*domain = 0;

	// Remove illegal prefixes
	for ( DWORD i = 0; *domain == 0 && g_notallowed[ i ] != 0; i++ )
		if ( !strnicmp( pLink, g_notallowed[ i ], strlen( g_notallowed[ i ] ) ) )
			strcpy_sz( domain, &pLink[ strlen( g_notallowed[ i ] ) ] );

	// Did we remove anything?
	if ( *domain != 0 ) { strcpy( pDomain, domain ); }
	else if ( pLink != pDomain ) strcpy( pDomain, pLink );

	// Cut at \ or /
	for ( i = 0; pDomain[ i ] != 0; i++ )
		if ( pDomain[ i ] == '\\' || pDomain[ i ] == '/' ) pDomain[ i ] = 0;

	return ( *pDomain != 0 );
}

typedef struct tagLINKPREFIX
{	char	*pre;
	DWORD	len;
	char	*add;
	DWORD	type;
} LINKPREFIX, *LPLINKPREFIX; // end typedef struct

static LINKPREFIX g_prefix[] = 
{
	{ "http://", 7, "", LINKTYPE_HTTP },
	{ "ftp://", 6, "", LINKTYPE_FTP },
	{ "www.", 4, "http://", LINKTYPE_HTTP },
	{ "ftp.", 4, "ftp://", LINKTYPE_FTP },
	{ "mailto:", 7, "", LINKTYPE_EMAIL },
	{ "news:", 5, "", LINKTYPE_NEWS },
	{ "mms://", 6, "", LINKTYPE_MMS },
	{ NULL, 0, NULL, 0 }
};

DWORD CAsyncWinSock::GetLinkType( LPCTSTR pLink, LPSTR pAdd)
{_STT();
	// Sanity check
	if ( pLink == NULL ) return LINKTYPE_NONE;

	DWORD len = strlen( pLink );

	for ( DWORD i = 0; g_prefix[ i ].pre != NULL; i++ )

		// Long enough to be this link?
		if ( len > g_prefix[ i ].len )

			// Is this the start of a link?
			if ( 	pLink[ 0 ] == g_prefix[ i ].pre[ 0 ] &&
					!strncmp( pLink, g_prefix[ i ].pre, g_prefix[ i ].len ) )
			{
				// Do they want to know how to modify the link?
				if ( pAdd != NULL ) strcpy( pAdd, g_prefix[ i ].add );

				// Return the type
				return g_prefix[ i ].type;

			} // end if

	return LINKTYPE_NONE;
}

DWORD CAsyncWinSock::GetLinkCat(LPCTSTR pLink)
{_STT();
	CRACKURLINFO ci;

	// Crack the url
	if ( !CrackUrl( pLink, &ci ) ) return LINKCAT_NONE;

	char mime[ CWF_STRSIZE ];
	if ( !CMime::GetContentType( pLink, mime ) )
		return LINKCAT_NONE;
	
	// Check for text file
	if ( !strnicmp( "text", mime, 4 ) ) return LINKCAT_TXT;

	// Check for image type
	if ( !strnicmp( "image", mime, 5 ) ) return LINKCAT_IMG;

	return LINKCAT_NONE;

}

DWORD CAsyncWinSock::GetLocalAddress()
{_STT();
	char name[ CWF_STRSIZE ] = { '~', 0x0 };
	if ( !gethostname( name, CWF_STRSIZE - 1 ) )
	{
		LPHOSTENT he = gethostbyname( name );
		if ( he != NULL ) return (DWORD)*( he->h_addr_list );

	} // end if

	return 0;
}

#endif

