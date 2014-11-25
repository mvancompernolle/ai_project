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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiColorNormalization.cpp
 * authors ....: Suat Akyol, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 30.03.2003
 * revisions ..: $Id: ltiColorNormalizationBase.cpp,v 1.9 2006/09/05 10:06:47 ltilib Exp $
 */

#include "ltiColorNormalizationBase.h"

namespace lti {
  // --------------------------------------------------
  // colorNormalizationBase::parameters
  // --------------------------------------------------

  // default constructor
  colorNormalizationBase::parameters::parameters()
    : modifier::parameters() {
  }

  // copy constructor
  colorNormalizationBase::parameters::parameters(const parameters& other)
    : modifier::parameters() {
    copy(other);
  }

  // destructor
  colorNormalizationBase::parameters::~parameters() {
  }

  // get type name
  const char* colorNormalizationBase::parameters::getTypeName() const {
    return "colorNormalizationBase::parameters";
  }

  // copy member

  colorNormalizationBase::parameters&
    colorNormalizationBase::parameters::copy(const parameters& other) {
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
  functor::parameters* colorNormalizationBase::parameters::clone() const {
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
  bool colorNormalizationBase::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool colorNormalizationBase::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::write(handler,complete);
# else
    bool (modifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      modifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,complete);
# endif

    return b;
  }

# ifdef _LTI_MSC_6
  bool colorNormalizationBase::parameters::write(ioHandler& handler,
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
  bool colorNormalizationBase::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool colorNormalizationBase::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::read(handler,complete);
# else
    bool (modifier::parameters::* p_readMS)(ioHandler&,const bool) =
      modifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,complete);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }
    
    return b;
  }

# ifdef _LTI_MSC_6
  bool colorNormalizationBase::parameters::read(ioHandler& handler,
                                    const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // colorNormalizationBase
  // --------------------------------------------------

  // default constructor
  colorNormalizationBase::colorNormalizationBase()
    : modifier() {
  }

  // copy constructor
  colorNormalizationBase::colorNormalizationBase(const colorNormalizationBase& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  colorNormalizationBase::~colorNormalizationBase() {
  }

  // returns the name of this type
  const char* colorNormalizationBase::getTypeName() const {
    return "colorNormalizationBase";
  }

  // copy member
  colorNormalizationBase&
  colorNormalizationBase::copy(const colorNormalizationBase& other) {
    modifier::copy(other);
    return (*this);
  }
  
  // return parameters
  const colorNormalizationBase::parameters&
    colorNormalizationBase::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

}
