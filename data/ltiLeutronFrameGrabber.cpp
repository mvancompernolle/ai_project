/*
 * Copyright (C) 2003, 2004, 2005, 2006
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

 
/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiLeutronGrabber.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 02.01.2003
 * revisions ..: $Id: ltiLeutronFrameGrabber.cpp,v 1.9 2006/09/05 10:36:09 ltilib Exp $
 */

#include "ltiHardwareConfig.h"
#ifdef _USE_LEUTRON_GRABBER

#include "ltiLeutronFrameGrabber.h"
#include <sstream>

//if debug is set to 1 then contruction messages like the connected camera
// are output only. For bigger debugs also messages during operation are output
#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#include <ltiTimer.h>
using std::cout;
using std::endl;
using std::flush;
#endif

namespace lti {
  // -----------------------------------------------------------------
  // leutronFrameGrabber::static members
  // -----------------------------------------------------------------
  LvGrabberNode* leutronFrameGrabber::m_pGrabber      = 0;
  HGRABBER* leutronFrameGrabber::m_pGrabberHandle     = 0;
  HCONNECTOR* leutronFrameGrabber::m_pConnectorHandle = 0;
  int leutronFrameGrabber::m_numConnectedCameras      ( 0 );
  int leutronFrameGrabber::m_instanceCount            ( 0 );
  int leutronFrameGrabber::m_numTargets               ( 0 );
  bool leutronFrameGrabber::m_greyOnly            ( false );
  U16BIT leutronFrameGrabber::m_masterCameraType      ( 0 );
  U16BIT leutronFrameGrabber::m_slaveCameraType       ( 0 ); 
  int leutronFrameGrabber::m_srcWidth                 ( 0 );
  int leutronFrameGrabber::m_srcHeight                ( 0 );
  bool leutronFrameGrabber::m_connect              ( true );
  bool leutronFrameGrabber::m_activate             ( true );
  bool leutronFrameGrabber::m_otherCamera          ( true );
  bool leutronFrameGrabber::m_turnaround           ( true );
  U8BIT leutronFrameGrabber::m_tgt[4] = { Tgt_1, Tgt_2, Tgt_3, Tgt_4 };


  // --------------------------------------------------
  // leutronFrameGrabber::parameters
  // --------------------------------------------------

  // default constructor
  leutronFrameGrabber::parameters::parameters() 
    : frameGrabber::parameters(),
      targetIndex(0),
      camera(std::string("PAL_S_CCIR")),
      cameraIndex(0),
      scaleFactorHPS(1,1),
      autoDetectCamera(true),
      scaleFactorBRS(1,1),
      waitSync(None),
      offset(0,0),
      scale(true),
      color(RGBa)
  {
    target[0] = true;
    target[1] = target[2] = target[3] = false;     
  }

  // copy constructor
  leutronFrameGrabber::parameters::parameters(const parameters& other) {
    copy(other);
  }

  // destructor
  leutronFrameGrabber::parameters::~parameters() {
  }

  // get type name
  const char* leutronFrameGrabber::parameters::getTypeName() const {
    return "leutronFrameGrabber::parameters";
  }
  
  // copy member

  leutronFrameGrabber::parameters& 
    leutronFrameGrabber::parameters::copy(const parameters& other) {
# ifndef _LTI_WIN32
    // MS Visual C++ 6 is not able to compile this...
    frameGrabber::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    frameGrabber::parameters& (frameGrabber::parameters::* p_copy)
      (const frameGrabber::parameters&) = 
      frameGrabber::parameters::copy;
    (this->*p_copy)(other);
# endif
    
      targetIndex = other.targetIndex;
      camera = other.camera;
      cameraIndex = other.cameraIndex;
      scaleFactorHPS = other.scaleFactorHPS;
      autoDetectCamera = other.autoDetectCamera;
      scaleFactorBRS = other.scaleFactorBRS;
      waitSync = other.waitSync;
      offset = other.offset;
      scale = other.scale;
      for (int i=0; i<4; i++)
        target[i] = other.target[i];
      color = other.color;

    return *this;
  }

  // alias for copy member
  leutronFrameGrabber::parameters& 
  leutronFrameGrabber::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* leutronFrameGrabber::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_WIN32
  bool leutronFrameGrabber::parameters::write(ioHandler& handler,
                                         const bool complete) const 
# else
  bool leutronFrameGrabber::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const 
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"targetIndex",targetIndex);
      lti::write(handler,"camera",camera);
      lti::write(handler,"cameraIndex",cameraIndex);
      lti::write(handler,"scaleFactorHPS",scaleFactorHPS);
      lti::write(handler,"autoDetectCamera",autoDetectCamera);
      lti::write(handler,"scaleFactorBRS",scaleFactorBRS);
      switch(waitSync) {
      case Field1:
        lti::write(handler,"waitSync","Field1");
	break;
      case Field2:
        lti::write(handler,"waitSync","Field2");
	break;
      case Frame:
        lti::write(handler,"waitSync","Frame");
	break;
      case VertSync:
        lti::write(handler,"waitSync","VertSync");
	break;
      default:
        lti::write(handler,"waitSync","None");
	break;
      }
      lti::write(handler,"waitSync",waitSync);
      lti::write(handler,"offset",offset);
      lti::write(handler,"scale",scale);
      lti::write(handler,"target", target[0]);
      for (int i=1; i<4; i++)
      lti::write(handler," , ",target[i]);
      switch (color) {
      case Grey:
        lti::write(handler,"color", "Grey");
        break;
      case YUV:
        lti::write(handler,"color", "YUV");
        break;
      default:
        lti::write(handler,"color", "RGBa");
        break;
      }
    }

