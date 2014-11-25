/*
 * Copyright (C) 2004, 2005, 2006
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
 * file .......: ltiFireWireDCAM.cpp
 * authors ....: Arnd Hannemann
 * organization: LTI, RWTH Aachen
 * creation ...: 21.10.2004
 * revisions ..: $Id: ltiFireWireDCAM.cpp,v 1.8 2008/08/17 22:20:12 alvarado Exp $
 */

#include "ltiHardwareConfig.h"

#ifdef _USE_FIRE_WIRE_DCAM

#include "ltiFireWireDCAM.h"
#include "ltiMutex.h"
#include "ltiLispStreamHandler.h"
#include "ltiTimer.h"

#include <cstring>
#include <fstream>

#undef _LTI_DEBUG
// #define _LTI_DEBUG 1
#include "ltiDebug.h"


namespace lti {
  

  // --------------------------------------------------
  // fireWireDCAM::parameters
  // --------------------------------------------------

  std::string fireWireDCAM::parameters::cameraNamesFile = "cameras.txt";

  // default constructor
  fireWireDCAM::parameters::parameters()
    : frameGrabber::parameters() {
    
    fixMode = Upload;
    resolution.set(640,480);
    encoding = YUV422;
    framerate = 15.0f;
    
    brightnessState = FeatureAuto;
    brightness = 0;

    exposureState = FeatureAuto;
    exposure=0;

    sharpnessState = FeatureAuto;
    sharpness = 0;

    whiteBalanceState = FeatureAuto;
    redGain = 0;
    blueGain = 0;
    
    hueState = FeatureAuto;
    hue = 0;

    saturationState = FeatureAuto;
    saturation = 0;

    gammaState = FeatureAuto;
    gamma = 0;

    shutterState = FeatureAuto;
    shutter = 0;

    gainState = FeatureAuto;
    gain = 0;

    irisState = FeatureAuto;
    iris = 0;

    focusState = FeatureAuto;
    focus = 0;

    zoomState = FeatureAuto;
    zoom = 0;

    panState = FeatureAuto;
    pan = 0;

    tiltState = FeatureAuto;
    tilt = 0;

    waitOnePush = false;
    onePushUpdateRate = 2.0f;
    onePushTimeout = 10.0f;

    oneShot = true;
    dmaBufferSize = 2;
    dropFrames = true;
    deviceFile = "/dev/video1394/0";

    bayerParameters.method = bayerDemosaicing::NearestNeighbor;
    bayerParameters.bayerPattern = bayerDemosaicing::RGGB;

    cameraName = "Default";
  }

  // copy constructor
  fireWireDCAM::parameters::parameters(const parameters& other)
    : frameGrabber::parameters() {
    copy(other);
  }

  // destructor
  fireWireDCAM::parameters::~parameters() {

  }

  // get type name
  const char* fireWireDCAM::parameters::getTypeName() const {
    return "fireWireDCAM::parameters";
  }

  // copy member

  fireWireDCAM::parameters&
    fireWireDCAM::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
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

    fixMode=other.fixMode;
    resolution.copy(other.resolution);
    encoding=other.encoding;
    framerate = other.framerate;
    
    brightnessState = other.brightnessState;
    brightness = other.brightness;

    exposureState = other.exposureState;
    exposure = other.exposure;

    sharpnessState = other.sharpnessState;
    sharpness = other.sharpness;

    whiteBalanceState = other.whiteBalanceState;
    redGain = other.redGain;
    blueGain = other.blueGain;
    
    hueState = other.hueState;
    hue = other.hue;

    saturationState = other.saturationState;
    saturation = other.saturation;

    gammaState = other.gammaState;
    gamma = other.gamma;

    shutterState = other.shutterState;
    shutter = other.shutter;

    gainState = other.gainState;
    gain = other.gain;

    irisState = other.irisState;
    iris = other.iris;

    focusState = other.focusState;
    focus = other.focus;

    zoomState = other.zoomState;
    zoom = other.zoom;

    panState = other.panState;
    pan = other.pan;

    tiltState = other.tiltState;
    tilt = other.tilt;

    waitOnePush = other.waitOnePush;
    onePushUpdateRate = other.onePushUpdateRate;
    onePushTimeout = other.onePushTimeout;

    oneShot = other.oneShot;
    dmaBufferSize = other.dmaBufferSize;
    dropFrames = other.dropFrames;
    deviceFile = other.deviceFile;

    bayerParameters.copy(other.bayerParameters);

    // cameraNamesFile = other.cameraNamesFile; // not necessary (is static)
    cameraName = other.cameraName;

    return *this;
  }
    
  // alias for copy member
    fireWireDCAM::parameters&
    fireWireDCAM::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* fireWireDCAM::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was succeful
   */
# ifndef _LTI_MSC_6
  bool fireWireDCAM::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool fireWireDCAM::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"fixMode",fixModeName(fixMode));
      lti::write(handler,"resolution",resolution);
      lti::write(handler,"encoding",encodingName(encoding));
      lti::write(handler,"framerate",framerate);

      lti::write(handler,"brightnessState",brightnessState);
      lti::write(handler,"brightness",brightness);

      lti::write(handler,"exposureState",exposureState);
      lti::write(handler,"exposure",exposure);

      lti::write(handler,"sharpnessState",sharpnessState);
      lti::write(handler,"sharpness",sharpness);

      lti::write(handler,"whiteBalanceState",whiteBalanceState);
      lti::write(handler,"redGain",redGain);
      lti::write(handler,"blueGain",blueGain);
    
      lti::write(handler,"hueState",hueState);
      lti::write(handler,"hue",hue);

      lti::write(handler,"saturationState",saturationState);
      lti::write(handler,"saturation",saturation);

      lti::write(handler,"gammaState",gammaState);
      lti::write(handler,"gamma",gamma);

      lti::write(handler,"shutterState",shutterState);
      lti::write(handler,"shutter",shutter);

      lti::write(handler,"gainState",gainState);
      lti::write(handler,"gain",gain);

      lti::write(handler,"irisState",irisState);
      lti::write(handler,"iris",iris);

      lti::write(handler,"focusState",focusState);
      lti::write(handler,"focus",focus);

      lti::write(handler,"zoomState",zoomState);
      lti::write(handler,"zoom",zoom);

      lti::write(handler,"panState",panState);
      lti::write(handler,"pan",pan);

      lti::write(handler,"tiltState",tiltState);
      lti::write(handler,"tilt",tilt);

      lti::write(handler,"waitOnePush",waitOnePush);
      lti::write(handler,"onePushUpdateRate",onePushUpdateRate);
      lti::write(handler,"onePushTimeout",onePushTimeout);

      lti::write(handler,"oneShot",oneShot);
      lti::write(handler,"dmaBufferSize",dmaBufferSize);
      lti::write(handler,"dropFrames",dropFrames);
      lti::write(handler,"deviceFile",deviceFile);

      lti::write(handler,"bayerParameters",bayerParameters);

      lti::write(handler,"cameraNamesFile",cameraNamesFile);
      lti::write(handler,"cameraName",cameraName);
      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && frameGrabber::parameters::write(handler,false);
# else
    bool (frameGrabber::parameters::* p_writeMS)(ioHandler&,const bool) const =
      frameGrabber::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

  // convert the enum to a string with its name
  const char* 
  fireWireDCAM::parameters::encodingName(const eColorMode& code) const {
    switch (code) {
    case Mono8:
      return "Mono8";
      break;
    case Mono16:
      return "Mono16";
      break;
    case Mono16s:
      return "Mono16s";
      break;
    case YUV411:
      return "YUV411";
      break;
    case YUV422:
      return "YUV422";
      break;
    case YUV444:
      return "YUV444";
      break;
    case RGB8:
      return "RGB8";
      break;
    case RGB16:
      return "RGB16";
      break;
    case RGB16s:
      return "RGB16s";
      break;
    case Raw8:
      return "Raw8";
      break;
    case Raw16:
      return "Raw16";
      break;
    default:
      return "UnknownEncoding";
      break;
    }
    
    return "Unknown";
  }

  // read format 7 code mode
  const fireWireDCAM::eColorMode 
  fireWireDCAM::parameters::getEncoding(const std::string& str) const {
    
    if (str.find("ono8") != std::string::npos) {
      return Mono8;
    } else if (str.find("ono16s") != std::string::npos) {
      return Mono16s;
    } else if (str.find("ono16") != std::string::npos) {
      return Mono16;
    } else if (str.find("YUV411") != std::string::npos) {
      return YUV411;
    } else if (str.find("YUV422") != std::string::npos) {
      return YUV422;
    } else if (str.find("YUV444") != std::string::npos) {
      return YUV444;
    } else if (str.find("RGB8") != std::string::npos) {
      return RGB8;
    } else if (str.find("RGB16s") != std::string::npos) {
      return RGB16s;
    } else if (str.find("RGB16") != std::string::npos) {
      return RGB16;
    } else if (str.find("aw8") != std::string::npos) {
      return Raw8;
    } else if (str.find("aw16") != std::string::npos) {
      return Raw16;
    } if (str.find("ONO8") != std::string::npos) {
      // Try also the names used by libdc1394 (uppercase)
      return Mono8;
    } else if (str.find("ONO16S") != std::string::npos) {
      return Mono16s;
    } else if (str.find("ONO16") != std::string::npos) {
      return Mono16;
    } else if (str.find("RGB16S") != std::string::npos) {
      return RGB16s;
    } else if (str.find("AW8") != std::string::npos) {
      return Raw8;
    } else if (str.find("AW16") != std::string::npos) {
      return Raw16;
    }
    
    return UnknownEncoding;
  } 

  /*
   * Get a string for the fix mode
   */
  const char* 
  fireWireDCAM::parameters::fixModeName(const eFixMode& mode) const {
    switch(mode) {
      case AutoFix:
        return "AutoFix";
        break;
      case Upload:
        return "Upload";
        break;
      default:
        return "NoFix";
    }
    return 0;
  }

  /*
   * For the given string, return a valid fix mode 
   */
  const fireWireDCAM::eFixMode 
  fireWireDCAM::parameters::getFixMode(const std::string& name) const {
    if (name.find("uto") != std::string::npos) {
      return AutoFix;
    } else if (name.find("oad")) {
      return Upload;
    } 
    return NoFix;
  }


# ifdef _LTI_MSC_6
  bool fireWireDCAM::parameters::write(ioHandler& handler,
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
   * @return true if read was succeful
   */
# ifndef _LTI_MSC_6
  bool fireWireDCAM::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool fireWireDCAM::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      std::string str;
      lti::read(handler,"fixMode",str);
      fixMode = getFixMode(str);
      lti::read(handler,"resolution",resolution);
      lti::read(handler,"encoding",str);
      encoding = getEncoding(str);
      lti::read(handler,"framerate",framerate);

      lti::read(handler,"brightnessState",brightnessState);
      lti::read(handler,"brightness",brightness);

      lti::read(handler,"exposureState",exposureState);
      lti::read(handler,"exposure",exposure);

      lti::read(handler,"sharpnessState",sharpnessState);
      lti::read(handler,"sharpness",sharpness);

      lti::read(handler,"whiteBalanceState",whiteBalanceState);
      lti::read(handler,"redGain",redGain);
      lti::read(handler,"blueGain",blueGain);
    
      lti::read(handler,"hueState",hueState);
      lti::read(handler,"hue",hue);

      lti::read(handler,"saturationState",saturationState);
      lti::read(handler,"saturation",saturation);

      lti::read(handler,"gammaState",gammaState);
      lti::read(handler,"gamma",gamma);

      lti::read(handler,"shutterState",shutterState);
      lti::read(handler,"shutter",shutter);

      lti::read(handler,"gainState",gainState);
      lti::read(handler,"gain",gain);

      lti::read(handler,"irisState",irisState);
      lti::read(handler,"iris",iris);

      lti::read(handler,"focusState",focusState);
      lti::read(handler,"focus",focus);

      lti::read(handler,"zoomState",zoomState);
      lti::read(handler,"zoom",zoom);

      lti::read(handler,"panState",panState);
      lti::read(handler,"pan",pan);

      lti::read(handler,"tiltState",tiltState);
      lti::read(handler,"tilt",tilt);

      lti::read(handler,"waitOnePush",waitOnePush);
      lti::read(handler,"onePushUpdateRate",onePushUpdateRate);
      lti::read(handler,"onePushTimeout",onePushTimeout);

      lti::read(handler,"oneShot",oneShot);
      lti::read(handler,"dmaBufferSize",dmaBufferSize);
      lti::read(handler,"dropFrames",dropFrames);
      lti::read(handler,"deviceFile",deviceFile);

      lti::read(handler,"bayerParameters",bayerParameters);

      lti::read(handler,"cameraNamesFile",cameraNamesFile);
      lti::read(handler,"cameraName",cameraName);
    }

# ifndef _LTI_MSC_6
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

