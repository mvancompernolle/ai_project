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
// WinAvi.cpp: implementation of the CWinAvi class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <vfw.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// ICCompressorChoose
// ICCompressGetFormat
// ICSeqCompressFrameStart
// AVISaveOptions

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWinAvi::CWinAvi()
{_STT();
	AVIFileInit();

	m_dwError = 0;
	*m_szError = 0;
	*m_szFile = 0;
	m_lSample = 0;
	m_paviFile = NULL;
	m_paviStream = NULL;
	m_paviCompStream = NULL;
	ZeroMemory( &m_aviStreamInfo, sizeof( m_aviStreamInfo ) );
	ZeroMemory( &m_aviCompOptions, sizeof( m_aviCompOptions ) );
	ZeroMemory( &m_bmpInfo, sizeof( m_bmpInfo ) );
}

CWinAvi::~CWinAvi()
{_STT();
	Destroy();

	AVIFileExit();
}

void CWinAvi::Destroy()
{_STT();
	Close();
}

void CWinAvi::Close()
{_STT();
	// Lose the dc
	m_dc.Destroy();

	if ( m_paviCompStream != NULL )
	{	AVIStreamRelease( m_paviCompStream );
		m_paviCompStream = NULL;
	} // end if

	if ( m_paviStream != NULL )
	{	AVIStreamRelease( m_paviStream );
		m_paviStream = NULL;
	} // end if

	if ( m_paviFile != NULL )
	{	AVIFileRelease( m_paviFile );
		m_paviFile = NULL;
	} // end if

	// Zero information structures
	ZeroMemory( &m_aviStreamInfo, sizeof( m_aviStreamInfo ) );
	ZeroMemory( &m_aviCompOptions, sizeof( m_aviCompOptions ) );
	ZeroMemory( &m_bmpInfo, sizeof( m_bmpInfo ) );

	m_lSample = 0;
	*m_szFile = 0;
}

