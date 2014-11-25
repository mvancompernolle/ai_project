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
#include "ReportCreator.h"
#include "defines.h"

using namespace std;
using namespace ScannerLibrary;

//----------------------------------------------------------------------------
ReportCreator::ReportCreator( const std::string & directory  )
: Scanner( directory, Short )
{
}
//----------------------------------------------------------------------------
ReportCreator::~ReportCreator()
{
}
//----------------------------------------------------------------------------
void ReportCreator::onEndScanning()
{
  const string theOutput( "\nMP3 files : %d\n" );

  ::fprintf( stderr, theOutput.c_str(), getNumberOfFiles() );
  ::printf( theOutput.c_str(), getNumberOfFiles() );
}
//----------------------------------------------------------------------------
void ReportCreator::onEnterDirectory()
{
  ::fprintf( stderr, "%c", EnterDirectory );
}
//----------------------------------------------------------------------------
void ReportCreator::onFoundFiles( const ScannerLibrary::tStringList & files )
{
  ::fprintf( stderr, "%c", PlaylistUnchanged );
  ::printf( "%s\n", getCurrentDirectory().c_str() );

  for( unsigned int i = 0; i < files.size(); i++ )
  {
    ::printf( "   %s\n", files[i].c_str() );
  }
}
//----------------------------------------------------------------------------