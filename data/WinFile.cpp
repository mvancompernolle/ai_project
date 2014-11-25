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
// WinFile.cpp: implementation of the CWinFile class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "vfw.h"

#include <ShlObj.h>
#include <Shellapi.h>
#include <stdio.h>

#ifndef ASSERT
#define ASSERT
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWinFile::CWinFile()
{_STTEX();
	// Cache
	m_bEnableCache = FALSE;
	m_dwWrCachePtr = 0;
	m_pucWrCache = NULL;
	m_dwWrCacheSize = CWINFILE_DEFAULTCACHESIZE;
	m_dwRdCachePtr = 0;
	m_dwRdCacheBytes = 0;
	m_pucRdCache = NULL;
	m_dwRdCacheSize = CWINFILE_DEFAULTCACHESIZE;

	m_hFile = INVALID_HANDLE_VALUE;	
	DefaultOpenParams();

#ifdef DYNAMIC_CRC_TABLE
	crc_table = NULL;
#endif

#ifdef CWF_SUPPORTENCRYPTION
	m_dwHashPtr = sizeof( m_pHashBuf );
	*m_szKey = 0;
	ZeroMemory( m_pHashBuf, sizeof( m_pHashBuf ) );
#endif

}

CWinFile::~CWinFile()
{_STTEX();
	Close();

#ifdef DYNAMIC_CRC_TABLE
	if ( crc_table != NULL )
	{
		delete [] crc_table;
		crc_table = NULL;
	} // end if
#endif

}

LONGLONG CWinFile::GetPtrPos()
{_STTEX();
	if ( !IsOpen() ) return 0;

	// Get the current file pointer position
	LONG lHi = 0;
	DWORD pos = SetFilePointer( m_hFile, 0, &lHi, FILE_CURRENT );

	// Was there an error?
	if ( pos == 0xffffffff ) return 0;

	return pos;

} // end GetPointer()

BOOL CWinFile::GetPtrPos( LPDWORD ptr )
{_STTEX();
	// Sanity check
	if ( ptr == NULL ) return FALSE;
	if ( !IsOpen() ) return FALSE;

	// Get the current file pointer position
	*ptr = SetFilePointer( m_hFile, 0, NULL, FILE_CURRENT );

	// Was there an error?
	if ( *ptr == 0xffffffff ) 
	{
		*ptr = 0;
		return FALSE; 
	} // end if

	// 
	return TRUE;

} // end GetPointer()

BOOL CWinFile::GetPtrPos( LONGLONG *ptr )
{_STTEX();
	// Sanity check
	if ( ptr == NULL ) return FALSE;
	if ( !IsOpen() ) return FALSE;

	// Get the current file pointer position
	LONG lHi = 0;
	*ptr = SetFilePointer( m_hFile, 0, &lHi, FILE_CURRENT );

	// Was there an error?
	if ( *ptr == 0xffffffff && GetLastError() != NO_ERROR ) 
	{	*ptr = 0;
		return FALSE; 
	} // end if

	// Add the hi part of the pointer
	*ptr |= ( ( (LONGLONG)lHi ) << 32 );

	// 
	return TRUE;

} // end GetPointer()

void CWinFile::Close()
{_STTEX();
	// Call to delete cache buffers
	FlushCache();

	// Close the file if open
	if ( !IsOpen() ) return;
	FlushFileBuffers( m_hFile );
	CloseHandle( m_hFile );
	m_hFile = INVALID_HANDLE_VALUE;

#ifdef CWF_SUPPORTENCRYPTION
	m_dwHashPtr = sizeof( m_pHashBuf );
#endif

}

BOOL CWinFile::Open( LPCTSTR pFilename, DWORD access )
{_STTEX();
	// Close any open file
	Close();

	if ( access != 0xffffffff ) m_ofp.dwDesiredAccess = access;

	// Set the users filename if needed
	if ( pFilename != NULL ) SetPath( pFilename );

	// Make sure we have a filename
	if ( !IsPath() ) return FALSE;

	// Attempt to open a new file
	m_hFile = CreateFile(	m_szPath, 
							m_ofp.dwDesiredAccess,
							m_ofp.dwShareMode,
							m_ofp.lpSecurityAttributes,
							m_ofp.dwCreationDisposition,
							m_ofp.dwFlagsAndAttributes,
							m_ofp.hTemplateFile );
	return IsOpen();
}

BOOL CWinFile::OpenNew( LPCTSTR pFilename, DWORD access )
{_STTEX();
	DefaultOpenParams();
	m_ofp.dwCreationDisposition = CREATE_ALWAYS;

	return Open( pFilename, access );

}

BOOL CWinFile::OpenExisting( LPCTSTR pFilename, DWORD access )
{_STTEX();
	DefaultOpenParams();
	m_ofp.dwCreationDisposition = OPEN_EXISTING;

	return Open( pFilename, access );
}

BOOL CWinFile::OpenAlways(LPCTSTR pFilename, DWORD access )
{_STTEX();
	DefaultOpenParams();

	return Open( pFilename, access );
}

void CWinFile::DefaultOpenParams()
{_STTEX();
	m_ofp.dwDesiredAccess			= GENERIC_READ | GENERIC_WRITE;
	m_ofp.dwShareMode				= FILE_SHARE_READ | FILE_SHARE_WRITE;
	m_ofp.lpSecurityAttributes		= NULL;
	m_ofp.dwCreationDisposition		= OPEN_ALWAYS;	
	m_ofp.dwFlagsAndAttributes		= FILE_ATTRIBUTE_NORMAL;
	m_ofp.hTemplateFile				= NULL;
}


BOOL CWinFile::Read(LPVOID pBuf, DWORD dwLen, LPDWORD pdwRead, BOOL bBypassCache )
{_STTEX();
	DWORD	read = 0;
	
	// Sanity checks
	if ( !IsOpen() || pBuf == NULL || dwLen == 0 ) return FALSE;

	// Attempt to use the cache
	if ( !bBypassCache && m_bEnableCache )
		if ( ReadCache( pBuf, dwLen, pdwRead ) ) return TRUE;

	RULIB_TRY
	{
		// Attempt to read from the file
		if ( !ReadFile( m_hFile, pBuf, dwLen, &read, NULL ) )
			return FALSE;

#ifdef CWF_SUPPORTENCRYPTION
		CryptoDoHash( (LPBYTE)pBuf, dwLen );
#endif
		
	} // end try
	RULIB_CATCH_ALL
	{
		ASSERT( 0 );
		return FALSE;
	} // end catch

	// Let the user know how many bytes were read if interested
	if ( pdwRead != NULL ) *pdwRead = read;

	return ( read != 0 );
}

BOOL CWinFile::Write( LPVOID pBuf, DWORD dwLen, LPDWORD pdwWritten, BOOL bBypassCache )
{
	DWORD	written;

	// Sanity checks
	if ( !IsOpen() || pBuf == NULL ) return FALSE;

	// Zero length means NULL teminated string
	if ( dwLen == 0 )
	{
		// Count characters
		while ( ((LPBYTE)pBuf)[ dwLen ] != 0x0 ) dwLen++;

	} // end if
	if ( dwLen == 0 ) return FALSE;

	// Attempt to use the cache
	if ( !bBypassCache && m_bEnableCache )
		if ( WriteCache( pBuf, dwLen, pdwWritten ) ) return TRUE;

	RULIB_TRY
	{
#ifdef CWF_SUPPORTENCRYPTION
		TMem< BYTE > buf;
		if ( *m_szKey )
		{	
			if ( buf.allocate( dwLen + 1 ) )
			{	memcpy( buf, pBuf, dwLen );
				pBuf = buf;
				CryptoDoHash( (LPBYTE)pBuf, dwLen );
			} // end if

		} // end if
#endif

		// Attempt to read from the file
		if ( !WriteFile( m_hFile, pBuf, dwLen, &written, NULL ) )
			return FALSE;

	} // end try
	RULIB_CATCH_ALL
	{
		ASSERT( 0 );
		return FALSE;
	} // end catch

	// Let the user know how many bytes went out if interested
	if ( pdwWritten != 0x0 ) *pdwWritten = written;

	return TRUE;
}

BOOL CWinFile::SaveStruct( LPVOID pStruct, DWORD dwLen )
{_STTEX();
	if ( dwLen == 0x0 ) dwLen = *((LPDWORD)pStruct);

	return Write( pStruct, dwLen );
}

BOOL CWinFile::RestoreStruct( LPVOID pStruct, DWORD dwLen )
{_STTEX();
	if ( dwLen == 0x0 ) dwLen = *((LPDWORD)pStruct);

	return Read( pStruct, dwLen );
}

BOOL CWinFile::CopyFile( HANDLE hSource, HANDLE hTarget )
{_STTEX();
	if (	hSource == INVALID_HANDLE_VALUE ||
			hTarget == INVALID_HANDLE_VALUE ) return FALSE;
	
	BYTE	buf[ 1024 ];
	DWORD	bytes;
		
	// For the entire file
	while ( ReadFile( hSource, buf, 1024, &bytes, NULL ) && bytes )
	{
		// Write the bytes to the end of the file
		if ( !WriteFile( hTarget, buf, bytes, &bytes, NULL ) )
			return FALSE;
	} // end while

	return TRUE;
}

BOOL CWinFile::CopyFile(HANDLE hFile)
{_STTEX();
	// Do we have a file open
	if ( !IsOpen() ) return FALSE;

	// copy to the current file location
	return CopyFile( hFile, m_hFile );
}

BOOL CWinFile::Copy(HANDLE hSource, HANDLE hTarget, DWORD dwCopy, LPDWORD pdwCopied)
{_STTEX();
	// Sanity check
	if (	hSource == INVALID_HANDLE_VALUE ||
			hTarget == INVALID_HANDLE_VALUE ) return FALSE;
	
	BYTE	buf[ 1024 ];
	DWORD	bytes;
	DWORD	want;

	// No bytes yet
	if ( pdwCopied != NULL ) *pdwCopied = 0;
	
	do 
	{
		// Update byte stuff
		want = ( dwCopy > 1024 ) ? 1024 : dwCopy;
		if ( dwCopy > 1024 ) dwCopy -= 1024;
		else dwCopy = 0;

		// Read in the desired number of bytes
		if ( !ReadFile( hSource, buf, want, &bytes, NULL ) )
			return FALSE;

		// Is there anything to copy?
		if ( bytes == 0 ) return TRUE;

		// Write the bytes to the end of the file
		if ( !WriteFile( hTarget, buf, bytes, &bytes, NULL ) )
			return FALSE;

		// Add the number of bytes actually copied
		if ( pdwCopied != NULL ) *pdwCopied += bytes;

	} while ( dwCopy > 0 );

	return TRUE;
}

