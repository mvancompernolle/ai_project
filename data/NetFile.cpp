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
// NetFile.cpp: implementation of the CNetFile class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetFile::CNetFile( LPCTSTR pUserAgent )
{_STT();
	*m_szUrl = NULL;
	*m_szLocal = NULL;

	m_hFile = NULL;
	m_hInternet = NULL;
	m_hConnect = NULL;

	m_pMem = NULL;
	m_bMem = FALSE;

	m_dwTransferStatus = 0;
	m_dwTransferError = 0;

	m_bCloseFileAfterDownload = FALSE;

	m_dwDataRead = 0;

	m_dwBlockSize = 4096;

	m_dwInetFlags = 0;
	m_dwOpenFlags = INTERNET_FLAG_RESYNCHRONIZE;
	m_dwAccessFlags = INTERNET_OPEN_TYPE_PRECONFIG;

	if ( pUserAgent != NULL ) { strcpy_sz( m_szUserAgent, pUserAgent ); }
	else strcpy( m_szUserAgent, "WheresJames Software (www_wheresjames_com)" );	

	m_hWndParent = NULL;

	m_bUpload = FALSE;

	m_dwDataWritten = 0;

	*m_szUsername = 0;
	*m_szPassword = 0;

	m_hWndCallback = NULL;
	m_uMsgCallback = 0;

	m_pCallback = NULL;

	m_bPassive = TRUE;

	m_bGetTotalLength = FALSE;
	m_dwTotalLength = 0;
}

CNetFile::~CNetFile()
{_STT();
	Destroy();
}

void CNetFile::Destroy()
{_STT();
	StopThread();
	
	Release();
}

void CNetFile::Release()
{_STT();
	m_local.Close();

	if ( m_pMem != NULL )
	{
		delete [] m_pMem;
		m_pMem = NULL;
	} // end if
	m_bMem = FALSE;

	if ( m_hFile != NULL ) 
	{
		InternetCloseHandle( m_hFile );
		m_hFile = NULL;
	} // end if

	if ( m_hConnect != NULL ) 
	{
		InternetCloseHandle( m_hConnect );
		m_hConnect = NULL;
	} // end if

	if ( m_hInternet != NULL ) 
	{
		InternetCloseHandle( m_hInternet );
		m_hInternet = NULL;
	} // end if

	*m_szUrl = NULL;
	*m_szLocal = NULL;

	m_dwTransferStatus = 0;
	m_dwTransferError = 0;

	m_dwDataRead = 0;

	m_hWndParent = NULL;

	m_bUpload = FALSE;

	m_dwDataWritten = 0;

	m_sHeaders.destroy();
	m_sData.destroy();
}


