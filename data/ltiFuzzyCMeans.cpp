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
 * file .......: ltiFuzzyCMeans.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 5.2.2002
 * revisions ..: $Id: ltiFuzzyCMeans.cpp,v 1.7 2006/09/05 09:58:20 ltilib Exp $
 */

#include "ltiFuzzyCMeans.h"
#include "ltiL1Distance.h"
#include "ltiL2Distance.h"

namespace lti {
  // --------------------------------------------------
  // fuzzyCMeans::parameters
  // --------------------------------------------------

  // default constructor
  fuzzyCMeans::parameters::parameters()
    : centroidClustering::parameters() {

      fuzzifier = 2.0;
      norm = L2;
      epsilon = 0.02;
      maxIterations = 100;
      nbOfClusters = 2;
  }

  // copy constructor
  fuzzyCMeans::parameters::parameters(const parameters& other)
    : centroidClustering::parameters()  {
    copy(other);
  }

  // destructor
  fuzzyCMeans::parameters::~parameters() {
  }

  // get type name
  const char* fuzzyCMeans::parameters::getTypeName() const {
    return "fuzzyCMeans::parameters";
  }

  // copy member

  fuzzyCMeans::parameters&
    fuzzyCMeans::parameters::copy(const parameters& other) {
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


      fuzzifier = other.fuzzifier;
      norm = other.norm;
      epsilon = other.epsilon;
      maxIterations = other.maxIterations;
      nbOfClusters = other.nbOfClusters;

    return *this;
  }

  // alias for copy member
  fuzzyCMeans::parameters&
    fuzzyCMeans::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  classifier::parameters* fuzzyCMeans::parameters::clone() const {
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
  bool fuzzyCMeans::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool fuzzyCMeans::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"fuzzifier",fuzzifier);
      lti::write(handler,"epsilon",epsilon);
      lti::write(handler,"maxIterations",maxIterations);
      lti::write(handler,"nbOfClusters",nbOfClusters);
      switch(norm) {
        case L1:
          lti::write(handler,"norm","L1distance");
          break;
        case L2:
          lti::write(handler,"norm","L2distance");
          break;
      }
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
  bool fuzzyCMeans::parameters::write(ioHandler& handler,
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
  bool fuzzyCMeans::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool fuzzyCMeans::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"fuzzifier",fuzzifier);
//      lti::read(handler,"norm",norm);
      lti::read(handler,"epsilon",epsilon);
      lti::read(handler,"maxIterations",maxIterations);
      lti::read(handler,"nbOfClusters",nbOfClusters);
      std::string str;
      lti::read(handler,"norm",str);

      if (str == "L1distance") {
        norm = L1;
      } else if (str == "L2distance") {
        norm = L2;
      } else {
        norm = L2;
      }
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
  bool fuzzyCMeans::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // fuzzyCMeans
  // --------------------------------------------------