BOOL CWinFile::Copy(HANDLE hFile, DWORD dwCopy, LPDWORD pdwCopied)
{_STTEX();
	// Do we have a file open
	if ( !IsOpen() ) return FALSE;

	// copy to the current file location
	return Copy( hFile, m_hFile, dwCopy, pdwCopied );
}

void CWinFile::GetFileNameFromPath( LPCTSTR pPath, LPSTR pFileName, BOOL bNoExt )
{_STTEX();
	// Sanity check
	if ( pPath == NULL || pFileName == NULL ) return;

	char *fname;
	
	// Start at the beginning
	fname = (char*)pPath;

	// Point to the byte after the last '\'
	for (	DWORD i = 0; 
			i < CWF_STRSIZE && pPath[ i ] != 0x0;
			i++ ) 
		if ( ( pPath[ i ] == '\\' || 
			 ( pPath[ i ] == '/' ) && pPath[ i + 1 ] != '/' && 
			   ( i == 0 || pPath[ i - 1 ] != '/' ) ) )
			fname = (char*)&( pPath[ i + 1 ] );

	// Copy this for the caller
	strcpy( pFileName, fname );

	if ( bNoExt )
	{
		char *ptr = NULL;
		for ( DWORD i = 0; i < CWF_STRSIZE && pFileName[ i ] != 0x0; i++ )
		{
			if ( pFileName[ i ] == '.' ) ptr = &pFileName[ i ];
			if ( pFileName[ i ] == '\\' || pFileName[ i ] == '/' ) 
				ptr = NULL;

		} // end if
		if ( ptr != NULL ) *ptr = 0x0;
	} // end if

}

BOOL CWinFile::GetPathFromFileName(LPCTSTR pFileName, LPSTR pPath)
{_STTEX();
	if ( pFileName == NULL || pPath == NULL ) return FALSE;

	BOOL	bIsPath = FALSE;
	char	*ptr;
	char	temp[ CWF_STRSIZE ];
	strcpy( temp, pFileName );

	// Assume no path
	*pPath = 0x0;

	// Point to the last '\'
	for (	DWORD i = 0; 
			i < CWF_STRSIZE && temp[ i ] != 0x0;
			i++ ) 
		if ( pFileName[ i ] == '\\' || pFileName[ i ] == '/' ) 
		{
			bIsPath =TRUE;
			ptr = &(temp[ i ] );
		} // end if
	if ( !bIsPath ) return FALSE;

	// Give the user just the path
	*ptr = 0x0;
	strcpy( pPath, temp );

	return TRUE;

}

LPCTSTR CWinFile::WebBuildPath(LPSTR pFullPath, LPCTSTR pPath, LPCTSTR pFileName )
{_STTEX();
	return BuildPath( pFullPath, pPath, pFileName, '/' ); }

LPCTSTR CWinFile::BuildPath(LPSTR pFullPath, LPCTSTR pPath, LPCTSTR pFileName, char sep )
{_STTEX();
	if ( pFullPath == NULL ) return NULL;

	char	fullpath[ CWF_STRSIZE ];
	char    sepstr[ 2 ] = { sep, 0 };

	// Copy the path
	if ( pPath != NULL ) { strcpy_sz( fullpath, pPath ); }
	else *fullpath = NULL;

	// 
	if ( pFileName != NULL )
	{
		// Skip leading '\\'
		while ( *pFileName == '\\' || *pFileName == '/' || *pFileName == sep )
			pFileName++;

		if ( *pFileName != NULL )
		{
			if (	fullpath[ 0 ] != NULL && 
					fullpath[ strlen( fullpath ) - 1 ] != '\\' &&
					fullpath[ strlen( fullpath ) - 1 ] != '/' )
				strcat( fullpath, sepstr );
			strncat( fullpath, pFileName, CWF_STRSIZE - strlen( fullpath ) - 1 );
			fullpath[ CWF_STRSIZE - 1 ] = NULL;
		} // end if
	} // end if

	// Remove trailing backslashes
	long l;
	while ( ( l = strlen( fullpath ) ) > 0 &&
			( fullpath[ l - 1 ] == '\\' ||
			  fullpath[ l - 1 ] == '/' ||
			  fullpath[ l - 1 ] == sep ) )
		fullpath[ l - 1 ] = NULL;

	// Copy to user buffer and correct separators
	DWORD i = 0;
	while ( fullpath[ i ] != 0 && i < CWF_STRSIZE )
	{
		if ( fullpath[ i ] == '\\' || fullpath[ i ] == '/' )
			pFullPath[ i ] = sep;
		else pFullPath[ i ] = fullpath[ i ];

		i++;

	} // end while
	pFullPath[ i ] = 0;

	return pFullPath;
}

BOOL CWinFile::IsRelative(LPCTSTR pFileName)
{_STTEX();
	if ( pFileName == NULL ) return FALSE;

	if ( !strcmpi( pFileName, "." ) ) return TRUE;
	else if ( !strcmpi( pFileName, ".." ) ) return TRUE;

	return FALSE;
}

BOOL CWinFile::CreateTemp( LPSTR pFileName )
{_STTEX();
	char	path[ CWF_STRSIZE ];
	char	fname[ CWF_STRSIZE ];

	// Sanity check
	if ( pFileName == NULL ) return FALSE;

	// First see if we have a temporary path
	if ( IsTempPath() ) strcpy( path, m_szTempPath );

	// Where does the user keep her temporary files
	else if ( !::GetTempPath( ( CWF_STRSIZE - 12 ), path ) )
		return FALSE;

	// Make sure this directory exists
	if ( !DoesExist( path ) )
		if ( !CreateDirectory( path ) ) return FALSE;

	// Create a temporary file in that directory
	if ( !GetTempFileName( path, "~WF", 0, fname ) )
		return FALSE;

	strcpy( pFileName, fname );

	return TRUE;
}

BOOL CWinFile::OpenTemp()
{_STTEX();
	Close();

	if ( !CreateTemp( m_szPath ) )
	{
		*m_szPath = 0x0;
		return FALSE;
	} // end if

	// Open the file
	return OpenExisting( m_szPath );
}


BOOL CWinFile::GetSaveFileName(	LPSTR pFileName, LPCTSTR pTitle, 
								LPCTSTR pFilter, LPCTSTR pDefExt,
								HWND hOwner, BOOL bPreview, DWORD dwFlags )
{_STTEX();
	// Sanity check
	if ( pFileName == NULL ) return FALSE;

	// Open file structure
	OPENFILENAME	ofn;

	// Initialize open filename structure
	ZeroMemory( (LPVOID)&ofn, sizeof( OPENFILENAME ) );

	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.lpstrFilter	= ( pFilter != NULL ) ? pFilter : "All Files (*.*)\x0*.*\x0\x0";
	ofn.lpstrTitle	= ( pTitle != NULL ) ? pTitle : "Save as";
	ofn.Flags		= dwFlags;
	ofn.lpstrFile	= pFileName;
	ofn.nMaxFile	= CWF_STRSIZE;
	ofn.lpstrDefExt	= pDefExt;
	ofn.hwndOwner	= hOwner;

//	if ( bPreview ) return ::GetSaveFileNamePreview( &ofn );
	return ::GetSaveFileName( &ofn );
}

BOOL CWinFile::GetOpenFileName(	LPSTR pFileName, LPCTSTR pTitle, 
								LPCTSTR pFilter, LPCTSTR pDefExt,
								HWND hOwner, BOOL bPreview, DWORD dwFlags,
								DWORD dwBufSize )
{_STTEX();
	// Sanity check
	if ( pFileName == NULL ) return FALSE;

	// Open file structure
	OPENFILENAME	ofn;

	// Initialize open filename structure
	ZeroMemory( (LPVOID)&ofn, sizeof( OPENFILENAME ) );

	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.lpstrFilter	= ( pFilter != NULL ) ? pFilter : "All Files (*.*)\x0*.*\x0\x0";
	ofn.lpstrTitle	= ( pTitle != NULL ) ? pTitle : "Open file";
	ofn.Flags		= dwFlags;
	ofn.lpstrFile	= pFileName;
	ofn.nMaxFile	= CWF_STRSIZE;
	ofn.lpstrDefExt	= pDefExt;
	ofn.hwndOwner	= hOwner;

//	if ( bPreview ) return ::GetOpenFileNamePreview( &ofn );	
	return ::GetOpenFileName( &ofn );
}

BOOL CWinFile::CreateDirectory(LPCTSTR pDir)
{_STTEX();
	DWORD	i = 0;
	char	path[ CWF_STRSIZE ];

	// Sanity check
	if ( pDir == NULL || *pDir == NULL )
		return FALSE;
	
	// Does it already exist
	if ( DoesExist( pDir ) ) return TRUE;

	while ( *pDir != NULL && i < CWF_STRSIZE )
	{
		// Copy in path delimiters
		while( *pDir == '\\' || *pDir == '/' )
		{
			path[ i++ ] = *pDir;
			pDir++;
		} // end while		
		
		// Copy over a piece of the directory
		while(	*pDir != '\\' && 
				*pDir != '/' &&
				*pDir != NULL )
		{
			path[ i++ ] = *pDir;
			pDir++;
		} // end while		
		path[ i ] = NULL;
		
		// Don't try to create a drive
		if (	path[ strlen( path ) - 1 ] != ':' &&
				path[ strlen( path ) - 1 ] != '/' &&
				path[ strlen( path ) - 1 ] != '\\' )
		{		
			// Attempt to create this directory
			if ( !::CreateDirectory( path, NULL ) )

				// Don't worry about it if it's already there
				if ( GetLastError() != ERROR_ALREADY_EXISTS )
					return FALSE;

		} // end if

	} // end while

	// I guess we made it
	return TRUE;
}

