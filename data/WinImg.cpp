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
// WinImg.cpp: implementation of the CWinImg class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#ifdef ENABLE_WINIMAGE

#include "..\CxImage\xImage.h"

#include <setjmp.h>

extern "C" {
 #include "../jpeg/jpeglib.h"
 #include "../jpeg/jerror.h"
}

//==================================================================
// CCxCustomImg
//
/// Custom CxImage class
/**
	This class is completely encapsulated and hidden by CWinImg
*/
//==================================================================
class CCxCustomImg : public CxImage
{
public:

};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWinImg::CWinImg()
{_STT();
	m_pimg = new CCxCustomImg();

	m_lMem = 0;
	m_pMem = NULL;

	m_pEncoderState = NULL;

	m_bTransparent = FALSE;
	m_rgbTransparent = RGB( 0, 0, 0 );

	ZeroMemory( &m_rect, sizeof( m_rect ) );
}

CWinImg::~CWinImg()
{_STT();
	Destroy();

	// Get image object
	if ( m_pimg == NULL ) return;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Destroy image object
	if ( pimg != NULL ) delete pimg;

	// No more object
	m_pimg = NULL;
}

void CWinImg::Destroy()
{
	// Get image object
	if ( m_pimg == NULL ) return;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Lose
	m_filename.destroy();

	// Destroy the image object
	pimg->Destroy();

	// Make it like new
	pimg->CxImage::CxImage( 0 );

	// Release encode memory
	ReleaseEncodeMemory();

	// Release encoder memory
	ReleaseEncodeJpg();
}

LPCTSTR CWinImg::GetLastError()
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return "Invalid Object";
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Return last error string
	return pimg->GetLastError();	
}

DWORD CWinImg::GetFileType(LPCTSTR pFile)
{_STT();
	// Sanity check
	if ( pFile == NULL ) return MAXDWORD;

	DWORD type = MAXDWORD;
	LPCTSTR ext = CWinFile::GetExtension( pFile );
	if ( ext == NULL || *ext == 0 ) ext = pFile;

	if ( 0 );

	// Check for bitmap
#if CXIMAGE_SUPPORT_JPG
	else if ( !strcmpi( ext, "jpg" ) || !strcmpi( ext, "jpeg" ) ) type = CXIMAGE_FORMAT_JPG;
#endif
#if CXIMAGE_SUPPORT_BMP
	else if ( !strcmpi( ext, "bmp" ) || !strcmpi( ext, "dib" ) ) type = CXIMAGE_FORMAT_BMP;
#endif
#if CXIMAGE_SUPPORT_PNG
	else if ( !strcmpi( ext, "png" ) ) type = CXIMAGE_FORMAT_PNG;
#endif
#if CXIMAGE_SUPPORT_WMF
	else if ( !strcmpi( ext, "wmf" ) ) type = CXIMAGE_FORMAT_WMF;
	else if ( !strcmpi( ext, "emf" ) ) type = CXIMAGE_FORMAT_WMF;
#endif
#if CXIMAGE_SUPPORT_GIF
	else if ( !strcmpi( ext, "gif" ) ) type = CXIMAGE_FORMAT_GIF;
#endif
#if CXIMAGE_SUPPORT_MNG
	else if ( !strcmpi( ext, "mng" ) ) type = CXIMAGE_FORMAT_MNG;
#endif
#if CXIMAGE_SUPPORT_ICO
	else if ( !strcmpi( ext, "ico" ) ) type = CXIMAGE_FORMAT_ICO;
#endif
#if CXIMAGE_SUPPORT_TIF
	else if ( !strcmpi( ext, "tif" ) || !strcmpi( ext, "tiff" ) ) type = CXIMAGE_FORMAT_TIF;
#endif
#if CXIMAGE_SUPPORT_TGA
	else if ( !strcmpi( ext, "tga" ) ) type = CXIMAGE_FORMAT_TGA;
#endif
#if CXIMAGE_SUPPORT_PCX
	else if ( !strcmpi( ext, "pcx" ) ) type = CXIMAGE_FORMAT_PCX;
#endif
#if CXIMAGE_SUPPORT_JP2
	else if ( !strcmpi( ext, "jp2" ) ) type = CXIMAGE_FORMAT_JP2;
#endif

	return type;
}

