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
 * file .......: ltiKMeansClustering.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 05.10.2001
 * revisions ..: $Id: ltiKMeansClustering.cpp,v 1.7 2006/09/05 09:58:51 ltilib Exp $
 */

#include "ltiKMeansClustering.h"
#include "ltiL1Distance.h"
#include "ltiL2Distance.h"
#include "ltiScramble.h"

namespace lti {

  // *** kMeansClustering::parameters ***

  kMeansClustering::parameters::parameters()
    : centroidClustering::parameters() {
    numberOfClusters=2;
  }

  kMeansClustering::parameters::parameters(const parameters& other)
    : centroidClustering::parameters()  {
    copy(other);
  }

  kMeansClustering::parameters::~parameters() {
  }

  // get type name
  const char* kMeansClustering::parameters::getTypeName() const {
    return "kMeansClustering::parameters";
  }

  kMeansClustering::parameters& kMeansClustering::parameters::copy(const parameters& other) {

# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    centroidClustering::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    centroidClustering::parameters& (centroidClustering::parameters::* p_copy)
      (const centroidClustering::parameters&) =
      centroidClustering::parameters::copy;
    (this->*p_copy)(other);
# endif

    numberOfClusters=other.numberOfClusters;

    return *this;
  }

  classifier::parameters* kMeansClustering::parameters::clone() const {

    return new parameters(*this);
  }

# ifndef _LTI_MSC_6
  bool kMeansClustering::parameters::write(ioHandler& handler,
                                           const bool complete) const
# else
  bool kMeansClustering::parameters::writeMS(ioHandler& handler,
                                             const bool complete) const
# endif
  {

    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"numberOfClusters",numberOfClusters);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && centroidClustering::parameters::write(handler,false);
# else
    bool (clustering::parameters::* p_writeMS)(ioHandler&,const bool) const =
      clustering::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool kMeansClustering::parameters::write(ioHandler& handler,
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
  bool kMeansClustering::parameters::read(ioHandler& handler,
                                          const bool complete)
# else
    bool kMeansClustering::parameters::readMS(ioHandler& handler,
                                              const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"numberOfClusters",numberOfClusters);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && centroidClustering::parameters::read(handler,false);
# else
    bool (clustering::parameters::* p_readMS)(ioHandler&,const bool) =
      clustering::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool kMeansClustering::parameters::read(ioHandler& handler,
                             const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif



  // *** kMeansClustering ***

  kMeansClustering::kMeansClustering() : centroidClustering() {
    parameters p;
    setParameters(p);
  };

  kMeansClustering::kMeansClustering(const kMeansClustering& other) 
    : centroidClustering() {
    copy(other);
  };

  kMeansClustering::~kMeansClustering() {
  }

  const char* kMeansClustering::getTypeName() const {
    return "kMeansClustering";
  }

  classifier* kMeansClustering::clone() const {
    return new kMeansClustering(*this);
  }

  kMeansClustering& kMeansClustering::copy(const kMeansClustering& other) {

    centroidClustering::copy(other);
    return (*this);
  }

  // return parameters
  const kMeansClustering::parameters& kMeansClustering::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool kMeansClustering::train(const dmatrix& data) {

    bool ok=true;

    parameters* p=dynamic_cast<parameters*>(params);
    int nbClusters=p->numberOfClusters;
    parameters::eClusterMode mode=p->clusterMode;

    selectRandomPoints(data, nbClusters, centroids);

    switch (mode) {
    case parameters::batch:
      ok = trainBatch(data);
      break;
    case parameters::sequential:
      ok = trainSequential(data);
      break;
    default:
      ok = false;
      setStatusString("Illegal value for clusterMode. Only batch and sequential are valid options.");
    }

    if(ok) {

      //Put the id information into the result object
      //Each cluster has the id of its position in the matrix
      ivector tids(nbClusters);
      int i;
      for (i=0; i<nbClusters; i++) {
        tids.at(i)=i;
      }
      outTemplate=outputTemplate(tids);
    }

    return ok;
  }


  bool kMeansClustering::trainBatch(const dmatrix& data) {

    l2Distance<double> dist;
	int i;

    int nbClusters=dynamic_cast<parameters*>(params)->numberOfClusters;
    int nbPoints=data.rows();

    ivector ids(nbPoints, 0);
    ivector lastIds(nbPoints, 1);
    dmatrix lastCentroids;
    dvector distances(nbClusters);

    //while any data point changes clusters
    while (!ids.equals(lastIds)) {

      //save last ids for condition
      lastIds.copy(ids);

      // ** assign data to clusters
      for (i=0; i<nbPoints; i++) {
        dist.apply(centroids, data.getRow(i), distances);
        ids.at(i)=distances.getIndexOfMinimum();
      }

      // ** calculate new centroids
	  dvector count(nbClusters, 0.);
      //remember centroids
      lastCentroids.copy(centroids);
      //calculate mean
      centroids.fill(0.);
      for (i=0; i<nbPoints; i++) {
        centroids.getRow(ids.at(i)).add(data.getRow(i));
        count.at(ids.at(i))++;
      }
      // if centroid has no data don't change it
      for (i=0; i<nbClusters; i++) {
        if (count[i]!=0) {
          centroids.getRow(i).divide(count[i]);
        } else {
          centroids.setRow(i, lastCentroids.getRow(i));
        }
      }

    }

    return true;
  }

  bool kMeansClustering::trainSequential(const dmatrix& data) {

    l2Distance<double> dist;
    scramble<int> shuffle;
	int i;

    int nbClusters=dynamic_cast<parameters*>(params)->numberOfClusters;
    int nbPoints=data.rows();

    dvector distances(nbClusters);
    ivector ids(nbPoints, 0);
    ivector idx(nbPoints);
    for (i=0; i<nbPoints; i++) {
      idx.at(i)=i;
    }

    // ** assign data to clusters
    for (i=0; i<nbPoints; i++) {
      dist.apply(centroids, data.getRow(i), distances);
      ids.at(i)=distances.getIndexOfMinimum();
    }

    // ** calculate new centroids like in batch just once
	dvector counts(nbClusters, 0.);
    //calculate mean
    centroids.fill(0.);
    for (i=0; i<nbPoints; i++) {
      centroids.getRow(ids.at(i)).add(data.getRow(i));
      counts[ids.at(i)]++;
    }
    for (i=0; i<nbClusters; i++) {
      centroids.getRow(i).divide(counts[i]);
    }

    bool change=true;
    double currCount, oldCount;
    int currId, oldId;

    while(change) {

      change=false;
      shuffle.apply(idx);

      for (vector<int>::iterator it=idx.begin(); it!=idx.end(); it++) {

        dist.apply(centroids, data.getRow(*it), distances);
        currId=distances.getIndexOfMinimum();

        if (currId!=ids.at(*it)) {
          oldId=ids.at(*it);
          ids.at(*it)=currId;
          change=true;
          currCount=oldCount=0;
          dvector& currC=centroids.getRow(currId);
          dvector& oldC=centroids.getRow(oldId);
          currC.fill(0.);
          oldC.fill(0.);
          for (i=0; i<nbPoints; i++) {
            if (ids.at(i)==currId) {
              currC.add(data.getRow(i));
              currCount++;
            } else if (ids.at(i)==oldId) {
              oldC.add(data.getRow(i));
              oldCount++;
            }
          }
          currC.divide(currCount);
          oldC.divide(oldCount);
        }
      }
    }

    return true;
  }

  bool kMeansClustering::train(const dmatrix& data, ivector& ids) {

    return centroidClustering::train(data, ids);
  }

}
