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
// WinRect.cpp: implementation of the CWinRect class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWinRect::CWinRect()
{_STT();
}

CWinRect::~CWinRect()
{_STT();
}

BOOL CWinRect::AddRectToList(LPRECT pRect, LPRECT pRectList, DWORD dwRectListSize, LONG xThreshold, LONG yThreshold )
{_STT();
	// Add if possible
	RECT dst;
	DWORD i = 0, blank = MAXDWORD;
	for ( i = 0; i < dwRectListSize; i++ )
	{
		// If valid area
		if ( RW( pRectList[ i ] ) != 0 && RH( pRectList[ i ] ) != 0 )
		{	if ( IsAdjacentRect( &pRectList[ i ], pRect, xThreshold, yThreshold ) )
				if ( UnionRect( &dst, &pRectList[ i ], pRect ) )
				{	CopyRect( &pRectList[ i ], &dst ); return TRUE; }
		} // end if

		// Save blank location
		else if ( blank == MAXDWORD ) 
			blank = i;

	} // end for

	// Any more blank slots?
	if ( blank == MAXDWORD ) 
	{
		// Attempt to create an empty slot
		if ( !CombineOverlapping( pRectList, dwRectListSize, xThreshold, yThreshold, 1, &blank ) )
			return FALSE;

	} // end if

	// Add to slot
	CopyRect( &pRectList[ blank ], pRect );

	return TRUE;
}

BOOL CWinRect::IsAdjacentRect(LPRECT pRect1, LPRECT pRect2, LONG xThreshold, LONG yThreshold)
{_STT();
	// Sanity check
	if ( pRect1 == NULL || pRect2 == NULL ) 
		return FALSE;

	// Rect to the left?
	if ( pRect1->right < ( pRect2->left - xThreshold ) )
		return FALSE;

	// Rect to the right?
	if ( pRect1->left > ( pRect2->right + xThreshold ) )
		return FALSE;

	// Rect above
	if ( pRect1->bottom < ( pRect2->top - yThreshold ) )
		return FALSE;

	// Rect below
	if ( pRect1->top > ( pRect2->bottom + yThreshold ) )
		return FALSE;

	return TRUE;
}

DWORD CWinRect::CombineOverlapping(LPRECT pRectList, DWORD dwRectListSize, LONG xThreshold, LONG yThreshold, DWORD max, LPDWORD pdwFree)
{_STT();
	// Combine overlapping rects
	BOOL dwCombined = 0;
	for ( DWORD i = 0; i < dwRectListSize; i++ )
		for ( DWORD c = i + 1; c < dwRectListSize; c++ )
		{	RECT dst;
			if ( 	( RW( pRectList[ i ] ) != 0 && RH( pRectList[ i ] ) != 0 ) &&
					( RW( pRectList[ c ] ) != 0 && RH( pRectList[ c ] ) != 0 ) &&
					IsAdjacentRect( &pRectList[ i ], &pRectList[ c ] ) )
				if ( UnionRect( &dst, &pRectList[ i ], &pRectList[ c ] ) )
				{	dwCombined++;
					
					// Copy the new rect
					CopyRect( &pRectList[ i ], &dst ); 
					
					// Lose the old rect
					ZeroMemory( &pRectList[ c ], sizeof( pRectList[ c ] ) ); 

					// Save pointer to blank if needed
					if ( dwCombined == 1 && pdwFree != NULL ) *pdwFree = c;					

					// Punt if user doesn't want any more
					if ( !max ) return dwCombined;

					i = 0; c = 1; max--;

				} // end if
		} // end for

	return dwCombined;
}
