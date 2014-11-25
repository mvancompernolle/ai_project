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
// GrDC.cpp: implementation of the CGrDC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define		PI		((double)3.141592654)	
#define		PI2		((double)2 * PI )	

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGrDC::CGrDC()
{_STT();
	m_hDC = NULL;

	m_hOldPen = NULL;
	m_hOldBrush = NULL;
	m_hOldFont = NULL;

	m_bAutoDelete = FALSE;
}

CGrDC::CGrDC( HDC hDC )
{_STT();
	m_hDC = hDC;

	m_hOldPen = NULL;
	m_hOldBrush = NULL;
	m_hOldFont = NULL;

	m_bAutoDelete = TRUE;
}

CGrDC::CGrDC( HDC hDC, BOOL bAutoDelete )
{_STT();
	m_hDC = hDC;

	m_hOldPen = NULL;
	m_hOldBrush = NULL;
	m_hOldFont = NULL;

	m_bAutoDelete = bAutoDelete;
}

CGrDC::~CGrDC()
{_STT();
	Destroy();
}

void CGrDC::Destroy()
{_STT();
	// Select old devices back into dc
	Restore();

	if ( m_hDC != NULL )
	{
		// Do we delete the DC?
		if ( m_bAutoDelete ) DeleteDC( m_hDC );

		m_hDC = NULL;

	} // end if

}

BOOL CGrDC::GetDC(HWND hWnd)
{_STT();
	Destroy();

	m_hDC = ::GetDC( hWnd );

	return IsDC();
}

BOOL CGrDC::ReleaseDC( HWND hWnd )
{_STT();
	if ( !IsDC() ) return FALSE;

	return ::ReleaseDC( hWnd, m_hDC );
}

BOOL CGrDC::FillSolidRect(HDC hDC, LPRECT pRect, COLORREF rgb)
{_STT();
	// Sanity check
	if ( hDC == NULL || pRect == NULL ) return FALSE;

	// Fill in the rect
	HBRUSH hBrush = CreateSolidBrush( rgb );
	FillRect( hDC, pRect, hBrush );
	DeleteObject( hBrush );

	return TRUE;
}


BOOL CGrDC::GetRect(LPRECT pRect)
{_STT();
	if ( pRect == NULL ) return FALSE;

	return FALSE;
}

BOOL CGrDC::SelectObject(HPEN hPen)
{_STT();
	if ( hPen == NULL )
	{
		// Select old pen
		if ( m_hOldPen != NULL ) 
		{
			::SelectObject( m_hDC, m_hOldPen );
			m_hOldPen = NULL;
			return TRUE;
		} // end if

		return FALSE;
	} // end if

	// Select pen
	HPEN pen = (HPEN)::SelectObject( m_hDC, hPen );
	if ( m_hOldPen == NULL ) m_hOldPen = pen;

	return TRUE;
}

BOOL CGrDC::SelectObject(HBRUSH hBrush)
{_STT();
	if ( hBrush == NULL )
	{
		// Select old pen
		if ( m_hOldBrush != NULL ) 
		{
			::SelectObject( m_hDC, m_hOldBrush );
			m_hOldBrush = NULL;
			return TRUE;
		} // end if

		return FALSE;
	} // end if

	// Select pen
	HBRUSH brush = (HBRUSH)::SelectObject( m_hDC, hBrush );
	if ( m_hOldBrush == NULL ) m_hOldBrush = brush;

	return TRUE;
}

BOOL CGrDC::SelectObject(HFONT hFont)
{_STT();
	if ( hFont == NULL )
	{
		// Select old pen
		if ( m_hOldFont != NULL ) 
		{
			::SelectObject( m_hDC, m_hOldFont );
			m_hOldFont = NULL;
			return TRUE;
		} // end if

		return FALSE;
	} // end if

	// Select pen
	HFONT font = (HFONT)::SelectObject( m_hDC, hFont );
	if ( m_hOldFont == NULL ) m_hOldFont = NULL;

	return TRUE;
}

void CGrDC::Restore()
{_STT();
	// Restore 
	SelectObject( (HPEN)NULL );
	SelectObject( (HBRUSH)NULL );
	SelectObject( (HFONT)NULL );
}

#define HGRSTEP		1
#define VGRSTEP		1

BOOL CGrDC::VertGradientFill(HDC hDC, LPRECT pRect, COLORREF top, COLORREF bottom)
{_STT();
	// Sanity checks
	if ( hDC == NULL || pRect == NULL ) return FALSE;
	if ( pRect->left >= pRect->right || pRect->top >= pRect->bottom ) return FALSE;

	// Do we need a gradient
	if ( top == bottom ) return FillSolidRect( hDC, pRect, top );

	RECT	rect;
	CopyRect( &rect, pRect );


	long	rd = GetRValue( top ) - GetRValue( bottom );
	if ( rd < 0 ) rd = -rd;
	long	gd = GetGValue( top ) - GetGValue( bottom );
	if ( gd < 0 ) gd = -gd;
	long	bd = GetBValue( top ) - GetBValue( bottom );
	if ( bd < 0 ) bd = -bd;

	DWORD	my = 0;
	DWORD	md = ( rd > gd ) ? ( ( rd > bd ) ? rd : bd ) : ( ( gd > bd ) ? gd : bd );

	// Do we have a change in color?
	if ( md == 0 ) 
	{
		HBRUSH hBrush = CreateSolidBrush( top );
		FillRect( hDC, &rect, hBrush );
		DeleteObject( hBrush );
		return TRUE;
	} // end if

	rect.bottom = rect.top;

	DWORD y = md;
	while ( rect.top < pRect->bottom )
	{
		rect.top = rect.bottom;
		my += ( pRect->bottom - pRect->top ) * VGRSTEP;
		while ( my >= md ) { my -= md; rect.bottom += 1; }

		if ( rect.top != rect.bottom )
		{
			DWORD red = ( 	( ( GetRValue( top ) * y ) / md ) + 
							( ( GetRValue( bottom ) * ( md - y ) ) / md ) );
			DWORD green = ( ( ( GetGValue( top ) * y ) / md ) + 
							( ( GetGValue( bottom ) * ( md - y ) ) / md ) );
			DWORD blue = ( 	( ( GetBValue( top ) * y ) / md ) + 
							( ( GetBValue( bottom ) * ( md - y ) ) / md ) );

			COLORREF col = RGB( red, green, blue );
			HBRUSH hBrush = CreateSolidBrush( col );
			FillRect( hDC, &rect, hBrush );
			DeleteObject( hBrush );
		} // end if

		if ( y > VGRSTEP ) y -= VGRSTEP;

	} // end for

	return TRUE;
}


