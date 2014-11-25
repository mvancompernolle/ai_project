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
// Mime.cpp: implementation of the CMime class.
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


CMime::CMime()
{_STT();
	m_header = NULL;

	m_dwSize = 0;
	m_pHead = NULL;
	m_pTail = NULL;

	m_dwAttachments = 0;
	m_dwExtras = 0;

	m_dwPriority = 3;
}

CMime::~CMime()
{_STT();
	Destroy();
}

void CMime::Destroy()
{_STT();
	m_header = NULL;

	// Lose the data
	m_dwSize = 0;
	m_pTail = NULL;
	while ( m_pHead != NULL )
	{
		LPMIMEBLOCK next = m_pHead->pNext;

		// Lose block data
		if ( m_pHead->pdata != NULL )
		{	delete [] m_pHead->pdata;
			m_pHead->pdata = NULL;
		} // end if

		// Lose the descriptor
		delete m_pHead;
		m_pHead = next;

	} // end while	

	m_dwAttachments = 0;
	m_dwExtras = 0;
}

LPMIMEBLOCK CMime::AddBlock(LPMIMEBLOCK node)
{_STT();
	if ( node == NULL ) return NULL;

	// Count an attachment
	if ( ( node->f1 & MBF1_ATTACHMENT ) != 0 )
		m_dwAttachments++;

	// Is it an extra file?
	else if ( *node->fname ) 
		node->f1 |= MBF1_EXTRA, m_dwExtras++;

	else if ( *node->ctype )
		if ( strnicmp( node->ctype, "multipart/", strlen( "multipart/" ) ) != 0 &&
			 strnicmp( node->ctype, "text/", strlen( "text/" ) ) != 0 )
			node->f1 |= MBF1_EXTRA, m_dwExtras++;

	// Decode the data
	DecodeBlock( node );

	// Add this node to the list
	node->pNext = NULL;
	node->pPrev = m_pTail;
	if ( m_pTail == NULL ) m_pHead = node;
	else m_pTail->pNext = node;
	m_pTail = node;
	m_dwSize++;	

	return node;
}

BOOL CMime::RemoveBlock(LPMIMEBLOCK node)
{_STT();
	if ( node == NULL ) return FALSE;

	// Remove this node from the list
	if ( node->pPrev != NULL ) node->pPrev->pNext = node->pNext;
	if ( node->pNext != NULL ) node->pNext->pPrev = node->pPrev;
	if ( m_pHead == node ) m_pHead = node->pNext;			
	if ( m_pTail == node ) m_pTail = node->pPrev;		
	
	// Strike one attachment
	if ( ( node->f1 & MBF1_ATTACHMENT ) != 0 )
		if ( m_dwAttachments ) m_dwAttachments--;

	// Lose block data
	if ( node->pdata != NULL )
	{	delete [] node->pdata;
		node->pdata = NULL;
	} // end if

	// Delete this node
	delete node;

	// Decrement the count
	if ( m_dwSize ) m_dwSize--;

	return TRUE;
}

BOOL CMime::Load(LPCTSTR pFile)
{_STT();
	// Out with the old
	Destroy();

	CWinFile	file;

	// Open the file
	if ( !file.OpenExisting( pFile, GENERIC_READ ) )
		return FALSE;

	// Get file size
	DWORD size = file.Size();
	if ( size == 0 ) return FALSE;

	// Allocate memory
	TMem< BYTE > buf;
	if ( !buf.allocate( size + 1 ) ) return FALSE;

	// Read in the data into ram
	DWORD read;
	if ( !file.Read( buf, size, &read ) || read != size )
		return FALSE;
	buf[ size ] = 0;

	// Load the file
	if ( !LoadFromMemory( buf, size ) )
		return FALSE;

	return TRUE;
}

BOOL CMime::LoadFromMemory(LPBYTE buf, DWORD size)
{_STT();
	Destroy();

	DWORD i = 0;

	// Sanity check
	if ( buf == NULL || size == 0 ) return FALSE;

	// Read all MIME blocks (up to 100)
	DWORD max = 100;
	while( --max && i < size && ReadBlock( &buf[ i ], size - i, &i ) );	

	return TRUE;
}


BOOL CMime::ReadBlock(LPBYTE buf, DWORD size, LPDWORD bsize)
{_STT();
	BOOL endvars = FALSE;
	DWORD i = 0;
	char token[ MIME_STRSIZE ];

	// Allocate memory for block header
	LPMIMEBLOCK node = new MIMEBLOCK;
	if ( node == NULL ) return FALSE;
	ZeroMimeBlock( node );

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
			if ( i >= size ) { delete node; return FALSE; }

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

				// +++
				e--; 

				// Lose trailing white space
				while ( e > i && ( buf[ e ] <= ' ' || buf[ e ] > '~' ) ) e--;

				if ( e > i ) 
				{
					// Add this variable
					node->var.AddVar( token, (LPVOID)&buf[ i ], e - i + 1 );

				} // end if

				// Encoding type
				if ( !strcmpi( token, "Content-Type" ) )
				{
					// Save the content-type
					DWORD c;
					for ( c = 0; c < sizeof( node->ctype ) && 
										buf[ i + c ] > ' ' && 
										buf[ i + c ] != ';'; c++ )
						node->ctype[ c ] = buf[ i + c ];
					node->ctype[ c ] = 0;

					if ( buf[ i + c ] == ';' ) c++;

					// Skip to next var
					while( ( i + c ) < size && 
							( buf[ i + c ] <= ' ' || buf[ i + c ] > '~' ) ) c++;

					if ( e > ( i + c ) )
					{
						// Is their a boundry string?
						CCfgFile	cfg;
						cfg.SetTerminator( ';' );
						cfg.LoadFromMem( &buf[ i + c ], e - ( i + c ) );

						// Get file name if specified
						cfg.GetValue( (LPCTSTR)NULL, "name", node->fname, sizeof( node->fname ) );

						// Get boundry string
						cfg.GetValue(	(LPCTSTR)NULL, "boundary", 
										(LPSTR)&node->boundry, sizeof( node->boundry ) );
					} // end if

				} // end if

				// Encoding type
				if ( !strcmpi( token, "Content-Transfer-Encoding" ) )
				{
					if ( !strnicmp( (LPCTSTR)&buf[ i ], "base64", strlen( "base64" ) ) )
						node->encode |= MBEN_BASE64;
					else if ( !strnicmp( (LPCTSTR)&buf[ i ], "quoted-printable", strlen( "quoted-printable" ) ) )
						node->encode |= MBEN_QP;
				} // end if

				// Attachment?
				if ( !strcmpi( token, "Content-Disposition" ) )
				{
					if ( !strnicmp( (LPCTSTR)&buf[ i ], "attachment", strlen( "attachment" ) ) )
						node->f1 |= MBF1_ATTACHMENT;

					CCfgFile	cfg;
					cfg.LoadFromMem( &buf[ i ], e - i );

					// Get filename
					cfg.GetValue( (LPCTSTR)NULL, "filename", node->fname, sizeof( node->fname ) );
					
				} // end if

				// cid:
				if ( !strcmpi( token, "Content-ID" ) )
				{
					DWORD s = i;
					while( s < e && buf[ s ] != '<' ) s++;

					// Copy the cid
					if ( buf[ s ] == '<' )
					{	DWORD x = 0;
						s++;
						while( s < e && buf[ s ] != '>' ) node->cid[ x++ ] = buf[ s++ ];
						node->cid[ x ] = 0;
					} // end if
					
				} // end if

				// Forge ahead
				i = skip;

			} // end if

			// Skip to next line
			else while ( i < size && buf[ i ] != 0x0d && buf[ i ] != 0x0a ) i++;

		} // end if

		else endvars = TRUE;

	} // end while	

	// Skip next CRLF
	if ( buf[ i ] == 0x0d ) i++;
	if ( buf[ i ] == 0x0a ) i++;

	// Save header if non attached multipart
	if (	m_header != NULL &&
			( node->f1 & MBF1_ATTACHMENT ) == 0 &&
			!strnicmp( node->ctype, "multipart/", strlen( "multipart/" ) ) )
		i = 0;

	// Was this the header info?
	if ( m_header == NULL ) m_header = node;

	DWORD done = FALSE;
	DWORD start = i;
	DWORD end = i;
	DWORD blen = strlen( m_header->boundry );
	while( !done && i < size )
	{
		// This may be the end
		end = i;

		// Check for end tag ( some of this here for efficiency )
		if (	buf[ i ] == '-' && ( i + 1 + blen ) < size && 
				buf[ i + 1 ] == m_header->boundry[ 0 ] &&
				!strnicmp( (LPCTSTR)&buf[ i + 1 ], m_header->boundry, blen ) )
		{
			// Forget this line
			while ( end > start && buf[ end ] != '\r' && buf[ end ] != '\n' ) end--;

			if ( bsize != NULL )
				*bsize += i + 1 + strlen( m_header->boundry );
			done = TRUE;
		} // end if	

		if ( !done ) i++;

	} // end while

	// Attempt to allocate memory for block
	node->dsize = end - start + 1;
	node->pdata = new BYTE[ node->dsize + 1 ];
	if ( node->pdata == NULL ) { delete node; return done; }
	memcpy( node->pdata, &buf[ start ], node->dsize );

	// Attempt to add the block to the list
	if ( !AddBlock( node ) ) delete node;

	return done;
}


