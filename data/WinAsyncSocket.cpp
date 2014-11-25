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
// WinAsyncSocket.cpp: implementation of the CWinAsyncSocket class.
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

CWinAsyncSocket::CWinAsyncSocket()
{
	// Socket is not connected
	m_uStatus = eStatusDisconnected;
}

CWinAsyncSocket::~CWinAsyncSocket()
{
	// Stop any running thread
	Stop();
}

BOOL CWinAsyncSocket::InitThread( LPVOID pData )
{
	return TRUE;
}

BOOL CWinAsyncSocket::DoThread( LPVOID pData )
{
	HANDLE phEvents[ 4 ];

	// Get events
	phEvents[ 0 ] = m_hStop;
	phEvents[ 1 ] = m_fifo.GetDataReadyHandle();
	phEvents[ 2 ] = m_ws.GetEventHandle();
	phEvents[ 3 ] = IsConnected() ? GetDataReadyHandle() : NULL;

	// How many valid handles
	UINT uHandles = !phEvents[ 2 ] ? 2 : !phEvents[ 3 ] ? 3 : 4;

	// Wait forever
	UINT uRet = WaitForMultipleObjects( uHandles, phEvents, FALSE, INFINITE );

	// Stop command?
	if ( uRet == WAIT_OBJECT_0 ) return FALSE;

	// Command?
	else if ( uRet == WAIT_OBJECT_0 + 1 )

		// Process the command
		OnCommand();

	// Write data waiting to go out?
	else if ( uRet == WAIT_OBJECT_0 + 2 )

		// Process socket event
		OnSocketEvent();

	// Write data waiting to go out?
	else if ( uRet == WAIT_OBJECT_0 + 3 )

		// Process socket event
		if ( IsConnected() ) OnDataReady();

		// Wait connection
		else Sleep( 15 );

	return TRUE;
}

BOOL CWinAsyncSocket::EndThread( LPVOID pData )
{
	m_ws.Destroy();

	return TRUE;
}

BOOL CWinAsyncSocket::OnCommand()
{	
	// How large is the buffer?
	DWORD dwCmdLen = 0;
	if ( !m_fifo.Read( NULL, 0, &dwCmdLen ) )
		return FALSE;

	// Allocate space for the command and read it in
	TMem< BYTE > mem;
	if (	!mem.allocate( dwCmdLen ) ||
			!m_fifo.Read( mem.ptr(), dwCmdLen, &dwCmdLen ) )
		return FALSE;

	// Get a pointer to the command structure
	SCmdData *pCd = (SCmdData*)mem.ptr();
	char *pExtra = (char*)mem.ptr( sizeof( SCmdData ) );

	// Attempt connection
	m_ws.Connect( pExtra, pCd->uParam1 );

	return TRUE;
}

