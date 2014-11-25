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
// CsvFile.cpp: implementation of the CCsvFile class.
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

CCsvFile::CCsvFile()
{_STTEX();
	m_col = 0;
	m_row = 0;
	m_rowptr = 0;
	m_pTable = NULL;
	m_pDesc = NULL;
	m_pTypes = NULL;

	m_dwErrors = 0;

	m_dwRecordSize = 4096;
	m_dwRowBlockSize = 16;

	m_ucSep = ',';

	m_dwSkip = 0;
}

CCsvFile::~CCsvFile()
{_STTEX();
	Destroy();
}

void CCsvFile::Destroy()
{_STTEX();
	// Delete table memory
	if ( m_pTable != NULL )
	{
		// Delete column memory
		for ( DWORD i = 0; i < m_row; i++ )
			if ( m_pTable[ i ] != NULL )
			{
				delete[] m_pTable[ i ];
				m_pTable[ i ] = NULL;
			} // end if

		// Delete table
		delete[] m_pTable;
		m_pTable = NULL;

	} // end if

	// Delete descriptions memory
	if ( m_pDesc != NULL )
	{
		delete[] m_pDesc;
		m_pDesc = NULL;
	} // end if

	// Delete type memory
	if ( m_pTypes != NULL )
	{
		delete[] m_pTypes;
		m_pTypes = NULL;
	} // end if

	m_col = 0;
	m_row = 0;
	m_rowptr = 0;

	m_dwErrors = 0;
}

BOOL CCsvFile::AllocateColumns(DWORD col)
{_STTEX();
	Destroy();

	// Do we want any columns?
	if ( col == 0 ) return TRUE;

	// Save number of columns
	m_col = col;

	// Allocate description memory
	m_pDesc = new BYTE[ m_col * m_dwRecordSize ];
	if ( m_pDesc == NULL ) { Destroy(); return FALSE; }
	ZeroMemory( m_pDesc, m_col * m_dwRecordSize );

	// Allocate types memory
	m_pTypes = new BYTE[ m_col ];
	ZeroMemory( m_pTypes, m_col );

	return TRUE;
}


BOOL CCsvFile::AllocateRows(DWORD size)
{_STTEX();
	// Do we already have enough rows?
	if ( size <= m_row ) return TRUE;

	// Sanity check
	if ( m_dwRecordSize == 0 ) return FALSE;

	LPBYTE	*oldtable = m_pTable;

	// Allocate new table memory
	m_pTable = new LPBYTE[ size ];
	if ( m_pTable == NULL )
	{	m_pTable = oldtable;
		return FALSE;
	} // end if
	
	// Initialize memory in case of failure
	ZeroMemory( m_pTable, sizeof( LPBYTE ) * size );

	// Copy old table pointers
	if ( oldtable != NULL )
	{	memcpy( m_pTable, oldtable, sizeof( LPBYTE ) * m_row );
		delete[] oldtable;
	} // end if

	// allocate new rows
	for ( DWORD i = m_row; i < size; i++ )
	{
		// Allocate row memory
		m_pTable[ i ] = new BYTE[ m_col * m_dwRecordSize ];		
		if ( m_pTable[ i ] == NULL )
		{	m_row = i; return FALSE; }
	
		// Init the memory
		ZeroMemory( m_pTable[ i ], m_col * m_dwRecordSize );

	} // end for

	// Set new row size
	m_row = size;

	return TRUE;
}


BOOL CCsvFile::LoadFile(LPCTSTR pFile, BOOL bAdd )
{_STTEX();
	// Lose old record
	if ( !bAdd ) Destroy();

	CWinFile	file;

	// Open the file
	if ( !file.OpenExisting( pFile, GENERIC_READ ) )
		return FALSE;

	// Get file size
	DWORD size = file.Size();
	if ( size == 0 ) return FALSE;

	// Allocate memory
	LPBYTE buf = new BYTE[ size + 1 ];
	if ( buf == NULL ) return FALSE;

	// Read in the data into ram
	DWORD read;
	if ( !file.Read( buf, size, &read ) || read != size )
	{	delete[] buf; return FALSE; }
	buf[ size ] = 0;

	// Load the file
	if ( !LoadFromMem( buf, size, bAdd ) )
	{	delete[] buf; return FALSE; }

	delete buf;

	return TRUE;
}

