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
// CfgFile.cpp: implementation of the CCfgFile class.
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

CCfgFile::CCfgFile()
{_STTEX();
	m_dwSize = 0;
	m_pHead = NULL;
	m_pTail = NULL;

	m_dwErrors = 0;

	m_hCurGroup = NULL;

	*m_szFileName = 0;
	*m_szKey = 0;
	*m_szNum = 0;

	m_ucTerm = 0x0d;
}

CCfgFile::~CCfgFile()
{_STTEX();
	Destroy();
}

void CCfgFile::Destroy()
{_STTEX();
	// For each group
	while ( m_pHead != NULL )
	{
		LPCFGGROUPINFO next = m_pHead->pNext;

		// Delete all elements
		DeleteElements( m_pHead );

		// For good measure
		m_pHead->tail = NULL;
		m_pHead->size = 0;

		// Delete this element
		delete m_pHead;
		m_pHead = next;

	} // end while

	m_pTail = NULL;
	m_dwSize = 0;

	m_dwErrors = 0;

	m_hCurGroup = NULL;

	*m_szFileName = 0;
}

void CCfgFile::DeleteElements(HGROUP hGroup)
{_STTEX();
	// For each element
	while ( hGroup->head != NULL )
	{
		LPCFGELEMENTINFO enext = hGroup->head->pNext;

		// Delete element value memory if needed
		if ( hGroup->head->size != 0 && hGroup->head->value != NULL )
		{
			delete[] hGroup->head->value;
			hGroup->head->value = NULL;
		} // end if

		// Delete this element
		delete hGroup->head;
		hGroup->head = enext;

	} // end while
}


BOOL CCfgFile::AddElement( HGROUP hGroup, LPCTSTR name, DWORD type, DWORD size, LPVOID value )
{_STTEX();
	BOOL	bNew = FALSE;
	LPCFGELEMENTINFO node = FindElement( hGroup, name );

	// Find the NULL group
	if ( hGroup == NULL )
	{	hGroup = VerifyGroup( NULL );
		if ( hGroup == NULL ) hGroup = AddGroup( NULL );
		if ( hGroup == NULL ) return FALSE;
	} // end if

	// Does it already exist
	if ( node == NULL ) 
	{	bNew = TRUE;
		node = new CFGELEMENTINFO;
		if ( node == NULL ) return FALSE;
	} // end if

	// Lose previous memory
	else if ( node->size != 0 && node->value != NULL ) 
		delete[] node->value;

	// Save element name
	if ( name != NULL ) { strcpy_sz( node->name, name ); }
	else *node->name = NULL;

	// Save type and size information
	node->type = type;
	node->size = size;

	// Just store value if size is zero
	if ( size == 0 ) node->value = value;

	// Allocate memory if needed
	else
	{
		// Allocate memory for user information
		node->value = new BYTE[ size + 1 ];

		// Punt if no memory
		if ( node->value == NULL ) { delete node; return FALSE; }

		// Copy user data
		if ( value != NULL ) memcpy( node->value, value, size );
		else { ZeroMemory( node->value, size ); }

		// NULL terminate data
		( (LPBYTE)node->value )[ size ] = 0;

	} // end else

	// Don't add if not new
	if ( !bNew ) return TRUE;

	// Add this node to the list
	node->pNext = NULL;
	node->pPrev = hGroup->tail;
	if ( hGroup->tail == NULL ) hGroup->head = node;
	else hGroup->tail->pNext = node;
	hGroup->tail = node;
	hGroup->size++;	

	return TRUE;
}

BOOL CCfgFile::RemoveElement(LPCTSTR pGroup, LPCTSTR pElement)
{_STTEX();
	HGROUP hGroup = FindGroup( pGroup );
	LPCFGELEMENTINFO pcei = FindElement( hGroup, pElement );

	return RemoveElement( hGroup, pcei );
}

