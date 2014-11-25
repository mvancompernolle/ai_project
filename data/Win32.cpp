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
// Win32.cpp: implementation of the CWin32 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <shellapi.h>

#include <WinSock2.h>
#include <WinInet.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWin32::CWin32()
{_STT();

}

CWin32::~CWin32()
{_STT();

}

typedef HRESULT (WINAPI *SHAUTOCOMPLETEFN) (HWND hTarget, DWORD dwFlags);
BOOL CWin32::AutoComplete(HWND hTarget, DWORD dwFlags)
{_STT();
	if ( hTarget == NULL ) return FALSE;

	HINSTANCE hSHLWAPIDLL = NULL;
	hSHLWAPIDLL= LoadLibrary("SHLWAPI.DLL");
	if ( hSHLWAPIDLL== NULL ) return FALSE;

    SHAUTOCOMPLETEFN pSHAC = 
		(SHAUTOCOMPLETEFN)GetProcAddress(	hSHLWAPIDLL, 
											"SHAutoComplete" );

    BOOL ret = SUCCEEDED( pSHAC( hTarget, dwFlags ) );

	FreeLibrary( hSHLWAPIDLL );

    return ret;
}

BOOL CWin32::Reboot()
{_STT();
	// Get operating system info
	OSVERSIONINFO  osVersion ;
	osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
	if ( !::GetVersionEx( &osVersion ) )
		osVersion.dwPlatformId = VER_PLATFORM_WIN32_WINDOWS;
	
	// NT doesn't just let us reboot the computer
	if ( osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT ) 
	{
		HANDLE				hToken;
		TOKEN_PRIVILEGES	tp;

		// Get our token information
		if ( OpenProcessToken(	GetCurrentProcess(), 
								TOKEN_ADJUST_PRIVILEGES |
								TOKEN_QUERY,
								&hToken ) )
		{
			// Get 
			if ( LookupPrivilegeValue(	NULL,
										SE_SHUTDOWN_NAME,
										&tp.Privileges[ 0 ].Luid ) )
			{
				// Set up structure
				tp.PrivilegeCount = 1;
				tp.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;

				// Attempt to adjust security token
				AdjustTokenPrivileges( hToken, FALSE, &tp, 0, NULL, 0 );
			} // end if

		} // end if

	} // end NT

	// Give the command
	return ExitWindowsEx( EWX_REBOOT, 0 );
}

BOOL CWin32::StartNewApp()
{_STT();
	// Get exe file
	char exe[ CWF_STRSIZE ];
	GetModuleFileName( NULL, exe, sizeof( exe ) );

	// Get current directory
	char dir[ CWF_STRSIZE ];
	GetCurrentDirectory( sizeof( dir ), dir );

	// Restart the app
	return ( ( (DWORD)ShellExecute( NULL, "open", exe, NULL, dir, SW_SHOWNORMAL ) ) > 32 );
}

BOOL CWin32::RunModalLoop(HWND hWnd)
{_STT();
/*	// Must have window
	if ( !IsWindow( hWnd ) ) return FALSE;

    int nResult = IDABORT;
	
    // allow OLE servers to disable themselves
//    CWinApp* pApp = AfxGetApp();
//  pApp->EnableModeless( FALSE );

    // Get active, foreground and focus window
    HWND hActive = ::GetActiveWindow();
    HWND hFore = ::GetForegroundWindow();
    HWND hFocus = ::GetFocus();

    // Notify window with capture
    HWND hWndCapture = ::GetCapture();
    if( hWndCapture )
        ::SendMessage( hWndCapture, WM_CANCELMODE, 0, 0 );

    // Required
    EnableWindow( hWnd, TRUE );

    // Set this as active...
    SetActiveWindow( hWnd );
    // ...foreground...
    SetForegroundWindow( hWnd );
    // ... and focus window
    SetFocus( hWnd );
    
	HWND hParent = ::GetParent( hWnd );
	while ( ::IsWindow( hParent ) )
	{	::EnableWindow( hParent, FALSE ); hParent = ::GetParent( hParent ); }

    // Run the modal loop
//    nResult = RunModalLoop();

    // Destroy window on exit
    DestroyWindow( hWnd );

	// Get parent window
	hParent = ::GetParent( hWnd );
	while ( ::IsWindow( hParent ) )
	{	::EnableWindow( hParent, TRUE ); hParent = ::GetParent( hParent ); }
	
    // allow OLE servers to enable themselves
    pApp->EnableModeless( TRUE );

    // Restore active, foreground and focused windows
    ::SetActiveWindow( hActive );
    ::SetForegroundWindow( hFore );
    ::SetFocus( hFocus );

    return nResult;
*/
	return FALSE;
}

