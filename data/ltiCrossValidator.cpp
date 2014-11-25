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
 * file .......: ltiCrossValidator.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 25.6.2002
 * revisions ..: $Id: ltiCrossValidator.cpp,v 1.8 2006/09/05 09:57:49 ltilib Exp $
 */

//TODO: include files
#include "ltiCrossValidator.h"

#include "ltiObjectFactory.h"
#include "ltiSupervisedInstanceClassifier.h"
#include "ltiRbf.h"

namespace lti {
  // --------------------------------------------------
  // crossValidator::parameters
  // --------------------------------------------------

  uniformDistribution lti::crossValidator::randomGenerator;

  // default constructor
  crossValidator::parameters::parameters() 
    : functor::parameters() {    
    
    nbOfSplits = 5;
    classify = new rbf();
  }

  // copy constructor
  crossValidator::parameters::parameters(const parameters& other) {
    copy(other);
  }
  
  // destructor
  crossValidator::parameters::~parameters() {    
  }

  // get type name
  const char* crossValidator::parameters::getTypeName() const {
    return "crossValidator::parameters";
  }
  
  // copy member

  crossValidator::parameters& 
    crossValidator::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters& (functor::parameters::* p_copy)
      (const functor::parameters&) = 
      functor::parameters::copy;
    (this->*p_copy)(other);
# endif
    
    
      nbOfSplits = other.nbOfSplits;
      classify = other.classify;

    return *this;
  }

  // alias for copy member
  crossValidator::parameters& 
    crossValidator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* crossValidator::parameters::clone() const {
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
  bool crossValidator::parameters::write(ioHandler& handler,
                                         const bool complete) const 
# else
  bool crossValidator::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const 
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"nbOfSplits",nbOfSplits);
      classify->write(handler,false);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to 
    // compile...
    b = b && functor::parameters::write(handler,false);
# else
    bool (functor::parameters::* p_writeMS)(ioHandler&,const bool) const = 
      functor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool crossValidator::parameters::write(ioHandler& handler,
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
  bool crossValidator::parameters::read(ioHandler& handler,
                                        const bool complete) 
# else
  bool crossValidator::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"nbOfSplits",nbOfSplits);
      classify->read(handler,false);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to 
    // compile...
    b = b && functor::parameters::read(handler,false);
