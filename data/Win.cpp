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
// Win.cpp: implementation of the CWin class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWin::CWin()
{_STT();
	m_hWnd = NULL;

	m_pPrevWndProc = NULL;

	m_bAutoCenter = TRUE;
	m_bCenterOnParent = TRUE;

	m_pWin = NULL;						   

	m_hInst = NULL;

	m_hAtom = NULL;

	DefaultWNDCLASS();
	DefaultCREATESTRUCT();
}

CWin::~CWin()
{_STT();
	Destroy();
}

void CWin::Destroy()
{_STT();
	UnhookHwnd();
	DestroyWindow();
	UnregisterClass();
}

void CWin::CenterWindow(HWND hWnd, HWND hParent, LPRECT pRect )
{_STT();
	RECT		rect;
	RECT		center;

	// Get our position
	if ( !GetWindowRect( hWnd, &rect ) ) return;

	// What are we centering to?

	// Parent window
	if ( ::IsWindow( hParent ) && GetWindowRect( hParent, &center ) );

	// User rectangle
	else if ( pRect != NULL ) ::CopyRect( &center, pRect );

	// The whole screen
	else ::SetRect( &center, 0, 0, 
					GetSystemMetrics( SM_CXSCREEN ), 
					GetSystemMetrics( SM_CYSCREEN ) );

	// Coords are relative to parent client if child
	if ( ::IsWindow( hParent ) && GetWindowLong( hWnd, GWL_STYLE ) & WS_CHILD )
	{
		POINT	pt = { 0, 0 };
		ClientToScreen( hParent, &pt ); 
		OffsetRect( &center, -pt.x, -pt.y );
	} // end if

	// Calculate x
	int	x = ( center.right - center.left ) - ( rect.right - rect.left );
	x /= 2; x += center.left;
	// Calculate y
	int	y = ( center.bottom - center.top ) - ( rect.bottom - rect.top );
	y /= 2; y += center.top;

	// Move the window
	SetPos( hWnd, x, y );
}

LRESULT CALLBACK CWin::WindowProc(	HWND hWnd,      // handle to window
									UINT uMsg,      // message identifier
									WPARAM wParam,  // first message parameter
									LPARAM lParam ) // second message parameter
{_STT();
	BOOL	bSubclassing = FALSE;
	CWin	*pWin = NULL;

	if ( ::IsWindow( hWnd ) )
	{
		// Are we subclassing?
		pWin = (CWin*)GetProp( hWnd, "_CWin_Settings_Class_Pointer_050599_" );

	} // end if

	if ( pWin != NULL ) bSubclassing = TRUE;

	else
	{		
		// Special case for WM_CREATE
		if ( uMsg == WM_CREATE && lParam != NULL )
		{
			LPCREATESTRUCT	pCreate = (LPCREATESTRUCT)lParam;
			if ( pCreate == NULL ) return -1;

			// Save the dialogs hWnd
			( (CWin*)pCreate->lpCreateParams )->m_hWnd = hWnd;
			return ( (CWin*)pCreate->lpCreateParams )->OnCreate();
		} // end if

		// Get a pointer to the window class
		pWin = (CWin*)GetWindowLong( hWnd, GWL_USERDATA );
	} // end else

	// Did we make it?
	if ( pWin == NULL ) return DefWindowProc( hWnd, uMsg, wParam, lParam );

	// Allow processing
	if ( pWin->OnWMMessage( uMsg, wParam, lParam ) )
		return TRUE;

	switch ( uMsg )
	{
		case WM_SETFOCUS:
		case WM_ACTIVATE:
			pWin->RedrawWindow();
			break;

		case WM_COMMAND:
			if ( pWin->OnCommand( wParam, lParam ) )
				return 0;
			break;
	
		case WM_ERASEBKGND:
			if ( pWin->OnEraseBkGnd( (HDC)wParam ) )
				return TRUE;
			break;
	
		case WM_PAINT:
		{
			pWin->m_bEndPaint = FALSE;

			BOOL ret = pWin->OnPaint( (HDC)wParam );

			if ( pWin->m_bEndPaint ) 
			{
				pWin->m_bEndPaint = FALSE;
				::EndPaint( pWin->GetSafeHwnd(), &pWin->m_ps );
			} // end if

			if ( ret ) 
			{
				ValidateRect( pWin->GetSafeHwnd(), NULL );
				ValidateRgn( pWin->GetSafeHwnd(), NULL );
				return 0;
			} // end if
		} // end case
		break;

		case WM_CLOSE:
			pWin->Quit( 0 );
			pWin->DestroyWindow();
			break;
	
		default: break;
	} // end switch

	// Call the original procedure if we are subclassing.
	if ( bSubclassing ) return pWin->CallWindowProc( uMsg, wParam, lParam );

	// Just let windows handle it
	if ( ::IsWindow( hWnd ) ) return DefWindowProc( hWnd, uMsg, wParam, lParam );
	
	return 0;
} 


