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
// Str.cpp: implementation of the CStr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStr::CStr()
{_STTEX();
}

CStr::~CStr()
{_STTEX();

}


CStr& CStr::operator =(unsigned long n)
{_STTEX();
	char num[ 32 ];
	wsprintf( num, "%lu", n );
	copy( num );
	return *this;
}

CStr& CStr::operator =(long n)
{_STTEX();
	char num[ 32 ];
	wsprintf( num, "%li", n );
	copy( num );
	return *this;
}

CStr& CStr::operator =(LPCTSTR str)
{_STTEX();
	copy( str );
	return *this;
}

CStr& CStr::operator =(double n)
{_STTEX();
	char num[ 32 ];
	sprintf( num, "%g", n );
	copy( num );
	return *this;
}

CStr& CStr::operator +=(unsigned long n)
{_STTEX();
	char num[ 32 ];
	wsprintf( num, "%lu", n );
	*this += num;
	return *this;
}

CStr& CStr::operator +=(long n)
{_STTEX();
	char num[ 32 ];
	wsprintf( num, "%li", n );
	*this += num;
	return *this;
}

CStr& CStr::operator +=(LPCTSTR str)
{_STTEX();
	DWORD end = strlen();
	if ( grow( ::strlen( str ) + end + 1 ) )
		strcpy( &( ptr()[ end ] ), str );
	return *this;
}

CStr& CStr::operator +=(double n)
{_STTEX();
	char num[ 32 ];
	sprintf( num, "%f", n );
	*this += num;
	return *this;
}

LPCTSTR CStr::Set(double n, LPCTSTR pTemplate)
{_STTEX();
	char num[ 32 ];
	sprintf( num, pTemplate, n );
	*this = num;
	return str();
}
