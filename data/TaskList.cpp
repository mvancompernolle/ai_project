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
//*****************************************************************/
// TaskList.cpp: implementation of the CTaskList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// PSAPI
typedef BOOL (WINAPI *PFN_EnumProcesses)( DWORD * lpidProcess, DWORD cb, DWORD * cbNeeded );
typedef BOOL (WINAPI *PFN_EnumProcessModules)( HANDLE hProcess, HMODULE * lphModule, DWORD cb, LPDWORD lpcbNeeded );
typedef DWORD (WINAPI *PFN_GetModuleFileNameEx)( HANDLE hProcess, HMODULE hModule, LPTSTR lpFilename, DWORD nSize );
typedef DWORD (WINAPI *PFN_GetModuleBaseName)( HANDLE hProcess, HMODULE hModule, LPTSTR lpBaseName, DWORD nSize );
typedef DWORD (WINAPI *PFN_GetProcessMemoryInfo)( HANDLE hProcess, PPROCESS_MEMORY_COUNTERS ppsmemCounters, DWORD cb );
typedef DWORD (WINAPI *PFN_GetModuleInformation)( HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb );

// Toolhelp
typedef HANDLE (WINAPI *PFN_CreateToolhelp32Snapshot)( DWORD dwFlags, DWORD th32ProcessID );
typedef BOOL (WINAPI *PFN_Process32First)( HANDLE hSnapshot, LPPROCESSENTRY32 lppe );
typedef BOOL (WINAPI *PFN_Process32Next)( HANDLE hSnapshot, LPPROCESSENTRY32 lppe );
typedef BOOL (WINAPI *PFN_Module32First)( HANDLE hSnapshot, LPMODULEENTRY32 lpme );
typedef BOOL (WINAPI *PFN_Module32Next)( HANDLE hSnapshot, LPMODULEENTRY32 lpme );
 
// VDMDBG
typedef INT (WINAPI *PFN_VDMEnumTaskWOWEx)( DWORD dwProcessId, TASKENUMPROCEX fp, LPARAM lparam ); 


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTaskList::CTaskList()
{_STT();
	*m_szErr = 0;
}

CTaskList::~CTaskList()
{_STT();

}

BOOL CTaskList::Init( DWORD mode, BOOL bVdmdbg )
{_STT();
	// Is something already loaded?
	if ( mode == AUTO && IsPsapi() || IsToolhelp() ) return TRUE;
	if ( mode == PSAPI && IsPsapi() ) return TRUE;
	if ( mode == TOOLHELP && IsToolhelp() ) return TRUE;

	// Load PSAPI
	if ( mode == AUTO || mode == PSAPI )
		if (	!m_psapi.LoadLibrary( "Psapi.dll" ) ||
				!m_psapi.AddFunction( "EnumProcesses" ) ||			// 0
				!m_psapi.AddFunction( "EnumProcessModules" ) ||		// 1
				!m_psapi.AddFunction( "GetModuleFileNameExA" ) ||	// 2
				!m_psapi.AddFunction( "GetModuleBaseNameA" ) ||		// 3
				!m_psapi.AddFunction( "GetProcessMemoryInfo" ) ||	// 4
				!m_psapi.AddFunction( "GetModuleInformation" ) )	// 5
			m_psapi.Destroy();

	// Load Toolhelp
	if ( ( mode == AUTO && !m_psapi.IsLoaded() ) || mode == TOOLHELP )
		if (	!m_th.LoadLibrary( "Kernel32.dll" ) ||
				!m_th.AddFunction( "CreateToolhelp32Snapshot" ) ||	// 0
				!m_th.AddFunction( "Process32First" ) ||			// 1
				!m_th.AddFunction( "Process32Next" ) ||				// 2
				!m_th.AddFunction( "Module32First" ) ||				// 3
				!m_th.AddFunction( "Module32Next" ) )				// 4
			m_th.Destroy();

	// Load VDMDBG
	if ( bVdmdbg )
		if (	!m_vdmdbg.LoadLibrary( "Vdmdbg.dll" ) ||
				!m_vdmdbg.AddFunction( "VDMEnumTaskWOWEx" ) )	// 0
			m_vdmdbg.Destroy();

	return ( IsPsapi() || IsToolhelp() );
}