BOOL CGrDC::HorzGradientFill(HDC hDC, LPRECT pRect, COLORREF left, COLORREF right)
{_STT();
	// Sanity checks
	if ( hDC == NULL || pRect == NULL ) return FALSE;
	if ( pRect->left >= pRect->right || pRect->top >= pRect->bottom ) return FALSE;

	// Do we need a gradient
	if ( left == right ) return FillSolidRect( hDC, pRect, left );

	RECT	rect;
	CopyRect( &rect, pRect );

	long rd = GetRValue( left ) - GetRValue( right );
	if ( rd < 0 ) rd = -rd;
	long gd = GetGValue( left ) - GetGValue( right );
	if ( gd < 0 ) gd = -gd;
	long bd = GetBValue( left ) - GetBValue( right );
	if ( bd < 0 ) bd = -bd;

	DWORD	my = 0;
	DWORD	md = ( rd > gd ) ? ( ( rd > bd ) ? rd : bd ) : ( ( gd > bd ) ? gd : bd );

	// Do we have a change in color?
	if ( md == 0 ) 
	{
		HBRUSH hBrush = CreateSolidBrush( left );
		FillRect( hDC, &rect, hBrush );
		DeleteObject( hBrush );
		return TRUE;
	} // end if

	rect.right = rect.left;

	DWORD x = md;
	while ( rect.left < pRect->right )
	{
		rect.left = rect.right;
		my += ( pRect->right - pRect->left ) * HGRSTEP;
		while ( my >= md ) { my -= md; rect.right += 1; }

		if ( rect.left != rect.right )
		{
			DWORD red = ( 	( ( GetRValue( left ) * x ) / md ) + 
							( ( GetRValue( right ) * ( md - x ) ) / md ) );
			DWORD green = ( ( ( GetGValue( left ) * x ) / md ) + 
							( ( GetGValue( right ) * ( md - x ) ) / md ) );
			DWORD blue = ( 	( ( GetBValue( left ) * x ) / md ) + 
							( ( GetBValue( right ) * ( md - x ) ) / md ) );

			COLORREF col = RGB( red, green, blue );
			HBRUSH hBrush = CreateSolidBrush( col );
			FillRect( hDC, &rect, hBrush );
			DeleteObject( hBrush );
		} // end if

		if ( x > HGRSTEP ) x -= HGRSTEP;

	} // end for

	return TRUE;
}

BOOL CGrDC::GradientFill(HDC hDC, LPRECT pRect, COLORREF lefttop, COLORREF righttop, COLORREF leftbottom, COLORREF rightbottom)
{_STT();
	// Sanity checks
	if ( hDC == NULL || pRect == NULL ) return FALSE;
	if ( pRect->left >= pRect->right || pRect->top >= pRect->bottom ) return FALSE;

	// Do we need a full gradient
	if ( lefttop == righttop && leftbottom == rightbottom )
		return VertGradientFill( hDC, pRect, lefttop, leftbottom );
	else if ( lefttop == leftbottom && righttop == rightbottom )
		return HorzGradientFill( hDC, pRect, lefttop, righttop );

	RECT	rect;
	CopyRect( &rect, pRect );

	// Left side
	long	lrd = GetRValue( lefttop ) - GetRValue( leftbottom );
	if ( lrd < 0 ) lrd = -lrd;
	long	lgd = GetGValue( lefttop ) - GetGValue( leftbottom );
	if ( lgd < 0 ) lgd = -lgd;
	long	lbd = GetBValue( lefttop ) - GetBValue( leftbottom );
	if ( lbd < 0 ) lbd = -lbd;
	DWORD	lmy = 0;
	DWORD	lmd = ( lrd > lgd ) ? ( ( lrd > lbd ) ? lrd : lbd ) : ( ( lgd > lbd ) ? lgd : lbd );

	// Right side
	long	rrd = GetRValue( righttop ) - GetRValue( rightbottom );
	if ( rrd < 0 ) rrd = -rrd;
	long	rgd = GetGValue( righttop ) - GetGValue( rightbottom );
	if ( rgd < 0 ) rgd = -rgd;
	long	rbd = GetBValue( righttop ) - GetBValue( rightbottom );
	if ( rbd < 0 ) rbd = -rbd;
	DWORD	rmy = 0;
	DWORD	rmd = ( rrd > rgd ) ? ( ( rrd > rbd ) ? rrd : rbd ) : ( ( rgd > rbd ) ? rgd : rbd );

	rect.bottom = rect.top;

	DWORD ly = lmd;
	DWORD ry = rmd;

	while ( rect.top < pRect->bottom )
	{
		rect.top = rect.bottom;

		// Circle if needed
		if ( lmy < lmd ) lmy += ( PRH( pRect ) * VGRSTEP );
		if ( rmy < rmd ) rmy += ( PRH( pRect ) * VGRSTEP );

		// Find next division
		while ( lmy >= lmd && rmy >= rmd ) 
		{ lmy -= lmd; rmy -= rmd; rect.bottom += 1; }

		// If space for gradient
		if ( rect.top < rect.bottom )
		{
			COLORREF lcol, rcol;

			if ( lmd != 0 )
			{
				// Calculate left color
				DWORD lred = ( 	( ( GetRValue( lefttop ) * ly ) / lmd ) + 
								( ( GetRValue( leftbottom ) * ( lmd - ly ) ) / lmd ) );
				DWORD lgreen = ( ( ( GetGValue( lefttop ) * ly ) / lmd ) + 
								( ( GetGValue( leftbottom ) * ( lmd - ly ) ) / lmd ) );
				DWORD lblue = ( 	( ( GetBValue( lefttop ) * ly ) / lmd ) + 
								( ( GetBValue( leftbottom ) * ( lmd - ly ) ) / lmd ) );
				lcol = RGB( lred, lgreen, lblue );
			} // end if
			else lcol = lefttop;

			if ( rmd != 0 )
			{
				// Calculate right color
				DWORD rred = ( 	( ( GetRValue( righttop ) * ry ) / rmd ) + 
								( ( GetRValue( rightbottom ) * ( rmd - ry ) ) / rmd ) );
				DWORD rgreen = ( ( ( GetGValue( righttop ) * ry ) / rmd ) + 
								( ( GetGValue( rightbottom ) * ( rmd - ry ) ) / rmd ) );
				DWORD rblue = ( 	( ( GetBValue( righttop ) * ry ) / rmd ) + 
								( ( GetBValue( rightbottom ) * ( rmd - ry ) ) / rmd ) );
				rcol = RGB( rred, rgreen, rblue );
			} // end if
			else rcol = leftbottom;

			// Do horz gradient
			HorzGradientFill( hDC, &rect, lcol, rcol );

		} // end if

		if ( ly > VGRSTEP ) ly -= VGRSTEP;
		if ( ry > VGRSTEP ) ry -= VGRSTEP;

	} // end for

	return TRUE;
}

