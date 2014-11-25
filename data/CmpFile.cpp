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
// CmpFile.cpp: implementation of the CCmpFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef ENABLE_ZLIB

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define CHUNKID1	0x77c3152f
#define CHUNKID2	0xe013ba83
#define CHUNKID3	0xf03bb264
#define CHUNKID4	0x385107ca

#define BLOCKID1	0xf4a5275a
#define BLOCKID2	0x8422dc0e
#define BLOCKID3	0xccb5296b
#define BLOCKID4	0x91a7510f

#define FILEID		"www.wheresjames.com Data File v1.0\r\n"

// -----------------------------------------------------------------
// | FILEID | HDR CHUNK | DATA CHUNK | DATA BLOCKS | ...
// -----------------------------------------------------------------

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCmpFile::CCmpFile() : CWinFile(), CHList()
{
	m_dwIndex = 0;
	m_dwChunks = 0;
	m_dwBadChunks = 0;
	m_dwBadBlocks = 0;
	m_llLastGoodChunk = 0;
}

CCmpFile::~CCmpFile()
{
	Destroy();
}																								 

void CCmpFile::Destroy()
{
	CWinFile::Close();
	CLList::Destroy();
	m_header.Destroy();

	m_dwIndex = 0;
	m_dwChunks = 0;
	m_dwBadChunks = 0;
	m_dwBadBlocks = 0;
	m_llLastGoodChunk = 0;
}

BOOL CCmpFile::InitObject( void *node )
{
	CHList::InitObject( node );

	LPCMPINFO pci = (LPCMPINFO)node;
	if ( pci == NULL ) return NULL;

	pci->cfg = new CCfgFile();
	if ( pci->cfg == NULL ) { CLList::Delete( pci ); return NULL; }

	return TRUE;
}

void CCmpFile::DeleteObject( void *node )
{
	LPCMPINFO pci = (LPCMPINFO)node;
	if ( pci == NULL ) return;

	if ( pci->cfg != NULL )
	{	delete pci->cfg;
		pci->cfg = NULL;
	} // end if

	// Lose binary data
	if ( pci->pdata != NULL )
	{	delete [] pci->pdata;
		pci->pdata = NULL;
	} // end if

	CHList::DeleteObject( node );
}

DWORD CCmpFile::Load( LPCTSTR pFile, BOOL bMerge )
{
	Destroy();

	if ( !OpenExisting( pFile ) ) 
		return 0;

	DWORD read = 0;
	char buf[ 512 ];

	// Read the file id
	DWORD len = strlen( FILEID );
	if ( !Read( buf, len, &read )
			|| read != len ) return 0;
	buf[ len ] = 0;

	// Verify id
	if ( strcmpi( buf, FILEID ) ) return 0;

	// Read in the index data
	if ( !Read( &m_index, sizeof( m_index ), &read ) || 
			read != sizeof( m_index ) ) return 0;

	// Save file pointer position
	LONGLONG offset = GetPtrPos();

	// Verify file integrity
	if ( !VerifyMD5() ) return 0;

	// Restore file pointer
	SetPtrPosBegin( offset );

	// Load the header
	LoadChunk( TRUE );

	// Load chunks from file
	LONGLONG ret = 1;
	while ( ret )
	{	ret = LoadChunk();
		if ( ret == CMPRET_BADCHUNK ) m_dwBadChunks++;
//		else if ( ret == CMPRET_SKIPCHUNK );
	} // end while

	// Init resource lookup
	InitLookup();

	return 1;
}


LONGLONG CCmpFile::LoadChunk( BOOL bHeader )
{
	// Allocate memory for chunk
	LPCMPINFO pci;	
	if ( !bHeader )
	{	pci = (LPCMPINFO)New();
		if ( pci == NULL ) return 0;
	} // end if
	else pci = new CMPINFO;

	if ( pci == NULL ) return 0;

	// Save offset to start of chunk
	LONGLONG offset = GetPtrPos();

	// Cannot back up in file
	if ( offset < m_llLastGoodChunk ) return 0;

	DWORD			read;
	CMPCHUNKINFO	ci;

	DWORD			filesize = CWinFile::Size();

	// Find start of chunk
	BOOL bFound = FALSE;
	BOOL bError = FALSE;
	while ( !bFound )
	{
		// Read chunk info
		if ( !Read( &ci, sizeof( ci ), &read ) || read != sizeof( ci ) )
		{	if ( bHeader ) delete pci; else CLList::Delete( pci ); return 0; }

		// Verify block id
		if (	ci.id[ 0 ] != CHUNKID1 || ci.id[ 1 ] != CHUNKID2 ||
				ci.id[ 2 ] != CHUNKID3 || ci.id[ 3 ] != CHUNKID4 )
		{
			// Count one bad chunk
			if ( !bError ) 
			{	
				// Set one shot flag
				bError = TRUE; 
				
				// Count the error
				m_dwBadChunks++; 
				
				// Go back to the last know good chunk
				// This is in case the chunk size was blown away
				if ( m_llLastGoodChunk )
					SetPtrPosBegin( m_llLastGoodChunk + 16 );
			}

			// Any data left?
			if ( ( offset + sizeof( ci ) + 1 ) >= filesize ) return 0;

			// Skip ahead one byte
			// In case of file damage slide forward 
			// until we find the next block
			SetPtrPosBegin( ++offset );

		} // end if

		else bFound = TRUE;

	} // end while

	// Save good chunk id position
	m_llLastGoodChunk = offset;

	// Skip if it's the manifest
	if ( ( ci.type & CMPTYPE_MASK ) == CMPTYPE_MANIFEST )
	{	CHList::Delete( pci );
		SetPtrPosBegin( offset + ci.size );	
		return CMPRET_SKIPCHUNK;
	} // end if
	
	// Is there a header?
	if ( bHeader && ( ci.type & CMPTYPE_MASK ) != CMPTYPE_HEADER )
//	if ( bHeader && ( ci.type & CMPTYPE_HEADER ) != 0 )
	{	SetPtrPosBegin( offset );
		delete pci;
		return 0;
	} // end if

	// Save chunk data
	pci->foff = offset;
	pci->blocks = ci.blocks;
	pci->size = ci.size;
	pci->type = ci.type;
	pci->user = ci.user;

	// Skip Chunk info
	SetPtrPosBegin( offset + ci.sz );

	// Read 
	CMPBLOCKINFO	cbi;

	// Read chunk info
	if ( !Read( &cbi, sizeof( cbi ), &read ) || read != sizeof( cbi ) )
	{	if ( bHeader ) delete pci; else CLList::Delete( pci ); 
		return MAXDWORD; 
	} // end if

	// Check for empty chunk
	if (	cbi.id[ 0 ] == CHUNKID1 && cbi.id[ 1 ] == CHUNKID2 &&
			cbi.id[ 2 ] == CHUNKID3 && cbi.id[ 3 ] == CHUNKID4 )
	{	// Backup to start of block
		SetPtrPosBegin( offset + sizeof( ci ) );
		if ( bHeader ) delete pci; else CLList::Delete( pci ); 
		return 1;
	} // end if

	// Verify block id
	if (	cbi.id[ 0 ] != BLOCKID1 || cbi.id[ 1 ] != BLOCKID2 ||
			cbi.id[ 2 ] != BLOCKID3 || cbi.id[ 3 ] != BLOCKID4 )
	{
		if ( bHeader ) delete pci; else CLList::Delete( pci ); 
		return MAXDWORD; 
	}

	// Is there any information?
	if ( ( cbi.flags & CMPBFLAG_INFO ) == 0 )
	{
		// Skip this chunk
		SetPtrPosBegin( offset + ci.size );	

		// Return chunk size
		return ci.size;
	
	} // end if

	TMem< BYTE > buf;

	// Allocate memory
	if ( !buf.allocate( cbi.usize + 1 ) ) 
	{	if ( bHeader ) delete pci; else CLList::Delete( pci ); return MAXDWORD; }

	// Extract the information
	if ( !ExtractInfo( &cbi, buf, cbi.usize ) )
	{	if ( bHeader ) delete pci; else CLList::Delete( pci ); return MAXDWORD; }

	// NULL terminate
	buf[ cbi.usize ] = 0;

	// Are we loading the header?
	if ( bHeader )
	{	m_header.LoadFromMem( buf, cbi.usize );
		delete pci;
		return TRUE;
	} // end if

	// Load the data
	if ( !pci->cfg->LoadFromMem( buf, cbi.usize ) )
	{	if ( bHeader ) delete pci; else CLList::Delete( pci ); return MAXDWORD; }

	// Get values
	pci->cfg->GetValue( "Source", "HKEY", (LPDWORD)&pci->hkey );
	pci->cfg->GetValue( "Source", "String", pci->str, sizeof( pci->str ) );
	pci->cfg->GetValue( "Source", "Name", pci->name, sizeof( pci->name ) );
	pci->cfg->GetValue( "Source", "Size", (LPDWORD)&pci->dsize );
	pci->cfg->GetValue( "Source", "Mask", pci->mask, sizeof( pci->mask ) );

	// Skip this chunk
	SetPtrPosBegin( offset + ci.size );	

	// Return size of chunk
	return ci.size;
}

