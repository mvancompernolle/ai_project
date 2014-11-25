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
// FileSearch.cpp: implementation of the CFileSearch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define SM_ESCAPE		'&'
#define SM_INCLUDE		'+'
#define SM_EXCLUDE		'-'
#define SM_QUOTE		'\"'


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileSearch::CFileSearch()
{_STT();

}

CFileSearch::~CFileSearch()
{_STT();

}

BOOL CFileSearch::Search(LPCTSTR pText, LPCTSTR pDir, LPCTSTR pExt)
{_STT();
	// Sanity checks
	if ( pText == NULL || pDir == NULL || pExt == NULL )
		return FALSE;

	return FindString( pText, pDir, pExt );
}

BOOL CFileSearch::AddFile(LPCTSTR pFile, LPCTSTR pLoc)
{_STT();
	// Sanity check
	if ( pFile == NULL ) return FALSE;

	char fname[ CWF_STRSIZE ] = { 0 };
	CWinFile::GetFileNameFromPath( pFile, fname );

	// Allocate memory
	LPFILESEARCHINFO pfsi = (LPFILESEARCHINFO)New( NULL, NULL, fname );
	if ( pfsi == NULL ) return FALSE;

	// Save path
	strcpy( pfsi->fname, pFile );
	
	// Save relative location
	if ( pLoc == NULL ) *pfsi->loc = 0;
	else strcpy( pfsi->loc, pLoc );

	return TRUE;
}


BOOL CFileSearch::GetExtension(DWORD ext, LPSTR pExt, LPCTSTR pExtList, BYTE sep)
{_STT();
	if ( pExt == NULL || pExtList == NULL ) return FALSE;

	DWORD i = 0, x = 0;

	while ( pExtList[ i ] != 0 )
	{
		if ( pExtList[ i ] == sep ) 
		{
			if ( ext == 0 ) { pExt[ x ] = 0; return TRUE; }
			else ext--, x = 0;
		} // end if

		else if ( pExtList[ i ] > ' ' && pExtList[ i ] <= '~' )
			pExt[ x++ ] = pExtList[ i ];

		// Next char
		i++;

	} // end while

	// Did we get one?
	if ( ext == 0 && x > 0 ) 
	{	pExt[ x ] = 0; return TRUE; }

	return FALSE;
}

