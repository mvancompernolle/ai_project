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
// FifoSync.cpp: implementation of the CFifoSync class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFifoSync::CFifoSync()
{_STT();
	m_dwMaxBuffers = 1024;

	m_dwHeadPtr = 0;
	m_dwTailPtr = 0;
}

CFifoSync::~CFifoSync()
{_STT();

}

BOOL CFifoSync::Write( void const * pBuf, DWORD dwSize, DWORD dwEncode )
{_STT();
	// Lock the buffer
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return FALSE;

	// Prepare to write data
	if ( !InitFifoWrite() ) 
		return FALSE;

	// Add data to the buffer
	if ( !AddFifo( pBuf, dwSize, dwEncode ) )
		return FALSE;

	// Commit buffer changes
	return EndFifoWrite();
}

BOOL CFifoSync::AllocateBuffers()
{_STT();
	// Ensure size tag space
	if ( m_rdwSize.size() != m_dwMaxBuffers )
		if ( !m_rdwSize.grow( m_dwMaxBuffers ) )
			return FALSE;

	return TRUE;
}

BOOL CFifoSync::AddFifo(void const * pBuf, DWORD dwSize, DWORD dwEncode)
{_STT();
	// Poke the data into the buffer
	if ( !Poke( pBuf, dwSize, dwEncode ) )
		return FALSE;

	// Keep track of the size of this sub buffer
	m_rdwSize[ m_dwHeadPtr ] += dwSize;

	return TRUE;
}

BOOL CFifoSync::InitFifoWrite()
{_STT();
	// Ensure buffer space
	if ( !AllocateBuffers() ) return FALSE;

	// Do we have a buffer
	if ( !GetMaxWrite( m_dwTailPtr, m_dwHeadPtr, m_dwMaxBuffers ) )
		return FALSE;

	// Normalize the head pointer
	m_dwHeadPtr = NormalizePtr( m_dwHeadPtr, m_dwMaxBuffers );

	// Initialize buffer pointers
	m_rdwSize[ m_dwHeadPtr ] = 0;

	// Get ready to poke the buffer
	InitPoke();

	return TRUE;
}

BOOL CFifoSync::EndFifoWrite()
{_STT();
	// Advance the head pointer
	m_dwHeadPtr = AdvancePtr( m_dwHeadPtr, 1, m_dwMaxBuffers );

	return EndPoke();
}

BOOL CFifoSync::Read( LPVOID pBuf, DWORD dwSize, LPDWORD pdwRead, DWORD dwEncode )
{_STT();
	// Read the data block
	BOOL bRet = Peek( pBuf, dwSize, pdwRead, 0, dwEncode );

	// Next read block
	if ( pBuf && dwSize ) SkipBlock();

	return bRet;
}

BOOL CFifoSync::SkipBlock()
{_STT();
	// Lock the buffer
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return FALSE;

	// Anything to read?
	if ( !GetMaxRead( m_dwTailPtr, m_dwHeadPtr, m_dwMaxBuffers ) )
		return FALSE;

	// Skip to the next block
	AdvanceReadPtr( m_rdwSize[ m_dwTailPtr ] );

	// Advance the tail pointer
	m_dwTailPtr = NormalizePtr( AdvancePtr( m_dwTailPtr, 1, m_dwMaxBuffers ), m_dwMaxBuffers );

	// Wrap pointers if buffer is empty
	if ( m_dwTailPtr == NormalizePtr( m_dwHeadPtr, m_dwMaxBuffers ) && m_dwTailPtr == 0 )
		m_dwTailPtr = m_dwHeadPtr = 0;

	return TRUE;
}

BOOL CFifoSync::Peek( LPVOID pBuf, DWORD dwSize, LPDWORD pdwRead, long lOffset, DWORD dwEncode )
{_STT();
	// Lock the buffer
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return FALSE;

	// Anything to read?
	if ( !GetMaxRead( m_dwTailPtr, m_dwHeadPtr, m_dwMaxBuffers ) )
		return FALSE;

	// Are they asking for the size?
	if ( pBuf == NULL || !dwSize )
	{	if ( !pdwRead ) return FALSE;

		// Get the buffer size
		*pdwRead = m_rdwSize[ m_dwTailPtr ];

		// Correct for offset
		if ( lOffset > (long)*pdwRead ) return FALSE;
		*pdwRead -= lOffset;

		return TRUE;

	} // end if

	// What's the maximum amount of data in this block
	if ( dwSize > m_rdwSize[ m_dwTailPtr ] ) 
		dwSize = m_rdwSize[ m_dwTailPtr ];

	// Anything left to read?
	if ( dwSize <= (DWORD)lOffset ) return FALSE;

	// Subtract the offset
	dwSize -= (DWORD)lOffset;

	// Peek the data
	return CCircBuf::Peek( pBuf, dwSize, pdwRead, lOffset, dwEncode );
}