DWORD CWin32::GetIEVersion()
{_STT();
	// Load the library
	CModule	mod;
	if (	mod.LoadLibrary( "shdocvw.dll" ) &&
			mod.AddFunction( "DllGetVersion" ) )		// 0
	{
		DLLVERSIONINFO    dvi;
		ZeroMemory(&dvi, sizeof(dvi));
		dvi.cbSize = sizeof(dvi);

		// Get the module version number
		if (	SUCCEEDED( ( (DLLGETVERSIONPROC)mod[ 0 ] )( &dvi ) ) &&
				( dvi.dwMajorVersion != 0 || dvi.dwMinorVersion != 0 || dvi.dwBuildNumber != 0 ) )
			return VER_FULL( dvi.dwMajorVersion, dvi.dwMinorVersion, dvi.dwBuildNumber );
	} // end if

	return 0;
}


#define XP_LWA_COLORKEY		1
#define XP_LWA_ALPHA		2
#define XP_WS_EX_LAYERED	0x80000
typedef BOOL (WINAPI *PFN_Slwa)( HWND, COLORREF, BYTE, DWORD );

BOOL CWin32::SetTranslucency(HWND hWnd, DWORD dwTl)
{_STT();
	// Ensure window
	if ( !::IsWindow( hWnd ) ) return FALSE;

	// Are we translucent?
	if ( dwTl >= 255 )
	{
		::SetWindowLong (	hWnd, GWL_EXSTYLE, 
							GetWindowLong( hWnd, GWL_EXSTYLE ) & 
							~XP_WS_EX_LAYERED );
		return TRUE;
	} // end else

	// Set layered style
	else ::SetWindowLong (	hWnd, GWL_EXSTYLE, 
							GetWindowLong( hWnd, GWL_EXSTYLE ) | 
							XP_WS_EX_LAYERED );

	// Get user32 module handle
	HMODULE hUser32 = LoadLibrary ( "user32" );	
	if ( hUser32 == NULL ) return FALSE;

	BOOL bSet = FALSE;

	// Get function pointer
	PFN_Slwa pSlwa = 
		(PFN_Slwa)GetProcAddress( hUser32, "SetLayeredWindowAttributes" );

	// Set attribute
	if ( pSlwa != NULL ) 
		bSet = pSlwa( hWnd, 0, 200, XP_LWA_ALPHA );

	// Free lib
	FreeLibrary( hUser32 );

	return bSet;
}

BOOL CWin32::SetTransparency(HWND hWnd, COLORREF rgbTransparent)
{_STT();
	// Ensure window
	if ( !::IsWindow( hWnd ) ) return FALSE;

	// Check for invalid color flag
	if ( rgbTransparent == MAXDWORD )
	{
		::SetWindowLong (	hWnd, GWL_EXSTYLE, 
							GetWindowLong( hWnd, GWL_EXSTYLE ) & 
							~XP_WS_EX_LAYERED );
		return TRUE;
	} // end else

		// Set layered style
	else ::SetWindowLong (	hWnd, GWL_EXSTYLE, 
							GetWindowLong( hWnd, GWL_EXSTYLE ) | 
							XP_WS_EX_LAYERED );

	// Get user32 module handle
	HMODULE hUser32 = LoadLibrary ( "user32" );	
	if ( hUser32 == NULL ) return FALSE;

	BOOL bSet = FALSE;

	// Get function pointer
	PFN_Slwa pSlwa = 
		(PFN_Slwa)GetProcAddress( hUser32, "SetLayeredWindowAttributes" );

	// Set attribute
	if ( pSlwa != NULL ) 
		bSet = pSlwa( hWnd, rgbTransparent, 0, XP_LWA_COLORKEY );

	// Free lib
	FreeLibrary( hUser32 );

	return bSet;
}

