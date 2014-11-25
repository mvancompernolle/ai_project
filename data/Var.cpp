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
// Var.cpp: implementation of the CVar class.
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

CVar::CVar()
{_STTEX();
}

CVar::~CVar()
{_STTEX();
	Destroy();
}

void CVar::DeleteObject(void *node)
{_STTEX();
	LPVAR pv = (LPVAR)node;
	if ( pv == NULL ) return;

	// Release memory
	if ( pv->size > 0 && pv->val != NULL )
		delete [] pv->val;

	CLList::DeleteObject( node );
}

LPVAR CVar::AddVar(LPCTSTR pVar, DWORD dwType, LPVOID pVal, DWORD dwSize)
{_STTEX();
	// Sanity checks
	if ( pVar == NULL ) return NULL;

	// Does the variable already exist?
	BOOL  add = FALSE;
	LPVAR node = FindVar( pVar );
	if ( node != NULL )
	{
		// Delete variable if needed
		if ( node->size != 0 && node->val != NULL ) 
		{
			delete [] node->val;
			node->val = NULL;
		} // end if
	} // end if
	else
	{
		// Allocate memory
		node = (LPVAR)New();
		if ( node == NULL ) return NULL;

	} // end if

	// Save variable type
	node->type = dwType;

	// Copy variable name
	strcpy_sz( node->name, pVar );

	// Copy variable size
	node->size = dwSize;

	// Copy the data
	if ( dwSize == 0 ) node->val = pVal;
	else
	{
		// Allocate variable value memory plus one for NULL char
		node->val = new LPBYTE[ dwSize + 1 ];
		if ( node->val == NULL ) { Delete( node ); return NULL; }

		// Check for init data
		if ( pVal == NULL ) ZeroMemory( node->val, dwSize );
		
		// Copy the value
		else memcpy( node->val, pVal, dwSize );

		// NULL terminate for good measure
		( (LPBYTE)node->val )[ dwSize ] = 0;

	} // end else

	return node;
}


LPVAR CVar::FindVar(LPCTSTR pVar)
{_STTEX();
	// Sanity check
	if ( pVar == NULL ) return FALSE;

	// Find the variable
	LPVAR pv = NULL;
	while ( ( pv = (LPVAR)GetNext( pv ) ) != NULL )
		if ( !strcmpi( pv->name, pVar ) )
			return pv;

	return NULL;
}


BOOL CVar::RemoveVar(LPCTSTR pVar)
{_STTEX();
	LPVAR node = FindVar( pVar );
	if ( node == NULL ) return FALSE;
	Delete( node );
	return TRUE;
}


BOOL CVar::ReadMIME(LPBYTE buf, DWORD size)
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
					AddVar( token, VAR_STR, (LPSTR)&buf[ i ], e - i + 1 );

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

BOOL CVar::ReadInline(LPBYTE buf, DWORD size, char sep, BOOL bDeCanonicalize)
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
		while ( i < size &&
				buf[ i ] > ' ' && buf[ i ] <= '~' &&
				buf[ i ] != '=' )
			token[ t++ ] = buf[ i++ ];
		token[ t ] = 0;

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

							// Add variable
							AddVar( token, VAR_STR, mem.ptr(), dwSize );

					} // end if

				} // end if

				// Add variable if any
				else AddVar( token, VAR_STR, &buf[ s ], i - s );

			} // end if

			// Skip separator
			if ( buf[ i ] == sep ) i++;

		} // end if

		// Add null
		else AddVar( token, VAR_VOID, NULL, 0 );

	} // end while

	return TRUE;
}


BOOL CVar::Replace(CPipe *out, LPDWORD op, LPCTSTR in, DWORD dwin, LPCTSTR pBegin, LPCTSTR pEnd, LPSTR pBreak, CVar *params, LPDWORD pdwBreak, char sep)
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

	// Lose old params
	if ( params != NULL ) params->Destroy();

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

			DWORD e = i;
			while ( e > p && in[ e - 1 ] == ' ' ) e--;

			// Read in vars if any
			if ( e > p && params != NULL )
				params->ReadInline( (LPBYTE)&in[ p ], e - p, sep );

			// Skip end tag
			if ( !strnicmp( &in[ i ], endrep, dwendrep ) ) i += dwendrep;

			// Point to start of good data
			s = i;

			LPVAR pv = FindVar( token );
			if ( pv != NULL )
			{				
				// Check for break
				if ( pBreak != NULL && pv->type == VAR_VOID )
				{	if ( pdwBreak != NULL ) *pdwBreak = i;
					strcpy( pBreak, token );
					return TRUE;
				} // end if

				// Write out the replace value
				else out->Write( pv->val, pv->size );

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


BOOL CVar::Replace(LPSTR out, LPDWORD op, DWORD dwout, LPCTSTR in, DWORD dwin, LPCTSTR pBegin, LPCTSTR pEnd, LPSTR pBreak, CVar *params, LPDWORD pdwBreak, char sep)
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

	// Lose old params
	if ( params != NULL ) params->Destroy();

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
				*op += Write( out, *op, dwout, (LPVOID)&in[ s ], i - s );
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

			DWORD e = i;
			while ( e > p && in[ e - 1 ] == ' ' ) e--;

			// Read in vars if any
			if ( e > p && params != NULL )
				params->ReadInline( (LPBYTE)&in[ p ], e - p, sep );

			// Skip end tag
			if ( !strnicmp( &in[ i ], endrep, dwendrep ) ) i += dwendrep;

			// Point to start of good data
			s = i;

			LPVAR pv = FindVar( token );
			if ( pv != NULL )
			{				
				// Check for break
				if ( pBreak != NULL && pv->type == VAR_VOID )
				{	if ( pdwBreak != NULL ) *pdwBreak = i;
					strcpy( pBreak, token );
					return TRUE;
				} // end if

				// Write out the replace value
				else *op += Write( out, *op, dwout, pv->val, pv->size );

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
	if ( i != s ) 
	{
		*op += Write( out, *op, dwout, (LPVOID)&in[ s ], i - s );
	} // end if

	// NULL terminate
	out[ *op ] = 0;

	return FALSE;
}

DWORD CVar::Write(LPVOID dst, DWORD ptr, DWORD max, LPVOID src, DWORD size)
{_STTEX();
	// Ensure buffer space left
	if ( ptr >= max ) return 0;

	// How much can we copy?
	if ( size > max - ptr ) size = max - ptr;

	// Copy the data
	memcpy( &( (LPBYTE)dst )[ ptr ], src, size );

	return size;
}
