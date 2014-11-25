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
// AviEncode.cpp: implementation of the CAviEncode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAviEncode::CAviEncode() :
	m_hSave( CreateEvent( NULL, TRUE, FALSE, NULL ) ),
	m_hReady( CreateEvent( NULL, TRUE, FALSE, NULL ) ),
	m_hStartRec( CreateEvent( NULL, TRUE, FALSE, NULL ) ),
	m_hStopRec( CreateEvent( NULL, TRUE, FALSE, NULL ) )
{_STT();
	m_pDc = NULL;
	m_phData = NULL;
	m_dwBuffers = 0;

	m_dwHeadFrame = 0;
	m_dwTailFrame = 0;

	m_dwCreateBuffers = 32;

	*m_szFile = 0;
	*m_szRename = 0;
	ZeroMemory( &m_rAvi, sizeof( m_rAvi ) );
	m_dwFrames = 15;
	m_dwSeconds = 1;
	m_dwFourCC = mmioFOURCC('M','P','G','4');

}

CAviEncode::~CAviEncode()
{_STT();
	Destroy();
	CloseHandle( m_hSave );
	CloseHandle( m_hReady );
	CloseHandle( m_hStartRec );
	CloseHandle( m_hStopRec );
}

BOOL CAviEncode::InitThread(LPVOID pData)
{_STT();
	AVIFileInit();

	// Allocate AVI buffers
	if ( !AllocateBuffers() )
		return FALSE;

	SetEvent( m_hReady );

	return TRUE;
}

BOOL CAviEncode::DoThread(LPVOID pData)
{_STT();
	// This should not happen
	if ( !m_dwBuffers ) return FALSE;

	// One extra buffer allocated for stop event
	HANDLE	phEvents[ 4 ];
	phEvents[ 0 ] = m_hSave;
	phEvents[ 1 ] = m_hStartRec;
	phEvents[ 2 ] = m_hStopRec;
	phEvents[ 3 ] = GetStopEvent();

	// Wait for a frame to come in
	DWORD ret = WaitForMultipleObjects( 4, phEvents, FALSE, INFINITE );

	// Stop event
	if ( ret == WAIT_OBJECT_0 + 2 )
	{	m_avi.Destroy();
		if ( *m_szRename != NULL )
			MoveFile( m_szFile, m_szRename ), *m_szRename = 0;
		ResetEvent( m_hStopRec );
	} // end if

	// Start event
	if ( ret == WAIT_OBJECT_0 + 1 )
	{	m_avi.OpenNew( m_szFile, &m_rAvi, m_dwFrames, m_dwSeconds, m_dwFourCC, NULL, 0, FALSE );
		ResetEvent( m_hStartRec );
	} // end if

	// Save frames event
	if ( ret == WAIT_OBJECT_0 )
	{	ResetEvent( m_hSave );

		RULIB_TRY 
		{
			// If we have an avi open
			if ( m_avi.IsOpen() ) 
			{
				// Add buffered frames to avi
				while ( m_dwTailFrame != m_dwHeadFrame )
				{	m_avi.AddFrame( m_pDc[ m_dwTailFrame ] );
					ResetEvent( m_phData[ m_dwTailFrame ] );
					if ( ++m_dwTailFrame >= m_dwBuffers ) m_dwTailFrame = 0;
					Sleep( 0 ); // No cpu lockdown
				} // end while
			} // end if

		} RULIB_CATCH_ALL { ASSERT( 0 ); m_dwTailFrame = m_dwHeadFrame; }

	} // end if

	return TRUE;
}

BOOL CAviEncode::EndThread(LPVOID pData)
{_STT();
	ResetEvent( m_hReady );

	// Release avi buffers
	ReleaseBuffers();

	AVIFileExit();

	return TRUE;
}

BOOL CAviEncode::Init(DWORD dwBuffers)
{_STT();
	Destroy();

	// Copy information for thread
	m_dwCreateBuffers = dwBuffers;

	StartThread();

	return TRUE;
}

void CAviEncode::Destroy()
{_STT();
	StopThread();
}

