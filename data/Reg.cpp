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
// Reg.cpp: implementation of the CReg class.
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

CReg::CReg()
{_STTEX();
	m_pCurKey = NULL;
	m_dwMinSize = 0;
}

CReg::~CReg()
{_STTEX();
	Destroy();
}

void CReg::Destroy()
{_STTEX();
	m_pCurKey = NULL;

	CHList::Destroy();
}

BOOL CReg::InitObject( void *node )
{_STTEX();
	CHList::InitObject( node );

	LPREGKEY prk = (LPREGKEY)node;
	if ( prk == NULL ) return NULL;

	prk->key = new CRKey();
	if ( prk->key == NULL ) { Delete( prk ); return NULL; }

	return TRUE;
}

void CReg::DeleteObject( void *node )
{_STTEX();
	LPREGKEY prk = (LPREGKEY)node;
	if ( prk == NULL ) return;

	// Delete reg key class
	if ( prk->key != NULL )
	{	delete prk->key;
		prk->key = NULL;
	} // end if

	CHList::DeleteObject( node );
}

CRKey* CReg::FindKey(LPCTSTR pKey)
{_STTEX();
	// Does key already exist?
	LPREGKEY prk = (LPREGKEY)Find( pKey );
	if ( prk == NULL ) return NULL;

	// Return the key
	return prk->key;
}

CRKey* CReg::GetKey(LPCTSTR pKey)
{_STTEX();
	LPREGKEY prk = AddKey( pKey );
	if ( prk == NULL ) return NULL;

	return prk->key;
}

LPREGKEY CReg::AddKey(LPCTSTR pKey)
{_STTEX();
	// Does key already exist?
	LPREGKEY prk = (LPREGKEY)Find( pKey );
	if ( prk != NULL ) return prk;

	// Create new key
	prk = (LPREGKEY)New( NULL, 0, (LPVOID)pKey );
	if ( prk == NULL ) return FALSE;

	// Get key
	if ( prk->key != NULL )
	{	prk->key->SetMinSize( m_dwMinSize );
		prk->key->SetName( pKey );
	} // end if

	return prk;
}


BOOL CReg::RenameKey(LPCTSTR pOld, LPCTSTR pNew)
{_STTEX();
	CRKey *pRk = FindKey( pOld );
	if ( pRk == NULL ) return FALSE;

	return pRk->SetName( pNew );
}

BOOL CReg::GetKey(HKEY hKey, LPCTSTR pKey, DWORD dwSubkeys, BOOL bMerge, LPCTSTR pKeyname )
{_STTEX();
	if ( !bMerge ) Destroy();

	HKEY hOKey = NULL;

	// Open the key
	if ( RegOpenKeyEx(	hKey, pKey,0, 
		KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, 
		&hOKey ) != ERROR_SUCCESS )
		return FALSE;

	DWORD i = 0;
	char name[ 1024 ];
	char keyname[ 1024 ];

	// For each 
	while( RegEnumKey( hOKey, i++, name, sizeof( name ) ) == ERROR_SUCCESS )
	{
		// Build the key name
		if ( pKeyname != NULL ) CWinFile::BuildPath( keyname, pKeyname, name );
		else CRKey::GetKeyName( hKey, pKey, keyname );

		// Add key
		LPREGKEY prf = (LPREGKEY)AddKey( keyname );
		if ( prf != NULL )
		{
			// Get key
			prf->key->SetName( keyname );
			prf->key->GetKey( hOKey, name );

			// Recurse for subkeys
			if ( dwSubkeys ) GetKey( hOKey, name, dwSubkeys - 1, TRUE, keyname );

		} // end if		

	} // end while

	RegCloseKey( hOKey );

	return TRUE;
}


DWORD CReg::Save(LPCTSTR pFile)
{_STTEX();
	return SaveRegFile( pFile );
}

DWORD CReg::Load(LPCTSTR pFile, BOOL bMerge)
{_STTEX();
	return LoadRegFile( pFile, NULL, bMerge );
}


