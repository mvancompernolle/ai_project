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
// PFile.cpp: implementation of the CPFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPFile::CPFile()
{

}

CPFile::CPFile(LPCTSTR pFile, DWORD access, BOOL bNew)
{
	if ( bNew ) m_file.OpenNew( pFile, access );
	else m_file.OpenExisting( pFile, access );
}

CPFile::CPFile(LPCTSTR pFile, LPCTSTR pKey, DWORD access, BOOL bNew)
{
	// Set encryption key
	if ( pKey != NULL ) m_file.CryptoSetKey( pKey );

	if ( bNew ) m_file.OpenNew( pFile, access );
	else m_file.OpenExisting( pFile, access );
}

CPFile::~CPFile()
{

}

BOOL CPFile::ProcessWrite( LPVOID buf, DWORD size, LPDWORD written )
{	return m_file.Write( buf, size, written );
}


BOOL CPFile::ProcessRead( LPVOID buf, DWORD size, LPDWORD read )
{	return m_file.Read( buf, size, read );
}

