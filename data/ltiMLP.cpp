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
 * file .......: ltiMLP.cpp
 * authors ....: Bastian Ibach, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 13.8.2002
 * revisions ..: $Id: ltiMLP.cpp,v 1.11 2006/09/05 09:59:23 ltilib Exp $
 */

#include <limits>
#include "ltiMLP.h"
#include "ltiClassName.h"
#include "ltiUniformDist.h"
#include "ltiScramble.h"
#include <cstdio>
// #define _LTI_DEBUG 1

#ifdef _LTI_DEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif


namespace lti {
  // --------------------------------------------------
  // MLP static members
  // --------------------------------------------------

  const MLP::activationFunctor *const MLP::parameters::activationFunctors[] = {
    new MLP::linearActFunctor, new MLP::signFunctor, new MLP::sigmoidFunctor, 0
  };

  objectFactory<MLP::activationFunctor>
  MLP::parameters::objFactory(activationFunctors);


  // --------------------------------------------------
  // MLP::parameters
  // --------------------------------------------------



  // default constructor
  MLP::parameters::parameters()
    : supervisedInstanceClassifier::parameters() {

    trainingMode = ConjugateGradients;
    batchMode = bool(true);
    momentum = double(0.0);
    hiddenUnits = ivector(1,4);
    learnrate = float(0.1);
    maxNumberOfEpochs = int(500);
    stopError = double(0.005);

    activationFunctions.resize(hiddenUnits.size()+1);
    for (unsigned int i=0;i<activationFunctions.size();++i) {
      activationFunctions[i] = new MLP::sigmoidFunctor;
    }
  }

  // copy constructor
  MLP::parameters::parameters(const parameters& other)
    : supervisedInstanceClassifier::parameters()  {
    copy(other);
  }

  // destructor
  MLP::parameters::~parameters() {
    for (unsigned int i=0;i<activationFunctions.size();++i) {
      delete activationFunctions[i];
      activationFunctions[i] = 0;
    }
  }

  bool MLP::parameters::setLayers(const int hidden,
                                  const activationFunctor& activ) {

    unsigned int i;

    for (i=0;i<activationFunctions.size();++i) {
      delete activationFunctions[i];
      activationFunctions[i] = 0;
    }

    const int layers = 2;
    hiddenUnits.resize(1,hidden,false,true);
    activationFunctions.resize(layers,0);
    for (i=0;i<activationFunctions.size();++i) {
      activationFunctions[i] = activ.clone();
    }

    return true;
  }

  bool MLP::parameters::setLayers(const activationFunctor& activ) {
    unsigned int i;

    for (i=0;i<activationFunctions.size();++i) {
      delete activationFunctions[i];
      activationFunctions[i] = 0;
    }

    const int layers = 1;
    hiddenUnits.resize(0);
    activationFunctions.resize(layers,0);
    for (i=0;i<activationFunctions.size();++i) {
      activationFunctions[i] = activ.clone();
    }

    return true;
  }

  bool MLP::parameters::setLayerActivation(const int layer,
                                           const activationFunctor& aFct){

    const unsigned int layers = hiddenUnits.size()+1;

    if ( layers < activationFunctions.size() ) {
      // delete the rest of the members
      unsigned int i;
      for (i=layers;i<activationFunctions.size();++i) {
        delete activationFunctions[i];
        activationFunctions[i]=0;
      }
    }

    activationFunctions.resize(layers,0);

    if ((layer>0) && (static_cast<unsigned int>(layer)<layers)) {
      delete activationFunctions[layer];
      activationFunctions[layer] = aFct.clone();
      return true;
    }

    return false;
  }


  // get type name
  const char* MLP::parameters::getTypeName() const {
    return "MLP::parameters";
  }

  // copy member
  MLP::parameters&
    MLP::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    supervisedInstanceClassifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    supervisedInstanceClassifier::parameters& (supervisedInstanceClassifier::parameters::* p_copy)
      (const supervisedInstanceClassifier::parameters&) =
      supervisedInstanceClassifier::parameters::copy;
    (this->*p_copy)(other);
# endif

    trainingMode = other.trainingMode;
    batchMode = other.batchMode;
    momentum = other.momentum;
    hiddenUnits = other.hiddenUnits;
    learnrate = other.learnrate;
    maxNumberOfEpochs = other.maxNumberOfEpochs;
    stopError = other.stopError;

    unsigned int i;

    for (i=0;i<activationFunctions.size();++i) {
      delete activationFunctions[i];
      activationFunctions[i] = 0;
    }

    activationFunctions.resize(other.activationFunctions.size());

    for (i=0;i<activationFunctions.size();++i) {
      activationFunctions[i] = other.activationFunctions[i]->clone();
    }