# ifndef _LTI_WIN32
    // This is the standard C++ code, which MS Visual C++ 6 is not able to 
    // compile...
    b = b && frameGrabber::parameters::write(handler,false);
# else
    bool (frameGrabber::parameters::* p_writeMS)
      (ioHandler&,const bool) const = 
      frameGrabber::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif
    
    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }
  
# ifdef _LTI_WIN32
  bool leutronFrameGrabber::parameters::write(ioHandler& handler,
                                         const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /*
   * read the parameters from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if write was successful
   */
# ifndef _LTI_WIN32
  bool leutronFrameGrabber::parameters::read(ioHandler& handler,
                                        const bool complete) 
# else
  bool leutronFrameGrabber::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }
    
    if (b) {
      std::string str;
      lti::read(handler,"targetIndex",targetIndex);
      lti::read(handler,"camera",camera);
      lti::read(handler,"cameraIndex",cameraIndex);
      lti::read(handler,"scaleFactorHPS",scaleFactorHPS);
      lti::read(handler,"autoDetectCamera",autoDetectCamera);
      lti::read(handler,"scaleFactorBRS",scaleFactorBRS);
      lti::read(handler,"waitSync",str);
      if (str == "Field1")
        waitSync = Field1;
      else if (str == "Field2")
        waitSync = Field2;
      else if (str == "Frame")
        waitSync = Frame;
      else if (str == "VertSync")
        waitSync = VertSync;
      else
        waitSync = None;
      lti::read(handler,"offset",offset);
      lti::read(handler,"scale",scale);
      lti::read(handler,"target[4]",target[4]);
      lti::read(handler,"color",str);
      if (str == "YUV")
        color = YUV;
      else if (str == "Grey")
        color = Grey;
      else
        color = RGBa;
    }
    
# ifndef _LTI_WIN32
    // This is the standard C++ code, which MS Visual C++ 6 is not able to 
    // compile...
    b = b && frameGrabber::parameters::read(handler,false);
