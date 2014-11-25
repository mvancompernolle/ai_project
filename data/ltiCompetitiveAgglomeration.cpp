/*
 * Copyright (C) 2004, 2005, 2006
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
 * file .......: ltiCompetitiveAgglomeration.cpp
 * authors ....: Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 21.1.2004
 * revisions ..: $Id: ltiCompetitiveAgglomeration.cpp,v 1.5 2006/09/05 09:57:29 ltilib Exp $
 */

#include "ltiCompetitiveAgglomeration.h"

namespace lti {
  // --------------------------------------------------
  // competitiveAgglomeration::parameters
  // --------------------------------------------------

  // default constructor
  competitiveAgglomeration::parameters::parameters()
    : centroidClustering::parameters() {

    // set default values of fuzzyCMeans parameters to reasonable values
    // note, that the fuzzy c means clustering is used only for pre-classification and
    // should therefore be performed quickly and only give a raw coverage of the data.
    initialNumberOfClusters = 20;
    initialIterations = 10;
    fuzzifier = 2.0;
    maxIterations = 100;

    cardinalityFactor = 5.0;
    timeConstant = 10.0;
    minimumCardinality = 5.0;

    convergenceThreshold = 0.02;
  }

  // copy constructor
  competitiveAgglomeration::parameters::parameters(const parameters& other)
    : centroidClustering::parameters() {
    copy(other);
  }

  // destructor
  competitiveAgglomeration::parameters::~parameters() {
  }

  // get type name
  const char* competitiveAgglomeration::parameters::getTypeName() const {
    return "competitiveAgglomeration::parameters";
  }

  // copy member

  competitiveAgglomeration::parameters&
    competitiveAgglomeration::parameters::copy(const parameters& other) {
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

    
    initialNumberOfClusters = other.initialNumberOfClusters;
    initialIterations = other.initialIterations;
    fuzzifier = other.fuzzifier;
    maxIterations = other.maxIterations;
    cardinalityFactor = other.cardinalityFactor;
    timeConstant = other.timeConstant;
    minimumCardinality = other.minimumCardinality;
    convergenceThreshold = other.convergenceThreshold;

    return *this;
  }

  // alias for copy member
  competitiveAgglomeration::parameters&
    competitiveAgglomeration::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  classifier::parameters* competitiveAgglomeration::parameters::clone() const {
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
  bool competitiveAgglomeration::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool competitiveAgglomeration::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"initialNumberOfClusters",initialNumberOfClusters);
      lti::write(handler,"initialIterations",initialIterations);
      lti::write(handler,"fuzzifier",fuzzifier);
      lti::write(handler,"maxIterations",maxIterations);
      lti::write(handler,"cardinalityFactor",cardinalityFactor);
      lti::write(handler,"timeConstant",timeConstant);
      lti::write(handler,"minimumCardinality",minimumCardinality);
      lti::write(handler,"convergenceThreshold",convergenceThreshold);
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
  bool competitiveAgglomeration::parameters::write(ioHandler& handler,
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
   * @return true if read was successful
   */
# ifndef _LTI_MSC_6
  bool competitiveAgglomeration::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool competitiveAgglomeration::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"initialNumberOfClusters",initialNumberOfClusters);
      lti::read(handler,"initialIterations",initialIterations);
      lti::read(handler,"fuzzifier",fuzzifier);
      lti::read(handler,"maxIterations",maxIterations);
      lti::read(handler,"cardinalityFactor",cardinalityFactor);
      lti::read(handler,"timeConstant",timeConstant);
      lti::read(handler,"minimumCardinality",minimumCardinality);
      lti::read(handler,"convergenceThreshold",convergenceThreshold);
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
  bool competitiveAgglomeration::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // competitiveAgglomeration
  // --------------------------------------------------

