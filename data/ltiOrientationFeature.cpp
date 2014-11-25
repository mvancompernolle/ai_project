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
 * file .......: ltiOrientationFeature.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 28.5.2001
 * revisions ..: $Id: ltiOrientationFeature.cpp,v 1.9 2006/09/05 10:24:13 ltilib Exp $
 */

#include "ltiOrientationFeature.h"

namespace lti {
  // --------------------------------------------------
  // orientationFeature::parameters
  // --------------------------------------------------

  // default constructor
  orientationFeature::parameters::parameters()
    : globalFeatureExtractor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    size = int(36);
    normalize = bool(true);
  }

  // copy constructor
  orientationFeature::parameters::parameters(const parameters& other)
    : globalFeatureExtractor::parameters()  {
    copy(other);
  }

  // destructor
  orientationFeature::parameters::~parameters() {
  }

  // get type name
  const char* orientationFeature::parameters::getTypeName() const {
    return "orientationFeature::parameters";
  }

  // copy member

  orientationFeature::parameters&
    orientationFeature::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    globalFeatureExtractor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    globalFeatureExtractor::parameters& (globalFeatureExtractor::parameters::* p_copy)
      (const globalFeatureExtractor::parameters&) =
      globalFeatureExtractor::parameters::copy;
    (this->*p_copy)(other);
# endif


    size = other.size;
    normalize = other.normalize;

    return *this;
  }

  // alias for copy member
  orientationFeature::parameters&
    orientationFeature::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* orientationFeature::parameters::clone() const {
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
  bool orientationFeature::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool orientationFeature::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"size",size);
      lti::write(handler,"normalize",normalize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::write(handler,false);
# else
    bool (globalFeatureExtractor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      globalFeatureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool orientationFeature::parameters::write(ioHandler& handler,
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
  bool orientationFeature::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool orientationFeature::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"size",size);
      lti::read(handler,"normalize",normalize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::read(handler,false);
# else
    bool (globalFeatureExtractor::parameters::* p_readMS)(ioHandler&,const bool) =
      globalFeatureExtractor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool orientationFeature::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // orientationFeature
  // --------------------------------------------------

  // default constructor
  orientationFeature::orientationFeature()
    : globalFeatureExtractor(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  orientationFeature::orientationFeature(const orientationFeature& other)
    : globalFeatureExtractor()  {
    copy(other);
  }

  // destructor
  orientationFeature::~orientationFeature() {
  }

  // returns the name of this type
  const char* orientationFeature::getTypeName() const {
    return "orientationFeature";
  }

  // copy member
  orientationFeature&
    orientationFeature::copy(const orientationFeature& other) {
      globalFeatureExtractor::copy(other);
    return (*this);
  }

  // clone member
  functor* orientationFeature::clone() const {
    return new orientationFeature(*this);
  }

  // return parameters
  const orientationFeature::parameters&
    orientationFeature::getParameters() const {
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


  // On copy apply for type channel!
  bool orientationFeature::apply(const channel& orientation,
                                 const channel& magnitude,
                                 dvector& dest) const {

    if (magnitude.empty() || orientation.empty()) {
      dest.clear();
      setStatusString("input data empty");
      return false;
    }

    if (magnitude.size() != orientation.size()) {
      dest.clear();
      setStatusString("magnitude and orientation must have the same size");
      return false;
    }

    const parameters& param = getParameters();

    int y;
    vector<channel::value_type>::const_iterator mit,mite,oit;
    const float cst = float(param.size/Pi);
    int idx;

    // initialize histogram with 0
    dest.resize(param.size,0.0,false,true);

    for (y=0;y<magnitude.rows();++y) {

      const vector<channel::value_type>& magv = magnitude.getRow(y);
      const vector<channel::value_type>& oriv = orientation.getRow(y);

      for (mit=magv.begin(),mite=magv.end(),oit=oriv.begin();
           mit!=mite;
           ++mit,++oit) {
        idx = int((*oit)*cst) % param.size;
        if (idx < 0) {
          idx += param.size;
        }

        dest.at(idx)+=(*mit);
      }
    }

    if (param.normalize) {
      dest.divide(dest.sumOfElements());
    }

    return true;
  };
}
