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
 * file .......: ltiMorphology.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 19.7.2000
 * revisions ..: $Id: ltiMorphology.cpp,v 1.6 2006/02/08 11:31:47 ltilib Exp $
 */

#include "ltiMorphology.h"

namespace lti {
  // --------------------------------------------------
  // morphology::parameters
  // --------------------------------------------------

  // default constructor
  morphology::parameters::parameters()
    : modifier::parameters() {

  }

  // copy constructor
  morphology::parameters::parameters(const parameters& other)
    : modifier::parameters()  {
    copy(other);
  }

  // destructor
  morphology::parameters::~parameters() {
  }

  // get type name
  const char* morphology::parameters::getTypeName() const {
    return "morphology::parameters";
  }

  // copy member

  morphology::parameters&
    morphology::parameters::copy(const parameters& other) {
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

    return *this;
  }

  // clone member
  functor::parameters* morphology::parameters::clone() const {
    return new parameters(*this);
  }

  // --------------------------------------------------
  // morphology
  // --------------------------------------------------

  // default constructor
  morphology::morphology()
    : modifier(){
  }

  // copy constructor
  morphology::morphology(const morphology& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  morphology::~morphology() {
  }

  // returns the name of this type
  const char* morphology::getTypeName() const {
    return "morphology";
  }

  // copy member
  morphology& morphology::copy(const morphology& other) {
    modifier::copy(other);
    return (*this);
  }

  // return parameters
  const morphology::parameters& morphology::getParameters() const {
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
