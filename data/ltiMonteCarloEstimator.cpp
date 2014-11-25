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
 * file .......: ltiMonteCarloEstimator.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 6.4.2004
 * revisions ..: $Id: ltiMonteCarloEstimator.cpp,v 1.4 2006/09/05 10:22:44 ltilib Exp $
 */

#include "ltiMonteCarloEstimator.h"
#include "ltiTransformEstimatorFactory.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1

#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // monteCarloEstimator::parameters
  // --------------------------------------------------

  // default constructor
  monteCarloEstimator::parameters::parameters()
    : robustEstimator::parameters() {
 
    maxError = double(.8);
    transform = 0;
    transformEstimatorFactory fact;
    transform = fact.newInstance("lti::homography8DofEstimator");
    _lti_debug("monteCarloEstimator::parameters: default constrcutor"<<"\n");
  }

  // copy constructor
  monteCarloEstimator::parameters::parameters(const parameters& other)
    : robustEstimator::parameters() {
    transform = 0;
    _lti_debug("monteCarloEstimator::parameters: copy constructor"<<"\n");
    _lti_debug("___ tr " << other.transform);
    copy(other);
    _lti_debug("  _constructed\n");
  }

  // destructor
  monteCarloEstimator::parameters::~parameters() {
    _lti_debug("monteCarloEstimator::parameters: destructor"<<"\n");
    if ( notNull(transform) ) {
      //delete transform;
      transform = 0;
      _lti_debug("destroyed transform"<<"\n");
    }
    _lti_debug("monteCarloEstimator::parameters: destructor FINISHED"<<"\n");
  }

  // get type name
  const char* monteCarloEstimator::parameters::getTypeName() const {
    return "monteCarloEstimator::parameters";
  }

  void monteCarloEstimator::parameters
  ::setTransform(eTransformEstimatorType name) {
    
    if ( notNull(transform)) {
      delete transform;
      transform = 0;
    }
    _lti_debug("---setTransform "<<notNull(transform)<<"\n");
    transformEstimatorFactory fact;
    transform = fact.newInstance(name);
    _lti_debug("setTransform "<<notNull(transform)<<"\n");
  }

  void monteCarloEstimator::parameters
  ::setTransform(const transformEstimator& estimator) {
    if ( notNull(transform) ) {
      delete transform;
      transform = 0;
    }
    transform = dynamic_cast<transformEstimator*>(estimator.clone());    
  }

  bool monteCarloEstimator::parameters::existsTransform() const {
    return notNull(transform);
  }

  const transformEstimator& monteCarloEstimator::parameters::getTransform() const {
    
    if ( isNull(transform) ) {
      throw exception("Null pointer to transformEstimator");
    }
    return *transform;
  }

  // copy member
  monteCarloEstimator::parameters&
    monteCarloEstimator::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    robustEstimator::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    robustEstimator::parameters& (robustEstimator::parameters::* p_copy)
      (const robustEstimator::parameters&) =
      robustEstimator::parameters::copy;
    (this->*p_copy)(other);
# endif
  
    _lti_debug("monteCarloEstimator::parameters::copy invoked\n");    
    _lti_debug("transform = "<<transform<<"\n");

    if ( notNull(transform)) {
      delete transform;
      transform = 0;
      _lti_debug("destroyed transform"<<"\n");
    }

    if ( notNull(other.transform) ) {
      transform = dynamic_cast<transformEstimator*>(other.transform->clone());
      _lti_debug("cloned transform "<<transform<<"\n");
    }

    return *this;
  }

  // alias for copy member
  monteCarloEstimator::parameters&
    monteCarloEstimator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* monteCarloEstimator::parameters::clone() const {
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
  bool monteCarloEstimator::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool monteCarloEstimator::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"maxError",maxError);
      if ( notNull(transform) ) {
        lti::write(handler,"transform", transform->getTypeName());
        transform->getParameters().write(handler,complete);
      } else {
        lti::write(handler,"transform","NoTransformEstimator");
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && robustEstimator::parameters::write(handler,false);
# else
    bool (robustEstimator::parameters::* p_writeMS)(ioHandler&,const bool) const =
      robustEstimator::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool monteCarloEstimator::parameters::write(ioHandler& handler,
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
  bool monteCarloEstimator::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool monteCarloEstimator::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
   
      lti::read(handler,"maxError",maxError);

      if ( notNull(transform)) {
        delete transform;
        transform = 0;
      }
 
      std::string str;
      lti::read(handler,"transform",str);
      //factory returns null-pointer if the class name is unknown
      transformEstimatorFactory fact;
      transform = fact.newInstance(std::string("lti::")+str);
      if ( notNull(transform) ) {
        transform->read(handler,complete);
      }

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && robustEstimator::parameters::read(handler,false);
# else
    bool (robustEstimator::parameters::* p_readMS)(ioHandler&,const bool) =
      robustEstimator::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool monteCarloEstimator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // monteCarloEstimator
  // --------------------------------------------------

  // default constructor
  monteCarloEstimator::monteCarloEstimator()
    : robustEstimator(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  monteCarloEstimator::monteCarloEstimator(const parameters& par)
    : robustEstimator() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  monteCarloEstimator::monteCarloEstimator(const monteCarloEstimator& other)
    : robustEstimator() {
    copy(other);
  }

  // destructor
  monteCarloEstimator::~monteCarloEstimator() {
  }

  // returns the name of this type
  const char* monteCarloEstimator::getTypeName() const {
    return "monteCarloEstimator";
  }

  // copy member
  monteCarloEstimator&
    monteCarloEstimator::copy(const monteCarloEstimator& other) {
      robustEstimator::copy(other);

    return (*this);
  }

  // alias for copy member
  monteCarloEstimator&
    monteCarloEstimator::operator=(const monteCarloEstimator& other) {
    return (copy(other));
  }

  // return parameters
  const monteCarloEstimator::parameters&
    monteCarloEstimator::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&robustEstimator::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }
  
  monteCarloEstimator::parameters&
  monteCarloEstimator::getWritableParameters() {
    parameters* par =
      dynamic_cast<parameters*>(&robustEstimator::getWritableParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool monteCarloEstimator
  ::setParameters(const transformEstimator::parameters& theParams) {
    parameters& par = getWritableParameters();
    
    if ( isNull(par.transform) ) {
      return false;
    }
    const std::string tParaName  ( theParams.getTypeName() );
    const std::string transName (  par.transform->getTypeName() );
    if ( tParaName.find(transName) == std::string::npos ) {
      return false;
    }
    return par.transform->setParameters(theParams);
  }

  bool monteCarloEstimator
  ::setParameters(const pointSetNormalization::parameters& theParams) {
    return robustEstimator::setParameters(theParams); 
  }

  bool monteCarloEstimator::setParameters(const parameters& theParams) {
    return robustEstimator::setParameters(theParams);
  }
  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

}

#include "ltiUndebug.h"
