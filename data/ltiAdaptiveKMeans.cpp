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
 * file .......: ltiAdaptiveKMeans.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 28.2.2002
 * revisions ..: $Id: ltiAdaptiveKMeans.cpp,v 1.8 2006/09/05 09:56:06 ltilib Exp $
 */

#include "ltiAdaptiveKMeans.h"
#include "ltiL1Distance.h"
#include "ltiL2Distance.h"
#include "ltiKMeansClustering.h"

namespace lti {
  // --------------------------------------------------
  // adaptiveKMeans::parameters
  // --------------------------------------------------

  // default constructor
  adaptiveKMeans::parameters::parameters()
    : centroidClustering::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    minNeighborhood = double(0.);
    maxNeighborhood = double(0.);
    learnRate = 1.0;
    nbNeighborhoods = 20;
    minNumberOfPoints = double();
    nbClusters = 10;
    detectNeighborhood = true;
    maxIterations = 10000;
    maxDistance = 0.001;
  }

  // copy constructor
  adaptiveKMeans::parameters::parameters(const parameters& other)
    : centroidClustering::parameters()  {
    copy(other);
  }

  // destructor
  adaptiveKMeans::parameters::~parameters() {
  }

  // get type name
  const char* adaptiveKMeans::parameters::getTypeName() const {
    return "adaptiveKMeans::parameters";
  }

  // copy member

  adaptiveKMeans::parameters&
    adaptiveKMeans::parameters::copy(const parameters& other) {
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


      minNeighborhood = other.minNeighborhood;
      maxNeighborhood = other.maxNeighborhood;
      learnRate = other.learnRate;
      nbNeighborhoods = other.nbNeighborhoods;
      minNumberOfPoints = other.minNumberOfPoints;
      nbClusters = other.nbClusters;
      detectNeighborhood = other.detectNeighborhood;
      maxIterations = other.maxIterations;
      maxDistance = other.maxDistance;

    return *this;
  }

  // alias for copy member
  adaptiveKMeans::parameters&
    adaptiveKMeans::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  classifier::parameters* adaptiveKMeans::parameters::clone() const {
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
  bool adaptiveKMeans::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool adaptiveKMeans::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"minNeighborhood",minNeighborhood);
      lti::write(handler,"maxNeighborhood",maxNeighborhood);
      lti::write(handler,"learnRate",learnRate);
      lti::write(handler,"nbNeighborhoods",nbNeighborhoods);
      lti::write(handler,"minNumberOfPoints",minNumberOfPoints);
      lti::write(handler,"nbClusters",nbClusters);
      lti::write(handler,"detectNeighborhood",detectNeighborhood);
      lti::write(handler,"maxIterations",maxIterations);
      lti::write(handler,"maxDistance",maxDistance);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && centroidClustering::parameters::write(handler,false);
# else
    bool (centroidClustering::parameters::* p_writeMS)(ioHandler&,const bool) const =
      centroidClustering::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool adaptiveKMeans::parameters::write(ioHandler& handler,
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
  bool adaptiveKMeans::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool adaptiveKMeans::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"minNeighborhood",minNeighborhood);
      lti::read(handler,"maxNeighborhood",maxNeighborhood);
      lti::read(handler,"learnRate",learnRate);
      lti::read(handler,"nbNeighborhoods",nbNeighborhoods);
      lti::read(handler,"minNumberOfPoints",minNumberOfPoints);
      lti::read(handler,"nbClusters",nbClusters);
      lti::read(handler,"detectNeighborhood",detectNeighborhood);
      lti::read(handler,"maxIterations",maxIterations);
      lti::read(handler,"maxDistance",maxDistance);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && centroidClustering::parameters::read(handler,false);
