/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiGlobalFeatureExtractor.cpp
 * authors ....: Pablo Alvarado, Arnd Hannemann
 * organization: LTI, RWTH Aachen
 * creation ...: 9.8.2000
 * revisions ..: $Id: ltiGlobalFeatureExtractor.cpp,v 1.4 2006/02/07 18:59:49 ltilib Exp $
 */

#include "ltiMacroSymbols.h"
#include "ltiGlobalFeatureExtractor.h"

namespace lti {
  // --------------------------------------------------
  // globalFeatureExtractor::parameters
  // --------------------------------------------------

  // default constructor
  globalFeatureExtractor::parameters::parameters()
#ifndef _LTI_MSC_6
    : featureExtractor::parameters() 
#endif
  {

  }

  // copy constructor
  globalFeatureExtractor::parameters::parameters(const parameters& other)
#ifndef _LTI_MSC_6
    : featureExtractor::parameters()  
#endif 
  {
    copy(other);
  }

  // destructor
  globalFeatureExtractor::parameters::~parameters() {
  }

  // get type name
  const char* globalFeatureExtractor::parameters::getTypeName() const {
    return "globalFeatureExtractor::parameters";
  }

  // copy member

  globalFeatureExtractor::parameters&
    globalFeatureExtractor::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    featureExtractor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    featureExtractor::parameters& (featureExtractor::parameters::* p_copy)
      (const featureExtractor::parameters&) =
      featureExtractor::parameters::copy;
    (this->*p_copy)(other);
# endif



    return *this;
  }

  // clone member
  functor::parameters* globalFeatureExtractor::parameters::clone() const {
    return new parameters(*this);
  }

  // --------------------------------------------------
  // globalFeatureExtractor
  // --------------------------------------------------

  // default constructor
  globalFeatureExtractor::globalFeatureExtractor()
    : featureExtractor(){
  }

  // copy constructor
  globalFeatureExtractor::globalFeatureExtractor(const globalFeatureExtractor& other)
    : featureExtractor()  {
    copy(other);
  }

  // destructor
  globalFeatureExtractor::~globalFeatureExtractor() {
  }

  // returns the name of this type
  const char* globalFeatureExtractor::getTypeName() const {
    return "globalFeatureExtractor";
  }

  // copy member
  globalFeatureExtractor&
    globalFeatureExtractor::copy(const globalFeatureExtractor& other) {
      featureExtractor::copy(other);
    return (*this);
  }

  // clone member
  functor* globalFeatureExtractor::clone() const {
    return new globalFeatureExtractor(*this);
  }

  // return parameters
  const globalFeatureExtractor::parameters&
    globalFeatureExtractor::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&featureExtractor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On copy apply for type channel8!
  bool globalFeatureExtractor::apply(const channel& src,dvector& dest) const {
    setStatusString("Method not implemented");
    dest.clear();
    return false;
  };


  // On copy apply for type channel8!
  bool globalFeatureExtractor::apply(const channel8& src,dvector& dest) const {
    setStatusString("Method not implemented");
    dest.clear();
    return false;
  };

  // On copy apply for type image!
  bool globalFeatureExtractor::apply(const image& src,dvector& dest) const {
    setStatusString("Method not implemented");
    dest.clear();
    return false;
  };

}
