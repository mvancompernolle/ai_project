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
// Base64.cpp: implementation of the CBase64 class.
//
////////////////////////////////////////////////////////////////////
// Bmp.cpp: implementation of the CBmp class.
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

CBmp::CBmp()
{_STT();
	m_pDib = NULL;
	m_dwSize = 0;

	m_hPal = NULL;

	m_dwErr = 0;
	*m_szErr = 0;

	m_hRgn = NULL;
	ZeroMemory( &m_rectRgn, sizeof( m_rectRgn ) );
	
	m_bChroma = FALSE;
	m_lCPos = 0;
	m_lCNeg = 0;

	m_bTransparent = FALSE;
	m_hMaskDC = NULL;
	m_hMaskBmp = NULL;
	m_hMaskOld = NULL;

	m_hFastDC = NULL;
	m_hFastBmp = NULL;
	m_hFastOld = NULL;
	ZeroMemory( &m_rectFast, sizeof( m_rectFast ) );

	m_rgbTransparent = RGB( 0, 0, 0 );

	m_drawx = 0;
	m_drawy = 0;
}

CBmp::~CBmp()
{_STT();
	Destroy();
	ReleaseFastDraw();
}

void CBmp::Destroy()
{_STT();
	if ( m_pDib != NULL )
	{		 
		delete [] m_pDib;
		m_pDib = NULL;
	} // end if
	m_dwSize = 0;

	if ( m_hPal != NULL )
	{
		::DeleteObject( m_hPal );
		m_hPal = NULL;
	} // end if

	// Lose the region if any
	DestroyRgn();

	ReleaseMask();
}

BOOL CBmp::Load(LPCTSTR pFile)
{_STT();
	CWinFile	file;

	// Lose any old file
	Destroy();

	// Open the new file
	if ( !file.OpenExisting( pFile ) ) 
	{
		SetError( CBMP_ERR_FILEERROR );
		return FALSE;
	} // end if
	if ( file.Size() < sizeof( BITMAPFILEHEADER ) )
	{
		SetError( CBMP_ERR_FILESIZE );
		return FALSE;
	} // end if

	// Attempt to read the bitmap header
	if ( !file.Read( &m_bfh, sizeof( BITMAPFILEHEADER ) ) )
	{
		SetError( CBMP_ERR_FILEREADERROR );
		return FALSE;
	} // end if

	// Make sure it is a bitmap file
	if( m_bfh.bfType != CBMP_DIB_HEADER_MARKER )
	{
		SetError( CBMP_ERR_FILETYPE );
		return FALSE;
	} // end if


	// Allocate memory
	m_pDib = new BYTE[ file.Size() - sizeof( BITMAPFILEHEADER ) + 16 ];
	if ( m_pDib == NULL )
	{
		SetError( CBMP_ERR_MEMORY );
		return FALSE;
	} // end if

	// Read in the data
	if (	!file.Read( m_pDib, file.Size() - sizeof( BITMAPFILEHEADER ), &m_dwSize ) ||
			m_dwSize != ( file.Size() - sizeof( BITMAPFILEHEADER ) ) )
	{
		Destroy();
		SetError( CBMP_ERR_FILEREADERROR );
		return FALSE;
	} // end if

	// We're done with the file
	file.Close();

	// Create the palette
	if ( !CreatePalette() ) 
	{
		Destroy();
		return FALSE;
	} // end if

	// Create mask if needed
	if ( m_bTransparent ) CreateMask();

	return TRUE;	
}

BOOL CBmp::CreatePalette()
{_STT();
	if ( !IsDib() )
	{
		SetError( CBMP_ERR_NOBMP );
		return FALSE;
	} // end if


	LPBITMAPINFO		pBMI		= (LPBITMAPINFO)m_pDib;
	LPBITMAPCOREINFO	pBMC		= (LPBITMAPCOREINFO)m_pDib;
	LPLOGPALETTE		pLogPal		= NULL;
	DWORD				dwNumColors	= GetColorCount( m_pDib );
	BOOL				bWinStyleDIB;

	// Make sure there is a palette
	if ( dwNumColors == 0 ) return TRUE;

	// Allocate some room for the palette
	pLogPal = (LPLOGPALETTE) new char[	sizeof( LOGPALETTE ) + 
										sizeof(PALETTEENTRY) * 
										dwNumColors ];
	if ( pLogPal == NULL )
	{
		SetError( CBMP_ERR_MEMORY );
		return FALSE;
	} // end if
	
	pLogPal->palVersion = CBMP_PALVERSION;
	pLogPal->palNumEntries = (UINT)dwNumColors;
	bWinStyleDIB = CBMP_IS_WIN30_DIB( m_pDib );

	for ( DWORD i = 0; i < dwNumColors; i++ )
	{
		if ( bWinStyleDIB )
		{
			pLogPal->palPalEntry[ i ].peRed   = pBMI->bmiColors[ i ].rgbRed;
			pLogPal->palPalEntry[ i ].peGreen = pBMI->bmiColors[ i ].rgbGreen;
			pLogPal->palPalEntry[ i ].peBlue  = pBMI->bmiColors[ i ].rgbBlue;
		} // end if
		else
		{
			pLogPal->palPalEntry[ i ].peRed   = pBMC->bmciColors[ i ].rgbtRed;
			pLogPal->palPalEntry[ i ].peGreen = pBMC->bmciColors[ i ].rgbtGreen;
			pLogPal->palPalEntry[ i ].peBlue  = pBMC->bmciColors[ i ].rgbtBlue;
		} // end else
		pLogPal->palPalEntry[ i ].peFlags = 0;

	} // end for

	// Attempt to create a palette
	m_hPal = ::CreatePalette( pLogPal );

	// Lose the buffer
	delete [] pLogPal;

	if ( !IsDib() )
	{
		SetError( CBMP_ERR_GDI );
		return FALSE;
	} // end if

	return IsPalette();
}

DWORD CBmp::GetColorCount( LPBYTE buf )
{_STT();
	if ( buf == NULL ) return 0;

	DWORD	wBitCount;
	DWORD	dwClrUsed;

	if ( CBMP_IS_WIN30_DIB( buf ) )
	{
		dwClrUsed = ( (LPBITMAPINFOHEADER)buf )->biClrUsed;
		if( dwClrUsed != 0 ) return (WORD)dwClrUsed;
		wBitCount = ( (LPBITMAPINFOHEADER)buf )->biBitCount;
	} // end if
	else wBitCount = ( (LPBITMAPCOREHEADER)buf )->bcBitCount;

	switch ( wBitCount )
	{
		case 1:  return 2;
		case 4:  return 16;
		case 8:  return 256;
//		case 16:  return 65536;
	}
	
	return 0;
}