void CTaskList::Destroy()
{_STT();
	*m_szErr = 0;
	m_tasks.Destroy();
	m_psapi.Destroy();
	m_th.Destroy();
	m_vdmdbg.Destroy();
}

BOOL CTaskList::EnumProcesses( DWORD mode, BOOL bVdmdbg )
{_STT();
	// Ensure library
	if ( !Init( mode, bVdmdbg ) ) return FALSE;

	try
	{
		if ( IsPsapi() ) return EnumProcPsapi();

		else if ( IsToolhelp() ) return EnumProcToolhelp();

		else return FALSE;

	} 	catch( const char* err ) { strcpy_sz( m_szErr, err ); return FALSE; }
		catch( ... ) { ASSERT( 0 ); return FALSE; }

	return TRUE;
}

BOOL CTaskList::GetTaskInfo(DWORD dwPid, LPTASKINFO pti, BOOL bSubs)
{_STT();
	try
	{
		if ( IsPsapi() ) return GetPsapiInfo( dwPid, pti, bSubs);

		else if ( IsToolhelp() ) return GetToolhelpInfo( dwPid, pti, bSubs );

		else return FALSE;

	} 	catch( const char* err ) { strcpy_sz( m_szErr, err ); return FALSE; }
		catch( ... ) { ASSERT( 0 ); return FALSE; }

	return TRUE;
}

BOOL CTaskList::EnumProcPsapi()
{_STT();
	// Ensure PSAPI module loaded
	if ( !m_psapi.IsLoaded() ) return FALSE;

	// Lose previous tasks
	m_tasks.Destroy();

	TMem< DWORD > processes;
	DWORD size = 64;
	DWORD cbNeeded = 0;
	do
	{	
		// Double size
		size <<= 1;
		if ( !processes.allocate( size ) ) return FALSE;

		// Get process info
		( (PFN_EnumProcesses)m_psapi[ 0 ] )( processes.ptr(), size * sizeof( DWORD ), &cbNeeded );

	// Until we get them all
	} while ( size < 4096 && size <= ( cbNeeded * sizeof( DWORD ) ) );

	// How many processes?
	DWORD dwProcesses = cbNeeded / sizeof( DWORD );

	// For each process
	for ( DWORD i = 0; i < dwProcesses; i++ )
	{
		// Allocate task info memory
		LPTASKINFO pti = (LPTASKINFO)m_tasks.New();

		// Just get process info
		if ( pti != NULL )
		{
			// Get 16-bit process
			if ( !strcmpi( pti->fname, "ntvdm.exe" ) && m_vdmdbg.IsLoaded() )
				( (PFN_VDMEnumTaskWOWEx)m_vdmdbg[ 0 ] )( pti->dwPid, &Enum16Proc, (LPARAM)pti );

			// Try to get Psapi info
			else if ( !GetPsapiInfo( processes[ i ], pti, FALSE ) ) m_tasks.Delete( pti );

		} // end if

	} // end for

	return TRUE;
}

