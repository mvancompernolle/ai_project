/*
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006
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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiTransform.cpp
 * authors ....: Stefan Syberichs
 * organization: LTI, RWTH Aachen
 * creation ...: 06.12.99
 * revisions ..: $Id: ltiTransform.cpp,v 1.6 2006/02/08 11:57:04 ltilib Exp $
 */


#include "ltiTransform.h"
#include "ltiException.h"

namespace lti {
  //--- transform

  // base class for all transform classes is transform

  functor::parameters* transform::parameters::clone() const {
    return (new parameters(*this));
  }

  const char* transform::parameters::getTypeName() const {
    return "transform::parameters";
  }

  transform::parameters& transform::parameters::copy(const parameters& other) {
#ifndef _LTI_MSC_6
    // for normal  ANSI C++
    functor::parameters::copy(other);
#else
    // this doesn't work with MS-VC++ 6.0 (an once again... another
    // FUCKIN' MICROSOFT BUG)
    // ...so we have to use this workaround.
    // Due to that, copy may not be virtual.
    functor::parameters&
      (functor::parameters::* p_copy)(const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
#endif

    return (*this);
  }

  // construction
  transform::transform(const parameters& theParam) {
    setParameters(theParam);
  }

  // returns the name of this type
  const char* transform::getTypeName() const {
    return "transform";
  }


  // returns the current parameters
  const transform::parameters& transform::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0)
      throw invalidParametersException(getTypeName());
    return *params;
  }
} // namespace lti