BOOL CMime::SaveToFile(LPMIMEBLOCK pmb, LPCTSTR pFile)
{_STT();
	// Sanity check
	if ( pmb == NULL || pFile == NULL ) return FALSE;
	if ( pmb->pdata == NULL || pmb->dsize == 0 ) return FALSE;

	if( ( pmb->f1 & MBF1_DECODED ) == 0 )
	{
		// If encoding
		if ( ( pmb->encode & MBEN_BASE64 ) != 0 )
			return SaveBase64( pmb, pFile );
	} // end if

	// Open file
	CWinFile f;
	if ( !f.OpenNew( pFile ) ) return FALSE;

	// Just write out if no encoding
	f.Write( pmb->pdata, pmb->dsize );
	f.Close();

	return TRUE;
}

BOOL CMime::SaveBase64(LPMIMEBLOCK pmb, LPCTSTR pFile)
{_STT();
	// Open file
	CWinFile f;
	if ( !f.OpenNew( pFile ) ) return FALSE;

	DWORD	i = 0;
	BYTE	buf[ 1024 ];
	DWORD	size = sizeof( buf ), decoded;
	DWORD	done = 0;

	// Decode to file
	while( !done && i < pmb->dsize )
	{
		DWORD block = 1024 < ( pmb->dsize - i ) ? 1024 : pmb->dsize - i;

		if ( !CBase64::Decode( &( (LPCTSTR)pmb->pdata )[ i ], &block, buf, &decoded, &done ) )
			done = TRUE;

		// Write data to file
		if ( decoded ) f.Write( buf, decoded );

		// Increment buffer pointer
		i += block;

	} // end while

	// Close the file
	f.Close();

	return TRUE;
}

BOOL CMime::SaveAttachments(LPCTSTR pDir)
{_STT();
	char fname[ MIME_STRSIZE ];

	LPMIMEBLOCK		pmb = NULL;

	while ( ( pmb = GetNext( pmb ) ) != NULL )
	{
		if ( ( pmb->f1 & MBF1_ATTACHMENT ) != 0 )
		{
			// Name the nameless
			if ( !pmb->fname[ 0 ] ) 
			{
				char ext[ 256 ];

				// Get content extension
				if ( GetContentExtension( pmb->ctype, ext ) )
				{	strcpy( pmb->fname, "Attachment." ); strcat( pmb->fname, ext ); }

				// If message we'll assume outlook can handle it
				else if ( !strnicmp( pmb->ctype, "message/", sizeof( "message/" ) ) )
					strcpy( pmb->fname, "Attachment.dat" );

			} // end if

			if ( pmb->fname[ 0 ] )
			{
				if ( pDir != NULL ) CWinFile::BuildPath( fname, pDir, pmb->fname );
				else strcpy( fname, pmb->fname );

				// Ensure unique file name
				CWinFile::GetUniqueFileName( fname, fname );

				// Ensure directory exists
				char path[ MIME_STRSIZE ];
				CWinFile::GetPathFromFileName( fname, path );
				CWinFile::CreateDirectory( path );

				// Copy new file name
				CWinFile::GetFileNameFromPath( fname, pmb->fname );

				// Save to disk
				SaveToFile( pmb, fname );

			} // end if

		} // end if

	} // end while

	return TRUE;
}

// files that are not attachments
BOOL CMime::SaveExtras(LPCTSTR pDir)
{_STT();
	char fname[ MIME_STRSIZE ];

	LPMIMEBLOCK		pmb = NULL;

	while ( ( pmb = GetNext( pmb ) ) != NULL )
	{
		if ( ( pmb->f1 & MBF1_ATTACHMENT ) == 0 )
		{
			// Name the nameless
			if ( !pmb->fname[ 0 ] ) 
			{
				char ext[ 256 ];

				// Get content extension
				if ( GetContentExtension( pmb->ctype, ext ) )
				{	strcpy( pmb->fname, "Extra." ); strcat( pmb->fname, ext ); }

				// If message we'll assume outlook can handle it
				else if ( !strnicmp( pmb->ctype, "message/", strlen( "message/" ) ) )
					strcpy( pmb->fname, "Extra.dat" );

			} // end if

			if ( pmb->fname[ 0 ] )
			{
				if ( pDir != NULL ) CWinFile::BuildPath( fname, pDir, pmb->fname );
				else strcpy( fname, pmb->fname );

				// Ensure unique file name
				CWinFile::GetUniqueFileName( fname, fname );

				// Ensure directory exists
				char path[ MIME_STRSIZE ];
				CWinFile::GetPathFromFileName( fname, path );
				CWinFile::CreateDirectory( path );

				// Copy new file name
				CWinFile::GetFileNameFromPath( fname, pmb->fname );

				// Save to disk
				SaveToFile( pmb, fname );

			} // end if

		} // end if

	} // end while

	return TRUE;
}

