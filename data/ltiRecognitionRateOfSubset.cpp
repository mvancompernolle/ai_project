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
 * file .......: ltiRecognitionRateOfSubset.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 22.10.2002
 * revisions ..: $Id: ltiRecognitionRateOfSubset.cpp,v 1.8 2006/09/05 10:01:51 ltilib Exp $
 */

#include "ltiRecognitionRateOfSubset.h"

namespace lti {
  // --------------------------------------------------
  // recognitionRateOfSubset::parameters
  // --------------------------------------------------

  // default constructor
  recognitionRateOfSubset::parameters::parameters() 
    : costFunction::parameters() {    
    
    cvParameter = crossValidator::parameters();
  }

  // copy constructor
  recognitionRateOfSubset::parameters::parameters(const parameters& other) {
    copy(other);
  }
  
  // destructor
  recognitionRateOfSubset::parameters::~parameters() {    
  }

  // get type name
  const char* recognitionRateOfSubset::parameters::getTypeName() const {
    return "recognitionRateOfSubset::parameters";
  }
  
  // copy member

  recognitionRateOfSubset::parameters& 
    recognitionRateOfSubset::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    costFunction::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    costFunction::parameters& (costFunction::parameters::* p_copy)
      (const costFunction::parameters&) = 
      costFunction::parameters::copy;
    (this->*p_copy)(other);
# endif
    
    cvParameter = other.cvParameter;

    return *this;
  }

  // alias for copy member
  recognitionRateOfSubset::parameters& 
    recognitionRateOfSubset::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* recognitionRateOfSubset::parameters::clone() const {
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
  bool recognitionRateOfSubset::parameters::write(ioHandler& handler,
                                         const bool complete) const 
# else
  bool recognitionRateOfSubset::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const 
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      cvParameter.write(handler,false);      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to 
    // compile...
    b = b && costFunction::parameters::write(handler,false);
# else
    bool (costFunction::parameters::* p_writeMS)(ioHandler&,const bool) const = 
      costFunction::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool recognitionRateOfSubset::parameters::write(ioHandler& handler,
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
  bool recognitionRateOfSubset::parameters::read(ioHandler& handler,
                                        const bool complete) 
# else
  bool recognitionRateOfSubset::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      cvParameter.read(handler,false);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to 
    // compile...
    b = b && costFunction::parameters::read(handler,false);
# else
    bool (costFunction::parameters::* p_readMS)(ioHandler&,const bool) = 
      costFunction::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool recognitionRateOfSubset::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // recognitionRateOfSubset
  // --------------------------------------------------

  // default constructor
  recognitionRateOfSubset::recognitionRateOfSubset()
    : costFunction(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }



//   recognitionRateOfSubset::recognitionRateOfSubset(const dmatrix& src,const ivector& srcId)
//     : costFunction(src,srcId){
//     // create an instance of the parameters with the default values
//     parameters defaultParameters;
//     // set the default parameters
//     setParameters(defaultParameters);

//   }  

  // copy constructor
  recognitionRateOfSubset::recognitionRateOfSubset(const recognitionRateOfSubset& other) {
    copy(other);
  }

  // destructor
  recognitionRateOfSubset::~recognitionRateOfSubset() {
  }

  // returns the name of this type
  const char* recognitionRateOfSubset::getTypeName() const {
    return "recognitionRateOfSubset";
  }

  // copy member
  recognitionRateOfSubset&
  recognitionRateOfSubset::copy(const recognitionRateOfSubset& other) {
    costFunction::copy(other);

    return (*this);
  }

  // alias for copy member
  recognitionRateOfSubset&
    recognitionRateOfSubset::operator=(const recognitionRateOfSubset& other) {
    return (copy(other));
  }

  // clone member
  functor* recognitionRateOfSubset::clone() const {
    return new recognitionRateOfSubset(*this);
  }

  // return parameters
  const recognitionRateOfSubset::parameters&
    recognitionRateOfSubset::getParameters() const {
    const parameters* par = 
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException();
    }
    return *par;
  }
  
  void recognitionRateOfSubset::setSrc(const dmatrix& src,
                                       const ivector& srcIds) {
    costFunction::setSrc(src,srcIds);
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On copy apply for type dmatrix!
  bool recognitionRateOfSubset::apply(const std::list<int>& in,double& value) {

    int i;
    bool ok=true;
    dmatrix tmp(pSrc.rows(),in.size());
    std::list<int>::const_iterator inIt;
    for (inIt=in.begin(), i=0; inIt!=in.end(); inIt++, i++) {
      tmp.setColumn(i,pSrc.getColumnCopy(*inIt));
    } 

    crossValidator cV;
    crossValidator::parameters cVParam;
    cVParam=getParameters().cvParameter;
    cV.setParameters(cVParam);

    ok =ok && cV.apply(tmp,pSrcIds,value);
    
    return ok;
  }

};
