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

#include <iostream>
#include <string>

#include "PlaylistCreator.h"
#include "ReportCreator.h"
#include "ReportCreatorDirsOnly.h"
#include "ReportCreatorAllDirsOnly.h"
#include "ReportCreatorFilesOnly.h"
#include "defines.h"
#include "ScannerLibrary/ScannerFunctions.h"

using namespace std;

//----------------------------------------------------------------------------
void help()
{
  cout << Program << " " << Version << endl;
  cout << "Copyright (C) " << Author << endl;
  cout << endl;
  cout << Short << " comes with ABSOLUTELY NO WARRANTY. This is free software, and" << endl;
  cout << "you are welcome to redistribute it under certain conditions." << endl;
  cout << endl;
  cout << Short << " scans recursive from the current/specified directory down to directories" << endl;
  cout << "containing MP3 files and can generate a playlist for each directory." << endl;
  cout << "By default " << Short << " doesn't re-create playlists on each run." << endl;
  cout << "A playlist will be re-created if the content has been changed or if " << Short << endl;
  cout << "has been started with the 'force' option." << endl;
  cout << endl;
  cout << "While scanning different actions are shown..." << endl;
  cout << "  - Entering a directory is indicated by a " << EnterDirectory << endl;
  cout << "  - Generating a playlist is indicated by a " << WritePlaylist << endl;
  cout << "  - An unchanged playlist is indicated by a " << PlaylistUnchanged << endl;
  cout << endl;
  cout << "Syntax: gpl [-d|--dir <path>] [-n|--name <playlist>] [-f|--force] [-r|--report [level]] [-h|--help|/?]" << endl;
  cout << endl;
  cout << "  -d --dir            : Start scanning in directory. If path not specified" << endl;
  cout << "                        scanning starts in current directory." << endl;
  cout << "  -n --name           : Name of the generated playlists. If not specified" << endl;
  cout << "                        the playlists will be named '" << Playlist << "'." << endl;
  cout << "  -f --force          : Force of playlist generation." << endl;
  cout << "                        Not recognized in conjunction with -r/--report." << endl;
  cout << "  -r --report [level] : Generate a report containg directories and MP3 files." << endl;
  cout << "                        You should redirect the output into a file." << endl;
  cout << "                        Level = 0 : Will report mp3 files only." << endl;
  cout << "                        Level = 1 : Will report directories with mp3 files only." << endl;
  cout << "                        Level = 2 : Will report all directories." << endl;
  cout << "  -h --help /?        : Show this help information." << endl;

  // TODO: -e --exclude <file> Datei mit Verzeichnissen für die keine playlist generiert wird
}
//----------------------------------------------------------------------------
int main(int argc, char* argv[] )
{
  bool doReport = false;
  bool doForce = false;
  string dir;  // start scanning in directory
  string name; // name all playlist with this string
  int reportLevel = -1; // select report instance

  if( argc > 1 )
  {
    unsigned int dirFound = 0;
    unsigned int nameFound = 0;
    
    int i = 1;
    while( i != argc )
    {
      const string param( argv[i] );

      if( (param == "-h") || (param == "--help") || (param == "/?") )
      {
        help();
        return 0;
      }

      if( doReport && (reportLevel == -1) )
      {
        if( param == "0" )
        {
          reportLevel = 0;
        }
        else
        if( param == "1" )
        {
          reportLevel = 1;
        }
        else
        if( param == "2" )
        {
          reportLevel = 2;
        }
      }

      if( ((param == "-r") || (param == "--report")) && (! doReport) )
      {
        doReport = true;
      }

      if( ((param == "-f") || (param == "--force" )) && (! doForce) )
      {
        doForce = true;
      }

      if( dirFound == 1 )
      {
        dir = argv[i];
        dirFound++;
      }

      if( ((param == "-d") || (param == "--dir" )) && (dirFound == 0) )
      {
        dirFound++;
      }

      if( nameFound == 1 )
      {
        name = argv[i];
        nameFound++;
      }

      if( ((param == "-n") || (param == "--name" )) && (nameFound == 0) )
      {
        nameFound++;
      }

      i++;
    }
  }

  try
  {
    if( doReport )
    {
      ReportCreator * theReportCreator = NULL;

      if( reportLevel == 0 )
      {
        theReportCreator = new ReportCreatorFilesOnly( dir );
      }
      else
      if( reportLevel == 1 )
      {
        theReportCreator = new ReportCreatorDirsOnly( dir );
      }
      else
      if( reportLevel == 2 )
      {
        theReportCreator = new ReportCreatorAllDirsOnly( dir );
      }
      else
      {
        theReportCreator = new ReportCreator( dir );        
      }

      if( theReportCreator != NULL )
      {
        theReportCreator->startScanning();

        delete theReportCreator;
        theReportCreator = NULL;
      }
      else
      {
        ScannerLibrary::error( Short, "unsupported report level\n" );
        return -1;
      }
    }
    else
    {
      PlaylistCreator thePlaylistCreator( dir, name, doForce );
      thePlaylistCreator.startScanning();
    }
  }
  catch( const exception & theException )
  {
    fprintf( stderr, theException.what() );
  }

	return 0;
}