  // default constructor
  fuzzyCMeans::fuzzyCMeans()
    : centroidClustering(){


    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // copy constructor
  fuzzyCMeans::fuzzyCMeans(const fuzzyCMeans& other)
    : centroidClustering()  {
    copy(other);
  }

  // destructor
  fuzzyCMeans::~fuzzyCMeans() {
  }

  // returns the name of this type
  const char* fuzzyCMeans::getTypeName() const {
    return "fuzzyCMeans";
  }

  // copy member
  fuzzyCMeans&
    fuzzyCMeans::copy(const fuzzyCMeans& other) {
      centroidClustering::copy(other);


    return (*this);
  }

  // alias for copy member
  fuzzyCMeans&
    fuzzyCMeans::operator=(const fuzzyCMeans& other) {
    return (copy(other));
  }


  // clone member
  classifier* fuzzyCMeans::clone() const {
    return new fuzzyCMeans(*this);
  }

  // return parameters
  const fuzzyCMeans::parameters&
    fuzzyCMeans::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  /*
   * sets the classifier's parameters.
   * The functor keeps its own copy of the given parameters.
   */
  bool fuzzyCMeans::setParameters(const classifier::parameters& theParams) {
      bool ok=classifier::setParameters(theParams);
      if (getParameters().norm!=parameters::L1
	               || getParameters().norm!=parameters::L2) {
	  ok=false;
	  setStatusString("no valid norm selected; selected L2-norm instead");
      }
      return ok;
  }


  // -------------------------------------------------------------------
  // The train-methods!
  // -------------------------------------------------------------------

  // Calls the same method of the superclass.
  bool fuzzyCMeans::train(const dmatrix& input, ivector& ids) {

    return centroidClustering::train(input, ids);
  }

  // implements the Fuzzy C Means algorithm
  bool fuzzyCMeans::train(const dmatrix& data) {

    bool ok=true;
    int t=0;
    // create the distance functor according to the paramter norm
    distanceFunctor<double>* distFunc = 0;
    switch (getParameters().norm)  {
      case parameters::L1:
        distFunc = new l1Distance<double>;
        break;
      case parameters::L2:
        distFunc = new l2Distance<double>;
        break;
      default:
        break;
    }
    int nbOfClusters=getParameters().nbOfClusters;
    int nbOfPoints=data.rows();
    if(nbOfClusters>nbOfPoints) {
      setStatusString("more Clusters than points");
      ok = false;
    }
    double q=getParameters().fuzzifier;
    if (q<=1) {
      setStatusString("q has to be bigger than 1");
      ok = false;
    }
    // select some points of the given data to initialise the centroids
    selectRandomPoints(data,nbOfClusters,centroids);
    // initialize variables
    centroids.resize(nbOfClusters,data.columns(),0.0);
    dmatrix memberships(nbOfPoints, nbOfClusters, 0.0);
    double terminationCriterion=0;
    double newDistance;
    dvector newCenter(data.columns());
    dvector currentPoint(data.columns());
    dmatrix newCentroids(nbOfClusters,data.columns(),0.0);
    double sumOfMemberships=0;
    double membership=0;
    double dist1;
    double dist2;
    int i,j,k,m;
    do {
        // calculate new memberships
      memberships.fill(0.0);  //  clear old memberships
      for (i=0; i<nbOfPoints; i++) {
        for (j=0; j<nbOfClusters; j++) {
          newDistance=0;
          dist1=distFunc->apply(data.getRow(i),
                                centroids.getRow(j));
          for (k=0; k<nbOfClusters; k++) {
            dist2=distFunc->apply(data.getRow(i),
                                  centroids.getRow(k));
       // if distance is 0, normal calculation of membership is not possible.
            if (dist2!=0) {
              newDistance+=pow((dist1/dist2),(1/(q-1)));
            }
          }
      // if point and centroid are equal
          if (newDistance!=0)
            memberships.at(i,j)=1/newDistance;
          else {
            dvector row(memberships.columns(),0.0);
            memberships.setRow(i,row);
            memberships.at(i,j)=1;
            break;
          }
        }
      }
      t++;  // counts the iterations

     // calculate new centroids based on modified memberships
      for (m=0; m<nbOfClusters; m++) {
        newCenter.fill(0.0);
        sumOfMemberships=0;
        for (i=0; i<nbOfPoints; i++) {
          currentPoint=data.getRow(i);
          membership=pow(memberships.at(i,m),q);
          sumOfMemberships+=membership;
          currentPoint.multiply(membership);
          newCenter.add(currentPoint);
        }
        newCenter.divide(sumOfMemberships);
        newCentroids.setRow(m,newCenter);
      }
      terminationCriterion=distFunc->apply(centroids,newCentroids);
      centroids=newCentroids;
    }
    // the termination criterions
    while ( (terminationCriterion>getParameters().epsilon)
            && (t<getParameters().maxIterations));

    int nbClusters = nbOfClusters;
    //Put the id information into the result object
    //Each cluster has the id of its position in the matrix
    ivector tids(nbClusters);
    for (i=0; i<nbClusters; i++) {
      tids.at(i)=i;
    }
    outTemplate=outputTemplate(tids);
    return ok;


  }





}