BOOL CWinImg::Load(LPCTSTR pFile, LPCTSTR pType )
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Copy filename
	m_filename.copy( pFile );

	// Get the file type
	DWORD type = MAXDWORD;
	if ( pType != NULL ) type = GetFileType( pType );
	else type = GetFileType( pFile );
	if ( type == MAXDWORD ) return FALSE;

	// Load the file
	BOOL bRet = pimg->Load( pFile, type );

	GetRect();

	return bRet;
}


BOOL CWinImg::Save(LPCTSTR pFile, LPCTSTR pType)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Copy filename
	m_filename.copy( pFile );

	// Get the file type
	DWORD type = MAXDWORD;
	if ( pType != NULL ) type = GetFileType( pType );
	else type = GetFileType( pFile );
	if ( type == MAXDWORD ) return FALSE;

	// Load the file
	return pimg->Save( pFile, type );
}

BOOL CWinImg::Decode(LPBYTE buf, DWORD size, LPCTSTR pType )
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Copy filename
	if ( pType != NULL ) m_filename.copy( pType );
	
	// Get the file type
	DWORD type = MAXDWORD;
	if ( pType != NULL ) type = GetFileType( pType );
	else return FALSE;
	if ( type == MAXDWORD ) return FALSE;

	// Encode the image
	if ( !pimg->Decode( buf, size, type ) )
		return FALSE;

	GetRect();

	return TRUE;
}

BOOL CWinImg::Encode(LPBYTE *buf, LPDWORD size, LPCTSTR pType)
{_STT();
	// Lose old memory
	ReleaseEncodeMemory();

	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Copy filename
	m_filename.copy( pType );

	// Get the file type
	DWORD type = MAXDWORD;
	if ( pType != NULL ) type = GetFileType( pType );
	else return FALSE;
	if ( type == MAXDWORD ) return FALSE;
	
	// Encode the image
	if ( !pimg->Encode( m_pMem, m_lMem, type ) )
		return FALSE;

	// Save data
	if ( buf != NULL ) *buf = m_pMem;
	if ( size != NULL ) *size = (DWORD)m_lMem;

	// Verify memory
	if ( m_pMem == NULL || m_lMem == 0 ) return FALSE;

	return TRUE;
}

void CWinImg::ReleaseEncodeMemory()
{_STT();
	m_lMem = 0;
	if ( m_pMem != NULL )
	{	free( m_pMem );
		m_pMem = NULL;
	} // end if
}

BOOL CWinImg::Draw(HDC hDC, LPRECT pRect, BOOL bStretch)
{_STT();
	// Sanity checks
	if ( hDC == NULL || pRect == NULL ) return FALSE;

	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Stretch?
	if ( bStretch ) 
	{
		// use decent looking stretch mode for color
		int oldsbm = SetStretchBltMode( hDC, HALFTONE );
		SetBrushOrgEx( hDC, 0, 0, NULL );
		
		// Draw the image
		BOOL ret = pimg->Draw( hDC, *pRect );

		// Restore stretch mode
		SetStretchBltMode( hDC, oldsbm );

		return ret;
	} // end if

	// Ironically, it is actually more trouble not to stretch
	RECT rect;
	CopyRect( &rect, pRect );

	// Set true width and height
	rect.right = rect.left + pimg->GetWidth();
	rect.bottom = rect.top + pimg->GetHeight();

	// Do it
	return pimg->Draw( hDC, rect );
}


long CWinImg::GetWidth()
{_STTEX();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->GetWidth();
}

long CWinImg::GetHeight()
{_STTEX();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->GetHeight();
}

BOOL CWinImg::IsValid()
{_STTEX();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->IsValid();
}

BOOL CWinImg::CreateFromHBITMAP(HBITMAP hBmp)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create the image
	pimg->CreateFromHBITMAP( hBmp );

	GetRect();

	return pimg->IsValid();
}

BOOL CWinImg::SetQuality(DWORD q)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create the image
	pimg->SetJpegQuality( (BYTE)q );

	return TRUE;
}

