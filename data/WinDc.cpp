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
// WinDc.cpp: implementation of the CWinDc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define BP( x, y, w ) ( ( y * w ) + x )
#define	BPSW( x, y, w, sw ) ( ( ( y * w ) + x ) * 3 ) + ( y * sw )

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DWORD CWinDc::m_dwMotionFrame = 0;

CWinDc::CWinDc()
{_STT();
	Init();
}

CWinDc::CWinDc( HDC hDC, HBITMAP hOldBmp, BOOL bAutoDetach )
{_STT();
	Init();

	// Attach to DC
	Attach( hDC, hOldBmp, bAutoDetach );
}

CWinDc::CWinDc( HDC hDC, long x, long y, long w, long h)
{_STT();
	Init();
	Create( hDC, x, y, w, h );
}

CWinDc::CWinDc( HDC hDC, LPRECT pRect)
{_STT();
	Init();
	Create( hDC, pRect );
}

void CWinDc::Init()
{_STT();
	m_hBmp = NULL;
	m_hDC = NULL;
	m_hOldBmp = NULL;
	ZeroMemory( &m_rect, sizeof( m_rect ) );

	m_bAutoDetach = FALSE;

	m_mask = NULL;
	m_bTransparent = FALSE;
	m_rgbTransparent = RGB( 0, 0, 0 );

	m_lCNeg = 0;
	m_lCPos = 0;

	m_pBits = NULL;

	m_bAntiAlias = FALSE;
}

CWinDc::~CWinDc()
{_STT();
	Destroy();
}

BOOL CWinDc::Create(HDC hDC, LPRECT pRect)
{_STT();
	// Sanity check
	if ( pRect == NULL ) return FALSE;

	// Create offscreen buffer only if needed
	if ( 	m_hDC != NULL &&
			PRW( pRect ) == RW( m_rect ) &&
			PRH( pRect ) == RH( m_rect ) )
		return TRUE;

	// Get a dc template
	BOOL bNoDC = ( hDC == NULL );
	if ( hDC == NULL ) hDC = GetDC( NULL );

	// Lose the old offscreen buffer
	Destroy();

	// Save size
	CopyRect( &m_rect, pRect );
	OffsetRect( &m_rect, -m_rect.left, -m_rect.top );

	// Create a compatible dc
	m_hDC = CreateCompatibleDC( hDC );
	if ( m_hDC == NULL ) 
	{
		if ( bNoDC ) ReleaseDC( NULL, hDC );
		return FALSE;
	} // end if

	// Create a compatible bitmap
	m_hBmp = CreateCompatibleBitmap( hDC, PRW( pRect ), PRH( pRect ) );

	if ( m_hBmp == NULL )
	{	::DeleteDC( m_hDC );
		m_hDC = NULL;
		if ( bNoDC ) ReleaseDC( NULL, hDC );
		return FALSE;
	} // end if

	// Select the bitmap into the object
	m_hOldBmp = (HBITMAP)::SelectObject( m_hDC, m_hBmp );

	// Release DC if needed
	if ( bNoDC ) ReleaseDC( NULL, hDC );

	return TRUE;
}

BOOL CWinDc::CreateDIBSection(HDC hDC, long w, long h, long bpp, LPBYTE *pBits)
{_STT();
	// Lose last DC
	Destroy();

	// Assume no bits
	if ( pBits != NULL ) *pBits = NULL;

	// Use screen DC if non provided
	BOOL bNoDC = ( hDC == NULL );
	if ( hDC == NULL ) hDC = ::GetDC( NULL );

	// Create a device context
	m_hDC = CreateCompatibleDC( hDC );
	if ( m_hDC == NULL )
	{	if ( bNoDC ) ::ReleaseDC( NULL, hDC );
		return FALSE;
	} // end if

	// Save the size
	SetRect( &m_rect, 0, 0, w, h );

	// Create a DIB section
	BITMAPINFOHEADER bih;
	ZeroMemory( &bih, sizeof( bih ) );
	bih.biSize = sizeof( BITMAPINFOHEADER );
	bih.biWidth = w;
	bih.biHeight = h;
	bih.biPlanes = 1;
	bih.biBitCount = (UINT)bpp;
	bih.biCompression = BI_RGB;

	// Create dib section
//	m_hBmp = ::CreateDIBSection( m_hDC, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (void**)&m_pBits, NULL, 0 );
	m_hBmp = ::CreateDIBSection( hDC, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (void**)&m_pBits, NULL, 0 );

	// Select the dib section
	if ( m_hBmp != NULL ) m_hOldBmp = (HBITMAP)SelectObject( m_hDC, m_hBmp );
	else Destroy();

	// Release DC if needed
	if ( bNoDC ) ::ReleaseDC( NULL, hDC );

	// Does the user want a pointer to the bits
	if ( GetSafeHdc() != NULL && pBits != NULL ) *pBits = m_pBits;

	// Did we get it?
	return ( GetSafeHdc() != NULL );
}


void CWinDc::Destroy()
{_STT();
	// Complete drawing operations
	GdiFlush();

	// Auto detach?
	if ( m_bAutoDetach )
	{	m_bAutoDetach = FALSE;
		Detach(); return;
	}

	// Delete offscreen stuff
	if ( m_hDC != NULL )
	{	::SelectObject( m_hDC, m_hOldBmp );
		::DeleteDC( m_hDC );
		m_hDC = NULL;
	} // end if
	if ( m_hBmp != NULL )
	{	::DeleteObject( m_hBmp );
		m_hBmp = NULL;
	} // end if
	m_hOldBmp = NULL;

	// Zero rectangle
	ZeroMemory( &m_rect, sizeof( m_rect ) );

	// Release the mask
	ReleaseMask();
}

BOOL CWinDc::rDraw(HDC hDC, LPRECT pRect, BOOL bStretch, DWORD dwRop)
{_STT();
	// Copy to screen if needed
	if ( m_hDC == NULL ) return FALSE;

	long tw = pRect->right - pRect->left;
	long th = pRect->bottom - pRect->top;
	long sw = m_rect.right - m_rect.left;
	long sh = m_rect.bottom - m_rect.top;

	if ( !bStretch || ( tw == sw && th == sh ) )
	{
		if ( m_bTransparent )
		{
			// First the mask
			if ( m_mask != NULL && m_mask->GetSafeHdc() != NULL )
				m_mask->Draw( hDC, pRect, bStretch, SRCAND );

			// Draw The Bitmap Into The Device
			if ( !::BitBlt( hDC,	pRect->left, pRect->top,
									PRW( pRect ), PRH( pRect ),
									m_hDC, 0, 0, SRCINVERT ) ) return FALSE;

			return TRUE;

		} // end if

		// Draw the image
		else return ::BitBlt( hDC,	pRect->left, pRect->top, PRW( pRect ), PRH( pRect ),
									m_hDC, 0, 0, dwRop );
	} // end if
	// use decent looking stretch mode for color
	SetStretchBltMode( hDC, HALFTONE );
	SetBrushOrgEx( hDC, 0, 0, NULL );

	// First the mask
	if ( m_bTransparent )
	{
		// Mask first
		if ( m_mask != NULL && m_mask->GetSafeHdc() != NULL )
			m_mask->Draw( hDC, pRect, bStretch, SRCAND );

		// Draw the image
		return StretchBlt(	hDC, pRect->left, pRect->top, PRW( pRect ), PRH( pRect ),
							m_hDC, m_rect.left, m_rect.top, RW( m_rect ), RH( m_rect ),
							SRCINVERT );
	} // end if

	// Draw the image
	return StretchBlt(	hDC, pRect->left, pRect->top, PRW( pRect ), PRH( pRect ),
						m_hDC, m_rect.left, m_rect.top, RW( m_rect ), RH( m_rect ),
						dwRop );
}

BOOL CWinDc::Draw(HDC hDC, LPRECT pRect, BOOL bStretch, DWORD dwRop)
{_STT();
	// Anti-alias draw?
	if ( m_bAntiAlias ) return TlDraw( hDC, pRect, 0, bStretch );

	// Create mask if needed
	if ( m_bTransparent )
	{	if ( m_mask == NULL || m_mask->GetSafeHdc() == NULL ) 
			CreateMask();
	} else ReleaseMask();

	// Do the draw
	return rDraw( hDC, pRect, bStretch, dwRop );
}


BOOL CWinDc::Copy(HDC hDC, LPRECT pRect, BOOL bStretch)
{_STT();
	RECT screen;
	BOOL bDesktop = FALSE;

	if ( hDC == NULL )
	{	
		if ( pRect == NULL )
		{	SetRect(	&screen, 0, 0, 
						GetSystemMetrics( SM_CXFULLSCREEN ),
						GetSystemMetrics( SM_CYFULLSCREEN ) );
			pRect = &screen;
		} // end if

		// Create DC for screen shot if needed
		if ( GetSafeHdc() == NULL ) Create( hDC, &screen );

		// Get the screen
		hDC = GetDC( NULL );
		if ( hDC == NULL ) return FALSE;
		bDesktop = TRUE;
	} // end if

	BOOL ret = FALSE;
	long tw = 0, th = 0;
	if ( pRect != NULL )
	{	tw = pRect->right - pRect->left;
		th = pRect->bottom - pRect->top;
	} // end if
	long sw = m_rect.right - m_rect.left;
	long sh = m_rect.bottom - m_rect.top;

	if ( pRect == NULL )
		// Draw the image
		ret = ::BitBlt( m_hDC,	m_rect.left, m_rect.top,
								RW( m_rect ), RH( m_rect ),
								hDC, 0, 0, SRCCOPY );

	// Draw the image
	else 
	{
		if ( !bStretch || ( tw == sw ) && ( th == sh ) )
		{
			// Draw the image
			ret = ::BitBlt( m_hDC,	0, 0, PRW( pRect ), PRH( pRect ),
									hDC, pRect->left, pRect->top, SRCCOPY );
		} // end if
		else
		{
			SetStretchBltMode( m_hDC, HALFTONE );
			ret = StretchBlt(	m_hDC,	m_rect.left, m_rect.top,
										RW( m_rect ), RH( m_rect ),
								hDC,	pRect->left, pRect->top,
										PRW( pRect ), PRH( pRect ),
								SRCCOPY );
		} // end if

	} // end else

	// Release DC if needed
	if ( bDesktop ) ReleaseDC( NULL, hDC );

	return ret;
}

