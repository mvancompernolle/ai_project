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
// Capture.cpp: implementation of the CCapture class.
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
				
CCapture::CCapture()
{_STT();
	m_hWnd = NULL;

	m_hGrab = NULL;

	m_bConnected = FALSE;
	m_dwCurrentDriver = CCAPTURE_INVALIDDRIVER;
	m_dwNumDrivers = 0;

	m_pVideoData = NULL;
	m_dwVideoDataSize = 0;
	m_pVideoFormat = NULL;

	m_pAudioData = NULL;
	m_dwAudioDataSize = 0;
	m_pAudioFormat = NULL;

	// Windows default filename
	strcpy( m_szCaptureFileName, "C:\\CAPTURE.AVI" );

	m_bCallbacksEnabled = FALSE;

	m_dwOnStatus = 0;
	m_dwOnError = 0;
	m_dwOnFrame = 0;
	
	m_hFrameWnd = NULL;
	m_uFrameMsg = 0;

	m_hDrawDc = NULL;
	m_hDrawDib = NULL;
}

CCapture::~CCapture()
{_STT();
	Destroy();
}

BOOL CCapture::Create(	LPCTSTR pTitle, 
						HWND hwndParent, 
						DWORD dwStyle, 
						long x, long y, 
						long width, long height, 
						int nID )
{_STT();
	// Lose old window
	Destroy();
	
	// Attempt to create a capture window
	m_hWnd = capCreateCaptureWindow( pTitle, dwStyle, x, y, width, height, hwndParent, nID );

	return IsWnd();
}

void CCapture::Destroy()
{_STT();
	// Disconnect from driver
	Disconnect();

	if ( m_hWnd != NULL )
	{
		::DestroyWindow( m_hWnd );
		m_hWnd = NULL;
	} // end if

	if ( m_hGrab != NULL )
	{	CloseHandle( m_hGrab );
		m_hGrab = NULL;
	} // end if
	
	// Release allocated buffers
	ReleaseVideoFormatData();
	ReleaseAudioFormatData();

	// Reset vars
	m_dwCurrentDriver = CCAPTURE_INVALIDDRIVER;
	m_dwNumDrivers = 0;

	// Windows default filename
	strcpy( m_szCaptureFileName, "C:\\CAPTURE.AVI" );

	m_dwOnStatus = 0;
	m_dwOnError = 0;
	m_dwOnFrame = 0;

	if ( m_hDrawDib != NULL ) 
	{	DrawDibClose( m_hDrawDib );
		m_hDrawDib = NULL;
	} // end if
}

void CCapture::Disconnect()
{_STT();
	if ( !IsWnd() ) return;

	// No more callbacks
	DisableCallbacks();

	// Disconnect from capture driver
	if ( m_bConnected ) 
	{
		capDriverDisconnect( m_hWnd );
		m_bConnected = FALSE;
	} // end if

	if ( m_hDrawDib != NULL ) 
	{
		DrawDibClose( m_hDrawDib );
		m_hDrawDib = NULL;
	} // end if
}

void CCapture::ReleaseAudioFormatData()
{_STT();
	if ( m_pAudioData != NULL )
	{
		delete [] m_pAudioData;
		m_pAudioData = NULL;
	} // end if
	m_dwAudioDataSize = 0;
	m_pAudioFormat = NULL;
}

void CCapture::ReleaseVideoFormatData()
{_STT();
	if ( m_pVideoData != NULL )
	{
		delete [] m_pVideoData;
		m_pVideoData = NULL;
	} // end if
	m_dwVideoDataSize = 0;
	m_pVideoFormat = NULL;
}

BOOL CCapture::Connect(DWORD i)
{_STT();
	if ( !IsWnd() ) return FALSE;

	// Disconnect from old driver
	Disconnect();

	// Attempt to connect to the specified driver
	m_bConnected = capDriverConnect( m_hWnd, i );
	if ( !m_bConnected ) return FALSE;

	// Save the driver number
	m_dwCurrentDriver = i;

	// Get driver capabilities
	capDriverGetCaps( m_hWnd, &m_cdc, sizeof( m_cdc ) );

	// Initialize draw dib
	m_hDrawDib = DrawDibOpen();

	return TRUE;		
}

BOOL CCapture::Reconnect()
{_STT();
	// Disconnect from capture driver
	if ( !m_bConnected ) return FALSE;

	// Disconnect capture driver
	capDriverDisconnect( m_hWnd );

	// Attempt to connect to the specified driver
	m_bConnected = capDriverConnect( m_hWnd, m_dwCurrentDriver );
	if ( !m_bConnected ) { Disconnect(); return FALSE; }

	// Get driver capabilities
	capDriverGetCaps( m_hWnd, &m_cdc, sizeof( m_cdc ) );

	return TRUE;
}