HICON CWinImg::CreateIcon(long w, long h, COLORREF bck, HDC hDC)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return NULL;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;
	
	HICON hIcon = NULL;

	// Get a template DC
	BOOL bReleaseDC = FALSE;
	if ( hDC == NULL ) 
	{	bReleaseDC = TRUE;
		hDC = GetDC( NULL );
	} // end if

	// Create a bitmap
	HBITMAP hBmp = CreateCompatibleBitmap( hDC, w, h );
	if ( hBmp == NULL )
	{	if ( bReleaseDC ) ReleaseDC( NULL, hDC );
		return FALSE;
	} // end if

	// Create the mask
	HBITMAP hMask = CreateCompatibleBitmap( hDC, w, h );
	if ( hMask == NULL ) 
	{	DeleteObject( hBmp );
		if ( bReleaseDC ) ReleaseDC( NULL, hDC );
		return FALSE;
	} // end if

	// Create a DC
	HDC hCDC = CreateCompatibleDC( hDC );
	if ( hCDC == NULL )
	{	DeleteObject( hBmp );
		DeleteObject( hMask );
		if ( bReleaseDC ) ReleaseDC( NULL, hDC );
		return FALSE;
	} // end if

	// Select the bitmap
	HBITMAP oldbmp = (HBITMAP)SelectObject( hCDC, hBmp );

	// Prepare the icon bitmap
	RECT	rect;
	SetRect( &rect, 0, 0, w, h );

	// Fill in the background
	CGrDC::FillSolidRect( hCDC, &rect, bck );

	// Correct aspect ratio for image
	CGrDC::AspectCorrect( &rect, pimg->GetWidth(), pimg->GetHeight() );

	// Draw the metafile into the icon bitmap
	Draw( hCDC, &rect );	

	// Fill in the mask with zeros
	SelectObject( hCDC, hMask );
	FillRect( hCDC, &rect, (HBRUSH)GetStockObject( BLACK_BRUSH ) ); 

	// Lose the DC
	SelectObject( hCDC, oldbmp );
	DeleteDC( hCDC );

	// Fill in icon information
	ICONINFO	ii;
	ii.fIcon = TRUE;
	ii.xHotspot = w / 2;
	ii.yHotspot = h / 2;
	ii.hbmMask = hMask;
	ii.hbmColor = hBmp;

	// Create a new icon
	hIcon = CreateIconIndirect( &ii );

	// Clean up
	DeleteObject( hBmp );
	DeleteObject( hMask );
	if ( bReleaseDC ) ReleaseDC( NULL, hDC );

	return hIcon;
}

BOOL CWinImg::Mirror()
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Mirror();
}

BOOL CWinImg::Flip()
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Flip();
}

BOOL CWinImg::GrayScale()
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->GrayScale();
}

BOOL CWinImg::Negative()
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Negative();
}

BOOL CWinImg::Rotate(float angle, CWinImg *pDst)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	CCxCustomImg *pdst = NULL;
	if ( pDst != NULL ) pdst = (CCxCustomImg*)pDst->m_pimg;

	return pimg->Rotate( angle, pdst );
}

BOOL CWinImg::Resample(long newx, long newy, int fast)
{_STT();
	// Get image object
	if ( !IsValid() ) return FALSE;
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Resample( newx, newy, fast );
}

BOOL CWinImg::Dither(long method)
{_STT();
	// Get image object
	if ( !IsValid() ) return FALSE;
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Dither( method );
}

BOOL CWinImg::Crop(long left, long top, long right, long bottom, CWinImg *pDst)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	CCxCustomImg *pdst = NULL;
	if ( pDst != NULL ) pdst = (CCxCustomImg*)pDst->m_pimg;

	return pimg->Crop( left, top, right, bottom, pdst );
}

BOOL CWinImg::Noise(long level)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Noise( level );
}

BOOL CWinImg::Median(long Ksize)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Median( Ksize );
}

BOOL CWinImg::Gamma(float gamma)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Gamma( gamma );
}

BOOL CWinImg::ShiftRGB(long r, long g, long b)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->ShiftRGB( r, g, b );
}

BOOL CWinImg::Threshold(BYTE level)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Threshold( level );
}

BOOL CWinImg::Colorize(BYTE hue, BYTE sat)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Colorize( hue, sat );
}

BOOL CWinImg::Light(long level, long contrast)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Light( level, contrast );
}


BOOL CWinImg::Erode(long Ksize)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Erode( Ksize );
}

BOOL CWinImg::Dilate(long Ksize)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Dilate( Ksize );
}

long CWinImg::Histogram(long *red, long *green, long *blue, long *gray, long colorspace)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Histogram( red, green, blue, gray, colorspace );
}


