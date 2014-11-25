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
// FindFiles.cpp: implementation of the CFindFiles class.
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

CFindFiles::CFindFiles()
{_STT();
	m_hFind = NULL;
	ZeroMemory( &m_wfd, sizeof( m_wfd ) );
}

CFindFiles::CFindFiles( LPCTSTR pDir, LPCTSTR pMask )
{_STT();
	m_hFind = NULL;
	ZeroMemory( &m_wfd, sizeof( m_wfd ) );
	FindFirst( pDir, pMask );
}

CFindFiles::~CFindFiles()
{_STT();
	Destroy();
}

void CFindFiles::Destroy()
{_STT();

	// Close the find
	if ( m_hFind != NULL )
	{	FindClose( m_hFind );
		m_hFind = NULL;
	} // end if

	// Lose the file info
	*m_szPath = 0;
	*m_szFullPath = 0;
	ZeroMemory( &m_wfd, sizeof( m_wfd ) );
}

BOOL CFindFiles::FindFirst(LPCTSTR pDir, LPCTSTR pMask)
{_STT();
	char search[ CWF_STRSIZE ];

	// Lose previous find
	Destroy();

	// Save path
	strcpy_sz( m_szPath, pDir );

	// Build search string
	if ( pDir != NULL ) strcpy_sz( search, m_szPath );
	CWinFile::BuildPath( search, search, pMask );
	
	// Get first file
	m_hFind = FindFirstFile( search, &m_wfd );
	if ( m_hFind == INVALID_HANDLE_VALUE ) return FALSE;

	// Can we use this one?
	if ( BuildPaths() ) return TRUE;

	// Try the next one
	return FindNext();
}

BOOL CFindFiles::FindNext()
{_STT();

	// Punt if no find handle
	if ( !m_hFind ) return FALSE;

	do
	{
		// Get next file
		if ( !FindNextFile( m_hFind, &m_wfd ) ) 
		{	Destroy(); return FALSE; }
		
	// Build the filename
	} while ( !BuildPaths() );

	return TRUE;
}

BOOL CFindFiles::BuildPaths()
{_STT();
	// Ensure valid filename
	if ( *m_wfd.cFileName == NULL ||
		 !strcmp( m_wfd.cFileName, "." ) ||
		 !strcmp( m_wfd.cFileName, ".." ) ) return FALSE;

	// Build Full path
	CWinFile::BuildPath( m_szFullPath, m_szPath, m_wfd.cFileName );

	return TRUE;
}


