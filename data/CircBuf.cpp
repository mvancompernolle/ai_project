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
// CircBuf.cpp: implementation of the CCircBuf class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCircBuf::CCircBuf( BOOL bSync, DWORD dwSize, BOOL bAutoGrow )
{_STTEX();
	m_bEmpty = TRUE;
	m_dwSize = 0;
	m_dwReadPtr = 0;
	m_dwWritePtr = 0;

	m_bAutoGrow = bAutoGrow;
	m_dwMaxSize = CCIRCBUF_DEFAULT_MAXBUFFER;

	if ( bSync ) m_hDataReady = CreateEvent( NULL, TRUE, FALSE, NULL );
	else m_hDataReady = NULL;

	if ( dwSize ) Allocate( dwSize );
}


CCircBuf::~CCircBuf()
{_STTEX();
	Destroy();

	// Lose the sync
	if ( m_hDataReady )
	{	CloseHandle( m_hDataReady );
		m_hDataReady = NULL;
	} // end if
}

BOOL CCircBuf::Allocate( DWORD dwSize )
{_STTEX();
	// Lock the buffer
	CTlLocalLock ll( &m_lock );
	if ( !ll.IsLocked() ) return FALSE;

	// Lose the old buffer
	Destroy(); 
	
	// Create a new one
	if ( !m_buf.allocate( dwSize ) ) 
		return FALSE; 
	
	// Remember the size of the new buffer
	m_dwSize = dwSize; 
	
	return TRUE; 
}

DWORD CCircBuf::Resize(DWORD dwNewSize)
{_STTEX();
	// Lock the buffer
	CTlLocalLock ll( &m_lock );
	if ( !ll.IsLocked() ) return FALSE;
	
	// Ensure we don't go over the maximum
	if ( dwNewSize > m_dwMaxSize ) return FALSE;

	// Punt if we're already there
	if ( dwNewSize == m_dwSize ) return FALSE;

	// Create new buffer
	TMem< BYTE > temp;
	if ( !temp.allocate( dwNewSize ) ) 
		return FALSE;

	DWORD dwWritten = GetMaxRead( m_dwReadPtr, m_dwWritePtr, m_dwSize );
	DWORD dwPoked = GetMaxRead( m_dwReadPtr, m_dwPokePtr, m_dwSize );
	DWORD dwUsed = MAX_VAL( dwPoked, dwWritten );

	// How many will we copy over?
	if ( dwUsed > m_dwSize ) dwUsed = m_dwSize;
	if ( dwUsed > dwNewSize ) dwUsed = dwNewSize;

	if ( dwUsed )
	{
		LPBYTE pView;
		DWORD i = 0, dwRead = 0, dwView;

		// Read all the blocks into the buffer
		while ( GetView( i++, m_dwReadPtr, dwUsed, m_buf, m_dwSize, &pView, &dwView ) )
			memcpy( &( (LPBYTE)temp )[ dwRead ], pView, dwView ), dwRead += dwView;

	} // end if

	// Lose the old buffer
	Destroy();

	// Swap over the buffer pointer
	m_buf.attach( temp );

	// Save the new buffer size
	m_dwSize = dwNewSize;

	// Adjust write pointers
	m_dwWritePtr = dwWritten;
	m_dwPokePtr = dwPoked;

	// Return the new buffer size
	return dwNewSize;		
}

DWORD CCircBuf::EnsureWriteSpace( DWORD dwSize, DWORD dwReadPtr, DWORD dwWritePtr, DWORD dwMax )
{_STTEX();
	// We need one more byte as a marker
	dwSize++;

	// Do we have enough space?
	DWORD dwAvailable = GetMaxWrite( dwReadPtr, dwWritePtr, dwMax );
	if ( dwAvailable >= dwSize ) return TRUE;

	// Double the amount of space
	dwAvailable = 2; 
	while ( dwAvailable && dwAvailable < ( m_dwSize + dwSize ) ) dwAvailable <<= 1;

	// Do we have a valid size?
	if ( !dwAvailable ) return FALSE;

	// Create a buffer large enough for the data
	Resize( dwAvailable );

	return TRUE;
}

void CCircBuf::Destroy()
{_STTEX();
	// Lock the buffer
	CTlLocalLock ll( &m_lock );
	if ( !ll.IsLocked() ) return;
	
	// Empty the buffer
	Empty();

	m_dwSize = 0;
	m_buf.destroy();
}

