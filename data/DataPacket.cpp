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
// DataPacket.cpp: implementation of the CDataPacket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataPacket::CDataPacket()
{_STT();
	m_dwEncode = 0;
	m_bValidPacket = FALSE;
}

CDataPacket::CDataPacket( DWORD dwSize ) :
				CCircBuf( TRUE, dwSize )
{_STT();
	m_dwEncode = 0;
	m_bValidPacket = FALSE;
}

CDataPacket::~CDataPacket()
{_STT();
	Destroy();
}

void CDataPacket::Destroy()
{_STT();
	m_bValidPacket = FALSE;
}

// *** Packet Writing ***

BOOL CDataPacket::WritePacket(DWORD dwPacketType, DWORD dwDataType, LPVOID pData, DWORD dwData)
{_STT();
	// Lock the buffer
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return FALSE;

	// Initialize packet
	if ( !InitPacket( dwPacketType, 1, dwData ) )
		return FALSE;

	// Add the users data
	if ( !AddPacketData( dwDataType, pData, dwData ) )
		return FALSE;

	// Add end packet data
	return EndPacket();
}

BOOL CDataPacket::vWriteMultiPacket( DWORD dwPacketType, DWORD dwBuffers, LPVOID *pArgs )
{_STT();
	// Lock the buffer
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return FALSE;

	// Variable params
	LPVOID *ptrExtra = pArgs;

	// Calculate the total size needed
	DWORD dwTotalSize = 0, i;
	for ( i = 0; i < dwBuffers; i++ )
	{
		RULIB_TRY // This could GPF if caller screws up
		{
			DWORD	dwType = *(LPDWORD)ptrExtra;
			LPBYTE	pPtr = *(LPBYTE*)( ptrExtra + 1 );
			DWORD 	dwSize = *(LPDWORD)( ptrExtra + 2 );

			// Track the total size
			dwTotalSize += dwSize;

			// Skip these parameters
			ptrExtra += 3;

		} // end try
		RULIB_CATCH_ALL { return FALSE; }

	} // end for

	// Initialize packet
	if ( !InitPacket( dwPacketType, dwBuffers, dwTotalSize ) )
		return FALSE;

	// Now add the data
	ptrExtra = pArgs;
	for ( i = 0; i < dwBuffers; i++ )
	{
		RULIB_TRY // This could GPF if caller screws up
		{
			DWORD	dwType = *(LPDWORD)ptrExtra;
			LPBYTE	pPtr = *(LPBYTE*)( ptrExtra + 1 );
			DWORD 	dwSize = *(LPDWORD)( ptrExtra + 2 );

			// Add the users data
			if ( !AddPacketData( dwType, pPtr, dwSize ) )
				return FALSE;

			// Skip these parameters
			ptrExtra += 3;

		} // end try
		RULIB_CATCH_ALL { return FALSE; }

	} // end for

	// Add end packet data
	return EndPacket();

}

BOOL CDataPacket::InitPacket( DWORD dwType, DWORD dwDataBlocks, DWORD dwTotalDataSize)
{_STT();
	SPacketHeader ph;

	// Initialize the check sum
	CMd5Rsa::MD5Init( &m_md5Context );

	// Initialize the packet data
	ph.dwId = DPID_HEADER;

	// Calculate the total length of the packet
	ph.dwLength =	GetMinimumPacketOverhead() +
					( dwDataBlocks * sizeof( SDataHeader ) ) + 
					dwTotalDataSize;


	// The type of packet
	ph.dwType = dwType;

	// Number of data blocks
	ph.dwDataBlocks = dwDataBlocks;

	// Prepare to poke the buffer
	InitPoke();

	// Write data into the packet
	return WritePacketData( &ph, sizeof( ph ));
}

BOOL CDataPacket::AddPacketData( DWORD dwType, LPVOID pData, DWORD dwSize )
{_STT();
	SDataHeader dh;
	dh.dwLength = dwSize;
	dh.dwType = dwType;

	// Write the data block header
	if ( !WritePacketData( &dh, sizeof( dh ) ) )
		return FALSE;

	// Write the data
	return WritePacketData( pData, dwSize, m_dwEncode );
}

