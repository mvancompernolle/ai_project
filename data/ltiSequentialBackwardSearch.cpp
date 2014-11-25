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
 * file .......: ltiSequentialBackwardSearch.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 8.7.2002
 * revisions ..: $Id: ltiSequentialBackwardSearch.cpp,v 1.3 2006/02/07 18:25:16 ltilib Exp $
 */

//TODO: include files
#include "ltiSequentialBackwardSearch.h"

namespace lti {
  // --------------------------------------------------
  // sequentialBackwardSearch
  // --------------------------------------------------

  // default constructor
  sequentialBackwardSearch::sequentialBackwardSearch()
    : plusLTakeAwayR(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // copy constructor
  sequentialBackwardSearch::sequentialBackwardSearch(const sequentialBackwardSearch& other) {
    copy(other);
  }

  // destructor
  sequentialBackwardSearch::~sequentialBackwardSearch() {
  }

  // returns the name of this type
  const char* sequentialBackwardSearch::getTypeName() const {
    return "sequentialBackwardSearch";
  }

  // copy member
  sequentialBackwardSearch&
    sequentialBackwardSearch::copy(const sequentialBackwardSearch& other) {
      plusLTakeAwayR::copy(other);

    return (*this);
  }

  // alias for copy member
  sequentialBackwardSearch&
    sequentialBackwardSearch::operator=(const sequentialBackwardSearch& other) {
    return (copy(other));
  }

  // clone member
  functor* sequentialBackwardSearch::clone() const {
    return new sequentialBackwardSearch(*this);
  }

  // return parameters
  const sequentialBackwardSearch::parameters&
    sequentialBackwardSearch::getParameters() const {
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

  
  bool sequentialBackwardSearch::apply(const dmatrix& src,
                                       const ivector& srcIds,
                                       dmatrix& dest) {
    bool ok;
    parameters param;
    param=getParameters();
    param.l=0;
    param.r=1;
    setParameters(param);
    ok=plusLTakeAwayR::apply(src,srcIds,dest);

    return ok;
  };



}
