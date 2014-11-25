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
// HookStdio.cpp: implementation of the CHookStdio class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "io.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHookStdio::CHookStdio( DWORD dwStreamId, DWORD dwBufferSize )
{
	m_hRead = NULL;
	m_hWrite = NULL;
	m_hOriginal = NULL;

	// Hook stream if needed
	if ( dwStreamId ) Start( dwStreamId, dwBufferSize );
}

CHookStdio::~CHookStdio()
{	Stop();
}

BOOL CHookStdio::Start( DWORD dwStreamId, DWORD dwBufferSize )
{
	// Kill any current hook
	Stop();

	// Verify valid stream
	if (	STD_INPUT_HANDLE != dwStreamId &&
			STD_OUTPUT_HANDLE != dwStreamId &&
			STD_ERROR_HANDLE != dwStreamId )
		return FALSE;
	
	// Save stream id
	m_dwStreamId = dwStreamId;

	// Create pipe
	if ( !CreatePipe( &m_hRead, &m_hWrite, NULL, dwBufferSize ) )
	{	Stop(); return FALSE; }

    // Save current stream handle
    m_hOriginal = GetStdHandle( dwStreamId );

	// Set new stream handle
	if ( !SetStdHandle( dwStreamId, m_hWrite ) )
	{	Stop(); return FALSE; }

	// Associate stream handle
	int nStream = _open_osfhandle( (long)GetStdHandle( dwStreamId ), 0 );

	// Associate stream handle
	FILE* hfStream = _fdopen( nStream, "w" );

	// Hook appropriate stream
	switch ( dwStreamId )
	{
		case STD_INPUT_HANDLE :

			// Switch streams
			*stdin = *hfStream;

			// Disable stream buffering
			setvbuf( stdin, NULL, _IONBF, 0 );

			break;

		case STD_OUTPUT_HANDLE :

			// Switch streams
			*stdout = *hfStream;

			// Disable stream buffering
			setvbuf( stdout, NULL, _IONBF, 0 );

			break;

		case STD_ERROR_HANDLE :

			// Switch streams
			*stderr = *hfStream;

			// Disable stream buffering
			setvbuf( stderr, NULL, _IONBF, 0 );

			break;

	} // end switch
	
	return TRUE;
}

void CHookStdio::Stop()
{
	if ( m_hOriginal )
	{	SetStdHandle( m_dwStreamId, m_hOriginal );
		m_hOriginal = NULL;
	} // end if

	if ( m_hWrite )
	{	CloseHandle( m_hWrite );
		m_hWrite = NULL;
	} // end if

	if ( m_hRead )
	{	CloseHandle( m_hRead );
		m_hRead = NULL;
	} // end if
}

DWORD CHookStdio::Read(LPVOID pBuf, DWORD dwSize)
{
	// Punt if no read handle
	if ( NULL == m_hRead || !pBuf || !dwSize ) return 0;

	DWORD dwRead = 0;

	// Ensure there is data to read
	if ( !PeekNamedPipe( m_hRead, NULL, NULL, NULL, &dwRead, NULL ) || !dwRead )
		return 0;

	// Attempt to read from the stream
	if ( !ReadFile( m_hRead, pBuf, dwSize, &dwRead, NULL ) ) 
		return 0;

	// Check for error
	if ( !dwRead || dwRead > dwSize )
		return 0;

	// Return the number of bytes read
	return dwRead;
}

DWORD CHookStdio::Write(LPVOID pBuf, DWORD dwSize)
{
	// Punt if no write handle
	if ( NULL == m_hWrite || !pBuf || !dwSize ) return 0;

	DWORD dwWritten = 0;

	// Attempt to write to the stream
	if ( !WriteFile( m_hWrite, pBuf, dwSize, &dwWritten, NULL ) ) 
		return 0;

	// Check for error
	if ( !dwWritten || dwWritten > dwSize )
		return 0;

	// Return the number of bytes written
	return dwWritten;
}