BOOL CMime::DecodeQP(LPCTSTR pSrc, DWORD dwSrc, LPSTR pDst, LPDWORD pdwDst)
{_STT();
	// Sanity checks
	if ( pSrc == NULL || dwSrc == 0 ) return FALSE;
	if ( pDst == NULL || pdwDst == NULL || *pdwDst == 0 ) return FALSE;
	
	DWORD x = 0, y = 0;

	while ( x < dwSrc )
	{
		// Save if normal character
		if ( pSrc[ x ] != '=' )
		{
			if ( y < *pdwDst ) pDst[ y++ ] = pSrc[ x++ ];
			else x = dwSrc;
		} // end if

		else
		{
			// Skip '='
			x++;

			// Check for hex encoding
			if (	( pSrc[ x ] >= '0' && pSrc[ x ] <= '9' ) ||
					( pSrc[ x ] >= 'A' && pSrc[ x ] <= 'F' ) ||
					( pSrc[ x ] >= 'a' && pSrc[ x ] <= 'f' ) )
			{
				pDst[ y++ ] = (BYTE)ahtoud( &pSrc[ x ], 2 );
				x += 2;
			} // end if

			else while ( x < dwSrc && 
						 ( pSrc[ x ] <= ' ' || pSrc[ x ] > '~' ) ) x++;

		} // end else

	} // end while

	// NULL terminate
	pDst[ y ] = 0;

	// How many chars did we copy?
	*pdwDst = y;

	return TRUE;
}

DWORD CMime::ahtoud( LPCTSTR pBuffer, BYTE ucBytes )
{_STT();
	DWORD num, i;
                                              
	num = 0; 

	// For Each ASCII Digit
	for ( i = 0; i < ucBytes; i++ )
	{
		// Convert ASCII Digit Between 0 And 9
		if ( pBuffer[ i ] >= '0' && pBuffer[ i ] <= '9' )
			num = ( num << 4 ) + ( pBuffer[ i ] - '0' );
		
		// Convert ASCII Digit Between A And F
		else if ( pBuffer[ i ] >= 'A' && pBuffer[ i ] <= 'F' )
			num = ( num << 4 ) + ( pBuffer[ i ] - 'A' ) + 10;

		// Convert ASCII Digit Between a And f
		else if ( pBuffer[ i ] >= 'a' && pBuffer[ i ] <= 'f' )
			num = ( num << 4 ) + ( pBuffer[ i ] - 'a' ) + 10;

	} // end for
    return num;	

} // end ahtou()

BOOL CMime::DecodeBlock(LPMIMEBLOCK pmb)
{_STT();
	// Is it already decoded?
	if ( ( pmb->f1 & MBF1_DECODED ) != 0 ) return TRUE;

	// If base64 encoded
	if ( ( pmb->encode & MBEN_BASE64 ) != 0 )
	{
		DWORD done = FALSE;
		DWORD decoded = 0;
		DWORD size = pmb->dsize;

		TMem< BYTE > buf;
		if ( !buf.allocate( size + 1 ) ) return FALSE;

		// Do the decode
		if ( CBase64::Decode(	(LPCTSTR)pmb->pdata, &size, 
								buf, &decoded, &done ) && decoded > 0 )
		{
			// Allocate buffer of correct size
			LPBYTE cbuf = new BYTE[ decoded + 1 ];
			if ( cbuf == NULL ) return FALSE;

			// Copy data
			memcpy( cbuf, buf, decoded );

			// Save decoded data
			delete [] pmb->pdata;
			pmb->pdata = cbuf;
			pmb->dsize = decoded;			
			pmb->f1 |= MBF1_DECODED;

		} // end if

	} // end if

	// If quoted-printable encoded
	else if ( ( pmb->encode & MBEN_QP ) != 0 )
	{
		DWORD size = pmb->dsize;
		TMem< char > buf;
		if ( !buf.allocate( size + 1 ) ) return FALSE;

		if ( DecodeQP( (LPCTSTR)pmb->pdata, pmb->dsize, buf, &size ) )
		{
			// Allocate buffer of correct size
			LPBYTE cbuf = new BYTE[ size + 1 ];
			if ( cbuf == NULL ) return FALSE;

			// Copy data
			memcpy( cbuf, buf, size );

			// Save decoded data
			delete [] pmb->pdata;
			pmb->pdata = cbuf;
			pmb->dsize = size;
			pmb->f1 |= MBF1_DECODED;
		} // end if				

	} // end else if

	return TRUE;
}


BOOL CMime::ReplaceCid(LPCTSTR pCid, LPCTSTR pReplace, LPCTSTR buf, DWORD size, LPSTR dest)
{_STT();
	DWORD i = 0, x = 0;
	DWORD cidlen = strlen( pCid );

	TMem< char > cid;
	if ( !cid.allocate( cidlen + 32 ) ) return FALSE;

	// Fix up cid
	strcpy( cid, "cid:" );
	strcat( cid, pCid );
	cidlen = strlen( cid );

	// Simple replace routine
	while( i < size )
	{
		if ( strnicmp( cid, &buf[ i ], cidlen ) ) dest[ x++ ] = buf[ i++ ];
		else
		{	strcpy( &dest[ x ], pReplace );
			x += strlen( pReplace );
			i += cidlen;
		} // end else

	} // end while
	dest[ x ] = 0;

	return TRUE;
}

BOOL CMime::ReplaceCid(LPMIMEBLOCK pmb, LPCTSTR pPath )
{_STT();
	if ( pmb == NULL || pmb->pdata == NULL ) return FALSE;
	
	LPMIMEBLOCK	extra = NULL;

	while ( ( extra = GetNext( extra ) ) != NULL )
	{
		if (	/*( extra->f1 & MBF1_ATTACHMENT ) == 0 && */
				extra->fname[ 0 ] != 0 && extra->cid[ 0 ] != 0 )	 
		{
			char *buf = new char[ pmb->dsize * 2 ];
			if ( buf == NULL ) return FALSE;

			char fname[ CWF_STRSIZE ];
			if ( pPath != NULL ) CWinFile::BuildPath( fname, pPath, extra->fname );
			else strcpy( fname, extra->fname );

			ReplaceCid( extra->cid, fname, (LPCTSTR)pmb->pdata, pmb->dsize, buf );

			delete [] pmb->pdata;
			pmb->pdata = buf;
			pmb->dsize = strlen( buf );

		} // end if

	} // end while

	return TRUE;
}

DWORD CMime::m_dwCounter = 0;

