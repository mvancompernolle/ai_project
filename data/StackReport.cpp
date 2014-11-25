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
// StackReport.cpp: implementation of the CStackReport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStackReport::CStackReport()
{_STT();
}

CStackReport::~CStackReport()
{_STT();
}

BOOL CStackReport::CallStack(CReg *pReg, CStackTrace *pSt )
{_STT();
	CTlLocalLock ll( *pSt );
	if ( !ll.IsLocked() ) return FALSE;

	// Reset object
	pReg->Destroy();

	DWORD dwThreadId = GetCurrentThreadId();

	char szMsg[ 256 ] = "";
	CStackTrace::iterator it = NULL;
	CStackTrace::list *pList = pSt->GetList();
	while ( NULL != ( it = pList->next( it ) ) )
	{
		wsprintf( szMsg, "Thread %lu ( 0x%lx )", *it->key(), *it->key() );

		CRKey *pRk = pReg->GetKey( szMsg );
		if ( pRk )
		{
			// Flag if this is the current thread
			if ( dwThreadId == *it->key() ) pRk->Set( "current_thread", dwThreadId );
			
			// Save the thread id
			pRk->Set( "thread_id", (DWORD)*it->key() );

			// Get stack information
			UINT uStack = (*it)->GetStackPtr();
			LPCTSTR *pStack = (*it)->GetStack();

			// Save
			if ( uStack ) for ( UINT i = 0, p = uStack - 1; i < uStack; i++, p-- )
			{
				wsprintf( szMsg, "f_%lu", i );

				// If we have a valid string
				if ( pStack[ i ] ) pRk->Set( szMsg, pStack[ p ] );

			} // end for

		} // end if

	} // end while

	return TRUE;
}

BOOL CStackReport::History(CReg *pReg, CStackTrace *pSt )
{_STT();
#ifndef ENABLE_STACK_HISTORY

	return FALSE;

#else

	CTlLocalLock ll( *pSt );
	if ( !ll.IsLocked() ) return FALSE;

	// Reset object
	pReg->Destroy();

	DWORD dwThreadId = GetCurrentThreadId();

	char szMsg[ 256 ] = "";
	CStackTrace::iterator it = NULL;
	CStackTrace::list *pList = pSt->GetList();
	while ( NULL != ( it = pList->next( it ) ) )
	{
		wsprintf( szMsg, "Thread %lu ( 0x%lx )", *it->key(), *it->key() );

		CRKey *pRk = pReg->GetKey( szMsg );
		if ( pRk )
		{
			// Flag if this is the current thread
			if ( dwThreadId == *it->key() ) pRk->Set( "current_thread", dwThreadId );
			
			// Save the thread id
			pRk->Set( "thread_id", (DWORD)*it->key() );

			// Get stack information
			UINT uSize = (*it)->GetMaxHistory();
			UINT uPtr = (*it)->GetHistoryPtr();
			LPCTSTR *pHistory = (*it)->GetHistory();

			// Is there anything in the history?
			if ( uSize )
			{
				// Save
				for ( UINT i = 0; i < uSize; i++ )
				{
					// Adjust pointer
					if ( uPtr ) uPtr--; else uPtr = uSize - 1;

					wsprintf( szMsg, "f_%lu", i );

					if ( pHistory[ uPtr ] ) pRk->Set( szMsg, pHistory[ uPtr ] );

				} // end for

			} // end if

		} // end if

	} // end while

	return TRUE;

#endif

}

BOOL CStackReport::Profile(CReg *pReg, CStackTrace *pSt )
{_STT();
#ifndef ENABLE_STACK_PROFILE

	return FALSE;

#else

	CTlLocalLock ll( *pSt );
	if ( !ll.IsLocked() ) return FALSE;

	// Reset object
	pReg->Destroy();

	DWORD dwThreadId = GetCurrentThreadId();

	char szMsg[ 256 ] = "";
	CStackTrace::iterator it = NULL;
	CStackTrace::list *pList = pSt->GetList();
	while ( NULL != ( it = pList->next( it ) ) )
	{
		wsprintf( szMsg, "Thread %lu ( 0x%lx )", *it->key(), *it->key() );

		CRKey *pRk = pReg->GetKey( szMsg );
		if ( pRk )
		{
			// Flag if this is the current thread
			if ( dwThreadId == *it->key() ) pRk->Set( "current_thread", dwThreadId );
			
			// Save the thread id
			pRk->Set( "thread_id", *it->key() );

			TStrList< CStackTrace::CStack::SProfileItem > *pProfile = (*it)->GetProfile();
			if ( pProfile )
			{
				DWORD i = 0;
				TStrList< CStackTrace::CStack::SProfileItem >::iterator itPi = NULL;
				while ( NULL != ( itPi = pProfile->next( itPi ) ) )
				{
					// Save function name
					wsprintf( szMsg, "f_%lu", i );
					pRk->Set( szMsg, itPi->key() );

					// Save total time
					wsprintf( szMsg, "t_%lu", i );
					pRk->Set( szMsg, (double)( (double)(*itPi)->llTotalTime / (double)CHqTimer::GetTimerFrequency() ) );

					// Next index
					i++;

				} // end while

			} // end if

		} // end if

	} // end while

	return TRUE;
	
#endif
}