# ifdef _LTI_MSC_6
  bool fireWireDCAM::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // fireWireDCAM::busInfo
  // --------------------------------------------------
  fireWireDCAM::busInfo::busInfo() :
    handles(0),cameraNodes(0),cameraNum(-1),portCameraNum(0),portNum(0),
    cardFound(false) {
  }

  fireWireDCAM::busInfo::~busInfo() {
    deleteCameraNodes();
  }

  int fireWireDCAM::busInfo::busResetHandler(raw1394handle_t,
                                             unsigned int) {
    _lti_debug("Bus reset handler was called!\n");

    // TODO:  we need to decide what is to be done if we get here
    
    return 1;
  }
  
  bool fireWireDCAM::busInfo::get() {
    return getNodes();
  }

  bool fireWireDCAM::busInfo::deleteCameraNodes() {
    if (notNull(cameraNodes)   &&
        notNull(portCameraNum) &&
        notNull(handles)) {
        
      for (int port=0;port<portNum;port++) {
        dc1394_free_camera_nodes(cameraNodes[port]);
        cameraNodes[port]=0;
        dc1394_destroy_handle(handles[port]);
        handles[port]=0;
      }
      delete[] cameraNodes;
      cameraNodes=0;

      delete[] portCameraNum;
      portCameraNum=0;

      delete[] handles;
      handles=0;

      return true;
    } else {
      cameraNodes=0;
      portCameraNum=0;
      handles=0;
    }

    return false;
  }

  bool fireWireDCAM::busInfo::getNodes() {
    // The following code has been adapted from the method GetCameraNodes in
    // the file camera.c of coriander (by Damien Douxchamps)
    
    raw1394handle_t tmpHandle;
    int port;

    tmpHandle=raw1394_new_handle(); // calling the library handler method
    cardFound=false;
    cameraNum=0;

    if (notNull(tmpHandle)) {
      // Just get how many ports are required
      portNum=raw1394_get_port_info(tmpHandle, NULL, 0);      
      // and destroy the handler
      raw1394_destroy_handle(tmpHandle);

      // ensure that no nodes are being used
      deleteCameraNodes();

      typedef nodeid_t* nodeIdPtr;

      // allocate the required memory
      cameraNodes=new nodeIdPtr[portNum];
      portCameraNum=new int[portNum];
      handles=new raw1394handle_t[portNum];

      // initialize the memory
      for (port=0;port<portNum;port++) {
        // get a handle to the current interface card
        handles[port]=dc1394_create_handle(port);
        if (notNull(handles[port])) { // if the card is present
          cardFound=true;

          // set bus reset handler
          raw1394_set_bus_reset_handler(handles[port], busResetHandler);

          // probe the IEEE1394 bus for DC camera:
          // this method write the number of cameras in the portCameraNum[.]
          cameraNodes[port]=dc1394_get_camera_nodes(handles[port],
                                                    &portCameraNum[port],
                                                    0); // 0 don't show cams.
          // accumulate the total number of cameras
          cameraNum+=portCameraNum[port];
        } else {
          cameraNodes[port]=0;
          portCameraNum[port]=0;
        }
      }
    }
    
    return cardFound;
  }

  // --------------------------------------------------
  // fireWireDCAM::cameraInfo
  // --------------------------------------------------
  fireWireDCAM::cameraInfo::cameraInfo() {
    // ensure that all data is initialized to some value
    memset(&info,0,sizeof(dc1394_camerainfo));
    memset(&featureSet,0,sizeof(dc1394_feature_set));
    memset(&miscInfo,0,sizeof(dc1394_miscinfo));
    info.handle=0;
    inUse=false;
  }

  fireWireDCAM::cameraInfo::~cameraInfo() {
    inUse=true;
    if (notNull(info.handle)) {
      dc1394_destroy_handle(info.handle);
    }
  }

  bool fireWireDCAM::cameraInfo::getFormat7Capabilities() {
     int i, f;
     quadlet_t value;
     bool error=false;
  
     if (dc1394_query_supported_formats(info.handle, info.id, &value) !=
         DC1394_SUCCESS) {
       format7.available = false;
       errorStr+="Could not query supported formats. ";
       error=true;
     } else {
       if (value & (0x1 << 24)) { // is format7 supported?
         if (dc1394_query_supported_modes(info.handle,
                                          info.id,
                                          FORMAT_SCALABLE_IMAGE_SIZE,
                                          &value)!=DC1394_SUCCESS) {
           format7.available = false;
           errorStr+="Could not query Format7 supported modes. ";
           error=true;
         } else {
           format7.available = true;
           for (i=0,f=MODE_FORMAT7_MIN; f<=MODE_FORMAT7_MAX; f++,i++) {
             format7.mode[i].present = ( (value & (0x1 << (31-i))) != 0);
             getFormat7ModeInfo(f);
           }
         }
       }
       else { // format7 is not supported!!
         format7.available = false;
         for (i=0,f=MODE_FORMAT7_MIN;f<=MODE_FORMAT7_MAX;f++,i++) {
           format7.mode[i].present=false;
         }
       }
     }
     return !error;
  }

  bool 
  fireWireDCAM::cameraInfo::getFormat7ModeInfo(const unsigned int modeId) {

    // modeId : 

    format7ModeInfo& mode = format7.mode[modeId-MODE_FORMAT7_MIN];
    bool error=false;

    if (mode.present) { // check for mode presence before query
      if (dc1394_query_format7_max_image_size(info.handle,
                                              info.id,
                                              modeId,
                                              &mode.maxSize.x,
                                              &mode.maxSize.y) != 
          DC1394_SUCCESS) {
        errorStr += "Error querying format7 max image size. ";
        error=true;
      }
      
      if (dc1394_query_format7_unit_size(info.handle,
                                         info.id,
                                         modeId,
                                         &mode.unitSize.x,
                                         &mode.unitSize.y) !=
          DC1394_SUCCESS) {
        errorStr += "Error querying format7 unit size. ";
        error=true;
      }

      // quick hack to keep size/position even. If pos/size is ODD,
      // strange color/distorsions occur on some cams (e.g. Basler
      // cams). This will have to be really fixed later. 
      // REM: this is fixed by using the unit_position: 

      // fprintf(stderr,"Using pos units = %d %d\n",info->mode[i].step_pos_x,
      // info->mode[i].step_pos_y);

      if (dc1394_query_format7_unit_position(info.handle,
                                             info.id,
                                             modeId,
                                             &mode.unitPos.x,
                                             &mode.unitPos.y) !=
          DC1394_SUCCESS) {
        errorStr += "Error querying format7 unit position. ";
        error=true;
      }

      mode.unitPos.set(0,0);
    }

    if (dc1394_query_format7_image_position(info.handle,
                                            info.id,
                                            modeId,
                                            &mode.pos.x,
                                            &mode.pos.y)!=DC1394_SUCCESS) {
      errorStr += "Error querying format7 image position. ";
      error=true;
    }

    if (dc1394_query_format7_image_size(info.handle,
                                        info.id,
                                        modeId,
                                        &mode.size.x,
                                        &mode.size.y) != DC1394_SUCCESS) {
      errorStr += "Error querying format7 image size. ";
      error=true;
    }
    
    if (dc1394_query_format7_byte_per_packet(info.handle,
                                             info.id,
                                             modeId,
                                             &mode.bpp) != DC1394_SUCCESS) {
      errorStr += "Error querying format7 bytes per packet. ";
      error=true;
    }

    if (mode.bpp==0) {
      // sometimes a camera will not set the bpp register until a
      // valid image size has been set after boot. If BPP is zero, we
      // therefore try again after setting the image size to the
      // maximum size.
      errorStr += "Camera reported a BPP of ZERO.";
      errorStr += "Trying to set maximum size to correct this. ";

      if (dc1394_set_format7_image_position(info.handle,
                                            info.id,
                                            modeId,0,0) != DC1394_SUCCESS) {
        errorStr += "Error setting format7 image position. ";
        error=true;
      }

      if (dc1394_set_format7_image_size(info.handle,
                                        info.id,
                                        modeId,
                                        mode.maxSize.x,
                                        mode.maxSize.y) != DC1394_SUCCESS) {
	errorStr += "Error setting format7 image size. ";
        error=true;
      }
       
      // maybe we should also force a color coding here.
      if (dc1394_query_format7_byte_per_packet(info.handle,
                                               info.id,
                                               modeId,
                                               &mode.bpp) != DC1394_SUCCESS) {
	errorStr += "Error querying format7 bytes per packet. ";
        error=true;
      }

      if (mode.bpp==0) {
	errorStr += "    BPP still zero. Giving up. ";
        error=true;
      }
    }

    if (dc1394_query_format7_packet_para(info.handle,
                                         info.id,
                                         modeId,
                                         &mode.minBpp,
                                         &mode.maxBpp) !=DC1394_SUCCESS) {
      errorStr += "Error querying format7 packet parameters. ";
      error=true;
    }

    if (dc1394_query_format7_pixel_number(info.handle,
                                          info.id,
                                          modeId,
                                          &mode.pixnum) !=DC1394_SUCCESS) {
      errorStr += "Error querying format7 pixel number. ";
      error=true;
    }

    if (dc1394_query_format7_total_bytes(info.handle,
                                         info.id,
                                         modeId,
                                         &mode.totalBytes) !=DC1394_SUCCESS) {
      errorStr += "Error querying format7 total bytes per frame. ";
      error=true;
    }

    if (dc1394_query_format7_color_coding_id(info.handle,
                                             info.id,
                                             modeId,
                                             &mode.colorCodingId) !=
        DC1394_SUCCESS) {
      errorStr += "Error querying format7 color coding ID. ";
      error=true;
    }

    if (dc1394_query_format7_color_coding(info.handle,
                                          info.id,
                                          modeId,
                                          &mode.colorCoding) != 
        DC1394_SUCCESS) {
      errorStr += "Error querying format7 color coding. ";
      error=true;
    }

    return (!error);
  }

  bool fireWireDCAM::cameraInfo::
  getSupportedModesAndFramerates(const nodeid_t& node) {

    static const uint32 fbits[] = { 0x1<<31, // Format 0
                                    0x1<<30, // Format 1
                                    0x1<<29, // Format 2
                                    0x1<<28, // Format 3
                                    0x1<<27, // Format 4
                                    0x1<<26, // Format 5
                                    0x1<<25, // Format 6
                                    0x1<<24  // Format 7
    };
    
    static const uint32 fmin[] = { MODE_FORMAT0_MIN,
                                   MODE_FORMAT1_MIN,
                                   MODE_FORMAT2_MIN,
                                   1,
                                   1,
                                   1,
                                   MODE_FORMAT6_MIN,
                                   MODE_FORMAT7_MIN };

    static const uint32 fmax[] = { MODE_FORMAT0_MAX,
                                   MODE_FORMAT1_MAX,
                                   MODE_FORMAT2_MAX,
                                   0,
                                   0,
                                   0,
                                   MODE_FORMAT6_MAX,
                                   MODE_FORMAT7_MAX };

    static const float framerates[] = {1.875f,
                                       3.75f,
                                       7.5f,
                                       15.f,
                                       30.f,
                                       60.f,
                                       120.f,
                                       240.f};

    // clear all available information
    int i,total(0);
    uint32 j,f;
    supportedModes.clear();
    supportedFramerates.clear();
    quadlet_t formats=0,modes=0,fr=0;

    if (dc1394_query_supported_formats(info.handle,
                                       node,
                                       &formats) != DC1394_SUCCESS) {
      errorStr += "Could not get supported formats";
      return false;
    }

    // for each format (0 to 7)
    for (i=0;i<8;++i) {
      // if the format is supported
      if ((formats & fbits[i]) != 0) {
        // format i is supported! -> get modes for such format
        if ((fmin[i]<=fmax[i]) &&
            (dc1394_query_supported_modes(info.handle,
                                          node,
                                          i+FORMAT_MIN,
                                          &modes) == DC1394_SUCCESS)) {
          for (j=fmin[i];j<=fmax[i];++j) {
            if (modes & (0x1<<(31-(j-fmin[i])))) {
              supportedModes.insert(j);
              total++;

              // now that we have a supported mode, we can ask for
              // supported framerates
              if (dc1394_query_supported_framerates(info.handle, node,
                                                    i+FORMAT_MIN,j,
                                                    &fr)==DC1394_SUCCESS) {
                for (f=0;f<NUM_FRAMERATES;++f) {
                  if (fr & (0x1 << (31-f))) {
                    supportedFramerates[j].insert(framerates[f]);
                  }
                }
              }
            }
          }
        }
      }
    }
    
    return true;
  }

  bool fireWireDCAM::cameraInfo::get(const int port,const nodeid_t& node) {
    
    info.handle=dc1394_create_handle(port);
    bool error=false;
    errorStr.clear();

    if (dc1394_get_camera_info(info.handle, node, &info) !=
        DC1394_SUCCESS) {
      errorStr="Could not get camera basic information! ";
      error=true;
    }

    // after the previous call the info struct contains now the node also:
    if (dc1394_get_camera_misc_info(info.handle, info.id, &miscInfo) !=
        DC1394_SUCCESS) {
      errorStr+= "Could not get camera misc information! ";
      error=true;
    }

    if (dc1394_get_camera_feature_set(info.handle,info.id, &featureSet) !=
        DC1394_SUCCESS) {
      errorStr+= "Could not get camera feature information! ";
      error=true;
    }

    if (dc1394_query_basic_functionality(info.handle,info.id, &basics) !=
        DC1394_SUCCESS) {
      errorStr+= "Could not get camera basics information! ";
      error=true;
    }
    

    error = error || !getFormat7Capabilities();
    error = error || !getSupportedModesAndFramerates(node);
    
    return !error;
  }

  // --------------------------------------------------
  // fireWireDCAM::cameraSet
  // --------------------------------------------------
  fireWireDCAM::cameraSet::cameraSet() {
  }

  fireWireDCAM::cameraSet::~cameraSet() {
  }
  
  const fireWireDCAM::cameraInfo& 
  fireWireDCAM::cameraSet::operator[](const int idx) const {
    assert(static_cast<unsigned int>(idx)<cams_.size());
    return cams_[idx];
  }

  fireWireDCAM::cameraInfo& 
  fireWireDCAM::cameraSet::operator[](const int idx) {
    assert(static_cast<unsigned int>(idx)<cams_.size());
    return cams_[idx];
  }
  
  unsigned int fireWireDCAM::cameraSet::size() const {
    return cams_.size();
  }

  bool fireWireDCAM::cameraSet::get(busInfo& bus) {
    lock_.lock();
    cams_.clear(); // begin with an empty camera set
    int port,i,end;
    cameraInfo dummyCam;
    bool error = false;

    // for each available port
    for (port=0;port<bus.portNum;port++) {
      // if this port has some handles
      if (notNull(bus.handles[port])) {
        // for each camera in the port
        for (i=0;i<bus.portCameraNum[port];i++) {
          // get camera data
          cams_.push_back(dummyCam);
          if (!cams_.back().get(port,*(bus.cameraNodes[i]))) {
            error=true;
            errorStr+=cams_.back().errorStr;
            cams_.pop_back();
          } else {
            // check that the camera is not yet found through another
            // interface card (for strange bus topologies):
            end=cams_.size()-1;  // check all but the last one
            for (i=0;i<end;++i) {
              if (cams_[i].info.euid_64 == cams_.back().info.euid_64) {
                // camera already there!  remove and continue
                cams_.pop_back();
              }
            }
          }
        }
      }
    }
    lock_.unlock();
    return !error;
  }

  bool fireWireDCAM::cameraSet::findCamera(const u_int64_t& euID64,
                                           int& camIdx) const {
    unsigned int i;
    camIdx=-1;
    lock_.lock();
    for (i=0;i<cams_.size();++i) {
      if (cams_[i].info.euid_64 == euID64) {
        camIdx=i;
        i=cams_.size(); // break;
      }
    }
    lock_.unlock();
    return (camIdx>=0);
  }

  bool fireWireDCAM::cameraSet::findFreeCamera(int& camIdx) const {
    unsigned int i;
    camIdx=-1;
    lock_.lock();
    for (i=0;i<cams_.size();++i) {
      if (!cams_[i].inUse) {
        camIdx=i;
        i=cams_.size(); // break;
      }
    }
    lock_.unlock();
    return (camIdx>=0);
  }

  bool fireWireDCAM::cameraSet::reserveIndex(const int idx) {
    lock_.lock();
    bool inUse = cams_[idx].inUse;  // if this is true, cam is already in use
    if (!inUse) {
      cams_[idx].inUse = true;
    }
    lock_.unlock();
    return (!inUse);
  }

  bool fireWireDCAM::cameraSet::releaseIndex(const int idx) {
    if (!cams_[idx].inUse) {
      return false; // cannot be release, because is not in use!
    }
    lock_.lock();
    cams_[idx].inUse = false;
    lock_.unlock();
    return true;
  }

  // --------------------------------------------------
  // fireWireDCAM::nameDB
  // --------------------------------------------------
  fireWireDCAM::nameDB::nameDB() 
  : init_(false),modified_(false) {
  }

  fireWireDCAM::nameDB::~nameDB() {
    lock_.lock();
    dump();
    lock_.unlock();
  }
  
  
  bool fireWireDCAM::nameDB::use(const std::string& filename) {
    if ((filename != filename_) || !init_) {
      lock_.lock();
      if ((filename != filename_) && init_) {
        dump();
      }
      if ( (init_ = read(filename)) ) {
        filename_=filename;      
      } else {
        filename_.clear(); // to avoid future problems... use it as a flag
      }
      lock_.unlock();
    }
    return init_;
  }

  bool fireWireDCAM::nameDB::query(const std::string& name,
                                   u_int64_t& id) const {    
    bool error = false;
    if (!init_) {
      return false;
    }
    lock_.lock();
    std::map<std::string,u_int64_t>::const_iterator it;
    it = data_.find(name);
    if (it != data_.end()) {
      id=(*it).second;
    } else {
      id = 0; // invalid value
      error = true;
    }
    lock_.unlock();
    return !error;
  }

  bool fireWireDCAM::nameDB::add(const std::string& name,u_int64_t& id) {
    bool error = false;
    if (!init_) {
      return false;
    }

    lock_.lock();
    // try to find if the data already exists
    std::map<std::string,u_int64_t>::const_iterator it;
    it = data_.find(name);
    if (it != data_.end()) {
      error = true; // cannot add it!  It already exists!
    } else {
      data_[name]=id; // add the data
      modified_=true;
    }
    lock_.unlock();
    return !error;
  }
  
  bool fireWireDCAM::nameDB::del(const std::string& name) {
    bool error = false;
    if (!init_) {
      return false;
    }
    lock_.lock();
    std::map<std::string,u_int64_t>::iterator it;
    it = data_.find(name);
    if (it != data_.end()) {
      data_.erase(it);
      modified_=true;
    } else {
      error = true;
    }
    lock_.unlock();
    return !error;
  }
  
  bool fireWireDCAM::nameDB::dump() {
    bool error = true;
    if (!init_) {
      return false;
    }
    if (!modified_) {
      return true;
    }

    std::ofstream out(filename_.c_str());
    if (out) {
      error=false;
      lispStreamHandler lsh(out);
      lsh.writeComment("Camera name and corresponding hex ID");
      lsh.writeComment("This file will be automatically modified");
      out << std::endl;

      std::string str;
      std::map<std::string,u_int64_t>::const_iterator it;
      for (it=data_.begin();it!=data_.end();++it) {
        hexToString((*it).second,str);
        lsh.writeBegin();
        lsh.write((*it).first);
        lsh.writeKeyValueSeparator();
        lsh.write(str);
        lsh.writeEnd();
      }
      out << std::endl;
      out.close();
    }
    return !error;
  }

  bool fireWireDCAM::nameDB::read(const std::string& filename) {
    bool error = false;
    modified_=false;

    std::ifstream in(filename.c_str());
    if (in) {
      lispStreamHandler lsh(in);
      data_.clear(); // erase all data known before, since we will get it all
                     // from a file
      // we read lines until we get an error
      bool cont=true;
      std::string name,id;
      u_int64_t d;
      do {
        bool b = true;
        b = b && lsh.readBegin();
        b = b && lsh.read(name);
        b = b && lsh.readKeyValueSeparator();
        b = b && lsh.read(id);
        b = b && lsh.readEnd();
        if (b) {
          if (!stringToHex(id,d)) {
            error=true;
            break;
          }
          data_[name]=d;
        } else {
          cont = false;
        }
      } while (cont);
      in.close();
    } else {
      // well, maybe the file does not exist.  Create it empty
      std::ofstream out(filename.c_str());
      if (out) {
        out << std::endl;
        out.close();
      } else {
        error = true;
      }
    }
    return !error;  
  }

  void fireWireDCAM::nameDB::hexToString(const u_int64_t& data,
                                         std::string& str) const {

    static const char d2a[] = {'0','1','2','3','4','5','6','7','8','9',
                               'A','B','C','D','E','F'};
    str.clear();
    u_int64_t d = data;
    uint n;

    do {
      n= static_cast<uint>(d & 0x0F);
      str = d2a[n]+str;// append to the left the new digit
      d=d>>4;// take the lowest 4 bits away shifting at the same time the rest.
    } while(d!=0);
    str="0x"+str;
  }
  
  bool fireWireDCAM::nameDB::stringToHex(const std::string& str,
                                         u_int64_t& data) const {

    // to avoid segfaults, just fill 128 chars of ascii to nibble LUT.
    static const char a2d[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
                               0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    data = 0;
    if (str.find("0x") == 0) {
      unsigned int i = 2;
      while (i<str.length()) {
        data = data<<4;// shift 4 bits to the left
        data+=a2d[static_cast<int>(str[i] & 0x7F)];
        ++i;
      }
      return true;
    } 
    return false;
  }

  // --------------------------------------------------
  // fireWireDCAM::static attributes
  // --------------------------------------------------
  bool fireWireDCAM::hwCreated_ = false;
  fireWireDCAM::busInfo fireWireDCAM::bus_;
  fireWireDCAM::cameraSet fireWireDCAM::cams_;
  fireWireDCAM::nameDB fireWireDCAM::cameraNames_;

  // --------------------------------------------------
  // fireWireDCAM
  // --------------------------------------------------

  // default constructor
  fireWireDCAM::fireWireDCAM()
    : frameGrabber() {

    activeCamera_=-1; // set an invalid value

    oneShotCapable_ = false;

    handle_=0;
    features_=0;
    node_=0;
    
    busChannel_ = 0;
    busSpeed_ = 0;
    isoTransStarted_ = false;

    colorMode_ = YUV422;
    
    memset(&camera_,0,sizeof(dc1394_cameracapture));
    setupCapture_ = false;
   
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // this constructor must not be implemented!!!
  fireWireDCAM::fireWireDCAM(const parameters& par)
    : frameGrabber() {
    
    activeCamera_=-1; // set an invalid value

    oneShotCapable_ = false;

    handle_=0;
    features_=0;
    node_=0;

    busChannel_ = 0;
    busSpeed_ = 0;
    isoTransStarted_ = false;

    colorMode_ = YUV422;
    
    memset(&camera_,0,sizeof(dc1394_cameracapture));
    setupCapture_ = false;
   
    // set the default parameters
    setParameters(par);
  }


  // copy constructor
  fireWireDCAM::fireWireDCAM(const fireWireDCAM& other)
    : frameGrabber() {

    activeCamera_=-1; // set an invalid value

    oneShotCapable_ = false;

    handle_=0;
    features_=0;
    node_=0;

    busChannel_ = 0;
    busSpeed_ = 0;
    isoTransStarted_ = false;

    colorMode_ = YUV422;
    
    memset(&camera_,0,sizeof(dc1394_cameracapture));
    setupCapture_ = false;

    copy(other);
  }

  // destructor
  fireWireDCAM::~fireWireDCAM() {
    releaseCamera();
  }

  // returns the name of this type
  const char* fireWireDCAM::getTypeName() const {
    return "fireWireDCAM";
  }

  // copy member
  fireWireDCAM&
  fireWireDCAM::copy(const fireWireDCAM& other) {
    frameGrabber::copy(other);
    
    // It makes no sense to "copy" a grabbing functor here, since
    // only one instance can talk to one camera at the same time.
    // The only possible meaning is to copy the parameters, an let
    // the updateParameters do its job.  It may fail

    // Free all camera stuff here! 
    releaseCamera();     
    
    return (*this);
  }

  // alias for copy member
  fireWireDCAM&
  fireWireDCAM::operator=(const fireWireDCAM& other) {
    return (copy(other));
  }


  // clone member
  functor* fireWireDCAM::clone() const {
    return new fireWireDCAM(*this);
  }

  // return parameters
  const fireWireDCAM::parameters&
  fireWireDCAM::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if (isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // return parameters
  fireWireDCAM::parameters& fireWireDCAM::getRWParameters() {
    parameters* par = dynamic_cast<parameters*>(&functor::getParameters());
    if (isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }


  bool fireWireDCAM::isCameraActive() const {
    return (activeCamera_>=0);
  }

  bool fireWireDCAM::hasCameraFormat7() const {
    if (activeCamera_>=0) { // only if the camera is active, the array
                            // contains meaningful data
      return cams_[activeCamera_].format7.available;
    }
    return false;
  }

  bool fireWireDCAM::releaseCamera() {
    if (!isCameraActive()) {
      // nothing to release
      return true;
    }

    // stop transmission
    if (isoTransStarted_) {
      dc1394_stop_iso_transmission(*handle_,*node_);
      isoTransStarted_=false;
    }
    
    if (setupCapture_) {
      _lti_debug("release old camera structure...\n");

      // remember: even if this is called release "camera", it just
      // releases the "capture" relevant stuff
      dc1394_dma_release_camera(*handle_,&camera_);
      setupCapture_ = false;
    }

    if ((cams_[activeCamera_].basics & (0x1<<16)) !=0) {
      dc1394_camera_off(*handle_,*node_);
    }

    cams_.releaseIndex(activeCamera_);
    activeCamera_=-1;
    activeCameraName_.clear();
    handle_=0;
    features_=0;
    node_=0;
    
    return true;
  }

  fireWireDCAM::eFeatureState 
  fireWireDCAM::fixState(const unsigned int states,
                         const eFeatureState desired) const {

    if ((states & desired)==0) {
      // oh oh, desired state not supported, so let us just priorize a bit
      if ((states & FeatureAuto) != 0) {
        return FeatureAuto;          
      } else if ((states & FeatureManual) != 0) {
        return FeatureManual;
      } else if ((states & FeatureAbsolute) != 0) {
        return FeatureAbsolute;
      } else if ((states & FeatureOnePush) != 0) {
        return FeatureOnePush;
      } else if ((states & FeatureOff) != 0) {
        return FeatureOff;
      }
      return FeatureNA; // invalid value
    }
    return desired; // feature supported
  }

  void fireWireDCAM::fixFeature(const unsigned int featureId,
                                float& value,
                                eFeatureState& state) const {

    if (isFeatureAvailable(featureId)) {
      const dc1394_feature_info& fi = 
        cams_[activeCamera_].featureSet.feature[featureId-FEATURE_MIN];
      if (value < fi.min) {
        value = fi.min;
      } else if (value > fi.max) {
        value = fi.max;
      }

      // fix mode
      unsigned int states = checkFeatureStates(featureId);
      state = fixState(states,state);
    } else {
      value=0;
      state=FeatureNA;
    }

  }


  bool fireWireDCAM::fixParameters(parameters& par) const {
    if (!isCameraActive()) {
      setStatusString("Cannot fix parameters of a non-active camera");
      return false;
    }

    const cameraInfo& cam = cams_[activeCamera_];

    // we have to keep most attributes of par untouched, but the following
    // have to be copied from the current set:
    par.cameraName = activeCameraName_;
    par.deviceFile = getParameters().deviceFile;

    // first, get the resolutions and encodings available in the IIDC standard
    unsigned int mode;
    mode = convertMode(par.resolution,par.encoding);

    // now, we have to check if the mode is supported.
    // If it is a mode of the format7, then due to the implementation of 
    // the previous method, it will be supported.  If the mode is of formats
    // 0, 1 or 2, then we may require many adaptions
    if (mode <= MODE_FORMAT2_MAX) {
      mode = computeClosestMode(mode);      
    } 

    convertMode(mode,par.resolution,par.encoding); // get it back to LTI-Lib
    par.framerate = computeClosestFrameRate(mode,par.framerate);

    // now use the camera information to adjust some values if necessary
    fixFeature(FEATURE_BRIGHTNESS,par.brightness,par.brightnessState);
    fixFeature(FEATURE_EXPOSURE,par.exposure,par.exposureState);
    fixFeature(FEATURE_SHARPNESS,par.sharpness,par.sharpnessState);

    if (isFeatureAvailable(FEATURE_WHITE_BALANCE)) {
      const dc1394_feature_info& fi = 
        cam.featureSet.feature[FEATURE_WHITE_BALANCE-FEATURE_MIN];
      if (par.redGain < fi.min) {
        par.redGain = fi.min;
      } else if (par.redGain > fi.max) {
        par.redGain = fi.max;
      }
      if (par.blueGain < fi.min) {
        par.blueGain = fi.min;
      } else if (par.blueGain > fi.max) {
        par.blueGain = fi.max;
      }
 
      // fix mode
      unsigned int states = checkFeatureStates(FEATURE_WHITE_BALANCE);
      par.whiteBalanceState = fixState(states,par.whiteBalanceState);
    } else {
      par.blueGain = par.redGain = 0;
      par.whiteBalanceState = FeatureNA;
    }

    fixFeature(FEATURE_HUE,par.hue,par.hueState);
    fixFeature(FEATURE_SATURATION,par.saturation,par.saturationState);
    fixFeature(FEATURE_GAMMA,par.gamma,par.gammaState);
    fixFeature(FEATURE_SHUTTER,par.shutter,par.shutterState);
    fixFeature(FEATURE_GAIN,par.gain,par.gainState);
    fixFeature(FEATURE_IRIS,par.iris,par.irisState);
    fixFeature(FEATURE_FOCUS,par.focus,par.focusState);
    fixFeature(FEATURE_ZOOM,par.zoom,par.zoomState);
    fixFeature(FEATURE_PAN,par.pan,par.panState);
    fixFeature(FEATURE_TILT,par.tilt,par.tiltState);

    return true;
  }
  
  bool fireWireDCAM::isModeColor(const unsigned int mode) const {
    // to make this method efficient, we use some low-level information about
    // the bit settings for the formats.
    return !(((mode & 0x07) >= 5) ||
             (((mode>=MODE_FORMAT1_MIN) && (mode & 0x07)==2)));
  }

  float fireWireDCAM::computeClosestFrameRate(const unsigned int mode,
                                              const float fr) const {

    const cameraInfo& cam = cams_[activeCamera_];

    // get closest supported framerate for the selected mode
    std::map<uint32,std::set<float> >::const_iterator frsi;
    frsi = cam.supportedFramerates.find(mode);
    if (frsi == cam.supportedFramerates.end()) {
      // this should never happen because:
      // - mode is always supported (we have just ensure that)
      // - the camera method should fill the valid FR for all supported modes
      return 0.0f;
    }

    const std::set<float>& frates = (*frsi).second;
    std::set<float>::const_iterator fri = frates.begin();
    float bestFr = *fri;
    float dfr = abs(bestFr-fr);
    float tmpdfr;

    ++fri;
    while(fri!=frates.end()) {
      tmpdfr = abs((*fri)-fr);
      if (tmpdfr<dfr) {
        dfr=tmpdfr;
        bestFr = (*fri);
      }
      ++fri;
    }

    return bestFr;
  }

  unsigned int 
  fireWireDCAM::computeClosestMode(const unsigned int mode) const {
  
    const std::set<uint32>& sm = cams_[activeCamera_].supportedModes;
    if (sm.empty()) {
      return 0; // as error signal!
    }

    if (mode>MODE_FORMAT2_MAX) {
      return 0;  // not one of the 
    }

    // the approximation is done with the following priorities:
    // 1. keep the mono or color attribute (unless your camera is
    //    monochrome, of course)
    // 2. select the closest resolution.
    
    // separate sets in color and monochrome modes
    std::set<uint32> inColor,monochrome;
    std::set<uint32>::const_iterator it,best;

    it = sm.find(mode);
    if (it != sm.end()) {
      return mode; // it is supported
    }
    

    for (it = sm.begin(); it != sm.end(); ++it) {
      if (isModeColor(*it)) {
        inColor.insert(*it);
      } else {
        monochrome.insert(*it);
      }
    }

    eColorMode encoding;
    ipoint resolution;
    convertMode(mode,resolution,encoding);
    const int numPxl = resolution.x*resolution.y;
    const int enc = encoding;
    int d,tmpd,ed,tmped; // distance
    // first try to find for each particular case
    if (!isModeColor(mode) && !monochrome.empty()) {
      // the user wants a monochrome mode, and there are some available
      it = best = monochrome.begin();
      convertMode(*it,resolution,encoding);
      d = abs(numPxl-(resolution.x*resolution.y));
      ed = abs(enc - static_cast<int>(encoding));
      ++it;
      while (it != monochrome.end()) {
        convertMode(*it,resolution,encoding);
        tmpd = abs(numPxl-(resolution.x*resolution.y));
        tmped = abs(enc - static_cast<int>(encoding));
        if ((tmpd<d) || ((tmpd<=d) && (tmped<ed)))  {
          best=it;
          d=tmpd;
          ed=tmped;
        }
        ++it;
      }
      return (*best);
    } // else: we have only color modes!, so get the best it gets!

    it = best = inColor.begin();
    convertMode(*it,resolution,encoding);
    d = abs(numPxl-(resolution.x*resolution.y));
    ed = abs(enc - static_cast<int>(encoding));
    ++it;
    while (it != inColor.end()) {
      convertMode(*it,resolution,encoding);
      tmpd = abs(numPxl-(resolution.x*resolution.y));
      tmped = abs(enc - static_cast<int>(encoding));
      if ((tmpd<d) || ((tmpd<=d) && (tmped<ed)))  {
        best=it;
        d=tmpd;
        ed=tmped;
      }
      ++it;
    }
    return (*best);
  }

  // only updateParameters when camera is initialized
  bool fireWireDCAM::updateParameters() {
    bool b = true;

    parameters& par = getRWParameters();
    
    if (!isCameraActive()) {
      if (!initCamera(par)) {
        return false;
      }
      activeCameraName_ = par.cameraName;
    } else if (activeCameraName_ != par.cameraName) {
      // it seems that a different camera has to be used
      releaseCamera();
      if (!initCamera(par)) {
        return false;
      }
      activeCameraName_ = par.cameraName;
    }
      
    if (par.fixMode == Upload) {
      dumpParameters(par);
    } else if (par.fixMode == AutoFix) {
      fixParameters(par);
    }

    _lti_debug("setting parameters in camera...\n");

    // brightness
    if (isFeatureAvailable(FEATURE_BRIGHTNESS)) {
      b = b && setBrightness(par.brightnessState,par.brightness);
    }

    // exposure
    if (isFeatureAvailable(FEATURE_EXPOSURE)) {
      b = b && setExposure(par.exposureState,par.exposure);
    }

    // sharpness
    if (isFeatureAvailable(FEATURE_SHARPNESS)) {
      b = b && setSharpness(par.sharpnessState,par.sharpness);
    }

    // white balance
    if (isFeatureAvailable(FEATURE_WHITE_BALANCE)) {
      b = b && setWhiteBalance(par.whiteBalanceState,par.blueGain,par.redGain);
    }

    // hue
    if (isFeatureAvailable(FEATURE_HUE)) {
      b = b && setHue(par.hueState,par.hue);
    }

    // saturation
    if (isFeatureAvailable(FEATURE_SATURATION)) {
      b = b && setSaturation(par.saturationState,par.saturation);
    }

    // gamma
    if (isFeatureAvailable(FEATURE_GAMMA)) {
      b = b && setGamma(par.gammaState,par.gamma);
    }

    // shutter
    if (isFeatureAvailable(FEATURE_SHUTTER)) {
      b = b && setShutter(par.shutterState,par.shutter);
    }

    // gain
    if (isFeatureAvailable(FEATURE_GAIN)) {
      b = b && setGain(par.gainState,par.gain);
    }

    // iris
    if (isFeatureAvailable(FEATURE_IRIS)) {
      b = b && setIris(par.irisState,par.iris);
    }

    // focus
    if (isFeatureAvailable(FEATURE_FOCUS)) {
      b = b && setFocus(par.focusState,par.focus);
    }

    // zoom
    if (isFeatureAvailable(FEATURE_ZOOM)) {
      b = b && setZoom(par.zoomState,par.zoom);
    }

    // pan
    if (isFeatureAvailable(FEATURE_PAN)) {
      b = b && setPan(par.panState,par.pan);
    }

    // tilt
    if (isFeatureAvailable(FEATURE_TILT)) {
      b = b && setTilt(par.tiltState,par.tilt);
    }

    // if user want this so, we will wait
    if (par.waitOnePush) {
      b = b && onePushWait();
    } else {
      onePushLock_.lock();
      waitingFeatures_.clear();
      onePushLock_.unlock();
    }

    unsigned int format = 0;
    unsigned int mode = 0;
    unsigned int framerate = convertFramerate(par.framerate);

    if (b) {
      // check if the mode in the parameters is supported.
      std::set<uint32>::const_iterator it;
      const std::set<uint32>& supportedModes =
        cams_[activeCamera_].supportedModes;

      mode = convertMode(par.resolution,par.encoding);
      it = supportedModes.find(mode);
      if (it == supportedModes.end()) {
        b=false;
        setStatusString("Selected mode not supported by this camera");
        return b;
      } else {
        // determine color mode and format
        ipoint res;
        convertMode(mode,res,colorMode_);
        format=getFormat(res);
      }
    }
      
    if (setupCapture_) {
      _lti_debug("release old camera structure...\n");

      // stop transmission
      if (isoTransStarted_) {
        dc1394_stop_iso_transmission(*handle_,*node_);
        isoTransStarted_=false;
      }

      // remember: even if this is called release "camera", it just
      // releases the "capture" buffers and related stuff
      dc1394_dma_release_camera(*handle_,&camera_);
      setupCapture_ = false;
    }
      
      
    _lti_debug("setup capture...\n");
    int drop = (par.dropFrames) ? 1 : 0;

    // the libdc1394 will take charge of the deviceFile string deallocation
    // so we need to create a copy of it using malloc (as the lib will use
    // free)
    char* devFile = 
      reinterpret_cast<char*>(malloc(par.deviceFile.length()+1));
    strcpy(devFile,par.deviceFile.c_str());

    if (dc1394_dma_setup_capture(*handle_,
                                 *node_,
                                 busChannel_,
                                 format,
                                 mode,
                                 busSpeed_,
                                 framerate,
                                 par.dmaBufferSize,
                                 drop,
                                 devFile,
                                 &camera_)==DC1394_SUCCESS) {
      setupCapture_ = true;
    } else {
      _lti_debug("unable to setup capture\n");
      b = false;
    } 
      
    bool oneShot = (par.oneShot && oneShotCapable_);

    // if oneShot mode is not set turn on transmission
    if (!oneShot) {
      _lti_debug("start iso-transmission...\n");
      if (dc1394_start_iso_transmission(*handle_,*node_)
          !=DC1394_SUCCESS) {
        _lti_debug("unable to start iso transmission\n");
        isoTransStarted_ = false;
        b = false;
      } else {
        isoTransStarted_ = true;
      }
    }

    return b;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // helper functio to capture a frame 
  bool fireWireDCAM::captureFrame() {
    const parameters& par = getParameters();

    // is there a camera handle?
    if (!setupCapture_) {
      // there should be a status-string from updateParameters()
      return false;
    }

    if (par.oneShot && oneShotCapable_) {      
      _lti_debug("set into one shot mode...\n");
      if (dc1394_set_one_shot(*handle_,*node_)!=DC1394_SUCCESS) {
        _lti_debug("unable to enter one shot mode...\n");
        setStatusString("unable to enter one shot mode.\n");
        return false;
      }
    }

    _lti_debug("last dma buffer: " << camera_.dma_last_buffer<<" of "
               camera_.num_dma_buffers << "\n");

    // capture and wait
    if (dc1394_dma_single_capture(&camera_)!=DC1394_SUCCESS) {
      _lti_debug("unable to do dma_single_capture\n");
      setStatusString("Unable to capture single frame. Check camera setup.\n");
      return false;
    }
    return true;
  }

  
  // On place apply for type image!
  bool fireWireDCAM::apply(image& srcdest) {
    // capture frame
    if (!(captureFrame())) {
      // statusString is set in captureFrame();
      return false;
    }

    // determine how the conversion should be done
    switch (colorMode_) {
      case (YUV411) : {
        _lti_debug("convert frame YUV411 -> RGB...\n");
        yuv411toRGB(srcdest);
        break;
      }
      case (YUV422) : {
        _lti_debug("convert frame YUV422 -> RGB...\n");
        yuv422toRGB(srcdest);
        break;
      }
      case (Mono8) : {
        _lti_debug("bayer conversion of frame...\n");
        const int rows = camera_.frame_height;
        const int cols = camera_.frame_width;
        ubyte* sPtr = reinterpret_cast<ubyte*>(camera_.capture_buffer);
        ch8_.useExternData(rows,cols,sPtr);
        bayer_.apply(ch8_,srcdest);
        break;
      }
      default: 
        setStatusString("Unsupported encoding type");
        return false;
        break;
    }
  
    _lti_debug("release buffer...\n");
    dc1394_dma_done_with_buffer(&camera_);
    
    return true;
  }

  bool fireWireDCAM::apply(channel8& dest) {
    // TODO: FIXME: too slow if it is indeed a ch8!

    image tmp;
    if (apply(tmp)) {
      dest.castFrom(tmp);
    } else {
      return false;
    }
    
    return true;
  }

  bool fireWireDCAM::grabRaw(matrix<ubyte>& dest) {
    // nothing done when not in mono mode
    if ((colorMode_!=Mono8) && (colorMode_!=Raw8)) {
      setStatusString(
        "Error: to grab raw images the camera colormode must be mono8\n");
      return false;
    }

    // capture frame
    if (!(captureFrame())) {
      // statusString is set in captureFrame();
      return false;
    } 

    // just resize and copy data
    _lti_debug("copy frame...\n");
    int rows = camera_.frame_height;
    int cols = camera_.frame_width;
    dest.resize(rows,cols,ubyte(),false,false);
    ubyte* ptr = reinterpret_cast<ubyte*>(&dest.at(0,0));
    memcpy(ptr,camera_.capture_buffer,sizeof(ubyte)*rows*cols);


    // never forget to release the buffer ;-)
    _lti_debug("release buffer...\n");
    dc1394_dma_done_with_buffer(&camera_);

    return true;
  }


  // simple conversion from yuv422 to rgb, NO interpolation is done
  void fireWireDCAM::yuv422toRGB(image& dest) const {
    int rows = camera_.frame_height;
    int cols = camera_.frame_width;
    dest.resize(rows,cols,rgbPixel(),false,false);



   // 4 pixels are stored in 8 byte
    const int elemSize = 8;

    // iterator for destination pixels
    image::iterator itd = dest.begin();

    ubyte* sPtr = reinterpret_cast<ubyte*>(camera_.capture_buffer);
    // iterator for capture buffer
    ubyte* iPtr;
    // end of capture buffer
    ubyte* ePtr = sPtr + ((rows*cols)/4)*elemSize;
    
    for (iPtr = sPtr; iPtr != ePtr;) {
      // byte structure is uyvyuyvyuyvy
      const ubyte u1 = *(iPtr++);
      const ubyte y1 = *(iPtr++);
      const ubyte v1 = *(iPtr++);
      const ubyte y2 = *(iPtr++);
      // 1st pixel
      merger_.apply(y1,u1,v1,*(itd++));
      // 2nd pixel
      merger_.apply(y2,u1,v1,*(itd++));      
    }
  }

  // simple conversion from yuv411 to rgb, NO interpolation is done
  void fireWireDCAM::yuv411toRGB(image& dest) const {
    int rows = camera_.frame_height;
    int cols = camera_.frame_width;
    dest.resize(rows,cols,rgbPixel(),false,false);



   // 4 pixels are stored in 6 byte
    const int elemSize = 6;

    // iterator for destination pixels
    image::iterator itd = dest.begin();

    ubyte* sPtr = reinterpret_cast<ubyte*>(camera_.capture_buffer);
    // iterator for capture buffer
    ubyte* iPtr;
    // end of capture buffer
    ubyte* ePtr = sPtr + ((rows*cols)/4)*elemSize;
    
    for (iPtr = sPtr; iPtr != ePtr;) {
      // byte structure is uyyvyyuyyvyy
      const ubyte u1 = *(iPtr++);
      const ubyte y1 = *(iPtr++);
      const ubyte y2 = *(iPtr++);
      const ubyte v1 = *(iPtr++);
      const ubyte y3 = *(iPtr++);
      const ubyte y4 = *(iPtr++);
      // 1st pixel
      merger_.apply(y1,u1,v1,*(itd++));
      // 2nd pixel
      merger_.apply(y2,u1,v1,*(itd++));      
      // 3rd pixel
      merger_.apply(y3,u1,v1,*(itd++));
      // 4th pixel
      merger_.apply(y4,u1,v1,*(itd++));      

    }
  }

  bool fireWireDCAM::dumpParameters(parameters& par) {
    if (!isCameraActive()) {
      return false;
    }

    bool b = true;;

    // the initial set of parameters is surely  almost complete
    if (&par != &getParameters()) {
      par.copy(getParameters());
    }

    par.fixMode = Upload;

    // oneShotCapable_ was initialized by cameraInit()
    par.oneShot = (par.oneShot && oneShotCapable_);
    
    _lti_debug("reading parameters from camera...\n");

    // videomode
    b = b && getVideoMode(par.resolution,par.encoding);
    _lti_debug(" videomode is: " << par.resolution <<
               " encoding: " << par.encoding << "\n");

    // framerate
    b = b && getFramerate(par.framerate);

    // sometimes the camera does not support the reported framerate, so fixit
    unsigned int mode = convertMode(par.resolution,par.encoding);
    par.framerate = computeClosestFrameRate(mode,par.framerate);

    _lti_debug(" framerate is: "<<par.framerate<<"\n");

    // brightness
    if (isFeatureReadable(FEATURE_BRIGHTNESS)) {
      b = b && getBrightness(par.brightnessState,par.brightness);     
    } else {
      par.brightness = 0;
      par.brightnessState = FeatureNA;
    }
    
    // exposure
    if (isFeatureReadable(FEATURE_EXPOSURE)) {
      b = b && getExposure(par.exposureState,par.exposure);     
    } else {
      par.exposure = 0;
      par.exposureState = FeatureNA;
    }

    // sharpness
    if (isFeatureReadable(FEATURE_SHARPNESS)) {
      b = b && getSharpness(par.sharpnessState,par.sharpness);     
    } else {
      par.sharpness = 0;
      par.sharpnessState = FeatureNA;
    }

    // white balance (redGain/blueGain)
    if (isFeatureReadable(FEATURE_WHITE_BALANCE)) {
      b = b && getWhiteBalance(par.whiteBalanceState,par.blueGain,par.redGain);
    } else {
      par.redGain = 0;
      par.blueGain = 0;
      par.whiteBalanceState = FeatureNA;
    }
    
    // hue
    if (isFeatureReadable(FEATURE_HUE)) {
      b = b && getHue(par.hueState,par.hue);     
    } else {
      par.hue = 0;
      par.hueState = FeatureNA;
    }

    // saturation
    if (isFeatureReadable(FEATURE_SATURATION)) {
      b = b && getSaturation(par.saturationState,par.saturation);     
    } else {
      par.saturation = 0;
      par.saturationState = FeatureNA;
    }

    // gamma
    if (isFeatureReadable(FEATURE_GAMMA)) {
      b = b && getGamma(par.gammaState,par.gamma);     
    } else {
      par.gamma = 0;
      par.gammaState = FeatureNA;
    }

    // shutter
    if (isFeatureReadable(FEATURE_SHUTTER)) {
      b = b && getShutter(par.shutterState,par.shutter);     
    } else {
      par.shutter = 0;
      par.shutterState = FeatureNA;
    }

    // gain
    if (isFeatureReadable(FEATURE_GAIN)) {
      b = b && getGain(par.gainState,par.gain);     
    } else {
      par.gain = 0;
      par.gainState = FeatureNA;
    }

    // iris
    if (isFeatureReadable(FEATURE_IRIS)) {
      b = b && getIris(par.irisState,par.iris);     
    } else {
      par.iris = 0;
      par.irisState = FeatureNA;
    }

    // focus
    if (isFeatureReadable(FEATURE_FOCUS)) {
      b = b && getFocus(par.focusState,par.focus);     
    } else {
      par.focus = 0;
      par.focusState = FeatureNA;
    }

    // zoom
    if (isFeatureReadable(FEATURE_ZOOM)) {
      b = b && getZoom(par.zoomState,par.zoom);     
    } else {
      par.zoom = 0;
      par.zoomState = FeatureNA;
    }

    // pan
    if (isFeatureReadable(FEATURE_PAN)) {
      b = b && getPan(par.panState,par.pan);     
    } else {
      par.pan = 0;
      par.panState = FeatureNA;
    }

    // tilt
    if (isFeatureReadable(FEATURE_TILT)) {
      b = b && getTilt(par.tiltState,par.tilt);     
    } else {
      par.tilt = 0;
      par.tiltState = FeatureNA;
    }

    return b;
  }
  
  // protected internal helper
  bool fireWireDCAM::isFeatureAvailable(const unsigned int featureId) const {
    assert (notNull(features_));
    assert((featureId>=FEATURE_MIN) && (featureId<=FEATURE_MAX));

    const dc1394_feature_info& fts=features_->feature[featureId-FEATURE_MIN];

    // we are assuming this is true.  If some time in the future it isn't
    // something has to shout outloud!!
    assert(fts.feature_id == featureId);

    return (fts.available==DC1394_TRUE);
  }
 

  // protected internal helper
  bool fireWireDCAM::isFeatureReadable(const unsigned int featureId) const {
    assert (notNull(features_));
    assert((featureId>=FEATURE_MIN) && (featureId<=FEATURE_MAX));

    const dc1394_feature_info& fts=features_->feature[featureId-FEATURE_MIN];

    // we are assuming this is true.  If some time in the future it isn't
    // something has to shout outloud!!
    assert(fts.feature_id == featureId);

    return ( (fts.available==DC1394_TRUE) && 
             (fts.readout_capable==DC1394_TRUE) );
  }
 
  unsigned int 
  fireWireDCAM::checkFeatureStates(const unsigned int featureId) const {
    assert (notNull(features_));
    assert((featureId>=FEATURE_MIN) && (featureId<=FEATURE_MAX));

    const dc1394_feature_info& fts=features_->feature[featureId-FEATURE_MIN];

    unsigned int result = 0;
    if (fts.available==DC1394_TRUE) {
      if (fts.on_off_capable == DC1394_TRUE) {
        result |= FeatureOff;
      }
      if (fts.auto_capable == DC1394_TRUE) {
        result |= FeatureAuto;
      }
      if (fts.manual_capable == DC1394_TRUE) {
        result |= FeatureManual;
      }
      if (fts.one_push == DC1394_TRUE) {
        result |= FeatureOnePush;
      }
      if (fts.absolute_capable == DC1394_TRUE) {
        result |= FeatureAbsolute;
      }
    }
    return result;
  }
  
  bool fireWireDCAM::checkFeatureValueRange(const unsigned int featureId,
                                            int& minValue,
                                            int& maxValue) const {
    assert (notNull(features_));
    assert((featureId>=FEATURE_MIN) && (featureId<=FEATURE_MAX));
    
    const dc1394_feature_info& fts=features_->feature[featureId-FEATURE_MIN];

    minValue = fts.min;
    maxValue = fts.max;
    
    return (fts.available==DC1394_TRUE);
  }

  // Get the camera feature state
  bool fireWireDCAM::getFeatureState(const unsigned int featureId,
                                     eFeatureState& state) const {
    assert (notNull(features_));
    assert((featureId>=FEATURE_MIN) && (featureId<=FEATURE_MAX));
    
    const dc1394_feature_info& fts=features_->feature[featureId-FEATURE_MIN];

    if (fts.available == DC1394_FALSE) {
      state = FeatureNA;
      return true;
    }

    if ( (fts.on_off_capable == DC1394_TRUE) &&
         (fts.is_on == DC1394_FALSE) ) {
      state = FeatureOff;
      return true;  // feature available and turned off! everything is ok
    }

    // if we got here, wether the feature is always on (not on_off_capable),
    // or it can be turned off, but it is anyway on!

    if ( (fts.auto_capable == DC1394_TRUE) &&
         (fts.auto_active == DC1394_TRUE) ) {
      state=FeatureAuto;
      return true;
    }

    if ( (fts.one_push == DC1394_TRUE) &&
         (fts.one_push_active == DC1394_TRUE) ) {
      state=FeatureOnePush;
      return true;
    }

    if ( (fts.absolute_capable == DC1394_TRUE) &&
         (fts.abs_control == DC1394_TRUE) ) {
      state=FeatureAbsolute;
      return true;
    }

    if (fts.manual_capable == DC1394_TRUE) {
      state=FeatureManual;
      return true;
    }

    state = FeatureNA;
    // if we get until here, then the camera is maybe not fully IIDC compliant
    return false;        // report an error anyway
  }


  // protected internal helper
  bool fireWireDCAM::isFeatureAuto(const unsigned int featureId) const {
    assert (notNull(features_));
    assert((featureId>=FEATURE_MIN) && (featureId<=FEATURE_MAX));

    dc1394bool_t isAuto;
    if (dc1394_is_feature_auto(*handle_,*node_,featureId,&isAuto) 
        != DC1394_SUCCESS) {
      // return false if an error occurs
      _lti_debug("unable to ask if feature " <<
                 dc1394_feature_desc[featureId-FEATURE_MIN]<<" is auto\n");
      
      return false;
    }
    return (isAuto==DC1394_TRUE);
  }

  // protected internal helper
  bool fireWireDCAM::setFeatureAuto(const unsigned int featureId, 
                                    const bool on) const {
    assert (notNull(features_));
    assert((featureId>=FEATURE_MIN) && (featureId<=FEATURE_MAX));

    // we have to update some attributes here (following corianders code...)
    // so, we also need to get a read-writable reference to the feature's
    // data structure.
    dc1394_feature_info& fts =
      cams_[activeCamera_].featureSet.feature[featureId-FEATURE_MIN];

    const unsigned int val = on ? 1 : 0;

    if ( fts.auto_capable == DC1394_TRUE) {
      if (dc1394_auto_on_off(*handle_,*node_,featureId,val) != 
          DC1394_SUCCESS) {
        std::string str = "Auto setting for feature ";
        str += dc1394_feature_desc[featureId-FEATURE_MIN];
        str += " could not be turned ";
        str += (on?"on":"off");
        setStatusString(str.c_str());
        
        return false;
      } else {
        fts.auto_active = on ? DC1394_TRUE : DC1394_FALSE; // update the state
      }
    } else if (on) {
      // there is an error only if the user wants to turn the auto mode on
      // the system may accept manual, but no auto
      std::string str = "Feature ";
      str += dc1394_feature_desc[featureId-FEATURE_MIN];
      str += " does not support the auto state.";
      setStatusString(str.c_str());      
      return false;          
    }
    
    return true;
  }


  bool fireWireDCAM::setFeatureOn(const unsigned int featureId,
                                  const bool on) {
    assert (notNull(features_));
    assert((featureId>=FEATURE_MIN) && (featureId<=FEATURE_MAX));

    // we have to update some attributes here (following corianders code...)
    // so, we also need to get a read-writable reference to the feature's
    // data structure.
    dc1394_feature_info& fts =
      cams_[activeCamera_].featureSet.feature[featureId-FEATURE_MIN];

    const unsigned int val = on ? 1 : 0;

    if ( fts.on_off_capable == DC1394_TRUE) {
      if (dc1394_feature_on_off(*handle_,*node_,featureId,val) != 
          DC1394_SUCCESS) {
        std::string str = "Feature ";
        str += dc1394_feature_desc[featureId-FEATURE_MIN];
        str += " could not be turned ";
        str += (on?"on":"off");
        setStatusString(str.c_str());
        
        return false;
      } else {
        fts.is_on = on ? DC1394_TRUE : DC1394_FALSE; // update the state
      }
    } else {
      std::string str = "Feature ";
      str += dc1394_feature_desc[featureId-FEATURE_MIN];
      str += " does not support the on/off capability";
      setStatusString(str.c_str());
      
      return false;          
    }
    
    return true;
  }

  bool fireWireDCAM::setFeatureAbsoluteState(const unsigned int featureId,
                                             const bool on) {
    assert (notNull(features_));
    assert((featureId>=FEATURE_MIN) && (featureId<=FEATURE_MAX));
    
    // we have to update some attributes here (following corianders code...)
    // so, we also need to get a read-writable reference to the feature's
    // data structure.
    dc1394_feature_info& fts =
      cams_[activeCamera_].featureSet.feature[featureId-FEATURE_MIN];

    const unsigned int val = on ? 1 : 0;

    if ( fts.absolute_capable == DC1394_TRUE) {
      if (dc1394_absolute_setting_on_off(*handle_,*node_,featureId,val) != 
          DC1394_SUCCESS) {
        std::string str = "Absolute setting for feature ";
        str += dc1394_feature_desc[featureId-FEATURE_MIN];
        str += " could not be ";
        str += (on?"activated":"deactivated");
        setStatusString(str.c_str());
        
        return false;
      } else {
        fts.abs_control = on ? DC1394_TRUE : DC1394_FALSE; // update the state
      }
    } else {
      std::string str = "Feature ";
      str += dc1394_feature_desc[featureId-FEATURE_MIN];
      str += " does not support the absolute setting.";
      setStatusString(str.c_str());
      
      return false;          
    }
    
    return true;
  }

  // Set feature state
  bool fireWireDCAM::setFeatureState(const unsigned int featureId,
                                     const eFeatureState state) {
    assert (notNull(features_));
    assert((featureId>=FEATURE_MIN) && (featureId<=FEATURE_MAX));
    
    // read only reference to 
    const dc1394_feature_info& fts=features_->feature[featureId-FEATURE_MIN];

    switch(state) {
      case FeatureNA:
        return true;
        break;
      case FeatureOff:
        return setFeatureOn(featureId,false);
        break;
      case FeatureAuto:
        // for auto we have to turn the feature on, if necessary:
        if (fts.on_off_capable == DC1394_TRUE) {
          if (!setFeatureOn(featureId)) { // turn it on
            return false; // if error occurred
          }
        }
        
        // for absolute setting
        if (fts.absolute_capable == DC1394_TRUE) {
          if (!setFeatureAbsoluteState(featureId,false)) {
            return false;
          }
        }
        
        // now to the auto stuff
        if (!setFeatureAuto(featureId)) {
          return false;
        }
        
        return true;  // feature available and turned off! everything is ok
        break;

      case FeatureManual:
        if ( fts.manual_capable == DC1394_TRUE) {

          // for manual we have to turn the feature on, if necessary:
          if (fts.on_off_capable == DC1394_TRUE) {
            if (!setFeatureOn(featureId)) { // turn it on
              return false; // if error occurred
            }
          }
          
          // for absolute setting
          if (fts.absolute_capable == DC1394_TRUE) {
            if (!setFeatureAbsoluteState(featureId,false)) {
              return false;
            }
          }

          // turn off the auto state
          if (!setFeatureAuto(featureId,false)) {
            return false;
          }
          
        } else {
          std::string str = "Feature ";
          str += dc1394_feature_desc[featureId-FEATURE_MIN];
          str += " does not support manual settings.";
          setStatusString(str.c_str());
          
          return false;          
        }
        return true;  // feature available and turned off! everything is ok
        break;
      case FeatureOnePush:
        // for one-push we have to turn the feature on, if necessary:
        if (fts.on_off_capable == DC1394_TRUE) {
          if (!setFeatureOn(featureId)) { // turn it on
            return false; // if error occurred
          }
        }
        
        // for absolute setting
        if (fts.absolute_capable == DC1394_TRUE) {
          if (!setFeatureAbsoluteState(featureId,false)) {
            return false;
          }
        }
        
        return onePushQueueInsert(featureId);
        break;
      case FeatureAbsolute:
        if ( fts.absolute_capable == DC1394_TRUE) {
          
          // for absolute state we have to turn the feature on, if necessary:
          if (fts.on_off_capable == DC1394_TRUE) {
            if (!setFeatureOn(featureId)) { // turn it on
              return false; // if error occurred
            }
          }
          
          // turn off the auto state
          if (setFeatureAuto(featureId,false)) {
            return false;
          }
          
          // for absolute setting
          if (fts.absolute_capable == DC1394_TRUE) {
            if (!setFeatureAbsoluteState(featureId,true)) {
              return false;
            }
          }
          
          return true;
        }
        break;
      default:
        return false;
    }
    
    return false;
  }

  // print actual feature set for debug purpose
  bool fireWireDCAM::printFeatures() const {
    dc1394_print_feature_set(const_cast<dc1394_feature_set*>(features_));
    return true;
  }

  bool fireWireDCAM::init() {
    return initHardware();
  }

  // init camera
  bool fireWireDCAM::initHardware() {
    if (!hwCreated_) { // static attribute
      if (bus_.get()) {
        if (cams_.get(bus_)) {            
          hwCreated_=true;
        } else {
          setStatusString(cams_.errorStr.c_str());
        }
      } else {
        setStatusString("Could not access camera. " \
                        "Check if the camera is conected, " \
                        "if you have permissions on devices /dev/raw1394 and "\
                        "/dev/video1394, or check if the firewire kernel "\
                        "modules are loaded.");
      }
    } 

    return hwCreated_;
  }

  bool fireWireDCAM::initCamera(const parameters& par) {

    if (!hwCreated_) {
      if (!initHardware()) {
        return false;
      }
    }

    bool error = false;

    // well, theoretically 'par' contains which camera we are looking for:
    if (!cameraNames_.use(par.cameraNamesFile)) {
      std::string msg = "Could not open or understand file " +
        par.cameraNamesFile;
      setStatusString(msg.c_str());
      return false;
    }
    u_int64_t euid64=0;

    // Flag to indicate if the cameraName is not being used and has to be
    // added to the database as soon as the first active camera has being
    // detected.
    bool needToAdd=false;
    needToAdd = !cameraNames_.query(par.cameraName,euid64);

    if (needToAdd) {
      if (!cams_.findFreeCamera(activeCamera_)) {
        setStatusString("Camera already in use!");
        return false;
      }

      if (!cameraNames_.add(par.cameraName,cams_[activeCamera_].info.euid_64)){
        // this should not happen ever
        setStatusString("Could not register camera name");
        return false;
      }
    } else {
      if (!cams_.findCamera(euid64,activeCamera_)) {
        std::string msg = "Camera " + par.cameraName + " is not working.";
        setStatusString(msg.c_str());
        return false;
      }      
    }
    
    // we found the camera, but let's check if we can use it
    if (!cams_.reserveIndex(activeCamera_)) {
      // ups, somebody was using it already!
      std::string msg = "Camera " + par.cameraName + " already in use.";
      setStatusString(msg.c_str());
      return false;
    }    

    const cameraInfo& cam = cams_[activeCamera_];
    const dc1394_camerainfo& info = cam.info;
    const dc1394_miscinfo& miscInfo = cam.miscInfo;
        
    // initialize shadows/shortcuts
    handle_=&info.handle;
    node_=&info.id;
    features_=&cam.featureSet;

    error = isNull(*handle_);
    
#ifdef _LTI_DEBUG
    // get camerainfo for debugging
    _lti_debug(" Using " << info.model << 
               " (" << info.vendor << ") on Node " << node_ << ".\n");
#endif

    oneShotCapable_ = (miscInfo.one_shot_capable == DC1394_TRUE);

    // first turn camera on...
    if ((cam.basics & (0x1<<16)) != 0) { // check if power on/off is available
      if (dc1394_camera_on(*handle_,*node_) != DC1394_SUCCESS) {
        setStatusString("Could not turn camera on");
        return false;      
      }
    }

    if (dc1394_get_iso_channel_and_speed(*handle_, 
                                         *node_,
                                         &busChannel_,
                                         &busSpeed_) != DC1394_SUCCESS) {
      // something went really wrong here!
      _lti_debug("Error: could not get IEEE1394 bus channel and speed");
      setStatusString("Could not get IEEE1394 bus channel and speed");
      return false;
    }

    return isCameraActive();
  }

  // -----------------------------------------------------------------------
  // Methods for direct access to camera
  // -----------------------------------------------------------------------


  inline bool fireWireDCAM::setFeatureValue(const unsigned int featureId,
                                            const eFeatureState state,
                                            const float value) {

    if (state == FeatureNA) {
      return true;
    }

    if (!isFeatureAvailable(featureId)) {
      std::string str = "Feature ";
      str += dc1394_feature_desc[featureId-FEATURE_MIN];
      str += " not supported.";
      setStatusString(str.c_str());
      return false;
    }

    if (setFeatureState(featureId,state)) {
      if (state == FeatureManual) {
        if (dc1394_set_feature_value(*handle_,*node_,featureId,
                                  static_cast<unsigned int>(value))
            != DC1394_SUCCESS) {
          std::string str = "Could not set ";
          str += dc1394_feature_desc[featureId-FEATURE_MIN];
          str += " value";
          setStatusString(str.c_str());
          return false;
        }
      } else if (state == FeatureAbsolute) {
        if (dc1394_set_absolute_feature_value(*handle_,*node_,
                                              featureId,value)
            != DC1394_SUCCESS) {
          std::string str = "Could not set ";
          str += dc1394_feature_desc[featureId-FEATURE_MIN];
          str += " absolute value";
          setStatusString(str.c_str());
          return false;
        }
      }

      return true;
    }
        
    return false;
  }

  inline bool fireWireDCAM::getFeatureValue(const unsigned int featureId,
                                            eFeatureState& state,
                                            float& value) const {
    
    if (!getFeatureState(featureId,state)) {
      return false;
    }
    
    const dc1394_feature_info& fts=features_->feature[featureId-FEATURE_MIN];
    
    if (fts.readout_capable == DC1394_FALSE) {
      std::string str = "Feature";
      str += dc1394_feature_desc[featureId-FEATURE_MIN];
      str += " is not read-out capable.";
      setStatusString(str.c_str());
      return false;
    }

    if (state != FeatureAbsolute) {
      unsigned int tmp;
      if (dc1394_get_feature_value(*handle_,*node_,featureId,&tmp)
          != DC1394_SUCCESS) {
        std::string str = "Value for feature ";
        str += dc1394_feature_desc[featureId-FEATURE_MIN];
        str += " could not be read.";
        setStatusString(str.c_str());
        return false;
      }
      value=static_cast<float>(tmp);
    } else {
      if (dc1394_query_absolute_feature_value(*handle_,*node_,featureId,&value)
          != DC1394_SUCCESS) {
        std::string str = "Absolute value for feature ";
        str += dc1394_feature_desc[featureId-FEATURE_MIN];
        str += " could not be read.";
        setStatusString(str.c_str());
        return false;
      }
    }
    
    return true;
  }

  bool fireWireDCAM::setBrightness(const eFeatureState state,
                                   const float brightness) {

    if (setFeatureValue(FEATURE_BRIGHTNESS,state,brightness)) {
      parameters& par = getRWParameters();
      par.brightnessState = state;
      par.brightness      = brightness;
      return true;
    }
        
    return false;
  }
  
  bool fireWireDCAM::getBrightness(eFeatureState& state,
                                   float& brightness) const {
    return getFeatureValue(FEATURE_BRIGHTNESS,state,brightness);
  }
  
  bool fireWireDCAM::setExposure(const eFeatureState state,
                                 const float exposure) {
    if (setFeatureValue(FEATURE_EXPOSURE,state,exposure)) {
      parameters& par = getRWParameters();
      par.exposureState = state;
      par.exposure      = exposure;
      return true;
    }
        
    return false;
  }
  
  bool fireWireDCAM::getExposure(eFeatureState& state,
                                 float& exposure) const {
    return getFeatureValue(FEATURE_EXPOSURE,state,exposure);
  }
  
  bool fireWireDCAM::setSharpness(const eFeatureState state,
                                  const float sharpness) {
    if (setFeatureValue(FEATURE_SHARPNESS,state,sharpness)) {
      parameters& par = getRWParameters();
      par.sharpnessState = state;
      par.sharpness      = sharpness;
      return true;
    }
        
    return false;
  }
  
  bool fireWireDCAM::getSharpness(eFeatureState& state,
                                  float& sharpness) const {
    return getFeatureValue(FEATURE_SHARPNESS,state,sharpness);
  }

  bool fireWireDCAM::setWhiteBalance(const eFeatureState state,
                                     const float ubGain,
                                     const float rvGain) {
    
    if (!isFeatureAvailable(FEATURE_WHITE_BALANCE)) {
      setStatusString("Feature white balance not supported");
      return false;
    }

    if (setFeatureState(FEATURE_WHITE_BALANCE,state)) {
      if ((state == FeatureManual) || (state == FeatureAbsolute)) {
        if (dc1394_set_white_balance(*handle_,*node_,
                                     static_cast<unsigned int>(ubGain),
                                     static_cast<unsigned int>(rvGain))
            != DC1394_SUCCESS) {
          setStatusString("Could not set white balance B/U and R/V gains");
          return false;
        }
      } 

      parameters& par = getRWParameters();
      par.whiteBalanceState = state;
      par.blueGain          = ubGain;
      par.redGain           = rvGain;
      
      return true;
    }
        
    // BUG? How to set the values in absolute mode?

    return false;

  }
  
  bool fireWireDCAM::getWhiteBalance(eFeatureState& state,
                                     float& ubGain,
                                     float& vrGain) {
    if (!getFeatureState(FEATURE_WHITE_BALANCE,state)) {
      return false;
    }
    
    const dc1394_feature_info& fts =
      features_->feature[FEATURE_WHITE_BALANCE-FEATURE_MIN];
    
    if (fts.readout_capable == DC1394_FALSE) {
      setStatusString("Feature white balance is not read-out capable.");
      return false;
    }

    unsigned int bg,rg;
    if (dc1394_get_white_balance(*handle_,*node_,&bg,&rg)
        != DC1394_SUCCESS) {
      setStatusString("Value for feature white balance could not be read.");
      return false;
    }
    ubGain=static_cast<float>(bg);
    vrGain=static_cast<float>(rg);
    
    return true;
  }
  
  bool fireWireDCAM::setHue(const eFeatureState state,
                            const float hue) {
    if (setFeatureValue(FEATURE_HUE,state,hue)) {
      parameters& par = getRWParameters();
      par.hueState = state;
      par.hue      = hue;
      return true;
    }
        
    return false;
  }
  
  bool fireWireDCAM::getHue(eFeatureState& state,
                            float& hue) const {
    return getFeatureValue(FEATURE_HUE,state,hue);
  }
  
  bool fireWireDCAM::setSaturation(const eFeatureState state,
                                   const float saturation) {
    if (setFeatureValue(FEATURE_SATURATION,state,saturation)) {
      parameters& par = getRWParameters();
      par.saturationState = state;
      par.saturation      = saturation;
      return true;
    }
        
    return false;
  }
  
  bool fireWireDCAM::getSaturation(eFeatureState& state,
                                   float& saturation) const {
    
    return getFeatureValue(FEATURE_SATURATION,state,saturation);
  }
  
  bool fireWireDCAM::setGamma(const eFeatureState state,
                              const float gamma) {
    if (setFeatureValue(FEATURE_GAMMA,state,gamma)) {
      parameters& par = getRWParameters();
      par.gammaState = state;
      par.gamma      = gamma;
      return true;
    }
        
    return false;
  }

  bool fireWireDCAM::getGamma(eFeatureState& state,
                              float& gamma) const {
    return getFeatureValue(FEATURE_GAMMA,state,gamma);
  }

  bool fireWireDCAM::setShutter(const eFeatureState state,
                                const float shutter) {
    if (setFeatureValue(FEATURE_SHUTTER,state,shutter)) {
      parameters& par = getRWParameters();
      par.shutterState = state;
      par.shutter      = shutter;
      return true;
    }
        
    return false;
  }

  bool fireWireDCAM::getShutter(eFeatureState& state,
                                float& shutter) const {
    return getFeatureValue(FEATURE_SHUTTER,state,shutter);
  }

  bool fireWireDCAM::setGain(const eFeatureState state,
                             const float gain) {
    if (setFeatureValue(FEATURE_GAIN,state,gain)) {
      parameters& par = getRWParameters();
      par.gainState = state;
      par.gain      = gain;
      return true;
    }
        
    return false;
  }

  bool fireWireDCAM::getGain(eFeatureState& state,
                             float& gain) const {
    return getFeatureValue(FEATURE_GAIN,state,gain);
  }

  bool fireWireDCAM::setIris(const eFeatureState state,
                             const float iris) {
     if (setFeatureValue(FEATURE_IRIS,state,iris)) {
      parameters& par = getRWParameters();
      par.irisState = state;
      par.iris      = iris;
      return true;
    }
        
    return false;
 }

  bool fireWireDCAM::getIris(eFeatureState& state,
                             float& iris) const {
    return getFeatureValue(FEATURE_IRIS,state,iris);
  }

  bool fireWireDCAM::setFocus(const eFeatureState state,
                              const float focus) {
    if (setFeatureValue(FEATURE_FOCUS,state,focus)) {
      parameters& par = getRWParameters();
      par.focusState = state;
      par.focus      = focus;
      return true;
    }
        
    return false;
  }

  bool fireWireDCAM::getFocus(eFeatureState& state,
                              float& focus) const {
    return getFeatureValue(FEATURE_FOCUS,state,focus);
  }

  bool fireWireDCAM::setZoom(const eFeatureState state,
                             const float zoom) {
    if (setFeatureValue(FEATURE_ZOOM,state,zoom)) {
      parameters& par = getRWParameters();
      par.zoomState = state;
      par.zoom      = zoom;
      return true;
    }
        
    return false;
  }

  bool fireWireDCAM::getZoom(eFeatureState& state,
                             float& zoom) const {
    return getFeatureValue(FEATURE_ZOOM,state,zoom);
  }

  bool fireWireDCAM::setPan(const eFeatureState state,
                            const float pan) {
    if (setFeatureValue(FEATURE_PAN,state,pan)) {
      parameters& par = getRWParameters();
      par.panState = state;
      par.pan      = pan;
      return true;
    }
        
    return false;
  }

  bool fireWireDCAM::getPan(eFeatureState& state,
                            float& pan) const {
    return getFeatureValue(FEATURE_PAN,state,pan);
  }

  bool fireWireDCAM::setTilt(const eFeatureState state,
                             const float tilt) {
     if (setFeatureValue(FEATURE_TILT,state,tilt)) {
      parameters& par = getRWParameters();
      par.tiltState = state;
      par.tilt      = tilt;
      return true;
    }
        
    return false;
  }

  bool fireWireDCAM::getTilt(eFeatureState& state,
                             float& tilt) const {
    return getFeatureValue(FEATURE_TILT,state,tilt);
  }
  
  
  // -----------------------------------------------------------------------
  // End of methods for direct access to camera
  // -----------------------------------------------------------------------

  bool fireWireDCAM::getFramerate(float& rate) const {
    unsigned int irate;
    bool b = true;
    b = b && (dc1394_get_video_framerate(*handle_,*node_,&irate)
              ==DC1394_SUCCESS);
    if (b) {
      // same enumeration values
      rate = convertFramerate(irate);
    } else {
      rate=0.0f;
    }

    return b;
  }

  unsigned int fireWireDCAM::convertFramerate(const float fps) const {
    static const float framerates[] = {1.875f,
                                       3.75f,
                                       7.5f,
                                       15.f,
                                       30.f,
                                       60.f,
                                       120.f,
                                       240.f};

    static const unsigned int frSize = 8;
    float min,tmp; 
    unsigned int minIdx,i;

    min=abs(fps-framerates[0]);
    minIdx=0;

    for (i=1;i<frSize;++i) {
      tmp = abs(fps-framerates[i]);
      if (tmp<min) {
        min=tmp;
        minIdx=i;
      } else {
        break; // since framerates[] is sorted is unnecessary to continue
      }
    }
    
    return (FRAMERATE_MIN+minIdx);
  }

  float fireWireDCAM::convertFramerate(const unsigned int eFR) const {
    switch(eFR) {
      case FRAMERATE_1_875:
        return 1.875f;
        break;
      case FRAMERATE_3_75:
        return 3.75f;
        break;
      case FRAMERATE_7_5:
        return 7.5f;
        break;
      case FRAMERATE_15:
        return 15.f;
        break;
      case FRAMERATE_30:
        return 30.f;
        break;
      case FRAMERATE_60:
        return 60.f;
        break;
      case FRAMERATE_120:
        return 120.f;
        break;
      case FRAMERATE_240:
        return 240.f;
        break;
      default:
        return 0.0f;
    }
    return 0.0f;
  }

  fireWireDCAM::eColorMode 
  fireWireDCAM::getColorCode7(const unsigned int code) const {
    switch (code) {
      case COLOR_FORMAT7_MONO8:
        return Mono8;
        break;
      case COLOR_FORMAT7_YUV411:
        return YUV411;
        break;
      case COLOR_FORMAT7_YUV422:
        return YUV422;
        break;
      case COLOR_FORMAT7_YUV444:
        return YUV444;
        break;
      case COLOR_FORMAT7_RGB8:
        return RGB8;
        break;
      case COLOR_FORMAT7_MONO16:
        return Mono16;
        break;
      case COLOR_FORMAT7_RGB16:
        return RGB16;
        break;
      case COLOR_FORMAT7_MONO16S:
        return Mono16s;
        break;
      case COLOR_FORMAT7_RGB16S:
        return RGB16s;
        break;
      case COLOR_FORMAT7_RAW8:
        return Raw8;
        break;
      case COLOR_FORMAT7_RAW16:
        return Raw16;
        break;
      default:
        return UnknownEncoding;
    }
    return UnknownEncoding;
  }

  bool fireWireDCAM::getVideoMode(ipoint& resolution,
                                  eColorMode& encoding) const {
    unsigned int imode;
    bool b = true;
    b = b && (dc1394_get_video_mode(*handle_,*node_,&imode)
              ==DC1394_SUCCESS);
    
    // some times the camera reports a mode it does not support!
    imode = computeClosestMode(imode);

    if (b) {
      if (imode <= MODE_FORMAT2_MAX) {
        // same enumeration values
        convertMode(imode,resolution,encoding);
      } else if (imode >= MODE_FORMAT7_MIN) {
        unsigned int x(0),y(0);
        b = b && (dc1394_query_format7_image_size(*handle_,*node_,
                                                  imode, &x, &y)
                  ==DC1394_SUCCESS);
        resolution.set(static_cast<int>(x),static_cast<int>(y));
        b = b && (dc1394_query_format7_color_coding_id(*handle_,*node_,
                                                       imode,&x)
                  ==DC1394_SUCCESS);
        encoding = getColorCode7(x);
      } else {
        // unsupported mode (or TODO yet) 
        resolution.set(0,0);
        encoding = Mono8;
      }
    }
    return b;
  }

  unsigned int fireWireDCAM::convertMode(const ipoint& resolution,
                                         const eColorMode& encoding) const {
    static const int resols[] = {(160*120),
                                 (320*240),
                                 (640*480),
                                 (800*600),
                                 (1024*768),
                                 (1280*960),
                                 (1600*1200)};
    
    int i,idx,d,tmp;
    idx=0;
    // comparisons will be done on the total number of pixels
    const int np = resolution.x*resolution.y;
    d=abs(np - resols[0]);
    for (i=1;i<7;++i) {
      tmp=abs(np - resols[i]);
      if (tmp<d) {
        d=tmp;
        idx=i;
      } else {
        // since the array is sorted, we can abort here as no smaller
        // distances will be found
        break;
      }
    }

    // we have to check if the active camera supports format7.  If so,
    // we switch to format7
    if (hasCameraFormat7() && (d>0)) {
      // since d>0 then the resolution is not one of the usual ones
      // and we need to activate format7
      // Let us look for the format7 mode with the nearest resolution that
      // is larger than the desired one
      
      const format7Info& f7i = cams_[activeCamera_].format7;
      i=0;
      // find first format7 mode available
      while((i<NUM_MODE_FORMAT7) && !f7i.mode[i].present){
        ++i;
      }
      if (i<NUM_MODE_FORMAT7) {
        // we get here, if there is at least one format7 mode present!
        // otherwise, we will get below the most similar non-format7
        // mode.

        // CHECKME: is here maxSize correct? or would it be better size?

        d=(f7i.mode[i].maxSize.x*f7i.mode[i].maxSize.y)-np;
        idx=i;
        ++i;
        while(i<NUM_MODE_FORMAT7) {
          tmp = (f7i.mode[i].maxSize.x*f7i.mode[i].maxSize.y) - np;
          if (f7i.mode[i].present && (tmp>0) && ((d<0) || (tmp<d))) {
            d = tmp;
            idx = i;
          }
        }

        // idx contains the index of the only present format7 mode, or
        // the index of the mode which is able to contain the desired
        // resolution the closest it gets.
        return MODE_FORMAT7_MIN+idx;
        return true;
      }
    }


    switch(idx) {
      case 0: // 160x120
        return MODE_160x120_YUV444;  // this is the only availabe mode for this
                                     // resolution
        break;
      case 1: // 320x240
        return MODE_320x240_YUV422;  // this is the only availabe mode for this
                                     // resolution
        break;
      case 2: // 640x480
        switch(encoding) {
          case Mono8:
            return MODE_640x480_MONO;
            break;
          case Mono16:
          case Mono16s:
            return MODE_640x480_MONO16;
            break;
          case YUV411:
            return MODE_640x480_YUV411;
            break;
          case YUV422:
            return MODE_640x480_YUV422;
            break;
          case YUV444:
            return MODE_640x480_YUV422; // nearest?
            break;
          case RGB8:
            return MODE_640x480_RGB;
            break;
          case RGB16:
          case RGB16s:
            return MODE_640x480_RGB; // nearest?
          default:
            return 0;
        }
        break;
      case 3: // 800x600
        switch(encoding) {
          case Mono8:
            return MODE_800x600_MONO;
            break;
          case Mono16:
          case Mono16s:
            return MODE_800x600_MONO16;
            break;
          case YUV411:
            return MODE_800x600_YUV422; // nearest?
            break;
          case YUV422:
            return MODE_800x600_YUV422;
            break;
          case YUV444:
            return MODE_800x600_YUV422; // nearest?
            break;
          case RGB8:
            return MODE_800x600_RGB;
            break;
          case RGB16:
          case RGB16s:
            return MODE_800x600_RGB; // nearest?
          default:
            return 0;
        }
        break;
      case 4: // 1024x768
        switch(encoding) {
          case Mono8:
            return MODE_1024x768_MONO;
            break;
          case Mono16:
          case Mono16s:
            return MODE_1024x768_MONO16;
            break;
          case YUV411:
            return MODE_1024x768_YUV422; // nearest?
            break;
          case YUV422:
            return MODE_1024x768_YUV422;
            break;
          case YUV444:
            return MODE_1024x768_YUV422; // nearest?
            break;
          case RGB8:
            return MODE_1024x768_RGB;
            break;
          case RGB16:
          case RGB16s:
            return MODE_1024x768_RGB; // nearest?
          default:
            return 0;
        }
        break;
      case 5: // 1280x960
         switch(encoding) {
          case Mono8:
            return MODE_1280x960_MONO;
            break;
          case Mono16:
          case Mono16s:
            return MODE_1280x960_MONO16;
            break;
          case YUV411:
            return MODE_1280x960_YUV422; // nearest?
            break;
          case YUV422:
            return MODE_1280x960_YUV422;
            break;
          case YUV444:
            return MODE_1280x960_YUV422; // nearest?
            break;
          case RGB8:
            return MODE_1280x960_RGB;
            break;
          case RGB16:
          case RGB16s:
            return MODE_1280x960_RGB; // nearest?
          default:
            return 0;
        }
       break;
      case 6: // 1600x1200
         switch(encoding) {
          case Mono8:
            return MODE_1600x1200_MONO;
            break;
          case Mono16:
          case Mono16s:
            return MODE_1600x1200_MONO16;
            break;
          case YUV411:
            return MODE_1600x1200_YUV422; // nearest?
            break;
          case YUV422:
            return MODE_1600x1200_YUV422;
            break;
          case YUV444:
            return MODE_1600x1200_YUV422; // nearest?
            break;
          case RGB8:
            return MODE_1600x1200_RGB;
            break;
          case RGB16:
          case RGB16s:
            return MODE_1600x1200_RGB; // nearest?
            break;
          default:
            return 0;
        }
       break;
      default:
        return 0; // error, we cannot get here!
    }
    
  }
  
  void fireWireDCAM::convertMode(const unsigned int value,
                                 ipoint& resolution,
                                 eColorMode& encoding) const {
    switch(value) {
      case MODE_160x120_YUV444:
        resolution.set(160,120);
        encoding = YUV444;
        break;
      case MODE_320x240_YUV422:
        resolution.set(320,240);
        encoding = YUV422;
        break;
      case MODE_640x480_YUV411:
        resolution.set(640,480);
        encoding = YUV411;
        break;
      case MODE_640x480_YUV422:
        resolution.set(640,480);
        encoding = YUV422;
        break;
      case MODE_640x480_RGB:
        resolution.set(640,480);
        encoding = RGB8;
        break;
      case MODE_640x480_MONO:
        resolution.set(640,480);
        encoding = Mono8;
        break;
      case MODE_640x480_MONO16:
        resolution.set(640,480);
        encoding = Mono16;
        break;
      case MODE_800x600_YUV422:
        resolution.set(800,600);
        encoding = YUV422;
        break;
      case MODE_800x600_RGB:
        resolution.set(800,600);
        encoding = RGB8;
        break;
      case MODE_800x600_MONO:
        resolution.set(800,600);
        encoding = Mono8;
        break;
      case MODE_1024x768_YUV422:
        resolution.set(1024,768);
        encoding = YUV422;
        break;
      case MODE_1024x768_RGB:
        resolution.set(1024,768);
        encoding = RGB8;
        break;
      case MODE_1024x768_MONO:
        resolution.set(1024,768);
        encoding = Mono8;
        break;
      case MODE_800x600_MONO16:
        resolution.set(800,600);
        encoding = Mono16;
        break;
      case MODE_1024x768_MONO16:
        resolution.set(1024,768);
        encoding = Mono16;
        break;
      case MODE_1280x960_YUV422:
        resolution.set(1280,960);
        encoding = YUV422;
        break;
      case MODE_1280x960_RGB:
        resolution.set(1280,960);
        encoding = RGB8;
        break;
      case MODE_1280x960_MONO:
        resolution.set(1280,960);
        encoding = Mono8;
        break;
      case MODE_1600x1200_YUV422:
        resolution.set(1600,1200);
        encoding = YUV422;
        break;
      case MODE_1600x1200_RGB:
        resolution.set(1600,1200);
        encoding = RGB8;
        break;
      case MODE_1600x1200_MONO:
        resolution.set(1600,1200);
        encoding = Mono8;
        break;
      case MODE_1280x960_MONO16:
        resolution.set(1280,960);
        encoding = Mono16;
        break;
      case MODE_1600x1200_MONO16:
        resolution.set(1600,1200);
        encoding = Mono16;
        break;
      case MODE_EXIF:
        // dummy values, as this mode maybe does not even exist in real cams.
        resolution.set(0,0);
        encoding = Mono8;
        break;
      case MODE_FORMAT7_0:
      case MODE_FORMAT7_1:
      case MODE_FORMAT7_2:
      case MODE_FORMAT7_3:
      case MODE_FORMAT7_4:
      case MODE_FORMAT7_5:
      case MODE_FORMAT7_6:
      case MODE_FORMAT7_7:
        convertMode7(value,resolution,encoding);
        break;
      default:
        resolution.set(0,0);
        encoding = Mono8;
        break;
    }
  }

  void fireWireDCAM::convertMode7(const unsigned int value,
                                 ipoint& resolution,
                                 eColorMode& encoding) const {
    if (hasCameraFormat7()) {
      if ((value>=MODE_FORMAT7_MIN) && (value<=MODE_FORMAT7_MAX)) {
        const format7ModeInfo& f7m = 
          cams_[activeCamera_].format7.mode[value-MODE_FORMAT7_MIN];
        
        if (f7m.present) {
          resolution.castFrom(f7m.maxSize);
          encoding = getColorCode7(f7m.colorCodingId);
          return;
        }
      }
      // else -> not format7
    }

    // unsupported format
    resolution.set(0,0);
    encoding = Mono8;
  }

  unsigned int fireWireDCAM::getFormat(const ipoint& resolution) const {
    const int d = resolution.x*resolution.y;
    if ((d==160*120) ||
        (d==320*240) ||
        (d==640*480)) {
      return FORMAT_VGA_NONCOMPRESSED;
    } else if ((d==800*600) ||
               (d==1024*768)) {
      return FORMAT_SVGA_NONCOMPRESSED_1;
    } else if ((d==1280*960) ||
               (d==1600*1200)) {
      return FORMAT_SVGA_NONCOMPRESSED_2;
    } 
    return FORMAT_SCALABLE_IMAGE_SIZE;
  }

  /*
   * Read function for fireWireDCAM::eFeatureState type
   *
   * @ingroup gStorable
   */
  bool read(ioHandler& handler, fireWireDCAM::eFeatureState& data) {
    std::string str;
    if (handler.read(str)) {
      if (str.find("NA") != std::string::npos) {
        data=fireWireDCAM::FeatureNA;
      } else if (str.find("ff") != std::string::npos) {
        data=fireWireDCAM::FeatureOff;
      } else if (str.find("uto") != std::string::npos) {
        data=fireWireDCAM::FeatureAuto;
      } else if (str.find("anual") != std::string::npos) {
        data=fireWireDCAM::FeatureManual;
      }  else if (str.find("ush") != std::string::npos) {
        data=fireWireDCAM::FeatureOnePush;
      } else if (str.find("bsolute") != std::string::npos) {
        data=fireWireDCAM::FeatureAbsolute;
      } else {
        data=fireWireDCAM::FeatureOff;
        str="Unknown symbol " + str;
        handler.setStatusString(str.c_str());
        return false; // unknown symbol
      }
      return true;
    } 
    return false;
  }

  /*
   * Write function for fireWireDCAM::eFeatureState type
   *
   * @ingroup gStorable
   */
  bool write(ioHandler& handler, const fireWireDCAM::eFeatureState& data) {
    switch(data) {
      case fireWireDCAM::FeatureNA:
        return write(handler,"FeatureNA");
        break;
      case fireWireDCAM::FeatureOff:
        return write(handler,"FeatureOff");
        break;
      case fireWireDCAM::FeatureAuto:
        return write(handler,"FeatureAuto");
        break;
      case fireWireDCAM::FeatureManual:
        return write(handler,"FeatureManual");
        break;
      case fireWireDCAM::FeatureOnePush:
        return write(handler,"FeatureOnePush");
        break;
      case fireWireDCAM::FeatureAbsolute:
        return write(handler,"FeatureAbsolute");
        break;
      default:
        return write(handler,"Unknown");
        break;
    }
    return false;
  }

  // ------------------------------------------------------------------------
  // One-Push scheduler
  // ------------------------------------------------------------------------
  bool fireWireDCAM::onePushQueueInsert(const unsigned int featureId) {
    // read only reference to 
    const dc1394_feature_info& fts=features_->feature[featureId-FEATURE_MIN];

    if (fts.one_push == DC1394_TRUE) {
      // one push supported for that feature
      if (dc1394_start_one_push_operation(*handle_,*node_,featureId) 
          != DC1394_SUCCESS) {
        std::string str = "One-push for feature ";
        str += dc1394_feature_desc[featureId-FEATURE_MIN];
        str += " could not be activated";
        setStatusString(str.c_str());
        return false;
      }
      onePushLock_.lock();
      waitingFeatures_.insert(featureId);
      onePushLock_.unlock();
      return true;
    }

    return false; // not supported at all
  }

  bool fireWireDCAM::onePushWait() {
    const parameters& par = getParameters();

    double theTime = 0;
    const double timeStep = 1000000.0/par.onePushUpdateRate;
    const double timeout = par.onePushTimeout;
    std::set<unsigned int>::iterator it,tmpIt;
    bool ready = false;

    while (!ready && (theTime < timeout)) {
      onePushLock_.lock();
      it = waitingFeatures_.begin();
      while (it != waitingFeatures_.end()) {
        const unsigned int featureId = *it;
        dc1394bool_t value;
        
        if (dc1394_is_one_push_in_operation(*handle_,*node_,featureId,&value)
            ==DC1394_SUCCESS) {
          if (value == DC1394_FALSE) {
            // this feature was ready: remove it from the queue
            tmpIt=it;
            ++it;
            waitingFeatures_.erase(tmpIt);
          } else {
            // still active, just continue
            ++it;
          }        
        } else {
          // we cannot get information on that feature, better remove it
          tmpIt=it;
          ++it;
          waitingFeatures_.erase(tmpIt);
        }
      }
      ready = waitingFeatures_.empty();
      onePushLock_.unlock();
      // now wait and then try again, if we still have time
      passiveWait(static_cast<int>(timeStep));
      theTime+=timeStep;
    }
    
    return ready;
  }

  
}

#include "ltiUndebug.h"

#endif //USE_FIRE_WIRE_DCAM