BOOL CCsvFile::LoadFromMem(LPBYTE buf, DWORD size, BOOL bAdd )
{_STTEX();
	// Lose old record
	if ( !bAdd ) Destroy();

	// Sanity check
	if ( buf == NULL || size == 0 ) return FALSE;

	// Try to make sense out of the buffer
	for ( DWORD i = 0; i < size; )
	{
		if ( m_dwSkip ) m_dwSkip--;
		else
		{
			// Do we need to load the header?
			if ( !IsTable() )
			{	
				if ( !LoadColumns( (LPCTSTR)&buf[ i ] ) ) m_dwErrors++;
			} // end if

			// Skip header if adding
			else if ( i == 0 && bAdd );

			// Add this line as a record
			else if ( !AddLine( (LPCTSTR)&buf[ i ] ) ) m_dwErrors++;

		} // end else

		// Find the next line
		while ( i < size && buf[ i ] >= ' ' && buf[ i ] <= '~' ) i++;
		while ( i < size && ( buf[ i ] < ' ' || buf[ i ] > '~' ) ) i++;
	
	} // end for

	return TRUE;
}

CSV_HROW CCsvFile::AppendNewRow()
{_STTEX();
	// Allocate more rows if needed
	if ( m_rowptr >= m_row )
	{
		if ( !AllocateRows( m_row + m_dwRowBlockSize ) ) return FALSE;
		if ( m_rowptr >= m_row ) return FALSE;
	} // end if

	// Return this row
	DWORD i = m_rowptr;

	// Count a row
	m_rowptr++;

	// Return new row
	return m_pTable[ i ]; 
}

BOOL CCsvFile::AddLine(LPCTSTR pLine)
{_STTEX();
	BOOL bQuoted = FALSE;
	
	// Get new row pointer
	CSV_HROW hrow = AppendNewRow();
	if ( hrow == NULL ) return FALSE;

	// Read in each column
	DWORD b = 0, o = 0, c = 0;
	for ( DWORD i = 0; pLine[ i ] >= ' ' && pLine[ i ] <= '~'; i++ )
	{
		// Handle separator
		if ( !bQuoted && pLine[ i ] == m_ucSep )
		{
			// Terminate string
			hrow[ b + o ] = 0;

			// Next element
			o = 0; b += m_dwRecordSize; c++;

			// Skip multiple separators
			while ( pLine[ i + 1 ] == m_ucSep ) i++;

 			// Are we done?
			if ( c >= m_col ) return TRUE;

		} // end if

		// Store the byte if room
		else if ( o < ( m_dwRecordSize - 1 ) )
		{
			// Turn quoted mode on
			if ( !bQuoted && pLine[ i ] == '\"' ) bQuoted = TRUE;

			// Toggle quoted mode
			else if ( pLine[ i ] == '\"' && pLine[ i + 1 ] != '\"' )
				bQuoted = !bQuoted;

			else
			{
				// Skip escape character
				if ( pLine[ i ] == '\"' ) i++;

				// Save character
				hrow[ b + o++ ] = pLine[ i ];

			} // end else

		} // end else if
		
	} // end for

	// Null terminate string
	hrow[ b + o ] = 0;

	return TRUE;
}

BOOL CCsvFile::LoadColumns(LPCTSTR pLine)
{_STTEX();
	BOOL	bQuoted = FALSE;
	DWORD	col = 0;

	// How many columns will there be?
	DWORD i;
	for ( i = 0; pLine[ i ] >= ' ' && pLine[ i ] <= '~'; i++ )
	{
		// Don't count multiple sep
		if ( pLine[ i ] == m_ucSep ) col++;
		while ( pLine[ i ] == m_ucSep ) i++;
	} // end for

	// Quit if no columns
	if ( col == 0 ) return FALSE;
	col += 1;

	// Allocate the columns
	if ( !AllocateColumns( col ) ) return FALSE;

	// Read in each column
	DWORD b = 0, o = 0, c = 0;
	for ( i = 0; pLine[ i ] >= ' ' && pLine[ i ] <= '~'; i++ )
	{
		// Handle separator
		if ( !bQuoted && pLine[ i ] == m_ucSep )
		{
			// Terminate string
			m_pDesc[ b + o ] = 0;

			// Next element
			o = 0; b += m_dwRecordSize; c++;

			// Skip multiple separators
			while ( pLine[ i + 1 ] == m_ucSep ) i++;

			// Are we done?
			if ( c >= m_col ) return TRUE;

		} // end if

		// Store the byte if room
		else if ( o < ( m_dwRecordSize - 1 ) )
		{
			// Turn quoted mode on
			if ( !bQuoted && pLine[ i ] == '\"' ) bQuoted = TRUE;

			// Toggle quoted mode
			else if ( pLine[ i ] == '\"' && pLine[ i + 1 ] != '\"' )
				bQuoted = !bQuoted;

			else
			{
				// Skip escape character
				if ( pLine[ i ] == '\"' ) i++;

				// Save character
				m_pDesc[ b + o++ ] = pLine[ i ];

			} // end else

		} // end else if
		
	} // end for

	// Null terminate string
	m_pDesc[ b + o ] = 0;

	return TRUE;
}

