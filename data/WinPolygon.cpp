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
// WinPolygon.cpp: implementation of the CWinPolygon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWinPolygon::~CWinPolygon()
{_STT();
}


BOOL CWinPolygon::CombinePoint(LPPOINT pptList, DWORD dwListSize, LPPOINT ppt)
{_STT();
	LPLINE pLn = (LPLINE)pptList;
	if ( pLn == NULL ) return FALSE;

	return FALSE;
}

BOOL CWinPolygon::PtInPolygon( LPPOINT pptList, DWORD dwListSize, LPPOINT ppt )
{_STT();
	DWORD hits = 0, i = 0;
	long ySave = 0;

	// Skip halfline intersection
	while ( i < dwListSize && pptList[ i ].y == ppt->y ) i++;
	if ( i >= dwListSize ) return FALSE;

	// For each line
	for ( DWORD n = 0; n < dwListSize; n++ )
	{
		// Next point in line
		DWORD j = i + 1; if ( j >= dwListSize ) j = 0;

		// Distance
		long dx = pptList[ j ].x - pptList[ i ].x;
		long dy = pptList[ j ].y - pptList[ i ].y;

		// Ignore horizontal edges
		if ( dy != 0 )
		{
			// Check for halfline intersection
			int rx = ppt->x - pptList[ i ].x;
			int ry = ppt->y - pptList[ i ].y;

			// Deal with edges starting or ending on the halfline
			if ( pptList[ j ].y == ppt->y && pptList[ j ].x >= ppt->x )
				ySave = pptList[ i ].y;

			// Look for intersection
			if ( pptList[ i ].y == ppt->y && pptList[ i ].x >= ppt->x ) 
				if ( ( ySave > ppt->y ) != ( pptList[ j ].y > ppt->y ) ) hits--;

			// Tally intersections with halfline
			float s = (float)ry / (float)dy;
			if (s >= 0.0f && s <= 1.0f && ( s * dx ) >= rx ) hits++;

		} // end if

		// Skip to next line
		i = j;

	} // end for

	// Inside if odd intersections
	return hits & 1;
}

#define BP( x, y, w ) ( ( y * w ) + x )
#define BPSW( x, y, w, sw ) ( ( ( y * w ) + x ) * 3 ) + ( y * sw )
BOOL CWinPolygon::FromMap(LPDWORD map, long x, long y, long w, long h, UINT th, LPBYTE buf)
{_STT();
	// Get the size
	long sz = w * h; if ( sz <= 0 ) return FALSE;

	// sanity checks
	if ( map == NULL ) return FALSE;
	if ( x < 0 || x >= w ) return FALSE;
	if ( y < 0 || y >= h ) return FALSE;
	if ( th == 0 ) return FALSE;

	// Ensure first pixel passes the muster
	long i = BP( x, y, w );
	if ( map[ i ] < th ) return FALSE;

	// Allocate trail if needed
	TMem< BYTE > apTrail;
	if ( buf == NULL )
	{	if ( !apTrail.allocate( sz ) ) return FALSE;
		buf = apTrail;
	} // end if

	// Empty the trail
	ZeroMemory( buf, sz );

	// Do the fill
	while ( ( buf[ i ] & 0x0f ) <= 3 )
	{
		if ( ( buf[ i ] & 0x0f ) == 0 )
		{
			// Change the map value
			map[ i ] = 0;

			AddPoint( i % w, i / w );

			// Point to next direction
			buf[ i ] &= 0xf0, buf[ i ] |= 1;
	
			// Can we go up?
			if ( y < ( h - 1 ) )
			{	long n = BP( x, ( y + 1 ), w );
				if ( map[ n ] >= th ) 
				{	y++; i = BP( x, y, w ); buf[ i ] = 0x10; }
			} // end if

		} // end if

		if ( ( buf[ i ] & 0x0f ) == 1 )
		{
			// Point to next direction
			buf[ i ] &= 0xf0, buf[ i ] |= 2;

			// Can we go right?
			if ( x < ( w - 1 ) )
			{	long n = BP( ( x + 1 ), y, w );
				if ( map[ n ] >= th ) 
				{	x++; i = BP( x, y, w ); buf[ i ] = 0x20; }
			} // end if

		} // end if

		if ( ( buf[ i ] & 0x0f ) == 2 )
		{
			// Point to next direction
			buf[ i ] &= 0xf0, buf[ i ] |= 3;
		
			// Can we go down?
			if ( y > 0 )
			{	long n = BP( x, ( y - 1 ), w );
				if ( map[ n ] >= th ) 
				{	y--; i = BP( x, y, w ); buf[ i ] = 0x30; }
			} // end if

		} // end if

		if ( ( buf[ i ] & 0x0f ) == 3 )
		{
			// Point to next
			buf[ i ] &= 0xf0, buf[ i ] |= 4;

			// Can we go left
			if ( x > 0 )
			{	long n = BP( ( x - 1 ), y, w );
				if ( map[ n ] >= th ) 
				{	x--; i = BP( x, y, w ); buf[ i ] = 0x40; }
			} // end if

		} // end if

		// Time to backup?
		while ( ( buf[ i ] & 0xf0 ) > 0 && ( buf[ i ] & 0x0f ) > 3 )
		{
			// Go back
			if ( ( buf[ i ] & 0xf0 ) == 0x10 ) y--;
			else if ( ( buf[ i ] & 0xf0 ) == 0x20 ) x--;
			else if ( ( buf[ i ] & 0xf0 ) == 0x30 ) y++;
			else if ( ( buf[ i ] & 0xf0 ) == 0x40 ) x++;

			i = BP( x, y, w );

		} // end if

	} // end if

	return TRUE;
}


