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
// NetCom.cpp: implementation of the CNetCom class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetCom::CNetCom( CNetMsg *pRx, CNetMsg *pTx )
{_STT();
	// Get object pointers
	if ( pRx != NULL ) m_prx = pRx;
	else m_prx = _PTR_NEW CNetMsg();

	// Get object pointers
	if ( pTx != NULL ) m_ptx = pTx;
	else m_ptx = _PTR_NEW CNetMsg();

	// Want notifications of events
	m_prx->SetCallbackFunction( NetMsg, this );
	m_ptx->SetCallbackFunction( NetMsg, this );

	// Create a unique address for this class
	CoCreateGuid( *m_ptx );

}

CNetCom::~CNetCom()
{_STT();
	// Release tx/rx pins
	_PTR_DELETE( m_prx );
	_PTR_DELETE( m_ptx );

}

long CNetCom::NetMsg( LPVOID pData, WPARAM wParam, LPARAM lParam )
{_STT();
	// Get a pointer to the class
	CNetCom *pNc = (CNetCom*)pData;
	if ( pNc == NULL ) return FALSE;

	// Let the instance handle it
	return pNc->OnNetMsg( wParam, lParam );
}

long CNetCom::OnNetMsg(WPARAM wParam, LPARAM lParam)
{_STT();
	// Is it a message from the receiver?
	if ( wParam == CNetMsg::eRx && lParam == (LPARAM)m_prx ) 
		return OnRx();

	// Handle tx messages
	else if ( wParam == CNetMsg::eTx && lParam == (LPARAM)m_ptx ) 
		return OnTx();

	return FALSE;
}

BOOL CNetCom::OnRx()
{_STT();
	// Read dst
	if ( !m_prx->ReadPacketData( 0, NETMSGDT_DSTADDRESS, &m_conn.addrNext, sizeof( m_conn.addrNext ) ) )
		return FALSE;

	// Read src
	if ( !m_prx->ReadPacketData( 0, NETMSGDT_SRCADDRESS, &m_conn.addrThis, sizeof( m_conn.addrThis ) ) )
		return FALSE;
		
	// Read message data
	if ( !m_prx->ReadPacketData( 0, NETMSGDT_MSG, &m_msg, sizeof( m_msg ) ) )
		return FALSE;

	// Authenticate the connection
	if ( !OnRxAuthenticate( &m_conn.addrNext.guidNode, &m_conn.addrNext.guidClass, m_msg.dwFunction ) ) 
		return FALSE;

	// Hook NetCmd messages
	if ( IsEqualGUID( NetCmd::IID, m_conn.addrNext.guidClass ) )
	{
		// Look for remote information packet
		if ( m_msg.dwFunction == NetCmd::efGetConnectionInfo )
		{
			// Send connection information
			Msg( NULL, &NetCmd::IID, CNetMsg::fReply | NetCmd::efGetConnectionInfo, NULL,
					1, 1, m_ptx->GetConnectionDescription(), 0 );

		} // end if

		// Look for remote information packet
		else if ( m_msg.dwFunction == ( CNetMsg::fReply | NetCmd::efGetConnectionInfo ) )
		{
			// Read remote information
			char szDescription[ 512 ];
			if ( m_prx->ReadPacketString( 0, 1, szDescription, sizeof( szDescription ) ) )
				m_prx->SetConnectionDescription( szDescription );

		} // end if

	} // end if

	// Dispatch the message to interested parties
	return MMessage( m_msg.dwFunction, (LPARAM)this, &m_conn.addrNext.guidClass );
}
	 

BOOL CNetCom::GetParams(CReg *pParams, GUID *pGuid)
{_STT();
	if ( pParams == NULL ) return FALSE;

	// See if there are any params
	DWORD dwParams = 0; pParams->Destroy();
	if ( m_prx->ReadPacketData( 0, NETMSGDT_PARAMS, NULL, 0, &dwParams ) && dwParams )
	{	TMem< BYTE > buf;
		if (	buf.allocate( dwParams ) &&
				m_prx->ReadPacketData( 0, NETMSGDT_PARAMS, buf.ptr(), buf.size() ) )
			pParams->LoadRegFromMem( buf, buf.size() );
	} // end if

	// Get the hash for these params
	if ( pGuid ) GetParamsHash( pGuid );

	return TRUE;
}

BOOL CNetCom::VerifyParams(const GUID *pGuid)
{_STT();
	return m_prx->VerifyPacketData( 0, NETMSGDT_PARAMS, pGuid );
}

BOOL CNetCom::GetParamsHash(GUID *pGuid)
{_STT();
	return m_prx->GetPacketDataHash( pGuid, 0, NETMSGDT_PARAMS );
}

BOOL CNetCom::OnInitSession()
{_STT();
	// Request connection information
	return Msg( NULL, &NetCmd::IID, NetCmd::efGetConnectionInfo );
}



