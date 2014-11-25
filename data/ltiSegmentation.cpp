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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiSegmentation.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 24.08.99
 * revisions ..: $Id: ltiSegmentation.cpp,v 1.7 2006/09/05 10:30:29 ltilib Exp $
 */

#include "ltiSegmentation.h"

namespace lti {
  segmentation::parameters::parameters()
    : functor::parameters() {
  }

  segmentation::parameters::~parameters() {
  }

  segmentation::parameters&
    segmentation::parameters::copy(const parameters& other) {

#ifndef _LTI_MSC_6
    // for normal  ANSI C++
    functor::parameters::copy(other);
#else
    // this doesn't work with MS-VC++ 6.0 (once again... another bug)
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters&
      (functor::parameters::* p_copy)(const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
#endif

    return (*this);
  }

  const char* segmentation::parameters::getTypeName() const {
    return "segmentation::parameters";
  }

  functor::parameters* segmentation::parameters::clone() const {
    segmentation::parameters* newParam = new segmentation::parameters;
    newParam->copy(*this);
    return newParam;
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool segmentation::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool segmentation::parameters::writeMS(ioHandler& handler,
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

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool segmentation::parameters::write(ioHandler& handler,
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
  bool segmentation::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool segmentation::parameters::readMS(ioHandler& handler,
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

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool segmentation::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // destructor
  segmentation::~segmentation() {
  }

  // copy data of "other" functor.
  segmentation& segmentation::copy(const segmentation& other) {
    functor::copy(other);
    return (*this);
  }

  functor* segmentation::clone() const {
    segmentation* newSeg = new segmentation;
    newSeg->copy(*this);
    return (newSeg);
  }

  const char* segmentation::getTypeName() const {
    return "segmentation";
  }

} // namespace lti
