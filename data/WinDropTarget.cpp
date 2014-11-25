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
// WinDropTarget.cpp: implementation of the CWinDropTarget class.
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

CWinDropTarget::CWinDropTarget()
{_STT();
	m_dwRefCount = 0;
	m_hWnd = NULL;
	*m_szData = 0;

	m_bPlainTextMode = TRUE;
}

CWinDropTarget::~CWinDropTarget()
{_STT();
	Revoke();
}

HRESULT CWinDropTarget::DragEnter(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{_STT();
	// Save drag object
	m_spdtoDragging = pdto;
	
	// Clear previous data
	*m_szData = 0;	

	if ( m_bPlainTextMode )
	{
		// Check for plain text format
		STGMEDIUM stgMedium;
		FORMATETC fmt = { CF_TEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		if ( pdto->GetData( &fmt, &stgMedium ) == S_OK )
		{
			RULIB_TRY
			{
				switch ( stgMedium.tymed )
				{
					case TYMED_FILE:

						// Copy the filename
						wcstombs( m_szData, stgMedium.lpszFileName, sizeof( m_szData ) );
						break;

					case TYMED_MFPICT:
					case TYMED_HGLOBAL:
					{
						LPCTSTR pStr = (LPCTSTR)GlobalLock( stgMedium.hGlobal );
						if ( pStr != NULL )
						{	strcpy_sz( m_szData, pStr );
							GlobalUnlock( stgMedium.hGlobal );
						} // end if

					} break;

				} // end switch

				// Release memory
				::ReleaseStgMedium( &stgMedium );

			} // end try

			// Catch all
			RULIB_CATCH_ALL { ASSERT( 0 ); }
		} // end if

	} // end if

	*pdwEffect = DROPEFFECT_LINK | DROPEFFECT_COPY | DROPEFFECT_MOVE;

	return S_OK;
}

HRESULT CWinDropTarget::DragLeave()
{_STT();
	// Release drag object
	if ( m_spdtoDragging != NULL ) m_spdtoDragging.Release(); 

	return S_OK;
}

HRESULT CWinDropTarget::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{_STT();
	*pdwEffect = DROPEFFECT_LINK | DROPEFFECT_COPY | DROPEFFECT_MOVE;

	return S_OK;
}

HRESULT CWinDropTarget::Drop(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{_STT();
	// Plain text mode
	if ( m_bPlainTextMode )
	{
		// Did we get any data?
		if ( *m_szData != 0 ) *pdwEffect = DROPEFFECT_LINK | DROPEFFECT_COPY | DROPEFFECT_MOVE;
		else *pdwEffect = DROPEFFECT_NONE;

		// Release drag object
		if ( m_spdtoDragging != NULL ) m_spdtoDragging.Release(); 

		// Inform window
		if ( *m_szData != 0 && ::IsWindow( m_hWnd ) )
			SendMessage( m_hWnd, m_uMsg, CF_TEXT, (LPARAM)m_szData );

	} // end if

	else if ( pdto != NULL && ::IsWindow( m_hWnd ) )
	{				  
		// Set effect
		*pdwEffect = DROPEFFECT_LINK | DROPEFFECT_COPY;

		// Initialize data structure
		WINDROPTARGETDATA wdtd;
		wdtd.pdto = pdto;
		wdtd.grfKeyState = grfKeyState;
		wdtd.ptl.x = ptl.x; wdtd.ptl.y = ptl.y;
		wdtd.dwEffect = *pdwEffect;

		// Send the message
		SendMessage( m_hWnd, m_uMsg, 0, (LPARAM)&wdtd );

		// Release drag object
		pdto->Release(); 

	} // end else

	return S_OK;
}

BOOL CWinDropTarget::Register(HWND hWnd, UINT uMsg)
{_STT();
    m_hWnd = hWnd;
	m_uMsg = uMsg;
    return ( RegisterDragDrop( m_hWnd, this ) == S_OK );
}

BOOL CWinDropTarget::Revoke()
{_STT();
	// Revoke drag drop target if any
	if ( ::IsWindow( m_hWnd ) ) return ( RevokeDragDrop( m_hWnd ) == S_OK );
	m_hWnd = NULL;
	return TRUE;
}

BOOL CWinDropTarget::GetData(IDataObject *pDo, UINT uClipboardFormat, TMem< BYTE > &buf)
{_STT();
	// Sanity checks
	if ( pDo == NULL ) return FALSE;

	// Lose buffer
	buf.destroy();

	RULIB_TRY
	{
		STGMEDIUM stgMedium;
		FORMATETC fmt = { uClipboardFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

		// Attempt to get the data
		if ( pDo->GetData( &fmt, &stgMedium ) != S_OK )
			return FALSE;

		switch ( stgMedium.tymed )
		{
			case TYMED_FILE:
			{				   
				size_t len = wcslen( stgMedium.lpszFileName );
				if ( len )
				{
					if ( buf.allocate( len + 1 ) )
					{
						// Get the filename
						wcstombs( (char*)buf.ptr(), stgMedium.lpszFileName, sizeof( len ) );

						buf.ptr()[ len ] = 0;
						
					} // end if

				} // end if

			} break;

			case TYMED_MFPICT:
			case TYMED_HGLOBAL:
			{
				LPBYTE pBuf = (LPBYTE)GlobalLock( stgMedium.hGlobal );
				if ( pBuf != NULL )
				{	
					SIZE_T len = GlobalSize( stgMedium.hGlobal );
					if ( len && buf.allocate( len + 1 ) )
					{
						// copy the data
						memcpy( buf.ptr(), pBuf, len );

						buf.ptr()[ len ] = 0;

					} // end if

					// Release the buffer
					GlobalUnlock( stgMedium.hGlobal );

				} // end if

			} break;

		} // end switch

		// Release memory
		::ReleaseStgMedium( &stgMedium );

	} // end try

	// Catch all
	RULIB_CATCH_ALL { ASSERT( 0 ); }

	return TRUE;
}
