// Mem.cpp: implementation of the CMem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mem.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMem::CMem()
{

}

CMem::~CMem()
{

}

void* CMem::New(DWORD size)
{	return new BYTE[ size ];
}

void* CMem::New(DWORD size, BYTE init )
{	void* buf = new BYTE[ size ];
	if ( buf == NULL ) return NULL;
	memset( buf, init, size );
	return buf;
}

LPSTR CMem::NewStr(DWORD size)
{	return (LPSTR)New( size );
}

LPSTR CMem::NewStr(LPCTSTR pStr)
{
	DWORD size = 0;
	if ( pStr == NULL || *pStr == 0 ) size = 1;
	else size = strlen( pStr );

	LPSTR buf = NewStr( size );

	if ( pStr == NULL || *pStr == 0 ) *buf = 0;
	else
	{	strncpy( buf, pStr, size );
		buf[ size ] = 0;
	} // end else

	return buf;
}

void CMem::DeleteStr(LPSTR *pBuf)
{
	if ( pBuf == NULL || *pBuf == NULL ) return;

	// Release the memory
	delete [] *pBuf;
	*pBuf = NULL;
}

