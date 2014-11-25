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
 * file .......: ltiStatisticsFunctor.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 25.9.2000
 * revisions ..: $Id: ltiStatisticsFunctor.cpp,v 1.6 2006/02/08 12:46:50 ltilib Exp $
 */

#include "ltiStatisticsFunctor.h"

namespace lti {
  // --------------------------------------------------
  // statisticsFunctor::parameters
  // --------------------------------------------------

  // default constructor
  statisticsFunctor::parameters::parameters()
    : functor::parameters() {
  }

  // copy constructor
  statisticsFunctor::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  statisticsFunctor::parameters::~parameters() {
  }

  // get type name
  const char* statisticsFunctor::parameters::getTypeName() const {
    return "statisticsFunctor::parameters";
  }

  // copy member

  statisticsFunctor::parameters&
    statisticsFunctor::parameters::copy(const parameters& other) {
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
  functor::parameters* statisticsFunctor::parameters::clone() const {
    return new parameters(*this);
  }

  // --------------------------------------------------
  // statisticsFunctor
  // --------------------------------------------------

  // default constructor
  statisticsFunctor::statisticsFunctor()
    : functor(){
  }

  // copy constructor
  statisticsFunctor::statisticsFunctor(const statisticsFunctor& other)
    : functor()  {
    copy(other);
  }

  // destructor
  statisticsFunctor::~statisticsFunctor() {
  }

  // returns the name of this type
  const char* statisticsFunctor::getTypeName() const {
    return "statisticsFunctor";
  }

  // copy member
  statisticsFunctor&
    statisticsFunctor::copy(const statisticsFunctor& other) {
      functor::copy(other);
    return (*this);
  }

  // clone member
  functor* statisticsFunctor::clone() const {
    return new statisticsFunctor(*this);
  }

  // return parameters
  const statisticsFunctor::parameters&
    statisticsFunctor::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }


}
