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
// MovableObj.cpp: implementation of the CMovableObj class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define PI		( (double)3.141592654 )
#define PI2		( PI * (double)2 )

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMovableObj::CMovableObj( LPRECT pRect )
{_STT();
	m_dDistance = 0;
	ZeroMemory( m_dAngle, sizeof( m_dAngle ) );

	if ( pRect ) CopyRect( &m_rect, pRect );
	else ZeroMemory( &m_rect, sizeof( m_rect ) );

	m_lXMargin = 4;
	m_lYMargin = 4;

	m_lXMin = 16;
	m_lYMin = 16;

	m_lXOff = 0;
	m_lYOff = 0;

	m_bButtonDown = FALSE;

	m_bSelected = FALSE;
}

CMovableObj::~CMovableObj()
{_STT();

}

void CMovableObj::Destroy()
{_STT();
	m_dDistance = 0;
	ZeroMemory( m_dAngle, sizeof( m_dAngle ) );

	ZeroMemory( &m_rect, sizeof( m_rect ) );

	m_lXMargin = 4;
	m_lYMargin = 4;

	m_lXMin = 16;
	m_lYMin = 16;

	m_lXOff = 0;
	m_lYOff = 0;

	m_bButtonDown = FALSE;
}

BOOL CMovableObj::Draw(HDC hDC, LPRECT pRect)
{_STT();
	RECT rect, intersection;
	CopyRect( &rect, &m_rect );
	OffsetRect( &rect, -m_lXOff, -m_lYOff );

	// Punt if off the canvas
	if ( !IntersectRect( &intersection, pRect, &rect ) )
		return FALSE;	

	// Draw the object
	OnDraw( hDC, &rect );

	if ( m_bSelected )
	{
		// Draw the rect frame
		HBRUSH hBrush = CreateHatchBrush( HS_BDIAGONAL, RGB( 0, 0, 0 ) );
		FrameRect( hDC, &rect, hBrush );
		DeleteObject( hBrush );

		// Draw a focus rect
		DrawFocusRect( hDC, &rect );

	} // end if

	return TRUE;
}

DWORD CMovableObj::GetCursorFlags(LPPOINT pPt)
{_STT();
	POINT pt = { pPt->x + m_lXOff, pPt->y + m_lYOff };
	DWORD dwCursor = 0;

	// Punt if out of our area
	if ( pt.x < m_rect.left - m_lXMargin || pt.x > m_rect.right + m_lXMargin ||
		 pt.y < m_rect.top - m_lYMargin || pt.y > m_rect.bottom + m_lYMargin )
		return dwCursor;

	// Left cursor position
	if ( pt.x <= ( m_rect.left + m_lXMargin ) ) dwCursor |= MOCUR_LEFT;

	// Right cursor position
	else if ( pt.x >= ( m_rect.right - m_lXMargin ) ) dwCursor |= MOCUR_RIGHT;

	// Top cursor position
	if ( pt.y <= ( m_rect.top + m_lYMargin ) ) dwCursor |= MOCUR_TOP;

	// Bottom cursor position
	else if ( pt.y >= ( m_rect.bottom - m_lYMargin ) ) dwCursor |= MOCUR_BOTTOM;

	// Check for move cursor
	if ( dwCursor == 0 )
	{	if ( GetAsyncKeyState( VK_SHIFT ) )
			dwCursor |= MOCUR_ROTATE1;		
		else if ( GetAsyncKeyState( VK_CONTROL ) )
			dwCursor |= MOCUR_ROTATE2;		
		else if ( GetAsyncKeyState( VK_MENU ) )
			dwCursor |= MOCUR_ROTATE3;		
		else dwCursor |= MOCUR_MOVE;		
	} // end if

	return dwCursor;
}
									  
HCURSOR CMovableObj::GetCursor( DWORD dwCursor )
{_STT();
	if ( dwCursor == 0 ) return NULL;

	HCURSOR hCursor = NULL;

	if ( dwCursor & MOCUR_LEFT && dwCursor & MOCUR_TOP )
		hCursor = LoadCursor( NULL, IDC_SIZENWSE );

	else if ( dwCursor & MOCUR_RIGHT && dwCursor & MOCUR_TOP )
		hCursor = LoadCursor( NULL, IDC_SIZENESW );

	else if ( dwCursor & MOCUR_LEFT && dwCursor & MOCUR_BOTTOM )
		hCursor = LoadCursor( NULL, IDC_SIZENESW );

	else if ( dwCursor & MOCUR_RIGHT && dwCursor & MOCUR_BOTTOM )
		hCursor = LoadCursor( NULL, IDC_SIZENWSE );

	else if ( dwCursor & MOCUR_LEFT || dwCursor & MOCUR_RIGHT )
		hCursor = LoadCursor( NULL, IDC_SIZEWE );

	else if ( dwCursor & MOCUR_TOP || dwCursor & MOCUR_BOTTOM )
		hCursor = LoadCursor( NULL, IDC_SIZENS );

	else if ( dwCursor & MOCUR_MOVE ) 
		hCursor = LoadCursor( NULL, IDC_SIZEALL );	

	else if ( dwCursor & ( MOCUR_ROTATE1 | MOCUR_ROTATE2 | MOCUR_ROTATE3 ) ) 
		hCursor = LoadCursor( NULL, IDC_CROSS );	

	return hCursor;
}

