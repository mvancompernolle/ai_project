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
 * file .......: ltiStudentDistribution.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 20.9.2004
 * revisions ..: $Id: ltiStudentDistribution.cpp,v 1.8 2006/09/05 10:41:39 ltilib Exp $
 */

#include "ltiStudentDistribution.h"
#include "ltiMath.h"
#include <limits>

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // studentDistribution::parameters
  // --------------------------------------------------

  // default constructor
  studentDistribution::parameters::parameters()
    : mathFunction::parameters() {
  
    useLUT = bool(false);
    maxDOF = int(30);
    oneSided = bool(false);
    alpha = float(.05);
  }

  // copy constructor
  studentDistribution::parameters::parameters(const parameters& other)
    : mathFunction::parameters() {
    copy(other);
  }

  // destructor
  studentDistribution::parameters::~parameters() {
  }

  // get type name
  const char* studentDistribution::parameters::getTypeName() const {
    return "studentDistribution::parameters";
  }

  // copy member

  studentDistribution::parameters&
    studentDistribution::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    mathFunction::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    mathFunction::parameters& (mathFunction::parameters::* p_copy)
      (const mathFunction::parameters&) =
      mathFunction::parameters::copy;
    (this->*p_copy)(other);
# endif

    
      useLUT = other.useLUT;
      maxDOF = other.maxDOF;
      oneSided = other.oneSided;
      alpha = other.alpha;

    return *this;
  }

  // alias for copy member
  studentDistribution::parameters&
    studentDistribution::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* studentDistribution::parameters::clone() const {
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
  bool studentDistribution::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool studentDistribution::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"useLUT",useLUT);
      lti::write(handler,"maxDOF",maxDOF);
      lti::write(handler,"oneSided",oneSided);
      lti::write(handler,"alpha",alpha);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && mathFunction::parameters::write(handler,false);
# else
    bool (mathFunction::parameters::* p_writeMS)(ioHandler&,const bool) const =
      mathFunction::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool studentDistribution::parameters::write(ioHandler& handler,
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
  bool studentDistribution::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool studentDistribution::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"useLUT",useLUT);
      lti::read(handler,"maxDOF",maxDOF);
      lti::read(handler,"oneSided",oneSided);
      lti::read(handler,"alpha",alpha);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && mathFunction::parameters::read(handler,false);