BOOL CTaskList::GetPsapiInfo(DWORD dwPid, LPTASKINFO pti, BOOL bSubs)
{_STT();
	// Ensure PSAPI module loaded
	if ( !m_psapi.IsLoaded() || pti == NULL ) return FALSE;

	// Save process info
	if ( !bSubs ) pti->dwPid = dwPid;
	
	// Get process handle
	HANDLE hProcess = OpenProcess(	PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
									FALSE, dwPid );
	if ( hProcess == NULL ) return FALSE;

	DWORD size = 64;
	DWORD cbNeeded = 0;
	TMem< HMODULE > mod;

	do
	{	// Double size
		size <<= 1;
		if ( !mod.allocate( size ) ) return FALSE;

		// Get module info
		if ( !( (PFN_EnumProcessModules)m_psapi[ 1 ] )( hProcess, mod.ptr(), size * sizeof( HMODULE ), &cbNeeded ) ) 
			return FALSE;

	// Until we get them all
	} while ( size < 4096 && size <= ( cbNeeded * sizeof( HMODULE ) ) );

	// How many modules?
	DWORD dwModules = cbNeeded / sizeof( HMODULE );
	if ( !bSubs ) { pti->dwModules = dwModules; dwModules = 1; }

	// Create sub list if needed
	CTask *pTask = NULL;
	if ( bSubs ) 
	{	pTask = m_tasks.CreateSubList( pti );
		if ( pTask == NULL ) return FALSE;
		pTask->Destroy();
	} // end if

	// Enum modules
	for ( DWORD m = 0; m < dwModules; m++ )
	{
		char str[ CWF_STRSIZE ];

		if ( !bSubs || ( bSubs && m > 0 ) )
		{
			// Get module file name
			if ( ( (PFN_GetModuleFileNameEx)m_psapi[ 2 ] )( hProcess, mod.ptr()[ m ], str, sizeof( str ) ) )
			{ 	strcpy_sz( pti->file, str );
				CWinFile::GetFileNameFromPath( str, pti->fname );
			} // end if

			// Get module base name
			if ( ( (PFN_GetModuleBaseName)m_psapi[ 3 ] )( hProcess, mod.ptr()[ m ], str, sizeof( str ) ) )
			{ 	strcpy_sz( pti->name, str );
			} // end if

			PROCESS_MEMORY_COUNTERS pmc;
			if ( ( (PFN_GetProcessMemoryInfo)m_psapi[ 4 ] )( hProcess, &pmc, sizeof( pmc ) ) )
			{	pti->dwPageFaultCount = pmc.PageFaultCount;
				pti->dwPeakWorkingSetSize = pmc.PageFaultCount;
				pti->dwWorkingSetSize = pmc.WorkingSetSize;
				pti->dwQuotaPeakPagedPoolUsage = pmc.QuotaPeakPagedPoolUsage;
				pti->dwQuotaPagedPoolUsage = pmc.QuotaPagedPoolUsage;
				pti->dwQuotaPeakNonPagedPoolUsage = pmc.QuotaPeakNonPagedPoolUsage;
				pti->dwQuotaNonPagedPoolUsage = pmc.QuotaNonPagedPoolUsage;
				pti->dwPageFileUsage = pmc.PagefileUsage;
				pti->dwPeakPageFileUsage = pmc.PeakPagefileUsage;
			} // end if

			MODULEINFO mi;
			if ( ( (PFN_GetModuleInformation)m_psapi[ 5 ] )( hProcess, mod.ptr()[ m ], &mi, sizeof( mi ) ) )
			{	pti->dwModuleSize = mi.SizeOfImage;
				pti->pBaseAddr = mi.lpBaseOfDll;
				pti->pEntryPoint = mi.EntryPoint;

				MEMORY_BASIC_INFORMATION mbi;
				if ( VirtualQuery( mi.EntryPoint, &mbi, sizeof( mbi ) ) )
				{
					int x = 0;

				} // end if

			} // end if
			

		} // end if
		
		// Create new module info if needed
		if ( pTask != NULL ) pti = (LPTASKINFO)pTask->New();

	} // end for

	// Close the process handle
	CloseHandle( hProcess );

	return TRUE;
}