BOOL CCapture::LoadDriverInfo()
{_STT();
	// Assume no drivers
	m_dwNumDrivers = 0;

	// Load driver info
	for ( DWORD i = 0; i < CCAPTURE_MAXDRIVERS; i++ )
	{
		if ( capGetDriverDescription(	i, 
										m_cdi[ m_dwNumDrivers ].szName, sizeof( m_cdi[ m_dwNumDrivers ].szName ),
										m_cdi[ m_dwNumDrivers ].szVer, sizeof( m_cdi[ m_dwNumDrivers ].szVer ) ) )
			m_cdi[ m_dwNumDrivers++ ].index = i;

		// Fake drivers
//		else if ( i < 5 )
//		{
//			m_cdi[ m_dwNumDrivers ].index = i;
//			wsprintf( m_cdi[ m_dwNumDrivers ].szName, "Fake driver %lu", i );
//			strcpy( m_cdi[ m_dwNumDrivers ].szVer, "$.$.$" );
//			m_dwNumDrivers++;
//		} // end else if

		// No more drivers
//		else i = CCAPTURE_MAXDRIVERS;

	} // end if

	// Return TRUE if we found any drivers
	return ( m_dwNumDrivers > 0 );
}


BOOL CCapture::ShowDialogBox(DWORD db)
{_STT();
	if ( !IsConnected() ) return FALSE;

	// What dialog box do we want to show
	switch( db )
	{
		case CCAPTURE_DLG_VIDSOURCE:
			if ( m_cdc.fHasDlgVideoSource ) return capDlgVideoSource( m_hWnd );
			break;

		case CCAPTURE_DLG_VIDFORMAT:
			if ( m_cdc.fHasDlgVideoFormat ) return capDlgVideoFormat( m_hWnd );
			break;

		case CCAPTURE_DLG_VIDDISPLAY:
			if ( m_cdc.fHasDlgVideoDisplay ) return capDlgVideoDisplay( m_hWnd );
			break;

		case CCAPTURE_DLG_VIDCOMPRESSION:
			return capDlgVideoCompression( m_hWnd );
			break;

		default: break;
	} // end switch

	return FALSE;
}

BOOL CCapture::GetVideoFormat( LPBITMAPINFO * ppbmp, LPDWORD psize )
{_STT();
	if ( ppbmp != NULL ) *ppbmp = NULL;
	if ( psize != NULL ) *psize = 0;
	if ( !IsConnected() ) return FALSE;

	// Lose the video data
	ReleaseVideoFormatData();

	// Get the size of the format buffer
	m_dwVideoDataSize = capGetVideoFormatSize( m_hWnd );
	if ( m_dwVideoDataSize == 0 ) return FALSE;

	// Allocate memory
	m_pVideoData = new BYTE[ m_dwVideoDataSize + 1 ];
	if ( m_pVideoData == NULL )
	{ 
		m_dwVideoDataSize = 0;
		return FALSE;
	} // end if

	// Set the pointer
	m_pVideoFormat = (BITMAPINFO*)m_pVideoData;

	if ( !capGetVideoFormat( m_hWnd, m_pVideoFormat, m_dwVideoDataSize ) )
	{
		ReleaseVideoFormatData();
		return FALSE;
	} // end if

	// For good measure
	m_pVideoData[ m_dwVideoDataSize ] = 0;

	// Does the caller want a copy of the pointer?
	if ( ppbmp != NULL ) *ppbmp = m_pVideoFormat;
	if ( psize != NULL ) *psize = m_dwVideoDataSize;

	return TRUE;
}

BOOL CCapture::AddInfoChunk(FOURCC type, LPBYTE buf, DWORD size)
{_STT();
	if ( buf == NULL || size == 0 ) return FALSE;

	CAPINFOCHUNK	cic;

	cic.fccInfoID = type;
	cic.lpData = buf;
	cic.cbData = size;

	return capFileSetInfoChunk( m_hWnd, &cic );
}