BOOL CWinDc::Create(HDC hDC, long x, long y, long width, long height)
{_STT();
	RECT rect;
	SetRect( &rect, x, y, x + width, y + height );
	return Create( hDC, &rect );
}


BOOL CWinDc::DetectMotion(LPMOTION pm, HDC hDC, LPRECT pRect)
{_STT();
	// Sanity checks
	if ( pm == NULL || hDC == NULL || pRect == NULL ) return FALSE;

	// Next motion frame
	m_dwMotionFrame++;

	// Calc params
	long grid = pm->grid; if ( grid == 0 ) grid = 16;
	long w = pRect->right - pRect->left;
	long h = pRect->bottom - pRect->top;
	long wm = w / grid;
	long hm = h / grid;

	// How much space for motion data?
	DWORD size = ( wm + 1 ) * ( hm + 1 );

	BOOL bCopy = FALSE;

	// Do we need a new buffer?
	if ( pm->plMotion == NULL || size != pm->dwMotion )
	{
		bCopy = TRUE;
		pm->dwMotionCycles = 0;

		// Lose old memory
		pm->dwMotion = 0;
		if ( pm->plMotion != NULL )
		{	delete [] pm->plMotion;
			pm->plMotion = NULL;
		} // end if
		if ( pm->plDelta != NULL )
		{	delete [] pm->plDelta;
			pm->plDelta = NULL;
		} // end if

		// Allocate new memory
		pm->plMotion = new long[ size ];
		if ( pm->plMotion == NULL ) return FALSE;
		pm->plDelta = new long[ size ];
		if ( pm->plDelta == NULL ) return FALSE;

		// Save data
		pm->dwMotion = size;
		pm->lxMotion = wm;
		pm->lyMotion = hm;

		// Initialize array
		ZeroMemory( pm->plMotion, size * sizeof( long ) );

	} // end if

	if ( bCopy )
	{
		// Must not have zero grid
		if ( pm->grid == 0 ) pm->grid = 10;

		for ( long y = pRect->top + ( pm->grid >> 1 ), gy = 0; y < pRect->bottom; y += pm->grid, gy++ )
			for ( long x = pRect->left + ( pm->grid >> 1 ), gx = 0; x < pRect->right; x += pm->grid, gx++ )
			{
				DWORD m = ( gy * wm ) + gx;
				if ( m < size )
				{
					COLORREF rgb = ::GetPixel( hDC, x, y );
					if ( rgb != CLR_INVALID )
					{
						// Get intensity
						long intensity =	GetRValue( rgb ) + GetGValue( rgb ) + 
											GetBValue( rgb ) + ( pm->plMotion[ m ] * pm->smoothing ) ;
						pm->plMotion[ m ] = ( intensity / ( pm->smoothing + 3 ) );

						pm->plDelta[ m ] = 0;

					} // end if

				} // end if

			} // end for

	} // end if

	else
	{
		DWORD samples = 0;
		float bright = 0;
		float motion = 0; 

		// Reset max delta
		pm->lMaxDelta = 0;

		// Scan for motion
		for ( long y = pRect->top + ( pm->grid >> 1 ), gy = 0; y < pRect->bottom; y += pm->grid, gy++ )
			for ( long x = pRect->left + ( pm->grid >> 1 ), gx = 0; x < pRect->right; x += pm->grid, gx++ )
			{
				DWORD m = ( gy * wm ) + gx;
				if ( m < size )
				{
					samples++;

					COLORREF rgb = ::GetPixel( hDC, x, y );
					if ( rgb != CLR_INVALID )
					{
						// Get intensity
						long intensity =	GetRValue( rgb ) + GetGValue( rgb ) + 
											GetBValue( rgb ) + ( pm->plMotion[ m ] * pm->smoothing );
						intensity /= ( pm->smoothing + 3 );

						// Check for motion
						float dmotion = 0;
						if ( intensity > ( pm->plMotion[ m ] + pm->threshold ) )
							dmotion = (float)( intensity - pm->plMotion[ m ] );
						else if ( pm->plMotion[ m ] > ( intensity + pm->threshold ) )
							dmotion = (float)( pm->plMotion[ m ] - intensity );

						// Update motion
						motion += dmotion;

						// Upate brightness
						bright += (float)( intensity - pm->plMotion[ m ] );

						// Track pixel intensity
						pm->plMotion[ m ] = intensity;

						// Save motion data
						pm->plDelta[ m ] = ( pm->plDelta[ m ] + (long)dmotion ) / 2;

						// Track max delta
						if ( pm->plDelta[ m ] > pm->lMaxDelta ) pm->lMaxDelta = pm->plDelta[ m ];

					} // end if

				} // end if

			} // end for
		
		// Scale
		if ( motion < 0 ) motion = -motion;
		motion /= samples;
		motion = ( motion * 1000 ) / 4;

		if ( bright < 0 ) bright = -bright;
		bright /= samples;
		bright = ( bright * 1000 ) / 8;

		// Attenuate motion
		float scale = bright * 2;
		if ( scale > 1900 ) motion = 0;
		else if ( scale > 100 )
		{	motion = ( motion * ( 1000 - scale ) ) / 1000;
		} // end if

		// Attenuate brightness
		scale = motion * 2;
		if ( scale > 1900 ) bright = 0;
		else if ( scale > 100 )
		{	bright = ( bright * ( 1000 - scale ) ) / 1000;
		} // end if

		// Save motion/brightness detected
		pm->sensebright = (long)bright;
		pm->sensemotion = (long)motion;

		// Count motion cycle
		pm->dwMotionCycles++;

		// Did we go over motion thresholds?
		if (	pm->dwMotionCycles > 10 &&
				( ( pm->thmotion < 1000 && motion > pm->thmotion ) || 
				( pm->thbright < 1000 && bright > pm->thbright ) ) )
		{
			// Motion detected!
			pm->dwMotionFrame = m_dwMotionFrame;

			return TRUE;

		} // end if

	} // end if

	return FALSE;
}


void CWinDc::DefaultMotionStruct(LPMOTION pm)
{_STT();
	ZeroMemory( pm, sizeof( MOTION ) );

	// Scan params
	pm->grid = 10;
	pm->smoothing = 8;
	pm->threshold = 10;

	// Threasholds
	pm->thbright = 500;
	pm->thmotion = 500;
}

void CWinDc::ReleaseMotion(LPMOTION pm)
{_STT();
	if ( pm == NULL ) return;

	if ( pm->plMotion != NULL )
	{	delete [] pm->plMotion;
		pm->plMotion = NULL;
	} // end if

	if ( pm->plDelta != NULL )
	{	delete [] pm->plDelta;
		pm->plDelta = NULL;
	} // end if
}

BOOL CWinDc::DrawMotion(LPMOTION pm, HDC hDC, LPRECT pRect)
{_STT();
	// Sanity check
	if (	pm == NULL || pm->plMotion == NULL ||
			pm->lxMotion == 0 || pm->lyMotion == 0 ) 
		return FALSE;

	CGrPen	pen;
	CGrDC	dc( hDC, FALSE );

	// Calc canvas size
	long w = pRect->right - pRect->left;
	long h = pRect->bottom - pRect->top;

	// Calc grid width and height
	long gw = w / pm->lxMotion;
	long gh = h / pm->lyMotion;

	long thmotion = ( pm->thmotion * 255 ) / 1000;
	if ( thmotion == 0 ) thmotion = -1;

	if ( pm->scantype == MSCANTYPE_GRID )
	{
		DWORD i = 0;
		for ( long y = 0; y < pm->lyMotion; y++ )
			for ( long x = 0; x < pm->lxMotion; x++ )
			{
				DWORD m = ( y * pm->lxMotion ) + x;
				long motion = 0;
				long max = pm->lMaxDelta;
				if ( max < 6 ) max = 6;
				if ( max != 0 )
					motion = ( pm->plDelta[ m ] * 255 ) / max;
				
				if ( motion < 6 ) motion = 6;
				else if ( motion > 255 ) motion = 255;

				if ( motion > thmotion )
				{
					BYTE g = (BYTE)motion;
					if ( g > 128 ) g = ( 255 - g ) >> 1;
					else g += 128;

					COLORREF rgb = RGB( motion, g, 0 );

					long px = ( x * w ) / pm->lxMotion;
					long py = ( y * h ) / pm->lyMotion;
					long px2 = ( ( x + 1 ) * w ) / pm->lxMotion;
					long py2 = ( ( y + 1 ) * h ) / pm->lyMotion;

					// Create correct color pen
					pen.Create( PS_SOLID, 1, rgb );
					dc.SelectObject( pen );

					// Draw box corner
					dc.MoveTo( px2, py );
					dc.LineTo( px, py );
					dc.LineTo( px, py2 );
					dc.LineTo( px2, py2 );
					dc.LineTo( px2, py );

					dc.Restore();

				} // end if

			} // end for

	} // end if

	else
	{
		DWORD i = 0;
		for ( long y = 0; y < pm->lyMotion; y++ )
			for ( long x = 0; x < pm->lxMotion; x++ )
			{
				DWORD m = ( y * pm->lxMotion ) + x;
				long motion = 0;
				long max = pm->lMaxDelta;
				if ( max < 6 ) max = 6;
				if ( max != 0 )
					motion = ( pm->plDelta[ m ] * 255 ) / max;
				
				if ( motion < 0 ) motion = 0;
				else if ( motion > 255 ) motion = 255;

				if ( motion > thmotion )
				{
					BYTE g = (BYTE)motion;
					if ( g > 128 ) g = ( 255 - g ) >> 1;
					else g += 128;

					COLORREF rgb = RGB( motion, g, 0 );

					long px = ( x * w ) / pm->lxMotion;
					long py = ( y * h ) / pm->lyMotion;

					// Create correct color pen
					pen.Create( PS_SOLID, 1, rgb );
					dc.SelectObject( pen );

					long dw = ( motion * 6 ) / 255;

					// Draw box corner
					RECT e;
					SetRect( &e, px - 3, py - 3, px - 3 + dw, py - 3 + dw );
					dc.FillSolidEllipse( &e, rgb, rgb );

					dc.Restore();

				} // end if

			} // end for

	} // end else

	return TRUE;
}


