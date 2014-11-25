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
// NetCmd.cpp: implementation of the CNetCmd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetCmd::CNetCmd()
{_STT();
	m_lError = 0;
}

CNetCmd::~CNetCmd()
{_STT();

}

BOOL CNetCmd::vMsg( const GUID *pNode, const GUID *pClass, DWORD dwFunction, CReg *pParams, DWORD dwBuffers, LPVOID *pArgs )
{_STT();
	// Verify we have authority to send this message
	if ( !OnTxAuthenticate( pNode, pClass, dwFunction ) )
		return FALSE;

	// Get destination
	CNetCom *pNc = GetNode( pNode );
	if ( pNc == NULL || !pNc->IsValid() ) 
		return FALSE;

	// Copy message data
	CNetMsg::SMsg msg;
	ZeroMemory( &msg, sizeof( msg ) );
	msg.dwFunction = dwFunction;
	
	// Encode params
	CPipe params;
	if ( pParams ) pParams->SaveRegFile( &params, NULL, "" );

	// Calculate the total size needed
	DWORD dwBlocks = 3; if ( params.GetBufferSize() ) dwBlocks++;
	DWORD dwTotalSize = ( sizeof( CNetMsg::SAddress ) * 2 ) +  
						sizeof( CNetMsg::SMsg ) +
						params.GetBufferSize();

	DWORD i;
	LPVOID *ptrExtra = pArgs;
	for ( i = 0; i < dwBuffers; i++ )
	{	RULIB_TRY 
		{	DWORD	dwType = *(LPDWORD)( ptrExtra );
			LPBYTE	pPtr = *(LPBYTE*)( ptrExtra + 1 );
			DWORD 	dwSize = *(LPDWORD)( ptrExtra + 2 );
			dwBlocks++;

			// Zero means NULL terminated
			if ( dwSize == 0 && pPtr != NULL ) dwSize = strlen( (LPCTSTR)pPtr );

			dwTotalSize += dwSize;

			ptrExtra += 3;
		} // end try
		RULIB_CATCH_ALL { return FALSE; }
	} // end for

	// Lock the tx buffer
	CTlLocalLock ll( *pNc->Tx() );
	if ( !ll.IsLocked() ) return FALSE;

	// Initialze the packet
	pNc->Tx()->InitPacket( NETMSG_PACKETTYPE, dwBlocks, dwTotalSize );

	CNetMsg::SAddress addr;

	// Destination information	
	memcpy( &addr.guidNode, pNc->Rx()->Address(), sizeof( GUID ) );
	if ( pClass ) memcpy( &addr.guidClass, pClass, sizeof( GUID ) );
	else ZeroMemory( &addr.guidClass, sizeof( addr.guidClass ) );
	pNc->Tx()->AddPacketData( NETMSGDT_DSTADDRESS, &addr, sizeof( addr ) );
	
	// Source information	
	memcpy( &addr.guidNode, pNc->Tx()->Address(), sizeof( GUID ) );
	ZeroMemory( &addr.guidClass, sizeof( addr.guidClass ) );
	pNc->Tx()->AddPacketData( NETMSGDT_SRCADDRESS, &addr, sizeof( addr ) );

	// Add message
	pNc->Tx()->AddPacketData( NETMSGDT_MSG, &msg, sizeof( msg ) );

	// Add params if any
	if ( params.GetBufferSize() )
		pNc->Tx()->AddPacketData( NETMSGDT_PARAMS, params.GetBuffer(), params.GetBufferSize() );

	// Add the data to the packet
	ptrExtra = pArgs;
	for ( i = 0; i < dwBuffers; i++ )
	{	RULIB_TRY 
		{	DWORD	dwType = *(LPDWORD)( ptrExtra );
			LPBYTE	pPtr = *(LPBYTE*)( ptrExtra + 1 );
			DWORD 	dwSize = *(LPDWORD)( ptrExtra + 2 );

			// Zero means NULL terminated
			if ( dwSize == 0 && pPtr != NULL ) dwSize = strlen( (LPCTSTR)pPtr );

			// Add packet data block
			pNc->Tx()->AddPacketData( dwType, pPtr, dwSize );

			ptrExtra += 3;
		} // end try
		RULIB_CATCH_ALL { return FALSE; }
	} // end for

	// Make it official
	return pNc->Tx()->EndPacket();
}

BOOL CNetCmd::NotifyConnect(long lError)
{_STT();
	// Save error code
	m_lError = lError;

	// Dispatch the message to interested parties
	return MMessage( NetCmd::efConnect, (LPARAM)this, &NetCmd::IID );
}

BOOL CNetCmd::NotifyDisconnect(long lError)
{_STT();
	// Save error code
	m_lError = lError;

	// Dispatch the message to interested parties
	return MMessage( NetCmd::efDisconnect, (LPARAM)this, &NetCmd::IID );
}


BOOL CNetCmd::LocalMsg(const GUID *pguidClass, DWORD dwFunction)
{_STT();
	// Dispatch the message to interested parties
	return MMessage( dwFunction, (LPARAM)this, pguidClass );
}
