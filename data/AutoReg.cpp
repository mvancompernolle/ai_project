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
// Reg.cpp: implementation of the CAutoReg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAutoReg::CAutoReg()
{_STTEX();
	// Make sure the list is empty
	RemoveAll();

	// Start with the local machine
	SetComputer( NULL );
}

CAutoReg::~CAutoReg()
{_STTEX();

}

BOOL CAutoReg::CreateEntry(	LPREGENTRY pReg, HKEY hKey, LPCTSTR pKey, DWORD dwType,
						LPCTSTR pName, void* pvoidData, DWORD dwLength,
						DWORD dwDef, void* pvoidDef, DWORD dwDefLength,
						DWORD dwUser, LPCTSTR pComputer )
{_STTEX();
	// Sanity Checks
	if ( pReg == NULL ) return FALSE;
	if ( pKey == NULL ) return FALSE;

	// Blank out the structure
	ZeroMemory( (LPVOID)pReg, sizeof( REGENTRY ) );

	// Save the users data
	
	// Key info
	pReg->hKey		= hKey;
	strcpy( pReg->szKey, pKey );
	pReg->dwType	= dwType;

	// User variable info
	pReg->dwLength	= dwLength;
	if ( pName == NULL || pName[ 0 ] == 0x0 ) pReg->szName[ 0 ] = 0x0;
	else strcpy( pReg->szName, pName );
	pReg->pvoidData	= pvoidData;
	
	// Set user default value info
	SetDefault(	pReg, dwDef, pvoidDef, dwDefLength );

	// User value
	pReg->dwUser = dwUser;

	// Remote computer info
	if ( pComputer != NULL && pComputer[ 0 ] != 0x0 )
		strcpy( pReg->szComputer, pComputer );
	else pReg->szComputer[ 0 ] = 0x0;

	return TRUE;
} 


BOOL CAutoReg::AddEntry(	HKEY hKey, LPCTSTR pKey, DWORD dwType,
						LPCTSTR pName, void* pvoidData, DWORD dwLength,
						DWORD dwDef, void* pvoidDef, DWORD dwDefLength,
						DWORD dwUser, LPCTSTR pComputer )
{_STTEX();
	CAutoRegEntry reg;

	// Fill in the data
	CreateEntry(	&reg, hKey, pKey, dwType, pName, pvoidData, dwLength,
					dwDef, pvoidDef, dwDefLength, dwUser, pComputer );

	// Add it to the list
	m_Entrys += reg;

	return TRUE;
}


void CAutoReg::RemoveAll()
{_STTEX();
	m_Entrys.empty();
}

DWORD CAutoReg::Size()
{_STTEX();
	return m_Entrys.size();
}

BOOL CAutoReg::Get(LPREGENTRY pRE, DWORD dwIndex)
{_STTEX();
	DWORD size = Size();
	LPREGENTRY reg;

	if ( pRE == NULL || !size || dwIndex >= size ) return FALSE;

	// Get the settings
	reg = (LPREGENTRY)&m_Entrys[ dwIndex ];	

	// Copy the data
	memcpy( (LPVOID)pRE, (LPVOID)reg, sizeof( REGENTRY ) );	

	return TRUE;
}


BOOL CAutoReg::Restore(LPCTSTR pName, DWORD *pdwNumRead )
{_STTEX();
	BOOL	bRet = TRUE;
	DWORD	size = Size();
	LPREGENTRY reg;

	 if ( pdwNumRead != NULL ) *pdwNumRead = 0;
	for ( DWORD i = 0; i < size; i++ )
	{
		// Get the settings
		reg = (LPREGENTRY)&m_Entrys[ i ];	

		if ( !ReadReg( pName, reg, m_szComputer ) ) bRet = FALSE;
		else if ( pdwNumRead != NULL ) (*pdwNumRead)++;
	} // end for

	return bRet;
}

