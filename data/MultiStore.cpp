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
// MultiStore.cpp: implementation of the CMultiStore class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMultiStore::CMultiStore()
{_STT();
	m_dwRecord = 0;
	m_pvIndex = NULL;

	m_dwBits = 0;
	m_dwDepth = 0;
	m_dwNodeSize = 0;

	m_dwMaxMem = 0;
	m_dwMaxDisk = 0;
}

CMultiStore::~CMultiStore()
{_STT();
	Destroy();
}

BOOL CMultiStore::Write(LPSMultiStoreInfo pMsi, void *pMem, DWORD dwMem, void *pDisk, DWORD dwDisk, BOOL bOverwrite )
{_STT();
	// Allocate record if needed
	if ( pMsi->dwIndex == 0 ) pMsi->dwIndex = ++m_dwRecord;
	else if ( !bOverwrite ) return FALSE;

	// Set file position
	m_store.SetPtrPosBegin( pMsi->dwIndex * m_dwMaxDisk );

	// Write data to memory
	if ( !pMem ) dwMem = 0;
	if ( dwMem > m_dwMaxMem ) dwMem = m_dwMaxMem;
	pMsi->dwMemSize = dwMem;
	if ( pMem && dwMem ) memcpy( MemPtr( pMsi ), pMem, dwMem );
	
	// Write data to disk
	if ( !pDisk ) dwDisk = 0;
	if ( dwDisk > m_dwMaxDisk ) dwDisk = m_dwMaxDisk;
	pMsi->dwDiskSize = dwDisk;
	if ( pDisk && dwDisk ) if ( !m_store.Write( pDisk, dwDisk ) )
		return FALSE;

	return TRUE;
}

BOOL CMultiStore::Read(LPSMultiStoreInfo pMsi, void *pMem, DWORD dwMem, void *pDisk, DWORD dwDisk)
{_STT();
	// Allocate record if needed
	if ( pMsi->dwIndex == 0 ) return FALSE;

	// Read from memory
	if ( pMem && dwMem )
	{	DWORD dwRead = dwMem;
		if ( dwRead > m_dwMaxMem ) dwRead = m_dwMaxMem;
		if ( dwRead > pMsi->dwMemSize ) dwRead = pMsi->dwMemSize;
		memcpy( pMem, MemPtr( pMsi ), dwRead );
		if ( dwRead < dwMem ) ( (LPBYTE)pMem )[ dwRead ] = 0;
	} // end if
	
	if ( pDisk && dwDisk )
	{	DWORD dwRead = dwDisk;
		if ( dwRead > m_dwMaxDisk ) dwRead = m_dwMaxDisk;
		if ( dwRead > pMsi->dwDiskSize ) dwRead = pMsi->dwDiskSize;

		// Set file position
		if ( !m_store.SetPtrPosBegin( pMsi->dwIndex * m_dwMaxDisk ) )
			return FALSE;

		// Read the data from disk
		if ( !m_store.Read( pDisk, dwRead, &dwRead ) || !dwRead )
			return FALSE;

		// NULL terminate if room
		if ( dwDisk > dwRead ) ( (LPBYTE)pMem )[ dwRead ] = 0;

	} // end if

	return TRUE;
}

BOOL CMultiStore::Open( LPCTSTR pFile, DWORD dwMaxMem, DWORD dwMaxDisk, DWORD dwBits )
{_STT();
	// Lose previous
	Destroy();

	// Save data params
	m_dwMaxMem = dwMaxMem;
	m_dwMaxDisk = dwMaxDisk;

	// Number of bits to use
	m_dwBits = ( dwBits <= 0 || dwBits >= 32 ) ? 8 : dwBits;

	// Ensure it's a power of 2
	if ( m_dwBits & ( m_dwBits - 1 ) ) m_dwBits = 8;

	// Calculate index depth
	m_dwDepth = 32 / m_dwBits;

	// Calculate node size
	m_dwNodeSize = 1 << m_dwBits;

	// Open the data file
	return m_store.Open( pFile, GENERIC_READ | GENERIC_WRITE );
}

void CMultiStore::Destroy()
{_STT();
	// Lose the index
	if ( m_pvIndex != NULL ) DestroyIndex( &m_pvIndex );

	m_dwRecord = 0;

	m_dwBits = 0;
	m_dwDepth = 0;
	m_dwNodeSize = 0;

	m_dwMaxMem = 0;
	m_dwMaxDisk = 0;
}

void CMultiStore::DestroyIndex(LPVOID *pvIndex, DWORD dwDepth)
{_STT();
	if ( pvIndex == NULL || *pvIndex == NULL ) return;

	// If memory depth
	if ( dwDepth + 1 < m_dwDepth )

		// Delete each sub array
		for ( DWORD i = 0; i < m_dwNodeSize; i++ )

			DestroyIndex( &( (LPVOID*)*pvIndex )[ i ], dwDepth + 1 );

	// Delete the index
	_PTR_DELETE_ARR( *pvIndex );

}

