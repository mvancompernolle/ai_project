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
// StackTrace.cpp: implementation of the CStackTrace class.
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

#if defined( CStackTrace_USING_TLSAPI )

/// Tls API index
DWORD CStackTrace::m_tls_dwIndex = TlsAlloc();

#elif defined( CStackTrace_USING_TLS )

/// Thread specific stack pointer
_declspec( thread ) CStackTrace::CStack *CStackTrace::m_tls_pStack = NULL;

#endif

BOOL CStackTrace::m_bShutdown = FALSE;

/// Instance of stack trace
CStackTrace *CStackTrace::m_pst = NULL;

CStackTrace::CStackTrace()
{
}

CStackTrace::~CStackTrace()
{

}


BOOL CStackTrace::Save(LPCTSTR pFile)
{
	CTlLocalLock ll( m_lock );
	if ( !ll.IsLocked() ) return FALSE;



	return TRUE;
}