# else
    bool (mathFunction::parameters::* p_readMS)(ioHandler&,const bool) =
      mathFunction::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool studentDistribution::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // studentDistribution
  // --------------------------------------------------

  // default constructor
  studentDistribution::studentDistribution()
    : mathFunction(),
      alpha_(-1.),
      lut_() {

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  studentDistribution::studentDistribution(const parameters& par)
    : mathFunction(),
      alpha_(-1.),
      lut_() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  studentDistribution::studentDistribution(const studentDistribution& other)
    : mathFunction(),
      alpha_(-1.),
      lut_() {
    copy(other);
  }

  // destructor
  studentDistribution::~studentDistribution() {
  }

  // returns the name of this type
  const char* studentDistribution::getTypeName() const {
    return "studentDistribution";
  }

  // copy member
  studentDistribution&
    studentDistribution::copy(const studentDistribution& other) {
      mathFunction::copy(other);
      
      alpha_ = other.alpha_;
      lut_    = other.lut_;
    return (*this);
  }

  // alias for copy member
  studentDistribution&
    studentDistribution::operator=(const studentDistribution& other) {
    return (copy(other));
  }


  // clone member
  functor* studentDistribution::clone() const {
    return new studentDistribution(*this);
  }

  // return parameters
  const studentDistribution::parameters&
    studentDistribution::getParameters() const {
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
  
  bool studentDistribution::apply (const int n, const float& tValue,
                                   float& confidence) const {

    if ( n <= 0 ) {
      setStatusString("\napplyLUT: n (degrees of freedom) must be > 0\n");
      return false;
    }
    confidence = distribution(n, tValue);
    return true;
  };
 
  float studentDistribution::bound (const int n) const {
    
    const parameters& par = getParameters();
    const float alpha ( par.oneSided ? 2.f*par.alpha : par.alpha );
    return computeBound(n,alpha);
  }
 
  bool studentDistribution::accept(const int n, const float& tValue) {
    
    const parameters& par = getParameters();
    if ( par.useLUT ) {
      if ( !generateLUT() ) {
        //exception because returning false does not give cue for error
        throw exception(getStatusString());
      }
      return acceptLUT(n, tValue);
    } else {
      return acceptCompute(n, tValue);
    }
  }

  bool studentDistribution
  ::acceptCompute(const int n, const float& tValue) const {
    
    if ( n <= 0 ) {
      setStatusString("\napplyLUT: n (degrees of freedom) must be > 0\n");
      return false;
    }
    const parameters& par = getParameters();
    const float alpha ( par.oneSided ? 2.f*par.alpha : par.alpha );
    _lti_debug(" _conf "<<distribution(n, tValue));
    return distribution(n, tValue) > 1.f-alpha;
  }

  // -------------------------------------------------------------------
  // Computation of student distribution
  // -------------------------------------------------------------------

  //todo
  bool studentDistribution::generateLUT() {
    
    const parameters& par = getParameters();
    const float alpha ( par.oneSided ? 2.f*par.alpha : par.alpha );

    //check if the LUT is up to date
    if ( lut_.size() > 0 && alpha == alpha_ ) {
      return true;
    }
   const float epsilon ( std::numeric_limits<float>::epsilon() );

    //only allow common alpha
    //this is quite restrictive, but I do not know the limit for
    // alpha_onesided > .2
    if ( alpha < 0-epsilon || alpha > .2+epsilon ) {
      setStatusString("\n generateLUT: invalid alpha.");
      setStatusString(" Must be >= 0 and <= .1\n");
      return false;
    }
    _lti_debug(" _alpha "<<alpha);

    //generate the LUT (values until 100)
    const float epsLUT ( .1E-03f ); //3digits precision
    int n ( 1 );
    float last ( 0. );
    float tmp  ( computeBound(n, alpha)   );
    while ( abs(last-tmp) > epsLUT && n <= par.maxDOF ) {
      lut_.push_back(tmp);
      last = tmp;
      tmp = computeBound(++n, alpha);
    }
    
    _lti_debug("\n n "<<1<<":\t "<<lut_.front()<<endl);
    _lti_debug(" n "<<lut_.size()<<":\t "<<lut_.back()<<endl);

    //for common alpha: also store limit values
    if ( abs(alpha-.2f) < epsilon  ) {
      lut_.push_back(1.282f);
    } else if ( abs(alpha-.1f) < epsilon  ) {
      lut_.push_back(1.645f);
    } else if ( abs(alpha-.05f) < epsilon  ) {
      lut_.push_back(1.960f);
    } else if ( abs(alpha-.025f) < epsilon  ) {
      lut_.push_back(2.241f);
    } else if ( abs(alpha-.02f) < epsilon  ) {
      lut_.push_back(2.326f);
    } else if ( abs(alpha-.01f) < epsilon  ) {
      lut_.push_back(2.576f);
    } else if ( abs(alpha-.005f) < epsilon  ) {
      lut_.push_back(2.807f);
    } else if ( abs(alpha-.003f) < epsilon  ) {
      lut_.push_back(2.968f);
    } else if ( abs(alpha-.002f) < epsilon  ) {
      lut_.push_back(3.090f);
    } else if ( abs(alpha-.001f) < epsilon  ) {
      lut_.push_back(3.291f);
    }
    
    _lti_debug("limit:\t "<<lut_.back()<<endl);

    //remember the alpha
    alpha_ = alpha;

#if defined(_LTI_DEBUG) && _LTI_DEBUG>1
    {
      _lti_debug("\n alpha " << alpha << "\n student distribution:\n");
      std::vector<float>::const_iterator it  ( lut_.begin() );
      std::vector<float>::const_iterator end ( lut_.end() );
      int n ( 1 );
      for ( ; it!=end; ++it,++n ) {
        _lti_debug(" n "<<n<<": "<<*it<<endl);
      }
    }
#endif

    return true;
  }

  float studentDistribution::computeBound(const int n,
                                          const float& alpha) const {
    
    //the limit for commom alpha is above 1.5
    float t ( 1.5 );
    //values in statistical tables have 3 digits -> use 4 here
    for ( ; distribution(n,t)<1.f-alpha; t+=.0001f ) {
    }
    return t;
  }

  float studentDistribution::distribution(const int n,
                                          const float& tValue) const {

    //this computes a two-sided confidence
    //for improved precision we use double internally
    //betai is computed by iterative approximation
    const double v ( static_cast<double>(n) );
    const double x ( v / (v + sqr(static_cast<double>(tValue)) ) );
    return static_cast<float>(1-betai(v*.5, .5, x));
  }
}

#include "ltiUndebug.h"