typedef LONG (WINAPI *tQuerySystemInfo)(UINT,PVOID,ULONG,PULONG);

typedef struct tagSYSBASICINFO
{
    DWORD			dwUnknown1;
    ULONG			uKeMaximumIncrement;
    ULONG			uPageSize;
    ULONG			uMmNumberOfPhysicalPages;
    ULONG			uMmLowestPhysicalPage;
    ULONG			uMmHighestPhysicalPage;
    ULONG			uAllocationGranularity;
    PVOID			pLowestUserAddress;
    PVOID			pMmHighestUserAddress;
    ULONG			uKeActiveProcessors;
    BYTE			bKeNumberProcessors;
    BYTE			bUnknown2;
    WORD			wUnknown3;
} SYSBASICINFO, *LPSYSBASICINFO; // end typedef struct

typedef struct tagSYSPERFINFO
{
    LARGE_INTEGER   liIdleTime;
    DWORD           dwSpare[76];
} SYSPERFINFO, *LPSYSPERFINFO; // end typedef struct

typedef struct tagSYSTIMEINFO
{
    LARGE_INTEGER	liKeBootTime;
    LARGE_INTEGER	liKeSystemTime;
    LARGE_INTEGER	liExpTimeZoneBias;
    ULONG			uCurrentTimeZoneId;
    DWORD			dwReserved;

} SYSTIMEINFO, *LPSYSTIMEINFO; // end typedef struct

#define SYS_BASIC_INFO       0
#define SYS_PERFORMANCE_INFO 2
#define SYS_TIME_INFO        3

#define Li2Double(x) ((double)((x).HighPart) * double( 0x100000000 ) + (double)((x).LowPart))

double CWin32::GetCpuLoad()
{_STT();
	static ACTIVESYSPARAMS s_asp;
	if ( !GetSysInfo( &s_asp ) ) return 0;
	return s_asp.cpu;
}

