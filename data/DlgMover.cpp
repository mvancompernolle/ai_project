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
// DlgMover.cpp: implementation of the CDlgMover class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include <shellapi.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgMover::CDlgMover()
{_STT();
	m_hWnd = NULL;

	m_xDock = 0;
	m_yDock = 0;
	m_bSizeDock = FALSE;

	m_bDocked = FALSE;
	m_dwDockPos = 0;
	m_hFrame = NULL;

	m_hAppBar = NULL;
	m_uAppBarMsg = 0;
	m_bAppBar = FALSE;

	m_dwDockPosOnClick = 0;

	ZeroMemory( &m_rSize, sizeof( m_rSize ) );
	ZeroMemory( &m_rDockedSize, sizeof( m_rDockedSize ) );

	m_dwSize = 0;
	m_ptMove.x = 0; m_ptMove.y = 0;
	m_bResized = FALSE;

	m_bButtonDown = FALSE;

	m_szDocking.cx = 60;
	m_szDocking.cy = 60;

	m_ptDrag.x = MAXLONG;
	m_ptDrag.y = MAXLONG;

	m_szMinTrackSize.cx = 32;
	m_szMinTrackSize.cy = 32;

	m_szMaxTrackSize.cx = MAXLONG;
	m_szMaxTrackSize.cy = MAXLONG;

	m_lxEdge = 4;
	m_lyEdge = 4;

	m_dwAllowDocking = DM_DOCKALL;
}

CDlgMover::~CDlgMover()
{_STT();
	DestroyAppBar();
}


void CDlgMover::OnLButtonDown(HWND hWnd, POINT point, HWND hCapture)
{_STT();
	if ( !::IsWindow( hWnd ) ) return;

	// The button was clicked
	m_bButtonDown = TRUE;

	// Save the current docking status
	m_dwDockPosOnClick = m_dwDockPos;

	// Get cursor position
	ClientToScreen( hWnd, &point );

	// Save this point
	m_point.x = point.x;
	m_point.y = point.y;

	// Capture all mouse movements
	if ( hCapture != NULL ) ::SetCapture( hCapture );
	else ::SetCapture( hWnd );

	// Remember the window that cares
	m_hWnd = hWnd;

	// See if it is a resize request
	if ( m_dwSize && SetSizeCursor( point, TRUE ) ) return;

	// Refresh window position if not docked
	if ( !m_bSizeDock || !m_bDocked )
		GetWindowRect( m_hWnd, &m_rSize );

	// Undock window
	else KillAppBar();

	if ( m_bSizeDock && m_bDocked )
	{
		long ow = m_rSize.right - m_rSize.left;
		long oh = m_rSize.bottom - m_rSize.top;

		RECT rect;
		GetWindowRect( m_hWnd, &rect );

		// Set undocking click point
		if ( m_ptDrag.x == MAXLONG )
		{	m_rSize.left = point.x - ( RW( m_rUndockedSize ) >> 1 );
			m_rSize.right = m_rSize.left + RW( m_rUndockedSize );
		} // end if
		else
		{	m_rSize.left = point.x - m_ptDrag.x;
			m_rSize.right = m_rSize.left + RW( m_rUndockedSize );
		} // end else

		if ( m_ptDrag.y == MAXLONG )
		{	m_rSize.top = point.y - ( RH( m_rUndockedSize ) >> 1 );
			m_rSize.bottom = m_rSize.top + RH( m_rUndockedSize );
		} // end if
		else
		{	m_rSize.top = point.y - m_ptDrag.y;
			m_rSize.bottom = m_rSize.top + RH( m_rUndockedSize );
		} // end else

	} // end else

}