BOOL CAutoReg::ReadReg(LPCTSTR pName, LPREGENTRY pReg, LPCTSTR pComputer)
{_STTEX();
	HKEY	hKey, hRoot;
	char	szKey[ MAX_PATH ];
	BOOL	bRet = TRUE;
	DWORD	dwType;
	DWORD	dwLength;
	BOOL	bRemote = FALSE;

	if ( pReg == NULL ) return FALSE;

	// Generate key name
	szKey[ 0 ] = 0x0;
	strcpy( szKey, pReg->szKey );
	if ( pName != NULL && pName[ 0 ] != 0x0 )
	{
		if ( szKey[ 0 ] != 0x0 ) strcat( szKey, "\\" );
		strcat( szKey, pName );
	} // end if

	// Connect to remote computer if needed
	if ( pReg->szComputer[ 0 ] != 0x0 ) 
	{
		if ( RegConnectRegistry(	pReg->szComputer, 
									pReg->hKey, 
									&hRoot ) != ERROR_SUCCESS ) return FALSE;		
		bRemote = TRUE;
	} else if ( pComputer != NULL && pComputer[ 0 ] != 0x0 )
	{
		if ( RegConnectRegistry(	(char*)pComputer, 
									pReg->hKey, 
									&hRoot ) != ERROR_SUCCESS ) return FALSE;		
		bRemote = TRUE;
	} // end else if
	// Must want the local machine
	else hRoot = pReg->hKey;

	// Open The Key
	if( RegOpenKeyEx(	hRoot, szKey, 0,
						KEY_QUERY_VALUE, &hKey ) != ERROR_SUCCESS )
	{
		// Close the remote key if open
		if ( bRemote ) RegCloseKey( hRoot );
		Default( pReg );
		return FALSE;
	} // end if
	
	// Record the length and type
	dwLength	= pReg->dwLength;
	dwType		= pReg->dwType;

	// Read The Data
	if( RegQueryValueEx(	hKey, pReg->szName, 0, &dwType, 
							(LPBYTE) pReg->pvoidData, &dwLength ) != ERROR_SUCCESS )
		bRet = FALSE;

	// Make sure it is the correct type
	// And The Correct Length
	if( !bRet || pReg->dwType != dwType )
	{
		bRet = FALSE;
		Default( pReg );
	} // end if

	// Close The Key
	RegCloseKey( hKey );	

	// Close the remote key if open
	if ( bRemote ) RegCloseKey( hRoot );

	return bRet;
}

BOOL CAutoReg::SetDefault(	LPREGENTRY pReg, DWORD dwDef, 
						void* pvoidDef, DWORD dwDefLength )
{_STTEX();
	if ( pReg == NULL ) return FALSE;
	
	pReg->dwDef = dwDef;
	if ( pvoidDef == NULL )
	{
		pReg->dwDefSource = 1;
		pReg->pvoidDef = NULL;
		pReg->dwDefLength = sizeof( DWORD );
	} // end if
	else
	{	
		if ( dwDefLength == 0 )
		{
			pReg->dwDefSource = 2;
			pReg->pvoidDef = NULL;
			strcpy( pReg->szDef, (char*)pvoidDef );
			pReg->dwDefLength = strlen( pReg->szDef ) + 1;
		} // end if
		else
		{
			pReg->dwDefSource = 0;
			pReg->dwDefLength = dwDefLength;
			pReg->pvoidDef = pvoidDef;
		} // end else
	} // end else
	return TRUE;
}

BOOL CAutoReg::Default(LPREGENTRY pReg)
{_STTEX();
	if ( pReg == NULL ) return FALSE;

	switch( pReg->dwDefSource )
	{
		case 0 :
			memcpy( pReg->pvoidData, pReg->pvoidDef, pReg->dwDefLength );
			break;
		case 1 :
			*((LPDWORD)pReg->pvoidData) = pReg->dwDef;
			break;
		case 2 :
			memcpy( pReg->pvoidData, (LPVOID)pReg->szDef, pReg->dwDefLength );
			break;
		default : break;
	} // end switch	

	return TRUE;
}

BOOL CAutoReg::Save( LPCTSTR pName )
{_STTEX();
	BOOL		bRet = TRUE;
	DWORD		size = Size();
	LPREGENTRY	reg;

	for ( DWORD i = 0; i < size; i++ )
	{
		// Get the settings
		reg = (LPREGENTRY)&m_Entrys[ i ];	

		if ( !WriteReg( pName, reg, m_szComputer ) ) bRet = FALSE;

	} // end for

	return bRet;
}


