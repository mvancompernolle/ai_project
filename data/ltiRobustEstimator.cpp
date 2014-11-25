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
 * file .......: ltiRobustEstimator.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 6.4.2004
 * revisions ..: $Id: ltiRobustEstimator.cpp,v 1.7 2006/09/05 10:29:30 ltilib Exp $
 */

#include "ltiRobustEstimator.h"
#include "ltiIsotropicNormalization.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1

#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // robustEstimator::parameters
  // --------------------------------------------------

  // default constructor
  robustEstimator::parameters::parameters()
    : functor::parameters() {
 
    numIterations = int(50);
    normalization = new isotropicNormalization;
    _lti_debug("robustEstimator::parameters: default constrcutor"<<"\n");
  }

  // copy constructor
  robustEstimator::parameters::parameters(const parameters& other)
    : functor::parameters() {
    normalization = 0;
    _lti_debug("robustEstimator::parameters: copy constructor"<<"\n");
    _lti_debug("___ norm " << other.normalization);
    copy(other);
    _lti_debug("  _constructed\n");
  }

  // destructor
  robustEstimator::parameters::~parameters() {
    _lti_debug("robustEstimator::parameters: destructor"<<"\n");
    if ( notNull(normalization) ) {
      delete normalization;
      normalization = 0;
      _lti_debug("destroyed normalization"<<"\n");
    }
  _lti_debug("robustEstimator::parameters: destructor FINISHED"<<"\n");
  }

  // get type name
  const char* robustEstimator::parameters::getTypeName() const {
    return "robustEstimator::parameters";
  }

  void robustEstimator::parameters
  ::setNormalization(ePointSetNormalizationType name) {
   
    if ( notNull(normalization) ) {
      delete normalization;
      normalization = 0;
    }
    if ( name == IsotropicNormalization ) {
      _lti_debug(" ******* isotropic norma ");
      normalization = new isotropicNormalization;
    } else {
      _lti_debug(" ******* no norma ");
      normalization = 0;
    }
    _lti_debug("setNormalization "<<notNull(normalization)<<"\n");
  }

  void robustEstimator::parameters
  ::setNormalization(const pointSetNormalization& method) {

    if ( notNull(normalization) ) {
      delete normalization;
      normalization = 0;
    }
    normalization = dynamic_cast<pointSetNormalization*>(method.clone());
  }

  bool robustEstimator::parameters::existsNormalization() const {
    return notNull(normalization);
  }

  const pointSetNormalization& 
  robustEstimator::parameters::getNormalization() const {
    
    if ( isNull(normalization) ) {
      throw exception("Null pointer to pointSetNormalization");
    }
    return *normalization;
  }

  // copy member
  robustEstimator::parameters&
    robustEstimator::parameters::copy(const parameters& other) {
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
    
    numIterations = other.numIterations;

    _lti_debug("robustEstimator::parameters::copy invoked\n"); 
    _lti_debug("normalization = "<<normalization<<"\n");

    if ( notNull(normalization)) {
      delete normalization;
      normalization = 0;
      _lti_debug("destroyed normalization"<<"\n");
    }

    if ( notNull(other.normalization) ) {
      normalization 
        = dynamic_cast<pointSetNormalization*>(other.normalization->clone());
      _lti_debug("cloned normalization "<<normalization<<"\n");
    }

    return *this;
  }

  // alias for copy member
  robustEstimator::parameters&
    robustEstimator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* robustEstimator::parameters::clone() const {
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
  bool robustEstimator::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool robustEstimator::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"numIterations",numIterations);
      if ( notNull(normalization) ) {
        lti::write(handler,"normalization", normalization->getTypeName());
        normalization->getParameters().write(handler,complete);
      } else {
        lti::write(handler,"normalization", "NoPointSetNormalization");
      }
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
  bool robustEstimator::parameters::write(ioHandler& handler,
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
  bool robustEstimator::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool robustEstimator::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"numIterations",numIterations);
 
      if ( notNull(normalization)) {
        delete normalization;
        normalization = 0;
      }

      std::string str;
      lti::read(handler,"normalization",str);
      if ( str.find("sotropicNormalization") != std::string::npos) {
        isotropicNormalization::parameters para;
        b &=para.read(handler,complete);
        normalization = new isotropicNormalization (para);
      }
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
  bool robustEstimator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // robustEstimator
  // --------------------------------------------------

  // default constructor
  robustEstimator::robustEstimator()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  robustEstimator::robustEstimator(const parameters& par)
    : functor() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  robustEstimator::robustEstimator(const robustEstimator& other)
    : functor() {
    copy(other);
  }

  // destructor
  robustEstimator::~robustEstimator() {
  }

  // returns the name of this type
  const char* robustEstimator::getTypeName() const {
    return "robustEstimator";
  }

  // copy member
  robustEstimator&
    robustEstimator::copy(const robustEstimator& other) {
      functor::copy(other);

    return (*this);
  }

  // alias for copy member
  robustEstimator&
    robustEstimator::operator=(const robustEstimator& other) {
    return (copy(other));
  }

  // return parameters
  const robustEstimator::parameters&
    robustEstimator::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }
  
  robustEstimator::parameters&
  robustEstimator::getWritableParameters() {
    parameters* par =
      dynamic_cast<parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool robustEstimator
  ::setParameters(const pointSetNormalization::parameters& theParams) {

    parameters& par = getWritableParameters();
    
    if ( isNull(par.normalization) ) {
      return false;
    }
    const std::string nParaName  ( theParams.getTypeName() );
    const std::string normaName (  par.normalization->getTypeName() );
    if ( nParaName.find(normaName) == std::string::npos ) {
      return false;
    }
    return par.normalization->setParameters(theParams);
  }

  bool robustEstimator::setParameters(const parameters& theParams) {
    _lti_debug("robustEstimator::setParameters eNorma"
               << theParams.existsNormalization());
    return functor::setParameters(theParams);
  }
  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

}

#include "ltiUndebug.h"
