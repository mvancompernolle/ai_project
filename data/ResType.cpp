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
// ResType.cpp: implementation of the CResType class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifndef IS_INTRESOURCE
#define IS_INTRESOURCE( w ) ( ( ( (DWORD)w ) & 0xffff0000 ) == 0 )
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResType::CResType()
{_STT();
	m_hModule = NULL;
	*m_szType = 0;
	m_dwType = 0;
	m_dwMaxEnum = MAXDWORD;
}

CResType::~CResType()
{_STT();
	Destroy();
}

void CResType::Destroy()
{_STT();
	FreeMod();
	*m_szType = 0;
	m_dwType = 0;
	m_dwMaxEnum = MAXDWORD;
}

void CResType::FreeMod()
{_STT();
	// Unload module if needed
	if ( m_bUnload && m_hModule != NULL )
		FreeLibrary( m_hModule ); 
	m_bUnload = FALSE;
	m_hModule = NULL;
}

BOOL CALLBACK CResType::EnumResNameProc(	HMODULE hModule, LPCTSTR lpszType, 
											LPTSTR lpszName, LPARAM lParam )
{_STT();
	CResType* pResType = (CResType*)lParam;
	if ( pResType == NULL ) return FALSE;

	return pResType->OnEnum( hModule, lpszType, lpszName );
}

BOOL CResType::Enum(LPCTSTR pFile, LPCTSTR pType, DWORD max)
{_STT();
	// Lose previous module
	Destroy();

	// Attempt to load the file
	m_hModule = LoadLibraryEx( pFile, NULL, LOAD_LIBRARY_AS_DATAFILE );
	if ( m_hModule == NULL ) return FALSE;

	// Need to unload this one
	m_bUnload = TRUE;

	// Save resource type
	m_dwType = (DWORD)pType;
	if ( !IS_INTRESOURCE( pType ) ) strcpy_sz( m_szType, pType );

	// How many do they want?
	m_dwMaxEnum = max;

	// Enum the resources
	return EnumResourceNames( m_hModule, pType, CResType::EnumResNameProc, (LONG)this );
}

BOOL CResType::Enum(HMODULE hMod, LPCTSTR pType, DWORD max)
{_STT();
	// Lose previous module
	Destroy();

	// Save module handle
	m_hModule = ( hMod != NULL ) ? hMod : GetModuleHandle( NULL );

	// Save resource type
	m_dwType = (DWORD)pType;
	if ( !IS_INTRESOURCE( pType ) ) strcpy_sz( m_szType, pType );

	// How many do they want?
	m_dwMaxEnum = max;

	// Enum the resources
	return EnumResourceNames( hMod, pType, CResType::EnumResNameProc, (LONG)this );
}

BOOL CResType::OnEnum(HMODULE hModule, LPCTSTR pType, LPCTSTR pName)
{_STT();
	// Do we want to enum any more?
	if ( Size() >= m_dwMaxEnum ) return FALSE;

	// Add it
	return Add( pName );	
}

BOOL CResType::Add(LPCTSTR pName)
{_STT();
	DWORD dwName = (DWORD)pName;
	LPCTSTR szName = pName;

	// What kind of title is it?
	if ( IS_INTRESOURCE( dwName ) ) szName = NULL;

	// Allocate node
	LPRESTYPEINFO prti = (LPRESTYPEINFO)New( NULL, dwName, szName );
	if ( prti == NULL ) return FALSE;

	return TRUE;
}

BOOL CResType::LoadResource(LPCTSTR pResource, LPBYTE pPtr, LPDWORD pdwSize, LPCTSTR pType, HMODULE hModule)
{_STT();
	if ( pResource == NULL ) return FALSE;

	// Find the resource
	HRSRC hRsrc = FindResource( hModule, pResource, pType );
	if ( hRsrc == NULL ) return FALSE;

	// Save the size
	DWORD dwSize = SizeofResource( hModule, hRsrc );
	if ( pdwSize != NULL ) *pdwSize = dwSize;
		
	// Is this all they wanted to know
	if ( pPtr == NULL ) return TRUE;

	// Don't copy if size is zero
	if ( dwSize == 0 ) return TRUE;

	// Load the resource
	HGLOBAL hGlobal = ::LoadResource( hModule, hRsrc );
	if ( hGlobal == NULL ) return FALSE;

	// Lock the resource
	LPBYTE pData = (LPBYTE)LockResource( hGlobal );
	if ( pData == NULL )
	{
		FreeResource( hGlobal );
		return FALSE;
	} // end if

	// Copy the data
	memcpy( pPtr, pData, dwSize );

	// Done with this resource
	FreeResource( hGlobal );  

	return TRUE;
}

LPCTSTR CResType::GetType( DWORD dwType, LPCTSTR pType )
{_STT();
	// What type of identifier is it?
	if ( IS_INTRESOURCE( dwType ) )
		return (LPCTSTR)dwType;

	return pType;
}

HICON CResType::LoadIcon(LPRESTYPEINFO prti, DWORD i, LPDWORD count)
{_STT();
	// Verify pointer
	if ( prti == NULL ) return NULL;
/*
	// Check for group icon	
	if ( GetType() == RT_GROUP_ICON )
	{
		// Find the resource
		DWORD size = 0;
		if (	!LoadResource( GetType( prti->user, prti->cpkey ), NULL, &size, GetType(), m_hModule ) 
				|| size == 0 ) return NULL;

		TMem< BYTE > mem;
		if ( !mem.allocate( size ) ) return FALSE;

		// Get a pointer
		LPGRPICONDIR pgid = (LPGRPICONDIR)mem.ptr();
		if ( pgid == NULL ) return FALSE;

		// Let user know the score
		if ( count != NULL ) *count = pgid->idCount;

		// Is the index reasonable?
		if ( i > pgid->idCount ) return FALSE;

		// Return the icon
		return ::LoadIcon( m_hModule, MAKEINTRESOURCE( pgid->idEntries[ i ].nID ) );

	} // end if
*/
	// Attempt to load the icon
	return ::LoadIcon( m_hModule, GetType( prti->user, prti->cpkey ) );
}

LPRESTYPEINFO CResType::FindRes(LPCTSTR pRes)
{_STT();
	// Check for int
	if ( IS_INTRESOURCE( pRes ) ) return (LPRESTYPEINFO)CLList::Find( (DWORD)pRes );

	// Find string
	return (LPRESTYPEINFO)Find( pRes );
}


