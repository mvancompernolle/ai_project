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
 * file .......: ltiDecimation.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 6.12.2000
 * revisions ..: $Id: ltiDecimation.cpp,v 1.8 2006/09/05 10:08:56 ltilib Exp $
 */

#include "ltiDecimation.h"

namespace lti {
  // --------------------------------------------------
  // decimation::parameters
  // --------------------------------------------------

  // default constructor
  decimation::parameters::parameters()
    : filter::parameters() {
    factor = point(2,2);
  }

  // copy constructor
  decimation::parameters::parameters(const parameters& other)
    : filter::parameters()  {
    copy(other);
  }

  // destructor
  decimation::parameters::~parameters() {
  }

  // get type name
  const char* decimation::parameters::getTypeName() const {
    return "decimation::parameters";
  }

  // copy member

  decimation::parameters&
    decimation::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    filter::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    filter::parameters& (filter::parameters::* p_copy)
      (const filter::parameters&) =
      filter::parameters::copy;
    (this->*p_copy)(other);
# endif

    factor = other.factor;

    return *this;
  }

  // alias for copy member
  decimation::parameters&
    decimation::parameters::operator=(const parameters& other) {
    return copy(other);
  }


  // clone member
  functor::parameters* decimation::parameters::clone() const {
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
  bool decimation::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool decimation::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"factor",factor);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && filter::parameters::write(handler,false);
# else
    bool (modifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      modifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool decimation::parameters::write(ioHandler& handler,
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
  bool decimation::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool decimation::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"factor",factor);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && filter::parameters::read(handler,false);
# else
    bool (modifier::parameters::* p_readMS)(ioHandler&,const bool) =
      modifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool decimation::parameters::read(ioHandler& handler,
                                    const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // decimation
  // --------------------------------------------------

  // default constructor
  decimation::decimation()
    : filter(){
    parameters par;
    setParameters(par);
  }

  // default constructor
  decimation::decimation(const point& factor)
    : filter(){
    parameters par;
    par.factor = factor;
    setParameters(par);
  }


  // copy constructor
  decimation::decimation(const decimation& other)
    : filter()  {
    copy(other);
  }

  // destructor
  decimation::~decimation() {
  }

  // returns the name of this type
  const char* decimation::getTypeName() const {
    return "decimation";
  }

  // copy member
  decimation& decimation::copy(const decimation& other) {
    filter::copy(other);
    return (*this);
  }

  // clone member
  functor* decimation::clone() const {
    return new decimation(*this);
  }

  // return parameters
  const decimation::parameters&
    decimation::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }


}
