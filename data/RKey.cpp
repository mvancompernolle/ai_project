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
// RegKey.cpp: implementation of the CRKey class.
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

CRKey::CRKey()
{_STTEX();
	m_dwMinSize = 0;
}

CRKey::~CRKey()
{_STTEX();
	m_name.destroy();
	m_num.destroy();

	Destroy();
}

BOOL CRKey::SetName( LPCTSTR pName )
{_STTEX();
	// Is there a new name
	if ( pName == NULL ) return FALSE;

	// Allocate memory
	if ( !m_name.allocate( strlen( pName ) + 1 ) )
		return FALSE;

	// Copy the name
	strcpy( m_name, pName );

	return TRUE;

} // end if

void CRKey::DeleteObject( void *node )
{_STTEX();
	LPREGVALUE prv = (LPREGVALUE)node;
	if ( prv == NULL ) return;

	if ( prv->data != NULL )
	{	delete [] prv->data;
		prv->data = NULL;
	} // end if

	CHList::DeleteObject( node );
}

LPREGVALUE CRKey::Add( DWORD dwType, LPCTSTR pName, const void * pValue, DWORD dwValue, BOOL bFile)
{_STTEX();
	BOOL bNew = FALSE;
	LPREGVALUE prv = (LPREGVALUE)Find( pName );

	// Did we find an existing object?
	if ( prv != NULL ) 
		ResetObject( prv, NULL, 0, (LPVOID)pName ); 

	else // Create new object
	{	prv = (LPREGVALUE)New( NULL, 0, (LPVOID)pName );
		if ( prv == NULL ) return NULL;
	} // end else

	// Save the type
	prv->type = dwType;

	RULIB_TRY
	{
		if ( bFile )
		{
			CWinFile wf;
			if ( wf.OpenExisting( (char*)pValue ) )
			{
				DWORD dwSize = wf.Size();
				if ( dwSize < m_dwMinSize ) dwSize = m_dwMinSize;
				if ( dwSize )
				{
					// Allocate memory
					prv->data = new BYTE[ dwSize + 1 ];
					if ( prv->data == NULL ) 
					{	Delete( prv ); return FALSE; }
					prv->size = dwSize;

					// Read data from file
					if ( !wf.Size() || !wf.Read( prv->data, wf.Size() ) )
						ZeroMemory( prv->data, prv->size );

				} // end if

			} // end if

			else bFile = FALSE;

		} // end if

		// Save the data
		if ( !bFile && ( dwValue != 0 || m_dwMinSize != 0 ) )
		{
			// What size should we use
			DWORD sz = m_dwMinSize < dwValue ? dwValue : m_dwMinSize;

			// Allocate memory
			prv->data = new BYTE[ sz + 1 ];
			if ( prv->data == NULL ) 
			{	Delete( prv ); return FALSE; }
			prv->size = sz;

			// Initialize data
			if ( pValue != NULL && dwValue != 0 ) 
			{	memcpy( prv->data, pValue, dwValue );
				( (LPBYTE)prv->data )[ dwValue ] = 0;
			} // end if
			else ZeroMemory( prv->data, sz + 1 );

		} // end if	

	} // end try

	// Just forget it if error
	RULIB_CATCH_ALL { ASSERT( 0 ); return NULL; }

	return prv;
}


BOOL CRKey::GetKey(HKEY hKey, LPCTSTR pKey, BOOL bMerge )
{_STTEX();
	if ( !bMerge ) Destroy();

	HKEY	hOKey;
	DWORD	i = 0;
	char	name[ 1024 ];
	DWORD	nsize = sizeof( name );
	DWORD	type;
	DWORD	size;
	
	// Open the key
	if ( RegOpenKeyEx(	hKey, pKey,0, KEY_ALL_ACCESS, &hOKey ) != ERROR_SUCCESS )
		return FALSE;

	// Enumerate each value
	while ( RegEnumValue( hOKey, i++, name, &nsize, NULL, &type, NULL, &size ) == ERROR_SUCCESS )
	{
		// Reset the name buffer size
		nsize = sizeof( name );

		// Allocate reg entry
		LPREGVALUE prv = Add( type, name, NULL, size );
		if ( prv != NULL && size > 0 )
		{
			// Get the data
			if ( RegQueryValueEx( hOKey, name, NULL, NULL, 
								  (LPBYTE)prv->data, &size ) 
									== ERROR_SUCCESS )
			{
				// NULL terminate buffer 
				( (LPBYTE)prv->data )[ size ] = 0;
				prv->size = size;

			} // end if

			// Just forget it
			else Delete( prv );

		} // end if

	} // end while

	// Close the key
	RegCloseKey( hOKey );

	return TRUE;
}

typedef struct tagKEYTYPES
{	HKEY	key;
	LPCTSTR	name;
} KEYTYPES; // end typedef struct

static KEYTYPES g_keytypes[] =
{
	{ HKEY_CLASSES_ROOT, "HKEY_CLASSES_ROOT" },
	{ HKEY_CURRENT_USER, "HKEY_CURRENT_USER" },
	{ HKEY_LOCAL_MACHINE, "HKEY_LOCAL_MACHINE" },
	{ HKEY_USERS, "HKEY_USERS" },
	{ HKEY_PERFORMANCE_DATA, "HKEY_PERFORMANCE_DATA" },
	{ HKEY_CURRENT_CONFIG, "HKEY_CURRENT_CONFIG" },
	{ HKEY_DYN_DATA, "HKEY_DYN_DATA" },
};

