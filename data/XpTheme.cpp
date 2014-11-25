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
// XpTheme.cpp: implementation of the CXpTheme class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AtlBase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DWORD		CXpTheme::m_dwLast = 1;
LRESULT		CXpTheme::m_hrLast[ CXPTHEME_MAXERR ] = { S_OK };

HMODULE		CXpTheme::m_hModule = NULL;
void*		CXpTheme::m_ixptf[ ixptf_PFNSIZE ] = 
{	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// 20
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// 40
	0,0,0,0,0,0,0								// 47
};

CXpTheme::CXpTheme()
{_STT();
	m_bReleaseTheme = FALSE;
	m_hTheme = NULL;

	// Load module
	Init();
}

CXpTheme::CXpTheme( HWND hWnd, LPCTSTR pClass )
{_STT();
	m_bReleaseTheme = FALSE;
	m_hTheme = NULL;

	// Load module
	Init();

	// Open the theme
	Open( hWnd, pClass );
}

CXpTheme::~CXpTheme()
{_STT();
	Destroy();
}

void CXpTheme::Destroy()
{_STT();
	// Close theme if needed
	Close();
}

BOOL CXpTheme::Init()
{_STT();
	if ( m_hModule != NULL ) return TRUE;

	// Load the module
	m_hModule = LoadLibrary( "UxTheme.dll" );
	if ( m_hModule == NULL ) return FALSE;

	// Get function pointers
	LoadFunctionPointers();

	return TRUE;
}

void CXpTheme::Uninit()
{_STT();
	// No more function pointers
	ZeroMemory( m_ixptf, sizeof( m_ixptf ) );

	// Free library
	if ( m_hModule != NULL )
	{	FreeLibrary( m_hModule );
		m_hModule = NULL;
	} // end if
}

LRESULT CXpTheme::SetErr(LRESULT err)
{_STT();
	if ( err == S_OK ) return S_OK;

	// Store error in buffer
	m_hrLast[ m_dwLast ] = err;
	if ( ++m_dwLast >= CXPTHEME_MAXERR ) m_dwLast = 0;

	return err;
}

LRESULT CXpTheme::GetLastError()
{_STT();
	DWORD err = m_dwLast;
	if ( err ) err--; else err = CXPTHEME_MAXERR - 1;

	return m_hrLast[ err ];
}