BOOL CDlgMover::OnLButtonUp()
{_STT();
	if ( m_hWnd == NULL || !IsWindow( m_hWnd ) ) return FALSE;

	// The button is up
	m_bButtonDown = FALSE;

	// Handle Appbar
	if ( m_hAppBar != NULL && m_bSizeDock )
	{
		// Kill Appbar if not un-docking
		if ( m_dwDockPos == 0 ) KillAppBar();

		// Just need a resize?
		else if ( m_bResized && m_dwDockPos == m_dwDockPosOnClick )
		{
			RECT rect;
			GetAppBarPos( &rect );
			SetAppBarPos( &rect );

			// Save new docked size
			if ( ( m_dwDockPos & ( DM_DOCKLEFT | DM_DOCKRIGHT ) ) != 0 ) 
				m_szDocking.cx = RW( rect );
			else if ( ( m_dwDockPos & ( DM_DOCKTOP | DM_DOCKBOTTOM ) ) != 0 ) 
				m_szDocking.cy = RH( rect );

		} // end if

		// Set the Appbar
		else 
		{	
			RECT rect;
			CopyRect( &rect, &m_rDockedSize );
			CreateAppBar( &rect );
		} // end else

		// Clear resized flag
		m_bResized = FALSE;

	} // end if

	// Release the users mouse
	::ReleaseCapture();

	return TRUE;
}

BOOL CDlgMover::OnMouseMove(UINT nFlags, POINT point)
{_STT();
	// Quit if invalid window or left button is up
	if ( m_hWnd == NULL || !IsWindow( m_hWnd ) ) return FALSE;

	// Get screen position
	ClientToScreen( m_hWnd, &point );

	// Check for button up
	if ( ( nFlags & MK_LBUTTON ) == 0 ) m_bButtonDown = FALSE;

	// See if it is a resize request
	if ( m_dwSize && SetSizeCursor( point, m_bButtonDown ) )
		return TRUE;

	// Punt if button not pressed
	if ( !m_bButtonDown ) return FALSE;

	// Has cursor position changed?
	if ( point.x == m_point.x && point.y == m_point.y ) return FALSE;

	// Calculate the new position
	POINT	pt;
	pt.x = m_rSize.left + point.x - m_point.x;
	pt.y = m_rSize.top + point.y - m_point.y;

	// Dock window if needed
	if ( m_xDock > 0 || m_yDock > 0 )
		m_bDocked = Dock( m_hWnd, &pt, &point );

	// Set window position if not docked
	if ( !m_bSizeDock || !m_bDocked ) SetPos( m_hWnd, pt );

	return TRUE;
}