BOOL CCfgFile::RemoveElement( HGROUP hGroup, LPCFGELEMENTINFO node)
{_STTEX();
	// Find the NULL group
	if ( hGroup == NULL )
	{	hGroup = VerifyGroup( NULL );
		if ( hGroup == NULL ) return FALSE;
	} // end if

	if ( node == NULL ) return FALSE;

	// Remove this node from the list
	if ( node->pPrev != NULL ) node->pPrev->pNext = node->pNext;
	if ( node->pNext != NULL ) node->pNext->pPrev = node->pPrev;
	if ( hGroup->head == node ) hGroup->head = node->pNext;			
	if ( hGroup->tail == node ) hGroup->tail = node->pPrev;		
	
	// Delete this node
	delete node;

	// Decrement the count
	if ( m_dwSize ) m_dwSize--;

	return TRUE;
}

DWORD CCfgFile::WriteToMem(LPBYTE buf, DWORD size, LPCTSTR src, DWORD srcsize)
{_STTEX();
	if ( src == NULL ) return 0;

	// Get size if not supplied
	if ( srcsize == 0 ) srcsize = strlen( src );

	if ( buf != NULL )
	{
		// Don't overflow buffer
		if ( srcsize > size ) srcsize = size;

		// Copy data
		memcpy( buf, src, srcsize );
	} // end if

	return srcsize;
}

#define WRITESZ( str )		i += WriteToMem( (LPBYTE)( ptr == NULL ? NULL : &ptr[ i ] ), \
												len > i ? len - i : 0, str, 0 )
#define WRITE( str, sz )	i += WriteToMem( (LPBYTE)( ptr == NULL ? NULL : &ptr[ i ] ), \
												len > i ? len - i : 0, str, sz )
DWORD CCfgFile::SaveToMem(LPBYTE ptr, DWORD len)
{_STTEX();
	DWORD		i = 0;
	CWinFile	file;

	// Set crypto key
	if ( *m_szKey ) file.CryptoSetKey( m_szKey );

	// Write out header
	WRITESZ( "; Configuration File\r\n;\r\n" );

	LPCFGGROUPINFO	pcgi = NULL;
	while ( ( pcgi = GetNext( pcgi ) ) != NULL )
	{
		// Write out the name
		if ( *pcgi->name != NULL )
		{	WRITESZ( "\r\n;-----------------------------------------------------------------" );
			WRITESZ( "\r\n[" );
			WRITESZ( pcgi->name );
			WRITESZ( "]" );
			WRITESZ( "\r\n;-----------------------------------------------------------------" );
			WRITESZ( "\r\n;\r\n" );
		} // end if

		// Write out file data
		TMem< char >		buf;
		DWORD				size;
		char				msg[ CFG_STRSIZE ];
		LPCFGELEMENTINFO	pcei = NULL;
		while ( ( pcei = GetNextElement( pcgi, pcei ) ) != NULL )
		{
			// Write out type
			if ( pcei->type != 0 )
			{
				wsprintf( msg, "%lu:", pcei->type );
				WRITESZ( msg );
			} // end if

			// Write out the name
			size = GetMinCanonicalizeBufferSize( strlen( pcei->name ) );
			if ( buf.allocate( size + 1 ) )
			{
				// Write string
				if ( CanonicalizeBuffer( buf, (LPBYTE)pcei->name, strlen( pcei->name ) ) )
					WRITE( buf, strlen( buf ) );

				buf.destroy();

			} // end if

			if ( pcei->size != 0 || pcei->value != NULL || pcei->type != 0 )
			{
				// Write out the equator
				if ( pcei->type == CFG_DWORD ) 
					WRITESZ( "=#" );
				else WRITESZ( "=>" );

				// Write out DWORD value
				if ( pcei->type == CFG_DWORD )
				{
					wsprintf( msg, "%lu:", pcei->value );
					WRITESZ( msg );
				} // end if

				else if ( size > 0 )
				{
					// Write out the data
					size = GetMinCanonicalizeBufferSize( pcei->size );
					if ( buf.allocate( size + 1 ) )
					{
							// Write string
						if ( CanonicalizeBuffer( buf, (LPBYTE)pcei->value, pcei->size ) )
							WRITE( buf, strlen( buf ) );

						buf.destroy();
					} // end if

				} // end else

			} // end if

			WRITESZ( "\r\n" );
					
		} // end while

	} // end while

	return i;
}