BOOL CWinAvi::OpenNew( LPCTSTR pFile, LPRECT pRect, DWORD dwFrames, DWORD dwSeconds, 
					   DWORD fourCC, LPVOID pCompData, DWORD dwCompDataSize, BOOL bCreateDIBSection )
{_STT();
	// Lose old file
	Destroy();

	// Sanity checks
	if ( pFile == NULL || pRect == NULL )
	{	m_dwError = MAXDWORD;
		strcpy( m_szError, "Invalid Params" );
		return FALSE;
	} // end if

	long w = PRW( pRect );
	long h = PRH( pRect );
	strcpy( m_szFile, pFile );

	// Open the avi file
	m_dwError = AVIFileOpen( &m_paviFile, m_szFile, OF_CREATE | OF_WRITE, NULL );
	if ( m_dwError != S_OK )
	{	wsprintf( m_szError, "%s : Unable to open AVI file : %lu : %s", GetErrorStr( m_dwError ), m_dwError, m_szFile ); return FALSE; }

	// Fill in stream information
	ZeroMemory( &m_aviStreamInfo, sizeof( m_aviStreamInfo ) );
	m_aviStreamInfo.fccType = streamtypeVIDEO;
	m_aviStreamInfo.fccHandler = fourCC;
	m_aviStreamInfo.dwScale = dwSeconds;
	m_aviStreamInfo.dwRate = dwFrames;
	m_aviStreamInfo.dwQuality = MAXDWORD;
	m_aviStreamInfo.dwSuggestedBufferSize = w * h * 4;
	CopyRect( &m_aviStreamInfo.rcFrame, pRect );
	strcpy( m_aviStreamInfo.szName, "Created with CWinAvi (www.wheresjames.com)" );

	// Create the stream
	m_dwError = AVIFileCreateStream(	m_paviFile, &m_paviStream, 
								&m_aviStreamInfo );
	if ( m_dwError != S_OK )
	{	wsprintf( m_szError, "%s : Unable to create AVI stream : %lu", GetErrorStr( m_dwError ), m_dwError ); Destroy(); return FALSE; }

	PAVISTREAM pFormatStream = m_paviStream;

	// Do we need a compressor?
	if ( fourCC != 0 )
	{
		// Fill in compressed stream information
		ZeroMemory( &m_aviCompOptions, sizeof( m_aviCompOptions ) );
		m_aviCompOptions.fccType = streamtypeVIDEO;
		m_aviCompOptions.fccHandler = fourCC;
		m_aviCompOptions.dwFlags = AVICOMPRESSF_KEYFRAMES | AVICOMPRESSF_VALID;
		m_aviCompOptions.dwKeyFrameEvery = 1;
		m_aviCompOptions.lpParms = pCompData;
		m_aviCompOptions.cbParms = dwCompDataSize;

		// Create the compressed stream
		m_dwError = AVIMakeCompressedStream(	&m_paviCompStream, m_paviStream, 
												&m_aviCompOptions, NULL );
		if ( m_dwError != S_OK )
		{	wsprintf( m_szError, "%s : Unable to create compressed AVI stream : %lu", GetErrorStr( m_dwError ), m_dwError ); Destroy(); return FALSE; }

		// Set the format on the compressed stream
		pFormatStream = m_paviCompStream;

	} // end if

		// Fill in bitmap information
		ZeroMemory( &m_bmpInfo, sizeof( m_bmpInfo ) );
		m_bmpInfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
		m_bmpInfo.bmiHeader.biWidth = w;
		m_bmpInfo.bmiHeader.biHeight = h;
		m_bmpInfo.bmiHeader.biPlanes = 1;
		m_bmpInfo.bmiHeader.biBitCount = 24;
		m_bmpInfo.bmiHeader.biCompression = BI_RGB;

		// Set the stream format
		m_dwError = AVIStreamSetFormat(	pFormatStream, 0, 
									&m_bmpInfo, m_bmpInfo.bmiHeader.biSize );
		if ( m_dwError != S_OK )
		{	wsprintf( m_szError, "%s : Unable to set AVI stream format : %lu", GetErrorStr( m_dwError ), m_dwError ); Destroy(); return FALSE; }


	// Create dib section
	if ( bCreateDIBSection )
		if ( !m_dc.CreateDIBSection( NULL, w, h, 24 ) )
		{	strcpy( m_szError, "Unable to create DIB section" ); Destroy(); return FALSE; }

	return TRUE;
}

BOOL CWinAvi::AddFrame(CWinDc *pDC)
{_STT();
	// Ensure open stream
	if ( m_paviStream == NULL && m_paviCompStream == NULL ) 
		return FALSE;

	LONG	lImgSize = 0;
	LPBYTE	pBits = pDC->GetBits();

	// Check for direct pixel access
	if ( pBits == NULL )
	{
		// Ensure dc
		if ( m_dc.GetSafeHdc() == NULL || m_dc.GetBits() == NULL )
			return FALSE;

		// Draw to ensure correct format
		pDC->Draw( m_dc, m_dc );

		pBits = m_dc.GetBits();
		if ( pBits == NULL ) return FALSE;

		lImgSize = m_dc.GetImageSize();

	} // end if

	else lImgSize = pDC->GetImageSize();

	// Is there a compressed stream?
	if ( m_paviCompStream != NULL )
		return AVIStreamWrite(	m_paviCompStream, m_lSample++, 1, 
								pBits, lImgSize, 
								0, NULL, NULL ) == S_OK;

	// Write out the image to uncompressed stream
	else if ( m_paviStream != NULL )
		return AVIStreamWrite(	m_paviStream, m_lSample++, 1, 
								pBits, lImgSize, 
								0, NULL, NULL ) == S_OK;

	return FALSE;
}


