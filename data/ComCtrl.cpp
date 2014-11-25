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
// ComCtrl.cpp: implementation of the CComCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include <shellapi.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CComCtrl::CComCtrl()
{_STT();

}

CComCtrl::~CComCtrl()
{_STT();

}

int CComCtrl::ListGetFirstSelectedItem(HWND hListCtrl)
{_STT();
	if ( !IsWindow( hListCtrl ) ) return -1;

	// Get listbox count
	int count = SendMessage( hListCtrl, LVM_GETITEMCOUNT, 0, 0L );
	if ( count == 0 ) return -1;

	// Find first selected item
	int sel = 0;
	while ( sel < count && 
			SendMessage( hListCtrl, LVM_GETITEMSTATE, sel, LVIS_SELECTED ) 
				!= LVIS_SELECTED )
		sel++;

	// Is anything selected
	if ( sel == count ) return -1;

	return sel;
}

int CComCtrl::ComboGetItemByData(HWND hWnd, DWORD data)
{_STT();
	if ( !IsWindow( hWnd ) ) return CB_ERR;

	// Get item count
	int count = SendMessage( hWnd, CB_GETCOUNT, 0, 0L );
	if ( count == 0 ) return CB_ERR;

	// Check each item
	for ( int i = 0; i < count; i++ )
		if ( data == (DWORD)SendMessage( hWnd, CB_GETITEMDATA, i, 0L ) )
			return i;

	return CB_ERR;
}

int CComCtrl::ComboSelectItemByData(HWND hWnd, DWORD data)
{_STT();
	int i = ComboGetItemByData( hWnd, data );
	if ( i == CB_ERR ) return CB_ERR;

	SendMessage( hWnd, CB_SETCURSEL, i, 0L );

	return i;
}

HTREEITEM CComCtrl::TreeInsertItem(HWND hTree, HTREEITEM hParent, LPCTSTR pText, DWORD dwImage, DWORD dwUser, BOOL bExpand, HTREEITEM hInsertAfter, UINT uMask)
{_STT();
	HTREEITEM			hItem = NULL;
	TV_INSERTSTRUCT		tv;

	tv.item.pszText = (char*)pText;
	tv.item.mask = uMask;
	tv.item.state = 0;	
	tv.item.iImage = dwImage;
	tv.item.iSelectedImage = dwImage;
	tv.item.lParam = dwUser;
	tv.hInsertAfter = hInsertAfter;
	
	if ( hParent != NULL ) tv.hParent = hParent;
	else tv.hParent = TVI_ROOT;
	
	if ( IsWindow( hTree ) )
		hItem = (HTREEITEM)SendMessage( hTree, TVM_INSERTITEM, 0, (LPARAM)&tv );

	// Start expanded
	if ( bExpand && hParent != NULL )
		SendMessage( hTree, TVM_EXPAND, TVE_EXPAND, (LPARAM)hParent );

	return hItem;

}

DWORD CComCtrl::TreeGetItemData(HWND hTree, HTREEITEM hItem)
{_STT();
	if ( !IsWindow( hTree ) ) return 0;

	TV_ITEM tv;

	tv.hItem = hItem;
	tv.mask = TVIF_HANDLE | TVIF_PARAM;

	if ( !SendMessage( hTree, TVM_GETITEM, 0, (LPARAM)&tv ) )
		return 0;

	return tv.lParam;	
}

DWORD CComCtrl::ListGetFileIcon(LPCTSTR pFile, HWND hWnd, BOOL bSmall, BOOL bLarge)
{_STT();
	// Sanity checks
	if ( pFile == NULL ) return 0;

	SHFILEINFO	sfi;
	HIMAGELIST	hImgList;

	char filetype[ CWF_STRSIZE ];
	const char *ext = CWinFile::GetExtension( pFile );
	strcpy( filetype, "file." );
	if ( ext != NULL ) strcat( filetype, ext );

	// Only if there is a list control
	if ( bLarge )
	{
		// Get large icon for this file type
		hImgList = 
			(HIMAGELIST)SHGetFileInfo(	pFile, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( sfi ),
										SHGFI_ICON | 
										SHGFI_SYSICONINDEX | SHGFI_LARGEICON );
		if ( hImgList == NULL ) hImgList = 
			(HIMAGELIST)SHGetFileInfo(	filetype, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( sfi ),
										SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | 
										SHGFI_SYSICONINDEX | SHGFI_LARGEICON );

		// Set image list in control if needed
		if ( hImgList != NULL && ::IsWindow( hWnd ) )
			::SendMessage( hWnd, LVM_SETIMAGELIST, LVSIL_NORMAL, (LPARAM)hImgList );

	} // end if

	if ( bSmall )
	{
		// Get small icon for this file type
		hImgList = 
			(HIMAGELIST)SHGetFileInfo(	pFile, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( sfi ),
										SHGFI_ICON | 
										SHGFI_SYSICONINDEX | SHGFI_SMALLICON );
		if ( hImgList == NULL ) hImgList = 
			(HIMAGELIST)SHGetFileInfo(	filetype, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( sfi ),
										SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | 
										SHGFI_SYSICONINDEX | SHGFI_SMALLICON );

		// Set image list in control if needed
		if ( hImgList != NULL && ::IsWindow( hWnd ) )
			::SendMessage( hWnd, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)hImgList );

	} // end if

	// Return the index
	return sfi.iIcon;
}

