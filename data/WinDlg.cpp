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
// WinDlg.cpp: implementation of the CWinDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWinDlg::CWinDlg()
{_STT();
	m_bAutoCenter = TRUE;
	m_bCenterOnParent = TRUE;

	m_bRun = FALSE;
	m_nRet = 0;
	
	m_hWnd = NULL;
	m_hInst = NULL;
	m_pTemplate = NULL;

	m_hwndParent = NULL;

	m_bInitialized = FALSE;

	m_bModeless = TRUE;

	m_hAccel = NULL;
}

CWinDlg::~CWinDlg()
{_STT();
	DestroyWindow();
}

BOOL CWinDlg::OnInitDlg( HWND hwndFocus )
{_STT();
	// Center the dialog box
	if ( m_bAutoCenter ) CenterWindow();

	m_bInitialized = TRUE;

	return TRUE;
}

BOOL CWinDlg::OnCommand( WPARAM wParam, LPARAM lParam )
{_STT();
	switch ( LOWORD( wParam ) )
	{
		case IDOK:
			if ( HIWORD( wParam ) == BN_CLICKED )
				OnOk();
		break;

		case IDCANCEL:
			if ( HIWORD( wParam ) == BN_CLICKED )
				OnCancel();
		break;

		default:	
			return FALSE;
	} // end switch

	return TRUE;
}

void CWinDlg::OnOk()
{_STT();
	EndDlg( IDOK );
}

void CWinDlg::OnCancel()
{_STT();
	EndDlg( IDCANCEL );
}

BOOL CALLBACK CWinDlg::DlgProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam )
{_STT();
	// Special case for WM_INITDIALOG
	if (	uMessage == WM_INITDIALOG &&
			lParam != NULL )
	{
		// Save the dialogs hWnd
		( (CWinDlg*)lParam )->m_hWnd = hWnd;
		return ( (CWinDlg*)lParam )->OnInitDlg( (HWND)wParam );
	} // end if
	
	// Get a pointer to the dialog box class
	CWinDlg	*pDlg = (CWinDlg*)GetWindowLong( hWnd, DWL_USER );
	if ( pDlg == NULL ) return FALSE; // We're screwed if this actually happens

	switch ( uMessage )
	{
		case WM_SETFOCUS:
		case WM_ACTIVATE:
			pDlg->RedrawWindow();
			break;

		case WM_NCPAINT:
			pDlg->UpdateWindow();
			break;

		case WM_KEYDOWN:
			return pDlg->OnKeyDown( (int)wParam, (DWORD)lParam );
			
		case WM_COMMAND:
			return pDlg->OnCommand( wParam, lParam );

		case WM_ENDSESSION:
			return pDlg->OnEndSession( (BOOL)wParam, (DWORD)lParam );

		default: return pDlg->OnWMMessage( uMessage, wParam, lParam ); break;
	} // end switch

	return FALSE;
}

BOOL CWinDlg::OnWMMessage( UINT uMessage, WPARAM wParam, LPARAM lParam )
{_STT();
	return FALSE;
}

void CWinDlg::CenterWindow(HWND hWnd, HWND hParent, LPRECT pRect )
{_STT();
	RECT		rect;
	RECT		center;

	// Get our position
	if ( !GetWindowRect( hWnd, &rect ) ) return;

	// What are we centering to?

	// Parent window
	if ( hParent != NULL && GetWindowRect( hParent, &center ) );

	// User rectangle
	else if ( pRect != NULL ) ::CopyRect( &center, pRect );

	// The whole screen
	else ::SetRect( &center, 0, 0, 
					GetSystemMetrics( SM_CXSCREEN ), 
					GetSystemMetrics( SM_CYSCREEN ) );

	// Calculate x
	int	x = ( center.right - center.left ) - ( rect.right - rect.left );
	x /= 2; x += center.left;
	// Calculate y
	int	y = ( center.bottom - center.top ) - ( rect.bottom - rect.top );
	y /= 2; y += center.top;

	// Move the window
	SetPos( hWnd, x, y );
	
/*
	// Calculate the center position
	::SetRect(	&rect, x, y, 
				x + ( rect.right - rect.left ), 
				y + ( rect.bottom - rect.top ) );

	// Let's go
	MoveWindow(	hWnd, rect.left, rect.top,
				rect.right - rect.left,
				rect.bottom - rect.top,
				TRUE );
*/
}