BOOL CMime::GenBoundry(LPSTR pBoundry)
{_STT();
	CMD5	md5;

	if ( pBoundry == NULL ) return FALSE;

	SYSTEMTIME st;
	GetLocalTime( &st );

	// Get random hash
	md5.Random( "Mime (October 4, 2002)" );

	wsprintf( pBoundry, "----_=_NextPart_%03lX_%04lX_%08lX.%08lX.%08lX%08lX",
				m_dwCounter++, 
				( ( st.wYear % 16 ) * 0x1000 ) + ( st.wMonth * 0x100 ) + st.wDay,
				md5.m_hash[ 0 ], md5.m_hash[ 1 ], md5.m_hash[ 2 ], md5.m_hash[ 3 ] );

	return TRUE;
}

static char s_days[][ 8 ] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char s_months[][ 8 ] = {	"Inv",	"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
								"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

LPMIMEBLOCK CMime::Create( LPCTSTR pTo, LPCTSTR pFrom, LPCTSTR pSubject )
{_STT();
	// Lose old mime message
	Destroy();

	// Allocate memory for header
	LPMIMEBLOCK node = new MIMEBLOCK;
	if ( node == NULL ) return NULL;
	ZeroMimeBlock( node );

	// Header is always first in the list
	m_header = node;

	// Fill in header information
	strcpy_sz( m_header->ctype, "multipart/mixed" );
	
	// Get boundry type
	GenBoundry( m_header->boundry );

	// Add user information

	// Copy from email list
	if ( pFrom != NULL && *pFrom != 0 ) 
	{	TMem< char > buf;
		if ( buf.allocate( ( strlen( pFrom ) * 2 ) + 1 ) )
		{	VerifyEmailList( buf, pFrom, ",\r\n\t" ); 
			m_header->var.AddVar( "From", buf );
		} // end if
	} // end if

	// Copy to email list
	if ( pTo != NULL && *pTo != 0 ) 
	{	TMem< char > buf;
		if ( buf.allocate( ( strlen( pTo ) * 2 ) + 1 ) )
		{	VerifyEmailList( buf, pTo, ",\r\n\t" ); 
			m_header->var.AddVar( "To", buf );
		} // end if
	} // end if

	// Copy subject
	if ( pSubject != NULL ) m_header->var.AddVar( "Subject", pSubject );

	// add time/date
		SYSTEMTIME st;
		GetLocalTime( &st );

		// Get time-zone information
		TIME_ZONE_INFORMATION tzi;
		ZeroMemory( &tzi, sizeof( tzi ) );
		GetTimeZoneInformation( &tzi );

		// Change to correct format
		long hour = tzi.Bias / 60;
		if ( tzi.Bias < 0 ) tzi.Bias = -tzi.Bias;
		long min = tzi.Bias % 60;
		if ( hour <= 12 ) hour = -hour;
		else if ( hour > 12 ) hour = 24 - hour;

		char tzone[ MIME_STRSIZE ] = { 0 };
		if ( hour < 0 ) wsprintf( tzone, "%03li%02lu", hour, min );
		else wsprintf( tzone, "+%02li%02lu", hour, min );

		char date[ MIME_STRSIZE ] = { 0 };
		wsprintf( date, "%s, %lu %s %lu %02lu:%02lu:%02lu %s",
						s_days[ st.wDayOfWeek ],
						st.wDay,
						s_months[ st.wMonth ],
						st.wYear,
						st.wHour, st.wMinute, st.wSecond,
						tzone );

		char tzname[ MIME_STRSIZE ];
		wcstombs( tzname, tzi.StandardName, sizeof( tzname ) - 1 );

		m_header->var.AddVar( "Date", date );

	// end add time/date

	// Mime version
	m_header->var.AddVar( "MIME-Version", "1.0" );

	char type[ MIME_STRSIZE ];
	wsprintf( type, "multipart/mixed;\r\n\tboundary=\"%s\"", m_header->boundry );
 	m_header->var.AddVar( "Content-Type", type );

	// Add X- MIME Data

	m_header->var.AddVar( "X-Time-Zone", tzname );

	char pri[ MIME_STRSIZE ];
	wsprintf( pri, "%lu", m_dwPriority );
	m_header->var.AddVar( "X-Priority", pri );

	m_header->var.AddVar( "X-MIME-Encoder", "WheresJames MIME Encoder (www.wheresjames.com) " MIMEENCODERVERSTR );

	// Default message
	char *msg = "This message is in MIME format. Since your mail\r\n"
				"reader does not understand this format, some or\r\n"
				"all of this message may not be legible.";

	// Add message to header
	m_header->dsize = strlen( msg );
	m_header->pdata = new char[ m_header->dsize + 1 ];
	if ( m_header->pdata != NULL )
	{	memcpy( m_header->pdata, msg, m_header->dsize );
		( (LPBYTE)m_header->pdata )[ m_header->dsize ] = 0;
	} // end if
	else m_header->dsize = 0;

	// Add the node to the list
	return AddBlock( node );
}


BOOL CMime::Save(LPCTSTR pFile)
{_STT();
	CWinFile f;

	// Attempt to open the new file
	if ( !f.OpenNew( pFile ) ) return FALSE;

	// How much data
	DWORD size = SaveToMem( NULL, 0 );
	if ( size == 0 ) return TRUE;

	// Allocate memory
	TMem< BYTE > buf;
	if ( !buf.allocate( size + 1 ) ) return FALSE;

	// Write MIME to buffer
	DWORD bytes = SaveToMem( buf, size );

	// Write out the data to disk file
	f.Write( buf, bytes );

	return TRUE;
}

DWORD CMime::SaveToMem(LPBYTE buf, DWORD size)
{_STT();
	// Anything to save?
	if ( Size() == 0 ) return 0;
	if ( m_header == NULL ) return 0;

	DWORD i = 0;	
	LPMIMEBLOCK pmb = NULL;

	// Do they just want to know the size?
	if ( buf == NULL ) size = MAXDWORD;

	// For each block
	while ( i < size && ( pmb = GetNext( pmb ) ) != NULL )
	{
		RULIB_TRY
		{
			if ( size > i )
			{
				// Get size
				if ( buf == NULL ) i += SaveBlock( pmb, 0, 0 );

				// Save block information to buffer
				else if ( i < size ) i +=  SaveBlock( pmb, &buf[ i ], size - i );

			} // end if

		} // end try

		// Ignore bad blocks
		RULIB_CATCH_ALL { ASSERT( 0 ); }

		// Will there be another block?
		// If so write out the boundry
//		if ( pmb->pNext != NULL && m_header != NULL )
		{
			if ( buf != NULL ) strcpy( (char*)&buf[ i ], "\r\n\r\n--" );
			i += 6;

			if ( buf != NULL ) strcpy( (char*)&buf[ i ], m_header->boundry );
			i += strlen( m_header->boundry );

			// Be like Outlook
			if ( pmb->pNext != NULL )
			{	if ( buf != NULL ) strcpy( (char*)&buf[ i ], "\r\n" );
				i += 2;
			} // end if
			else
			{	if ( buf != NULL ) strcpy( (char*)&buf[ i ], "--\r\n" );
				i += 4;
			} // end else

		} // end if

	} // end while

	// NULL terminate
	if ( buf != NULL )
	{	if ( i < size ) buf[ i++ ] = 0; }
	else i++;

	return i;
}


DWORD CMime::SaveBlock( LPMIMEBLOCK pmb, LPBYTE buf, DWORD size)
{_STT();
	// Sanity checks
	if ( pmb == NULL ) return 0;

	DWORD	i = 0;
	LPVAR	pvar = NULL;

	// Write out variables
	while( ( pvar = (LPVAR)pmb->var.GetNext( pvar ) ) != NULL )
	{
		// Save the name
		if ( buf != NULL ) strcpy( (char*)&buf[ i ], pvar->name );
		i += strlen( pvar->name );

		// Separator
		if ( buf != NULL ) buf[ i++ ] = ':', buf[ i++ ] = ' ';
		else i += 2;

		if ( pvar->size == 0 )
		{
			char num[ MIME_STRSIZE ];
			wsprintf( num, "%lu", (DWORD)pvar->val );
			if ( buf != NULL ) strcpy( (char*)&buf[ i ], num );
			i += strlen( num );
		} // end if
		else
		{
			// Copy variable data
			if ( buf != NULL ) memcpy( &buf[ i ], pvar->val, pvar->size );
			i += pvar->size;

		} // end else		

		// Next line
		if ( buf != NULL ) strcpy( (char*)&buf[ i ], "\r\n" );
		i += 2;

	} // end while

	// End of variables
	if ( buf != NULL ) strcpy( (char*)&buf[ i ], "\r\n" );
	i += 2;

	// Is there any block data?
	if ( pmb->pdata != NULL && pmb->dsize > 0 )
	{
		// Was it decoded?
		if ( ( pmb->f1 & MBF1_DECODED ) == 0 )
		{
			// Copy block data
			if ( buf != NULL ) memcpy( &buf[ i ], pmb->pdata, pmb->dsize );
			i += pmb->dsize;
		} // end if

		// Base-64 encoding
		else if ( ( pmb->encode & MBEN_BASE64 ) != 0 )
		{
			if ( buf != NULL && size > i ) 
				i += CBase64::Encode( (char*)&buf[ i ], size - i, (LPBYTE)pmb->pdata, pmb->dsize );
			else i += CBase64::GetEncodedSize( pmb->dsize ), i += 4;

		} // end if

		// Quoted Printable encoding
		else if ( ( pmb->encode & MBEN_QP ) != 0 )
		{

		} // end if

		// Un-encoded
		else
		{
			// Copy block data
			if ( buf != NULL ) memcpy( &buf[ i ], pmb->pdata, pmb->dsize );
			i += pmb->dsize;

		} // end if
		
	} // end if

	return i;
}

LPMIMEBLOCK CMime::AddPlainText(LPCTSTR pText, DWORD dwSize)
{_STT();
	if ( pText == NULL ) return NULL;
	if ( dwSize == 0 ) dwSize = strlen( pText );
	if ( dwSize == 0 ) return NULL;
							  
	// Allocate memory
	LPMIMEBLOCK node = new MIMEBLOCK;
	if ( node == NULL ) return NULL;
	ZeroMimeBlock( node );

	// Plain text
	strcpy( node->ctype, "text/plain" );
	node->var.AddVar( "Content-Type", "text/plain;\r\n\tcharset=\"iso-8859-1\"" );
	node->var.AddVar( "Content-Transfer-Encoding", "7bit" );

	// Allocate memory for text
	node->pdata = new char[ dwSize /*+ dots*/ + 1 ];
	if ( node->pdata == NULL ) { delete node; return FALSE; }
	char *buf = (char*)node->pdata;
	
	DWORD c = 0;
	for ( DWORD i = 0; i < dwSize && pText[ i ] != 0; i++ )
	{
		char ch = pText[ i ];
		
		// Ignore non 7-bit characters
		if ( ch <= 0 );

		// Just save the character
		else buf[ c++ ] = ch;

	} // end for

	// Save size and NULL terminate
	node->dsize = c;
	buf[ c ] = 0;

	return AddBlock( node );
}

LPMIMEBLOCK CMime::AddFile(LPCTSTR pFile, LPCTSTR pName, DWORD flags )
{_STT();
	// Sanity checks
	if ( pFile == NULL ) return FALSE;
	if ( !CWinFile::DoesExist( pFile ) ) return FALSE;

	// Open the file
	CWinFile f;
	if ( !f.OpenExisting( pFile, GENERIC_READ ) ) return FALSE;
	DWORD size = f.Size();

	// Allocate memory
	LPMIMEBLOCK node = new MIMEBLOCK;
	if ( node == NULL ) return NULL;
	ZeroMimeBlock( node );

	// Get the file name
	if ( pName != NULL ) { strcpy_sz( node->fname, pName ); }
	else CWinFile::GetFileNameFromPath( pFile, node->fname );

	// Get MIME type
	GetContentType( node->fname, node->ctype );

	// Create Content-Type string
	char typestr[ MIME_STRSIZE ];
	wsprintf( typestr, "%s;\r\n\tname=\"%s\"", node->ctype, node->fname );

	// Save header information
	node->var.AddVar( "Content-Type", typestr );
	node->var.AddVar( "Content-Transfer-Encoding", "base64" );

	if ( ( flags & MBF1_ATTACHMENT ) != 0 )
	{
		char disp[ MIME_STRSIZE ];
		wsprintf( disp, "attachment;\r\n\tfilename=\"%s\"", node->fname );
		node->var.AddVar( "Content-Disposition", disp );
	} // end if

	// Save flags
	node->f1 = flags;
	node->f1 |= MBF1_DECODED;

	// Use base64 encoding
	node->encode = MBEN_BASE64;

	if ( size > 0 )
	{
		// Allocate memory
		node->pdata = new char[ size + 1 ];
		if ( node->pdata == NULL ) { delete node; return FALSE; }

		// Read in file data
		if ( !f.Read( node->pdata, size, &node->dsize ) )
		{	delete [] node->pdata; delete node; return FALSE; }

		// NULL terminate (just in case)
		( (LPBYTE)node->pdata )[ size ] = 0;

	} // end if

	return AddBlock( node );
}

LPMIMEBLOCK CMime::AddFile( LPCTSTR pFile, LPBYTE buf, DWORD size, DWORD flags )
{_STT();
	// Allocate memory
	LPMIMEBLOCK node = new MIMEBLOCK;
	if ( node == NULL ) return NULL;
	ZeroMimeBlock( node );

	// Get the file name
	if ( pFile != NULL ) { strcpy_sz( node->fname, pFile ); }

	// Get MIME type
	GetContentType( node->fname, node->ctype );

	// Create Content-Type string
	char typestr[ MIME_STRSIZE ];
	wsprintf( typestr, "%s;\r\n\tname=\"%s\"", node->ctype, node->fname );

	// Save header information
	node->var.AddVar( "Content-Type", typestr );
	node->var.AddVar( "Content-Transfer-Encoding", "base64" );

	if ( ( flags & MBF1_ATTACHMENT ) != 0 )
	{
		char disp[ MIME_STRSIZE ];
		wsprintf( disp, "attachment;\r\n\tfilename=\"%s\"", node->fname );
		node->var.AddVar( "Content-Disposition", disp );
	} // end if

	// Save flags
	node->f1 = flags;
	node->f1 |= MBF1_DECODED;

	// Use base64 encoding
	node->encode = MBEN_BASE64;

	// Any data?
	if ( size > 0 )
	{
		// Allocate memory
		node->pdata = new char[ size + 1 ];
		if ( node->pdata == NULL ) { delete node; return FALSE; }

		// Copy the data
		memcpy( node->pdata, buf, size );
		node->dsize = size;

		// NULL terminate (just in case)
		( (LPBYTE)node->pdata )[ size ] = 0;

	} // end if

	return AddBlock( node );
}

static char *m_types[] =
{
	"aif", "audio/x-aiff",
	"aiff", "audio/x-aiff",
	"aifc", "audio/x-aiff",
	"au", "audio/basic",
	"snd", "audio/basic",
	"avi", "video/x-msvideo",
	"bas", "text/plain",
	"bat", "text/plain",
	"bin", "application/octet-stream",
	"bmp", "image/bmp",
	"cer", "application/x-x509-ca-cert",
	"class", "application/java-class",
	"cmd", "text/plain",
	"com", "application/octet-stream",
	"css", "text/css",
	"doc", "application/msword",
	"eml", "message/rfc822",
	"etx", "text/x-setext",
	"evy", "application/envoy",
	"exe", "application/x-msdownload",
	"gif", "image/gif",
	"gz", "application/x-gzip",
	"htm", "text/html",
	"html", "text/html",
	"ief", "image/ief",
	"jar", "application/java-archive",
	"jardiff", "application/x-java-archive-diff",
	"java", "text/x-java-source",
	"jpg", "image/jpeg",
	"jpeg", "image/jpeg",
	"jpe", "image/jpeg",
	"jnlp", "application/x-java-jnlp-file",
	"js", "application/x-javascript",
	"mid", "audio/midi",
	"midi", "audio/midi",
	"mov", "video/quicktime",
	"qt", "video/quicktime",
	"movie", "video/x-sgi-movie",
	"mp3", "audio/mpeg",
	"mpg", "video/mpeg",
	"mpeg", "video/mpeg",
	"mpe", "video/mpeg",
	"oda", "application/oda",
	"ogg", "application/x-ogg",
	"pdm", "image/x-portable-bitmap",
	"pdf", "application/pdf",
	"pgm", "image/x-portable-graymap",
	"png", "image/png",
	"ppm", "image/x-portable-pixmap",
	"ps", "application/postscript",
	"eps", "application/postscript",
	"ai", "application/postscript",
	"ra", "audio/x-pn-realaudio",
	"rm", "audio/x-pn-realaudio",
	"ram", "audio/x-pn-realaudio",
	"rgb", "image/x-rgb",
	"rtf", "application/rtf",
	"rtx", "text/richtext",
	"ser", "application/x-java-serialized-object",
	"ssi", "text/x-server-parsed-html",
	"shtml", "text/x-server-parsed-html",
	"swf", "application/x-shockwave-flash",
	"tar", "application/x-tar",
	"tif", "image/tiff",
	"tiff", "image/tiff",
	"tsv", "text/tab-separated-values",
	"txt", "text/plain",
	"text", "text/plain",
	"asc", "text/plain",
	"usr", "application/x-x509-user-cert",
	"vcf", "text/x-vcard",
	"vew", "application/groupwise",
	"wav", "audio/wav",
	"w61", "application/wordperfect6.1",
	"wml", "x-world/x-vrml",
	"wp", "application/wordperfect",
	"wpd", "application/wordperfect",
	"wp5", "application/wordperfect",
	"w60", "application/wordperfect6.0",
	"xml", "application/xml",
	"zip", "application/zip",

	0
};

BOOL CMime::GetContentType(LPCTSTR pFile, LPSTR pType )
{_STT();
	// Sanity check
	if ( pFile == NULL || pType == NULL ) return FALSE;

	// Get extention
	LPCTSTR ext = CWinFile::GetExtension( pFile );
	if ( ext == NULL || *ext == 0 ) ext = pFile;

	*pType = NULL;

	for ( DWORD i = 0; m_types[ i ] != NULL; i += 2 )
		if ( !strnicmp( ext, m_types[ i ], strlen( m_types[ i ] ) ) )
		{
			strcpy( pType, m_types[ i + 1 ] );
			return TRUE;
		} // end if

	// Just use binary type
	strcpy( pType, "application/octet-stream" );

	return FALSE;
}

static char *m_exctypes[] =
{
	"aif", "audio/x-aiff",
	"snd", "audio/basic",
	"avi", "video/x-msvideo",
	"bin", "application/octet-stream",
	"bmp", "image/bmp",
	"cer", "application/x-x509-ca-cert",
	"class", "application/java-class",
	"css", "text/css",
	"doc", "application/msword",
	"eml", "message/rfc822",
	"etx", "text/x-setext",
	"evy", "application/envoy",
	"exe", "application/x-msdownload",
	"gif", "image/gif",
	"gz", "application/x-gzip",
	"htm", "text/html",
	"ief", "image/ief",
	"jar", "application/java-archive",
	"jardiff", "application/x-java-archive-diff",
	"java", "text/x-java-source",
	"jpg", "image/jpeg",
	"jnlp", "application/x-java-jnlp-file",
	"js", "application/x-javascript",
	"mid", "audio/midi",
	"mov", "video/quicktime",
	"movie", "video/x-sgi-movie",
	"mp3", "audio/mpeg",
	"mpg", "video/mpeg",
	"oda", "application/oda",
	"ogg", "application/x-ogg",
	"pdm", "image/x-portable-bitmap",
	"pdf", "application/pdf",
	"pgm", "image/x-portable-graymap",
	"png", "image/png",
	"ppm", "image/x-portable-pixmap",
	"ps", "application/postscript",
	"ra", "audio/x-pn-realaudio",
	"rgb", "image/x-rgb",
	"rtf", "application/rtf",
	"rtx", "text/richtext",
	"ser", "application/x-java-serialized-object",
	"ssi", "text/x-server-parsed-html",
	"shtml", "text/x-server-parsed-html",
	"swf", "application/x-shockwave-flash",
	"tar", "application/x-tar",
	"tif", "image/tiff",
	"tsv", "text/tab-separated-values",
	"txt", "text/plain",
	"usr", "application/x-x509-user-cert",
	"vcf", "text/x-vcard",
	"vew", "application/groupwise",
	"wav", "audio/wav",
	"w61", "application/wordperfect6.1",
	"wml", "x-world/x-vrml",
	"wp", "application/wordperfect",
	"wpd", "application/wordperfect",
	"w60", "application/wordperfect6.0",
	"xml", "application/xml",
	"zip", "application/zip",

	0
};

BOOL CMime::GetContentExtension(LPCTSTR pMime, LPSTR pExt)
{_STT();
	// Sanity check
	if ( pMime == NULL || pExt == NULL ) return FALSE;

	*pExt = NULL;

	for ( DWORD i = 0; m_exctypes[ i ] != NULL; i += 2 )
		if ( !strnicmp( pMime, m_exctypes[ i + 1 ], strlen( m_exctypes[ i + 1 ] ) ) )
		{	strcpy( pExt, m_exctypes[ i ] );
			return TRUE;
		} // end if

	return FALSE;
}

BOOL CMime::GetEmailComponents(LPCTSTR pEmail, LPSTR pName, LPSTR pUser, LPSTR pDomain, LPDWORD pdwNext )
{_STT();
	// Sanity checks
	if ( pEmail == NULL ) return FALSE;

	if ( pName != NULL ) *pName = 0;
	if ( pDomain != NULL ) *pDomain = 0;
	if ( pUser != NULL ) *pUser = 0;

	BOOL bUser = FALSE, bDomain = FALSE;

	for ( DWORD i = 0; pEmail[ i ] != 0 && i < MIME_STRSIZE; i++ )
	{
		// Check for name
		if ( pEmail[ i ] == '\"' )
		{	i++;
			if ( pName != NULL )
			{			
				DWORD x = 0;
				while ( i < MIME_STRSIZE && x < MIME_STRSIZE &&
						pEmail[ i ] != 0 && pEmail[ i ] != '\"' )
					pName[ x++ ] = pEmail[ i++ ];
				pName[ x ] = 0;
			} // end if
			else while (	i < MIME_STRSIZE &&
							pEmail[ i ] != 0 && pEmail[ i ] != '\"' ) i++;

		} // end if

		// Check for name
		else if ( pEmail[ i ] == '\'' )
		{	i++;
			if ( pName != NULL )
			{			
				DWORD x = 0;
				while ( i < MIME_STRSIZE && x < MIME_STRSIZE &&
						pEmail[ i ] != 0 && pEmail[ i ] != '\'' )
					pName[ x++ ] = pEmail[ i++ ];
				pName[ x ] = 0;
			} // end if
			else while (	i < MIME_STRSIZE &&
							pEmail[ i ] != 0 && pEmail[ i ] != '\'' ) i++;
		} // end if

		else if ( pEmail[ i ] == '@' )
		{
			// Backup to start of email address
			while ( i && IsValidEmailCharacter( pEmail[ i - 1 ] ) )
				i--;

			// Copy the user name
			DWORD x = 0;
			while ( pEmail[ i ] != '@' ) 
			{	bUser = TRUE;
				if ( pUser != NULL ) pUser[ x++ ] = pEmail[ i++ ];
				else i++;
			} // end while
			if ( pUser != NULL ) pUser[ x ] = 0;

			// Skip the '@'
			i++;

			// Copy the domain name
			x = 0;
			while (	i < MIME_STRSIZE && x < MIME_STRSIZE &&
					IsValidEmailCharacter( pEmail[ i ] ) )
			{	bDomain = TRUE;
				if ( pDomain != NULL ) pDomain[ x++ ] = pEmail[ i++ ];
				else i++;
			} // end while
			if ( pDomain != NULL ) pDomain[ x ] = 0;

			// What was the last character
			if ( pdwNext != NULL ) *pdwNext = i;

			// Did we get a valid address?
			return ( bUser && bDomain );

		} // end else if

	} // end for

	return FALSE;
}

// Valid characters in e-mail address 0-9 a-z A-Z -._ 
BOOL CMime::IsValidEmailCharacter(char ch)
{
	return (	( ch >= '0' && ch <= '9' ) ||
				( ch >= 'A' && ch <= 'Z' ) ||
				( ch >= 'a' && ch <= 'z' ) ||
				ch == '-' ||
				ch == '.' ||
				ch == '_' );
}

BOOL CMime::BuildEmail(LPSTR pEmail, LPCTSTR pName, LPCTSTR pUser, LPCTSTR pDomain, LPCTSTR pQuote, LPCTSTR pStart, LPCTSTR pEnd )
{_STT();
	DWORD i = 0, x = 0;

	BOOL bUser = FALSE, bDomain = FALSE;

	// No string yet
	*pEmail = 0;

	// Copy name if any
	if ( pName != NULL && *pName != 0 )
	{
		if ( pQuote != NULL ) 
		{	strcpy( &pEmail[ i ], pQuote );
			i += strlen( pQuote );
		} // end if
		else pEmail[ i++ ] = '\"';		

		x = 0;
		while ( pName[ x ] != 0 ) 
		{
			// Copy valid characters from buffer
			if (	pName[ x ] >= ' ' && pName[ x ] <= '~' && 
					pName[ x ] != '\"' && pName[ x ] != '\'' )
				pEmail[ i++ ] = pName[ x++ ];
			else x++;
		} // end while

		if ( pQuote != NULL ) 
		{	strcpy( &pEmail[ i ], pQuote );
			i += strlen( pQuote );
		} // end if
		else pEmail[ i++ ] = '\"';		
		pEmail[ i++ ] = ' ';

	} // end if

	// Open real address
	if ( pStart != NULL ) 
	{	strcpy( &pEmail[ i ], pStart );
		i += strlen( pStart );
	} // end if
	else pEmail[ i++ ] = '<';		
	
	// Copy user
	x = 0;
	while ( IsValidEmailCharacter( pUser[ x ] ) )
	{	bUser = TRUE; pEmail[ i++ ] = pUser[ x++ ]; }

	// Separator
	pEmail[ i++ ] = '@';
	
	// Copy domain
	x = 0;
	while ( IsValidEmailCharacter( pDomain[ x ] ) )
	{	bDomain = TRUE; pEmail[ i++ ] = pDomain[ x++ ]; }

	// Close real address
	if ( pEnd != NULL ) 
	{	strcpy( &pEmail[ i ], pEnd );
		i += strlen( pEnd );
	} // end if
	else pEmail[ i++ ] = '>';		

	// NULL Terminate
	pEmail[ i ] = 0;	
	
	// Is it valid?
	return ( bUser && bDomain );
}

BOOL CMime::VerifyEmailList(LPSTR pDst, LPCTSTR pSrc, LPCTSTR pSep)
{_STT();
	// Sanity check
	if ( pDst == NULL || pSrc == NULL ) return FALSE;

	DWORD i = 0, skip = 0;   
	LPSTR pCopy = pDst;
	TMem< char > buf;

	// Are the destination and source the same?
	if ( pDst == pSrc )
	{	if ( !buf.allocate( ( strlen( pSrc ) * 2 ) + 1 ) ) return FALSE;
		pCopy = buf.ptr();
	} // end if

	// Empty email list
	*pCopy = 0;

	char name[ MIME_STRSIZE ];
	char user[ MIME_STRSIZE ];
	char domain[ MIME_STRSIZE ];

	// Pick apart the list then rebuild it
	while( GetEmailComponents( &pSrc[ i ], name, user, domain, &skip ) )
	{
		// Skip to next address
		i += skip;

		// Add separator
		if ( *pCopy != 0 ) strcat( pCopy, pSep );

		// Append email address
		BuildEmail( &pCopy[ strlen( pCopy ) ], name, user, domain );

	} // end while

	// Copy if dest and src are the same
	if ( pDst == pSrc ) strcpy( pDst, pCopy );
	 
	return TRUE;
}


DWORD CMime::GetNextEmail(LPSTR pName, LPSTR pAddress, LPCTSTR pList, DWORD dwOffset, LPCTSTR pStart, LPCTSTR pEnd)
{_STT();
	if ( pList == NULL ) return 0;

	char user[ MIME_STRSIZE ];
	char domain[ MIME_STRSIZE ];

	// Get email components
	DWORD i = 0;
	if ( !GetEmailComponents( &pList[ dwOffset ], pName, user, domain, &i ) )
		return 0;

	// Build email address
	if ( !BuildEmail( pAddress, NULL, user, domain, NULL, pStart, pEnd ) )
		return i;

	// Return the length
	return i;
}


BOOL CMime::AddToHeader(LPCTSTR pParam, LPCTSTR pValue)
{_STT();
	// Sanity check
	if ( pParam == NULL || pValue == NULL || m_header == NULL ) 
		return FALSE;

	// Add param to header
	return ( m_header->var.AddVar( pParam, pValue ) != NULL );
}

void CMime::ZeroMimeBlock(LPMIMEBLOCK pMb)
{_STT();
	if ( pMb == NULL ) return;

	pMb->size = 0;
	pMb->type = 0;
	pMb->f1 = 0;
	pMb->encode = 0;
	*pMb->ctype = 0;
	*pMb->fname = 0;
	*pMb->boundry = 0;
	*pMb->cid = 0;

	pMb->dsize = 0;
	pMb->pdata = NULL;

	pMb->pNext = NULL;
	pMb->pPrev = NULL;

}

BOOL CMime::GetTextBlocks(CMime *pMime, LPMIMEBLOCK *plaintext, LPMIMEBLOCK *htmltext, LPMIMEBLOCK *multipart)
{
	LPMIMEBLOCK pmb = NULL;

	*plaintext = NULL;
	*htmltext = NULL;
	*multipart = NULL;

	// Find text blocks
	while ( ( NULL == *htmltext || NULL == *plaintext ) &&
			NULL != ( pmb = pMime->GetNext( pmb ) ) )
	{
		if ( 	( pmb->f1 & MBF1_ATTACHMENT ) == 0 &&
				!strnicmp( pmb->ctype, "multipart/alternative", strlen( "multipart/alternative" ) ) )
			*multipart = pmb;

		else if ( 	( pmb->f1 & MBF1_ATTACHMENT ) == 0 &&
					pmb->dsize > 2 && *plaintext == NULL &&
					!strnicmp( pmb->ctype, "text/plain", strlen( "text/plain" ) ) )
					*plaintext = pmb;

		else if ( 	( pmb->f1 & MBF1_ATTACHMENT ) == 0 &&
					pmb->dsize > 2 && *htmltext == NULL &&
					!strnicmp( pmb->ctype, "text/html", strlen( "text/html" ) ) )
			*htmltext = pmb;

		else if (	( pmb->f1 & MBF1_ATTACHMENT ) == 0 && pmb->ctype[ 0 ] == 0 &&
					pmb->dsize > 2 && *plaintext == NULL ) 
			*plaintext = pmb;
	} // end while

	return TRUE;
}

BOOL CMime::GetTextBlocks(CMime *pMulti, LPMIMEBLOCK *plaintext, LPMIMEBLOCK *htmltext)
{
	LPMIMEBLOCK multipart = NULL;

	// Get text blocks
	CMime::GetTextBlocks( this, plaintext, htmltext, &multipart );

	// Get blocks from sub block(s)
	BOOL quit = FALSE;
	LPMIMEBLOCK oldmulti = NULL;
	while ( !quit &&
			( *plaintext == NULL || *htmltext == NULL ) && 
			multipart != NULL && multipart != oldmulti )
	{
		// One shot
		oldmulti = multipart;

		if ( pMulti->LoadFromMemory( (LPBYTE)multipart->pdata, multipart->dsize ) )
		{
			LPMIMEBLOCK		pt = NULL, ht = NULL;
			CMime::GetTextBlocks( pMulti, &pt, &ht, &multipart );

			// Set pointers
			if ( pt != NULL && *plaintext == NULL ) quit = TRUE, *plaintext = pt;

			if ( ht != NULL && *htmltext == NULL ) quit = TRUE, *htmltext = ht;

		} // end if

	} // end while

	return TRUE;
}


BOOL CMime::ExtractHtml(LPCTSTR pHtmlFile, LPCTSTR pPlainText, LPCTSTR pCidFolder, LPCTSTR pAttachmentsFolder)
{
	CMime		multi;
	LPMIMEBLOCK plaintext = NULL;
	LPMIMEBLOCK htmltext = NULL;

	// Get text components
	GetTextBlocks( &multi, &plaintext, &htmltext );
	
	// Extract attachments
	if ( pAttachmentsFolder )
	{	::CreateDirectory( pAttachmentsFolder, NULL );
		SaveAttachments( pAttachmentsFolder );
	} // end if

	// Extract extra files
	if ( pCidFolder )
	{	
		// Create CID folder
		::CreateDirectory( pCidFolder, NULL );
		SaveExtras( pCidFolder );
		
		// Replace the cid's with actual file links
		ReplaceCid( htmltext, pCidFolder );

	} // end if

	// We need to process any multi-part as well
	// Is there a multipart?
	if ( multi.Size() )
	{	
		// Extract attachments
		if ( pAttachmentsFolder )
			multi.SaveAttachments( pAttachmentsFolder );
		
		if ( pCidFolder )
		{	multi.SaveExtras( pCidFolder );		
			multi.ReplaceCid( htmltext, pCidFolder );
		} // end if

	} // end if
  
	// Extract the html document if any
	if ( pHtmlFile && htmltext )
	{
		CWinFile f;
		if ( f.OpenNew( pHtmlFile ) )
			f.Write( htmltext->pdata , htmltext->dsize );

	} // end if

	// Extract the plain text if any
	if ( pPlainText && plaintext )
	{
		CWinFile f;
		if ( f.OpenNew( pPlainText ) )
			f.Write( plaintext->pdata , plaintext->dsize );

	} // end if

	return TRUE;
}