BOOL CCapture::EnableCallbacks()
{_STT();
	if ( !IsWnd() ) return FALSE;

	// Reset the number of callbacks
	m_dwOnStatus = 0;
	m_dwOnError = 0;
	m_dwOnFrame = 0;

	// Set our class pointer
	SetLastError( 0 );
	// Give the window a pointer to the class
	if (	!SetWindowLong( GetSafeHwnd(), GWL_USERDATA, (LONG)this ) &&
			GetLastError() != 0 )
	{
		// Give up if we can't set the class pointer
		return FALSE;
	} // end if

	m_bCallbacksEnabled = TRUE;

	// Enable error callbacks
	capSetCallbackOnError( m_hWnd, CCapture::ErrorCallbackProc );

	// Enable status callbacks
	capSetCallbackOnStatus( m_hWnd, CCapture::StatusCallbackProc );

	// Enable frame callbacks
	capSetCallbackOnFrame( m_hWnd, CCapture::FrameCallbackProc );

	return TRUE;
}
void CCapture::DisableCallbacks()
{_STT();
	m_bCallbacksEnabled = FALSE;

	capSetCallbackOnError( m_hWnd, NULL );
	capSetCallbackOnStatus( m_hWnd, NULL );
	capSetCallbackOnFrame( m_hWnd, NULL );
}

LRESULT PASCAL CCapture::ErrorCallbackProc(HWND hWnd, int nErrID, LPSTR lpErrorText)
{_STT();
	// Retrieve our class pointer
	CCapture	*pCap = (CCapture*)GetWindowLong( hWnd, GWL_USERDATA );
	if ( pCap == NULL ) return 0;

	return pCap->OnError( nErrID, lpErrorText );
}
BOOL CCapture::OnError(int nID, LPCTSTR pStatus)
{_STT();
	// Count errors
	m_dwOnError++;

	if ( nID != 0 && pStatus != NULL && *pStatus != 0 )
	{
		// Log error
//		_Log( MB_ICONERROR, "Capture Driver", pStatus );

	} // end if

	return TRUE;
}

LRESULT PASCAL CCapture::StatusCallbackProc( HWND hWnd, int nID, LPSTR lpStatusText )
{_STT();
	// Retrieve our class pointer
	CCapture *pCap = (CCapture*)GetWindowLong( hWnd, GWL_USERDATA );
	if ( pCap == NULL ) return 0;

	return pCap->OnStatus( nID, lpStatusText );
}

BOOL CCapture::OnStatus(int nID, LPCTSTR pStatus)
{_STT();
	// Count status updates
	m_dwOnStatus++;

	return TRUE;
}

BOOL CCapture::GrabFrame()
{_STT();
	RULIB_TRY 
	{
		// If we need a grab event
		if ( m_hGrab == NULL ) 
			m_hGrab = CreateEvent( NULL, TRUE, FALSE, NULL );
		else ResetEvent( m_hGrab );
		
		// Aquire lock
		if ( m_capturelock.lock() )
		{
			// Grab frame
			BOOL ret = capGrabFrame( m_hWnd ); 
			SetEvent( m_hGrab );

			// Unlock
			m_capturelock.unlock();

			// Return status
			return ret;
		} 
		
	} RULIB_CATCH_ALL { ASSERT( 0 ); return FALSE; } 

	return FALSE;
}


LRESULT PASCAL CCapture::FrameCallbackProc(HWND hWnd, LPVIDEOHDR lpVHdr)
{_STT();
	// Retrieve our class pointer
	CCapture	*pCap = (CCapture*)GetWindowLong( hWnd, GWL_USERDATA );
	if ( pCap == NULL ) return 0;

	return pCap->OnFrame( lpVHdr );
}
/*
	typedef struct 
	{
		LPSTR lpData;            // address of video buffer
		DWORD dwBufferLength;    // size, in bytes, of the 
								 // data buffer
		DWORD dwBytesUsed;       // see below
		DWORD dwTimeCaptured;    // see below
		DWORD dwUser;            // user-specific data
		DWORD dwFlags;           // see below
		DWORD dwReserved[4];     // reserved; do not use
	} VIDEOHDR;
 
	- Members -

	dwBytesUsed:
	Number of bytes used in the data buffer. 

	dwTimeCaptured:
	Time, in milliseconds, when the frame was captured relative to 
	the first frame in the stream. 
	Synchronization of audio and video in the resulting AVI file 
	depends solely on this parameter, which should be derived 
	directly from a counter (VSYNC_Count) incremented by a vertical 
	sync interrupt. When using NTSC frame rates and frame-based 
	interrupts, the returned value would be: 

	dwTimeCaptured = VSYNC_Count * 1/29.97.
 
	The capture driver must maintain VSYNC_Count accurately by 
	allowing recursive entry into the interrupt service routine 
	(if only for the purpose of incrementing VSYNC_Count) and by not
	disabling interrupts for long periods of time. 

	dwFlags:
	Flags giving information about the data buffer. The following 
	flags are defined for this field: VHDR_DONE  Set by the device 
	driver to indicate it is finished with the data buffer and it is
	returning the buffer to the application.  

	VHDR_PREPARED	Set by the system to indicate the data buffer has 
					been prepared with videoStreamPrepareHeader.  
	VHDR_INQUEUE	Set by the system to indicate the data buffer is 
					queued for playback.  
	VHDR_KEYFRAME	Set by the device driver to indicate a key frame.  
*/
BOOL CCapture::OnFrame(LPVIDEOHDR pVHdr)
{_STT();
	// Aquire lock
	CTlLocalLock ll( &m_lockInCallback, 0 );

	// Count frames
	m_dwOnFrame++;

	// Setup callback information
	m_fci.pVHdr = pVHdr;
	m_fci.pCap = this;

	if ( m_hDrawDc != NULL )
	{
		// Lock the DC
		if ( LockDC() )
		{
			// Attempt to decode the frame
			DrawDibDraw(	m_hDrawDib, m_hDrawDc,
							m_rectDraw.left, m_rectDraw.top,
							( m_rectDraw.right - m_rectDraw.left ),
							( m_rectDraw.bottom - m_rectDraw.top ),
							(LPBITMAPINFOHEADER)m_pVideoFormat,
							pVHdr->lpData,
							0, 0, 
							m_pVideoFormat->bmiHeader.biWidth,
							m_pVideoFormat->bmiHeader.biHeight,
							0 );

			// Let the user in on the good news
			if ( m_hFrameWnd != NULL )
				::SendMessage(	m_hFrameWnd, m_uFrameMsg, 
								(WPARAM)m_dwOnFrame, (LPARAM)&m_fci );
			
			// Unlock the DC
			UnlockDC();
		} // end if
	} // end if

	// Let the user in on the good news
	else if ( m_hFrameWnd != NULL )
		::SendMessage(	m_hFrameWnd, m_uFrameMsg, 
						(WPARAM)m_dwOnFrame, (LPARAM)&m_fci );

	// Grab Complete
	SetEvent( m_hGrab );

	return TRUE;
}

