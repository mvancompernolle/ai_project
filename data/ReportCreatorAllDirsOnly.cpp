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
#include "ReportCreatorAllDirsOnly.h"
#include "defines.h"

using namespace std;

//----------------------------------------------------------------------------
ReportCreatorAllDirsOnly::ReportCreatorAllDirsOnly( const std::string & directory  )
: ReportCreator( directory )
, nDirectories( 0 )
{
}
//----------------------------------------------------------------------------
ReportCreatorAllDirsOnly::~ReportCreatorAllDirsOnly()
{
}
//----------------------------------------------------------------------------
void ReportCreatorAllDirsOnly::onEndScanning()
{
  const string theOutput( "\nDirectory count : %d\n" );

  ::fprintf( stderr, theOutput.c_str(), nDirectories );
  ::printf( theOutput.c_str(), nDirectories );
}
//----------------------------------------------------------------------------
void ReportCreatorAllDirsOnly::onEnterDirectory()
{
  ReportCreator::onEnterDirectory();
  ::printf( "%s\n", getCurrentDirectory().c_str() );
  nDirectories++;
}
//----------------------------------------------------------------------------
void ReportCreatorAllDirsOnly::onFoundFiles( const ScannerLibrary::tStringList & files )
{
  static_cast<void>( files );
}
//----------------------------------------------------------------------------