BOOL CWin32::GetSysInfo(LPACTIVESYSPARAMS pAsp)
{_STT();
	// Verify pointer
	if ( pAsp == NULL ) return FALSE;

	// Get OS info
	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	if( !GetVersionEx( &os ) ) return FALSE;

	// If Windows 95/98/ME
	if ( os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
	{
		HKEY hKey = NULL;

		// Open performance key
		if ( RegOpenKeyEx(	HKEY_DYN_DATA, "PerfStats\\StartStat", 
							0, KEY_READ, &hKey ) != ERROR_SUCCESS )
			return FALSE;

		// Save the cpu time
		DWORD dwCpu = 0, dwSize = sizeof( DWORD );
		if ( RegQueryValueEx(	hKey, "KERNEL\\CPUUsage", NULL, NULL,
								(LPBYTE)&dwCpu, &dwSize ) != ERROR_SUCCESS )
			dwCpu = 0;
		pAsp->cpu = (double)dwCpu;

		RegCloseKey( hKey );		

		return TRUE;

	} // end if

	// Windows NT/2K/XP
	else if ( os.dwPlatformId == VER_PLATFORM_WIN32_NT )
	{
		SYSBASICINFO	sbi;
		SYSPERFINFO		spi;
		SYSTIMEINFO		sti;

		// Get magic function address
		tQuerySystemInfo	fQsi = (tQuerySystemInfo)GetProcAddress(
														GetModuleHandle( "ntdll" ),
														"NtQuerySystemInformation" );
		if ( fQsi == NULL ) return FALSE;

		// Get system info
		if ( fQsi( SYS_BASIC_INFO, &sbi, sizeof( sbi ), NULL ) != NO_ERROR )
			return FALSE;

		if ( fQsi( SYS_PERFORMANCE_INFO, &spi, sizeof( spi ), NULL ) != NO_ERROR )
			return FALSE;

		if ( fQsi( SYS_TIME_INFO, &sti, sizeof( sti ), NULL ) != NO_ERROR )
			return FALSE;

		// Do we have values to use in the calculation?
		if ( !pAsp->bValid ) pAsp->bValid = TRUE;

		else
		{
			// CurrentValue = NewValue - OldValue
			double cpu = 0;
			double dIdle = Li2Double( spi.liIdleTime ) - pAsp->oldIdle;
			double dSys = Li2Double( sti.liKeSystemTime ) - pAsp->oldSys;
			double dTotal = dSys + dIdle;

			// Calculate idle time
			if ( dTotal == 0 ) pAsp->cpu = 0;
			else pAsp->cpu = ( dIdle * 100 ) / dTotal;

			// Validate cpu usage range
			if ( pAsp->cpu > 100 ) pAsp->cpu = 100;
			else if ( pAsp->cpu < 0 ) pAsp->cpu = 0;

			// Invert for busy time
			pAsp->cpu = (double)100 - pAsp->cpu;

			// Save other data
			pAsp->dwProcessors = sbi.bKeNumberProcessors;
			pAsp->dwTotalPhysicalPages = sbi.uMmNumberOfPhysicalPages;
			pAsp->dwAllocGranularity = sbi.uAllocationGranularity;
			pAsp->liIdleTime = spi.liIdleTime;
			pAsp->liSystemTime = sti.liKeSystemTime;

		} // end if

		// Save old values
		pAsp->oldIdle = Li2Double( spi.liIdleTime );
		pAsp->oldSys = Li2Double( sti.liKeSystemTime );

		return TRUE;

	} // end else if
				   
	return FALSE;
}

BOOL CWin32::GetSystemErrorMsg(DWORD dwErr, LPSTR pMsg, LPCTSTR pTemplate)
{_STT();
	char msg[ CWF_STRSIZE ];

	if ( pMsg == NULL ) return FALSE;
	*pMsg = 0x0;

	// Default template
	if ( pTemplate == NULL ) pTemplate = "%lu ( 0x%lX ) : %s";

	// What does windows say this error is?
	if ( !FormatMessage(	FORMAT_MESSAGE_FROM_SYSTEM,
							NULL, dwErr, 0, msg, CWF_STRSIZE, NULL ) )

		// Get winsock error
		GetWSAErrorMsg( msg, dwErr );

	// Format error message
	wsprintf( pMsg, pTemplate, dwErr, dwErr, msg );

	return TRUE;
}

BOOL CWin32::ShowError(HWND hParent, DWORD dwError, LPCTSTR pTitle, LPCTSTR pAdd, DWORD dwType )
{_STT();
	char msg[ 4 * 1024 ];

	GetSystemErrorMsg( dwError, msg );

	if ( pAdd != NULL )
	{
		strcat( msg, "\r\n\r\n" );
		strcat( msg, pAdd );
	} // end if

	return MessageBox( hParent, msg, pTitle, dwType );
}

BOOL CWin32::GetWSAErrorMsg(LPSTR pMsg, DWORD err)
{_STT();
	char *ptr = NULL;

	switch( err )
	{
		case WSAEACCES:
			ptr = "Access Denied";
			break;
		case WSAEADDRINUSE:
			ptr = "Address already in use";
			break;
		case WSAEADDRNOTAVAIL:
			ptr = "Cannot assign requested address";
			break;
		case WSAEAFNOSUPPORT:
			ptr = "Address family not supported by protocol family";
			break;
		case WSAEALREADY:
			ptr = "Operation already in progress";
			break;
		case WSAECONNABORTED:
			ptr = "Software caused connection abort";
			break;
		case WSAECONNREFUSED:
			ptr = "Connection refused";
			break;
		case WSAECONNRESET:
			ptr = "Connection reset by peer";
			break;
		case WSAEDESTADDRREQ:
			ptr = "Destination addres required";
			break;
		case WSAEFAULT:
			ptr = "Bad Address";
			break;
		case WSAEHOSTDOWN:
			ptr = "Host is down";
			break;
		case WSAEHOSTUNREACH:
			ptr = "Host is unreachable";
			break;
		case WSAEINPROGRESS:
			ptr = "Operation is now in progress";
			break;
		case WSAEINTR:
			ptr = "Interrupted function call";
			break;
		case WSAEINVAL:
			ptr = "Invalid argument";
			break;
		case WSAEISCONN:
			ptr = "Socket is already connected";
			break;
		case WSAEMFILE:
			ptr = "Too many open files";
			break;
		case WSAEMSGSIZE:
			ptr = "Message is too long";
			break;
		case WSAENETDOWN:
			ptr = "Network is down";
			break;
		case WSAENETRESET:
			ptr = "Network dropped connection on reset";
			break;
		case WSAENETUNREACH:
			ptr = "Network is unreachable";
			break;
		case WSAENOBUFS:
			ptr = "Insufficient buffer space is available";
			break;
		case WSAENOPROTOOPT:
			ptr = "Bad protocol option";
			break;
		case WSAENOTCONN:
			ptr = "Socket is not connected";
			break;
		case WSAENOTSOCK:
			ptr = "Socket operation on non-socket";
			break;
		case WSAEOPNOTSUPP:
			ptr = "Operation not supported";
			break;
		case WSAEPFNOSUPPORT:
			ptr = "Protocol family not supported";
			break;
		case WSAEPROCLIM:
			ptr = "Too many processes";
			break;
		case WSAEPROTONOSUPPORT:
			ptr = "Protocol not supported";
			break;
		case WSAEPROTOTYPE:
			ptr = "Protocol wrong type for socket";
			break;
		case WSAESHUTDOWN:
			ptr = "Cannot send after socket shutdown";
			break;
		case WSAESOCKTNOSUPPORT:
			ptr = "Socket type not supported";
			break;
		case WSAETIMEDOUT:
			ptr = "Connection timed out";
			break;
		case WSATYPE_NOT_FOUND:
			ptr = "Class type not found";
			break;
		case WSAEWOULDBLOCK:
			ptr = "Resource temporarily unavailable (Would block)";
			break;
		case WSAHOST_NOT_FOUND:
			ptr = "Host not found";
			break;
		case WSA_INVALID_HANDLE:
			ptr = "Specified event object handle is invalid";
			break;
		case WSA_INVALID_PARAMETER:
			ptr = "One or mor parameters are invalid";
			break;
//		case WSAINVALIDPROCTABLE;
//			ptr = "Invalid procedure table from service provider";
//			break;
//		case WSAINVALIDPROVIDER:
//			ptr = "Invalid service provider version number";
//			break;
		case WSA_IO_INCOMPLETE:
			ptr = "Overlapped I/O event object not in signaled state";
			break;
		case WSA_IO_PENDING:
			ptr = "Overlapped I/O operations will complete later";
			break;
		case WSA_NOT_ENOUGH_MEMORY:
			ptr = "Insufficient memory available";
			break;
		case WSANOTINITIALISED: 
			ptr = "Successful WSAStartup not yet performed";
			break;
		case WSANO_DATA:
			ptr = "Valid name, no data record of requested type";
			break;
		case WSANO_RECOVERY:
			ptr = "Non-recoverable error has occured";
			break;
//		case WSAPROVIDERFAILEDINIT:
//			ptr = "Unable to initialize a service provider";
//			break;
		case WSASYSCALLFAILURE:
			ptr = "System call failure";
			break;
		case WSASYSNOTREADY:
			ptr = "Network subsystem is unavailable";
			break;
		case WSATRY_AGAIN:
			ptr = "Non-authoritative host not found";
			break;
		case WSAVERNOTSUPPORTED:
			ptr = "WINSOCK.DLL version not supported";
			break;
		case WSAEDISCON:
			ptr = "Graceful shutdown in progress";
			break;
		case WSA_OPERATION_ABORTED:
			ptr = "Overlapped I/O operation has been aborted";
			break;
//		case WSAE:
//			ptr = "";
//			break;
		default: 
			strcpy( pMsg, "Unknown Winsock error" );
			return FALSE;
			break;

	} // end switch

	strcpy( pMsg, ptr );

	return TRUE;
}

LPCTSTR CWin32::GuidToString( LPSTR pStr, const GUID *pGuid )
{_STT();
	if ( pStr == NULL ) return NULL;
	*pStr = 0;

	GUID guid; 
	if ( pGuid == NULL )
	{	pGuid = &guid;
		CoCreateGuid( &guid );
	} // end if

	// DD05F574-2D69-4463-95DD-F76C9F7C5E6D
	wsprintf( pStr, "%0.8lX-%0.4lX-%0.4lX-%0.2lX%0.2lX-%0.2lX%0.2lX%0.2lX%0.2lX%0.2lX%0.2lX",
					(DWORD) pGuid->Data1, 
					(DWORD) pGuid->Data2,
					(DWORD) pGuid->Data3,
					(DWORD) pGuid->Data4[ 0 ],
					(DWORD) pGuid->Data4[ 1 ],
					(DWORD) pGuid->Data4[ 2 ],
					(DWORD) pGuid->Data4[ 3 ],
					(DWORD) pGuid->Data4[ 4 ],
					(DWORD) pGuid->Data4[ 5 ],
					(DWORD) pGuid->Data4[ 6 ],
					(DWORD) pGuid->Data4[ 7 ] );

	return pStr;
}

const GUID * CWin32::StringToGuid(GUID *pGuid, LPCTSTR pString)
{_STT();
	// Sanity check
	if ( pString == NULL || strlen( pString ) < 36 )
		return NULL;

	// Verify separators
	// DD05F574-2D69-4463-95DD-F76C9F7C5E6D
	if (	pString[ 8 ] != '-' || pString[ 13 ] != '-' ||
			pString[ 18 ] != '-' || pString[ 23 ] != '-' )
		return NULL;

	// Convert each component
	DWORD dw = 0;
	if ( !ahtoudw( &dw, pString, 8 ) ) return NULL;
	pGuid->Data1 = dw;

	if ( !ahtoudw( &dw, &pString[ 9 ], 4 ) ) return NULL;
	pGuid->Data2 = (WORD)dw;

	if ( !ahtoudw( &dw, &pString[ 14 ], 4 ) ) return NULL;
	pGuid->Data3 = (WORD)dw;

	DWORD i;
	for ( i = 0; i < 2; i++ )
	{	if ( !ahtoudw( &dw, &pString[ 19 + ( i << 1 ) ], 2 ) ) return NULL;
		pGuid->Data4[ i ] = (BYTE)dw;
	} // end for	

	for ( i = 0; i < 6; i++ )
	{	if ( !ahtoudw( &dw, &pString[ 24 + ( i << 1 ) ], 2 ) ) return NULL;
		pGuid->Data4[ 2 + i ] = (BYTE)dw;
	} // end for

	return pGuid;
}

BOOL CWin32::ahtoudw( LPDWORD pdwNum, LPCTSTR pBuffer, BYTE ucBytes, BOOL bSkipInvalid )
{_STT();
	DWORD num = 0;

	// For Each ASCII Digit
	for ( DWORD i = 0; ucBytes && pBuffer[ i ]; i++ )
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

		// Do we just skip invalid digits?
		else if ( !bSkipInvalid ) return FALSE;

		ucBytes--;

	} // end for

	// Save number
	if ( pdwNum ) *pdwNum = num;

	return TRUE;
}


