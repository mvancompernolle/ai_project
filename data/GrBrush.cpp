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
// GrBrush.cpp: implementation of the CGrBrush class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGrBrush::CGrBrush()
{_STT();
	m_hBrush = NULL;
}

CGrBrush::CGrBrush( COLORREF rgb ) :
	m_hBrush( CreateSolidBrush( rgb ) )
{_STT();
}

CGrBrush::CGrBrush( HBRUSH hBrush )
{_STT();
	m_hBrush = hBrush;
}

CGrBrush::CGrBrush( int nStyle, COLORREF rgb )
{_STT();
	m_hBrush = CreateHatchBrush( nStyle, rgb );
}

CGrBrush::CGrBrush( HBITMAP hBmp )
{_STT();
	m_hBrush = CreatePatternBrush( hBmp );
}

CGrBrush::~CGrBrush()
{_STT();
	Destroy();
}

void CGrBrush::Destroy()
{_STT();
	if ( m_hBrush != NULL )
	{
		DeleteObject( m_hBrush );
		m_hBrush = NULL;
	} // end if

}

BOOL CGrBrush::Create(COLORREF rgb)
{_STT();
	Destroy();

	m_hBrush = CreateSolidBrush( rgb );

	return IsObject();
}

BOOL CGrBrush::Create(int nStyle, COLORREF rgb)
{_STT();
	Destroy();

	m_hBrush = CreateHatchBrush( nStyle, rgb );

	return IsObject();
}

BOOL CGrBrush::Create(HBITMAP hBmp)
{_STT();
	Destroy();

	m_hBrush = CreatePatternBrush( hBmp );

	return IsObject();
}