LPCMPINFO CCmpFile::AddBin(void * buf, DWORD size, LPCTSTR pName, DWORD dwUser, BOOL bCopy )
{
	// Sanity checks
	if ( size == 0 ) return NULL;

	// Remove 
	Remove( pName, dwUser );

	// Allocate memory for chunk
	LPCMPINFO pci = (LPCMPINFO)New( NULL, 0, pName );
	if ( pci == NULL ) return NULL;

	// Copy block information
	pci->type = CMPTYPE_BIN;
	pci->user = dwUser;
	if ( pName != NULL ) strcpy_sz( pci->name, pName );

	// Do we want to copy the data?
	if ( bCopy ) 
	{	pci->pdata = new BYTE[ size ];
		if ( pci->pdata == NULL )
		{	CLList::Delete( pci ); return NULL; }
		if ( buf != NULL ) memcpy( pci->pdata, buf, size );
		else ZeroMemory( pci->pdata, size );
		pci->dsize = size;
	} // end if
	else
	{	pci->pdata = NULL;
		pci->copy = buf;
		pci->dsize = size;
	} // end else

	return pci;
}

LPCMPINFO CCmpFile::AddFile(LPCTSTR pFile, LPCTSTR pName, DWORD dwUser)
{
	if ( pFile == NULL || *pFile == 0 )
		return NULL;

	// Remove 
	Remove( pName, dwUser );

	// Allocate memory for chunk
	LPCMPINFO pci = (LPCMPINFO)New( NULL, 0, pName );
	if ( pci == NULL ) return NULL;

	// Copy block information
	pci->type = CMPTYPE_FILE;
	pci->user = dwUser;
	if ( pName != NULL ) strcpy_sz( pci->name, pName );
	strcpy_sz( pci->str, pFile );

	return pci;
}


LPCMPINFO CCmpFile::AddDir(LPCTSTR pDir, LPCTSTR pMask, LPCTSTR pName, DWORD dwUser)
{
	if ( pDir == NULL || *pDir == 0 )
		return NULL;

	// Remove 
	Remove( pName, dwUser );

	// Allocate memory for chunk
	LPCMPINFO pci = (LPCMPINFO)New( NULL, 0, pName );
	if ( pci == NULL ) return NULL;

	// Copy block information
	pci->type = CMPTYPE_DIR;
	pci->user = dwUser;
	if ( pName != NULL ) strcpy_sz( pci->name, pName );
	strcpy_sz( pci->str, pDir );
	strcpy_sz( pci->mask, pMask );

	// Add file list if directory
	AddFileInfo( pci );		 

	return pci;
}

BOOL CCmpFile::AddLinks(CHList *pLinks, LPCTSTR pName, DWORD dwUser)
{
	// Sanity checks
	if ( pLinks == NULL ) return FALSE;

	// Remove 
	Remove( pName, dwUser );

	// Allocate memory for chunk
	LPCMPINFO pci = (LPCMPINFO)New( NULL, 0, pName );
	if ( pci == NULL ) return NULL;

	// Copy block information
	pci->type = CMPTYPE_WEB;
	pci->user = dwUser;
	strcpy_sz( pci->name, pName );

	// Create file list group
	HGROUP hGroup = pci->cfg->AddGroup( "Files" );
	if ( hGroup == NULL ) return FALSE;

	DWORD i = 0;
	char msg[ 512 ];
	LPLLISTINFO plli = NULL;
	while ( ( plli = (LPLLISTINFO)pLinks->GetNext( plli ) ) != NULL )
	{	wsprintf( msg, "F%lu", i );
		if ( pci->cfg->SetValue( hGroup, msg, plli->cpkey ) ) i++;
	} // end while

	return TRUE;
}

LPCMPINFO CCmpFile::AddReg(LPCTSTR pReg, LPCTSTR pName, DWORD dwUser)
{
	if ( pReg == NULL || *pReg == 0 )
		return NULL;

	// Remove 
	Remove( pName, dwUser );

	// Allocate memory for chunk
	LPCMPINFO pci = (LPCMPINFO)New( NULL, 0, pName );
	if ( pci == NULL ) return NULL;

	// Copy block information
	pci->type = CMPTYPE_REG;

	pci->user = dwUser;
	if ( pName != NULL ) strcpy_sz( pci->name, pName );

	strcpy_sz( pci->str, pReg );

	return pci;
}