BOOL CNetFile::Download(LPCTSTR pUrl, LPCTSTR pLocal, BOOL bCloseFileAfterDownload, HWND hWndParent)
{_STT();
	// Lose previous file
	Destroy();

	// Downloading
	m_bUpload = FALSE;

	// Save parent window
	m_hWndParent = hWndParent;

	// Save close file status
	m_bCloseFileAfterDownload = bCloseFileAfterDownload;

	// Sanity check
	if ( pUrl == NULL || *pUrl == NULL ) return FALSE;

	// Are we downloading to file or ram?
	m_bMem = ( pLocal == NULL );

	{ // Copy the url
		
		char buf[ sizeof( m_szUrl ) ];
		DWORD size = sizeof( m_szUrl ) - 1;

		// Is it a local file?
		if ( GetFileAttributes( pUrl ) == MAXDWORD )
		{
			// Fix the url
			if ( InternetCanonicalizeUrl( pUrl, buf, &size, 0 ) )
			{	strcpy_sz( m_szUrl, buf ); }

			// Copy the url name
			else strcpy_sz( m_szUrl, pUrl );
		} // end if
		
		// Copy the local file name
		else strcpy_sz( m_szUrl, pUrl );

	} // end copy url	

	// Is it a local file?
	if ( GetFileAttributes( m_szUrl ) != MAXDWORD )
	{
		CWinFile	file;

		if ( file.OpenExisting( m_szUrl, GENERIC_READ ) )
		{
			DWORD size = file.Size();

			if ( m_bMem )
			{
				// Allocate memory
				m_pMem = new BYTE[ size + 1 ];
				if ( m_pMem == NULL ) 
				{	m_dwTransferStatus = NETFILE_DS_ERROR;
					return FALSE;
				} // end if

				// Read in the file
				if ( !file.Read( m_pMem, size, &m_dwDataRead ) )
				{	m_dwTransferStatus = NETFILE_DS_ERROR;
					return FALSE;
				} // end if

				// NULL terminate for good measure
				m_pMem[ size ] = 0;

			} // end if

			else
			{
				// Where to download the file
				if ( pLocal != DOWNLOADTEMP ) { strcpy_sz( m_szLocal, pLocal ); }
				else CWinFile::CreateTemp( m_szLocal );	

				// Copy the file
				CopyFile( m_szUrl, m_szLocal, FALSE );

				// Open the file
				if ( !m_local.OpenExisting( m_szLocal, GENERIC_READ | GENERIC_WRITE ) )
				{	m_dwTransferStatus = NETFILE_DS_ERROR;
					return FALSE;
				} // end if

				// Get the file size
				m_dwDataRead = m_local.Size();

			} // end else

			// Memory read complete
			m_dwTransferStatus = NETFILE_DS_DONE;

			return TRUE;

		} // end if

	} // end if

	if ( !m_bMem )
	{
		// Where to download the file
		if ( pLocal != DOWNLOADTEMP ) { strcpy_sz( m_szLocal, pLocal ); }
		else CWinFile::CreateTemp( m_szLocal );	

		// Create a file to load data
		if ( !m_local.OpenNew( m_szLocal, GENERIC_READ | GENERIC_WRITE ) )
			return FALSE;
	} // end else
	
	// Set status	
	m_dwTransferStatus = NETFILE_DS_INITIALIZING;

	// Create a thread to download the file
	if ( !StartThread() )
	{	Destroy();
		m_dwTransferStatus = NETFILE_DS_ERROR;		
		return FALSE;
	} // end if

	return TRUE;
}

BOOL CNetFile::Open(LPCTSTR pUrl, LPCTSTR pLocal )
{_STT();
	return Download( pUrl, pLocal, FALSE );
}