# else
    bool (functor::parameters::* p_readMS)(ioHandler&,const bool) = 
      functor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool crossValidator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // crossValidator
  // --------------------------------------------------

  // default constructor
  crossValidator::crossValidator()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // copy constructor
  crossValidator::crossValidator(const crossValidator& other) {
    copy(other);
  }

  // destructor
  crossValidator::~crossValidator() {
  }

  // returns the name of this type
  const char* crossValidator::getTypeName() const {
    return "crossValidator";
  }

  // copy member
  crossValidator& crossValidator::copy(const crossValidator& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  crossValidator&
    crossValidator::operator=(const crossValidator& other) {
    return (copy(other));
  }


  // clone member
  functor* crossValidator::clone() const {
    return new crossValidator(*this);
  }

  // return parameters
  const crossValidator::parameters&
    crossValidator::getParameters() const {
    const parameters* par = 
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException();
    }
    return *par;
  }
  
  bool crossValidator::splitData(const dmatrix& data,
                                 std::list<ivector>& splittedData) const{

    int i,j;
    int nbSplits=getParameters().nbOfSplits;
    int nbDataPoints=data.rows();
    int dim=data.columns();
    int pointsPerSplit=nbDataPoints/nbSplits;
    int restPoints=nbDataPoints-pointsPerSplit*nbSplits;


    ivector index(nbDataPoints);
    for (i=0; i<nbDataPoints; i++) {
      index.at(i)=i;
    }

    int rValue;
    int pointsLeft=nbDataPoints;
    ivector* ptTmp = new ivector(pointsPerSplit);
    j=0; // counts the elements that are already in the vector
    for (i=0; i<nbDataPoints; i++) {
      
      // create temporary matrix, that build one split 
     if (j==ptTmp->size()) { // if ptTmp is full
        splittedData.push_back((*ptTmp));
        delete ptTmp;
        // if one more point must be added because the number of points in 
        // data can not be divided exactly in nbOfSplits parts
        if (nbSplits-(int)splittedData.size()-1>=restPoints) { 
          ptTmp = new ivector(pointsPerSplit);
          j=0; // restart counting
        } else {
          ptTmp = new ivector(pointsPerSplit+1,dim);
          j=0; // restart counting
        }
      }
//      double r=randomGenerator.draw();
//      rValue = static_cast<int>(r*pointsLeft);
     rValue = iround(randomGenerator.draw()*pointsLeft);
      ptTmp->at(j)=index.at(rValue);

      index.at(rValue)=index.at(pointsLeft-1);
      pointsLeft--;
      if (pointsLeft==0) {
        pointsLeft=nbDataPoints;
      }
      j++;
    }
    splittedData.push_back(*ptTmp); // pushback the last vector
    delete ptTmp;
    return true;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  double crossValidator::apply(const dmatrix& data, const ivector& ids) const {
    double d=0.;
    apply(data,ids,d);
    return d;
  }
  
    bool crossValidator::apply(const dmatrix& data, const ivector& ids,
                               double& averageRecogRate) const {
      parameters param=getParameters();
      averageRecogRate=0;
      int nbRecogs=0; // the number of classification that are already done
      int i,j;
      bool ok=true;
      // each vector will be filled with the ids of the corresponding 
      // rows in data matrix
      std::list<ivector> pointsInSplits; 

      int nbPoints=data.rows();
      int dim=data.columns();

      // returns a list of vectors; each vector represents one split; the 
      // numbers in the the vectors identify the position of the point in data
      ok = ok && splitData(data,pointsInSplits);
      // for each possible combination of training points, train the classifier
      // and test the recognition rate
      std::list<ivector>::iterator it,it2; // it shows the actual training data
      for (it=pointsInSplits.begin(); it!=pointsInSplits.end(); it++) {
        // faster with new instances in each iteration, instead of resizes
        dmatrix testData((*it).size(),dim); // the test data
        ivector testIds(testData.rows()); // the ids corresponding to the test
        dmatrix trainingData(nbPoints-testData.rows(),dim);
        ivector trainingIds(nbPoints-testData.rows());
        // get the testdata from given data
        for (j=0; j<testData.rows(); j++) { 
          testData.setRow(j,data.getRow((*it).at(j)));
          testIds.at(j)=ids.at((*it).at(j));
        }
        j=0;
        // copy all other points that are not in testData to the traingData
        for (it2=pointsInSplits.begin(); it2!=pointsInSplits.end(); it2++) {
          // exclude it2=it, because these points are in the testData. 
          if (it!=it2) {
            // Copy all other point, that are not in the test data into the 
            // training data set.
            for (i=0; i<(*it2).size(); i++) {
              trainingData.setRow(j,data.getRow((*it2).at(i)));
              trainingIds.at(j)=ids.at((*it2).at(i));
              j++;
            }
          }
        }
        // now the training and the test data is ready; start training
        // it is expected that the classifier restarts empty with each calling 
        // of the training function 
        param.classify->train(trainingData,trainingIds);
        supervisedInstanceClassifier::outputVector result;
        int winnerId; double value;
        // test each feature in testData and view if it has been correctly 
        // classified
        for (int i=0; i<testData.rows(); i++) {
          param.classify->classify(testData.getRow(i),result);
          result.maxPair(winnerId,value);
          if (winnerId==testIds.at(i)) {
            averageRecogRate++;
          }
          nbRecogs++;
        }
      }
      // compute the average recognition rate
      averageRecogRate=averageRecogRate/nbRecogs;
      return ok;
    };



  }