BOOL CWinAsyncSocket::OnSocketEvent()
{
	// Must have a socket handle
	if ( !m_ws.IsSocket() ) return FALSE;

	// Use local structure if one was not provided
	WSANETWORKEVENTS wne;

	// Reset the network event
	ResetEvent( m_ws.GetEventHandle() );

	// Get network events
	if ( 0 != WSAEnumNetworkEvents( m_ws, m_ws.GetEventHandle(), &wne ) )
		return FALSE;

	// *** These messages are sent in a purposeful order

	// Ensure connect messages are first
	if ( ( wne.lNetworkEvents & FD_CONNECT ) != 0 )
	{	if ( wne.iErrorCode ) m_uStatus = eStatusDisconnected;
		else m_uStatus = eStatusConnected;
		OnConnect( wne.iErrorCode[ FD_CONNECT_BIT ] );
		MMessage( (WPARAM)FD_CONNECT, (LPARAM)wne.iErrorCode[ FD_CONNECT_BIT ] );
	} // end if

	if ( ( wne.lNetworkEvents & FD_ACCEPT ) != 0 )
	{	m_uStatus = eStatusConnected;
		OnAccept( wne.iErrorCode[ FD_ACCEPT_BIT ] );
		MMessage( (WPARAM)FD_ACCEPT, (LPARAM)wne.iErrorCode[ FD_ACCEPT_BIT ] );
	} // end if
	
	if ( ( wne.lNetworkEvents & FD_OOB ) != 0 )
	{	m_uStatus = eStatusConnected;
		OnOOB( wne.iErrorCode[ FD_OOB_BIT ] );
		MMessage( (WPARAM)FD_OOB, (LPARAM)wne.iErrorCode[ FD_OOB_BIT ] );
	} // end if

	if ( ( wne.lNetworkEvents & FD_QOS ) != 0 )
	{	m_uStatus = eStatusConnected;
		OnQOS( wne.iErrorCode[ FD_QOS_BIT ] );
		MMessage( (WPARAM)FD_QOS, (LPARAM)wne.iErrorCode[ FD_QOS_BIT ] );
	} // end if

	if ( ( wne.lNetworkEvents & FD_GROUP_QOS ) != 0 )
	{	m_uStatus = eStatusConnected;
		OnGroupQOS( wne.iErrorCode[ FD_GROUP_QOS_BIT ] );
		MMessage( (WPARAM)FD_GROUP_QOS, (LPARAM)wne.iErrorCode[ FD_GROUP_QOS_BIT ] );
	} // end if

	if ( ( wne.lNetworkEvents & FD_ROUTING_INTERFACE_CHANGE ) != 0 )
	{	m_uStatus = eStatusConnected;
		OnRoutingInterfaceChange( wne.iErrorCode[ FD_ROUTING_INTERFACE_CHANGE_BIT ] );
		MMessage( (WPARAM)FD_ROUTING_INTERFACE_CHANGE, (LPARAM)wne.iErrorCode[ FD_ROUTING_INTERFACE_CHANGE_BIT ] );
	} // end if

	if ( ( wne.lNetworkEvents & FD_WRITE ) != 0 )
	{	m_uStatus = eStatusConnected;
		OnWrite( wne.iErrorCode[ FD_WRITE_BIT ] );
		MMessage( (WPARAM)FD_WRITE, (LPARAM)wne.iErrorCode[ FD_WRITE_BIT ] );
	} // end if

	if ( ( wne.lNetworkEvents & FD_READ ) != 0 )
	{	m_uStatus = eStatusConnected;
		OnRead( wne.iErrorCode[ FD_READ_BIT ] );
		MMessage( (WPARAM)FD_READ, (LPARAM)wne.iErrorCode[ FD_READ_BIT ] );
	} // end if

	// Ensure close messages are last
	if ( ( wne.lNetworkEvents & FD_CLOSE ) != 0 )
	{	m_uStatus = eStatusDisconnected;
		OnClose( wne.iErrorCode[ FD_CLOSE_BIT ] );
		MMessage( (WPARAM)FD_CLOSE, (LPARAM)wne.iErrorCode[ FD_CLOSE_BIT ] );
	} // end if

	return TRUE;
}

BOOL CWinAsyncSocket::Connect(LPCTSTR pIpAddress, UINT uPort)
{
	if ( !pIpAddress || !*pIpAddress ) return FALSE;

	// Lock the fifo buffer
	CTlLocalLock ll( m_fifo );
	if ( !ll.IsLocked() ) return FALSE;

	// Command data
	SCmdData cd;
	cd.uCmd = eCmdOpen;
	cd.uParam1 = uPort;
	cd.uDataLen = strlen( pIpAddress ) + 1;

	// Initialize fifo write	
	if ( !m_fifo.InitFifoWrite() ) return FALSE;

	// Write the command header
	if ( !m_fifo.AddFifo( &cd, sizeof( cd ) ) ) return FALSE;

	// Add ip address string
	if ( !m_fifo.AddFifo( pIpAddress, cd.uDataLen ) ) return FALSE;

	// Finalize the write
	if ( !m_fifo.EndFifoWrite() ) return FALSE;

	// Start the thread if needed
	if ( !IsRunning() ) Start();

	return TRUE;
}

BOOL CWinAsyncSocket::Close()
{
	// Lock the fifo buffer
	CTlLocalLock ll( m_fifo );
	if ( !ll.IsLocked() ) return FALSE;

	// Command data
	SCmdData cd;
	cd.uCmd = eCmdClose;
	cd.uParam1 = 0;
	cd.uDataLen = 0;

	// Initialize fifo write	
	if ( !m_fifo.InitFifoWrite() ) return FALSE;

	// Write the command header
	if ( !m_fifo.AddFifo( &cd, sizeof( cd ) ) ) return FALSE;

	// Finalize the write
	return m_fifo.EndFifoWrite();
}

#endif

