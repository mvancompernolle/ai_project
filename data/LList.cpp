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
// LList.cpp: implementation of the CLList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "math.h"

#ifndef ASSERT
#define ASSERT
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLList::CLList()
{
	Init( FALSE );
}

CLList::~CLList()
{
	Destroy();
}

void CLList::Init(BOOL bDestroy)
{	if ( bDestroy ) Destroy();

	m_dwSize = 0;
	m_pHead = NULL;
	m_pTail = NULL; 
	m_dwLockTimeout = 3000;
}

void CLList::Destroy()
{	DeleteAllItems();
}

void CLList::DeleteAllItems()
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return;

	m_dwSize = 0;
	m_pTail = NULL;

	RULIB_TRY
	{
		while ( m_pHead != NULL )
		{	LPLLISTINFO next = m_pHead->pNext;
			ReleaseObject( m_pHead );
			m_pHead = next;
		} // end while	
	} RULIB_CATCH_ALL { ASSERT( 0 ); }
}

void CLList::ReleaseObject(void *node)
{
	// Get node pointer
	LPLLISTINFO pli = (LPLLISTINFO)node;
	if ( pli == NULL ) return;

	// Delete the object memory
	DeleteObject( node );

	// Release the memory
	delete [] (LPBYTE)node;
}

void CLList::DeleteObject(void * node)
{
	// Get node pointer
	LPLLISTINFO pli = (LPLLISTINFO)node;
	if ( pli == NULL ) return;

	// Release key memory if any
	if ( pli->vpkey != NULL )
	{	delete [] pli->vpkey;
		pli->vpkey = NULL;
	} // end if

}

BOOL CLList::InitObject(void *node)
{
	return TRUE;
}

void* CLList::SetObject( void* obj, const void *init, DWORD dwUser, const void *pKey, DWORD dwKeySize)
{
	if ( obj == NULL ) return NULL;
	LPLLISTINFO node = (LPLLISTINFO)obj;

	// Get the object size
	DWORD size = GetObjSize();

	RULIB_TRY
	{
		if ( size > sizeof( LLISTINFO ) )
		{
			// Did we get initialization data?
			if ( init != NULL ) 
				memcpy( &( ( (LPBYTE)node )[ sizeof( LLISTINFO ) ] ), 
						&( ( (LPBYTE)init )[ sizeof( LLISTINFO ) ] ), 
						size - sizeof( LLISTINFO ) );

			// Zero the whole thing
			else ZeroMemory(	&( ( (LPBYTE)node )[ sizeof( LLISTINFO ) ] ),
								size - sizeof( LLISTINFO ) );
		} // end if

		// Save size
		node->dwObjSize = size;

		// Set the key
		if ( pKey != NULL ) SetKey( node, pKey, dwKeySize );

		// Save user data
		node->user =dwUser;

		// Initialize the object
		if ( !InitObject( node ) ) 
		{	delete [] (LPBYTE)node;
			return FALSE;
		} // end if

	} // end try

	// Forget it if bad pointer
	RULIB_CATCH_ALL { ASSERT( 0 ); return NULL; }

	return obj;
}

void* CLList::NewObj(DWORD dwSize, const void *init, DWORD dwUser, const void *pKey, DWORD dwKeySize)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	// Must be at least as large as a llist
	if ( dwSize < sizeof( LLISTINFO ) ) return NULL;

	// Allocate list (extra byte is for zero terminator)
	LPLLISTINFO node = (LPLLISTINFO)new BYTE[ dwSize + 1 ];
	if ( node == NULL ) return FALSE;

	// Init our part of the structure
	ZeroMemory( node, sizeof( LLISTINFO ) );

	// Setup the object
	if (	!SetObject( node, init, dwUser, pKey, dwKeySize ) ||
			!Append( node, NULL ) )
	{	RULIB_TRY { delete [] (LPBYTE)node; }
		RULIB_CATCH_ALL { ASSERT( 0 ); }
		return FALSE;
	} // end if

	return node;
}

