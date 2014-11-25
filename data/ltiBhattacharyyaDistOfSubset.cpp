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
 * file .......: ltiBhattacharyyaDistOfSubset.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 22.10.2002
 * revisions ..: $Id: ltiBhattacharyyaDistOfSubset.cpp,v 1.3 2006/02/07 18:11:57 ltilib Exp $
 */

//TODO: include files
#include "ltiBhattacharyyaDistOfSubset.h"
#include "ltiBhattacharyyaDistance.h"
#include <map>

namespace lti {

  // --------------------------------------------------
  // bhattacharyyaDistOfSubset
  // --------------------------------------------------

  bhattacharyyaDistOfSubset::bhattacharyyaDistOfSubset()
    : costFunction(){
  }

  // copy constructor
  bhattacharyyaDistOfSubset::bhattacharyyaDistOfSubset(const bhattacharyyaDistOfSubset& other) {
    copy(other);
  }

  // destructor
  bhattacharyyaDistOfSubset::~bhattacharyyaDistOfSubset() {
  }

  // returns the name of this type
  const char* bhattacharyyaDistOfSubset::getTypeName() const {
    return "bhattacharyyaDistOfSubset";
  }

  // copy member
  bhattacharyyaDistOfSubset&
  bhattacharyyaDistOfSubset::copy(const bhattacharyyaDistOfSubset& other) {
    costFunction::copy(other);
    
    clusters=other.clusters;

    return (*this);
  }

  // alias for copy member
  bhattacharyyaDistOfSubset&
  bhattacharyyaDistOfSubset::operator=(const bhattacharyyaDistOfSubset& other){
    return (copy(other));
  }
  

  // clone member
  functor* bhattacharyyaDistOfSubset::clone() const {
    return new bhattacharyyaDistOfSubset(*this);
  }

 void bhattacharyyaDistOfSubset::setSrc(const dmatrix& src, 
                                        const ivector& srcIds) {

    pSrc.copy(src);
    pSrcIds.copy(srcIds);

      // divide src, and save one dmatrix for each different id
      // clusters is a std::list of dmatrix with each element in the list
      // a cluster
    std::map<int,std::list<int> > myMap;
    std::map<int,std::list<int> >::iterator myMapIt;
    std::list<int>::iterator listIter;
    int i;
    int dim=src.columns();
    for (i=0; i<srcIds.size(); i++) {
        myMap[srcIds.at(i)].push_back(i);
    }
    for (myMapIt=myMap.begin(); myMapIt!=myMap.end(); myMapIt++) {
      std::list<int> points=(*myMapIt).second;
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

  
  // On copy apply for type dmatrix!
  bool bhattacharyyaDistOfSubset::apply(const std::list<int>& in,double& value) {
    bool ok=true;
    value=0.0;
    bhattacharyyaDistance bDist;
    std::list<dmatrix>::const_iterator it1,it2;
    std::list<int>::const_iterator it3=in.begin();
    int i;
    std::list<dmatrix> current;

    for (it1=clusters.begin(); it1!=clusters.end(); it1++) {
      dmatrix tmp(it1->rows(),in.size()); 
      i=0;
      for (it3=in.begin(); it3!=in.end(); it3++) {
        tmp.setColumn(i,it1->getColumnCopy((*it3)));
        i++;
      }
      current.push_back(tmp);
    }
    
    int counter=0;
    for (it1=current.begin(); it1!=--current.end(); it1++) {
      for (it2=it1; it2!=current.end(); it2++) {
        if (it1==it2) it2++;
        value+=bDist.apply((*it1),(*it2));
        counter++;
      }
    }

    value=value/double(counter);
    return ok;
  };



}