COLORREF CGrDC::ScaleColor(COLORREF col, long scale)
{_STT();
	long r = GetRValue( col );
	long g = GetGValue( col );
	long b = GetBValue( col );

	// Scale values
	r += ( scale * r ) >> 8;
	g += ( scale * g ) >> 8;
	b += ( scale * b ) >> 8;

	// Bounds check
	if ( r > 255 ) r = 255; if ( r < 0 ) r = 0;
	if ( g > 255 ) g = 255; if ( g < 0 ) g = 0;
	if ( b > 255 ) b = 255; if ( b < 0 ) b = 0;

	// Cool effect
//	r &= 0xff;
//	g &= 0xff;
//	b &= 0xff;

	return ( RGB( r, g, b ) );
}

COLORREF CGrDC::ScaleColor(COLORREF c1, COLORREF c2, long scale)
{_STT();
	long r1 = GetRValue( c1 );
	long g1 = GetGValue( c1 );
	long b1 = GetBValue( c1 );
	long r2 = GetRValue( c2 );
	long g2 = GetGValue( c2 );
	long b2 = GetBValue( c2 );

	// Scale color values
	r1 += ( ( r2 - r1 ) * scale ) / 1000;
	g1 += ( ( g2 - g1 ) * scale ) / 1000;
	b1 += ( ( b2 - b1 ) * scale ) / 1000;

	// Return the new color
	return ( RGB( r1, g1, b1 ) );
}

BOOL CGrDC::IsLightColor( COLORREF rgb )
{
	return ( GrayScale( rgb ) > 128 );
}

BOOL CGrDC::Center(LPRECT pRect, LPRECT pRef, BOOL x, BOOL y)
{_STT();
	if ( pRect == NULL || pRef == NULL ) return FALSE;

	// Calculate values
	if ( x )
	{
		long w1 = pRect->right - pRect->left;
		long w2 = pRef->right - pRef->left;
		
		pRect->left = pRef->left + ( ( w2 - w1 ) >> 1 );
		pRect->right = pRect->left + w1;

	} // end if

	if ( y )
	{	
		long h1 = pRect->bottom - pRect->top;
		long h2 = pRef->bottom - pRef->top;

		pRect->top = pRef->top + ( ( h2 - h1 ) >> 1 );
		pRect->bottom = pRect->top + h1;

	} // end if

	return TRUE;
}

BOOL CGrDC::Box3d(HDC hDC, LPRECT pRect, DWORD size, COLORREF lt, COLORREF rb, BOOL bInvert )
{_STT();
	if ( hDC == NULL || pRect == NULL ) return FALSE;

	RECT rect;
	CopyRect( &rect, pRect );
	rect.right--; rect.bottom--;

	HPEN hLtPen = ::CreatePen( PS_SOLID, 1, lt );
	HPEN hDkPen = ::CreatePen( PS_SOLID, 1, rb );
	HPEN hOldPen = (HPEN)::SelectObject( hDC, hLtPen );

	// Invert if needed
	if ( bInvert ) { HPEN t = hLtPen; hLtPen = hDkPen; hDkPen = t; }

	for ( DWORD i = 0; i < size; i++ )
	{
		::SelectObject( hDC, hLtPen );
		::MoveToEx( hDC, rect.left, rect.bottom, NULL );
		::LineTo( hDC, rect.left, rect.top );
		::LineTo( hDC, rect.right, rect.top );

		::SelectObject( hDC, hDkPen );
		::MoveToEx( hDC, rect.right, rect.top, NULL );
		::LineTo( hDC, rect.right, rect.bottom );
		::LineTo( hDC, rect.left, rect.bottom );

		// Next box
		InflateRect( &rect, -1, -1 );

	} // end if

	::SelectObject( hDC, hOldPen );
	::DeleteObject( hLtPen );
	::DeleteObject( hDkPen );

	return TRUE;
}

BOOL CGrDC::RoundedBox3d(HDC hDC, LPRECT pRect, DWORD size, COLORREF lt, COLORREF rb, BOOL bInvert )
{_STT();
	if ( hDC == NULL || pRect == NULL ) return FALSE;

	RECT rect;
	CopyRect( &rect, pRect );
	rect.right--; rect.bottom--;

	HPEN hLtPen = ::CreatePen( PS_SOLID, 1, lt );
	HPEN hDkPen = ::CreatePen( PS_SOLID, 1, rb );
	HPEN hOldPen = (HPEN)::SelectObject( hDC, hLtPen );

	// Invert if needed
	if ( bInvert ) { HPEN t = hLtPen; hLtPen = hDkPen; hDkPen = t; }

	for ( DWORD i = 0; i < size; i++ )
	{
		::SelectObject( hDC, hLtPen );

		::MoveToEx( hDC, rect.left, pRect->bottom - i - 1, NULL );
		::LineTo( hDC, rect.left, pRect->top + i );

		::MoveToEx( hDC, pRect->left + i, rect.top, NULL );
		::LineTo( hDC, pRect->right - i - 1, rect.top );

		::SelectObject( hDC, hDkPen );

		::MoveToEx( hDC, rect.right, pRect->top + i, NULL );
		::LineTo( hDC, rect.right, pRect->bottom - i - 1 );

		::MoveToEx( hDC, pRect->right - i - 1, rect.bottom, NULL );
		::LineTo( hDC, pRect->left + i, rect.bottom );

		// Next box
		InflateRect( &rect, 1, 1 );

	} // end if

	::SelectObject( hDC, hOldPen );
	::DeleteObject( hLtPen );
	::DeleteObject( hDkPen );

	return TRUE;
}

LPRECT CGrDC::AspectCorrect(LPRECT r1, LPRECT r2)
{_STT();
	// Ensure valid data
	if ( r1 == NULL || r2 == NULL ) return r1;

	// Correct aspect ratio
	if ( AspectCorrect( r1, PRW( r2 ), PRH( r2 ) ) == NULL )
		return NULL;

	return r1;
}

LPRECT CGrDC::AspectCorrect(LPRECT r1, long w, long h)
{_STT();
	// Ensure valid data
	if ( r1 == NULL ) return NULL;

	// No dividing by zero!
	if ( w == 0 || h == 0 ) return r1;

	long w1 = PRW( r1 ), h1 = PRH( r1 );

	RECT rect;
	CopyRect( &rect, r1 );

	// Calculate new sizes
	long sw = ( ( h1 * w ) / h );
	long sh = ( ( w1 * h ) / w ); 

	// Which one should we use?
	if ( sw <= w1 ) r1->right = r1->left + sw;
	else r1->bottom = r1->top + sh;

	// Center in old rectangle
	Center( r1, &rect );

	return r1;
}

