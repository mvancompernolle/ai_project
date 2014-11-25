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
 * file .......: ltiITIFrameGrabber.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 13.08.99
 * revisions ..: $Id: ltiITIFrameGrabber.cpp,v 1.5 2006/02/08 12:04:18 ltilib Exp $
 */

#ifdef _USE_ITI_FRAME_GRABBER
#ifdef _LTI_MSC_6

#include "ltiITIFrameGrabber.h"

namespace lti {

  // error codes

  const short itiFrameGrabber::NoError   = 0;
  const short itiFrameGrabber::Error 	   = 1;
  const short itiFrameGrabber::NoGrabber = 2;

  // Hardware constants

  const double itiFrameGrabber::itiAMStd::maxVoltage = 5.0;
  const double itiFrameGrabber::itiAMStd::minVoltage = 0.0;
  const double itiFrameGrabber::itiAMStd::minDiffVoltage = 2.0;


  itiFrameGrabber::parameters::parameters() : frameGrabber::parameters() {

    mode = QuarterImageFiltered;

    trgbPixel<double> negRef(0.75,0.75,0.75);
    trgbPixel<double> posRef(3.25,3.25,3.25);

    referenceNegative.copy(negRef);
    referencePositive.copy(posRef);

    // determine default filename of configuration file
    configFile = "ltiiti.cnf";

    std::string path;
		int pos;
		path = getenv("CNF15040");	// configurations file path
		pos = path.length();		    // end of the path
    if (pos == 0) { // empty path?
		  path = configFile;
		}	else {
		  if ((path.at(pos-1) == '/') || (path.at(pos-1) == '\\')) {
				path += configFile;
		  } else {
			  path += '\\';
				path += configFile;
		  }
    }

    configFile = path;

    // low pass filter default is on
    lowPassFilterOn = true;
  }

  functor::parameters* itiFrameGrabber::parameters::clone() const {
    return ( new parameters(*this) );
  }

  const char* itiFrameGrabber::parameters::getTypeName() const {
    return "itiFrameGrabber::parameters";
  }

  itiFrameGrabber::parameters&
    itiFrameGrabber::parameters::copy(const parameters& other ) {

#   ifndef _LTI_MSC_6
      // for normal  ANSI C++
      frameGrabber::parameters::copy(other);
#   else
      // this doesn't work with MS-VC++ 6.0 (an once again... another bug)
      // ...so we have to use this workaround.
      // Conditional on that, copy may not be virtual.
      frameGrabber::parameters&
        (frameGrabber::parameters::* p_copy)(const frameGrabber::parameters&) =
        frameGrabber::parameters::copy;
      (this->*p_copy)(other);
#   endif

    configFile = other.configFile;
    mode       = other.mode;
    referenceNegative.copy(other.referenceNegative);
    referencePositive.copy(other.referencePositive);
    lowPassFilterOn = other.lowPassFilterOn;

    return ( *this );
  }

  // --------------------------------

  itiFrameGrabber::itiFrameGrabber(const bool& initialize) : frameGrabber() {
    active = false;
    halfPict = 0;
    module = 0;

    parameters tmp;
    setParameters(tmp);

    if (initialize)
      init();
  }

  itiFrameGrabber::itiFrameGrabber( const parameters& theParam ) {
    active = false;
    halfPict = 0;
    module = 0;

    setParameters( theParam );
  }

  itiFrameGrabber::itiFrameGrabber(const itiFrameGrabber& other) {
    active = false;
    halfPict = 0;
    module = 0;

    copy(other);
  }

  itiFrameGrabber::~itiFrameGrabber() {
    delete [] halfPict;
    halfPict = 0;

    // delete frame buffer
  	icp_delete_frame( icpmod, frameID );

    delete module;
    module = 0;
  }

  const char* itiFrameGrabber::getTypeName( void ) const {
    return "itiFrameGrabber";
  }

