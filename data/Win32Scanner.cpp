/*
 * Copyright (C) 2004  Norbert Drees, Norbert.Drees@GMX.de
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <windows.h>

#include "ScannerFunctions.h"
#include "Win32Scanner.h"

using namespace std;

namespace ScannerLibrary
{
  //----------------------------------------------------------------------------
  Win32Scanner::Win32Scanner( const std::string & environment )
  : ISystemScanner( environment )
  {
  }
  //----------------------------------------------------------------------------
  Win32Scanner::~Win32Scanner()
  {
  }
  //----------------------------------------------------------------------------
  bool Win32Scanner::changeDirectory( const std::string & directory )
  {
    if( ::SetCurrentDirectory( directory.c_str() ) == TRUE )
    {
      return true;
    }
    return false;
  }
  //----------------------------------------------------------------------------
  std::string Win32Scanner::getCurrentDirectory() const
  {
    char dir[MAX_PATH] = { 0 };

    DWORD result = ::GetCurrentDirectory( MAX_PATH, dir );
    if( result == 0 )
    {
      error( getEnvironment(), getErrorMessage( result ) );
      return string();
    }
    return string( dir );
  }
  //----------------------------------------------------------------------------
  void Win32Scanner::getFiles( tStringList & files )
  {
    WIN32_FIND_DATA file;
    HANDLE hSearch;
    DWORD dwAttrs;
    bool done = false;

    hSearch = ::FindFirstFile( string( getCurrentDirectory() + "\\*.mp3" ).c_str(), &file );
    if( hSearch != INVALID_HANDLE_VALUE )
    {
      while( ! done )
      {
        dwAttrs = ::GetFileAttributes( file.cFileName );
        if(    !(dwAttrs & FILE_ATTRIBUTE_SYSTEM)
            && !(dwAttrs & FILE_ATTRIBUTE_DIRECTORY)
        ) {
          files.push_back( file.cFileName );
        }

        if( ! ::FindNextFile( hSearch, &file ) )
        {
          if( ::GetLastError() == ERROR_NO_MORE_FILES )
          {
            done = true;
          }
          else
          {
            error( getEnvironment(), "Couldn't find next file." );
          }
        }
      }
    }

    static_cast<void>( ! ::FindClose( hSearch ) );
  }
  //----------------------------------------------------------------------------
  void Win32Scanner::getDirectories( tStringList & directories )
  {
    WIN32_FIND_DATA file;
    HANDLE hSearch;
    DWORD dwAttrs;
    bool done = false;

    hSearch = ::FindFirstFile( string( getCurrentDirectory() + "\\*" ).c_str(), &file );
    if( hSearch != INVALID_HANDLE_VALUE )
    {
      while( ! done )
      {
        dwAttrs = ::GetFileAttributes( file.cFileName );
        if(    !(dwAttrs & FILE_ATTRIBUTE_SYSTEM)
            &&  (dwAttrs & FILE_ATTRIBUTE_DIRECTORY)
            && strcmp( file.cFileName, "." )
            && strcmp( file.cFileName, ".." )
        ) {
          // Not neccessary to check the current directory.
          // We are in!
          const string dir = getCurrentDirectory() + "\\" + file.cFileName;
          directories.push_back( dir );
        }

        if( ! ::FindNextFile( hSearch, &file ) )
        {
          if( ::GetLastError() == ERROR_NO_MORE_FILES )
          {
            done = true;
          }
          else
          {
            error( getEnvironment(), "Couldn't find next file." );
          }
        }
      }
    }

    static_cast<void>( ::FindClose( hSearch ) );
  }
  //----------------------------------------------------------------------------
  std::string Win32Scanner::getErrorMessage( const long error ) const
  {
    LPVOID lpMsgBuf;
    ::FormatMessage( 
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      error,
      MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // Default language
      reinterpret_cast<LPTSTR>( &lpMsgBuf ),
      0,
      NULL 
    );

    string result( static_cast<LPCTSTR>( lpMsgBuf ) );
    ::LocalFree( lpMsgBuf );

    return result;
  }
  //----------------------------------------------------------------------------
}