BOOL CTaskList::EnumProcToolhelp()
{_STT();
	// Ensure Kernal32 module loaded
	if ( !m_th.IsLoaded() ) return FALSE;

	// Lose previous tasks
	m_tasks.Destroy();

	// Get process snapshot
	HANDLE hProcessSnap = ( (PFN_CreateToolhelp32Snapshot)m_th[ 0 ] )( TH32CS_SNAPPROCESS, 0 );
	if ( hProcessSnap == INVALID_HANDLE_VALUE ) return FALSE;
	
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof( pe32 );

	// Get info on first process
	if ( !( (PFN_Process32First)m_th[ 1 ] )( hProcessSnap, &pe32 ) ) 
	{	CloseHandle( hProcessSnap ); return FALSE; }

	// Skip first process.  (It's a duplicate or our own process)
	( (PFN_Process32Next)m_th[ 2 ] )( hProcessSnap, &pe32 );

	do
	{
		// Allocate task info memory
		LPTASKINFO pti = (LPTASKINFO)m_tasks.New();

		// Just get process info
		if ( pti != NULL )
		{
			// Save process ID
			pti->dwPid = pe32.th32ProcessID;

			// Get file name
			strcpy_sz( pti->file, pe32.szExeFile );
			CWinFile::GetFileNameFromPath( pti->file, pti->fname );
			strcpy_sz( pti->name, pti->fname );

			// Save other variables
			pti->dwReferences = pe32.cntUsage;
			pti->dwThreads = pe32.cntThreads;
			pti->dwParentPid = pe32.th32ParentProcessID; 
			pti->dwDefaultHeapId = pe32.th32DefaultHeapID;
			pti->dwModuleId = pe32.th32ModuleID;

			// Get 16-bit process
			if ( !strcmpi( pti->fname, "ntvdm.exe" ) && m_vdmdbg.IsLoaded() )
				( (PFN_VDMEnumTaskWOWEx)m_vdmdbg[ 0 ] )( pti->dwPid, &Enum16Proc, (LPARAM)pti );

			// Get extended module information
			else GetToolhelpInfo( pti->dwPid, pti, FALSE );

			// Delete if nothing
			if ( strlen( pti->file ) < 4  ) m_tasks.Delete( pti );

		} // end if

	// Each process
	} while ( ( (PFN_Process32Next)m_th[ 2 ] )( hProcessSnap, &pe32 ) );  

	// Close the handle
	CloseHandle( hProcessSnap );

	return TRUE;
}

BOOL CTaskList::GetToolhelpInfo(DWORD dwPid, LPTASKINFO pti, BOOL bSubs)
{_STT();
	// Ensure Kernal32 module loaded
	if ( !m_th.IsLoaded() || pti == NULL ) return FALSE;

	// Get process snapshot
	HANDLE hModuleSnap = ( (PFN_CreateToolhelp32Snapshot)m_th[ 0 ] )( TH32CS_SNAPMODULE, dwPid );
	if ( hModuleSnap == INVALID_HANDLE_VALUE ) return FALSE;

	MODULEENTRY32 me32;
	me32.dwSize = sizeof( me32 );

	// Get info on first process
	if ( !( (PFN_Module32First)m_th[ 3 ] )( hModuleSnap, &me32 ) ) 
	{	CloseHandle( hModuleSnap ); return FALSE; }

	LPTASKINFO ptiParent = pti;

	// Create sub list if needed
	CTask *pTask = NULL;
	if ( bSubs ) 
	{	pTask = m_tasks.CreateSubList( pti );
		if ( pTask == NULL ) return FALSE;
		pTask->Destroy();
	} // end if

	DWORD m = 0;
	do
	{
		// Just get process info
		if ( pti != NULL )
		{
			if ( m > 0 )
				// Save process ID
				pti->dwPid = me32.th32ProcessID;

			// Get file name
			strcpy_sz( pti->file, me32.szExePath );
			CWinFile::GetFileNameFromPath( pti->file, pti->fname );
			strcpy_sz( pti->name, me32.szModule );

			// Save other variables
			pti->dwReferences = me32.GlblcntUsage;
			pti->dwModuleId = me32.th32ModuleID;
			pti->pBaseAddr = me32.modBaseAddr;
			pti->dwModuleSize = me32.modBaseSize;
			pti->dwProccntUsage = me32.ProccntUsage;
			pti->dwGlblcntUsage = me32.GlblcntUsage;

		} // end if

		if ( pTask != NULL )
		{	m++;
			ptiParent->dwTotalMemory += pti->dwModuleSize;
			pti = (LPTASKINFO)pTask->New();						
		} // end if

	// Each process
	} while ( ( bSubs || pTask != NULL ) && ( (PFN_Module32Next)m_th[ 4 ] )( hModuleSnap, &me32 ) );  

	// Check for sub modules
	if ( !bSubs && ( (PFN_Module32Next)m_th[ 4 ] )( hModuleSnap, &me32 ) )
		pti->dwModules = 2;

	// Close the handle
	CloseHandle( hModuleSnap );

	return TRUE;
}