// Deletes a directory - all files and sub-directories
// If bEmptyOnly is TRUE the directory pDir is not removed
BOOL CWinFile::DeleteDirectory( LPCTSTR pDir, BOOL bEmptyOnly, BOOL bSubDirectories, BOOL bSubDirectoriesAndFilesOnly )
{_STTEX();
	DWORD				dwError = 0;
	WIN32_FIND_DATA		fd;
	char				search[ CWF_STRSIZE ];
	HANDLE				hFind;

	BuildPath( search, pDir, "*.*" );

	// Find the first file in this directory
	hFind = FindFirstFile( search, &fd );

	// Did we get anything?
	if ( hFind == INVALID_HANDLE_VALUE ) 
	{
		// Just remove the directory
		return RemoveDirectory( pDir );
	} // end if

	do
	{
		// Watch out for the '.' and '..'
		if (	*fd.cFileName != NULL &&
				strcmp( fd.cFileName, "." ) &&
				strcmp( fd.cFileName, ".." ) )
		{
			// Build a full path to the file or directory
			char	path[ CWF_STRSIZE ];
			BuildPath( path, pDir, fd.cFileName );

			// Recurse if it is a directory
			if ( 	bSubDirectories &&
					fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				DeleteDirectory( path, bEmptyOnly );

			// Just delete it if file
			else if ( !bEmptyOnly )
			{
				// Clear all file attributes and delete it
				SetFileAttributes( path, FILE_ATTRIBUTE_NORMAL );
				Delete( path );

				// Save error if any
				DWORD res = GetLastError();
				if ( res && dwError == 0 ) dwError = res;

			} // end else

		} // end if

	// Get the next file or directory
	} while ( FindNextFile( hFind, &fd ) );

	// Close the find handle
	FindClose( hFind );

	// Just spring cleaning?
	if ( bSubDirectoriesAndFilesOnly )
	{
		// Clue the caller in on the errors we think are important
		SetLastError( dwError );

		return TRUE;
	} // end if

	// Now that it is empty we should be able to remove it
	if ( !RemoveDirectory( pDir ) )
		if ( dwError == 0 ) dwError = GetLastError();

	// Clue the caller in on the errors we think are important
	SetLastError( dwError );

	// Well, did we succeed?
	return ( dwError == 0 );
}


char CWinFile::m_szTempPath[ CWF_STRSIZE ] = { 0 };

BOOL CWinFile::SetTempPath(LPCTSTR pPath)
{_STTEX();
	if ( pPath != NULL && *pPath != 0x0 )
	{
		if ( !CreateDirectory( pPath ) ) return FALSE;
		strcpy( m_szTempPath, pPath );
		return TRUE;
	} // end if

	// Make sure the path is empty
	*m_szTempPath = 0x0;

	if ( !CreateTempDir( m_szTempPath ) )
	{
		*m_szTempPath = 0x0;
		return FALSE;
	} // end if

	return TRUE;
}

BOOL CWinFile::DeleteTempPath()
{_STTEX();
	if ( !IsTempPath() ) return TRUE;

	// Is there anything to delete?
	if ( !DoesExist( m_szTempPath ) ) return TRUE;

	BOOL ret = DeleteDirectory( m_szTempPath );

	// The path is no longer valid if we removed it
	if ( ret ) *m_szTempPath = 0x0;

	return ret;
}

BOOL CWinFile::CleanTempPath()
{_STTEX();
	if ( !IsTempPath() ) return FALSE;

	// Just delete all files and sub-directories
	return DeleteDirectory( m_szTempPath, TRUE );
}

BOOL CWinFile::CreateTempDir(LPSTR pDir)
{_STTEX();
	char	path[ CWF_STRSIZE ];

	if ( pDir == NULL ) return FALSE;

	if ( !CreateTemp( path ) ) return FALSE;
	Delete( path );

	// Remove the extention for Win95
	for ( long x = strlen( path ); x > 1; x-- )
		if ( path[ x ] == '.' ) 
		{
			path[ x ] = 0x0;
			x = 1;
		} // end if

	// Attemp to create this directory
	if ( !CreateDirectory( path ) ) return FALSE;

	strcpy( pDir, path );

	return TRUE;
}

BOOL CWinFile::VerboseCleanup( HWND hWnd, DWORD dwAttempts )
{_STTEX();
	DWORD attempts = 0;

	while ( !CWinFile::DeleteTempPath() )
	{
		DWORD err = GetLastError();

		// Are we flogging a dead horse?
		if (	err == ERROR_PATH_NOT_FOUND ||
				err == ERROR_FILE_NOT_FOUND ) return TRUE;

		attempts++;
		if ( attempts > dwAttempts )
		{
			char msg[ 1024 ];

			attempts = 0;

			strcpy( msg, "Unable to delete the temporary directory\r\n\r\n" ),
				strcat( msg, CWinFile::GetTempPath() );
			strcat( msg,	"\r\n\r\nIf you choose to cancel, you will need to remove this\r\n"
							"directory yourself" );

			if ( ShowLastError( "Cleanup", msg, MB_RETRYCANCEL ) != IDRETRY )
				return FALSE;

		} // end if
		else Sleep( 100 );

	} // end while

	return TRUE;
}


#define	SIZE_KB		((double)1024)
#define	SIZE_MB		((double)1048567)
#define	SIZE_GB		((double)1073732608)
#define	SIZE_TB		((double)1099502190592)

BOOL CWinFile::CreateSizeString( LPSTR pString, double size )
{_STTEX();

	if ( pString == NULL ) return FALSE;

	if ( size < SIZE_KB )
	{
		DWORD prnt = (DWORD)size;
		sprintf( pString, "%lu Bytes", (ULONG)size );

	} // end if
	else if ( size < SIZE_MB )
	{
		size /= SIZE_KB;
		sprintf( pString, "%.2f KB", size );
	} // end if
	else if ( size < SIZE_GB )
	{
		size /= SIZE_MB;
		sprintf( pString, "%.2f MB", size );
	} // end if
	else if ( size < SIZE_TB )
	{
		size /= SIZE_GB;
		sprintf( pString, "%.2f GB", size );
	} // end if
	else
	{
		size /= SIZE_TB;
		sprintf( pString, "%.2f TB", size );
	} // end if

	return TRUE;
}

BOOL CWinFile::GetDiskSpace( LPCTSTR pDrive, double *total, double *free )
{_STTEX();
	char drv[ 4 ] = "C:\\";

	if ( pDrive == NULL ) return FALSE;

	// Get a path to the drive
	*drv = *pDrive;

/*	// Get operating system info
	OSVERSIONINFO  osVersion ;
	osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
	if (	!::GetVersionEx( &osVersion ) ||
			osVersion.dwBuildNumber < 1000 )
*/	{
		// This version doesn't work on large drives
		// Hopefully since they have a crappy machine
		// They also have a small hard-drive


		DWORD dwSectorsPerCluster;
		DWORD dwBytesPerSector;
		DWORD dwNumberOfFreeClusters;
		DWORD dwTotalNumberOfClusters;

		// Attempt to get disk information
		if ( !GetDiskFreeSpace(	drv, 
									&dwSectorsPerCluster,
									&dwBytesPerSector,
									&dwNumberOfFreeClusters,
									&dwTotalNumberOfClusters ) )
			return FALSE;

		if ( total != NULL )
			*total =	double( dwBytesPerSector ) *
						double( dwSectorsPerCluster ) * 
						double( dwTotalNumberOfClusters );

		if ( free != NULL )
			*free =	double( dwBytesPerSector ) *
					double( dwSectorsPerCluster ) * 
					double( dwNumberOfFreeClusters );
	} // end if

	// Most machines will go here
/*	else
	{
		ULARGE_INTEGER qwFree;
		ULARGE_INTEGER qwTotal;
		ULARGE_INTEGER qwTotalFree;

		// Get information
		if ( !GetDiskFreeSpaceEx( drv, &qwFree, &qwTotal, &qwTotalFree ) )
			return FALSE;

		if ( total != NULL )
		{
			*total = qwTotal.HighPart;
			*total *= double( 0x100000000 );
			*total += qwTotal.LowPart;
		} // end if
		if ( free != NULL )
		{
			*free = qwFree.HighPart;
			*free *= double( 0x100000000 );
			*free += qwFree.LowPart;
		} // end if
	} // end else
*/
	return TRUE;
}

BOOL CWinFile::GetSystemErrorMsg(DWORD err, LPSTR pMsg, LPCTSTR pTemplate)
{_STTEX();
	char msg[ CWF_STRSIZE ];

	if ( pMsg == NULL ) return FALSE;
	*pMsg = 0x0;

	// What does windows say this error is?
	if ( !FormatMessage(	FORMAT_MESSAGE_FROM_SYSTEM,
							NULL, err, 0, msg, CWF_STRSIZE, NULL ) )
		CWin32::GetWSAErrorMsg( msg, err );

	wsprintf( pMsg, pTemplate, err, msg );

	return TRUE;
}

int CWinFile::ShowLastError(LPCTSTR pTitle, LPCTSTR pAdd, UINT type)
{_STTEX();
	char msg[ 1024 ] = { 0 };

	GetSystemErrorMsg( GetLastError(), msg );

	if ( pAdd != NULL )
	{
		strcat( msg, "\r\n\r\n" );
		strcat( msg, pAdd );
	} // end if

	return MessageBox( NULL, msg, pTitle, type );

}

BOOL CWinFile::GetSpecialFolder(int nID, LPSTR buf)
{_STTEX();
	// Sanity check
	if ( buf == NULL ) return FALSE;

	if ( nID == CSIDL_USER_NONE )
	{
		*buf = 0x0;
		return TRUE;
	} // end if

	// If they want the system directory
	if ( nID == CSIDL_USER_SYSTEM )
		return ( ::GetSystemDirectory( buf, CWF_STRSIZE ) != 0x0 );

	// If they want the Windows directory
	else if ( nID == CSIDL_USER_WINDOWS )
		return ( ::GetWindowsDirectory( buf, CWF_STRSIZE ) != 0x0 );

	// If they want the Temprorary directory
	else if ( nID == CSIDL_USER_TEMP )
		return ( ::GetTempPath( CWF_STRSIZE , buf ) != 0x0 );

	// If they want the Current directory
	else if ( nID == CSIDL_USER_CURRENT )
		return ( ::GetCurrentDirectory( CWF_STRSIZE , buf ) != 0x0 );

	// If they want the Current directory
	else if ( nID == CSIDL_USER_DEFDRIVE )
	{
		if ( ::GetWindowsDirectory( buf, CWF_STRSIZE ) == 0x0 )
			return FALSE;
		buf[ 3 ] = 0x0;
		return TRUE;
	} // end else if

	// Check for invalid
	else if ( nID == -1 ) return FALSE;

	// Ask windows what they want
	else
	{	
		LPMALLOC		pMalloc;
		LPITEMIDLIST	pidl;

		if ( SHGetSpecialFolderLocation( NULL, nID, &pidl ) != NOERROR )
			return FALSE;
	
		// Get the path name
		BOOL ret = SHGetPathFromIDList( pidl, buf );

		// Free the memory
		if ( SHGetMalloc( &pMalloc ) == NOERROR )
			pMalloc->Free( pidl );

		return ret;

	} // end else

	// !!??
	return FALSE;
}

int CALLBACK CWinFile::BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{_STTEX();
	char szDir[ MAX_PATH ];
	
	// Pain, pain, pain

	// Set initial directory
	if ( uMsg == BFFM_INITIALIZED && lpData != NULL )
		SendMessage( hwnd, BFFM_SETSELECTION, TRUE, lpData ); 

	// Set status text
	else if ( uMsg == BFFM_SELCHANGED )
      if (SHGetPathFromIDList((LPITEMIDLIST) lParam ,szDir))
         SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);

	return 0;
}

