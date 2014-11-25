/*******************************************************************
// Copyright (c) 2002, Robert Umbehant ( www.wheresjames.com )
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
// Comp2.cpp: implementation of the CComp2 class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#ifdef ENABLE_ZLIB

#include "..\zlib\ZLib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//#define BLOCKSIZE 1
//#define BLOCKSIZE	1024
//#define BLOCKSIZE	2048
//#define BLOCKSIZE	4096
#define BLOCKSIZE	8192
//#define BLOCKSIZE	16384
//#define BLOCKSIZE	32768

#define VERIFY_CRC
//#undef VERIFY_CRC

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CComp2::CComp2()
{_STT();
	m_dwCRC = 0;
	*m_szExpandFileName = NULL;
	*m_szFileID = NULL;
	m_dwExpandFileSize = 0;
	m_dwSourcePos = 0;
	m_dwSource = 0;
	m_pSource = NULL;
	m_dwDestPos = 0;
	m_dwDest = 0;
	m_pDest = NULL;
	m_bNoCloseSrc = FALSE;
	m_bNoCloseDst = FALSE;

	m_zs = new z_stream;

#ifdef CCOMP2_MULTITHREAD
	m_cti = NULL;
#endif
}

CComp2::~CComp2()
{_STT();
	Destroy();
	_PTR_DELETE( m_zs );
}


BOOL CComp2::SetSource(LPCTSTR pFile, HANDLE hFile, BOOL bSaveFileName, DWORD max)
{_STT();
	// Open the file
	if ( pFile == NULL ){ m_bNoCloseSrc = TRUE; m_fSource.Attach( hFile ); }
	else if ( !m_fSource.OpenExisting( pFile, GENERIC_READ ) )
		return FALSE;

	m_dwSourcePos = 0;
	m_pSource = NULL;
	m_dwSource = ( max != 0 ) ? max : m_fSource.Size();

	// Do we want to save the filename?
	if ( bSaveFileName )
	{
		ZeroMemory( m_szExpandFileName, sizeof( m_szExpandFileName ) );
		strcpy( m_szExpandFileName, m_fSource.GetPath() );
		CWinFile::GetFileNameFromPath( m_szExpandFileName, m_szExpandFileName );
	} // end if
	else *m_szExpandFileName = 0x0;

	return TRUE;
}

BOOL CComp2::SetDest(LPCTSTR pFile, HANDLE hFile )
{_STT();
	// Open the file
	if ( pFile == NULL ) { m_bNoCloseDst = TRUE; m_fDest.Attach( hFile ); }
	else if ( !m_fDest.OpenNew( pFile, GENERIC_WRITE ) )
		return FALSE;

	m_dwDestPos = 0;
	m_pDest = NULL;
	m_dwDest = 0;

	return TRUE;
}

void CComp2::Destroy()
{_STT();
	m_dwCRC = 0;
	*m_szExpandFileName = NULL;
	*m_szFileID = NULL;
	m_dwExpandFileSize = 0;
	m_dwSourcePos = 0;
	m_dwSource = 0;
	m_pSource = NULL;
	m_dwDestPos = 0;
	m_dwDest = 0;
	m_pDest = NULL;

	// Close or detach from files
	if ( m_bNoCloseSrc ) m_fSource.Detach();
	else m_fSource.Close();
	if ( m_bNoCloseDst ) m_fDest.Detach();
	else m_fDest.Close();

	m_bNoCloseSrc = FALSE;
	m_bNoCloseDst = FALSE;

#ifdef CCOMP2_MULTITHREAD
	if ( m_cti != NULL )
	{
		delete [] m_cti;
		m_cti = NULL;
	} // end if
#endif
}

BOOL CComp2::SetSource(LPBYTE buf, DWORD size)
{_STT();
	if ( m_bNoCloseSrc ) m_fSource.Detach();
	else m_fSource.Close();
	m_bNoCloseSrc = FALSE;

	if( buf == NULL || size == 0 )
	{
		m_pSource = NULL;
		m_dwSource = 0;
		return FALSE;
	} // end if

	m_dwSourcePos = 0;
	m_pSource = buf;
	m_dwSource = size;

	return TRUE;
}

BOOL CComp2::SetDest(LPBYTE buf, DWORD size)
{_STT();
	if ( m_bNoCloseDst ) m_fDest.Detach();
	else m_fDest.Close();
	m_bNoCloseDst = FALSE;

	if( buf == NULL || size == 0 )
	{
		m_pDest = NULL;
		m_dwDest = 0;
		return FALSE;
	} // end if

	m_dwDestPos = 0;
	m_pDest = buf;
	m_dwDest = size;
	
	return TRUE;
}

#ifdef CCOMP2_COMPRESS

BOOL CComp2::CompressInit()
{_STT();
	if ( !IsSource() || !IsDest() ) return FALSE;
	if ( !WriteHeader() ) return FALSE;

	ZeroMemory( m_zs, sizeof( z_stream ) );

	( (z_stream*)m_zs)->zalloc = (alloc_func)0;
	( (z_stream*)m_zs)->zfree = (free_func)0;
	( (z_stream*)m_zs)->opaque = (voidpf)0;
	
	// Init zlib
	if ( deflateInit( (z_stream*)m_zs, Z_BEST_COMPRESSION ) != Z_OK )
		return FALSE;

	return TRUE;
}

BOOL CComp2::CompressContinue(BOOL *pDone)
{_STT();
	if ( pDone != NULL ) *pDone = FALSE;
	if ( !IsSource() || !IsDest() ) return FALSE;

	// Are we done?
	if ( m_dwSourcePos >= m_dwSource )
		return FALSE;

	DWORD	read = 0; 
	char	in[ BLOCKSIZE ];
	char	out[ BLOCKSIZE ];

	// Read the data
	if ( !Read( in, BLOCKSIZE, &read ) || read == 0 )
		return FALSE;

	// Keep track of the crc
#ifdef VERIFY_CRC
	m_dwCRC = CWinFile::CRC32( m_dwCRC, (LPBYTE)in, read );
#endif

	( (z_stream*)m_zs)->next_in = (Bytef*)in;
	( (z_stream*)m_zs)->avail_in = read;
	( (z_stream*)m_zs)->total_in = 0;

	do
	{
		( (z_stream*)m_zs)->next_out = (Bytef*)out;
		( (z_stream*)m_zs)->avail_out = BLOCKSIZE;
		( (z_stream*)m_zs)->total_out = 0;

		// Deflate this block
		if ( deflate( (z_stream*)m_zs, Z_NO_FLUSH ) != Z_OK ) 
			return FALSE;

		// Write out the data
		Write( out, ( (z_stream*)m_zs)->total_out );

	} while ( ( (z_stream*)m_zs)->total_out == BLOCKSIZE );

	// Are we done
	if ( m_dwSourcePos >= m_dwSource )
	{
		int err;
		// Finish the compression
		do
		{
			( (z_stream*)m_zs)->total_out = 0;
			( (z_stream*)m_zs)->next_out = (Bytef*)out;
			( (z_stream*)m_zs)->avail_out = BLOCKSIZE;
			err = deflate( (z_stream*)m_zs, Z_FINISH );
			// Write out the data
			Write( out, ( (z_stream*)m_zs)->total_out );
		} while ( err == Z_OK );
		if ( err != Z_STREAM_END ) return FALSE;
		
		// End compression
		if ( deflateEnd( (z_stream*)m_zs ) != Z_OK ) return FALSE;

		// Write out the crc
		if ( !Write( &m_dwCRC, sizeof( DWORD ) ) )
			return FALSE;
		
		// Reset
//		Destroy();

		// Signal that we're done
		if ( pDone != NULL ) *pDone = TRUE;
		return TRUE;
	} // end if

	// Keep going
	return TRUE;
}

BOOL CComp2::WriteHeader()
{_STT();
	// Write out the file ID
	if ( *m_szFileID != NULL )
	{
		if ( !Write( m_szFileID, strlen( m_szFileID ) ) )
			return FALSE;
	} // end if

	// Write out the size of the source file
	if ( !Write( &m_dwSource, sizeof( DWORD ) ) )
		return FALSE;

	// Write out the file name
	if ( !Write( m_szExpandFileName, strlen( m_szExpandFileName ) + 1 ) )
		return FALSE;

	m_dwCRC = 0;

	return TRUE;
}

#endif

#ifdef CCOMP2_EXPAND

BOOL CComp2::ExpandInit()
{_STT();
	if ( !IsSource() ) return FALSE;
	if ( !ReadHeader() ) return FALSE;

	ZeroMemory( m_zs, sizeof( z_stream ) );

	( (z_stream*)m_zs)->zalloc = (alloc_func)0;
	( (z_stream*)m_zs)->zfree = (free_func)0;
	( (z_stream*)m_zs)->opaque = (voidpf)0;

	if ( inflateInit( (z_stream*)m_zs ) != Z_OK ) return FALSE;
	
	return TRUE;
}

BOOL CComp2::ExpandContinue(BOOL *pDone)
{_STT();
	if ( pDone != NULL ) *pDone = FALSE;
	if ( !IsSource() || !IsDest() ) return FALSE;

	// Are we done?
	if ( m_dwDestPos >= m_dwExpandFileSize )
		return FALSE;

	DWORD	read = 0; 
	char	in[ BLOCKSIZE ];
	char	out[ BLOCKSIZE ];

	// Read the data
	if ( !Read( in, BLOCKSIZE, &read ) || read == 0 )
		return FALSE;

	( (z_stream*)m_zs)->next_in = (Bytef*)in;
	( (z_stream*)m_zs)->avail_in = read;
	( (z_stream*)m_zs)->total_in = 0;

	int err;

	// Expand this block
	do 
	{
		( (z_stream*)m_zs)->next_out = (Bytef*)out;
		( (z_stream*)m_zs)->avail_out = BLOCKSIZE;
		( (z_stream*)m_zs)->total_out = 0;

		// Inflate the block
		err = inflate( (z_stream*)m_zs, Z_NO_FLUSH );

		// Ignore buffer errors
		if ( err == Z_BUF_ERROR ) err = 0;

		// Write out the data
		if ( !Write( out, ( (z_stream*)m_zs)->total_out ) )
			return FALSE;
		
		// Keep track of the crc
#ifdef VERIFY_CRC
		m_dwCRC = CWinFile::CRC32( m_dwCRC, (LPBYTE)out, ( (z_stream*)m_zs)->total_out );
#endif

	} while ( err == Z_OK && ( (z_stream*)m_zs)->total_out == BLOCKSIZE );
	
	// Are we done
	if ( m_dwDestPos >= m_dwExpandFileSize )
	{
		// End compression
		if ( inflateEnd( (z_stream*)m_zs ) != Z_OK ) return FALSE;

		// Verify crc
#ifdef VERIFY_CRC
		{
			DWORD crc = 0;

			// Read the crc
			m_fSource.SetPtrPosCur( -long( sizeof( DWORD ) ) );
			m_dwSourcePos = m_dwSource - sizeof( DWORD );
			Read( &crc, sizeof( DWORD ) );
			if ( m_dwCRC != crc ) return FALSE;

		} // end check crc
#endif

		// Reset
		Destroy();

		// Signal that we're done
		if ( pDone != NULL ) *pDone = TRUE;
		return TRUE;
	} // end if
	else if ( err != Z_OK ) 
	{
		Destroy();
		return FALSE;
	} // end else if

	// Keep going
	return TRUE;
}

BOOL CComp2::ReadHeader()
{_STT();
	char id[ MAX_PATH ];

	// Verify file id
	if ( *m_szFileID != NULL )
	{
		if ( !Read( id, strlen( m_szFileID ) ) )
			return FALSE;
		id[ strlen( m_szFileID ) ] = NULL;
		if ( strcmp( id, m_szFileID ) ) return FALSE;
	} // end if

	// Read in the file size
	if ( !Read( &m_dwExpandFileSize, sizeof( DWORD ) ) )
		return FALSE;

	DWORD bufpos = m_dwSourcePos;
	DWORD filepos = 0;
	m_fSource.GetPtrPos( &filepos );

	// Read in the file name
	if ( !Read( m_szExpandFileName, MAX_PATH ) )
		return FALSE;

	// Set the pointer to the start of the data
	m_dwSourcePos = bufpos + strlen( m_szExpandFileName ) + 1;
	m_fSource.SetPtrPosBegin( filepos + strlen( m_szExpandFileName ) + 1 );

	m_dwCRC = 0;

	return TRUE;
}

#endif

BOOL CComp2::Write(LPVOID buf, DWORD size)
{_STT();
	if ( buf == NULL || size == 0 ) return TRUE;

	// Write to buffer
	if ( m_pDest != NULL )
	{
		if ( m_dwDestPos > m_dwDest ) return FALSE;
		if ( size > ( m_dwDest - m_dwDestPos ) ) return FALSE;
		memcpy( &m_pDest[ m_dwDestPos ], buf, size );
		m_dwDestPos += size;
		return TRUE;
	} // end if

	// Write to file
	else if ( m_fDest.IsOpen() )
	{
		m_fDest.Write( buf, size );
		m_dwDestPos += size;
		return TRUE;
	} // end else if

	return FALSE;
}

BOOL CComp2::Read(LPVOID buf, DWORD size, LPDWORD read )
{_STT();
	if ( m_pSource != NULL )
	{
		if ( m_dwSourcePos >= m_dwSource ) return FALSE;
		if ( size > ( m_dwSource - m_dwSourcePos ) )
		{
			// Read whatever we have
			if ( m_dwSource - m_dwSourcePos == 0 ) return FALSE;
			memcpy( buf, &m_pSource[ m_dwSourcePos ], ( m_dwSource - m_dwSourcePos ) );
			if ( read != NULL ) *read = ( m_dwSource - m_dwSourcePos );
			m_dwSourcePos = m_dwSource;
			return TRUE;			
		} // end if
		memcpy( buf, &m_pSource[ m_dwSourcePos ], size );
		if ( read != NULL ) *read = size;
		m_dwSourcePos += size;
		return TRUE;
	} // end if
	
	// Write to file
	else if ( m_fSource.IsOpen() ) 
	{
		// Is there a max bytes we can read?
		if ( m_dwSource > 0 )
		{
			if ( m_dwSourcePos >= m_dwSource ) return FALSE;
			if ( size > ( m_dwSource - m_dwSourcePos ) )
				size = m_dwSource - m_dwSourcePos;
		} // end if

		DWORD dwRead = 0;
		if ( !m_fSource.Read( buf, size, &dwRead ) )
			return FALSE;
		m_dwSourcePos += dwRead;
		if ( read != NULL ) *read = dwRead;
		return TRUE;
	} // end else if

	return FALSE;
}



//==================================================================
// Multi thread support
//==================================================================
#ifdef CCOMP2_MULTITHREAD

BOOL CComp2::MTCompressContinue(BOOL *pDone)
{_STT();
	if ( pDone != NULL ) *pDone = FALSE;
	if ( !IsSource() || !IsDest() ) return FALSE;

	// Are we done?
	if ( m_dwSourcePos >= m_dwSource )
		return FALSE;

	DWORD	read = 0; 
	char	in[ BLOCKSIZE ];
	char	out[ BLOCKSIZE ];

	// Read the data
	if ( !Read( in, BLOCKSIZE, &read ) || read == 0 )
		return FALSE;

	// Keep track of the crc
#ifdef VERIFY_CRC
	m_dwCRC = CWinFile::CRC32( m_dwCRC, (LPBYTE)in, read );
#endif

	( (z_stream*)m_zs)->next_in = (Bytef*)in;
	( (z_stream*)m_zs)->avail_in = read;
	( (z_stream*)m_zs)->total_in = 0;

	do
	{
		( (z_stream*)m_zs)->next_out = (Bytef*)out;
		( (z_stream*)m_zs)->avail_out = BLOCKSIZE;
		( (z_stream*)m_zs)->total_out = 0;

		// Deflate this block
		if ( deflate( &m_zs, Z_NO_FLUSH ) != Z_OK ) 
			return FALSE;

		// Write out the data
		Write( out, ( (z_stream*)m_zs)->total_out );

	} while ( ( (z_stream*)m_zs)->total_out == BLOCKSIZE );

	// Are we done
	if ( m_dwSourcePos >= m_dwSource )
	{
		int err;
		// Finish the compression
		do
		{
			( (z_stream*)m_zs)->total_out = 0;
			( (z_stream*)m_zs)->next_out = (Bytef*)out;
			( (z_stream*)m_zs)->avail_out = BLOCKSIZE;
			err = deflate( &m_zs, Z_FINISH );
			// Write out the data
			Write( out, ( (z_stream*)m_zs)->total_out );
		} while ( err == Z_OK );
		if ( err != Z_STREAM_END ) return FALSE;
		
		// End compression
		if ( deflateEnd( &m_zs ) != Z_OK ) return FALSE;

		// Write out the crc
		if ( !Write( &m_dwCRC, sizeof( DWORD ) ) )
			return FALSE;
		
		// Reset
		Destroy();

		// Signal that we're done
		if ( pDone != NULL ) *pDone = TRUE;
		return TRUE;
	} // end if

	// Keep going
	return TRUE;

}

BOOL CComp2::MTExpandContinue(BOOL *pDone)
{_STT();

	return TRUE;
}

DWORD FAR PASCAL CComp2::Thread ( LPVOID pData )
{_STT();
	// Get a pointer to our data
	LPCOMPTHREADINFO	cti = (LPCOMPTHREADINFO)pData;
	if ( cti == NULL ) return 1;

	if ( cti->bCompress )
	{
		// Deflate this block
		cti->err = deflate( &cti->zs, Z_NO_FLUSH );
	} // end if
	else
	{
		// Inflate the block
		cti->err = inflate( &cti->zs, Z_NO_FLUSH );
	} // end else

	return 0;
}

#endif // CCOMP_MULTITHREAD

#endif // ENABLE_ZLIB