int CWinDlg::DoModal( HINSTANCE hInst, HWND hParent, int nCmdShow )
{_STT();
	HWND hTemp;

	m_bModeless = TRUE;

	// Get the real parent window
	hTemp = hParent;
	do
	{
		hParent = hTemp;
		hTemp = ::GetParent( hParent );
	} while ( hTemp != NULL );
	
	// Disable the parent window
	if ( hParent != NULL ) EnableWindow( hParent, FALSE );

	// Create a dialog box
	if ( !CreateDlg( hInst, hParent ) )
	{
		m_bModeless = FALSE;
		return FALSE;
	} // end if

	// They probably want to see the dialog
	ShowWindow( nCmdShow );
	UpdateWindow();

	// Wait for the dialog box to close
//	while ( DoMessagePump( GetSafeHwnd(), GetAccel() ) && DoMessagePump() && m_bRun )
	while ( DoMessagePump() && m_bRun )
	{		
		// Do Idle processing
		if ( m_bInitialized && !OnIdle() )
			Sleep( 15 );
//			WaitMessage();
	} // end while

	// Re-enable the parent window
	if ( hParent != NULL ) 
	{
		::EnableWindow( hParent, TRUE );
	} // end if

	// We're done with the dialog
	DestroyWindow();

	// Not modal anymore
	m_bModeless = FALSE;

	// Tell them how it went
	return m_nRet;
}

void CWinDlg::DestroyWindow()
{_STT();
	if ( m_hWnd != NULL )
	{
		::DestroyWindow( m_hWnd );
		m_hWnd = NULL;
	} // end if

	m_bRun = FALSE;
}

BOOL CWinDlg::CreateDlg( HINSTANCE hInst, HWND hParent )
{_STT();
	// Destroy any current window
	DestroyWindow();

	m_bRun = TRUE;

	m_hInst = hInst;
	m_hwndParent = hParent;
	
	// Create the dialog box
	m_hWnd = CreateDialogParam (	m_hInst, m_pTemplate, 
									m_hwndParent, (DLGPROC)DlgProc,
									(LPARAM)this );

	// Clear the previous error
	SetLastError( 0 );
	// Give the dialog a pointer to the class
	if (	!SetWindowLong( m_hWnd, DWL_USER, (LONG)this ) &&
			GetLastError() != 0 )
	{
		// We must fail if we can't give the dialog our pointer
		DestroyWindow();
		return FALSE;
	} // end if

	// Did we make it?
	return IsWindow();
}

BOOL CWinDlg::DoMessagePump( HWND hWnd, HACCEL hAccel )
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
	}

	return TRUE;
}

BOOL CWinDlg::OnIdle()
{_STT();
	return FALSE;
}

void CWinDlg::EndDlg(int ret)
{_STT();
	m_nRet = ret;
	m_bRun = FALSE;

	// If we're not modal then destroy the dialog box
	if ( !m_bModeless ) DestroyWindow();
}

BOOL CWinDlg::OnKeyDown(int nVirtKey, DWORD dwKeyData)
{_STT();
	return FALSE;
}

BOOL CWinDlg::OnEndSession(BOOL bEndSession, DWORD fLogOff)
{_STT();
	return FALSE;
}

COLORREF	CWinDlg::m_rgbCustomColors[ 16 ];
BOOL CWinDlg::ChooseColor(COLORREF * pCol, HWND hOwner, DWORD dwFlags)
{_STT();
	CHOOSECOLOR	cc;

	if ( pCol == NULL ) return FALSE;

	// Set up the color info
	ZeroMemory( &cc, sizeof( CHOOSECOLOR ) );
	cc.lStructSize = sizeof( CHOOSECOLOR );
	cc.hwndOwner = hOwner;
	cc.rgbResult = *pCol;
	cc.lpCustColors = m_rgbCustomColors;
	cc.Flags = dwFlags;

	// Get the users color
	if ( !::ChooseColor( &cc ) ) return FALSE;

	// Save the resulting color
	*pCol = cc.rgbResult;

	return TRUE;	
}


