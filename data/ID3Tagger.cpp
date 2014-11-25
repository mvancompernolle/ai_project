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

#include <id3/tag.h>
#include <id3/misc_support.h>

#include "TaggerDefines.h"
#include "ID3Tagger.h"
#include "ScannerLibrary/ScannerFunctions.h"

//----------------------------------------------------------------------------
ID3Tagger::ID3Tagger()
: theFile( "" )
, theArtist( "" )
, theAlbum( "" )
, theSong( "" )
, theYear( "" )
, theTrack( "" )
, theComment( "" )
{
}
//----------------------------------------------------------------------------
void ID3Tagger::setFile( const std::string & value )
{
  theFile = value;
}
//----------------------------------------------------------------------------
void ID3Tagger::setArtist( const std::string & value )
{
  theArtist = value;
}
//----------------------------------------------------------------------------
void ID3Tagger::setAlbum( const std::string & value )
{
  theAlbum = value;
}
//----------------------------------------------------------------------------
void ID3Tagger::setSong( const std::string & value )
{
  theSong = value;
}
//----------------------------------------------------------------------------
void ID3Tagger::setYear( const std::string & value )
{
  theYear = value;
}
//----------------------------------------------------------------------------
void ID3Tagger::setTrack( const std::string & value )
{
  theTrack = value;
}
//----------------------------------------------------------------------------
void ID3Tagger::setComment( const std::string & value )
{
  theComment = value;
}
//----------------------------------------------------------------------------
const std::string & ID3Tagger::getFile() const
{
  return theFile;
}
//----------------------------------------------------------------------------
const std::string & ID3Tagger::getArtist() const
{
  return theArtist;
}
//----------------------------------------------------------------------------
const std::string & ID3Tagger::getAlbum() const
{
  return theAlbum;
}
//----------------------------------------------------------------------------
const std::string & ID3Tagger::getSong() const
{
  return theSong;
}
//----------------------------------------------------------------------------
const std::string & ID3Tagger::getYear() const
{
  return theYear;
}
//----------------------------------------------------------------------------
const std::string & ID3Tagger::getTrack() const
{
  return theTrack;
}
//----------------------------------------------------------------------------
const std::string & ID3Tagger::getComment() const
{
  return theComment;
}
//----------------------------------------------------------------------------
bool ID3Tagger::isFileAvailable() const
{
  // Test file existence; may be simplified !? ;-)
  if( theFile != "" )
  {
    FILE * pFile = fopen( theFile.c_str(), "rb" );
    if( pFile != NULL )
    {
      fclose( pFile );
      pFile = NULL;

      return true;
    }
  }

  return false;
}
//----------------------------------------------------------------------------
bool ID3Tagger::isArtistAvailable() const
{
  return( theArtist != "" );
}
//----------------------------------------------------------------------------
bool ID3Tagger::isAlbumAvailable() const
{
  return( theAlbum != "" );
}
//----------------------------------------------------------------------------
bool ID3Tagger::isSongAvailable() const
{
  return( theSong != "" );
}
//----------------------------------------------------------------------------
bool ID3Tagger::isYearAvailable() const
{
  return( theYear != "" );
}
//----------------------------------------------------------------------------
bool ID3Tagger::isTrackAvailable() const
{
  return( theTrack != "" );
}
//----------------------------------------------------------------------------
bool ID3Tagger::isCommentAvailable() const
{
  return( theComment != "" );
}
//----------------------------------------------------------------------------
ID3Tagger::ErrorCode ID3Tagger::read()
{
  if( ! isFileAvailable() )
  {
    return ID3Tagger::FILE_NOT_FOUND;
  }

  try
  {
    ID3_Tag theTag( theFile.c_str() );
    char * theString = NULL;

    theString = ID3_GetArtist( &theTag );
    if( theString != NULL )
    {
      theArtist = theString;
    }

    theString = ID3_GetAlbum( &theTag );
    if( theString != NULL )
    {
      theAlbum = theString;
    }

    theString = ID3_GetTitle( &theTag );
    if( theString != NULL )
    {
      theSong = theString;
    }

    theString = ID3_GetYear( &theTag );
    if( theString != NULL )
    {
      theYear = theString;
    }

    // Want the track number only!
    const unsigned int theTrackNr = ID3_GetTrackNum( &theTag );
    if( theTrackNr != 0 )
    {
      char buffer[32] = { 0 }; // it's an assumpution but it should be enough; forever ;-)
      theTrack = itoa( theTrackNr, buffer, 10 );
    }

    theString = ID3_GetComment( &theTag, NULL );
    if( theString != NULL )
    {
      theComment = theString;
    }
  }
  catch( ... )
  {
    // Catch each exception while working with id3 lib.
    char buffer[1024] = { 0 };

    sprintf( buffer, "*** ID3LIB *** error while reading tags from file %s\n", theFile.c_str() );
    ScannerLibrary::error( ToolShort, buffer );

    return ID3Tagger::ID3LIB_ERROR;
  }

  return ID3Tagger::OK;
}
//----------------------------------------------------------------------------
ID3Tagger::ErrorCode ID3Tagger::update()
{
  if( ! isFileAvailable() )
  {
    return ID3Tagger::FILE_NOT_FOUND;
  }

  bool somethingToDo =    isArtistAvailable()
                       || isAlbumAvailable()
                       || isSongAvailable()
                       || isYearAvailable()
                       || isTrackAvailable()
                       || isCommentAvailable();

  if( somethingToDo )
  {
    try
    {
      ID3_Tag tag( theFile.c_str() );
      bool replace = false;

      if( theArtist != "" )
      {
        replace = ( ID3_GetArtist( &tag ) != NULL );
        static_cast<void>( ID3_AddArtist( &tag, theArtist.c_str(), replace ) );
        tag.Update();
      }

      if( theAlbum != "" )
      {
        replace = ( ID3_GetAlbum( &tag ) != NULL );
        static_cast<void>( ID3_AddAlbum( &tag, theAlbum.c_str(), replace ) );
        tag.Update();
      }

      if( theSong != "" )
      {
        replace = ( ID3_GetTitle( &tag ) != NULL );
        static_cast<void>( ID3_AddTitle( &tag, theSong.c_str(), replace ) );
        tag.Update();
      }

      if( theYear != "" )
      {
        replace = ( ID3_GetYear( &tag ) != NULL );
        static_cast<void>( ID3_AddYear( &tag, theYear.c_str(), replace ) );
        tag.Update();
      }

      if( theTrack != "" )
      {
        replace = ( ID3_GetTrackNum( &tag ) != 0 );
        static_cast<void>( ID3_AddTrack( &tag, atoi( theTrack.c_str() ), 0, replace ) );
        tag.Update();
      }

      if( theComment != "" )
      {
        replace = ( ID3_GetComment( &tag, NULL ) != NULL );
        static_cast<void>( ID3_AddComment( &tag, theComment.c_str(), replace ) );
        tag.Update();
      }
    }
    catch( ... )
    {
      // Catch each exception while working with id3 lib.
      char buffer[1024] = { 0 };

      sprintf( buffer, "*** ID3LIB *** error while writing tags to file %s\n", theFile.c_str() );
      ScannerLibrary::error( ToolShort, buffer );

      return ID3Tagger::ID3LIB_ERROR;
    }
  }
  else
  {
    return ID3Tagger::NOTHING_TO_TAG;
  }

  return ID3Tagger::OK;
}
//----------------------------------------------------------------------------
std::ostream & operator << ( std::ostream & os, const ID3Tagger & tagger )
{
  if( tagger.isFileAvailable() )
  {
    os << ToolShort << " ";
    os << ParamFile    << " \"" << tagger.getFile()    << "\" ";
    os << ParamArtist  << " \"" << tagger.getArtist()  << "\" ";
    os << ParamAlbum   << " \"" << tagger.getAlbum()   << "\" ";
    os << ParamYear    << " \"" << tagger.getYear()    << "\" ";
    os << ParamSong    << " \"" << tagger.getSong()    << "\" ";
    os << ParamTrack   << " \"" << tagger.getTrack()   << "\" ";
    os << ParamComment << " \"" << tagger.getComment() << "\"";
    os << std::endl;
  }
  else
  {
    char buffer[1024] = { 0 };

    sprintf( buffer, "no dump created for file %s\n", tagger.getFile().c_str() );
    ScannerLibrary::error( ToolShort, buffer );
  }
  
  return os;
}
//----------------------------------------------------------------------------