void CWinDc::Attach(HDC hDC, HBITMAP hOldBmp, BOOL bAutoDetach )
{_STT();
	Destroy();

	m_hDC = hDC;
	m_hOldBmp = hOldBmp;
//	m_hOldBmp = (HBITMAP)GetCurrentObject( m_hDC, OBJ_BITMAP );
	m_bAutoDetach = bAutoDetach;
}


void CWinDc::Assume(CWinDc *pDC)
{_STT();
	Destroy();

	if ( pDC == NULL ) return;

	m_hDC = pDC->m_hDC;
	m_hBmp = pDC->m_hBmp;
	m_hOldBmp = pDC->m_hOldBmp;

	CopyRect( &m_rect, &pDC->m_rect );

	m_bAutoDetach = pDC->m_bAutoDetach;

	m_bTransparent = pDC->m_bTransparent;
	m_rgbTransparent = pDC->m_rgbTransparent;
	m_lCPos = pDC->m_lCPos;
	m_lCNeg = pDC->m_lCNeg;
	m_bAntiAlias = pDC->m_bAntiAlias;

	m_pBits = pDC->m_pBits;

	m_mask = pDC->m_mask;

	pDC->Detach( TRUE );
}

DWORD CWinDc::GetScanWidth(DWORD w)
{	return ( w * 3 + 3 ) & ( ~3 );
}

DWORD CWinDc::CalcImageSize(long lWidth, long lHeight)
{	return GetScanWidth( lWidth ) * lHeight;
}

BOOL CWinDc::CreateMask(COLORREF rgb)
{_STT();
	// Set transparent color
	if ( rgb == MAXDWORD ) rgb = m_rgbTransparent;

	// Get width and height
	long w = GetWidth(), h = GetHeight();

	// Lose previous mask
	ReleaseMask();

	// Allocate memory
	m_mask = new CWinDc;
	if ( m_mask == NULL ) return FALSE;

	// Create mask
	LPBYTE pBits; 
	if ( !m_mask->CreateDIBSection( NULL, w, h, 24, &pBits ) || pBits == NULL )
		return FALSE;

	// Create source bitmap the same size
	CWinDc src;
	LPBYTE pSrcBits = GetBits();

	if ( pSrcBits == NULL )
	{
		if ( !src.CreateDIBSection( NULL, w, h, 24, &pSrcBits ) || pSrcBits == NULL )
			return FALSE;

		// Copy the image
		if ( !rDraw( src, src ) ) return FALSE;
	
	} // end if

	// Exact key?
	if ( m_lCPos == 0 && m_lCNeg == 0 )
	{
		// Create mask
		long sw = CWinDc::GetScanWidth( w );
		sw -= ( w * 3 );

		// Get transparent color information
		long r = GetRValue( m_rgbTransparent );
		long g = GetGValue( m_rgbTransparent );
		long b = GetBValue( m_rgbTransparent );

		for ( long y = 0; y < h; y++ )
			for ( long x = 0; x < w; x++ )
		{
			long i = ( ( ( y * w ) + x ) * 3 ) + ( y * sw );

			// Is it the 
			if ( pSrcBits[ i ] != b || pSrcBits[ i + 1 ] != g || pSrcBits[ i + 2 ] != r )
			{	
				pBits[ i ] = 0;
				pBits[ i + 1 ] = 0;
				pBits[ i + 2 ] = 0;

				if ( pSrcBits[ i ] == 0 && pSrcBits[ i + 1 ] == 0 && pSrcBits[ i + 2 ] == 0 )
					pSrcBits[ i ] = 1, pSrcBits[ i + 1 ] = 1, pSrcBits[ i + 2 ] = 1;
			} // end if
			else
			{
				pBits[ i ] = ~0;
				pBits[ i + 1 ] = ~0;
				pBits[ i + 2 ] = ~0;

				pSrcBits[ i ] = 0;
				pSrcBits[ i + 1 ] = 0;
				pSrcBits[ i + 2 ] = 0;
			} // end else

		} // end if
	} // end if
	else
	{
		// Create mask
		long sw = CWinDc::GetScanWidth( w );
		sw -= ( w * 3 );

		// Get transparent color information
		long r = GetRValue( m_rgbTransparent );
		long g = GetGValue( m_rgbTransparent );
		long b = GetBValue( m_rgbTransparent );

		long min = -m_lCNeg;
		long max = m_lCPos;

		for ( long y = 0; y < h; y++ )
			for ( long x = 0; x < w; x++ )
		{
			long i = ( ( ( y * w ) + x ) * 3 ) + ( y * sw );
			
			bool trans = FALSE;

			long red = pSrcBits[ i + 2 ] - r;
			if ( red >= min && red <= max )
			{	long green = pSrcBits[ i + 1 ] - g;
				if ( green >= min && green <= max )
				{	long blue = pSrcBits[ i ] - b;
					if ( blue >= min && blue <= max ) trans = TRUE;
				} // end if
			} // end if

			if ( trans )
			{	pBits[ i ] = ~0;
				pBits[ i + 1 ] = ~0;
				pBits[ i + 2 ] = ~0;

				pSrcBits[ i ] = 0;
				pSrcBits[ i + 1 ] = 0;
				pSrcBits[ i + 2 ] = 0;
			} // end if
			else
			{
				if ( pSrcBits[ i ] == 0 && pSrcBits[ i + 1 ] == 0 && pSrcBits[ i + 2 ] == 0 )
					pSrcBits[ i ] = 1, pSrcBits[ i + 1 ] = 1, pSrcBits[ i + 2 ] = 1;

				pBits[ i ] = 0;
				pBits[ i + 1 ] = 0;
				pBits[ i + 2 ] = 0;
			} // end if

		} // end for

	} // end else

	// Draw modified copy if not DIB section
	if ( GetBits() == NULL ) src.Draw( *this, *this );

	return TRUE;
}

void CWinDc::ReleaseMask()
{_STT();
	if ( m_mask != NULL )
	{	m_mask->Destroy();
		delete m_mask;
		m_mask = NULL;
	} // end if
}

BOOL CWinDc::BitBlt(HDC hDst, LPRECT pDst, LPRECT pSrc = NULL, BOOL bStretch, DWORD dwFlags)
{_STT();
	RECT src, dst;

	if ( m_hDC == NULL ) return FALSE;

	if ( pDst == NULL )
	{	pDst = &dst; 
		SetRect( &dst, 0, 0, GetWidth(), GetHeight() ); 
	} // end if

	if ( pSrc == NULL )
	{	pSrc = &src; 
		SetRect( &src, 0, 0, GetWidth(), GetHeight() ); 
	} // end if

	// To stretch or not to stretch
	if ( bStretch )
		return ::StretchBlt(	hDst, pDst->left, pDst->top, PRW( pDst ), PRH( pDst ),
								m_hDC, pSrc->left, pSrc->top, PRW( pSrc ), PRH( pSrc ), dwFlags );

	else return ::BitBlt(	hDst, pDst->left, pDst->top, PRW( pDst ), PRH( pDst ),
							m_hDC, pSrc->left, pSrc->top, dwFlags );

}