BOOL CNetFile::InitThread(LPVOID pData)
{_STT();
	// Verify open file
	if ( !m_local.IsOpen() && !m_bMem ) return FALSE;

	// Crack the url
	ZeroMemory( &m_cui, sizeof( m_cui ) );
	CWinFile::CrackUrl( m_szUrl, &m_cui );

	m_dwTransferError = 0;
	
	// Open internet handle
	m_dwTransferStatus = NETFILE_DS_INITIALIZING;

	// Don't know the length
	m_bGetTotalLength = FALSE;
	m_dwTotalLength = 0;

	// Post callback
	DoCallback( (WPARAM)m_dwTransferStatus, (LPARAM)this );

	m_hInternet = InternetOpen(	m_szUserAgent, m_dwAccessFlags,
//								INTERNET_OPEN_TYPE_DIRECT,
//								INTERNET_OPEN_TYPE_PRECONFIG,
								NULL, NULL, 
								m_dwInetFlags );

	if ( m_hInternet == NULL )
	{
		m_dwTransferStatus = NETFILE_DS_ERROR;
		m_dwTransferError = GetLastError();
		DoCallback( (WPARAM)m_dwTransferStatus, (LPARAM)this );
		return FALSE;
	} // end if

	if ( *m_szUsername != 0 || m_sHeaders.size() || m_sData.size() )
	{
		if ( m_bUpload )
		{
			DWORD dwPort = ( m_cui.dwPort != 0 ) ? m_cui.dwPort : INTERNET_DEFAULT_FTP_PORT;
			m_hConnect = InternetConnect(	m_hInternet, m_cui.szHostName, (UINT)dwPort,
											m_szUsername, m_szPassword, INTERNET_SERVICE_FTP,
											m_bPassive ? INTERNET_FLAG_PASSIVE : 0, 0 );
		} // end if
		else
		{
			DWORD dwPort = ( m_cui.dwPort != 0 ) ? m_cui.dwPort : INTERNET_DEFAULT_HTTP_PORT;
			m_hConnect = InternetConnect(	m_hInternet, m_cui.szHostName, (UINT)dwPort,
											m_szUsername, m_szPassword, INTERNET_SERVICE_HTTP,
											0, 0 );
		} // end else
										
		if ( m_hConnect == NULL )
		{
			m_dwTransferStatus = NETFILE_DS_ERROR;
			m_dwTransferError = GetLastError();
			DoCallback( (WPARAM)m_dwTransferStatus, (LPARAM)this );
			return FALSE;
		} // end if

	} // end if

	// Attempt to open the file
	m_dwTransferStatus = NETFILE_DS_CONNECTING;

	// Post callback
	DoCallback( (WPARAM)m_dwTransferStatus, (LPARAM)this );

	if ( !m_bUpload )
	{
		HINTERNET handle = m_hInternet;
		if ( m_hConnect != NULL ) handle = m_hConnect;

		if ( m_sHeaders.size() == 0 && m_sData.size() == 0 )
		{
			m_hFile = InternetOpenUrl(	handle,
										m_szUrl,
										NULL, 0,
										INTERNET_FLAG_TRANSFER_BINARY | m_dwOpenFlags, 
										0 );
			if ( m_hFile == NULL )
			{
				m_dwTransferStatus = NETFILE_DS_ERROR;
				m_dwTransferError = GetLastError();
				DoCallback( (WPARAM)m_dwTransferStatus, (LPARAM)this );
				return FALSE;
			} // end if

		} // end if

		else
		{
			CStr fname = m_cui.szUrlPath;
			
			DWORD dwData = m_sData.strlen();
			LPCTSTR pData = m_sData;
			
			// Add variables to url if GET
			if ( !strcmpi( m_sMethod, "GET" ) )
			{	fname += "?"; fname += m_sData; 
				pData = NULL; dwData = 0;
			} // end if

			// Accept whatever parameters came with the link
			else fname += m_cui.szExtraInfo;

			m_hFile = HttpOpenRequest(	handle, m_sMethod, fname,
										NULL, NULL, NULL, 
										INTERNET_FLAG_TRANSFER_BINARY | m_dwOpenFlags, 
										0 );
			if ( m_hFile == NULL )
			{	m_dwTransferStatus = NETFILE_DS_ERROR;
				m_dwTransferError = GetLastError();
				DoCallback( (WPARAM)m_dwTransferStatus, (LPARAM)this );
				return FALSE;
			} // end if

			if ( !HttpSendRequest( m_hFile, m_sHeaders, m_sHeaders.strlen(),
											(LPVOID)pData, dwData ) )
			{	m_dwTransferStatus = NETFILE_DS_ERROR;
				m_dwTransferError = GetLastError();
				DoCallback( (WPARAM)m_dwTransferStatus, (LPARAM)this );
				return FALSE;
			} // end if

		} // end else

		// Want to know the length
		m_bGetTotalLength = TRUE;

	} // end if

	else
	{
		m_dwDataWritten = 0;

		if ( m_hConnect == NULL )
		{
			m_hFile = InternetOpenUrl(	m_hInternet,
										m_szUrl,
										NULL, 0,
										INTERNET_FLAG_TRANSFER_BINARY | m_dwOpenFlags, 
										0 );
		} // end if
		else
		{
			DWORD i = 0;
			while(	m_cui.szUrlPath[ i ] != 0 &&
					( m_cui.szUrlPath[ i ] == '\\' || m_cui.szUrlPath[ i ] == '/' ) ) i++;

			m_hFile = FtpOpenFile(	m_hConnect, &m_cui.szUrlPath[ i ], 
									GENERIC_WRITE, INTERNET_FLAG_TRANSFER_BINARY, 0 );

		} // end else

		if ( m_hFile == NULL )
		{
			m_dwTransferStatus = NETFILE_DS_ERROR;
			m_dwTransferError = GetLastError();
			DoCallback( (WPARAM)m_dwTransferStatus, (LPARAM)this );
			return FALSE;
		} // end if

		if ( !m_bMem )
		{
			if ( m_pMem != NULL )
			{	delete m_pMem;
				m_pMem = NULL;
			} // end if
			m_dwMemSize = m_local.Size();

			// Anything to upload?
			if ( m_dwMemSize == 0 ) return FALSE;

			// Allocate memory
			m_pMem = new BYTE[ m_dwMemSize + 1 ];
			if ( m_pMem == NULL ) return FALSE;

			// Read in the data
			DWORD read = 0;
			if ( !m_local.Read( m_pMem, m_dwMemSize, &read ) || read != m_dwMemSize )
			{
				m_dwTransferStatus = NETFILE_DS_ERROR;
				m_dwTransferError = GetLastError();
				DoCallback( (WPARAM)m_dwTransferStatus, (LPARAM)this );
				return FALSE;
			} // end if
									
		} // end if	
	
	} // end else

	return TRUE;
}

