/*
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006
 * Lehrstuhl fuer Technische Informatik, RWTH-Aachen, Germany
 *
 * This file is part of the LTI-Computer Vision Library (LTI-Lib)
 *
 * The LTI-Lib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License (LGPL)
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * The LTI-Lib is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the LTI-Lib; see the file LICENSE.  If
 * not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiQuickCam.cpp
 * authors ....: Peter Mathes
 * organization: LTI, RWTH Aachen
 * creation ...: 13.08.99
 * revisions ..: $Id: ltiQuickCam.cpp,v 1.2 2006/02/07 19:58:56 ltilib Exp $
 */

#include "ltiHardwareConfig.h"

#ifdef _USE_QUICKCAM_GRABBER

#include <fstream>
#include <vector>
#include <map>
#include "ltiQuickCam.h"

namespace lti {

#ifdef __linux__

  quickCam::parameters::parameters()
    : frameGrabber::parameters(), cameraPort( 0 ), detectionMode( 1 ),
    autoAdjust( 0 ), bpp( 32 ), decimation( 4 ), despeckleMode( 1 ),
    x( 320 ), y( 240 ) {
  }

  //--------------------------------------------------------------------------

  functor::parameters* quickCam::parameters::clone() const {
    return ( new parameters(*this) );
  }

  //--------------------------------------------------------------------------

  quickCam::parameters& quickCam::parameters::copy(const parameters& other) {

    frameGrabber::parameters::copy(other);

    cameraPort    = other.cameraPort;
    detectionMode = other.detectionMode;
    autoAdjust    = other.autoAdjust;
    bpp           = other.bpp;
    decimation    = other.decimation;
    despeckleMode = other.despeckleMode;
    x             = other.x;
    y             = other.y;

    return (*this);
  }

  //-------------------------------------------------------------------------

  const char* quickCam::parameters::getTypeName() const	{
    return "quickCam::parameters";
  }

   //------------------------------------------------------------------------

  quickCam::quickCam() {
    parameters theParameters;
    frameGrabber::setParameters(theParameters);
  }

  quickCam::quickCam(const quickCam::parameters& theParams) {
    frameGrabber::setParameters(theParams);
  }

  //-------------------------------------------------------------------------

  quickCam::~quickCam() {
  }

  //-------------------------------------------------------------------------