BOOL CCfgFile::Save(LPCTSTR pFile)
{_STTEX();
	CWinFile	file;

	// Set crypto key
	if ( *m_szKey ) file.CryptoSetKey( m_szKey );

	// Verify file name
	if ( pFile == NULL ) pFile = m_szFileName;

	// Open the file
	if ( !file.OpenNew( pFile, GENERIC_WRITE ) ) return FALSE;

	// Write out header
	file.Write( "; Configuration File\r\n;\r\n" );

	LPCFGGROUPINFO	pcgi = NULL;
	while ( ( pcgi = GetNext( pcgi ) ) != NULL )
	{
		// Write out the name
		if ( *pcgi->name != NULL )
		{	file.Write( "\r\n;-----------------------------------------------------------------" );
			file.Write( "\r\n[" );
			file.Write( pcgi->name );
			file.Write( "]" );
			file.Write( "\r\n;-----------------------------------------------------------------" );
			file.Write( "\r\n;\r\n" );
		} // end if

		// Write out file data
		TMem< char >		buf;
		DWORD				size;
		char				msg[ CFG_STRSIZE ];
		LPCFGELEMENTINFO	pcei = NULL;
		while ( ( pcei = GetNextElement( pcgi, pcei ) ) != NULL )
		{
			if ( !strcmpi( pcei->name, "font" ) )
			{	int x = 0; }

			// Write out type
			if ( pcei->type != 0 )
			{
				wsprintf( msg, "%lu:", pcei->type );
				if ( !file.Write( msg ) ) return FALSE;
			} // end if

			// Write out the name
			size = GetMinCanonicalizeBufferSize( strlen( pcei->name ) );
			if ( buf.allocate( size + 1 ) )
			{
				// Write string
				if ( CanonicalizeBuffer( buf, (LPBYTE)pcei->name, strlen( pcei->name ) ) )
					file.Write( buf, strlen( buf ) );

				buf.destroy();
			} // end if

			if ( pcei->size != 0 || pcei->value != NULL || pcei->type != 0 )
			{
				// Write out the equator
				if ( pcei->type == CFG_DWORD ) 
					file.Write( "=#" );
				else file.Write( "=>" );

				// Write out DWORD value
				if ( pcei->type == CFG_DWORD )
				{
					wsprintf( msg, "%lu:", pcei->value );
					if ( !file.Write( msg ) ) return FALSE;
				} // end if

				else if ( size > 0 )
				{
					// Write out the data
					size = GetMinCanonicalizeBufferSize( pcei->size );
					if ( buf.allocate( size + 1 ) )
					{
							// Write string
						if ( CanonicalizeBuffer( buf, (LPBYTE)pcei->value, pcei->size ) )
							file.Write( buf, strlen( buf ) );

						buf.destroy();
					} // end if

				} // end else

			} // end if

			file.Write( "\r\n" );
					
		} // end while

	} // end while

	return TRUE;
}

LPCFGELEMENTINFO CCfgFile::FindElement( HGROUP hGroup, LPCTSTR pName )
{_STTEX();
	// Find the NULL group
	if ( hGroup == NULL )
	{	hGroup = VerifyGroup( NULL );
		if ( hGroup == NULL ) return FALSE;
	} // end if

	LPCFGELEMENTINFO	pcei = NULL;
	while ( ( pcei = GetNextElement( hGroup, pcei ) ) != NULL )
		if ( !strcmpi( pName, pcei->name ) )
			return pcei;

	return NULL;
}