BOOL CNetFile::DoThread(LPVOID pData)
{_STT();
	if ( !m_bUpload )
	{
		m_dwTransferStatus = NETFILE_DS_DOWNLOADING;

		// Post callback
		DoCallback( (WPARAM)m_dwTransferStatus, (LPARAM)this );

		DWORD ready = 0;

		// Get available data
		if ( !InternetQueryDataAvailable( m_hFile, &ready, 0, 0 ) )
		{	m_dwTransferError = GetLastError();
			return FALSE;
		} // end if

		// Quit if no more data
		if ( ready == 0 ) return FALSE;

		// Get total length if needed
		if ( m_bGetTotalLength )
		{	m_bGetTotalLength = FALSE;

			// Send the query command
			char bufQuery[ 32 ] = "0";
			DWORD dwLengthBufQuery = sizeof( bufQuery );
			DWORD dwHeaderIndex = 0;
			HttpQueryInfo( m_hFile, HTTP_QUERY_CONTENT_LENGTH,
							bufQuery, &dwLengthBufQuery, &dwHeaderIndex );

			// Did we find the header?
			if ( dwHeaderIndex != ERROR_HTTP_HEADER_NOT_FOUND )

				// Convert length from ASCII string to a DWORD.
				m_dwTotalLength = (DWORD)atol( bufQuery );

		} // end if

		while ( ready )
		{
			// Don't byte of more than this computer can chew
			DWORD size = ( ready < m_dwBlockSize ) ? ready : m_dwBlockSize;

			// Allocate a buffer to save new data
			LPBYTE buf = new BYTE[ size + 1 ];
			if ( buf == NULL )
			{
				m_dwTransferError = ERROR_NOT_ENOUGH_MEMORY;
				return FALSE;
			} // end if

			// Attempt to read new data
			DWORD	read = 0;
			if ( !InternetReadFile( m_hFile, buf, size, &read ) )
			{			   
				delete [] buf;
				m_dwTransferError = GetLastError();
				return FALSE;
			} // end if

			// Check for end of file
			if ( read == 0 )
			{
				m_dwTransferStatus = NETFILE_DS_DONE;
				return FALSE;
			} // end if

			if ( m_bMem )
			{
				if ( m_pMem != NULL )
				{
					LPBYTE temp = new BYTE[ m_dwDataRead + size + 1 ];
					if ( temp == NULL )
					{
						m_dwTransferError = ERROR_NOT_ENOUGH_MEMORY;
						return FALSE;
					} // end if
					memcpy( temp, m_pMem, m_dwDataRead );
					memcpy( &temp[ m_dwDataRead ], buf, size );
					temp[ m_dwDataRead + size ] = NULL;
					delete [] m_pMem;
					m_pMem = temp;
				} // end if
				else
				{
					m_pMem = new BYTE[ size + 1 ];
					if ( m_pMem == NULL )
					{
						m_dwTransferError = ERROR_NOT_ENOUGH_MEMORY;
						return FALSE;
					} // end if
					memcpy( m_pMem, buf, size );
					m_pMem[ size ] = NULL;
				} // end else
			} // end if

			// Write the data to the file
			else if ( !m_local.Write( buf, read ) )
			{
				delete [] buf;
				m_dwTransferError = GetLastError();
				return FALSE;
			} // end if

			// Delete buffer
			delete [] buf;

			// Add this to data received
			m_dwDataRead += size;

			// Read the next block
			ready -= size;

		} // end while

	}

	else
	{		
		// Do we have any memory
		if ( m_pMem == NULL || m_dwMemSize == 0 )
		{	m_dwTransferStatus = NETFILE_DS_ERROR;
			return FALSE;
		} // end if

		m_dwTransferStatus = NETFILE_DS_UPLOADING;

		// Post callback
		DoCallback( (WPARAM)m_dwTransferStatus, (LPARAM)this );

		// Don't byte of more than this computer can chew
		DWORD left = m_dwMemSize - m_dwDataWritten;
		DWORD write = ( left < m_dwBlockSize ) ? left : m_dwBlockSize;
		DWORD written = 0;
		
		// Attempt to write some data
		if ( !InternetWriteFile( m_hFile, &m_pMem[ m_dwDataWritten ], write, &written ) )
		{
			m_dwTransferStatus = NETFILE_DS_ERROR;
			m_dwTransferError = GetLastError();
			return FALSE;
		} // end if

		// Count this data
		m_dwDataWritten += written;

		// Have we written all the data?
		if ( m_dwDataWritten == m_dwMemSize )
		{
			m_dwTransferStatus = NETFILE_DS_DONE;
			return FALSE;
		} // end if

	} // end else

	// Wait a bit
	Sleep( 15 );

	// Keep going
	return TRUE; 
}