BOOL CCmpFile::Store(LPCTSTR pFile)
{
	if ( pFile == NULL ) return FALSE;
	
	// Open file for write access
	if ( !OpenNew( pFile, GENERIC_WRITE ) )
		return FALSE;
										 
	// First write out the file id
	if ( !Write( FILEID ) ) return FALSE;

	// Write out the index
	if ( !Write( &m_index, sizeof( m_index ) ) ) return FALSE;

	// Save start of chunk
	LONGLONG startptr = GetPtrPos();

	// Save chunk info
	CMPCHUNKINFO	ci;
	ZeroMemory( &ci, sizeof( ci ) );

	// Fill in size of chunk information
	ci.sz = sizeof( ci );

	// Save chunk id's
	ci.id[ 0 ] = CHUNKID1;
	ci.id[ 1 ] = CHUNKID2;
	ci.id[ 2 ] = CHUNKID3;
	ci.id[ 3 ] = CHUNKID4;

	ci.type = CMPTYPE_HEADER;
	ci.blocks = 0;

	// Have to fill this in later
	ci.size = 0;

	// Write out the information
	if ( !Write( &ci, sizeof( ci ) ) ) 
	{	SetPtrPosBegin( startptr );
		return FALSE;
	} // end if		

	// Save header information
	SaveCfg( &m_header );

	// Get the chunk size
	LONGLONG endptr = GetPtrPos();
	SetPtrPosBegin( startptr );

	// Add new information
	ci.size = endptr - startptr;
	if ( !Write( &ci, sizeof( ci ) ) ) 
	{	SetPtrPosBegin( startptr );
		return FALSE;
	} // end if
					
	// Set pointer to end of data
	SetPtrPosBegin( endptr );

	// Save all chunks
	LPCMPINFO pci = NULL;
	while ( ( pci = (LPCMPINFO)GetNext( pci ) ) != NULL )
	{
		// Save the chunk
		SaveChunk( pci ); 

	} // end while

	// Open file for read/write access
	if ( !OpenExisting( pFile, GENERIC_WRITE | GENERIC_READ ) )
		return FALSE;

	// Add the manifest
	SaveManifest();

	return TRUE;
}

BOOL CCmpFile::SaveCfg(CCfgFile * pCfg)
{
	// Sanity check
	if ( pCfg == NULL ) return FALSE;

	// Is there anything to save?
	if ( pCfg->Size() == 0 ) return FALSE;
	DWORD size = pCfg->SaveToMem( NULL, 0 );
	if ( size == 0 ) return FALSE;

	// Allocate memory for chunk info
	LPBYTE buf = new BYTE[ size + 1 ];
	if ( buf == NULL ) return FALSE;

	// Serialize chunk info
	if ( !pCfg->SaveToMem( buf, size ) )
	{	delete [] buf; return FALSE; }

	CMPBLOCKINFO	cbi;

	// Save size of block information
	cbi.sz = sizeof( cbi );

	// Save block id's
	cbi.id[ 0 ] = BLOCKID1;
	cbi.id[ 1 ] = BLOCKID2;
	cbi.id[ 2 ] = BLOCKID3;
	cbi.id[ 3 ] = BLOCKID4;

	// Save data
	cbi.csize = 0;
	cbi.usize = size;
	cbi.index = m_dwIndex++;
	cbi.flags = CMPBFLAG_ZLIB | CMPBFLAG_INFO;

	// Save pointer position
	LONGLONG startptr = GetPtrPos();

	// Write block info
	if ( !Write( &cbi, sizeof( cbi ) ) )
	{	delete [] buf; return FALSE; }

	// Get ready to compress the info
	CComp2 cmp;		
	cmp.SetSource( buf, size );
	cmp.SetDest( NULL, GetHandle() );
	cmp.CompressInit();

	// Compress the information
	BOOL bDone = FALSE;
	while ( cmp.CompressContinue( &bDone ) && !bDone );
	if ( !bDone ) { delete [] buf; return FALSE; }
	delete [] buf;

	// Save the compressed size
	LONGLONG endinfo = GetPtrPos();
	SetPtrPosBegin( startptr );
	cbi.csize = cmp.GetBytesWritten();

	// Write completed block info
	if ( !Write( &cbi, sizeof( cbi ) ) ) return FALSE;

	// Skip back to the end of the information
	SetPtrPosBegin( endinfo );

	return TRUE;
}

BOOL CCmpFile::SaveChunk(LPCMPINFO pCi)
{
	if ( pCi == NULL ) return FALSE;

	// Save start of chunk
	LONGLONG startptr = GetPtrPos();

//	static counter = 0;
//	if ( ++counter > 1 )
//		int x = 0;

	// Save chunk info
	CMPCHUNKINFO	ci;

	// Save size of chunk information
	ci.sz = sizeof( ci );

	// Save chunk id's
	ci.id[ 0 ] = CHUNKID1;
	ci.id[ 1 ] = CHUNKID2;
	ci.id[ 2 ] = CHUNKID3;
	ci.id[ 3 ] = CHUNKID4;

	// Save type information
	ci.type = pCi->type;
	ci.user = pCi->user;

	// Have to fill this in later
	ci.size = 0;
	ci.blocks = 0;

	// Write out the information
	if ( !Write( &ci, sizeof( ci ) ) ) 
	{	SetPtrPosBegin( startptr );
		return FALSE;
	} // end if		

	// Add save information
	pCi->cfg->SetValue( "Source", "HKEY", (DWORD)pCi->hkey );
	pCi->cfg->SetValue( "Source", "String", pCi->str );
	pCi->cfg->SetValue( "Source", "Name", pCi->name );
	pCi->cfg->SetValue( "Source", "Size", pCi->dsize );
	pCi->cfg->SetValue( "Source", "Mask", pCi->mask );

	// Save the configuration
	SaveCfg( pCi->cfg );

	DWORD blocks = 0;

	// Save chunk data
	switch ( ( pCi->type & CMPTYPE_MASK ) )
	{
		case CMPTYPE_BIN : 
			if ( pCi->pdata != NULL && pCi->dsize != 0 )
				blocks = SaveBin( pCi->pdata, pCi->dsize );
			else if ( pCi->copy != NULL && pCi->dsize != 0 )
				blocks = SaveBin( pCi->copy, pCi->dsize );
			break;
		case CMPTYPE_FILE : blocks = SaveFile( pCi->str ); break;
		case CMPTYPE_DIR : blocks = SaveDir( pCi ); break;
		case CMPTYPE_REG : blocks = SaveReg( pCi->hkey, pCi->str ); break;
		case CMPTYPE_WEB : blocks = SaveLinks( pCi ); break;

	} // end switch

	// Get the chunk size
	LONGLONG endptr = GetPtrPos();
	SetPtrPosBegin( startptr );

	// Add new information
	ci.size = endptr - startptr;
	ci.blocks = blocks;
	if ( !Write( &ci, sizeof( ci ) ) ) 
	{	SetPtrPosBegin( startptr );
		return FALSE;
	} // end if
					
	// Set pointer to end of data
	SetPtrPosBegin( endptr );

	return TRUE;
}