BOOL CWinFile::GetBrowseDir( HWND hWnd, LPCTSTR pTitle, LPSTR pDir, LPCTSTR pInit )
{_STTEX();
	BROWSEINFO	bi;
	char	name[ MAX_PATH ];

	if ( pDir == NULL ) return FALSE;

	// Fill in browse info
	ZeroMemory( &bi, sizeof( bi ) );
	bi.hwndOwner = hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = name;
	bi.lpszTitle = pTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;

	// What a pain
	if ( pInit != NULL )
	{	bi.lpfn = &BrowseCallbackProc;
		bi.lParam = (LPARAM)pInit;
	} // end if
	else
	{	bi.lpfn = NULL;
		bi.lParam = NULL;
	} // end else
	bi.iImage = 0;
			
	LPITEMIDLIST pidlBrowse = SHBrowseForFolder( &bi );
	if ( pidlBrowse == NULL ) return FALSE;

	{ // Get Path
		LPMALLOC	pMalloc;

		// Get the path name
		SHGetPathFromIDList( pidlBrowse, pDir );

		// Free the memory
		if ( SHGetMalloc( &pMalloc ) == NOERROR )
			pMalloc->Free( pidlBrowse );

	} // end Get Path

	return TRUE;
}

LPCTSTR CWinFile::GetExtension(LPCTSTR pFile)
{_STTEX();
	LPCTSTR	ptr = NULL;

	if ( pFile == NULL ) return NULL;

	DWORD i;
	for ( i = 0; pFile[ i ] != 0x0; i++ )
	{
		if ( pFile[ i ] == '.' ) ptr = &pFile[ i + 1 ];
		else if ( pFile[ i ] == '\\' ) ptr = NULL;
		else if ( pFile[ i ] == '/' ) ptr = NULL;

	} // end for
	
	if ( ptr == NULL ) ptr = &pFile[ i ];

	return ptr;
}

BOOL CWinFile::DoesExist(LPCTSTR pFilename)
{_STTEX();
	WIN32_FIND_DATA	fd;
	HANDLE hFind = FindFirstFile( pFilename, &fd );
	if ( hFind == INVALID_HANDLE_VALUE ) return FALSE;
	FindClose( hFind );
	return TRUE;
}

BOOL CWinFile::WriteCache(LPVOID pBuffer, DWORD dwLength, LPDWORD pdwWritten)
{_STTEX();
	if ( !IsOpen() ) return FALSE;
	if ( !m_bEnableCache ) return FALSE;

	if ( m_pucWrCache == NULL )
	{
		m_dwWrCachePtr = 0;
		m_pucWrCache = new BYTE[ m_dwWrCacheSize ];
		if ( m_pucWrCache == NULL ) return FALSE;
	} // end if

	// Is there still room in the cache
	if ( pBuffer != NULL && ( m_dwWrCachePtr + dwLength ) < m_dwWrCacheSize )
	{
		if ( dwLength == 1 ) m_pucWrCache[ m_dwWrCachePtr++ ] = *((BYTE*)pBuffer);
		else
		{
			memcpy( (LPVOID)&m_pucWrCache[ m_dwWrCachePtr ], (LPVOID)pBuffer, dwLength );
			m_dwWrCachePtr += dwLength;
		} // end else
	} // end if
	else 
	{
		DWORD dwBytesWritten = 0;

		// Write the cache to the disk
		if ( !Write( m_pucWrCache, m_dwWrCachePtr, &dwBytesWritten, TRUE ) ||
							dwBytesWritten != m_dwWrCachePtr ) return FALSE;
		m_dwWrCachePtr = 0;

		if ( pBuffer != NULL )
		{
			// Save the rest of the data in the cache
			if ( dwLength == 1 ) m_pucWrCache[ m_dwWrCachePtr++ ] = *((BYTE*)pBuffer);
			else
			{
				memcpy( (LPVOID)m_pucWrCache, (LPVOID)pBuffer, dwLength );
				m_dwWrCachePtr += dwLength;
			} // end else
		} // end if
	} // end else

	// Tell the user how it went if their interested
	if ( pdwWritten != NULL ) *pdwWritten = dwLength;

	return TRUE;
}

BOOL CWinFile::ReadCache(LPVOID pBuffer, DWORD dwLength, LPDWORD pdwRead)
{_STTEX();
	if ( !IsOpen() ) return FALSE;
	if ( !m_bEnableCache ) return FALSE;

	if ( m_pucRdCache == NULL )
	{
		m_dwRdCachePtr = 0;
		m_dwRdCacheBytes = 0;
		m_pucRdCache = new BYTE[ m_dwRdCacheSize ];
		if ( m_pucRdCache == NULL ) return FALSE;
	} // end if

	// Are there enough bytes left in the cache
	if ( ( m_dwRdCacheBytes - m_dwRdCachePtr ) >= dwLength )
	{
		if ( dwLength == 1 ) *((BYTE*)pBuffer) = m_pucRdCache[ m_dwRdCachePtr++ ];
		else
		{
			memcpy( (LPVOID)pBuffer, (LPVOID)&m_pucRdCache[ m_dwRdCachePtr ], dwLength );
			m_dwRdCachePtr += dwLength;
		} // end else
	} // end if
	else
	{
		DWORD dwBytesRead = 0;

		// Adjust CacheSize
		m_dwRdCacheBytes -= m_dwRdCachePtr;

		// Move bytes back in the buffer
		if ( m_dwRdCacheBytes )
			memmove(	(LPVOID)m_pucRdCache, 
						&m_pucRdCache[ m_dwRdCachePtr ],
						m_dwRdCacheBytes );
		m_dwRdCachePtr = 0;

		// Read some more bytes
		if ( !Read( &m_pucRdCache[ m_dwRdCacheBytes ], 
					m_dwRdCacheSize - m_dwRdCacheBytes,
					&dwBytesRead,
					TRUE ) ) return FALSE;

		// Add the bytes we read
		m_dwRdCacheBytes += dwBytesRead;

		// Let's try it again
		if ( m_dwRdCacheBytes > dwLength )
		{
			if ( dwLength == 1 ) *((BYTE*)pBuffer) = m_pucRdCache[ m_dwRdCachePtr++ ];
			else
			{
				memcpy( (LPVOID)pBuffer, (LPVOID)m_pucRdCache, dwLength );
				m_dwRdCachePtr += dwLength;
			} // end else
		} // end if
		else return FALSE;

	} // end else

	return TRUE;

}

void CWinFile::FlushReadCache()
{_STTEX();
	if ( m_pucRdCache == NULL ) return;
	delete [] m_pucRdCache;
	m_pucRdCache = NULL;
	m_dwRdCachePtr = 0;
	m_dwRdCacheBytes = 0;
}

void CWinFile::FlushWriteCache()
{_STTEX();
	if ( m_pucWrCache == NULL ) return;

	// Flush the data
	WriteCache( NULL, 0, NULL );

	delete [] m_pucWrCache;
	m_pucWrCache = NULL;
	m_dwWrCachePtr = 0;
}

#define DO1(buf) crc = crc_table[ ( crc ^ ( *buf++ ) ) & 0xff ] ^ ( crc >> 8 );
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);


DWORD CWinFile::CRC32(DWORD crc, LPBYTE buf, DWORD size)
{_STTEX();

#ifdef DYNAMIC_CRC_TABLE
	if ( crc_table == NULL ) 
	{
		CreateCRC32Table();
		if ( crc_table == NULL ) return crc;
	} // end if

#endif
	
	crc = crc ^ 0xffffffffL; 
	while ( size >= 8 ) 
	{
		DO8 ( buf ); 
		size -= 8; 		
	} 
	if ( size ) do 
	{
		DO1 ( buf ); 		
	} while ( --size ); 

	return ( crc ^ 0xffffffffL ); 
}

BOOL CWinFile::AddCRC32(LPCTSTR pFile)
{_STTEX();
	CWinFile	file;
	BYTE		buf[ 2048 ];
	DWORD		read;
	DWORD		crc = 0;

	// Open the file
	if ( !file.OpenExisting( pFile ) )
		return FALSE;

	// Read a block from the file
	while ( file.Read( buf, 2048, &read ) && read > 0 )
	{
		// Calculate CRC for this block
		crc = CRC32( crc, buf, read );
	} // end while

	// Tack the CRC onto the end
	if ( !file.Write( &crc, sizeof( DWORD ) ) )
		return FALSE;

	return TRUE;

}

BOOL CWinFile::ValidateCRC32(LPCTSTR pFile, LPDWORD pCrc )
{_STTEX();
	CWinFile	file;
	BYTE		buf[ 2048 ];
	DWORD		crc = 0;
	DWORD		size;
	DWORD		i = 0;

	// Open the file
	if ( !file.OpenExisting( pFile, GENERIC_READ ) )
		return FALSE;

	// Get the file size
	size = file.Size();
	size -= sizeof( DWORD );

	// Read a block from the file
	while ( i < size )
	{
		DWORD read;
		DWORD want = 2048;
		if ( want > ( size - i ) ) want = size - i;

		// Read a few bytes
		if ( !file.Read( buf, want, &read ) || read == 0 ) return FALSE;

		// Calculate CRC for this block
		crc = CRC32( crc, buf, read );

		// Keep track of position
		i += read;

	} // end while

	DWORD filecrc;
	if ( !file.Read( &filecrc, sizeof( DWORD ) ) ) return FALSE;

	// Save crc
	if ( pCrc != NULL ) *pCrc = crc;

	// How did it come out?
	return ( crc == filecrc );
}

BOOL CWinFile::ValidateModuleCRC32(HMODULE hModule, LPDWORD pCrc )
{_STTEX();
	char exe[ CWF_STRSIZE ];

	// Get module file name
	if ( !GetModuleFileName( hModule, exe, CWF_STRSIZE - 1 ) )
		return FALSE;

	// Validate the CRC
	if ( !ValidateCRC32( exe, pCrc ) )
		return FALSE;

	return TRUE;
}

#ifdef DYNAMIC_CRC_TABLE

LPDWORD CWinFile::crc_table;