void * CLList::New( const void *init, DWORD dwUser, const void* pKey, DWORD dwKeySize )
{	return NewObj( GetObjSize(), init, dwUser, pKey, dwKeySize );
}

void CLList::Delete(void * ptr, BOOL bDelete)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return;

	LPLLISTINFO node = (LPLLISTINFO)ptr;
	if ( node == NULL ) return;

	// Remove this node from the list
	if ( node->pPrev != NULL ) node->pPrev->pNext = node->pNext;
	if ( node->pNext != NULL ) node->pNext->pPrev = node->pPrev;
	if ( m_pHead == node ) m_pHead = node->pNext;			
	if ( m_pTail == node ) m_pTail = node->pPrev;		
	
	// Decrement the count
	if ( m_dwSize ) m_dwSize--;

	RULIB_TRY
	{
		// Delete this node
		if ( bDelete ) ReleaseObject( node ); 

	} RULIB_CATCH_ALL { ASSERT( 0 ); }
}

BOOL CLList::MoveUp(void * ptr)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	LPLLISTINFO node = (LPLLISTINFO)ptr;

	// Can't move up any more if we're already first
	if ( node == NULL || node->pPrev == NULL ) return FALSE;

	// Swap us and the previous
	LPLLISTINFO prev = node->pPrev;

	// Adjust head / tail pointers
	if ( m_pHead == prev ) m_pHead = node;
	if ( m_pTail == node ) m_pTail = prev;
	
	// Adjust pointers
	node->pPrev = prev->pPrev;
	if ( node->pPrev != NULL ) node->pPrev->pNext = node;
	prev->pNext = node->pNext;
	if ( prev->pNext != NULL ) prev->pNext->pPrev = prev;
	node->pNext = prev;
	prev->pPrev = node;
	
	return TRUE;
}

BOOL CLList::MoveDown(void * ptr)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	LPLLISTINFO node = (LPLLISTINFO)ptr;

	// Can't move down any more if we're already last
	if ( node == NULL || node->pNext == NULL ) return FALSE;

	// Swap us and the previous
	LPLLISTINFO next = node->pNext;

	// Adjust head / tail pointers
	if ( m_pHead == node ) m_pHead = next;
	if ( m_pTail == next ) m_pTail = node;

	// Swap us and the next node
	node->pNext = next->pNext;
	if ( node->pNext != NULL ) node->pNext->pPrev = node;
	next->pPrev = node->pPrev;
	if ( next->pPrev != NULL ) next->pPrev->pNext = next;
	next->pNext = node;
	node->pPrev = next;

	return TRUE;
}

BOOL CLList::Insert(void * ptr, void * before)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	LPLLISTINFO node = (LPLLISTINFO)ptr;
	LPLLISTINFO pBefore = (LPLLISTINFO)before;

	if ( node == NULL ) return FALSE;

	if ( pBefore == NULL )
	{
		// Add to the start
		node->pNext = m_pHead;
		node->pPrev = NULL;

		// Add this node to the list
		if ( m_pHead != NULL ) m_pHead->pPrev = node;
		m_pHead = node;
		if ( m_pTail == NULL ) m_pTail = node;

		// Count one
		m_dwSize++;

		return TRUE;		
	} // end if

	// Add us in
	node->pNext = pBefore;
	node->pPrev = pBefore->pPrev;
	if ( node->pPrev != NULL ) node->pPrev->pNext = node;
	pBefore->pPrev = node;

	// Take the old guys place
	if ( m_pHead == pBefore ) m_pHead = node;
	if ( m_pTail == NULL ) m_pTail = node;

	m_dwSize++;

	return TRUE;
}