BOOL CXpTheme::LoadFunctionPointers()
{_STT();
	if ( m_hModule == NULL ) return FALSE;

	m_ixptf[ ixptf_PFNOPENTHEMEDATA ] = GetProcAddress( m_hModule, "OpenThemeData" );
	m_ixptf[ ixptf_PFNCLOSETHEMEDATA ] = GetProcAddress( m_hModule, "CloseThemeData" );
	m_ixptf[ ixptf_PFNDRAWTHEMEBACKGROUND ] = GetProcAddress( m_hModule, "DrawThemeBackground" );
	m_ixptf[ ixptf_PFNDRAWTHEMETEXT ] = GetProcAddress( m_hModule, "DrawThemeText" );
	m_ixptf[ ixptf_PFNGETTHEMEBACKGROUNDCONTENTRECT ] = GetProcAddress( m_hModule, "GetThemeBackgroundContentRect" );
	m_ixptf[ ixptf_PFNGETTHEMEBACKGROUNDEXTENT ] = GetProcAddress( m_hModule, "GetThemeBackgroundExtent" );
	m_ixptf[ ixptf_PFNGETTHEMEPARTSIZE ] = GetProcAddress( m_hModule, "GetThemePartSize" );
	m_ixptf[ ixptf_PFNGETTHEMETEXTEXTENT ] = GetProcAddress( m_hModule, "GetThemeTextExtent" );
	m_ixptf[ ixptf_PFNGETTHEMETEXTMETRICS ] = GetProcAddress( m_hModule, "GetThemeTextMetrics" );
	m_ixptf[ ixptf_PFNGETTHEMEBACKGROUNDREGION ] = GetProcAddress( m_hModule, "GetThemeTextMetrics" );
	m_ixptf[ ixptf_PFNHITTESTTHEMEBACKGROUND ] = GetProcAddress( m_hModule, "HitTestThemeBackground" );
	m_ixptf[ ixptf_PFNDRAWTHEMEEDGE ] = GetProcAddress( m_hModule, "DrawThemeEdge" );
	m_ixptf[ ixptf_PFNDRAWTHEMEICON ] = GetProcAddress( m_hModule, "DrawThemeIcon" );
	m_ixptf[ ixptf_PFNISTHEMEPARTDEFINED ] = GetProcAddress( m_hModule, "IsThemePartDefined" );
	m_ixptf[ ixptf_PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT ] = GetProcAddress( m_hModule, "IsThemeBackgroundPartiallyTransparent" );
	m_ixptf[ ixptf_PFNGETTHEMECOLOR ] = GetProcAddress( m_hModule, "GetThemeColor" );
	m_ixptf[ ixptf_PFNGETTHEMEMETRIC ] = GetProcAddress( m_hModule, "GetThemeMetric" );
	m_ixptf[ ixptf_PFNGETTHEMESTRING ] = GetProcAddress( m_hModule, "GetThemeString" );
	m_ixptf[ ixptf_PFNGETTHEMEBOOL ] = GetProcAddress( m_hModule, "GetThemeBool" );
	m_ixptf[ ixptf_PFNGETTHEMEINT ] = GetProcAddress( m_hModule, "GetThemeInt" );
	m_ixptf[ ixptf_PFNGETTHEMEENUMVALUE ] = GetProcAddress( m_hModule, "GetThemeEnumValue" );
	m_ixptf[ ixptf_PFNGETTHEMEPOSITION ] = GetProcAddress( m_hModule, "GetThemePosition" );
	m_ixptf[ ixptf_PFNGETTHEMEFONT ] = GetProcAddress( m_hModule, "GetThemeFont" );
	m_ixptf[ ixptf_PFNGETTHEMERECT ] = GetProcAddress( m_hModule, "GetThemeRect" );
	m_ixptf[ ixptf_PFNGETTHEMEMARGINS ] = GetProcAddress( m_hModule, "GetThemeMargins" );
	m_ixptf[ ixptf_PFNGETTHEMEINTLIST ] = GetProcAddress( m_hModule, "GetThemeIntList" );
	m_ixptf[ ixptf_PFNGETTHEMEPROPERTYORIGIN ] = GetProcAddress( m_hModule, "GetThemePropertyOrigin" );
	m_ixptf[ ixptf_PFNSETWINDOWTHEME ] = GetProcAddress( m_hModule, "SetWindowTheme" );
	m_ixptf[ ixptf_PFNGETTHEMEFILENAME ] = GetProcAddress( m_hModule, "GetThemeFilename" );
	m_ixptf[ ixptf_PFNGETTHEMESYSCOLOR ] = GetProcAddress( m_hModule, "GetThemeSysColor" );
	m_ixptf[ ixptf_PFNGETTHEMESYSCOLORBRUSH ] = GetProcAddress( m_hModule, "GetThemeSysColorBrush" );
	m_ixptf[ ixptf_PFNGETTHEMESYSBOOL ] = GetProcAddress( m_hModule, "GetThemeSysBool" );
	m_ixptf[ ixptf_PFNGETTHEMESYSSIZE ] = GetProcAddress( m_hModule, "GetThemeSysSize" );
	m_ixptf[ ixptf_PFNGETTHEMESYSFONT ] = GetProcAddress( m_hModule, "GetThemeSysFont" );
	m_ixptf[ ixptf_PFNGETTHEMESYSSTRING ] = GetProcAddress( m_hModule, "GetThemeSysString" );
	m_ixptf[ ixptf_PFNGETTHEMESYSINT ] = GetProcAddress( m_hModule, "GetThemeSysInt" );
	m_ixptf[ ixptf_PFNISTHEMEACTIVE ] = GetProcAddress( m_hModule, "IsThemeActive" );
	m_ixptf[ ixptf_PFNISAPPTHEMED ] = GetProcAddress( m_hModule, "IsAppThemed" );
	m_ixptf[ ixptf_PFNGETWINDOWTHEME ] = GetProcAddress( m_hModule, "GetWindowTheme" );
	m_ixptf[ ixptf_PFNENABLETHEMEDIALOGTEXTURE ] = GetProcAddress( m_hModule, "EnableThemeDialogTexture" );
	m_ixptf[ ixptf_PFNISTHEMEDIALOGTEXTUREENABLED ] = GetProcAddress( m_hModule, "IsThemeDialogTextureEnabled" );
	m_ixptf[ ixptf_PFNGETTHEMEAPPPROPERTIES ] = GetProcAddress( m_hModule, "GetThemeAppProperties" );
	m_ixptf[ ixptf_PFNSETTHEMEAPPPROPERTIES ] = GetProcAddress( m_hModule, "SetThemeAppProperties" );
	m_ixptf[ ixptf_PFNGETCURRENTTHEMENAME ] = GetProcAddress( m_hModule, "GetCurrentThemeName" );
	m_ixptf[ ixptf_PFNGETTHEMEDOCUMENTATIONPROPERTY ] = GetProcAddress( m_hModule, "GetThemeDocumentationProperty" );
	m_ixptf[ ixptf_PFNDRAWTHEMEPARENTBACKGROUND ] = GetProcAddress( m_hModule, "DrawThemeParentBackground" );
	m_ixptf[ ixptf_PFNENABLETHEMING ] = GetProcAddress( m_hModule, "EnableTheming" );

	return TRUE;
}

