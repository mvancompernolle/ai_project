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
 * file .......: ltiVariablySpacedSamplesInterpolator.cpp
 * authors ....: Peter Doerfler, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 3.5.2003
 * revisions ..: $Id: ltiVariablySpacedSamplesInterpolator.cpp,v 1.8 2006/09/05 10:42:00 ltilib Exp $
 */

#include "ltiVariablySpacedSamplesInterpolator.h"

namespace lti {
  // --------------------------------------------------
  // variablySpacedSamplesInterpolator::parameters
  // --------------------------------------------------

  // default constructor
  variablySpacedSamplesInterpolator::parameters::parameters()
    : interpolator::parameters() {
    
  }

  // copy constructor
  variablySpacedSamplesInterpolator::parameters::parameters(const parameters& other)
    : interpolator::parameters() {
    copy(other);
  }

  // destructor
  variablySpacedSamplesInterpolator::parameters::~parameters() {
  }

  // get type name
  const char* variablySpacedSamplesInterpolator::parameters::getTypeName() const {
    return "variablySpacedSamplesInterpolator::parameters";
  }

  // copy member

  variablySpacedSamplesInterpolator::parameters&
    variablySpacedSamplesInterpolator::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    interpolator::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    interpolator::parameters& (interpolator::parameters::* p_copy)
      (const interpolator::parameters&) =
      interpolator::parameters::copy;
    (this->*p_copy)(other);
# endif

    

    return *this;
  }

  // alias for copy member
  variablySpacedSamplesInterpolator::parameters&
    variablySpacedSamplesInterpolator::parameters::operator=(const parameters& other) {
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
  bool variablySpacedSamplesInterpolator::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool variablySpacedSamplesInterpolator::parameters::writeMS(ioHandler& handler,
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
    b = b && interpolator::parameters::write(handler,false);
# else
    bool (interpolator::parameters::* p_writeMS)(ioHandler&,const bool) const =
      interpolator::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool variablySpacedSamplesInterpolator::parameters::write(ioHandler& handler,
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
  bool variablySpacedSamplesInterpolator::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool variablySpacedSamplesInterpolator::parameters::readMS(ioHandler& handler,
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
    b = b && interpolator::parameters::read(handler,false);
# else
    bool (interpolator::parameters::* p_readMS)(ioHandler&,const bool) =
      interpolator::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool variablySpacedSamplesInterpolator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // variablySpacedSamplesInterpolator
  // --------------------------------------------------

  // default constructor
  variablySpacedSamplesInterpolator::variablySpacedSamplesInterpolator()
    : interpolator(){
  }

  // copy constructor
  variablySpacedSamplesInterpolator::variablySpacedSamplesInterpolator(const variablySpacedSamplesInterpolator& other)
    : interpolator() {
    copy(other);
  }

  // destructor
  variablySpacedSamplesInterpolator::~variablySpacedSamplesInterpolator() {
  }

  // returns the name of this type
  const char* variablySpacedSamplesInterpolator::getTypeName() const {
    return "variablySpacedSamplesInterpolator";
  }

  // copy member
  variablySpacedSamplesInterpolator& variablySpacedSamplesInterpolator::
  copy(const variablySpacedSamplesInterpolator& other) {
    interpolator::copy(other);
    
    return (*this);
  }

  // alias for copy member
  variablySpacedSamplesInterpolator&
  variablySpacedSamplesInterpolator::operator=(const variablySpacedSamplesInterpolator& other) {
    return (copy(other));
  }


  // return parameters
  const variablySpacedSamplesInterpolator::parameters&
    variablySpacedSamplesInterpolator::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

}