BOOL CNetFile::EndThread(LPVOID pData)
{_STT();
	if ( m_hFile != NULL ) 
	{
		InternetCloseHandle( m_hFile );
		m_hFile = NULL;
	} // end if

	if ( m_hConnect != NULL ) 
	{
		InternetCloseHandle( m_hConnect );
		m_hConnect = NULL;
	} // end if

	if ( m_hInternet != NULL ) 
	{
		InternetCloseHandle( m_hInternet );
		m_hInternet = NULL;
	} // end if

	// Close file if needed
	if ( m_bCloseFileAfterDownload ) m_local.Close();

	// Set pointer back to start of file
	else m_local.SetPtrPosBegin( 0 );

	// Update download status
	if ( m_dwTransferError != 0 ) m_dwTransferStatus = NETFILE_DS_ERROR;
	m_dwTransferStatus = NETFILE_DS_DONE;

	// Lose upload buffers
	if ( m_bUpload )
	{
		if ( m_pMem != NULL )
		{
			delete m_pMem;
			m_pMem = NULL;
		} // end if
		m_dwMemSize = 0;
	} // end if

	// Post callback
	DoCallback( (WPARAM)m_dwTransferStatus, (LPARAM)this );

	return TRUE;
}

LPCTSTR CNetFile::GetStatus( DWORD status )
{_STT();
	if ( status == 0 ) return "Idle";

	else if ( ( status & NETFILE_DS_INITIALIZING ) != 0 )
		return "Initializing...";

	else if ( ( status & NETFILE_DS_CONNECTING ) != 0 )
		return "Connecting...";

	else if ( ( status & NETFILE_DS_UPLOADING ) != 0 )
		return "Uploading...";

	else if ( ( status & NETFILE_DS_DOWNLOADING ) != 0 )
		return "Downloading...";

	else if ( ( status & NETFILE_DS_ERROR ) != 0 )
		return "Error";

	else if ( ( status & NETFILE_DS_DONE ) != 0 )
		return "Complete";

	else return "Unknown State";

}

void CNetFile::Close()
{_STT();
	Destroy();
}


BOOL CNetFile::Upload(LPCTSTR pUrl, LPCTSTR pLocal, BOOL bPassive)
{_STT();
	// Stop whatever else we were doing
	if ( pLocal != NULL ) Destroy();

	// Uploading file
	m_bUpload = TRUE;
	m_dwDataWritten = 0;
	m_bPassive = bPassive;
	
	// Save parent window
	m_hWndParent = NULL;

	// Sanity check
	if ( pUrl == NULL || *pUrl == NULL ) return FALSE;

	// Are we downloading to file or ram?
	m_bMem = ( pLocal == NULL );

	{ // Copy the url
		
		char buf[ sizeof( m_szUrl ) ];
		DWORD size = sizeof( m_szUrl ) - 1;

		// Is it a local file?
		if ( GetFileAttributes( pUrl ) == MAXDWORD )
		{
			// Fix the url
			if ( InternetCanonicalizeUrl( pUrl, buf, &size, 0 ) )
			{ strcpy_sz( m_szUrl, buf ); }

			// Copy the url name
			else strcpy_sz( m_szUrl, pUrl );
		} // end if
		
		// Copy the local file name
		else strcpy_sz( m_szUrl, pUrl );

	} // end copy url	

	if ( !m_bMem )
	{
		// Where to download the file
		if ( pLocal != DOWNLOADTEMP ) { strcpy_sz( m_szLocal, pLocal ); }
		else CWinFile::CreateTemp( m_szLocal );	

		// Create a file to load data
		if ( !m_local.OpenExisting( m_szLocal, GENERIC_READ | GENERIC_WRITE ) )
			return FALSE;
	} // end else

	// Set status	
	m_dwTransferStatus = NETFILE_DS_INITIALIZING;

	// Create a thread to download the file
	if ( !StartThread() )
	{	Destroy();
		m_dwTransferStatus = NETFILE_DS_INITIALIZING;
		return FALSE;
	} // end if

	return TRUE;
}