  // returns the current parameters
  const quickCam::parameters& quickCam::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>( &functor::getParameters() );
    if( params==0 ) throw invalidParametersException(getTypeName());
    return *params;
  }


  //-------------------------------------------------------------------------

  const char* quickCam::getTypeName() const {
    return "quickCam";
  }


  //-------------------------------------------------------------------------

  functor* quickCam::clone() const {
    return ( new quickCam(*this) );
  }

  //-------------------------------------------------------------------------

  bool quickCam::setParameters( const functor::parameters& theParams ) {
    frameGrabber::setParameters(theParams);


#   ifdef DEBUG
      std::cout << "quickCam::setParameters START...\n";
      std::cout <<"Parameters port="<<getParameters().cameraPort<<"  \n";
      std::cout <<"Parameters mode="<<getParameters().detectionMode<<"  \n";
#   endif

    camera = new camera_t(getParameters().cameraPort, getParameters().detectionMode );

    upperBound    = 253;
    lowerBound    =   5;
    xSize = getParameters().x;
    ySize = getParameters().y;

#   ifdef DEBUG
      std::cout <<"Parameters step 0 \n";
      std::cout <<"Parameters bpp="<<getParameters().bpp<<"  \n";
      std::cout <<"Parameters deci="<<getParameters().decimation<<"  \n";
#   endif

    camera->set_bpp( getParameters().bpp );
    camera->set_decimation( getParameters().decimation );
    std::cout <<"Parameters step 1 \n";
    std::cout <<"Parameters xSize="<<xSize<<"  \n";
    std::cout <<"Parameters ySize="<<ySize<<"  \n";

    camera->set_width( xSize );
    camera->set_height( ySize );
    width  = camera->get_pix_width();
    height = camera->get_pix_height();

#   ifdef DEBUG
      std::cout <<"Parameters step 2 \n";
      fprintf( stderr, "Camera version: 0x%x.\n", camera->get_version() );
      fprintf( stderr, "Camera status:  " );
      int cqStatus = camera->get_status();
      for ( int b=0; b<8; b++ ) {
        fprintf( stderr, "%d", (cqStatus & 0x80)>>7 );
        cqStatus *= 2;
      }
      fprintf( stderr, "\n" );
      std::cout << "quickCam initialized...\n";
      std::cout << "quickCam::setParameters END...\n";
#   endif

    return true;
  }

  //-------------------------------------------------------------------------

  bool quickCam::init() {// : camera( getParameters().camera_port,
    //	getParameters().detection_mode )
    // probe for and initialize the cam

    upperBound    = 253;
    lowerBound    =   5;
    xSize         =   getParameters().x;
    ySize         =   getParameters().y;

    camera->set_bpp( getParameters().bpp );
    camera->set_decimation( getParameters().decimation );
    camera->set_width( xSize );
    camera->set_height( ySize );
    width  = camera->get_pix_width();
    height = camera->get_pix_height();

#   ifdef DEBUG
      fprintf( stderr, "Camera version: 0x%x.\n", camera->get_version() );
      fprintf( stderr, "Camera status:  " );
      int cq_status = camera->get_status();
      for ( int b=0; b<8; b++ ) {
        fprintf( stderr, "%d", (cq_status & 0x80)>>7 );
        cq_status *= 2;
      }
      fprintf( stderr, "\n" );
      std::cout << "quickCam initialized...\n";
#   endif

    return true;
  }


  //------------------------------------------------------------------------

  bool quickCam::apply( image& theImage ) {

    theImage.resize( ySize, xSize, 0, true, false );

    int limit = height*width;

    unsigned char *scan = camera->get_frame();

    if ( camera->get_bpp() == 32 )
      scan = raw32_to_24( scan, width, height );

#   ifdef DEBUG
      std::cout << "Got frame...\n";
#   endif

    int rtemp = camera->get_red();
    int gtemp = camera->get_green();
    int btemp = camera->get_blue();

    get_rgb_adj( scan, limit, rtemp, gtemp, btemp );

    camera->set_red( rtemp );
    camera->set_green( gtemp );
    camera->set_blue( btemp );

    int britemp;
    int done = get_brightness_adj( scan, limit, britemp );
    if ( !done ) {
      int cur_bri = camera->get_brightness() + britemp;
      if ( cur_bri > upperBound )
        cur_bri = upperBound - 1;
      if ( cur_bri < lowerBound )
        cur_bri = lowerBound + 1;
      if ( britemp > 0 )
        lowerBound = camera->get_brightness() + 1;
      else
	      upperBound = camera->get_brightness() - 1;

#     ifdef DEBUG
        fprintf( stderr, "Brightness %s to %d (%d..%d)\n",
	               (britemp<0)? "decreased" : "increased", cur_bri,
	               lower_bound, upper_bound );
#     endif

      camera->set_brightness( cur_bri );

#     ifdef DEBUG
        std::cout << "Brightness set...\n";
#     endif
    }

    do_rgb_adj( scan, limit,
                camera->get_red(),
                camera->get_green(),
                camera->get_blue() );

    if ( camera->get_bpp() == 24 )
      scan = despeckle( scan, width, height );


#   ifdef DEBUG
      std::cout << "Frame despeckled..\n";
#   endif

// 	   if ( auto_adjust )
// 	   {
// 		   int done = 0;
// 		   int upper_bound = 253;
// 			int lower_bound = 5;
// 			int loops       = 0;
//
// 		   do
// 		   {
// 			   scan = camera->get_frame();
//             std::cout << "Grabbed frame for adjusting cam...\n";
// 			   if ( camera->get_bpp() == 32 )
// 				   scan = raw32_to_24( scan, width, height );
// 	         int britemp = 0;
// 		      done = get_brightness_adj( scan, width*height, britemp );
//             std::cout << "DONE: " << ((done)? "TRUE":"FALSE") << "\n";
// 			   if ( !done )
//    	      {
// 				   int cur_bri = camera->get_brightness() + britemp;
//                std::cout << " #1"
//                          << "  Actual brightness: " << camera->get_brightness()
//                			 << "  britemp:" << britemp
//                          << "  cur_bri:" << cur_bri
//                          << "  upper_bound:" << upper_bound
//                          << "  lower_bound:" << lower_bound
//                          << "\n";
// 				   if ( cur_bri > upper_bound )
// 					   cur_bri = upper_bound - 1;
// 				   if ( cur_bri < lower_bound )
// 					   cur_bri = lower_bound + 1;
// 				   if ( britemp > 0 )
// 					   lower_bound = camera->get_brightness() + 1;
// 				   else
// 					   upper_bound = camera->get_brightness() - 1;
// 				   camera->set_brightness( cur_bri );
//                std::cout << " #2"
//                			 << "  Actual brightness: " << camera->get_brightness()
//                          << "  britemp:" << britemp
//                          << "  cur_bri:" << cur_bri
//                          << "  upper_bound:" << upper_bound
//                          << "  lower_bound:" << lower_bound
//                          << "\n";
// 				   delete[] scan;
// 			   }
// 		   } while ( (!done) && (upper_bound > lower_bound) && (++loops <= 10) );
// 			std::cout << "Cam adjusted, grabbing frame now...\n";
// 		   scan = camera->get_frame();
// 		   if ( camera->get_bpp() == 32 )
// 			   scan = raw32_to_24( scan, width, height );
// 	   }
// 	   else
//    	{
// 		   scan = camera->get_frame();
// 		   if ( camera->get_bpp() == 32 )
// 			   scan = raw32_to_24( scan, width, height );
// 	   }
//
// 	   int rtemp, gtemp, btemp;
//
// 	   get_rgb_adj( scan, width*height, rtemp, gtemp, btemp );
// 	   camera->set_red( rtemp );
//    	camera->set_green( gtemp );
// 	   camera->set_blue( btemp );
//
// //		if ( despeckle_mode && camera->get_bpp() == 24 )
// //			scan = despeckle( scan, width, height );
//
// 	   do_rgb_adj( scan, width*height, camera->get_red(), camera->get_green(), camera->get_blue() );

    lti::rgbPixel pixel;
    ubyte *scanp;
    scanp = scan;

    for ( int y=0; y < height; y++ ) {
      for ( int x=0; x < width; x++ ) {
        pixel.setRed(   *scanp ); scanp++;
        pixel.setGreen( *scanp ); scanp++;
        pixel.setBlue(  *scanp ); scanp++;
        theImage.at( y, x ) = pixel;
      }
    }

#   ifdef DEBUG
      std::cout << "Frame converted...\n";
#   endif

    delete[] scan;

#   ifdef DEBUG
      std::cout << "SCAN deleted...\n";
#   endif

    return true;
  }


  bool quickCam::apply(channel8& theChannel)
  {
    theChannel.resize( ySize, xSize, 0, true, false );

    int limit = height*width;

    unsigned char *scan = camera->get_frame();

    if ( camera->get_bpp() == 32 )
      scan = raw32_to_24( scan, width, height );

#   ifdef DEBUG
      std::cout << "Got frame...\n";
#   endif

    int rtemp = camera->get_red();
    int gtemp = camera->get_green();
    int btemp = camera->get_blue();

    get_rgb_adj( scan, limit, rtemp, gtemp, btemp );

    camera->set_red( rtemp );
    camera->set_green( gtemp );
    camera->set_blue( btemp );

    int britemp;
    int done = get_brightness_adj( scan, limit, britemp );
    if ( !done ) {
      int cur_bri = camera->get_brightness() + britemp;
      if ( cur_bri > upperBound )
        cur_bri = upperBound - 1;
      if ( cur_bri < lowerBound )
        cur_bri = lowerBound + 1;
      if ( britemp > 0 )
        lowerBound = camera->get_brightness() + 1;
      else
	      upperBound = camera->get_brightness() - 1;

#     ifdef DEBUG
        fprintf( stderr, "Brightness %s to %d (%d..%d)\n",
	               (britemp<0)? "decreased" : "increased", cur_bri,
	               lower_bound, upper_bound );
#     endif
      camera->set_brightness( cur_bri );
#     ifdef DEBUG
        std::cout << "Brightness set...\n";
#     endif
    }

    do_rgb_adj( scan, limit,
                camera->get_red(),
                camera->get_green(),
                camera->get_blue() );

    if ( camera->get_bpp() == 24 )
      scan = despeckle( scan, width, height );


#   ifdef DEBUG
      std::cout << "Frame despeckled..\n";
#   endif

    ubyte *scanp;
    scanp = scan;

    for ( int y=0; y < height; y++ ) {
      for ( int x=0; x < width; x++ ) {
        theChannel.at( y, x ) = (int) ((*scanp+*(scanp+1)+*(scanp+2))/3);
        scanp=scanp+3;
      }
    }

#   ifdef DEBUG
      std::cout << "Frame converted...\n";
#   endif

    delete[] scan;

#   ifdef DEBUG
      std::cout << "SCAN deleted...\n";
#   endif

    return true;
  }


  bool quickCam::apply(channel& theChannel)
  {
    theChannel.resize( ySize, xSize, 0, true, false );

    int limit = height*width;

    unsigned char *scan = camera->get_frame();

    if ( camera->get_bpp() == 32 )
      scan = raw32_to_24( scan, width, height );

#   ifdef DEBUG
      std::cout << "Got frame...\n";
#   endif

    int rtemp = camera->get_red();
    int gtemp = camera->get_green();
    int btemp = camera->get_blue();

    get_rgb_adj( scan, limit, rtemp, gtemp, btemp );

    camera->set_red( rtemp );
    camera->set_green( gtemp );
    camera->set_blue( btemp );

    int britemp;
    int done = get_brightness_adj( scan, limit, britemp );
    if ( !done ) {
      int cur_bri = camera->get_brightness() + britemp;
      if ( cur_bri > upperBound )
        cur_bri = upperBound - 1;
      if ( cur_bri < lowerBound )
        cur_bri = lowerBound + 1;
      if ( britemp > 0 )
        lowerBound = camera->get_brightness() + 1;
      else
	      upperBound = camera->get_brightness() - 1;

#     ifdef DEBUG
        fprintf( stderr, "Brightness %s to %d (%d..%d)\n",
	               (britemp<0)? "decreased" : "increased", cur_bri,
	               lower_bound, upper_bound );
#     endif

      camera->set_brightness( cur_bri );

#     ifdef DEBUG
        std::cout << "Brightness set...\n";
#     endif
    }

    do_rgb_adj( scan, limit,
                camera->get_red(),
                camera->get_green(),
                camera->get_blue() );

    if ( camera->get_bpp() == 24 )
      scan = despeckle( scan, width, height );


#   ifdef DEBUG
      std::cout << "Frame despeckled..\n";
#   endif

    lti::rgbPixel pixel;
    ubyte *scanp;
    scanp = scan;

    for ( int y=0; y < height; y++ ) {
      for ( int x=0; x < width; x++ ) {
        theChannel.at( y, x ) = 1.0*(*scanp+*(scanp+1)+*(scanp+2))/3.0;
        scanp=scanp+3;
      }
    }

#   ifdef DEBUG
      std::cout << "Frame converted...\n";
#   endif

    delete[] scan;

#   ifdef DEBUG
      std::cout << "SCAN deleted...\n";
#   endif

    return true;
  }


  bool quickCam::isActive() const {
    return true;
  }
}

#endif // #ifndef __linux__

#endif // #ifdef _USE_QUICKCAM_GRABBER