typedef HTHEME (__stdcall *PFNOPENTHEMEDATA )(HWND hwnd, LPCWSTR pszClassList);
BOOL CXpTheme::Open( HWND hwnd, LPCTSTR pszClassList )
{_STT();
	Close();

	PFNOPENTHEMEDATA pf = (PFNOPENTHEMEDATA)m_ixptf[ ixptf_PFNOPENTHEMEDATA ];
	if ( pf == NULL ) return FALSE;
	m_hTheme = pf( hwnd, CComBSTR( pszClassList ) );
	if ( m_hTheme == NULL ) return FALSE;

	m_bReleaseTheme = TRUE;

	return TRUE;
}

typedef HRESULT(__stdcall *PFNCLOSETHEMEDATA)(HTHEME hTheme);
BOOL CXpTheme::Close()
{_STT();
	if ( !IsDll() ) { m_hTheme = NULL; return FALSE; }
	if ( m_hTheme == NULL ) return FALSE;

	// Are we releasing the handle?
	if ( m_bReleaseTheme )
	{	m_bReleaseTheme = FALSE;

		PFNCLOSETHEMEDATA pf = (PFNCLOSETHEMEDATA)m_ixptf[ ixptf_PFNCLOSETHEMEDATA ];
		if ( pf == NULL ) return FALSE;

		pf( m_hTheme );

	} // end if

	m_hTheme = NULL;
	
	return TRUE;
}

typedef HRESULT(__stdcall *PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect,  const RECT *pClipRect);
BOOL CXpTheme::DrawThemeBackground(HDC hDC, int iPartId, int iStateId, LPRECT pRect, LPRECT pClipRect)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNDRAWTHEMEBACKGROUND pf = (PFNDRAWTHEMEBACKGROUND)m_ixptf[ ixptf_PFNDRAWTHEMEBACKGROUND ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, hDC, iPartId, iStateId, pRect, pClipRect ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNDRAWTHEMETEXT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect);
BOOL CXpTheme::DrawThemeText(HDC hDC, int iPartId, int iStateId, LPCTSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, LPRECT pRect)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNDRAWTHEMETEXT pf = (PFNDRAWTHEMETEXT)m_ixptf[ ixptf_PFNDRAWTHEMETEXT ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, hDC, iPartId, iStateId, CComBSTR( pszText ), iCharCount, dwTextFlags, dwTextFlags2, pRect ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNGETTHEMEBACKGROUNDCONTENTRECT)(HTHEME hTheme,  HDC hdc, int iPartId, int iStateId,  const RECT *pBoundingRect, RECT *pContentRect);
BOOL CXpTheme::GetThemeBackgroundContentRect(HDC hDC, int iPartId, int iStateId, LPRECT pBoundingRect, LPRECT pContentRect)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMEBACKGROUNDCONTENTRECT pf = (PFNGETTHEMEBACKGROUNDCONTENTRECT)m_ixptf[ ixptf_PFNGETTHEMEBACKGROUNDCONTENTRECT ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, hDC, iPartId, iStateId, pBoundingRect, pContentRect ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNGETTHEMEBACKGROUNDEXTENT)(HTHEME hTheme,  HDC hdc, int iPartId, int iStateId, const RECT *pContentRect, RECT *pExtentRect);
BOOL CXpTheme::GetThemeBackgroundExtent(HDC hDC, int iPartId, int iStateId, LPRECT pContentRect, LPRECT pExtentRect)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMEBACKGROUNDEXTENT pf = (PFNGETTHEMEBACKGROUNDEXTENT)m_ixptf[ ixptf_PFNGETTHEMEBACKGROUNDEXTENT ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, hDC, iPartId, iStateId, pContentRect, pExtentRect ) ) == S_OK;
}