  itiFrameGrabber& itiFrameGrabber::copy(const itiFrameGrabber& other) {
    frameGrabber::copy(other);
    active = false;
    delete module;
    module = 0;
    delete[] halfPict;
    halfPict = 0;

    return (*this);
  }

  functor* itiFrameGrabber::clone() const {
    return (new itiFrameGrabber(*this));
  }

  const itiFrameGrabber::parameters& itiFrameGrabber::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if( params==0 )
      throw invalidParametersException(getTypeName());
    return *params;
  }

  void itiFrameGrabber::init() {
    const parameters& param=getParameters();

    delete [] halfPict;
    halfPict = 0;

    delete module;
    module      = 0;

    active      = false;

	  char buffer[128];
	  strcpy(buffer,param.configFile.c_str());

    if( itx_load_cnf( buffer ) != ITX_NO_ERROR ) {
		  TRACE("Grabber: itx_load_cnf failed.\n");
      throw frameGrabberException( "itx_load_cnf failed" );
    }

	  if( itx_get_slotmodcnf( 0, 0, "ICP", &icpmod ) != ITX_NO_ERROR ) {
		  TRACE( "Grabber: No ICP installed\n" );
      throw frameGrabberException( "no ICP installed" );
    }

	  if( itx_init( icpmod ) != ITX_NO_ERROR ) {
      TRACE("Grabber: Initialisation failed\n");

      throw frameGrabberException( "grabber base board initialization failed" );
    }

    // determine frame grabber modul, reserve frame buffer on ICP
    try
    {
      module = moduleFactory.generateModule( icpmod );
    } catch( ... ) {
      throw frameGrabberException( "Cannot find ITI module" );
    }

    icp_get_acq_dim( icpmod, &xCam, &yCam );
	  frameID = icp_create_frame( icpmod, xCam, yCam, ICP_PIX24, ICP_RGB );

	  if( ! ( halfPict = new char[ xCam*yCam*2 ] ) ) {
		  TRACE("kein Speicher für Halbbild verfügbar\n");
      throw frameGrabberException( "no host memory available" );
    }

    // get begin address of first field
	  firstFieldStart = icp_get_bm_firstf_start(icpmod);

    // get begin address of second field
	  secondFieldStart = icp_get_bm_secondf_start(icpmod);

    // both addresses are needed in the initialitation of the BM-Transfer

    // begin snap at an even field
    icp_start_field( icpmod, ICP_EVEN );

    // set voltage references from the parameters
    if ((module->setGreenRef(param.referenceNegative.getGreen(),
                             param.referencePositive.getGreen()) == NoError) &&
        (module->setRedRef(param.referenceNegative.getRed(),
                           param.referencePositive.getRed()) == NoError) &&
        (module->setBlueRef(param.referenceNegative.getBlue(),
                            param.referencePositive.getBlue()) == NoError)) {

      TRACE("Voltage references set\n");
      active = true;
    } else {
      TRACE("Error by setting voltage references!");
      active = false;
      throw frameGrabberException("error setting voltage references");
    }

    module->setRGB();

    if (param.lowPassFilterOn) {
      module->lowpassOn();
    } else {
      module->lowpassOff();
    }

  }

  // ----------------------------------------------------------------

  bool itiFrameGrabber::isActive() const {
    return active;
  }

  /// load an (color) image from the grabber
  image& itiFrameGrabber::apply(image& theImage) {
    const parameters& param = getParameters();
    bool ok = false;

    if (isActive() && snap()) {
      // convert it to an image
      switch(param.mode) {
      case parameters::FullImageRectangular:
        ok = captureFull(theImage);
        break;
      case parameters::FullImageSquare:
        ok = captureFullSquare(theImage);
        break;
      case parameters::QuarterImageFiltered:
      case parameters::QuarterImageSampled:
        ok = captureQuarter(theImage);
        break;
      }
    }

    if (!ok) {
      theImage.resize(16,16,rgbPixel(255,0,0),false);
    }

    return theImage;
  }

  // load a grey value channel from the
  channel8& itiFrameGrabber::apply(channel8& theChannel) {
    // TODO
    return theChannel;
  }

  // capture quarter image
  bool itiFrameGrabber::captureQuarter(image& theImage) {
  	DWORD region;

    theImage.resize(256,256,rgbPixel(),false,false);

    // aktivate bus master transfer
    if ( icp_bm_mode( icpmod, ICP_ENABLED ) )	{
  		TRACE("Grabber::Busmaster not available\n");
	  }

		// set horizontal width for BM-Transfer
	  icp_put_bm_aoix( icpmod, 128, ICP_PIX24 );
    // read only one field
	  icp_bm_ilace( icpmod, ICP_DISABLE );

    // set horizontal width for BM-Transfer
  	icp_image_pitch( icpmod, 128, ICP_PIX24 );

    // set destination address for  BM-Transfer
	  icp_put_bm_dst_addr( icpmod, (DWORD)halfPict );
    // set the begin address of the field
	  icp_put_bm_firstf_start( icpmod, firstFieldStart );
	  icp_put_bm_secondf_start( icpmod, secondFieldStart);

    // Memory lock for host memory
	  if ( ! ( region = icp_bm_lock( icpmod, halfPict, xCam*yCam*2 ) ) )	{
		  TRACE("Grabber::transfer_quater: region can not be locked\n");
		  return false; // error
	  }

    // Start bus master transfer for (hope) one frame
    if( icp_bm_read( icpmod, region ) != ICP_NO_ERROR )	{
		  TRACE("Grabber::transfer_quater: Fehler beim Übertragen des Bildes in den Hostspeicher\n");
		  return false; // error
	  }

    // free the region
	  icp_bm_unlock( icpmod, region );

	  icp_bm_mode( icpmod, ICP_DISABLE );

    // half image to quater image
	  cutToSquareAndDecimate(theImage);

  	return true;
  }

  // capture the full image, and extract a square from it
  bool itiFrameGrabber::captureFullSquare(image& theImage) {
    theImage.resize(512,512,rgbPixel(),false,false);

  	icp_bm_mode( icpmod, ICP_ENABLED );
	  icp_read_area( icpmod, frameID, 128, 0, 512, 512, (DWORD *)(&theImage.at(0,0)));
	  icp_bm_mode( icpmod, ICP_DISABLED );

    return true;
  }

  // capture the full rectangular image
  bool itiFrameGrabber::captureFull(image& theImage) {
    theImage.resize(512,768,rgbPixel(),false,false);

  	icp_bm_mode( icpmod, ICP_ENABLED );
	  icp_read_area( icpmod, frameID, 0, 0, 768, 512, (DWORD *)(&theImage.at(0,0)));
	  icp_bm_mode( icpmod, ICP_DISABLED );

	  return true;
  }

  // cut to square and decimate the buffer
  bool itiFrameGrabber::cutToSquareAndDecimate(image& theImage) {
	  int x,y;
	  rgbPixel* help1 = 0;
    rgbPixel* help2 = 0;

	  help1 = (rgbPixel*)halfPict;
	  help2 = &theImage.at(0,0);

	  for ( y = 0; y < 256; y++) {
		  help1 += 128;					// link border
		  for ( x = 0; x < 256; x++ ) {
			  *help2++ = *help1++;
			  help1++;				    // take each 2nd pixel
		  }
		  help1 += 128;					// cut right border, so that we get
                            // a square
	  }

    return true;
  }

  bool itiFrameGrabber::snap() {
    if( icp_snap( icpmod, frameID ) ) {	// take the picture
  		TRACE("Grabber::snap: no picture taken\n");
	  	return false;
	  }

	  return true;
  }

  // --------------------------- itiModule --------------------------

  itiFrameGrabber::itiModule::~itiModule( void ) {
  }

  bool itiFrameGrabber::itiModule::isAMCLR( void ) {
    return ( dynamic_cast<itiAMClr*>(this) != 0 );
  }

  bool itiFrameGrabber::itiModule::isAMSTD( void ) {
    return ( dynamic_cast<itiAMStd*>(this) != 0 );
  }

  short itiFrameGrabber::itiModule::setHSI( void ) {
    if ( isAMCLR() ) {
      return dynamic_cast<itiAMClr*>(this)->setHSI();
    } else {
      return 0;
    }
  }

  // Factory creates grabber modules
  lti::itiFrameGrabber::itiModule*  lti::itiFrameGrabber::itiFactory::generateModule( MODCNF* icpmod ) {
    MODCNF* mod = 0;
    mod = itx_get_modcnf( icpmod, "AM-STD", 0 );
    if ( mod == 0 ) {
      TRACE("Grabber: No AM_STD installed\n");
      mod = itx_get_modcnf( icpmod, "AM-CLR", 0 );
      if ( mod == 0 ) {
        TRACE("Grabber: No AM_CLR installed\n");
      } else {
        return new itiAMClr( icpmod, mod );
      }
    } else {
      return new itiAMStd( icpmod, mod );
    }

    return 0;
  }


  itiFrameGrabber::itiAMStd::itiAMStd( MODCNF* theIcpmod, MODCNF* theModule )
    : icpmod ( theIcpmod ), module ( theModule ) {

    TRACE( " itiFrameGrabber::itiAMStd::itiAMStd()\n" );
    short model = ams_model( module );
    short result = 0;

    if ( model != AMSTD_RGB ) {
      TRACE("\tModel %d not supported yet\n", model);
      throw frameGrabberException("Model not supported yet");
    }
    result = ams_cscbypass( module, AMS_ENABLE );

    TRACE( "Color Space Converter bypass = %d\n", result );

    ams_sel_port( module, 0 );
    camera = ams_get_port( module, 0 );

    if ( camera == 0 ) {
      TRACE("Cannot get camera configuration from port 0\n");
      throw frameGrabberException("Model not supported yet");
    }
    ams_set_port( module, camera, 1 );
  }

  itiFrameGrabber::itiAMStd::~itiAMStd( void ) {
  }

  short itiFrameGrabber::itiAMStd::setRGB( void ) {
    TRACE("itiFrameGrabber::itiAMStd::setRGB() cannot modify. No matrix available\n");
    return 0;
  }

  short itiFrameGrabber::itiAMStd::setBGR( void ) {
    TRACE("itiFrameGrabber::itiAMStd::setBGR() cannot modify. No matrix available\n");
    return 0;
  }

  short itiFrameGrabber::itiAMStd::lowpassOn( void ) {
    ams_lowpass( module, camera, AMS_6MHZ);
    return 0;
  }

  short itiFrameGrabber::itiAMStd::lowpassOff( void ) {
    ams_lowpass( module, camera, AMS_BYPASS_LPF);
    return 0;
  }

  short itiFrameGrabber::itiAMStd::setRedRef( double neg, double pos ) {

	  if( (neg < minVoltage) || (pos > maxVoltage) || (pos - neg < minDiffVoltage) ) {
		  TRACE("setRedRef: Values out of range\n");
		  return Error;
    }

  	if ( ams_rpref( module, camera, (short)(pos/(maxVoltage/255) ) ) ) {
      // set positive Reference
		  TRACE("setRedRef: positive red reference not set\n");
		  return Error;
    }

	  if ( ams_rnref( module, camera, (short)(neg/(maxVoltage/255) ) ) ) {
      // set negative Reference
		  TRACE("setRedRef: negative red reference not set\n");
		  return Error;
	  }
	  return NoError;
  }

  short itiFrameGrabber::itiAMStd::setGreenRef( double neg, double pos ) {

    if( (neg < minVoltage) || (pos > maxVoltage) || (pos - neg < minDiffVoltage) ) {
		  TRACE("setGreenRef: Values out of range\n");
		  return Error;
	  }

	  if ( ams_gpref( module, camera, (short)(pos/(maxVoltage/255) ) ) ) {
      // set positive Reference
		  TRACE("setGreenRef: green reference not set\n");
		  return Error;
	  }

	  if ( ams_gnref( module, camera, (short)(neg/(maxVoltage/255) ) ) ) {
      // set negative Reference
		  TRACE("setGreenRef: green reference not set\n");
		  return Error;
	  }

	  return NoError;
  }

  short itiFrameGrabber::itiAMStd::setBlueRef( double neg, double pos ) {
    if( (neg < minVoltage) || (pos > maxVoltage) || (pos - neg < minDiffVoltage) ) {
		  TRACE("setBlueRef: Values out of range\n");
		  return Error;
	  }

	  if ( ams_bpref( module, camera, (short)(pos/(maxVoltage/255) ) ) ) {
      // set positive Reference
      TRACE("setBlueRef: blue reference not set\n");
      return Error;
	  }

	  if( ams_bnref( module, camera, (short)(neg/(maxVoltage/255) ) ) ) {
      // set negative Reference
		  TRACE("setBlueRef: blue reference not set\n");
		  return Error;
	  }

	  return NoError;
  }

  short itiFrameGrabber::itiAMStd::getRedRef( double &neg, double &pos ) {

  	neg = (double)ams_rnref( module, camera, INQUIRE ) * (maxVoltage/255);
	  pos = (double)ams_rpref( module, camera, INQUIRE ) * (maxVoltage/255);
	  return NoError;
  }

  short itiFrameGrabber::itiAMStd::getGreenRef( double &neg, double &pos ) {

	  neg = (double)ams_gnref( module, camera, INQUIRE ) * (maxVoltage/255);
	  pos = (double)ams_gpref( module, camera, INQUIRE ) * (maxVoltage/255);
	  return NoError;
  }

  short itiFrameGrabber::itiAMStd::getBlueRef( double &neg, double &pos ) {
	  neg = (double)ams_bnref( module, camera, INQUIRE ) * (maxVoltage/255);
	  pos = (double)ams_bpref( module, camera, INQUIRE ) * (maxVoltage/255);
	  return NoError;
  }

  short itiFrameGrabber::itiAMStd::selectPort(int port) {
    int error = 0;
    TRACE( "Grabber:selectPort(%d)\n", port );
    switch ( port ) {
    case 0:
      error = amc1_sel_port( module, AMC1_PORT0 );
      break;
    case 1:
      error = amc1_sel_port( module, AMC1_PORT1 );
      break;
    default:
	    TRACE("Grabber::selectPort: false port number\n");
	    return Error;
    }

    if ( error != 0) {
      TRACE("\t kann gewuenschten Port nicht anwaehlen\n");
      return Error;
    }

	  return NoError;
  }

  short itiFrameGrabber::itiAMStd::getPort(int &port) {
	  port = ams_sel_port( module, INQUIRE );
    return NoError;
  }


  itiFrameGrabber::itiAMClr::itiAMClr( MODCNF* theIcpmod, MODCNF* theModule )
    : icpmod ( theIcpmod ), module ( theModule ) {

    short NoMatrix = 0;

	  amc1_lin_iluts( module );							        // linear input-lookuptables
	  amc1_sel_port( module, AMC1_PORT0 );				  // choose camera port 0

	  camera = amc1_get_port( module, AMC1_PORT0 );	// get camera configuration from port 0
    if ( amc1_set_port( module, camera, AMC1_PORT1 ) ) {
      // assing port 1 the same camera configuration as port 1
			TRACE("Grabber: couldn´t set camera information for Port 1\n");
	  }

    if ( amc1_oluttype( module, camera, AMC1_LINEAR ) )	{
      // set linear Output-Lookuptables
		  TRACE("Grabber: couldn´t set Output-Luts to linear\n");
	  }

    if ( amc1_get_matrix( module, "DEF_RGB_RGB", &rgbmat ) ) {
      // get RGB-matrix
  		TRACE("Grabber: couldn´t get RGB-matrix\n");
	  	NoMatrix=1;
	  }

    if ( amc1_get_matrix( module, "DEF_RGB_RYYBY", &hsimat) ) {
      // get HSI-matrix
  		TRACE("Grabber: couldn´t get HSI-matrix\n");
	  	NoMatrix=1;
	  }

	  if ( amc1_cp_matrix( module, rgbmat, &bgrmat, "DEF_RGB_BGR" ) )	{
		  TRACE("Grabber: couldn´t copy BGR-matrix\n");
		  NoMatrix=1;
	  }

	  float bgrval[9] = { 0.0, 0.0, 1.0,
                        0.0, 1.0, 0.0,
                        1.0, 0.0, 0.0  };

	  if ( amc1_mxdata ( module, bgrmat, (float *)&bgrval ) )	{
		  TRACE("Grabber: couldn´t load BGR-matrixelements\n");
		  NoMatrix=1;
	  }

	  if( !NoMatrix ) {
      // load RGB-matrix
		  amc1_load_mx ( module, rgbmat );
	  }
  }

  itiFrameGrabber::itiAMClr::~itiAMClr( void ) {
  }

  short itiFrameGrabber::itiAMClr::setRGB( void ) {
    if ( amc1_oluttype ( module, camera, AMC1_LINEAR ) ) {
      // linear Output-Lookuptables
  		TRACE("Grabber::setRGB: konnte Output-Lut nicht setzen\n");
	  	return Error;
	  }

    // load RGB-matrix
	  amc1_load_mx( module, rgbmat );
	  return NoError;
  }

  short itiFrameGrabber::itiAMClr::setBGR( void ) {
    if ( amc1_oluttype( module, camera, AMC1_LINEAR ) )	{
      // linear Output_Lookuptables
		  TRACE("Grabber::setBGR: output LUT not set\n");
		  return Error;
	  }

    // load RGB-matrix
    amc1_load_mx( module, bgrmat );
	  return NoError;
  }

  short itiFrameGrabber::itiAMClr::setHSI( void ) {
    if ( amc1_oluttype( module, camera, AMC1_HSI ) ) {
      // set output-LUT to HSI
			TRACE("Grabber::setHSI: output LUT not set\n");
	  	return Error;
	  }

    // load HSI-matrix
	  amc1_load_mx( module, hsimat );
	  return Error;
  }

  short itiFrameGrabber::itiAMClr::lowpassOn( void ) {

    if ( amc1_lowpass( module, camera, AMC1_LPFON ) )	{
      // activate low-pass filter
  		TRACE("Grabber::lowpassOn: low-pass filter not activated\n");
	  	return Error;
	  }
	  return NoError;
  }

  short itiFrameGrabber::itiAMClr::lowpassOff( void ) {
    if ( amc1_lowpass( module, camera, AMC1_LPFOFF ) ) {
      // load HSI-matrix
		  TRACE("Grabber::lowpassOff: low-pass filter not activated\n");
		  return Error;
	  }

	  return NoError;
  }

  short itiFrameGrabber::itiAMClr::setRedRef( double neg, double pos ) {
	  if( (neg < 0.0) || (neg > 1.476) || (pos < 0.7) || (pos > 1.97) || (pos < neg) ) {
		  TRACE("setRedRef: values out of range\n");
		  return Error;
	  }

    if ( amc1_rpref( module, camera, (short)(pos/(1.97/252) ) ) )	{ // set positive Reference
		  TRACE("setRedRef: red reference not set\n");
		  return Error;
	  }

    if ( amc1_rnref( module, camera, (short)(neg/(1.476/252) ) ) ) {		// set negative Reference
		  TRACE("setRedRef: red reference not set\n");
		  return Error;
	  }
	  return NoError;
  }

  short itiFrameGrabber::itiAMClr::setGreenRef( double neg, double pos ) {
    if ( (neg < 0.0) || (neg > 1.476) || (pos < 0.7) || (pos > 1.97) || (pos < neg) ) {
		  TRACE("setGreenRef: values out of range\n");
		  return Error;
	  }

    if ( amc1_gpref( module, camera, (short)(pos/(1.97/252) ) ) )	{
      // set positive Reference
		  TRACE("setGreenRef: green reference not set\n");
		  return Error;
	  }

    if ( amc1_gnref( module, camera, (short)(neg/(1.476/252) ) ) ) {
      // set negative Reference
			TRACE("setGreenRef: green reference not set\n");
	  	return Error;
	  }

	  return NoError;
  }

  short itiFrameGrabber::itiAMClr::setBlueRef( double neg, double pos ) {
	  if( (neg < 0.0) || (neg > 1.476) || (pos < 0.7) || (pos > 1.97) || (pos < neg) ) {
		  TRACE("setBlueRef: values out of range\n");
		  return Error;
	  }

    if ( amc1_bpref( module, camera, (short)(pos/(1.97/252) ) ) )	{ // set positive Reference
			TRACE("setBlueRef: blue reference not set\n");
	  	return Error;
	  }

    if( amc1_bnref( module, camera, (short)(neg/(1.476/252) ) ) ) {			// set negative Reference
		  TRACE("setBlueRef: blue reference not set\n");
		  return Error;
	  }

	  return NoError;
  }

  short itiFrameGrabber::itiAMClr::getRedRef( double &neg, double &pos ) {
	  neg = (double)amc1_rnref( module, camera, INQUIRE ) * (1.476/252);
	  pos = (double)amc1_rpref( module, camera, INQUIRE ) * (1.97/252);
	  return NoError;
  }


  short itiFrameGrabber::itiAMClr::getGreenRef( double &neg, double &pos ) {
	  neg = (double)amc1_gnref( module, camera, INQUIRE ) * (1.476/252);
	  pos = (double)amc1_gpref( module, camera, INQUIRE ) * (1.97/252);
	  return NoError;
  }

  short itiFrameGrabber::itiAMClr::getBlueRef( double &neg, double &pos ) {
	  neg = (double)amc1_bnref( module, camera, INQUIRE ) * (1.476/252);
	  pos = (double)amc1_bpref( module, camera, INQUIRE ) * (1.97/252);
	  return NoError;
  }

  short itiFrameGrabber::itiAMClr::selectPort(int port) {
    int error = 0;
    TRACE( "Grabber:selectPort(%d)\n", port );
    switch ( port ) {
    case 0:
      error = amc1_sel_port( module, AMC1_PORT0 );
      break;
    case 1:
      error = amc1_sel_port( module, AMC1_PORT1 );
      break;
    case 2:
      error = amc1_sel_port( module, AMC1_PORT2 );
      break;
    case 3:
      error = amc1_sel_port( module, AMC1_PORT3 );
      break;
    default:
		  TRACE("Grabber::selectPort: false parameter port\n");
		  return NoGrabber;
    }

    if ( error ) {
      TRACE("\t port not activated\n");
      return Error;
    }

	  return NoError;
  }

  short itiFrameGrabber::itiAMClr::getPort(int &port) {
	  short help = amc1_sel_port( module, INQUIRE );

    switch ( help ) {
      case AMC1_PORT0:
		    port = 0;
        break;
      case AMC1_PORT1:
		    port = 1;
        break;
      case AMC1_PORT2:
		    port = 2;
        break;
      case AMC1_PORT3:
		    port = 3;
        break;
      default:
        return Error;
    }
    return NoError;
  }

} // namespace lti

#endif // _USE_ITI_FRAME_GRABBER

#endif // _LTI_MSC_6