DWORD CComCtrl::TreeGetFileIcon(LPCTSTR pFile, HWND hWnd, BOOL bSmall, BOOL bLarge)
{_STT();
	// Sanity checks
	if ( pFile == NULL ) return 0;

	SHFILEINFO	sfi;
	HIMAGELIST	hImgList;

	char filetype[ CWF_STRSIZE ];
	const char *ext = CWinFile::GetExtension( pFile );
	strcpy( filetype, "file." );
	if ( ext != NULL ) strcat( filetype, ext );

	char fname[ CWF_STRSIZE ];
	CWinFile::GetFileNameFromPath( pFile, fname );
	if ( *fname == 0 || !strcmpi( pFile, fname ) )
	{	if (	strncmp( pFile, "http://", 7 ) ||
				strncmp( pFile, "ftp://", 6 ) ||
				strncmp( pFile, "www.", 4 ) ||
				strncmp( pFile, "ftp.", 4 ) )
			strcpy( filetype, "file.htm" );
	} // end if
	

	if ( bSmall )
	{
		// Get small icon for this file type
		hImgList = 
			(HIMAGELIST)SHGetFileInfo(	pFile, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( sfi ),
										SHGFI_ICON | 
										SHGFI_SYSICONINDEX | SHGFI_SMALLICON );
		if ( hImgList == NULL ) hImgList = 
			(HIMAGELIST)SHGetFileInfo(	filetype, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( sfi ),
										SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | 
										SHGFI_SYSICONINDEX | SHGFI_SMALLICON );

		// Set image list in control if needed
		if ( hImgList != NULL && ::IsWindow( hWnd ) )
			::SendMessage( hWnd, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)hImgList );

	} // end if

	
	else if ( bLarge )
	{
		// Get large icon for this file type
		hImgList = 
			(HIMAGELIST)SHGetFileInfo(	pFile, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( sfi ),
										SHGFI_ICON | 
										SHGFI_SYSICONINDEX | SHGFI_LARGEICON );
		if ( hImgList == NULL ) hImgList = 
			(HIMAGELIST)SHGetFileInfo(	filetype, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof( sfi ),
										SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | 
										SHGFI_SYSICONINDEX | SHGFI_LARGEICON );

		// Set image list in control if needed
		if ( hImgList != NULL && ::IsWindow( hWnd ) )
			::SendMessage( hWnd, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)hImgList );

	} // end if

	// Return the index
	return sfi.iIcon;
}

BOOL CComCtrl::TreeGetItemPath(HWND hTree, HTREEITEM hItem, LPSTR pPath, DWORD dwDepth )
{_STT();
	if ( hItem == NULL ) return FALSE;

	// Get the item label
	char str[ CWF_STRSIZE ];
	if ( !TreeGetItemText( hTree, hItem, str, sizeof( str ) ) ) return FALSE;

	strcpy( pPath, str );

	HTREEITEM hParent = hItem;
	while( ( hParent = TreeView_GetParent( hTree, hParent ) ) != NULL )
	{
		// Are we stopping here?
		if ( dwDepth > 0 && TreeView_GetParent( hTree, hParent ) == NULL )
			return TRUE;

		// Get text
		if ( TreeGetItemText( hTree, hParent, str, sizeof( str ) ) )

			// Build path
			CWinFile::BuildPath( pPath, str, pPath );

	} // end while

	return TRUE;
}