typedef HRESULT(__stdcall *PFNGETTHEMEPARTSIZE)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, RECT * pRect, enum THEMESIZE eSize,  SIZE *psz);
BOOL CXpTheme::GetThemePartSize(HDC hDC, int iPartId, int iStateId, LPRECT pRect, enum THEMESIZE eSize, LPSIZE psz)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMEPARTSIZE pf = (PFNGETTHEMEPARTSIZE)m_ixptf[ ixptf_PFNGETTHEMEPARTSIZE ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, hDC, iPartId, iStateId, pRect, eSize, psz ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNGETTHEMETEXTEXTENT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags,  const RECT *pBoundingRect, RECT *pExtentRect);
BOOL CXpTheme::GetThemeTextExtent(HDC hDC, int iPartId, int iStateId, LPCTSTR pszText, int iCharCount, DWORD dwTextFlags, LPRECT pBoundingRect, LPRECT pExtentRect)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMETEXTEXTENT pf = (PFNGETTHEMETEXTEXTENT)m_ixptf[ ixptf_PFNGETTHEMETEXTEXTENT ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, hDC, iPartId, iStateId, CComBSTR( pszText ), iCharCount, dwTextFlags, pBoundingRect, pExtentRect ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNGETTHEMETEXTMETRICS)(HTHEME hTheme,  HDC hdc, int iPartId, int iStateId,  TEXTMETRIC* ptm);
BOOL CXpTheme::GetThemeTextMetrics(HDC hDC, int iPartId, int iStateId, LPTEXTMETRIC ptm)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMETEXTMETRICS pf = (PFNGETTHEMETEXTMETRICS)m_ixptf[ ixptf_PFNGETTHEMETEXTMETRICS ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, hDC, iPartId, iStateId, ptm ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNGETTHEMEBACKGROUNDREGION)(HTHEME hTheme,  HDC hdc, int iPartId, int iStateId, const RECT *pRect,  HRGN *pRegion);
BOOL CXpTheme::GetThemeBackgroundRegion(HDC hDC, int iPartId, int iStateId, LPRECT pRect, HRGN *pRegion)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMEBACKGROUNDREGION pf = (PFNGETTHEMEBACKGROUNDREGION)m_ixptf[ ixptf_PFNGETTHEMEBACKGROUNDREGION ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, hDC, iPartId, iStateId, pRect, pRegion ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNHITTESTTHEMEBACKGROUND)(HTHEME hTheme,  HDC hdc, int iPartId, int iStateId, DWORD dwOptions, const RECT *pRect,  HRGN hrgn, POINT ptTest,  WORD *pwHitTestCode);
BOOL CXpTheme::HitTestThemeBackground(HDC hDC, int iPartId, int iStateId, DWORD dwOptions, LPRECT pRect, HRGN hRgn, POINT ptTest, LPWORD pwHitTestCode)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNHITTESTTHEMEBACKGROUND pf = (PFNHITTESTTHEMEBACKGROUND)m_ixptf[ ixptf_PFNHITTESTTHEMEBACKGROUND ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, hDC, iPartId, iStateId, dwOptions, pRect, hRgn, ptTest, pwHitTestCode ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNDRAWTHEMEEDGE)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pDestRect, UINT uEdge, UINT uFlags,   RECT *pContentRect);
BOOL CXpTheme::DrawThemeEdge(HDC hDC, int iPartId, int iStateId, LPRECT pDestRect, UINT uEdge, UINT uFlags, LPRECT pContentRect)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNDRAWTHEMEEDGE pf = (PFNDRAWTHEMEEDGE)m_ixptf[ ixptf_PFNDRAWTHEMEEDGE ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, hDC, iPartId, iStateId, pDestRect, uEdge, uFlags, pContentRect ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNDRAWTHEMEICON)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex);
BOOL CXpTheme::DrawThemeIcon(HDC hDC, int iPartId, int iStateId, LPRECT pRect, HIMAGELIST himl, int iImageIndex)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNDRAWTHEMEICON pf = (PFNDRAWTHEMEICON)m_ixptf[ ixptf_PFNDRAWTHEMEICON ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, hDC, iPartId, iStateId, pRect, himl, iImageIndex ) ) == S_OK;
}

typedef BOOL (__stdcall *PFNISTHEMEPARTDEFINED)(HTHEME hTheme, int iPartId, int iStateId);
BOOL CXpTheme::IsThemePartDefined(int iPartId, int iStateId)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNISTHEMEPARTDEFINED pf = (PFNISTHEMEPARTDEFINED)m_ixptf[ ixptf_PFNISTHEMEPARTDEFINED ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return pf( m_hTheme, iPartId, iStateId );
}

typedef BOOL (__stdcall *PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT)(HTHEME hTheme, int iPartId, int iStateId);
BOOL CXpTheme::IsThemeBackgroundPartiallyTransparent(int iPartId, int iStateId)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT pf = (PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT)m_ixptf[ ixptf_PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return pf( m_hTheme, iPartId, iStateId );
}

typedef HRESULT (__stdcall *PFNGETTHEMECOLOR)(HTHEME hTheme, int iPartId, int iStateId, int iPropId,  COLORREF *pColor);
BOOL CXpTheme::GetThemeColor(int iPartId, int iStateId, int iPropId, LPCOLORREF pColor)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMECOLOR pf = (PFNGETTHEMECOLOR)m_ixptf[ ixptf_PFNGETTHEMECOLOR ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, iPartId, iStateId, iPropId, pColor ) ) == S_OK;
}

COLORREF CXpTheme::GetThemeColor(int iPartId, int iStateId, int iPropId, COLORREF rgbDefault)
{_STT();
	COLORREF rgb = rgbDefault;
	if ( GetThemeColor( iPartId, iStateId, iPropId, &rgb ) ) return rgb;
	return rgbDefault;
}