void CWinFile::CreateCRC32Table()
{_STTEX();
	DWORD c; 
	int n, k; 
	DWORD poly; 

	if ( crc_table == NULL )
	{
		crc_table = new DWORD[ 256 ];
		if ( crc_table == NULL ) return;
	} // end if

	// polynomial exclusive-or pattern
	// terms of polynomial defining this crc (except x^32):
	static const BYTE p[ ] = 
	{
		0, 1, 2, 4, 5, 7, 8, 10, 11, 12, 16, 22, 23, 26 
	}; 
	
	// make exclusive-or pattern from polynomial (0xedb88320L)
	poly = 0L; 
	for ( n = 0; n < sizeof ( p ) / sizeof ( BYTE ); n ++ ) poly |= 1L << ( 31 - p[ n ] ); 
	for ( n = 0; n < 256; n ++ ) 
	{
		c = ( DWORD ) n; 
		for ( k = 0; k < 8; k ++ ) c = c & 1 ? poly ^ ( c >> 1 ) : c >> 1; 
		crc_table[ n ] = c; 
		
	} 
}

#else

DWORD CWinFile::crc_table[ 256 ] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

#endif

BOOL CWinFile::LoadResource( LPCTSTR pResource, LPBYTE pPtr, LPDWORD pdwSize, LPCTSTR pType, HMODULE hModule )
{_STTEX();
	if ( pResource == NULL ) return FALSE;

	// Find the resource
	HRSRC hRsrc = FindResource( hModule, pResource, pType );
	if ( hRsrc == NULL ) return FALSE;

	// Save the size
	DWORD dwSize = SizeofResource( hModule, hRsrc );

	// Is this all they wanted to know
	if ( pPtr == NULL )
	{	if ( pdwSize != NULL ) *pdwSize = dwSize;
		return TRUE;
	} // end if

	// Do we want to terminate?
	BOOL bNullTerminate = ( pdwSize != NULL && *pdwSize > dwSize );
	if ( pdwSize == NULL ) bNullTerminate = TRUE;

	// Save actual size
	if ( pdwSize != NULL ) *pdwSize = dwSize;		

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

	// Terminate if needed
	if ( bNullTerminate ) pPtr[ dwSize ] = 0;

	return TRUE;
}

BOOL CWinFile::ExtractResource(LPCTSTR pFile, LPCTSTR pResource, LPCTSTR pType, HMODULE hModule, BOOL bOverwrite )
{_STTEX();
	// Do we want to overwrite?
	if ( !bOverwrite && DoesExist( pFile ) ) return FALSE;

	TMem< BYTE >	buf;
	DWORD size = 0;
	
	// Get the size
	if ( !LoadResource( pResource, NULL, &size, pType, hModule ) )
		return FALSE;

	// Allocate memory
	if ( size == 0 ) return FALSE;
	if ( !buf.allocate( size ) ) return FALSE;

	// Extract the resource
	if ( !LoadResource( pResource, buf, &size, pType, hModule ) )
		return FALSE;

	CWinFile file;

	// Create new file
	if ( !file.OpenNew( pFile, GENERIC_WRITE ) )
		return FALSE;

	// Write the data to the file
	file.Write( buf, size );
	file.Close();

	return TRUE;

}


COLORREF	CWinFile::m_rgbCustomColors[ 16 ] = {	RGB( 0, 0, 0 ),
													RGB( 255, 255, 255 ),
													RGB( 255, 0, 0 ),
													RGB( 0, 255, 0 ),
													RGB( 0, 0, 255 ),
													RGB( 255, 255, 0 ),
													RGB( 0, 255, 255 ),
													RGB( 255, 0, 255 ),
													RGB( 128, 128, 128 ),
													RGB( 200, 200, 200 ),
													RGB( 128, 0, 0 ),
													RGB( 0, 128, 0 ),
													RGB( 0, 0, 128 ),
													RGB( 128, 128, 0 ),
													RGB( 0, 128, 128 ),
													RGB( 128, 0, 128 ) };

BOOL CWinFile::ChooseColor(COLORREF * pCol, HWND hOwner, DWORD dwFlags)
{_STTEX();
	CHOOSECOLOR	cc;

	if ( pCol == NULL ) return FALSE;

	// Set up the color info
	ZeroMemory( &cc, sizeof( CHOOSECOLOR ) );
	cc.lStructSize = sizeof( CHOOSECOLOR );
	cc.hwndOwner = hOwner;
	cc.rgbResult = *pCol;
	cc.lpCustColors = m_rgbCustomColors;
	cc.Flags = dwFlags;

	// Get the users color
	if ( !::ChooseColor( &cc ) ) return FALSE;

	// Save the resulting color
	*pCol = cc.rgbResult;

	return TRUE;	
}
#define STR_COMMON	"Common "
#define STR_USER	"Current User "
#define STR_WIN		""
#define CSIDL_CASE( id, type, str, f )			\
	else if ( nID == id || dwIndex == index++ )	\
	{											\
		strcpy( pName, type str );				\
		flags |= f;								\
		if ( pnID != NULL ) *pnID = id;			\
	}

BOOL CWinFile::GetSpecialFolderName(int nID, LPSTR pName, LPDWORD pdwFlags, DWORD dwIndex, int *pnID )
{_STTEX();
	DWORD	flags = 0;
	DWORD	index = 1;

	if ( nID == -1 && dwIndex == 0 ) return FALSE;

	CSIDL_CASE( CSIDL_USER_SYSTEM, STR_WIN, "*System Files", 0 )
	CSIDL_CASE( CSIDL_USER_WINDOWS, STR_WIN, "*Windows", 0 )
	CSIDL_CASE( CSIDL_USER_TEMP, STR_WIN, "*Temporary", 0 )
	CSIDL_CASE( CSIDL_USER_CURRENT, STR_WIN, "*Current", 0 )
	CSIDL_CASE( CSIDL_USER_DEFDRIVE, STR_WIN, "*Default Drive", 0 )
//	CSIDL_CASE( CSIDL_, STR_, "", 0 )

		
//	CSIDL_CASE( CSIDL_ALTSTARTUP, STR_USER, "Non-local Startup", 0 )
	CSIDL_CASE( CSIDL_APPDATA, STR_USER, "Application Data", 0 )
	CSIDL_CASE( CSIDL_BITBUCKET, STR_USER, "Recycle Bin", CSIDL_FLAG_VIRTUAL )
//	CSIDL_CASE( CSIDL_COMMON_ALTSTARTUP, STR_COMMON, "Non-local Startup", 0 )
//	CSIDL_CASE( CSIDL_COMMON_APPDATA, STR_COMMON, "Application Data", 0 )
	CSIDL_CASE( CSIDL_COMMON_DESKTOPDIRECTORY, STR_COMMON, "Desktop", 0 )
//	CSIDL_CASE( CSIDL_COMMON_FAVORITES, STR_COMMON, "Favorites", 0 )
	CSIDL_CASE( CSIDL_COMMON_PROGRAMS, STR_COMMON, "Start->Programs", 0 )
	CSIDL_CASE( CSIDL_COMMON_STARTMENU, STR_COMMON, "Start Menu", 0 )
	CSIDL_CASE( CSIDL_COMMON_STARTUP, STR_COMMON, "Startup", 0 )
	CSIDL_CASE( CSIDL_CONTROLS, STR_WIN, "Contorl Panel Applications", CSIDL_FLAG_VIRTUAL )
//	CSIDL_CASE( CSIDL_COOKIES, STR_WIN, "Internet Cookies", 0 )
	CSIDL_CASE( CSIDL_DESKTOP, STR_WIN, "Desktop", CSIDL_FLAG_VIRTUAL )
	CSIDL_CASE( CSIDL_DESKTOPDIRECTORY, STR_WIN, "Desktop Directory", 0 )
	CSIDL_CASE( CSIDL_DRIVES, STR_WIN, "My Computer", CSIDL_FLAG_VIRTUAL )
	CSIDL_CASE( CSIDL_FAVORITES, STR_USER, "Favorites", 0 )
	CSIDL_CASE( CSIDL_FONTS, STR_WIN, "Fonts", 0 )
//	CSIDL_CASE( CSIDL_HISTORY, STR_WIN, "Internet History", 0 )
//	CSIDL_CASE( CSIDL_INTERNET, STR_WIN, "Internet", CSIDL_FLAG_VIRTUAL )
//	CSIDL_CASE( CSIDL_INTERNET_CACHE, STR_WIN, "Internet Cache", 0 )
//	CSIDL_CASE( CSIDL_LOCAL_APPDATA, STR_WIN, "Local Application Data", 0 )
//	CSIDL_CASE( CSIDL_MYPICTURES, STR_WIN, "My Pictures", 0 )
	CSIDL_CASE( CSIDL_NETHOOD, STR_WIN, "Network Neighborhood", 0 )
	CSIDL_CASE( CSIDL_NETWORK, STR_WIN, "Network", CSIDL_FLAG_VIRTUAL )
	CSIDL_CASE( CSIDL_PERSONAL, STR_WIN, "Personal Documents", 0 )
	CSIDL_CASE( CSIDL_PRINTERS, STR_WIN, "Printers", CSIDL_FLAG_VIRTUAL )
	CSIDL_CASE( CSIDL_PRINTHOOD, STR_WIN, "Network Printers", 0 )
//	CSIDL_CASE( CSIDL_PROFILE, STR_USER, "Profile", 0 )
//	CSIDL_CASE( CSIDL_PROGRAM_FILES, STR_WIN, "Program Files", 0 )
//	CSIDL_CASE( CSIDL_PROGRAM_FILESX86, STR_WIN, "Program Files (x86)", 0 )
	CSIDL_CASE( CSIDL_PROGRAMS, STR_USER, "Programs", 0 )
	CSIDL_CASE( CSIDL_RECENT, STR_USER, "Recent Documents", 0 )
	CSIDL_CASE( CSIDL_SENDTO, STR_WIN, "\'Send To\' Items", 0 )
	CSIDL_CASE( CSIDL_STARTMENU, STR_WIN, "Start Menu Items", 0 )
	CSIDL_CASE( CSIDL_STARTUP, STR_USER, "Startup", 0 )
//	CSIDL_CASE( CSIDL_SYSTEM, STR_WIN, "System Files", 0 )
//	CSIDL_CASE( CSIDL_SYSTEMX86, STR_WIN, "System Files (x86)", 0 )
	CSIDL_CASE( CSIDL_TEMPLATES, STR_WIN, "Templates", 0 )
//	CSIDL_CASE( CSIDL_WINDOWS, STR_WIN, "Windows", 0 )

	else return FALSE;

	if ( pdwFlags != NULL ) *pdwFlags = TRUE;

	return TRUE;
}

