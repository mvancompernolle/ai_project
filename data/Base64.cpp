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
// Base64.cpp: implementation of the CBase64 class.
//
////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BYTE CBase64::m_b64encode[] = 
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

BYTE CBase64::m_b64decode[] = 
{
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0x3e, 0xff, 0xff, 0xff, 0x3f,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
	0x3c, 0x3d, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff,
	0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
	0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
	0x17, 0x18, 0x19, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
	0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff,

	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};


CBase64::CBase64()
{_STT();
}

CBase64::~CBase64()
{_STT();

}
				
BOOL CBase64::Decode(LPCTSTR src, LPDWORD srcsize, LPBYTE dst, LPDWORD dstsize, LPDWORD done)
{_STT();
	DWORD x = 0, y = 0;
	DWORD b = 0, last = 0;
	DWORD max = *srcsize;

	while( x < max )
	{
		BYTE ch = m_b64decode[ src[ x++ ] ];

		if ( !( ch & 0x80 ) )
		{
			switch ( b )
			{
				case 0 :
					dst[ y ] = ch << 2;
					break;

				case 1 :
					dst[ y++ ] |= ( ch & 0x30 ) >> 4;
					dst[ y ] = ( ch & 0x0f ) << 4;
					break;

				case 2 :
					dst[ y++ ] |= ( ch & 0x3c ) >> 2;
					dst[ y ] = ( ch & 0x03 ) << 6;
					break;
				
				case 3 :
					dst[ y++ ] |= ( ch & 0x3f );
					break;

			} // end switch

			// increment state machine
			if ( ++b > 3 )
			{	b = 0;
				*srcsize = x;
				*dstsize = y;
			} // end if

		} // end if

		// Is this the end of the stream?
		else if ( ch == 0xfe )
		{	
			*srcsize = x;
			*dstsize = y;
			if ( done != NULL ) *done = 1; 
			return TRUE; 
		} // end else if

	} // end while

	return TRUE;
}

DWORD CBase64::Encode(LPSTR dst, DWORD dstsize, LPBYTE src, DWORD srcsize)
{_STT();
	DWORD x = 0, y = 0;
	DWORD b = 0, last = 0;
	DWORD line = 0;

	DWORD val = 0;

	while( x < srcsize && y < dstsize )
	{
		switch( b )
		{
			case 0 : 
				val = src[ x ] >> 2;
				break;

			case 1 :
				val = ( src[ x++ ] & 0x03 ) << 4;
				if ( x < srcsize ) val |= src[ x ] >> 4;
				break;

			case 2 :
				val = ( src[ x++ ] & 0x0f ) << 2;
				if ( x < srcsize ) val |= src[ x ] >> 6;
				break;

			case 3 :
				val = src[ x++ ] & 0x3f;
				break;

		} // end switch

		// Next byte mask
		if ( ++b > 3 ) b = 0;
		
		// Save encoded value
		dst[ y++ ] = m_b64encode[ val ];

		// Line Wrap
		if ( ++line >= CBASE64_LINELEN && ( y + 2 ) < dstsize ) 
		{	line = 0;
			dst[ y++ ] = '\r';
			dst[ y++ ] = '\n';
		} // end if

	} // end while

	// Add padding
	switch ( srcsize % 3 )
	{
		case 1 :
			dst[ y++ ] = '=';
		case 2 :
			dst[ y++ ] = '=';

	} // end switch

	// NULL terminate for good measure
	if ( y < dstsize ) dst[ y ] = 0;

	return y;
}