BOOL CGrDC::FillSolidPolygon(HDC hDC, LPPOINT pt, DWORD pts, COLORREF rgb)
{_STT();
	// Sanity check
	if ( hDC == NULL || pt == NULL || pts <= 1 ) return FALSE;

	// Fill in the rect
	HBRUSH hBrush = CreateSolidBrush( rgb );
	HBRUSH hOldBrush = (HBRUSH)::SelectObject( hDC, hBrush );

	Polygon( hDC, pt, pts );

	::SelectObject( hDC, hOldBrush );
	::DeleteObject( hBrush );

	return TRUE;
}

BOOL CGrDC::DrawButton(HDC hDC, BOOL bPressed, LPPOINT pt, DWORD pts, COLORREF color, HICON hIcon)
{_STT();
	// Sanity checks
	if ( hDC == NULL || pt == NULL || pts <= 1 ) return FALSE;

	HPEN hLtPen = ::CreatePen( PS_SOLID, 1, CGrDC::ScaleColor( color, 100 ) );
	HPEN hDkPen = ::CreatePen( PS_SOLID, 1, CGrDC::ScaleColor( color, -100 ) );
	HPEN hOldPen = (HPEN)::SelectObject( hDC, hLtPen );

	// Invert if needed
	if ( bPressed ) { HPEN t = hLtPen; hLtPen = hDkPen; hDkPen = t; }

	// Fill polygon
	CGrDC::FillSolidPolygon( hDC, pt, pts, color );

	DWORD last = pts - 1;
	DWORD half = pts >> 1;

	// Draw top lines
	::SelectObject( hDC, hLtPen );
	::MoveToEx( hDC, pt[ last ].x, pt[ last ].y, NULL );
	DWORD i;
	for ( i = 0; i < half; i++ )
		::LineTo( hDC, pt[ i ].x, pt[ i ].y );

	// Draw bottom lines
	::SelectObject( hDC, hDkPen );
	::MoveToEx( hDC, pt[ half ].x, pt[ half ].y, NULL );
	for ( i = ( half + 1 ); i < pts; i++ )
		::LineTo( hDC, pt[ i ].x, pt[ i ].y );

	::SelectObject( hDC, hOldPen );
	::DeleteObject( hLtPen );
	::DeleteObject( hDkPen );

	return TRUE;
}

// For rectangular shapes
BOOL CGrDC::InflatePolygon(LPPOINT pt, DWORD pts, long x, long y )
{_STT();
	RECT rect;
	long cx, cy;

	// Get the bounding rectangle
	if ( !GetPolygonRect( &rect, pt, pts ) ) return FALSE;

	// Find center points
	cx = rect.left + ( RW( rect ) >> 1 );
	cy = rect.top + ( RH( rect ) >> 1 );

	for ( DWORD i = 0; i < pts; i++ )
	{
		if ( pt[ i ].x < cx ) pt[ i ].x -= (short)x;
		else pt[ i ].x += (short)x;

		if ( pt[ i ].y < cy ) pt[ i ].y -= (short)y;
		else pt[ i ].y += (short)y;

	} // end for


	return TRUE;
}

// For illregular shapes
BOOL CGrDC::InflatePolygon(LPPOINT pt, DWORD pts, double mag)
{_STT();
	RECT rect;
	long cx, cy;

	// Get the bounding rectangle
	if ( !GetPolygonRect( &rect, pt, pts ) ) return FALSE;

	// Find center points
	cx = rect.left + ( RW( rect ) >> 1 );
	cy = rect.top + ( RH( rect ) >> 1 );

	for ( DWORD i = 0; i < pts; i++ )
	{
		double dx = pt[ i ].x - cx;
		double dy = pt[ i ].y - cy;
		double a = GetAngle( pt[ i ].x, pt[ i ].y, cx, cy );
		double angle = ( GetAngle( pt[ i ].x, pt[ i ].y, cx, cy ) * PI2 ) / (double)360;
		double dist = sqrt( ( dx * dx ) + ( dy * dy ) ) + mag;

		// Replot points
		pt[ i ].x = short( (double)cx - ( cos( angle ) * dist ) );
		pt[ i ].y = short( (double)cy + ( sin( angle ) * dist ) );

	} // end for

	return TRUE;
}

BOOL CGrDC::GetPolygonRect(LPRECT pRect, LPPOINT pt, DWORD pts)
{_STT();
	// Sanity checks
	if ( pRect == NULL || pt == NULL || pts == 0 ) return FALSE;

	// Initialize rectangle
	pRect->left = pRect->right = pt[ 0 ].x;
	pRect->top = pRect->bottom = pt[ 0 ].y;

	// For each point
	for ( DWORD i = 1; i < pts; i++ )
	{
		// Find min/max X
		if ( pt[ i ].x < pRect->left ) pRect->left = pt[ i ].x;
		else if ( pt[ i ].x > pRect->right ) pRect->right = pt[ i ].x;

		// Find min/max Y
		if ( pt[ i ].y < pRect->top ) pRect->top = pt[ i ].y;
		else if ( pt[ i ].y > pRect->bottom ) pRect->bottom = pt[ i ].y;

	} // end for

	return TRUE;
}

double CGrDC::GetAngle(long x1, long y1, long x2, long y2)
{	return -atan2( (double)( y2 - y1 ), (double) ( x2 - x1 ) ) * (double)360 / PI2;
}

double CGrDC::GetAngle(double x1, double y1, double x2, double y2)
{	return -atan2( y2 - y1, x2 - x1 ) * 360 / PI2;
}

BOOL CGrDC::PtInPolygon(LPPOINT pt, LPPOINT ppt, DWORD pts)
{_STT();
	HRGN hRgn = ::CreatePolygonRgn( ppt, pts, WINDING );
	if ( hRgn == NULL ) return FALSE;

	BOOL hit = PtInRegion( hRgn, pt->x, pt->y );

	::DeleteObject( hRgn );

	return hit;
}

BOOL CGrDC::FillHatchedPolygon(HDC hDC, LPPOINT pt, DWORD pts, COLORREF bck, COLORREF lines, int hatch)
{_STT();
	// Sanity check
	if ( hDC == NULL || pt == NULL || pts <= 1 ) return FALSE;

	// Fill in the rect
	HBRUSH hBrush = CreateHatchBrush( hatch, lines );
	HBRUSH hOldBrush = (HBRUSH)::SelectObject( hDC, hBrush );

	COLORREF oldbk = SetBkColor( hDC, bck );

	Polygon( hDC, pt, pts );

	SetBkColor( hDC, oldbk );

	::SelectObject( hDC, hOldBrush );
	::DeleteObject( hBrush );

	return TRUE;
}

