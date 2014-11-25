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
// NetMsg.cpp: implementation of the CNetMsg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetMsg::CNetMsg()
{_STT();
	// Default to the broadcast address
	memcpy( &m_guidAddress, &CLSID_CNETMSG_BROADCAST, sizeof( m_guidAddress ) );
}

CNetMsg::~CNetMsg()
{_STT();

}

BOOL CNetMsg::VerifyPacket()
{_STT();
	do
	{
		// Verify a valid data packet
		if ( !CDataPacket::VerifyPacket() )
			return FALSE;

		// Verify the packet is for us
		SAddress dst;
		if ( ReadPacketData( 0, NETMSGDT_DSTADDRESS, &dst, sizeof( dst ) ) )
		{
			// Is this packet addressed to us?
			if ( IsEqualGUID( dst.guidNode, m_guidAddress ) ||
				 IsEqualGUID( dst.guidNode, CLSID_CNETMSG_BROADCAST ) )
				return TRUE;

		} // end if	

	// Look to the next packet
	} while ( SkipPacket() );

	return FALSE;
}

BOOL CNetMsg::ReadPacket( LPVOID pBuf, DWORD dwSize )
{_STT();
	// Process packets
	if ( CDataPacket::ReadPacket( pBuf, dwSize ) ) do
	{
		// Process this packet
		MSendMessage( eRx, (LPARAM)this );

	// Skip this packet
	} while ( SkipPacket() );

	return TRUE;
}

BOOL CNetMsg::OnWrite()
{_STT();
	MSendMessage( eTx, (LPARAM)this );

	return TRUE;
}