// *** Requires sorting ***
//
// Based on "Andrew's Monotone Chain algorithm"
//
// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.
//
BOOL CWinPolygon::ConvexHull()
{_STT();
	// How many edges?
	long lPoints = (long)GetNumPoints();
	if ( lPoints <= 3 ) return TRUE;

	// Get edge pointer
	LPPOINT pPts = GetPoints();
	if ( pPts == NULL ) return FALSE;

	// Allocate new array of points
	TMem< POINT > npts;
	if ( !npts.allocate( lPoints + 1 ) ) return FALSE;

	LPPOINT pNPts = npts;

	long bot = 0, top = -1;

	// Find the bottom
	long i, minmin = 0, minmax, xmin = pPts[ 0 ].x;
	for ( i = 1; i < lPoints && pPts[ i ].x == xmin; i++ );
	minmax = i - 1;

	// Verify valid points
	if ( i == lPoints ) return FALSE;

	// Find the top
	long maxmin, maxmax = lPoints - 1, xmax = pPts[ lPoints - 1 ].x;
	for ( i = lPoints - 2; i >= 0 && pPts[ i ].x == xmax; i-- );
	maxmin = i + 1;

	// Verify valid points
	if ( minmax >= maxmin ) return FALSE;

	pNPts[ ++top ] = pPts[ minmin ];

	i = minmax;
	while( ++i <= maxmin )
	{
		// lower line joines minmin to maxmin
		if ( isLeft( pPts[ minmin ], pPts[ maxmin ], pPts[ i ] ) >= 0 && i < maxmin )
			continue;

		// if at least two points on the stack
		while ( top > 0 )
		{
			// test if i is left of the line at top of stack
			if ( isLeft( pNPts[ top - 1 ], pNPts[ top ], pPts[ i ] ) > 0 ) break;
			else top--;

		} // end while

		// push i onto stack
		pNPts[ ++top ] = pPts[ i ];

	} // end while


	if ( maxmax != maxmin ) pNPts[ ++top ] = pPts[ maxmax ];
	bot = top; i = maxmin;

	while( --i >= minmax )
	{
		if ( isLeft( pPts[ maxmax ], pPts[ minmax ], pPts[ i ] ) >= 0 && i > minmax )
			continue;

		while( top > bot )
		{
			if ( isLeft( pNPts[ top - 1 ], pNPts[ top ], pPts[ i ] ) > 0 )
				break;
			else top--;

		} // end while

		pNPts[ ++top ] = pPts[ i ];

	} // end while

	if ( minmax != minmin ) pNPts[ ++top ] = pPts[ minmin ];


	// new size is ( top + 1 )
	m_dwPts = top + 1;

	// Switch to new point list
	m_pts.destroy();
	m_pts.attach( npts.detach(), m_dwPts );

	return TRUE;
}

float CWinPolygon::SimpleArea()
{_STT();
	// How many edges?
	long lPoints = (long)GetNumPoints();
	if ( lPoints <= 2 ) return 0;

	// Get edge pointer
	LPPOINT pPts = GetPoints();
	if ( pPts == NULL ) return 0;

	// Calculate area
    float area = 0;
    for ( long i = 1, j = 2, k = 0; j < lPoints; i++, j++, k++ )
        area += pPts[ i ].x * ( pPts[ j ].y - pPts[ k ].y );
	if ( area < 0 ) area = -area;

    return area / 2.f;
}