BOOL CWinFile::DeletePath(LPCTSTR pPath, BOOL bNextReboot, BOOL bEmptyOnly )
{_STTEX();
	BOOL bSuccess = FALSE;

	// Get path attributes
	DWORD	attributes = GetFileAttributes( pPath );
	if ( attributes == 0xffffffff ) return FALSE;

	if ( ( attributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
		bSuccess = DeleteDirectory( pPath, bEmptyOnly );
	else bSuccess = Delete( pPath );

	if ( !bSuccess && bNextReboot )
	{
		// Get operating system info
		OSVERSIONINFO  osVersion ;
		osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
		if ( !::GetVersionEx( &osVersion ) )
			osVersion.dwPlatformId = VER_PLATFORM_WIN32_WINDOWS;

		// NT systems 
		if ( osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT ) 
			return MoveFileEx( pPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );

		// 95/98 systems
		else
		{
			char szWinInitFile[ CWF_STRSIZE ];
			char szShortPath[ CWF_STRSIZE ];
			
			// Get the windows init ini file name
			GetWindowsDirectory( szWinInitFile, CWF_STRSIZE );
			strcat( szWinInitFile, "\\WinInit.ini" );
			
			// Get the short path for the file
			GetShortPathName( pPath, szShortPath, CWF_STRSIZE );

			char buf[ 32767 ];

			// Get the current profile stuff
			DWORD len = GetPrivateProfileSection( "Rename", buf, sizeof( buf ), szWinInitFile );
			
			// Append this file
			strcpy( &buf[ len ], "NUL=" );
			strcat( &buf[ len ], szShortPath );

			// Add extra null terminator
			buf[ len + strlen( &buf[ len ] ) + 1 ] = NULL;

			// Write the changes back to the file
			WritePrivateProfileSection( "Rename", buf, szWinInitFile );

		} // end else

	} // end if

	return TRUE;
}

BOOL CWinFile::GetMultiSelectFileName(LPCTSTR pFilenames, DWORD dwMax, LPSTR pFile, DWORD dwIndex, BYTE ucSep)
{_STTEX();
	// Sanity checks
	if ( pFilenames == NULL || pFile == NULL || dwMax == 0 )
		return FALSE;

	BOOL	bPath = TRUE;
	DWORD	i = 0;
	DWORD	c = 0;
	char	path[ CWF_STRSIZE ];
	char	file[ CWF_STRSIZE ];

	// While there is data
	while ( pFilenames[ i ] != NULL )
	{
		// Read in the first string
		c = 0;
		while ( i < dwMax && c < CWF_STRSIZE && pFilenames[ i ] != ucSep && pFilenames[ i ] != NULL ) 
			file[ c++ ] = pFilenames[ i++ ];

		// Null terminate string
		if ( c < CWF_STRSIZE ) file[ c ] = NULL;

		// Are we reading the path?
		if ( bPath )
		{
			// Are there any more file names?
			if (	!dwIndex && 
					pFilenames[ i ] == ucSep &&
					pFilenames[ i + 1 ] == NULL )
			{
				strcpy( pFile, file );

				return TRUE;
			} // end if

			bPath = FALSE;
			strcpy( path, file );
	
		} // end if

		else
		{
			// is this our guy?
			if ( !dwIndex )
			{
				// Build a path from here
				BuildPath( pFile, path, file );

				return TRUE;
			} // end if

			// Count one
			else dwIndex--;

		} // end else
		
		// Did we stop for a separator?
		if ( pFilenames[ i ] != ucSep ) return FALSE;

		// Skip the separator
		i++; if ( i >= dwMax ) return FALSE;	
			
	} // end while

	return FALSE;
}


BOOL CWinFile::CreateLink(LPCTSTR pPath, LPCTSTR pLink, LPCTSTR pDesc)
{_STTEX();
	// Initialize the shell link
	HRESULT hInit = CoInitialize( NULL );
	if ( hInit != S_OK && hInit != S_FALSE ) return FALSE;

	BOOL			bSuccess = FALSE;
	IShellLink*		isl = NULL;
	IPersistFile*	ipf	= NULL;

	// Attempt to acquire a ShellLink interface
	if ( CoCreateInstance(	CLSID_ShellLink,			// Class id
							NULL,						// Aggregate
							CLSCTX_INPROC_SERVER,		// Context
							IID_IShellLink,				// Reference ID
							(LPVOID*)&isl ) != S_OK ||	// pointer
							isl == NULL )
		return FALSE;
	
	// Set link info
	isl->SetPath( pPath );
	isl->SetDescription( pDesc );
	isl->SetIconLocation( pPath, 0 );
	
	// Save the link
	if ( isl->QueryInterface( IID_IPersistFile, (LPVOID*)&ipf ) == S_OK && ipf != NULL )
	{
		WORD	wLink[ CWF_STRSIZE ];
		MultiByteToWideChar( CP_ACP, 0, pLink, -1, (LPWSTR)wLink, CWF_STRSIZE ); 
		ipf->Save( (LPCOLESTR)wLink, TRUE );
		ipf->Release();
		bSuccess = TRUE;
	} // end if

	isl->Release();

	if ( hInit == S_OK ) CoUninitialize();

	return bSuccess;
}

BOOL CWinFile::CleanChars(LPSTR pFile)
{_STTEX();
	if ( pFile == NULL ) return FALSE;

	for ( DWORD i = 0; pFile[ i ] != 0; i++ )
	{
		if (	pFile[ i ] == '\\' ||
				pFile[ i ] == '/' ) pFile[ i ] = '-';


		if ( pFile[ i ] == '*' ) pFile[ i ] = '-';

		else if ( pFile[ i ] == ':' ) pFile[ i ] = '-';

		else if ( pFile[ i ] == ';' ) pFile[ i ] = '-';

		else if ( pFile[ i ] == '\"' ) pFile[ i ] = '\'';

		else if ( pFile[ i ] == '?' ) pFile[ i ] = '_';

		else if ( pFile[ i ] == '|' ) pFile[ i ] = '!';

		else if ( pFile[ i ] == '<' ) pFile[ i ] = '[';

		else if ( pFile[ i ] == '>' ) pFile[ i ] = ']';

		else if ( pFile[ i ] < ' ' ) pFile[ i ] = '_';

		else if ( pFile[ i ] > '~' ) pFile[ i ] = '_';
		
		else if ( pFile[ i ] < ' ' ) pFile[ i ] = '_';

	} // end for

	return TRUE;
}

BOOL CWinFile::GetUniqueFileName(LPSTR pUnique, LPCTSTR pFile, BOOL bNoExt)
{_STTEX();
	// Sanity check
	if ( pUnique == NULL || pFile == NULL ) return FALSE;

	// Is it already unique?
	if ( !CWinFile::DoesExist( pFile ) )
	{	strcpy( pUnique, pFile ); return TRUE; }
	
	char unique[ CWF_STRSIZE ];
	char file[ CWF_STRSIZE ];
	char ext[ CWF_STRSIZE ];

	strcpy( file, pFile );
	char *pext = (LPSTR)CWinFile::GetExtension( file );

	// Extract the file extention
	if ( !bNoExt && pext != NULL && pext != file )
	{	strcpy( ext, pext );
		*(--pext) = NULL;
	} // endif
	else *ext = 0;

	// Start with original file name
	strcpy( unique, pFile );
	
	// Ensure unique file name
	DWORD max = 1;
	while ( max < 10000 && CWinFile::DoesExist( unique ) )
		wsprintf( unique, "%s (%lu).%s", file, max++, ext );

	// Let the user in on the good news
	strcpy( pUnique, unique );

	return TRUE;
}

#ifdef CWF_SUPPORTENCRYPTION

void CWinFile::CryptoSetPos(DWORD pos)
{_STTEX();
	// Is there a key?
	if ( *m_szKey == 0 ) 
	{	ZeroMemory( m_pHashBuf, sizeof( m_pHashBuf ) );
		return;
	} // end if

	DWORD i			= 0;
	DWORD keylen	= strlen( m_szKey );

	// Re init hash
	m_md5.Init();
	m_dwHashPtr = 0;

	// Skip data we don't need
	while( ( pos - i ) > sizeof( m_pHashBuf ) )
	{	m_md5.Transform( (LPBYTE)m_szKey, keylen );
		i += m_md5.GetHashLen();
	} // end while

	// Calculate hash pointer position
	m_dwHashPtr = i % sizeof( m_pHashBuf );

	// Fill in the hash
	for ( DWORD h = 0; h < sizeof( m_pHashBuf ); )
	{	m_md5.Transform( (LPBYTE)m_szKey, keylen );
		for ( DWORD c = 0; c < m_md5.GetHashLen(); c++ )
			m_pHashBuf[ h + c ] = m_md5.GetHash()[ c ];
		h += m_md5.GetHashLen();		
	} // end while
}

void CWinFile::CryptoRefillHashBuffer()
{_STTEX();
	m_dwHashPtr = 0;
	if ( *m_szKey == 0 ) return;

	DWORD keylen = strlen( m_szKey );

	// Fill in the hash
	for ( DWORD h = 0; h < sizeof( m_pHashBuf ); )
	{	m_md5.Transform( (LPBYTE)m_szKey, keylen );
		for ( DWORD c = 0; c < m_md5.GetHashLen(); c++ )
			m_pHashBuf[ h + c ] = m_md5.GetHash()[ c ];
		h += m_md5.GetHashLen();		
	} // end while
}

void CWinFile::CryptoDoHash(LPBYTE buf, DWORD size)
{_STTEX();
	if ( *m_szKey == 0 ) return;

	DWORD i = 0;

	while ( i < size )
	{
		// Need more hash?
		if ( m_dwHashPtr >= sizeof( m_pHashBuf ) )
			CryptoRefillHashBuffer();

		// DWORD hashes
		while ( ( size - i ) >= 4 && 
				( sizeof( m_pHashBuf ) - m_dwHashPtr ) >= 4 )
		{
			*(LPDWORD)( &buf[ i ] ) ^= *(LPDWORD)( &m_pHashBuf[ m_dwHashPtr ] );
			i += 4; m_dwHashPtr += 4;
		} // end while

		// Finish up
		while ( ( size - i ) > 0 && 
				( sizeof( m_pHashBuf ) - m_dwHashPtr ) > 0 )
			buf[ i++ ] ^= m_pHashBuf[ m_dwHashPtr++ ];

	} // end for
}

#endif


BOOL CWinFile::TokenFromTextString(LPCTSTR pText, LPSTR pToken, LPDWORD pdwNext, BOOL bUseDOSCommandRules )
{_STTEX();
	DWORD i = 0, x = 0;

	// Pick up where we left off
	if ( pdwNext != NULL ) i = *pdwNext;

	// Skip white space
	while ( pText[ i ] != 0 && ( pText[ i ] <= ' ' || pText[ i ] > '~' ) ) i++;

	// Any data left?
	if ( pText[ i ] == 0 ) return FALSE;

	// Look for quoted data path
	if ( pText[ i ] == '\"' )
	{
		i++;
		while ( ( pText[ i ] != '\"' || pText[ i - 1 ] == '\\' ) && pText[ i ] != 0 )
			pToken[ x++ ] = pText[ i++ ];
		pToken[ x ] = 0;

		// Does caller want to know where we left off?
		if ( pdwNext != NULL ) *pdwNext = i;

		return TRUE;
	} // end if

	if ( bUseDOSCommandRules )
	{
		// Accept spaces until we hit a file extention
		// I'm not really sure how DOS does this...
		BOOL bExt = FALSE;
		while ( pText[ i ] > ' ' && pText[ i ] <= '~' ||
				( pText[ i ] == ' ' && !bExt ) )
		{	if ( pText[ i ] == '.' ) bExt = TRUE;
			pToken[ x++ ] = pText[ i++ ];
		} // end while
	} // end if

	// Just copy until we hit white space
	else while ( pText[ i ] > ' ' && pText[ i ] <= '~' )
		pToken[ x++ ] = pText[ i++ ];

	// NULL Terminate
	pToken[ x ] = 0;

	// Does caller want to know where we left off?
	if ( pdwNext != NULL ) *pdwNext = i;

	return TRUE;
}

HICON CWinFile::GetIconForFile(LPCTSTR pFile, BOOL bLargeIcon)
{_STTEX();
	if ( pFile == NULL ) return LoadIcon( NULL, IDI_WINLOGO );

	SHFILEINFO	sfi;

	char filetype[ CWF_STRSIZE ];
	const char *ext = GetExtension( pFile );
	strcpy( filetype, "file." );
	if ( ext != NULL ) strcat( filetype, ext );


	// Get large icon for this file type
	if ( bLargeIcon )
	{
		if ( SHGetFileInfo(	pFile, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( sfi ),
							SHGFI_ICON | SHGFI_LARGEICON ) != 0 )
			return ( sfi.hIcon != NULL ) ? sfi.hIcon : LoadIcon( NULL, IDI_WINLOGO );

		if ( SHGetFileInfo(	filetype, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( sfi ),
							SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_LARGEICON ) != 0 )
			return ( sfi.hIcon != NULL ) ? sfi.hIcon : LoadIcon( NULL, IDI_WINLOGO );

	} // end if

	else
	{
		if ( SHGetFileInfo(	pFile, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( sfi ),
							SHGFI_ICON | SHGFI_SMALLICON ) != 0 )
			return ( sfi.hIcon != NULL ) ? sfi.hIcon : LoadIcon( NULL, IDI_WINLOGO );

		if ( SHGetFileInfo(	filetype, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( sfi ),
							SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_SMALLICON ) != 0 )
			return ( sfi.hIcon != NULL ) ? sfi.hIcon : LoadIcon( NULL, IDI_WINLOGO );

	} // end if

	return LoadIcon( NULL, IDI_WINLOGO );
}

LPCTSTR CWinFile::GetExePath(LPSTR buf, DWORD size)
{_STTEX();
	// Try to get path from exe	
	if ( GetModuleFileName( NULL, buf, size ) )
		CWinFile::GetPathFromFileName( buf, buf );

	// Fall back on current directory (I need something)
	else if ( GetCurrentDirectory( size, buf ) );

	// Give up
	else *buf = 0;

	return buf;
}

BOOL CWinFile::CrackUrl(LPCTSTR pUrl, LPURLINFO pui)
{_STTEX();
	// Sanity check
	if ( pUrl == NULL || pui == NULL ) return FALSE;

	// Init values
	*pui->szScheme = NULL;
	pui->dwScheme = 0;
	*pui->szHostName = NULL;
	pui->dwPort = 0;
	*pui->szUserName = NULL;
	*pui->szPassword = NULL;
	*pui->szUrlPath = NULL;
	*pui->szExtraInfo = NULL;

	pui->urlc.dwStructSize = sizeof( URL_COMPONENTS );
	pui->urlc.lpszScheme = pui->szScheme;
	pui->urlc.dwSchemeLength = sizeof( pui->szScheme );
	pui->urlc.lpszHostName = pui->szHostName;
	pui->urlc.dwHostNameLength = sizeof( pui->szHostName );
	pui->urlc.lpszUserName = pui->szUserName;
	pui->urlc.dwUserNameLength = sizeof( pui->szUserName );
	pui->urlc.lpszPassword = pui->szPassword;
	pui->urlc.dwPasswordLength = sizeof( pui->szPassword );
	pui->urlc.lpszUrlPath = pui->szUrlPath;
	pui->urlc.dwUrlPathLength = sizeof( pui->szUrlPath );
	pui->urlc.lpszExtraInfo = pui->szExtraInfo;
	pui->urlc.dwExtraInfoLength = sizeof( pui->szExtraInfo );

	// Let windows crack the url
	if ( !InternetCrackUrl( pUrl, strlen( pUrl ), ICU_DECODE | ICU_ESCAPE, &pui->urlc ) )
		return FALSE;

	// Move a little data
	pui->dwScheme = (DWORD)pui->urlc.nScheme;
	pui->dwPort = (DWORD)pui->urlc.nPort;

	return TRUE;
}

BOOL CWinFile::GetRootPath(DWORD dwFolders, LPCTSTR pPath, LPSTR pRoot)
{_STTEX();
	if ( pPath == NULL ) return FALSE;

	char path[ CWF_STRSIZE ];
	strcpy( path, pPath );

	// Search path
	for ( DWORD i = 0; path[ i ] != 0; i++ )
	{
		// Search for folder break
		if ( path[ i ] == '/' || path[ i ] == '\\' )
		{
			// Is this the one we want?
			if ( !dwFolders )
			{	path[ i ] = 0;
				if ( pRoot != NULL ) strcpy( pRoot, path );
				return TRUE;

			} // end if

			// Count one folder
			else dwFolders--;
		
		} // end if

	} // end for

	// Ran out of path
	return FALSE;
}

BOOL CWinFile::GetSubPath(DWORD dwFolders, LPCTSTR pPath, LPSTR pSub)
{_STTEX();
	if ( pPath == NULL ) return FALSE;

	char path[ CWF_STRSIZE ];
	strcpy( path, pPath );

	// Search path
	for ( DWORD i = 0; path[ i ] != 0; i++ )
	{
		// Search for folder break
		if ( path[ i ] == '/' || path[ i ] == '\\' )
		{
			// Is this the one we want?
			if ( !dwFolders )
			{	while ( path[ i ] == '/' || path[ i ] == '\\' ) i++;
				if ( pSub != NULL ) strcpy( pSub, &path[ i ] );
				return TRUE;
			} // end if

			// Count one folder
			else dwFolders--;
		
		} // end if

	} // end for

	// Ran out of path
	return FALSE;
}

BOOL CWinFile::GetSpecialText(DWORD i, LPSTR buf, DWORD size)
{_STTEX();
	return FALSE;

}

BOOL CWinFile::GetSpecialTextName(DWORD i, LPSTR buf, DWORD size)
{_STTEX();
	return FALSE;
}

BOOL CWinFile::ChangeExtension(LPSTR pFile, LPCTSTR pExt)
{_STTEX();
	// Sanity check
	if ( pFile == NULL || pExt == NULL ) return FALSE;

	char *ext = (char*)GetExtension( pFile );
	if ( ext == NULL ) return FALSE;

	// Are we removing the extension?
	if ( pExt == NULL || *pExt == 0 )
	{	*( --ext ) = 0; return TRUE; }

	// Add dot if none
	if ( *ext == 0 ) *ext = '.', *( ++ext ) = 0;

	// Copy the new extension
	strcpy( ext, pExt );

	return TRUE;
}



BOOL CWinFile::Quote(LPSTR dst, LPCTSTR src, char ch, char esc)
{_STTEX();
	// Sanity checks
	if ( dst == NULL ) return FALSE;
	if ( src == NULL ) src = dst;

	DWORD len = strlen( src );
	
	// Allocate memory in case the buffers are the same
	TMem< char > buf;
	if ( !buf.allocate( ( len * 2 ) + 4 ) ) return FALSE;

	DWORD i = 0;
	
	// Add quote
	buf[ i++ ] = ch;

	if ( esc != 0 )
	{
		DWORD x = 0;

		// Copy the string
		while ( src[ x ] != 0 )
		{	if ( src[ x ] == ch ) buf[ i++ ] = '\\';
			buf[ i++ ] = src[ x++ ];
		} // end while

	} // end if

	// Copy string
	else strcpy( &buf.ptr()[ 1 ], src ), i = len + 1;

	// Close quotes
	buf[ i++ ] = ch; buf[ i ] = 0;

	// Copy string to destination buffer
	strcpy( dst, buf );

	return TRUE;
}

BOOL CWinFile::Unquote(LPSTR dst, LPCTSTR src, char ch, char esc)
{_STTEX();
	// Sanity checks
	if ( dst == NULL ) return FALSE;
	if ( src == NULL ) src = dst;

	DWORD len = strlen( src );
	
	DWORD i = 0, x = 0; 
	
	if ( esc != 0 )
	{
		while ( src[ x ] != 0 )
		{	
			// Check for escape character
			if ( src[ x ] == esc && src[ x + 1 ] == ch ) 
			{	x++; dst[ i++ ] = src[ x ]; }

			// Else copy the character
			else if ( src[ x ] != ch ) dst[ i++ ] = src[ x ];

			// Next character
			x++;

		} // end while

	} // end if

	else
	{
		while ( src[ x ] != 0 )
		{	if ( src[ x ] != ch ) dst[ i++ ] = src[ x ]; x++; }

	} // end else

	// NULL terminate
	dst[ i ] = 0;

	return TRUE;
}


LPCTSTR CWinFile::GetPathElement(LPSTR pElement, LPCTSTR pPath, DWORD dwElement, LPDWORD pLeft, BOOL bReverse)
{_STTEX();
	DWORD i = 0, x = 0;

	// Punt if none
	if ( pElement == NULL || pPath == NULL ) return NULL;
	
	if ( !bReverse )
	{
		// Find specified element
		while ( dwElement && i < CWF_STRSIZE && pPath[ i ] != 0 )
		{
			// Is it the path delimiter?
			if ( pPath[ i ] == '\\' || pPath[ i ] == '/' )
			{	dwElement--; while ( pPath[ i ] == '\\' || pPath[ i ] == '/' ) i++; }

			// Next character
			else i++;

		} // end while

		// Any more paths?
		if ( i >= CWF_STRSIZE || pPath[ i ] == 0 ) return NULL;

		// Copy the element
		while ( i < CWF_STRSIZE && pPath[ i ] != 0 &&
				pPath[ i ] != '\\' && pPath[ i ] != '/' )
			pElement[ x++ ] = pPath[ i++ ];

		// Are there any left?
		if ( pLeft != NULL ) *pLeft = ( pPath[ i ] == '\\' || pPath[ i ] == '/' );

	} // end if

	else
	{
		// Get last element
		i = strlen( pPath ); 
		while ( i && ( pPath[ i ] == '\\' || pPath[ i ] == '/' ) ) i--; 
		if ( !i ) return FALSE;
		if ( i >= CWF_STRSIZE ) i = CWF_STRSIZE - 1;
		
		dwElement++;
		while ( i && dwElement ) 
		{
			// Is it the path delimiter?
			if ( i <= 1 || pPath[ i ] == '\\' || pPath[ i ] == '/' )
			{	
				// Count one element
				dwElement--; 

				// Do we need to keep going?
				if ( dwElement ) while ( i && ( pPath[ i ] == '\\' || pPath[ i ] == '/' ) ) i--; 
			}

			// Skip character
			else i--;

		} // end while

		// Count separator if any data left
		if ( dwElement == 1 && i == 0 && pPath[ i ] != '\\' && pPath[ i ] != '/' )
			dwElement--;

		// Did we find it?
		if ( !dwElement )
		{
			// Anything left?
			if ( pLeft != NULL ) *pLeft = i > 0;

			// Skip back to the data
			while ( i < CWF_STRSIZE && pPath[ i ] != 0 && 
					( pPath[ i ] == '\\' || pPath[ i ] == '/' ) ) i++; 

			// Copy the element
			while ( i < CWF_STRSIZE && pPath[ i ] != 0 &&
					pPath[ i ] != '\\' && pPath[ i ] != '/' )
				pElement[ x++ ] = pPath[ i++ ];

		} // end if
					

	} // end else

	// NULL Terminate
	pElement[ x ] = 0;

	return pElement;
}

BOOL CWinFile::CmpExt(LPCTSTR pFile, LPCTSTR pExt, DWORD len)
{_STTEX();
	if ( pFile == NULL ) return FALSE;

	// Get the extension
	LPCTSTR ext = GetExtension( pFile );

	// Checking for NULL extension?
	if ( pExt == NULL || *pExt == 0 )
	{	if ( ext == NULL || *ext == 0 ) return TRUE;
		return FALSE;
	} // end if

	// Compare file extension
	for ( DWORD i = 0; i < len; i++ )
	{
		// Check this character
		if ( ext[ i ] != pExt[ i ] ) return FALSE;

		// Is this the end?
		if ( ext[ i ] == 0 ) return TRUE;				

	} // end while

	// Close enough
	return TRUE;
}

BOOL CWinFile::IsFile(LPCTSTR pFile)
{_STTEX();
	DWORD attrib = GetFileAttributes( pFile );
	if ( attrib == MAXDWORD ) return FALSE;

	// Ensure it's not a directory
	if ( attrib & FILE_ATTRIBUTE_DIRECTORY ) return FALSE;

	return TRUE;
}

BOOL CWinFile::CopyFolder(LPCTSTR pSrc, LPCTSTR pDst, BOOL bSubFolders)
{_STTEX();
	WIN32_FIND_DATA		fd;
	char				search[ CWF_STRSIZE ];
	HANDLE				hFind;
										   
	// Ensure source directory
	if ( !DoesExist( pSrc ) ) return FALSE;

	// Create the destination folder
	if ( !CreateDirectory( pDst ) ) return FALSE;

	// Build search path
	BuildPath( search, pSrc, "*.*" );

	// Find the first file in this directory
	hFind = FindFirstFile( search, &fd );

	// Did we get anything?
	if ( hFind == INVALID_HANDLE_VALUE ) return FALSE;

	// File buffers
	char src[ CWF_STRSIZE ], dst[ CWF_STRSIZE ];

	do
	{
		// Watch out for the '.' and '..'
		if (	*fd.cFileName != 0 &&
				strcmp( fd.cFileName, "." ) &&
				strcmp( fd.cFileName, ".." ) )
		{
			// Build a full path to the file or directory
			BuildPath( src, pSrc, fd.cFileName );
			BuildPath( dst, pDst, fd.cFileName );

			// Recurse if it is a directory
			if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{	if ( bSubFolders ) CopyFolder( src, dst, TRUE ); }

			// Just copy it if file
			else ::CopyFile( src, dst, FALSE );

		} // end if

	// Get the next file or directory
	} while ( FindNextFile( hFind, &fd ) );

	// Close the find handle
	FindClose( hFind );

	return TRUE;
}

DWORD CWinFile::GetFileCount(LPCTSTR pPath, LPCTSTR pMask)
{_STTEX();
	// Verify directory
	DWORD fa = GetFileAttributes( pPath );
	if ( fa == MAXDWORD ) return 0;

	// Punt if not directory
	if ( ( fa & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
		return 1;

	char search[ MAX_PATH ];
	CWinFile::BuildPath( search, pPath, pMask );

	DWORD				count = 0;
	WIN32_FIND_DATA		fd;
	HANDLE				hFind;

	// Find first file
	hFind = FindFirstFile( search, &fd );
	if ( hFind == INVALID_HANDLE_VALUE ) return 0;

	do 
	{
		if (	strcmp( fd.cFileName, "." ) &&
				strcmp( fd.cFileName, ".." ) &&
				( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
			count++;
	
	} while ( FindNextFile( hFind, &fd ) );

	return count;
}

BOOL CWinFile::IsValidFileChar(BYTE ch)
{
	if ( ch <= ' ' || ch > '~' ) return FALSE;
	if (	ch == '*' || 
			ch == ':' || 
			ch == '\\' ||
			ch == '/' ||
			ch == '?' ||
			ch == '|' ||
			ch == '<' ||
			ch == '<' ||
			ch == '\"' ) return FALSE;

	return TRUE;
}

BOOL CWinFile::IsValidPathChar(BYTE ch)
{
	if ( ch <= ' ' || ch > '~' ) return FALSE;
	if (	ch == '*' || 
			ch == '|' ||
			ch == '<' ||
			ch == '>' ||
			ch == '\"' ||
			ch == '\'' ) return FALSE;

	return TRUE;
}

BOOL CWinFile::MatchPatterns(LPCTSTR pFile, LPCTSTR pPattern, char cSep, BOOL bAll)
{_STTEX();
	char szPattern[ CWF_STRSIZE ] = { 0 };
	strcpy_sz( szPattern, pPattern );

	BOOL bEnd = FALSE;
	DWORD dwStart = 0, dwStop = 0;
	while( !bEnd )
	{
		// Find the end of the first pattern
		while ( szPattern[ dwStop ] && szPattern[ dwStop ] != cSep ) dwStop++;

		// Did we reach the end
		if ( !szPattern[ dwStop ] ) bEnd = TRUE;

		// NULL terminate
		else szPattern[ dwStop ] = 0;

		// Attempt match on this pattern
		if ( MatchPattern( pFile, &szPattern[ dwStart ] ) )
		{	if ( !bAll ) return TRUE; }
		else if ( bAll ) return FALSE;

		// If there is more to go
		if ( !bEnd )
		{
			// Seek to next pattern
			dwStart = dwStop + 1;
			while ( szPattern[ dwStart ] && szPattern[ dwStart ] == cSep ) dwStart++;

			// Punt if no more patterns
			if ( !szPattern[ dwStart ] ) return FALSE;

			// Set the stop marker
			dwStop = dwStart;

		} // end if

	} // end while	

	return bAll;
}

BOOL CWinFile::MatchPattern(LPCTSTR pFile, LPCTSTR pPattern)
{_STTEX();
	if ( pFile == NULL || pPattern == NULL ) return FALSE;

	DWORD i = 0, p = 0;

	// Skip multiple '*'
	while ( pPattern[ p ] == '*' && pPattern[ p + 1 ] == '*' ) p++;

	// Check for the 'any' pattern
	if ( pPattern[ p ] == '*' && pPattern[ p + 1 ] == 0 )
		return TRUE;

	// While we're not at the end
	while ( pFile[ i ] != 0 )
	{
		// Are we on a wildcard?
		if ( pPattern[ p ] == '*' )
		{
			// Are we matching everything?
			if ( pPattern[ p + 1 ] == 0 ) return TRUE;

			// Check for pattern advance
			if (	pFile[ i ] == pPattern[ p + 1 ] ||

					(
						pFile[ i ] >= 'a' && pFile[ i ] <= 'z' && 
						( pFile[ i ] - ( 'a' - 'A' ) ) == pPattern[ p + 1 ] 
					) ||

					(
						pFile[ i ] >= 'A' && pFile[ i ] <= 'Z' && 
						( pFile[ i ] + ( 'a' - 'A' ) ) == pPattern[ p + 1 ] 
					) 

				) p += 2;
				
		} // end if

		// Just accept this character
		else if ( pPattern[ p ] == '?' ) p++;

		// Otherwise advance if equal
		else if ( pFile[ i ] == pPattern[ p ] ) p++;

		// Case insensitive
		else if (	(
						pFile[ i ] >= 'a' && pFile[ i ] <= 'z' && 
						( pFile[ i ] - ( 'a' - 'A' ) ) == pPattern[ p ] 
					) ||
					(
						pFile[ i ] >= 'A' && pFile[ i ] <= 'Z' && 
						( pFile[ i ] + ( 'a' - 'A' ) ) == pPattern[ p ] 
					) 
				) p++;

		// Back up in the pattern
		else while ( p && pPattern[ p ] != '*' ) p--;

		// Return true if we're at the end of the pattern
		if ( pPattern[ p ] == 0 ) return TRUE;

		// Next char
		i++;

	} // end while

	// Skip wild cards
	while ( pPattern[ p ] == '*' ) p++;

	// Did we match?
	return ( pPattern[ p ] == 0 );
}


BOOL CWinFile::GetCacheFileName(LPCTSTR pFilename, LPCTSTR pDir, LPSTR pCacheFilename)
{_STTEX();
	// Sanity checks
	if ( pFilename == NULL || pCacheFilename == NULL ) return FALSE;

	char fname[ CWF_STRSIZE ] = { 0 };
	strcpy_sz( fname, pFilename );

	char path[ CWF_STRSIZE ] = { 0 };
	if ( pDir != NULL ) strcpy_sz( path, pDir );

	// Create hash string for filename
	CMD5 md5;
	md5.Transform( fname ); md5.End();

	// Use base 64 to encode md5 hash
	CBase64 base64;
	base64.Encode( pCacheFilename, 32, md5.GetHash(), md5.GetHashLen() );

	// Remove junk characters
	CleanChars( pCacheFilename );

	// Add path if needed
	if ( *path != 0 ) CWinFile::BuildPath( pCacheFilename, path, pCacheFilename );

	// Add extension
	LPCTSTR pExt = GetExtension( fname );
	if ( pExt != NULL ) { strcat( pCacheFilename, "." ); strcat( pCacheFilename, pExt ); }

	return TRUE;

}