# else
    bool (centroidClustering::parameters::* p_readMS)(ioHandler&,const bool) =
      centroidClustering::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool adaptiveKMeans::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // adaptiveKMeans
  // --------------------------------------------------

  // default constructor
  adaptiveKMeans::adaptiveKMeans()
    : centroidClustering(){


    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // copy constructor
  adaptiveKMeans::adaptiveKMeans(const adaptiveKMeans& other)
    : centroidClustering()  {
    copy(other);
  }

  // destructor
  adaptiveKMeans::~adaptiveKMeans() {
  }

  // returns the name of this type
  const char* adaptiveKMeans::getTypeName() const {
    return "adaptiveKMeans";
  }

  // copy member
  adaptiveKMeans&
    adaptiveKMeans::copy(const adaptiveKMeans& other) {
      centroidClustering::copy(other);
    return (*this);
  }

  // alias for copy member
  adaptiveKMeans&
    adaptiveKMeans::operator=(const adaptiveKMeans& other) {
    return (copy(other));
  }


  // clone member
  classifier* adaptiveKMeans::clone() const {
    return new adaptiveKMeans(*this);
  }

  // return parameters
  const adaptiveKMeans::parameters&
  adaptiveKMeans::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw classifier::invalidParametersException(getTypeName());
    }
    return *par;
  }

 // -------------------------------------------------------------------
  // The train-methods!
  // -------------------------------------------------------------------

  // Calls the same method of the superclass.
  bool adaptiveKMeans::train(const dmatrix& input, ivector& ids) {

    return centroidClustering::train(input, ids);
  }


  bool adaptiveKMeans::train(const dmatrix& data) {

    // create variables
    int i,j;
    bool ok=true; // return value, set false if any error occurs
    parameters p=getParameters();
    int nbClusters=p.nbClusters;
    double learnRate=p.learnRate;
    const int maxIterations=p.maxIterations;
    int iterations=0; // counts the iterations
    bool stable=false;
    int nbPoints=data.rows();
    dvector counter1(nbClusters,0.0);
    dvector counter2(nbClusters,0.0);
    dmatrix delta1Y(nbClusters,data.columns(),0.0);
    dmatrix delta2Y(nbClusters,data.columns(),0.0);
    int index1;
    dvector distances(nbClusters);
    double distance,multiplier,norm1,norm2;
    bool  restart=false;
    int testing=0;
    dvector temp(data.columns(),0.0);
    clusterSpreading.resize(nbClusters+1);
    dmatrix newCentroids(nbClusters,data.columns());
    // initiliaze the centroids
    centroids.resize(nbClusters,data.columns());
    selectRandomPoints(data,nbClusters,centroids);
    // create functors
    l2Distance<double> dist;
    kMeansClustering kMeansClusterer;
    kMeansClustering::parameters kMeansParameters;
    // detect neighborhood automaticly
    if (p.detectNeighborhood) {
        /* maxNeighborhood is the distance between the centroids of a
           k means clustering with two clusters; minDistance ist the
           minimum distance between a k means clustering with the maximum
           number of clusters
        */
      kMeansClustering kMeansClusterer;
      kMeansClustering::parameters kMeansParameters;
      kMeansParameters.numberOfClusters=2;
      kMeansClusterer.setParameters(kMeansParameters);
      kMeansClusterer.train(data);
      dist.apply(kMeansClusterer.getCentroids().getRow(0),
                 kMeansClusterer.getCentroids().getRow(1),
                 p.maxNeighborhood);
      kMeansParameters.numberOfClusters=nbClusters;
      kMeansClusterer.setParameters(kMeansParameters);
      kMeansClusterer.train(data);
      dmatrix centroids(p.nbClusters,data.columns());
      dmatrix distances(nbClusters,nbClusters);
      centroids=kMeansClusterer.getCentroids();
      distance=p.maxNeighborhood;
      double tmp;
      // find smallest distance
      for (i=0; i<nbClusters; i++) {
        for (j=i+1; j<nbClusters; j++) {
          tmp=dist.apply(centroids.getRow(i),centroids.getRow(j));
          if (tmp<distance) distance=tmp;
        }
      }
      p.minNeighborhood=distance;
      if (p.minNeighborhood==p.maxNeighborhood)
          p.minNeighborhood=0.0;
    }
    double increment=(p.maxNeighborhood-p.minNeighborhood)/
        (double)p.nbNeighborhoods;
    if (increment==0) increment=0.01;
    double neighborhood;
    for (neighborhood = p.minNeighborhood;
         neighborhood <= p.maxNeighborhood;
         neighborhood += increment) {
        // restart with smaller learn rate if centroids don't converge
      if (restart) {
        learnRate=learnRate/2;
        nbClusters=p.nbClusters;
        neighborhood=p.minNeighborhood;
        centroids.resize(nbClusters,data.columns());
        selectRandomPoints(data,nbClusters,centroids);
        stable=false;
        restart=false;
        setStatusString("learn rate was too big, halved it");
        clusterSpreading.fill(0);
      }
      // if a cluster was deleted, resize
      if (nbClusters!=newCentroids.rows()) {
        centroids.resize(nbClusters,centroids.columns());
        newCentroids.resize(nbClusters,centroids.columns());
        distances.resize(nbClusters,0.0,false);
      }
      while (!stable) {
        // initialize variables
        counter1.fill(0.0);
        counter2.fill(0.0);
        delta1Y.fill(0.0);
        delta2Y.fill(0.0);
        for (i=0; i<nbPoints; i++) {
            // Cluster closest to point i
          dist.apply(centroids,data.getRow(i),distances);
          // if centroids moved too far away from the data points
          // restart with smaller learn Rate
          if (centroids.maximum()>data.maximum()*10000) {
            restart=true;
            stable=true;
            ok=false;
          }
          index1=distances.getIndexOfMinimum();
          temp=data.getRow(i);
          temp.subtract(centroids.getRow(index1));
          temp.multiply(learnRate);
          delta1Y.getRow(index1).add(temp);
          counter1.at(index1)++;
          // clusters in neighborhood of clusternumber index1
          dist.apply(centroids,centroids.getRow(index1),
                     distances);
          for (j=0; j<nbClusters; j++) {
              // if cluster j is in neighborhood calculate displacement vector
            if ((distances.at(j) < neighborhood) && (j!=index1)) {
              temp=centroids.getRow(j);
              temp.subtract(centroids.getRow(index1));
              delta2Y.getRow(j).subtract(temp);
              counter2.at(j)++;
            }
          }
        }
        newCentroids.copy(centroids);
        for (j=0; j<nbClusters; j++) {
/* calculate multiplier; the multiplier ensures that that each of the two
   aspects of the cost function are equally weighted for each cluster center */
          if (counter2.at(j)!=0 && counter1.at(j)!=0) {
              // calculate norms of the displament vectors
            norm1=dist.apply(delta1Y.getRow(j));
            norm2=dist.apply(delta2Y.getRow(j));
            multiplier=(counter2.at(j)*norm1)/(counter1.at(j)*norm2);
          }
          else {
            multiplier=0.0;
          }
          if (counter1.at(j)==0) multiplier=1;
          temp=newCentroids.getRow(j);
          temp.add(delta1Y.getRow(j));
          delta2Y.getRow(j).multiply(multiplier);
          temp.add(delta2Y.getRow(j));
          newCentroids.setRow(j,temp);
        }
        // if centroids moved 5 times less than parameter ... or more
        // than maxIterations were made centroids are stable
        if (centroids.prettyCloseTo(newCentroids,p.maxDistance)) {
          testing++;
        } else {
          testing=0;
        }
        if (testing >5 || iterations>maxIterations) {
          stable =true;
        }
        centroids.copy(newCentroids);
        iterations++; // counts the iterations
      }
      // initialize variables for next value of neighborhood
      testing=0;
      stable=false;
      iterations=0;

	  // delete clusters with less points than parameter minNumberOfPoints

      for (i=0; i<nbClusters; i++)
	{
            if ((counter1.at(i)) < getParameters().minNumberOfPoints) {
              centroids.setRow(i,centroids.getRow(nbClusters-1));
              //centroids.resize(centroids.rows()-1,centroids.columns());
              counter1.at(i)=counter1.at((nbClusters-1));
              nbClusters--;
              i--;
            }
        }
      nbClusters=centroids.rows();
      /* combine clusters with inter-cluster distance < neighborhood  */
      for (i=0; i<nbClusters; i++) {
        for (j=i+1; j<nbClusters; j++) {
          distance=dist.apply(centroids.getRow(i),centroids.getRow(j));
          if (distance < neighborhood) {
            temp=centroids.getRow(i);
            temp.multiply(counter1.at(i));
            centroids.getRow(j).multiply(counter1.at(j));
            temp+=centroids.getRow(j);
            temp.divide((counter1.at(i)+counter1.at(j)));
            centroids.setRow(i,temp);
            centroids.setRow(j,centroids.getRow(nbClusters-1));
            nbClusters--;
          }
        }
      }
      clusterSpreading.at(nbClusters)++;
    }
    // do k means with the most likeliest number of clusters
    nbClusters=clusterSpreading.getIndexOfMaximum();
    if (nbClusters!=centroids.rows()) {
      centroids.resize(nbClusters,centroids.columns());
      kMeansParameters.numberOfClusters=nbClusters;
      kMeansClusterer.setParameters(kMeansParameters);
      kMeansClusterer.train(data);
      centroids=kMeansClusterer.getCentroids();
    }

    //Put the id information into the result object
    //Each cluster has the id of its position in the matrix
    ivector tids(nbClusters);
    for (i=0; i<nbClusters; i++) {
      tids.at(i)=i;
    }
    outTemplate=outputTemplate(tids);

    setParameters(p);

    return ok;
  }

    /**
     * returns the spreading of the clusters
     */
    const ivector& adaptiveKMeans::getClusterSpreading() const {
        return clusterSpreading;
    }
  // -------------------------------------------------------------------
  // The classify-method!
  // -------------------------------------------------------------------

  //TODO Usually this method does not need to be implemented use inherited
  // method from direct subclass of classifier (ie centroidClassifier) instead
  // In this case delete. Otherwise comment!
//      virtual bool
//      classify(const dvector& feature, outputVector& result) const {
//    const classifier::output&
//    adaptiveKMeans::classify(const dvector& feature) {

//      return getOutput();
//     }



}