BOOL CWinImg::LoadFromResource(LPCTSTR pResource, LPCTSTR pResType, LPCTSTR pImgType, HMODULE hModule)
{_STT();
	// Sanity checks
	if ( pResource == NULL || pResType == NULL || pImgType == NULL )
		return FALSE;

	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	DWORD	size = 0;
	char	*buf = NULL;
	
	// Get resource size
	if ( !CWinFile::LoadResource( pResource, NULL, &size, pResType, hModule ) || size == 0 )
		return FALSE;

	// Allocate memory
	buf = new char[ size + 1 ];
	if ( buf == NULL ) return FALSE;

	// Load resource
	if ( !CWinFile::LoadResource( pResource, (LPBYTE)buf, &size, pResType, hModule ) )
	{	delete [] buf;
		return FALSE;
	} // end if

	// Decode the image
	BOOL bRet = Decode( (LPBYTE)buf, size, pImgType );

	// Release memory
	delete [] buf;

	return bRet;
}

LPBYTE CWinImg::GetBits()
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->GetBits();
}

DWORD CWinImg::GetEffWidth()
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->GetEffWidth();
}

BOOL CWinImg::Create(long w, long h, long bpp)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return ( pimg->Create( w, h, bpp ) != NULL );
}

BOOL CWinImg::Copy(CWinImg *pImg)
{_STT();
	// Ensure image to copy
	if ( pImg == NULL || pImg->m_pimg == NULL ) 
		return FALSE;

	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Copy the thing
	pimg->Copy( *( (CCxCustomImg*)pImg->m_pimg ) );

	return IsValid();
}

BOOL CWinImg::Resample(LPRECT pRect, int fast)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return pimg->Resample( PRW( pRect ), PRH( pRect ), fast );
}

HBITMAP CWinImg::CreateHBITMAP( HDC hDC )
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create bitmap
	return pimg->MakeBitmap( hDC );
}

BOOL CWinImg::IsTransparent()
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create bitmap
	return pimg->IsTransparent();
}

BOOL CWinImg::SetTransColor( RGBQUAD rgb )
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create bitmap
	pimg->SetTransColor( rgb );

	return TRUE;
}

RGBQUAD CWinImg::GetTransColor()
{_STT();
	// Get image object
	if ( m_pimg == NULL ) 
	{	RGBQUAD rgb = { 0, 0, 0, 0 }; return rgb; }
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create bitmap
	return pimg->GetTransColor();
}

DWORD CWinImg::GetBpp()
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return NULL;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create bitmap
	return pimg->GetBpp();
}

BOOL CWinImg::IncreaseBpp(DWORD bpp)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create bitmap
	return pimg->IncreaseBpp( bpp );
}

RGBQUAD CWinImg::GetPixel(long x, long y)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) 
	{	RGBQUAD rgb = { 0, 0, 0, 0 }; return rgb; }
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create bitmap
	return pimg->GetPixelColor( x, y );
}

BOOL CWinImg::SetPixel(long x, long y, RGBQUAD rgb)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create bitmap
	pimg->SetPixelColor( x, y, rgb );

	return TRUE;
}

long CWinImg::GetTransIndex()
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create bitmap
	return pimg->GetTransIndex( );
}

BOOL CWinImg::SetTransIndex(long index)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create bitmap
	pimg->SetTransIndex( index );

	return TRUE;
}

BYTE CWinImg::GetNearestIndex(RGBQUAD rgb)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create bitmap
	return pimg->GetNearestIndex( rgb );
}

BOOL CWinImg::DecreaseBpp(DWORD bpp, bool bErrorDiffusion, RGBQUAD *ppal)
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create bitmap
	return pimg->DecreaseBpp( bpp, bErrorDiffusion, ppal );
}

BOOL CWinImg::FromClipboard( HWND hWnd )
{_STT();
	// Lose current image
	Destroy();

	// Is there an image on the clipboard
	if ( !IsClipboardFormatAvailable( CF_DIB ) )
		return FALSE;

	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Open clipboard
	if ( !::OpenClipboard( hWnd ) ) return FALSE;

	HANDLE hData = GetClipboardData( CF_DIB );
	if ( hData != NULL )

		// Load the image from the handle
		pimg->CreateFromHANDLE( hData );


	// Close the clipboard
	::CloseClipboard();

	// Did we get a valid image
	return pimg->IsValid();
}