BOOL CGrDC::DottedBox(HDC hDC, LPRECT pRect, COLORREF rgbPen, COLORREF rgbBck)
{_STT();
	// Setup
	HPEN hPen = CreatePen( PS_DOT, 1, rgbPen );
	HPEN hOldPen = (HPEN)::SelectObject( hDC, hPen );
	HBRUSH hBrush = CreateSolidBrush( rgbBck );
	HBRUSH hOldBrush = (HBRUSH)::SelectObject( hDC, (HBRUSH)GetStockObject( NULL_BRUSH ) );
	COLORREF oldcolor = SetBkColor( hDC, rgbBck );

	// Draw the box
	Rectangle( hDC, pRect->left, pRect->top, pRect->right, pRect->bottom );

	// Release drawing objects
	SetBkColor( hDC, oldcolor );
	::SelectObject( hDC, hOldBrush );
	::SelectObject( hDC, hOldPen );
	::DeleteObject( hPen );

	return TRUE;
}

BOOL CGrDC::FillSolidEllipse(HDC hDC, LPRECT pRect, COLORREF pen, COLORREF bck)
{_STT();
	HPEN hPen = CreatePen( PS_SOLID, 1, pen );
	HPEN hOldPen = (HPEN)::SelectObject( hDC, hPen );
	HBRUSH hBrush = CreateSolidBrush( bck );
	HBRUSH hOldBrush = (HBRUSH)::SelectObject( hDC, hBrush );

	// Draw the ellipse
	::Ellipse( hDC, pRect->left, pRect->top, pRect->right, pRect->bottom );

	::SelectObject( hDC, hOldBrush );
	::DeleteObject( hBrush );
	::SelectObject( hDC, hOldPen );
	::DeleteObject( hPen );

	return TRUE;
}

COLORREF CGrDC::HtmlToRgb(LPCTSTR pHtml)
{_STT();
	if ( pHtml == NULL || *pHtml != '#' || strlen( pHtml ) < 7 ) return 0;

	// Convert to RGB value
	return RGB( ahtodw( &pHtml[ 1 ], 2 ),
				ahtodw( &pHtml[ 3 ], 2 ),
				ahtodw( &pHtml[ 5 ], 2 ) );
}

LPCTSTR CGrDC::RgbToHtml(COLORREF rgb, LPSTR pHtml)
{_STT();
	if ( pHtml == NULL ) return NULL;

	DWORD i = 0;

	// Convert to HTML format
	pHtml[ i++ ] = '#';
	i += dwtohstr( GetRValue( rgb ), pHtml, i );
	i += dwtohstr( GetGValue( rgb ), pHtml, i );
	i += dwtohstr( GetBValue( rgb ), pHtml, i );

	return pHtml;
}

DWORD CGrDC::dwtohstr(DWORD num, LPSTR buf, DWORD index, DWORD pad, DWORD max)
{_STT();
	DWORD 	i = 0;
	DWORD	c = 8;

	// Correct pointer
	buf = &buf[ index ];
	
	while ( c )
	{
		buf[ i ] = (BYTE)( '0' + ( ( num & 0xf0000000 ) >> 28 ) );
		if ( buf[ i ] > '9' ) buf[ i ] += 'A' - '9' - 1;
		if ( pad >= c || i || buf[ i ] != '0' ) if ( i < max ) i++;
		num <<= 4;
 		c--;
	} // end while
	
	if ( !i ) buf[ i++ ] = '0';
	buf[ i ] = 0;

	return i;
}

DWORD CGrDC::ahtodw(LPCTSTR pBuffer, DWORD dwBytes)
{_STT();
	DWORD num = 0;
                                              
	// For Each ASCII Digit
	for ( DWORD i = 0; i < dwBytes; i++ )
	{
		// Convert ASCII Digit Between 0 And 9
		if ( pBuffer[ i ] >= '0' && pBuffer[ i ] <= '9' )
			num = ( num << 4 ) + ( pBuffer[ i ] - '0' );
		
		// Convert ASCII Digit Between A And F
		else if ( pBuffer[ i ] >= 'A' && pBuffer[ i ] <= 'F' )
			num = ( num << 4 ) + ( pBuffer[ i ] - 'A' ) + 10;

		// Convert ASCII Digit Between a And f
		else if ( pBuffer[ i ] >= 'a' && pBuffer[ i ] <= 'f' )
			num = ( num << 4 ) + ( pBuffer[ i ] - 'a' ) + 10;

	} // end for
    
	return num;	
}

BOOL CGrDC::Line(HDC hDC, COLORREF rgb, long w, long x1, long y1, long x2, long y2, int nStyle, COLORREF rgbBrush )
{_STT();
	CGrPen pen;
	CGrBrush brush;

	pen.Create( nStyle, w, rgb );

	HPEN hOldPen = (HPEN)::SelectObject( hDC, (HPEN)pen );
	HBRUSH hOldBrush = NULL;
	if ( rgbBrush != MAXDWORD ) 
	{	brush.Create( rgbBrush );
		hOldBrush = (HBRUSH)::SelectObject( hDC, (HBRUSH)brush );
	} // end if

	::MoveToEx( hDC, x1, y1, NULL );
	::LineTo( hDC, x2, y2 );

	// Select old objects
	::SelectObject( hDC, hOldPen );
	if ( hOldBrush ) ::SelectObject( hDC, hOldBrush );

	return TRUE;
}

BOOL CGrDC::DrawIcon(HDC hDC, LPRECT pRect, HICON hIcon, long x, long y, BOOL bDisabled )
{_STT();
	if ( hDC == NULL || hIcon == NULL ) return FALSE;

	RECT icon;

	// Copy position
	if ( pRect != NULL ) CopyRect( &icon, pRect );
	else SetRect( &icon, x, y, x + 16, y + 16 );

	ICONINFO	ii;

	// Get icon info
	if ( ::GetIconInfo( hIcon, &ii ) && ii.hbmColor != NULL )
	{
		// Figure out how big this thing is
		BITMAP	info;
		if ( ::GetObject( ii.hbmColor, sizeof( info ), &info ) )
		{
			// Get icon size
			icon.right = icon.left + info.bmWidth;
			icon.bottom = icon.top + info.bmHeight;

			// Center rectangle
			if ( pRect != NULL ) Center( &icon, pRect );

		} // end if

		// Delete the bitmaps
		DeleteObject( ii.hbmColor );
		DeleteObject( ii.hbmMask );

	} // end if	   

	if ( bDisabled )
	{	
		// Create disabled icon
		HICON hGrayIcon = CGrDC::CreateMonoChromeIcon( hIcon, GetSysColor( COLOR_3DFACE ) );

		// Draw the disabled icon
		BOOL ret = DrawIconEx(	hDC, icon.left, icon.top, hGrayIcon, 
								RW( icon ), RH( icon ), 0, NULL, 
								DI_IMAGE | DI_MASK );

		// Lose the disabled icon
		DestroyIcon( hGrayIcon );

		return ret;

	} // end if

	// Draw the icon
	return DrawIconEx(	hDC, icon.left, icon.top, hIcon, 
						RW( icon ), RH( icon ), 0, NULL, DI_IMAGE | DI_MASK );
}

