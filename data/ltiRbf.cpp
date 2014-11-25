/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiRbf.cpp
 * authors ....: Peter Doerfler, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 11.8.2000
 * revisions ..: $Id: ltiRbf.cpp,v 1.9 2006/09/05 10:01:41 ltilib Exp $
 */

#include "ltiRbf.h"
#include "ltiL1Distance.h"
#include "ltiL2Distance.h"
#include "ltiSort.h"
#include "ltiScramble.h"
#include "ltiEquationSystem.h"
#include "ltiPseudoInverseMP.h"
#include "ltiLispStreamHandler.h"
#include "ltiBinaryStreamHandler.h"
#include <cstdio>
#include <iostream>
#include <fstream>


#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {


  // --------------------------------------------------
  // rbf::parameters
  // --------------------------------------------------

  // default constructor
  rbf::parameters::parameters()
    : classifier::parameters() {

    // --------------
    // the parameters
    // --------------

//      loadNet     = true;
//      trainNewNet = false;
//      convertNet  = false;

    doTrain2 = false;


//      binary = true;

//      ascii = false;

//      doStatistics = false;

//      convertBinToAscii = true;
//      noObjectProbs = false;
//      classifyStat = true;
//      mseStat = false;
//      mseSave = false;
//      firstBestSave = false;
//      threeBestSave = true;
//      alsoTrain = false;

    nbPresentations1 = 0;
    nbPresentations2 = 20;
    nbHiddenNeurons = 1;

    learnRate1 = 0.3;
    learnRate2 = 0.1;
    learnFactor = 0.3;
    windowSize = 0.2;
    sigma = 0.0;
    sigmaFactor = 1.6;
    lambda = 0.0;

    high = 0.99;

    norm = L2distance;
    lvqInitType = rbf::parameters::LvqMaxDist;

    lvqType1 = OLVQ1;
    lvqType2 = OLVQ3;

//      netTrainFile = "";
//      netTrainFile2 = "";
//      netTestFile = "" ;
//      netFilename = "net.rbf";
//      statisticsFile = "";
//      netFilename2 = "";
  }

  // copy constructor
  rbf::parameters::parameters(const parameters& other)
    : classifier::parameters()  {
    copy(other);
  }

  // destructor
  rbf::parameters::~parameters() {
  }

  // get type name
  const char* rbf::parameters::getTypeName() const {
    return "rbf::parameters";
  }

  // copy member
  rbf::parameters&
  rbf::parameters::copy(const parameters& other) {
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

//      loadNet     = other.loadNet;
//      trainNewNet = other.trainNewNet;
//      convertNet  = other.convertNet;

    doTrain2 = other.doTrain2;

//      binary = other.binary;

//      ascii = other.ascii;

//      doStatistics = other.doStatistics;

//      convertBinToAscii = other.convertBinToAscii;
//      noObjectProbs = other.noObjectProbs;
//      classifyStat = other.classifyStat;
//      mseStat = other.mseStat;
//      mseSave = other.mseSave;
//      firstBestSave = other.firstBestSave;
//      threeBestSave = other.threeBestSave;
//      alsoTrain = other.alsoTrain;

    nbPresentations1 = other.nbPresentations1;
    nbPresentations2 = other.nbPresentations2;
    nbHiddenNeurons = other.nbHiddenNeurons;

    learnRate1 = other.learnRate1;
    learnRate2 = other.learnRate2;
    learnFactor = other.learnFactor;
    windowSize = other.windowSize;
    sigma = other.sigma;
    sigmaFactor = other.sigmaFactor;
    lambda = other.lambda;


    high = other.high;

    norm = other.norm;
    lvqInitType = other.lvqInitType;

    lvqType1 = other.lvqType1;
    lvqType2 = other.lvqType2;

//      netTrainFile = other.netTrainFile;
//      netTrainFile2 = other.netTrainFile2;
//      netTestFile = other.netTestFile;
//      netFilename = other.netFilename;
//      statisticsFile = other.statisticsFile;
//      netFilename2 = other.netFilename2;

    return *this;
  }

  // clone member
  classifier::parameters* rbf::parameters::clone() const {
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
  bool rbf::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool rbf::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {

    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

//        lti::write(handler,"trainNewNet",trainNewNet);
//        lti::write(handler,"convertNet",convertNet);
      lti::write(handler,"doTrain2",doTrain2);
//        lti::write(handler,"loadNet",loadNet);
//        lti::write(handler,"doStatistics",doStatistics);
//        lti::write(handler,"binary",binary);
//        lti::write(handler,"ascii",ascii);
//        lti::write(handler,"convertBinToAscii",convertBinToAscii);
//        lti::write(handler,"noObjectProbs",noObjectProbs);
//        lti::write(handler,"classifyStat",classifyStat);
//        lti::write(handler,"mseStat",mseStat);
//        lti::write(handler,"mseSave",mseSave);
//        lti::write(handler,"firstBestSave",firstBestSave);
//        lti::write(handler,"threeBestSave",threeBestSave);
//        lti::write(handler,"alsoTrain",alsoTrain);

      lti::write(handler,"nbPresentations1",nbPresentations1);
      lti::write(handler,"nbPresentations2",nbPresentations2);
      lti::write(handler,"nbHiddenNeurons",nbHiddenNeurons);

      lti::write(handler,"learnRate1",learnRate1);
      lti::write(handler,"learnRate2",learnRate2);
      lti::write(handler,"learnFactor",learnFactor);
      lti::write(handler,"windowSize",windowSize);
      lti::write(handler,"sigma",sigma);
      lti::write(handler,"sigmaFactor",sigmaFactor);
      lti::write(handler,"lambda",lambda);

      lti::write(handler,"high",high);

      switch(norm) {
        case L1distance:
          lti::write(handler,"norm","L1distance");
          break;
        case L2distance:
          lti::write(handler,"norm","L2distance");
          break;
      }

      switch(lvqInitType) {
        case LvqRand:
          lti::write(handler,"lvqInitType","LvqRand");
          break;
        case LvqMaxDist:
          lti::write(handler,"lvqInitType","LvqMaxDist");
          break;
      }

      switch(lvqType1) {
        case NO_LVQ:
          lti::write(handler,"lvqType1","NO_LVQ");
          break;
        case LVQ1:
          lti::write(handler,"lvqType1","LVQ1");
          break;
        case OLVQ1:
          lti::write(handler,"lvqType1","OLVQ1");
          break;
        case LVQ3:
          lti::write(handler,"lvqType1","LVQ3");
          break;
        case OLVQ3:
          lti::write(handler,"lvqType1","OLVQ3");
          break;
        case LVQ4:
          lti::write(handler,"lvqType1","LVQ4");
          break;
      }

      switch(lvqType2) {
        case NO_LVQ:
          lti::write(handler,"lvqType2","NO_LVQ");
          break;
        case LVQ1:
          lti::write(handler,"lvqType2","LVQ1");
          break;
        case OLVQ1:
          lti::write(handler,"lvqType2","OLVQ1");
          break;
        case LVQ3:
          lti::write(handler,"lvqType2","LVQ3");
          break;
        case OLVQ3:
          lti::write(handler,"lvqType2","OLVQ3");
          break;
        case LVQ4:
          lti::write(handler,"lvqType2","LVQ4");
          break;
      }

//        lti::write(handler,"netTrainFile",netTrainFile);
//        lti::write(handler,"netTrainFile2",netTrainFile2);
//        lti::write(handler,"netTestFile",netTestFile);
//        lti::write(handler,"statisticsFile",statisticsFile);
//        lti::write(handler,"netFilename",netFilename);
//        lti::write(handler,"netFilename2",netFilename2);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && supervisedInstanceClassifier::parameters::write(handler,false);
# else
    bool (supervisedInstanceClassifier::parameters::* p_writeMS)
      (ioHandler&, const bool) const =
      supervisedInstanceClassifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool rbf::parameters::write(ioHandler& handler,
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
  bool rbf::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool rbf::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
//        lti::read(handler,"trainNewNet",trainNewNet);
//        lti::read(handler,"convertNet",convertNet);
      lti::read(handler,"doTrain2",doTrain2);
//        lti::read(handler,"loadNet",loadNet);
//        lti::read(handler,"doStatistics",doStatistics);
//        lti::read(handler,"binary",binary);
//        lti::read(handler,"ascii",ascii);
//        lti::read(handler,"convertBinToAscii",convertBinToAscii);
//        lti::read(handler,"noObjectProbs",noObjectProbs);
//        lti::read(handler,"classifyStat",classifyStat);
//        lti::read(handler,"mseStat",mseStat);
//        lti::read(handler,"mseSave",mseSave);
//        lti::read(handler,"firstBestSave",firstBestSave);
//        lti::read(handler,"threeBestSave",threeBestSave);
//        lti::read(handler,"alsoTrain",alsoTrain);

      lti::read(handler,"nbPresentations1",nbPresentations1);
      lti::read(handler,"nbPresentations2",nbPresentations2);
      lti::read(handler,"nbHiddenNeurons",nbHiddenNeurons);

      lti::read(handler,"learnRate1",learnRate1);
      lti::read(handler,"learnRate2",learnRate2);
      lti::read(handler,"learnFactor",learnFactor);
      lti::read(handler,"windowSize",windowSize);
      lti::read(handler,"sigma",sigma);
      lti::read(handler,"sigmaFactor",sigmaFactor);
      lti::read(handler,"lambda",lambda);

      lti::read(handler,"high",high);

      std::string str;
      lti::read(handler,"norm",str);

      if (str == "L1distance") {
        norm = L1distance;
      } else if (str == "L2distance") {
        norm = L2distance;
      } else {
        norm = L2distance;
      }

      lti::read(handler,"lvqInitType",str);

      if (str == "LvqRand") {
        lvqInitType = LvqRand;
      } else if (str == "LvqMaxDist") {
        lvqInitType = LvqMaxDist;
      } else {
        lvqInitType = LvqMaxDist;
      }

      lti::read(handler,"lvqType1",str);
      if (str == "NO_LVQ") {
        lvqType1 = NO_LVQ;
      } else if (str == "LVQ1") {
        lvqType1 = LVQ1;
      } else if (str == "OLVQ1") {
        lvqType1 = OLVQ1;
      } else if (str == "LVQ3") {
        lvqType1 = LVQ3;
      } else if (str == "OLVQ3") {
        lvqType1 = OLVQ3;
      } else if (str == "LVQ4") {
        lvqType1 = LVQ4;
      } else {
        lvqType1 = OLVQ1;
      }

      lti::read(handler,"lvqType2",str);
      if (str == "NO_LVQ") {
        lvqType2 = NO_LVQ;
      } else if (str == "LVQ1") {
        lvqType2 = LVQ1;
      } else if (str == "OLVQ1") {
        lvqType2 = OLVQ1;
      } else if (str == "LVQ3") {
        lvqType2 = LVQ3;
      } else if (str == "OLVQ3") {
        lvqType2 = OLVQ3;
      } else if (str == "LVQ4") {
        lvqType2 = LVQ4;
      } else {
        lvqType2 = OLVQ1;
      }

//        lti::read(handler,"netTrainFile",netTrainFile);
//        lti::read(handler,"netTrainFile2",netTrainFile2);
//        lti::read(handler,"netTestFile",netTestFile);
//        lti::read(handler,"statisticsFile",statisticsFile);
//        lti::read(handler,"netFilename",netFilename);
//        lti::read(handler,"netFilename2",netFilename2);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && supervisedInstanceClassifier::parameters::read(handler,false);
# else
    bool (supervisedInstanceClassifier::parameters::* p_readMS)
      (ioHandler&,const bool) =
      supervisedInstanceClassifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool rbf::parameters::read(ioHandler& handler,
                             const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // rbf::layer1
  // --------------------------------------------------

  bool rbf::layer1::train(const dvector& input,
                          const int& trainID,
                          trainFunctor& trainFunc) {

    std::string text = "Training for RBF nets is not implemented!\n";
    text += "Initialization is sufficient for use";

    setStatusString(text.c_str());
    return false;
  }

  void rbf::layer1::getH(const dmatrix& features,
                         stdLayer::propagationFunctor& prop,
                         dmatrix& H) {

    for(int i=0;i<H.rows();i++) {
      prop(features[i],weights,H[i]);
    }
  }

  void rbf::layer1::getHtilde(stdLayer::propagationFunctor& prop,
                              dmatrix& Htilde) {
    for(int i=0;i<sizeOut;i++) {
      prop(weights[i],weights,Htilde[i]);
    }
  }


  void rbf::layer1::findSigmas(const double& sigmaFac,
                               dvector& sigmas) {
    // Get number of neurons per class from class information
    // works since there is an equal number of neurons for each
    // class (yet :-)
    int nbNeurObj=1;
    int firstID=outID[0];
    while(outID[nbNeurObj]==firstID) {
      nbNeurObj++;
    }

    dmatrix distance(sizeOut,sizeOut,1E+99);

    //Find the distances between prototypes of different classes
    //Distance between same classes is set to big number 1E+99
    int i;
    l2Distance<double> l2norm;
    for(i=0; i<sizeOut; i++) {
      for(int j=i+(nbNeurObj-i%nbNeurObj); j<sizeOut; j++) {
        distance[i][j]=l2norm.apply(weights[i],weights[j]);
        distance[j][i]=distance[i][j];
      }
    }

    //Set sigmas proportional to smalles distance to other class
    //prototype
    for(i=0;i<sizeOut;i++) {
      sigmas[i]=sigmaFac*(distance[i].minimum());
    }
  }

  void
  rbf::layer1::setTrainParams(rbf::initFunctor1& initFunctor,
                              const parameters::eLvqTrainType& theTrainType,
                              const double& theLearnRate,
                              const double& theLrFac,
                              const double& theWinSize) {
    initFunctor.setTrainParams(theTrainType,
                               theLearnRate,
                               theLrFac,
                               theWinSize);
  }


  // --------------------------------------------------
  // rbf::layer2
  // --------------------------------------------------

  bool rbf::layer2::train(const dvector& input,
                          const int& trainID,
                          trainFunctor& trainFunc) {

    std::string text = "Training for RBF nets is not implemented!\n";
    text += "Initialization is sufficient for use";

    setStatusString(text.c_str());
    return false;
  }


  // --------------------------------------------------
  // rbf::rbInitFunctor1
  // --------------------------------------------------

  rbf::initFunctor1::initFunctor1(const dmatrix& theFeatures,
                                  const ivector& theTrainIDs,
                                  const ivector& theNbViewsObj,
                                  const int& theNbObj,
                                  const int& theNbNeurObj,
                                  const parameters::eLvqInit& theLvqInit,
                                  const eNormType& theNorm)
    : features(theFeatures),trainIDs(theTrainIDs),nbViewsObj(theNbViewsObj),
      nbObj(theNbObj),nbNeurObj(theNbNeurObj),lvqInit(theLvqInit),
      norm(theNorm), linear(stdLayer::linearActFunctor()),
      distProp(linear,theNorm), trainer(0), step(0) {

    changed=true;
    modified=true;
    norm=theNorm;

    index.resize(features.rows(),0,false);
    dummy.resize(nbObj*nbNeurObj,0.,false);
  }

  rbf::initFunctor1::~initFunctor1() {
    delete trainer;
  }

  bool rbf::initFunctor1::operator()(dmatrix& weights, ivector& outID) {
    // The initialization of the first layer of a RBF net is analog to the
    // training of a LVQ net. Thus, this function is analog to the train() of
    // the LVQ net


    // In the first step the weights of this layer must be initialized.
    if (step==0) {

      if (lvqInit == parameters::LvqRand) {
        lvq::randInitFunctor initFunc(features, trainIDs, nbViewsObj,
                                      nbObj, nbNeurObj);
        initFunc(weights, outID);
      } else {
        lvq::maxDistInitFunctor initFunc(features, trainIDs, nbViewsObj,
                                         nbObj, nbNeurObj,norm);
        initFunc(weights, outID);
      }

      //Initializations
      step++;
      for(int i=0;i<features.rows();i++) {
        index[i]=i;
      }

      modified = true;

    } else if (modified) { //Perform a training step

      // if the type of lvqTrainFunctor was changed since last use, set
      // trainer to the appropriate functor
      if (changed) {
        delete trainer;
        trainer = 0;

        switch (lvqType) {
          case parameters::LVQ1: {
            trainer = new lvq::lvq1TrainFunctor(distProp);
            trainer->setLearnRate(learnRate);
          }
          break;

          case parameters::OLVQ1: {
            trainer = new lvq::olvq1TrainFunctor(distProp);
            trainer->setLearnRate(learnRate);
          }
          break;

          case parameters::LVQ3: {
            trainer = new lvq::lvq3TrainFunctor(distProp);
            trainer->setLearnFactor(lrFac);
            trainer->setWindowSize(winSize);
            trainer->setLearnRate(learnRate);
          }
          break;

          case parameters::OLVQ3: {
            trainer = new lvq::olvq3TrainFunctor(distProp);
            trainer->setLearnFactor(lrFac);
            trainer->setWindowSize(winSize);
            trainer->setLearnRate(learnRate);
          }
          break;

          case parameters::LVQ4: {
            trainer = new lvq::lvq4TrainFunctor(distProp);
            trainer->setLearnFactor(lrFac);
            trainer->setWindowSize(winSize);
            trainer->setLearnRate(learnRate);
          }
          break;

          default: {
            return false; // "unknown eLvqTrainType in rbfInitFunctor1!");
          }
        }
      }

      //present training data in different order on each rum
      scramble<int> scrambler;
      scrambler.apply(index);

      modified = false;
      bool modStep;

      //present each training vector
      for(int i=0;i<index.size();i++) {
        (*trainer)(features[index[i]], weights, dummy, outID,
                   trainIDs[index[i]], modStep);
        modified = modified || modStep;
      }

      step++;
      changed=false;
    }

    return true;
  }

  void rbf::initFunctor1::setTrainParams(const parameters::eLvqTrainType&
                                                                 theTrainType,
                                         const double& theLearnRate,
                                         const double& theLrFac,
                                         const double& theWinSize) {

    lvqType=theTrainType;
    learnRate=theLearnRate;
    lrFac=theLrFac;
    winSize=theWinSize;
    if (winSize==0. && (lvqType==parameters::LVQ3 ||
                        lvqType==parameters::LVQ4)) {
      std::string txt;
      txt =  "initFunctor1::setTrainParams needs 4 parameters ";
      txt += "for LVQ3/4!";
      throw exception(txt);
    }

    if ((lrFac!=0. || winSize!=0.) && (lvqType==parameters::LVQ1 ||
                                       lvqType==parameters::OLVQ1)) {
      std::string txt;
      txt  = "rbfInitFunctor1::setTrainParams needs only 2 parameters ";
      txt += "for (O)LVQ1!";
      throw exception(txt);
    }

    changed=true;
  }

  // --------------------------------------------------
  // rbf::rbfInitFunctor2
  // --------------------------------------------------


  rbf::initFunctor2::initFunctor2(dmatrix& theH,
                                  dmatrix& theHtilde,
                                  const double & theLambda,
                                  const ivector& theTrainIDs)
    : H(theH),Htilde(theHtilde),trainIDs(theTrainIDs),
      lambda(theLambda) {
  }


  bool rbf::initFunctor2::operator()(dmatrix& weights, ivector& outID) {

    //Initialize the IDs
    int i;
    for(i=0;i<outID.size();i++) {
      outID[i]=i;
    }

    // Create a vector with as many columns as Objects, rows as many as
    // feature vectors in the training set
    // Values chosen to 1 for correct and -1 for incorrect
    // MUST NOT BE CHANGED SINCE USAGE OF SIGMOID FUNCTOR DEPENDS ON THIS

    const double starter=1;
    dmatrix Y(H.rows(), outID.size() ,-starter);
    for (i=0;i<trainIDs.size();i++) {
      Y[i][trainIDs[i]]=starter;
    }

    dmatrix dummy;
    pseudoInverseMP<double> pinv(lambda);
    pinv.apply(H, Htilde, dummy);

    dummy.multiply(Y);

    dummy.transpose();
    dummy.detach(weights);

    return true;
  }

  // --------------------------------------------------
  // rbf
  // --------------------------------------------------

  rbf::rbf()
    : supervisedInstanceClassifier(),
      rbf1("Layer 1"),rbf2("Layer 2"),distProp(0),
      dotProp(0),gaussAct(0),sigmoidAct(0) {
//  trainStat(0),testStat(0),lvqStat(0)

    // default parameters
    parameters par;
    _lti_debug("Setting params\n");
    setParameters(par);
    _lti_debug("Done Setting params, creating template\n");

    // default output object
    outTemplate=outputTemplate();
    _lti_debug("done with template\n");
  }

  rbf::rbf(const rbf& other)
    : supervisedInstanceClassifier(),
      rbf1("Layer 1"),rbf2("Layer 2"),distProp(0),
      dotProp(0),gaussAct(0),sigmoidAct(0) {
//       trainStat(0),testStat(0),lvqStat(0)
    copy(other);
  }

  rbf::rbf(const parameters& param)
    : supervisedInstanceClassifier(),
      rbf1("Layer 1"),rbf2("Layer 2"),distProp(0),
      dotProp(0),gaussAct(0),sigmoidAct(0) {

    setParameters(param);

    // default output object
    outTemplate=outputTemplate();
  }

  rbf::~rbf() {
    delete distProp;
    delete dotProp;
    delete gaussAct;
    delete sigmoidAct;
  }

  // get type name
  const char* rbf::getTypeName() const {
    return "rbf";
  }

  bool rbf::train(const dmatrix& input,
                  const ivector& ids) {

    _lti_debug("ltiRbf::train\n");

    if (input.empty()) {
      setStatusString("Train data empty");
      return false;
    }


    if (ids.size()!=input.rows()) {
      std::string str;
      str = "dimensionality of IDs vector and the number of rows ";
      str+= "of the input matrix must have the same size.";
      setStatusString(str.c_str());
      return false;
    }

    const parameters& param = getParameters();

    int i;

    if (validProgressObject()) {

      getProgressObject().reset();
      getProgressObject().setTitle("RBF: Training");
      getProgressObject().setMaxSteps(param.nbPresentations1+
                                      param.nbPresentations2+3);

    }

    // extract usefull information from the input data

    // how many objects are in the ids vector?
    realToIntern.clear();

    int counter;

    std::map<int,int>::iterator mit;
    std::vector<int> nbViewsObjTmp;
    std::vector<int> internToRealTmp;

    // initialize the tables realToIntern, internToReal and nbViewsObj
    counter = 0;

    _lti_debug("constructing maps\n");

    for (i=0;i<ids.size();i++) {
      mit = realToIntern.find(ids.at(i));
      if (mit==realToIntern.end()) {
        realToIntern[ids.at(i)]=counter;
        internToRealTmp.push_back(ids.at(i));
        nbViewsObjTmp.push_back(1);
        counter++;
      }
      else {
        // increment the number of patterns per object
        nbViewsObjTmp[(*mit).second]++;
      }
    }

    internToReal.castFrom(internToRealTmp);
    ivector nbViewsObj(nbViewsObjTmp);

    // the number of objects in training set:
    sizeOut=counter;
    // sizes of input and output layers
    sizeIn=input.columns();

    _lti_debug("init members\n");

    // initialize various arrays
    rbf1.setSize(sizeIn, param.nbHiddenNeurons*sizeOut);
    rbf2.setSize(param.nbHiddenNeurons*sizeOut, sizeOut);
    sigmas.resize(param.nbHiddenNeurons*sizeOut, 0.);
    internToReal.resize(sizeOut,0);

//      if(param.doStatistics) {
//        lvqNnoutput.resize(sizeOut,sizeOut);
//      }

    features.copy(input);

    // initialize the member trainID, which contains the internal
    // class id for the respective input vector
    trainID.resize(ids.size(),0,false,false);

    for (i=0;i<ids.size();i++) {
      trainID.at(i) = realToIntern[ids.at(i)];
    }

    initFunctor1 layerOneInit(features,
                              trainID,
                              nbViewsObj,
                              sizeOut,
                              param.nbHiddenNeurons,
                              param.lvqInitType,
                              param.norm);


    if (validProgressObject()) {
      getProgressObject().step("Initializing layer 1");
    }

    _lti_debug("initialize first layer\n");

    // Initialization of LVQ-training
    rbf1.initWeights(layerOneInit);

    //Init Statistics
//      if (param.doStatistics) {
//        initStatistics();

//        statistics(*testStat,testIDs,testFeatures,true);

//        if(param.alsoTrain) {
//          statistics(*trainStat, trainStatIDs, features, false);
//        }
//      }


    if (param.lvqType1!=parameters::NO_LVQ) {
      _lti_debug("doing first training\n");
      rbf1.setTrainParams(layerOneInit, param.lvqType1, param.learnRate1);
      std::string display;
      char buf[50];

      if (validProgressObject()) {
        display="layer 1 ";
        display+= (param.lvqType1==parameters::LVQ1 ?
                   "LVQ1 step " :
                   "OLVQ1 step ");
      }


      for (i=0; i<param.nbPresentations1; i++) {
        // unsupported by g++: itoa(i,buf,10);
        // workaround:

        _lti_debug(".");
        if (validProgressObject()) {
          sprintf(buf,"%i",i);

          getProgressObject().step(display+buf);
        }

        rbf1.initWeights(layerOneInit);

//          if (param.doStatistics) {
//            statistics(*testStat,testIDs,testFeatures,true);
//            if (param.alsoTrain) {
//              statistics(*trainStat, trainStatIDs, features, false);
//            }
//          }
      }
    }

    if (param.lvqType2 != parameters::NO_LVQ) {
      _lti_debug("doing second training\n");
      std::string display;
      rbf1.setTrainParams(layerOneInit,
                          param.lvqType2,
                          param.learnRate2,
                          param.learnFactor,
                          param.windowSize);

      if (validProgressObject()) {
        display="layer 1 ";
        switch (param.lvqType2) {
          case parameters::LVQ3: {
            display+="LVQ3 step ";
          }
          break;

          case parameters::LVQ4: {
            display+="LVQ4 step ";
          }
          break;

          case parameters::OLVQ3: {
            display+="OLVQ3 step ";
          }
          break;

          case parameters::NO_LVQ:
            break;

          case parameters::LVQ1: {
            display+="LVQ1 step ";
          }
          break;

          case parameters::OLVQ1:  {
            display+="OLVQ1 step ";
          }
          break;
        }
      }

      for (i=0; i<param.nbPresentations2; i++) {
        // unsupported by g++: itoa(i,buf,10);
        // workaround:

        _lti_debug(".");

        if (validProgressObject()) {
          char buf[50];
          sprintf(buf,"%i",i);
          getProgressObject().step(display+buf);
        }

        rbf1.initWeights(layerOneInit);

//          if (param.doStatistics) {
//            statistics(*testStat,testIDs,testFeatures,true);

//            if (param.alsoTrain) {
//              statistics(*trainStat, trainStatIDs, features, false);
//            }
//          }
      }
    }

    _lti_debug("calculating sigmas\n");
    rbf1.findSigmas(param.sigmaFactor, sigmas);

    if (validProgressObject()) {
      getProgressObject().step("Initializing layer 2");
    }

    delete gaussAct;
    gaussAct=new stdLayer::gaussActFunctor(0., sigmas);

    delete sigmoidAct;
    sigmoidAct = new stdLayer::sigmoidActFunctor(-log(1/param.high-1));

    delete distProp;
    distProp = new stdLayer::distancePropFunctor(*gaussAct, param.norm);

    delete dotProp;
    dotProp = new stdLayer::dotPropFunctor(*sigmoidAct);

    _lti_debug("training second layer\n");

    // Computation of the Moore-Penrose Pseudo Inverse
    int outDim = sizeOut*param.nbHiddenNeurons;
    dmatrix H(features.rows(), outDim, 0.);
    dmatrix Htilde(outDim,outDim, 0.);

    rbf1.getH(features, *distProp, H);
    rbf1.getHtilde(*distProp, Htilde);
    initFunctor2 rbfFunctor2(H, Htilde, param.lambda, trainID);
    rbf2.initWeights(rbfFunctor2);

    // Calculate object probabilities
    if (param.doTrain2) {
      lti::vector<int> train2InternIds(train2Ids.size(),0);
      for(int i=0;i<train2InternIds.size();i++) {
        train2InternIds[i]=realToIntern[train2Ids[i]];
      }

      if (!calcObjProbs(train2InternIds, train2Features, *distProp)) {
        setStatusString("Very bad training! One class never activated!");
        return false;
      }
    } else {
      if(!calcObjProbs(trainID, features, *distProp)) {
        setStatusString("Very bad training! One class never activated!");
        return false;
      }
    }

    if (validProgressObject()) {
      getProgressObject().step("Training ready.");
    }


    return true;

  }

  bool rbf::classify(const dvector& feature, outputVector& result) const {


    dvector rbf1Output, rbf2Output;

    bool b=true;

    if (notNull(distProp)) {
      b = b && rbf1.propagate(feature,*distProp,rbf1Output);
    }

    if (notNull(dotProp)) {
      b = b && rbf2.propagate(rbf1Output,*dotProp,rbf2Output);
    }

    // abs. value = 1 for prob. distribution
    double numerator = rbf2Output.sumOfElements();
    if (numerator != 0.0) {
      numerator = 1.0/numerator;
    }

    dvector outStat;
    outStat.multiply(rbf2Output,numerator);

    b = b && (outStat.size()==outTemplate.size());

    if (b) {
      outputVector ov=outTemplate.apply(outStat, result);
      result.setWinnerAtMax();
      return true;
    } else {
      outStat=dvector(outTemplate.size(), 0.);
      outTemplate.apply(outStat, result);
      result.setReject(true);
      return false;
    }
  }


  bool rbf::write(ioHandler& handler,const bool complete) const {
    bool b(true);

    if (complete) {
      b = handler.writeBegin();
    }

    // write the standard data (output and parameters)
    supervisedInstanceClassifier::write(handler,false);

    if (b) {
      lti::write(handler,"rbf1",rbf1);
      lti::write(handler,"rbf2",rbf2);
      lti::write(handler,"sizeIn",sizeIn);
      lti::write(handler,"sizeOut",sizeOut);
      lti::write(handler,"sigmas",sigmas);
    }

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

  bool rbf::read(ioHandler& handler,const bool complete) {
    bool b(true);

    if (complete) {
      b = handler.readBegin();
    }

    // read the standard data (output and parameters)
    supervisedInstanceClassifier::read(handler,false);

    const parameters& param = getParameters();

    if (b) {
      lti::read(handler,"rbf1",rbf1);
      lti::read(handler,"rbf2",rbf2);
      lti::read(handler,"sizeIn",sizeIn);
      lti::read(handler,"sizeOut",sizeOut);
      lti::read(handler,"sigmas",sigmas);

      if (sigmas.size() != sizeOut*param.nbHiddenNeurons) {
        sigmas.resize(sizeOut*param.nbHiddenNeurons,param.sigma,true,true);
      }

      delete gaussAct;
      gaussAct = new stdLayer::gaussActFunctor(0.,sigmas);

      delete sigmoidAct;
      sigmoidAct = new stdLayer::sigmoidActFunctor(-log(1/param.high-1));

      delete distProp;
      distProp = new stdLayer::distancePropFunctor(*gaussAct,param.norm);

      delete dotProp;
      dotProp = new stdLayer::dotPropFunctor(*sigmoidAct);

    }

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;

  }

#ifdef _INCLUDE_DEPRECATED

//    bool rbf::save(std::ostream& outstream) {
//      const parameters& param = getParameters();
//      output& nnoutput = getOutput();

//      if (!outstream.good()) {
//        return false;
//      } else {

//        outstream << "RBF Net                   v.0.1  LTI-Lib (c) 2000 \n"
//                  << "========\n\n\n"
//                  << "Input neurons: " << sizeIn << "\n"
//                  << "Hidden Neurons per class: " << param.nbHiddenNeurons
//                  << "\n"
//                  << "Classes: " << sizeOut <<"\n"
//                  << "Data for Layer 1:\n"
//                  << "\tNorm: "<< (param.norm==L1distance ? "L1" : "L2") <<"\n"
//                  << "\tLambda for smoothing: "<<param.lambda<<"\n";
//        if (param.nbPresentations1 != 0) {
//          outstream << "\tTrainingsteps "
//                    << ((param.lvqType1==parameters::OLVQ1)? "O" : "")
//                    << "LVQ1: "
//                    << param.nbPresentations1 << "\n"
//                    << "\t\tLearnrate: " << param.learnRate1 << "\n";
//        }

//        if (param.nbPresentations2 != 0) {
//          outstream << "\tTrainingsteps "
//                    << ((param.lvqType2==parameters::OLVQ3)? "O" : "")
//                    << "LVQ3: "
//                    << param.nbPresentations2 << "\n"
//                    << "\t\tLearnrate: " << param.learnRate2 << "\n"
//                    << "\t\tWindow parameter: " << param.windowSize << "\n"
//                    << "\t\tModification of learnrate: "
//                    << param.learnFactor << "\n";
//        }

//        outstream << "\tSigmas:\n";

//        for (int i=0; i<sigmas.size(); i++) {
//          outstream.width(9);
//          outstream << "  " << sigmas[i];
//          if ((i+1)%param.nbHiddenNeurons == 0) {
//            outstream <<"\n";
//          }
//        }

//        outstream << "\nData for Layer 2:\n"
//                  << "\tHigh Value: " << param.high <<"\n\n\n";

//        rbf1.save(outstream);
//        outstream<<"\n\n\n";
//        rbf2.save(outstream);

//        outstream << "\n\nNNOutput:\n";
//        nnoutput.writeAscii(outstream);

//      }

//      return true;
//    }

//    bool rbf::saveBinary(const char* fname) {
//      const parameters& param = getParameters();
//      output& nnoutput = getOutput();

//      std::ofstream outfile;
//      outfile.open(fname,std::ios::out|std::ios::binary);
//      if (!outfile.is_open()) {
//        return false;
//      } else {
//        outfile.write((char*)&(sizeIn), sizeof(int));
//        outfile.write((char*)&(param.nbHiddenNeurons), sizeof(int));
//        outfile.write((char*)&(sizeOut), sizeof(int));

//        outfile.write((char*)&(param.norm), sizeof(eNormType));
//        outfile.write((char*)&(param.lambda), sizeof(double));
//        outfile.write((char*)&(sizeIn), sizeof(int));
//        outfile.write((char*)&(param.sigma), sizeof(double));

//        outfile.write((char*)&(sigmas[0]),sigmas.size()*sizeof(double));

//        outfile.write((char*)&(param.high), sizeof(double));

//        rbf1.saveBinary(outfile);
//        rbf2.saveBinary(outfile);

//        nnoutput.writeAscii(outfile);
//      }
//      outfile.close();
//      return true;
//    }

//    bool rbf::load(std::istream& instream) {

//      parameters& param = getParameters();
//      output& nnoutput = getOutput();

//      std::string tmpStr;
//      std::string::size_type pos;

//      if (!instream.good()) {
//        return false;
//      }

//      // first line MUST contain "RBF Net"
//      getline(instream,tmpStr);

//      if (tmpStr.find("RBF Net")==std::string::npos) {
//        // not a RBF file format
//        return false;
//      }

//      // input neurons
//      do {
//        getline(instream,tmpStr);
//      } while(tmpStr.find("Input neurons:") == std::string::npos);

//      pos = tmpStr.find_first_not_of(": \t",tmpStr.find(":"));
//      sizeIn=atoi(tmpStr.substr(pos).c_str());

//      // hidden neurons per class
//      do {
//        getline(instream,tmpStr);
//      } while (tmpStr.find_first_not_of(" \t") == std::string::npos);
//      if (tmpStr.find("Hidden Neurons") == std::string::npos) {
//        return false;
//      }
//      pos = tmpStr.find_first_not_of(": \t",tmpStr.find(":"));
//      param.nbHiddenNeurons=atoi(tmpStr.substr(pos).c_str());

//      // classes
//      do {
//        getline(instream,tmpStr);
//      } while (tmpStr.find_first_not_of(" \t") == std::string::npos);
//      if (tmpStr.find("Classes") == std::string::npos) {
//        return false;
//      }
//      pos = tmpStr.find_first_not_of(": \t",tmpStr.find(":"));
//      sizeOut=atoi(tmpStr.substr(pos).c_str());

//      // get line "Data for Layer 1:"
//      do {
//        getline(instream,tmpStr);
//      } while ((tmpStr.find("Data for Layer") == std::string::npos) &&
//               (!instream.eof()));

//      // norm
//      do {
//        getline(instream,tmpStr);
//      } while (tmpStr.find_first_not_of(" \t") == std::string::npos);
//      if (tmpStr.find("Norm") == std::string::npos) {
//        return false;
//      }
//      pos = tmpStr.find("L",tmpStr.find(":"));
//      pos++;

//      param.norm = (tmpStr[pos]=='1') ? L1distance : L2distance;

//      // lambda for smoothing
//      do {
//        getline(instream,tmpStr);
//      } while (tmpStr.find_first_not_of(" \t") == std::string::npos);
//      if (tmpStr.find("Lambda for smoothing") == std::string::npos) {
//        return false;
//      }
//      pos = tmpStr.find_first_not_of(": \t",tmpStr.find(":"));

//      param.lambda=atof(tmpStr.substr(pos).c_str());

//      // sigmas
//      do {
//        getline(instream,tmpStr);
//      } while (tmpStr.find_first_not_of(" \t") == std::string::npos);
//      if (tmpStr.find("Sigmas") == std::string::npos) {
//        return false;
//      }

//      sigmas.resize(sizeOut*param.nbHiddenNeurons,0.,false);
//      for (int i=0; i<sigmas.size(); i++) {
//        instream >> sigmas[i];
//      }

//      // Data layer 2

//      // get line "Data for Layer 2:"
//      do {
//        getline(instream,tmpStr);
//      } while ((tmpStr.find("Data for Layer") == std::string::npos) &&
//               (!instream.eof()));

//      // High Output Value
//      do {
//        getline(instream,tmpStr);
//      } while (tmpStr.find_first_not_of(" \t") == std::string::npos);
//      if (tmpStr.find("") == std::string::npos) {
//        return false;
//      }
//      pos = tmpStr.find_first_not_of(": \t",tmpStr.find(":"));

//      param.high=atof(tmpStr.substr(pos).c_str());

//      // layer 1
//      if (!rbf1.load(instream)) {
//        return false;
//      }

//      // layer 2
//      if (!rbf2.load(instream)) {
//        return false;
//      }

//      // nnoutput
//      do {
//        getline(instream,tmpStr);
//      } while ((tmpStr.find("NNOutput") == std::string::npos) &&
//               (!instream.eof()));

//      nnoutput.readAscii(instream);

//      delete gaussAct;
//      gaussAct = new stdLayer::gaussActFunctor(0.,sigmas);

//      delete sigmoidAct;
//      sigmoidAct = new stdLayer::sigmoidActFunctor(-log(1/param.high-1));

//      delete distProp;
//      distProp = new stdLayer::distancePropFunctor(*gaussAct,param.norm);

//      delete dotProp;
//      dotProp = new stdLayer::dotPropFunctor(*sigmoidAct);

//      return true;
//    }

//    bool rbf::loadBinary(const char * fname) {
//      parameters& param = getParameters();
//      output& nnoutput = getOutput();

//      std::ifstream infile;
//      infile.open(fname,std::ios::in|std::ios::binary);
//      if (!infile.is_open()) {
//        return false;
//      } else {
//        infile.read((char*)&(sizeIn), sizeof(int));
//        infile.read((char*)&(param.nbHiddenNeurons), sizeof(int));
//        infile.read((char*)&(sizeOut), sizeof(int));

//        infile.read((char*)&(param.norm), sizeof(eNormType));
//        infile.read((char*)&(param.lambda), sizeof(double));
//        infile.read((char*)&(sizeIn), sizeof(int));
//        infile.read((char*)&(param.sigma), sizeof(double));

//        sigmas.resize(sizeOut*param.nbHiddenNeurons,0.);
//        infile.read((char*)&(sigmas[0]),sigmas.size()*sizeof(double));

//        infile.read((char*)&(param.high), sizeof(double));

//        if (!rbf1.loadBinary(infile)) {
//          infile.close();
//          return false;
//        }

//        if (!rbf2.loadBinary(infile)) {
//          infile.close();
//          return false;
//        }
//        nnoutput.readAscii(infile);
//      }

//      infile.close();

//      delete gaussAct;
//      gaussAct = new stdLayer::gaussActFunctor(0.,sigmas);

//      delete sigmoidAct;
//      sigmoidAct = new stdLayer::sigmoidActFunctor(-log(1/param.high-1));

//      delete distProp;
//      distProp = new stdLayer::distancePropFunctor(*gaussAct,param.norm);

//      delete dotProp;
//      dotProp = new stdLayer::dotPropFunctor(*sigmoidAct);

//      return true;
//    }

#endif

//    void rbf::noObjProbs() {
//      // ok, das ist ein bischen gepfuscht sollte aber klappen
//      output& nnoutput = getOutput();

//      output::namesMap objNames = nnoutput.getClassNames();
//      output::namesMap::const_iterator names = objNames.begin();

//      for(int i=0;i<sizeOut;i++,names++) {
//        nnoutput.getOutputVector()[i].objects.clear();
//        objectProb objProb;
//        objProb.id = names->first;
//        objProb.prob =  1;
//        nnoutput.getOutputVector()[i].objects.push_back(objProb);
//      }
//    }

  bool rbf::calcObjProbs(ivector& internIds,
                         dmatrix &feats,
                         stdLayer::distancePropFunctor& distProp) {

    bool allOk=true;
    lti::matrix<int> clCount(sizeOut,sizeOut,0);
    // const lti::vector<int>& outID = rbf2.getOutID();
    int i,j,k;
    outputVector outV;
    ivector ideez(sizeOut);
    for (i=0; i<sizeOut; i++) {
      ideez.at(i)=i;
    }
    outTemplate=outputTemplate(ideez);

    for(i=0;i<feats.rows();i++) {
      allOk = classify(feats[i], outV) && allOk;
      clCount[outV.maxPosition()][internIds[i]]++;
    }

    double rowsum;
    int rowsize;
    outTemplate=outputTemplate(sizeOut);
    for(i=0;i<sizeOut;i++) {
      rowsum=clCount[i].sumOfElements();
      outputVector rowV;
      if (rowsum!=0) {
        rowsize=0;
        for (j=0;j<sizeOut;j++) {
          if (clCount[i][j]!=0) {
            rowsize++;
          }
        }
        rowV=outputVector(rowsize);
        for(j=0,k=0;j<sizeOut;j++) {
          if (clCount[i][j]!=0) {
            rowV.setPair(k++, internToReal[j], clCount[i][j]/rowsum);
          }
        }
      } else {
        allOk=false;
      }
      outTemplate.setProbs(i, rowV);
    }
    outTemplate.setIds(internToReal);
    return allOk;
  }

  void rbf::setTestSet(const dmatrix& input,
                       const ivector& ids) {

    testIDs.copy(ids);
    testFeatures.copy(input);
  }


  void rbf::setTrainSet2(const dmatrix& input,
                         const ivector& ids) {

    train2Ids.copy(ids);
    train2Features.copy(input);
  }

//    void rbf::statistics(statFunctor& stat,
//                         ivector& ids,
//                         dmatrix& feats,
//                         bool saveMe) {

//      const parameters& param = getParameters();
//      output& nnoutput = getOutput();

//      rbf1.findSigmas(param.sigmaFactor, sigmas);

//      delete gaussAct;
//      gaussAct=new stdLayer::gaussActFunctor(0., sigmas);

//      delete sigmoidAct;
//      sigmoidAct = new stdLayer::sigmoidActFunctor(-log(1/param.high-1));

//      distProp = new stdLayer::distancePropFunctor(*gaussAct, param.norm);
//      dotProp = new stdLayer::dotPropFunctor(*sigmoidAct);

//      // Computation of the Moore-Penrose Pseudo Inverse
//      dmatrix H(features.rows(), sizeOut*param.nbHiddenNeurons, 0.);
//      dmatrix Htilde(sizeOut*param.nbHiddenNeurons,
//                     sizeOut*param.nbHiddenNeurons, 0.);

//      rbf1.getH(features, *distProp, H);
//      rbf1.getHtilde(*distProp, Htilde);
//      initFunctor2 rbfFunctor2(H, Htilde, param.lambda, trainID);
//      rbf2.initWeights(rbfFunctor2);

//      dvector lvqOut;
//      dvector rbfOut;

//      int i;
//      for (i=0; i<feats.rows(); i++) {
//        rbf1.propagate(feats[i],*distProp);
//        lvqOut=rbf1.getOutput();
//        lvqNnoutput.setProbVector(lvqOut);
//        (*lvqStat)(ids[i],lvqNnoutput);
//        rbf2.propagate(lvqOut, *dotProp);
//        rbfOut=rbf2.getOutput();
//        nnoutput.setProbVector(rbfOut);
//        stat(ids[i],nnoutput);
//      }

//      lvqStat->writeStat();
//      stat.writeStat();

//      bool reallySave=
//        (param.mseStat && !param.classifyStat && param.mseSave &&
//         (dynamic_cast<mseStatFunctor*>(&stat))->isMaxMse())
//        || (!param.mseStat &&  param.classifyStat &&
//            (param.firstBestSave &&
//             (dynamic_cast<classifyStatFunctor*>(&stat))->isMaxCorrect()))
//        || (param.threeBestSave &&
//            (dynamic_cast<classifyStatFunctor*>(&stat))->isMax3Best())
//        || (param.mseStat &&  param.classifyStat &&
//            (param.firstBestSave &&
//             (dynamic_cast<mseClassifyStatFunctor*>(&stat))->isMaxCorrect()))
//        || (param.threeBestSave &&
//            (dynamic_cast<mseClassifyStatFunctor*>(&stat))->isMax3Best())
//        || (param.mseSave &&
//            (dynamic_cast<mseClassifyStatFunctor*>(&stat))->isMaxMse());

//      bool saveBest=param.mseSave || param.firstBestSave || param.threeBestSave;

//      if (saveMe && saveBest && reallySave) {
//        // Calculate object probabilities
//        if (param.doTrain2) {
//          ivector train2InternIds(train2Ids.size(),0);
//          for(int i=0;i<train2InternIds.size();i++) {
//            train2InternIds[i]=realToIntern[train2Ids[i]];
//          }
//          calcObjProbs(train2InternIds, train2Features, *distProp);
//        } else {
//          calcObjProbs(trainID, features, *distProp);
//        }

//        if (param.ascii && !param.binary) {
//          std::ofstream out(param.netFilename.c_str());
//          lispStreamHandler lsh(out);
//          write(lsh);
//          out.close();
//        }

//        if (param.ascii && param.binary) {
//          std::ofstream out(param.netFilename2.c_str());
//          lispStreamHandler lsh(out);
//          write(lsh);
//          out.close();
//        }

//        if(param.binary) {
//          std::ofstream out(param.netFilename.c_str());
//          binaryStreamHandler bsh(out);
//          write(bsh);
//          out.close();
//        }

//        for(i=0;i<sizeOut;i++) {
//          nnoutput.getOutputVector()[i].objects.clear();
//          objectProb objProb;
//          objProb.id = internToReal[i];
//          objProb.prob =  1;
//          nnoutput.getOutputVector()[i].objects.push_back(objProb);
//        }
//      }

//      delete gaussAct;
//      delete sigmoidAct;
//      gaussAct = 0;
//      sigmoidAct = 0;
//      stat.reset();
//    }

//    void rbf::initStatistics() {

//      const parameters& param = getParameters();
//      output& nnoutput = getOutput();

//      std::string name, extension, trainfile, testfile, lvqfile;
//      name.assign(param.statisticsFile,0,param.statisticsFile.find("."));
//      extension.assign(param.statisticsFile,
//                       param.statisticsFile.find("."),
//                       param.statisticsFile.size());
//      trainfile=name+"_train"+extension;
//      testfile=name+"_test"+extension;
//      lvqfile=name+"_lvq"+extension;

//      delete lvqStat;
//      lvqStat=new classifyStatFunctor(lvqfile.c_str());

//      if (param.mseStat) {
//        for (int i=0; i<sizeOut; i++) {
//          targetVec[internToReal[i]].resize(sizeOut,1-param.high,false);
//          targetVec[internToReal[i]].at(i) = param.high;
//        }
//      }

//      if (param.classifyStat && !param.mseStat) {
//        delete testStat;
//        testStat= new classifyStatFunctor(testfile.c_str());
//      } else if (param.mseStat && !param.classifyStat) {
//        delete testStat;
//        testStat= new mseStatFunctor(testfile.c_str());
//        (dynamic_cast<mseStatFunctor*>(testStat))->setTargetMap(targetVec);
//      } else if (param.mseStat && param.classifyStat) {
//        delete testStat;
//        testStat= new mseClassifyStatFunctor(testfile.c_str());
//        (dynamic_cast<mseClassifyStatFunctor*>(testStat))->setTargetMap(targetVec);
//      } else {
//        throw exception("No Type of StatFunctor defined in rbf::train");
//      }

//      if (param.alsoTrain) {
//        if (param.classifyStat && !param.mseStat) {
//          trainStat= new classifyStatFunctor(trainfile.c_str());
//        } else if (param.mseStat && !param.classifyStat) {
//          trainStat= new mseStatFunctor(trainfile.c_str());
//          (dynamic_cast<mseStatFunctor*>(trainStat))->setTargetMap(targetVec);
//        } else if (param.mseStat && param.classifyStat) {
//          trainStat= new mseClassifyStatFunctor(trainfile.c_str());
//          (dynamic_cast<mseClassifyStatFunctor*>(trainStat))->setTargetMap(targetVec);
//        }
//        //    trainStat->writeHeader();
//      }

//      //  testStat->writeHeader();

//      if (param.alsoTrain) {
//        trainStatIDs.resize(features.rows(),0,false,false);
//        int i;
//        for(i=0;i<features.rows();i++) {
//          trainStatIDs[i]=internToReal[trainID[i]];
//        }
//      }

//      int i;
//      for(i=0;i<sizeOut;i++) {
//        nnoutput.getOutputVector()[i].objects.clear();
//        objectProb objProb;
//        objProb.id = internToReal[i];
//        objProb.prob =  1;
//        nnoutput.getOutputVector()[i].objects.push_back(objProb);
//      }

//      for(i=0;i<sizeOut;i++) {
//        int idx = i*param.nbHiddenNeurons;
//        for (int j=0; j<param.nbHiddenNeurons; j++) {
//          lvqNnoutput.getOutputVector()[idx+j].objects.clear();
//          objectProb objProb;
//          objProb.id = internToReal[i];
//          objProb.prob =  1;
//          lvqNnoutput.getOutputVector()[idx+j].objects.push_back(objProb);
//        }
//      }
//    }

  // return parameters
  const rbf::parameters& rbf::getParameters() const {
    const parameters* par =
      dynamic_cast<const rbf::parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

//    // return parameters
//    rbf::parameters& rbf::getParameters() {
//      parameters* par =
//        dynamic_cast<rbf::parameters*>(&classifier::getParameters());
//      if(isNull(par)) {
//        throw invalidParametersException(getTypeName());
//      }
//      return *par;
//    }

  rbf& rbf::copy(const rbf& other) {
    supervisedInstanceClassifier::copy(other);

    rbf1 = other.rbf1;
    rbf2 = other.rbf2;

//      lvqNnoutput = other.lvqNnoutput;

    sizeIn = other.sizeIn;
    sizeOut = other.sizeOut;

    trainID.copy(other.trainID);
    internToReal.copy(other.internToReal);
    realToIntern = realToIntern;
    features.copy(other.features);
    train2Features.copy(other.train2Features);
    train2Ids.copy(other.train2Ids);

    sigmas = other.sigmas;
    testFeatures = other.testFeatures;
    testIDs = other.testIDs;
    trainStatIDs = other.trainStatIDs;

    targetVec = other.targetVec;

    delete distProp;
    if (notNull(other.distProp)) {
      distProp = new stdLayer::distancePropFunctor(*other.distProp);
    } else {
      distProp = 0;
    }

    delete dotProp;
    if (notNull(other.dotProp)) {
      dotProp = new stdLayer::dotPropFunctor(*other.dotProp);
    } else {
      dotProp = 0;
    }

    delete gaussAct;
    if (notNull(other.gaussAct)) {
      gaussAct = new stdLayer::gaussActFunctor(*other.gaussAct);
    } else {
      gaussAct = 0;
    }

    delete sigmoidAct;
    if (notNull(other.sigmoidAct)) {
      sigmoidAct = new stdLayer::sigmoidActFunctor(*other.sigmoidAct);
    } else {
      sigmoidAct = 0;
    }

//      delete trainStat;
//      if (notNull(other.trainStat)) {
//        trainStat = other.trainStat->clone();
//      } else {
//        trainStat = 0;
//      }

//      delete testStat;
//      if (notNull(other.testStat)) {
//        testStat = other.testStat->clone();
//      } else {
//        testStat = 0;
//      }

//      delete lvqStat;
//      if (notNull(other.lvqStat)) {
//        lvqStat = other.lvqStat->clone();
//      } else {
//        lvqStat = 0;
//      }

    return *this;
  }

  classifier* rbf::clone() const {
    return new rbf(*this);
  }


  const dmatrix& rbf::getWeights1() {
    return rbf1.getWeights();
  }

  const dmatrix& rbf::getWeights2() {
    return rbf2.getWeights();
  }

  const dvector& rbf::getSigmas1() {
    return sigmas;
  }

  const ivector& rbf::getIds() {
    return rbf2.getOutID();
  }

  void rbf::setSigmas(const dvector& s) {
    sigmas=s;
  }

  void rbf::setLayer1Data(const dmatrix& weights) {
    rbf1.setSize(weights.columns(), weights.rows());
    rbf1.setWeights(weights);
    ivector v(weights.rows(),int(0));
    rbf1.setIDs(v);
  }

  void rbf::setLayer2Data(const dmatrix& weights, const ivector& ids) {
    rbf2.setSize(weights.columns(), weights.rows());
    rbf2.setWeights(weights);
    rbf2.setIDs(ids);
  }

  void rbf::initNet(const int& inSize, const int& outSize) {

    const parameters& param = getParameters();

    sizeIn=inSize;
    sizeOut=outSize;

    delete gaussAct;
    gaussAct = new stdLayer::gaussActFunctor(0.,sigmas);

    delete sigmoidAct;
    sigmoidAct = new stdLayer::sigmoidActFunctor(-log(1/param.high-1));

    delete distProp;
    distProp = new stdLayer::distancePropFunctor(*gaussAct,param.norm);

    delete dotProp;
    dotProp = new stdLayer::dotPropFunctor(*sigmoidAct);

  }

}
