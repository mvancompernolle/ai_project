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
// vErr.cpp: implementation of the CvErr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CErrLog		*CvErr::m_errlog = NULL;

CvErr::CvErr()
{_STT();
}

CvErr::~CvErr()
{_STT();
}

HRESULT CvErr::Failed(HRESULT hr)
{_STT();
	return ( hr != S_OK );
}

HRESULT CvErr::Succeeded(HRESULT hr)
{_STT();
	return ( hr == S_OK );
}

LONG WINAPI CvErr::ExceptionHandler( struct _EXCEPTION_POINTERS *ExceptionInfo )
{_STT();

	// Don't recurse crash reporting
	static BOOL bCrashInProgress = FALSE;
	if ( !bCrashInProgress )
	{
		// Crash reporting
		bCrashInProgress = TRUE;

		// Save error log
		CvErr::CreateErrorLog( TRUE, FALSE );

		// Crash Complete
		bCrashInProgress = FALSE;

	} // end if

	return 0;
}

BOOL CvErr::CreateErrorLog(BOOL bCrash, BOOL bTest)
{_STT();

	// Create path to logs folder
	char szPath[ CWF_STRSIZE ] = "";
	CWinFile::GetExePath( szPath );
	CWinFile::BuildPath( szPath, szPath, "logs" );

	// Add time to path if crash
	if ( bCrash )
	{
		CWinTime wt;
		char szTime[ CWF_STRSIZE ] = "";
		wt.GetString( szTime, "%Y_%c_%d__%g_%m_%s" );
		CWinFile::BuildPath( szPath, szPath, szTime );

	} // end if

	// Create logs directory if needed
	if ( !CWinFile::DoesExist( szPath ) )
		CWinFile::CreateDirectory( szPath );

	// Log file
	char szLog[ CWF_STRSIZE ] = "";

	// *** Write Error log
	if ( CvErr::m_errlog )
	{
		// Path for error log file
		CWinFile::BuildPath( szLog, szPath, "ErrorLog.txt" );

		CWinFile f;
		if ( f.OpenNew( szLog ) )
		{
			LPERRORITEMINFO pEii = NULL;
			while ( ( pEii = (LPERRORITEMINFO)CvErr::m_errlog->GetNext( pEii ) ) != NULL )
			{
				char szMsg[ 1024 * 4 ] = { 0 };

				CWinTime wt;
				wt.SetUnixTime( pEii->dwTime );
				wt.GetString( szMsg, "%w %b %D %Y - %g:%m:%s" );

				f.Write( szMsg );
				f.Write( "\r\n -> " );
				
				f.Write( pEii->cpkey );
				f.Write( "\r\n -> " );

				CWin32::GetSystemErrorMsg( pEii->dwCode, szMsg );
				f.Write( szMsg );				
				f.Write( "\r\n\r\n" );

			} // end while

		} // end if

	} // end if
	
	// Do they want crash information?
	if ( !bCrash ) return FALSE;

	// Do we have a stack track object>
	if ( CStackTrace::m_pst )
	{
		// *** Stack trace
		CReg reg;

		CWinFile::BuildPath( szLog, szPath, "Callstack.txt" );
		CStackReport::CallStack( &reg, CStackTrace::m_pst );
		reg.SaveRegFile( szLog, NULL, "Callstack" );

		CWinFile::BuildPath( szLog, szPath, "StackTrace.txt" );
		CStackReport::History( &reg, CStackTrace::m_pst );
		reg.SaveRegFile( szLog, NULL, "Stack Trace" );

		CWinFile::BuildPath( szLog, szPath, "Profile.txt" );
		CStackReport::Profile( &reg, CStackTrace::m_pst );
		reg.SaveRegFile( szLog, NULL, "Profile" );

	} // end if

	return TRUE;
}

BOOL CvErr::SetDefaultExceptionHandler()
{
	// Set our exception handler
	SetUnhandledExceptionFilter( &CvErr::ExceptionHandler ); 

	return TRUE;
}