BOOL CLList::Append(void * ptr, void * after)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	LPLLISTINFO node = (LPLLISTINFO)ptr;
	LPLLISTINFO pAfter = (LPLLISTINFO)after;

	if ( node == NULL ) return FALSE;

	if ( pAfter == NULL )
	{
		// Add to the end
		node->pNext = NULL;
		node->pPrev = m_pTail;

		// Add this node to the list
		if ( m_pTail == NULL ) m_pHead = node;
		else m_pTail->pNext = node;
		m_pTail = node;

		m_dwSize++;

		return TRUE;		
	} // end if

	// Add us in
	node->pNext = pAfter->pNext;
	node->pPrev = pAfter;
	if ( node->pNext != NULL ) node->pNext->pPrev = node;
	pAfter->pNext = node;

	// Take the old guys place
	if ( m_pHead == NULL ) m_pHead = node;
	if ( m_pTail == pAfter ) m_pTail = node;

	m_dwSize++;

	return TRUE;
}

BOOL CLList::MoveToTop(void * ptr)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	LPLLISTINFO node = (LPLLISTINFO)ptr;

	// Can't move up any more if we're already first
	if ( node == NULL || node->pPrev == NULL ) return FALSE;

	// Remove from current position
	Delete( node, FALSE );

	// Move to top of list
	return Insert( node, NULL );
}

BOOL CLList::MoveToBottom(void * ptr)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	LPLLISTINFO node = (LPLLISTINFO)ptr;

	// Can't move up any more if we're already first
	if ( node == NULL || node->pNext == NULL ) return FALSE;

	// Remove from current position
	Delete( node, FALSE );

	// Move to bottom of list
	return Append( node, NULL );
}

DWORD CLList::Save(LPCTSTR pFile)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	CWinFile f;

	if ( !f.OpenNew( pFile, GENERIC_WRITE ) )
		return 0;

	DWORD i = 0;

	RULIB_TRY
	{
		LPLLISTINFO plii = NULL;
		while (	( plii = (LPLLISTINFO)GetNext( plii ) ) != NULL )
		{
			// Is there room for data
			f.Write( plii, plii->dwObjSize );

			// Increment byte count
			i += plii->dwObjSize;

		} // end while

	} // end try

	RULIB_CATCH_ALL { ASSERT( 0 ); return 0; }

	return i;
}

DWORD CLList::Load(LPCTSTR pFile, BOOL bMerge )
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	if ( pFile == NULL ) return 0;

	// Lose old record
	if ( !bMerge ) Destroy();

	CWinFile	file;

	// Open the file
	if ( !file.OpenExisting( pFile, GENERIC_READ ) )
		return 0;

	// Get file size
	DWORD size = file.Size();
	if ( size == 0 ) return 0;

	// Allocate memory
	TMem< BYTE > buf;
	if ( !buf.allocate( size + 1 ) ) return 0;

	// Read in the data into ram
	DWORD read;
	if ( !file.Read( buf, size, &read ) || read != size )
		return 0;
	buf[ size ] = 0;

	// Load the file
	if ( !Load( buf, size, bMerge ) )
		return 0;

	return size;
}

DWORD CLList::Save(LPBYTE buf, DWORD size)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	DWORD i = 0;

	RULIB_TRY
	{
		LPLLISTINFO plii = NULL;
		while (	( plii = (LPLLISTINFO)GetNext( plii ) ) != NULL )
		{
			// Just want to know the size?
			if ( buf == NULL ) i += plii->dwObjSize;

			// Is there room for data
			else if ( ( i + plii->dwObjSize ) < size )
			{	memcpy( &buf[ i ], plii, plii->dwObjSize );
				i += plii->dwObjSize;
			} // end if

			// Out of space?
			else return i;

		} // end while

	} // end try

	RULIB_CATCH_ALL { ASSERT( 0 ); return 0; }

	return i;
}

