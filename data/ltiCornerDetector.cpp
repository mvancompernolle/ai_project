/*
 * Copyright (C) 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiCornerDetector.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 29.10.2002
 * revisions ..: $Id: ltiCornerDetector.cpp,v 1.8 2006/09/05 10:07:50 ltilib Exp $
 */

#include "ltiCornerDetector.h"

namespace lti {
  // --------------------------------------------------
  // cornerDetector::parameters
  // --------------------------------------------------

  // default constructor
  cornerDetector::parameters::parameters()
    : modifier::parameters() {

    cornerValue = 255;
    noCornerValue = 0;
  }

  // copy constructor
  cornerDetector::parameters::parameters(const parameters& other)
    : modifier::parameters()  {
    copy(other);
  }

  // destructor
  cornerDetector::parameters::~parameters() {
  }

  // get type name
  const char* cornerDetector::parameters::getTypeName() const {
    return "cornerDetector::parameters";
  }

  // copy member

  cornerDetector::parameters&
    cornerDetector::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    modifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    modifier::parameters& (modifier::parameters::* p_copy)
      (const modifier::parameters&) =
      modifier::parameters::copy;
    (this->*p_copy)(other);
# endif

    cornerValue = other.cornerValue;
    noCornerValue = other.noCornerValue;

    return *this;
  }

  // alias for copy member
  cornerDetector::parameters&
    cornerDetector::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* cornerDetector::parameters::clone() const {
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
  bool cornerDetector::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool cornerDetector::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::write(handler,false);
# else
    bool (modifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      modifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    lti::write(handler,"cornerValue",cornerValue);
    lti::write(handler,"noCornerValue",noCornerValue);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool cornerDetector::parameters::write(ioHandler& handler,
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
  bool cornerDetector::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool cornerDetector::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::read(handler,false);
# else
    bool (modifier::parameters::* p_readMS)(ioHandler&,const bool) =
      modifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    lti::read(handler,"cornerValue",cornerValue);
    lti::read(handler,"noCornerValue",noCornerValue);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool cornerDetector::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // cornerDetector
  // --------------------------------------------------

  // default constructor
  cornerDetector::cornerDetector()
    : modifier(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  cornerDetector::cornerDetector(const cornerDetector& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  cornerDetector::~cornerDetector() {
  }

  // returns the name of this type
  const char* cornerDetector::getTypeName() const {
    return "cornerDetector";
  }

  // copy member
  cornerDetector&
    cornerDetector::copy(const cornerDetector& other) {
      modifier::copy(other);

    return (*this);
  }

  // alias for copy member
  cornerDetector&
    cornerDetector::operator=(const cornerDetector& other) {
    return (copy(other));
  }

  // return parameters
  const cornerDetector::parameters&
    cornerDetector::getParameters() const {
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


  // On place apply for type channel8!
  bool cornerDetector::apply(channel8& srcdest) const {

    channel8 tmp;
    if (apply(srcdest,tmp)) {
      // can we just use the result as new image?
      if (srcdest.getMode() == channel8::Connected) {
        // yes: just transfer the data
        tmp.detach(srcdest);
      } else {
        // no: srcdest is part of another bigger image, just copy
        // the data.
        srcdest.fill(tmp);
      }
      return true;
    }

    return false;
  };

  // On place apply for type channel!
  bool cornerDetector::apply(channel& srcdest) const {
    channel tmp;
    if (apply(srcdest,tmp)) {
      // can we just use the result as new image?
      if (srcdest.getMode() == channel::Connected) {
        // yes: just transfer the data
        tmp.detach(srcdest);
      } else {
        // no: srcdest is part of another bigger image, just copy
        // the data.
        srcdest.fill(tmp);
      }
      return true;
    }

    return false;
  };

  /*
   * operates on a copy of the given %parameters.
   * @param src channel8 with the source data.
   * @param dest channel8 where the result will be left.
   * @return true if apply successful or false otherwise.
   */
  bool cornerDetector::apply(const channel8& src,channel8& dest) const {

    const parameters& par = getParameters();
    pointList cornerList;
    pointList::const_iterator it;

    if (apply(src,cornerList)) {

      dest.resize(src.size(),par.noCornerValue,false,true);
      for (it=cornerList.begin();it!=cornerList.end();++it) {
        dest.at(*it)=par.cornerValue;
      }

      return true;
    }

    return false;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src channel with the source data.
   * @param dest channel where the result will be left.
   * @return true if apply successful or false otherwise.
   */
  bool cornerDetector::apply(const channel& src,channel& dest) const {
    const parameters& par = getParameters();
    pointList cornerList;
    pointList::const_iterator it;

    channel8 in;
    in.castFrom(src);

    if (apply(in,cornerList)) {
      dest.resize(src.size(),float(par.noCornerValue)/255.0f,false,true);
      const float on = float(par.cornerValue)/255.0f;
      for (it=cornerList.begin();it!=cornerList.end();++it) {
        dest.at(*it)=on;
      }

      return true;
    }

    return false;
  }


  /*
   * operates on a copy of the given %parameters.
   * @param src channel8 with the source data.
   * @param dest list of corners
   * @return true if apply successful or false otherwise.
   */
//   bool cornerDetector::apply(const channel8& src,pointList& dest) const {
//     setStatusString("void cornerDetector::apply-method called");
//     return false;
//   }

  /*
   * operates on a copy of the given %parameters.
   * @param src channel with the source data.
   * @param dest list of corners
   * @return true if apply successful or false otherwise.
   */
//   bool cornerDetector::apply(const channel& src,pointList& dest) const {
//     setStatusString("void cornerDetector::apply-method called");
//     return false;
//   }


}