void* CTaskList::GetPreferredBase(DWORD dwPid, void *pModule)
{_STT();
	// Open process
	HANDLE hProcess = OpenProcess( PROCESS_VM_READ, FALSE, dwPid );
	if ( hProcess == NULL ) return NULL;

	IMAGE_DOS_HEADER idh;
	IMAGE_NT_HEADERS inh;
	ZeroMemory( &idh, sizeof( idh ) );
	ZeroMemory( &inh, sizeof( inh ) );

	//Read DOS header
	ReadProcessMemory( hProcess, pModule, &idh, sizeof( idh ), NULL );

	// Read NT headers if any
	if ( IMAGE_DOS_SIGNATURE != idh.e_magic )
	{	CloseHandle( hProcess ); return FALSE; }

	// Read process memory
    ReadProcessMemory( hProcess, (LPBYTE)pModule + idh.e_lfanew, &inh, sizeof( inh ), NULL );

	// Close process handle
	CloseHandle( hProcess ); 
    
	// NT header ok?
	if ( IMAGE_NT_SIGNATURE != inh.Signature ) return NULL;

	// Return base
	return (LPVOID)inh.OptionalHeader.ImageBase;
}

BOOL CALLBACK CTaskList::Enum16Proc( DWORD dwThreadId, WORD hMod16, WORD hTask16, LPSTR pszModName, LPSTR pszFileName, LPARAM lpUserDefined )
{_STT();
	LPTASKINFO pti = (LPTASKINFO)lpUserDefined;
	if ( pti == NULL ) return FALSE;

	CTask *pTask = CTask::CreateSubList( pti );
	if ( pTask == NULL ) return FALSE;

	LPTASKINFO spti = (LPTASKINFO)pTask->New();
	if ( spti == NULL ) return FALSE;

	// Save info
	spti->dwModuleId = hMod16;
	spti->dwThreadId = dwThreadId;
	spti->dwTaskId = hTask16;

	// Get file name
	memcpy( spti, pti, sizeof( TASKINFO ) );
	if ( pszFileName != NULL ) strcpy_sz( spti->file, pszFileName );
	CWinFile::GetFileNameFromPath( spti->file, spti->fname );

	// Get module name
	if ( pszModName != NULL ) strcpy_sz( spti->name, pszModName );

	return TRUE;
}

LPTASKINFO CTaskList::FindByPid( HMODULE hModule )
{_STT();
	// Get tasks
	CTask *pTask = GetTasks();
	if ( pTask == NULL ) return NULL;

	// Search for match
	LPTASKINFO pti = NULL;
	while ( ( pti = (LPTASKINFO)pTask->GetNext( pti ) ) != NULL )
	{
		// Is this the module?
		if ( (DWORD)pti->dwPid == (DWORD)hModule ) return pti;

		// Get sub tasks
//		EnumModules( pti );
//		if ( pti->subtasks != NULL )
//		{
//			LPTASKINFO spti = NULL;
//			while ( ( spti = (LPTASKINFO)pti->subtasks->GetNext( spti ) ) != NULL )
//				if ( (DWORD)spti->dwPid == (DWORD)hModule ) return spti;
//		} // end if

	} // end while

	
	return NULL;
}

LPTASKINFO CTaskList::FindTask(LPCTSTR pFile, LPCTSTR pName)
{_STT();
	// Enumerate tasks if none
	if ( m_tasks.Size() == 0 )
	{	EnumProcesses();
		if ( m_tasks.Size() == 0 ) return FALSE;
	} // end if

	LPTASKINFO pti = NULL;
	while( ( pti = (LPTASKINFO)m_tasks.GetNext( pti ) ) != NULL )
	{
		// Match file name
		BOOL bFoundFile = FALSE;
		if ( !pFile || !*pFile ) bFoundFile = TRUE;
		else bFoundFile = CWinFile::MatchPattern( pti->file, pFile );
				
		// Match application name
		BOOL bFoundName = FALSE;
		if ( !pName || !*pName ) bFoundName = TRUE;
		else bFoundName = CWinFile::MatchPattern( pti->name, pName );

		// How did it turn out?
		if ( bFoundFile && bFoundName )
			return pti;		

	} // end while

	return NULL;
}



