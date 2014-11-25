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
#include "defines.h"
#include "ID3Reporter.h"
#include "TaggerDefines.h"
#include "ID3Tagger.h"

using namespace std;
using namespace ScannerLibrary;

//----------------------------------------------------------------------------
ID3Reporter::ID3Reporter( const std::string & directory  )
: Scanner( directory, ToolShort )
, theErrorCount( 0 )
{
}
//----------------------------------------------------------------------------
ID3Reporter::~ID3Reporter()
{
}
//----------------------------------------------------------------------------
void ID3Reporter::onEndScanning()
{
  const string theOutput( "\nMP3 files : %d\n" );
  ::fprintf( stderr, theOutput.c_str(), getNumberOfFiles() );

  // Error handling and messages will be done by base class!
  // In case of an error, dump the simple error count only.
  if( theErrorCount != 0 )
  {
    const string theError(  "\n%d error(s) occured\n" );
    ::fprintf( stderr, theError.c_str(), theErrorCount );
  }
}
//----------------------------------------------------------------------------
void ID3Reporter::onEnterDirectory()
{
  ::fprintf( stderr, "%c", EnterDirectory );
}
//----------------------------------------------------------------------------
void ID3Reporter::onFoundFiles( const ScannerLibrary::tStringList & files )
{                         
  for( unsigned int i = 0; i < files.size(); i++ )
  {
    std::string theFile = getCurrentDirectory();
    theFile += "\\";
    theFile += files[i];

    ID3Tagger theTagger;
    // Use absolute path! This is important for the dump.
    theTagger.setFile( theFile );

    const ID3Tagger::ErrorCode theErrorCode = theTagger.read();
    if( theErrorCode == ID3Tagger::OK )
    {
      // Will dump a full blown output (all parameters and there values).
      cout << theTagger;
    }
    else
    if( theErrorCode == ID3Tagger::FILE_NOT_FOUND )
    {
      theErrorCount++;
    }
  }
}
//----------------------------------------------------------------------------