BOOL CBmp::Draw(HDC hDC, LPRECT pRect, BOOL bStretch )
{_STT();
	if ( hDC == NULL )
	{
		SetError( CBMP_ERR_INVALIDHDC );
		return FALSE;
	} // end if
	if ( !IsDib() )
	{
		SetError( CBMP_ERR_NOBMP );
		return FALSE;
	} // end if
	
	BOOL		ret = FALSE;
	HPALETTE	oldpalette;
	RECT		rectDIB, rectDC;
	LPSTR		pDIBits = (char*)GetBits();

	if ( pRect == NULL )
	{ 
		pRect = &rectDC;
		GetDibRect( &rectDC );
	} // end if

	// Get the DIB size
	GetDibRect( &rectDIB );

	// Select palette if needed
	if ( m_hPal != NULL ) oldpalette = SelectPalette( hDC, m_hPal, TRUE );

	if ( 	( !m_bTransparent || !IsMask() ) &&
			( !bStretch || ::EqualRect( pRect, &rectDIB ) ) )
	{
		// use decent looking stretch mode for color
		SetStretchBltMode( hDC, HALFTONE );
		SetBrushOrgEx( hDC, 0, 0, NULL );

		pRect->right = pRect->left + ( rectDIB.right - rectDIB.left );
		pRect->bottom = pRect->top + ( rectDIB.bottom - rectDIB.top );
		ret =
			::SetDIBitsToDevice( hDC,	// device context
				pRect->left,			// destination x-origin
				pRect->top,				// destination y-origin
				pRect->right - 
				pRect->left,			// destination width
				pRect->bottom -
				pRect->top,				// destination height
				rectDIB.left,			// source x-origin
				rectDIB.top,			// source y-origin
				0,						// nStartScan
				rectDIB.bottom,			// nNumScans
				pDIBits,				// lpBits
				(LPBITMAPINFO)m_pDib,	// lpBitsInfo
				DIB_RGB_COLORS );		// palette type		
	} // end if
	else
	{
		// use decent looking stretch mode for color
		SetStretchBltMode( hDC, STRETCH_HALFTONE );
		SetBrushOrgEx( hDC, 0, 0, NULL );

		// Regular draw
		if ( !m_bTransparent || !IsMask() )
		{
			ret = 
				::StretchDIBits( hDC,		// device context
					pRect->left,			// destination x-origin
					pRect->top,				// destination y-origin
					pRect->right - 
					pRect->left,			// destination width
					pRect->bottom -
					pRect->top,				// destination height
					rectDIB.left,			// source x-origin
					rectDIB.top,			// source y-origin
					rectDIB.right -
					rectDIB.left,			// source width
					rectDIB.bottom -
					rectDIB.top,			// source height
					pDIBits,				// lpBits
					(LPBITMAPINFO)m_pDib,	// lpBitsInfo
					DIB_RGB_COLORS,			// palette type
					SRCCOPY );				// ROP2 copy mode
		} // end if

		// Transparent draw
		else 
		{
			// Create a mask if we need one
			if ( !IsMask() ) CreateMask( hDC );

			// Draw The Mask Into The Device
			if ( !::StretchBlt( hDC,	pRect->left, pRect->top,
										pRect->right - pRect->left,
										pRect->bottom - pRect->top,
										m_hMaskDC, rectDIB.left, rectDIB.top,
										rectDIB.right - rectDIB.left,
										rectDIB.bottom - rectDIB.top,										
										SRCAND ) ) return FALSE;

			// Draw the image
			ret = 
				::StretchDIBits( hDC,		// device context
					pRect->left,			// destination x-origin
					pRect->top,				// destination y-origin
					pRect->right - 
					pRect->left,			// destination width
					pRect->bottom -
					pRect->top,				// destination height
					rectDIB.left,			// source x-origin
					rectDIB.top,			// source y-origin
					rectDIB.right -
					rectDIB.left,			// source width
					rectDIB.bottom -
					rectDIB.top,			// source height
					pDIBits,				// lpBits
					(LPBITMAPINFO)m_pDib,	// lpBitsInfo
					DIB_RGB_COLORS,			// palette type
					SRCINVERT );			// ROP2 copy mode
		} // end else
	} // end if
	
	// Reselect the old palette
	if ( m_hPal != NULL ) SelectPalette( hDC, oldpalette, TRUE );

	if ( ret == 0 || ret == GDI_ERROR )
	{
		SetError( CBMP_ERR_GDI );
		return FALSE;
	} // end if

	return ret;
}

BOOL CBmp::GetDibRect(LPRECT pRect)
{_STT();
	if ( !IsDib() )
	{
		SetError( CBMP_ERR_NOBMP );
		return FALSE;
	} // end if

	// Get Dib size
	pRect->left	= pRect->top = 0;
	pRect->right	= CBMP_DIBWIDTH( m_pDib );
	pRect->bottom	= CBMP_DIBHEIGHT( m_pDib );

	return TRUE;
}

BOOL CBmp::Save(LPCTSTR pFile)
{_STT();
	if ( !IsDib() )
	{
		SetError( CBMP_ERR_NOBMP );
		return FALSE;
	} // end if

	CWinFile			file;

	// Attempt to open a new file
	if ( !file.OpenNew( pFile ) )
	{
		SetError( CBMP_ERR_FILEERROR );
		return FALSE;
	} // end if

	BITMAPFILEHEADER	bmf;
	LPBITMAPINFOHEADER	pDIB = ( LPBITMAPINFOHEADER )m_pDib;
	DWORD				dwFileSize;

	bmf.bfType = CBMP_DIB_HEADER_MARKER;
	dwFileSize = *( (LPDWORD)m_pDib ) + CBMP_PALETTESIZE( m_pDib );
	if ( !CBMP_ISCOMPRESSED( pDIB ) )
		pDIB->biSizeImage = pDIB->biHeight *
			CBMP_SCANWIDTH( pDIB->biWidth * (DWORD)pDIB->biBitCount );
	dwFileSize += pDIB->biSizeImage;

	bmf.bfSize = dwFileSize + sizeof( BITMAPFILEHEADER );
	bmf.bfReserved1 = 0;
	bmf.bfReserved2 = 0;
	bmf.bfOffBits = (DWORD)sizeof( BITMAPFILEHEADER ) +
						pDIB->biSize + CBMP_PALETTESIZE( m_pDib );

	if ( !file.Write( &bmf, sizeof( BITMAPFILEHEADER ) ) ) 
	{
		SetError( CBMP_ERR_FILEWRITEERROR );
		return FALSE;
	} // end if
	if ( !file.Write( m_pDib, dwFileSize ) )
	{
		SetError( CBMP_ERR_FILEWRITEERROR );
		return FALSE;
	} // end if

	return TRUE;
}

BOOL CBmp::GetBmpBits(LPBYTE *buf, LPDWORD width, LPDWORD height, LPDWORD size, LPDWORD scanwidth)
{_STT();
	// Sanity check
	if ( !IsDib() )
	{
		SetError( CBMP_ERR_NOBMP );
		return FALSE;
	} // end if
	if ( GetBitCount() < 24 )
	{
		SetError( CBMP_ERR_BMPLT24BIT );
		return FALSE;
	} // end if

	// Get a pointer to the data
	if ( buf != NULL ) *buf = GetBits();
	if ( width != NULL ) *width = GetWidth();
	if ( height != NULL ) *height = GetHeight();
	if ( size != NULL ) *size = Size();
	if ( scanwidth != NULL ) *scanwidth = GetScanWidth();

	return TRUE;

}

#define CBMP_SWITCHERR( err ) case CBMP_ERR_##err : return CBMP_ERRSTR_##err;

LPCTSTR CBmp::GetErrorString(DWORD err)
{_STT();
	switch( err )
	{
		CBMP_SWITCHERR( NONE );
		CBMP_SWITCHERR( FILEERROR );
		CBMP_SWITCHERR( FILESIZE );
		CBMP_SWITCHERR( FILEREADERROR );
		CBMP_SWITCHERR( FILETYPE );
		CBMP_SWITCHERR( MEMORY );
		CBMP_SWITCHERR( INVALIDHDC );
		CBMP_SWITCHERR( NOBMP );
		CBMP_SWITCHERR( FILEWRITEERROR );
		CBMP_SWITCHERR( BMPLT24BIT );
		CBMP_SWITCHERR( GDI );

	} // end switch

	return "Unknown error code";
}

HBITMAP CBmp::CreateHBITMAP( HDC hDC )
{_STT();
	if ( !IsDib() )
	{
		SetError( CBMP_ERR_NOBMP );
		return FALSE;
	} // end if

	HPALETTE	hPal = NULL;
	
	// Get a valid DC
	BOOL bReleaseDC = ( hDC == NULL );
	hDC = ( hDC == NULL ) ? GetDC( NULL ) : hDC;
	if ( hDC == NULL ) return FALSE;

	if( m_hPal )
	{
		hPal = SelectPalette( hDC, m_hPal, FALSE );
		RealizePalette( hDC );
	} // end if

	HBITMAP hBmp = 
		CreateDIBitmap( hDC, 
						GetBitmapInfoHeader(),
						CBM_INIT, GetBits(),
						GetBitmapInfo(),
						DIB_RGB_COLORS );

	// Reselect old palette
	if( m_hPal ) SelectPalette( hDC, hPal, FALSE );
	if ( bReleaseDC ) ReleaseDC( NULL, hDC );

	return hBmp;
}


