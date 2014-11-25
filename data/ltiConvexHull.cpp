/*
 * Copyright (C) 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiConvexHull.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 16.11.2002
 * revisions ..: $Id: ltiConvexHull.cpp,v 1.8 2006/09/05 10:07:27 ltilib Exp $
 */

#include "ltiConvexHull.h"

namespace lti {
  // --------------------------------------------------
  // convexHull::parameters
  // --------------------------------------------------

  // default constructor
  convexHull::parameters::parameters()
    : modifier::parameters() {
  }

  // copy constructor
  convexHull::parameters::parameters(const parameters& other)
    : modifier::parameters()  {
    copy(other);
  }

  // destructor
  convexHull::parameters::~parameters() {
  }

  // get type name
  const char* convexHull::parameters::getTypeName() const {
    return "convexHull::parameters";
  }

  // copy member

  convexHull::parameters&
    convexHull::parameters::copy(const parameters& other) {
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

  // alias for copy member
  convexHull::parameters&
    convexHull::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* convexHull::parameters::clone() const {
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
  bool convexHull::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool convexHull::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::write(handler,false);
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
  bool convexHull::parameters::write(ioHandler& handler,
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
  bool convexHull::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool convexHull::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::read(handler,false);
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
  bool convexHull::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // convexHull
  // --------------------------------------------------

  // default constructor
  convexHull::convexHull()
    : modifier(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  convexHull::convexHull(const parameters& par)
    : modifier() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  convexHull::convexHull(const convexHull& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  convexHull::~convexHull() {
  }

  // returns the name of this type
  const char* convexHull::getTypeName() const {
    return "convexHull";
  }

  // copy member
  convexHull& convexHull::copy(const convexHull& other) {
    modifier::copy(other);
    
    return (*this);
  }

  // alias for copy member
  convexHull&
    convexHull::operator=(const convexHull& other) {
    return (copy(other));
  }


  // clone member
  functor* convexHull::clone() const {
    return new convexHull(*this);
  }

  // return parameters
  const convexHull::parameters&
    convexHull::getParameters() const {
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

  /*
   * operates on a copy of the given %parameters.
   * @param src pointList with the source data.
   * @param dest pointList where the result will be left.
   * @return true if apply successful or false otherwise.
   */
  bool convexHull::apply(const pointList& src,polygonPoints& dest) const {
    dest.convexHullFrom(src);
    return true;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src pointList with the source data.
   * @param dest pointList where the result will be left.
   * @return true if apply successful or false otherwise.
   */
  bool convexHull::apply(const ioPoints& src,polygonPoints& dest) const {
    dest.castFrom(src);
    return true;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src pointList with the source data.
   * @param dest pointList where the result will be left.
   * @return true if apply successful or false otherwise.
   */
  bool convexHull::apply(const tpointList<float>& src,
                         tpolygonPoints<float>& dest) const {
    dest.convexHullFrom(src);
    return true;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src pointList with the source data.
   * @param dest pointList where the result will be left.
   * @return true if apply successful or false otherwise.
   */
  bool convexHull::apply(const tpointList<double>& src,
                         tpolygonPoints<double>& dest) const {
    dest.convexHullFrom(src);
    return true;
  }

}