DWORD CLList::Load(LPBYTE buf, DWORD size, BOOL bMerge)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	if ( buf == NULL || size == 0 ) return 0;

	// Lose old record
	if ( !bMerge ) Destroy();

	DWORD i = 0;

	RULIB_TRY
	{
		LPLLISTINFO plii = NULL;

		while ( i < size )
		{
			// Get pointer to object
			plii = (LPLLISTINFO)&buf[ i ];

			// Add object
			if ( ( i + plii->dwObjSize ) < size )
			{	
				// Add new object if same size
				if ( plii->dwObjSize == GetObjSize() ) 
					New( plii );

				// Next object
				i += plii->dwObjSize;

			} // end if

			// Out of space?
			else return i;

		} // end while

	} // end try

	RULIB_CATCH_ALL { ASSERT( 0 ); return 0; }

	return i;
}

void* CLList::Find( const void * pKey, DWORD dwKey )
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	if ( pKey == NULL ) return NULL;

	// Zero len means NULL terminated
	if ( dwKey == NULL ) dwKey = strlen( (LPCTSTR)pKey );

	// Search for this key name
	LPLLISTINFO prf = NULL;
	while( ( prf = (LPLLISTINFO)GetNext( prf ) ) != NULL )
		if ( dwKey == prf->ksize )
			if ( !memcmp( pKey, prf->vpkey, dwKey ) ) return prf;

	return NULL;
}

void* CLList::IFind( LPCTSTR pKey )
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	if ( pKey == NULL ) return NULL;

	LPLLISTINFO prf = NULL;

	// Search for this key name
	while( ( prf = (LPLLISTINFO)GetNext( prf ) ) != NULL && 
			prf->cpkey != NULL )
		if ( !strcmpi( pKey, prf->cpkey ) ) return prf;

	return NULL;
}

void* CLList::Find(DWORD user)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	LPLLISTINFO prf = NULL;

	// Search for this key name
	while( ( prf = (LPLLISTINFO)GetNext( prf ) ) != NULL )
		if ( prf->user == user ) return prf;

	return NULL;
}

BOOL CLList::Link(LPLLISTINFO *list, DWORD size )
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	if ( list == NULL || size == 0 ) return FALSE;

	// Set pointer info
	m_dwSize = size;
	m_pHead = list[ 0 ];
	m_pTail = list[ size - 1 ];

	// Set first node
	list[ 0 ]->pPrev = NULL;
	if ( size > 1 ) list[ 0 ]->pNext = list[ 1 ];
	else list[ 0 ] = NULL;

	// Set last node
	list[ size - 1 ]->pNext = NULL;
	if ( size > 1 ) list[ size - 1 ]->pPrev = list[ size - 2 ];
	else list[ size - 1 ] = NULL;

	// Relink the list
	for ( DWORD i = 1; i < size - 1; i++ )
	{	list[ i ]->pNext = list[ i + 1 ];
		list[ i ]->pPrev = list[ i - 1 ];
	} // end for

	return TRUE;
}

BOOL CLList::Randomize(DWORD seed, DWORD loops)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	DWORD size = Size();

	// Anything to sort?
	if ( size < 2 ) return FALSE;

	// Allocate memory for pointers (speeds up sort)
	LPLLISTINFO	*pList = new LPLLISTINFO[ size + 1 ];
	if ( pList == NULL ) return FALSE;

	// Fill in the index
	DWORD i = 0;
	LPLLISTINFO plli = NULL;
	while ( i < size && ( plli = (LPLLISTINFO)GetNext( plli ) ) != NULL )
		pList[ i++ ] = plli;	

	DWORD crc = 0, i1, i2;
	LPLLISTINFO temp;

	// Randomize the index
	for ( DWORD l = 0; l < loops; l++ )
		for ( i = 0; i < size; i++ )
		{
			// Get the crc
			crc = CWinFile::CRC32( crc, (LPBYTE)&seed, sizeof( seed ) );
			i1 = crc % size; seed ^= crc;
			crc = CWinFile::CRC32( crc, (LPBYTE)&seed, sizeof( seed ) );
			i2 = crc % size; seed ^= crc;

			// Swap these nodes
			temp = pList[ i1 ];
			pList[ i1 ] = pList[ i2 ];
			pList[ i2 ] = temp;		

		} // end for

	// Relink the list
	Link( pList, size );

	// Lose the index
	delete [] pList;

	return TRUE;
}