BOOL CAutoReg::WriteReg( LPCTSTR pName,  LPREGENTRY pReg, LPCTSTR pComputer )
{_STTEX();
	HKEY hRoot;
	HKEY hKey;
	BOOL bRet = TRUE;
	char szKey[ MAX_PATH ];
	BOOL bRemote = FALSE;

	// Generate key name
	szKey[ 0 ] = 0x0;
	strcpy( szKey, pReg->szKey );
	if ( pName != NULL && pName[ 0 ] != 0x0 )
	{
		if ( szKey[ 0 ] != 0x0 ) strcat( szKey, "\\" );
		strcat( szKey, pName );
	} // end if


	// Connect to remote computer if needed
	if ( pReg->szComputer[ 0 ] != 0x0 ) 
	{
		if ( RegConnectRegistry(	pReg->szComputer, 
									pReg->hKey, 
									&hRoot ) != ERROR_SUCCESS ) return FALSE;		
		bRemote = TRUE;
	} else if ( pComputer != NULL && pComputer[ 0 ] != 0x0 )
	{
		if ( RegConnectRegistry(	(char*)pComputer, 
									pReg->hKey, 
									&hRoot ) != ERROR_SUCCESS ) return FALSE;		
		bRemote = TRUE;
	} // end else if
	// Must want the local machine
	else hRoot = pReg->hKey;

	// Open The Key
	if( RegCreateKey( hRoot, szKey, &hKey ) != ERROR_SUCCESS )
	{
		// Close the remote key if open
		if ( bRemote ) RegCloseKey( hRoot );
		return FALSE;
	} // end if

	// Set The Key Value
	if( RegSetValueEx(	hKey, pReg->szName, 0, pReg->dwType, 
					(LPBYTE) pReg->pvoidData, pReg->dwLength ) 
					!= ERROR_SUCCESS )
		bRet = FALSE;

	RegCloseKey( hKey );

	// Close the remote key if open
	if ( bRemote ) RegCloseKey( hRoot );
	return bRet;	

}

void CAutoReg::DefaultAll()
{_STTEX();
	DWORD	size = Size();

	for ( DWORD i = 0; i < size; i++ )
	{
		// Set Defaults
		Default( (LPREGENTRY)&m_Entrys[ i ] ) ;
	} // end for
}

LPREGENTRY CAutoReg::GetEntry(LPCTSTR pName)
{_STTEX();
	DWORD size = m_Entrys.size();
	LPREGENTRY	reg = NULL;

	// If the list is empty
	if ( !size ) return NULL;

	// Search the list and find the correct entry
	for ( DWORD i = 0; i < size && reg == NULL; i++ )
		if ( m_Entrys[ i ] == pName ) 
			reg = (LPREGENTRY)&m_Entrys[ i ];
	return reg;
}