BOOL CBmp::Load(HBITMAP hBmp)
{_STT();
	// Unload previous bitmap
	Destroy();

	// Punt if invalid handle
	if ( hBmp == NULL ) return FALSE;

	// Retrieve the bitmap's color format, width, and height. 
	BITMAP bmp; 
	if ( !GetObject( hBmp, sizeof(BITMAP), (LPSTR)&bmp) ) 
		return FALSE;

	// Create an empty bitmap
	if ( !CreateBitmap( bmp.bmWidth, bmp.bmHeight, 24 ) ) return FALSE;

	// Copy the bits
	HDC	hDC = GetDC( NULL );
	int ret = GetDIBits(	hDC, hBmp, 0, GetHeight(), GetBits(),
							(PBITMAPINFO)m_pDib, DIB_RGB_COLORS );
	if ( hDC != NULL ) ReleaseDC( NULL, hDC );

	if ( ret == 0 )
	{
		Destroy();
		return FALSE;
	} // end if
				
	// Create the palette
	if ( !CreatePalette() ) 
	{
		Destroy();
		return FALSE;
	} // end if

	// Create mask if needed
	if ( m_bTransparent ) CreateMask();

	return TRUE;
}

void CBmp::SetPixel( DWORD x, DWORD y, COLORREF clr )
{_STT();
	// Sanity checks
	if ( !IsDib() ) return;
	if ( x > GetWidth() ) return;
	if ( y > GetHeight() ) return;

	// Get a pointer to the bytes
	LPBYTE buf = GetBits();
	if ( buf == NULL ) return;

	switch( GetBitCount() )
	{
		case 1 :
		break;

		case 4 :
		break;

		case 8 :
		break;

		case 16 :
		break;

		// Set 24 bit pixel color
		case 24 :
		{
			buf += ( ( y * GetScanWidth() ) + x ) * 3;
			buf[ 0 ] = GetRValue( clr );
			buf[ 1 ] = GetGValue( clr );
			buf[ 2 ] = GetBValue( clr );
		}
		break;

	} // end switch
}

COLORREF CBmp::GetPixel(DWORD x, DWORD y)
{_STT();
	// Sanity checks
	if ( !IsDib() ) return RGB( 0, 0, 0 );
	if ( x > GetWidth() ) return RGB( 0, 0, 0 );
	if ( y > GetHeight() ) return RGB( 0, 0, 0 );

	// Get a pointer to the bytes
	LPBYTE buf = GetBits();
	if ( buf == NULL ) return RGB( 0, 0, 0 );

	switch( GetBitCount() )
	{
		case 1 :
		break;

		case 4 :
		break;

		case 8 :
		break;

		case 16 :
		break;

		// Set 24 bit pixel color
		case 24 :
		{
			buf += ( ( y * GetScanWidth() ) + x ) * 3;
			return ( RGB( buf[ 0 ], buf[ 1 ], buf[ 2 ] ) );
		}
		break;

	} // end switch


	return RGB( 0, 0, 0 );
}

BOOL CBmp::CreateBitmap(DWORD width, DWORD height, DWORD bitsperpixel)
{_STT();
	// Lose any old bitmap
	Destroy();

	// Sanity check
	if ( width == 0 || height == 0 || bitsperpixel == 0 ) return FALSE;

	// Check size
	if (	bitsperpixel != 32 &&
			bitsperpixel != 24 &&
			bitsperpixel != 16 &&
			bitsperpixel != 8 ) return FALSE;

	// Calculate scanwidth
	DWORD bytesperpixel = bitsperpixel / 8;
	DWORD scanwidth = width * bytesperpixel; 
	while ( ( scanwidth ) & 0x03 ) scanwidth++;

	// Calculate memory needed for the bitmap
	m_dwSize = sizeof( BITMAPINFOHEADER ) + ( height * scanwidth * bytesperpixel );
	if ( bitsperpixel < 24 ) m_dwSize += sizeof( RGBQUAD ) * ( 1 << bitsperpixel );

	m_pDib = new BYTE[ m_dwSize ]; 
	if ( m_pDib == NULL )
	{
		m_dwSize = 0;
		return FALSE;
	} // end if

	// Zero memory
	ZeroMemory( m_pDib, m_dwSize );

	PBITMAPINFOHEADER	pbih = GetBitmapInfoHeader();

	// Prepare the header
	pbih->biSize = sizeof( BITMAPINFOHEADER );
	pbih->biWidth = width;
	pbih->biHeight = height;
	pbih->biPlanes = 1;
	pbih->biBitCount = UINT( bitsperpixel );
	pbih->biXPelsPerMeter = 0;
	pbih->biYPelsPerMeter = 0;
	if ( bitsperpixel >= 24 ) pbih->biClrUsed = 0;
	else pbih->biClrUsed = ( 1 << bitsperpixel );
	pbih->biCompression = BI_RGB;
	pbih->biSizeImage = ( height * scanwidth * bytesperpixel );
	pbih->biClrImportant = 0;

	return TRUE;
}

BOOL CBmp::GetBmpMetrics(LPBYTE *buf, LPDWORD width, LPDWORD height, LPDWORD size, LPDWORD scanwidth)
{_STT();
	if ( !IsDib() ) return FALSE;

	// Verify bit content
	LPBITMAPINFOHEADER	pbmh = GetBitmapInfoHeader();
	if ( pbmh == NULL ) return FALSE;

	// Get a pointer to the data
	if ( buf != NULL ) *buf = GetBits();
	if ( width != NULL ) *width = GetWidth();
	if ( height != NULL ) *height = GetHeight();
	if ( size != NULL ) *size = Size();
	if ( scanwidth != NULL ) *scanwidth = GetScanWidth();

	return TRUE;
}

#define AVG( v1, v2 )														\
	if (	( v1 > v2 && ( v1 - v2 ) > long( threshold ) ) ||				\
			( v1 < v2 && ( v2 - v1 ) > long( threshold ) ) )				\
	{																		\
		v1 = BYTE( ( long( v1 ) + long( v1 ) + long( v2 ) ) / long( 3 ) );	\
	}

BOOL CBmp::FxSmooth(DWORD strength, DWORD threshold)
{_STT();
	LPBYTE	bits;
	DWORD	width, height, size, scanwidth;

	// Get Bitmap info
	if ( !GetBmpMetrics( &bits, &width, &height, &size, &scanwidth ) ) 
		return FALSE;

	while ( strength-- )
	{

	DWORD i = 0;
	for ( DWORD y = 0; y < width; y++ )
		for ( DWORD x = 0; x < height; x++ )
		{
			i = ( ( ( y * width ) + x ) * 3 ) + ( y * ( scanwidth - width ) );
			if ( i < size )
		{
			COLORREF	*pixel = (COLORREF*)( bits + i );
			
			BYTE r = GetRValue( *pixel );
			BYTE g = GetGValue( *pixel );
			BYTE b = GetBValue( *pixel );

			BYTE		ar, ag, ab;
			COLORREF	*a;

			// Left
			if ( x > 1 ) 
			{
				a = (COLORREF*)( bits + i - 3 );
				ar = GetRValue( *a );
				ag = GetGValue( *a );
				ab = GetBValue( *a );
				AVG( r, ar )
				AVG( g, ag );
				AVG( b, ab )
			} // end if

			// Right
			if ( ( x + 1 ) < width ) 
			{
				a = (COLORREF*)( bits + i + 3 );
				ar = GetRValue( *a );
				ag = GetGValue( *a );
				ab = GetBValue( *a );
				AVG( r, ar )
				AVG( g, ag );
				AVG( b, ab )
			} // end if
			
			// top
			if ( i > scanwidth ) 
			{
				a = (COLORREF*)( bits + i - scanwidth );
				ar = GetRValue( *a );
				ag = GetGValue( *a );
				ab = GetBValue( *a );
				AVG( r, ar )
				AVG( g, ag );
				AVG( b, ab )
			} // end if

			// bottom
			if ( i < ( size - scanwidth ) ) 
			{
				a = (COLORREF*)( bits + i + scanwidth );
				ar = GetRValue( *a );
				ag = GetGValue( *a );
				ab = GetBValue( *a );
				AVG( r, ar )
				AVG( g, ag );
				AVG( b, ab )
			} // end if

			// Round up
//			if ( r < 255 ) r++;
//			if ( g < 255 ) g++;
//			if ( b < 255 ) b++;

			// Write new pixel data
			bits[ i ] = r; bits[ i + 1 ] = g; bits[ i + 2 ] = b;

			// Skip to next pixel
//			i += 3;

	} // end if
		} // end for
	} // end for
	return TRUE;
}

