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
 * file .......: ltiValidator.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 5.12.2001
 * revisions ..: $Id: ltiValidator.cpp,v 1.12 2006/09/05 10:41:49 ltilib Exp $
 */

#include "ltiConfig.h"
#include "ltiObject.h"


#ifndef _LTI_MSC_6
#define _GNU_SOURCE 1
#include <cfloat>
#include "ltiMath.h"
#else
#include <cfloat>
#endif

#include "ltiValidator.h"

namespace lti {
  // --------------------------------------------------
  // validator::parameters
  // --------------------------------------------------

  // default constructor
  validator::parameters::parameters()
    : functor::parameters() {

    isDoubleValid=&defaultValidateDouble;
    isFloatValid=&defaultValidateFloat;
  }

  // copy constructor
  validator::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  validator::parameters::~parameters() {
  }

  // get type name
  const char* validator::parameters::getTypeName() const {
    return "validator::parameters";
  }

  // copy member

  validator::parameters&
    validator::parameters::copy(const parameters& other) {
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

    isDoubleValid=other.isDoubleValid;
    isFloatValid=other.isFloatValid;

    return *this;
  }

  // alias for copy member
  validator::parameters&
    validator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* validator::parameters::clone() const {
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
  bool validator::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool validator::parameters::writeMS(ioHandler& handler,
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
  bool validator::parameters::write(ioHandler& handler,
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
  bool validator::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool validator::parameters::readMS(ioHandler& handler,
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
  bool validator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  bool validator::parameters::defaultValidateDouble(double x) {
    //return !((isnan(x) != 0) || (isinf(x) != 0));
# ifndef _LTI_GNUC_3    
    return (finite(x) != 0);
# else
    return (isfinite(x) != 0);
# endif    
  }

  bool validator::parameters::defaultValidateFloat(float x) {
    //return !((isnan(x) != 0) || (isinf(x) != 0));
# ifndef _LTI_GNUC_3
    return (finite(x) != 0);
# else
    return (isfinite(x) != 0);
# endif    
  }


  // --------------------------------------------------
  // validator
  // --------------------------------------------------

  // default constructor
  validator::validator()
    : functor(){

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // copy constructor
  validator::validator(const validator& other)
    : functor()  {
    copy(other);
  }

  // destructor
  validator::~validator() {
  }

  // returns the name of this type
  const char* validator::getTypeName() const {
    return "validator";
  }

  // copy member
  validator& validator::copy(const validator& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  validator& validator::operator=(const validator& other) {
    return (copy(other));
  }

  // clone member
  functor* validator::clone() const {
    return new validator(*this);
  }

  // return parameters
  const validator::parameters&
    validator::getParameters() const {
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


  // On place apply for type vector<double>!
  bool validator::apply(const vector<double>& src) const {
    bool valid=true;

    doubleValidator& isvalid=*getParameters().isDoubleValid;

    vector<double>::const_iterator it=src.begin();
    const vector<double>::const_iterator itend=src.end();
    while (it != itend && valid) {
      valid=(valid && isvalid(*it++));
    }

    return valid;
  };

  // On place apply for type vector<float>!
  bool validator::apply(const vector<float>& src) const {
    bool valid=true;

    floatValidator& isvalid=*getParameters().isFloatValid;

    vector<float>::const_iterator it=src.begin();
    const vector<float>::const_iterator itend=src.end();
    while (it != itend && valid) {
      valid=(valid && isvalid(*it++));
    }

    return valid;
  };

  // On place apply for type matrix<double>!
  bool validator::apply(const matrix<double>& src) const {
    bool valid=true;

    doubleValidator& isvalid=*getParameters().isDoubleValid;

    matrix<double>::const_iterator it=src.begin();
    const matrix<double>::const_iterator itend=src.end();
    while (it != itend && valid) {
      valid=(valid && isvalid(*it++));
    }

    return valid;
  };

  // On place apply for type matrix<float>!
  bool validator::apply(const matrix<float>& src) const {
    bool valid=true;

    floatValidator& isvalid=*getParameters().isFloatValid;

    matrix<float>::const_iterator it=src.begin();
    const matrix<float>::const_iterator itend=src.end();
    while (it != itend && valid) {
      valid=(valid && isvalid(*it++));
    }

    return valid;
  };


}
