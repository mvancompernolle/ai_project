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
 * file .......: ltiPolygonApproximation.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 16.11.2002
 * revisions ..: $Id: ltiPolygonApproximation.cpp,v 1.9 2006/09/05 10:25:45 ltilib Exp $
 */

#include "ltiPolygonApproximation.h"

namespace lti {
  // --------------------------------------------------
  // polygonApproximation::parameters
  // --------------------------------------------------

  // default constructor
  polygonApproximation::parameters::parameters()
    : modifier::parameters() {

    minStep = int(-1);
    searchMaxDistance = bool(false);
    maxDistance = double(1);
    closed = bool(true);
  }

  // copy constructor
  polygonApproximation::parameters::parameters(const parameters& other)
    : modifier::parameters()  {
    copy(other);
  }

  // destructor
  polygonApproximation::parameters::~parameters() {
  }

  // get type name
  const char* polygonApproximation::parameters::getTypeName() const {
    return "polygonApproximation::parameters";
  }

  // copy member

  polygonApproximation::parameters&
    polygonApproximation::parameters::copy(const parameters& other) {
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


      minStep = other.minStep;
      searchMaxDistance = other.searchMaxDistance;
      maxDistance = other.maxDistance;
      closed = other.closed;

    return *this;
  }

  // alias for copy member
  polygonApproximation::parameters&
    polygonApproximation::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* polygonApproximation::parameters::clone() const {
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
  bool polygonApproximation::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool polygonApproximation::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"minStep",minStep);
      lti::write(handler,"searchMaxDistance",searchMaxDistance);
      lti::write(handler,"maxDistance",maxDistance);
      lti::write(handler,"closed",closed);
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
  bool polygonApproximation::parameters::write(ioHandler& handler,
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
  bool polygonApproximation::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool polygonApproximation::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"minStep",minStep);
      lti::read(handler,"searchMaxDistance",searchMaxDistance);
      lti::read(handler,"maxDistance",maxDistance);
      lti::read(handler,"closed",closed);
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
  bool polygonApproximation::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // polygonApproximation
  // --------------------------------------------------

  // default constructor
  polygonApproximation::polygonApproximation()
    : modifier(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  polygonApproximation::polygonApproximation(const parameters& par)
    : modifier() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  polygonApproximation::polygonApproximation(const polygonApproximation& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  polygonApproximation::~polygonApproximation() {
  }

  // returns the name of this type
  const char* polygonApproximation::getTypeName() const {
    return "polygonApproximation";
  }

  // copy member
  polygonApproximation&
  polygonApproximation::copy(const polygonApproximation& other) {
    modifier::copy(other);
    return (*this);
  }

  // alias for copy member
  polygonApproximation&
  polygonApproximation::operator=(const polygonApproximation& other) {
    return (copy(other));
  }


  // clone member
  functor* polygonApproximation::clone() const {
    return new polygonApproximation(*this);
  }

  // return parameters
  const polygonApproximation::parameters&
  polygonApproximation::getParameters() const {
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


  // On copy apply for type pointList!
  bool polygonApproximation::apply(const borderPoints& src,
                                         polygonPoints& dest) const {

    const parameters& p = getParameters();
    dest.approximate(src,p.minStep,p.maxDistance,p.closed,p.searchMaxDistance);

    return true;
  };


  // On copy apply for type pointList!
  bool polygonApproximation::apply(const borderPoints& src,
                                   const pointList& forcedVertices,
                                         polygonPoints& dest) const {

    const parameters& p = getParameters();
    dest.approximate(src,forcedVertices,
                     p.minStep,p.maxDistance,p.closed,p.searchMaxDistance);

    return true;
  };


}
