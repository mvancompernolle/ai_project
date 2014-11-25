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
 * file .......: ltiCostFunction.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 4.9.2002
 * revisions ..: $Id: ltiCostFunction.cpp,v 1.3 2006/02/07 18:15:25 ltilib Exp $
 */

//TODO: include files
#include "ltiCostFunction.h"

namespace lti {

  // --------------------------------------------------
  // costFunction
  // --------------------------------------------------

  costFunction::costFunction()
    : functor  (){
      
  }

  // copy constructor
  costFunction::costFunction(const costFunction& other) {
    copy(other);
  }

  // destructor
  costFunction::~costFunction() {
  }

  // returns the name of this type
  const char* costFunction::getTypeName() const {
    return "costFunction";
  }

  // copy member
  costFunction& costFunction::copy(const costFunction& other) {
    functor::copy(other);
    
    pSrc=other.pSrc;
    pSrcIds=other.pSrcIds;

    return (*this);
  }

  // alias for copy member
  costFunction&
    costFunction::operator=(const costFunction& other) {
    return (copy(other));
  }


  void costFunction::setSrc(const dmatrix& src, const ivector& srcIds) {
    pSrc.copy(src);
    pSrcIds.copy(srcIds);
  }
  
  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool costFunction::apply(const ivector& in,double& value) {
    ivector::const_iterator it;
    std::list<int> tmp;
    for (it=in.begin(); it!=in.end(); it++) {
      tmp.push_back(*it);
    }    
    return apply(tmp,value);
  };

  double costFunction::apply(const ivector& in) {
    ivector::const_iterator it;
    std::list<int> tmp;
    double value;
    for (it=in.begin(); it!=in.end(); it++) {
      tmp.push_back(*it);
    }    
    apply(tmp,value);
    return value;
  };
  
  double costFunction::apply(const std::list<int>& in) {
    double value;
    apply(in,value);
    return value;
  };
  


}