  // default constructor
  competitiveAgglomeration::competitiveAgglomeration()
    : centroidClustering(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  competitiveAgglomeration::competitiveAgglomeration(const parameters& par)
    : centroidClustering() {

    //TODO: comment the attributes of your classifier
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  competitiveAgglomeration::competitiveAgglomeration(const competitiveAgglomeration& other)
    : centroidClustering() {
    copy(other);
  }

  // destructor
  competitiveAgglomeration::~competitiveAgglomeration() {
  }

  // returns the name of this type
  const char* competitiveAgglomeration::getTypeName() const {
    return "competitiveAgglomeration";
  }

  // copy member
  competitiveAgglomeration&
    competitiveAgglomeration::copy(const competitiveAgglomeration& other) {
      centroidClustering::copy(other);

    //TODO: comment the attributes of your classifier
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  competitiveAgglomeration&
    competitiveAgglomeration::operator=(const competitiveAgglomeration& other) {
    return (copy(other));
  }


  // clone member
  classifier* competitiveAgglomeration::clone() const {
    return new competitiveAgglomeration(*this);
  }

  // return parameters
  const competitiveAgglomeration::parameters&
    competitiveAgglomeration::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The train-methods!
  // -------------------------------------------------------------------

    // train the prototypes
  bool competitiveAgglomeration::train(const dmatrix& input) {
    const parameters& caParameters = getParameters();

    int initialIterations = caParameters.initialIterations;
    int maxIterations = caParameters.maxIterations;

    selectRandomPoints(input, caParameters.initialNumberOfClusters, centroids);
    partitionMatrix.resize(centroids.rows(), input.rows(), 1.0 / centroids.rows(), false, true);

    // save dimensions and fuzzifier
    numberOfClusters = centroids.rows();
    numberOfPoints = input.rows();
    numberOfDimensions = input.columns();
    fuzzifier = caParameters.fuzzifier;

    updateCardinality();
    distanceMatrix.resize(numberOfClusters, numberOfPoints);


    // pre-clustering: fuzzy-c-means clustering with given number of iterations
    int iteration;
    for (iteration = 0; iteration < initialIterations; ++iteration) {
      updateDistance(input);
      updateMembership(0.0);
      updateCardinality();
      updatePrototypes(input);
    }

    bool prototypesStable = false;

    updateCardinality();

    iteration = 0;
    while (!prototypesStable && iteration < maxIterations && numberOfClusters > 1) {

      updateDistance(input);

      double alpha = calculateAlpha(iteration, caParameters.timeConstant);
      updateMembership(alpha);

      updateCardinality();

      // save old prototypes
      dmatrix oldCentroids = centroids;

      // discard spurious clusters
      int k = 0;
      while (k < numberOfClusters && numberOfClusters > 1) {
        if (clusterCardinality.at(k) < caParameters.minimumCardinality)
          discardCluster(k);
        else
          ++k;
      }

      updatePrototypes(input);

      // compare location of new and old prototypes
      if (oldCentroids.rows() == centroids.rows())
        prototypesStable = (distFunc.apply(oldCentroids, centroids) <= caParameters.convergenceThreshold);

      ++iteration;
    }

    // free unused memory
    partitionMatrix.clear();
    distanceMatrix.clear();
    clusterCardinality.clear();

    // set output template
    ivector clusterID(numberOfClusters);
    for (int c = 0; c < numberOfClusters; ++c)
      clusterID.at(c) = c;
    setOutputTemplate(outputTemplate(clusterID));

    return true;
  }



  // calculate distance function for all training points and clusters
  void competitiveAgglomeration::updateDistance(const dmatrix &trainingData) {

    // update distances
    for (int k = 0; k < numberOfClusters; ++k) {
      for (int t = 0; t < numberOfPoints; ++t) {
        // special (faster) treatment of square fuzzifier
        if (fuzzifier == 2.0)
          distanceMatrix.at(k, t) = distanceSqr(trainingData.getRow(t), centroids.getRow(k));
        else
          distanceMatrix.at(k, t) = fuzzify(distFunc.apply(trainingData.getRow(t), centroids.getRow(k)));
      }
    }
  }



  // calculate alpha value for this iteration
  double competitiveAgglomeration::calculateAlpha(int iteration, double timeConstant) const {

    double numerator = 0.0;
    double denominator = 0.0;
    double u_kt;

    for (int k = 0; k < numberOfClusters; ++k) {
      double sum = 0.0;

      for (int t = 0; t < numberOfPoints; ++t) {
        u_kt = partitionMatrix.at(k, t);
        sum += u_kt;
        numerator += fuzzify(u_kt) * distanceMatrix.at(k, t);
      }
      denominator += fuzzify(sum);
    }

    double alpha = getParameters().cardinalityFactor * numerator / denominator;
    if (timeConstant != 0.0)
      alpha *= exp(-double(iteration) / timeConstant);

    return alpha;
  }




  // calculate cardinality of each cluster from the current membership values
  void competitiveAgglomeration::updateCardinality() {

    clusterCardinality.resize(numberOfClusters);
    for (int k = 0; k < numberOfClusters; ++k)
      clusterCardinality.at(k) = partitionMatrix.getRow(k).sumOfElements();
  }




  // update membership matrix
  void competitiveAgglomeration::updateMembership(double cardinalityFactor) {

    // k always serves as the cluster iterator, j as point iterator
    int k, t;
    double dist;

    dvector weightedCardinality(numberOfPoints);
    dvector inverseSum(numberOfPoints);

    // calculate weighted cardinalities and sums of inverse distance of all points
    for (t = 0; t < numberOfPoints; ++t) {
      for (k = 0; k < numberOfClusters; ++k) {
        dist = distanceMatrix.at(k, t);

        // point lies directly on center of prototype
        if (dist == 0.0) {
          inverseSum.at(t) = 1.0;
          weightedCardinality.at(t) = clusterCardinality.at(k);
          break;
        }
        else {
          inverseSum.at(t) += 1.0 / dist;
          weightedCardinality.at(t) += clusterCardinality.at(k) / dist;
        }
      }

      weightedCardinality.at(t) /= inverseSum.at(t);
    }

    // calculate partition
    for (t = 0; t < numberOfPoints; ++t) {
      for (k = 0; k < numberOfClusters; ++k) {

        double& u_kt = partitionMatrix.at(k, t);
        dist = distanceMatrix.at(k, t);

        // if point lies directly on center of prototype ...
        if (dist == 0.0) {
          // ... all other prototypes get membership 0.0
          partitionMatrix.setColumn(t, dvector(numberOfClusters, 0.0));
          u_kt = 1.0;
          break;
        }
        // calculate membership
        else {
          // u_kt = uFCM_kt + uBIAS_kt
          u_kt = (1.0 / dist) / inverseSum.at(t);
          if (cardinalityFactor != 0.0)
            u_kt += (cardinalityFactor / dist) * (clusterCardinality.at(k) - weightedCardinality.at(t));

          // clip membership to [0,1]
          if (u_kt < 0.0)
            u_kt = 0.0;
          else if (u_kt > 1.0)
            u_kt = 1.0;
        }
      }
    }

  }




  // throw cluster away, i.e. delete from clusterCardinality, distanceMatrix, membership and centroids
  void competitiveAgglomeration::discardCluster(int k) {
    if (k < 0 || k >= numberOfClusters)
      return;

    // decrease number of clusters
    --numberOfClusters;

    // copy value from last element
    if (k != numberOfClusters) {
      centroids.setRow(k, centroids.getRow(numberOfClusters));
      distanceMatrix.setRow(k, distanceMatrix.getRow(numberOfClusters));
      partitionMatrix.setRow(k, partitionMatrix.getRow(numberOfClusters));
      clusterCardinality.at(k) = clusterCardinality.at(numberOfClusters);
    }

    // discard last cluster
    centroids.resize(numberOfClusters, numberOfDimensions);
    distanceMatrix.resize(numberOfClusters, numberOfPoints);
    partitionMatrix.resize(numberOfClusters, numberOfPoints);
    clusterCardinality.resize(numberOfClusters);
  }

  // update prototypes
  void competitiveAgglomeration::updatePrototypes(const dmatrix &trainingData) {
    centroids.resize(numberOfClusters, numberOfDimensions, 0.0, false, true);
    double u_kt;

    for (int k = 0; k < numberOfClusters; ++k) {

      // calculate center of current prototype
      dvector& currentCluster = centroids.getRow(k);

      // add weighted data
      double squareSum = 0.0;
      for (int t = 0; t < numberOfPoints; ++t) {
        u_kt = fuzzify(partitionMatrix.at(k, t));

        currentCluster.addScaled(u_kt, trainingData.getRow(t));
        squareSum += u_kt;
      }

      // average
      currentCluster.divide(squareSum);
    }
  }

}