HICON CGrDC::CreateMonoChromeIcon(HICON hIcon, COLORREF key, COLORREF bck )
{_STT();
	CWinDc dc;

	ICONINFO	ii;

	// Get icon info
	if ( !::GetIconInfo( hIcon, &ii ) || ii.hbmColor == NULL )
		return NULL;

	BITMAP	info;
	if ( !::GetObject( ii.hbmColor, sizeof( info ), &info ) )
		return NULL;

	// Create bitmap
	if ( !dc.Create( NULL, 0, 0, info.bmWidth, info.bmHeight ) )
		return NULL;

	CGrDC::FillSolidRect( dc, dc, RGB( 0, 0, 0 ) );

	// Draw the icon
	if ( !DrawIconEx(	dc, 0, 0, hIcon, dc.GetWidth(), dc.GetHeight(),
						0, NULL, DI_IMAGE | DI_MASK ) ) return NULL;

	// Monochrome icon
	dc.MonoChrome( key );

	// Create icon from dc
	HICON hRet = dc.CreateIcon( info.bmWidth, info.bmHeight, bck, NULL, ii.hbmMask );

	DeleteObject( ii.hbmColor );
	DeleteObject( ii.hbmMask );

	return hRet;
}

HICON CGrDC::CreateDisabledIcon(HICON hIcon, COLORREF bck )
{_STT();
	CWinDc dc;

	ICONINFO	ii;

	// Get icon info
	if ( !::GetIconInfo( hIcon, &ii ) || ii.hbmColor == NULL )
		return NULL;

	BITMAP	info;
	if ( !::GetObject( ii.hbmColor, sizeof( info ), &info ) )
		return NULL;

	// Create bitmap
	if ( !dc.Create( NULL, 0, 0, info.bmWidth, info.bmHeight ) )
		return NULL;

	DrawState(	dc, NULL, NULL, (LPARAM)hIcon, 0, 0, 0,
				dc.GetWidth(), dc.GetHeight(), DST_ICON | DSS_DISABLED );

	// Create icon from dc
	HICON hRet = dc.CreateIcon( info.bmWidth, info.bmHeight, bck, NULL, ii.hbmMask );

	DeleteObject( ii.hbmColor );
	DeleteObject( ii.hbmMask );

	return hRet;
}

BOOL CGrDC::GetIconRect(HICON hIcon, LPRECT pRect)
{_STT();
	// Sanity checks
	if ( hIcon == NULL || pRect == NULL ) return FALSE;

	ICONINFO	ii;

	// Get icon info
	if ( !::GetIconInfo( hIcon, &ii ) || ii.hbmColor == NULL )
		return FALSE;

	BITMAP	info;
	if ( ::GetObject( ii.hbmColor, sizeof( info ), &info ) )

		// Copy icon params
		SetRect( pRect, 0, 0, info.bmWidth, info.bmHeight );

	// Release bitmaps
	if ( ii.hbmColor != NULL ) DeleteObject( ii.hbmColor );
	if ( ii.hbmMask != NULL ) DeleteObject( ii.hbmMask );

	return TRUE;
}


BOOL CGrDC::DrawGrid(HDC hDC, LPRECT pRect, COLORREF rgb, long cx, long cy, long ox, long oy, int pentype )
{_STT();
	// Sanity checks
	if ( hDC == NULL || pRect == NULL || ( cx == 0 && cy == 0 ) ) return FALSE;

	CGrDC dc( hDC, FALSE );
	CGrPen pen( pentype, 1, rgb );
	dc.SelectObject( pen );

	// Draw vertical lines
	if ( cx > 0 ) for ( long x = pRect->left + ox; x < pRect->right; x += cx )
		dc.MoveTo( x, pRect->top ), dc.LineTo( x, pRect->bottom );

	// Draw horizontal lines
	if ( cy > 0 ) for ( long y = pRect->top + oy; y < pRect->bottom; y += cy )
		dc.MoveTo( pRect->left, y ), dc.LineTo( pRect->right, y );

	dc.Restore();

	return TRUE;
}

HICON CGrDC::RotateIcon(HICON hIcon, float angle)
{_STT();
#ifdef ENABLE_WINIMAGE

	CWinImg	img, mask;

	ICONINFO	ii;

	// Get icon info
	if ( !::GetIconInfo( hIcon, &ii ) || ii.hbmColor == NULL )
		return NULL;

	BITMAP	info;
	if ( !::GetObject( ii.hbmColor, sizeof( info ), &info ) )
		return NULL;

	// Create image
	if ( !img.CreateFromHBITMAP( ii.hbmColor ) ) return FALSE;

	// Create mask
	if ( ii.hbmMask != NULL ) mask.CreateFromHBITMAP( ii.hbmMask );

	// Rotate images (one day, I'll do my own rotate function)
	img.Rotate( angle );
	mask.Rotate( angle );

	// Create the icon
	HBITMAP hIconBmp = img.CreateHBITMAP();
	HBITMAP hMaskBmp = mask.CreateHBITMAP();

	// Create the icon
	HICON hRet = CGrDC::CreateIcon( info.bmWidth, info.bmHeight, 0, 0, 0, NULL, hIconBmp, hMaskBmp );

	if ( hIconBmp != NULL ) DeleteObject( hIconBmp );
	if ( hMaskBmp != NULL ) DeleteObject( hMaskBmp );
	if ( ii.hbmColor != NULL ) DeleteObject( ii.hbmColor );
	if ( ii.hbmMask != NULL ) DeleteObject( ii.hbmMask );

	return hRet;
#else
	return NULL;
#endif
}

float CGrDC::GetAngleDif(float a1, float a2)
{_STT();
	float max, min;

	min = a2 - a1;

	if ( min > 0 ) 
	{
		if ( min > 180 )
		{
			max = min;
			min = -360 + max;
		} // end if
		else max = -360 + min;

	} // end if
	else
	{
		if ( min < -180 )
		{
			max = min;
			min = 360 + max;
		} // end if
		else max = 360 + min;
	} // end else if

	return min;
}