HWND CWin32::SearchWindow(LPCTSTR pClass, LPCTSTR pName, HWND hRoot, BOOL bChild)
{_STT();
	// Ensure root
	if ( hRoot == NULL ) hRoot = GetDesktopWindow();
	if ( !::IsWindow( hRoot ) ) return NULL;

	// Get child window
	char szStr[ 1024 ];
	HWND hChild = ::GetWindow( hRoot, GW_CHILD );
	while ( ::IsWindow( hChild ) )
	{
		BOOL bNameMatch = FALSE;
		BOOL bClassMatch = FALSE;

		// Compare name
		if ( !pName || !*pName ) bNameMatch = TRUE;		
		else if ( ::GetWindowText( hChild, szStr, sizeof( szStr ) ) )
			bNameMatch = CWinFile::MatchPattern( szStr, pName );

		// Compare class
		if ( !pClass || !*pClass ) bClassMatch = TRUE;		
		else if ( ::GetClassName( hChild, szStr, sizeof( szStr ) ) )
			bClassMatch = CWinFile::MatchPattern( szStr, pName );

		// Did we get a match?
		if ( bNameMatch && bClassMatch ) return hChild;

		// Do we want to search child windows?
		if ( bChild )
		{	HWND hChildChild = SearchWindow( pClass, pName, hChild, bChild );
			if ( ::IsWindow( hChildChild ) ) return hChildChild;
		} // end if

		// Next window
		hChild = ::GetWindow( hChild, GW_HWNDNEXT );

	} // end while

	return NULL;
}

