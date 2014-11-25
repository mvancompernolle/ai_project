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
// WinDcQueue.cpp: implementation of the CWinDcQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWinDcQueue::CWinDcQueue()
{_STT();
	m_head = NULL;
	m_tail = NULL;

	m_dwFrames = 0;

	m_llAvgFrameDelay = 0;

	m_hFrameReady = CreateEvent( NULL, TRUE, FALSE, NULL );
}

CWinDcQueue::~CWinDcQueue()
{_STT();
	Destroy();

	// Lose the sync
	if ( m_hFrameReady )
	{	CloseHandle( m_hFrameReady );
		m_hFrameReady = NULL;
	} // end if
}

BOOL CWinDcQueue::Create(DWORD dwSize, HDC hDC, long lWidth, long lHeight, BOOL bDibSection )
{_STT();
	CTlLocalLock ll( &m_lock );
	if ( !ll.IsLocked() ) return FALSE;

	Destroy();

	m_lWidth = lWidth;
	m_lHeight = lHeight;

	BOOL bReleaseDc = FALSE;
	if ( hDC == NULL )
	{	bReleaseDc = TRUE;
		hDC = ::GetDC( NULL );
	} // end if

	// Create objects
	CFrame *pFrame = NULL;
	for ( DWORD i = 0; i < dwSize; i++ )
	{
		// Allocate a frame
		pFrame = m_frames.push_back();
		if ( pFrame == NULL ) return FALSE;

		// Create the DC
		CWinDc *pDC = *pFrame;
		if ( bDibSection ) pDC->CreateDIBSection( hDC, lWidth, lHeight );
		else pDC->Create( hDC, 0, 0, lWidth, lHeight );

	} // end for

	// Release desktop dc if needed
	if ( bReleaseDc ) ::ReleaseDC( NULL, hDC );

	// Point the pointers
	m_head = m_tail = m_frames.begin();

	return TRUE;
}

void CWinDcQueue::Destroy()
{_STT();
	CTlLocalLock ll( &m_lock );
	if ( !ll.IsLocked() ) return;

	m_head = NULL;
	m_tail = NULL;

	m_dwFrames = 0;

	m_frames.Destroy();
}

BOOL CWinDcQueue::WaitFrame(DWORD dwTimeout)
{_STT();
	if ( WaitForSingleObject( m_hFrameReady, dwTimeout ) != WAIT_OBJECT_0 )
		return FALSE;

	return TRUE;
}

CWinDcQueue::CFrame* CWinDcQueue::GetOpenFrame()
{_STT();
	CTlLocalLock ll( &m_lock );
	if ( !ll.IsLocked() ) return FALSE;

	TLList< CFrame >::iterator next = m_head;

	// Advance to valid frame
	if ( next == NULL || next == m_frames.end() ) 
		m_head = next = m_frames.begin();	  

	// Skip to the next frame
	if ( ( next = m_frames.next( next ) ) == NULL )
		next = m_frames.begin();

	// Any left?
	if ( next == m_tail ) return NULL;

	return *m_head;
}

CWinDcQueue::CFrame* CWinDcQueue::AdvanceHeadPtr()
{_STT();
	CTlLocalLock ll( &m_lock );
	if ( !ll.IsLocked() ) return FALSE;

	// Point to next frame
	m_head = m_frames.next( m_head );

	// Loop
	if ( m_head == m_frames.end() )
		m_head = m_frames.begin();

	// Copy over old frames if buffer is full
	if ( m_head == m_tail )
	{	m_tail = m_frames.next( m_tail );
		if ( m_tail == m_frames.end() )
			m_tail = m_frames.begin();
	} // end if

	// Count one frame
	if ( ++m_dwFrames > m_frames.Size() )
		m_dwFrames = m_frames.Size();

	// Signal that a frame is ready
	SetEvent( m_hFrameReady );

	return *m_head;
}


CWinDcQueue::CFrame* CWinDcQueue::AdvanceTailPtr()
{_STT();
	CTlLocalLock ll( &m_lock );
	if ( !ll.IsLocked() ) return FALSE;

	// Punt if no frames
	if ( m_tail == NULL ) m_tail = m_frames.begin();
	if ( m_tail == NULL || m_tail == m_head )
	{	m_dwFrames = 0; ResetEvent( m_hFrameReady ); return NULL; }

	// Save this frame
	TLList< CFrame >::iterator cur = m_tail;
	
	// Inc and loop
	m_tail = m_frames.next( m_tail );
	if ( m_tail == m_frames.end() ) m_tail = m_frames.begin();

	// Hose one frame
	if ( m_dwFrames ) m_dwFrames--;
	if ( m_dwFrames == 0 ) ResetEvent( m_hFrameReady );

	return *cur;
}

CWinDcQueue::CFrame* CWinDcQueue::GetNextQueuedFrame()
{_STT();
	CTlLocalLock ll( &m_lock );
	if ( !ll.IsLocked() ) return FALSE;

	// Punt if no frames
	if ( m_tail == NULL ) m_tail = m_frames.begin();
	if ( m_tail == NULL || m_tail == m_head )
	{	m_dwFrames = 0; ResetEvent( m_hFrameReady ); return NULL; }

	return *m_tail;
}