BOOL CComCtrl::TreeGetItemText(HWND hTree, HTREEITEM hItem, LPSTR pText, DWORD dwSize)
{_STT();
	// Sanity check
	if ( pText == NULL ) return FALSE;

	TV_ITEM	tv;

	tv.hItem = hItem;
	tv.mask = TVIF_HANDLE | TVIF_TEXT;
	tv.pszText = pText;
	tv.cchTextMax = dwSize;

	*pText = 0;

	// Get item data
	if ( !TreeView_GetItem( hTree, &tv ) )
		return FALSE;

	return TRUE;	
}

BOOL CComCtrl::TreeValidateDirItem(HWND hTree, HTREEITEM hItem)
{_STT();
	WIN32_FIND_DATA	fd;
	char path[ CWF_STRSIZE ];

	if ( TreeView_GetParent( hTree, hItem ) != NULL )
	{
		// Validate this item
		TreeGetItemPath( hTree, hItem, path );

		HANDLE	hFind = FindFirstFile( path, &fd );
		FindClose( hFind );
		if ( hFind == INVALID_HANDLE_VALUE )
		{	TreeView_DeleteItem( hTree,hItem );
			return FALSE;
		} // end if
	} // end if
	
	// Validate children
	HTREEITEM hChild = TreeView_GetChild( hTree, hItem );
	HTREEITEM hDel[ 1024 ];
	DWORD		ptr = 0;
	// Valid if no children
	if ( hChild == NULL ) return TRUE;
	do
	{
		TreeGetItemPath( hTree, hChild, path );
		HANDLE	hFind = FindFirstFile( path, &fd );
		FindClose( hFind );
		if ( hFind == INVALID_HANDLE_VALUE )
			hDel[ ptr++ ] = hChild;
	} while (	( hChild = TreeView_GetNextSibling( hTree, hChild ) ) != NULL &&
				ptr < 1024 ); // Yeah, get real

	// Can't delete these while calling GetNextSiblingITem()
	if ( ptr ) while( ptr-- )
	{
		TreeView_DeleteItem( hTree, hDel[ ptr ] );
	} // end if

	return TRUE;
}

BOOL CComCtrl::TreeFillDirectory( HWND hTree, HTREEITEM hItem, LPCTSTR pPath)
{_STT();
	WIN32_FIND_DATA	fd;

	char str[ CWF_STRSIZE ];
	char search[ CWF_STRSIZE ];
	strcpy( search, pPath );
	if ( search[ strlen( search ) - 1 ] != '\\' )
		strcat( search,"\\" );
	strcat( search, "*.*" );

	HANDLE	hFind = FindFirstFile( search, &fd );

	if ( hFind == INVALID_HANDLE_VALUE ) return FALSE;

	do
	{
		// Add all the directorys into the tree
		if (	fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
				strcmpi( fd.cFileName, "." ) && strcmpi( fd.cFileName, ".." ) )
		{
			char full[ CWF_STRSIZE ];
			CWinFile::BuildPath( full, pPath, fd.cFileName );

			// See if it is already in the tree
			BOOL bExist = FALSE;
			HTREEITEM hChild = TreeView_GetChild( hTree, hItem );
			if ( hChild != NULL )
				do
				{
					TreeGetItemText( hTree, hChild, str, sizeof( str ) );
					if ( !strcmpi( str, fd.cFileName ) )
						bExist = TRUE;
				} while ( !bExist && ( hChild = TreeView_GetNextSibling( hTree, hChild ) ) != NULL );

			if ( !bExist ) TreeInsertItem( hTree, hItem, fd.cFileName, TreeGetFileIcon( full, hTree ) );

		} // end if

	} while( FindNextFile( hFind, &fd ) );

	// Close the find
	FindClose( hFind );

	return TRUE;
}

BOOL CComCtrl::TreeSetItemImage(HWND hTree, HTREEITEM hItem, DWORD dwImage)
{_STT();
	TV_ITEM	tv;

	tv.hItem = hItem;
	tv.iImage = dwImage;
	tv.iSelectedImage = dwImage;
	tv.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

	// Get item data
	if ( !SendMessage( hTree, TVM_SETITEM, 0, (LPARAM)&tv ) )
		return FALSE;

	return TRUE;	
}