BOOL CCsvFile::SaveFile(LPCTSTR pFile)
{_STTEX();
	CWinFile	file;

	// Is there a table to save?
	if ( !IsTable() ) return FALSE;

	// Open the file
	if ( !file.OpenNew( pFile, GENERIC_WRITE ) )
		return FALSE;

	// Is there anything to save?
	DWORD size = SaveToMem( NULL, 0 );
	if ( size == 0 ) return FALSE;

	// Allocate memory for table data
	LPBYTE buf = new BYTE[ size + 1 ];
	if ( buf == NULL ) return FALSE;

	// Write the data to a buffer
	if ( SaveToMem( buf, size ) == 0 ) 
	{	delete[] buf; return FALSE; }

	// Write the data to a file
	if ( !file.Write( buf, size ) ) 
	{	delete[] buf; return FALSE; }

	// Lose the memory
	delete[] buf;

	return TRUE;
}

DWORD CCsvFile::SaveToMem(LPBYTE buf, DWORD size)
{_STTEX();
	DWORD ptr = 0;
	DWORD i = 0;

	DWORD rows = GetNumRows();
	DWORD cols = GetNumColumns();

	// Separator string
	char sep[ 2 ] = { m_ucSep, 0 };

	// Anything to save?
	if ( rows == 0 || cols == 0 ) return 0;

	// Write out the header
	for ( i = 0; i < cols; i++ )
	{	LPCTSTR pCol = GetColDesc( i );
		if ( i != 0 ) Copy( buf, &ptr, size, sep );
		if ( pCol != NULL ) Copy( buf, &ptr, size, pCol );
	} // end for

	// CRLF
	Copy( buf, &ptr, size, "\r\n" );

	// For each row
	for ( i = 0; i < rows; i++ )
	{
		CSV_HROW hRow = GetRow( i );
		if ( hRow != NULL )
		{
			// Write out each column element and separator
			for ( DWORD c = 0; c < cols; c++ )
			{	LPCTSTR pStr = (LPCTSTR)GetRowElement( hRow, c );
				if ( c != 0 ) Copy( buf, &ptr, size, sep );
				if ( pStr != NULL ) Copy( buf, &ptr, size, pStr );
			} // end for

			// CRLF
			Copy( buf, &ptr, size, "\r\n" );	
		} // end if

	} // end for

	// Add for NULL character
	ptr += 1;

	return ptr;
}

BOOL CCsvFile::Copy(LPVOID buf, LPDWORD ptr, DWORD size, LPCTSTR str)
{_STTEX();
	DWORD p = 0;
	DWORD l = strlen( str );

	// Get size from caller
	if ( ptr != NULL ) p = *ptr;

	// Sanity checks
	if ( l == 0 ) return TRUE;
	if ( str == NULL ) return FALSE;

	// Copy the string
	if ( buf != NULL )
	{
		DWORD i;
		for ( i = 0; i < l && ( p + i ) < size; i++ )
			( (char*)buf )[ p + i ] = str[ i ];

		// Check for buffer overflow
		if ( p + i >= size ) return FALSE;

		// NULL terminate
		( (char*)buf )[ p + i ] = NULL;
	
	} // end if

	// Add str length
	if ( ptr != NULL ) *ptr += l;

	return TRUE;
}

DWORD CCsvFile::GetRowString(DWORD row, LPSTR pRow, DWORD size)
{_STTEX();
	// Get row handle
	CSV_HROW hRow = GetRow( row );
	if ( hRow == NULL ) return 0;

	DWORD cols = GetNumColumns();

	// Separator string
	char sep[ 2 ] = { m_ucSep, 0 };

	// NULL row
	if ( pRow != NULL ) *pRow = 0;

	DWORD ptr = 0;

	// Write out each column element and separator
	for ( DWORD c = 0; c < cols; c++ )
	{
		LPCTSTR pStr = (LPCTSTR)GetRowElement( hRow, c );
		if ( pStr != NULL )
		{	if ( c != 0 ) Copy( pRow, &ptr, size, sep );
			Copy( pRow, &ptr, size, pStr );			
		} // end if
	} // end for

	// Add NULL character
	ptr += 1;

	return ptr;
}

