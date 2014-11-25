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
 * file .......: ltiCamera.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 28.8.2003
 * revisions ..: $Id: ltiCamera.cpp,v 1.7 2006/09/05 10:34:33 ltilib Exp $
 */

#include "ltiCamera.h"

namespace lti {
  // --------------------------------------------------
  // camera::parameters
  // --------------------------------------------------
  //ToDo??: init. const values like this
  //const float camera::parameters::maxValue1 = 3.f;

  // default constructor
  camera::parameters::parameters() 
    : 
#ifndef _LTI_MSC_6     
      functor::parameters(),
#endif
      autoWhiteBalance(true),
      autoGain(true),
      gain(maxGain),
      redGain(maxGain),
      blueGain(maxGain),
      autoShutter(true),
      shutterSpeed(minShutterSpeed),
      autoFocus(true),
      focus(maxFocus),
      zoom(0.f) {
  }

  // copy constructor
  camera::parameters::parameters(const parameters& other)
#ifndef _LTI_MSC_6
    : functor::parameters()
#endif
  {
    copy(other);
  }

  // destructor
  camera::parameters::~parameters() {
  }

  // get type name
  const char* camera::parameters::getTypeName() const {
    return "camera::parameters";
  }

  // copy member

  camera::parameters&
    camera::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters& (functor::parameters::* p_copy)
      (const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
# endif

    autoWhiteBalance = other.autoWhiteBalance;
    autoGain = other.autoGain;
    gain = other.gain;
    redGain = other.redGain;
    blueGain = other.blueGain;
    autoShutter = other.autoShutter;
    shutterSpeed = other.shutterSpeed;
    autoFocus = other.autoFocus;
    focus = other.focus;
    zoom = other.zoom;

    return *this;
  }

  // alias for copy member
  camera::parameters&
    camera::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* camera::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool camera::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool camera::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"autoWhiteBalance",autoWhiteBalance);
      lti::write(handler,"autoGain",autoGain);
      lti::write(handler,"gain",gain);
      lti::write(handler,"redGain",redGain);
      lti::write(handler,"blueGain",blueGain);
      lti::write(handler,"minGain",minGain);
      lti::write(handler,"maxGain",maxGain);
      lti::write(handler,"minRBGain",minRBGain);
      lti::write(handler,"maxRBGain",maxRBGain);
      lti::write(handler,"autoShutter",autoShutter);
      lti::write(handler,"shutterSpeed",shutterSpeed);
      lti::write(handler,"minShutterSpeed",minShutterSpeed);
      lti::write(handler,"maxShutterSpeed",maxShutterSpeed);
      lti::write(handler,"autoFocus",autoFocus);
      lti::write(handler,"focus",focus);
      lti::write(handler,"minFocus",minFocus);
      lti::write(handler,"maxFocus",maxFocus);
      lti::write(handler,"zoom",zoom);
      lti::write(handler,"maxZoom",maxZoom);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::write(handler,false);
# else
    bool (functor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      functor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool camera::parameters::write(ioHandler& handler,
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
  bool camera::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool camera::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      float dummy;
      lti::write(handler,"autoWhiteBalance",autoWhiteBalance);
      lti::write(handler,"autoGain",autoGain);
      lti::write(handler,"gain",gain);
      lti::write(handler,"redGain",redGain);
      lti::write(handler,"blueGain",blueGain);
      lti::write(handler,"minGain",dummy);
      lti::write(handler,"maxGain",dummy);
      lti::write(handler,"minRBGain",dummy);
      lti::write(handler,"maxRBGain",dummy);
      lti::write(handler,"autoShutter",autoShutter);
      lti::write(handler,"shutterSpeed",shutterSpeed);
      lti::write(handler,"minShutterSpeed",dummy);
      lti::write(handler,"maxShutterSpeed",dummy);
      lti::write(handler,"autoFocus",autoFocus);
      lti::write(handler,"focus",focus);
      lti::write(handler,"minFocus",dummy);
      lti::write(handler,"maxFocus",dummy);
      lti::write(handler,"zoom",zoom);
      lti::write(handler,"maxZoom",dummy);   
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::read(handler,false);
# else
    bool (functor::parameters::* p_readMS)(ioHandler&,const bool) =
      functor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool camera::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // camera
  // --------------------------------------------------

  // default constructor
  camera::camera()
    : functor() {

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  camera::camera(const parameters& par)
    : functor() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  camera::camera(const camera& other)
    : functor() {
    copy(other);
  }

  // destructor
  camera::~camera() {
  }

  // returns the name of this type
  const char* camera::getTypeName() const {
    return "camera";
  }

  // copy member
  camera& camera::copy(const camera& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  camera&
    camera::operator=(const camera& other) {
    return (copy(other));
  }

  // return parameters
  const camera::parameters&
    camera::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  


}