void CWin::DefaultWNDCLASS()
{_STT();
		m_wc.style 			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		m_wc.lpfnWndProc	= CWin::WindowProc;
		m_wc.cbClsExtra		= 0;
		m_wc.cbWndExtra		= 0;
		m_wc.hInstance		= GetInstance();
		m_wc.hIcon			= LoadIcon ( NULL, IDI_APPLICATION );
		m_wc.hCursor		= LoadCursor ( NULL, IDC_ARROW );
		m_wc.hbrBackground  = (HBRUSH)GetStockObject( WHITE_BRUSH );
		m_wc.lpszMenuName	= NULL;

		// Setup a unique class name
		GetUniqueString( m_classname, MAX_PATH, "WNDCLASS" );
		m_wc.lpszClassName  = m_classname;
}

BOOL CWin::RegisterClass()
{_STT();
	m_hAtom = ::RegisterClass( &m_wc ); 

	return ( m_hAtom != NULL );
}


void CWin::DefaultCREATESTRUCT()
{_STT();
	m_cs.lpCreateParams = NULL;
	m_cs.hInstance = GetInstance();
	m_cs.hMenu = NULL;
	m_cs.hwndParent = NULL;
	m_cs.cy = 0;
	m_cs.cx = 0;
	m_cs.y = 200;
	m_cs.x = 200;
	m_cs.style = WS_OVERLAPPED;
	m_cs.lpszName = "";
	m_cs.lpszClass = m_wc.lpszClassName;
	m_cs.dwExStyle = 0;
}

BOOL CWin::Create( LPCTSTR pTitle )
{_STT();
	// Lose any old window
	DestroyWindow();
	
	m_hWnd = CreateWindowEx(	m_cs.dwExStyle,
								m_cs.lpszClass,
								m_cs.lpszName,
								m_cs.style,
								m_cs.x,
								m_cs.y,
								m_cs.cx,
								m_cs.cy,
								m_cs.hwndParent,
								m_cs.hMenu,
								m_cs.hInstance,
								this );

	if ( m_hWnd == NULL ) return FALSE;
/*
	// Clear the previous error
	SetLastError( 0 );
	// Give the window a pointer to the class
	if (	!SetWindowLong( m_hWnd, GWL_USERDATA, (LONG)this ) &&
			GetLastError() != 0 )
	{
		return -1;
	} // end if
*/
	return TRUE;
}


BOOL CWin::OnCommand(WPARAM wParam, LPARAM lParam)
{_STT();
	return FALSE;
}

BOOL CWin::GetUniqueString(LPSTR buf, DWORD max, LPCTSTR prefix)
{_STT();
	char			temp[ MAX_PATH ];
	static DWORD	inst = 0;

	// Are we doomed from the start?
	if ( strlen( prefix ) > max ) return FALSE;

	// Create a unique string
	wsprintf( temp, "_%s_%lu_%lu", "UNIQUE", (ULONG)inst++, (ULONG)GetTickCount() );

	strcpy( buf, prefix );
	if ( ( strlen( temp ) + strlen( prefix ) ) < max )
		strcat( buf, temp );

	else return FALSE;

	return TRUE;
}

LRESULT CWin::OnCreate()
{_STT();
	// Clear the previous error
	SetLastError( 0 );
	// Give the window a pointer to the class
	if (	!SetWindowLong( GetSafeHwnd(), GWL_USERDATA, (LONG)this ) &&
			GetLastError() != 0 )
	{
		return -1;
	} // end if

	return 0;
}

void CWin::DestroyWindow()
{_STT();
	if ( m_hWnd != NULL )
	{
		if ( m_pPrevWndProc != NULL ) UnhookHwnd();
		else ::DestroyWindow( m_hWnd );
		m_hWnd = NULL;
	} // end if
}

BOOL CWin::OnEraseBkGnd(HDC hDC)
{_STT();
	return FALSE;
}

BOOL CWin::OnPaint(HDC hDC)
{_STT();
	return FALSE;
}

HDC CWin::BeginPaint()
{_STT();
	HDC hDC = ::BeginPaint( GetSafeHwnd(), &m_ps );

	if ( hDC != NULL ) 	m_bEndPaint = TRUE;

	return hDC;
}

BOOL CWin::OnWMMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{_STT();
	return FALSE;
}

BOOL CWin::HookHwnd( HWND hWnd )
{_STT();
	// Lose last window
	Destroy();

	// Do we have a valid window?
	if ( !::IsWindow( hWnd ) ) return FALSE;

	// Attach
	Attach( hWnd );

	// Set a pointer to our class
	if ( !SetProp( m_hWnd, "_CWin_Settings_Class_Pointer_050599_", (HANDLE)this ) )
	{	Detach(); return FALSE; }

	// Tell the window to call us instead
	SetLastError( 0 );
	m_pPrevWndProc = (WNDPROC)SetWindowLong( hWnd, GWL_WNDPROC, (LONG)CWin::WindowProc );
	if ( m_pPrevWndProc == 0 || GetLastError() != 0 )
	{	Detach(); return FALSE; }

	return TRUE;
}

