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
 * file .......: ltiBhattacharyyaDistance.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 23.8.2002
 * revisions ..: $Id: ltiBhattacharyyaDistance.cpp,v 1.4 2006/02/08 12:12:34 ltilib Exp $
 */

//TODO: include files
#include "ltiBhattacharyyaDistance.h"
#include "ltiMatrixDecomposition.h"
#include "ltiMatrixInversion.h"



namespace lti {
  // --------------------------------------------------
  // bhattacharyyaDistance
  // --------------------------------------------------

  // default constructor
  bhattacharyyaDistance::bhattacharyyaDistance()
    : linearAlgebraFunctor(){

  }

  // copy constructor
  bhattacharyyaDistance::bhattacharyyaDistance(const bhattacharyyaDistance& other) {
    copy(other);
  }

  // destructor
  bhattacharyyaDistance::~bhattacharyyaDistance() {
  }

  // returns the name of this type
  const char* bhattacharyyaDistance::getTypeName() const {
    return "bhattacharyyaDistance";
  }

  // copy member
  bhattacharyyaDistance&
  bhattacharyyaDistance::copy(const bhattacharyyaDistance& other) {
    linearAlgebraFunctor::copy(other);
    return (*this);
  }

  // alias for copy member
  bhattacharyyaDistance&
    bhattacharyyaDistance::operator=(const bhattacharyyaDistance& other) {
    return (copy(other));
  }

  // clone member
  functor* bhattacharyyaDistance::clone() const {
    return new bhattacharyyaDistance(*this);
  }

  // return parameters
  const bhattacharyyaDistance::parameters&
    bhattacharyyaDistance::getParameters() const {
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

  
  bool bhattacharyyaDistance::apply(const dmatrix& cluster1,
                                    const dmatrix& cluster2,
                                    double& distance) const {
    dmatrix cov1,cov2;
    dvector centroid1;
    dvector centroid2;
    bool ok=true;

    varianceFunctor<double> coVGen;
    varianceFunctor<double>::parameters coVGenParam;
    coVGenParam.correlation=false;
    coVGenParam.type=varianceFunctor<double>::parameters::eVarianceType(1);
    coVGen.setParameters(coVGenParam);
    
    coVGen.covarianceMatrixOfRows(cluster1,cov1);
    coVGen.covarianceMatrixOfRows(cluster2,cov2);

    // compute the mean of parameter cluster
    meansFunctor<double> means;
    means.meanOfRows(cluster1,centroid1);
    means.meanOfRows(cluster2,centroid2);

    ok = ok && apply(centroid1,cov1,centroid2,cov2,distance);     

    return ok;

  };

  double bhattacharyyaDistance::apply(const dmatrix& cluster1,
                                      const dmatrix& cluster2) const {

    dmatrix cov1,cov2;
    dvector centroid1;
    dvector centroid2;

    varianceFunctor<double> coVGen;
    varianceFunctor<double>::parameters coVGenParam;
    coVGenParam.correlation=false;
    coVGenParam.type=varianceFunctor<double>::parameters::eVarianceType(1);
    coVGen.setParameters(coVGenParam);
    // compute the covariances of the points in the clusters    
    coVGen.covarianceMatrixOfRows(cluster1,cov1);
    coVGen.covarianceMatrixOfRows(cluster2,cov2);

    // compute the mean of parameter cluster
    meansFunctor<double> means;
    means.meanOfRows(cluster1,centroid1);
    means.meanOfRows(cluster2,centroid2);

    return apply(centroid1,cov1,centroid2,cov2);     
  };

  bool bhattacharyyaDistance::apply(const dvector& centroid1,
                                    const dmatrix& cov1,
                                    const dvector& centroid2,
                                    const dmatrix& cov2,
                                    double& distance) const {
    bool ok=true;

    dvector diff;
    dvector tmp;
    dmatrix sumOfMat;
    matrixInversion<double> invFunc; // functor to invert the covariance Matrix
    luDecomposition<double> detFunc;
    double detSum,det1,det2;
    diff.subtract(centroid1,centroid2); // distance between centroids
    sumOfMat.add(cov1,cov2);
    sumOfMat.multiply(0.5);
    // compute the determinates
    detSum=detFunc.det(sumOfMat);
    det1=detFunc.det(cov1);
    det2=detFunc.det(cov2);
    // compute the second part of the bhattacharyya distance
    distance=0.5*(log(abs(detSum/(sqrt(det1*det2)))));
    invFunc.apply(sumOfMat);
    sumOfMat.transpose(); // transpose,because row vector multiplied with a 
                          // is not supported
    sumOfMat.multiply(diff,tmp); // multiply sumOfMat with diff and leave 
    distance+=((tmp.dot(diff))/8.0);
    return ok;
  };

  double bhattacharyyaDistance::apply(const dvector& centroid1,
                                      const dmatrix& cov1,
                                      const dvector& centroid2,
                                      const dmatrix& cov2) const {
    double distance;
    apply(centroid1,cov1,centroid2,cov2,distance);
    return distance;

  };



}