BOOL CCircBuf::Read(LPVOID pBuf, DWORD dwMax, LPDWORD pdwRead, LPDWORD pdwPtr, DWORD dwEncode )
{_STTEX();
	// Lock the buffer
	CTlLocalLock ll( &m_lock );
	if ( !ll.IsLocked() ) return FALSE;

	// Where to start?
	DWORD dwPtr = NormalizePtr( pdwPtr ? *pdwPtr : 0, m_dwSize );

	// Anything to read?
	DWORD dwSize = GetMaxRead( dwPtr, m_dwWritePtr, m_dwSize );
	if ( !dwSize ) return FALSE;

	// Do they just want to know the size?
	if ( pBuf == NULL || dwMax == 0 )
	{	if ( pdwRead ) *pdwRead = dwSize;
		return TRUE;
	} // end if

	// Are we short a few bytes?
	if ( dwSize < dwMax ) 
	{
		// Assume we MUST read dwMax if we can't tell the user otherwise
		if ( pdwRead == NULL ) return FALSE;

	} // end if

	// Do we have more than enough?
	else if ( dwSize > dwMax )
	{		
		// Truncate to user buffer
		dwSize = dwMax; 

	} // end else

	LPBYTE pView;
	DWORD i = 0, dwRead = 0, dwView;

	// Read all the blocks into the buffer
	while ( GetView( i, dwPtr, dwSize, m_buf, m_dwSize, &pView, &dwView ) )
	{
		// Copy the data
		memcpy( &( (LPBYTE)pBuf )[ dwRead ], pView, dwView );

		// Decode if needed
		if ( dwEncode ) OnDecode( dwEncode, i, &( (LPBYTE)pBuf )[ dwRead ], dwView );

		// For inspecting the actual read data
		OnInspectRead( i, pView, dwView );

		// Keep count of the bytes read
		dwRead += dwView;

		// Next block
		i++;

	} // end while

	// Update the pointer if required
	if ( pdwPtr ) *pdwPtr = NormalizePtr( AdvancePtr( dwPtr, dwSize, m_dwSize ), m_dwSize );

	// How many bytes were read?
	if ( pdwRead ) *pdwRead = dwRead;

	return TRUE;
}

BOOL CCircBuf::Write(void const * pBuf, DWORD dwSize, LPDWORD pdwPtr, DWORD dwEncode )
{_STTEX();
	// Lock the buffer
	CTlLocalLock ll( &m_lock );
	if ( !ll.IsLocked() ) return FALSE;

	// Just ignore a NULL write
	if ( dwSize == 0 ) return TRUE;

	// Where to start?
	DWORD dwPtr = NormalizePtr( pdwPtr ? *pdwPtr : 0, m_dwSize );

	// Ensure buffer space
	if ( dwSize > GetMaxWrite( m_dwReadPtr, dwPtr, m_dwSize ) ) return FALSE;

	LPBYTE pView;
	DWORD i = 0, dwWritten = 0, dwView;

	// Read all the blocks into the buffer
	while ( GetView( i, dwPtr, dwSize, m_buf, m_dwSize, &pView, &dwView ) )
	{	
		// Copy the data
		memcpy( pView, &( (LPBYTE)pBuf )[ dwWritten ], dwView );

		// Encode if needed
		if ( dwEncode ) OnEncode( dwEncode, i, pView, dwView );

		// For inspecting the actual Write data
		OnInspectWrite( i, pView, dwView );

		// Track the number of bytes written
		dwWritten += dwView;

		// Next block
		i++;

	} // end while


	// Update the pointer if required
	if ( pdwPtr ) *pdwPtr = AdvancePtr( dwPtr, dwSize, m_dwSize );

	return TRUE;
}


DWORD CCircBuf::GetMaxRead( DWORD dwReadPtr, DWORD dwWritePtr, DWORD dwSize )
{_STTEX();
	// Do we have a buffer?
	if ( dwSize == 0 ) return 0;

	// Is the buffer empty?
	if ( dwReadPtr == dwWritePtr ) return 0;

	// Check for inside use
	if ( dwWritePtr > dwReadPtr ) 
		return dwWritePtr - dwReadPtr;

	// Outside use
	return dwSize - ( dwReadPtr - dwWritePtr );
}