BOOL CWin::UnhookHwnd()
{_STT();
	if ( m_pPrevWndProc == NULL ) return TRUE;

	if ( !::IsWindow( m_hWnd ) ) 
	{
		m_pPrevWndProc = NULL;
		m_hWnd = NULL;
		return TRUE;
	} // end if

	// Set the old window procedure pointer
	SetWindowLong( m_hWnd, GWL_WNDPROC, (LONG)m_pPrevWndProc );

	// Remove our class info
	RemoveProp( m_hWnd, "_CWin_Settings_Class_Pointer_050599_" );
			
	// Detach from window
	Detach();

	return TRUE;
}

LRESULT CWin::CallWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{_STT();
	if ( !IsWindow() ) return NULL;

	if ( m_pPrevWndProc == NULL ) return FALSE;

	return ::CallWindowProc( m_pPrevWndProc, m_hWnd, uMsg, wParam, lParam );
}


BOOL CWin::DoMessagePump(HWND hWnd, HACCEL hAccel)
{_STT();
	while ( PumpMessage( hWnd, hAccel ) )
	{		
		DWORD dwCalls = 0;

		// Do Idle processing
		if (  !OnIdle( dwCalls++ ) ) Sleep( 15 );
//			WaitMessage();

	} // end while

	return TRUE;
}

BOOL CWin::PumpMessage(HWND hWnd, HACCEL hAccel)
{_STT();
	MSG		msg;

	// See if a message is waiting
	if ( PeekMessage( &msg, hWnd, 0, 0, PM_NOREMOVE ) )
 	{
		// Get if from the queue
		if( GetMessage( &msg, hWnd, 0, 0 ) )
		{
			// Give the dialog box a chance at this message
			if ( !IsDialogMessage( msg.hwnd, &msg ) )
			{
				// Translate the message
				if ( hAccel == NULL || !TranslateAccelerator( msg.hwnd, hAccel, &msg ) )
					TranslateMessage ( &msg );

				// Dispatch the message
				DispatchMessage ( &msg );
			} // end if
		} // end else
		else return FALSE;
	} // end if

	return TRUE;
}

BOOL CWin::PumpMessages(HWND hWnd, HACCEL hAccel)
{_STT();
	MSG		msg;

	// See if a message is waiting
	while ( PeekMessage( &msg, hWnd, 0, 0, PM_NOREMOVE ) )
 	{
		// Get if from the queue
		if( GetMessage( &msg, hWnd, 0, 0 ) )
		{
			// Give the dialog box a chance at this message
			if ( !IsDialogMessage( msg.hwnd, &msg ) )
			{
				// Translate the message
				if ( hAccel == NULL || !TranslateAccelerator( msg.hwnd, hAccel, &msg ) )
					TranslateMessage ( &msg );

				// Dispatch the message
				DispatchMessage ( &msg );
			} // end if
		} // end else
		else return FALSE;
	} // end while

	return TRUE;
}

BOOL CWin::OnIdle(DWORD dwCalls)
{_STT();
	return FALSE;
}


void CWin::Quit( int nRet )
{_STT();
	DestroyWindow();
	PostQuitMessage( nRet );
}

void CWin::UnregisterClass()
{_STT();
	if ( m_hAtom != NULL )
	{
		::UnregisterClass( (LPCTSTR)MAKELONG( m_hAtom, 0 ), GetInstance() );
		m_hAtom = NULL;
	} // end if
}

LRESULT CWin::OnWindowProc(BOOL bSubclassing, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{_STT();
	// Allow processing
	if ( OnWMMessage( uMsg, wParam, lParam ) )
		return TRUE;

	switch ( uMsg )
	{
		case WM_SETFOCUS:
		case WM_ACTIVATE:
			RedrawWindow();
			break;

		case WM_COMMAND:
			if ( OnCommand( wParam, lParam ) )
				return 0;
			break;
	
		case WM_ERASEBKGND:
			if ( OnEraseBkGnd( (HDC)wParam ) )
				return TRUE;
			break;
	
		case WM_PAINT:
		{
			m_bEndPaint = FALSE;

			BOOL ret = OnPaint( (HDC)wParam );

			if ( m_bEndPaint ) 
			{
				m_bEndPaint = FALSE;
				::EndPaint( GetSafeHwnd(), &m_ps );
			} // end if

			if ( ret ) 
			{
				ValidateRect( GetSafeHwnd(), NULL );
				ValidateRgn( GetSafeHwnd(), NULL );
				return 0;
			} // end if
		} // end case
		break;

		case WM_CLOSE:
			Quit( 0 );
			DestroyWindow();
			break;
	
		default: break;
	} // end switch

	// Call the original procedure if we are subclassing.
	if ( bSubclassing ) return CallWindowProc( uMsg, wParam, lParam );

	// Just let windows handle it
	if ( ::IsWindow( hWnd ) ) return DefWindowProc( hWnd, uMsg, wParam, lParam );
	
	return 0;
}
