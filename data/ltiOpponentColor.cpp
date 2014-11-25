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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiOpponentColor.cpp
 * authors ....: Axel Berner
 * organization: LTI, RWTH Aachen
 * creation ...: 7.8.2001
 * revisions ..: $Id: ltiOpponentColor.cpp,v 1.8 2006/09/05 10:23:50 ltilib Exp $
 */

#include "ltiOpponentColor.h"
#include "ltiGaussianPyramid.h"


namespace lti {
  // --------------------------------------------------
  // opponentColor::parameters
  // --------------------------------------------------

  // default constructor
  opponentColor::parameters::parameters()
    : transform::parameters() {

    centerLevel = 0;
    centerSign = -1;
    surroundLevel = 1;
    surroundSign = +1;
    outFunc = parameters::Abs;
  }

  // copy constructor
  opponentColor::parameters::parameters(const parameters& other)
    : transform::parameters()  {
    copy(other);
  }

  // destructor
  opponentColor::parameters::~parameters() {
  }

  // get type name
  const char* opponentColor::parameters::getTypeName() const {
    return "opponentColor::parameters";
  }

  // copy member

  opponentColor::parameters&
    opponentColor::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    transform::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    transform::parameters& (transform::parameters::* p_copy)
      (const transform::parameters&) =
      transform::parameters::copy;
    (this->*p_copy)(other);
# endif

    centerLevel   = other.centerLevel;
    centerSign    = other.centerSign;
    surroundLevel = other.surroundLevel;
    surroundSign  = other.surroundSign;
    outFunc       = other.outFunc;

    return *this;
  }

  // alias for copy member
  opponentColor::parameters&
    opponentColor::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* opponentColor::parameters::clone() const {
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
  bool opponentColor::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool opponentColor::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
       lti::write(handler,"centerLevel"  ,centerLevel);
       lti::write(handler,"centerSign"   ,centerSign);
       lti::write(handler,"surroundLevel",surroundLevel);
       lti::write(handler,"surroundSign" ,surroundSign);
       std::string s;
       switch(outFunc) {
	 case parameters::Normal: s = "Normal"; break;
	 case parameters::Abs   : s = "Abs";    break;
	 case parameters::Ramp  : s = "Ramp";   break;
       };
       lti::write(handler,"outFunc"      ,s);
     }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::write(handler,false);
# else
    bool (transform::parameters::* p_writeMS)(ioHandler&,const bool) const =
      transform::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool opponentColor::parameters::write(ioHandler& handler,
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
  bool opponentColor::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool opponentColor::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"centerLevel"  ,centerLevel);
      lti::read(handler,"centerSign"   ,centerSign);
      lti::read(handler,"surroundLevel",surroundLevel);
      lti::read(handler,"surroundSign" ,surroundSign);
      std::string s;
      lti::read(handler,"outFunc"      ,s);
      if     (s == "Normal")
	outFunc = parameters::Normal;
      else if(s == "Abs")
	outFunc = parameters::Abs;
      else if(s == "Ramp")
	outFunc = parameters::Ramp;
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::read(handler,false);
# else
    bool (transform::parameters::* p_readMS)(ioHandler&,const bool) =
      transform::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool opponentColor::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // opponentColor
  // --------------------------------------------------

  // default constructor
  opponentColor::opponentColor()
    : transform(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  opponentColor::opponentColor(const opponentColor& other)
    : transform()  {
    copy(other);
  }

  // destructor
  opponentColor::~opponentColor() {
  }

  // returns the name of this type
  const char* opponentColor::getTypeName() const {
    return "opponentColor";
  }

  // copy member
  opponentColor&
    opponentColor::copy(const opponentColor& other) {
      transform::copy(other);
    return (*this);
  }

  // clone member
  functor* opponentColor::clone() const {
    return new opponentColor(*this);
  }

  // return parameters
  const opponentColor::parameters&
    opponentColor::getParameters() const {
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

  bool opponentColor::apply(const lti::channel& centerSrc,
			    const lti::channel& surroundSrc,
			    lti::channel& dest) const {
  // set parameters
    const lti::opponentColor::parameters& param = getParameters();
    return apply(centerSrc, surroundSrc,
		 param.centerLevel,   param.centerSign,
		 param.surroundLevel, param.surroundSign,
		 param.outFunc,
                 dest);
  }

  bool opponentColor::apply(const lti::channel& centerSrc,
			    const lti::channel& surroundSrc,
			    const int& centerLevel,
                            const float& centerSign,
			    const int& surroundLevel,
                            const float& surroundSign,
			    const parameters::eFunctionType& outFunc,
                            lti::channel& dest) const {

    // check parameters
    if(centerSrc.size() != surroundSrc.size()) {
      setStatusString("Center and Surround must have the same size");
      return false;
    }

    if(centerLevel > surroundLevel) {
      setStatusString("centerLevel must be lower than surroundLevel");
      return false;
    }

    // generate needed pyramids with needed depth
    lti::gaussianPyramid<lti::channel> centerPyra;
    centerPyra.generate(centerSrc,centerLevel+1);
    lti::gaussianPyramid<lti::channel> surroundPyra;
    surroundPyra.generate(surroundSrc,surroundLevel+1);

    // sample surroundColorImage up to size from centerColorImage
    lti::channel tempUp;
    surroundPyra.reconstruct(centerLevel,surroundLevel,tempUp);

    lti::channel& tempCenter = centerPyra.at(centerLevel);

    // calculate "differences"
    if(centerSign == +1 && surroundSign == -1) // for faster computation
      dest.subtract(tempCenter,tempUp);
    else if(surroundSign == +1 && centerSign == -1) // for faster computation
      dest.subtract(tempUp,tempCenter);
    else
      dest.addScaled(surroundSign,tempUp,centerSign,tempCenter);

    // handle with the the negative values
    switch (outFunc) {
      case parameters::Normal:
	// nothing
	break;
      case parameters::Abs:
	dest.apply(&abs); // absolute value
	break;
      case parameters::Ramp:
	dest.apply(&rectify);// negative values to zero
	break;
    }
    return true;
  }

}