BOOL CBmp::FxBlur(DWORD strength)
{_STT();
	LPBYTE	bits;
	DWORD	width, height, size, scanwidth;

	// Get Bitmap info
	if ( !GetBmpMetrics( &bits, &width, &height, &size, &scanwidth ) ) 
		return FALSE;

	while ( strength-- )
	{

	DWORD i = 0;
	for ( DWORD y = 0; y < width; y++ )
		for ( DWORD x = 0; x < height; x++ )
		{
			i = ( ( ( y * width ) + x ) * 3 ) + ( y * ( scanwidth - width ) );
			if ( i < size )
		{
			COLORREF	*pixel = (COLORREF*)( bits + i );
			
			BYTE r = GetRValue( *pixel );
			BYTE g = GetGValue( *pixel );
			BYTE b = GetBValue( *pixel );

			BYTE		ar, ag, ab;
			COLORREF	*a;

			// Left
			if ( x > 1 ) 
			{
				a = (COLORREF*)( bits + i - 3 );
				ar = GetRValue( *a );
				ag = GetGValue( *a );
				ab = GetBValue( *a );
				r = BYTE( ( long( r ) + long( ar ) ) / 2 );
				g = BYTE( ( long( g ) + long( ag ) ) / 2 );
				b = BYTE( ( long( b ) + long( ab ) ) / 2 );
			} // end if

			// Right
			if ( ( x + 1 ) < width ) 
			{
				a = (COLORREF*)( bits + i + 3 );
				ar = GetRValue( *a );
				ag = GetGValue( *a );
				ab = GetBValue( *a );
				r = BYTE( ( long( r ) + long( ar ) ) / 2 );
				g = BYTE( ( long( g ) + long( ag ) ) / 2 );
				b = BYTE( ( long( b ) + long( ab ) ) / 2 );
			} // end if
			
			// top
			if ( i > scanwidth ) 
			{
				a = (COLORREF*)( bits + i - scanwidth );
				ar = GetRValue( *a );
				ag = GetGValue( *a );
				ab = GetBValue( *a );
				r = BYTE( ( long( r ) + long( ar ) ) / 2 );
				g = BYTE( ( long( g ) + long( ag ) ) / 2 );
				b = BYTE( ( long( b ) + long( ab ) ) / 2 );
			} // end if

			// bottom
			if ( i < ( size - scanwidth ) ) 
			{
				a = (COLORREF*)( bits + i + scanwidth );
				ar = GetRValue( *a );
				ag = GetGValue( *a );
				ab = GetBValue( *a );
				r = BYTE( ( long( r ) + long( ar ) ) / 2 );
				g = BYTE( ( long( g ) + long( ag ) ) / 2 );
				b = BYTE( ( long( b ) + long( ab ) ) / 2 );
			} // end if

			// Round up
			if ( r < 255 ) r++;
			if ( g < 255 ) g++;
			if ( b < 255 ) b++;

			// Write new pixel data
			bits[ i ] = r; bits[ i + 1 ] = g; bits[ i + 2 ] = b;

	} // end if

		} // end for

	} // end while

	return TRUE;
}

BOOL CBmp::FxDither(DWORD strength)
{_STT();
	LPBYTE	bits;
	DWORD	width, height, size, scanwidth;

	// Get Bitmap info
	if ( !GetBmpMetrics( &bits, &width, &height, &size, &scanwidth ) ) 
		return FALSE;

	DWORD i = 0;
	for ( DWORD y = 0; y < width; y++ )
		for ( DWORD x = 0; x < height; x++ )
		{
			i = ( ( ( y * width ) + x ) * 3 ) + ( y * ( scanwidth - width ) );
			if ( i < size )
		{
			COLORREF	*pixel = (COLORREF*)( bits + i );

			BYTE r = GetRValue( *pixel );
			BYTE g = GetGValue( *pixel );
			BYTE b = GetBValue( *pixel );

			if ( ( ( y & 1 ) ^ ( i & 1 ) ) ) 
			{
				if ( r < ( 255 - strength ) ) r += BYTE( strength ); else r = 255;
				if ( g < ( 255 - strength ) ) g += BYTE( strength ); else g = 255;
				if ( b < ( 255 - strength ) ) b += BYTE( strength ); else b = 255;
			}
			else
			{
				if ( r > BYTE( strength ) ) r -= BYTE( strength ); else r = 0;
				if ( g > BYTE( strength ) ) g -= BYTE( strength ); else g = 0;
				if ( b > BYTE( strength ) ) b -= BYTE( strength ); else b = 0;
			} // end else

			// Write new pixel data
			bits[ i ] = r; bits[ i + 1 ] = g; bits[ i + 2 ] = b;

	} // end if
		} // end for

	return TRUE;
}

BOOL CBmp::FxGrayscale(BYTE r, BYTE g, BYTE b)
{_STT();
	LPBYTE	bits;
	DWORD	width, height, size, scanwidth;

	// Get Bitmap info
	if ( !GetBmpMetrics( &bits, &width, &height, &size, &scanwidth ) ) 
		return FALSE;

	DWORD i = 0;
	for ( DWORD y = 0; y < width; y++ )
		for ( DWORD x = 0; x < height; x++ )
		{
			i = ( ( ( y * width ) + x ) * 3 ) + ( y * ( scanwidth - width ) );
			if ( i < size )
		{
			COLORREF	*pixel = (COLORREF*)( bits + i );

			BYTE avg = ( GetRValue( *pixel ) + GetGValue( *pixel ) + GetBValue( *pixel ) ) / 3;

			// Write new pixel data
			if ( r == 0 ) bits[ i ] = avg;
			else bits[ i ] = BYTE( double( avg ) * ( ( double( 255 ) - double( r ) ) / double( 255 ) ) ); 
			if ( g == 0 ) bits[ i + 1 ] = avg;
			else bits[ i + 1 ] = BYTE( double( avg ) * ( ( double( 255 ) - double( g ) ) / double( 255 ) ) );  
			if ( b == 0 ) bits[ i + 2 ] = avg;
			else bits[ i + 2 ] = BYTE( double( avg ) * ( ( double( 255 ) - double( b ) ) / double( 255 ) ) ); 

	} // end if
		} // end for

	return TRUE;
}

BOOL CBmp::LoadFromBuffer(LPBYTE buf, DWORD size)
{_STT();
	// Lose the old bitmap
	Destroy();

	// Sanity check
	if ( buf == NULL || size < sizeof( BITMAPINFO ) ) return FALSE;

	// Allocate memory
	m_pDib = new BYTE[ size + 16 ];
	if ( m_pDib == NULL )
	{
		SetError( CBMP_ERR_MEMORY );
		return FALSE;
	} // end if

	// Read in the data
	memcpy( m_pDib, buf, size );

	// Save the size of the data
	m_dwSize = size;

	// Create the palette
	if ( !CreatePalette() ) 
	{
		Destroy();
		return FALSE;
	} // end if

	// Create mask if needed
	if ( m_bTransparent ) CreateMask();

	return TRUE;	
}