BOOL CMovableObj::SetCursor(LPPOINT pPt)
{_STT();
	HCURSOR hCursor = NULL;
	
	// Get cursor
	if ( m_bButtonDown ) hCursor = GetCursor( m_dwClickFlags );
	else hCursor = GetCursor( GetCursorFlags( pPt ) );

	// Load default cursor
	if ( hCursor == NULL ) hCursor = LoadCursor( NULL, IDC_ARROW );

	// Let windows in on our decission
	::SetCursor( hCursor );

	return TRUE;
}

BOOL CMovableObj::OnMouseMove(UINT uFlags, LPPOINT pPt)
{_STT();
	// Validate button down
	if ( !( uFlags & MK_LBUTTON ) )
	{	m_bButtonDown = FALSE; m_dwClickFlags = 0; }

	// Punt if button up
	if ( !m_bButtonDown ) return FALSE;

	return Move( pPt );
}

BOOL CMovableObj::OnLButtonDown(UINT uFlags, LPPOINT pPt)
{_STT();
	// Save click point
	m_ptClick.x = pPt->x + m_lXOff;
	m_ptClick.y = pPt->y + m_lYOff;

	// Save click flags
	m_dwClickFlags = GetCursorFlags( pPt );

	// Button is down
	m_bButtonDown = TRUE;

	return TRUE;
}

BOOL CMovableObj::OnLButtonUp(UINT uFlags, LPPOINT pPt)
{_STT();
	m_bButtonDown = FALSE;
	m_dwClickFlags = 0;

	return TRUE;
}

BOOL CMovableObj::Move(LPPOINT pPt)
{_STT();
	POINT pt = { pPt->x + m_lXOff, pPt->y + m_lYOff };

	DWORD dwCursor = m_dwClickFlags;

	if ( dwCursor & MOCUR_LEFT && dwCursor & MOCUR_TOP )
	{	m_rect.left = pt.x;
		m_rect.top = pt.y;
	} // end if

	else if ( dwCursor & MOCUR_RIGHT && dwCursor & MOCUR_TOP )
	{	m_rect.right = pt.x;
		m_rect.top = pt.y;
	} // end if

	else if ( dwCursor & MOCUR_LEFT && dwCursor & MOCUR_BOTTOM )
	{	m_rect.left = pt.x;
		m_rect.bottom = pt.y;
	} // end if

	else if ( dwCursor & MOCUR_RIGHT && dwCursor & MOCUR_BOTTOM )
	{	m_rect.right = pt.x;
		m_rect.bottom = pt.y;
	} // end if

	else if ( dwCursor & MOCUR_LEFT )
		m_rect.left = pt.x;

	else if ( dwCursor & MOCUR_RIGHT )
		m_rect.right = pt.x;

	else if ( dwCursor & MOCUR_TOP )
		m_rect.top = pt.y;
		
	else if ( dwCursor & MOCUR_BOTTOM )
		m_rect.bottom = pt.y;

	else if ( dwCursor & MOCUR_MOVE ) 
	{	long w = RW( m_rect );
		long h = RH( m_rect );
		m_rect.left = pt.x - ( w >> 1 );
		m_rect.top = pt.y - ( h >> 1 );
		m_rect.right = m_rect.left + w;
		m_rect.bottom = m_rect.top + h;
	} // end if

	else if ( dwCursor & MOCUR_ROTATE1 )
	{	double dx = pt.x - GetX();
		double dy = pt.y - GetY();
		m_dAngle[ 0 ] = -atan2( dy, dx );
		m_dDistance = sqrt( dx * dx + dy * dy );
	} // end else if

	else if ( dwCursor & MOCUR_ROTATE2 )
	{	double dx = pt.x - GetX();
		double dy = pt.y - GetY();
		m_dAngle[ 1 ] = -atan2( dy, dx );
		m_dDistance = sqrt( dx * dx + dy * dy );
	} // end else if

	else if ( dwCursor & MOCUR_ROTATE3 )
	{	double dx = pt.x - GetX();
		double dy = pt.y - GetY();
		m_dAngle[ 2 ] = -atan2( dy, dx );
		m_dDistance = sqrt( dx * dx + dy * dy );
	} // end else if

	else if ( dwCursor & MOCUR_ROTATE4 )
	{	double dx = pt.x - GetX();
		double dy = pt.y - GetY();
		m_dAngle[ 3 ] = -atan2( dy, dx );
		m_dDistance = sqrt( dx * dx + dy * dy );
	} // end else if

	if ( m_rect.right < m_rect.left + m_lXMin )
	{	if ( dwCursor & MOCUR_RIGHT )
			m_rect.right = m_rect.left + m_lXMin;
		else if ( dwCursor & MOCUR_LEFT )
			m_rect.left = m_rect.right - m_lXMin;
	} // end if

	if ( m_rect.bottom < m_rect.top + m_lYMin )
	{	if ( dwCursor & MOCUR_BOTTOM )
			m_rect.bottom = m_rect.top + m_lYMin;
		else if ( dwCursor & MOCUR_TOP )
			m_rect.top = m_rect.bottom - m_lXMin;
	} // end if

	return TRUE;
}