// Return Values
//
// 0 = No Access or invalid key data
// 1 = KEY_QUERY_VALUE
// 2 = KEY_READ
// 3 = KEY_WRITE
// 4 = KEY_QUERY_VALUE | KEY_WRITE
// 5 = KEY_READ | KEY_WRITE
//
DWORD CAutoReg::TestAccess( LPCTSTR pName, LPREGENTRY pReg, LPCTSTR pComputer )
{_STTEX();
	HKEY	hKey, hRoot;
	char	szKey[ MAX_PATH ];
	BOOL	bRemote = FALSE;
	DWORD	access = 0;

	if ( pReg == NULL ) return 0;

	// Generate key name
	szKey[ 0 ] = 0x0;
	strcpy( szKey, pReg->szKey );
	if ( pName != NULL && pName[ 0 ] != 0x0 )
	{
		if ( szKey[ 0 ] != 0x0 ) strcat( szKey, "\\" );
		strcat( szKey, pName );
	} // end if

	// Connect to remote computer if needed
	if ( pReg->szComputer[ 0 ] != 0x0 ) 
	{
		if ( RegConnectRegistry(	pReg->szComputer, 
									pReg->hKey, 
									&hRoot ) != ERROR_SUCCESS ) return 0;		
		bRemote = TRUE;
	} else if ( pComputer != NULL && pComputer[ 0 ] != 0x0 )
	{
		if ( RegConnectRegistry(	(char*)pComputer, 
									pReg->hKey, 
									&hRoot ) != ERROR_SUCCESS ) return 0;		
		bRemote = TRUE;
	} // end else if
	// Must want the local machine
	else hRoot = pReg->hKey;

	// Query access
	if( RegOpenKeyEx(	hRoot, szKey, 0,
						KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS )
	{
		RegCloseKey( hKey );
		// We have KEY_QUERY_VALUE access
		access = 1;
	} // end if

	// Read access
	if( RegOpenKeyEx(	hRoot, szKey, 0,
						KEY_READ, &hKey ) == ERROR_SUCCESS )
	{
		RegCloseKey( hKey );
		// We have KEY_READ access
		access = 2;
	} // end if

	// Write access
	if( RegOpenKeyEx(	hRoot, szKey, 0,
						KEY_WRITE, &hKey ) == ERROR_SUCCESS )
	{
		RegCloseKey( hKey );
		// We have KEY_WRITE access
		access += 3;
	} // end if

	if ( bRemote ) RegCloseKey( hRoot );

	// Tell the caller how it turned out
	return access;
}

DWORD CAutoReg::TestAccess( LPCTSTR pName, LPREGENTRY pReg )
{_STTEX();
	LPREGENTRY	reg;

	if ( pReg != NULL ) reg = pReg;
	else reg = (LPREGENTRY)&m_Entrys[ 0 ];

	// Get the access level
	return TestAccess( pName, reg, m_szComputer );
}

char* CAutoReg::AccessString( DWORD access )
{_STTEX();
#define SW_ACCESS( l, str )	case l : return str; break;

	switch( access )
	{
		SW_ACCESS( 1, "Query access to data only" );
		SW_ACCESS( 2, "Read access to data only" );
		SW_ACCESS( 3, "Write access to data only" );
		SW_ACCESS( 4, "Query and Write access to data only" );
		SW_ACCESS( 5, "Full Read Write access to data granted" );
		default : SW_ACCESS( 0, "Access to data is denied" );
	} // end switch
}

BOOL CAutoReg::DeleteEntry ( LPCTSTR pName, LPREGENTRY pReg, LPCTSTR pComputer )
{_STTEX();
	BOOL	bRet = FALSE;
	HKEY	hKey, hRoot;
	char	szKey[ MAX_PATH ];
	BOOL	bRemote = FALSE;
	DWORD	access = 0;

	// Sanity Check
	if ( pReg == NULL ) return FALSE;
	if ( pName == NULL && pReg->szName[ 0 ] == 0x0 ) return FALSE;

	// Generate key name
	strcpy( szKey, pReg->szKey );

	// Connect to remote computer if needed
	if ( pReg->szComputer[ 0 ] != 0x0 ) 
	{
		if ( RegConnectRegistry(	pReg->szComputer, 
									pReg->hKey, 
									&hRoot ) != ERROR_SUCCESS ) return 0;		
		bRemote = TRUE;
	} else if ( pComputer != NULL && pComputer[ 0 ] != 0x0 )
	{
		if ( RegConnectRegistry(	(char*)pComputer, 
									pReg->hKey, 
									&hRoot ) != ERROR_SUCCESS ) return 0;		
		bRemote = TRUE;
	} // end else if

	// Must want the local machine
	else hRoot = pReg->hKey;

	// Open The Key
	if( RegOpenKeyEx(	hRoot, szKey, 0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
	{
		// Close the remote key if open
		if ( bRemote ) RegCloseKey( hRoot );
		return FALSE;
	} // end if

	// Attempt to delete the key
	if( RegDeleteKey( hKey, ( pName != NULL ) ? pName : pReg->szName ) == ERROR_SUCCESS ) bRet = TRUE;

	// Close the keys
	RegCloseKey( hKey );
	if ( bRemote ) RegCloseKey( hRoot );

	return bRet;
}

BOOL CAutoReg::DeleteSubKeys ( HKEY hRoot, LPCTSTR pKey )
{_STTEX();
	HKEY		hKey;
	char		szKey[ MAX_PATH ];
	DWORD		dwSize = MAX_PATH - 1;
	FILETIME	ft;

	ZeroMemory( (LPVOID)&ft, sizeof( FILETIME ) );

	// Open The Key
	if( RegOpenKeyEx(	hRoot, pKey, 0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
		return FALSE;

	// For each sub key
	while ( RegEnumKeyEx(	hKey, 0, szKey, &dwSize, 
							NULL, NULL, NULL, &ft ) == ERROR_SUCCESS )
	{
		// A little recursion
		DeleteSubKeys( hKey, szKey );

		// Attempt to delete the key
		RegDeleteKey( hKey, szKey );

		dwSize = MAX_PATH - 1;

	} // end while

	// Close the key
	RegCloseKey( hKey );

	return TRUE;
}

BOOL CAutoReg::DeleteSubKeys ( LPCTSTR pName, LPREGENTRY pReg, LPCTSTR pComputer )
{_STTEX();
	BOOL	bRet = FALSE;
	HKEY	hRoot;
	char	szKey[ MAX_PATH ];
	BOOL	bRemote = FALSE;
	DWORD	access = 0;

	// Sanity Check
	if ( pReg == NULL ) return FALSE;

	// Generate key name
	szKey[ 0 ] = 0x0;
	strcpy( szKey, pReg->szKey );
	if ( pName != NULL && pName[ 0 ] != 0x0 )
	{
		if ( szKey[ 0 ] != 0x0 ) strcat( szKey, "\\" );
		strcat( szKey, pName );
	} // end if

	// Connect to remote computer if needed
	if ( pReg->szComputer[ 0 ] != 0x0 ) 
	{
		if ( RegConnectRegistry(	pReg->szComputer, 
									pReg->hKey, 
									&hRoot ) != ERROR_SUCCESS ) return 0;		
		bRemote = TRUE;
	} else if ( pComputer != NULL && pComputer[ 0 ] != 0x0 )
	{
		if ( RegConnectRegistry(	(char*)pComputer, 
									pReg->hKey, 
									&hRoot ) != ERROR_SUCCESS ) return 0;		
		bRemote = TRUE;
	} // end else if

	// Must want the local machine
	else hRoot = pReg->hKey;

	// Attempt to delete the Sub-keys
	if( DeleteSubKeys( hRoot, szKey ) == ERROR_SUCCESS ) bRet = TRUE;

	// Close the remote key if needed
	if ( bRemote ) RegCloseKey( hRoot );

	return bRet;
}

BOOL CAutoReg::GetSubKey( DWORD index, LPREGENTRY pDest, LPCTSTR pName, LPREGENTRY pReg, LPCTSTR pComputer )
{_STTEX();
	
	BOOL	bRet = FALSE;
	HKEY	hKey, hRoot;
	char	szKey[ MAX_PATH ];
	BOOL	bRemote = FALSE;
	DWORD	access = 0;
	DWORD	dwSize = sizeof( pReg->szName );

	// Sanity Check
	if ( pReg == NULL ) return FALSE;
	if ( pDest == NULL ) return FALSE;

	// Blank out the structure
	ZeroMemory( (LPVOID)pDest, sizeof( REGENTRY ) );

	// Generate key name
	szKey[ 0 ] = 0x0;
	strcpy( szKey, pReg->szKey );
	if ( pName != NULL && pName[ 0 ] != 0x0 )
	{
		if ( szKey[ 0 ] != 0x0 ) strcat( szKey, "\\" );
		strcat( szKey, pName );
	} // end if
	if ( pReg->szName[ 0 ] != 0x0 )
	{
		if ( szKey[ 0 ] != 0x0 ) strcat( szKey, "\\" );
		strcat( szKey, pReg->szName );
	} // end if

	// Connect to remote computer if needed
	if ( pReg->szComputer[ 0 ] != 0x0 ) 
	{
		strcpy( pDest->szComputer, pReg->szComputer );
		if ( RegConnectRegistry(	pReg->szComputer, 
									pReg->hKey, 
									&hRoot ) != ERROR_SUCCESS ) return 0;		
		bRemote = TRUE;
	} else if ( pComputer != NULL && pComputer[ 0 ] != 0x0 )
	{
		strcpy( pDest->szComputer, pComputer );
		if ( RegConnectRegistry(	(char*)pComputer, 
									pReg->hKey, 
									&hRoot ) != ERROR_SUCCESS ) return 0;		
		bRemote = TRUE;
	} // end else if

	// Must want the local machine
	else hRoot = pReg->hKey;

	// Open The Key
	if( RegOpenKeyEx(	hRoot, szKey, 0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
	{
		// Close the remote key if open
		if ( bRemote ) RegCloseKey( hRoot );
		return FALSE;
	} // end if

	pDest->dwUser = 0;
	pDest->hKey = pReg->hKey; 

	// Save the location of the key
	strcpy( pDest->szKey, szKey );

	FILETIME	ft;
	// Get sub-key info
	if ( RegEnumKeyEx( hKey, index, pDest->szName, &dwSize, 
							NULL, NULL, NULL, &ft ) == ERROR_SUCCESS )
	{
		bRet = TRUE;
		// Attempt to get sub-key info
		if( RegQueryValueEx(	hKey, pDest->szName, 0, &pDest->dwType, 
								(LPBYTE) pDest->pvoidData, &pDest->dwLength ) 
								== ERROR_SUCCESS ) 
		{
			SetDefault( pDest );
		} // end if
		else
		{
			pDest->dwType = REG_NONE;
			pDest->dwLength = 0;
		} // end else
		dwSize = MAX_PATH -1;
	} // end if	   

	// Close the keys
	RegCloseKey( hKey );
	if ( bRemote ) RegCloseKey( hRoot );

	return bRet;
}