BOOL CDlgMover::SetPos(HWND hWnd, POINT point)
{_STT();
	if ( !::IsWindow( hWnd ) ) return FALSE;

	// Restore the location of the dialog box
	return 
		::SetWindowPos(	hWnd,
						NULL,
						point.x,
						point.y,
						0, 0,
						SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
	
}

BOOL CDlgMover::GetPos(HWND hWnd, POINT * pPoint)
{_STT();
	if ( !::IsWindow( hWnd ) ) return FALSE;
	if ( pPoint == NULL ) return FALSE;

	RECT	rect;

	GetWindowRect( hWnd, &rect );
	pPoint->x = rect.left;
	pPoint->y = rect.top;

	return TRUE;
}


BOOL CDlgMover::Dock(HWND hWnd, LPPOINT pt, LPPOINT cursor)
{_STT();
	RECT work;

	// Sanity check
	if ( pt == NULL || cursor == NULL ) return FALSE;

	// Save undocked position
	if ( m_dwDockPos == 0 )
	{	GetWindowRect( hWnd, &m_rUndockedSize );
	} // end if
			
	// Is there a frame window to dock to?
	if ( m_hFrame != NULL && ::IsWindow( m_hFrame ) )
		GetWindowRect( m_hFrame, &work );

	// Get workspace area
	else if ( !SystemParametersInfo( SPI_GETWORKAREA, 0, (PVOID)&work, 0 ) )
		SetRect(	&work, 0, 0,
					GetSystemMetrics( SM_CXFULLSCREEN ),
					GetSystemMetrics( SM_CYFULLSCREEN ) );

	// Get original size
	long w = RW( m_rUndockedSize );
	long h = RH( m_rUndockedSize );

	// Window border sizes
	long xborder = GetSystemMetrics( SM_CXDLGFRAME );
	long yborder = GetSystemMetrics( SM_CYDLGFRAME );

	// Save old dock position
	DWORD olddockpos = m_dwDockPos;

	// Clear docked status
	m_dwDockPos = 0;

	// Where should it dock?
	if ( pt->x < ( work.left + m_xDock ) ) m_dwDockPos |= DM_DOCKLEFT;
	if ( ( pt->x + w ) > ( work.right - m_xDock ) ) m_dwDockPos |= DM_DOCKRIGHT;
	if ( pt->y < ( work.top + m_yDock ) ) m_dwDockPos |= DM_DOCKTOP;
	if ( ( pt->y + h ) > ( work.bottom - m_yDock ) ) m_dwDockPos |= DM_DOCKBOTTOM;
	
	// Ensure allowed
	m_dwDockPos &= m_dwAllowDocking;

	// Adjust the point for horz dock
	if ( ( m_dwDockPos & DM_DOCKLEFT ) != 0 ) pt->x = work.left - xborder;
	else if ( ( m_dwDockPos & DM_DOCKRIGHT ) != 0 )
	{	if ( m_bSizeDock ) pt->x = work.right - m_szDocking.cx + xborder;
		else pt->x = work.right - RW( m_rUndockedSize );
	} // end else if

	// Adjust the point for vert dock
	if ( ( m_dwDockPos & DM_DOCKTOP ) != 0 ) pt->y = work.top - yborder;
	else if ( ( m_dwDockPos & DM_DOCKBOTTOM ) != 0 ) 
	{	if ( m_bSizeDock ) pt->y = work.bottom - m_szDocking.cy + yborder;
		else pt->y = work.bottom - RH( m_rUndockedSize );
	} // end else if

	// Resize docked window if needed
	if ( m_bSizeDock && olddockpos != m_dwDockPos )
	{		
		RECT	rect;

		// Check if we need to resize
		if ( m_dwDockPos != 0 )
		{
			SetRect( &rect, pt->x, pt->y, pt->x + m_szDocking.cx, pt->y + m_szDocking.cy );

			// Dock to side first
			if ( ( m_dwDockPos & DM_DOCKLEFT ) != 0 || ( m_dwDockPos & DM_DOCKRIGHT ) != 0 )
				rect.top = work.top - yborder,  rect.bottom = work.bottom - work.top + yborder;

			// Dock top or bottom
			else if ( ( m_dwDockPos & DM_DOCKTOP ) != 0 || ( m_dwDockPos & DM_DOCKBOTTOM ) != 0 )
				rect.left = work.left - xborder, rect.right = work.right - work.left + xborder;

			// Copy docked position
			CopyRect( &m_rDockedSize, &rect );

		} // end if

		// Set undocked position
		else SetRect( &rect, pt->x, pt->y, pt->x + RW( m_rUndockedSize ), pt->y + RH( m_rUndockedSize ) );

		// Minimum size
		if ( RW( rect ) < m_szMinTrackSize.cx ) rect.right = rect.left + m_szMinTrackSize.cx;
		if ( RH( rect ) < m_szMinTrackSize.cy ) rect.bottom = rect.top + m_szMinTrackSize.cy;

		// Maximum size
		if ( RW( rect ) > m_szMaxTrackSize.cx ) rect.right = rect.left + m_szMaxTrackSize.cx;
		if ( RH( rect ) > m_szMaxTrackSize.cy ) rect.bottom = rect.top + m_szMaxTrackSize.cy;

		// Set the new window position
		SetWindowPos( hWnd, NULL, rect.left, rect.top,
							rect.right - rect.left,
							rect.bottom - rect.top,
							SWP_NOZORDER | SWP_NOACTIVATE );

	} // end if	
	
	return ( m_dwDockPos != 0 );
}

BOOL CDlgMover::InitAppBar( HWND hWnd, UINT uMsg )
{_STT();
	// Lose old appbar
	KillAppBar();

	m_hAppBar = hWnd;
	m_uAppBarMsg = uMsg;

	return TRUE;
}

void CDlgMover::KillAppBar()
{_STT();
	if ( m_hAppBar == NULL ) return;

	APPBARDATA abd;

	ZeroMemory( &abd, sizeof( abd ) );

	abd.cbSize = sizeof( abd );
	abd.hWnd = m_hAppBar;
	
	SHAppBarMessage( ABM_REMOVE, &abd );

}

void CDlgMover::DestroyAppBar()
{_STT();
	KillAppBar();
	m_hAppBar = NULL;
}

BOOL CDlgMover::CreateAppBar(LPRECT pRect)
{_STT();
	// Lose old appbar
	KillAppBar();

	// Ensure valid window
	if ( m_hAppBar == NULL || !IsWindow( m_hAppBar ) ) 
		return FALSE;

	APPBARDATA abd;

	ZeroMemory( &abd, sizeof( abd ) );

	abd.cbSize = sizeof( abd );
	abd.hWnd = m_hAppBar;
	abd.uCallbackMessage = m_uAppBarMsg;
	
	// Register new appbar
	if ( !SHAppBarMessage( ABM_NEW, &abd ) ) return FALSE;

	// Set rect position if needed
	if ( pRect != NULL ) SetAppBarPos( pRect );

	return TRUE;
}

BOOL CDlgMover::SetAppBarPos(LPRECT pRect)
{_STT();
	APPBARDATA abd;

	ZeroMemory( &abd, sizeof( abd ) );
	abd.cbSize = sizeof( abd );
	abd.hWnd = m_hAppBar;

	// Which edge?
	if ( ( m_dwDockPos & DM_DOCKLEFT ) != 0 ) abd.uEdge = ABE_LEFT;
	else if ( ( m_dwDockPos & DM_DOCKRIGHT ) != 0 ) abd.uEdge = ABE_RIGHT;
	else if ( ( m_dwDockPos & DM_DOCKTOP ) != 0 ) abd.uEdge = ABE_TOP;
	else if ( ( m_dwDockPos & DM_DOCKBOTTOM ) != 0 ) abd.uEdge = ABE_BOTTOM;

	// Copy the rect
	CopyRect( &abd.rc, pRect );

	// Set app bar position
	SHAppBarMessage( ABM_SETPOS, &abd );

	// Where does Windows think we should be?
	CopyRect( pRect, &abd.rc );

	return TRUE;
}


LRESULT CDlgMover::OnAppBarMsg(WPARAM wParam, LPARAM lParam)
{_STT();
	switch( wParam )
	{
		case ABN_POSCHANGED :
			
			if ( m_hAppBar != NULL && m_dwDockPos != 0 )
			{
				RECT rect;
				GetAppBarPos( &rect );

				// Minimum size
				if ( RW( rect ) < m_szMinTrackSize.cx ) rect.right = rect.left + m_szMinTrackSize.cx;
				if ( RH( rect ) < m_szMinTrackSize.cy ) rect.bottom = rect.top + m_szMinTrackSize.cy;

				// Maximum size
				if ( RW( rect ) > m_szMaxTrackSize.cx ) rect.right = rect.left + m_szMaxTrackSize.cx;
				if ( RH( rect ) > m_szMaxTrackSize.cy ) rect.bottom = rect.top + m_szMaxTrackSize.cy;

				// Set the new window position
				SetWindowPos( m_hWnd, NULL, rect.left, rect.top,
									rect.right - rect.left,
									rect.bottom - rect.top,
									SWP_NOZORDER | SWP_NOACTIVATE );

			} // end case
			break;

		default : break;

	} // end switch
	
	return 0;
}

BOOL CDlgMover::GetAppBarPos(LPRECT pRect)
{_STT();
	// Ensure valid window
	if ( m_hAppBar == NULL || !IsWindow( m_hAppBar ) ) 
		return FALSE;

	APPBARDATA abd;

	ZeroMemory( &abd, sizeof( abd ) );
	abd.cbSize = sizeof( abd );
	abd.hWnd = m_hAppBar;

	RECT rect;
	GetWindowRect( m_hWnd, &rect );

	// Which edge?
	if ( ( m_dwDockPos & DM_DOCKLEFT ) != 0 )
	{	abd.uEdge = ABE_LEFT;
		m_rDockedSize.left = rect.left;
		m_rDockedSize.right = rect.right;
	} // end if
	else if ( ( m_dwDockPos & DM_DOCKRIGHT ) != 0 ) 
	{	abd.uEdge = ABE_RIGHT;
		m_rDockedSize.left = rect.left;
		m_rDockedSize.right = rect.right;
	} // end else if
	else if ( ( m_dwDockPos & DM_DOCKTOP ) != 0 ) 
	{	abd.uEdge = ABE_TOP;
		m_rDockedSize.top = rect.top;
		m_rDockedSize.bottom = rect.bottom;
	} // end else if
	else if ( ( m_dwDockPos & DM_DOCKBOTTOM ) != 0 ) 
	{	abd.uEdge = ABE_BOTTOM;
		m_rDockedSize.top = rect.top;
		m_rDockedSize.bottom = rect.bottom;
	} // end else if

	// Copy the rect
	CopyRect( &abd.rc, &m_rDockedSize );

	// Set app bar position
	SHAppBarMessage( ABM_QUERYPOS, &abd );

	// Where does Windows think we should be?
	CopyRect( pRect, &abd.rc );

	return TRUE;
}

BOOL CDlgMover::OnMove(int x, int y)
{_STT();
	// Are we changing system work area?
	if ( m_hAppBar == NULL || m_dwDockPos == 0 || !m_bSizeDock ) 
		return FALSE;

	RECT rect;
	GetAppBarPos( &rect );

	// Minimum size
	if ( RW( rect ) < m_szMinTrackSize.cx ) rect.right = rect.left + m_szMinTrackSize.cx;
	if ( RH( rect ) < m_szMinTrackSize.cy ) rect.bottom = rect.top + m_szMinTrackSize.cy;

	// Maximum size
	if ( RW( rect ) > m_szMaxTrackSize.cx ) rect.right = rect.left + m_szMaxTrackSize.cx;
	if ( RH( rect ) > m_szMaxTrackSize.cy ) rect.bottom = rect.top + m_szMaxTrackSize.cy;

	// Set the new window position
	SetWindowPos( m_hWnd, NULL, rect.left, rect.top,
						rect.right - rect.left,
						rect.bottom - rect.top,
						SWP_NOZORDER | SWP_NOACTIVATE );

	return TRUE;
}

BOOL CDlgMover::OnSize()
{_STT();
	// Are we changing system work area?
	if ( m_hAppBar == NULL || m_dwDockPos == 0 || !m_bSizeDock ) 
		return FALSE;

	RECT rect;	
	GetAppBarPos( &rect );
	SetAppBarPos( &rect );

	return TRUE;	
}


BOOL CDlgMover::SetSizeCursor(POINT pt, BOOL bMove)
{_STT();
	// Ensure we have a window
	if ( m_hWnd == NULL || !IsWindow( m_hWnd ) ) 
		return FALSE;

	RECT	edit;
	GetWindowRect( m_hWnd, &edit );


	// 0 = Outside box
	// 1 = Move
	// 2 = size left
	// 3 = size right
	// 4 = size top
	// 5 = size bottom
	// 6 = size top left
	// 7 = size top right
	// 8 = size bottom left
	// 9 = size bottom right

	if ( !bMove )
	{
		DWORD	cursor = 0;

		// Choose a cursor
		if ( PtInRect( &edit, pt ) )
		{			
			if ( ( m_dwSize & DM_SIZELEFT ) && pt.x <= ( edit.left + m_lxEdge ) ) cursor = 2;
			else if ( ( m_dwSize & DM_SIZERIGHT ) && pt.x >= ( edit.right - m_lxEdge ) ) cursor = 3;
			
			if ( ( m_dwSize & DM_SIZETOP ) && pt.y <= ( edit.top + m_lyEdge ) ) 
			{	if ( cursor == 2 ) cursor = 6;
				else if ( cursor == 3 ) cursor = 7;
				else cursor = 4;
			} // end if

			else if ( ( m_dwSize & DM_SIZEBOTTOM ) && pt.y >= ( edit.bottom - m_lyEdge ) )
			{
				if ( cursor == 2 ) cursor = 8;
				else if ( cursor == 3 ) cursor = 9;
				else cursor = 5;
			} // end else if
	
			if ( cursor == 0 ) cursor = 1;

		} // end else

		// Don't resize docked edges
		if ( ( m_dwDockPos & DM_DOCKLEFT ) != 0 )
		{	if ( cursor == 2 ) cursor = 0;
			else if ( cursor == 6 ) cursor = 4;
			else if ( cursor == 8 ) cursor = 5;
			if ( m_bSizeDock && ( cursor == 4 || cursor == 5 ) ) cursor = 0;
		} // end if

		else if ( ( m_dwDockPos & DM_DOCKRIGHT ) != 0 )
		{	if ( cursor == 3 ) cursor = 0;
			else if ( cursor == 7 ) cursor = 4;
			else if ( cursor == 9 ) cursor = 5;
			if ( m_bSizeDock && ( cursor == 4 || cursor == 5 ) ) cursor = 0;
		} // end if

		else if ( ( m_dwDockPos & DM_DOCKTOP ) != 0 )
		{	if ( cursor == 4 ) cursor = 0;
			else if ( cursor == 6 ) cursor = 2;
			else if ( cursor == 7 ) cursor = 3;
			if ( m_bSizeDock && ( cursor == 2 || cursor == 3 ) ) cursor = 0;
		} // end if

		else if ( ( m_dwDockPos & DM_DOCKBOTTOM ) != 0 )
		{	if ( cursor == 5 ) cursor = 0;
			else if ( cursor == 8 ) cursor = 2;
			else if ( cursor == 9 ) cursor = 3;
			if ( m_bSizeDock && ( cursor == 2 || cursor == 3 ) ) cursor = 0;
		} // end if

		// Set cursor
		m_dwCursor = cursor;

	} // end if

	// Switch cursors if needed
	HCURSOR	hCursor = NULL;

	switch( m_dwCursor )
	{
		case 2: case 3: hCursor = LoadCursor( NULL, IDC_SIZEWE ); break;
		case 4: case 5: hCursor = LoadCursor( NULL, IDC_SIZENS ); break;
		case 7: case 8: hCursor = LoadCursor( NULL, IDC_SIZENESW ); break;
		case 6: case 9: hCursor = LoadCursor( NULL, IDC_SIZENWSE ); break;

		default: hCursor = LoadCursor( NULL, IDC_ARROW ); break;

	} // end switch

	// Set the new cursor
	::SetCursor( hCursor );

	if ( bMove && m_dwCursor > 1 )
	{
		long offx = pt.x - m_ptMove.x;
		long offy = pt.y - m_ptMove.y;

		// Save the point the first time through
		if ( m_ptMove.x == 0 && m_ptMove.y == 0 )
		{
			GetWindowRect( m_hWnd, &m_rectRef );
			memcpy( &m_ptMove, &pt, sizeof( POINT ) );
			return ( m_dwCursor != 0 );
		} // end if

		RECT	compare, e;
		GetWindowRect( m_hWnd, &compare );
		CopyRect( &e, &m_rectRef );
		
		// Book if not moving
		if ( offx == 0 && offy == 0 ) return ( m_dwCursor > 1 );

		else if ( m_dwCursor == 2 ) 
		{	e.left += offx; 
			if ( RW( e ) < m_szMinTrackSize.cx ) e.left = e.right - m_szMinTrackSize.cx;
			if ( RW( e ) > m_szMaxTrackSize.cx ) e.left = e.right - m_szMaxTrackSize.cx;
		} // end else if

		else if ( m_dwCursor == 3 ) 
		{	e.right += offx; 
			if ( RW( e ) < m_szMinTrackSize.cx ) e.right = e.left + m_szMinTrackSize.cx;
			if ( RW( e ) > m_szMaxTrackSize.cx ) e.right = e.left + m_szMaxTrackSize.cx;
		} // end else if

		else if ( m_dwCursor == 4 ) 
		{ 	e.top += offy; 
			if ( RH( e ) < m_szMinTrackSize.cy ) e.top = e.bottom - m_szMinTrackSize.cy;
			if ( RH( e ) > m_szMaxTrackSize.cy ) e.top = e.bottom - m_szMaxTrackSize.cy;
		} // end else if

		else if ( m_dwCursor == 5 ) 
		{ 	e.bottom += offy; 
			if ( RH( e ) < m_szMinTrackSize.cy ) e.bottom = e.top + m_szMinTrackSize.cy;
			if ( RH( e ) > m_szMaxTrackSize.cy ) e.bottom = e.top + m_szMaxTrackSize.cy;
		} 

		else if ( m_dwCursor == 6 ) 
		{	e.left += offx; e.top += offy;
			if ( RW( e ) < m_szMinTrackSize.cx ) e.left = e.right - m_szMinTrackSize.cx;
			if ( RW( e ) > m_szMaxTrackSize.cx ) e.left = e.right - m_szMaxTrackSize.cx;
			if ( RH( e ) < m_szMinTrackSize.cy ) e.top = e.bottom - m_szMinTrackSize.cy;
			if ( RH( e ) > m_szMaxTrackSize.cy ) e.top = e.bottom - m_szMaxTrackSize.cy;
		} // end else if

		else if ( m_dwCursor == 7 ) 
		{
			e.right += offx; e.top += offy;
			if ( RW( e ) < m_szMinTrackSize.cx ) e.right = e.left + m_szMinTrackSize.cx;
			if ( RW( e ) > m_szMaxTrackSize.cx ) e.right = e.left + m_szMaxTrackSize.cx;
			if ( RH( e ) < m_szMinTrackSize.cy ) e.top = e.bottom - m_szMinTrackSize.cy;
			if ( RH( e ) > m_szMaxTrackSize.cy ) e.top = e.bottom - m_szMaxTrackSize.cy;
		} // end else if

		else if ( m_dwCursor == 8 ) 
		{
			e.left += offx; e.bottom += offy;
			if ( RW( e ) < m_szMinTrackSize.cx ) e.left = e.right - m_szMinTrackSize.cx;
			if ( RW( e ) > m_szMaxTrackSize.cx ) e.left = e.right - m_szMaxTrackSize.cx;
			if ( RH( e ) < m_szMinTrackSize.cy ) e.bottom = e.top + m_szMinTrackSize.cy;
			if ( RH( e ) > m_szMaxTrackSize.cy ) e.bottom = e.top + m_szMaxTrackSize.cy;
		} // end else if

		else if ( m_dwCursor == 9 ) 
		{
			e.right += offx; e.bottom += offy;
			if ( RW( e ) < m_szMinTrackSize.cx ) e.right = e.left + m_szMinTrackSize.cx;
			if ( RW( e ) > m_szMaxTrackSize.cx ) e.right = e.left + m_szMaxTrackSize.cx;
			if ( RH( e ) < m_szMinTrackSize.cy ) e.bottom = e.top + m_szMinTrackSize.cy;
			if ( RH( e ) > m_szMaxTrackSize.cy ) e.bottom = e.top + m_szMaxTrackSize.cy;
		} // end else if

		else return ( m_dwCursor > 1 );

		// Minimum size
		if ( RW( e ) < m_szMinTrackSize.cx ) e.right = e.left + m_szMinTrackSize.cx;
		if ( RH( e ) < m_szMinTrackSize.cy ) e.bottom = e.top + m_szMinTrackSize.cy;

		// Maximum size
		if ( RW( e ) > m_szMaxTrackSize.cx ) e.right = e.left + m_szMaxTrackSize.cx;
		if ( RH( e ) > m_szMaxTrackSize.cy ) e.bottom = e.top + m_szMaxTrackSize.cy;

		// Set the new window position
		SetWindowPos( m_hWnd, NULL, e.left, e.top,
							e.right - e.left,
							e.bottom - e.top,
							SWP_NOZORDER | SWP_NOACTIVATE );

		// Have things changed?
		if ( !EqualRect( &compare, &e ) ) m_bResized = TRUE;

	} // end if

	// Reset move point
	else { m_ptMove.x = 0; m_ptMove.y = 0; }

	return ( m_dwCursor > 1 );
}