long CLList::Compare(LPLLISTINFO p1, LPLLISTINFO p2)
{
	LPBYTE register ptr1 = p1->bpkey;
	LPBYTE register ptr2 = p2->bpkey;
	LPBYTE end1;
	LPBYTE end2;

	// Check for missing names
	if ( ptr1 == NULL )
	{	if ( ptr2 == NULL ) return 0;
		else return -1;
	} else if ( ptr2 == NULL ) return 1;

	long register b1;
	long register b2;
	for/*ever*/( ; ; )
	{
		// Get bytes
		b1 = *ptr1;
		b2 = *ptr2;

		// Sort numbers correctly
		if ( b1 >= '0' && b1 <= '9' && b2 >= '0' && b2 <= '9' )
		{	b1 = strtoul( (char*)ptr1, (char**)&end1, 10 );
			b2 = strtoul( (char*)ptr2, (char**)&end2, 10 );

			// Use length of numbers if equal
			if ( b1 == b2 ) b1 = end1 - ptr1, b2 = end2 - ptr2;

			// Point to next characters
			ptr1 = end1;
			ptr2 = end2;

		} // end if

		// Next byte
		else ptr1++, ptr2++;

		// Correct case
		if ( b1 >= 'A' && b1 <= 'Z' ) b1 += ( 'a' - 'A' );
		if ( b2 >= 'A' && b2 <= 'Z' ) b2 += ( 'a' - 'A' );

		// Compare
		if ( b1 != b2 ) return b1 - b2;
		
		// Must be the same to get here

		// Quit if end of data
		if ( b1 == 0 ) return 0;
		
	} // end for

	// Can't get here
	return 0;
}


BOOL CLList::QuickSort( BOOL bRecursive )
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	DWORD size = Size();

	// Anything to sort?
	if ( size < 2 ) return FALSE;

	RULIB_TRY
	{
		// Allocate memory for pointers (speeds up sort)
		LPLLISTINFO	*pList = new LPLLISTINFO[ size + 1 ];
		if ( pList == NULL ) return FALSE;

		// Fill in the index
		DWORD i = 0;
		LPLLISTINFO plli = NULL;
		while ( i < size && ( plli = (LPLLISTINFO)GetNext( plli ) ) != NULL )
			pList[ i++ ] = plli;	

		// Do the sort
		if ( bRecursive ) rQuickSort( pList, 0, m_dwSize - 1 );
		else QuickSort( pList, 0, m_dwSize - 1 );

		// Relink the list
		Link( pList, size );

		// Lose the index
		delete [] pList;
	} // end try

	RULIB_CATCH_ALL { ASSERT( 0 ); return FALSE; }

	return TRUE;
}

void CLList::rQuickSort(LPLLISTINFO * list, DWORD left, DWORD right)
{
	LPLLISTINFO register pivot = list[ left ];
	DWORD register l = left;
	DWORD register r = right;

	while ( l < r )
	{
		// Find smaller value
		while ( Compare( list[ r ], pivot ) >= 0 && l < r ) r--;

		// Need to copy?
		if ( l != r ) list[ l++ ] = list[ r ];

		// Find larger value
		while ( Compare( list[ l ], pivot ) <= 0 && l < r ) l++;

		// Need to copy?
		if ( l != r ) list[ r-- ] = list[ l ];
		
	} // end while

	// Copy pivot point
	list[ l ] = pivot;

	// Recurse to sort sub lists
	if ( left < l ) rQuickSort( list, left, l - 1 );
	if ( right > l ) rQuickSort( list, l + 1, right );
}