#define MAXRECTS		256
HRGN CBmp::CreateRgnFromBitmap ( COLORREF rgbTransparent, HRGN hRgn, HBITMAP hBmp, LPRECT pRect, DWORD xOffset, DWORD yOffset )
{_STT();
	DWORD		w,h;
	HDC			hTDC = NULL;
	HBITMAP		hTBmp = NULL, hOldBmp;
	LPBYTE		pBits = NULL;

	// Get bitmap information
	if ( hBmp == NULL )
	{
		if ( IsDib() ) return FALSE;
		if ( pRect == NULL ) 
		{
			w = GetWidth();
			h = GetHeight();
		} // end if
		else
		{
			w = pRect->right - pRect->left;
			h = pRect->bottom - pRect->top;
			xOffset = pRect->left;
			yOffset = pRect->right;
		} // end else

	} // end if
	else
	{
		BITMAP		bm;
		if ( GetObject( hBmp, sizeof( bm ), &bm ) == NULL ) 
		{
			ReleaseDC( NULL, hTDC );
			return FALSE;
		} // end if
		w = bm.bmWidth;
		h = bm.bmHeight;
	} // end else

	// Create DC
	hTDC = CreateCompatibleDC( NULL );
	if ( hTDC == NULL ) return FALSE;

	// Prepare the header
	BITMAPINFOHEADER	bih;
	bih.biSize = sizeof( BITMAPINFOHEADER );
	bih.biWidth = w;
	bih.biHeight = h;
	bih.biPlanes = 1;
	bih.biBitCount = UINT( 32 );
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	if ( bih.biBitCount >= 24 ) bih.biClrUsed = 0;
	else bih.biClrUsed = ( 1 << bih.biBitCount );
	bih.biCompression = BI_RGB;
	bih.biSizeImage = 0;
	bih.biClrImportant = 0;

	// Attempt to create the bitmap
	hTBmp = CreateDIBSection( hTDC, (LPBITMAPINFO)&bih, DIB_RGB_COLORS, (void**)&pBits, NULL, 0 );
	if ( hTBmp == NULL || pBits == NULL )
	{
		DeleteDC( hTDC );
		return FALSE;
	} // end if

	// Select the bitmap into the DC
	hOldBmp = (HBITMAP)SelectObject( hTDC, hTBmp );
	
	if ( hBmp == NULL )
	{
		RECT rect;
		SetRect( &rect, 0, 0, w, h );
		Draw( hTDC, &rect, ( pRect != NULL ) );
	} // end if

	else 
	{
		HDC hDC = CreateCompatibleDC( NULL );
		if ( hDC == NULL )
		{
			SelectObject( hTDC, hOldBmp );
			DeleteDC( hTDC );
			DeleteObject( hTBmp );
			return FALSE;
		} // end if

		// Select the bitmap into the DC
		HBITMAP hOld = (HBITMAP)SelectObject( hDC, hBmp );

		// Make a copy of the bitmap
		BitBlt( hTDC, 0, 0, w, h, hDC, 0, 0, SRCCOPY );

		// Lose the DC
		SelectObject( hDC, hOld );
		DeleteDC( hDC );

	} // end else
								
	LPRGNDATA	prd = (LPRGNDATA)( new BYTE[ sizeof( RGNDATA ) + ( MAXRECTS * sizeof( RECT ) ) ] );
	if ( prd == NULL )
	{
		SelectObject( hTDC, hOldBmp );
		DeleteDC( hTDC );
		DeleteObject( hTBmp );
		return FALSE;
	} // end if

	// Initialize structure
	prd->rdh.dwSize = sizeof( RGNDATAHEADER );
	prd->rdh.iType = RDH_RECTANGLES;
	prd->rdh.nCount = 0;
	prd->rdh.nRgnSize = 0;
	SetRect( &prd->rdh.rcBound, 0, 0, 0, 0 );

	// Get scanwidth
	DWORD scanwidth = w << 2;
	while ( scanwidth & 0x03 ) scanwidth++;

	BOOL		working = FALSE;
	LPCOLORREF	pcol;
	LPRECT		prects = (LPRECT)prd->Buffer;

	// Create an empty region if needed
	if ( hRgn == NULL ) hRgn = CreateRectRgn( 0, 0, 0, 0 );
	if ( hRgn == NULL )
	{
		SelectObject( hTDC, hOldBmp );
		DeleteDC( hTDC );
		DeleteObject( hTBmp );
		return FALSE;
	} // end if

	// Add each pixel
	for ( DWORD y = 0; y < h; y++ )
		for ( DWORD x = 0; x < w; x++ )
		{
			// Get pointer to color
			pcol = (LPCOLORREF)&pBits[ ( ( h - y - 1 ) * scanwidth ) + ( x << 2 ) ];

			// Is it transparent?
			if ( *pcol != rgbTransparent ) 
			{
				if ( !working )
				{
					working = TRUE;
					SetRect(	&prects[ prd->rdh.nCount ], 
								x + xOffset, y + yOffset, 
								x + xOffset + 1, y + yOffset + 1 );
				} // end if
				else prects[ prd->rdh.nCount ].right++;

			} // end if
			
			if ( working )
				if ( *pcol == rgbTransparent || x == ( w - 1 ) )
			{
				working = FALSE;
				UnionRect(	&prd->rdh.rcBound,
							&prd->rdh.rcBound,
							&prects[ prd->rdh.nCount ] );
				prd->rdh.nCount++;
			} // end else if

			// Is it time to add the region?
			if (	prd->rdh.nCount >= MAXRECTS ||
					( y == ( h - 1 ) && x == ( w - 1 ) ) )
			{
				if ( working )
				{
					working = FALSE;
					UnionRect(	&prd->rdh.rcBound,
								&prd->rdh.rcBound,
								&prects[ prd->rdh.nCount ] );
					prd->rdh.nCount++;
				} // end else if

				HRGN hTemp = 
					ExtCreateRegion(	NULL, 
										prd->rdh.dwSize + 
										( prd->rdh.nCount * sizeof( RECT ) ),
										prd );
				if ( hTemp != NULL )
				{
					CombineRgn( hRgn, hRgn, hTemp, RGN_OR );
					DeleteObject( hTemp );
				} // end if		
				prd->rdh.nCount = 0;
				SetRect( &prd->rdh.rcBound, 0, 0, 0, 0 );
			} // end if
		} // end for

	// Delete temporary stuff
	SelectObject( hTDC, hOldBmp );
	DeleteDC( hTDC );
	DeleteObject( hTBmp );
	delete [] (LPBYTE)prd;

	return hRgn;

}

void CBmp::DestroyRgn()
{_STT();
	if ( m_hRgn != NULL )
	{
		DeleteObject( m_hRgn );
		m_hRgn = NULL;
	} // end if

}