BOOL CCapture::GetAudioFormat( LPWAVEFORMATEX *ppwfx )
{_STT();
	if ( !IsConnected() ) return FALSE;

	// Lose the old audio data
	ReleaseAudioFormatData();

	// Get the size of the format buffer
	m_dwAudioDataSize = capGetAudioFormatSize( m_hWnd );
	if ( m_dwAudioDataSize == 0 ) return FALSE;

	// Allocate memory
	m_pAudioData = new BYTE[ m_dwAudioDataSize + 1 ];
	if ( m_pAudioData == NULL )
	{ 
		m_dwAudioDataSize = 0;
		return FALSE;
	} // end if

	// Set the pointer
	m_pAudioFormat = (LPWAVEFORMATEX)m_pAudioData;

	if ( !capGetAudioFormat( m_hWnd, m_pAudioFormat, m_dwAudioDataSize ) )
	{
		ReleaseAudioFormatData();
		return FALSE;
	} // end if

	// Does the caller want a copy of the pointer?
	if ( ppwfx != NULL ) *ppwfx = m_pAudioFormat;

	return TRUE;
}


BOOL CCapture::HBitmapFromFrame(HBITMAP *phbmp, LPVIDEOHDR pvhdr)
{_STT();
	if ( pvhdr == NULL ) return FALSE;
	
	return FALSE;
}


BOOL CCapture::SetCaptureWindowPos(HWND hwndAfter, int x, int y, int cx, int cy, UINT flags)
{_STT();
	// Punt if no window
	if ( !::IsWindow( m_hWnd ) ) return FALSE;

	return SetWindowPos( m_hWnd, hwndAfter, x, y, cx, cy, flags );
}

BOOL CCapture::SetFrameMessageTarget(HDC hDC, LPRECT pRect, HWND hWnd, UINT uMsg)
{_STT();
	// Are we clearing the target?
	if ( hDC == NULL || pRect == NULL || hWnd == NULL )
	{	m_hDrawDc = NULL;
		m_hFrameWnd = NULL;
		m_uFrameMsg = NULL;
	} // end if

	// Copy the destination rectangle
	CopyRect( &m_rectDraw, pRect );

	// Save information
	m_hDrawDc = hDC;
	m_hFrameWnd = hWnd; 
	m_uFrameMsg = uMsg;

	// Get the bitmap information
	LPBITMAPINFO pbi;
	if ( !GetVideoFormat( &pbi ) || pbi == NULL ) return FALSE;

	// Set up the DrawDib
	return DrawDibBegin(	m_hDrawDib, hDC, 
							( pRect->right - pRect->left ),
							( pRect->bottom - pRect->top ),
							(LPBITMAPINFOHEADER)pbi, 
							pbi->bmiHeader.biWidth,
							pbi->bmiHeader.biHeight,
							0 );//DDF_DONTDRAW );
}