BOOL CLList::QuickSort(LPLLISTINFO *list, DWORD left, DWORD right)
{
	// Allocate scratch buffers
	DWORD	ptr = 0;

	TMem< DWORD > buf( right - left );
	TMem< BYTE > dir( right - left );
	if ( !buf.size() || !dir.size() ) return FALSE;

	dir[ ptr ] = 0;

	DWORD mx = 0;
	DWORD register l, r;
	LPLLISTINFO register pivot;

	do
	{
		if ( mx < ptr ) mx = ptr;

		// Call function or pop the stack
		if ( dir[ ptr ] == 4 ) { if ( ptr ) ptr--; }			
		else
		{
			// Init variables
			l = left; r = right; pivot = list[ left ];

			while ( l < r )
			{
				// Find smaller value
				while ( Compare( list[ r ], pivot ) >= 0 && l < r ) r--;

				// Need to copy?
				if ( l != r ) list[ l++ ] = list[ r ];

				// Find larger value
				while ( Compare( list[ l ], pivot ) <= 0 && l < r ) l++;

				// Need to copy?
				if ( l != r ) list[ r-- ] = list[ l ];
				
			} // end while

			// Copy pivot point
			list[ l ] = pivot;

		} // end else

		// Squeeze right
		if ( dir[ ptr ] == 0 )
		{	dir[ ptr ]++;
			if ( left < l ) { buf[ ptr++ ] = right; dir[ ptr ] = 0; right = l - 1; }
			else dir[ ptr ]++;
		} // end if
		else if ( dir[ ptr ] == 1 ) { right = buf[ ptr ]; dir[ ptr ]++; }

		// Squeeze left
		if ( dir[ ptr ] == 2 )
		{	dir[ ptr ]++;
			if ( right > l ) { buf[ ptr++ ] = left; dir[ ptr ] = 0; left = l + 1; }
			else dir[ ptr ]++;
		} // end if

		// Restore
		else if ( dir[ ptr ] == 3 ) { left = buf[ ptr ]; dir[ ptr ]++; }

	} while ( ptr != 0 );

	return TRUE;
}



void* CLList::GetByIndex(DWORD i)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	if ( i >= Size() ) return NULL;

	LPLLISTINFO prf = NULL;

	// Search for this index
	while( ( prf = (LPLLISTINFO)GetNext( prf ) ) != NULL )
		if ( !i ) return prf; else i--;

	return NULL;
}

void* CLList::VerifyPointer(void *ptr)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	if ( ptr == NULL || Size() == 0 ) return NULL;

	LPLLISTINFO prf = NULL;

	// Search for this pointer
	while( ( prf = (LPLLISTINFO)GetNext( prf ) ) != NULL )
		if ( ptr == prf ) return prf;

	return NULL;
}	 


void* CLList::FindByUserData(DWORD data)
{
	// Aquire lock
	CTlLocalLock ll( GetLock(), m_dwLockTimeout );
	if ( !ll.IsLocked() ) return NULL;

	LPLLISTINFO prf = NULL;

	// Search for this data
	while( ( prf = (LPLLISTINFO)GetNext( prf ) ) != NULL )
		if ( prf->user == data ) return prf;

	return NULL;
}
		
const void* CLList::SetKey(void *ptr, const void *pKey, DWORD dwKeySize)
{
	LPLLISTINFO pli = (LPLLISTINFO)ptr;
	if ( pli == NULL ) return pKey;
	
	// Release key memory if any
	if ( pli->vpkey != NULL )
	{	delete [] pli->vpkey;
		pli->vpkey = NULL;
	} // end if

	// Punt if not setting a new key
	pli->ksize = 0;
	if ( pKey == NULL ) return pKey;

	// Zero length means NULL terminated
	if ( dwKeySize == 0 ) 
		while ( ( (LPBYTE)pKey )[ dwKeySize ] != 0 ) dwKeySize++;

	// Verify that we got the memory
	pli->vpkey = new BYTE[ dwKeySize + 1 ];
	if( pli->vpkey == NULL ) { delete [] (LPBYTE)pli; return NULL; }

	// Copy the data
	memcpy( pli->vpkey, pKey, dwKeySize );
	pli->cpkey[ dwKeySize ] = 0;
	pli->ksize = dwKeySize;

	return pKey;
}