BOOL CBmp::CutOut(HWND hWnd, LPRECT pRect, BOOL bAddRgn)
{_STT();
	BOOL	bOffScreen = TRUE;

	if ( !IsDib() ) return FALSE;

	RECT rect;
	if ( pRect != NULL ) ::CopyRect( &rect, pRect );
	else ::GetClientRect( hWnd, &rect );

	long sx1 = m_rectRgn.right - m_rectRgn.left;
	long sx2 = pRect->right - pRect->left;
	long sy1 = m_rectRgn.bottom - m_rectRgn.top;
	long sy2 = pRect->bottom - pRect->top;
	long dx = ( sx1 > sx2 ) ? sx1 - sx2 : sx2 - sx1;
	long dy = ( sy1 > sy2 ) ? sy1 - sy2 : sy2 - sy1;
	long ox = ( pRect->left > m_rectRgn.left ) ? pRect->left - m_rectRgn.left : m_rectRgn.left - pRect->left;
	long oy = ( pRect->top > m_rectRgn.top ) ? pRect->top - m_rectRgn.top : m_rectRgn.top - pRect->top;

	if ( m_hRgn == NULL || dx > 2 || dy > 2 )
	{
		// Create region
		if ( !CreateRgn( pRect, RGB( 0, 0, 0 ) ) ) return FALSE;

	} // end if

	// Correct position
	else if ( ox > 2 || oy > 2 )
	{
		// Set the new offset
		::OffsetRgn( m_hRgn, -m_rectRgn.left, -m_rectRgn.top );
		::OffsetRgn( m_hRgn, rect.left, rect.top );

		// Save the new rectangle
		::CopyRect( &m_rectRgn, &rect );

	} // end else if

	if ( m_hRgn == NULL ) return FALSE;

	{ // Set the region
		// Create an empty region
		HRGN	hRgn = CreateRectRgn( 0, 0, 0, 0 );
		if ( hRgn == NULL ) return FALSE;

		::GetWindowRect( hWnd, &rect );
		POINT	pt = { 0, 0};
		::ClientToScreen( hWnd, &pt );

		// Attempt to cut out the defined region
		if ( !bAddRgn ) 
		{
			// Copy the complex region
			if ( CombineRgn( hRgn, m_hRgn, NULL, RGN_COPY ) == ERROR )
			{
				DeleteObject( hRgn );
				return FALSE; 
			} // end if
			::OffsetRgn( hRgn, pt.x - rect.left, pt.y - rect.top );
			SetWindowRgn( hWnd, hRgn, TRUE );
		} // end if
		else
		{
			if ( GetWindowRgn( hWnd, hRgn ) != ERROR )
			{
				HRGN hTemp = CreateRectRgn( 0, 0, 0, 0 );
				if ( CombineRgn( hTemp, m_hRgn, NULL, RGN_COPY ) == ERROR )
				{
					DeleteObject( hTemp );
					DeleteObject( hRgn );
					return FALSE; 
				} // end if
				::OffsetRgn( hTemp, pt.x - rect.left, pt.y - rect.top );
				CombineRgn(	hRgn, hRgn, hTemp, RGN_OR );
				SetWindowRgn( hWnd, hRgn, TRUE );
			} // end if
		} // end else

	} // end Set the region

	return TRUE;
}

BOOL CBmp::CreateRgn(  LPRECT pRect, COLORREF rgbTransparent )
{_STT();
	DestroyRgn();

	if ( m_hFastBmp != NULL )
	{
		SelectObject( m_hFastDC, m_hFastOld );
		
		// Create a region
		m_hRgn = CreateRgnFromBitmap( rgbTransparent, NULL, m_hFastBmp, NULL, pRect->left, pRect->top );

		SelectObject( m_hFastDC, m_hFastBmp );
	} // end if
	else
	{
		// Create an HBITMAP
//		HBITMAP	hBmp = CreateHBITMAP();
//		if ( hBmp == NULL ) return FALSE;
	
		// Create a region
//		m_hRgn = CreateRgnFromBitmap( rgbTransparent, hBmp, NULL, pRect->left, pRect->top );
		m_hRgn = CreateRgnFromBitmap( rgbTransparent, NULL, NULL, pRect );

//		DeleteObject( hBmp );

	} // end else

	if ( m_hRgn == NULL ) return FALSE;

	return TRUE;
}

BOOL CBmp::Copy(CBmp *pBmp, DWORD x, DWORD y )
{_STT();
	if ( pBmp == NULL ) return FALSE;

	// Get width/height
	if ( x == 0 || y == 0 )
	{
		if ( !IsDib() ) return FALSE;

		if ( x == 0 ) x = GetWidth();
		if ( y == 0 ) y = GetHeight();

	} // end if

	// If they are the same size just copy the bits
	if (	IsDib() &&
			x == pBmp->GetWidth() &&
			y == pBmp->GetHeight() )
	{
		// Just copy the bits
		memcpy( GetBits(), 
				pBmp->GetBits(), 
				GetScanWidth() * GetHeight() );
		return TRUE;
	} // end if

	else
	{ // Do a stretch copy

		HDC		hDesktop = GetDC( NULL );
		if ( hDesktop == NULL ) return FALSE;

		HDC		hSrcDC = CreateCompatibleDC( hDesktop );
		HDC		hDstDC = CreateCompatibleDC( hDesktop );
		HBITMAP	hSrcBmp = pBmp->CreateHBITMAP( hDesktop );
		HBITMAP hDstBmp = CreateCompatibleBitmap( hDesktop, x, y );
		ReleaseDC( NULL, hDesktop );

		// Did we get everything?
		if (	hSrcDC == NULL || hDstDC == NULL || 
				hSrcBmp == NULL || hDstBmp == NULL )
		{
			if ( hSrcDC != NULL ) DeleteDC( hSrcDC );
			if ( hDstDC != NULL ) DeleteDC( hDstDC );
			if ( hSrcBmp != NULL ) DeleteObject( hSrcBmp );
			if ( hDstBmp != NULL ) DeleteObject( hDstBmp );
		} // end if

		// Select Bitmaps
		HBITMAP hOldSrc = (HBITMAP)SelectObject( hSrcDC, hSrcBmp );
		HBITMAP hOldDst = (HBITMAP)SelectObject( hDstDC, hDstBmp );

		// use decent looking stretch mode for color
		SetStretchBltMode( hDstDC, HALFTONE );
		SetBrushOrgEx( hDstDC, 0, 0, NULL );
		SetStretchBltMode( hSrcDC, HALFTONE );
		SetBrushOrgEx( hSrcDC, 0, 0,NULL );

		// Copy the bitmap
		StretchBlt( hDstDC, 0, 0, x, y,
					hSrcDC, 0, 0, pBmp->GetWidth(), pBmp->GetHeight(), 
					SRCCOPY );

		// Lose the device contexts
		SelectObject( hSrcDC, hOldSrc );
		DeleteDC( hSrcDC );
		SelectObject( hDstDC, hOldDst );
		DeleteDC( hDstDC );

		// Copy the bitmap
		BOOL bResult = Load( hDstBmp );

		// Lose the bitmaps
		DeleteObject( hSrcBmp );
		DeleteObject( hDstBmp );

		return bResult;
	}

	return FALSE;
}

