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
#include "ScannerFunctions.h"
#include "Scanner.h"
#include "Win32Scanner.h"

using namespace std;

namespace ScannerLibrary
{
  //----------------------------------------------------------------------------
  Scanner::Scanner( const std::string & directory, const std::string & environment )
  : nFiles( 0 )
  , theRootDirectory( directory )
  , theSystemScanner( NULL )
  {
  #ifdef _WIN32
    theSystemScanner = new Win32Scanner( environment );
  #else
  #endif

    if( theSystemScanner == NULL )
    {
      throw exception();
    }

    if( theRootDirectory == string() )
    {
      theRootDirectory = getCurrentDirectory();
    }
  }
  //----------------------------------------------------------------------------
  Scanner::~Scanner()
  {
  }
  //----------------------------------------------------------------------------
  void Scanner::startScanning()
  {
    nFiles = 0;
    startScanning( theRootDirectory );
  }
  //----------------------------------------------------------------------------
  void Scanner::startScanning( const std::string & directory )
  {
    if( theSystemScanner->changeDirectory( directory ) )
    {
      tStringList files;
      theSystemScanner->getFiles( files );
      scanFiles( files );

      tStringList directories;
      theSystemScanner->getDirectories( directories );
      scanDirectories( directories );

      onEndScanning();
    }
    else
    {
      error( theSystemScanner->getEnvironment(), "Invalid path " + directory );
    }
  }
  //----------------------------------------------------------------------------
  string Scanner::getCurrentDirectory() const
  {
    return theSystemScanner->getCurrentDirectory();
  }
  //----------------------------------------------------------------------------
  void Scanner::scanFiles( const tStringList & files )
  {
    if( files.size() != 0 )
    {
      nFiles += files.size();
      onFoundFiles( files );
    }
  }
  //----------------------------------------------------------------------------
  void Scanner::scanDirectories( tStringList & directories )
  {
    tStringList theDirectories;
    for( unsigned int i = 0; i < directories.size(); i++ )
    {
      const string & theDirectory = directories[i];
      if( theSystemScanner->changeDirectory( theDirectory.c_str() ) )
      {
        onEnterDirectory();
        theSystemScanner->getDirectories( theDirectories );
    
        tStringList files;
        theSystemScanner->getFiles( files );
        scanFiles( files );

        if( theDirectories.size() != 0 )
        {
          scanDirectories( theDirectories );
        }
      }
      else
      {
        error( theSystemScanner->getEnvironment(), "Invalid path " + theDirectory );
        return;
      }
    }
    directories.clear();
  }
  //----------------------------------------------------------------------------
  std::string Scanner::getErrorMessage( const long error ) const
  {
    return theSystemScanner->getErrorMessage( error );
  }
  //----------------------------------------------------------------------------
  unsigned long Scanner::getNumberOfFiles() const
  {
    return nFiles;
  }
  //----------------------------------------------------------------------------
}