BOOL CComCtrl::TreeFillSystem(HWND hTree, HTREEITEM hRoot)
{_STT();
	char		drives[ CWF_STRSIZE ];
	char		*drive = drives;
	HTREEITEM	hItem;

	// Find out what drives are available
	if ( !GetLogicalDriveStrings( CWF_STRSIZE - 1, drives ) )
		return FALSE;

	while ( *drive != 0x0 )
	{
		// What type of drive is this?
		DWORD type = GetDriveType( drive );

		// Insert the item
		hItem = TreeInsertItem( hTree, hRoot, drive, TreeGetFileIcon( drive, hTree ) );

		switch( type )
		{
			case DRIVE_NO_ROOT_DIR :
			case DRIVE_FIXED :
			case DRIVE_RAMDISK :

				// Refresh the item
				TreeRefreshDirItem( hTree, hItem );

				break;

		} // end switch

		// Find the NULL + 1
		while ( *drive ) drive++; drive++;

	} // end while

	return TRUE;
}

BOOL CComCtrl::TreeRefreshDirItem(HWND hTree, HTREEITEM hItem)
{_STT();
	char path[ CWF_STRSIZE ];

	// Get the path for the item
	if ( !TreeGetItemPath( hTree, hItem, path ) ) return FALSE;

	// Fill in its directory
	TreeFillDirectory( hTree, hItem, path );

	// And its directories directories
	HTREEITEM hChild = TreeView_GetChild( hTree, hItem );
	if ( hChild != NULL )
		do
		{
			if ( TreeGetItemPath( hTree, hChild, path ) )
				TreeFillDirectory( hTree, hChild, path );
		} while ( ( hChild = TreeView_GetNextSibling( hTree, hChild ) ) != NULL );

	TreeValidateDirItem( hTree, hItem );

	return TRUE;
}

HTREEITEM CComCtrl::TreeInsertPath(HWND hTree, HTREEITEM hItem, LPCTSTR pPath, DWORD dwFolder, DWORD dwIcon)
{_STT();
	DWORD i = 0, left = 0;
	char name[ CWF_STRSIZE ];

	// For each path element
	while ( CWinFile::GetPathElement( name, pPath, i++, &left ) )
	{
		// Add path token
		if ( left ) 
		{
			HTREEITEM hNext = TreeFindItemByName( hTree, name, hItem );
			if ( hNext == NULL )
			{
				// Find the last folder item
				int iImage = 0;
				HTREEITEM hAfter = TVI_FIRST; 
				HTREEITEM hChild = TreeView_GetChild( hTree, hItem );
				do
				{
					iImage = TreeGetItemImage( hTree, hChild );
					if ( iImage == (int)dwFolder ) 
					{	hAfter = hChild;
						hChild = TreeView_GetNextSibling( hTree, hChild );
					} // end if

				} while ( iImage == (int)dwFolder && hChild != NULL );

				// Insert after all other folders
				hNext = TreeInsertItem( hTree, hItem, name, dwFolder, 0, FALSE, hAfter );

			} // end if
			hItem = hNext;
		} // end if

		// Add final token
		else
		{
			// Just return if it already exists
			HTREEITEM hFind = TreeFindItemByName( hTree, name, hItem );
			if ( hFind ) return hFind;

			// Add new item
			return TreeInsertItem( hTree, hItem, name, dwIcon );

		} // end else

	} // end while

	return NULL;
}

HTREEITEM CComCtrl::TreeFindItemByName(HWND hTree, LPCTSTR pName, HTREEITEM hRoot)
{_STT();
	char str[ CWF_STRSIZE ];
	HTREEITEM hChild = TreeView_GetChild( hTree, hRoot );

	if ( hChild != NULL )
		do
		{
			// Is this the one?
			if (	TreeGetItemText( hTree, hChild, str, sizeof( str ) ) &&
					!strcmpi( pName, str ) ) return hChild;

		} while ( ( hChild = TreeView_GetNextSibling( hTree, hChild ) ) != NULL );

	return NULL;
}

HTREEITEM CComCtrl::TreeFindItemByPath(HWND hTree, LPCTSTR pPath, HTREEITEM hRoot, BOOL bDisk )
{_STT();
	DWORD i = 0, left = 0;
	char name[ CWF_STRSIZE ];

	// Refresh the root item if disk
	if ( bDisk ) TreeRefreshDirItem( hTree, hRoot );

	// For each path element
	while ( CWinFile::GetPathElement( name, pPath, i++, &left ) )
	{
		// Add back slash to drive letter
		if ( name[ strlen( name ) - 1 ] == ':' )
			strcat( name, "\\" );

		// Find this item
		hRoot = TreeFindItemByName( hTree, name, hRoot );

		// Punt if not found
		if ( hRoot == NULL ) return FALSE;

		// Refresh this item if disk
		if ( bDisk ) TreeRefreshDirItem( hTree, hRoot );

	} // end while

	return hRoot;
}