BOOL CDataPacket::WritePacketData(LPVOID pData, DWORD dwSize, DWORD dwEncode )
{_STT();
	// Write out the packet header
	if ( !Poke( pData, dwSize, dwEncode ) ) return FALSE;

	return TRUE;
}

BOOL CDataPacket::OnInspectWrite( DWORD dwBlock, LPBYTE pBuf, DWORD dwSize )
{_STT();
	// Update the md5 hash
	CMd5Rsa::MD5Update( &m_md5Context, pBuf, dwSize );

	return TRUE;
}


BOOL CDataPacket::EndPacket()
{_STT();
	SCheckSum cs;

	// Get the md5
	CMd5Rsa::MD5Final( cs.md5, &m_md5Context );

	// Write out the check sum
	if ( !Poke( &cs, sizeof( cs ) ) ) return FALSE;

	// Commit the data
	EndPoke();

	return TRUE;
}

// *** Packet Reading ***

BOOL CDataPacket::ReadPacket(LPVOID pBuf, DWORD dwSize)
{_STT();
	// First write the data into the buffer
	Write( pBuf, dwSize );

	// Attempt to verify a packet in the buffer
	return VerifyPacket();
}

BOOL CDataPacket::SkipPacket()
{_STT();
	// Do we have a packet
	if ( !m_bValidPacket )

		// Can we get one?
		if ( !VerifyPacket() ) return FALSE;

	// Skip over this packet
	AdvanceReadPtr( m_ph.dwLength );

	// Not valid now
	m_bValidPacket = FALSE;

	// Attempt to find another packet
	return VerifyPacket();
}

BOOL CDataPacket::FindPacket(LPSPacketHeader pPh, LPDWORD pdwAvailable)
{_STT();

	// How many bytes are availble
	DWORD dwAvailable = GetMaxRead();

	do
	{
		// Do we have enough data for a packet?
		if ( dwAvailable < GetMinimumPacketOverhead() )
			return FALSE;

		// Take a sneak at the header
		DWORD dwRead = 0;
		if (	!Peek( pPh, sizeof( SPacketHeader ), &dwRead ) || 
				dwRead != sizeof( SPacketHeader ) )
			return FALSE;

		// Is this a valid header id?
		if ( pPh->dwId == DPID_HEADER )
		{	if ( pdwAvailable ) *pdwAvailable = dwAvailable;
			return TRUE;
		} // end if

		// Should be one less byte in the buffer
		dwAvailable--;

	// Skip ahead one byte
	} while ( AdvanceReadPtr( 1 ) );

	return FALSE;
}

BOOL CDataPacket::VerifyPacket()
{_STT();
	// Do we already know there is a valid packet?
	if ( m_bValidPacket ) return TRUE;

	DWORD dwAvailable = 0;

	do
	{
		// Attempt to find a packet in the buffer
		if ( !FindPacket( &m_ph, &dwAvailable ) )
			return FALSE;

		// Verify the length of the packet is available
		if ( m_ph.dwLength > dwAvailable ) return FALSE;

		// Enforce maximum packet size
		if ( m_ph.dwLength <= GetMaxSize() )
		{
			LPBYTE pBuf = NULL;
			DWORD dwSize = 0, dwView = 0;

			// Initialize the check sum
			SCheckSum csCur, csBuf;
			CMd5Rsa::MD5Init( &m_md5Context );

			// Calculate the md5 hash
			while ( GetReadView( dwView++, 0, m_ph.dwLength - sizeof( SCheckSum ), &pBuf, &dwSize ) )

				// Update the md5 hash
				CMd5Rsa::MD5Update( &m_md5Context, (LPBYTE)pBuf, dwSize );

			// What is our calculated checksum?
			CMd5Rsa::MD5Final( csCur.md5, &m_md5Context );

			// Read the checksum from the buffer
			Peek( &csBuf, sizeof( csBuf ), NULL, m_ph.dwLength - sizeof( SCheckSum ) );

			// Do the checksums match?
			if ( !memcmp( &csCur, &csBuf, sizeof( SCheckSum ) ) )
			{	m_bValidPacket = TRUE;
				return TRUE;
			} // end if

		} // end if

	// Skip over the id
	} while ( AdvanceReadPtr( 4 ) );

	return FALSE;
}