LPSMultiStoreInfo CMultiStore::GetIndexPtr(DWORD dwIndex, LPVOID *pvIndex, BOOL bCreate, DWORD dwDepth )
{_STT();
	ASSERT( dwDepth <= m_dwDepth );

	// Calculate sub index
	DWORD dwShift = ( ( m_dwDepth - 1 ) - dwDepth ) * m_dwBits;
	DWORD dwSubIndex = ( dwIndex  >> dwShift ) & ( ( 1 << m_dwBits ) - 1 );

	// Are we at item level?
	if ( dwDepth + 1 >= m_dwDepth )
	{
		DWORD dwUnitSize = sizeof( SMultiStoreInfo ) + m_dwMaxMem;

		// Allocate memory if needed
		if ( *pvIndex == NULL )
		{	if ( !bCreate ) return NULL;
			DWORD dwSize = dwUnitSize * m_dwNodeSize;
			*pvIndex = _PTR_NEW BYTE[ dwSize ];
			ZeroMemory( *pvIndex, dwSize );
		} // end if
		
		// Return a pointer to this block
		return (LPSMultiStoreInfo)&( (LPBYTE)*pvIndex )[ dwUnitSize * dwSubIndex ];

	} // end if

	if ( *pvIndex == NULL )
	{	if ( !bCreate ) return NULL;
		*pvIndex = _PTR_NEW LPVOID[ m_dwNodeSize ];
		ZeroMemory( *pvIndex, m_dwNodeSize * sizeof( LPVOID ) );
	} // end if

	// Keep digging
	return GetIndexPtr( dwIndex, &( (LPVOID*)*pvIndex )[ dwSubIndex ], bCreate, dwDepth + 1 );
}

LPSMultiStoreInfo CMultiStore::GetNextIndexPtr(DWORD dwIndex, LPVOID *pvIndex, DWORD dwDepth )
{_STT();
	ASSERT( dwDepth <= m_dwDepth );

	// Ensure valid pointer
	if ( pvIndex == NULL || *pvIndex == NULL ) return NULL;

	// Calculate sub index
	DWORD dwShift = ( ( m_dwDepth - 1 ) - dwDepth ) * m_dwBits;
	DWORD dwSubIndex = ( dwIndex  >> dwShift ) & ( ( 1 << m_dwBits ) - 1 );

	// Are we at item level?
	if ( dwDepth + 1 >= m_dwDepth )
	{
		DWORD dwUnitSize = sizeof( SMultiStoreInfo ) + m_dwMaxMem;

		// Find a valid item
		LPSMultiStoreInfo pMsi = (LPSMultiStoreInfo)&( (LPBYTE)*pvIndex )[ dwUnitSize * dwSubIndex ];
		while ( pMsi->dwIndex == 0 )
		{	if ( ++dwSubIndex >= m_dwNodeSize ) return NULL;
			pMsi = (LPSMultiStoreInfo)&( (LPBYTE)*pvIndex )[ dwUnitSize * dwSubIndex ];			
		} // end while
		
		// Return a pointer to this block
		return (LPSMultiStoreInfo)&( (LPBYTE)*pvIndex )[ dwUnitSize * dwSubIndex ];

	} // end if

	DWORD dwResetMask = 0;
	for ( DWORD i = 0; i < dwShift; i++ )
		dwResetMask <<=1, dwResetMask |= 1;

	// Find the next valid array
	while ( dwSubIndex < m_dwNodeSize )
	{	
		// Verify we're still in a valid range
//		if ( dwSubIndex >= m_dwNodeSize ) return NULL;

		// Get the next pointer
		LPVOID	*pvNext = &( (LPVOID*)*pvIndex )[ dwSubIndex ];

		// Dig for a valid object
		LPSMultiStoreInfo pMsi = GetNextIndexPtr( dwIndex, pvNext, dwDepth + 1 );
		if ( pMsi != NULL ) return pMsi;

		// Next block
		dwSubIndex++;
		dwIndex &= ~dwResetMask;

	} // end if

	return NULL;
}

LPSMultiStoreInfo CMultiStore::GetPrevIndexPtr(DWORD dwIndex, LPVOID *pvIndex, DWORD dwDepth )
{_STT();
	// Ensure valid pointer
	if ( dwDepth > m_dwDepth || pvIndex == NULL || *pvIndex == NULL ) return NULL;

	// Calculate sub index
	DWORD dwShift = ( ( m_dwDepth - 1 ) - dwDepth ) * m_dwBits;
	DWORD dwSubIndex = ( dwIndex  >> dwShift ) & ( ( 1 << m_dwBits ) - 1 );

	// Are we at item level?
	if ( dwDepth + 1 >= m_dwDepth )
	{
		DWORD dwUnitSize = sizeof( SMultiStoreInfo ) + m_dwMaxMem;

		// Find a valid item
		LPSMultiStoreInfo pMsi = (LPSMultiStoreInfo)&( (LPBYTE)*pvIndex )[ dwUnitSize * dwSubIndex ];
		while ( pMsi->dwIndex == 0 )
		{	if ( !dwSubIndex ) return NULL; dwSubIndex--;
			pMsi = (LPSMultiStoreInfo)&( (LPBYTE)*pvIndex )[ dwUnitSize * dwSubIndex ];			
		} // end while
		
		// Return a pointer to this block
		return (LPSMultiStoreInfo)&( (LPBYTE)*pvIndex )[ dwUnitSize * dwSubIndex ];

	} // end if

	DWORD dwResetMask = 0; // ( 1 << m_dwBits ) - 1;
	for ( DWORD i = 0; i < dwShift; i++ )
		dwResetMask <<=1, dwResetMask |= 1;

	// Find the next valid array
	do
	{	
		// Get the next pointer
		LPVOID	*pvNext = &( (LPVOID*)*pvIndex )[ dwSubIndex ];

		// Dig for a valid object
		LPSMultiStoreInfo pMsi = GetPrevIndexPtr( dwIndex, pvNext, dwDepth + 1 );
		if ( pMsi != NULL ) return pMsi;

		// Next block
		if ( dwSubIndex ) dwSubIndex--;
		dwIndex |= dwResetMask;

	} while ( dwSubIndex );

	return NULL;
}
