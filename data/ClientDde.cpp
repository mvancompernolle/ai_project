/*******************************************************************
// Copyright (c) 2002, Robert Umbehant
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
// ClientDde.cpp: implementation of the CClientDde class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DWORD CClientDde::m_dwInstServer = 0;

CClientDde::CClientDde()
{_STT();
	m_hConv = NULL;
}

CClientDde::~CClientDde()
{_STT();
	Destroy();
}

BOOL CClientDde::Init()
{_STT();
	// Already initialized?
	if ( m_dwInstServer ) return TRUE;

	// Initialize DDE lib
	UINT hRet = DdeInitialize(	&m_dwInstServer, &CClientDde::DdeCallback,
								APPCMD_CLIENTONLY, 0 );
	
	if ( hRet != DMLERR_NO_ERROR ) m_dwInstServer = NULL;

	return ( m_dwInstServer != NULL );
}

void CClientDde::Uninit()
{_STT();
	// Close DDE lib if any
	if ( m_dwInstServer )
	{	DdeUninitialize( m_dwInstServer );
		m_dwInstServer = 0;
	} // end if
}

void CClientDde::Destroy()
{_STT();
	Disconnect();
}

BOOL CClientDde::Connect(LPCTSTR pService, LPCTSTR pTopic)
{_STT();
	// Sanity checks
	if ( m_dwInstServer == NULL ) return FALSE;

	Disconnect();

	// Connect to DDE server
	m_hConv = DdeConnect(	m_dwInstServer, 
							CDdeSz( m_dwInstServer, pService ),
							CDdeSz( m_dwInstServer, pTopic ),
							NULL );
	if ( m_hConv == NULL )
	{  	UINT uiErr = DdeGetLastError( m_dwInstServer );
		return FALSE;
	} // end if

	// Set the class pointer
	DdeSetUserHandle( m_hConv, QID_SYNC, (DWORD)this );

	return TRUE;
}

HDDEDATA CClientDde::DdeCallback(	UINT uType, UINT uFmt, HCONV hconv,
									HSZ hsz1, HSZ hsz2, HDDEDATA hdata,
									DWORD dwData1, DWORD dwData2 )
{_STT();
	CONVINFO ci;
	ZeroMemory( &ci, sizeof( ci ) );

	ci.cb = sizeof( ci );
	DdeQueryConvInfo( hconv, QID_SYNC, &ci );

	CClientDde* pDlg = (CClientDde*)ci.hUser;
	if ( pDlg == NULL ) return NULL;

	// Call the dynamic function
	return pDlg->OnDdeCallback( uType, uFmt, hconv, hsz1, hsz2,
								hdata, dwData1, dwData2 );
}

HDDEDATA CClientDde::OnDdeCallback(	UINT uType, UINT uFmt, HCONV hconv,
									HSZ hsz1, HSZ hsz2, HDDEDATA hdata,
									DWORD dwData1, DWORD dwData2 )
{_STT();
	switch ( uType )  
	{ 
		case XTYP_REGISTER: 
		case XTYP_UNREGISTER: 
			return (HDDEDATA) NULL; 

		case XTYP_ADVDATA: 
			return (HDDEDATA) DDE_FACK; 

		case XTYP_XACT_COMPLETE: 
			// this one is called after a finish of TIMEOUT_ASYNC
			return (HDDEDATA) NULL; 

		case XTYP_DISCONNECT: 
			return (HDDEDATA) NULL; 

	} // end switch

	return (HDDEDATA) NULL; 
}

void CClientDde::Disconnect()
{_STT();
	if ( m_hConv ) DdeDisconnect( m_hConv ), m_hConv = NULL;
}

BOOL CClientDde::RequestData(LPCTSTR pName, DWORD dwTimeout)
{_STT();
	m_sData = "";

	// Sanity checks
	if ( m_dwInstServer == NULL || m_hConv == NULL ) return FALSE;

	// Get data
	HDDEDATA hData = DdeClientTransaction(	NULL, 0, m_hConv, 
											CDdeSz( m_dwInstServer, pName ),
											CF_TEXT, XTYP_REQUEST, dwTimeout, NULL );
	if ( hData == NULL ) return FALSE;

	// Get the data
	if ( dwTimeout != TIMEOUT_ASYNC )
	{
		// Get any data
		DWORD dwSize = DdeGetData( hData, NULL, 0, 0 );
		if ( !dwSize ) return TRUE;

		// Get the string
		LPSTR str = new CHAR[ dwSize + 1 ];
		if ( DdeGetData( hData, (LPBYTE)str, dwSize, 0 ) )
		{	m_sData = str; return TRUE; }

	} // end if

	return TRUE;
}

LPCTSTR CClientDde::GetData()
{_STT();
	return m_sData;
}

BOOL CClientDde::PokeData(LPCTSTR pName, LPBYTE pData, DWORD dwSize, DWORD dwTimeout)
{_STT();
	m_sData = "";

	// Sanity checks
	if ( m_dwInstServer == NULL || m_hConv == NULL ) return FALSE;

	// Poke data
	HDDEDATA hData = DdeClientTransaction(	pData, dwSize, m_hConv, 
											CDdeSz( m_dwInstServer, pName ),
											CF_TEXT, XTYP_POKE, dwTimeout, NULL );
	if ( hData == NULL ) return FALSE;

	// Get the data
	if ( dwTimeout != TIMEOUT_ASYNC )
	{
		// Get any data
		DWORD dwSize = DdeGetData( hData, NULL, 0, 0 );
		if ( !dwSize ) return TRUE;

		// Get the string
		LPSTR str = new CHAR[ dwSize + 1 ];
		if ( DdeGetData( hData, (LPBYTE)str, dwSize, 0 ) )
		{	m_sData = str; return TRUE; }

	} // end if

	return TRUE;
}