typedef HRESULT (__stdcall *PFNGETTHEMEMETRIC)(HTHEME hTheme,  HDC hdc, int iPartId, int iStateId, int iPropId,  int *piVal);
BOOL CXpTheme::GetThemeMetric(HDC hDC, int iPartId, int iStateId, int iPropId, int *piVal)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMEMETRIC pf = (PFNGETTHEMEMETRIC)m_ixptf[ ixptf_PFNGETTHEMEMETRIC ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, hDC, iPartId, iStateId, iPropId, piVal ) ) == S_OK;
}

int CXpTheme::GetThemeMetric(HDC hDC, int iPartId, int iStateId, int iPropId, int iDefault)
{_STT();
	int i = iDefault;
	if ( GetThemeMetric( hDC, iPartId, iStateId, iPropId, &i ) ) return i;
	return i;
}


typedef HRESULT (__stdcall *PFNGETTHEMESTRING)(HTHEME hTheme, int iPartId, int iStateId, int iPropId,  LPWSTR pszBuff, int cchMaxBuffChars);
BOOL CXpTheme::GetThemeString(int iPartId, int iStateId, int iPropId, LPSTR pszBuff, int cchMaxBufChars)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMESTRING pf = (PFNGETTHEMESTRING)m_ixptf[ ixptf_PFNGETTHEMESTRING ];
	if ( pf == NULL ) return FALSE;

	CComBSTR buf( cchMaxBufChars );

	// Do the call
	LRESULT ret = SetErr( pf( m_hTheme, iPartId, iStateId, iPropId, buf, cchMaxBufChars ) );

	wcstombs( pszBuff, buf, cchMaxBufChars );

	return ( ret == S_OK );
}

typedef HRESULT (__stdcall *PFNGETTHEMEBOOL)(HTHEME hTheme, int iPartId, int iStateId, int iPropId,  BOOL *pfVal);
BOOL CXpTheme::GetThemeBool(int iPartId, int iStateId, int iPropId, BOOL *pfVal)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMEBOOL pf = (PFNGETTHEMEBOOL)m_ixptf[ ixptf_PFNGETTHEMEBOOL ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, iPartId, iStateId, iPropId, pfVal ) ) == S_OK;
}

BOOL CXpTheme::GetThemeBool(int iPartId, int iStateId, int iPropId, BOOL bDefault)
{_STT();
	BOOL b = bDefault;
	if ( GetThemeBool( iPartId, iStateId, iPropId, &b ) ) return b;
	return b;
}

typedef HRESULT (__stdcall *PFNGETTHEMEINT)(HTHEME hTheme, int iPartId, int iStateId, int iPropId,  int *piVal);
BOOL CXpTheme::GetThemeInt(int iPartId, int iStateId, int iPropId, int *piVal)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMEINT pf = (PFNGETTHEMEINT)m_ixptf[ ixptf_PFNGETTHEMEINT ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, iPartId, iStateId, iPropId, piVal ) ) == S_OK;
}

int CXpTheme::GetThemeInt(int iPartId, int iStateId, int iPropId, int iDefault)
{_STT();
	int i = iDefault;
	if ( GetThemeInt( iPartId, iStateId, iPropId, &i ) ) return i;
	return iDefault;
}

typedef HRESULT (__stdcall *PFNGETTHEMEENUMVALUE)(HTHEME hTheme, int iPartId, int iStateId, int iPropId,  int *piVal);
BOOL CXpTheme::GetThemeEnumValue(int iPartId, int iStateId, int iPropId, int *piVal)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMEENUMVALUE pf = (PFNGETTHEMEENUMVALUE)m_ixptf[ ixptf_PFNGETTHEMEENUMVALUE ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, iPartId, iStateId, iPropId, piVal ) ) == S_OK;
}