BOOL CGrDC::FillHatchedRect(HDC hDC, LPRECT pRect, COLORREF bck, COLORREF lines, int hatch)
{_STT();
	// Sanity check
	if ( hDC == NULL ) return FALSE;

	// Fill in the rect
	HBRUSH hBrush = CreateHatchBrush( hatch, lines );
	HBRUSH hOldBrush = (HBRUSH)::SelectObject( hDC, hBrush );

	COLORREF oldbk = SetBkColor( hDC, bck );

	FillRect( hDC, pRect, hBrush );

	SetBkColor( hDC, oldbk );

	::SelectObject( hDC, hOldBrush );
	::DeleteObject( hBrush );

	return TRUE;
}

BOOL CGrDC::DrawSelectionBox(HDC hDC, LPRECT pRect, COLORREF rgbBck, COLORREF rgbHatch, DWORD dwTranslucency )
{_STT();
	// Wow
	CWinDc tl;
	tl.Create( hDC, pRect );

	FillHatchedRect( tl, tl, rgbBck, rgbHatch, HS_BDIAGONAL );

	tl.SetTransparency( TRUE );
	tl.SetTransparentColor( RGB( 0, 0, 0 ) );
	tl.CreateMask( RGB( 0, 0, 0 ) );

	tl.TlDraw( hDC, pRect, dwTranslucency, FALSE );

	return TRUE;
}

BOOL CGrDC::DrawPolygonOutline(HDC hDC, LPRECT pRect, LPPOINT ppt, DWORD pts, COLORREF rgb)
{_STT();
	if ( ppt == NULL || pts == 0 ) return FALSE;

	HPEN hPen = CreatePen( PS_DOT, 1, rgb );
	HPEN hOldPen = (HPEN)::SelectObject( hDC, hPen );
	HBRUSH hOldBrush = (HBRUSH)::SelectObject( hDC, GetStockObject( NULL_BRUSH ) );

	// Draw the selection
	Polygon( hDC, ppt, pts );

	::SelectObject( hDC, hOldBrush );
	::SelectObject( hDC, hOldPen );
	::DeleteObject( hPen );

	return TRUE;
}

BOOL CGrDC::DrawSelectionPolygon(HDC hDC, LPPOINT pPts, DWORD dwPts, COLORREF rgbBck, COLORREF rgbHatch, DWORD dwTranslucency)
{_STT();
	// Sanity checks
	if ( hDC == NULL || pPts == NULL || dwPts == 0 )
		return FALSE;

	RECT bounds;
	GetPolygonRect( &bounds, pPts, dwPts );

	// Wow
	CWinDc tl;
	tl.Create( hDC, &bounds );

	LPPOINT pt = new POINT[ dwPts ];
	if ( pt == NULL ) return FALSE;

	// Offset points
	for ( DWORD i = 0; i < dwPts; i++ )
	{	pt[ i ].x = pPts[ i ].x - bounds.left;
		pt[ i ].y = pPts[ i ].y - bounds.top;
	} // end if

	FillHatchedPolygon( tl, pt, dwPts, rgbBck, rgbHatch, HS_BDIAGONAL );

	delete [] pt;

/*	CBmp bmp;
	bmp.LoadFromHDC( tl );
	
	bmp.SetTransparency( TRUE );
	bmp.SetTransparentColor( RGB( 0, 0, 0 ) );
	bmp.CreateMask();
	
	bmp.TlFastDraw( hDC, &bounds, dwTranslucency, FALSE );
*/

	tl.SetTransparency( TRUE );
	tl.SetTransparentColor( RGB( 0, 0, 0 ) );
	tl.CreateMask( RGB( 0, 0, 0 ) );
	tl.TlDraw( hDC, &bounds, dwTranslucency, FALSE );

	return TRUE;
}

BOOL CGrDC::ScalePoints(LPPOINT psrcPts, LPPOINT pdstPts, DWORD dwPts, LPRECT pSrc, LPRECT pDst)
{_STT();
	long ox = pDst->left - pSrc->left;
	long oy = pDst->top - pSrc->top;

	long wsrc = PRW( pSrc ), wdst = PRW( pDst );
	long hsrc = PRH( pSrc ), hdst = PRH( pDst );

	for ( DWORD i = 0; i < dwPts; i++ )
	{	pdstPts[ i ].x = ox + ( ( psrcPts[ i ].x * wdst ) / wsrc );
		pdstPts[ i ].y = oy + ( ( psrcPts[ i ].y * hdst ) / hsrc );
	} // end for

	return TRUE;
}

BOOL CGrDC::Ellipse(HDC hDC, long sz, COLORREF rgb, LPPOINT pptCenter, long lRadius)
{_STT();
	if ( hDC == NULL || pptCenter == NULL ) return FALSE;

	// Create pen
	HPEN hPen = ::CreatePen( PS_SOLID, sz, rgb );
	HPEN hOldPen = (HPEN)::SelectObject( hDC, hPen );

	// NULL brush
	HBRUSH hOldBrush = (HBRUSH)::SelectObject( hDC, (HBRUSH)::GetStockObject( NULL_BRUSH ) );

	// Draw the circle
	::Ellipse( hDC,	pptCenter->x - lRadius,
					pptCenter->y - lRadius,
					pptCenter->x + lRadius,
					pptCenter->y + lRadius );

	// Clean up objects
	::SelectObject( hDC, hOldPen );
	::SelectObject( hDC, hOldBrush );
	::DeleteObject( hPen );

	return TRUE;
}

#if (WINVER < 0x0500)
	typedef struct { 
	  DWORD        bV5Size; 
	  LONG         bV5Width; 
	  LONG         bV5Height; 
	  WORD         bV5Planes; 
	  WORD         bV5BitCount; 
	  DWORD        bV5Compression; 
	  DWORD        bV5SizeImage; 
	  LONG         bV5XPelsPerMeter; 
	  LONG         bV5YPelsPerMeter; 
	  DWORD        bV5ClrUsed; 
	  DWORD        bV5ClrImportant; 
	  DWORD        bV5RedMask; 
	  DWORD        bV5GreenMask; 
	  DWORD        bV5BlueMask; 
	  DWORD        bV5AlphaMask; 
	  DWORD        bV5CSType; 
	  CIEXYZTRIPLE bV5Endpoints; 
	  DWORD        bV5GammaRed; 
	  DWORD        bV5GammaGreen; 
	  DWORD        bV5GammaBlue; 
	  DWORD        bV5Intent; 
	  DWORD        bV5ProfileData; 
	  DWORD        bV5ProfileSize; 
	  DWORD        bV5Reserved; 
	} BITMAPV5HEADER, *PBITMAPV5HEADER; 
#endif