BOOL CWinDc::TlDraw(HDC hDC, LPRECT pRect, DWORD dwTl, BOOL bStretch)
{_STT();
	// Just pass it off to draw
	if ( dwTl == 0 && !m_bAntiAlias ) Draw( hDC, pRect );
	dwTl = ( ( ( dwTl * 255 ) / 10000 ) & 0xff ) + 1;

	// Nothing to do if completly transparent
	if ( dwTl >= 255 ) return TRUE;
	
	// Sanity checks
	if ( hDC == NULL || pRect == NULL ) return FALSE;

	// Get the size
	long w = PRW( pRect );
	long h = PRH( pRect );

	// Create destination dib section
	LPBYTE pBits;
	CWinDc dst;
	if ( !dst.CreateDIBSection( NULL, w, h, 24, &pBits ) || pBits == NULL )
		return FALSE;

	// Copy the destination chunk of the screen
	::BitBlt( dst,	0, 0, 
					pRect->right - pRect->left,
					pRect->bottom - pRect->top,
					hDC, pRect->left, pRect->top, SRCCOPY );

	CWinDc src;
	LPBYTE pSrcBits;
	if ( m_pBits != NULL ) pSrcBits = m_pBits;
	else
	{
		// Create source bitmap the same size
		if ( !src.CreateDIBSection( NULL, w, h, 24, &pSrcBits ) || pSrcBits == NULL )
			return FALSE;

		// Copy the image
		if ( !rDraw( src, src ) ) return FALSE;

	} // end if

	// Scale Translucency value
	DWORD dwInvTl = 256 - dwTl;	
	DWORD aaTl = dwTl + ( ( 256 - dwTl ) >> 1 );
	DWORD aaInvTl = 256 - aaTl;

	// Get scan width
	long sw = CWinDc::GetScanWidth( w );
	sw -= ( w * 3 );

	if ( !m_bTransparent )
	{
		for ( long y = 0; y < h; y++ )
			for ( long x = 0; x < w; x++ )
			{
				long i = ( ( ( y * w ) + x ) * 3 ) + ( y * sw );
		
				pBits[ i ] = UCHAR( ( ( pBits[ i ] * dwTl ) + ( pSrcBits[ i ] * dwInvTl ) ) >> 8 );
				pBits[ i + 1 ] = UCHAR( ( ( pBits[ i + 1 ] * dwTl ) + ( pSrcBits[ i + 1 ] * dwInvTl ) ) >> 8 );
				pBits[ i + 2 ] = UCHAR( ( ( pBits[ i + 2 ] * dwTl ) + ( pSrcBits[ i + 2 ] * dwInvTl ) ) >> 8 );
			
			} // end for
	} // end if
	else if ( m_lCPos == 0 && m_lCNeg == 0 )
	{
		// Get transparent color
		long r = GetRValue( m_rgbTransparent );
		long g = GetGValue( m_rgbTransparent );
		long b = GetBValue( m_rgbTransparent );

		if ( m_bAntiAlias )
		{
			char *arry = new char[ w * h ];
			if ( arry == NULL ) return FALSE;

			long y;
			for ( y = 0; y < h; y++ )
				for ( long x = 0; x < w; x++ )
				{
					long i = ( ( ( y * w ) + x ) * 3 ) + ( y * sw );
			
					if (	pSrcBits[ i ] != b || 
							pSrcBits[ i + 1 ] != g || 
							pSrcBits[ i + 2 ] != r ) arry[ ( y * w ) + x ] = 1;
					else arry[ ( y * w ) + x ] = 0;

				} // end if

			
			for ( y = 0; y < h; y++ )
				for ( long x = 0; x < w; x++ )
				{
					long i = ( ( ( y * w ) + x ) * 3 ) + ( y * sw );

					DWORD aa = 0;
					if ( x ) { if ( !arry[ ( y * w ) + ( x - 1 ) ] ) aa++; }
					if ( x < ( w - 1 ) ) { if ( !arry[ ( y * w ) + ( x + 1 ) ] ) aa++; }
					if ( y ) { if ( !arry[ ( ( y - 1 ) * w ) + x ] ) aa++; }
					if ( y < ( h - 1 ) ) { if ( !arry[ ( ( y + 1 ) * w ) + x ] ) aa++; }
			
					if ( arry[ ( y * w ) + x ] != 0 )
					{
						if ( aa )
						{
							if ( aa < 4 )
							{
								aaTl = dwTl + ( 80 + ( ( 256 - dwTl ) >> ( 4 - aa ) ) );
								aaInvTl = 256 - aaTl;
								
								pBits[ i ] = UCHAR( ( ( pBits[ i ] * aaTl ) + ( pSrcBits[ i ] * aaInvTl ) ) >> 8 );
								pBits[ i + 1 ] = UCHAR( ( ( pBits[ i + 1 ] * aaTl ) + ( pSrcBits[ i + 1 ] * aaInvTl ) ) >> 8 );
								pBits[ i + 2 ] = UCHAR( ( ( pBits[ i + 2 ] * aaTl ) + ( pSrcBits[ i + 2 ] * aaInvTl ) ) >> 8 );
							} // end if
						} // end if
						else
						{	pBits[ i ] = UCHAR( ( ( pBits[ i ] * dwTl ) + ( pSrcBits[ i ] * dwInvTl ) ) >> 8 );
							pBits[ i + 1 ] = UCHAR( ( ( pBits[ i + 1 ] * dwTl ) + ( pSrcBits[ i + 1 ] * dwInvTl ) ) >> 8 );
							pBits[ i + 2 ] = UCHAR( ( ( pBits[ i + 2 ] * dwTl ) + ( pSrcBits[ i + 2 ] * dwInvTl ) ) >> 8 );
						} // end else

					} // end if

				} // end for

			delete [] arry;

		} // end if
		else
		{
			for ( long y = 0; y < h; y++ )
				for ( long x = 0; x < w; x++ )
				{
					long i = ( ( ( y * w ) + x ) * 3 ) + ( y * sw );
			
					if (	pSrcBits[ i ] != b || 
							pSrcBits[ i + 1 ] != g || 
							pSrcBits[ i + 2 ] != r )
					{
						pBits[ i ] = UCHAR( ( ( pBits[ i ] * dwTl ) + ( pSrcBits[ i ] * dwInvTl ) ) >> 8 );
						pBits[ i + 1 ] = UCHAR( ( ( pBits[ i + 1 ] * dwTl ) + ( pSrcBits[ i + 1 ] * dwInvTl ) ) >> 8 );
						pBits[ i + 2 ] = UCHAR( ( ( pBits[ i + 2 ] * dwTl ) + ( pSrcBits[ i + 2 ] * dwInvTl ) ) >> 8 );
					} // end if

				} // end for
		} // end else
	} // end if

	else
	{
		// Get transparent color
		long r = GetRValue( m_rgbTransparent );
		long g = GetGValue( m_rgbTransparent );
		long b = GetBValue( m_rgbTransparent );

		long min = -m_lCNeg;
		long max = m_lCPos;

		if ( m_bAntiAlias )
		{
			char *arry = new char[ w * h ];
			if ( arry == NULL ) return FALSE;

			long y;
			for ( y = 0; y < h; y++ )
				for ( long x = 0; x < w; x++ )
				{
					long i = ( ( ( y * w ) + x ) * 3 ) + ( y * sw );
			
					bool trans = FALSE;

					long red = pSrcBits[ i + 2 ] - r;
					if ( red >= min && red <= max )
					{	long green = pSrcBits[ i + 1 ] - g;
						if ( green >= min && green <= max )
						{	long blue = pSrcBits[ i ] - b;
							if ( blue >= min && blue <= max ) trans = TRUE;
						} // end if
					} // end if

					if ( trans ) arry[ ( y * w ) + x ] = 0;
					else arry[ ( y * w ) + x ] = 1;

				} // end if


			for ( y = 0; y < h; y++ )
				for ( long x = 0; x < w; x++ )
				{
					long i = ( ( ( y * w ) + x ) * 3 ) + ( y * sw );
			
					bool trans = arry[ ( y * w ) + x ] == 0;

					DWORD aa = 0;
					if ( x ) { if ( !arry[ ( y * w ) + ( x - 1 ) ] ) aa++; }
					if ( x < ( w - 1 ) ) { if ( !arry[ ( y * w ) + ( x + 1 ) ] ) aa++; }
					if ( y ) { if ( !arry[ ( ( y - 1 ) * w ) + x ] ) aa++; }
					if ( y < ( h - 1 ) ) { if ( !arry[ ( ( y + 1 ) * w ) + x ] ) aa++; }

					if ( !trans )
					{
						if ( aa )
						{	
							if ( aa < 4 )
							{
								aaTl = dwTl + ( 80 + ( ( 256 - dwTl ) >> ( 4 - aa ) ) );
								aaInvTl = 256 - aaTl;
								
								pBits[ i ] = UCHAR( ( ( pBits[ i ] * aaTl ) + ( pSrcBits[ i ] * aaInvTl ) ) >> 8 );
								pBits[ i + 1 ] = UCHAR( ( ( pBits[ i + 1 ] * aaTl ) + ( pSrcBits[ i + 1 ] * aaInvTl ) ) >> 8 );
								pBits[ i + 2 ] = UCHAR( ( ( pBits[ i + 2 ] * aaTl ) + ( pSrcBits[ i + 2 ] * aaInvTl ) ) >> 8 );
							} // end if
						} // end if
						else
						{
							pBits[ i ] = UCHAR( ( ( pBits[ i ] * dwTl ) + ( pSrcBits[ i ] * dwInvTl ) ) >> 8 );
							pBits[ i + 1 ] = UCHAR( ( ( pBits[ i + 1 ] * dwTl ) + ( pSrcBits[ i + 1 ] * dwInvTl ) ) >> 8 );
							pBits[ i + 2 ] = UCHAR( ( ( pBits[ i + 2 ] * dwTl ) + ( pSrcBits[ i + 2 ] * dwInvTl ) ) >> 8 );
						} // end else
					} // end if
				
				} // end for

			delete [] arry;

		} // end if

		else
		{
			for ( long y = 0; y < h; y++ )
				for ( long x = 0; x < w; x++ )
				{
					long i = ( ( ( y * w ) + x ) * 3 ) + ( y * sw );
			
					bool trans = FALSE;

					long red = pSrcBits[ i + 2 ] - r;
					if ( red >= min && red <= max )
					{	long green = pSrcBits[ i + 1 ] - g;
						if ( green >= min && green <= max )
						{	long blue = pSrcBits[ i ] - b;
							if ( blue >= min && blue <= max ) trans = TRUE;
						} // end if
					} // end if

					if ( !trans )
					{
						pBits[ i ] = UCHAR( ( ( pBits[ i ] * dwTl ) + ( pSrcBits[ i ] * dwInvTl ) ) >> 8 );
						pBits[ i + 1 ] = UCHAR( ( ( pBits[ i + 1 ] * dwTl ) + ( pSrcBits[ i + 1 ] * dwInvTl ) ) >> 8 );
						pBits[ i + 2 ] = UCHAR( ( ( pBits[ i + 2 ] * dwTl ) + ( pSrcBits[ i + 2 ] * dwInvTl ) ) >> 8 );
					} // end if
				
				} // end for
		} // end else

	} // end if

	// Draw The Bitmap Into The Device
	dst.Draw( hDC, pRect, FALSE );

	return TRUE;
}