BOOL CWinImg::ToClipboard( HWND hWnd )
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Open clipboard
	if ( !::OpenClipboard( hWnd ) ) return FALSE;

	// Lose current clipboard contents
	EmptyClipboard();

	// Get image handle
	HANDLE hData = pimg->CopyToHandle();
	if ( hData != NULL )

		// Set the data to the clipboard
		::SetClipboardData( CF_DIB, hData );
	
	// Close the clipboard
	::CloseClipboard();

	return TRUE;
}

BOOL CWinImg::HasAlpha()
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create bitmap
	return pimg->IsTransparent();
}

BOOL CWinImg::AlphaDelete()
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Create bitmap
	pimg->AlphaDelete();

	return TRUE;
}

BOOL CWinImg::Tile(HDC hDC, LPRECT pRect)
{_STT();
	// Sanity checks
	if ( hDC == NULL || pRect == NULL ) return FALSE;

	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	pimg->Tile( hDC, pRect );

	return TRUE;

}

#ifdef ENABLE_OPENCV

IplImage* CWinImg::InitIplHeader( CWinImg *pImg, IplImage *pIpl )
{_STT();
	ZeroMemory( pIpl, sizeof( IplImage ) );

	// Fill in the structure
	pIpl->nSize = sizeof( IplImage );
	pIpl->nChannels = 3;
	pIpl->depth = IPL_DEPTH_8U;
	pIpl->dataOrder = 0;
	pIpl->origin = 1;
	pIpl->align = 4;
	pIpl->width = pImg->GetWidth();
	pIpl->height = pImg->GetHeight();
	pIpl->imageSize = CWinDc::GetScanWidth( pImg->GetWidth() ) * pImg->GetHeight();
	pIpl->imageData = (char*)pImg->GetBits();
	pIpl->widthStep = CWinDc::GetScanWidth( pImg->GetWidth() );
	pIpl->imageDataOrigin = (char*)pImg->GetBits();

	return pIpl;
}

#endif


typedef class s_tagCxMemFile : public CxMemFile
{public:
	s_tagCxMemFile() : CxMemFile() {};
	s_tagCxMemFile( LPBYTE pBuf, DWORD dwSize ) : CxMemFile() 
	{	Attach( pBuf, dwSize ); }
	virtual void Alloc(DWORD nBytes) { ASSERT( 0 ); }
	void Attach( LPBYTE pBuf, DWORD dwSize )
	{	m_pBuffer = pBuf; m_Edge = dwSize; m_Size = m_Position = 0; m_bFreeOnClose = false; }
	void Detach() { GetBuffer( true ); }
}s_CxMemFile;

DWORD CWinImg::Encode(LPBYTE pBuf, DWORD dwSize, LPCTSTR pType)
{_STT();
	// Sanity checks
	if ( pBuf == NULL || dwSize == 0 ) 
		return 0;

	// Get image object
	if ( m_pimg == NULL ) return FALSE;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	// Use filename if not supplied
	if ( pType == NULL ) pType = m_filename;

	// Get the file type
	DWORD type = MAXDWORD;
	type = GetFileType( pType );
	if ( type == MAXDWORD ) return 0;
	
	// Wrap the memory pointer
	s_CxMemFile mf( pBuf, dwSize );

	// Encode the image
	if ( !pimg->Encode( &mf, type ) )
	{	mf.Detach(); return 0; }

	// Get the number of bytes encoded
	DWORD dwEncoded = mf.Size();

	// Detach from the buffer
	mf.Detach();

	return dwEncoded;
}

DWORD CWinImg::GetImageSize()
{_STT();
	// Get image object
	if ( m_pimg == NULL ) return 0;
	CCxCustomImg	*pimg = (CCxCustomImg*)m_pimg;

	return ( pimg->GetEffWidth() * pimg->GetHeight() );
}

class CJpegErr : public jpeg_error_mgr
{
public:

	CJpegErr() {}

	void Init( jpeg_compress_struct *pJcs )
	{_STT();

		// Error string
		*m_szError = 0;		

		// Error function
		error_exit = JpegErrorExit;
		pJcs->err = jpeg_std_error( this );

		// Set jump address
		setjmp( m_jbErr );
	}