BOOL CRKey::GetKeyName(HKEY hKey, LPCTSTR pKey, LPSTR pName)
{_STTEX();
	if ( pName == NULL ) return FALSE;

	// Get key name
	DWORD k, keys = sizeof( g_keytypes ) / sizeof( KEYTYPES );
	for ( k = 0; k < keys && hKey != g_keytypes[ k ].key; k++ );
	if ( k < keys ) strcpy( pName, g_keytypes[ k ].name );
	else strcpy( pName, "UNKNOWN" );

	// Construct full name
	if ( pKey != NULL ) CWinFile::BuildPath( pName, pName, pKey );

	return ( k < keys );
}

BOOL CRKey::GetKeyFromName(LPCTSTR pName, PHKEY phKey, LPSTR pKey)
{_STTEX();
	if ( pName == NULL ) return FALSE;

	// Get key name
	DWORD k, keys = sizeof( g_keytypes ) / sizeof( KEYTYPES );
	for (	k = 0; 
			k < keys && 
			strnicmp( pName, g_keytypes[ k ].name, strlen( g_keytypes[ k ].name ) ) != 0; 
			k++ );

	// Do they want the root key 
	if ( k < keys ) { if ( phKey != NULL ) *phKey = g_keytypes[ k ].key; }
	else if ( phKey != NULL ) phKey = NULL;

	// Do they want the key name
	if ( pKey != NULL )
	{	
		// Skip key type
		DWORD off = strlen( g_keytypes[ k ].name );

		// Find separator and skip
		while ( pName[ off ] != 0 && pName[ off ] == '/' || pName[ off ] == '\\' ) off++;
		while ( pName[ off ] == '/' || pName[ off ] == '\\' ) off++;

		// Copy the key name
		if ( k < keys && pName[ off ] != 0 ) strcpy( pKey, &pName[ off ] );
		else *pKey = 0;

	} // end if
	
	return ( k < keys );
}

LPREGVALUE CRKey::Get( LPCTSTR pName, LPDWORD pdw )
{_STTEX();
	if ( pName == NULL || pdw == NULL ) return NULL;
	LPREGVALUE prv = (LPREGVALUE)Find( pName );
	if ( prv == NULL ) return NULL;
	return Get( prv, pdw );
}

LPREGVALUE CRKey::Get( LPREGVALUE prv, LPDWORD pdw )
{_STTEX();
	// Sanity checks
	if ( prv == NULL || pdw == NULL || prv->data == NULL ) return NULL;

	// Get number
	if ( prv->type == REG_DWORD	)
		*pdw = *( (LPDWORD)prv->data );

	// Get string
	else if ( prv->type == REG_SZ && prv->data != NULL )
		*pdw = strtoul( (char*)prv->data, NULL, 10 );

	// Can't convert
	else { *pdw = 0; return NULL; }

	return prv;
} 

LPREGVALUE CRKey::Get( LPCTSTR pName, double *pd )
{_STTEX();
	if ( pName == NULL || pd == NULL ) return NULL;
	LPREGVALUE prv = (LPREGVALUE)Find( pName );
	if ( prv == NULL ) return NULL;
	return Get( prv, pd );
}

LPREGVALUE CRKey::Get( LPREGVALUE prv, double *pd )
{_STTEX();
	// Sanity checks
	if ( prv == NULL || pd == NULL || prv->data == NULL ) return NULL;

	// Get number
	if ( prv->type == REG_DWORD	)
		*pd = (double)*( (LPDWORD)prv->data );

	// Get string
	else if ( prv->type == REG_SZ && prv->data != NULL )
		*pd = strtod( (char*)prv->data, NULL );

	// Can't convert
	else { *pd = 0; return NULL; }

	return prv;
} 

LPREGVALUE CRKey::Get(LPCTSTR pName, LPSTR str, DWORD size)
{_STTEX();
	if ( pName == NULL || str == NULL || size == 0 ) return NULL;
	LPREGVALUE prv = (LPREGVALUE)Find( pName );
	if ( prv == NULL ) return NULL;
	return Get( prv, str, size );
}

LPREGVALUE CRKey::Get(LPREGVALUE prv, LPSTR str, DWORD size)
{_STTEX();
	if ( prv == NULL || str == NULL || size == 0 ) return NULL;

	// Get number
	if ( prv->type == REG_DWORD	)
	{	char num[ 256 ];
		wsprintf( num, "%lu", (DWORD)*( (LPDWORD)prv->data ) );
		strncpy( str, num, size ); str[ size - 1 ] = 0;
	} // end if

	// Get string
	else if ( prv->type == REG_SZ )
	{	DWORD max = ( size > prv->size ) ? prv->size : size;
		memcpy( str, prv->data, max ); 
		if ( prv->size < size ) str[ prv->size ] = 0;
		else str[ size - 1 ] = 0;
	}

	// Can't convert
	else { str[ 0 ] = 0; return NULL; }

	return prv;
}

LPREGVALUE CRKey::Get(LPCTSTR pName, void *ptr, DWORD size)
{_STTEX();
	if ( pName == NULL || ptr == NULL || size == 0 ) return NULL;
	LPREGVALUE prv = (LPREGVALUE)Find( pName );
	if ( prv == NULL ) return NULL;
	return Get( prv, ptr, size );
}