BOOL CWinDc::CaptureScreen( LPCTSTR pWindow, LPRECT pRect, long w, long h)
{_STT();
	// Get the window
	HDC hDC = NULL;
	HWND hWnd = NULL;
	RECT rect;

	if ( pWindow != NULL && strcmpi( pWindow, "< desktop >" ) )
	{	
		// Get active window if needed
		if ( !strcmpi( pWindow, "< active window >" ) )
			hWnd = ::GetForegroundWindow();

		// Find the window
		else hWnd = ::FindWindow( NULL, pWindow );

		// Is there such a window and is it visible?
		if ( !::IsWindow( hWnd ) || !::IsWindowVisible( hWnd ) )
			return TRUE;

		// Get the window DC
		if ( hWnd != NULL ) hDC = ::GetDC( hWnd );

		// Get the window rect
		if ( hDC != NULL ) { pRect = &rect; GetClientRect( hWnd, pRect ); }

	} // end if	

	// Get screen capture
	BOOL ret = Create( NULL, 0, 0, w, h ) && Copy( hDC, pRect );

	// Release DC if needed
	if ( hDC != NULL && ::IsWindow( hWnd ) )
		ReleaseDC( hWnd, hDC );

	return ret;
}

HICON CWinDc::CreateIcon(long w, long h, COLORREF bck, HDC hDC, HBITMAP hUseMask)
{_STT();
	// Is there a bitmap?
	if ( GetSafeHdc() == NULL ) return NULL;

	// Get bitmap and de-select
	HBITMAP hBmp = GetBitmap();
	if ( hBmp == NULL ) return NULL;
	::SelectObject( m_hDC, m_hOldBmp );

	// Get mask bitmap and de-select
	HBITMAP hMaskBmp = m_mask ? m_mask->GetBitmap() : NULL;
	if ( m_mask && hMaskBmp ) 
		::SelectObject( m_mask->m_hDC, m_mask->m_hOldBmp );

	// Create the icon
	HICON hIcon = CGrDC::CreateIcon(	w, h, GetWidth(), GetHeight(), 0,
										NULL, hBmp, hMaskBmp ); 

	// Clean up
	::SelectObject( m_hDC, hBmp );
	if ( m_mask && hMaskBmp ) 
		::SelectObject( m_mask->m_hDC, hMaskBmp );

	return hIcon;
}

BOOL CWinDc::MonoChrome( COLORREF key )
{_STT();
	// Get the size
	long w = GetWidth();
	long h = GetHeight();

	// Get scan width
	long sw = CWinDc::GetScanWidth( w );
	sw -= ( w * 3 );
	
	CWinDc dc;
	LPBYTE pBits;
	if ( m_pBits != NULL ) pBits = m_pBits;
	else
	{
		// Create source bitmap the same size
		if ( !dc.CreateDIBSection( NULL, w, h, 24, &pBits ) || pBits == NULL )
			return FALSE;

		// Copy the image
		if ( !Draw( dc, dc ) ) return FALSE;

	} // end if

	if ( key != MAXDWORD && key != RGB( 255, 255, 255 ) )
	{
		DWORD grred = GetRValue( key );
		DWORD grgreen = GetGValue( key );
		DWORD grblue = GetBValue( key );

		for ( long y = 0; y < h; y++ )
			for ( long x = 0; x < w; x++ )
			{
				long i = ( ( ( y * w ) + x ) * 3 ) + ( y * sw );
			
				COLORREF rgb = RGB( pBits[ i + 2 ], pBits[ i + 1 ], pBits[ i + 0 ] );
				DWORD gr = (BYTE)CGrDC::GrayScale( rgb );

				pBits[ i + 2 ] = (BYTE)( ( gr * grred ) >> 8 );
				pBits[ i + 1 ] = (BYTE)( ( gr * grgreen ) >> 8 );
				pBits[ i + 0 ] = (BYTE)( ( gr * grblue ) >> 8 );
			
			} // end for

	} // end if

	// Simple grayscale
	else for ( long y = 0; y < h; y++ )
		for ( long x = 0; x < w; x++ )
		{
			long i = ( ( ( y * w ) + x ) * 3 ) + ( y * sw );
		
			COLORREF rgb = RGB( pBits[ i + 2 ], pBits[ i + 1 ], pBits[ i + 0 ] );
			DWORD gr = (BYTE)CGrDC::GrayScale( rgb );

			pBits[ i + 2 ] = (BYTE)gr;
			pBits[ i + 1 ] = (BYTE)gr;
			pBits[ i + 0 ] = (BYTE)gr;
			
		} // end for

	// Draw back if needed
	if ( dc.GetSafeHdc() ) dc.Draw( GetSafeHdc(), GetRect() );

	return TRUE;
}


BOOL CWinDc::DrawState( UINT state, HBRUSH hBrush )
{_STT();
	// Get the size
	long w = GetWidth();
	long h = GetHeight();

	// Get scan width
	long sw = CWinDc::GetScanWidth( w );
	sw -= ( w * 3 );
	
	CWinDc dc;

	// Create source bitmap the same size
	if ( !dc.Create( *this, *this ) ) return FALSE;

	// Unselect the bitmap
	::SelectObject( m_hDC, m_hOldBmp );

	// Draw disabled
	::DrawState(	GetSafeHdc(), hBrush, NULL, 
					(LPARAM)m_hBmp, (WPARAM)0,
					0, 0, w, h, DST_BITMAP | state );

	// Select back into DC
	::SelectObject( m_hDC, m_hBmp );

	// Detach from the dc
	Capture( &dc );

	return TRUE;
}

BOOL CWinDc::Capture(CWinDc *pDc)
{_STT();
	if ( pDc == NULL ) return FALSE;

	// Lose our image
	Destroy();

	m_bTransparent = pDc->m_bTransparent;
	m_rgbTransparent = pDc->m_rgbTransparent;
	m_lCPos = pDc->m_lCPos;
	m_lCNeg = pDc->m_lCNeg;

	m_hDC = pDc->m_hDC;
	m_hBmp = pDc->m_hBmp;
	m_hOldBmp = pDc->m_hOldBmp;
	CopyRect( &m_rect, &pDc->m_rect );

	m_bAutoDetach = pDc->m_bAutoDetach;
	m_pBits = pDc->m_pBits;
	m_mask = pDc->m_mask;

	pDc->m_mask = NULL;
	pDc->Detach();

	return TRUE;
}

