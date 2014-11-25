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
// NetDispatch.cpp: implementation of the CNetDispatch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetDispatch::CNetDispatch()
{_STT();
	m_pNc = NULL;
	memcpy( &m_guidClass, &CLSID_ZERO, sizeof( GUID ) );
}

CNetDispatch::CNetDispatch( CNetCmd *pNc )
{_STT();
	m_pNc = pNc;
}

CNetDispatch::~CNetDispatch()
{_STT();
	Destroy();
}

void CNetDispatch::Destroy()
{_STT();
	if ( m_pNc )
	{
		GUID *pGuid = NULL;
		if ( !IsEqualGUID( m_guidClass, CLSID_ZERO ) )
			pGuid = &m_guidClass;
		
		// Remove callback function
		m_pNc->RemoveCallbackFunction( CNetDispatch::NetMsg, this, pGuid );

		m_pNc = NULL;

	} // end if
}

long CNetDispatch::NetMsg( LPVOID pData, WPARAM wParam, LPARAM lParam )
{_STT();
	CNetDispatch* pNd = (CNetDispatch*)pData;
	if ( pNd == NULL ) return FALSE;

	RULIB_TRY
	{
		// Attempt callback
		return pNd->OnNetMsg( wParam, lParam );

	} RULIB_CATCH_ALL { ASSERT( 0 ); }

	return E_FAIL;
}

long CNetDispatch::OnNetMsg( WPARAM wParam, LPARAM lParam )
{_STT();
	CNetCom *pNc = (CNetCom*)lParam;
	if ( pNc == NULL ) return FALSE;

	// Attempt callback
	return OnMsg( wParam, pNc );
}

void CNetDispatch::SetNetwork(CNetCmd *pNc, const GUID *pguidClass)
{_STT();
	Destroy();

	m_pNc = pNc;

	// Set the guid
	if ( pguidClass ) memcpy( &m_guidClass, pguidClass, sizeof( GUID ) );
	else memcpy( &m_guidClass, &CLSID_ZERO, sizeof( GUID ) );

	// Want callbacks
	if ( m_pNc ) m_pNc->SetCallbackFunction( CNetDispatch::NetMsg, this, pguidClass );
	
}