DWORD CCmpFile::SaveFile( LPCTSTR pFile, DWORD index )
{
	if ( pFile == NULL ) return 0;
	if ( !CWinFile::DoesExist( pFile ) ) return 0;

	CWinFile f;
	if ( !f.OpenExisting( pFile ) ) return 0;

	LONGLONG startptr = GetPtrPos();
	
	CMPBLOCKINFO	cbi;

	// Save size of block information
	cbi.sz = sizeof( cbi );

	// Save block id's
	cbi.id[ 0 ] = BLOCKID1;
	cbi.id[ 1 ] = BLOCKID2;
	cbi.id[ 2 ] = BLOCKID3;
	cbi.id[ 3 ] = BLOCKID4;

	// Save data
	cbi.csize = 0;
	cbi.usize = f.Size();
	cbi.index = index;
	cbi.flags = CMPBFLAG_ZLIB;

	// Write block info
	if ( !Write( &cbi, sizeof( cbi ) ) ) 
	{	SetPtrPosBegin( startptr ); return FALSE; }

	// Get ready to compress the info
	CComp2 cmp;		
	cmp.SetSource( NULL, f.GetHandle() );
	cmp.SetDest( NULL, GetHandle() );
	cmp.CompressInit();

	// Compress the information
	BOOL bDone = FALSE;
	while ( cmp.CompressContinue( &bDone ) && !bDone );
	if ( !bDone ) { SetPtrPosBegin( startptr ); return FALSE; }

	// Save the compressed size
	LONGLONG endinfo = GetPtrPos();
	SetPtrPosBegin( startptr );
	cbi.csize = cmp.GetBytesWritten();

	// Write completed block info
	if ( !Write( &cbi, sizeof( cbi ) ) )
	{	SetPtrPosBegin( startptr ); return FALSE; }

	// Skip back to the end of the information
	SetPtrPosBegin( endinfo );

	return 1;
}


DWORD CCmpFile::SaveDir( LPCMPINFO pCi )
{
	// Sanity check
	if ( pCi == NULL ) return 0;

	// Get file list
	HGROUP hGroup = pCi->cfg->FindGroup( "Files" );
	if ( hGroup == NULL ) return FALSE;

	char msg[ CWF_STRSIZE ];
	char fname[ CWF_STRSIZE ];
	char full[ CWF_STRSIZE ];

	// Add each file
	for ( DWORD i = 0; ; i++ )
	{
		// Get file name
		wsprintf( msg, "F%lu", i );
		if ( !pCi->cfg->GetValue( hGroup, msg, fname, sizeof( fname ) ) )
			return i;

		// Build full path
		CWinFile::BuildPath( full, pCi->str, fname );

		// Save this file
		SaveFile( full, i );

	} // end for	

	return i;
}

DWORD CCmpFile::SaveLinks(LPCMPINFO pCi)
{
	// Sanity check
	if ( pCi == NULL ) return 0;


	// Get file list
	HGROUP hGroup = pCi->cfg->FindGroup( "Files" );
	if ( hGroup == NULL ) return FALSE;

	char msg[ CWF_STRSIZE ];
	char link[ CWF_STRSIZE ];
	
	// Start downloading files
	if ( m_fnInitDownload != NULL ) m_fnInitDownload();

	// Get total size
	DWORD total = hGroup->size;

	// Add each file
	for ( DWORD i = 0; ; i++ )
	{
		// Get file name
		wsprintf( msg, "F%lu", i );
		if ( !pCi->cfg->GetValue( hGroup, msg, link, sizeof( link ) ) )
		{	if ( m_fnEndDownload != NULL ) m_fnEndDownload(); return i; }

		LPBYTE	buf = NULL;
		DWORD	size = 0;
		if (	m_fnDownload != NULL &&
				m_fnDownload( link, &buf, &size, i, total ) 
				&& buf != NULL && size > 0 )
			SaveBin( buf, size, i );

		// Save NULL Chunk if error
		else SaveBin( NULL, 0, i );

	} // end for
	
	// End file downloading
	if ( m_fnEndDownload != NULL ) m_fnEndDownload();

	return i;
}

DWORD CCmpFile::SaveReg( HKEY hKey, LPCTSTR pKey )
{

	return 1;
}


DWORD CCmpFile::SaveBin(void * buf, DWORD size, DWORD index)
{
	LONGLONG startptr = GetPtrPos();
	
	CMPBLOCKINFO	cbi;

	// Save size of block information
	cbi.sz = sizeof( cbi );

	// Save block id's
	cbi.id[ 0 ] = BLOCKID1;
	cbi.id[ 1 ] = BLOCKID2;
	cbi.id[ 2 ] = BLOCKID3;
	cbi.id[ 3 ] = BLOCKID4;

	// Save data
	cbi.csize = 0;
	cbi.usize = 0;
	cbi.index = index;
	cbi.flags = 0;
	cbi.offset = GetPtrPos();

	LONGLONG endinfo = 0;

	// If not a NULL block
	if ( buf != NULL && size > 0 )
	{
		cbi.flags = CMPBFLAG_ZLIB;
		cbi.usize = size;

		// Write block info
		if ( !Write( &cbi, sizeof( cbi ) ) ) 
		{	SetPtrPosBegin( startptr ); return FALSE; }

		// Get ready to compress the info
		CComp2 cmp;		
		cmp.SetSource( (LPBYTE)buf, size );
		cmp.SetDest( NULL, GetHandle() );
		cmp.CompressInit();

		// Compress the information
		BOOL bDone = FALSE;
		while ( cmp.CompressContinue( &bDone ) && !bDone );
		if ( !bDone ) { SetPtrPosBegin( startptr ); return FALSE; }

		// Save the compressed size
		endinfo = GetPtrPos();
		SetPtrPosBegin( startptr );
		cbi.csize = cmp.GetBytesWritten();

	} // end if

	// Write completed block info
	if ( !Write( &cbi, sizeof( cbi ) ) )
	{	SetPtrPosBegin( startptr ); return FALSE; }

	// Skip back to the end of the information
	if ( endinfo != 0 ) SetPtrPosBegin( endinfo );

	return 1;
}

