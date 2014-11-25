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

#include <id3/tag.h>
#include <id3/misc_support.h>

#include "defines.h" // gpl
#include "TaggerDefines.h"
#include "ID3Reporter.h"
#include "ID3Tagger.h"

using namespace std;

void help()
{
  cout << ToolProgram << " " << ToolVersion << endl;
  cout << "Copyright (C) " << Author << endl;
  cout << endl;
  cout << ToolShort << " is part of the " << Program << " tool." << endl;
  cout << "Distribution of " << ToolProgram << " is allowed in conjunction with" << endl;
  cout << Program << " only." <<  endl;
  cout << endl;
  cout << ToolShort << " comes with ABSOLUTELY NO WARRANTY. This is free software, and" << endl;
  cout << "you are welcome to redistribute it under certain conditions." << endl;
  cout << endl;
  cout << Short << " handles the usage of ID3 tags in MP3 files. The program is able to" << endl;
  cout << "work with ID3v1 and ID3v2 tags." << endl;
  cout << endl;
  cout << "Syntax: tagger --dir <path> | --file <file> [--artist  <string>]" << endl;
  cout << "                                            [--album   <string>]" << endl;
  cout << "                                            [--song    <string>]" << endl;
  cout << "                                            [--year    <string>]" << endl;
  cout << "                                            [--track   <string>]" << endl;
  cout << "                                            [--comment <string>]" << endl;
  cout << "               -h|--help|/?" << endl;
  cout << endl;
  cout << "  --dir        : Scans recursive from the specified directory down to directories" << endl;
  cout << "                 containing MP3 files and read ID3 tags from each file." << endl;
  cout << "                 A full blown output for batch processing will be generated." << endl;
  cout << "                 Therefore you should redirect the output into a file." << endl;
  cout << "  --file       : Write specified ID3 tags into file." << endl;
  cout << "  --help -h /? : Show this help information." << endl;
  cout << endl;
  cout << "All other parameters can be used in conjunction with --file paramter only." << endl;
  cout << "They should be self explaining; no further description!" << endl;
  cout << "Note that --dir and --file exclude each other. Moreover note that multi line comments" << endl;
  cout << "are not supported." << endl;
  cout << endl;
}

int main(int argc, char* argv[])
{
  ID3Tagger theTagger;
  std::string theDirectory;

  unsigned int dirFound     = 0; // will start scanning for id3 tags
  unsigned int fileFound    = 0; // will write id3 tags into file

  unsigned int artistFound  = 0;
  unsigned int albumFound   = 0;
  unsigned int songFound    = 0;
  unsigned int yearFound    = 0;
  unsigned int trackFound   = 0;
  unsigned int commentFound = 0;

  int i = 1;
  while( i != argc )
  {
    const string param( argv[i] );

    // *** HELP ***
    if( (param == "-h") || (param == "--help") || (param == "/?") )
    {
      help();
      return 0;
    }

    // *** DIR ***
    if( dirFound == 1 )
    {
      theDirectory = param;
      dirFound++;
    }

    if( (param == ParamDir) && (dirFound == 0) )
    {
      dirFound++;
    }

    // *** FILE ***
    if( fileFound == 1 )
    {
      theTagger.setFile( param );
      fileFound++;
    }

    if( (param == ParamFile) && (fileFound == 0) )
    {
      fileFound++;
    }

    // *** ARTIST ***
    if( artistFound == 1 )
    {
      theTagger.setArtist( param );
      artistFound++;
    }

    if( (param == ParamArtist) && (artistFound == 0) )
    {
      artistFound++;
    }

    // *** ALBUM ***
    if( albumFound == 1 )
    {
      theTagger.setAlbum( param );
      albumFound++;
    }

    if( (param == ParamAlbum) && (albumFound == 0) )
    {
      albumFound++;
    }

    // *** SONG ***
    if( songFound == 1 )
    {
      theTagger.setSong( param );
      songFound++;
    }

    if( (param == ParamSong) && (songFound == 0) )
    {
      songFound++;
    }

    // *** YEAR ***
    if( yearFound == 1 )
    {
      theTagger.setYear( param );
      yearFound++;
    }

    if( (param == ParamYear) && (yearFound == 0) )
    {
      yearFound++;
    }

    // *** TRACK ***
    if( trackFound == 1 )
    {
      theTagger.setTrack( param );
      trackFound++;
    }

    if( (param == ParamTrack) && (trackFound == 0) )
    {
      trackFound++;
    }

    // *** COMMENT ***
    if( commentFound == 1 )
    {
      theTagger.setComment( param );
      commentFound++;
    }

    if( (param == "--comment") && (commentFound == 0) )
    {
      commentFound++;
    }

    i++;
  }

  try
  {
    // Check what kind of program function the user wants
    if( (dirFound != 0) && (fileFound != 0) )
    {
      ::fprintf( stderr, "%s error: parameter %s and %s exclude each other\n", ToolShort, ParamDir, ParamFile );
      return -1;
    }
    else
    {
      if( dirFound != 0 )
      {
        // scanner indicates work; finally a summary will be dumped
        ID3Reporter theReporter( theDirectory );
        theReporter.startScanning();
      }
      else
      if( fileFound != 0 )
      {
        ID3Tagger::ErrorCode theErrorCode = theTagger.update();
        if( theErrorCode == ID3Tagger::FILE_NOT_FOUND )
        {
          ::fprintf( stderr, "%s error: Couldn't open file %s\n", ToolShort, theTagger.getFile().c_str() );
        }
        else
        if( theErrorCode == ID3Tagger::NOTHING_TO_TAG )
        {
          ::fprintf( stderr, "%s warning: Nothing to tag in file %s\n", ToolShort, theTagger.getFile().c_str() );
        }
        else
        {
          ::fprintf( stderr, "%s ok: %s\n", ToolShort, theTagger.getFile().c_str() );
        }
      }
      else
      {
        ::fprintf( stderr, "%s error: Missing command line parameter. Try --help for detailed information.\n", ToolShort );
      }
    }
  }
  catch( const exception & theException )
  {
    fprintf( stderr, theException.what() );
  }

	return 0;
}
