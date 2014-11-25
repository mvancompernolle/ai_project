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
// ErrLog.cpp: implementation of the CErrLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "time.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CErrLog::CErrLog()
{_STT();

}

CErrLog::~CErrLog()
{_STT();
	Destroy();
}

void CErrLog::Destroy()
{_STT();
	CLList::Destroy();
}

BOOL CErrLog::InitObject( void *node )
{_STT();
	// Do initiailization
	if ( !CLList::InitObject( node ) ) return FALSE;

	LPERRORITEMINFO peii = (LPERRORITEMINFO)node;
	if ( peii == NULL ) return FALSE;

	peii->sFile = new CStr;
	if ( peii->sFile == NULL ) return FALSE;

	peii->sFunction = new CStr;
	if ( peii->sFunction == NULL ) return FALSE;

	return TRUE;
}

void CErrLog::DeleteObject( void *node )
{_STT();
	LPERRORITEMINFO peii = (LPERRORITEMINFO)node;
	if ( peii == NULL ) return;

	// Lose the file
	_PTR_DELETE( peii->sFile );
	_PTR_DELETE( peii->sFunction );

	CLList::DeleteObject( node );
}


HRESULT _cdecl CErrLog::Add(	LPCTSTR pFile, DWORD dwLine, LPCTSTR pFunction,
								DWORD dwSeverity, DWORD dwErrCode, LPCTSTR pStr, ... )
{_STT();
	try // This could GPF if caller screws up
	{
		char str[ CWF_STRSIZE * 4 ] = { 0 };
		try
		{
			// Create error string
			LPVOID *args = ( (LPVOID*)&pStr ) + 1;
			if ( pStr != NULL )
				wvsprintf( str, pStr, (va_list)args );
			else strcpy( str, "< NULL >" );

		} catch( ... )
		{	try { if ( pStr != NULL ) strcpy( str, pStr ); 
			} catch( ... ) 
			{ 	strcpy( str, "Bad params passed to log file." ); 
			} // end catch
		} // end catch

		// Create error structure
		LPERRORITEMINFO	peii = (LPERRORITEMINFO)New( NULL, dwErrCode, str );
		if ( peii == NULL ) return dwErrCode;

		// Save error location information
		if ( pFile ) *peii->sFile = pFile; else *peii->sFile = "";
		peii->dwLine = dwLine;
		if ( pFunction ) *peii->sFunction = pFunction; else *peii->sFunction = "";

		// Save error info
		peii->dwCode = dwErrCode;
		peii->dwSeverity = dwSeverity;

		// Save the time of the error
		peii->dwTime = time( NULL );

	} // end try
	
	catch( ... ) { ASSERT( 0 ); return dwErrCode; }

	return dwErrCode;
}

