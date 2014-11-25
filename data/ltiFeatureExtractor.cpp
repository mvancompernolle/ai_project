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
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiFeatureExtractor.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 9.8.2000
 * revisions ..: $Id: ltiFeatureExtractor.cpp,v 1.7 2006/02/08 11:07:36 ltilib Exp $
 */

#include "ltiFeatureExtractor.h"

namespace lti {
  // --------------------------------------------------
  // featureExtractor::parameters
  // --------------------------------------------------

  // default constructor
  featureExtractor::parameters::parameters()
    : functor::parameters() {

  }

  // copy constructor
  featureExtractor::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  featureExtractor::parameters::~parameters() {
  }

  // get type name
  const char* featureExtractor::parameters::getTypeName() const {
    return "featureExtractor::parameters";
  }

  // copy member

  featureExtractor::parameters&
    featureExtractor::parameters::copy(const parameters& other) {
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

  // clone member
  functor::parameters* featureExtractor::parameters::clone() const {
    return new parameters(*this);
  }

  // --------------------------------------------------
  // featureExtractor
  // --------------------------------------------------

  // default constructor
  featureExtractor::featureExtractor()
    : functor(){
  }

  // copy constructor
  featureExtractor::featureExtractor(const featureExtractor& other)
    : functor()  {
    copy(other);
  }

  // destructor
  featureExtractor::~featureExtractor() {
  }

  // returns the name of this type
  const char* featureExtractor::getTypeName() const {
    return "featureExtractor";
  }

  // copy member
  featureExtractor&
    featureExtractor::copy(const featureExtractor& other) {
      functor::copy(other);
    return (*this);
  }

  // clone member
  functor* featureExtractor::clone() const {
    return new featureExtractor(*this);
  }

  // return parameters
  const featureExtractor::parameters&
    featureExtractor::getParameters() const {
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