LPREGVALUE CRKey::Get(LPREGVALUE prv, void *ptr, DWORD size)
{_STTEX();
	if ( prv == NULL || ptr == NULL || size == 0 ) return NULL;

	// What's the maximum amount of data we can copy?
	DWORD max = ( size > prv->size ) ? prv->size : size;

	// Copy the data
	memcpy( ptr, prv->data, max ); 

	return prv;
}

DWORD CRKey::GetDword( LPREGVALUE prv, DWORD dwDefault)
{_STTEX();
	if ( prv == NULL ) return NULL;
	Get( prv, &dwDefault );
	return dwDefault;
}

DWORD CRKey::GetDword(LPCTSTR pName, DWORD dwDefault)
{_STTEX();
	Get( pName, &dwDefault );
	return dwDefault;
}

LPCTSTR CRKey::GetSz(LPCTSTR pName, LPCTSTR pDefault)
{_STTEX();
	// Get the reg object
	if ( pName == NULL ) pName = "";
	LPREGVALUE prv = (LPREGVALUE)Find( pName );
	return GetSz( prv, pDefault );
}

LPCTSTR CRKey::GetSz(LPREGVALUE prv, LPCTSTR pDefault)
{_STTEX();
	// Never return NULL
	if ( pDefault == NULL ) pDefault = "";

	// Return pointer if possible
	if ( prv == NULL || prv->data == NULL ) return pDefault;

	if ( prv->type == REG_DWORD )
	{	if ( !m_num.allocate( 32 ) ) return pDefault;
		wsprintf( m_num, "%lu", (DWORD)*( (LPDWORD)prv->data ) );
		return m_num.ptr();
	} // end if

	// Must be type string
	if ( prv->type != REG_SZ ) return pDefault;

	// Return a pointer to the string
	return (LPCTSTR)prv->data;
}

double CRKey::GetDouble( LPREGVALUE prv, double dDefault)
{_STTEX();
	if ( prv == NULL ) return NULL;
	Get( prv, &dDefault );
	return dDefault;
}

double CRKey::GetDouble(LPCTSTR pName, double dDefault)
{_STTEX();
	Get( pName, &dDefault );
	return dDefault;
}

BOOL CRKey::DeleteKey(HKEY hKey, LPCTSTR pKey, BOOL bSubKeys)
{_STTEX();
	// Delete sub keys if needed
	if ( bSubKeys ) DeleteSubKeys( hKey, pKey );

	// Attempt to delete the key
	return ( RegDeleteKey( hKey, pKey ) == ERROR_SUCCESS );
}