BOOL CWinAvi::OpenExisting(LPCTSTR pFile )
{_STT();
	// Lose current file
	Destroy();

	if ( pFile == NULL ) return FALSE;
	strcpy( m_szFile, pFile );

	// Open the avi file
	if ( AVIFileOpen( &m_paviFile, m_szFile, OF_READ | OF_WRITE, NULL ) != S_OK )
		return FALSE;

	// Open the video stream
	if ( AVIFileGetStream(	m_paviFile, &m_paviStream, 
							streamtypeVIDEO, 0 ) != S_OK )
	{	Destroy(); return FALSE; }

	// Get stream information
	if ( AVIStreamInfo( m_paviStream, &m_aviStreamInfo, 
						sizeof( m_aviStreamInfo ) ) != S_OK )
	{	Destroy(); return FALSE; }

	// Fill in compressed stream information
	ZeroMemory( &m_aviCompOptions, sizeof( m_aviCompOptions ) );
	m_aviCompOptions.fccType = m_aviStreamInfo.fccType;
	m_aviCompOptions.fccHandler = m_aviStreamInfo.fccHandler;
	m_aviCompOptions.dwFlags = AVICOMPRESSF_KEYFRAMES | AVICOMPRESSF_VALID;
	m_aviCompOptions.dwKeyFrameEvery = 1;

	// Create the compressed stream
	if ( AVIMakeCompressedStream(	&m_paviCompStream, m_paviStream, 
									&m_aviCompOptions, NULL ) != S_OK )
	{	Destroy(); return FALSE; }

	// Fill in bitmap information
/*	ZeroMemory( &m_bmpInfo, sizeof( m_bmpInfo ) );
	m_bmpInfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	m_bmpInfo.bmiHeader.biWidth = RW( m_aviStreamInfo.rcFrame );
	m_bmpInfo.bmiHeader.biHeight = RH( m_aviStreamInfo.rcFrame );
	m_bmpInfo.bmiHeader.biPlanes = 1;
	m_bmpInfo.bmiHeader.biBitCount = 24;
	m_bmpInfo.bmiHeader.biCompression = BI_RGB;

	// Set the stream format
	if ( AVIStreamSetFormat(	m_paviCompStream, 0, 
								&m_bmpInfo, m_bmpInfo.bmiHeader.biSize ) != S_OK )
	{	Destroy(); return FALSE; }
*/
	// Move to last sample
	m_lSample = AVIStreamLength( m_paviCompStream );

	// Attempt to open existing stream
//	if ( AVIStreamOpenFromFile( &m_paviCompStream, pFile, streamtypeVIDEO, 
//								0, OF_WRITE | OF_READ, NULL ) != S_OK )
//		return FALSE;

	// Create dib section
	if ( !m_dc.CreateDIBSection( NULL,	RW( m_aviStreamInfo.rcFrame ), 
										RW( m_aviStreamInfo.rcFrame ), 
										24 ) )
	{	Destroy(); return FALSE; }

	return TRUE;
}



LPCTSTR CWinAvi::GetErrorStr(DWORD dwError)
{_STT();
	switch ( dwError )
	{
		case AVIERR_UNSUPPORTED :
			return "Unsupported operation";
		case AVIERR_BADFORMAT :
			return "Bad format";
		case AVIERR_INTERNAL :
			return "Internal Error";
		case AVIERR_BADFLAGS :
			return "Bad Flags";
		case AVIERR_BADPARAM :
			return "Bad Param";
		case AVIERR_BADSIZE :
			return "Bad Size";
		case AVIERR_BADHANDLE :
			return "Bad Handle";
		case AVIERR_FILEREAD :
			return "File read error";
		case AVIERR_FILEWRITE :
			return "File write error";
		case AVIERR_FILEOPEN :
			return "File open error";
		case AVIERR_COMPRESSOR :
			return "Compressor error";
		case AVIERR_NOCOMPRESSOR :
			return "No compressor";
		case AVIERR_READONLY :
			return "Read only";
		case AVIERR_NODATA :
			return "No data";
		case AVIERR_BUFFERTOOSMALL :
			return "Buffer is too small";
		case AVIERR_CANTCOMPRESS :
			return "Unable to compress";
		case AVIERR_USERABORT :
			return "User abort";
		case AVIERR_ERROR :
			return "General error";
		case REGDB_E_CLASSNOTREG :
			return "AVIFileOpen does not have a handler in registry";
		case CO_E_NOTINITIALIZED :
			return "AVI functions not initialized";

//		case AVIERR_BADFORMAT :
//			return "";

	} // end switch

	return "";
}