BOOL CCmpFile::AddFileInfo(LPCMPINFO pCi, LPCTSTR pPath, LPDWORD pindex, HGROUP hGroup )
{
	// Sanity check
	if ( pCi == NULL ) return FALSE;

	// Did the user provide a directory name?
	if ( *pCi->str == 0 ) return TRUE;

	// Reset file list
	if ( hGroup == NULL )
	{	hGroup = pCi->cfg->FindGroup( "Files" );
		if ( hGroup != NULL ) pCi->cfg->RemoveGroup( hGroup );
		hGroup = pCi->cfg->AddGroup( "Files" );
		if ( hGroup == NULL ) return FALSE;
	} // end if

	WIN32_FIND_DATA		fd;
	char				path[ 1024 ];
	char				search[ 1024 ];
	HANDLE				hFind;

	// All files if not specified
	if ( *pCi->mask == 0 ) strcpy( pCi->mask, "*.*" );
	
	// Get file path
	if ( pPath == NULL ) strcpy( path, pCi->str );
	else CWinFile::BuildPath( path, pCi->str, pPath );
	
	// Build search string
	CWinFile::BuildPath( search, path, pCi->mask );

	// Punt if empty
	hFind = FindFirstFile( search, &fd );
	if ( hFind == INVALID_HANDLE_VALUE ) return 0;
	
	// Ensure index
	DWORD index = 0;
	if ( pindex == NULL ) pindex = &index;

	do
	{
		// Do we need to copy this file?
		if (	*fd.cFileName != NULL &&
				strcmp( fd.cFileName, "." ) &&
				strcmp( fd.cFileName, ".." ) )
				
		{
			char full[ MAX_PATH ];

			// Build file name
			if ( pPath == NULL ) strcpy( full, fd.cFileName );
			else CWinFile::BuildPath( full, pPath, fd.cFileName );

			// Is it a file?
			if ( ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
			{
				char msg[ 512 ];
				wsprintf( msg, "F%lu", *pindex );
				if ( pCi->cfg->SetValue( hGroup, msg, full ) ) ( *pindex )++;
				
			} // end if

			// A little recursion
			else AddFileInfo( pCi, full, pindex, hGroup ); 

		} // end if

	// Get the next file or directory
	} while ( FindNextFile( hFind, &fd ) );

	// Close the find handle
	FindClose( hFind );

	return index;
}


BOOL CCmpFile::Extract(LPCMPINFO pci, LPCTSTR pName, LPCTSTR pFile)
{
	// Sanity check
	if ( pci == NULL || pFile == NULL ) return FALSE;

	CMPBLOCKINFO cbi;
	ZeroMemory( &cbi, sizeof( cbi ) );

	switch ( pci->type & CMPTYPE_MASK )
	{
		case CMPTYPE_VOID :
		case CMPTYPE_HEADER :
		case CMPTYPE_BIN :
		case CMPTYPE_FILE :
		case CMPTYPE_REG :
		case CMPTYPE_MASK :
		{
			// Find the first block
			if ( !FindBlock( pci, &cbi ) ) return FALSE;

			// Skip if info block
			while ( ( cbi.flags & CMPBFLAG_INFO ) != 0 )
				if ( !GetNextBlock( &cbi ) ) return FALSE;

			// Extract the data to file
			return ExtractInfo( &cbi, pFile );

		} break;

		case CMPTYPE_DIR :
		{
			// Find the first block
			if ( !FindBlock( pci, &cbi ) ) return FALSE;

			// Skip if info block
			while ( ( cbi.flags & CMPBFLAG_INFO ) != 0 )
				if ( !GetNextBlock( &cbi ) ) return FALSE;

			char msg[ 512 ];
			char fname[ CWF_STRSIZE ];
			char full[ CWF_STRSIZE ];
			char path[ CWF_STRSIZE ];

			// Get the file name list
			HGROUP hGroup = pci->cfg->FindGroup( "Files" );
			if ( hGroup == NULL ) return FALSE;

			for ( DWORD i = 0; i < pci->blocks; i++ )
			{
				// Get file name
				wsprintf( msg, "F%lu", i );
				if ( pci->cfg->GetValue( hGroup, msg, fname, sizeof( fname ) ) )
				{
					if ( pName == NULL || !strcmp( pName, fname ) )
					{
						if ( pName != NULL ) strcpy( full, pName );
						else
						{
							// Build full path
							CWinFile::BuildPath( full, pFile, fname );

							// Create directory
							CWinFile::GetPathFromFileName( full, path );
							CWinFile::CreateDirectory( path );

						} // end else

						// Extract this file
						ExtractInfo( &cbi, full );

						// We're done if name specified
						if ( pName != NULL ) return TRUE;

					} // end if

				} // end if

				// Get next block
				if ( !GetNextBlock( &cbi ) ) return FALSE;				

			} // end for

		} break;

	} // end switch
		
	// Set file position
//	if ( !SetPtrPosBegin( pci->foff ) ) return FALSE;

	return FALSE;
}

BOOL CCmpFile::ExtractInfo(LPCMPBLOCKINFO pcbi, LPCTSTR pFile)
{
	// Sanity check
	if ( pcbi == NULL || pFile == NULL ) return FALSE;

	CWinFile	out;
	if ( !out.OpenNew( pFile, GENERIC_WRITE ) ) return FALSE;

	// Is the data compressed?
	if ( ( pcbi->flags & CMPBFLAG_ZLIB ) != 0 && pcbi->usize > 0 )
	{
		// Get ready to expand the info
		CComp2 cmp;		
		if ( !cmp.SetSource( NULL, GetHandle(), FALSE, pcbi->csize ) ) return FALSE;
		if ( !cmp.SetDest( NULL, out.GetHandle() ) ) return FALSE;
		if ( !cmp.ExpandInit() ) return FALSE;

		// Expand the information
		BOOL bDone = FALSE;
		while ( cmp.ExpandContinue( &bDone ) && !bDone );
		if ( !bDone ) return FALSE;

	} // end if	
	
	// Read uncompressed information
	else out.Copy( GetHandle(), pcbi->usize );

	return TRUE;
}

BOOL CCmpFile::ExtractInfo(LPCMPBLOCKINFO pcbi, LPVOID buf, DWORD size)
{
	// Is the data compressed?
	if ( ( pcbi->flags & CMPBFLAG_ZLIB ) != 0 && pcbi->usize > 0 )
	{
		// Get ready to expand the info
		CComp2 cmp;		
		cmp.SetSource( NULL, GetHandle(), FALSE, pcbi->csize );
		cmp.SetDest( (LPBYTE)buf, size );
		cmp.ExpandInit();

		// Expand the information
		BOOL bDone = FALSE;
		while ( cmp.ExpandContinue( &bDone ) && !bDone );
		if ( !bDone ) return FALSE;

	} // end if	
	
	// Read uncompressed information
	else
	{
		DWORD read = 0, get = ( pcbi->usize <= size ) ? pcbi->usize : size;

		// Read uncompressed data
		if ( !Read( buf, get, &read ) || read != get ) return FALSE;

	} // end else

	return TRUE;
}


BOOL CCmpFile::FindBlock(LPCMPINFO pci, LPCMPBLOCKINFO pcbi, DWORD block)
{
	// Sanity check
	if ( pci == NULL || pcbi == NULL ) return FALSE;

	// Set pointer to start of data
	if ( !SetPtrPosBegin( pci->foff ) ) return FALSE;

	DWORD			read;
	CMPCHUNKINFO	ci;

	// Read chunk info
	if ( !Read( &ci, sizeof( ci ), &read ) || read != sizeof( ci ) )
		return FALSE;

	// Verify chunk id
	if (	ci.id[ 0 ] != CHUNKID1 || ci.id[ 1 ] != CHUNKID2 ||
			ci.id[ 2 ] != CHUNKID3 || ci.id[ 3 ] != CHUNKID4 )
		return FALSE;

	// Does this chunk have that many blocks?
	if ( block > ci.blocks ) return FALSE;

	// First block
	pcbi->offset = 0;

	do
	{
		// Get next block
		if ( !GetNextBlock( pcbi ) ) return FALSE;

		// Is this our block?
		if ( block == 0 ) return TRUE;

		// Next block
		block--;

		// Skip data
		SetPtrPosBegin( pcbi->offset + pcbi->csize );		

	} while ( true );

	return FALSE;
}

BOOL CCmpFile::GetNextBlock(LPCMPBLOCKINFO pcbi)
{
	// Sanity check
	if ( pcbi == NULL ) return FALSE;

	// Skip if valid offset
	if ( pcbi->offset != 0 )
		SetPtrPosBegin( pcbi->offset + pcbi->sz + pcbi->csize );

	// Save file position
	LONGLONG offset = GetPtrPos();

	// Zero Memory
	ZeroMemory( pcbi, sizeof( CMPBLOCKINFO ) );

	// Read 
	DWORD read = 0;
	if ( !Read( pcbi, sizeof( CMPBLOCKINFO ), &read ) || 
				read != sizeof( CMPBLOCKINFO ) )
		return FALSE;

	// Check for empty chunk
	if (	pcbi->id[ 0 ] == CHUNKID1 && pcbi->id[ 1 ] == CHUNKID2 &&
			pcbi->id[ 2 ] == CHUNKID3 && pcbi->id[ 3 ] == CHUNKID4 )
		return FALSE;

	// Verify block id
	if (	pcbi->id[ 0 ] != BLOCKID1 || pcbi->id[ 1 ] != BLOCKID2 ||
			pcbi->id[ 2 ] != BLOCKID3 || pcbi->id[ 3 ] != BLOCKID4 )
		return FALSE;

	// Save block offset
	pcbi->offset = offset;

	return TRUE;
}

LPCMPINFO CCmpFile::FindCmp(LPCTSTR pName, DWORD dwUser, LPCMPINFO pLast )
{
	LPCMPINFO pci = pLast;

	if ( pName == NULL && dwUser == 0 ) return NULL;

	// Find the item
	while ( ( pci = (LPCMPINFO)GetNext( pci ) ) != NULL )
	{
		// Find match
		if ( ( ( pName == NULL && pci->name == 0 ) || !strcmpi( pci->name, pName ) ) && pci->user == dwUser )
			return pci;

	} // end while
	
	return NULL;
}

DWORD CCmpFile::ExtractFile(LPCTSTR pName, DWORD dwUser, LPCTSTR pFile)
{
	// Find cmp structure
	LPCMPINFO pci = FindFile( pName, dwUser );
	if ( pci == NULL ) return 0;

	// Attempt to extract the data	
	return Extract( pci, pName, pFile );
}

DWORD CCmpFile::ExtractFile(LPCTSTR pName, DWORD dwUser, LPVOID buf, DWORD size)
{
	// Find cmp structure
	LPCMPINFO pci = FindFile( pName, dwUser );
	if ( pci == NULL ) return 0;

	// Attempt to extract the data	
	return Extract( pci, pName, buf, size );
}

DWORD CCmpFile::Extract(LPCMPINFO pci, LPCTSTR pName, LPVOID buf, DWORD size)
{
	// Sanity check
	if ( pci == NULL ) return 0;

	CMPBLOCKINFO cbi;
	ZeroMemory( &cbi, sizeof( cbi ) );

	switch ( pci->type & CMPTYPE_MASK )
	{
		case CMPTYPE_VOID :
		case CMPTYPE_HEADER :
		case CMPTYPE_BIN :
		case CMPTYPE_FILE :
		case CMPTYPE_REG :
		case CMPTYPE_MASK :
		{
			// Find the first block
			if ( !FindBlock( pci, &cbi ) ) return FALSE;

			// Skip if info block
			while ( ( cbi.flags & CMPBFLAG_INFO ) != 0 )
				if ( !GetNextBlock( &cbi ) ) return FALSE;

			// Do they want to know the size?
			if ( buf == NULL ) return cbi.usize;

			// Extract the data to file
			return ExtractInfo( &cbi, buf, size );

		} break;

		case CMPTYPE_DIR :
		{
			// Find the first block
			if ( !FindBlock( pci, &cbi ) ) return FALSE;

			// Skip if info block
			while ( ( cbi.flags & CMPBFLAG_INFO ) != 0 )
				if ( !GetNextBlock( &cbi ) ) return FALSE;

			char msg[ 512 ];
			char fname[ CWF_STRSIZE ];
			char full[ CWF_STRSIZE ];
			char root[ CWF_STRSIZE ];

			// Copy name
			strcpy( root, pci->name );

			// Get the file name list
			HGROUP hGroup = pci->cfg->FindGroup( "Files" );
			if ( hGroup == NULL ) return FALSE;

			for ( DWORD i = 0; i < pci->blocks; i++ )
			{
				// Get file name
				wsprintf( msg, "F%lu", i );
				if ( pci->cfg->GetValue( hGroup, msg, fname, sizeof( fname ) ) )
				{
					// Build full path
					CWinFile::BuildPath( full, root, fname );

					// Is this the guy?
					if ( pName == NULL || !strcmp( pName, full ) )
					{
						// Do they just want to know the size?
						if ( buf == NULL ) return cbi.usize;

						// Extract this file
						ExtractInfo( &cbi, buf, size );

						// We're done
						return TRUE;

					} // end if

				} // end if

				// Get next block
				if ( !GetNextBlock( &cbi ) ) return FALSE;				

			} // end for

		} break;

	} // end switch
		
	return 0;
}

BOOL CCmpFile::Remove(LPCTSTR pName, DWORD dwUser)
{
	LPCMPINFO pci = FindCmp( pName, dwUser );
	if ( pci == NULL ) return FALSE;

	CLList::Delete( pci );

	return TRUE;
}

LPCMPINFO CCmpFile::FindFile( LPCTSTR pFile, DWORD dwUser, LPSTR pLink, LPCMPINFO pLast )
{
	LPCMPINFO pci = pLast;

	// No link yet
	if ( pLink != NULL ) *pLink = 0;

	while( ( pci = (LPCMPINFO)GetNext( pci ) ) != NULL )

		// Verify user type
		if ( pci->user ==  dwUser )
		{
			if ( pci->type != CMPTYPE_DIR )
			{
				// Is this the guy?
				if ( !strcmp( pFile, pci->name ) ) 
				{
					// Copy link
					if ( pLink != NULL ) strcpy( pLink, pci->str );

					return pci;

				} // end if

			} // end if

			else
			{
				char full[ CWF_STRSIZE ];

				HGROUP hGroup = pci->cfg->FindGroup( "Files" );
				if ( hGroup != NULL )
				{
					LPCFGELEMENTINFO pcei = NULL;
					while( ( pcei = pci->cfg->GetNextElement( hGroup, pcei ) ) != NULL )
					{
						CWinFile::BuildPath( full, pci->name, (char*)pcei->value );

						// Is this the guy?
						if ( !strcmp( full, pFile ) ) 
						{
							// Copy link
							if ( pLink != NULL ) BuildPath( pLink, pci->str, (char*)pcei->value );
				
							return pci;
						} // end if

					} // end while

				} // end if
			} // end else if

		} // end if

	return NULL;
}
  

BOOL CCmpFile::InitLookup()
{
	// For each chunk
	LPCMPINFO pci = NULL;
	while( ( pci = (LPCMPINFO)GetNext( pci ) ) != NULL )
	{
		if ( pci->type == CMPTYPE_DIR )
		{
			char full[ CWF_STRSIZE ];

			HGROUP hGroup = pci->cfg->FindGroup( "Files" );
			if ( hGroup != NULL )
			{
				DWORD i = 0;
				CMPBLOCKINFO cbi;
				LPCFGELEMENTINFO pcei = NULL;
				while( ( pcei = pci->cfg->GetNextElement( hGroup, pcei ) ) != NULL )
				{
					CWinFile::BuildPath( full, pci->name, (char*)pcei->value );

					// Add this item
					if ( FindBlock( pci, &cbi, i++ ) )

						// Does it already exist?
						if ( !m_lookup.Find( full ) )

							// Add lookup
							AddLookup( full, cbi.offset );

				} // end while

			} // end if
		} // end else if

		else if ( pci->type == CMPTYPE_WEB )
		{
			HGROUP hGroup = pci->cfg->FindGroup( "Files" );
			if ( hGroup != NULL )
			{
				DWORD i = 1;
				CMPBLOCKINFO cbi;
				LPCFGELEMENTINFO pcei = NULL;
				while( ( pcei = pci->cfg->GetNextElement( hGroup, pcei ) ) != NULL )
				{
					// Add this item
					if ( FindBlock( pci, &cbi, i++ ) )

						// Does it already exist?
						if ( !m_lookup.Find( (char*)pcei->value ) )

							// Add lookup
							AddLookup( (char*)pcei->value, cbi.offset );

				} // end while

			} // end if
		} // end else if

		else if ( pci->type != CMPTYPE_FILE )

			// Does it already exist?
			if ( !m_lookup.Find( pci->name ) )

				// Add lookup
				AddLookup( pci->name, pci->foff );

	} // end while

	return TRUE;
}

BOOL CCmpFile::Get(LPCTSTR pName, TMem< BYTE > &mem )
{
	// Punt if bad param
	if ( pName == NULL ) return FALSE;

	DWORD offset = 0;
	if ( !GetLookup( pName, &offset ) ) return FALSE;

	// Set pointer to start of block
	if ( !SetPtrPosBegin( offset ) ) return FALSE;

	CMPBLOCKINFO cbi;
	if ( !Read( &cbi, sizeof( cbi ) ) ) return FALSE;

	// Verify block id
	if (	cbi.id[ 0 ] != BLOCKID1 || cbi.id[ 1 ] != BLOCKID2 ||
			cbi.id[ 2 ] != BLOCKID3 || cbi.id[ 3 ] != BLOCKID4 )
		return FALSE;

	// Must have size
	if ( cbi.usize == 0 ) return FALSE;

	// Allocate memory
	if ( !mem.allocate( cbi.usize ) ) return FALSE;

	// Move to correct position in file
//	SetPtrPosBegin( offset );

	// Extract the data
	return ExtractInfo( &cbi, mem.ptr(), cbi.usize );
}

BOOL CCmpFile::AddLookup(LPCTSTR pLink, LONGLONG offset)
{
	// Sanity check
	if ( pLink == NULL || *pLink == 0 ) return FALSE;

	CStr str;
	if ( !str.allocate( strlen( pLink ) + 1 ) ) return FALSE;

	// Normalize the link
	NormalizeLink( str, pLink );

	// Add the link
	return m_lookup.New( NULL, (DWORD)offset, str ) != NULL;
}

BOOL CCmpFile::GetLookup(LPCTSTR pLink, LPDWORD poffset)
{
	// Sanity check
	if ( pLink == NULL || *pLink == 0 || poffset == NULL ) 
		return FALSE;

	CStr str;
	if ( !str.allocate( strlen( pLink ) + 1 ) ) return FALSE;

	// Normalize the link
	NormalizeLink( str, pLink );

	// Find block data
	LPLLISTINFO pLi = (LPLLISTINFO)m_lookup.Find( str );
	if ( pLi == NULL ) return FALSE;

	// Save offset
	*poffset = pLi->user;

	return TRUE;	
}

BOOL CCmpFile::NormalizeLink(LPSTR dst, LPCTSTR src)
{
	DWORD x = 0, y = 0;

	// Skip leading \/
	while ( src[ x ] == '/' || src[ x ] == '\\' ) x++;

	// Copy string
	while ( src[ x ] != 0 )
	{
		// Save byte
		dst[ y++ ] = src[ x++ ];

		// Skip extra seps
		while ( ( src[ x ] == '/' || src[ x ] == '\\' ) &&
				( src[ x + 1 ] == '/' || src[ x + 1 ] == '\\' ) ) x++;

	} // end while

	// Lost trailing chars
	while ( y && ( dst[ y - 1 ] == '/' || dst[ y - 1 ] == '\\' ) ) y--;

	// NULL terminate
	dst[ y ] = 0;

	return TRUE;
}

BOOL CCmpFile::SaveManifest()
{
	// Get current file pointer
	SetPtrPosEnd( 0 );
	LONGLONG startptr = GetPtrPos();

	// Lose the old manifest
	m_manifest.Destroy();

	DWORD i = 0;
	char name[ 256 ];

	// Add md5's for all chunks
	LPCMPINFO pci = NULL;
	while ( ( pci = (LPCMPINFO)GetNext( pci ) ) != NULL )
	{
		if ( ( pci->type & CMPTYPE_MASK ) == CMPTYPE_DIR ||
			 ( pci->type & CMPTYPE_MASK ) == CMPTYPE_WEB )
		{
			// Set pointer to start of data
			if ( !SetPtrPosBegin( pci->foff ) ) return FALSE;

			DWORD			read;
			CMPCHUNKINFO	ci;

			// Read chunk info
			if ( Read( &ci, sizeof( ci ), &read ) || read != sizeof( ci ) )
			{

				// Verify chunk id
				if (	ci.id[ 0 ] == CHUNKID1 && ci.id[ 1 ] == CHUNKID2 &&
						ci.id[ 2 ] == CHUNKID3 && ci.id[ 3 ] == CHUNKID4 )
				{

					HGROUP hFiles = pci->cfg->FindGroup( "Files" );
					CMPBLOCKINFO cbi;
					while ( GetNextBlock( &cbi ) )
					{
						TMem< BYTE > mem;
						if ( mem.allocate( cbi.csize ) )
						{
							// Read in the 
							if ( Read( mem.ptr(), cbi.csize ) )
							{
								CMD5 md5;

								md5.Transform( mem.ptr(), cbi.csize );
								md5.End();

								// Get file name
								char fname[ CWF_STRSIZE ];
								wsprintf( name, "F%lu", cbi.index );
								if ( !pci->cfg->GetValue( hFiles, name, fname, sizeof( fname ) ) )
									wsprintf( name, "Gr%lu", i++ );

								HGROUP hGroup = m_manifest.AddGroup( fname );
								if ( hGroup != NULL )
								{	m_manifest.SetValue( hGroup, "md5", md5.GetHash(), md5.GetHashLen() );
									m_manifest.SetValue( hGroup, "off", (DWORD)pci->foff );
									m_manifest.SetValue( hGroup, "osz", (DWORD)pci->size );
									m_manifest.SetValue( hGroup, "type", (DWORD)2 );
								} // end if

							} // end if

						} // end if

					} // end while

				} // end if

			} // end if

		} // end if

		else
		{
			TMem< BYTE > mem;
			if ( mem.allocate( (DWORD)pci->size ) )
			{
				// Start of data
				SetPtrPosBegin( pci->foff );

				// Read in the 
				if ( Read( mem.ptr(), (DWORD)pci->size ) )
				{
					CMD5 md5;

					md5.Transform( mem.ptr(), (DWORD)pci->size );
					md5.End();

//					wsprintf( name, "Gr%lu", i++ );
//					HGROUP hGroup = m_manifest.AddGroup( name );
					HGROUP hGroup = m_manifest.AddGroup( pci->str );
					if ( hGroup != NULL )
					{	m_manifest.SetValue( hGroup, "md5", md5.GetHash(), md5.GetHashLen() );
						m_manifest.SetValue( hGroup, "off", (DWORD)pci->foff );
						m_manifest.SetValue( hGroup, "osz", (DWORD)pci->size );
						m_manifest.SetValue( hGroup, "type", (DWORD)1 );
					} // end if

				} // end if

			} // end if
		} // end if


	} // end while

	// Save chunk info
	CMPCHUNKINFO	ci;
	ZeroMemory( &ci, sizeof( ci ) );

	// Fill in size of chunk information
	ci.sz = sizeof( ci );

	// Save chunk id's
	ci.id[ 0 ] = CHUNKID1;
	ci.id[ 1 ] = CHUNKID2;
	ci.id[ 2 ] = CHUNKID3;
	ci.id[ 3 ] = CHUNKID4;

	ci.type = CMPTYPE_MANIFEST;
	ci.blocks = 0;

	// Have to fill this in later
	ci.size = 0;

	// Restore the pointer position
	SetPtrPosBegin( startptr );

	// Write out the information
	if ( !Write( &ci, sizeof( ci ) ) ) 
	{	SetPtrPosBegin( startptr );
		return FALSE;
	} // end if		

	// Save header information
	SaveCfg( &m_manifest );

	// Get the chunk size
	LONGLONG endptr = GetPtrPos();
	SetPtrPosBegin( startptr );

	// Add new information
	ci.size = endptr - startptr;
	if ( !Write( &ci, sizeof( ci ) ) ) 
	{	SetPtrPosBegin( startptr );
		return FALSE;
	} // end if
					
	// Save the manifest offset
	m_index.manifest = startptr;

	// Set pointer to start of index
	SetPtrPosBegin( strlen( FILEID ) );

	// Write the index
	Write( &m_index, sizeof( m_index ) );
							 
	// Set pointer back to end of data
	SetPtrPosBegin( endptr );

	return TRUE;
}

BOOL CCmpFile::VerifyMD5()
{

	return TRUE;
}

BOOL CCmpFile::LoadManifest(LPCTSTR pFile, CCfgFile *pManifest)
{
	// Need to open the file?
	if ( pFile != NULL ) 
		if ( !OpenExisting( pFile, GENERIC_READ ) ) return FALSE;

	// Load manifest
	if ( pManifest == NULL )
	{	pManifest = &m_manifest;
		m_manifest.Destroy();
	} // end if

	// Set pointer to start of index
	SetPtrPosBegin( strlen( FILEID ) );

	// Write the index
	if ( !Read( &m_index, sizeof( m_index ) ) ) return FALSE;

	// Set file pointer to start of manifest
	SetPtrPosBegin( m_index.manifest );

	// Read chunk
	CMPCHUNKINFO ci;
	if ( !Read( &ci, sizeof( ci ) ) ) return FALSE;

	// Verify chunk
	if (	ci.id[ 0 ] != CHUNKID1 || ci.id[ 1 ] != CHUNKID2 ||
			ci.id[ 2 ] != CHUNKID3 || ci.id[ 3 ] != CHUNKID4 )
		return FALSE;

	CMPBLOCKINFO cbi;
	if ( !Read( &cbi, sizeof( cbi ) ) ) return FALSE;

	// Verify block id
	if (	cbi.id[ 0 ] != BLOCKID1 || cbi.id[ 1 ] != BLOCKID2 ||
			cbi.id[ 2 ] != BLOCKID3 || cbi.id[ 3 ] != BLOCKID4 )
		return FALSE;

	// Must have size
	if ( cbi.usize == 0 ) return FALSE;

	// Allocate memory
	TMem< BYTE > mem;
	if ( !mem.allocate( cbi.usize ) ) return FALSE;

	// Extract the data
	if ( !ExtractInfo( &cbi, mem.ptr(), cbi.usize ) )
		return FALSE;

	// Load the cfg file
	return pManifest->LoadFromMem( mem.ptr(), cbi.usize );
}

#endif // ENABLE_ZLIB