typedef HRESULT (__stdcall *PFNGETTHEMEPOSITION)(HTHEME hTheme, int iPartId, int iStateId, int iPropId,  POINT *pPoint);
BOOL CXpTheme::GetThemePosition(int iPartId, int iStateId, int iPropId, LPPOINT pPoint)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMEPOSITION pf = (PFNGETTHEMEPOSITION)m_ixptf[ ixptf_PFNGETTHEMEPOSITION ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, iPartId, iStateId, iPropId, pPoint ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNGETTHEMEFONT)(HTHEME hTheme,  HDC hdc, int iPartId, int iStateId, int iPropId,  LOGFONT *pFont);
BOOL CXpTheme::GetThemeFont(HDC hDC, int iPartId, int iStateId, int iPropId, LPLOGFONT pFont)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMEFONT pf = (PFNGETTHEMEFONT)m_ixptf[ ixptf_PFNGETTHEMEFONT ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, hDC, iPartId, iStateId, iPropId, pFont ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNGETTHEMERECT)(HTHEME hTheme, int iPartId, int iStateId, int iPropId,  RECT *pRect);
BOOL CXpTheme::GetThemeRect(int iPartId, int iStateId, int iPropId, LPRECT pRect)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMERECT pf = (PFNGETTHEMERECT)m_ixptf[ ixptf_PFNGETTHEMERECT ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, iPartId, iStateId, iPropId, pRect ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNGETTHEMEMARGINS)(HTHEME hTheme,  HDC hdc, int iPartId, int iStateId, int iPropId,  RECT *prc,  MARGINS *pMargins);
BOOL CXpTheme::GetThemeMargins(HDC hDC, int iPartId, int iStateId, int iPropId, LPRECT prc, MARGINS *pMargins)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMEMARGINS pf = (PFNGETTHEMEMARGINS)m_ixptf[ ixptf_PFNGETTHEMEMARGINS ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, hDC, iPartId, iStateId, iPropId, prc, pMargins ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNGETTHEMEINTLIST)(HTHEME hTheme, int iPartId, int iStateId, int iPropId,  INTLIST *pIntList);
BOOL CXpTheme::GetThemeIntList(int iPartId, int iStateId, int iPropId, INTLIST *pIntList)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMEINTLIST pf = (PFNGETTHEMEINTLIST)m_ixptf[ ixptf_PFNGETTHEMEINTLIST ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, iPartId, iStateId, iPropId, pIntList ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNGETTHEMEPROPERTYORIGIN)(HTHEME hTheme, int iPartId, int iStateId, int iPropId,  enum PROPERTYORIGIN *pOrigin);
BOOL CXpTheme::GetThemePropertyOrigin(int iPartId, int iStateId, int iPropId, enum PROPERTYORIGIN *pOrigin)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMEPROPERTYORIGIN pf = (PFNGETTHEMEPROPERTYORIGIN)m_ixptf[ ixptf_PFNGETTHEMEPROPERTYORIGIN ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, iPartId, iStateId, iPropId, pOrigin ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNSETWINDOWTHEME)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
BOOL CXpTheme::SetWindowTheme(HWND hWnd, LPCTSTR pszSubAppName, LPCTSTR pszSubIdList)
{_STT();
	// Get function pointer
	PFNSETWINDOWTHEME pf = (PFNSETWINDOWTHEME)m_ixptf[ ixptf_PFNSETWINDOWTHEME ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( hWnd, CComBSTR( pszSubAppName ), CComBSTR( pszSubIdList ) ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNGETTHEMEFILENAME)(HTHEME hTheme, int iPartId, int iStateId, int iPropId,  LPWSTR pszThemeFileName, int cchMaxBuffChars);
BOOL CXpTheme::GetThemeFilename(int iPartId, int iStateId, int iPropId, LPSTR pszThemeFileName, int cchMaxBufChars)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMEFILENAME pf = (PFNGETTHEMEFILENAME)m_ixptf[ ixptf_PFNGETTHEMEFILENAME ];
	if ( pf == NULL ) return FALSE;

	CComBSTR fname( cchMaxBufChars );

	// Do the call
	LRESULT ret = SetErr( pf( m_hTheme, iPartId, iStateId, iPropId, fname, cchMaxBufChars ) );

	wcstombs( pszThemeFileName, fname, cchMaxBufChars );

	return ( ret == S_OK );
}

typedef COLORREF (__stdcall *PFNGETTHEMESYSCOLOR)(HTHEME hTheme, int iColorId);
COLORREF CXpTheme::GetThemeSysColor(int iColorId)
{_STT();
	if ( !IsTheme() ) return RGB( 0, 0, 0 );

	// Get function pointer
	PFNGETTHEMESYSCOLOR pf = (PFNGETTHEMESYSCOLOR)m_ixptf[ ixptf_PFNGETTHEMESYSCOLOR ];
	if ( pf == NULL ) return RGB( 0, 0, 0 );

	// Do the call
	return pf( m_hTheme, iColorId );
}

typedef HBRUSH (__stdcall *PFNGETTHEMESYSCOLORBRUSH)(HTHEME hTheme, int iColorId);
HBRUSH CXpTheme::GetThemeSysColorBrush(int iColorId)
{_STT();
	if ( !IsTheme() ) return NULL;

	// Get function pointer
	PFNGETTHEMESYSCOLORBRUSH pf = (PFNGETTHEMESYSCOLORBRUSH)m_ixptf[ ixptf_PFNGETTHEMESYSCOLORBRUSH ];
	if ( pf == NULL ) return NULL;

	// Do the call
	return pf( m_hTheme, iColorId );
}

typedef BOOL (__stdcall *PFNGETTHEMESYSBOOL)(HTHEME hTheme, int iBoolId);
BOOL CXpTheme::GetThemeSysBool(int iBoolId)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMESYSBOOL pf = (PFNGETTHEMESYSBOOL)m_ixptf[ ixptf_PFNGETTHEMESYSBOOL ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return pf( m_hTheme, iBoolId );
}

typedef int (__stdcall *PFNGETTHEMESYSSIZE)(HTHEME hTheme, int iSizeId);
int CXpTheme::GetThemeSysSize(int iSizeId)
{_STT();
	if ( !IsTheme() ) return 0;

	// Get function pointer
	PFNGETTHEMESYSSIZE pf = (PFNGETTHEMESYSSIZE)m_ixptf[ ixptf_PFNGETTHEMESYSSIZE ];
	if ( pf == NULL ) return 0;

	// Do the call
	return pf( m_hTheme, iSizeId );
}

typedef HRESULT (__stdcall *PFNGETTHEMESYSFONT)(HTHEME hTheme, int iFontId,  LOGFONT *plf);
BOOL CXpTheme::GetThemeSysFont(int iFontId, LPLOGFONT plf)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMESYSFONT pf = (PFNGETTHEMESYSFONT)m_ixptf[ ixptf_PFNGETTHEMESYSFONT ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, iFontId, plf ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNGETTHEMESYSSTRING)(HTHEME hTheme, int iStringId, LPWSTR pszStringBuff, int cchMaxStringChars);
BOOL CXpTheme::GetThemeSysString(int iStringId, LPSTR pszStringBuf, int cchMaxStringChars)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMESYSSTRING pf = (PFNGETTHEMESYSSTRING)m_ixptf[ ixptf_PFNGETTHEMESYSSTRING ];
	if ( pf == NULL ) return FALSE;

	CComBSTR str( cchMaxStringChars );

	// Do the call
	LRESULT ret = SetErr( pf( m_hTheme, iStringId, str, cchMaxStringChars ) );

	wcstombs( pszStringBuf, str, cchMaxStringChars );

	return ( ret == S_OK );
}