HTREEITEM CComCtrl::TreeSelectByPath(HWND hTree, LPCTSTR pPath)
{_STT();
	HTREEITEM hItem = TreeFindItemByPath( hTree, pPath );
	if ( hItem == NULL ) return FALSE;
	
	// Select the item
	TreeView_SelectItem( hTree, hItem );

	// Ensure it's visible
	TreeView_Expand( hTree, hItem, TVE_EXPAND );
	TreeView_EnsureVisible( hTree, hItem );

	return hItem;
}

int CComCtrl::TreeGetItemImage(HWND hTree, HTREEITEM hItem)
{_STT();
	if ( !IsWindow( hTree ) ) return 0;

	TV_ITEM tv;

	tv.hItem = hItem;
	tv.mask = TVIF_IMAGE | TVIF_PARAM;

	if ( !SendMessage( hTree, TVM_GETITEM, 0, (LPARAM)&tv ) )
		return 0;

	return tv.iImage;	
}

long CComCtrl::ListGetFullWidth(HWND hList)
{_STT();
	RECT rect;
	::GetClientRect( hList, &rect );
	return ( ( rect.right - rect.left ) - GetSystemMetrics( SM_CXVSCROLL ) - 4 );
}

BOOL _cdecl CComCtrl::ListSetColumns( HWND hList, DWORD dwCols, ... )
{_STT();
	long w = ListGetFullWidth( hList );

	RULIB_TRY // This could GPF if caller screws up
	{
		LPCTSTR *ptr = ( (LPCTSTR*)&dwCols ) + 1;

		// For each column
		for ( DWORD i = 0; i < dwCols; i++ )
		{
			// Column info
			LVCOLUMN lvc;
			lvc.mask = LVCF_TEXT | LVCF_WIDTH;
			lvc.cx = w / dwCols;
			lvc.pszText = (LPSTR)*ptr; ptr++;

			// Insert the column
			SendMessage( hList, LVM_INSERTCOLUMN, i, (LPARAM)&lvc );

		} // end for

	} // end try
	RULIB_CATCH_ALL { return FALSE; }

	return TRUE;
}

int CComCtrl::ListFindItemByText(HWND hList, int nSub, LPCTSTR pText)
{_STT();
	// Get listbox count
	int count = SendMessage( hList, LVM_GETITEMCOUNT, 0, 0L );
	if ( count == 0 ) return -1;

	int		sel = 0;
	LVITEM	lvi;
	char	text[ CWF_STRSIZE ];
	
	while ( sel < count )
	{		
		lvi.iItem = sel;
		lvi.iSubItem = nSub;
		lvi.mask = LVIF_TEXT;
		lvi.pszText = text;
		lvi.cchTextMax = sizeof( text );
		
		*text = 0;
		SendMessage( hList, LVM_GETITEMTEXT, sel, (LPARAM)&lvi );

		// Return this item if text matches
		if ( !strcmpi( text, pText ) ) return sel;

		sel++;

	} // end while

	return -1;
}

BOOL CComCtrl::ListSelectSingleItem(HWND hList, int item)
{_STT();
	// Get listbox count
	int count = SendMessage( hList, LVM_GETITEMCOUNT, 0, 0L );
	if ( count == 0 ) return FALSE;

	int		sel = 0;
	LVITEM	li;
	ZeroMemory( &li, sizeof( li ) );

	while ( sel < count )
	{
		if ( sel == item ) 
		{	ListView_SetItemState( hList, sel, LVIS_SELECTED, LVIS_SELECTED ); }
		else { ListView_SetItemState( hList, sel, 0, LVIS_SELECTED ); }

		sel++;
	} // end while

	return TRUE;
}

BOOL CComCtrl::EnableCtrl(HWND hWnd, UINT uCtrl, BOOL bEnable)
{_STT();
	if ( !IsWindow( hWnd ) ) return FALSE;
	HWND hCtrl = GetDlgItem( hWnd, uCtrl );
	if ( hCtrl == NULL || !IsWindow( hCtrl ) ) return FALSE;
	return EnableWindow( hCtrl, bEnable );
}