BOOL CNetFile::SetOption(DWORD option, LPVOID buf, DWORD size)
{_STT();
	return InternetSetOption( m_hInternet, option, buf, size );
}

BOOL CNetFile::GetErrorMsg(LPSTR msg, DWORD err)
{_STT();
	char *ptr = NULL;

	switch( err )
	{
		case ERROR_INTERNET_OUT_OF_HANDLES : ptr = "Out of handles"; break;
		case ERROR_INTERNET_TIMEOUT : ptr = "Operation timed out"; break;
		case ERROR_INTERNET_EXTENDED_ERROR : ptr = "Extended error"; break;
		case ERROR_INTERNET_INTERNAL_ERROR : ptr = "Internal Internet error"; break;
		case ERROR_INTERNET_INVALID_URL : ptr = "Invalid URL"; break;
		case ERROR_INTERNET_UNRECOGNIZED_SCHEME : ptr = "Unrecognized communication scheme"; break;
		case ERROR_INTERNET_NAME_NOT_RESOLVED : ptr = "Host name not resolved"; break;
		case ERROR_INTERNET_PROTOCOL_NOT_FOUND : ptr = "Protocol not found"; break;
		case ERROR_INTERNET_INVALID_OPTION : ptr = "Invalid option"; break;
		case ERROR_INTERNET_BAD_OPTION_LENGTH : ptr = "Bad option length"; break;
		case ERROR_INTERNET_OPTION_NOT_SETTABLE : ptr = "Option not setable"; break;
		case ERROR_INTERNET_SHUTDOWN : ptr = "Connection was shutdown"; break;
		case ERROR_INTERNET_INCORRECT_USER_NAME : ptr = "Invalid Username"; break;
		case ERROR_INTERNET_INCORRECT_PASSWORD : ptr = "Invalid Password"; break;
		case ERROR_INTERNET_LOGIN_FAILURE : ptr = "Login failed"; break;
		case ERROR_INTERNET_INVALID_OPERATION : ptr = "Invalid operation"; break;
		case ERROR_INTERNET_OPERATION_CANCELLED : ptr = "Operation canceled"; break;
		case ERROR_INTERNET_INCORRECT_HANDLE_TYPE : ptr = "Incorrect handle type"; break;
		case ERROR_INTERNET_INCORRECT_HANDLE_STATE : ptr = "Incorrect handle state"; break;
		case ERROR_INTERNET_NOT_PROXY_REQUEST : ptr = "Not proxy request"; break;
		case ERROR_INTERNET_REGISTRY_VALUE_NOT_FOUND : ptr = "Registry value not found"; break;
		case ERROR_INTERNET_BAD_REGISTRY_PARAMETER : ptr = "Bad registry parameter"; break;
		case ERROR_INTERNET_NO_DIRECT_ACCESS : ptr = "No direct access to data"; break;
		case ERROR_INTERNET_NO_CONTEXT : ptr = "No context"; break;
		case ERROR_INTERNET_NO_CALLBACK : ptr = "No callback"; break;
		case ERROR_INTERNET_REQUEST_PENDING : ptr = "Request pending"; break;
		case ERROR_INTERNET_INCORRECT_FORMAT : ptr = "Incorrect format"; break;
		case ERROR_INTERNET_ITEM_NOT_FOUND : ptr = "Item not found"; break;
		case ERROR_INTERNET_CANNOT_CONNECT : ptr = "Unable to connect to server"; break;
		case ERROR_INTERNET_CONNECTION_ABORTED : ptr = "Connection aborted"; break;
		case ERROR_INTERNET_CONNECTION_RESET : ptr = "Connection reset by server"; break;
		case ERROR_INTERNET_FORCE_RETRY : ptr = "Force retry"; break;
		case ERROR_INTERNET_INVALID_PROXY_REQUEST : ptr = "Invalid proxy request"; break;
		case ERROR_INTERNET_NEED_UI : ptr = "Need user input"; break;

		default : ptr = NULL;

	} // end switch

	if ( ptr == NULL ) 
	{
		strcpy( msg, "Unknown Internet Error" );
		return FALSE;
	} // end if

	// Copy the message
	strcpy( msg, ptr );

	return TRUE;
}

