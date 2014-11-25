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
 * file .......: ltiShapeFeatureExtractor.cpp
 * authors ....: Ruediger Weiler
 * organization: LTI, RWTH Aachen
 * creation ...: 7.6.2001
 * revisions ..: $Id: ltiShapeFeatureExtractor.cpp,v 1.8 2006/09/05 10:30:50 ltilib Exp $
 */


//TODO: include files
#include "ltiShapeFeatureExtractor.h"

namespace lti {
  // --------------------------------------------------
  // shapeFeatureExtractor::parameters
  // --------------------------------------------------

  // default constructor
  shapeFeatureExtractor::parameters::parameters()
    : functor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

  }

  // copy constructor
  shapeFeatureExtractor::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  shapeFeatureExtractor::parameters::~parameters() {
  }

  // get type name
  const char* shapeFeatureExtractor::parameters::getTypeName() const {
    return "shapeFeatureExtractor::parameters";
  }

  // copy member

  shapeFeatureExtractor::parameters&
    shapeFeatureExtractor::parameters::copy(const parameters& other) {
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



    return *this;
  }

  // alias for copy member
  shapeFeatureExtractor::parameters&
    shapeFeatureExtractor::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* shapeFeatureExtractor::parameters::clone() const {
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
  bool shapeFeatureExtractor::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool shapeFeatureExtractor::parameters::writeMS(ioHandler& handler,
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
  bool shapeFeatureExtractor::parameters::write(ioHandler& handler,
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
  bool shapeFeatureExtractor::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool shapeFeatureExtractor::parameters::readMS(ioHandler& handler,
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
  bool shapeFeatureExtractor::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // shapeFeatureExtractor
  // --------------------------------------------------

  // default constructor
  shapeFeatureExtractor::shapeFeatureExtractor()
    : functor(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  shapeFeatureExtractor::shapeFeatureExtractor(const shapeFeatureExtractor& other)
    : functor()  {
    copy(other);
  }

  // destructor
  shapeFeatureExtractor::~shapeFeatureExtractor() {
  }

  // returns the name of this type
  const char* shapeFeatureExtractor::getTypeName() const {
    return "shapeFeatureExtractor";
  }

  // copy member
  shapeFeatureExtractor&
    shapeFeatureExtractor::copy(const shapeFeatureExtractor& other) {
      functor::copy(other);
    return (*this);
  }

  // clone member
  functor* shapeFeatureExtractor::clone() const {
    return new shapeFeatureExtractor(*this);
  }

  // return parameters
  const shapeFeatureExtractor::parameters&
    shapeFeatureExtractor::getParameters() const {
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

  bool shapeFeatureExtractor::apply(const pointList& src, dvector& feature) const {

    return true;
  }

}