BOOL CDataPacket::ReadPacketData(DWORD dwBlock, DWORD dwType, LPVOID pBuf, DWORD dwMax, LPDWORD pdwRead, long lOffset)
{_STT();
	// Ensure valid packet
	if ( !m_bValidPacket ) return FALSE;

	// Is it a valid data block index?
	if ( dwBlock > m_ph.dwDataBlocks ) return FALSE;

	// Where to start looking
	DWORD dwOffset = sizeof( SPacketHeader );

	// We must find the block
	for ( ; ; )
	{
		SDataHeader dh;

		// Peek at the First data block
		DWORD dwRead = 0;
		if ( !Peek( &dh, sizeof( dh ), &dwRead, dwOffset ) || dwRead != sizeof( dh ) ) 
			return FALSE;

		// Skip over the header
		dwOffset += sizeof( SDataHeader );

		// Test for unreasonable size
		if ( dh.dwLength > m_ph.dwLength ) return FALSE;

		// Is this is the block type we're looking for?
		if ( dwType == 0 || dh.dwType == dwType )
		{
			// Is this our block
			if ( !dwBlock )
			{
				// Is the offset too much?
				if ( (DWORD)lOffset > dh.dwLength ) return FALSE;

				// Subtrack the offset
				dh.dwLength -= lOffset;

				// Do they just want to know how many bytes or are there none?
				if ( pBuf == NULL || dwMax == 0 || dh.dwLength == 0 )
				{	if ( pdwRead ) *pdwRead = dh.dwLength; return TRUE; }

				// Truncate length if needed
				if ( dh.dwLength > dwMax ) dh.dwLength = dwMax;

				// Get the data
				return Peek( pBuf, dh.dwLength, pdwRead, dwOffset + lOffset, m_dwEncode );

			} // end if

			// Count a block
			else dwBlock--;

		} // end if

		// Skip the data
		dwOffset += dh.dwLength;
	}

	return FALSE;
}

BOOL CDataPacket::GetPacketDataHash(GUID *pGuid, DWORD dwBlock, DWORD dwType)
{_STT();
	// Ensure guid pointer
	if ( pGuid == NULL ) return FALSE;

	// Ensure valid packet
	if ( !m_bValidPacket ) return FALSE;

	// Is it a valid data block index?
	if ( dwBlock > m_ph.dwDataBlocks ) return FALSE;

	// Where to start looking
	DWORD dwOffset = sizeof( SPacketHeader );

	// We must find the block
	for ( ; ; )
	{
		SDataHeader dh;

		// Peek at the First data block
		DWORD dwRead = 0;
		if ( !Peek( &dh, sizeof( dh ), &dwRead, dwOffset ) || dwRead != sizeof( dh ) ) 
			return FALSE;

		// Skip over the header
		dwOffset += sizeof( SDataHeader );

		// Test for unreasonable size
		if ( dh.dwLength > m_ph.dwLength ) return FALSE;

		// Is this is the block type we're looking for?
		if ( dwType == 0 || dh.dwType == dwType )
		{
			// Is this our block
			if ( !dwBlock )
			{
				// Initialize MD5 Hash
				MD5_CTX ctx;
				CMd5Rsa::MD5Init( &ctx );

				// Hash data in place
				LPBYTE pBuf = NULL;
				DWORD dwView = 0, dwSize = 0;
				while ( GetReadView( dwView++, 0, dh.dwLength, &pBuf, &dwSize ) )

					// Update
					CMd5Rsa::MD5Update( &ctx, pBuf, dwSize );

				// Get the hash
				CMd5Rsa::MD5Final( (LPBYTE)pGuid, &ctx );

				return TRUE;

			} // end if

			// Count a block
			else dwBlock--;

		} // end if

		// Skip the data
		dwOffset += dh.dwLength;

	} // end forever

	return FALSE;
}

BOOL CDataPacket::VerifyPacketData(DWORD dwBlock, DWORD dwType, const GUID *pGuid)
{_STT();
	GUID guid;

	// Verify the data hash
	if ( 	!pGuid ||
			!GetPacketDataHash( &guid, dwBlock, dwType ) ||
			!IsEqualGUID( guid, *pGuid ) ) return FALSE;
	
	return TRUE;

}