long CComCtrl::OnScroll(HWND hCtrl, UINT nSBCode, BOOL bRedraw, LONG lLine)
{_STT();
	// Get scroll bar info
	SCROLLINFO si; 
	si.cbSize = sizeof( SCROLLINFO );
	si.fMask = SIF_ALL;
	if ( !::SendMessage( hCtrl, SBM_GETSCROLLINFO, 0, (LPARAM)&si ) )
		return MAXLONG;

	switch( nSBCode )
	{
		case SB_BOTTOM :
			si.fMask = SIF_POS; si.nPos = si.nMax;
			::SendMessage( hCtrl, SBM_SETSCROLLINFO, bRedraw, (LPARAM)&si );
			break;

		case SB_LINEDOWN :
			si.fMask = SIF_POS; si.nPos = si.nPos + lLine;
			::SendMessage( hCtrl, SBM_SETSCROLLINFO, bRedraw, (LPARAM)&si );
			break;

		case SB_LINEUP :
			si.fMask = SIF_POS; si.nPos = si.nPos - lLine;
			::SendMessage( hCtrl, SBM_SETSCROLLINFO, bRedraw, (LPARAM)&si );
			break;

		case SB_PAGEDOWN :
			si.fMask = SIF_POS; si.nPos = si.nPos + si.nPage;
			::SendMessage( hCtrl, SBM_SETSCROLLINFO, bRedraw, (LPARAM)&si );
			break;

		case SB_PAGEUP :
			si.fMask = SIF_POS; si.nPos = si.nPos - si.nPage;
			::SendMessage( hCtrl, SBM_SETSCROLLINFO, bRedraw, (LPARAM)&si );
			break;

		case SB_THUMBPOSITION :
			si.fMask = SIF_POS; si.nPos = si.nTrackPos;
			::SendMessage( hCtrl, SBM_SETSCROLLINFO, bRedraw, (LPARAM)&si );
			break;

		case SB_THUMBTRACK :
			si.fMask = SIF_POS; si.nPos = si.nTrackPos;
			::SendMessage( hCtrl, SBM_SETSCROLLINFO, bRedraw, (LPARAM)&si );
			break;

		case SB_TOP :
			si.fMask = SIF_POS; si.nPos = si.nMin;
			::SendMessage( hCtrl, SBM_SETSCROLLINFO, bRedraw, (LPARAM)&si );
			break;

		default : return MAXLONG; break;

	} // end switch

	return si.nPos;
}

int CComCtrl::ListGetItemByData(HWND hWnd, DWORD data)
{_STT();
	if ( !IsWindow( hWnd ) ) return LB_ERR;

	// Get item count
	int count = ListView_GetItemCount( hWnd );
	if ( count == 0 ) return LB_ERR;

	// Check each item
	for ( int i = 0; i < count; i++ )
		if ( data == ListGetItemData( hWnd, i ) )
			return i;

	return LB_ERR;
}

DWORD CComCtrl::ListGetItemData(HWND hWnd, int item)
{_STT();
	if ( !IsWindow( hWnd ) ) return 0;

	LVITEM	lvi;
	lvi.iItem = item;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_PARAM;
	lvi.lParam = 0;

	ListView_GetItem( hWnd, &lvi );

	return lvi.lParam;
}

int CComCtrl::ListGetItemImage(HWND hWnd, int item)
{_STT();
	if ( !IsWindow( hWnd ) ) return 0;

	LVITEM	lvi;
	lvi.iItem = item;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE;
	lvi.iImage = -1;

	ListView_GetItem( hWnd, &lvi );

	return lvi.iImage;
}

BOOL CComCtrl::ListSetItemImage(HWND hWnd, int item, int iImage)
{_STT();
	if ( !IsWindow( hWnd ) ) return 0;

	LVITEM	lvi;
	lvi.iItem = item;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE;
	lvi.iImage = iImage;

	return 0 == ListView_SetItem( hWnd, &lvi );
}

BOOL CComCtrl::PosWindow(HWND hWnd, LPRECT pRect, BOOL bShow, BOOL bRepaint)
{_STT();
	if ( !IsWindow( hWnd ) ) return FALSE;

	// Verify window size
	if ( PRW( pRect ) <= 0 ) bShow = FALSE;
	else if ( PRH( pRect ) <= 0 ) bShow = FALSE;

	// Position the window
	if ( bShow )
		::MoveWindow( hWnd, pRect->left, pRect->top, 
							PRW( pRect ), PRH( pRect ), bRepaint );				

	// Ensure proper visibility
	if ( ::IsWindowVisible( hWnd ) )
	{	if ( !bShow ) ShowWindow( hWnd, SW_HIDE ); }
	else if ( bShow ) ShowWindow( hWnd, SW_SHOWNORMAL );

	return TRUE;
}