BOOL CBmp::CreateMask( HDC hDC, HBITMAP hBmp )
{_STT();
	if ( hBmp == NULL && !IsDib() ) return FALSE;

	// Release the previous mask
	ReleaseMask();

	// Make sure we have a valid DC
	BOOL bDesktop = FALSE;
	if ( hDC == NULL ) hDC = GetDC( NULL );

	// ensure a bitmap
	BOOL bCreateBmp = FALSE;
	if ( hBmp == NULL ) 
	{
		bCreateBmp = TRUE;
		hBmp = CreateHBITMAP( hDC );
	} // end if
	if ( hBmp == NULL ) return FALSE;

	// Get bitmap information
	BITMAP	bm;
	if ( !GetObject( hBmp, sizeof( BITMAP ), &bm ) ) return FALSE;

	// Create offscreen DC for mask
	m_hMaskDC = ::CreateCompatibleDC( hDC );
	if ( m_hMaskDC == NULL ) return FALSE;
	m_hMaskBmp = ::CreateCompatibleBitmap( hDC, bm.bmWidth, bm.bmHeight );	
	if ( m_hMaskBmp == NULL ) { ReleaseMask(); return FALSE; }

	{ // Create mask
		BITMAPINFOHEADER bih;

		ZeroMemory( &bih, sizeof( bih ) );
		bih.biSize = sizeof( BITMAPINFOHEADER );
		bih.biWidth = bm.bmWidth;
		bih.biHeight = bm.bmHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;

		// create dib
		LPDWORD pBits; 
		HBITMAP	hDibSection = ::CreateDIBSection( hDC, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (void**)&pBits, NULL, 0 );

		// copy ddb to 32-bit dib
		if ( !::GetDIBits( hDC, hBmp, 0, bm.bmHeight, pBits, (BITMAPINFO*)&bih, DIB_RGB_COLORS) )
		{
			ReleaseMask();
			return FALSE;
		} // end if
	
		// Create mask
		DWORD pixels = bm.bmWidth * bm.bmHeight;
		for ( DWORD i = 0; i < pixels; i++ )
			if ( pBits[ i ] != m_rgbTransparent ) pBits[ i ] = 0; 
			else pBits[ i ] = ~0;
		
		// now create a mono mask in ddb format
		if ( !::SetDIBits( hDC, m_hMaskBmp, 0, bm.bmHeight, pBits, (BITMAPINFO*)&bih, DIB_RGB_COLORS) )
		{
			ReleaseMask();
			return FALSE;
		} // end if

		// Delete the bitmap
		::DeleteObject( hDibSection );

	} // end Create mask

	// Select the mask bitmap into the mask DC
	m_hMaskOld = (HBITMAP)::SelectObject( m_hMaskDC, m_hMaskBmp );

	// Release desktop DC
	if ( bDesktop ) ReleaseDC( NULL, hDC );

	// Release temporary stuff
	if ( bCreateBmp ) DeleteObject( hBmp );

	return TRUE;
}

void CBmp::ReleaseMask()
{_STT();
	if ( m_hMaskDC != NULL )
	{
		if ( m_hMaskOld != NULL ) ::SelectObject( m_hMaskDC, m_hMaskOld );
		DeleteDC( m_hMaskDC );
	} // end if

	if ( m_hMaskBmp != NULL ) 
	{
		::DeleteObject( m_hMaskBmp );
	} // end if

	m_hMaskDC = NULL;
	m_hMaskBmp = NULL;
	m_hMaskOld = NULL;
}

BOOL CBmp::FastDraw(HDC hDC, LPRECT pRect, BOOL bStretch)
{_STT();
	// Sanity checks
//	if ( hDC == NULL || pRect == NULL ) return FALSE;

	RECT rect;
	if ( pRect == NULL )
	{
		pRect = &rect;
		GetDibRect( pRect );		
	} // end if

	// Use only DWORD aligned positions and sizes
//	OffsetRect( pRect, -( pRect->left & 3 ), -( pRect->top & 3 ) );

	// Check size delta
	long cx = m_rectFast.right - m_rectFast.left;
	long cy = m_rectFast.bottom - m_rectFast.top;
	long nx = pRect->right - pRect->left;
	long ny = pRect->bottom - pRect->top;
	long dx = ( cx > nx ) ? cx - nx : nx - cx;
	long dy = ( cy > ny ) ? cy - ny : ny - cy;

	// Check to see if our offscreen copy is acceptable
	// The moving of images can cause a two pixel error
	// in sizing
	if ( m_hFastDC == NULL || ( bStretch && ( dx >= 8 || dy >= 8 ) ) )
	{
		if ( !IsDib() ) return FALSE;

		// Create an offscreen device context
		if ( !CreateFastDraw( hDC, nx, ny ) ) 
			return Draw( hDC, pRect, bStretch );

		// Fill background in solid
		HBRUSH hBrush = CreateSolidBrush( RGB( 0, 0, 0 ) );
		FillRect( m_hFastDC, &m_rectFast, hBrush );
		DeleteObject( hBrush );

		// Draw the bitmap
		if ( !Draw( m_hFastDC, &m_rectFast, TRUE ) )
		{
			ReleaseFastDraw();
			return Draw( hDC, pRect, bStretch );
		} // end if

	} // end if

	// Gen only?
	if ( hDC == NULL ) return TRUE;

	// Do we want transparency?
	if ( m_bTransparent )
	{		
		// Create a mask if we need one
		if ( !IsMask() ) CreateMask( m_hFastDC, m_hFastBmp );

		if ( IsMask() )
		{
			// Draw The Bitmap Into The Device
			if ( !::BitBlt( hDC,	pRect->left, pRect->top,
									pRect->right - pRect->left,
									pRect->bottom - pRect->top,
									m_hMaskDC, 0, 0, SRCAND ) ) return FALSE;

			// Draw The Bitmap Into The Device
			if ( !::BitBlt( hDC,	pRect->left, pRect->top,
									pRect->right - pRect->left,
									pRect->bottom - pRect->top,
									m_hFastDC, 0, 0, SRCINVERT ) ) return FALSE;

			return TRUE;
		} // end if

	} // end if

	// Draw The Bitmap Into The Device
	return ::BitBlt( hDC,	pRect->left, pRect->top,
							pRect->right - pRect->left,
							pRect->bottom - pRect->top,
							m_hFastDC, 0, 0, SRCCOPY );

	return TRUE;
}

void CBmp::ReleaseFastDraw()
{_STT();
	if ( m_hFastDC != NULL )
	{
		if ( m_hFastOld != NULL ) ::SelectObject( m_hFastDC, m_hFastOld );
		DeleteDC( m_hFastDC );
	} // end if

	if ( m_hFastBmp != NULL ) 
	{
		::DeleteObject( m_hFastBmp );
	} // end if

	m_hFastDC = NULL;
	m_hFastBmp = NULL;
	m_hFastOld = NULL;

	ZeroMemory( &m_rectFast, sizeof( m_rectFast ) );

	ReleaseMask();
}

BOOL CBmp::CreateFastDraw(HDC hDC, DWORD x, DWORD y)
{_STT();
	// Release old fast draw stuff
	ReleaseFastDraw();

	// Set rect
	m_rectFast.left = 0;
	m_rectFast.top = 0;
	m_rectFast.right = x;
	m_rectFast.bottom = y;

	// Create offscreen DC
	m_hFastDC = CreateCompatibleDC( hDC );
	if ( m_hFastDC == NULL ) return FALSE;

	// Create bitmap
	m_hFastBmp = CreateCompatibleBitmap( hDC, m_rectFast.right, m_rectFast.bottom );
	if ( m_hFastBmp == NULL )
	{
		DeleteDC( m_hFastDC );
		m_hFastDC = NULL;
		return FALSE;
	} // end if

	// Select bitmap
	m_hFastOld = (HBITMAP)SelectObject( m_hFastDC, m_hFastBmp );

	// Fill in the background with black
	HBRUSH hBrush = CreateSolidBrush( RGB( 0, 0, 0 ) );
	FillRect( m_hFastDC, &m_rectFast, hBrush );
	DeleteObject( hBrush );

	return TRUE;
}