BOOL CCfgFile::CanonicalizeBuffer(LPSTR str, LPBYTE buf, DWORD size)
{_STTEX();
	// Sanity check
	if ( str == NULL || buf == NULL || size == 0 )
		return FALSE;

	DWORD i = 0, p = 0;
	while ( i < size )
	{
		// Normal characters
		if ( buf[ i ] > ' ' && buf[ i ] <= '~' && buf[ i ] != '=' && 
			 buf[ i ] != '\"' && buf[ i ] != '&' && buf[ i ] != '%' && buf[ i ] != '+' )
			str[ p++ ] = buf[ i++ ];

		// Binary characters
		else
		{	wsprintf( &str[ p ], "%%%02lX", (DWORD)buf[ i++ ] );
			p += 3;
		} // end else

	} // end for

	// NULL terminate
	str[ p ] = NULL;

	return TRUE;
}

BOOL CCfgFile::DeCanonicalizeBuffer(LPCTSTR str, LPBYTE buf, DWORD max, LPDWORD size)
{_STTEX();
	BOOL inquotes = FALSE;

	// Init size
	if ( size != NULL ) *size = 0;

	// Sanity check
	if ( str == NULL || buf == NULL ) return FALSE;

	DWORD i = 0, p = 0;
	while ( i < max && str[ i ] != 0 )
	{
		// Toggle quote mode
		if ( str[ i ] == '\"' ) 
			i++, inquotes = !inquotes;

		if ( i >= max );

		else if ( inquotes || str[ i ] != '%' ) buf[ p++ ] = str[ i++ ];

		else
		{
			i++;

			// Is it an actual %?
			if ( str[ i ] == '%' ) buf[ p++ ] = str[ i++ ];

			// Convert binary character
			else
			{	char *end;
				char num[ 3 ];

				// Convert two character hex code
				num[ 0 ] = str[ i++ ]; num[ 1 ] = str[ i++ ]; num[ 2 ] = 0;				
				buf[ p++ ] = (BYTE)strtoul( num, &end, 16 );

			} // end else

		} // end else

	} // end for

	// NULL terminate string
	buf[ p ] = 0;

	// Save size for user
	if ( size != NULL ) *size = p;

	return TRUE;
}

BOOL CCfgFile::Load(LPCTSTR pFile, BOOL bMerge)
{_STTEX();
	if ( pFile == NULL ) return FALSE;

	// Lose old record
	if ( !bMerge ) Destroy();
	if ( !bMerge ) strcpy( m_szFileName, pFile );

	CWinFile	file;

	// Set crypto key
	if ( *m_szKey ) file.CryptoSetKey( m_szKey );

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
	if ( !LoadFromMem( buf, size, bMerge ) )
		return FALSE;

	if ( !bMerge ) strcpy( m_szFileName, pFile );

	return TRUE;
}

BOOL CCfgFile::LoadFromMem(LPBYTE buf, DWORD size, BOOL bMerge)
{_STTEX();
	// Lose old record
	if ( !bMerge ) Destroy();

	// Sanity check
	if ( buf == NULL || size == 0 ) return FALSE;

	// Try to make sense out of the buffer
	for ( DWORD i = 0; i < size; )
	{
		// Add this line as a record
		if ( !AddLine( (LPSTR)&buf[ i ] ) ) m_dwErrors++;

		// Find the next line
		while ( i < size && buf[ i ] >= ' ' && buf[ i ] <= '~' && buf[ i ] != m_ucTerm ) i++;
		while ( i < size && ( buf[ i ] == m_ucTerm || buf[ i ] <= ' ' || buf[ i ] > '~' ) ) i++;
	
	} // end for

	return TRUE;
}

