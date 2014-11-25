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
 * file .......: ltiPanTiltUnit.cpp
 * authors ....: Claudia Goenner, Christian Hein
 * organization: LTI, RWTH Aachen
 * creation ...: 28.8.2003
 * revisions ..: $Id: ltiPanTiltUnit.cpp,v 1.10 2006/09/05 10:37:31 ltilib Exp $
 */

#include "ltiPanTiltUnit.h"
#include "ltiMath.h"
#include "ltiConstants.h"

namespace lti {
  // --------------------------------------------------
  // panTiltUnit::parameters
  // --------------------------------------------------

  // default constructor
  panTiltUnit::parameters::parameters()
    :
#ifndef _LTI_MSC_6
      functor::parameters(),
#endif
    angleFormat(Radiant),
    pan(0.f),
    tilt(0.f),
    relativeMovement(false),
		minPan(minPan),
	  maxPan(maxPan),
	  minTilt(minTilt),
	  maxTilt(maxTilt) {
  }

  // copy constructor
	  panTiltUnit::parameters::parameters(const parameters& other):
    minPan(other.minPan),
	  maxPan(other.maxPan),
	  minTilt(other.minTilt),
	  maxTilt(other.maxTilt)
  {
    copy(other);
  }

  // constructor
  panTiltUnit::parameters::parameters(float minPan,float maxPan, float minTilt, float maxTilt):
  #ifndef _LTI_MSC_6
      functor::parameters(),
	#endif
    angleFormat(Radiant),
		pan(0.f),
    tilt(0.f),
    relativeMovement(false),
	  minPan(minPan),
	  maxPan(maxPan),
	  minTilt(minTilt),
	  maxTilt(maxTilt) {
  }

  // destructor
  panTiltUnit::parameters::~parameters() {
  }

  // get type name
  const char* panTiltUnit::parameters::getTypeName() const {
    return "panTiltUnit::parameters";
  }

  panTiltUnit::parameters&
    panTiltUnit::parameters::copy(const parameters& other) {
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

    angleFormat = other.angleFormat;
    pan = other.pan;
    tilt = other.tilt;
    relativeMovement = other.relativeMovement;
    
    return *this;
  }

  // alias for copy member
  panTiltUnit::parameters&
    panTiltUnit::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* panTiltUnit::parameters::clone() const {
    return new parameters(*this);
  }

  /**
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool panTiltUnit::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool panTiltUnit::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      switch (angleFormat) {
      case Degrees:
	lti::write(handler,"angleFormat","Degrees");
	break;
      default:
	lti::write(handler,"angleFormat","Radiant");
	break;
      }
      lti::write(handler,"pan",pan);
      lti::write(handler,"tilt",tilt);
      lti::write(handler,"relativeMovement", relativeMovement);
      lti::write(handler,"minPan", minPan);
      lti::write(handler,"maxPan", maxPan);
      lti::write(handler,"minTilt", minTilt);
      lti::write(handler,"maxTilt", maxTilt);
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
  bool panTiltUnit::parameters::write(ioHandler& handler,
                                         const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /**
   * read the parameters from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool panTiltUnit::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool panTiltUnit::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {  
      std::string str;
      float dummy;
      lti::read(handler,"angleFormat",str);
      if (str == "Degrees")
	angleFormat = Degrees;
      else
	angleFormat = Radiant;  //default
      lti::read(handler,"pan", pan);
      lti::read(handler,"tilt", tilt);
      lti::read(handler,"relativeMovement", relativeMovement);
      lti::read(handler,"minPan", dummy);
      lti::read(handler,"maxPan", dummy);
      lti::read(handler,"minTilt", dummy);
      lti::read(handler,"maxTilt", dummy);
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
  bool panTiltUnit::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // panTiltUnit
  // --------------------------------------------------

  // default constructor
  panTiltUnit::panTiltUnit()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  panTiltUnit::panTiltUnit(const parameters& par)
    : functor() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  panTiltUnit::panTiltUnit(const panTiltUnit& other)
    : functor() {
    copy(other);
  }

  // destructor
  panTiltUnit::~panTiltUnit() {
  }

  // returns the name of this type
  const char* panTiltUnit::getTypeName() const {
    return "panTiltUnit";
  }

  // copy member
  panTiltUnit&
  panTiltUnit::copy(const panTiltUnit& other) {
      functor::copy(other);

    return (*this);
  }

  // alias for copy member
  panTiltUnit&
  panTiltUnit::operator=(const panTiltUnit& other) {
    return (copy(other));
  }

  // return parameters
  const panTiltUnit::parameters&
  panTiltUnit::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // --------------------------------------------------
  // panTiltUnit
  // --------------------------------------------------

  // Get the MaxPan and MaxTilt position.
  bool panTiltUnit::getMaxPanTilt(float& maxPan, float& maxTilt) const {
    const parameters& par = getParameters();
    maxPan = float(par.maxPan);
    maxTilt = float(par.maxTilt);
    if (par.angleFormat == parameters::Degrees) {
      maxPan *= 180.f / constants<float>::Pi();
      maxTilt *=180.f / constants<float>::Pi();
    }
    return true;
  }
 
 // Get the MinPan and MinTilt position.
  bool panTiltUnit::getMinPanTilt(float& minPan, float& minTilt) const {
    const parameters& par = getParameters();
    minPan = float(par.minPan);
    minTilt = float(par.minTilt);
    if (par.angleFormat == parameters::Degrees) {
      minPan *= 180.f / constants<float>::Pi();
      minTilt *=180.f / constants<float>::Pi();
    }
    return true;
  }
  
   float panTiltUnit::getMinPan() const {
    const parameters& par = getParameters();
  
    if (par.angleFormat == parameters::Degrees) {
      return static_cast<float>(par.minPan) / constants<float>::Pi() * 180.f;

    } else {
      return static_cast<float>(par.minPan);
    }
  }
  
  
  float panTiltUnit::getMaxPan() const {
    const parameters& par = getParameters();
  
    if (par.angleFormat == parameters::Degrees) {
      return static_cast<float>(par.maxPan) / constants<float>::Pi() * 180.f;

    } else {
      return static_cast<float>(par.maxPan);
    }
  }
  
  
  float panTiltUnit::getMinTilt() const {
    const parameters& par = getParameters();

    if (par.angleFormat == parameters::Degrees) {
      return static_cast<float>(par.minTilt) / constants<float>::Pi() * 180.f;
    } else {
      return static_cast<float>(par.minTilt);
    }
  }
  
  
  float panTiltUnit::getMaxTilt() const {
    const parameters& par = getParameters();

    if (par.angleFormat == parameters::Degrees) {
      return static_cast<float>(par.maxTilt) / constants<float>::Pi() * 180.f;
    } else {
      return static_cast<float>(par.maxTilt);
    }
  }

}