BOOL CBmp::TlFastDraw(HDC hDC, LPRECT pRect, DWORD dwTL, BOOL bStretch)
{_STT();
	// Check for zero translucency
	if ( dwTL == 0 ) return FastDraw( hDC, pRect, bStretch );

	// Sanity checks
	if ( hDC == NULL || pRect == NULL ) return FALSE;

	// Scale Translucency value
	dwTL = ( dwTL * 255 ) / 10000;
	dwTL &= 0xff;
	dwTL++;

	// Check size delta
	long cx = m_rectFast.right - m_rectFast.left;
	long cy = m_rectFast.bottom - m_rectFast.top;
	long nx = pRect->right - pRect->left;
	long ny = pRect->bottom - pRect->top;
	long dx = ( cx > nx ) ? cx - nx : nx - cx;
	long dy = ( cy > ny ) ? cy - ny : ny - cy;

	// Check to see if our offscreen copy is acceptable
	// The moving of images can cause a two pixel error
	// in sizing
	if ( m_hFastDC == NULL || ( bStretch && ( dx >= 8 || dy >= 8 ) ) )
	{
		if ( !IsDib() ) return FALSE;

		// Create an offscreen device context
		if ( !CreateFastDraw( hDC, nx, ny ) ) 
			return Draw( hDC, pRect, bStretch );

		// Fill background in solid
		HBRUSH hBrush = CreateSolidBrush( RGB( 0, 0, 0 ) );
		FillRect( m_hFastDC, &m_rectFast, hBrush );
		DeleteObject( hBrush );

		// Draw the bitmap
		if ( !Draw( m_hFastDC, &m_rectFast, TRUE ) )
		{
			ReleaseFastDraw();
			return Draw( hDC, pRect, bStretch );
		} // end if

	} // end if
	
	// Create a device context
	HDC hDCS = CreateCompatibleDC( hDC );
	if ( hDCS == NULL ) return FALSE;

	// Create a DIB section
	BITMAPINFOHEADER bih;
	ZeroMemory( &bih, sizeof( bih ) );
	bih.biSize = sizeof( BITMAPINFOHEADER );
	bih.biWidth = pRect->right - pRect->left;
	bih.biHeight = pRect->bottom - pRect->top;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	LPBYTE pBits; 
	HBITMAP	hDibSection = ::CreateDIBSection( hDC, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (void**)&pBits, NULL, 0 );

	// Select the dib section
	HBITMAP oldbmp = (HBITMAP)SelectObject( hDCS, hDibSection );
	

	// Draw The Bitmap Into The Device
	::BitBlt( hDCS,	0, 0, 
					pRect->right - pRect->left,
					pRect->bottom - pRect->top,
					hDC, pRect->left, pRect->top, SRCCOPY );

	// Create yet another dib section
	LPBYTE pSrcBits; 
	HBITMAP	hSrcBmp = ::CreateDIBSection( hDC, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (void**)&pSrcBits, NULL, 0 );

	// Copy source data
	SelectObject( m_hFastDC, m_hFastOld );
	::GetDIBits( hDC, m_hFastBmp, 0, pRect->bottom - pRect->top, pSrcBits, (BITMAPINFO*)&bih, DIB_RGB_COLORS);
	SelectObject( m_hFastDC, m_hFastBmp );

	// Do translucent effect
	DWORD width = ( pRect->right - pRect->left ) * 3;
	while ( width & 3 ) width++;
	DWORD pixels = width * ( pRect->bottom - pRect->top );
	DWORD dwInvTL = 256 - dwTL;
	for ( DWORD i = 0; i < pixels; i++ )
	{
		if ( pSrcBits[ i ] )
		{
			pBits[ i ] = UCHAR( ( ( pBits[ i ] * dwTL ) + ( pSrcBits[ i ] * dwInvTL ) ) >> 8 );
		} // end if
		else pBits[ i ] = 0;
	} // end if

	// Lose the source bitmap
	DeleteObject( hSrcBmp );

	// Do we want transparency?
	if ( m_bTransparent )
	{		
		// Create a mask if we need one
		if ( !IsMask() ) CreateMask( m_hFastDC, m_hFastBmp );

		if ( IsMask() )
		{
			// Draw The Bitmap Into The Device
			::BitBlt( hDC,	pRect->left, pRect->top,
							pRect->right - pRect->left,
							pRect->bottom - pRect->top,
							m_hMaskDC, 0, 0, SRCAND );

			// Draw The Bitmap Into The Device
			::BitBlt( hDC,	pRect->left, pRect->top,
							pRect->right - pRect->left,
							pRect->bottom - pRect->top,
							hDCS, 0, 0, SRCINVERT );
		} // end if

	} // end if
	else
	{

		// Draw The Bitmap Into The Device
		::BitBlt( hDC,	pRect->left, pRect->top,
						pRect->right - pRect->left,
						pRect->bottom - pRect->top,
						hDCS, 0, 0, SRCCOPY );
	} // end else

	// Lose the borrowed stuff
	SelectObject( hDCS, oldbmp );
	DeleteObject( hDibSection );
	DeleteDC( hDCS );

	return TRUE;
}

BOOL CBmp::LoadFromFastDraw()
{_STT();
	Destroy();

	// Is there a fast DC
	if ( !IsFastDraw() ) return FALSE;

	SelectObject( m_hFastDC, m_hFastOld );

	// Load from fast bmp
	BOOL ret = Load( m_hFastBmp );

	m_hFastOld = (HBITMAP)SelectObject( m_hFastDC, m_hFastBmp );

	return ret;
}

BOOL CBmp::LineTo(long x, long y, COLORREF rgb)
{_STT();
	// Draw the line
	BOOL bRet = Line( m_drawx, m_drawy, x, y, rgb );

	// Save new position
	m_drawx = x; m_drawy = y;

	// Return status
	return bRet;
}

BOOL CBmp::Line(long x1, long y1, long x2, long y2, COLORREF rgb)
{_STT();
	LPBYTE	bits;
	DWORD	w, h;
	DWORD   size, scanwidth;

	// Get bitmap params
	if ( !GetBmpMetrics( &bits, &w, &h, &size, &scanwidth ) ) return FALSE;

	// Which direction is the line going ?
	long ax = ( x1 < x2 ) ? 1 : -1;
	long ay = ( y1 < y2 ) ? 1 : -1;

	// Get the distance
	DWORD dx = ( x1 < x2 ) ? x2 - x1 : x1 - x2;
	DWORD dy = ( y1 < y2 ) ? y2 - y1 : y1 - y2;

	// When to move
	DWORD mx = 0;
	DWORD my = 0;

	// The line drawing algorithm
	do
	{
		// Set the pixel
		CBMP_SETPIXEL( bits, size, scanwidth, x1, y1, rgb );

		// Update the x		
		if ( x1 != x2 && ( mx += dx ) >= dy )
		{	x1 += ax; 
			mx -= dy;
		} // end if

		// Update the y		
		if ( y1 != y2 && ( my += dy ) >= dx )
		{	y1 += ay;
			my -= dx;
		} // end if

	} while ( x1 != x2 || y1 != y2 );

	return TRUE;
}

BOOL CBmp::Create(LPBITMAPINFO pbi)
{_STT();
	// Lose any old bitmap
	Destroy();

	// Calculate scanwidth
	DWORD bytesperpixel = pbi->bmiHeader.biBitCount / 8;
	DWORD scanwidth = pbi->bmiHeader.biWidth * bytesperpixel; 
	while ( ( scanwidth ) & 0x03 ) scanwidth++;

	// Calculate memory needed for the bitmap
	m_dwSize = sizeof( BITMAPINFOHEADER ) + ( pbi->bmiHeader.biHeight * scanwidth * bytesperpixel );
	if ( pbi->bmiHeader.biBitCount < 24 ) m_dwSize += sizeof( RGBQUAD ) * ( 1 << pbi->bmiHeader.biBitCount );

	m_pDib = new BYTE[ m_dwSize + 1 ]; 
	if ( m_pDib == NULL )
	{
		m_dwSize = 0;
		return FALSE;
	} // end if

	// Zero memory
	ZeroMemory( m_pDib, m_dwSize );

	// Copy the bitmap information
	PBITMAPINFOHEADER	pbih = GetBitmapInfoHeader();
	memcpy( pbih, pbi, sizeof( BITMAPINFOHEADER ) );
	pbih->biSize = sizeof( BITMAPINFOHEADER );

	return TRUE;
}

BOOL CBmp::LoadFromHDC(HDC hDC)
{_STT();
	HBITMAP hTmp = ::CreateBitmap( 1, 1, 1, 1, NULL );
	if ( hTmp == NULL ) return FALSE;

	HBITMAP hBmp = (HBITMAP)::SelectObject( hDC, hTmp );

	BOOL ret = Load( hBmp );

	::SelectObject( hDC, hBmp );
	::DeleteObject( hTmp );

	return ret;
}