# else
    bool (frameGrabber::parameters::* p_readMS)(ioHandler&,const bool) = 
      frameGrabber::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_WIN32
  bool leutronFrameGrabber::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // leutronFrameGrabber
  // --------------------------------------------------

  // default constructor
  leutronFrameGrabber::leutronFrameGrabber()
    : frameGrabber() {

    _lti_debug3(" leutronFrameGrabber()\n");
     m_instanceCount++;
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    if ( !setParameters(defaultParameters) ) {
      throw exception( getStatusString() );
    }
  }

  // default constructor
  leutronFrameGrabber::leutronFrameGrabber(const parameters& par)
    : frameGrabber() {

    _lti_debug3(" leutronFrameGrabber(const parameters& par)\n");
    m_instanceCount++;
    // set the given parameters
    if ( !setParameters(par) ) {
      throw exception( getStatusString() );
    }
  }


  // copy constructor
  leutronFrameGrabber::leutronFrameGrabber(const leutronFrameGrabber& other) {
    copy(other);
  }

  // destructor
  leutronFrameGrabber::~leutronFrameGrabber() {
 
    //ToDo: make mutex, this section changes static members
    m_instanceCount--;
    if ( m_instanceCount == 0 ) {
      if ( m_pGrabberHandle   != 0 ) delete m_pGrabberHandle;
      if ( m_pConnectorHandle != 0 ) delete m_pConnectorHandle;
      DsyClose();
    } 
  }

  // returns the name of this type
  const char* leutronFrameGrabber::getTypeName() const {
    return "leutronFrameGrabber";
  }

  // copy member
  leutronFrameGrabber& leutronFrameGrabber
  ::copy(const leutronFrameGrabber& other) {
    frameGrabber::copy(other);
    return (*this);
  }

  // alias for copy member
  leutronFrameGrabber&
    leutronFrameGrabber::operator=(const leutronFrameGrabber& other) {
    return (copy(other));
  }


  // clone member
  functor* leutronFrameGrabber::clone() const {
    return new leutronFrameGrabber(*this);
  }

  // return parameters
  const leutronFrameGrabber::parameters&
    leutronFrameGrabber::getParameters() const {
    const parameters* par = 
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException();
    }
    return *par;
  }

  // return writable parameters
  leutronFrameGrabber::parameters&
    leutronFrameGrabber::getWritableParameters() {
    parameters* par = 
      dynamic_cast<parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException();
    }
    return *par;
  }

  //set parameters
  bool leutronFrameGrabber::setParameters(const leutronFrameGrabber::parameters &theParams) {

    if ( !checkParameters(theParams) ) { return false; }
    //remember the parameters
    if ( !frameGrabber::setParameters(theParams) ) { return false; }
    //initialize framegrabber - requires parameters to be set
    if ( !init() ) { return false; }

    return true;
  }

  //check parameters
  bool leutronFrameGrabber
  ::checkParameters(const leutronFrameGrabber::parameters &theParams) {
    
   //range checks
    if ( theParams.color != parameters::Grey && m_greyOnly ) {
      setStatusString
	("leutronFrameGrabber::checkParameters: Monochrome only.");
      return false;
    }
    if ( theParams.targetIndex < 0 ||
	 ( m_numTargets > 0 && theParams.targetIndex >= m_numTargets ) ) {
      std::ostringstream error;
      error << "leutronFrameGrabber::checkParameters: "
	    << "Invalid target index. Range: [0,"
	    << m_numTargets << ")";
      setStatusString(error.str().c_str());
      return false;
    }

    if ( theParams.cameraIndex < 0 ||
	 ( m_numConnectedCameras > 0 && 
	   theParams.cameraIndex >= m_numConnectedCameras ) ) {
      std::ostringstream error;
      error <<"leutronFrameGrabber::init: Invalid camera index. Range: [0,"
	    << m_numConnectedCameras << ")";
      setStatusString(error.str().c_str());
      return false;
    }
    if ( theParams.offset.x < 0 || theParams.offset.y < 0 ) {
      setStatusString
	("leutronFrameGrabber::checkParameters: Offset into image must be bigger or equal than (0,0)");
      return false;
    }
    if ( theParams.scaleFactorHPS.x < 0 || theParams.scaleFactorHPS.y < 0 ) {
      setStatusString
	("leutronFrameGrabber::checkParameters: Scaleratio on HPS path must be bigger or equal than (0,0)");
      return false;
    }
    if ( theParams.scaleFactorBRS.x < 0 || theParams.scaleFactorHPS.y < 0 ) {
      setStatusString
	("leutronFrameGrabber::checkParameters: Scaleratio on BRS path must be bigger or equal than (0,0)");
      return false;
    }

    //check wether reinitialization of framegrabber necessary
    //not needed here: waitSync and targetIndex

    //at beginning parameters are null
    const parameters* lastPar = 
      dynamic_cast<const parameters*>(&functor::getParameters());
    if ( isNull(lastPar) ) {
      m_connect = true; m_activate = true;
      return true;
    }

    if ( theParams.camera != lastPar->camera ||
	 theParams.autoDetectCamera != lastPar->autoDetectCamera ) {
      m_connect = true;
      m_activate = true;
    }
    else if ( theParams.scaleFactorHPS != lastPar->scaleFactorHPS ||
	      theParams.scaleFactorBRS != lastPar->scaleFactorBRS ||
	      theParams.offset != lastPar->offset ||
	      theParams.scale != lastPar->scale ||
	      !theParams.target[theParams.targetIndex] ||
	      theParams.color != lastPar->color ) {
      m_activate = true;
    } 
    //multiplex grabber does not perform activation if camera index changes
    if ( theParams.cameraIndex != lastPar->cameraIndex ) {
      m_otherCamera = true;
    }
    if ( theParams.turnaround != lastPar->turnaround && m_pGrabber != 0) {
      m_turnaround = true;
    }
    
    return true;
  }

  bool leutronFrameGrabber::init() {

#if defined _LTI_DEBUG && _LTI_DEBUG > 2
    cout << " leutronFG::init " << m_instanceCount << endl;
#endif

    //if ( m_instanceCount > 1 ) return true;
#if defined _LTI_DEBUG && _LTI_DEBUG > 2
    //cout << " ....initializing " << endl;
#endif  
 
    if ( m_pGrabberHandle == 0 ) {
      //only initialize daisy if no other instance did so
      DsyInit();

      if ( !detectCameras() ) {
	return false;
      }

      //number of targets was not known so far
      const parameters& par = getParameters();
      if ( par.targetIndex >= m_numTargets ) {
	std::ostringstream error;
	error <<"leutronFrameGrabber::init: Invalid target index. Range: [0,"
	      << m_numTargets << ")";
	setStatusString(error.str().c_str());
	return false;
      }
      
      //number of cameras was not known before
      if ( par.cameraIndex >= m_numConnectedCameras ) {
	std::ostringstream error;
	error <<"leutronFrameGrabber::init: Invalid camera index. Range: [0,"
	      << m_numConnectedCameras << ")";
	setStatusString(error.str().c_str());
	return false;
      }

      if ( !activateCameras() ) { return false; }
      return turnaround();

    } else {
      if ( m_connect ) { 
	if ( !detectCameras() ) { return false; }
      }
      if ( m_activate || m_otherCamera ) {
	if ( !activateCameras() ) { return false; }
      }
      if ( m_turnaround ) { 
	return turnaround();
      }
    }
    
    return true;
  }

  // Switches the current camera
  bool leutronFrameGrabber::setCamera(int cameraIndex) {
 
    if ( cameraIndex < 0 || cameraIndex >= m_numConnectedCameras ) {
      std::ostringstream error;
      error << "leutronFrameGrabber::setCamera: "
	    << "Invalid camera index. Range: [0,"
	    << m_numConnectedCameras << ")";
      setStatusString(error.str().c_str());
      return false;
    }
    
    parameters& lastPar = getWritableParameters();
    if (cameraIndex != lastPar.cameraIndex) {
      lastPar.cameraIndex = cameraIndex;
      activateCameras();
    }
    
    return true;
  }

  //Switches the target buffer, from which is grabbed.
  bool leutronFrameGrabber::setTarget(int targetIndex) {

    if ( targetIndex < 0 || targetIndex >= m_numTargets ) {
      std::ostringstream error;
      error << "leutronFrameGrabber::setTargetBuffer: "
	    << "Invalid target index. Range: [0,"
	    << m_numTargets << ")";
      setStatusString(error.str().c_str());
      return false;
    }

    parameters& lastPar = getWritableParameters();
    if ( targetIndex != lastPar.targetIndex ) {
      lastPar.targetIndex = targetIndex;
      if ( !lastPar.target[targetIndex] )
	return activateCameras();
    }
    
    return true;
  }

  //Get the number of connected cameras
  int leutronFrameGrabber::getNumberOfConnectedCameras() {

    return m_numConnectedCameras;
  }

  // -------------------------------------------------------------------
  // Leutron stuff
  // -------------------------------------------------------------------

  //set turnaround, only has an affect after camera activation
  bool leutronFrameGrabber::turnaround() {

    const parameters& par = getParameters();
    LvCameraNode *camera 
      ( m_pGrabber->GetCameraPtr
	(m_pGrabber->GetConnectedCamera(par.cameraIndex)) );
    camera->Freeze(par.waitSync);
 
    //Leutron also supports Mirror_Vertical and Mirror_Horizontal
    if ( par.turnaround ) {
      camera->SetMirrorType(Mirror_HV);  //swap around x and y axis
    } else {
      camera->SetMirrorType(Mirror_None);
    }

    camera->Live(par.waitSync);    
    m_turnaround = false;

    return true;
  }


  bool leutronFrameGrabber::stopAquisition() const {

    const parameters& par = getParameters();
    LvCameraNode *camera
      ( m_pGrabber->GetCameraPtr
	(m_pGrabber->GetConnectedCamera(par.cameraIndex)) );
    camera->Freeze(par.waitSync);

    return true;
  }

  bool leutronFrameGrabber::startAquisition() const {

    const parameters& par = getParameters();
    LvCameraNode *camera
      ( m_pGrabber->GetCameraPtr
	(m_pGrabber->GetConnectedCamera(par.cameraIndex)) );
    camera->Live(par.waitSync);
    
    return true;
  }

  //Master and slave camera type are only needed by the multiplex grabber.
  //I do not wan't to duplicate the entire code because of these few lines.
  //Detection is usually done once only.
  bool leutronFrameGrabber::detectCameras() { 

    STATUS status;
    if ( ( status = DsyRecheckConnector() ) != I_NoError ) {
      return !setDsyErrorInStatusString(status, 
					"leutronFrameGrabber::detectCameras");
    }
    HANDLE hConnInfo(0);
    
    //autodetection
    if ( ( m_numConnectedCameras = DsyDetectCamera(&hConnInfo) ) <= 0 ) {
      setStatusString
	("leutronFrameGrabber::detectCameras: Could not detect any camera on grabber");
      return false;
    }

    LvCameraConnDlgInfo *camDlgInfo( (LvCameraConnDlgInfo *) GlobalLock(hConnInfo) );
    if ( camDlgInfo == 0 ) {
      setStatusString
	("leutronFrameGrabber::detectCameras: Could not get connection info");
      return false;
    }

    //use the camera specified in the parameters
    int cameraId(-1);
    const parameters& par = getParameters();
    if ( par.autoDetectCamera == false ) {
      LvCameraInfo camInfo;
      int i;
      for(i=0; DsyEnumCameraType(i,&camInfo)==I_NoError; i++) {
	//uncomment this to read all camera names in lvcamera.dat
	//cout << "    in lvcamera.dat " << camInfo.Name << endl; 
	if( par.camera.compare(camInfo.Name) == 0 ) {
	  cameraId=camInfo.Id;
	  break;
	}
      }
      if (cameraId == -1) {
	setStatusString
	  ("leutronFrameGrabber::detectCameras: Could not find the camera in lvCamera.dat");
	return false;
      }
    }
    
    //set master and slave
    int i ( 0 );
    bool secondGrabber ( false );
    while ( i<m_numConnectedCameras-1 && ! secondGrabber ) {
      if (camDlgInfo[i].Grabber != camDlgInfo[i+1].Grabber) {
	secondGrabber = true;
      }
      i++;
    }
    if ( !par.autoDetectCamera ) {
      m_masterCameraType = m_slaveCameraType = cameraId;
    } else {
      m_masterCameraType = m_slaveCameraType = camDlgInfo[0].CameraType;
      //ToDo: Rethink when testing a two grabber system
      //if (secondGrabber) m_slaveCameraType = camDlgInfo[i].CameraType;
    }

    if ( m_pGrabberHandle != 0 ) { delete m_pGrabberHandle; }
    m_pGrabberHandle = new HGRABBER[m_numConnectedCameras];
    if ( m_pConnectorHandle != 0 ) { delete m_pConnectorHandle; }
    m_pConnectorHandle = new HCONNECTOR[m_numConnectedCameras];
    

    //The camera-order is not preserved on VICTORIA due to some bug 
    //  - in leutron stuff ??
    //Therefore I check the order by hand
    bool connectedChinch (false), connectedCVBS0(false), connectedCVBS1(false);

    //Check that entries in LvCameraConnDlgInfo are correct
    //and remember the grabber and connector handle
    for (i=0; i<m_numConnectedCameras; i++) { //checking entries
  
      if ( camDlgInfo[i].Grabber == 0 ) {
	setStatusString
	  ("leutronFrameGrabber::detectCameras: null pointer to grabber in camDlgInfo");
	return false;
      }
      if ( camDlgInfo[i].hGrabber == HANDLE_INVALID ) {
	setStatusString
	  ("leutronFrameGrabber::detectCameras: invalid grabber handle in camDlgInfo");
	return false;
      }
      if ( camDlgInfo[i].hConn == HANDLE_INVALID ) {
	setStatusString
	  ("leutronFrameGrabber::detectCameras: invalid connector handle in camDlgInfo");
	return false;
      }
      
#if defined _LTI_DEBUG && _LTI_DEBUG > 0
      cout << " connecting camera " << i
	   << " on grabber " <<  camDlgInfo[i].Grabber->GetName() 
	   << " ... " << endl;
#endif      

      if ( par.autoDetectCamera ) {
	cameraId = camDlgInfo[i].CameraType;
      }
      HCAMERA hCamera ( HANDLE_INVALID );              //remember camera handle
      if ( (hCamera = camDlgInfo[i].Grabber
	    ->ConnectCamera(cameraId,
			    camDlgInfo[i].hConn,
			    camDlgInfo[i].SyncNr) )
	   == HANDLE_INVALID ) {
	setStatusString
	  ("leutronFrameGrabber::detectCameras: Could not connect a camera");
	return false;
      }
#if defined _LTI_DEBUG && _LTI_DEBUG > 1
      cout << " connected " << hCamera << flush << endl;
#endif      

      //remember grabber handle
      m_pGrabberHandle[i]   = camDlgInfo[i].hGrabber;
      m_pConnectorHandle[i] = camDlgInfo[i].hConn;
      
      LvCameraNode *camera = camDlgInfo[i].Grabber->GetCameraPtr(hCamera);
      if ( camera == 0 ) {
	setStatusString
	  ("leutronFrameGrabber::detectCameras: Null pointer to cameraNode");
	return false;
      }

      //show which camera was detected or connected
      char tmp[128];
      camera->GetDescription(tmp, sizeof(tmp));
#if defined _LTI_DEBUG && _LTI_DEBUG > 0
      cout << " connected camera " <<  tmp << endl << flush;
#endif 
      camDlgInfo[i].Grabber->GetConnectorName(hCamera, tmp, sizeof(tmp));
#if defined _LTI_DEBUG && _LTI_DEBUG > 0
      cout << " auto detected connector " << tmp << endl << flush;
#endif      

      //for some strange reason CVBS1 on victoria is detected as Chinch
      //  and connected first
      //ToDo: if multiple grabbers make sure that checked for each grabber
      //      seperately
      if (tmp[1] == 'h' && tmp[2] == 'i') connectedChinch = true;
      if (tmp[1] == 'V' && tmp[4] == '0') connectedCVBS0 = true;
      if (tmp[1] == 'V' && tmp[4] == '1') connectedCVBS0 = true;
    }

    //fix above error, this check is only performed if one grabber is installed
    if ( connectedChinch == true    && connectedCVBS0 == true 
	 && connectedCVBS1 == false && !secondGrabber ) {
      //check for CVBS1 on grabber with Chinch, Chinch is first entry
      DsyRecheckConnector();
      if ( par.autoDetectCamera ) {
	cameraId = camDlgInfo[0].CameraType;
      }
      int numConnectors ( camDlgInfo[0].Grabber
			  ->GetNrFreeConnectorEx(cameraId) );
#if defined _LTI_DEBUG && _LTI_DEBUG > 1
      cout << "numConnectors " << numConnectors << endl;
#endif
      LvConnectionInfo *connInfo ( new LvConnectionInfo[numConnectors] );
      camDlgInfo[0].Grabber->GetFreeConnectorInfoEx(cameraId,connInfo);
      for(i=0; i<numConnectors; i++) {
	//std::string name("CVBS");
	//if ( name.compare(connInfo[i].Description, 0, name.length()) == 0 ) {
	std::string name ( connInfo[i].Description );
	if ( name.find("CVBS") != 0 ) {
	  _lti_debug1
	    ("WARNING: reconnecting all cameras due to CVBS1-error\n");
	}
	HCAMERA hCamera(HANDLE_INVALID);
	//numConnectors >= m_numConnectedCameras
	if ( (hCamera = camDlgInfo[0].Grabber
	      ->ConnectCamera(cameraId,
			      connInfo[i].hConn,
			      connInfo[i].SyncNr) )
	     == HANDLE_INVALID ) {
	  setStatusString
	    ("leutronFrameGrabber::detectCameras: Could not connect camera (CVBS1)");
	  return false;
	}
	m_pConnectorHandle[i] = connInfo[i].hConn;
      }
      delete [] connInfo;
    }
    m_numTargets = camDlgInfo[0].Grabber->GetNrTarget();
    
    //Get the minimum of detected targets in the system
    //The color frame grabber only supports two targets
    for ( i=0; i < m_numConnectedCameras; i++ ) {

      m_pGrabber = DsyGetGrabberPtrFromHandle(m_pGrabberHandle[i]);
      if ( m_pGrabber == 0 ) {
	setStatusString
	  ("leutronFrameGrabber::detectCameras: No grabber initialized.");
	return false;
      }
      
      std::string name ( m_pGrabber->GetName() );
      m_numTargets = min(m_numTargets, m_pGrabber->GetNrTarget() );
      if ( name.find("olor") != 0 ) {
	m_numTargets = min(m_numTargets, 2);
      }
    }

    //ToDo: Test if a monochrome grabber is installed.
    //m_greyOnly = ...
    
    //GlobalUnlock(hConnInfo);  //gcc3.3.1: Warnung: statement with no effect
    GlobalFree(hConnInfo); 

    m_connect = false;
    return true;
  }
  
  bool leutronFrameGrabber::activateCameras() {

    _lti_debug3(" leutronFG::activateCameras\n");
    if ( m_pGrabberHandle == 0 ) {
      setStatusString
	("leutronFrameGrabber::activateCameras: No grabbers initialized.");
      return false;
    }
    parameters& par = getWritableParameters();
    //remember pointer current grabber
    if ( par.cameraIndex < m_numConnectedCameras ) {
      m_pGrabber = DsyGetGrabberPtrFromHandle
	(m_pGrabberHandle[par.cameraIndex]);
    }
    if ( m_pGrabber == 0 ) {
      setStatusString
	("leutronFrameGrabber::activateCameras: No grabber initialized.");
      return false;
    }
    HCAMERA hCamera ( m_pGrabber->GetConnectedCamera(par.cameraIndex) );
    LvCameraNode *camera ( m_pGrabber->GetCameraPtr(hCamera) );
    if ( camera == NULL) {
      setStatusString
	("leutronFrameGrabber::activateCameras: Null pointer to LvCameraNode");
      return false;
    }
    camera->Freeze(par.waitSync);

    par.target[par.targetIndex] = true;  //at least current target must be on
    U8BIT activeTargets ( 0 );
    int startPosY ( 0 );
    int i;
    for (i=0; i<4; i++) {
      if (par.target[i] == false) continue;

      //CAVEAUT: currently only two targets are supported
      if ( i > 1 ) {
	setStatusString
	  ("leutronFrameGrabber::activateCameras: only two targets supported so far");
	return false;
      }
      
      activeTargets |= m_tgt[i];
      STATUS status ( m_pGrabber->ActivateCamera(hCamera, activeTargets) );
      //STATUS status ( m_pGrabber->ActivateCamera(hCamera, m_tgt[i] ) );
      if ( status != DSY_I_NoError ) {
	return !setDsyErrorInStatusString
	  (status, "leutronFrameGrabber::activateCameras:");
      }
      
      //set scale mode: Important: acivate first
      if (par.scale) {
	m_pGrabber->SetScaleMode(hCamera,SMd_Scale, m_tgt[i]);
      } else {
	m_pGrabber->SetScaleMode(hCamera,SMd_Clip, m_tgt[i]);
      }
      
      //reset some  parameter of the source roi, i.e. offset inside image
      LvSourceInfo srcInfo;
      camera->GetSourceROI(&srcInfo);    //after activation
      srcInfo.StartX = par.offset.x;
      srcInfo.StartY = par.offset.y;

#if defined _LTI_DEBUG && _LTI_DEBUG > 2
      cout << "..max. src dim. " << srcInfo.MaxWidth 
           << " x " << srcInfo.MaxHeight << endl;
      //cout << "..src start     " << srcInfo.StartX 
      //<< " x " << srcInfo.StartY << endl;
#endif
      if ( ( status = camera->SetSourceROI(&srcInfo) ) != DSY_I_NoError ) {
	return !setDsyErrorInStatusString
	  (status, "leutronFrameGrabber::activateCameras:");
      }
      
      LvROI roi;
      m_pGrabber->GetConnectionInfo(hCamera,&roi, m_tgt[i]);
      //reset some parameter of the target roi
      roi.SetTargetBuffer(TgtBuffer_CPU);
      roi.SetDIBMode(TRUE);
      //BRS or HPS scale factor ?
      m_srcWidth  = srcInfo.Width;
      m_srcHeight = srcInfo.Height;
      if ( i == 1 ) {	
	roi.SetDimension((U16BIT)( m_srcWidth / par.scaleFactorBRS.x ),
			 (U16BIT)( m_srcHeight / par.scaleFactorBRS.y ) );
      } else {
	roi.SetDimension((U16BIT)( m_srcWidth / par.scaleFactorHPS.x ),
			 (U16BIT)( m_srcHeight / par.scaleFactorHPS.y ) );
      }
      roi.SetMemoryWidth(roi.GetWidth());
 
      if( i == 1 && par.color == parameters::RGBa ) {
	//BRS-Tgt_2 supports YUV 4:2:2 color format or Y only
	roi.SetColorFormat(ColF_YUV_422); //YUV
	par.color = parameters::YUV;
      } else {
	switch ( par.color ) {
	case parameters::YUV: //8 bit
	  roi.SetColorFormat(ColF_YUV_422);
	  break;
	case parameters::Grey: //8 bit
	  roi.SetColorFormat(ColF_Mono_8);
	  break;
	default:  //RGBa 32 bit
	  roi.SetColorFormat( ColF_RGB_888a);
	  break;
	}
      }

      roi.SetStartPosition(0, startPosY);
      if ( ( status = m_pGrabber->ActivateROI(hCamera,&roi, m_tgt[i]) )
	   != DSY_I_NoError ) {
	return !setDsyErrorInStatusString
	  (status, "leutronFrameGrabber::activateCameras:");
      }

      //adjust start position of the roi and consider size of each pixel
      //otherwise all images would be written in the same memory location
      startPosY += roi.GetPixelIncrement() * roi.GetHeight();
    } //for all targets
    
    camera->Live(par.waitSync);
    
    m_activate = m_otherCamera = false;
    return true;
  }


  // -----------------------------
  // set the optocouplers
  //------------------------------
  bool leutronFrameGrabber::setOptocoupler(eOptocouplerType theOpto) {
    U16BIT optoTable[33] = 
      { 0x00, Opto_0, Opto_1, Opto_2, Opto_3, Opto_4, Opto_5,
	Opto_6, Opto_7, Opto_8, Opto_9, Opto_10, Opto_11, Opto_12,
	Opto_13, Opto_14, Opto_15, Opto_16, Opto_17, Opto_18, Opto_19,
	Opto_20, Opto_21, Opto_22, Opto_23, Opto_24, Opto_25, Opto_26,
	Opto_27, Opto_28, Opto_29, Opto_30, Opto_31 };
    
    U16BIT mask ( optoTable[theOpto] );
    return m_pGrabber->SetOptocoupler(mask);    
  }

  bool leutronFrameGrabber
  ::setOptocoupler(const std::vector<eOptocouplerType>& theOptos) {
    U16BIT optoTable[33] =
      { 0x00, Opto_0, Opto_1, Opto_2, Opto_3, Opto_4, Opto_5,
	Opto_6, Opto_7, Opto_8, Opto_9, Opto_10, Opto_11, Opto_12,
	Opto_13, Opto_14, Opto_15, Opto_16, Opto_17, Opto_18, Opto_19,
	Opto_20, Opto_21, Opto_22, Opto_23, Opto_24, Opto_25, Opto_26,
	Opto_27, Opto_28, Opto_29, Opto_30, Opto_31 };
    
    //ToDo: 16 or 32?? - U32BIT was in the old code,
    // is 16BIT in Leutron manual
    U16BIT mask ( 0x00 );
    std::vector<eOptocouplerType>::const_iterator it ( theOptos.begin() );
    std::vector<eOptocouplerType>::const_iterator end ( theOptos.end() );
    for ( ; it!=end; it++ ) {
      mask |= optoTable[*it];
    }
    
    return m_pGrabber->SetOptocoupler(mask);
  }
  
  bool leutronFrameGrabber::setDsyErrorInStatusString(STATUS& status,
						      const char* error) {
    
    char errorMsg[1024];
    DsyGetErrorMsg(status, errorMsg, sizeof(errorMsg));
    std::string errorStr ( error );
    errorStr += std::string(errorMsg);
    setStatusString(errorStr.c_str());

    return true;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------
  bool leutronFrameGrabber::initRoi(LvROI& roi) {
    
    const parameters& par = getParameters();
    HCAMERA hCamera = m_pGrabber->GetConnectedCamera(par.cameraIndex);
    m_pGrabber->GetConnectionInfo(hCamera,&roi, 
				  m_tgt[par.targetIndex]);
    return true;
  }

  bool leutronFrameGrabber::apply(image& srcdest) {

    //if ( m_instanceCount > 1 ) return false;
#if defined _LTI_DEBUG && _LTI_DEBUG > 0
    lti::timer chron;
    chron.start();
#endif
    stopAquisition();
    LvROI roi;
    initRoi(roi);
    
    //roi dimensions might not fit parameter/image settings
    const int width  ( roi.GetWidth() );
    const int height ( roi.GetHeight() );
    srcdest.resize(height, width, 0, false, false);
    
    int lineIncr ( roi.GetLineIncrement() / roi.GetPixelIncrement() - width );
#if defined _LTI_DEBUG && _LTI_DEBUG > 2
    cout << "roi pixel incr " << roi.GetPixelIncrement() 
	 << " line incr " << lineIncr 
	 << " start address " << roi.StartAddress << endl;
#endif

    //CAUTION: if two targets are activated the pixel incr is corrupted,
    //         the color format is alway set to grey

    rgbPixel* srcPtr = &srcdest.at(0);
    //with the iterator it takes about twice as long
    //image::iterator srcPtr ( srcdest.begin() );

    //assumes that parameters::color is passed on to roi
    //if ( roi.GetColorFormat() == ColF_RGB_888a ) {
    const parameters& par = getParameters();
    if ( par.color == parameters::RGBa ) {
      //RGB or RGBalpha
      uint32 *roiPtr=(uint32 *)(roi.MemoryInfo.BaseAddress+roi.StartAddress);
      int j ( 0 );
      for (; j<height; j++) {
	int i ( 0 );
	for (; i<width; i++) {
	  (*srcPtr++).setValue(*(roiPtr++));
	}
	roiPtr += lineIncr;
      }

      //} else if ( roi.GetColorFormat() == ColF_YUV_422 ) {
    } else if ( par.color == parameters::YUV ) {
      //YUV has two BYTES (4:2:2), UV exchange on x-axis
      //for uneven width the lineincrement is increased by one
      //lineIncr is no multiple of pixelIncr neither -> correct
      U8BIT *roiPtr=(U8BIT *)(roi.MemoryInfo.BaseAddress+roi.StartAddress);
      if ( width % 2 != 0 && lineIncr % 2 == 1 ) { 
	lineIncr += 1;
#if defined _LTI_DEBUG && _LTI_DEBUG > 1
	cout << " new line incr " << lineIncr << endl;
#endif
      }
      const int width_1 ( width-1 );
      int j ( 0 );
      for (; j<height; j++) {
	ubyte u ( 0 );
	int i ( 0 );
	for (; i<width_1; i+=2) {	  
	  u = *(roiPtr++);
	  const ubyte y ( *(roiPtr++) );
	  const ubyte v ( *(roiPtr++) );  
	  (*srcPtr++).set(y,u,v);
	  (*srcPtr++).set(*roiPtr++,u,v);
	}
	if ( i < width ) {     //uneven width
	  const ubyte v ( *(roiPtr++) );
	  (*srcPtr++).set(*roiPtr++,u,v);
	}
	roiPtr += lineIncr;
      }
    } else {
      //grey mode has only 1 BYTE: set all colors to the same intensity
      U8BIT *roiPtr=(U8BIT *)(roi.MemoryInfo.BaseAddress+roi.StartAddress);
      int j ( 0 );
      for (; j<height; j++) {
	int i ( 0 );
	for (; i<width; i++) {
	  const ubyte g ( *(roiPtr++) );
	  (*srcPtr++).set(g,g,g);
	}
	roiPtr += lineIncr;
      }
    }
    startAquisition();
#if defined _LTI_DEBUG && _LTI_DEBUG > 1
    chron.stop();
    cout << "...apply " << float(chron.getTime())/1000000 << endl;
#endif
    return true;
  }

  bool leutronFrameGrabber::apply(channel8& srcdest) {

#if defined _LTI_DEBUG && _LTI_DEBUG > 0
    lti::timer chron;
    chron.start();
#endif
    stopAquisition();
    LvROI roi;
    initRoi(roi);
    
    //roi dimensions might not fit parameter/image settings
    const int width  ( roi.GetWidth() );
    const int height ( roi.GetHeight() );
    srcdest.resize(height, width, 0, false, false);
    
    int lineIncr ( roi.GetLineIncrement() / roi.GetPixelIncrement() - width );
    U8BIT *roiPtr = (U8BIT *)(roi.MemoryInfo.BaseAddress+roi.StartAddress);
    ubyte* srcPtr = &srcdest.at(0);

    const parameters& par = getParameters();
    //if ( roi.GetColorFormat() == ColF_RGB_888a ) {  //RGBa
    if ( par.color == parameters::RGBa ) {
      //RGB or RGBalpha
      const int pi (1);
      int j ( 0 );
      for (; j<height; j++) {
	int i ( 0 );
	for (; i<width; i++) {
	  (*srcPtr++) 
	    = ubyte ( (*(roiPtr++) + *(roiPtr++) + *(roiPtr++)) / 3 );
	  roiPtr += pi;
	}
	roiPtr += lineIncr;
      }

    } else if ( par.color == parameters::YUV ) {
      //YUV has two BYTES (4:2:2), UV exchange on x-axis
      //Use luminance only
      //for uneven width the lineincrement is increased by one
      //lineIncr is no multiple of pixelIncr neither -> correct
      if ( width % 2 != 0 && lineIncr % 2 == 1 ) { 
	lineIncr += 1;
      }
      int j ( 0 );
      for (; j<height; j++) {
	int i ( 0 );
	for (; i<width; i++) {
	  roiPtr++;
	  (*srcPtr++) = *(roiPtr++);  //Y
	}
	roiPtr += lineIncr;
      }
 
    } else {
      //grey mode has only 1 BYTE
      int j ( 0 );
      for (; j<height; j++) {
	int i ( 0 );
	for (; i<width; i++) {
	  (*srcPtr++) = *(roiPtr++); 
	}
	roiPtr += lineIncr;
      }
    }

    startAquisition();
#if defined _LTI_DEBUG && _LTI_DEBUG > 1
    chron.stop();
    cout << "...apply " << float(chron.getTime())/1000000 << endl;
#endif

    return true;
  }

  bool leutronFrameGrabber
  ::apply(channel8& band1, channel8& band2, channel8& band3) { 

#if defined _LTI_DEBUG && _LTI_DEBUG > 0
    lti::timer chron;
    chron.start();
#endif
    stopAquisition();
    LvROI roi;
    initRoi(roi);
    
    //roi dimensions might not fit parameter/image settings
    const int width  ( roi.GetWidth() );
    const int height ( roi.GetHeight() );
    band1.resize(height, width, 0, false, false);
    band2.resize(height, width, 0, false, false);
    band3.resize(height, width, 0, false, false);

    int lineIncr ( roi.GetLineIncrement() / roi.GetPixelIncrement() - width );
    U8BIT *roiPtr = (U8BIT *)(roi.MemoryInfo.BaseAddress+roi.StartAddress);

    ubyte* band1Ptr = &band1.at(0);
    ubyte* band2Ptr = &band2.at(0);
    ubyte* band3Ptr = &band3.at(0);

    const parameters& par = getParameters();
    if ( par.color == parameters::RGBa ) {
      //RGB or RGBalpha
      const int pi (1);
      int j ( 0 );
      for (; j<height; j++) {
	int i ( 0 );
	for (; i<width; i++) {
	  *(band3Ptr++) = *(roiPtr++); //R and B are exchanged
	  *(band2Ptr++) = *(roiPtr++);
	  *(band1Ptr++) = *(roiPtr++);
	  roiPtr += pi;
	}
	roiPtr += lineIncr;
      }

    } else if ( par.color == parameters::YUV ) {
      //YUV has two BYTES (4:2:2), UV exchange on x-axis (currently not)
      //for uneven width the lineincrement is increased by one
      //lineIncr is no multiple of pixelIncr neither -> correct
      if ( width % 2 != 0 && lineIncr % 2 == 1 ) { 
	lineIncr += 1;
      }
      const int width_1 ( width-1 );
      int j ( 0 );
      for (; j<height; j++) {
	int i ( 0 );
	ubyte  u ( 0 );
	for (; i<width_1; i+=2) {
	  const ubyte v ( *(roiPtr++) );
	  *(band2Ptr++) = v;
	  *(band2Ptr++) = v;
	  *(band1Ptr++) = *(roiPtr++);    //Y  1st
	  u = *(roiPtr++);
	  *(band3Ptr++) = u;
	  *(band3Ptr++) = u;
	  *(band1Ptr++) = *(roiPtr++);    //Y  1st
	}
	//uneven width -> discard U or V
	if ( i < width )  {
	  *(band2Ptr++) = *(roiPtr++);
	  *(band1Ptr++) = *(roiPtr++);   //Y is second band - luminance
	  *(band3Ptr++) = u;
	}
	roiPtr += lineIncr;
      }

    } else {
      //grey mode has only 1 BYTE
      int j ( 0 );
      for (; j<height; j++) {
	int i ( 0 );
	for (; i<width; i++) {
	  *(band3Ptr++) = *(roiPtr);
	  *(band2Ptr++) = *(roiPtr);
	  *(band1Ptr++) = *(roiPtr++);
	}
	roiPtr += lineIncr;
      }
    }
 
    startAquisition();
#if defined _LTI_DEBUG && _LTI_DEBUG > 1
    chron.stop();
    cout << "...apply " << float(chron.getTime())/1000000 << endl;
#endif

    return true;
  }

#include "ltiUndebug.h"
}//namespace

#endif //_USE_LEUTRON_GRABBER
