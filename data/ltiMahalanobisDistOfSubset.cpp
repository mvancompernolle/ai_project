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
 * file .......: ltiMahalanobisDistOfSubset.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 22.10.2002
 * revisions ..: $Id: ltiMahalanobisDistOfSubset.cpp,v 1.3 2006/02/07 18:20:54 ltilib Exp $
 */

#include "ltiMahalanobisDistOfSubset.h"
#include "ltiMahalanobisDistance.h"

namespace lti {
  // --------------------------------------------------
  // mahalanobisDistOfSubset
  // --------------------------------------------------

  // default constructor
  mahalanobisDistOfSubset::mahalanobisDistOfSubset()
    : costFunction(){
  }

  // copy constructor
  mahalanobisDistOfSubset::mahalanobisDistOfSubset(const mahalanobisDistOfSubset& other) {
    copy(other);
  }

  // destructor
  mahalanobisDistOfSubset::~mahalanobisDistOfSubset() {
  }

  // returns the name of this type
  const char* mahalanobisDistOfSubset::getTypeName() const {
    return "mahalanobisDistOfSubset";
  }

  // copy member
  mahalanobisDistOfSubset&
  mahalanobisDistOfSubset::copy(const mahalanobisDistOfSubset& other) {
    costFunction::copy(other);
    
    clusters=other.clusters;

    return (*this);
  }

  // alias for copy member
  mahalanobisDistOfSubset&
    mahalanobisDistOfSubset::operator=(const mahalanobisDistOfSubset& other) {
    return (copy(other));
  }


  // clone member
  functor* mahalanobisDistOfSubset::clone() const {
    return new mahalanobisDistOfSubset(*this);
  }

  void mahalanobisDistOfSubset::setSrc(const dmatrix& src, 
                                       const ivector& srcIds) {

    pSrc.copy(src);
    pSrcIds.copy(srcIds);

    std::map<int,list<int> > myMap;
    std::map<int,list<int> >::iterator myMapIt;
    std::list<int>::iterator listIter;
    int i;
    int dim=src.columns();
    // make a map that contains for each id the the row of the point in the
    // matrix. After this, each key in the map shows the rows of the points
    // in the matrix that belong to the id. For example:
    // id 1: points in the rows 2,3,56,78,.. belong to this id
    for (i=0; i<srcIds.size(); i++) {
      myMap[srcIds.at(i)].push_back(i);
    }
    // now for each id a dmatrix with its points in the rows is created an
    // saved in a list of dmatrix
    for (myMapIt=myMap.begin(); myMapIt!=myMap.end(); myMapIt++) {
      list<int> points=(*myMapIt).second;
      dmatrix tmp(points.size(),dim);
      listIter=points.begin();
      for (i=0; i<tmp.rows(); i++) {
        tmp.setRow(i,src.getRow(*listIter));
        listIter++;
      }
      clusters.push_back(tmp);
    }
  } 

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool mahalanobisDistOfSubset::apply(const std::list<int>& in,
                                      double& value) {
      
      mahalanobisDistance mDist;
      value = 0.0;
      bool ok=true;
      std::list<dmatrix>::iterator it1,it2;
      std::list<int>::const_iterator inIt;

      int i;
      list<dmatrix> current;
      it1=current.begin();
      // the vector "in" is a vector with the number of the dimension that is
      // already inserted from the saved distribution. These columns
      // are now copied into an temp matrix and then the distance is computed.
      for (it1=clusters.begin(); it1!=clusters.end(); it1++) {
        dmatrix tmp(it1->rows(),in.size());
        for (inIt=in.begin(), i=0; inIt!=in.end(); inIt++, i++) {
          tmp.setColumn(i,it1->getColumnCopy(*inIt));
        }
        current.push_back(tmp);
      }
      double tmp=0.0;
      int counter=0;
      dvector mean;
      dmatrix cov;
      meansFunctor<double> means;
      varianceFunctor<double> coVGen;
      varianceFunctor<double>::parameters coVGenParam;
      coVGenParam.correlation=false;
      coVGenParam.type=varianceFunctor<double>::parameters::eVarianceType(1);
      coVGen.setParameters(coVGenParam);

      for (it1=current.begin(); it1!=current.end(); it1++) {
        for (it2=it1; it2!=current.end(); it2++) {
          // compute for all points in it2 the mahalanobis distance 
          // to the distribution pointed by it1
          if ( it1!=it2) {
            tmp=0.0;
            // compute the mean and the covariance matrix of cluster pointed 
            // by it1
            means.meanOfRows((*it1),mean);
            coVGen.covarianceMatrixOfRows((*it1),cov);
            for (i=0; i<it2->rows(); i++) {
              tmp+=mDist.apply(it2->getRow(i),mean,cov);
            }
            // add next dist based on the number of points
            value=value+tmp/it2->rows();
            counter++;
          }
        }
      }
      value=value/counter;
  
    return ok;
  };

} //namespace lti