DWORD CCircBuf::GetMaxWrite( DWORD dwReadPtr, DWORD dwWritePtr, DWORD dwSize )
{_STTEX();
	// Do we have a buffer?
	if ( dwSize == 0 ) return 0;

	// Must normalize the write pointer
	DWORD dwNWritePtr = dwWritePtr;
	if ( dwNWritePtr >= dwSize ) dwNWritePtr %= dwSize;

	// Is the buffer empty?
	if ( dwReadPtr == dwNWritePtr ) 
	{	if ( dwReadPtr != dwWritePtr ) 
			return 0;
		return dwSize;
	} // end if

	// Check for inside use
	if ( dwNWritePtr > dwReadPtr ) 
		return dwSize - ( dwNWritePtr - dwReadPtr );

	// Check for unusable space	( write pointer can't advance onto read pointer )
	if ( ( dwNWritePtr + 1 ) == dwReadPtr )
		return 0;

	// Outside use
	return dwReadPtr - dwNWritePtr;
}

BOOL CCircBuf::Write(LPCTSTR pStr)
{_STTEX();
	// Write the string into the buffer
	return Write( pStr, strlen( pStr ) );
}

BOOL CCircBuf::Read(LPSTR pStr, DWORD dwMax)
{_STTEX();
	DWORD dwRead = 0;

	// Read the string from the buffer
	Read( pStr, dwMax, &dwRead );

	// NULL terminate string
	if ( dwRead >= dwMax ) dwRead = dwMax - 1;
	pStr[ dwRead ] = 0;

	return TRUE;
}

BOOL CCircBuf::Read( LPVOID pBuf, DWORD dwMax, LPDWORD pdwRead, DWORD dwEncode )
{_STTEX();
	if ( !Read( pBuf, dwMax, pdwRead, &m_dwReadPtr, dwEncode ) ) return FALSE;
	Defrag(); return TRUE;
}

BOOL CCircBuf::Peek( LPVOID pBuf, DWORD dwMax, LPDWORD pdwRead, long lOffset, DWORD dwEncode )
{_STTEX();
	if ( !m_dwSize ) return FALSE;

	// Where to peek
	DWORD dwReadPtr = m_dwReadPtr;	

	// Offset into the buffer
	dwReadPtr = AdvancePtr( dwReadPtr, lOffset, m_dwSize );

	// Read from here
	return Read( pBuf, dwMax, pdwRead, &dwReadPtr, dwEncode );
}

void CCircBuf::Defrag()
{_STTEX();
	// Lock the buffer
	CTlLocalLock ll( &m_lock );
	if ( !ll.IsLocked() ) return;

	// Set the pointers back to zero if the buffer is empty
	if ( m_dwReadPtr == NormalizePtr( m_dwWritePtr, m_dwSize ) ) Empty();
}

DWORD CCircBuf::AdvancePtr(DWORD dwPtr, long lStep, DWORD dwMax)
{_STTEX();
	// Ensure valid max
	if ( !dwMax ) return dwPtr;

	// Offset pointer
	dwPtr += lStep;

	// Wrap the pointer
	if ( dwPtr > dwMax ) dwPtr %= dwMax;

	return dwPtr;
}

DWORD CCircBuf::NormalizePtr(DWORD dwPtr, DWORD dwMax)
{_STTEX();
	if ( dwMax && dwPtr >= dwMax ) return dwPtr % dwMax;
	return dwPtr;
}

BOOL CCircBuf::GetView( DWORD dwView, DWORD dwPtr, DWORD dwSize, LPBYTE pRing, DWORD dwMax, LPBYTE *pBuf, LPDWORD pdwSize )
{_STTEX();
	// Verify buffers
	if ( pRing == NULL || !dwSize || !dwMax ) return FALSE;
	if ( pBuf == NULL || pdwSize == NULL ) return FALSE;
	if ( dwPtr >= dwMax ) return FALSE;

	// How many bytes left till the end of the buffer?
	DWORD dwLeft = dwMax - dwPtr;

	if ( dwView == 0 )
	{
		// Does it wrap?
		if ( dwSize > dwLeft ) dwSize = dwLeft;

		// Save buffer metrics on the first part of the buffer
		*pBuf = &pRing[ dwPtr ];
		*pdwSize = dwSize;

		return TRUE;

	} // end if

	else if ( dwView == 1 )
	{
		// Is there a second view?
		if ( dwSize <= dwLeft ) return FALSE;

		// The second part of the buffer
		*pBuf = pRing;
		*pdwSize = dwSize - dwLeft;

		return TRUE;

	} // end else if

	return FALSE;
}


BOOL CCircBuf::WaitData(DWORD dwTimeout)
{_STTEX();
	// Wait for data
	if ( WaitForSingleObject( m_hDataReady, dwTimeout ) != WAIT_OBJECT_0 )
		return FALSE;

	return TRUE;
}