BOOL CCfgFile::AddLine(LPSTR pLine)
{_STTEX();
	DWORD	i = 0, p, size, type;
	char	*start, *buf;
	char	name[ MAX_PATH * 3 ];
	BOOL	data = FALSE;

	// Find start of data
	while ( pLine[ i ] != 0 && ( pLine[ i ] <= ' ' || pLine[ i ] > '~' ) ) i++;
	if ( pLine[ i ] == 0 ) return FALSE;

	// Skip comments
	if ( pLine[ i ] == ';' ) return TRUE;

	// Is it a group name?
	if ( pLine[ i ] == '[' )
	{
		DWORD	g = 0;
		char	group[ MAX_PATH ];

		// Copy group name
		i++;
		while ( g < sizeof( group ) - 1 && 
				pLine[ i ] >= ' ' && 
				pLine[ i ] <= '~' && 
				pLine[ i ] != ']' )
			group[ g++ ] = pLine[ i++ ];

		// Terminate group name
		group[ g ] = 0;

		// Add a group
		m_hCurGroup = AddGroup( group );

		return TRUE;
	} // end if

	// Read in type
	DWORD ignore = i;
	if ( pLine[ i ] >= '0' && pLine[ i ] <= '9' )
	{
		// Read in type
		type = strtoul( &pLine[ i ], NULL, 10 );
		
		// Find start of name
		while ( pLine[ i ] != 0 && 
				pLine[ i ] != '=' && 
				pLine[ i ] != ':' ) i++;
		if ( pLine[ i ] == ':' ) i++;
		else { type = CFG_VOID; i = ignore; }

	} // end if
	else type = CFG_VOID;

	// Capture the name string
	start = &pLine[ i ];
	DWORD x = 0;
	while ( pLine[ i ] >= ' ' && pLine[ i ] <= '~' && pLine[ i ] != '=' ) 
		x++, i++;
	data = ( pLine[ i ] == '=' ); 
	if ( data ) i++;

	// Decanonicalize buffer
 	if ( !DeCanonicalizeBuffer( start, (LPBYTE)name, x, &size ) ) return FALSE;

	// Convert data
	if( !data )
	{
		size = 0;
		buf = NULL;
	} // end if

	else if ( pLine[ i ] == '#' )
	{	i++;
		size = 0;
		buf = (LPSTR)strtoul( &pLine[ i ], NULL, 10 );
	} // end if
	else
	{
		// Skip string marker
		if ( pLine[ i ] == '>' ) i++;

		// Capture the data string
		start = &pLine[ i ];
		p = 0; 
		while ( pLine[ i ] >= ' ' && pLine[ i ] <= '~' && pLine[ i ] != m_ucTerm ) 
			i++, p++;

		if ( p != 0 )
		{
			// DeCanonicalize data
			DWORD a = GetMinDeCanonicalizeBufferSize( p );

			buf = new char[ a ];
			if ( buf == NULL ) return FALSE;

			// Do it
			if ( !DeCanonicalizeBuffer( start, (LPBYTE)buf, p, &size ) ) return FALSE;

		} // end if

		// NULL data
		else buf = NULL, size = 0;

	} // end else

	// Now we have all the data, so add it
	BOOL ret = AddElement( m_hCurGroup, name, type, size, buf );

	// Lose the data buffer
	if ( size != 0 ) delete [] buf;

	return ret;

}

HGROUP CCfgFile::FindGroup(LPCTSTR pGroup)
{_STTEX();
	// Search for the group
	LPCFGGROUPINFO	pcgi = NULL;
	while ( ( pcgi = GetNext( pcgi ) ) != NULL )
	{
		// Are we looking for the NULL group
		if ( pGroup == NULL ) { if ( *pcgi->name == 0 ) return pcgi; }
		
		// Is this the correct name?
		else if ( !strcmpi( pGroup, pcgi->name ) ) return pcgi;

	} // end while

	return NULL;
}