BOOL CWin32::StartService(LPCTSTR pServiceName)
{
	// Open the service manager
	SC_HANDLE hManager = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if ( !hManager ) return FALSE;
	
	// Open the specified service
	BOOL bStarted = FALSE;
	SC_HANDLE hService = ::OpenService( hManager, pServiceName, SC_MANAGER_ALL_ACCESS );
	if ( hService )
	{
		// Start the service
		bStarted = ::StartService( hService, 0, NULL );

		// Close the service handle
		::CloseServiceHandle( hService );

	} // end if

	// Close the service manager
	::CloseServiceHandle( hManager );

	return bStarted;
}

BOOL CWin32::ControlService(LPCTSTR pServiceName, DWORD dwControl, LPWINDOWS_SERVICE_STATUS pStatus)
{
	// Open the service manager
	SC_HANDLE hManager = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if ( !hManager ) return FALSE;
	
	// Open the specified service
	BOOL bRet = FALSE;
	SC_HANDLE hService = ::OpenService( hManager, pServiceName, SC_MANAGER_ALL_ACCESS );
	if ( hService )
	{
		// Ensure valid structure pointer
		SERVICE_STATUS	ss;
		if ( !pStatus ) pStatus = (LPWINDOWS_SERVICE_STATUS)&ss;

		// Send the request
		bRet = ::ControlService( hService, dwControl, (LPSERVICE_STATUS)pStatus );

		// Close the service handle
		::CloseServiceHandle( hService );

	} // end if

	// Close the service manager
	::CloseServiceHandle( hManager );

	return bRet;
}

BOOL CWin32::PopupMenu(long x, long y, LPCTSTR pMenu, HWND hWnd, HMODULE hInstance, long lSub )
{
	// Must have window
	if ( !::IsWindow( hWnd ) ) return FALSE;

	// Load the template
	HMENU hMenu = LoadMenu( hInstance, pMenu );
	if ( hMenu == NULL ) return FALSE;

	HMENU hPopup = hMenu;
	if ( 0 <= lSub ) hPopup = ::GetSubMenu( hMenu, lSub );
	if ( hPopup == NULL ) return FALSE;

	// We must be the foreground window for this to work correctly
	::SetForegroundWindow( hWnd );

	// Show the popup menu
	::TrackPopupMenu( hPopup, TPM_RIGHTBUTTON, x, y, 0, hWnd, NULL );

	// Unload the popup menu
	if ( hMenu != hPopup ) ::DestroyMenu( hPopup );

	// Unload the menu
	::DestroyMenu( hMenu );

	return TRUE;
}

