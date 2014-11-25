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
 * file .......: ltiMahalanobisDistance.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 13.8.2002
 * revisions ..: $Id: ltiMahalanobisDistance.cpp,v 1.4 2006/02/08 12:32:33 ltilib Exp $
 */

#include "ltiMahalanobisDistance.h"
#include "ltiMatrixInversion.h"
#include "ltiMeansFunctor.h"

namespace lti {
  // --------------------------------------------------
  // mahalanobisDistance
  // --------------------------------------------------

  // default constructor
  mahalanobisDistance::mahalanobisDistance()
    : linearAlgebraFunctor(){

  }

  // copy constructor
  mahalanobisDistance::mahalanobisDistance(const mahalanobisDistance& other) {
    copy(other);
  }

  // destructor
  mahalanobisDistance::~mahalanobisDistance() {
  }

  // returns the name of this type
  const char* mahalanobisDistance::getTypeName() const {
    return "mahalanobisDistance";
  }

  // copy member
  mahalanobisDistance&
    mahalanobisDistance::copy(const mahalanobisDistance& other) {
      linearAlgebraFunctor::copy(other);

    return (*this);
  }

  // alias for copy member
  mahalanobisDistance&
    mahalanobisDistance::operator=(const mahalanobisDistance& other) {
    return (copy(other));
  }


  // clone member
  functor* mahalanobisDistance::clone() const {
    return new mahalanobisDistance(*this);
  }

  // return parameters
  const mahalanobisDistance::parameters&
    mahalanobisDistance::getParameters() const {
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

  

  // On copy apply for type dmatrix!
  bool mahalanobisDistance::apply(const dvector& p1,const dvector& p2, 
                                  const dmatrix& coV, double& distance) const {
    dvector diff(p1.size());
    dvector tmp(p1.size());
    dmatrix invCoV;
    matrixInversion<double> invFunc; // functor to invert the covariance Matrix
    invFunc.apply(coV,invCoV);
     // transpose inverted cov. Matrix to multiply it  with a row vector
    diff.subtract(p1,p2);
    invCoV.multiply(diff,tmp);
    distance = tmp.dot(diff);
    distance=sqrt(distance);
    return true;

  };

  bool mahalanobisDistance::apply(const dvector& p1,const dmatrix& cluster, 
                                  double& distance) const {
      // caluclate the mean of all points; it is expected the each row contains
      // a data point
    dvector mean;
    dmatrix coV;
    bool ok=true;
    // compute the mean of parameter cluster
    meansFunctor<double> means;
    means.meanOfRows(cluster,mean);
    // now the covarianceMatrix of the cluster is computed
    varianceFunctor<double> coVGen;
    varianceFunctor<double>::parameters coVGenParam;
    coVGenParam.correlation=false;
    coVGenParam.type=varianceFunctor<double>::parameters::eVarianceType(1);
    coVGen.setParameters(coVGenParam);
    coVGen.covarianceMatrixOfRows(cluster,coV);
    ok = apply(p1,mean,coV,distance);
    return ok;
  };

  double mahalanobisDistance::apply(const dvector& p1,
                                    const dmatrix& cluster) const {
    double d=0.0;
    apply(p1,cluster,d);
    return d;
  };

  double mahalanobisDistance::apply(const dvector& p1,const dvector& p2, 
                                    const dmatrix& coV) const {
    double d;
    apply(p1,p2,coV,d);
    return d;

  };
}