// 
// Similar to the BFP Approximate Hull Algorithm
// ( Bentley-Faust-Preparata, 1982 )
// but slightly faster
//
#define HULL_SECTIONS			32
BOOL CWinPolygon::ApxConvexHull()
{_STT();
	// How many points?
	long lPoints = GetNumPoints();
	if ( lPoints <= 3 ) return TRUE;

	// Get edge pointer
	LPPOINT pPts = GetPoints();
	if ( pPts == NULL ) return FALSE;

	// Allocate new array of points
	TMem< POINT > npts;
	if ( !npts.allocate( lPoints ) ) return FALSE;
	LPPOINT pNPts = npts;

	// Find xrange
	long i, xmin = pPts[ 0 ].x, xmax = pPts[ 0 ].x;
	for ( i = 1; i < lPoints; i++ )
		if ( pPts[ i ].x < xmin ) xmin = pPts[ i ].x;
		else if ( pPts[ i ].x > xmax ) xmax = pPts[ i ].x;

	// Ensure valid points
	if ( xmin == MAXLONG || xmax == MAXLONG ) 
		return FALSE;

	// Calculate range
	long xr = xmax - xmin;
	if ( xr <= 0 ) return FALSE;

	// Ensure no data overflow
	ASSERT( ( (double)xmax * (double)HULL_SECTIONS ) < (double)MAXLONG );

	// Allocate bins and mark invalid
	CWinPolygon::RANGE	bin[ HULL_SECTIONS ];
	for ( i = 0; i < HULL_SECTIONS; i++ )
		bin[ i ].min.x = bin[ i ].max.x = MAXLONG;

	// Min max points
	POINT min = pPts[ 0 ], max = pPts[ 0 ];

	// Get min/max points
	for ( i = 0; i < lPoints; i++ )	
	{
		if ( pPts[ i ].x < min.x ) min = pPts[ i ];
		else if ( pPts[ i ].x == min.x && pPts[ i ].y < min.y )
			min = pPts[ i ];
		
		if ( pPts[ i ].x > max.x ) max = pPts[ i ];
		else if ( pPts[ i ].x == max.x && pPts[ i ].y > max.y ) 
			max = pPts[ i ];
	} // end if

	// Get ranges
	for ( i = 0; i < lPoints; i++ )	
	{	
		long xi = ( ( pPts[ i ].x - xmin ) * ( HULL_SECTIONS - 1 ) / xr );

		ASSERT( xi < HULL_SECTIONS );
		
		// Top
		// sometimes works better with, sometimes without ???
//		if ( isLeft( min, max, pPts[ i ] ) > 0 )
		{	if ( bin[ xi ].max.x == MAXLONG || pPts[ i ].y > bin[ xi ].max.y )
				bin[ xi ].max = pPts[ i ];
		} // end if

		// Bottom
//		else
			if ( bin[ xi ].min.x == MAXLONG || pPts[ i ].y < bin[ xi ].min.y ) 
				bin[ xi ].min = pPts[ i ];

	} // end for
	
	// First the top half
	long lNPoints = 0;
	for ( i = 0; lNPoints < lPoints && i < HULL_SECTIONS; i++ )
	{	
		// If valid max
		if ( bin[ i ].max.x != MAXLONG )
		{
			// Backup if inside this point
			while ( ( lNPoints > 1 ) && isLeft( pNPts[ lNPoints - 2 ], pNPts[ lNPoints - 1 ], bin[ i ].max ) < 0 )
				lNPoints--;

			// Save this point
			pNPts[ lNPoints++ ] = bin[ i ].max;
		
		} // end if

	} // end for
		
	// Now for the bottom half
	long lRight = lNPoints + 1;
	for ( i = HULL_SECTIONS - 1; lNPoints < lPoints && i >= 0; i-- )
	{	
		// If valid min
		if ( bin[ i ].min.x != MAXLONG )
		{
			// Backup if inside this point
			while ( lNPoints > lRight && isLeft( pNPts[ lNPoints - 2 ], pNPts[ lNPoints - 1 ], bin[ i ].min ) < 0 )
				lNPoints--;

			// Save this point
			pNPts[ lNPoints++ ] = bin[ i ].min;

		} // end if

	} // end for

	// Just punt if no change
	if ( m_dwPts == (DWORD)lNPoints ) return TRUE;

	// New size
	m_dwPts = lNPoints;

	// Switch to new point list
	m_pts.destroy();
	m_pts.attach( npts.detach(), m_dwPts );

	return TRUE;
}


BOOL CWinPolygon::Inflate(long x, long y)
{_STT();
	// How many edges?
	DWORD dwPoints = (long)GetNumPoints();
	if ( dwPoints <= 2 ) return 0;

	// Get edge pointer
	LPPOINT pPts = GetPoints();
	if ( pPts == NULL ) return 0;

	POINT ptCenter = CalculateCenter();

    for ( DWORD i = 0; i < dwPoints; i++ )
	{
		if ( pPts[ i ].x < ptCenter.x ) pPts[ i ].x -= x;
		else pPts[ i ].x += x;
		
		if ( pPts[ i ].y < ptCenter.y ) pPts[ i ].y -= y;
		else pPts[ i ].y += y;

	} // end for

	return TRUE;
}


