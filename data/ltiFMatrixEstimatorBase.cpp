/*
 * Copyright (C) 2004, 2005, 2006
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
 * file .......: ltiFMatrixEstimatorBase.cpp
 * authors ....: Stephanus Hendradji
 * organization: LTI, RWTH Aachen
 * creation ...: 15.7.2004
 * revisions ..: $Id: ltiFMatrixEstimatorBase.cpp,v 1.6 2006/09/05 10:11:08 ltilib Exp $
 */

#include "ltiFMatrixEstimatorBase.h"

namespace lti {
  // --------------------------------------------------
  // fMatrixEstimatorBase::parameters
  // --------------------------------------------------

  // default constructor
  fMatrixEstimatorBase::parameters::parameters()
    : transformEstimator::parameters() {

    rank2Enforcement = Frobenius;
    distanceMeasure = Epipolar;
  }

  // copy constructor
  fMatrixEstimatorBase::parameters::parameters(const parameters& other)
    : transformEstimator::parameters() {
    copy(other);
  }

  // destructor
  fMatrixEstimatorBase::parameters::~parameters() {
  }

  // get type name
  const char* fMatrixEstimatorBase::parameters::getTypeName() const {
    return "fMatrixEstimatorBase::parameters";
  }

  // copy member

  fMatrixEstimatorBase::parameters&
    fMatrixEstimatorBase::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    transformEstimator::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    transformEstimator::parameters& (transformEstimator::parameters::* p_copy)
      (const transformEstimator::parameters&) =
      transformEstimator::parameters::copy;
    (this->*p_copy)(other);
# endif

    
      rank2Enforcement = other.rank2Enforcement;
      distanceMeasure = other.distanceMeasure;

    return *this;
  }

  // alias for copy member
  fMatrixEstimatorBase::parameters&
    fMatrixEstimatorBase::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* fMatrixEstimatorBase::parameters::clone() const {
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
  bool fMatrixEstimatorBase::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool fMatrixEstimatorBase::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      switch ( rank2Enforcement ) {
      case Frobenius:
        lti::write(handler,"rank2Enforcement","Frobenius");
      case TParameterization:
        lti::write(handler,"rank2Enforcement","TParameterization");
      default:
        lti::write(handler,"rank2Enforcement","Frobenius");
      }
      switch ( distanceMeasure ) {
      case Sampson:
        lti::write(handler, "distanceMeasure","Sampson");
      case Epipolar:
        lti::write(handler, "distanceMeasure","Epipolar");
      default:
        lti::write(handler, "distanceMeasure","Epipolar");
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transformEstimator::parameters::write(handler,false);
# else
    bool (transformEstimator::parameters::* p_writeMS)(ioHandler&,const bool) const =
      transformEstimator::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool fMatrixEstimatorBase::parameters::write(ioHandler& handler,
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
   * @return true if read was successful
   */
# ifndef _LTI_MSC_6
  bool fMatrixEstimatorBase::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool fMatrixEstimatorBase::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string str;
      lti::read(handler,"rank2Enforcement",str);
      if ( str.compare("TParametrization")  
           != static_cast<int>(std::string::npos) ||
           str.compare("tParametrization")  
           != static_cast<int>(std::string::npos) ) {
        //1st letter is important because L-parameterization could be added
        rank2Enforcement = TParameterization;
      } else {
        rank2Enforcement = Frobenius;
      }
      lti::read(handler,"distanceMeasure",str);
      if ( str.compare("ampson") 
           != static_cast<int>(std::string::npos) ) {
        distanceMeasure = Sampson;
      } else {
        distanceMeasure = Epipolar;
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transformEstimator::parameters::read(handler,false);
# else
    bool (transformEstimator::parameters::* p_readMS)(ioHandler&,const bool) =
      transformEstimator::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool fMatrixEstimatorBase::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // fMatrixEstimatorBase
  // --------------------------------------------------

  // default constructor
  fMatrixEstimatorBase::fMatrixEstimatorBase()
    : transformEstimator(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  fMatrixEstimatorBase::fMatrixEstimatorBase(const parameters& par)
    : transformEstimator() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  fMatrixEstimatorBase::fMatrixEstimatorBase(const fMatrixEstimatorBase& other)
    : transformEstimator() {
    copy(other);
  }

  // destructor
  fMatrixEstimatorBase::~fMatrixEstimatorBase() {
  }

  // returns the name of this type
  const char* fMatrixEstimatorBase::getTypeName() const {
    return "fMatrixEstimatorBase";
  }

  // copy member
  fMatrixEstimatorBase&
    fMatrixEstimatorBase::copy(const fMatrixEstimatorBase& other) {
      transformEstimator::copy(other);

    return (*this);
  }

  // alias for copy member
  fMatrixEstimatorBase&
    fMatrixEstimatorBase::operator=(const fMatrixEstimatorBase& other) {
    return (copy(other));
  }

  // return parameters
  const fMatrixEstimatorBase::parameters&
    fMatrixEstimatorBase::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

}
