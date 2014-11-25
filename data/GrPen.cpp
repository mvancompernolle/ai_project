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
// GrPen.cpp: implementation of the CGrPen class.
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

CGrPen::CGrPen()
{_STT();
	m_hPen = NULL;
}

CGrPen::CGrPen( HPEN hPen )
{_STT();
	m_hPen = hPen;
} 

CGrPen::CGrPen( int nStyle, int nWidth, COLORREF rgb ) :
	m_hPen( CreatePen( nStyle, nWidth, rgb ) )
{_STT();
	
} 

CGrPen::~CGrPen()
{_STT();
	Destroy();
}

void CGrPen::Destroy()
{_STT();
	if ( m_hPen != NULL )
	{
		DeleteObject( m_hPen );
		m_hPen = NULL;
	} // end if
}

BOOL CGrPen::Create(int nStyle, int nWidth, COLORREF rgb)
{_STT();
	Destroy();
	
	m_hPen = CreatePen( nStyle, nWidth, rgb );

	return ( m_hPen != NULL );
}