HGROUP CCfgFile::AddGroup(LPCTSTR pGroup)
{_STTEX();
	// Does this group already exist?
	LPCFGGROUPINFO	node = FindGroup( pGroup );
	if ( node != NULL ) return node;

	// Create new group node
	node = new CFGGROUPINFO;
	if ( node == NULL ) return NULL;
	ZeroMemory( node, sizeof( CFGGROUPINFO ) );

	// Copy group name
	if ( pGroup != NULL ) { strcpy_sz( node->name, pGroup ); }
	else *node->name = 0;

	// Add this node to the list
	node->pNext = NULL;
	node->pPrev = m_pTail;
	if ( m_pTail == NULL ) m_pHead = node;
	else m_pTail->pNext = node;
	m_pTail = node;
	m_dwSize++;	

	return node;
}

BOOL CCfgFile::RemoveGroup(HGROUP node)
{_STTEX();
	// Find the NULL group
	if ( node == NULL )
	{	node = VerifyGroup( NULL );
		if ( node == NULL ) return FALSE;
	} // end if

	// Remove this node from the list
	if ( node->pPrev != NULL ) node->pPrev->pNext = node->pNext;
	if ( node->pNext != NULL ) node->pNext->pPrev = node->pPrev;
	if ( m_pHead == node ) m_pHead = node->pNext;			
	if ( m_pTail == node ) m_pTail = node->pPrev;		
	
	// Delete this node
	DeleteElements( node );
	delete node;

	// Decrement the count
	if ( m_dwSize ) m_dwSize--;

	return TRUE;
}

BOOL CCfgFile::RemoveGroup(LPCTSTR pGroup)
{_STTEX();
	return RemoveGroup( FindGroup( pGroup ) );
}



HGROUP CCfgFile::VerifyGroup(HGROUP hGroup)
{_STTEX();
	// Search for the group
	LPCFGGROUPINFO	pcgi = NULL;
	while ( ( pcgi = GetNext( pcgi ) ) != NULL )
	{
		// Are we looking for the NULL group
		if ( hGroup == NULL ) { if ( *pcgi->name == 0 ) return pcgi; }
		
		// Is this the correct group
		else if ( hGroup == pcgi ) return pcgi;

	} // end while

	return NULL;
}


BOOL CCfgFile::SaveWindowPos(HWND hWnd, LPCTSTR pGroup, LPCTSTR pName, BOOL bToolwindow )
{_STTEX();
	// Sanity checks
	if ( !::IsWindow( hWnd ) || pGroup == NULL || pName == NULL ) return FALSE;

	char val[ CFG_STRSIZE ];
	BOOL bMinimized = FALSE, bMaximized = FALSE; 

	// Save topmost status
	DWORD exstyle = GetWindowLong( hWnd, GWL_EXSTYLE );
	BOOL bTopmost = ( exstyle & WS_EX_TOPMOST ) != 0;
	wsprintf( val, "%s (Topmost)", pName );
	SetValue( pGroup, val, (DWORD)( bTopmost ? 1 : 0 ) );

	if ( !bToolwindow )
	{
		bMinimized = IsIconic( hWnd );
		bMaximized = IsZoomed( hWnd );

		// Save minimized state
		wsprintf( val, "%s (Maximized)", pName );
		SetValue( pGroup, val, (DWORD)( bMaximized ? 1 : 0 ) );
		wsprintf( val, "%s (Minimized)", pName );
		SetValue( pGroup, val, (DWORD)( bMinimized ? 1 : 0 ) );

		if ( bMinimized || bMaximized )
		{
			// Restore so we can get the position
			ShowWindow( hWnd, SW_HIDE );
			ShowWindow( hWnd, SW_RESTORE );
		} // end if

	} // end if


	RECT rect;
	GetWindowRect( hWnd, &rect );
	wsprintf( val, "%s (Window Position)", pName );
	SetValue( pGroup, val, &rect, sizeof( rect ) );

	if ( bMinimized ) ShowWindow( hWnd, SW_SHOWMINIMIZED );
	else if ( bMaximized ) ShowWindow( hWnd, SW_SHOWMAXIMIZED );

	return TRUE;
}

