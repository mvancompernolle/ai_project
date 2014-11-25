/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiToUCam.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 24.04.01
 * revisions ..: $Id: ltiToUCam.cpp,v 1.18 2010/04/10 19:28:45 alvarado Exp $
 */

#include "ltiToUCam.h"
#include "ltiTypes.h"

#ifdef _USE_PHILIPS_TOUCAM
#ifdef __linux__

// C-stuff required for the device control

// these types are required by <linux/videodev.h> but some
// macro jungle may avoid their definition, so just use some
// configure trick to check if they can be obtained from <linux/types.h>
// or otherwise define them here
#if !defined (HAVE_U64_V4L)
typedef unsigned long ulong;
typedef lti::uint64 __u64;
typedef lti::int64 __s64;
#endif

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev.h>
#include <pwc-ioctl.h>

#include <cstdio>
#include <unistd.h>
#include <errno.h>
#include <cstring>

#undef _LTI_DEBUG
#define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {

  // -----------------------------------------------------------------
  // toUCam static members
  // -----------------------------------------------------------------
  /*
   * look up table to accelerate calculate saturation
   */
  const ubyte* toUCam::lutSaturation = 0;

  /*
   * look up tables to accelerate conversion Luv -> RGB
   */
  const int* toUCam::lutUg = 0;
  const int* toUCam::lutUb = 0;
  const int* toUCam::lutVg = 0;
  const int* toUCam::lutVr = 0;

  // -----------------------------------------------------------------
  //  toUCam::parameters
  // -----------------------------------------------------------------

  toUCam::parameters::parameters(const bool valid)
    : frameGrabber::parameters() {
    if (valid) {
      size = point(320,240);
      brightness = 0.5;
      hue = 0.5;
      saturation = 0.5;
      contrast = 0.5;
      gamma = 0.5;
      frameRate = 30;
      snapshotMode = false;
      compression = Medium;
      agc = false;
      gain = 0.75;
      autoShutter = false;
      shutterSpeed = 0.75;

      whiteBalance = Outdoor;
      redGain = 0.5f;
      blueGain = 0.5f;

      led = 0;

      device = "/dev/video0";
    } else {
      // initialize all last used parameters with invalid values to force
      // setting the correct parameters the first time setParameters is called
      size = point(-1,-1);
      brightness = -1;
      hue = -1;
      saturation = -1;
      contrast = -1;
      gamma = -1;
      frameRate = -1;
      snapshotMode = false;
      compression = Invalid;
      agc = false;
      gain = -1;
      autoShutter = false;
      shutterSpeed = -1;

      whiteBalance = InvalidWB;
      redGain = -1;
      blueGain = -1;

      led = -1;

      device = "/dev/video0";
    }
  }

  functor::parameters* toUCam::parameters::clone() const {
    return ( new parameters(*this) );
  }

  const char* toUCam::parameters::getTypeName() const {
    return "toUCam::parameters";
  }

  toUCam::parameters&
    toUCam::parameters::copy(const parameters& other ) {

#   ifndef _LTI_MSC_6
      // for normal  ANSI C++
      frameGrabber::parameters::copy(other);
#   else
      // this doesn't work with MS-VC++ 6.0 (an once again... another bug)
      // ...so we have to use this workaround.
      // Conditional on that, copy may not be virtual.
      frameGrabber::parameters&
        (frameGrabber::parameters::* p_copy)(const frameGrabber::parameters&)=
        frameGrabber::parameters::copy;
      (this->*p_copy)(other);
#   endif

    size = other.size;
    brightness = other.brightness;
    hue = other.hue;
    saturation = other.saturation;
    contrast = other.contrast;
    gamma = other.gamma;
    frameRate = other.frameRate;
    snapshotMode = other.snapshotMode;
    compression = other.compression;
    agc = other.agc;
    gain = other.gain;
    autoShutter = other.autoShutter;
    shutterSpeed = other.shutterSpeed;

    whiteBalance = other.whiteBalance;
    redGain = other.redGain;
    blueGain = other.blueGain;

    led = other.led;

    device = other.device;

    return ( *this );
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool toUCam::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool toUCam::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"size",size);
      lti::write(handler,"brightness",brightness);
      lti::write(handler,"hue",hue);
      lti::write(handler,"saturation",saturation);
      lti::write(handler,"contrast",contrast);
      lti::write(handler,"gamma",gamma);
      lti::write(handler,"frameRate",frameRate);
      lti::write(handler,"snapshotMode",snapshotMode);
      switch (compression) {
          case None:
            lti::write(handler,"compression","None");
            break;
          case Low:
            lti::write(handler,"compression","Low");
            break;
          case Medium:
            lti::write(handler,"compression","Medium");
            break;
          case High:
            lti::write(handler,"compression","High");
            break;
          default:
            lti::write(handler,"compression","Medium");
      }
      lti::write(handler,"agc",agc);
      lti::write(handler,"gain",gain);
      lti::write(handler,"autoShutter",autoShutter);
      lti::write(handler,"shutterSpeed",shutterSpeed);

      switch (whiteBalance) {
          case Indoor:
            lti::write(handler,"whiteBalance","Indoor");
            break;
          case Outdoor:
            lti::write(handler,"whiteBalance","Outdoor");
            break;
          case Fluorescent:
            lti::write(handler,"whiteBalance","Fluorescent");
            break;
          case Manual:
            lti::write(handler,"whiteBalance","Manual");
            break;
          case Auto:
            lti::write(handler,"whiteBalance","Auto");
            break;
          default:
            lti::write(handler,"whiteBalance","Outdoor");
      };

      lti::write(handler,"redGain",redGain);
      lti::write(handler,"blueGain",blueGain);

      lti::write(handler,"led",led);
      lti::write(handler,"device",device);
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