BOOL CReg::LoadRegFile(LPCTSTR pFile, LPCTSTR pKey, BOOL bMerge )
{_STTEX();
	if ( pFile == NULL ) return FALSE;

	// Lose old record
	if ( !bMerge ) Destroy();

	CWinFile	file;

	// Open the file
	if ( !file.OpenExisting( pFile, GENERIC_READ ) )
		return FALSE;

	// Set crypto key if needed
	if ( pKey != NULL ) file.CryptoSetKey( pKey );

	// Get file size
	DWORD size = file.Size();
	if ( size == 0 ) return FALSE;

	// Allocate memory
	LPBYTE buf = new BYTE[ size + 2 ];
	if ( buf == NULL ) return FALSE;

	// Read in the data into ram
	DWORD read;
	if ( !file.Read( buf, size, &read ) || read != size )
	{	delete[] buf; return FALSE; }
	buf[ size ] = 0;
	buf[ size + 1 ] = 0;

	// Check for wide character file
	if ( *( (unsigned short*)buf ) == 0xfffe || *( (unsigned short*)buf ) == 0xfeff )
	{
		LPBYTE b2 = new BYTE[ size + 1 ];
		if ( b2 == NULL ) { delete [] buf; return FALSE; }

		// Convert wide characters
		size = wcstombs( (char*)b2, (wchar_t*)&buf[ 2 ], size );

		// Did it work ?
		if ( size == 0 || size == MAXDWORD ) 
		{	delete [] buf; delete [] b2; return FALSE; }
		b2[ size ] = 0;	

		// Switch to multi-byte buffer
		delete [] buf;
		buf = b2;

	} // end if

	// Load the file
	if ( !LoadRegFromMem( buf, size, bMerge ) )
	{	delete[] buf; return FALSE; }

	delete[] buf;

	return TRUE;
}

BOOL CReg::LoadRegFromMem(LPBYTE buf, DWORD size, BOOL bMerge)
{_STTEX();
	DWORD i = 0;

	// Lose old record
	if ( !bMerge ) Destroy();

	// Sanity check
	if ( buf == NULL || size == 0 ) return FALSE;

	// No key currently
	m_pCurKey = NULL;

	i = SkipWhiteSpace( buf, size, i );

	// Read in first line tag
	if ( i < size && buf[ i ] > ' ' && buf[ i ] <= '~' && buf[ i ] != '[' )
	{
		// I really don't know what to do with this tag
		i = NextLine( buf, size, i );

	} // end if

	// Process file
	while ( i < size && buf[ i ] != 0 )
	{
		// Skip white space
		i = SkipWhiteSpace( buf, size, i );

		if ( i < size && buf[ i ] == ';' ) i = NextLine( buf, size, i );

		// Process this line if not a comment
		else if ( i < size && buf[ i ] != 0 )
		{
			DWORD skip = ProcessLine( &buf[ i ], size - i );

			// Did we read a valid line
			if ( skip != 0 ) i += skip;

			// Just skip invalid line
			else i = NextLine( buf, size, i );

		} // end if

	} // end while

	return TRUE;
}

DWORD CReg::ProcessLine(LPBYTE buf, DWORD size)
{_STTEX();
	DWORD i = 0;

	// Check for key name
	if ( buf[ i ] == '[' )
	{	
		i++; // Skip '['

		// Allocate memory
		LPBYTE name = new BYTE[ size + 1 ];
		if ( name == NULL ) return 0;

		// Read key name
		DWORD x = 0;
		while ( i < size && buf[ i ] != 0 && buf[ i ] != ']' &&
				buf[ i ] >= ' ' && buf[ i ] <= '~' )
			name[ x++ ] = buf[ i++ ];
		name[ x ] = 0;		

		// Add this key
		m_pCurKey = AddKey( (char*)name );

		delete [] name;

		// Skip line
		i = NextLine( buf, size, i );

		return i;

	} // end if

	// Allocate scratch memory
	LPBYTE data = new BYTE[ size + 1 ];
	if ( data == NULL ) return 0;

	// Old file type
	if ( m_pCurKey == NULL )
	{
		// Get name
		i += GetToken( data, size, &buf[ i ], size, '=' );

		// skip '='
		i++;

		char value[ 1024 ];
		CWinFile::GetFileNameFromPath( (char*)data, value );
		CWinFile::GetPathFromFileName( (char*)data, (char*)data );

		// Create key
		LPREGKEY prf = AddKey( (char*)data );

		// Check for NULL value
		if ( buf[ i ] != '=' ) prf->key->Set( value, "" );
			
		else
		{
			// Get the data
			i += GetToken( data, size, &buf[ i ], size - i, 0 );

			// Check for default value
			if ( *value == '@' ) prf->key->Set( "", (char*)data );

			// Add string
			prf->key->Set( value, (char*)data );

		} // end else

	} // end if

	else
	{
		BOOL bDefault = ( buf[ i ] == '@' );
		
		// Get name
		i += GetToken( data, size, &buf[ i ], size - i, '=' );

		// Check for NULL value
		if ( buf[ i ] != '=' ) 
			m_pCurKey->key->Add( REG_SZ, (LPCTSTR)data, "", 0 );

		else
		{
			// skip '='
			i++;

			char value[ 1024 ];
			if ( bDefault ) *value = 0;
			else strcpy_sz( value, (char*)data );

			DWORD dsize = size, type = 0;
			DWORD c = ConvertData( &type, data, &dsize, &buf[ i ], size - i );
			i += c;

			// Add reg value
			m_pCurKey->key->Add( type, value, data, dsize );

		} // end else

	} // end else

	// Release memory
	delete [] data;

	// Next line
	i = NextLine( buf, size, i );

	return i;
}


