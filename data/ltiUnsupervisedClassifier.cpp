/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiUnsupervisedClassifier.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 29.08.2001
 * revisions ..: $Id: ltiUnsupervisedClassifier.cpp,v 1.7 2006/02/07 18:27:41 ltilib Exp $
 */

#include "ltiClassifier.h"
#include "ltiUnsupervisedClassifier.h"
#include <ctime>

namespace lti {

  // --------------------------------------------------
  // unsupervisedClassifier::parameters
  // --------------------------------------------------

  // default constructor
  unsupervisedClassifier::parameters::parameters()
    : classifier::parameters() {
    
    // change default setting
    multipleMode=outputTemplate::Ignore;
  }
  
  // copy constructor
  unsupervisedClassifier::parameters::parameters(const parameters& other)
    : classifier::parameters()  {
    copy(other);
  }

  // destructor
  unsupervisedClassifier::parameters::~parameters() {
  }

  // get type name
  const char* unsupervisedClassifier::parameters::getTypeName() const {
    return "unsupervisedClassifier::parameters";
  }

  // copy member
  unsupervisedClassifier::parameters&
  unsupervisedClassifier::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    classifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    classifier::parameters& (classifier::parameters::* p_copy)
      (const classifier::parameters&) = classifier::parameters::copy;
    (this->*p_copy)(other);
# endif

    return *this;
  }

  unsupervisedClassifier::unsupervisedClassifier()
    : classifier() {}

  unsupervisedClassifier::unsupervisedClassifier(const unsupervisedClassifier& other)
    : classifier(other)  {}

  // returns the name of this type
  const char* unsupervisedClassifier::getTypeName() const {
    return "unsupervisedClassifier";
  }

  // copy member
  unsupervisedClassifier&
    unsupervisedClassifier::copy(const unsupervisedClassifier& other) {

    classifier::copy(other);
    return (*this);

  }

  // return parameters
  const unsupervisedClassifier::parameters&
  unsupervisedClassifier::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool unsupervisedClassifier::train(const dmatrix& data,
                                     ivector& ids) {

    bool rc=train(data);

    std::string msgTrain=getStatusString();
    
    int nbData=data.rows();

    // resize and clear ids
    ids.resize(nbData, 0, false);

    classifier::outputVector out;

    bool ok=true;
    for (int i=0; i<nbData; i++) {
      if (classify(data.getRow(i),out)) {
        ids.at(i)=out.maxId();
      } else {
        ids.at(i)=classifier::noObject;
        ok=false;
      }
    }

    if (!rc) {
      msgTrain=std::string(getStatusString())+"\n and also: \n"+msgTrain;
      setStatusString(msgTrain.c_str());
    }

    return rc&&ok;
  }

  bool unsupervisedClassifier::selectRandomPoints(const dmatrix& data,
                                                  int numberOfPoints,
                                                  dmatrix& randomPoints) {

    randomPoints.resize(numberOfPoints, data.columns());
    int i;

    int nbDataPoints=data.rows();
    ivector index(nbDataPoints);
    for (i=0; i<nbDataPoints; i++) {
      index.at(i)=i;
    }

    srand(static_cast<unsigned int>(time(0)));
    int rValue;
    int pointsLeft=nbDataPoints;
    for (i=0; i<numberOfPoints; i++) {

      int r=rand();
      rValue=r%(pointsLeft);

      randomPoints.setRow(i, data.getRow(index.at(rValue)));
      index.at(rValue)=index.at(pointsLeft-1);
      pointsLeft--;
      if (pointsLeft==0) {
        pointsLeft=nbDataPoints;
      }
    }

    return true;
  }

}
