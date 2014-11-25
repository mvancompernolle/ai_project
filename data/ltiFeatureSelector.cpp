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
 * file .......: ltiFeatureSelector.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 4.7.2002
 * revisions ..: $Id: ltiFeatureSelector.cpp,v 1.8 2006/09/05 09:58:10 ltilib Exp $
 */

//TODO: include files
#include "ltiFeatureSelector.h"

namespace lti {
  // --------------------------------------------------
  // featureSelector::parameters
  // --------------------------------------------------

  // default constructor
  featureSelector::parameters::parameters() 
    : functor::parameters() {    
    
    nbFeatures = 2;
  }

  // copy constructor
  featureSelector::parameters::parameters(const parameters& other) {
    copy(other);
  }
  
  // destructor
  featureSelector::parameters::~parameters() {    
  }

  // get type name
  const char* featureSelector::parameters::getTypeName() const {
    return "featureSelector::parameters";
  }
  
  // copy member

  featureSelector::parameters& 
    featureSelector::parameters::copy(const parameters& other) {
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
    
    
      nbFeatures = other.nbFeatures;
//      nbOfSplits = other.nbOfSplits;

    return *this;
  }

  // alias for copy member
  featureSelector::parameters& 
    featureSelector::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* featureSelector::parameters::clone() const {
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
  bool featureSelector::parameters::write(ioHandler& handler,
                                         const bool complete) const 
# else
  bool featureSelector::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const 
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"nbFeatures",nbFeatures);
//      lti::write(handler,"nbOfSplits",nbOfSplits);
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
  bool featureSelector::parameters::write(ioHandler& handler,
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
  bool featureSelector::parameters::read(ioHandler& handler,
                                        const bool complete) 
# else
  bool featureSelector::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"nbFeatures",nbFeatures);
//      lti::read(handler,"nbOfSplits",nbOfSplits);
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
  bool featureSelector::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // featureSelector
  // --------------------------------------------------

  // default constructor
  featureSelector::featureSelector()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  featureSelector::featureSelector(const featureSelector& other) {
    copy(other);
  }

  // destructor
  featureSelector::~featureSelector() {
  }

  // returns the name of this type
  const char* featureSelector::getTypeName() const {
    return "featureSelector";
  }

  // copy member
  featureSelector& featureSelector::copy(const featureSelector& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  featureSelector&
    featureSelector::operator=(const featureSelector& other) {
    return (copy(other));
  }

//    // clone member
//    functor* featureSelector::clone() const {
//      return new featureSelector(*this);
//    }

  // return parameters
  const featureSelector::parameters&
    featureSelector::getParameters() const {
    const parameters* par = 
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException();
    }
    return *par;
  }
  
  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // 
  dmatrix featureSelector::apply(const dmatrix& src,
                                  const ivector& srcIds) const {
    dmatrix dest;
    apply(src,srcIds,dest);
    return dest;
  }

}