DWORD CReg::NextLine(LPBYTE buf, DWORD size, DWORD i)
{_STTEX();
	// Skip to next line
	while(	i < size && buf[ i ] != 0 && 
			buf[ i ] != '\r' && buf[ i ] != '\n' ) i++;

	// Skip CRLF's
	while(	i < size && buf[ i ] != 0 && 
			( buf[ i ] == '\r' || buf[ i ] == '\n' ) ) i++;

	return i;
}

DWORD CReg::SkipWhiteSpace(LPBYTE buf, DWORD size, DWORD i, BOOL bIncludeNewLine)
{_STTEX();
	// Skip white space
	while(	i < size && buf[ i ] != 0 &&
			IsWhiteSpace( buf[ i ], bIncludeNewLine ) ) i++;

	return i;
}

DWORD CReg::SkipNonWhiteSpace(LPBYTE buf, DWORD size, DWORD i)
{_STTEX();
	// Skip non white space
	while(	i < size && buf[ i ] != 0 &&
			!IsWhiteSpace( buf[ i ], TRUE ) ) i++;

	return i;
}

BOOL CReg::IsWhiteSpace( BYTE ch, BOOL bIncludeNewLine )
{_STTEX();
	// New line
	if ( bIncludeNewLine )
		return ( ch <= ' ' || ch > '~' );

	// CRLF not included
	return  ( ch <= ' ' || ch > '~' ) &&
			ch != '\r' && ch != '\n';
} 

DWORD CReg::NextToken(LPBYTE buf, DWORD size, DWORD i)
{_STTEX();
	i = SkipWhiteSpace( buf, size, i );
	i = SkipNonWhiteSpace( buf, size, i );
	i = SkipWhiteSpace( buf, size, i );

	return i;
}

DWORD CReg::GetNonWhiteSpace( LPBYTE pToken, DWORD dwTokenSize, LPBYTE pBuf, DWORD dwSize, DWORD i )
{_STTEX();
	DWORD x = 0;
	while ( i < dwSize && x < dwTokenSize - 1 && 
			!CReg::IsWhiteSpace( pBuf[ i ], TRUE ) )
		pToken[ x++ ] = pBuf[ i++ ];
	pToken[ x ] = 0;

	return i;
}

DWORD CReg::GetToken(LPBYTE pToken, DWORD dwTokenSize, LPBYTE buf, DWORD size, BYTE ucTerm)
{_STTEX();
	BOOL bQuoted = FALSE;
	DWORD x = 0, i = 0;

	// Punt if not enough room for token
	if ( dwTokenSize <= 1 ) return i;

	// Make room for terminator
	dwTokenSize--;
	
	// Read in key name
	while ( x < dwTokenSize && i < size && buf[ i ] != 0 && ( bQuoted || buf[ i ] != ucTerm ) )
	{
		// Check for sudden end of line
		if ( buf[ i ] == '\r' || buf[ i ] == '\n' ) 
		{	while ( x && ( pToken[ x - 1 ] <= ' ' || pToken[ x - 1 ] > '~' ) ) x--;
			pToken[ x ] = 0; return i; 
		} // end if

		// Check for escape sequence
		else if ( buf[ i ] == '\\' )
		{	if ( buf[ i + 1 ] == '\"' ) i++, pToken[ x++ ] = '\"';
			else pToken[ x++ ] = '\\';
		} // end else if

		else if ( buf[ i ] == '\"' ) bQuoted = !bQuoted;

		// Save valid characters
		else if ( ( buf[ i ] >= ' ' && buf[ i ] <= '~' ) || buf[ i ] == '\t' )
			pToken[ x++ ] = buf[ i ];

		// Next character
		i++;

	} // end while

	// Eliminate trailing white space
	while ( x && ( pToken[ x - 1 ] <= ' ' || pToken[ x - 1 ] > '~' ) ) x--;

	// NULL terminate
	pToken[ x ] = 0; 

	return i;
}


