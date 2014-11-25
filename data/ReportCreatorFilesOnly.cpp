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
#include "ReportCreatorFilesOnly.h"
#include "defines.h"

using namespace std;

//----------------------------------------------------------------------------
ReportCreatorFilesOnly::ReportCreatorFilesOnly( const std::string & directory  )
: ReportCreator( directory )
{
}
//----------------------------------------------------------------------------
ReportCreatorFilesOnly::~ReportCreatorFilesOnly()
{
}
//----------------------------------------------------------------------------
void ReportCreatorFilesOnly::onFoundFiles( const ScannerLibrary::tStringList & files )
{
  for( unsigned int i = 0; i < files.size(); i++ )
  {
    ::printf( "%s\\%s\n", getCurrentDirectory().c_str(), files[i].c_str() );
  }
}
//----------------------------------------------------------------------------