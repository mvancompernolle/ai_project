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
// WinTimer.cpp: implementation of the CWinTimer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWinTimer::CWinTimer()
{_STTEX();
	m_hWnd = NULL;
	m_uId = 0;
	m_uEvent = 0;
	m_uValue = 0;
	m_pTimerProc = NULL;
}

CWinTimer::~CWinTimer()
{_STTEX();
	KillTimer();
}

BOOL CWinTimer::SetTimer(HWND hWnd, UINT uEvent, UINT uValue)
{_STTEX();
	KillTimer();
	m_uId = ::SetTimer( hWnd, uEvent, uValue, NULL );
	if ( m_uId == 0 ) return FALSE;

	m_hWnd = hWnd; 
	m_uEvent = uEvent; 
	m_uValue = uValue;
	m_pTimerProc = NULL;

	return TRUE;	
}

void CWinTimer::KillTimer()
{_STTEX();
	if ( m_uId != 0 )
	{	::KillTimer( m_hWnd, m_uId );
		m_uId = 0;
	} // end if
	m_hWnd = NULL;
	m_uEvent = 0;
	m_uValue = 0;
}

BOOL CWinTimer::SetTimer(UINT uValue, TIMERPROC pTimerProc)
{_STTEX();
	KillTimer();
	m_uId = ::SetTimer( NULL, NULL, uValue, pTimerProc );
	if ( m_uId == 0 ) return FALSE;

	m_uValue = uValue;
	m_pTimerProc = pTimerProc;

	return TRUE;	
}
