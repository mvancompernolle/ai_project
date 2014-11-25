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
// Module.cpp: implementation of the CModule class.
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

CModule::CModule()
{_STT();
	m_hModule = NULL;
}

CModule::CModule( LPCTSTR pFile )
{_STT();
	CModule();
	LoadLibrary( pFile );
}

CModule::~CModule()
{_STT();
	Destroy();
}

BOOL CModule::LoadLibrary(LPCTSTR pFile)
{_STT();

	// Do we already have this module loaded?
	if ( NULL != m_hModule && pFile && strcmpi( pFile, m_sFile ) )
		return TRUE;

	// Lose module
	Destroy();

	// Punt if bad filename
	if ( !pFile || !*pFile ) return FALSE;

	// Save file name
	m_sFile = pFile;

	// Load the module
	m_hModule = ::LoadLibrary( pFile );
	if ( m_hModule == NULL ) return FALSE;

	return TRUE;
}

void CModule::Destroy()
{_STT();
	// Free library
	if ( m_hModule != NULL )
	{	FreeLibrary( m_hModule );
		m_hModule = NULL;
	} // end if
	
	CHList::Destroy();
}

void* CModule::AddFunction(LPCTSTR pFunctionName)
{_STT();
	// Sanity check
	if ( pFunctionName == NULL ) return NULL;

	void *pf = Addr( pFunctionName );
	if ( pf != NULL ) return pf;

	pf = GetProcAddress( m_hModule, pFunctionName );
	if ( pf == NULL ) return FALSE;

	// Save index
	DWORD index = Size();

	// Allocate memory
	LPFUNCTIONINFO pfi = (LPFUNCTIONINFO)New( NULL, 0, pFunctionName );
	if ( pfi == NULL ) return FALSE;
	
	// Save function name and address
	pfi->addr = pf;

	// Need more space?
	if ( m_ptrs.size() <= index )
	{	if ( index < 8 ) m_ptrs.grow( 8 ); else m_ptrs.grow( index * 2 ); }
	m_ptrs[ index ] = pf;

	return pf;	
}

LPVOID CModule::Addr(LPCTSTR pFunctionName)
{_STT();
	LPFUNCTIONINFO pfi = (LPFUNCTIONINFO)Find( pFunctionName );
	if ( pfi == NULL ) return NULL;
	return pfi->addr;
}

LPVOID CModule::Addr(DWORD i)
{_STT();
	if ( i >= m_ptrs.size() ) return NULL;
	return m_ptrs[ i ];
}