DWORD CReg::ConvertData(LPDWORD pdwType, LPBYTE pData, LPDWORD pdwData, LPBYTE buf, DWORD size)
{_STTEX();
	DWORD i = 0;
	DWORD dwDataSize = pdwData ? *pdwData : 0;

	// Assume nothing
	if ( pdwData ) *pdwData = 0;

	// Skip to the data
	i = SkipWhiteSpace( buf, size, i );

	// Check for string
	if ( buf[ i ] == '\"' )
	{	if ( pdwType != NULL ) *pdwType = REG_SZ;
		DWORD dsize = GetToken( pData, dwDataSize, &buf[ i ], size, 0 );
		if ( pdwData != NULL ) *pdwData = strlen( (char*)pData );
		return ( i + dsize );
	} // end if

	char token[ 1024 ];
	DWORD x = 0;

	// Copy type token
	while ( i < size && buf[ i ] != 0 && buf[ i ] != ':' && 
			buf[ i ] != '\r' && buf[ i ] != '\n' ) token[ x++ ] = buf[ i++ ];
	token[ x ] = 0;

	// Did we get a type token?
	if ( buf[ i ] != ':' || *token == 0 ) 
	{
		if ( pdwType != NULL ) *pdwType = REG_SZ;

		// The token is a string then
		strcpy( (char*)pData, token );

		// Save Size of data
		if ( pdwData != NULL ) *pdwData = strlen( (char*)pData );

		return i; 
	} // end if
	
	// Skip ':'
	i++;

	// Skip to the data
	i = SkipWhiteSpace( buf, size, i );
	
	// check for dword
	if ( !strcmpi( token, "dword" ) )
	{
		if ( pdwType != NULL ) *pdwType = REG_DWORD;

		// Verify we got a hex number
		if (	( buf[ i ] < '0' || buf[ i ] > '9' ) &&
				( buf[ i ] < 'A' || buf[ i ] > 'F' ) &&
				( buf[ i ] < 'a' || buf[ i ] > 'f' ) ) return 0;

		// Get DWORD value
		*( (LPDWORD)pData ) = strtoul( (char*)&buf[ i ], NULL, 16 );
		if ( pdwData != NULL ) *pdwData = sizeof( DWORD );

	} // end else if

	// check for hex
	else if ( !strcmpi( token, "hex" ) )
	{
		if ( pdwType != NULL ) *pdwType = REG_BINARY;

		DWORD x = 0;

		// While we have characters
		while (	( buf[ i ] >= '0' && buf[ i ] <= '9' ) ||
				( buf[ i ] >= 'A' && buf[ i ] <= 'F' ) ||
				( buf[ i ] >= 'a' && buf[ i ] <= 'f' ) )
		{
			// Get byte value
			pData[ x++ ] = (BYTE)ahtoul( (char*)&buf[ i ], 2 );
			i += 2;

			// Find next character or end of line
			while (	i < size && buf[ i ] != 0 &&
					buf[ i ] != '\r' && buf[ i ] != '\n' &&
					( buf[ i ] < '0' || buf[ i ] > '9' ) &&
					( buf[ i ] < 'A' || buf[ i ] > 'F' ) &&
					( buf[ i ] < 'a' || buf[ i ] > 'f' ) ) 
			{
				// Check for continue to new line character
				if ( buf[ i ] == '\\' )
					i = NextLine( buf, size, i );

				else i++;

			} // end while

		} // end if		

		if ( pdwData != NULL ) *pdwData = x;

	} // end else if

	return i;
}