	static void JpegErrorExit( j_common_ptr cinfo )
	{_STT();

		CJpegErr *pJe = (CJpegErr*)cinfo;
		if ( pJe == NULL ) return;

		pJe->format_message( cinfo, pJe->m_szError );

	}

private:

	jmp_buf			m_jbErr;
	char			m_szError[ 256 ];
};

class CJpegDst : public jpeg_destination_mgr
{
public:

	CJpegDst()
	{	next_output_byte = NULL; free_in_buffer = 0;
		init_destination = &InitDestination;
		empty_output_buffer = &EmptyOutputBuffer;
		term_destination = &TermDestination;
	}

	void Init( LPBYTE pBuf, DWORD dwSize )
	{	next_output_byte = pBuf;
		free_in_buffer = dwSize;
	}

	// Buffer functions
	static void InitDestination(j_compress_ptr cinfo) { }
	static boolean EmptyOutputBuffer(j_compress_ptr cinfo) { return true; }
	static void TermDestination(j_compress_ptr cinfo) { }

};

typedef struct tagSJpegEncState
{ 
	struct jpeg_compress_struct		cinfo;
	CJpegErr						jerr;
	CJpegDst						dmgr;

	long							lWidth;
	long							lHeight;

} SJpegEncState;

BOOL CWinImg::InitEncodeJpg( long lWidth, long lHeight, long lQuality )
{
	// Lose old encoder
	ReleaseEncodeJpg();

	// Allocate memory for encoder
	SJpegEncState *pState = new SJpegEncState;
	m_pEncoderState = (void*)pState;

	// Save image size
	pState->lWidth = lWidth;
	pState->lHeight = lHeight;

	// Initialize cinfo structure
	ZeroMemory( &pState->cinfo, sizeof( pState->cinfo ) );

	// Initialize error handling
	pState->jerr.Init( &pState->cinfo );

	// Create compressor
	jpeg_create_compress( &pState->cinfo );

	// Initialize encode structure
	pState->cinfo.dest = &pState->dmgr;
	pState->cinfo.image_width = lWidth;
	pState->cinfo.image_height = lHeight;
	pState->cinfo.input_components = 3;
	pState->cinfo.in_color_space = JCS_RGB;
	pState->cinfo.density_unit = 1;
	pState->cinfo.X_density = 0;
	pState->cinfo.Y_density = 0;

	// Set jpeg defaults
	jpeg_set_defaults( &pState->cinfo );

	// Set jpeg quality
	jpeg_set_quality( &pState->cinfo, lQuality, TRUE );

	return TRUE;
}

void CWinImg::ReleaseEncodeJpg()
{
	if ( m_pEncoderState == NULL ) return;
	SJpegEncState *pState = (SJpegEncState*)m_pEncoderState;
	m_pEncoderState = NULL;	

	// Release compressor resources
	jpeg_destroy_compress( &pState->cinfo );

	delete pState;
}

DWORD CWinImg::EncodeJpg(LPBYTE pSrc, long lWidth, long lHeight, LPBYTE pDst, DWORD dwDst, long lQuality)
{_STT();

	SJpegEncState *pState = (SJpegEncState*)m_pEncoderState;

	// Initialize encoder	
	if ( pState == NULL || lWidth != pState->lWidth || lHeight != pState->lHeight ) 
		if ( !InitEncodeJpg( lWidth, lHeight, lQuality ) ) 
			return FALSE;

	// Get structure
	pState = (SJpegEncState*)m_pEncoderState;

	// Scan width
	long lScanWidth = CWinDc::GetScanWidth( lWidth );;

	pState->dmgr.Init( pDst, dwDst );

	// Initialize compressor
	jpeg_start_compress( &pState->cinfo, TRUE );

	// Compress each line
	LPBYTE pLine = &pSrc[ ( lHeight - 1 ) * lScanWidth ];
	for ( long y = 0; y < lHeight; y++, pLine -= lScanWidth )
	{	SwapRB( pLine, lScanWidth );
		jpeg_write_scanlines( &pState->cinfo, (JSAMPROW*)&pLine, 1 );
	} // end for

	// Complete image
	jpeg_finish_compress( &pState->cinfo );

	// Return number of bytes writen
	return dwDst - pState->dmgr.free_in_buffer;
}


#endif // ENABLE_WINIMAGE