BOOL CWinPolygon::ScanSort()
{_STT();
	// How many edges?
	long lPoints = (long)GetNumPoints();
	if ( lPoints <= 2 ) return 0;

	// Get edge pointer
	LPPOINT pPts = GetPoints();
	if ( pPts == NULL ) return 0;

	// Sort
	rScanSort( pPts, 0, lPoints - 1 );

	return TRUE;
}

void CWinPolygon::rScanSort(LPPOINT pPts, DWORD left, DWORD right)
{_STT();
	POINT register pivot = pPts[ left ];
	DWORD register l = left;
	DWORD register r = right;

	while ( l < r )
	{
		// Find smaller value
		while ( ComparePoint( pPts[ r ], pivot ) >= 0 && l < r ) r--;

		// Need to copy?
		if ( l != r ) pPts[ l++ ] = pPts[ r ];

		// Find larger value
		while ( ComparePoint( pPts[ l ], pivot ) <= 0 && l < r ) l++;

		// Need to copy?
		if ( l != r ) pPts[ r-- ] = pPts[ l ];
		
	} // end while

	// Copy pivot point
	pPts[ l ] = pivot;

	// Recurse to sort sub lists
	if ( left < l ) rScanSort( pPts, left, l - 1 );
	if ( right > l ) rScanSort( pPts, l + 1, right );
}

POINT CWinPolygon::CalculateCenter()
{_STT();
	POINT pt;
	LPPOINT pPts = m_pts; 	
	if ( pPts == NULL || m_dwPts == 0 ) 
	{	pt.x = MAXLONG; pt.y = MAXLONG; 
		return pt; 
	} // end if

	long minx, maxx, miny, maxy;
	minx = maxx = pPts[ 0 ].x;
	miny = maxy = pPts[ 0 ].y;

	for ( DWORD i = 1; i < m_dwPts; i++ )
	{	if ( pPts[ i ].x < minx ) minx = pPts[ i ].x;
		else if ( pPts[ i ].x > maxx ) maxx = pPts[ i ].x;
		if ( pPts[ i ].y < miny ) miny = pPts[ i ].y;
		else if ( pPts[ i ].y > maxy ) maxy = pPts[ i ].y;
	} // end for

	pt.x = minx + ( ( maxx - minx ) >> 1 ); 
	pt.y = miny + ( ( maxy - miny ) >> 1 ); 
	
	return pt;
}

// This is slow (2On^2), please replace
BOOL CWinPolygon::Intersect(LPPOINT pPts, DWORD dwPts)
{_STT();
	DWORD dwMyPts = (long)GetNumPoints();
	LPPOINT pMyPts = GetPoints();

	// Sanity check
	if ( dwMyPts == 0 || dwPts == 0 ) return FALSE;
	if ( pMyPts == 0 || pPts == NULL ) return 0;

	// Check passed points against ours
	DWORD i;
	for ( i = 0; i < dwPts; i++ )
		if ( PtInPolygon( pMyPts, dwMyPts, &pPts[ i ] ) )
			return TRUE;

	// Check our points against passed points
	for ( i = 0; i < dwMyPts; i++ )
		if ( PtInPolygon( pPts, dwPts, &pMyPts[ i ] ) )
			return TRUE;

	return FALSE;
}


BOOL CWinPolygon::Scale(LPRECT pSrc, LPRECT pDst)
{_STT();
	// Get point information
	DWORD dwPts = GetNumPoints();		
	LPPOINT pPts = GetPoints();
	if ( dwPts == 0 || pPts == NULL )
		return FALSE;

	TMem< POINT >	pts;
	if ( !pts.allocate( dwPts ) ) return FALSE;
	
	// Scale the points
	if ( !CGrDC::ScalePoints( pPts, pts, dwPts, pSrc, pDst ) )
		return FALSE;

	// Switch buffers
	m_pts.destroy();
	m_pts.attach( pts.detach() );

	return TRUE;
}

BOOL CWinPolygon::GetIBeam(LPPOINT ptTop, LPPOINT ptBottom, BOOL bInvY)
{_STT();
	LPPOINT pPts = m_pts; 	
	if ( pPts == NULL || m_dwPts == 0 ) 
		return FALSE; 

	ptBottom->x = ptTop->x = pPts[ 0 ].x;
	ptBottom->y = ptTop->y = pPts[ 0 ].y;

	if ( bInvY )
	{
		for ( DWORD i = 1; i < m_dwPts; i++ )
		{
			// Check for top position
			if ( pPts[ i ].y < ptTop->y )
			{	ptTop->x = pPts[ i ].x;
				ptTop->y = pPts[ i ].y;
				ptBottom->x = pPts[ i ].x;				
			} // end if

			// Check for bottom
			if ( pPts[ i ].y > ptBottom->y )
				ptBottom->y = pPts[ i ].y;

		} // end for

	} // end if

	return TRUE;
}
