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
// WspStream.cpp: implementation of the CWspGeneric class.
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

CWspStream::CWspStream()
{
	m_bTxBusy = FALSE;
	m_bRxBusy = FALSE;
}

CWspStream::~CWspStream()
{

}

BOOL CWspStream::OnWrite( int nErr )
{
	// Lock the transmit buffer
	CTlLocalLock ll( *Tx() );
	if ( !ll.IsLocked() ) return FALSE;		

	// Not blocking now
	m_bTxBusy = FALSE;

	// Send more data
	return OnTx();
}

BOOL CWspStream::OnRead( int nErr )
{
	// For some reason, active-x controls seem to re-enter here...
	if ( m_bRxBusy ) return FALSE;
	m_bRxBusy = TRUE;

	// Receive the bytes
	BYTE buf[ eMaxTransferBlockSize ];

	UINT uBytes;
	while ( ( uBytes = GetSocket()->Recv( buf, sizeof( buf ) ) ) > 0 && sizeof( buf ) >= uBytes )

		// Write into the rx'er
		m_rx.Write( buf, uBytes );

	// Not busy
	m_bRxBusy = FALSE;

	return TRUE;
}

BOOL CWspStream::OnTx()
{
	// Lock the transmit buffer
	CTlLocalLock ll( m_tx );
	if ( !ll.IsLocked() ) 
		return FALSE;

	// Punt if tx'er is full
	if ( m_bTxBusy ) return TRUE;

	BYTE buf[ eMaxTransferBlockSize ];
	DWORD dwReady;

	// Read blocks of data
	while ( m_tx.Peek( buf, sizeof( buf ), &dwReady ) )
	{
		// Send the data on
		int res = GetSocket()->Send( buf, dwReady );

		// Check for socket error
		if ( res == 0 || res == SOCKET_ERROR || (DWORD)res > dwReady ) 
		{
			// Is the tx'er full
			if ( WSAEWOULDBLOCK == GetSocket()->GetLastError() )
			{	m_bTxBusy = TRUE;
				return TRUE;
			} // end if

			// Remove the number of bytes sent from the buffer
			m_tx.AdvanceReadPtr( dwReady );

			return FALSE;

		} // end if

		// Remove the number of bytes sent from the buffer
		m_tx.AdvanceReadPtr( res );

		// Punt if tx'er is full
		if ( (DWORD)res != dwReady )
		{	m_bTxBusy = TRUE;
			return TRUE;
		} // end if

	} // end while

	return TRUE;
}

#endif