DWORD CReg::ahtoul( LPCTSTR pBuffer, BYTE ucBytes )
{_STTEX();
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
BOOL CReg::SaveRegFile(CPipe &pipe, LPCTSTR pKey, LPCTSTR pHeader)
{_STTEX();
	return SaveRegFile( &pipe, pKey, pHeader );
}

BOOL CReg::SaveRegFile(CPipe *pipe, LPCTSTR pKey, LPCTSTR pHeader)
{_STTEX();
	if ( pipe == NULL ) return FALSE;

	// Write tag
	pipe->Write( "REGEDIT4\r\n" );

	// Write header
	if ( pHeader == NULL ) pipe->Write( ";\r\n; WheresJames Software (www.wheresjames.com)\r\n" );
	else if ( *pHeader ) pipe->Write( pHeader );

	// If header
	if ( pHeader == NULL || *pHeader )
	{	
		pipe->Write( "\r\n; Created - " );

		{ // Write time and date
			char msg[ 512 ];

			CWinTime wt;

			wt.GetString( msg, "%W, %B %d, %Y - %h:%m:%s %A" );

			pipe->Write( msg );

		} // end write time and date

		pipe->Write ( "\r\n;\r\n" );

	} // end if

	if ( pKey != NULL )
	{	LPREGKEY prk = (LPREGKEY)Find( pKey );
		if ( prk != NULL ) SaveRegKey( pipe, prk );
	} // end if

	else
	{
		// Save each key
		LPREGKEY prk = NULL;
		while ( ( prk = (LPREGKEY)GetNext( prk ) ) != NULL )
			SaveRegKey( pipe, prk );

	} // end else

	return TRUE;
}

BOOL CReg::SaveRegFile(LPCTSTR pFile, LPCTSTR pKey, LPCTSTR pHeader)
{_STTEX();
	return SaveRegFile( CPFile( pFile, pKey, GENERIC_WRITE, TRUE ), pKey, pHeader );
}


BOOL CReg::SaveRegKey(CPipe *pipe, LPREGKEY pRk )
{_STTEX();
	if ( pipe == NULL ) return FALSE;

	LPREGVALUE prv = NULL;

	{ // Write out key name
		
		DWORD x = 0;
		char buf[ 1024 ];
		buf[ x++ ] = '\r';
		buf[ x++ ] = '\n';
		buf[ x++ ] = '[';
		LPCTSTR name = pRk->key->GetName();

		// Write the name to the buffer
		if ( name != NULL )
			for ( DWORD i = 0; name[ i ] != 0; i++ )
			{	if (	name[ i ] >= ' ' && name[ i ] <= '~' &&
						name[ i ] != ']' ) buf[ x++ ] = name[ i ];

				// Flush buffer if full
				if ( x > sizeof( buf ) - 8 ) pipe->Write( buf, x ), x = 0;
			} // end for

		buf[ x++ ] = ']';
		buf[ x++ ] = '\r';
		buf[ x++ ] = '\n';
		if ( x ) pipe->Write( buf, x );

	} // end write key name
		
	while ( ( prv = (LPREGVALUE)pRk->key->GetNext( prv ) ) != NULL )
	{
		// Write out the name
		if ( *prv->cpkey == 0 ) pipe->Write( "@" );
		else WriteStr( pipe, prv->cpkey );

		pipe->Write( "=" );

		switch ( prv->type )
		{
			case REG_SZ :
			{
				// Check for NULL value
				if ( prv->size == 0 || prv->data == NULL )
					pipe->Write( "\"\"\r\n" );

				else
				{
					WriteStr( pipe, (char*)prv->data );
					pipe->Write( "\r\n" );
				} // end else

			} // end case
			break;

			case REG_DWORD :
			{
				if ( prv->size < 4 || prv->data == NULL )
					pipe->Write( "dword:\r\n" );
				else
				{
					char num[ 512 ];
					wsprintf( num, "dword:%0.8lx\r\n", *(LPDWORD)prv->data );
					pipe->Write( num );
				} // end else

			} // end case
			break;

			default :
			{
				pipe->Write( "hex:" );

				DWORD l = 0;
				DWORD c = 0;
				DWORD i = 0, x = 0;
				char buf[ 1024 ];
				
				for ( i = 0; i < prv->size; i++ )
				{
					wsprintf( &buf[ x ], "%0.2lx", (DWORD)( ( (LPBYTE)prv->data )[ i ] ) );
					x += 2;

					// Separate values
					if ( i < ( prv->size - 1 ) ) buf[ x++ ] = ',';

					// Break lines
					c++;
					if ( l == 0 && c >= 20 ) 
					{	strcpy( &buf[ x ], "\\\r\n  " ); x += 5; c = 0; l++; }
					else if ( c >= 25 ) 
					{	strcpy( &buf[ x ], "\\\r\n  " ); x += 5; c = 0; l++; }

					// Flush buffer if full
					if ( x > sizeof( buf ) - 8 ) pipe->Write( buf, x ), x = 0;

				} // end for

				buf[ x++ ] = '\r';
				buf[ x++ ] = '\n';

				// Write the rest of the data
				if ( x ) pipe->Write( buf, x );

			} // end default
			break;

		} // end switch

	} // end while

	return TRUE;
}

BOOL CReg::WriteStr(CPipe *pipe, LPCTSTR pStr)
{_STTEX();
	if ( pipe == NULL || pStr == NULL ) return FALSE;
	
	DWORD i = 0, x = 0;
	char buf[ 1024 ];

	// Open quotes
	buf[ x++ ] = '\"';

	for ( i = 0; pStr[ i ] != 0; i++ )
	{
		// Escape quotes
		if ( pStr[ i ] == '\"' ) buf[ x++ ] = '\\';

		// Save character
		if ( ( pStr[ i ] >= ' ' && pStr[ i ] <= '~' ) ||
				pStr[ i ] == '\t' ) buf[ x++ ] = pStr[ i ];

		// Flush buffer if full
		if ( x > sizeof( buf ) - 8 )
			pipe->Write( buf, x ), x = 0;

	} // end for

	// Close the quotes
	buf[ x++ ] = '\"';

	// Write the rest of the data
	pipe->Write( buf, x );

	return TRUE;
}


HREGKEY CReg::Enum(LPCTSTR pKey, HREGKEY hRk)
{_STTEX();
	// Get key length
	DWORD cmp = strlen( pKey );

	// Search each reg item
	while ( ( hRk = (HREGKEY)GetNext( hRk ) ) != NULL )
		if ( hRk->key != NULL )
			if ( pKey == NULL || !strncmp( pKey, hRk->key->GetName(), cmp ) )
				return hRk;

	return NULL;
}

BOOL CReg::DeleteValue(LPCTSTR pKey, LPCTSTR pValue)
{_STTEX();
	CRKey *pRk = FindKey( pKey );
	if ( pRk == NULL ) return FALSE;

	return pRk->DeleteValue( pValue );
}

BOOL CReg::SaveWindowPos(HWND hWnd, LPCTSTR pKey, LPCTSTR pName)
{_STTEX();
	// Sanity checks
	if ( !::IsWindow( hWnd ) || pKey == NULL || pName == NULL ) 
		return FALSE;

	CRKey *pRk = GetKey( pKey );
	if ( pRk == NULL ) return FALSE;

	return pRk->SaveWindowPos( hWnd, pName );
}

BOOL CReg::RestoreWindowPos(HWND hWnd, LPCTSTR pKey, LPCTSTR pName, BOOL bSize, BOOL bPosition )
{_STTEX();
	// Sanity checks
	if ( !bPosition && !bSize ) return FALSE;
	if ( !::IsWindow( hWnd ) || pKey == NULL || pName == NULL ) 
		return FALSE;

	CRKey *pRk = FindKey( pKey );
	if ( pRk == NULL ) return FALSE;

	return pRk->RestoreWindowPos( hWnd, pName, bSize, bPosition );
}


BOOL CReg::Parse(LPCTSTR pStr, DWORD size, char sep)
{_STTEX();
	DWORD line = 0;
	DWORD s = 0, e = 0;

	if ( size == 0 ) size = strlen( pStr );

	while ( s < size && e < size )
	{
		// Find start of data
		while ( s < size && ( pStr[ s ] <= ' ' || pStr[ s ] > '~' ) ) s++;
		e = s;

		// Find the end of the data
		BOOL data = FALSE;
		while ( e < size && ( pStr[ e ] >= ' ' && pStr[ e ] <= '~' ) ) 
		{	if ( pStr[ e ] > ' ' ) data = TRUE; e++; }

		// Add to reg
		if ( e > s )
		{	CStr str( line++ );
			CRKey *pRk = GetKey( str );
			if ( pRk != NULL ) pRk->Parse( &pStr[ s ], e - s, sep );
		} // end if

		// Skip data
		s = e;

	} // end while

	return TRUE;
}

BOOL CReg::SaveSettings(LPCTSTR pFile, LPCTSTR pBak)
{_STTEX();
	char cfg[ CWF_STRSIZE ];
	char path[ CWF_STRSIZE ];

	CWinFile::GetExePath( path );

	BOOL saved = FALSE;

	// Save backup configuration
	if ( pBak != NULL )
	{
		CWinFile::BuildPath( cfg, path, pBak );
		saved = SaveRegFile( cfg );
	} // end if

	// Save configuration
	if ( pFile != NULL )
	{
		CWinFile::BuildPath( cfg, path, pFile );
		saved = ( SaveRegFile( cfg ) || saved );
	} // end if

	return TRUE;
}

BOOL CReg::RestoreSettings(LPCTSTR pFile, LPCTSTR pBak, BOOL bSafety )
{_STTEX();
	char cfg[ CWF_STRSIZE ];
	char path[ CWF_STRSIZE ];

	CWinFile::GetExePath( path );

	BOOL loaded = FALSE;
	if ( pFile != NULL )
	{
		// Load configuration
		CWinFile::BuildPath( cfg, path, "pFile" );
		loaded = LoadRegFile( cfg );

		// Set the safety if needed
		if ( bSafety ) 
		{	Set( "Settings", "RunTimes", (DWORD)1 );
			SaveRegFile( cfg );
		} // end if

	} // end if

	// Protect against file damage
	DWORD runs = 0;
	if (	pBak != NULL && 
			( !loaded || 
			!Get( "Settings", "RunTimes", &runs ) ) )
	{	
		CWinFile::BuildPath( cfg, path, pBak );

		loaded = LoadRegFile( cfg );

		// Set the safety if needed
		if ( bSafety ) 
		{	Set( "Settings", "RunTimes", (DWORD)1 );
			SaveRegFile( cfg );
		} // end if

	} // end if

	return loaded;
}


BOOL CReg::IsSet( LPCTSTR pKey, LPCTSTR pName )
{_STTEX();
	CRKey *prk = FindKey( pKey );
	if ( prk == NULL ) return NULL;
	return prk->IsSet( pName );
}


LPREGKEY CReg::FindWhere(LPCTSTR pName, LPCTSTR pValue)
{_STTEX();
	if ( pValue == NULL ) return NULL;

	LPREGKEY prk = NULL;
	while( ( prk = (LPREGKEY)GetNext( prk ) ) != NULL )
		if ( !strcmpi( pValue, prk->key->GetSz( pName ) ) )
			return prk;

	return NULL;
}

LPREGKEY CReg::FindWhere(LPCTSTR pName, DWORD dwValue)
{_STTEX();
	LPREGKEY prk = NULL;
	while( ( prk = (LPREGKEY)GetNext( prk ) ) != NULL )
		if ( dwValue == prk->key->GetDword( pName ) )
			return prk;

	return NULL;
}

BOOL CReg::Copy(CReg *pReg, BOOL bMerge)
{_STTEX();
	if ( !bMerge ) Destroy();
	if ( pReg == NULL ) return FALSE;

	LPREGKEY prk = NULL;
	while( ( prk = (LPREGKEY)pReg->GetNext( prk ) ) != NULL )
	{	CRKey *pRk = GetKey( prk->key->GetName() );
		if ( pRk ) pRk->Copy( prk->key, bMerge );
	} // end while

	return TRUE;
}

BOOL CReg::Set(LPCTSTR pKey, LPCTSTR pName, CReg *pReg)
{_STTEX();

	if ( !pReg || !pReg->Size() )
	{
		// Clear data
		Set( pKey, pName, "" );

		return TRUE;

	} // end if

	// Serialize variables
	CPipe pipe;
	pReg->EncodeUrl( &pipe );

	// Write a NULL character
	pipe.Write( "", 1 );

	// Save the serialized variables
	Set( pKey, pName, (char*)pipe.GetBuffer() );

	return TRUE;
}

BOOL CReg::Get(LPCTSTR pKey, LPCTSTR pName, CReg *pReg)
{_STTEX();

	if ( !pReg ) return FALSE;

	pReg->Destroy();

	return pReg->ReadInline(	(LPBYTE)GetValuePtr( pKey, pName ),
								GetValueSize( pKey, pName ) );
}

BOOL CReg::EncodeUrl(CPipe *pPipe, LPCTSTR pKey, char chSepNameVal, char chSepValues )
{
	CRKey *prk = FindKey( pKey );
	if ( prk == NULL ) return FALSE;

	return prk->EncodeUrl( pPipe, chSepNameVal, chSepValues );
}

BOOL CReg::EncodeUrl(CPipe *pPipe, char chSepNameVal, char chSepValues )
{
	// Ensure valid pipe
	if ( NULL == pPipe ) return FALSE;

	TMem< char > buf;
	LPREGKEY prk = NULL;
	while( ( prk = (LPREGKEY)GetNext( prk ) ) != NULL )
	{
		// Write separator if needed
		if ( pPipe->GetBufferSize() ) pPipe->Write( &chSepValues, 1 );

		LPCTSTR pName = prk->key->GetName();
		CRKey *pRk = GetKey( pName );
		if ( pRk ) 
		{
			// Write the name
			if ( buf.grow( CCfgFile::GetMinCanonicalizeBufferSize( strlen( pName ) ) ) )
			{	buf.Zero();
				CCfgFile::CanonicalizeBuffer( buf, (LPBYTE)pName, strlen( pName ) );
				pPipe->Write( buf );
			} // end if

			// Separator
			pPipe->Write( &chSepNameVal, 1 );

			// Save the key value
			CPipe tpipe;
			prk->key->EncodeUrl( &tpipe, 1, chSepNameVal, chSepValues );
			if ( buf.grow( CCfgFile::GetMinCanonicalizeBufferSize( tpipe.GetBufferSize() ) ) )
			{	buf.Zero();
				CCfgFile::CanonicalizeBuffer( buf, (LPBYTE)tpipe.GetBuffer(), tpipe.GetBufferSize() );
				pPipe->Write( buf );
			} // end if			

		} // end if

	} // end while

	return TRUE;
}

BOOL CReg::ReadInline(LPBYTE pBuf, DWORD dwSize)
{_STTEX();

	// Sanity checks
	if ( pBuf == NULL || dwSize == 0 ) return FALSE;

	DWORD	i = 0;
	char	token[ CWF_STRSIZE ];

	while ( i < dwSize && pBuf[ i ] != 0 )
	{
		DWORD t = 0;

		// Skip white space
		while ( i < dwSize && ( pBuf[ i ] <= ' ' || pBuf[ i ] > '~' ) && pBuf[ i ] != 0 ) i++;
		if ( i >= dwSize || pBuf[ i ] == 0 ) return TRUE;

		// Read in first token
		while ( i < dwSize && t < ( CWF_STRSIZE - 1 ) &&
				pBuf[ i ] && pBuf[ i ] != '=' )
			token[ t++ ] = pBuf[ i++ ];
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
		if ( pBuf[ i ] == '=' )
		{	i++;

			// Skip starting spaces
			while ( i < dwSize && pBuf[ i ] == ' ' ) i++;

			DWORD s = i;
			BOOL quoted = FALSE;

			// Find the end of the data
			while ( i < dwSize &&
					( ( quoted && pBuf[ i ] >= ' ' ) || pBuf[ i ] > ' ' ) && 
					pBuf[ i ] <= '~' && pBuf[ i ] != '&' ) 
			{
				// Toggle quote mode
				if ( pBuf[ i ] == '"' ) quoted = !quoted;

				// Next char
				i++;
			} // end if

			if ( i > s )
			{
				// DeCanonicalize data
				DWORD dwMin = CCfgFile::GetMinDeCanonicalizeBufferSize( i - s );

				TMem< BYTE > mem;
				if ( mem.allocate( dwMin + 1 ) ) 
				{
					// Do it
					DWORD dwSize = 0;
					if ( CCfgFile::DeCanonicalizeBuffer( (LPCTSTR)&pBuf[ s ], (LPBYTE)mem.ptr(), i - s, &dwSize ) ) 
					{
						CRKey *pRk = GetKey( token );
						pRk->ReadInline( mem.ptr(), dwMin );

					} // end if

				} // end if

			} // end if

			// Skip separator
			while ( i < dwSize && pBuf[ i ] == '&' ) i++;

		} // end if

	} // end while

	return TRUE;
}
