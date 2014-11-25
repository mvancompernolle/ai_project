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
// Tooltip.cpp: implementation of the CTooltip class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <shellapi.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTooltip::CTooltip()
{_STT();
	m_hWnd = NULL;
	m_hParent = NULL;
}

CTooltip::~CTooltip()
{_STT();
	Destroy();
}

BOOL CTooltip::Create(HWND hWnd, HINSTANCE hInstance )
{_STT();
	// Lose old tooltip window
	Destroy();

	// Create tooltip window
    m_hWnd = CreateWindowEx(	WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
								TTS_NOPREFIX | TTS_ALWAYSTIP,		
								CW_USEDEFAULT, CW_USEDEFAULT, 
								CW_USEDEFAULT, CW_USEDEFAULT,
								hWnd, NULL, hInstance, NULL );

	// Verify that we got the window
	if ( m_hWnd == NULL || !::IsWindow( m_hWnd ) )
		return FALSE;

	// Set tip position
    SetWindowPos(	m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, 
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

	m_hParent = hWnd;

	return TRUE;
}

void CTooltip::Destroy()
{_STT();
	// Lose the tool tip control
	if ( m_hWnd != NULL && ::IsWindow( m_hWnd ) )
		::DestroyWindow( m_hWnd );

	m_hWnd = NULL;
	m_hParent = NULL;
}

BOOL CTooltip::Add(LPRECT pRect, LPCTSTR pText, DWORD dwFlags)
{_STT();
	// Ensure tooltip control
	if ( m_hWnd == NULL || !::IsWindow( m_hWnd ) )
		return FALSE;
	
	// Set tooltip params
    TOOLINFO ti;
	ZeroMemory( &ti, sizeof( ti ) );
    ti.cbSize = sizeof( ti );
    ti.uFlags = dwFlags;
    ti.hwnd = m_hParent;
    ti.lpszText = (char*)pText;

	// Init tooltip rect
    ti.rect.left = pRect->left;    
    ti.rect.top = pRect->top;
    ti.rect.right = pRect->right;
    ti.rect.bottom = pRect->bottom;
    
	// Add tooltip
    BOOL ret = (BOOL)SendMessage( m_hWnd, TTM_ADDTOOL, 0, (LPARAM)&ti );	

	return ret;
}

BOOL CTooltip::Add(UINT uId, LPCTSTR pText, DWORD dwFlags)
{_STT();
	// Ensure tooltip control
	if ( m_hWnd == NULL || !::IsWindow( m_hWnd ) )
		return FALSE;
	
	// Set tooltip params
    TOOLINFO ti;
	ZeroMemory( &ti, sizeof( ti ) );
    ti.cbSize = sizeof( ti );
    ti.uFlags = dwFlags;
    ti.hwnd = m_hParent;
    ti.lpszText = (char*)pText;
	ti.uFlags |= TTF_IDISHWND;
	ti.uId = (UINT)GetDlgItem( m_hParent, uId );
    
	// Add tooltip
    BOOL ret = (BOOL)SendMessage( m_hWnd, TTM_ADDTOOL, 0, (LPARAM)&ti );	

	return ret;
}




BOOL CTooltip::TrackActivate(UINT uId, BOOL bActivate)
{_STT();
	// Ensure tooltip control
	if ( m_hWnd == NULL || !::IsWindow( m_hWnd ) )
		return FALSE;
	
	// Set tooltip params
    TOOLINFO ti;
	ZeroMemory( &ti, sizeof( ti ) );
    ti.cbSize = sizeof( ti );
    ti.hwnd = m_hParent;
	ti.uId = (UINT)GetDlgItem( m_hParent, uId );
    
	// Add tooltip
    BOOL ret = (BOOL)SendMessage( m_hWnd, TTM_TRACKACTIVATE, bActivate, (LPARAM)&ti );	

	return ret;
}

BOOL CTooltip::Track(long x, long y)
{_STT();
	// Add tooltip
    BOOL ret = (BOOL)SendMessage( m_hWnd, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG( x, y ) );	

	return ret;
}

BOOL CTooltip::RelayEvent(LPMSG pMsg)
{_STT();
	// Add tooltip
    BOOL ret = (BOOL)SendMessage( m_hWnd, TTM_RELAYEVENT , 0, (LPARAM)pMsg );	

	return ret;
}

BOOL CTooltip::RelayEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DWORD dwTime, POINT pt)
{_STT();
	MSG msg; ZeroMemory( &msg, sizeof( msg ) );
	msg.hwnd = hWnd;
	msg.message = uMsg;
	msg.wParam = wParam;
	msg.lParam = lParam;
	msg.time = dwTime;
	msg.pt = pt;

	return RelayEvent( &msg );
}

BOOL CTooltip::UpdateText(UINT uId, LPCTSTR pText)
{_STT();
	// Ensure tooltip control
	if ( m_hWnd == NULL || !::IsWindow( m_hWnd ) )
		return FALSE;
	
	// Set tooltip params
    TOOLINFO ti;
	ZeroMemory( &ti, sizeof( ti ) );
    ti.cbSize = sizeof( ti );
    ti.hwnd = m_hParent;
	ti.uId = (UINT)GetDlgItem( m_hParent, uId );
	ti.hinst = GetModuleHandle( NULL );
	ti.lpszText = (char*)pText;
    
	// Add tooltip
    BOOL ret = (BOOL)SendMessage( m_hWnd, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti );	

	return ret;
}