HCURSOR CGrDC::IconToCursor(HICON hIcon, long lxHotSpot, long lyHotSpot)
{_STT();
	// Sanity checks
	if ( hIcon == NULL ) return NULL;
	
	// Get icon information
	ICONINFO ii;
	if ( !GetIconInfo( hIcon, &ii ) ) return FALSE;

	ii.fIcon = FALSE;
	ii.xHotspot = lxHotSpot;
	ii.yHotspot = lyHotSpot;

	// Create the cursor
	HCURSOR hCursor = CreateIconIndirect( &ii );

	// Delete the bitmaps
	if ( ii.hbmColor ) DeleteObject( ii.hbmColor );
	if ( ii.hbmMask ) DeleteObject( ii.hbmMask );

	return hCursor;
}

HICON CGrDC::CreateIcon(long w, long h, long lSrcW, long lSrcH, COLORREF bck, HDC hDC, HBITMAP hIconBmp, HBITMAP hIconMask)
{_STT();
	// Is there a bitmap?
	if ( hIconBmp == NULL ) return NULL;

	HICON hIcon = NULL;

	// Get a template DC
	BOOL bReleaseDC = FALSE;
	if ( hDC == NULL ) 
	{	bReleaseDC = TRUE;
		hDC = ::GetDC( NULL );
	} // end if

	// Create a bitmap
	HBITMAP hBmp = CreateCompatibleBitmap( hDC, w, h );
	if ( hBmp == NULL )
	{	if ( bReleaseDC ) ::ReleaseDC( NULL, hDC );
		return FALSE;
	} // end if

	// Create the mask
	HBITMAP hMask = CreateCompatibleBitmap( hDC, w, h );
	if ( hMask == NULL ) 
	{	::DeleteObject( hBmp );
		if ( bReleaseDC ) ::ReleaseDC( NULL, hDC );
		return FALSE;
	} // end if

	// Create a DC
	HDC hCDC = CreateCompatibleDC( hDC );
	if ( hCDC == NULL )
	{	::DeleteObject( hBmp );
		::DeleteObject( hMask );
		if ( bReleaseDC ) ::ReleaseDC( NULL, hDC );
		return FALSE;
	} // end if

	// Select the bitmap
	HBITMAP oldbmp = (HBITMAP)::SelectObject( hCDC, hBmp );

	// Prepare the icon bitmap
	RECT	rect;
	SetRect( &rect, 0, 0, w, h );

	// Fill in the background
	CGrDC::FillSolidRect( hCDC, &rect, bck );

	// Correct aspect ratio for image
	if ( lSrcW && lSrcH ) CGrDC::AspectCorrect( &rect, lSrcW, lSrcH );

	// Draw into the icon bitmap
	{	HDC hMDC = CreateCompatibleDC( hDC );
		if ( hMDC != NULL )
		{	HBITMAP hOldBmp = (HBITMAP)::SelectObject( hMDC, hIconBmp );
			::BitBlt( hCDC, rect.left, rect.top, RW( rect ), RH( rect ), hMDC, 0, 0, SRCCOPY );
			::SelectObject( hMDC, hOldBmp );
			::DeleteDC( hMDC );
		} // end if
	} // end if

	// Fill in the mask with zeros
	::SelectObject( hCDC, hMask );

	// Fill mask with zeros
	CGrDC::FillSolidRect( hCDC, &rect, RGB( 0, 0, 0 ) );

	// Did the caller supply a mask?
	if ( hIconMask != NULL )
	{	HDC hMDC = CreateCompatibleDC( hDC );
		if ( hMDC != NULL )
		{	HBITMAP hOldBmp = (HBITMAP)::SelectObject( hMDC, hIconMask );
			::BitBlt( hCDC, rect.left, rect.top, RW( rect ), RH( rect ), hMDC, 0, 0, SRCCOPY );
			::SelectObject( hMDC, hOldBmp );
			::DeleteDC( hMDC );
		} // end if
	} // end if

	// Lose the DC
	::SelectObject( hCDC, oldbmp );
	::DeleteDC( hCDC );

	// Fill in icon information
	ICONINFO	ii;
	ii.fIcon = TRUE;
	ii.xHotspot = w / 2;
	ii.yHotspot = h / 2;
	ii.hbmMask = hMask;
	ii.hbmColor = hBmp;

	// Create a new icon
	hIcon = ::CreateIconIndirect( &ii );

	// Clean up
	::DeleteObject( hBmp );
	::DeleteObject( hMask );
	if ( bReleaseDC ) ::ReleaseDC( NULL, hDC );

	return hIcon;
}

BOOL CGrDC::ExFill(HDC hDc, LPRECT pInside, LPRECT pOutside, COLORREF rgb)
{_STT();
	// Sanity check
	if ( hDc == NULL || pInside == NULL || pOutside == NULL ) 
		return FALSE;

	// Anything to do?
	if ( EqualRect( pInside, pOutside ) ) return TRUE;

	// Fill in the rect
	HBRUSH hBrush = CreateSolidBrush( rgb );

	RECT side;

	// Left space
	if ( pInside->left != pOutside->left )
	{	SetRect( &side, pOutside->left, pOutside->top, pInside->left, pOutside->bottom );
		::FillRect( hDc, &side, hBrush );
	} // end if

	// Right space
	if ( pInside->right != pOutside->right )
	{	SetRect( &side, pInside->right, pOutside->top, pOutside->right, pOutside->bottom );
		::FillRect( hDc, &side, hBrush );
	} // end if

	// Top space
	if ( pInside->top != pOutside->top )
	{	SetRect( &side, pInside->left, pOutside->top, pInside->right, pInside->top );
		::FillRect( hDc, &side, hBrush );
	} // end if

	// Bottom space
	if ( pInside->bottom != pOutside->bottom )
	{	SetRect( &side, pInside->left, pInside->bottom, pInside->right, pOutside->bottom );
		::FillRect( hDc, &side, hBrush );
	} // end if

	DeleteObject( hBrush );

	return TRUE;
}

BOOL CGrDC::Shade(LPBYTE pBuf, long lWidth, long lHeight, LPRECT pRect, COLORREF rgbShade)
{_STT();
	long lMaxY = pRect->bottom, lMaxX = pRect->right * 3;
	long lScanWidth = CWinDc::GetScanWidth( lWidth );
	for ( long y = pRect->top; y < lMaxY; y++ )
		for ( long x = pRect->left * 3; x < lMaxX; x += 3 )
		{
			long i = ( y * lScanWidth ) + x;
			LPBYTE pPix = &pBuf[ i ];

			// Reduce intensity
			pPix[ 0 ] >>= 1;
			pPix[ 1 ] >>= 1;
			pPix[ 2 ] >>= 1;

			// Shade 
			pPix[ 0 ] += GetBValue( rgbShade );
			pPix[ 1 ] += GetGValue( rgbShade );
			pPix[ 2 ] += GetRValue( rgbShade );

		} // end for

	return TRUE;
}