BOOL CFileSearch::FindString(LPCTSTR pText, LPCTSTR pDir, LPCTSTR pExt, LPCTSTR pLoc )
{_STT();
	// Save output folder
	char path[ CWF_STRSIZE ];
	char loc[ CWF_STRSIZE ];

	WIN32_FIND_DATA		wfd;
	char				search[ CWF_STRSIZE ];
	HANDLE				hFind;

	// Get first file
	CWinFile::BuildPath( search, pDir, "*.*" );

	// Punt if empty
	hFind = FindFirstFile( search, &wfd );
	if ( hFind == INVALID_HANDLE_VALUE ) return FALSE;
	
	do
	{
		// Do we need to copy this file?
		if (	*wfd.cFileName != NULL &&
				strcmp( wfd.cFileName, "." ) &&	strcmp( wfd.cFileName, ".." ) )				
		{
			// Build full path to file
			CWinFile::BuildPath( path, pDir, wfd.cFileName );

			// Recurse if directory
			if ( ( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
			{
				if ( pLoc == NULL ) *loc = 0;
				else strcpy( loc, pLoc );
				CWinFile::BuildPath( loc, loc, wfd.cFileName );

				FindString( pText, path, pExt, loc );
			} // end if

			// Search this file
			else 
			{
				BOOL bCheck = FALSE;
				char ext[ 256 ];
				LPCTSTR fext = CWinFile::GetExtension( wfd.cFileName );

				if ( fext != NULL && *fext != 0 )
				{
					// Check the extension
					for ( DWORD i = 0; !bCheck && GetExtension( i, ext, pExt ); i++ )
						if ( !strcmpi( ext, fext ) ) bCheck = TRUE;

					// Check file if needed
					if ( bCheck && FindInFile( pText, path ) ) 
						AddFile( path, pLoc );

				} // end if

			} // end if

		} // end if

	// Get the next file or directory
	} while ( FindNextFile( hFind, &wfd ) );
	
	// Close the find handle
	FindClose( hFind );

	return TRUE;
}

BOOL CFileSearch::FindInFile(LPCTSTR pString, LPCTSTR pFile)
{_STT();
	// Sanity check
	if ( pString == NULL || pFile == NULL ) return FALSE;

	CWinFile	file;

	// Open the file
	if ( !file.OpenExisting( pFile, GENERIC_READ ) ) return FALSE;

	// Create a buffer
	TMem< char > buf;
	if ( !buf.allocate( file.Size() + 2 ) ) return FALSE;

	// Read in the data
	DWORD read = 0;
	if ( !file.Read( buf, file.Size(), &read ) || !read ) return FALSE;

	file.Close();

	return FindInMem( pString, buf, read );
}

BOOL CFileSearch::IsToken(LPCTSTR buf, DWORD len, LPCTSTR token)
{_STT();
	// Search buffer for token
	for ( DWORD i = 0; i < len && ( ( len - i ) + 1 ) > strlen( token ); i++ )
	{
		if ( !strnicmp( &buf[ i ], token, strlen( token ) ) )
			return TRUE;
	} // end for

	return FALSE;
}

BOOL CFileSearch::FindInMem(LPCTSTR pString, LPCTSTR buf, DWORD size)
{_STT();
	// Sanity check
	if ( pString == NULL || buf == NULL || size == 0 ) return FALSE;

	CWinFile	file;

	DWORD		tokens = 0;
	char		token[ 64 ][ CWF_STRSIZE ];
	DWORD		flags[ 64 ];
	
	{ // Parse search string
		DWORD	x = 0;
		DWORD	y = 0;
		BOOL	quo = FALSE;

		flags[ x ] = 0;

		for ( DWORD i = 0; x < 60 && i < CWF_STRSIZE && pString[ i ] != 0x0; i++ )
		{			
			// Escape character
			if ( pString[ i ] == SM_ESCAPE && pString[ i + 1 ] != 0 )
			{	i++;
				token[ x ][ y++ ] = pString[ i ];
			} // end if

			// Check for quoted string
			else if ( pString[ x ] == SM_QUOTE ) quo = !quo;

			// Check for quote
			else if ( quo ) token[ x ][ y++ ] = pString[ i ];

			// Check for include
			else if ( y == 0 && pString[ i ] == SM_INCLUDE ) flags[ x ] = 1;

			// Check for exclude
			else if ( y == 0 && pString[ i ] == SM_EXCLUDE ) flags[ x ] = 2;

			// Token break
			else if ( pString[ i ] == ' ' )
			{
				if ( y != 0 )
				{
					token[ x ][ y ] = 0x0;
					if ( x < 30 ) x++;
					y = 0;
				} // end if

				// Reset flags
				flags[ x ] = 0;
			} // end else if

			// Just store it
			else token[ x ][ y++ ] = pString[ i ];

		} // end for

		// terminate the last token
		token[ x ][ y ] = 0x0;

		// Save the number of tokens
		tokens = x;

		if ( y > 0 ) tokens++;

	} // end Parse

	BOOL ok = FALSE;
	BOOL reject = FALSE;

	// Perform search
	for ( DWORD i = 0; i < tokens && !reject; i++ )
	{
		if ( IsToken( buf, size, token[ i ] ) )
		{
			if ( flags[ i ] == 2 ) reject = TRUE;

			else ok = TRUE;

		} // end if

		else if ( flags[ i ] == 1 ) reject = TRUE;

	} // end for

	return ( !reject && ok );
}