DWORD CCsvFile::GetHeaderString(LPSTR buf, DWORD size)
{_STTEX();
	DWORD cols = GetNumColumns();

	// Separator string
	char sep[ 2 ] = { m_ucSep, 0 };

	// NULL row
	if ( buf != NULL ) *buf = 0;

	DWORD ptr = 0;

	// Write out each column element and separator
	for ( DWORD c = 0; c < cols; c++ )
	{
		LPCTSTR pCol = GetColDesc( c );
		if ( pCol != NULL )
		{	if ( c != 0 ) Copy( buf, &ptr, size, sep );
			Copy( buf, &ptr, size, pCol );			
		} // end if
	} // end for

	// Add NULL character
	ptr += 1;

	return ptr;
}

BOOL CCsvFile::AddColumn(LPCTSTR pDescription, DWORD insert)
{_STTEX();
	// Sanity check
	if ( m_dwRecordSize == 0 ) return FALSE;

	// Add one more column
	DWORD col = m_col + 1;

	// Allocate description memory
	LPBYTE pDesc = new BYTE[ col * m_dwRecordSize ];
	if ( pDesc == NULL ) return FALSE;
	ZeroMemory( pDesc, col * m_dwRecordSize );

	// Allocate types memory
	LPBYTE pTypes = new BYTE[ col ];
	if ( pTypes == NULL ) { delete [] pDesc; return FALSE; }
	ZeroMemory( pTypes, col );

	// How many rows do we need
	DWORD row = m_row;
	DWORD rowptr = m_rowptr;

	// Allocate new table memory
	LPBYTE *pTable = new LPBYTE[ row ];
	if ( pTable == NULL )
	{	delete [] pDesc; delete [] pTypes; return FALSE; }
	
	// Initialize memory in case of failure
	ZeroMemory( pTable, sizeof( LPBYTE ) * row );

	// allocate new rows
	DWORD i;
	for ( i = 0; i < row; i++ )
	{
		// Allocate row memory
		pTable[ i ] = new BYTE[ col * m_dwRecordSize ];		
		if ( pTable[ i ] == NULL )
		{	long k = i;
			while ( k >= 0 ) { delete [] pTable[ k-- ]; }
			delete [] pTable;
			delete [] pTypes;
			delete [] pDesc;
			return FALSE; 
		}
	
		// Init the memory
		ZeroMemory( pTable[ i ], col * m_dwRecordSize );

	} // end for

	// Copy user description
	if ( insert >= col ) insert = col - 1;
	strncpy( (char*)&pDesc[ insert * m_dwRecordSize ], pDescription, m_dwRecordSize );

	// Copy types and descriptions
	DWORD x = 0;
	for ( i = 0; i < m_col; i++ )
	{
		// Skip insert point
		if ( x == insert ) x++;

		// Copy description
		memcpy( &pDesc[ x * m_dwRecordSize ], &m_pDesc[ i * m_dwRecordSize ], m_dwRecordSize );

		// Copy type
		pTypes[ x ] = m_pTypes[ i ];

		x++;

	} // end for

	// Copy rows
	for ( DWORD r = 0; r < m_rowptr; r++ )
	{	x = 0;
		for ( i = 0; i < m_col; i++ )
		{
			// Skip insert point
			if ( x == insert ) x++;

			// Copy row information
			memcpy( &pTable[ r ][ x * m_dwRecordSize ], &m_pTable[ r ][ i * m_dwRecordSize ], m_dwRecordSize );

			x++;

		} // end for

	} // end for

	// Lose the old csv
	Destroy();

	// Set the new table into variables and pray
	m_pDesc = pDesc;
	m_pTypes = pTypes;
	m_pTable = pTable;
	m_col = col;
	m_row = row;
	m_rowptr = rowptr;

	return TRUE;
}


LPCTSTR CCsvFile::GetElementPtr(CSV_HROW hRow, LPCTSTR pCol, LPCTSTR pDef)
{_STTEX();
	// Sanity checks
	if ( hRow == NULL || pCol == NULL ) return pDef;

	// Find matching column
	DWORD dwCol = FindCol( pCol );
	if ( dwCol == MAXDWORD ) return pDef;

	// Return a pointer to the element value
	return (LPSTR)GetRowElement( hRow, dwCol );
}
