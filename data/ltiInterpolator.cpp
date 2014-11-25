/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiInterpolator.cpp
 * authors ....: Peter Doerfler, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 3.5.2003
 * revisions ..: $Id: ltiInterpolator.cpp,v 1.10 2006/09/05 10:40:18 ltilib Exp $
 */

#include "ltiInterpolator.h"

namespace lti {
  // --------------------------------------------------
  // interpolator::parameters
  // --------------------------------------------------

  // default constructor
  interpolator::parameters::parameters()
    : functor::parameters() {

    boundaryType = lti::Zero;
    
  }

  // constructor
  interpolator::parameters::parameters(const eBoundaryType btype)
    : functor::parameters() {

    boundaryType = btype;
    
  }

  // copy constructor
  interpolator::parameters::parameters(const parameters& other)
    : functor::parameters() {
    copy(other);
  }

  // destructor
  interpolator::parameters::~parameters() {
  }

  // get type name
  const char* interpolator::parameters::getTypeName() const {
    return "interpolator::parameters";
  }

  // copy member

  interpolator::parameters&
    interpolator::parameters::copy(const parameters& other) {
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

    boundaryType = other.boundaryType;

    return *this;
  }

  // alias for copy member
  interpolator::parameters&
    interpolator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool interpolator::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool interpolator::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
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

    if (b) {      
      lti::write(handler,"boundaryType",boundaryType);
    }

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool interpolator::parameters::write(ioHandler& handler,
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
  bool interpolator::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool interpolator::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
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

    if (b) {      
      lti::read(handler,"boundaryType",boundaryType);
    }

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool interpolator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // interpolator
  // --------------------------------------------------

  // default constructor
  interpolator::interpolator()
    : functor(){

  }

  // copy constructor
  interpolator::interpolator(const interpolator& other)
    : functor() {
    copy(other);
  }

  // destructor
  interpolator::~interpolator() {
  }

  // returns the name of this type
  const char* interpolator::getTypeName() const {
    return "interpolator";
  }

  // copy member
  interpolator&
  interpolator::copy(const interpolator& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  interpolator&
  interpolator::operator=(const interpolator& other) {
    return (copy(other));
  }

  // return parameters
  const interpolator::parameters&
  interpolator::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool 
  interpolator::setBoundaryType(const eBoundaryType& bt) {

    parameters* par = dynamic_cast<parameters*>(&functor::getParameters());
    if (notNull(par)) {
      par->boundaryType=bt;
      return true;
    }

    return false;
  }
}