    return *this;
  }

  // alias for copy member
  MLP::parameters&
    MLP::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  classifier::parameters* MLP::parameters::clone() const {
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
  bool MLP::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool MLP::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      switch(trainingMode) {
        case SteepestDescent:
          lti::write(handler,"trainingMode","SteepestDescent");
          break;
        case ConjugateGradients:
          lti::write(handler,"trainingMode","ConjugateGradients");
          break;
        default:
          lti::write(handler,"trainingMode","SteepestDescent");
      };

      lti::write(handler,"batchMode",batchMode);
      lti::write(handler,"momentum",momentum);
      lti::write(handler,"hiddenUnits",hiddenUnits);
      lti::write(handler,"learnrate",learnrate);
      lti::write(handler,"maxNumberOfEpochs",maxNumberOfEpochs);
      lti::write(handler,"stopError",stopError);

      className classNamer;

      unsigned int i;

      for (i=0;i<activationFunctions.size();++i) {
        lti::write(handler,"activationFunctorType",
                   classNamer.get(*activationFunctions[i]));
        lti::write(handler,"functor",*activationFunctions[i]);
      }

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && supervisedInstanceClassifier::parameters::write(handler,false);
# else
    bool (supervisedInstanceClassifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      supervisedInstanceClassifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool MLP::parameters::write(ioHandler& handler,
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
  bool MLP::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool MLP::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string str;
      lti::read(handler,"trainingMode",str);
      if (str == "ConjugateGradients") {
        trainingMode = ConjugateGradients;
      } else {
        trainingMode = SteepestDescent;
      }

      lti::read(handler,"batchMode",batchMode);
      lti::read(handler,"momentum",momentum);
      lti::read(handler,"hiddenUnits",hiddenUnits);
      lti::read(handler,"learnrate",learnrate);
      lti::read(handler,"maxNumberOfEpochs",maxNumberOfEpochs);
      lti::read(handler,"stopError",stopError);

      unsigned int i;

      for (i=0;i<activationFunctions.size();++i) {
        delete activationFunctions[i];
      }
      
      const int layers = hiddenUnits.size()+1;
      activationFunctions.resize(layers);
      std::string className;
      for (i=0;i<activationFunctions.size();++i) {
        lti::read(handler,"activationFunctorType",className);
        activationFunctions[i] = objFactory.newInstance(className);
        lti::read(handler,"functor",*activationFunctions[i]);
      }

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && supervisedInstanceClassifier::parameters::read(handler,false);
# else
    bool (supervisedInstanceClassifier::parameters::* p_readMS)(ioHandler&,const bool) =
      supervisedInstanceClassifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool MLP::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // MLP
  // --------------------------------------------------


  // default constructor
  MLP::MLP()
    : supervisedInstanceClassifier(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // copy constructor
  MLP::MLP(const MLP& other)
    : supervisedInstanceClassifier()  {
    copy(other);
  }

  // destructor
  MLP::~MLP() {
  }

  // returns the name of this type
  const char* MLP::getTypeName() const {
    return "MLP";
  }

  // copy member
  MLP&
  MLP::copy(const MLP& other) {
    supervisedInstanceClassifier::copy(other);

    //TODO: comment the attributes of your classifier
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  MLP& MLP::operator=(const MLP& other) {
    return (copy(other));
  }


  // clone member
  classifier* MLP::clone() const {
    return new MLP(*this);
  }

  // return parameters
  const MLP::parameters&
    MLP::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // write method
  bool MLP::write(ioHandler& handler,const bool complete) const {
    bool b(true);

    if (complete) {
      b = handler.writeBegin();
    }

    // write the standard data (output and parameters)
    supervisedInstanceClassifier::write(handler,false);

    if (b) {
      lti::write(handler,"inputs",inputs);
      lti::write(handler,"outputs",outputs);
      lti::write(handler,"weights",weights);

    }

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

  // read method
  bool MLP::read(ioHandler& handler,const bool complete) {
    bool b(true);

    if (complete) {
      b = handler.readBegin();
    }

    // read the standard data (output and parameters)
    supervisedInstanceClassifier::read(handler,false);

    if (b) {
      if (lti::read(handler,"inputs",inputs) && 
          lti::read(handler,"outputs",outputs)) {
        if (lti::read(handler,"weights",weights)) {
          b = initWeights(true); // initialize internal state keeping
          // the weights...
        } else {
          initWeights(false,-1,1);
          b = false;
        }
      } else {
        b = false;
      }
    }

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

  const dvector& MLP::getWeights() const {
    return weights;
  }

  // -------------------------------------------------------------------
  // The train-methods!
  // -------------------------------------------------------------------
  // Normal training method
  bool MLP::train(const dmatrix& data,
                  const ivector& ids) {
    checkHowManyOutputs(ids);
    inputs=data.columns();
    initWeights(false,-1,1);
    return train(weights,data,ids);
  }

  void MLP::checkHowManyOutputs(const ivector& ids) {
    // count how many different ids are present in the training set
    std::map<int,int> extToInt;
    std::map<int,int>::iterator it;

    int i,k;
    for (i=0,k=0;i<ids.size();++i) {
      it = extToInt.find(ids.at(i));
      if (it == extToInt.end()) {
        extToInt[ids.at(i)] = k;
        ++k;
      }
    }

    outputs = extToInt.size();
  }

  // TODO: comment your train method
  bool MLP::train(const dvector& theWeights,
                  const dmatrix& data,
                  const ivector& ids) {

    if (data.empty()) {
      setStatusString("Train data empty");
      return false;
    }


    if (ids.size()!=data.rows()) {
      std::string str;
      str = "dimensionality of IDs vector and the number of rows ";
      str+= "of the input matrix must have the same size.";
      setStatusString(str.c_str());
      return false;
    }


    // tracks the status of the training process.
    // if an error occurs set to false and use setStatusString()
    // however, training should continue, fixing the error as well as possible
    bool b=true;

    // vector with internal ids
    ivector newIds,idsLUT;
    newIds.resize(ids.size(),0,false,false);

    // map to get the internal Id to an external Id;
    std::map<int,int> extToInt;
    std::map<int,int>::iterator it;

    int i,k;
    for (i=0,k=0;i<ids.size();++i) {
      it = extToInt.find(ids.at(i));
      if (it != extToInt.end()) {
        newIds.at(i) = (*it).second;
      } else {
        extToInt[ids.at(i)] = k;
        newIds.at(i) = k;
        ++k;
      }
    }

    idsLUT.resize(extToInt.size());
    for (it=extToInt.begin();it!=extToInt.end();++it) {
      idsLUT.at((*it).second) = (*it).first;
    }

    // initialize the inputs and output units from the given data
    outputs = idsLUT.size();
    inputs  = data.columns();

    const parameters& param = getParameters();

    // display which kind of algorithm is to be used
    if (validProgressObject()) {
      getProgressObject().reset();
      std::string str("MLP: Training using ");
      switch(param.trainingMode) {
        case parameters::ConjugateGradients:
          str += "conjugate gradients";
          break;
        case parameters::SteepestDescent:
          str += "steepest descent";
          break;
        default:
          str += "unnamed method";
      }
      getProgressObject().setTitle(str);
      getProgressObject().setMaxSteps(param.maxNumberOfEpochs+1);
    }

    dvector grad;
    if (&theWeights != &weights) {
      weights.copy(theWeights);
    }

    if (!initWeights(true)) { // keep the weights
      setStatusString("Wrong weights!");
      return false;
    };

    computeErrorNorm(newIds);

    if (param.trainingMode == parameters::ConjugateGradients) {
      b = trainConjugateGradients(data,newIds);
    } else {
      if (param.batchMode) { // batch training mode:
        b = trainSteepestBatch(data,newIds);
      } else { // sequential training mode:
        b = trainSteepestSequential(data,newIds);
      }
    }

    if (validProgressObject()) {
      getProgressObject().step("Training ready.");
    }

    outputTemplate tmpOutTemp(idsLUT);
    setOutputTemplate(tmpOutTemp);

    // create the appropriate outputTemplate
    makeOutputTemplate(outputs,data,ids);

    return b;
  }

  bool MLP::trainSteepestBatch(const dmatrix& data,
                               const ivector& internalIds) {

    const parameters& param = getParameters();
    char buffer[256];
    bool abort = false;
    int i;

    if (param.momentum > 0) {
      // with momentum
      dvector grad,delta(weights.size(),0.0);

      // train with all epochs
      for (i=0;!abort && (i<param.maxNumberOfEpochs);++i) {

        calcGradient(data,internalIds,grad);
        delta.addScaled(param.learnrate,grad,param.momentum,delta);
        weights.add(delta);

        // update progress info object
        if (validProgressObject()) {
          sprintf(buffer,"Error=%f",totalError/errorNorm);
          getProgressObject().step(buffer);
          abort = abort || (totalError/errorNorm <= param.stopError);
          abort = abort || getProgressObject().breakRequested();
        }
      }
    } else {
      // without momentum
      dvector grad;

      // train with all epochs
      for (i=0;!abort && (i<param.maxNumberOfEpochs);++i) {

        calcGradient(data,internalIds,grad);
        weights.addScaled(param.learnrate,grad);

        // update progress info object
        if (validProgressObject()) {
          sprintf(buffer,"Error=%f",totalError/errorNorm);
          getProgressObject().step(buffer);
          abort = abort || (totalError/errorNorm <= param.stopError);
          abort = abort || getProgressObject().breakRequested();
        }
      }
    }

    return true;
  }

  bool MLP::trainSteepestSequential(const dmatrix& data,
                                    const ivector& internalIds) {

    const parameters& param = getParameters();
    char buffer[256];
    bool abort = false;
    scramble<int> scrambler;
    int i,j,k;
    double tmpError;
    ivector idx;
    idx.resize(data.rows(),0,false,false);
    for (i=0;i<idx.size();++i) {
      idx.at(i)=i;
    }

    if (param.momentum > 0) {
      // with momentum
      dvector grad,delta(weights.size(),0.0);

      for (i=0; !abort && (i<param.maxNumberOfEpochs); ++i) {
        scrambler.apply(idx); // present the pattern in a random sequence
        totalError = 0;
        for (j=0;j<idx.size();++j) {
          k=idx.at(j);
          calcGradient(data.getRow(k),internalIds.at(k),grad);
          computeActualError(internalIds.at(k),tmpError);
          totalError+=tmpError;
          delta.addScaled(param.learnrate,grad,param.momentum,delta);
          weights.add(delta);
        }

        // update progress info object
        if (validProgressObject()) {
          sprintf(buffer,"Error=%f",totalError/errorNorm);
          getProgressObject().step(buffer);
          abort = abort || (totalError/errorNorm <= param.stopError);
          abort = abort || getProgressObject().breakRequested();
        }
      }
    } else {
      // without momentum
      ivector idx;
      idx.resize(data.rows(),0,false,false);
      dvector grad;

      int i,j,k;
      double tmpError;
      for (i=0;i<idx.size();++i) {
        idx.at(i)=i;
      }
      for (i=0; !abort && (i<param.maxNumberOfEpochs); ++i) {
        scrambler.apply(idx); // present the pattern in a random sequence
        totalError = 0;
        for (j=0;j<idx.size();++j) {
          k=idx.at(j);
          calcGradient(data.getRow(k),internalIds.at(k),grad);
          computeActualError(internalIds.at(k),tmpError);
          totalError+=tmpError;
          weights.addScaled(param.learnrate,grad);
        }

        // update progress info object
        if (validProgressObject()) {
          sprintf(buffer,"Error=%f",totalError/errorNorm);
          getProgressObject().step(buffer);
          abort = abort || (totalError/errorNorm <= param.stopError);
          abort = abort || getProgressObject().breakRequested();
        }
      }
    }
    return true;
  }

  bool MLP::trainConjugateGradients(const dmatrix& data,
                                    const ivector& internalIds) {

    const parameters& param = getParameters();
    char buffer[256];
    bool abort = false;
    int i;

    // without momentum
    dvector gradTau,gradTauP1,r;
    double beta(0.0),eta,absg0,tmp;
    dvector newWeights;

    double bestError;
    dvector bestWeights(weights);

    // compute the negative gradient
    calcGradient(data,internalIds,gradTau);
    r.copy(gradTau);
    bestError = totalError;

#if  defined(_LTI_DEBUG) && (_LTI_DEBUG > 4)
    cout << "grad 0: \t" << gradTau << endl;
#endif

    absg0 = param.stopError*param.stopError*gradTau.dot(gradTau);

    // train with all epochs
    for (i=0;!abort && (i<param.maxNumberOfEpochs);++i) {

      lineSearch(data,internalIds,r,eta,newWeights);

      // exchanging the weights content and updating the weight matrices is
      // faster than copying the whole newWeights vector!
      weights.exchange(newWeights);
      updateWeightIndices(weights,matWeights); // matWeights must
                                               // share the memory
                                               // used by weights, and
                                               // with exchange we
                                               // changed that!

      calcGradient(data,internalIds,gradTauP1);

#if  defined(_LTI_DEBUG) && (_LTI_DEBUG > 4)
      cout << "grad " << i+1 << ": \t" << gradTauP1 << endl;
#endif

      if (gradTauP1.dot(gradTauP1) < absg0) {
        // gradient about zero -> local minimum reached.
        // check if it is good enough

        if (totalError/errorNorm > param.stopError) {
          // gradient is small enough, but the error is still too big!
          // we are in a local minimum! -> retry with other weights

          if (totalError < bestError) {
            // but keep first actual weights as the best so far
            bestError = totalError;
            bestWeights.fill(weights);
          }

          initWeights(false,-1,1);
          // compute the negative gradient
          calcGradient(data,internalIds,gradTau);
          r.copy(gradTau);
          absg0 = param.stopError*param.stopError*gradTau.dot(gradTau);
          beta=0.0;
        } else {
          // minimum is good enough!
          abort = true;
        }
      } else {
        tmp = gradTau.dot(gradTau);
        gradTau.subtract(gradTauP1);
        beta = max(0.0,-gradTauP1.dot(gradTau)/tmp);
        r.addScaled(gradTauP1,beta,r);
        gradTau.exchange(gradTauP1);
      }

      // update progress info object
      if (validProgressObject()) {
        if (beta != 0.0) {
          sprintf(buffer,"Error=%f",totalError/errorNorm);
        } else {
          sprintf(buffer,"Error=%f (*)",totalError/errorNorm);
        }
        getProgressObject().step(buffer);
        abort = abort || getProgressObject().breakRequested();
      }
    }

    if (!abort) {
      // maximal iterations reached!
      if (totalError > bestError) {
        // get the best weights found
        weights.copy(bestWeights);

        // update progress info object
        if (validProgressObject()) {
          sprintf(buffer,"Max Iteration reached, best error=%f",
                  bestError/errorNorm);
          getProgressObject().step(buffer);
        }
      }
    }

    return true;
  }

  /*
   * line search computes the eta scalar factor at which the error
   * is minimized.  It begins at the actual weight and follows the given
   * direction.
   */
  bool MLP::lineSearch(const dmatrix& inputs,
                       const ivector& ids,
                       const dvector& direction,
                       double& xmin,
                       dvector& newWeights) const {

    // following algorithms are based on Press, W.H. et.al. Numerical
    // Recipies in C. Chapter 10, Minimization or Maximization of
    // Functions. pp. 397ff

    static const double gold = 1.618034; // golden mean
    static const double glimit = 100.0;  // maximum magnification
                                         // allowed for parabolic-fit
                                         // step
    static const double tiny = 1.0e-20;
    static const int itmax = 100; // maximum allowed number of
                                  // iterations to find minimum
    static const double cgold = 0.3819660; // golden ration
    static const double zeps = 1.0e-10; // small number that protects
                                        // against trying to achieve
                                        // fractional accuracy for a
                                        // minimum that happens to be
                                        // exaclty 0


    const parameters& param = getParameters();
    const int layers = param.hiddenUnits.size()+1;
    std::vector<dmatrix> mWeights(layers);
    newWeights.copy(weights);
    updateWeightIndices(newWeights,mWeights);

    // -----------------------------------------------------------------------
    //  Initial Bracket
    // -----------------------------------------------------------------------

    double ax(0.0),bx(1.0),cx;
    double fa,fb,fc;
    double ulim,u,r,q,fu,dum;

    // evaluate error at eta=0 and eta=1
    computeTotalError(mWeights,inputs,ids,fa);
    newWeights.add(direction);
    computeTotalError(mWeights,inputs,ids,fb);

    if (fb > fa) { // switch roles of a and b so that we can go
                   // downhill in the direction from a to b
      ax=1.0;
      bx=0.0;
      dum=fa; fa=fb; fb=dum;
    }

    // first guess for c:
    cx = bx + gold*(bx-ax);
    newWeights.addScaled(weights,cx,direction);
    computeTotalError(mWeights,inputs,ids,fc);

    while (fb>fc) { // keep returning here until we bracket
      r=(bx-ax)*(fb-fc);  // Compute u by parabolic extrapolation from a,b,c.
      q=(bx-cx)*(fb-fa);
      if (q>r) {          // tiny is used to prevent any posible division by 0
        u=bx-((bx-cx)*q-(bx-ax)*r)/(2.0*(max(q-r,tiny)));
      } else {
        u=bx-((bx-cx)*q-(bx-ax)*r)/(-2.0*(max(r-q,tiny)));
      }

      ulim=bx+glimit*(cx-bx);
      // We won't go farther than this.  Test various possibilities:
      if ((bx-u)*(u-cx) > 0.0) {
        newWeights.addScaled(weights,u,direction);
        computeTotalError(mWeights,inputs,ids,fu);
        if (fu < fc) { // Got a minimum between b and c
          ax=bx;
          bx=u;
          fa=fb;
          fb=fu;
          break;
        } else if (fu > fb) { // Got a minimum between a and u
          cx=u;
          fc=fu;
          break;
        }
        u=cx+gold*(cx-bx); // Parabolic fit was no use. Use default
                           // magnification
        newWeights.addScaled(weights,u,direction);
        computeTotalError(mWeights,inputs,ids,fu);
      } else if ((cx-u)*(u-ulim) > 0.0) {
        // Parabolic fit is between c and its allowed limit.
        newWeights.addScaled(weights,u,direction);
        computeTotalError(mWeights,inputs,ids,fu);
        if (fu < fc) {
          bx=cx;
          cx=u;
          u=cx+gold*(cx-bx);
          fb=fc;
          fc=fu;
          newWeights.addScaled(weights,u,direction);
          computeTotalError(mWeights,inputs,ids,fu);
        }
      } else if ((u-ulim)*(ulim-cx) >= 0.0) { // Limit parabolic u to
                                             // maximum allowed value
        u=ulim;
        newWeights.addScaled(weights,u,direction);
        computeTotalError(mWeights,inputs,ids,fu);
      } else { // Reject parabolic u, use default magnification
        u=cx+gold*(cx-bx);
        newWeights.addScaled(weights,u,direction);
        computeTotalError(mWeights,inputs,ids,fu);
      }

      // Eliminate oldest point and continue
      ax=bx;
      bx=cx;
      cx=u;
      fa=fb;
      fb=fc;
      fc=fu;
    }

    // -----------------------------------------------------------------------
    //  Line search: Brent's method
    // -----------------------------------------------------------------------

    // fractional precision of found minimum
    static const double tol =
      2.0*sqrt(std::numeric_limits<double>::epsilon());
    double fv,fw,fx,etemp;
    double p,tol1,tol2,v,w,x,xm,a,b,d=0.0;
    int iter;

    // This will be the distance moved on the step before last
    double e=0.0;
    // a and b must be in ascending order, but input abscissas need not be.
    if (ax < cx) {
      a = ax;
      b = cx;
    } else {
      a = cx;
      b = ax;
    }
    // Initializations...
    x=w=v=bx;
    fw=fv=fx=fb;

    for (iter=1;iter<=itmax;++iter) { // main loop
      xm=0.5*(a+b);
      tol2=2.0*(tol1=tol*abs(x)+zeps);
      // test for done here
      if (abs(x-xm) <= (tol2-0.5*(b-a))) {
        xmin = x;
        newWeights.addScaled(weights,xmin,direction);
        return true;
      }
      if (abs(e) > tol1) {
        r=(x-w)*(fx-fv);
        q=(x-v)*(fx-fw);
        p=(x+v)*q - (x-w)*r;
        q=2.0*(q-r);
        if (q > 0.0) p = -p;
        q=abs(q);
        etemp=e;
        e=d;
        if (abs(p) >= abs(0.5*q*etemp) ||
            p <= q*(a-x) ||
            p >= q*(b-x)) {
          d = cgold*(e = (x >= xm ? a-x : b-x));
        }

        // The above conditions determine the acceptability of the
        // parabolic fit.  Here we take the golden section step into
        // the larger of the two segments

        else {
          d=p/q;
          u=x+d;
          if (u-a < tol2 || b-u < tol2) {
            d=(xm>=x)?abs(tol1):-abs(tol1);
          }
        }
      } else {
        d = cgold*(e=(x>=xm ? a-x : b-x));
      }
      u=(abs(d) >= tol1 ? x+d : x+(d>0.0?abs(tol1):-abs(tol1)));
      // This is the one function evaluation per iteration
      newWeights.addScaled(weights,u,direction);
      computeTotalError(mWeights,inputs,ids,fu);

      // Now decide what to do with our function evaluation
      if (fu < fx) {
        if (u>=x) a=x; else b=x;
        // Housekeeping follows:
        v=w;
        w=x;
        x=u;
        fv=fw;
        fw=fx;
        fx=fu;
      } else {
        if (u<x) a=u; else b=u;
        if (fu <= fw || w == x) {
          v=w;
          w=u;
          fv=fw;
          fw=fu;
        } else if (fu <= fv || v == x || v == w) {
          v=u;
          fv=fu;
        }
      }  // done with housekeeping.  Back for another iteration
    } // end of main loop.

    setStatusString("Too many iterations in brent line search");
    xmin=x;
    newWeights.addScaled(weights,xmin,direction);
    return false;
  }



  // -------------------------------------------------------------------
  // The classify-method!
  // -------------------------------------------------------------------

  //TODO Usually this method does not need to be implemented use inherited
  // method from direct subclass of classifier (ie centroidClassifier) instead
  // In this case delete. Otherwise comment!

  bool MLP::classify(const dvector& feature,
                     outputVector& result) const {

    // tResult is a vector that results from your classification process.
    // rename if desired.
    propagate(feature);

    if (outTemplate.apply(unitsOut[unitsOut.size()-1], result)) {
      result.setWinnerAtMax();
      return true;
    };

    return false;
  }

  /*
   * initialize weights with random values
   *
   */
  bool MLP::initWeights(const bool keepWeightVals,
                        const double& from,
                        const double& to) {

    const parameters& param = getParameters();
    const int layers = param.hiddenUnits.size()+1;
    int i;

    // count the number of weights required
    int weightsSize(0);

    matWeights.clear();
    matWeights.resize(layers);

    layerIndex.resize(layers,0);

    ivector units(layers+1);

    // fill the units vector beginning with the second element with the
    // number of hidden units in each layer.
    units.fill(param.hiddenUnits,1);
    units.at(0)=inputs;
    units.at(layers)=outputs;

    for (i=0;i<layers;++i) {
      layerIndex.at(i)=weightsSize;
      weightsSize+=((units.at(i+1))*(units.at(i)+1));
    }

    // resize the weights vector
    if (keepWeightVals) {
      if (weightsSize != weights.size()) {
        return false;
      }
    } else {
      weights.resize(weightsSize,0,false,false);
    }

    // create the weight matrices, using the same memory regions in
    // the weights vector
    for (i=0;i<layers;++i) {
      matWeights[i].useExternData(units.at(i+1),units.at(i)+1,
                                  &weights[layerIndex[i]]);
    }

    if (!keepWeightVals) {
      // initialize the matrix with random values
      uniformDistribution unif(from,to);
      for (i=0;i<weights.size();++i) {
        weights.at(i)=unif.draw();
      }
    }

    unitsOut.resize(layers);
    unitsNet.resize(layers);

    off = param.activationFunctions[layers-1]->offValue();
    on  = param.activationFunctions[layers-1]->onValue();

    return true;
  }

   /*
    * Given the weights vector, update the vector of matrices so that
    * each matrix uses its respective memory block in the vector.
    */
  void MLP::updateWeightIndices(dvector& theWeights,
                                std::vector<dmatrix>& theWMats) const {

    // create the weight matrices, using the same memory regions in
    // the weights vector
    for (unsigned int i=0;i<theWMats.size();++i) {
      const point& size = matWeights[i].size();
      theWMats[i].useExternData(size.y,size.x,
                                &theWeights[layerIndex[i]]);
    }
  }

  /*
   * calculate output of Units
   * @param weights vector
   * @param activFunctor, the activation function to use
   */
  bool MLP::propagate(const dvector& input) const {
    unsigned int i;
    const parameters& param = getParameters();
    i=0;

    biasMultiply(matWeights[i],input,unitsNet[i]);
    param.activationFunctions[i]->apply(unitsNet[i],unitsOut[i]);
#if  defined(_LTI_DEBUG) && (_LTI_DEBUG > 9)
    cout << "Output of Layer " << i << ":" << endl;
    cout << unitsOut[i] << endl;
#endif

    for (i=1;i<unitsOut.size();++i) {
      biasMultiply(matWeights[i],unitsOut[i-1],unitsNet[i]);
      param.activationFunctions[i]->apply(unitsNet[i],unitsOut[i]);

#if  defined(_LTI_DEBUG) && (_LTI_DEBUG > 9)
      cout << "Output of Layer " << i << ":" << endl;
      cout << unitsOut[i] << endl;
#endif
    }

    return true;
  }

  /*
   * calculate output of Units
   * @param weights vector
   * @param activFunctor, the activation function to use
   */
  bool MLP::propagate(const dvector& input,
                      const std::vector<dmatrix>& mWeights,
                      std::vector<dvector>& uNet,
                      std::vector<dvector>& uOut) const {
    unsigned int i;
    const parameters& param = getParameters();
    const int layers = param.hiddenUnits.size()+1;
    if (static_cast<int>(uOut.size()) != layers) {
      uNet.resize(layers);
      uOut.resize(layers);
    }

    i=0;
    biasMultiply(mWeights[i],input,uNet[i]);
    param.activationFunctions[i]->apply(uNet[i],uOut[i]);

    for (i=1;i<uOut.size();++i) {
      biasMultiply(mWeights[i],uOut[i-1],uNet[i]);
      param.activationFunctions[i]->apply(uNet[i],uOut[i]);
    }

    return true;
  }

  /*
   * compute mat*vct' where vct' is a vector with one additional element
   * (1.0) at the beginning of vct.
   */
  bool MLP::biasMultiply(const dmatrix& mat,
                         const dvector& vct,
                         dvector& res) const {
    int j;
    dmatrix::const_iterator it,eit;
    dvector::iterator rit;
    dvector::const_iterator vit,evit;

    res.resize(mat.rows(),0.0,false,false);
    it = mat.begin();
    eit = mat.end();
    rit = res.begin();
    evit = vct.end();

    for (j=0;j<mat.rows();++j,++rit) {
      *rit = *it;
      ++it;
      for (vit=vct.begin();vit!=evit;++it,++vit) {
        *rit += (*vit)*(*it);
      }
    }

    return true;
  }

  bool MLP::calcGradient(const dmatrix& inputs,
                         const ivector& ids,
                         dvector& grad) {

    if (inputs.rows() != ids.size()) {
      setStatusString("Number of vectors not consistent with number of ids");
      return false;
    }

    dvector tmp;
    int i;
    double tmpError;

    totalError = 0;
    calcGradient(inputs.getRow(0),ids.at(0),grad);
    computeActualError(ids.at(0),totalError);

    for (i=1;i<inputs.rows();++i) {
      calcGradient(inputs.getRow(i),ids.at(i),tmp);
      computeActualError(ids.at(i),tmpError);
      grad.add(tmp);
      totalError+=tmpError;
    }

    return true;
  }

   /*
    * compute the error using the last propagated input and the given
    * pattern
    */
  bool MLP::computeActualError(const int id,double& error) const {
    return computePatternError(id,unitsOut.back(),error);
  }

   /*
    * compute the error using the last propagated input and the given
    * pattern
    */
  bool MLP::computePatternError(const int id,
                                const dvector& outUnits,
                                double& error) const {

    const int lastIdx = outUnits.size();
    int j;

    double tmp;
    error = 0.0;
    for (j=0;j<lastIdx;++j) {
      tmp = (outUnits.at(j)-((j==id)?on:off));
      error += tmp*tmp;
    }

    error *= 0.5;

    return true;
  }

  /*
   * compute the error of the given weights for the whole training set.
   */
  bool MLP::computeTotalError(const std::vector<dmatrix>& mWeights,
                              const dmatrix& inputs,
                              const ivector& ids,
                              double& totalError) const {

    if (ids.size() != inputs.rows()) {
      return false;
    }

    const parameters& param = getParameters();
    const int layers = param.hiddenUnits.size()+1;
    std::vector<dvector> uNet(layers),uOut(layers);
    int i;
    double tmp;
    totalError=0.0;
    for (i=0;i<ids.size();++i) {
      propagate(inputs.getRow(i),mWeights,uNet,uOut);
      computePatternError(ids.at(i),uOut.back(),tmp);
      totalError+=tmp;
    }

    return true;
  }


  /*
   * calculate gradient of error surface using back-propagation algorithm
   *
   * @param input input vector
   * @param outputId desired output.  This value must be between 0 and
   *                 the number of output elements-1.
   * @param grad computed gradient of the error surface
   * @return true if successful, or false otherwise.
   */
  bool MLP::calcGradient(const dvector& input,
                         const int outputId,
                         dvector& grad) {

    const parameters& param = getParameters();
    const int layers = param.hiddenUnits.size()+1;

    propagate(input);
    grad.resize(weights.size());

    int i,j,jj,k,idx,lastIdx;
    int layer = layerIndex.size()-1;

    double delta;

    // compute f'(net) at unitsNet
    for (i=0;i<layers;++i) {
      param.activationFunctions[i]->deriv(unitsNet[i]);
    }

    // ---------------------------------------------
    // gradient for the elements of the output layer
    // ---------------------------------------------

    const dmatrix& outMat = matWeights[layer];

    const dvector* theInput = 0;
    if (layer>0) {
      theInput = &unitsOut[layer-1];
    } else {
      theInput = &input;
    }

    idx = layerIndex.at(layer);

    dvector lastDeltas,newDeltas;

    lastDeltas.resize(outMat.rows(),0,false,false);

    for (j=0;j<outMat.rows();++j) {
      delta = ((((j==outputId)?on:off) - unitsOut[layer].at(j)) *
               unitsNet[layer].at(j));

      lastDeltas.at(j)=delta;

      grad.at(idx)=delta; // bias = 1.0
      ++idx;
      for (i=0;i<theInput->size();++i,++idx) {
        // idx means layerIndex.at(layer)+i+j*ROWS
        grad.at(idx) = delta*theInput->at(i);
      }
    }

    // ----------------------------------------------
    // gradient for the elements of the hidden layers
    // ----------------------------------------------
    --layer;
    while (layer>=0) {

      const dmatrix& outMat = matWeights[layer];
      const dmatrix& lastMat = matWeights[layer+1];
      const dvector* theInput = 0;

      if (layer>0) {
        theInput = &unitsOut[layer-1];
      } else {
        theInput = &input;
      }

      idx = layerIndex.at(layer);
      lastIdx = theInput->size();

      newDeltas.resize(outMat.rows(),0.0,false,false);
      for (j=0,jj=1;j<outMat.rows();++j,++jj) {
        delta = 0;
        for (k=0;k<lastMat.rows();++k) {
          delta+=(lastDeltas.at(k)*lastMat.at(k,jj));
        }
        delta*=unitsNet[layer].at(j);
        newDeltas.at(j)=delta;

        grad.at(idx)=delta; // bias = 1.0
        ++idx;
        for (i=0;i<lastIdx;++i,++idx) {
          // idx means layerIndex.at(layer)+i+j*ROWS
          grad.at(idx) = delta*theInput->at(i);
        }
      }

      newDeltas.detach(lastDeltas);

      // continue with next layer
      --layer;
    };

    return true;
  }

  bool MLP::computeErrorNorm(const ivector& ids) {

    dvector vct(outputs,off*ids.size());

    const double fix=on-off;
    int i;
    double tmp,tmp2,v;

    // compute first the average outputs for the training set

    for (i=0;i<ids.size();++i) {
      vct.at(ids.at(i))+=fix;
    }

    vct.divide(ids.size());
    double offError(0.0);

    // now compute the error
    for (i=0;i<vct.size();++i) {
      tmp = off - vct.at(i);
      offError += (tmp*tmp);
    }

    errorNorm = 0.0;
    for (i=0;i<ids.size();++i) {
      v = vct.at(ids.at(i));
      tmp  = off - v;
      tmp2 = on  - v;
      errorNorm += (offError - tmp*tmp + tmp2*tmp2);
    }
    errorNorm *= 0.5;

    return true;
  }

  /*
   * Undocumented function for debug purposes only.  It display the
   * internal weight matrices
   */
  bool MLP::prettyPrint() {
#ifdef _LTI_DEBUG
    const parameters& param = getParameters();
    const int layers = param.hiddenUnits.size()+1;

    unsigned int i;
    cout << "--------------------------------------------------------" << endl;
    cout << "Neural Network: " << layers << " layers" << endl;
    cout << "Inputs : " << inputs << endl;
    cout << "Outputs: " << outputs << endl;
    cout << "Units in hidden layers: " << param.hiddenUnits << endl;
    cout << "Weight matrices: " << endl;
    for (i=0;i<matWeights.size();++i) {
      cout << "Layer " << i << ":" << endl;
      cout << matWeights[i] << endl;
    }
    cout << "--------------------------------------------------------" << endl;
    cout << endl;
#endif
    return true;
  }

  /**
   * write the functor::parameters in the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler, const MLP::activationFunctor& p,
             const bool complete) {
    return p.write(handler,complete);
  }

  /**
   * read the functor::parameters from the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool read(ioHandler& handler,MLP::activationFunctor& p,
            const bool complete) {
    return p.read(handler,complete);
  }

}