BOOL CAviEncode::AllocateBuffers()
{_STT();
	ReleaseBuffers();

	// Anything to do
	if ( !m_dwCreateBuffers ) return FALSE;

	// We shall attempt to create this number
	m_dwBuffers = m_dwCreateBuffers;

	RULIB_TRY
	{
		// Get thread events
		m_phData = new HANDLE[ m_dwBuffers + 3 ];
		if ( m_phData == NULL ) return FALSE;

		// Allocate buffer space
		m_pDc = new CWinDc*[ m_dwBuffers ];
		if ( m_pDc == NULL ) return FALSE;

		// Allocate as many buffers as we can
		for ( DWORD i = 0; i < m_dwBuffers; i++ )
		{	m_pDc[ i ] = new CWinDc();
			if ( m_pDc[ i ] == NULL ) m_dwBuffers = i;
			else m_phData[ i ] = CreateEvent( NULL, TRUE, FALSE, NULL );
		} // end for

	} RULIB_CATCH_ALL { ASSERT( 0 ); return 0; }

	return m_dwBuffers != 0;
}

void CAviEncode::ReleaseBuffers()
{_STT();
	// Delete buffers if any
	if ( m_pDc != NULL && m_phData != NULL )
	{
		RULIB_TRY
		{
			// Release buffers
			for ( DWORD i = 0; i < m_dwBuffers; i++ )
			{	if ( m_pDc[ i ] != NULL ) delete m_pDc[ i ];
				if ( m_phData[ i ] != NULL ) CloseHandle( m_phData[ i ] );
			} // end for
			delete [] m_phData;
			delete [] m_pDc;

		} RULIB_CATCH_ALL { ASSERT( 0 ); }

	} // end if

	m_pDc = NULL;
	m_phData = NULL;
	m_dwBuffers = 0;
	m_dwHeadFrame = 0;
	m_dwTailFrame = 0;

}

BOOL CAviEncode::AddFrame(CWinDc *pDc)
{_STT();
	// Sanity check
	if ( pDc == NULL || pDc->GetSafeHdc() == NULL ) 
		return FALSE;

	// Ensure we are still ready and running
	if ( WaitForSingleObject( m_hReady, 0 ) != WAIT_OBJECT_0 || !IsRunning() ) 
		return FALSE;

	// Ensure no buffer overflow
	if (	m_dwTailFrame != m_dwHeadFrame &&
			WaitForSingleObject( m_phData[ m_dwHeadFrame ], 0 ) == WAIT_OBJECT_0 )
	{	SetEvent( m_hSave ); return FALSE; }

	// Create frame buffer if needed
	if (	m_pDc[ m_dwHeadFrame ]->GetSafeHdc() == NULL ||
			m_pDc[ m_dwHeadFrame ]->GetWidth() != pDc->GetWidth() ||
			m_pDc[ m_dwHeadFrame ]->GetHeight() != pDc->GetHeight() )
		m_pDc[ m_dwHeadFrame ]->CreateDIBSection( *pDc, *pDc );

	// Draw the frame
	pDc->Draw( *m_pDc[ m_dwHeadFrame ], *m_pDc[ m_dwHeadFrame ] );

	// Signal for frame to be written to buffer
	SetEvent( m_phData[ m_dwHeadFrame ] );

	// Advance buffer pointers
	if ( ++m_dwHeadFrame >= m_dwBuffers ) m_dwHeadFrame = 0;
	if ( m_dwHeadFrame == m_dwTailFrame )
	{	ResetEvent( m_phData[ m_dwTailFrame ] );
		if ( ++m_dwTailFrame >= m_dwBuffers ) m_dwTailFrame = 0;
	} // end if

	// Trigger save frame
	SetEvent( m_hSave );

	return TRUE;

}

BOOL CAviEncode::StopRecording( LPCTSTR pRename )
{_STT();
	if ( pRename != NULL ) { strcpy_sz( m_szRename, pRename ); }
	SetEvent( m_hStopRec );
	return TRUE;
}

BOOL CAviEncode::StartRecording(LPCTSTR pFile, LPRECT pRect, DWORD dwFrames, DWORD dwSeconds, DWORD dwFourCC)
{_STT();
	// Are we already starting something?
	if ( WaitForSingleObject( m_hStartRec, 0 ) == WAIT_OBJECT_0 )
		return FALSE;

	// Copy data
	strcpy( m_szFile, pFile );
	CopyRect( &m_rAvi, pRect );
	OffsetRect( &m_rAvi, -m_rAvi.left, -m_rAvi.top );
	m_dwFrames = dwFrames;
	m_dwSeconds = dwSeconds;
	m_dwFourCC = dwFourCC;

	SetEvent( m_hStartRec );

	return TRUE;
}