typedef HRESULT (__stdcall *PFNGETTHEMESYSINT)(HTHEME hTheme, int iIntId, int *piValue);
BOOL CXpTheme::GetThemeSysInt(int iIntId, int *piValue)
{_STT();
	if ( !IsTheme() ) return FALSE;

	// Get function pointer
	PFNGETTHEMESYSINT pf = (PFNGETTHEMESYSINT)m_ixptf[ ixptf_PFNGETTHEMESYSINT ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( m_hTheme, iIntId, piValue ) ) == S_OK;
}

typedef BOOL (__stdcall *PFNISTHEMEACTIVE)();
BOOL CXpTheme::IsThemeActive()
{_STT();
	// Get function pointer
	PFNISTHEMEACTIVE pf = (PFNISTHEMEACTIVE)m_ixptf[ ixptf_PFNISTHEMEACTIVE ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return pf();
}

typedef BOOL(__stdcall *PFNISAPPTHEMED)();
BOOL CXpTheme::IsAppThemed()
{_STT();
	// Get function pointer
	PFNISAPPTHEMED pf = (PFNISAPPTHEMED)m_ixptf[ ixptf_PFNISAPPTHEMED ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return pf();
}

typedef HTHEME (__stdcall *PFNGETWINDOWTHEME)(HWND hwnd);
HTHEME CXpTheme::GetWindowTheme(HWND hWnd)
{_STT();
	// Get function pointer
	PFNGETWINDOWTHEME pf = (PFNGETWINDOWTHEME)m_ixptf[ ixptf_PFNGETWINDOWTHEME ];
	if ( pf == NULL ) return NULL;

	// Do the call
	return pf( hWnd );
}

typedef HRESULT (__stdcall *PFNENABLETHEMEDIALOGTEXTURE)(HWND hwnd, DWORD dwFlags);
BOOL CXpTheme::EnableThemeDialogTexture(HWND hWnd, DWORD dwFlags)
{_STT();
	// Get function pointer
	PFNENABLETHEMEDIALOGTEXTURE pf = (PFNENABLETHEMEDIALOGTEXTURE)m_ixptf[ ixptf_PFNENABLETHEMEDIALOGTEXTURE ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( hWnd, dwFlags ) ) == S_OK;
}

typedef BOOL (__stdcall *PFNISTHEMEDIALOGTEXTUREENABLED)(HWND hwnd);
BOOL CXpTheme::IsThemeDialogTextureEnabled(HWND hWnd)
{_STT();
	// Get function pointer
	PFNISTHEMEDIALOGTEXTUREENABLED pf = (PFNISTHEMEDIALOGTEXTUREENABLED)m_ixptf[ ixptf_PFNISTHEMEDIALOGTEXTUREENABLED ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return pf( hWnd );
}

typedef DWORD (__stdcall *PFNGETTHEMEAPPPROPERTIES)();
DWORD CXpTheme::GetThemeAppProperties()
{_STT();
	// Get function pointer
	PFNGETTHEMEAPPPROPERTIES pf = (PFNGETTHEMEAPPPROPERTIES)m_ixptf[ ixptf_PFNGETTHEMEAPPPROPERTIES ];
	if ( pf == NULL ) return 0;

	// Do the call
	return pf();
}

typedef void (__stdcall *PFNSETTHEMEAPPPROPERTIES)(DWORD dwFlags);
BOOL CXpTheme::SetThemeAppProperties(DWORD dwFlags)
{_STT();
	// Get function pointer
	PFNSETTHEMEAPPPROPERTIES pf = (PFNSETTHEMEAPPPROPERTIES)m_ixptf[ ixptf_PFNSETTHEMEAPPPROPERTIES ];
	if ( pf == NULL ) return 0;

	// Do the call
	pf( dwFlags );

	return TRUE;
}

typedef HRESULT (__stdcall *PFNGETCURRENTTHEMENAME)(LPWSTR pszThemeFileName, int cchMaxNameChars, LPWSTR pszColorBuff, int cchMaxColorChars, LPWSTR pszSizeBuff, int cchMaxSizeChars);
BOOL CXpTheme::GetCurrentThemeName(LPSTR pszThemeFileName, int cchMaxNameChars, LPSTR pszColorBuff, int cchMaxColorChars, LPSTR pszSizeBuff, int cchMaxSizeChars)
{_STT();
	// Get function pointer
	PFNGETCURRENTTHEMENAME pf = (PFNGETCURRENTTHEMENAME)m_ixptf[ ixptf_PFNGETCURRENTTHEMENAME ];
	if ( pf == NULL ) return FALSE;

	CComBSTR name( cchMaxNameChars );
	CComBSTR color( cchMaxColorChars );
	CComBSTR size( cchMaxSizeChars );

	// Do the call
	LRESULT ret = SetErr( pf( name, cchMaxNameChars, color, cchMaxColorChars, size, cchMaxSizeChars ) );

	wcstombs( pszThemeFileName, name, cchMaxNameChars );
	wcstombs( pszColorBuff, color, cchMaxColorChars );
	wcstombs( pszSizeBuff, size, cchMaxSizeChars );

	return ( ret == S_OK );
}

typedef HRESULT (__stdcall *PFNGETTHEMEDOCUMENTATIONPROPERTY)(LPCWSTR pszThemeName, LPCWSTR pszPropertyName,  LPWSTR pszValueBuff, int cchMaxValChars);
BOOL CXpTheme::GetThemeDocumentationProperty(LPCTSTR pszThemeName, LPCTSTR pszPropertyName, LPSTR pszValueBuff, int cchMaxValChars)
{_STT();
	// Get function pointer
	PFNGETTHEMEDOCUMENTATIONPROPERTY pf = (PFNGETTHEMEDOCUMENTATIONPROPERTY)m_ixptf[ ixptf_PFNGETTHEMEDOCUMENTATIONPROPERTY ];
	if ( pf == NULL ) return FALSE;

	CComBSTR valuebuf( cchMaxValChars );

	// Do the call
	LRESULT ret = SetErr( pf( CComBSTR( pszThemeName ), CComBSTR( pszPropertyName ), valuebuf, cchMaxValChars ) );

	wcstombs( pszValueBuff, valuebuf, cchMaxValChars );

	return ( ret == S_OK );
}

typedef HRESULT (__stdcall *PFNDRAWTHEMEPARENTBACKGROUND)(HWND hwnd, HDC hdc,  RECT* prc);
BOOL CXpTheme::DrawThemeParentBackground(HWND hWnd, HDC hDC, LPRECT pRect)
{_STT();
	// Get function pointer
	PFNDRAWTHEMEPARENTBACKGROUND pf = (PFNDRAWTHEMEPARENTBACKGROUND)m_ixptf[ ixptf_PFNDRAWTHEMEPARENTBACKGROUND ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( hWnd, hDC, pRect ) ) == S_OK;
}

typedef HRESULT (__stdcall *PFNENABLETHEMING)(BOOL fEnable);
BOOL CXpTheme::EnableTheming(BOOL bEnable)
{_STT();
	// Get function pointer
	PFNENABLETHEMING pf = (PFNENABLETHEMING)m_ixptf[ ixptf_PFNENABLETHEMING ];
	if ( pf == NULL ) return FALSE;

	// Do the call
	return SetErr( pf( bEnable ) ) == S_OK;
}

// This function exists because sometimes IsAppThemed lies
BOOL CXpTheme::IsWindowThemed(HWND hWnd)
{_STT();
	if ( !IsAppThemed() ) return FALSE;

	// But sometimes IsAppThemed returns TRUE anyway so...
	CXpTheme xpthm( hWnd, "WINDOW" );
	return xpthm.IsTheme();
}