#define MAXRGNRECTS		256
HRGN CWinDc::CreateRgn(HRGN hRgn, HDC hDC, LPRECT pRect, COLORREF rgbtrans, long xoff, long yoff, long cpos, long cneg)
{_STT();
	if ( hDC == NULL || pRect == NULL ) return FALSE;

	// Get the size
	long w = GetWidth();
	long h = GetHeight();

	// Get transparent color
	long r = GetRValue( rgbtrans );
	long g = GetGValue( rgbtrans );
	long b = GetBValue( rgbtrans );

	// color range
	long min = -cneg;
	long max = cpos;

	if ( min == 0 ) min = -1;
	if ( max == 0 ) max = 1;

	// Get scan width
	long sw = CWinDc::GetScanWidth( w );
	sw -= ( w * 3 );
	
	LPBYTE  pSrcBits = NULL;
	CWinDc  src;

	if ( m_pBits != NULL ) pSrcBits = m_pBits;
	else
	{
		// Create source bitmap the same size
		if ( !src.CreateDIBSection( NULL, w, h, 24, &pSrcBits ) || pSrcBits == NULL )
			return FALSE;

		// Copy the image
		if ( !rDraw( src, src ) ) return FALSE;

	} // end if

	// Ensure we got the pointer
	if ( pSrcBits == NULL ) return NULL;

	// Allocate rect memory
	LPRGNDATA	prd = (LPRGNDATA)( new BYTE[ sizeof( RGNDATA ) + ( MAXRGNRECTS * sizeof( RECT ) ) ] );
	if ( prd == NULL ) return NULL;

	// Initialize structure
	prd->rdh.dwSize = sizeof( RGNDATAHEADER );
	prd->rdh.iType = RDH_RECTANGLES;
	prd->rdh.nCount = 0;
	prd->rdh.nRgnSize = 0;
	SetRect( &prd->rdh.rcBound, 0, 0, 0, 0 );

	BOOL		working = FALSE;
	LPRECT		prects = (LPRECT)prd->Buffer;

	// Create an empty region if needed
	if ( hRgn == NULL ) hRgn = CreateRectRgn( 0, 0, 0, 0 );
	if ( hRgn == NULL ) { delete [] (LPBYTE)prd; return FALSE; }

	for ( long y = 0; y < h; y++ )
		for ( long x = 0; x < w; x++ )
		{
			long iy = h - y - 1;
			long i = ( ( ( iy * w ) + x ) * 3 ) + ( iy * sw );
	
			bool trans = FALSE;

			long red = pSrcBits[ i + 2 ] - r;
			if ( red >= min && red <= max )
			{	long green = pSrcBits[ i + 1 ] - g;
				if ( green >= min && green <= max )
				{	long blue = pSrcBits[ i ] - b;
					if ( blue >= min && blue <= max ) trans = TRUE;
				} // end if
			} // end if

			if ( !trans )
			{
				if ( !working )
				{
					working = TRUE;
					SetRect(	&prects[ prd->rdh.nCount ], 
								x + xoff, y + yoff, 
								x + xoff + 1, y + yoff + 1 );
				} // end if
				else prects[ prd->rdh.nCount ].right++;

			} // end if

			if ( working && ( !trans || x == ( w - 1 ) ) )
			{
				working = FALSE;
				UnionRect(	&prd->rdh.rcBound,
							&prd->rdh.rcBound,
							&prects[ prd->rdh.nCount ] );
				prd->rdh.nCount++;
			} // end else if

			// Is it time to add the region?
			if (	prd->rdh.nCount >= MAXRGNRECTS ||
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

	// Release rect memory
	delete [] (LPBYTE)prd;

	return hRgn;

}

BOOL CWinDc::SetRgn(HWND hWnd, HWND hParent, LPRECT pRect, BOOL bAddRgn, BOOL bOuter)
{_STT();
	BOOL	bOffScreen = TRUE;

	RECT rect;
	if ( pRect != NULL ) ::CopyRect( &rect, pRect );
	else ::GetClientRect( hWnd, &rect );

	HRGN hRgn = NULL;
	
	// Create HRGN
	if ( pRect != NULL ) 
	{
		if ( bOuter ) hRgn = CreateOuterRgn( NULL, pRect->left, pRect->top );
		else hRgn = CreateRgn( NULL, pRect->left, pRect->top );

	} // end if
	else
	{
		// Offset for client space
		POINT	pt = { 0, 0 };

		::GetWindowRect( hWnd, &rect );
		::ClientToScreen( hWnd, &pt );

		// Offset client space
		pt.x -= rect.left;
		pt.y -= rect.top;

		// Create rgn
		if ( bOuter ) hRgn = CreateOuterRgn( NULL, pt.x, pt.y );
		else hRgn = CreateRgn( NULL, pt.x, pt.y );

	} // end else

	// Ensure we have a region
	if ( hRgn == NULL ) return FALSE;

	// Are we adding?
	if ( bAddRgn )
	{
		// Create temporary region
		HRGN hTemp = CreateRectRgn( 0, 0, 0, 0 );

		int err = ERROR;

		// Get current window region
		if ( GetWindowRgn( hWnd, hTemp ) != ERROR )

			// Combine regions
			err = CombineRgn( hRgn, hTemp, hRgn, RGN_OR );

		// Lose the temporary region
		DeleteObject( hTemp );

		// Ensure there was no error
		if ( err == ERROR ) return FALSE;
		
	} // end else

	// Set the window region
	return ( SetWindowRgn( hWnd, hRgn, TRUE ) != ERROR );
}

BOOL CWinDc::Capture(HWND hWnd, LPRECT pRect)
{_STT();
	// Is there such a window and is it visible?
	if ( !::IsWindow( hWnd ) || !::IsWindowVisible( hWnd ) )
		return TRUE;

	// Get the window DC
	HDC hDC = ::GetDC( hWnd );
	if ( hDC == NULL ) return FALSE;

	// Create if needed
	if ( GetSafeHdc() == NULL )
	{
		// Get the window rect
		RECT rect;
		if ( pRect != NULL ) CopyRect( &rect, pRect );
		else ::GetClientRect( hWnd, &rect );

		Create( NULL, &rect );

	} // end if

	// Use DC rect if none specified
	if ( pRect == NULL ) pRect = GetRect();

	// Get screen capture
	BOOL ret = Copy( hDC, pRect, FALSE );

	// Release DC
	ReleaseDC( hWnd, hDC );

	return ret;
}

BOOL CWinDc::FloodFill(long x, long y, COLORREF fcol, COLORREF ncol)
{_STT();
	// Get the size
	long w = GetWidth();
	long h = GetHeight();
	long sz = w * h;

	// Bitmap is upside down
	y = ( h - y - 1 );

	if ( x < 0 || x >= w ) return FALSE;
	if ( y < 0 || y >= h ) return FALSE;
	if ( fcol == ncol ) return FALSE;

	// Get fill color
	BYTE r = GetRValue( fcol );
	BYTE g = GetGValue( fcol );
	BYTE b = GetBValue( fcol );

	// Get new color color
	BYTE nr = GetRValue( ncol );
	BYTE ng = GetGValue( ncol );
	BYTE nb = GetBValue( ncol );

	// Get scan width
	long sw = CWinDc::GetScanWidth( w );
	sw -= ( w * 3 );
	
	LPBYTE  pSrcBits = NULL;
	CWinDc  src;

	if ( m_pBits != NULL ) pSrcBits = m_pBits;
	else
	{
		// Create source bitmap the same size
		if ( !src.CreateDIBSection( NULL, w, h, 24, &pSrcBits ) || pSrcBits == NULL )
			return FALSE;

		// Copy the image
		if ( !rDraw( src, src ) ) return FALSE;

	} // end if

	// Ensure we got the pointer
	if ( pSrcBits == NULL ) return NULL;

	// Ensure starting pixel is fill color
	long ci = BPSW( x, y, w, sw );
	if ( pSrcBits[ ci ] != b || pSrcBits[ ci + 1 ] != g || pSrcBits[ ci + 2 ] != r )
		return FALSE;

	// Allocate memory
	LPBYTE buf = new BYTE[ sz ];
	if ( buf == NULL ) return FALSE;
	ZeroMemory( buf, sz );

	long i = BP( x, y, w );
	long isw = BPSW( x, y, w, sw );
	while ( ( buf[ i ] & 0x0f ) <= 3 )
	{
		if ( ( buf[ i ] & 0x0f ) == 0 )
		{
			// Change color of this pixel
			pSrcBits[ isw ] = nb;
			pSrcBits[ isw + 1 ] = ng;
			pSrcBits[ isw + 2 ] = nr;

			// Point to next direction
			buf[ i ] &= 0xf0, buf[ i ] |= 1;
	
			// Can we go up?
			if ( y < ( h - 1 ) )
			{
				long n = BPSW( x, ( y + 1 ), w, sw );
				if ( pSrcBits[ n ] == b && pSrcBits[ n + 1 ] == g && pSrcBits[ n + 2 ] == r ) 
				{	y++;
					i = BP( x, y, w );
					buf[ i ] = 0x10;
					isw = n;
				} // end if
			} // end if

		} // end if

		if ( ( buf[ i ] & 0x0f ) == 1 )
		{
			// Point to next direction
			buf[ i ] &= 0xf0, buf[ i ] |= 2;

			// Can we go right?
			if ( x < ( w - 1 ) )
			{
				long n = BPSW( ( x + 1 ), y, w, sw );
				if ( pSrcBits[ n ] == b && pSrcBits[ n + 1 ] == g && pSrcBits[ n + 2 ] == r ) 
				{	x++;
					i = BP( x, y, w );
					buf[ i ] = 0x20;
					isw = n;
				} // end if
			} // end if

		} // end if

		if ( ( buf[ i ] & 0x0f ) == 2 )
		{
			// Point to next direction
			buf[ i ] &= 0xf0, buf[ i ] |= 3;
		
			// Can we go down?
			if ( y > 0 )
			{
				long n = BPSW( x, ( y - 1 ), w, sw );
				if ( pSrcBits[ n ] == b && pSrcBits[ n + 1 ] == g && pSrcBits[ n + 2 ] == r ) 
				{	y--;
					i = BP( x, y, w );
					buf[ i ] = 0x30;
					isw = n;
				} // end if
			} // end if

		} // end if

		if ( ( buf[ i ] & 0x0f ) == 3 )
		{
			// Point to next
			buf[ i ] &= 0xf0, buf[ i ] |= 4;

			// Can we go left
			if ( x > 0 )
			{
				long n = BPSW( ( x - 1 ), y, w, sw );
				if ( pSrcBits[ n ] == b && pSrcBits[ n + 1 ] == g && pSrcBits[ n + 2 ] == r ) 
				{	x--; 
					i = BP( x, y, w );
					buf[ i ] = 0x40;
					isw = n;
				} // end if
			} // end if

		} // end if

		// Time to backup?
		while ( ( buf[ i ] & 0xf0 ) > 0 && ( buf[ i ] & 0x0f ) > 3 )
		{
			// Go back
			if ( ( buf[ i ] & 0xf0 ) == 0x10 ) y--;
			else if ( ( buf[ i ] & 0xf0 ) == 0x20 ) x--;
			else if ( ( buf[ i ] & 0xf0 ) == 0x30 ) y++;
			else if ( ( buf[ i ] & 0xf0 ) == 0x40 ) x++;

			i = BP( x, y, w );
			isw = BPSW( x, y, w, sw );

		} // end while

	} // end if

	delete [] buf;

	// Draw back if needed
	if ( src.GetSafeHdc() != NULL ) src.Draw( GetSafeHdc(), GetRect() );

	return TRUE;
}

BOOL CWinDc::MapFloodFill(LPBYTE map, long x, long y, long w, long h, BYTE fc, BYTE nc)
{_STT();
	// Get the size
	long sz = w * h;
	if ( sz == 0 ) return FALSE;

	// Sanity checks
	if ( map == NULL ) return FALSE;
	if ( x < 0 || x >= w ) return FALSE;
	if ( y < 0 || y >= h ) return FALSE;
	if ( fc == nc ) return FALSE;

	// Ensure starting pixel is fill color
	long i = BP( x, y, w );
	if ( map[ i ] != fc ) return FALSE;

	// Allocate memory
	LPBYTE buf = new BYTE[ sz ];
	if ( buf == NULL ) return FALSE;
	ZeroMemory( buf, sz );

	while ( ( buf[ i ] & 0x0f ) <= 3 )
	{
		if ( ( buf[ i ] & 0x0f ) == 0 )
		{
			// Change the map value
			map[ i ] = nc;

			// Point to next direction
			buf[ i ] &= 0xf0, buf[ i ] |= 1;
	
			// Can we go up?
			if ( y < ( h - 1 ) )
			{	long n = BP( x, ( y + 1 ), w );
				if ( map[ n ] == fc ) 
				{	y++; i = BP( x, y, w ); buf[ i ] = 0x10; }
			} // end if

		} // end if

		if ( ( buf[ i ] & 0x0f ) == 1 )
		{
			// Point to next direction
			buf[ i ] &= 0xf0, buf[ i ] |= 2;

			// Can we go right?
			if ( x < ( w - 1 ) )
			{	long n = BP( ( x + 1 ), y, w );
				if ( map[ n ] == fc ) 
				{	x++; i = BP( x, y, w ); buf[ i ] = 0x20; }
			} // end if

		} // end if

		if ( ( buf[ i ] & 0x0f ) == 2 )
		{
			// Point to next direction
			buf[ i ] &= 0xf0, buf[ i ] |= 3;
		
			// Can we go down?
			if ( y > 0 )
			{	long n = BP( x, ( y - 1 ), w );
				if ( map[ n ] == fc ) 
				{	y--; i = BP( x, y, w ); buf[ i ] = 0x30; }
			} // end if

		} // end if

		if ( ( buf[ i ] & 0x0f ) == 3 )
		{
			// Point to next
			buf[ i ] &= 0xf0, buf[ i ] |= 4;

			// Can we go left
			if ( x > 0 )
			{	long n = BP( ( x - 1 ), y, w );
				if ( map[ n ] == fc ) 
				{	x--; i = BP( x, y, w ); buf[ i ] = 0x40; }
			} // end if

		} // end if

		// Time to backup?
		while ( ( buf[ i ] & 0xf0 ) > 0 && ( buf[ i ] & 0x0f ) > 3 )
		{
			// Go back
			if ( ( buf[ i ] & 0xf0 ) == 0x10 ) y--;
			else if ( ( buf[ i ] & 0xf0 ) == 0x20 ) x--;
			else if ( ( buf[ i ] & 0xf0 ) == 0x30 ) y++;
			else if ( ( buf[ i ] & 0xf0 ) == 0x40 ) x++;

			i = BP( x, y, w );

		} // end if

	} // end if

	delete [] buf;

	return TRUE;
}

HRGN CWinDc::CreateOuterRgn(HRGN hRgn, HDC hDC, LPRECT pRect, COLORREF rgbtrans, long xoff, long yoff, long cpos, long cneg)
{_STT();
	if ( hDC == NULL || pRect == NULL ) return FALSE;

	// Get the size
	long w = GetWidth();
	long h = GetHeight();

	// Get transparent color
	long r = GetRValue( rgbtrans );
	long g = GetGValue( rgbtrans );
	long b = GetBValue( rgbtrans );

	// color range
	long min = -cneg;
	long max = cpos;

	// Get scan width
	long sw = CWinDc::GetScanWidth( w );
	sw -= ( w * 3 );
	
	LPBYTE  pSrcBits = NULL;
	CWinDc  src;

	if ( m_pBits != NULL ) pSrcBits = m_pBits;
	else
	{
		// Create source bitmap the same size
		if ( !src.CreateDIBSection( NULL, w, h, 24, &pSrcBits ) || pSrcBits == NULL )
			return FALSE;

		// Copy the image
		if ( !rDraw( src, src ) ) return FALSE;

	} // end if

	// Ensure we got the pointer
	if ( pSrcBits == NULL ) return NULL;

	// Allocate rect memory
	LPRGNDATA	prd = (LPRGNDATA)( new BYTE[ sizeof( RGNDATA ) + ( MAXRGNRECTS * sizeof( RECT ) ) ] );
	if ( prd == NULL ) return NULL;

	// Initialize structure
	prd->rdh.dwSize = sizeof( RGNDATAHEADER );
	prd->rdh.iType = RDH_RECTANGLES;
	prd->rdh.nCount = 0;
	prd->rdh.nRgnSize = 0;
	SetRect( &prd->rdh.rcBound, 0, 0, 0, 0 );

	BOOL		working = FALSE;
	LPRECT		prects = (LPRECT)prd->Buffer;

	// Create an empty region if needed
	if ( hRgn == NULL ) hRgn = CreateRectRgn( 0, 0, 0, 0 );
	if ( hRgn == NULL ) { delete [] (LPBYTE)prd; return FALSE; }

	BYTE *arry = new BYTE[ w * h ];
	if ( arry == NULL ) return FALSE;

	if ( cneg != 0 || cpos != 0 )
	{
		for ( long y = 0; y < h; y++ )
			for ( long x = 0; x < w; x++ )
			{
				long iy = ( h - y - 1 );
				long i = ( ( ( iy * w ) + x ) * 3 ) + ( iy * sw );
		
				bool trans = FALSE;

				long red = pSrcBits[ i + 2 ] - r;
				if ( red >= min && red <= max )
				{	long green = pSrcBits[ i + 1 ] - g;
					if ( green >= min && green <= max )
					{	long blue = pSrcBits[ i ] - b;
						if ( blue >= min && blue <= max ) trans = TRUE;
					} // end if
				} // end if

				if ( trans ) 
					arry[ ( y * w ) + x ] = 1;
				else 
					arry[ ( y * w ) + x ] = 0;

			} // end if
	} // end if
	else
	{
		for ( long y = 0; y < h; y++ )
			for ( long x = 0; x < w; x++ )
			{
				long iy = ( h - y - 1 );
				long i = ( ( ( iy * w ) + x ) * 3 ) + ( iy * sw );
		
				if (	pSrcBits[ i ] != b || 
						pSrcBits[ i + 1 ] != g || 
						pSrcBits[ i + 2 ] != r ) 
					arry[ ( y * w ) + x ] = 0;
				else arry[ ( y * w ) + x ] = 1;

			} // end if
	} // end else

	// Fill in around the edges
	long k;
	for ( k = 0; k < w; k++ )
	{	MapFloodFill( arry, k, 0, w, h, 1, 2 );
		MapFloodFill( arry, k, h - 1, w, h, 1, 2 );
	} // end for
	for ( k = 0; k < h; k++ )
	{	MapFloodFill( arry, 0, k, w, h, 1, 2 );
		MapFloodFill( arry, w - 1, k, w, h, 1, 2 );
	} // end for

	for ( long y = 0; y < h; y++ )
		for ( long x = 0; x < w; x++ )
		{
			BOOL trans = ( arry[ ( y * w ) + x ] == 2 );

			if ( !trans )
			{
				if ( !working )
				{
					working = TRUE;
					SetRect(	&prects[ prd->rdh.nCount ], 
								x + xoff, y + yoff, 
								x + xoff + 1, y + yoff + 1 );
				} // end if
				else prects[ prd->rdh.nCount ].right++;

			} // end if

			else 
				trans = trans;

			if ( working && ( !trans || x == ( w - 1 ) ) )
			{
				working = FALSE;
				UnionRect(	&prd->rdh.rcBound,
							&prd->rdh.rcBound,
							&prects[ prd->rdh.nCount ] );
				prd->rdh.nCount++;
			} // end else if

			// Is it time to add the region?
			if (	prd->rdh.nCount >= MAXRGNRECTS ||
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

	// Release rect memory
	delete [] (LPBYTE)prd;

	return hRgn;

}

// rgbFill = Fill color
// var = color variation (1-8)
BOOL CWinDc::DeepFill(LPRECT pRect, COLORREF rgbFill, DWORD var)
{_STT();
	// Ensure rect
	if ( pRect == NULL ) pRect = &m_rect;

	// Fill solid
	CGrDC::FillSolidRect( GetSafeHdc(), pRect, rgbFill );

	// Texturize the bitmap
	return Texturize( var );
}

BOOL CWinDc::Tile(HDC hDC, LPRECT pRect)
{_STT();
	// Sanity checks
	if ( hDC == NULL || pRect == NULL ) return FALSE;
	if ( RW( m_rect ) <= 0 || RH( m_rect ) <= 0 ) return FALSE;

	long y = pRect->top;
	while ( y < pRect->bottom )
	{
		long x = pRect->left;
		while ( x < pRect->right )
		{
			RECT draw;
			CopyRect( &draw, &m_rect );
			OffsetRect( &draw, x, y );

			// Limit size
			if ( draw.right > pRect->right ) draw.right = pRect->right;
			if ( draw.bottom > pRect->bottom ) draw.bottom = pRect->bottom;

			// Draw a tile
			Draw( hDC, &draw, FALSE );

			// Add width
			x += RW( m_rect );

		} // end while

		// Add height
		y += RH( m_rect );

	} // end if

	return TRUE;
}

BOOL CWinDc::Mask(CWinDc *pMask, LPRECT pRect, CWinDc *pBck, COLORREF rgbBck)
{_STT();
	// Sanity checks
	if ( pMask == NULL ) return FALSE;
	if ( pRect == NULL ) pRect = &m_rect;

	// Get the size
	long w = GetWidth();
	long h = GetHeight();

	// Get transparent color
	BYTE r = GetRValue( rgbBck );
	BYTE g = GetGValue( rgbBck );
	BYTE b = GetBValue( rgbBck );

	// Get scan width
	long sw = CWinDc::GetScanWidth( w );
	sw -= ( w * 3 );
	
	// Get source bits
	CWinDc  src;
	LPBYTE  pSrcBits = NULL;
	if ( m_pBits != NULL ) pSrcBits = m_pBits;
	else
	{
		// Create source bitmap the same size
		if ( !src.CreateDIBSection( NULL, w, h, 24, &pSrcBits ) || pSrcBits == NULL )
			return FALSE;

		// Copy the image
		if ( !rDraw( src, src ) ) return FALSE;

	} // end if

	// Ensure we got the pointer
	if ( pSrcBits == NULL ) return FALSE;

	// Get mask bits
	CWinDc  mask;
	LPBYTE  pMaskBits = NULL;
	if ( EqualRect( &m_rect, *pMask ) && pMask->m_pBits != NULL ) 
		pMaskBits = pMask->m_pBits;
	else
	{
		// Create source bitmap the same size
		if ( !mask.CreateDIBSection( NULL, w, h, 24, &pMaskBits ) || pMaskBits == NULL )
			return FALSE;

		// Copy the image
		if ( !pMask->rDraw( mask, mask ) ) return FALSE;

	} // end if

	// Ensure we got the pointer
	if ( pMaskBits == NULL ) return FALSE;

	UINT rnd = CWinFile::Crc16( (BYTE)r, 0x7412 );
	for ( long y = 0; y < h; y++ )
		for ( long x = 0; x < w; x++ )
		{
			long iy = h - y - 1;
			long i = ( ( ( iy * w ) + x ) * 3 ) + ( iy * sw );

			DWORD bTl = pMaskBits[ i ];
			DWORD bInvTl = 255 - bTl;
			DWORD gTl = pMaskBits[ i + 1 ];
			DWORD gInvTl = 255 - gTl;
			DWORD rTl = pMaskBits[ i + 2 ];
			DWORD rInvTl = 255 - rTl;

			pSrcBits[ i ] =		UCHAR( ( ( pSrcBits[ i ] * bTl ) + 
								( b * bInvTl ) ) >> 8 );
			pSrcBits[ i + 1 ] = UCHAR( ( ( pSrcBits[ i + 1 ] * gTl ) + 
								( g * gInvTl ) ) >> 8 );
			pSrcBits[ i + 2 ] = UCHAR( ( ( pSrcBits[ i + 2 ] * bTl ) + 
								( r * rInvTl ) ) >> 8 );

		} // end for

	// Draw back if needed
	if ( src.GetSafeHdc() ) src.Draw( GetSafeHdc(), GetRect() );

	return TRUE;
}

BOOL CWinDc::Texturize( DWORD var, LPRECT pRect )
{_STT();
	// Ensure rect
	if ( pRect == NULL ) pRect = &m_rect;

	// Get the size
	long w = GetWidth();
	long h = GetHeight();

	// Verify limits
	if ( w > pRect->right ) w = pRect->right;
	if ( h > pRect->bottom ) h = pRect->bottom;

	// Get scan width
	long sw = CWinDc::GetScanWidth( w );
	sw -= ( w * 3 );

	CWinDc  src;
	LPBYTE  pSrcBits = NULL;

	if ( m_pBits != NULL ) pSrcBits = m_pBits;
	else
	{
		// Create source bitmap the same size
		if ( !src.CreateDIBSection( NULL, w, h, 24, &pSrcBits ) || pSrcBits == NULL )
			return FALSE;

		// Copy the image
		if ( !rDraw( src, src ) ) return FALSE;

	} // end if

	// Ensure we got the pointer
	if ( pSrcBits == NULL ) return FALSE;

	COLORREF rgb;
	UINT rnd = CWinFile::Crc16( (BYTE)( w + h ), 0x7412 );
	for ( long y = pRect->top; y < h; y++ )
		for ( long x = pRect->left; x < w; x++ )
		{
			long iy = h - y - 1;
			long i = ( ( ( iy * w ) + x ) * 3 ) + ( iy * sw );

			// Randomize
			rnd = CWinFile::Crc16( (BYTE)x, rnd + i );
			COLORREF cur = RGB( pSrcBits[ i + 2 ], pSrcBits[ i + 1 ], pSrcBits[ i ] );

			// Calculate new color
			if ( ( rnd & 0x80 ) != 0 ) rgb = CGrDC::ScaleColor( cur, rnd % var );
			else rgb = CGrDC::ScaleColor( cur, -(long)( rnd % var ) );

			// Set the new color
			pSrcBits[ i ] = GetBValue( rgb );
			pSrcBits[ i + 1 ] = GetGValue( rgb );
			pSrcBits[ i + 2 ] = GetRValue( rgb );

		} // end for

	// Draw back if needed
	if ( src.GetSafeHdc() ) src.Draw( GetSafeHdc(), GetRect() );

	return TRUE;
}

BOOL CWinDc::Aluminize(DWORD var, DWORD run, LPRECT pRect)
{_STT();
	// Ensure rect
	if ( pRect == NULL ) pRect = &m_rect;

	// Get the size
	long w = GetWidth();
	long h = GetHeight();

	// Verify limits
	if ( w > pRect->right ) w = pRect->right;
	if ( h > pRect->bottom ) h = pRect->bottom;

	// Get scan width
	long sw = CWinDc::GetScanWidth( w );
	sw -= ( w * 3 );

	CWinDc  src;
	LPBYTE  pSrcBits = NULL;

	if ( m_pBits != NULL ) pSrcBits = m_pBits;
	else
	{
		// Create source bitmap the same size
		if ( !src.CreateDIBSection( NULL, w, h, 24, &pSrcBits ) || pSrcBits == NULL )
			return FALSE;

		// Copy the image
		if ( !rDraw( src, src ) ) return FALSE;

	} // end if

	// Ensure we got the pointer
	if ( pSrcBits == NULL ) return FALSE;

	COLORREF rgb;
	UINT rnd = CWinFile::Crc16( (BYTE)( w + h ), 0x7412 );
	UINT crun = 0;
	UINT step = 0;
	BOOL bUp = FALSE;
	for ( long y = pRect->top; y < h; y++ )
	{
		for ( long x = pRect->left; x < w; x++ )
		{
			long iy = h - y - 1;
			long i = ( ( ( iy * w ) + x ) * 3 ) + ( iy * sw );

			if ( crun == 0 )
			{
				// Calc new random number
				rnd = CWinFile::Crc16( (BYTE)y, rnd + y );

				// Get new run length
				crun = rnd % run;

				// Calc new direction
				bUp = ( rnd & 0x80 ) != 0;

				// Calc new step
				step = rnd % var;				

			} // end if

			else crun--;

			// Randomize
			COLORREF cur = RGB( pSrcBits[ i + 2 ], pSrcBits[ i + 1 ], pSrcBits[ i ] );

			// Calculate new color
			if ( bUp ) rgb = CGrDC::ScaleColor( cur, step );
			else rgb = CGrDC::ScaleColor( cur, -(long)( step ) );

			// Set the new color
			pSrcBits[ i ] = GetBValue( rgb );
			pSrcBits[ i + 1 ] = GetGValue( rgb );
			pSrcBits[ i + 2 ] = GetRValue( rgb );

		} // end for
	} // end for

	// Draw back if needed
	if ( src.GetSafeHdc() ) src.Draw( GetSafeHdc(), GetRect() );

	return TRUE;
}

BOOL CWinDc::SetPixel(long x, long y, COLORREF rgb)
{
	if ( m_hDC == NULL ) return FALSE;
	return ::SetPixel( m_hDC, x, y, rgb );
}

COLORREF CWinDc::GetPixel(long x, long y)
{
	if ( m_hDC == NULL ) return FALSE;
	return ::GetPixel( m_hDC, x, y );
}

BOOL CWinDc::GetMetrics(LPBYTE *pBits, LONG *plWidth, LONG *plHeight, LONG *plSize, LONG *plScanWidth)
{_STT();
	// Is there a dc
	if ( m_hDC == NULL ) return FALSE;

	long lScanWidth = GetScanWidth( GetWidth() );
	if ( pBits != NULL ) *pBits = m_pBits;
	if ( plWidth != NULL ) *plWidth = GetWidth();
	if ( plHeight != NULL ) *plHeight = GetHeight();
	if ( plSize != NULL ) *plSize = GetHeight() * lScanWidth;
	if ( plScanWidth != NULL ) *plScanWidth = lScanWidth;

	return TRUE;
}

#ifdef ENABLE_OPENCV

IplImage* CWinDc::InitIplHeader( CWinDc *pImg, IplImage *pIpl )
{_STT();
	// !!! Use CreateDIBSection()
	if ( pImg->GetBits() == NULL ) return NULL;

	// Initialize structure
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

IplImage* CWinDc::InitIplHeader(IplImage *pIpl, LPBYTE pBuf, DWORD dwMax, long lWidth, long lHeight)
{
	ZeroMemory( pIpl, sizeof( IplImage ) );

	// Ensure reasonable size
	pIpl->imageSize = CWinDc::GetScanWidth( lWidth ) * lHeight;

	// Return NULL if the size does not match
	if ( (DWORD)pIpl->imageSize > dwMax ) return NULL;

	// Fill in the structure
	pIpl->nSize = sizeof( IplImage );
	pIpl->nChannels = 3;
	pIpl->depth = IPL_DEPTH_8U;
	pIpl->dataOrder = 0;
	pIpl->origin = IPL_ORIGIN_BL;
	pIpl->align = 4;
	pIpl->width = lWidth;
	pIpl->height = lHeight;
	pIpl->imageData = (char*)pBuf;
	pIpl->widthStep = CWinDc::GetScanWidth( lWidth );
	pIpl->imageDataOrigin = pIpl->imageData;

	return pIpl;
}

#endif