BOOL CRKey::DeleteSubKeys(HKEY hRoot, LPCTSTR pKey)
{_STTEX();
	HKEY		hKey;
	char		szKey[ CWF_STRSIZE ];
	DWORD		dwSize = CWF_STRSIZE - 1;

	// Open The Key
	if( RegOpenKeyEx( hRoot, pKey, 0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
		return FALSE;

	// For each sub key
	while ( RegEnumKeyEx(	hKey, 0, szKey, &dwSize, 
							NULL, NULL, NULL, NULL ) == ERROR_SUCCESS )
	{
		// A little recursion
		DeleteSubKeys( hKey, szKey );

		// Attempt to delete the key
		RegDeleteKey( hKey, szKey );

		// Reset size
		dwSize = CWF_STRSIZE - 1;

	} // end while

	// Close the key
	RegCloseKey( hKey );

	return TRUE;
}

BOOL CRKey::Parse(LPCTSTR buf, DWORD size, char sep)
{_STTEX();
	// Sanity checks
	if ( buf == NULL || size == 0 ) return FALSE;

	DWORD	i = 0;
	char	token[ CWF_STRSIZE ];

	while ( i < size && buf[ i ] != 0 )
	{
		DWORD t = 0;

		// Skip white space
		while ( i < size && ( buf[ i ] <= ' ' || buf[ i ] > '~' ) && buf[ i ] != 0 ) i++;
		if ( i >= size || buf[ i ] == 0 ) return TRUE;

		// Read in first token
		while ( buf[ i ] > ' ' && buf[ i ] <= '~' && buf[ i ] != '=' )
			token[ t++ ] = buf[ i++ ];
		token[ t ] = 0;

		// Check for '='
		if ( buf[ i ] == '=' )
		{	i++;

			DWORD s = i;
			BOOL quoted = FALSE;

			// Find the end of the data
			while ( i < size &&
					( ( quoted && buf[ i ] >= ' ' ) || buf[ i ] > ' ' ) && 
					buf[ i ] <= '~' && ( quoted || buf[ i ] != sep ) ) 
			{
				// Toggle quote mode
				if ( buf[ i ] == '"' ) quoted = !quoted;

				// Next char
				i++;
			} // end if

			// Get value
			if ( i > s )
			{
				TMem< char > str;
				if ( str.allocate( ( i - s ) + 1 ) )
				{
					// Copy and NULL terminate
					memcpy( str.ptr(), &buf[ s ], i - s );
					str.ptr()[ i - s ] = 0;

					// Unquote the string
					CWinFile::Unquote( str.ptr() );

					// Add variable if any
					if ( i > s ) Add( REG_SZ, token, str, strlen( str ) );
					
				} // end if

			} // end if

			// Skip separator
			if ( buf[ i ] == sep ) i++;

		} // end if

	} // end while

	return TRUE;
}


BOOL CRKey::Replace(LPSTR out, LPDWORD op, DWORD dwout, LPCTSTR in, DWORD dwin, LPCTSTR pStop, LPSTR pBreak, CRKey *params, LPDWORD pdwBreak)
{_STTEX();
	// Sanity check
	if ( out == NULL || in == NULL ) return FALSE;

	// No break yet
	if ( pBreak != NULL ) *pBreak = 0;

	DWORD dwop = 0;
	if ( op != NULL ) dwop = *op;

	DWORD s = 0, i = 0;
	const char *strrep = pStop;
	if ( strrep == NULL ) strrep = "<!--$$$";
	DWORD dwrep = strlen( strrep );

	const char *endrep = "-->";
	DWORD dwendrep = strlen( endrep );

	// Pick up where we left off
	if ( pdwBreak != NULL ) s = i = *pdwBreak;

	while ( i < dwin && in[ i ] != 0 )
	{
		// Check for replace token
		if (	( dwin - i ) > dwrep && 
				*(LPDWORD)&in[ i ] == *(LPDWORD)strrep &&
				!strnicmp( &in[ i ], strrep, dwrep ) )
		{
			// Write out good data
			if ( i != s ) 
			{
				dwop += Write( out, dwop, dwout, (LPVOID)&in[ s ], i - s );
				if ( op != NULL ) *op = dwop;
			} // end if

			DWORD x = 0;
			char token[ CWF_STRSIZE ];

			// Skip replace flag
			i += dwrep;

			// Skip white space
			while ( i < dwin && ( in[ i ] <= ' ' || in[ i ] > '~' ) ) i++;
			if ( i >= dwin ) return TRUE;

			// Copy token
			while ( in[ i ] > ' ' && in[ i ] <= '~' && 
					strnicmp( &in[ i ], endrep, dwendrep ) )
			{	if ( x < sizeof( token ) - 1 ) token[ x++ ] = in[ i ]; i++; }
			token[ x ] = 0;

			DWORD p = i;

			// Skip to end of replace tag
			while ( i < dwin && strnicmp( &in[ i ], endrep, dwendrep ) && 
					in[ i ] != 0 ) i++; 

			// Read in vars if any
			if ( i > p && params != NULL )
				params->Parse( &in[ s ], i - p );

			// Skip end tag
			if ( !strnicmp( &in[ i ], endrep, dwendrep ) ) i += dwendrep;

			// Point to start of good data
			s = i;

			LPREGVALUE prv = (LPREGVALUE)IFind( token );
			if ( prv != NULL )
			{				
				// Check for break
				if ( pBreak != NULL && prv->type == 0 )
				{	if ( pdwBreak != NULL ) *pdwBreak = i;
					strcpy( pBreak, token );
					return TRUE;
				} // end if
				
				// Write out the replace value
				else 
				{	dwop += Write( out, dwop, dwout, GetSz( prv ) );
					if ( op != NULL ) *op = dwop;
				} // end else

			} // end if

			else if ( pBreak != NULL ) // Break if needed
			{
				if ( pdwBreak != NULL ) *pdwBreak = i;
				strcpy( pBreak, token );
				return TRUE;
			} // end else

		} // end if

		// Next
		i++;

	} // end while	

	// Write out what's left of the data
	if ( i > s ) 
	{	dwop += Write( out, dwop, dwout, (LPVOID)&in[ s ], i - s );
		if ( op != NULL ) *op = dwop;
	} // end if

	// NULL terminate
	out[ dwop ] = 0;

	return FALSE;
}

DWORD CRKey::Write(LPVOID dst, DWORD ptr, DWORD max, const void *src, DWORD size)
{_STTEX();
	// Ensure buffer space left
	if ( ptr >= max ) return 0;

	// Zero size means NULL terminated string
	if ( size == 0 ) size = strlen( (LPCTSTR)src );

	// How much can we copy?
	if ( size > max - ptr ) size = max - ptr;

	// Copy the data
	memcpy( &( (LPBYTE)dst )[ ptr ], src, size );

	// NULL terminate
	DWORD null = ptr + size;
	if ( null < max ) ( (LPBYTE)dst )[ null ] = 0;
	else if ( max > 0 ) ( (LPBYTE)dst )[ max - 1 ] = 0;
	else ( (LPBYTE)dst )[ 0 ] = 0;

	return size;
}

DWORD CRKey::GetValue(HKEY hKey, LPCTSTR pKey, LPCTSTR pValue, LPVOID pData, DWORD dwSize, LPDWORD pdwType)
{_STTEX();
	HKEY hOKey;
	if ( RegOpenKeyEx(	hKey, pKey, 0, KEY_READ, &hOKey ) != ERROR_SUCCESS )
		return 0;

	// Get the value
	BOOL ret = RegQueryValueEx( hOKey, pValue, NULL, NULL, (LPBYTE)pData, &dwSize );

	// Close the key
	RegCloseKey( hOKey );

	if ( ret != ERROR_SUCCESS ) return 0;

	return dwSize;	
}

BOOL CRKey::SetValue(HKEY hKey, LPCTSTR pKey, LPCTSTR pValue, LPVOID pData, DWORD dwSize, DWORD dwType)
{_STTEX();
	HKEY hOKey = NULL;
	if ( RegCreateKey( hKey, pKey, &hOKey ) != ERROR_SUCCESS )
		return FALSE;

	// Set the value
	RegSetValueEx( hOKey, pValue, 0, dwType, (const BYTE *)pData, dwSize );

	// Close the key
	RegCloseKey( hOKey );

	return TRUE;
}

BOOL CRKey::Replace(CPipe *out, LPDWORD op, LPCTSTR in, DWORD dwin, LPCTSTR pBegin, LPCTSTR pEnd, LPSTR pBreak, CVar *params, LPDWORD pdwBreak, char sep)
{_STTEX();
	// Sanity check
	if ( out == NULL || in == NULL ) return FALSE;

	// No break yet
	if ( pBreak != NULL ) *pBreak = 0;

	DWORD s = 0, i = 0;
	const char *strrep = pBegin;
	if ( strrep == NULL ) strrep = "<!--$$$";
	DWORD dwrep = strlen( strrep );

	const char *endrep = pEnd;
	if ( endrep == NULL ) endrep = "-->";
	DWORD dwendrep = strlen( endrep );

	// Pick up where we left off
	if ( pdwBreak != NULL ) s = i = *pdwBreak;

	while ( i < dwin )
	{
		// Check for replace token
		if (	( dwin - i ) > dwrep && 
				*(LPDWORD)&in[ i ] == *(LPDWORD)strrep &&
				!strnicmp( &in[ i ], strrep, dwrep ) )
		{
			// Write out good data
			if ( i != s ) 
			{
				out->Write( (LPVOID)&in[ s ], i - s );
			} // end if

			DWORD x = 0;
			char token[ CWF_STRSIZE ];

			// Skip replace flag
			i += dwrep;

			// Skip white space
			while ( i < dwin && ( in[ i ] <= ' ' || in[ i ] > '~' ) ) i++;
			if ( i >= dwin ) return TRUE;

			// Copy token
			while ( in[ i ] > ' ' && in[ i ] <= '~' && 
					strnicmp( &in[ i ], endrep, dwendrep ) )
			{	if ( x < sizeof( token ) - 1 ) token[ x++ ] = in[ i ]; i++; }
			token[ x ] = 0;

			DWORD p = i;

			// Skip to end of replace tag
			while ( i < dwin && strnicmp( &in[ i ], endrep, dwendrep ) && 
					in[ i ] != 0 ) i++; 

			// Read in vars if any
			if ( i > p && params != NULL )
				params->ReadInline( (LPBYTE)&in[ s ], i - p, sep );

			// Skip end tag
			if ( !strnicmp( &in[ i ], endrep, dwendrep ) ) i += dwendrep;

			// Point to start of good data
			s = i;

			LPREGVALUE prv = (LPREGVALUE)IFind( token );
			if ( prv != NULL )
			{				
				// Check for break
				if ( pBreak != NULL && prv->type == 0 )
				{	if ( pdwBreak != NULL ) *pdwBreak = i;
					strcpy( pBreak, token );
					return TRUE;
				} // end if
				
				// Write out the replace value
				else out->Write( GetSz( prv ) );

			} // end if

			else // Break
			{
				if ( pdwBreak != NULL ) *pdwBreak = i;
				strcpy( pBreak, token );
				return TRUE;
			} // end else

		} // end if

		// Next
		i++;

	} // end while	

	// Write out what's left of the data
	if ( i > s ) 
	{
		out->Write( (LPVOID)&in[ s ], i - s );
	} // end if

	return FALSE;
}

BOOL CRKey::Replace(LPCTSTR pSrc, LPCTSTR pDst)
{_STTEX();
	// Open files
	CWinFile src, dst;
	if ( !src.OpenExisting( pSrc, GENERIC_READ ) ) return FALSE;
	if ( !dst.OpenNew( pDst, GENERIC_WRITE ) ) return FALSE;

	// Read in data
	TMem< BYTE > in;	
	if ( !in.allocate( src.Size() ) ) return FALSE;
	if ( !src.Read( in.ptr(), in.size() ) ) return FALSE;
	src.Close();

	// Run replace function
	CPipe	outpipe;
	DWORD	op = 0, i = 0;
	char	token[ 256 ];
	while ( Replace(	&outpipe, &op, in.str(), in.size(), 
						NULL, NULL, token, NULL, &i ) )
	{

	} // end while

	// Write out the data
	return dst.Write( outpipe.GetBuffer(), outpipe.GetBufferSize() );
}

BOOL CRKey::Copy(CRKey *pRk, BOOL bMerge)
{_STTEX();
	// Sanity check
	if ( pRk == NULL ) return FALSE;

	// Destroy if not merging
	if ( !bMerge ) Destroy();

	// Add each value
	LPREGVALUE prv = NULL;
	while ( ( prv = (LPREGVALUE)pRk->GetNext( prv ) ) != NULL )
		Add( prv->type, prv->cpkey, prv->data, prv->size );

	return TRUE;
}

BOOL CRKey::Update(CRKey *pRk)
{_STTEX();
	// Sanity check
	if ( pRk == NULL ) return FALSE;

	// Add each value
	LPREGVALUE prv = NULL;
	while ( ( prv = (LPREGVALUE)GetNext( prv ) ) != NULL )
	{	LPREGVALUE uprv = (LPREGVALUE)pRk->Find( prv->cpkey );
		if ( uprv != NULL )
			Add( uprv->type, uprv->cpkey, uprv->data, uprv->size );
	} // end while

	return TRUE;
}

BOOL CRKey::Copy(CCfgFile *pCfg, LPCTSTR pGroup)
{_STTEX();
	// Sanity check
	if ( pCfg == NULL || pGroup == NULL ) return FALSE;

	// Find group
	HGROUP hGroup = pCfg->FindGroup( pGroup );
	if ( hGroup == NULL ) return FALSE;

	// Copy each element
	LPCFGELEMENTINFO	pcei = NULL;
	while ( ( pcei = pCfg->GetNextElement( hGroup, pcei ) ) != NULL )
	{
		if ( pcei->type == CFG_DWORD || pcei->size == 0 )
			Set( pcei->name, (DWORD)pcei->value );

		else switch( pcei->type )
		{
			case CFG_VOID :
			case CFG_BINARY :
				Set( pcei->name, pcei->value, pcei->size );
				break;
			
			case CFG_STR :
			case CFG_DOUBLE :
				Set( pcei->name, (LPSTR)pcei->value );
				break;

		} // end switch

	} // end while

	return TRUE;
}

DWORD CRKey::GetValueSize( LPCTSTR pName )
{_STTEX();
	LPREGVALUE prv = (LPREGVALUE)Find( pName );
	if ( prv == NULL ) return 0;

	// Get number
	if ( prv->type == REG_DWORD	) return sizeof( DWORD );

	return prv->size;
}

void* CRKey::GetValuePtr( LPCTSTR pName )
{_STTEX();
	LPREGVALUE prv = (LPREGVALUE)Find( pName );
	if ( prv == NULL ) return NULL;
	return prv->data;
}


BOOL CRKey::SaveWindowPos(HWND hWnd, LPCTSTR pName)
{_STTEX();
	// Sanity checks
	if ( !::IsWindow( hWnd ) || pName == NULL ) 
		return FALSE;

	char val[ CFG_STRSIZE ];

	BOOL bMinimized = IsIconic( hWnd );
	BOOL bMaximized = IsZoomed( hWnd );
	BOOL bVisible = IsWindowVisible( hWnd );

	// Save minimized state
	wsprintf( val, "%s (Maximized)", pName );
	Set( val, (DWORD)( bMaximized ? 1 : 0 ) );
	wsprintf( val, "%s (Minimized)", pName );
	Set( val, (DWORD)( bMinimized ? 1 : 0 ) );
	wsprintf( val, "%s (Visible)", pName );
	Set( val, (DWORD)( bVisible ? 1 : 0 ) );

	if ( bMinimized || bMaximized )
	{
		// Restore so we can get the position
		ShowWindow( hWnd, SW_HIDE );
		ShowWindow( hWnd, SW_RESTORE );
	} // end if
	
	RECT rect;
	GetWindowRect( hWnd, &rect );
	wsprintf( val, "%s (Window Position)", pName );
	Set( val, &rect, sizeof( rect ) );

	if ( bMinimized ) ShowWindow( hWnd, SW_SHOWMINIMIZED );
	else if ( bMaximized ) ShowWindow( hWnd, SW_SHOWMAXIMIZED );

	return TRUE;
}

BOOL CRKey::RestoreWindowPos(HWND hWnd, LPCTSTR pName, BOOL bSize, BOOL bPosition )
{_STTEX();
	// Sanity checks
	if ( !bPosition && !bSize ) return FALSE;
	if ( !::IsWindow( hWnd ) || pName == NULL ) 
		return FALSE;

	RECT rect;
	char val[ CFG_STRSIZE ];

	// Get rect
	wsprintf( val, "%s (Window Position)", pName );
	if ( !Get( val, &rect, sizeof( rect ) ) )
		return FALSE;

	// Size must be at least 50
	if ( ( rect.right - 50 ) < rect.left ) return FALSE;
	if ( ( rect.bottom - 50 ) < rect.top ) return FALSE;

	// Bounds check ( rect must lie completly on the screen )
	if (	rect.left < 0 || rect.right < 0 ||
			rect.top < 0 || rect.bottom < 0 ) return FALSE;
	long sx = GetSystemMetrics( SM_CXSCREEN );
	long sy = GetSystemMetrics( SM_CYSCREEN );
	if (	rect.left > sx || rect.right > sx ||
			rect.top > sy || rect.bottom > sy ) return FALSE;

	DWORD dwFlags = SWP_NOZORDER | SWP_NOACTIVATE;
	if ( !bPosition ) dwFlags |= SWP_NOMOVE;
	if ( !bSize ) dwFlags |= SWP_NOSIZE;

	// Restore the position
	SetWindowPos( hWnd, NULL,	rect.left, rect.top,
								rect.right - rect.left,
								rect.bottom - rect.top,
								dwFlags );

	BOOL maximized = 0, minimized = 0, visible = 0;
	wsprintf( val, "%s (Maximized)", pName );
	Get( val, (LPDWORD)&maximized );
	wsprintf( val, "%s (Minimized)", pName );
	Get( val, (LPDWORD)&minimized );
	wsprintf( val, "%s (Visible)", pName );
	Get( val, (LPDWORD)&visible );

	// Restore window position
	if ( !visible ) ShowWindow( hWnd, SW_HIDE );
	else if ( maximized ) ShowWindow( hWnd, SW_SHOWMAXIMIZED );
	else if ( minimized ) ShowWindow( hWnd, SW_SHOWMINIMIZED );
	else ShowWindow( hWnd, SW_SHOWNORMAL );
	

	return TRUE;
}

BOOL CRKey::IsSet(LPCTSTR pName)
{_STTEX();
	return ( Find( pName ) != NULL );
}

BOOL CRKey::EncodeHttpHeaders(CPipe *pPipe)
{_STTEX();
	// Add each value
	LPREGVALUE prv = NULL;
	while ( ( prv = (LPREGVALUE)GetNext( prv ) ) != NULL )
	{
		// Write separator if needed
//		if ( pPipe->GetBufferSize() ) pPipe->Write( "\r\n" );

		pPipe->Write( prv->cpkey );
		pPipe->Write( ": " );
		pPipe->Write( GetSz( prv ) );
		pPipe->Write( "\r\n" );

	} // end while

	return TRUE;
}

BOOL CRKey::EncodeUrl(CPipe *pPipe, DWORD dwEncoding, char chSepNameVal, char chSepValues)
{_STTEX();
	// Add each value
	TMem< char > buf;
	LPREGVALUE prv = NULL;
	while ( ( prv = (LPREGVALUE)GetNext( prv ) ) != NULL )
	{
		// Write separator if needed
		if ( pPipe->GetBufferSize() ) pPipe->Write( &chSepValues, 1 );

		switch( dwEncoding )
		{
			case 0 :
			{
				// Write the key
				pPipe->Write( prv->cpkey, prv->ksize );			

				// Separator
				pPipe->Write( &chSepNameVal, 1 );

				// Write raw value
				if ( REG_BINARY == prv->type )
					pPipe->Write( prv->pbdata, prv->size );
				else pPipe->Write( GetSz( prv ) );

			} break;

			case 1 :
			{

				// Write the name
				if ( buf.grow( CCfgFile::GetMinCanonicalizeBufferSize( strlen( prv->cpkey ) ) ) )
				{	buf.Zero();
					CCfgFile::CanonicalizeBuffer( buf, prv->bpkey, strlen( prv->cpkey ) );
					pPipe->Write( buf );
				} // end if

				// Separator
				pPipe->Write( &chSepNameVal, 1 );

				// Write binary buffers
				if ( REG_BINARY == prv->type )
				{
					// Binary encoding
					if ( buf.grow( CCfgFile::GetMinCanonicalizeBufferSize( prv->size ) ) )
					{	buf.Zero();
						CCfgFile::CanonicalizeBuffer( buf, prv->pbdata, prv->size );
						pPipe->Write( buf );
					} // end if

				} // end if

				else
				{
					// Write the name
					LPCTSTR val = GetSz( prv );
					if ( buf.grow( CCfgFile::GetMinCanonicalizeBufferSize( strlen( val ) ) ) )
					{	buf.Zero();
						CCfgFile::CanonicalizeBuffer( buf, (LPBYTE)val, strlen( val ) );
						pPipe->Write( buf );
					} // end if

				} // end if
			} break;

			case 2 :
			{
				TMem< char > buf2;

				// Write the name
				if ( buf.grow( CCfgFile::GetMinCanonicalizeBufferSize( strlen( prv->cpkey ) ) ) )
				{	buf.Zero();
					CCfgFile::CanonicalizeBuffer( buf, prv->bpkey, strlen( prv->cpkey ) );
					pPipe->Write( buf );
				} // end if

				// Separator
				pPipe->Write( &chSepNameVal, 1 );

				// Write binary buffers
				if ( REG_BINARY == prv->type )
				{
					// Binary encoding
					if ( buf.grow( CCfgFile::GetMinCanonicalizeBufferSize( prv->size ) ) )
					{	buf.Zero();
						CCfgFile::CanonicalizeBuffer( buf, prv->pbdata, prv->size );
					} // end if

				} // end if

				else
				{
					// Write the name
					LPCTSTR val = GetSz( prv );
					if ( buf.grow( CCfgFile::GetMinCanonicalizeBufferSize( strlen( val ) ) ) )
					{	buf.Zero();
						CCfgFile::CanonicalizeBuffer( buf, (LPBYTE)val, strlen( val ) );
					} // end if

				} // end if

				// Encoded again then add to buffer
				if ( buf2.grow( CCfgFile::GetMinCanonicalizeBufferSize( strlen( buf.ptr() ) ) ) )
				{	buf2.Zero();
					CCfgFile::CanonicalizeBuffer( buf2, (LPBYTE)buf.ptr(), strlen( buf.ptr() ) );
					pPipe->Write( buf2 );
				} // end if

			} break;

		} // end switch

	} // end while

	return TRUE;
}

BOOL CRKey::ReadMIME(LPBYTE buf, DWORD size)
{_STTEX();
	DWORD i = 0;

	BOOL endvars = FALSE;
	char token[ CWF_STRSIZE ];
	while ( !endvars && i < size )
	{
		// Forgive one CRLF
		if ( i < size && buf[ i ] == 0x0d ) i++;
		if ( i < size && buf[ i ] == 0x0a ) i++;

		// Skip white space
		while ( i < size && ( buf[ i ] <= ' ' || buf[ i ] > '~' ) &&
				buf[ i ] != 0x0d && buf[ i ] != 0x0a ) 
			i++;

		// Are there any variables?
		if ( buf[ i ] != 0x0d && buf[ i ] != 0x0a )
		{
			// Punt if end of data
			if ( i >= size ) { return TRUE; }

			// Copy token
			DWORD x = 0;
			while (	i < size &&
					buf[ i ] != ':' && 
					buf[ i ] > ' ' && 
					buf[ i ] <= '~' && 
					x < sizeof( token ) - 1 )
				token[ x++ ] = buf[ i++ ];
			token[ x ] = NULL;

			// Did we get the separator?
			if ( buf[ i++ ] == ':' )
			{
				// Skip white space
				while ( ( buf[ i ] <= ' ' || buf[ i ] > '~' ) &&
						buf[ i ] != 0x0d && buf[ i ] != 0x0a ) i++;

				DWORD e = i, skip;

				// Find the end 
				BOOL end = FALSE;
				while ( !end && !endvars ) 
				{
					// Find the end
					while ( buf[ e ] != 0x0d && buf[ e ] != 0x0a && e < size ) e++;
					if( buf[ e ] != 0x0d && buf[ e ] != 0x0a ) return FALSE;
					if ( buf[ ++e ] == 0x0a ) e++;
					if ( buf[ e ] > ' ' || buf[ e ] == 0x0d || buf[ e ] == 0x0a ) 
						end = TRUE;

					// Check for end of variables
					if ( buf[ e ] == 0x0d || buf[ e ] == 0x0a ) endvars = TRUE;

				} // end while

				// Remember where the end is
				skip = e;

				// Lose trailing white space
				e--; while ( e > i && ( buf[ e ] <= ' ' || buf[ e ] > '~' ) ) e--;
				
				if ( e > i )
				{
					// Add this variable
					Add( REG_SZ, token, &buf[ i ], e - i + 1 );

				} // end if

				// Forge ahead
				i = skip;

			} // end if

			// Skip to next line
			else while ( i < size && buf[ i ] != 0x0d && buf[ i ] != 0x0a ) i++;

		} // end if

		else endvars = TRUE;

	} // end while	

	return TRUE;
}

BOOL CRKey::ReadInline(LPBYTE buf, DWORD size, char sep, BOOL bDeCanonicalize)
{_STTEX();
	// Sanity checks
	if ( buf == NULL || size == 0 ) return FALSE;

	DWORD	i = 0;
	char	token[ CWF_STRSIZE ];

	while ( i < size && buf[ i ] != 0 )
	{
		DWORD t = 0;

		// Skip white space
		while ( i < size && ( buf[ i ] <= ' ' || buf[ i ] > '~' ) && buf[ i ] != 0 ) i++;
		if ( i >= size || buf[ i ] == 0 ) return TRUE;

		// Read in first token
		while ( i < size && t < ( CWF_STRSIZE - 1 ) &&
				buf[ i ] > ' ' && buf[ i ] <= '~' &&
				buf[ i ] != '=' )
			token[ t++ ] = buf[ i++ ];
		token[ t ] = 0;

		{ // DeCanonicalize token
			
			DWORD dwMin = CCfgFile::GetMinDeCanonicalizeBufferSize( strlen( token ) );
			
			TMem< BYTE > mem; DWORD dwSize = 0;
			if ( mem.allocate( dwMin + 1 ) ) 
			{	if ( CCfgFile::DeCanonicalizeBuffer( token, (LPBYTE)mem.ptr(), strlen( token ), &dwSize ) ) 
				{	memcpy( token, mem.ptr(), dwSize ); token[ dwSize ] = 0; }
			} // end if

		} // end decanoicalize token

		// Check for '='
		if ( buf[ i ] == '=' )
		{	i++;

			// Skip starting spaces
			while ( i < size && buf[ i ] == ' ' ) i++;

			DWORD s = i;
			BOOL quoted = FALSE;

			// Find the end of the data
			while ( i < size &&
					( ( quoted && buf[ i ] >= ' ' ) || buf[ i ] > ' ' ) && 
					buf[ i ] <= '~' && buf[ i ] != sep ) 
			{
				// Toggle quote mode
				if ( buf[ i ] == '"' ) quoted = !quoted;

				// Next char
				i++;
			} // end if

			if ( i > s )
			{
				if ( bDeCanonicalize )
				{
					// DeCanonicalize data
					DWORD dwMin = CCfgFile::GetMinDeCanonicalizeBufferSize( i - s );

					TMem< BYTE > mem;
					if ( mem.allocate( dwMin + 1 ) ) 
					{
						// Do it
						DWORD dwSize = 0;
						if ( CCfgFile::DeCanonicalizeBuffer( (LPCTSTR)&buf[ s ], (LPBYTE)mem.ptr(), i - s, &dwSize ) ) 

							// Add this variable
							Add( REG_SZ, token, mem.ptr(), dwMin );

					} // end if

				} // end if

				// Add variable if any
				else Add( REG_SZ, token, &buf[ s ], i - s );

			} // end if

			// Skip separator
			if ( buf[ i ] == sep ) i++;

		} // end if

		// Add null
		else Add( REG_DWORD, token, NULL, sizeof( DWORD ) );

	} // end while

	return TRUE;
}


BOOL CRKey::DeleteValue(LPCTSTR pValue)
{_STTEX();
	void *ptr = Find( pValue );
	if ( ptr == NULL ) return FALSE;

	Delete( ptr );

	return TRUE;
}

BOOL CRKey::Set(LPCTSTR pName, CRKey *pRk)
{_STTEX();

	if ( !pRk || !pRk->Size() )
	{
		// Clear data
		Set( pName, "" );

		return TRUE;

	} // end if

	// Serialize variables
	CPipe pipe;
	pRk->EncodeUrl( &pipe );

	// Write a NULL character
	pipe.Write( "", 1 );

	// Save the serialized variables
	Set( pName, (LPCTSTR)pipe.GetBuffer() );

	return TRUE;
}

BOOL CRKey::Get(LPCTSTR pName, CRKey *pRk)
{_STTEX();

	if ( !pRk ) return FALSE;

	// Lose current variables
	pRk->Destroy();

	// Read in new variables
	return pRk->ReadInline(	(LPBYTE)GetValuePtr( pName ),
							GetValueSize( pName ) );
}