BOOL CCfgFile::RestoreWindowPos(HWND hWnd, LPCTSTR pGroup, LPCTSTR pName, BOOL bSize, BOOL bPosition, BOOL bToolwindow, BOOL bTopmost )
{_STTEX();
	RECT rect;

	// Sanity checks
	if ( !bPosition && !bSize ) return FALSE;
	if ( !::IsWindow( hWnd ) || pGroup == NULL || pName == NULL ) return FALSE;

	char val[ CFG_STRSIZE ];

	// Get rect
	wsprintf( val, "%s (Window Position)", pName );
	if ( !GetValue( pGroup, val, &rect, sizeof( rect ) ) )
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

	HWND zOrder = HWND_NOTOPMOST;
	DWORD dwFlags = SWP_NOACTIVATE;
	if ( !bPosition ) dwFlags |= SWP_NOMOVE;
	if ( !bSize ) dwFlags |= SWP_NOSIZE;

	if ( bTopmost )
	{
		// Restore topmost position
		DWORD topmost = 0;
		wsprintf( val, "%s (Topmost)", pName );
		if ( !GetValue( pGroup, val, (LPDWORD)&topmost ) ) dwFlags |= SWP_NOZORDER;
		else if ( topmost != 0 ) zOrder = HWND_TOPMOST;
	} // end if
	else dwFlags |= SWP_NOZORDER;

	// Restore the position
	SetWindowPos( hWnd, zOrder,	rect.left, rect.top,
								rect.right - rect.left,
								rect.bottom - rect.top,
								dwFlags );

	if ( !bToolwindow )
	{
		BOOL maximized = 0, minimized = 0;
		wsprintf( val, "%s (Maximized)", pName );
		GetValue( pGroup, val, (LPDWORD)&maximized );
		wsprintf( val, "%s (Minimized)", pName );
		GetValue( pGroup, val, (LPDWORD)&minimized );

		// Restore window position
		if ( maximized ) ShowWindow( hWnd, SW_SHOWMAXIMIZED );
		else if ( minimized ) ShowWindow( hWnd, SW_SHOWMINIMIZED );
		else ShowWindow( hWnd, SW_SHOWNORMAL );

	} // end if
	
	return TRUE;
}

BOOL CCfgFile::Copy(HGROUP hGroup, CRKey *pRk)
{_STTEX();
	// Sanity check
	if ( hGroup == NULL || pRk == NULL ) return FALSE;

	LPREGVALUE prv = NULL;
	while ( ( prv = (LPREGVALUE)pRk->GetNext( prv ) ) != NULL )
	{
		switch ( prv->type )
		{
			case REG_DWORD :
				SetValue( hGroup, prv->cpkey, *( (LPDWORD)prv->data ) );
				break;

			case REG_SZ :
				SetValue( hGroup, prv->cpkey, (LPCTSTR)prv->pcdata );
				break;

			default :
				SetValue( hGroup, prv->cpkey, prv->data, prv->size );
				break;

		} // end switch

	} // end while

	return TRUE;
}

BOOL CCfgFile::CopyGroup( LPCTSTR pGroup, LPCTSTR pNewGroup )
{_STTEX();
	HGROUP hGroup = FindGroup( pGroup );
	if ( hGroup == NULL ) return FALSE;

	RemoveGroup( pNewGroup );
	HGROUP hNewGroup = AddGroup( pNewGroup );
	if ( hNewGroup == NULL ) return FALSE;

	// Copy each element
	LPCFGELEMENTINFO pei = NULL;
	while( ( pei = GetNextElement( hGroup, pei ) ) != NULL )
		AddElement( hNewGroup, pei->name, pei->type, pei->size, pei->value );

	return TRUE;
}