BOOL CNetFile::SetDataPtr(LPBYTE buf, DWORD size)
{_STT();
	// Sanity check
	if ( buf == NULL || size == 0 ) return FALSE;

	// Allocate a block of memory
	m_pMem = new BYTE[ size + 1 ];
	if ( m_pMem == NULL ) return FALSE;

	// Copy the data
	memcpy( m_pMem, buf, size );

	m_dwMemSize = size;
	m_bMem = TRUE;

	return TRUE;
}

BOOL CNetFile::Upload(LPCTSTR pUrl, LPBYTE buf, DWORD size, BOOL bPassive)
{_STT();
	if ( !SetDataPtr( buf, size ) ) return FALSE;
	return Upload( pUrl, NULL, bPassive );
}


BOOL CNetFile::HttpRequest(LPCTSTR pUrl, CRKey *pHeaders, CRKey *pData, LPCTSTR pMethod, LPCTSTR pLocal, BOOL bCloseFileAfterDownload, HWND hWndParent, DWORD dwUrlEncoding)
{_STT();
	// Lose previous file
	Destroy();

	// Downloading
	m_bUpload = FALSE;

	// Save parent window
	m_hWndParent = hWndParent;

	// Save close file status
	m_bCloseFileAfterDownload = bCloseFileAfterDownload;

	// Sanity check
	if ( pUrl == NULL || *pUrl == NULL ) return FALSE;

	// Are we downloading to file or ram?
	m_bMem = ( pLocal == NULL );

	{ // Copy the url
		
		char buf[ sizeof( m_szUrl ) ] = { 0 };
		DWORD size = sizeof( m_szUrl ) - 1;

		// Is it a local file?
		if ( GetFileAttributes( pUrl ) == MAXDWORD )
		{
			// Fix the url
			if ( InternetCanonicalizeUrl( pUrl, buf, &size, 0 ) )
			{	strcpy_sz( m_szUrl, buf ); }

			// Copy the url name
			else strcpy_sz( m_szUrl, pUrl );
		} // end if
		
		// Copy the local file name
		else strcpy_sz( m_szUrl, pUrl );

	} // end copy url	

	if ( !m_bMem )
	{
		// Where to download the file
		if ( pLocal != DOWNLOADTEMP ) { strcpy_sz( m_szLocal, pLocal ); }
		else CWinFile::CreateTemp( m_szLocal );	

		// Create a file to load data
		if ( !m_local.OpenNew( m_szLocal, GENERIC_READ | GENERIC_WRITE ) )
			return FALSE;
	} // end else
	
	// Set status	
	m_dwTransferStatus = NETFILE_DS_INITIALIZING;

	// Save request method
	if ( pMethod == NULL || *pMethod == 0 ) m_sMethod = "POST";
	else m_sMethod = pMethod;

	// Set data
	DWORD dwContentLength = 0;
	if ( pData != NULL )
	{	CPipe pipe;
		pData->EncodeUrl( &pipe, dwUrlEncoding );
		m_sData.copy( (LPCTSTR)pipe.GetBuffer(), pipe.GetBufferSize() );
		dwContentLength = pipe.GetBufferSize();
	} // end if

	// Set headers
	if ( strcmpi( m_sMethod, "GET" ) )
	{
		CRKey rkHeaders;
		if ( pHeaders != NULL ) rkHeaders.Copy( pHeaders );
		
		rkHeaders.Set( "Content-Type", "application/x-www-form-urlencoded" );
		rkHeaders.Set( "Content-Length", dwContentLength );

		CPipe pipe;
		rkHeaders.EncodeHttpHeaders( &pipe );
		m_sHeaders.copy( (LPCTSTR)pipe.GetBuffer(), pipe.GetBufferSize() );

	} // end if

	// Create a thread to download the file
	if ( !StartThread() )
	{	Destroy();
		m_dwTransferStatus = NETFILE_DS_ERROR;		
		return FALSE;
	} // end if

	return TRUE;
}