# ifdef _LTI_MSC_6
  bool toUCam::parameters::write(ioHandler& handler,
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
# ifndef _LTI_MSC_6
  bool toUCam::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool toUCam::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      b=b && lti::read(handler,"size",size);
      b=b && lti::read(handler,"brightness",brightness);
      b=b && lti::read(handler,"hue",hue);
      b=b && lti::read(handler,"saturation",saturation);
      b=b && lti::read(handler,"contrast",contrast);
      b=b && lti::read(handler,"gamma",gamma);
      b=b && lti::read(handler,"frameRate",frameRate);
      b=b && lti::read(handler,"snapshotMode",snapshotMode);

      std::string tmp;
      b=b && lti::read(handler,"compression",tmp);
      if (tmp == "None") {
        compression = None;
      } else if (tmp == "Low") {
        compression = Low;
      } else if (tmp == "Medium") {
        compression = Medium;
      } else if (tmp == "High") {
        compression = High;
      } else {
        compression = Medium; // default value
        b=false;
      }

      b=b && lti::read(handler,"agc",agc);
      b=b && lti::read(handler,"gain",gain);
      b=b && lti::read(handler,"autoShutter",autoShutter);
      b=b && lti::read(handler,"shutterSpeed",shutterSpeed);

      b=b && lti::read(handler,"whiteBalance",tmp);
      if (tmp == "Indoor") {
        whiteBalance = Indoor;
      } else if (tmp == "Outdoor") {
        whiteBalance = Outdoor;
      } else if (tmp == "Fluorescent") {
        whiteBalance = Fluorescent;
      } else if (tmp == "Manual") {
        whiteBalance = Manual;
      } else if (tmp == "Auto") {
        whiteBalance = Auto;
      } else {
        whiteBalance = Outdoor;
        b=false;
      }

      b=b && lti::read(handler,"redGain",redGain);
      b=b && lti::read(handler,"blueGain",blueGain);

      b=b && lti::read(handler,"led",led);
      b=b && lti::read(handler,"device",device);
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
  bool toUCam::parameters::read(ioHandler& handler,
                                const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------

  // --------------------------------

  toUCam::toUCam(const bool initialize)
    : frameGrabber(),initialized(false),lastGrabbed(NotUsedYet),
      lastUsedParams(false) {

    initializeLUTs();

    // default parameters
    parameters param;
    frameGrabber::setParameters(param);

    if (initialize) {
      init();
    }
  }

  toUCam::toUCam(const std::string& device,const bool initialize)
    : frameGrabber(),initialized(false),lastGrabbed(NotUsedYet),
      lastUsedParams(false) {

    initializeLUTs();

    // default parameters
    parameters param;
    param.device = device;
    frameGrabber::setParameters(param);

    if (initialize) {
      init();
    }
  }

  toUCam::toUCam( const parameters& theParam )
    : frameGrabber(),initialized(false),lastGrabbed(NotUsedYet),
      lastUsedParams(false) {

    initializeLUTs();

    setParameters(theParam);
  }

  toUCam::toUCam(const toUCam& other)
    : frameGrabber(),initialized(false),lastGrabbed(NotUsedYet),
      lastUsedParams(false) {
    copy(other);
  }

  toUCam::~toUCam() {
    if (initialized) {
      close(cameraHndl);
      cameraHndl = -1;
      initialized = false;
    }
    _lti_debug4("toUCam::destructor called and camera freed" << std::endl);
  }

  const char* toUCam::getTypeName() const {
    return "toUCam";
  }

  toUCam& toUCam::copy(const toUCam& other) {
    frameGrabber::copy(other);

    return (*this);
  }

  functor* toUCam::clone() const {
    return new toUCam(*this);
  }

  const toUCam::parameters& toUCam::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if( params==0 )
      throw invalidParametersException(getTypeName());
    return *params;
  }

  /*
   * setParameters overload to allow changing the camera attributes
   */
  bool toUCam::setParameters(const functor::parameters& theParam) {
    // call method of parent class
    frameGrabber::setParameters(theParam);

    if (isInitialized()) {
      return setCameraParameters();
    } else {
      return init();
    }
  }

  // Initialize the camera with the functor parameters
  bool toUCam::init() {
    if (initialized) {
      // already initialized
      return true;
    }

    const parameters& param = getParameters();

    // initialize toUCam frame grabber:
    struct video_capability vcap;
    struct pwc_probe probe;
    int type;

    // check if the given name belongs to a valid device
    struct stat devstat;
    if ((stat(param.device.c_str(),&devstat)!=0) ||
        (S_ISLNK(devstat.st_mode))) {
      setStatusString("Problems with device ");
      appendStatusString(param.device.c_str());
      appendStatusString(": ");
      appendStatusString(strerror(errno));
      return false;
    }

    // first try to open the device
    cameraHndl = open(param.device.c_str(),O_RDWR);
    
    if (cameraHndl == -1) {
      // some error by opening device
      _lti_debug("Error number: " << errno << std::endl);
      setStatusString(strerror(errno));
      return false;
    }

    if (ioctl(cameraHndl,VIDIOCGCAP,&vcap) < 0) {      
      // some error getting camera id (it seems we cannot query the
      // camera capabilities)
      setStatusString(strerror(errno));
      close(cameraHndl);
      cameraHndl=-1;
      return false;
    }
    
    // get some usefull information about allowed image sizes
    minSize.set(vcap.minwidth,vcap.minheight);
    maxSize.set(vcap.maxwidth,vcap.maxheight);

    // check if we have a valid pwc supported camera
    vcap.name[31]='\0'; // ensure that the next comparisons will end!
    memset(&probe,0,sizeof(probe)); // clear all data for sure
    std::string str = vcap.name;
    if ((ioctl(cameraHndl,VIDIOCPWCPROBE, &probe) == 0) &&
        (str == probe.name)) {
      // everything is ok
      type = probe.type;
    } else {
      setStatusString("No Philips camera detected.");
      close(cameraHndl);
      cameraHndl=-1;
      return false;
    }

    // remember the camera name
    camName = vcap.name;

    _lti_debug4(camName << " detected." << endl);

    // restore factory defaults
    if (ioctl(cameraHndl,VIDIOCPWCFACTORY) < 0) {
      // some error getting camera id
      setStatusString(strerror(errno));
      close(cameraHndl);
      cameraHndl=-1;
      return false;
    }

    initialized = true;

    if (setCameraParameters()) {
      return true;
    } else {
      initialized = false;
      close(cameraHndl);
      cameraHndl=-1;
      return false;
    }
  }

  // Initialize the camera with the functor parameters
  bool toUCam::setCameraParameters() {
    if (!isInitialized()) {
      // already initialized
      return init();
    }

    const parameters& param = getParameters();

    // --------------------------------------------------
    // capture windows
    // --------------------------------------------------

    // get default data
    video_window videoWin;
    if (ioctl(cameraHndl,VIDIOCGWIN,&videoWin) < 0) {
      // some error getting camera id
      setStatusString(strerror(errno));
      return false;
    }

    // set new size
    if ((lastUsedParams.size != param.size) ||
        (lastUsedParams.frameRate != param.frameRate) ||
        (lastUsedParams.snapshotMode != param.snapshotMode)) {

      videoWin.width = min(max(minSize.x,param.size.x),maxSize.x);
      videoWin.height = min(max(minSize.y,param.size.y),maxSize.y);

      lastUsedParams.size = param.size;

      // set new frame rate
      videoWin.flags &= ~PWC_FPS_FRMASK;
      videoWin.flags |= ((param.frameRate & 0x3F) << PWC_FPS_SHIFT);

      lastUsedParams.frameRate = param.frameRate;


      // snapshot mode
      if (param.snapshotMode) {
        videoWin.flags |= PWC_FPS_SNAPSHOT;
      } else {
        videoWin.flags &= ~PWC_FPS_SNAPSHOT;
      }

      lastUsedParams.snapshotMode = param.snapshotMode;

      // set the data
      if (ioctl(cameraHndl,VIDIOCSWIN,&videoWin) < 0) {
	if (errno==-EINVAL) {
	  setStatusString("Invalid frame-rate/resolution configuration. " \
			  "Check your settings.");
	} else {
          // some error getting camera id
          setStatusString(strerror(errno));
	}
        return false;
      }

      // get the data again to check the size really been used
      if (ioctl(cameraHndl,VIDIOCGWIN,&videoWin) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }

      // member with the real size:
      size = point(videoWin.width,videoWin.height);

#     if defined(_LTI_DEBUG) && _LTI_DEBUG >= 4
      if (ioctl(cameraHndl,VIDIOCGWIN,&videoWin) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }
      _lti_debug(<< "Window properties after set:" << endl
           << "  x = " << videoWin.x << endl
           << "  y = " << videoWin.y << endl
           << "  width = " << videoWin.width << endl
           << "  height = " << videoWin.height << endl
           << "  chromakey = " << videoWin.chromakey << endl
           << "  framerate = "
           << ((videoWin.flags & PWC_FPS_FRMASK) >> PWC_FPS_SHIFT) << endl
           << "  snapshot = " << (videoWin.flags & PWC_FPS_SNAPSHOT) << endl);
#     endif
    }

    // --------------------------------------------------
    // image properties
    // --------------------------------------------------

    if ((lastUsedParams.brightness != param.brightness) ||
        (lastUsedParams.hue != param.hue) ||
        (lastUsedParams.saturation != param.saturation) ||
        (lastUsedParams.contrast != param.contrast) ||
        (lastUsedParams.gamma != param.gamma)) {

      video_picture videoPic;

      // get the values
      if (ioctl(cameraHndl,VIDIOCGPICT,&videoPic) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }

      videoPic.brightness = static_cast<int>(0.5f+param.brightness*65535.0f);
      videoPic.hue = static_cast<int>(0.5f+param.hue*65535.0f);
      videoPic.colour = static_cast<int>(0.5f+param.saturation*65535.0f);
      videoPic.contrast = static_cast<int>(0.5f+param.contrast*65535.0f);
      videoPic.whiteness = static_cast<int>(0.5f+param.gamma*65535.0f);
      videoPic.depth = 32;

      // the new kernel module accepts only this palette
      // videoPic.palette = VIDEO_PALETTE_RGB32;
      videoPic.palette = VIDEO_PALETTE_YUV420P;

      lastUsedParams.brightness = param.brightness;
      lastUsedParams.hue = param.hue;
      lastUsedParams.saturation = param.saturation;
      lastUsedParams.contrast = param.contrast;
      lastUsedParams.gamma = param.gamma;

      // set the new values
      if (ioctl(cameraHndl,VIDIOCSPICT,&videoPic) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }

      lastGrabbed = Color; // indicate that color images can be directly
                           // grabbed

#     if defined(_LTI_DEBUG) && _LTI_DEBUG >= 4
      if (ioctl(cameraHndl,VIDIOCGPICT,&videoPic) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }

      _lti_debug("Image properties after set:" << endl
           << "  brightness = " << videoPic.brightness << endl
           << "  hue = " << videoPic.hue << endl
           << "  colour = " << videoPic.colour << endl
           << "  contrast = " << videoPic.contrast << endl
           << "  whiteness = " << videoPic.whiteness << endl
           << "  depth = " << videoPic.depth << endl
           << "  palette = " << videoPic.palette << endl);
#     endif
    }

    // --------------------------------------------------
    // philips stuff (private calls)
    // --------------------------------------------------

    int tmpint;

    if (lastUsedParams.compression != param.compression) {
      // compression rate
      tmpint = int(param.compression);
      if (ioctl(cameraHndl,VIDIOCPWCSCQUAL,&tmpint) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }

      lastUsedParams.compression = param.compression;

#     if defined(_LTI_DEBUG) && _LTI_DEBUG >= 4
      tmpint = 0;
      if (ioctl(cameraHndl,VIDIOCPWCGCQUAL,&tmpint) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }
      _lti_debug("  compression:" << tmpint << endl);
#     endif
    }

    // automatic gain control
    if ((lastUsedParams.agc != param.agc) ||
        (lastUsedParams.gain != param.gain)) {

      if (param.agc) {
        tmpint = -int(0.5f+param.gain*65535);
      } else {
        tmpint = int(0.5f+param.gain*65535);
      }

      if (ioctl(cameraHndl,VIDIOCPWCSAGC,&tmpint) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }

      lastUsedParams.agc = param.agc;
      lastUsedParams.gain = param.gain;

#     if defined(_LTI_DEBUG) && _LTI_DEBUG >=4
      tmpint = 0;
      if (ioctl(cameraHndl,VIDIOCPWCGAGC,&tmpint) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }
      _lti_debug("  agc:" << tmpint << endl);
#     endif
    }

    // shutter speed
    if ((lastUsedParams.autoShutter != param.autoShutter) ||
        (lastUsedParams.shutterSpeed != param.shutterSpeed)) {
      if (param.autoShutter) {
        tmpint = -int(0.5f+param.shutterSpeed*65535);
      } else {
        tmpint = int(0.5f+param.shutterSpeed*65535);
      }

      if (ioctl(cameraHndl,VIDIOCPWCSSHUTTER,&tmpint) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }

      lastUsedParams.autoShutter = param.autoShutter;
      lastUsedParams.shutterSpeed = param.shutterSpeed;

      _lti_debug4("  shutter:" << tmpint << endl);
    }

    // auto white balance
    if ((lastUsedParams.whiteBalance != param.whiteBalance) ||
        (lastUsedParams.redGain != param.redGain) ||
        (lastUsedParams.blueGain != param.blueGain)) {

      pwc_whitebalance pwcwb;

      pwcwb.mode = static_cast<int>(param.whiteBalance);
      pwcwb.manual_red = static_cast<int>(0.5f+65535.0f*param.redGain);
      pwcwb.manual_blue = static_cast<int>(0.5f+65535.0f*param.blueGain);

      if (ioctl(cameraHndl,VIDIOCPWCSAWB,&pwcwb) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }

      lastUsedParams.whiteBalance = param.whiteBalance;
      lastUsedParams.redGain = param.redGain;
      lastUsedParams.blueGain = param.blueGain;

#     if defined(_LTI_DEBUG) && _LTI_DEBUG >=4
      if (ioctl(cameraHndl,VIDIOCPWCGAWB,&pwcwb) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }

      _lti_debug4("  awb:" << pwcwb.mode << endl);
      _lti_debug4("  red gain:" << pwcwb.read_red << endl);
      _lti_debug4("  blue gain:" << pwcwb.read_blue << endl);
#     endif
    }

    // led status
    if (lastUsedParams.led != param.led) {
      pwc_leds pwcLED;
      if (param.led <= 0) {
        // turn off the led
        pwcLED.led_on  = 0;
        pwcLED.led_off = 101;
      } else if (param.led == 1) {
        // turn on the led
        pwcLED.led_on  = 101;
        pwcLED.led_off = 0;
      } else {
        pwcLED.led_on  = param.led % 65536;
        pwcLED.led_off = param.led / 65536;
      }
      
      if (ioctl(cameraHndl,VIDIOCPWCSLED,&pwcLED) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }

      lastUsedParams.led = param.led;
      
#     if defined(_LTI_DEBUG) && _LTI_DEBUG >=4
      if (ioctl(cameraHndl,VIDIOCPWCGLED,&pwcLED) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }
      
      _lti_debug4("  led on:  " << pwcLED.led_on << endl);
      _lti_debug4("  led off: " << pwcLED.led_off << endl);

#     endif

    }

    return true;
  }

  // ----------------------------------------------------------------
  bool toUCam::isInitialized() const {
    return initialized;
  }

  // load a color image from the usb port
  bool toUCam::apply(image& theImage) {

    if (!isInitialized()) {
      // if not initialized yet, do it now!
      if (!init()) {
        theImage.clear();
        // the error message should still be there...
        return false;
      }
    }

    if (lastGrabbed != Color) {
      // camera must be activated in color mode

      video_picture videoPic;

      // get the old values
      if (ioctl(cameraHndl,VIDIOCGPICT,&videoPic) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }

      videoPic.depth = 32;
      // the new kernel module accepts only this palette
      // videoPic.palette = VIDEO_PALETTE_RGB32;
      videoPic.palette = VIDEO_PALETTE_YUV420P;

      // set the new values
      if (ioctl(cameraHndl,VIDIOCSPICT,&videoPic) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }

      lastGrabbed = Color; // indicate that color images can be directly
                           // grabbed
    }

    // file descriptor set
    fd_set fdSet;
    timeval tv;
    int retval;

    // Watch camera to see when it has input.
    FD_ZERO(&fdSet);
    FD_SET(cameraHndl, &fdSet);

    /* Wait up to two seconds. */
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    retval = select(cameraHndl+1, &fdSet, NULL, NULL, &tv);

    // Don't rely on the value of tv now!

    if (retval) {
      int res;

      int bufferSize = size.x*size.y*3/2;
      theImage.resize(size,rgbPixel(),false,false);


      ubyte* buffer = new ubyte[bufferSize];

      // read image in Yuv 4:2:0 format (the only format available)
      res = ::read(cameraHndl,(void*)buffer,bufferSize);

      if (res<0) {
        setStatusString(strerror(errno));
        return false;
      }

      ubyte* pxptr = reinterpret_cast<ubyte*>(&theImage.at(0,0));
      pxptr += (size.x*size.y*4);

      ubyte *Y = buffer;
      ubyte *u = &buffer[size.x*size.y];
      ubyte *v = &buffer[5*size.x*size.y/4];
      ubyte *Yend;

      if (getParameters().turnaround) {
        ubyte* pxptr = reinterpret_cast<ubyte*>(&theImage.at(0,0));
        pxptr += (size.x*size.y*4);

        int yy,sy;

        const int usize = size.x/2;

        for (sy=0;sy<size.y;++sy) {
          Yend = Y+size.x;
          while (Y != Yend) {
            yy = (*Y)<<8;

            // dummy
            --pxptr;
            // red
            --pxptr;
            (*pxptr) = lutSaturation[((yy+lutVr[*v]) >> 8)&0x3FF];
            // green
            --pxptr;
            (*pxptr) = lutSaturation[((yy-lutUg[*u]-lutVg[*v]) >> 8)&0x3FF];
            // blue
            --pxptr;
            (*pxptr) = lutSaturation[((yy+lutUb[*u]) >> 8) & 0x3FF];


            // next pixel doesn't change uv values
            ++Y;
            yy = (*Y)<<8;

            // dummy
            --pxptr;
            // red
            --pxptr;
            (*pxptr) = lutSaturation[((yy+lutVr[*v]) >> 8)&0x3FF];
            // green
            --pxptr;
            (*pxptr) = lutSaturation[((yy-lutUg[*u]-lutVg[*v]) >> 8)&0x3FF];
            // blue
            --pxptr;
            (*pxptr) = lutSaturation[((yy+lutUb[*u]) >> 8)&0x3FF];

            ++Y;
            ++u;
            ++v;
          }
          if ((sy & 1) == 0) {
            u-=usize;
            v-=usize;
          }
        }

      } else {
        ubyte* pxptr = reinterpret_cast<ubyte*>(&theImage.at(0,0));

        Yend = u;
        int yy,sy;

        const int usize = size.x/2;

        for (sy=0;sy<size.y;++sy) {
          Yend = Y+size.x;
          while (Y != Yend) {
            yy = (*Y)<<8;

            // blue
            (*pxptr) = lutSaturation[((yy+lutUb[*u])>>8)&0x3FF];
            pxptr++;
            // green
            (*pxptr) = lutSaturation[((yy-lutUg[*u]-lutVg[*v])>>8)&0x3FF];
            pxptr++;
            // red
            (*pxptr) = lutSaturation[((yy+lutVr[*v])>>8)&0x3FF];
            pxptr++;
            // dummy
            pxptr++;

            // next pixel doesn't change uv values
            ++Y;
            yy = (*Y)<<8;

            // blue
            (*pxptr) = lutSaturation[((yy+lutUb[*u])>>8)&0x3FF];
            pxptr++;
            // green
            (*pxptr) = lutSaturation[((yy-lutUg[*u]-lutVg[*v])>>8)&0x3FF];
            pxptr++;
            // red
            (*pxptr) = lutSaturation[((yy+lutVr[*v])>>8)&0x3FF];
            pxptr++;
            // dummy
            pxptr++;

            ++Y;
            ++u;
            ++v;
          }
          if ((sy & 1) == 0) {
            u-=usize;
            v-=usize;
          }
        }
      }

      delete[] buffer;

    } else {
      theImage.clear();
      setStatusString("Timeout.  No image received");
      return false;
    }

    FD_ZERO(&fdSet);

    return true;
  }

  // load a grey value channel from the
  bool toUCam::apply(channel8& theChannel) {
    if (!isInitialized()) {
      // if not initialized yet, do it now!
      if (!init()) {
        theChannel.clear();
        // the error message should still be there...
        return false;
      }
    }

    if (lastGrabbed != Gray) {
      // camera must be activated in color mode
      video_picture videoPic;

      // get the old values
      if (ioctl(cameraHndl,VIDIOCGPICT,&videoPic) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }

      videoPic.depth = 32;
      videoPic.palette = VIDEO_PALETTE_YUV420P;

      // set the new values
      if (ioctl(cameraHndl,VIDIOCSPICT,&videoPic) < 0) {
        // some error getting camera id
        setStatusString(strerror(errno));
        return false;
      }

      lastGrabbed = Gray; // indicate that color images can be directly
                           // grabbed
    }

    // file descriptor set
    fd_set fdSet;
    timeval tv;
    int retval;

    // Watch camera to see when it has input.
    FD_ZERO(&fdSet);
    FD_SET(cameraHndl, &fdSet);

    /* Wait up to two seconds. */
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    retval = select(cameraHndl+1, &fdSet, NULL, NULL, &tv);

    // Don't rely on the value of tv now!

    if (retval) {
      int res;

      // read the Y channel
      theChannel.resize(size,ubyte(),false,false);
      res = ::read(cameraHndl,(void*)&theChannel.at(0,0),size.x*size.y);
      if (res<0) {
        setStatusString(strerror(errno));
        return false;
      }
      // read the rest of color information
      int bufferSize = size.x*size.y/2;
      ubyte* buffer = new ubyte[bufferSize];
      res = ::read(cameraHndl,(void*)buffer,bufferSize);
      delete[] buffer;

      if (res<0) {
        setStatusString(strerror(errno));
        return false;
      }
    } else {
      theChannel.clear();
      setStatusString("Timeout.  No image received");
      return false;
    }

    FD_ZERO(&fdSet);

    return true;
  }

  void toUCam::initializeLUTs() {
    if (lutSaturation == 0) {
      int i;
      /*
       * look up table to accelerate computation of saturation
       */
      static ubyte tmpSat[1024];
      for (i=-256;i<512;++i) {
        tmpSat[i&0x3FF] = (i<0) ? 0 : (i>255) ? 255 : i;
      }
      lutSaturation = tmpSat;

      /*
       * look up tables to accelerate conversion Luv -> RGB
       */
      //       int* lutUgTmp = new int[256];
      //       int* lutUbTmp = new int[256];
      //       int* lutVgTmp = new int[256];
      //       int* lutVrTmp = new int[256];

      static int lutUgTmp[256];
      static int lutUbTmp[256];
      static int lutVgTmp[256];
      static int lutVrTmp[256];      

      for (i=0;i<256;++i) {
        lutUgTmp[i] = (i-128)*88;
        lutUbTmp[i] = (i-128)*454;
        lutVgTmp[i] = (i-128)*183;
        lutVrTmp[i] = (i-128)*359;
      }

      lutUg = lutUgTmp;
      lutUb = lutUbTmp;
      lutVg = lutVgTmp;
      lutVr = lutVrTmp;

    }


  }

} // namespace lti

#endif // __linux__
#endif // _USE_PHILIPS_TOUCAM
