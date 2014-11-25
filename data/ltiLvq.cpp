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
 * file .......: ltiLvq.cpp
 * authors ....: Peter Doerfler, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 11.8.2000
 * revisions ..: $Id: ltiLvq.cpp,v 1.6 2006/02/07 18:19:41 ltilib Exp $
 */

#include "ltiObject.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdio> // for sprintf
#include "ltiVector.h"
#include "ltiLvq.h"
#include "ltiL1Distance.h"
#include "ltiL2Distance.h"
#include "ltiSort.h"
#include "ltiScramble.h"
#include "ltiLispStreamHandler.h"

using std::cout;

namespace lti {

  // following functions needed to be declared to avoid an error
  // message of the MS VC++... the reported error doesn't exist really,
  // but without this workaround it doesn't work!

  std::ostream& write(std::ostream& s,const lti::dvector& v) {
    lispStreamHandler out(s);
    v.write(out);
    return s;
  };

  std::istream& read(std::istream& s,lti::dvector& v) {
    lispStreamHandler in(s);
    v.read(in);
    return s;
  }

  // --------------------------------------------------
  // lvq::parameters
  // --------------------------------------------------

  // default constructor
  lvq::parameters::parameters()
    : classifier::parameters() {

    learnRate1        = double(0.3);
    learnRate2        = double(0.1);
    learnRateFactor   = double(0.3);
    windowSize        = double(0.2);
    nbNeuronsPerClass = int(4);
    nbPresentations1  = int(5);
    nbPresentations2  = int(20);
    norm              = L2distance;
    initType          = LvqMaxDist;
    flagOlvq1         = true;
    flagOlvq3         = true;
    sigmaFactor       = 1.6;
    doStatistics      = false;
    statisticsFilename= "lvqstat.dat";
    netFilename       = "lvqnet.lvq";
    doTrain2          = false;
    saveBest          = false;
    correctVs3Best    = true;
  }

  // copy constructor
  lvq::parameters::parameters(const parameters& other)
    : classifier::parameters()  {
    copy(other);
  }

  // destructor
  lvq::parameters::~parameters() {
  }

  // get type name
  const char* lvq::parameters::getTypeName() const {
    return "lvq::parameters";
  }

  // copy member
  lvq::parameters&
  lvq::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    classifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    classifier::parameters& (classifier::parameters::* p_copy)
      (const classifier::parameters&) =
      classifier::parameters::copy;
    (this->*p_copy)(other);
# endif

    learnRate1 = other.learnRate1;
    learnRate2 = other.learnRate2;
    learnRateFactor = other.learnRateFactor;
    windowSize = other.windowSize;
    nbNeuronsPerClass = other.nbNeuronsPerClass;
    nbPresentations1 = other.nbPresentations1;
    nbPresentations2 = other.nbPresentations2;
    norm = other.norm;
    initType = other.initType;
    flagOlvq1 = other.flagOlvq1;
    flagOlvq3 = other.flagOlvq3;
    sigmaFactor = other.sigmaFactor;
    doStatistics = other.doStatistics;
    statisticsFilename = other.statisticsFilename;
    netFilename = other.netFilename;
    doTrain2 = other.doTrain2;
    saveBest = other.saveBest;
    correctVs3Best = other.correctVs3Best;

    return *this;
  }

  // clone member
  classifier::parameters* lvq::parameters::clone() const {
    return new parameters(*this);
  }

  // --------------------------------------------------
  // lvq::layer
  // --------------------------------------------------
  lvq::layer::layer(const std::string& theName)
    : stdLayer(theName) {
  }

  void lvq::layer::findSigmas(const int &nbNeurCl,const double& sigFac,
                              dvector& sigma) {

    l2Distance<double> l2norm;

    sigma.resize(sizeOut,0.,false,true);
    dmatrix distance(sizeOut,sizeOut,1.0E+99);

    //Find the distances between prototypes of different classes
    //Distance between same classes is set to big number 1.0E+99
    int i;
    for(i=0; i<sizeOut; i++) {
      for(int j=i+nbNeurCl; j<sizeOut; j++) {
        distance[i][j]=l2norm.apply(weights[i],weights[j]);
        distance[j][i]=distance[i][j];
      }
    }

    //Set sigmas proportional to smalles distance to other class
    //prototype
    for(i=0;i<sizeOut;i++) {
      sigma[i]=sigFac*(distance[i].minimum());
    }
  }

  // --------------------------------------------------
  // lvq::lvqTrainFunctor
  // --------------------------------------------------

  /**
   * set the learn rate factor
   */
  void lvq::trainFunctor::setLearnFactor(const double& lrFac){
    learnFactor = lrFac;
  };

  /**
   * set the window size
   */
  void lvq::trainFunctor::setWindowSize(const double& winSize){
    windowSize = (1.0-winSize)/(1.0+winSize);
  };

  void lvq::trainFunctor::twoMinIndex(const dvector& vct,
                                      int& min1,
                                      int& min2) {
    double themin1,themin2;

    dvector::const_iterator it,e;
    int i;

    if (vct.size()>0) {
      it = vct.begin();
      e=vct.end();
      themin1 = themin2 = *it;
      min1 = min2 = 0;
      it++;
      if (it!=e) {
        if (*it < themin1) {
          themin1=*it;
          min1=1;
        } else {
          themin2=*it;
          min2=1;
        }
      }
      it++;
      for (i=2;it!=e;it++,i++) {
        if (*it < themin2) {
          if (*it < themin1) {
            themin2 = themin1;
            min2 = min1;
            themin1 = *it;
            min1 = i;
          } else {
            themin2 = *it;
            min2 = i;
          }
        }
      }
    }
  }

  // --------------------------------------------------
  // lvq::lvq1TrainFunctor
  // --------------------------------------------------

  bool lvq::lvq1TrainFunctor::operator()(const dvector& input,
                                         dmatrix& weights,
                                         dvector& outLayer,
                                         const ivector& outID,
                                         const int& trainID,
                                         bool& modified) {

    modified = true;

    if (firstUse) {
      delta.resize(input.size(),0.);
      firstUse=false;
    }

    // Propagate
    if (prop(input,weights,outLayer)) {

      // Lowest exitation = winner
      int winner=outLayer.getIndexOfMinimum();

      delta.subtract(input,weights[winner]);
      delta.multiply(learnRate);

      if (outID[winner]==trainID) {
        delta.add(weights[winner]);
        weights[winner]=delta;
      } else {
        delta.subtract(weights[winner],delta);
        weights[winner]=delta;
      }

      return true;
    }

    return false;

  }

  // --------------------------------------------------
  // lvq::olvq1TrainFunctor
  // --------------------------------------------------

  /**
   * set the learn rate
   */
  void lvq::olvq1TrainFunctor::setLearnRate(const double &theLr) {
    learnRate = theLr;
    lr.fill(theLr);
  }

  /**
   * apply operator
   */
  bool lvq::olvq1TrainFunctor::operator()(const dvector& input,
                                          dmatrix& weights,
                                          dvector& outLayer,
                                          const ivector& outID,
                                          const int& trainID,
                                          bool& modified) {

    modified = true;

    if (firstUse) {
      firstUse=false;
      lr.resize(outLayer.size(),learnRate);
      delta.resize(input.size());
    }

    // Propagate
    if (prop(input,weights,outLayer)) {

      // Lowest exitation = winner
      int winner=outLayer.getIndexOfMinimum();

      delta.subtract(input,weights[winner]);
      delta.multiply(lr[winner]);

      if (outID[winner]==trainID) {
        delta.add(weights[winner]);
        weights[winner]=delta;
        lr[winner]=lr[winner]/(1+lr[winner]);
      } else {
        delta.subtract(weights[winner],delta);
        weights[winner]=delta;
        lr[winner]=lr[winner]/(1-lr[winner]);
        if (lr[winner]>learnRate) {
          lr[winner]=learnRate;
        }
      }

      return true;
    }

    return false;
  }

  // --------------------------------------------------
  // lvq::lvq3TrainFunctor
  // --------------------------------------------------


  bool lvq::lvq3TrainFunctor::operator()(const dvector& input,
                                         dmatrix& weights,
                                         dvector& outLayer,
                                         const ivector& outID,
                                         const int& trainID,
                                         bool& modified) {
    if(firstUse) {
      delta.resize(input.size(),0.);
      delta2.resize(input.size(),0.);
      firstUse=false;
    }

    modified = false;

    // Propagate
    if (prop(input,weights,outLayer)) {

      // Lowest exitation = winner
      int winner,second;
      twoMinIndex(outLayer,winner,second);

      delta.subtract(input,weights[winner]);
      delta.multiply(learnRate);
      delta2.subtract(input,weights[second]);
      delta2.multiply(learnRate);

      if ((outID[winner]==trainID || outID[second]==trainID)
          && ((outLayer[winner] < outLayer[second] ?
               outLayer[winner]/outLayer[second] :
               outLayer[second]/outLayer[winner]) > windowSize))  {
        if(outID[winner]!=outID[second]) {
          if(outID[winner]==trainID) {
            delta.add(weights[winner]);
            weights[winner]=delta;
            delta2.subtract(weights[second],delta2);
            weights[second]=delta2;
          } else {
            delta2.add(weights[second]);
            weights[second]=delta2;
            delta.subtract(weights[winner],delta);
            weights[winner]=delta;
          }
        } else if(outID[winner]==trainID) {
          delta.multiply(learnFactor);
          delta.add(weights[winner]);
          weights[winner]=delta;
          delta2.multiply(learnFactor);
          delta2.add(weights[second]);
          weights[second]=delta2;
        }
        modified = true;
      }
      return true;
    }

    return false;
  }

  // --------------------------------------------------
  // lvq::olvq3TrainFunctor
  // --------------------------------------------------
  void lvq::olvq3TrainFunctor::setLearnRate(const double &theLr) {
    learnRate=theLr;
    lr.fill(theLr);
  }

  bool lvq::olvq3TrainFunctor::operator()(const dvector& input,
                                          dmatrix& weights,
                                          dvector& outLayer,
                                          const ivector& outID,
                                          const int& trainID,
                                          bool& modified) {

    modified = false;

    if(firstUse) {
      delta.resize(input.size(),0.);
      delta2.resize(input.size(),0.);
      firstUse=false;
      lr.resize(outLayer.size(),learnRate);
    }

    // Propagate
    if (prop(input,weights,outLayer)) {

      // Lowest exitation = winner
      int winner,second;
      twoMinIndex(outLayer,winner,second);

      delta.subtract(input,weights[winner]);
      delta.multiply(lr[winner]);
      delta2.subtract(input,weights[second]);
      delta2.multiply(lr[second]);

      if ((outID[winner]==trainID || outID[second]==trainID)
          && ((outLayer[winner] < outLayer[second] ?
               outLayer[winner]/outLayer[second] :
               outLayer[second]/outLayer[winner]) > windowSize)) {
        if(outID[winner]!=outID[second]) {
          if(outID[winner]==trainID) {
            delta.add(weights[winner]);
            weights[winner]=delta;
            delta2.subtract(weights[second],delta2);
            weights[second]=delta2;
            lr[winner]/=(1+lr[winner]);
            lr[second]/=(1-lr[second]);
            if (lr[second]>learnRate) {
              lr[second]=learnRate;
            }
          } else {
            delta2.add(weights[second]);
            weights[second]=delta2;
            delta.subtract(weights[winner],delta);
            weights[winner]=delta;
            lr[second]/=(1+lr[second]);
            lr[winner]/=(1-lr[winner]);
            if (lr[winner]>learnRate) {
              lr[winner]=learnRate;
            }
          }
        } else if (outID[winner]==trainID) {
          delta.multiply(learnFactor);
          delta.add(weights[winner]);
          weights[winner]=delta;
          delta2.multiply(learnFactor);
          delta2.add(weights[second]);
          weights[second]=delta2;
          lr[winner]/=(1+lr[winner]);
          lr[second]/=(1+lr[second]);
        }
        modified=true;
      }
      return true;
    }

    return false;
  }

  // --------------------------------------------------
  // lvq::lvq4TrainFunctor
  // --------------------------------------------------
  bool lvq::lvq4TrainFunctor::operator()(const dvector& input,
                                         dmatrix& weights,
                                         dvector& outLayer,
                                         const ivector& outID,
                                         const int& trainID,
                                         bool& modified) {

    modified = false;

    if(firstUse) {
      delta.resize(input.size(),0.);
      delta2.resize(input.size(),0.);
      firstUse=false;
    }

    // Propagate
    if (prop(input,weights,outLayer)) {

      // Lowest exitation = winner
      int winner,second;
      twoMinIndex(outLayer,winner,second);

      delta.subtract(input,weights[winner]);
      delta.multiply(learnRate);
      delta2.subtract(input,weights[second]);
      delta2.multiply(learnRate);

      if ((outID[winner]==trainID || outID[second]==trainID)
          && ((outLayer[winner] < outLayer[second] ?
               outLayer[winner]/outLayer[second] :
               outLayer[second]/outLayer[winner]) > windowSize))  {
        if(outID[winner]!=outID[second]) {
          if(outID[winner]==trainID) {
            delta.add(weights[winner]);
            weights[winner]=delta;
            delta2.subtract(weights[second],delta2);
            weights[second]=delta2;
          } else {
            delta2.add(weights[second]);
            weights[second]=delta2;
            delta.subtract(weights[winner],delta);
            weights[winner]=delta;
          }
        } else if(outID[winner]==trainID) {
          delta.multiply(learnFactor);
          delta.add(weights[winner]);
          weights[winner]=delta;
        }
        modified = true;
      } else if (outID[winner]!=trainID) {
        delta.multiply(learnFactor);
        delta.subtract(weights[winner],delta);
        weights[winner]=delta;
        modified = true;
      }
      return true;
    }

    return false;
  }
  // --------------------------------------------------
  // lvq::randInitFunctor
  // --------------------------------------------------
  /*
   * LVQ ANNs are best initialized with the training data
   * therefore the somewhat bulky constructor
   */
  lvq::randInitFunctor::randInitFunctor(const dmatrix& theFeatures,
                                        const ivector& theTrainIDs,
                                        const ivector& theNbViewsObj,
                                        const int& theNbObj,
                                        const int& theNbNeurObj)
    : stdLayer::initFunctor(),features(theFeatures),trainIDs(theTrainIDs),
      nbViewsObj(theNbViewsObj), nbObj(theNbObj),nbNeurObj(theNbNeurObj) {
  }

  bool lvq::randInitFunctor::operator()(dmatrix& weights, ivector& outID) {

    sort<int>::parameters sortParam;
    sort<int> sorter; // default ascending order!

    imatrix randInd(nbObj,nbNeurObj,0);

    srand((unsigned)time(0));

    // Find nbNeurObj random training-patterns for each object to
    // initialize the weights

    // For this, first find indices of these patterns and sort them
    // in increasing order

    int k;
    int i;
    ivector temp;

    for(i=0; i<nbObj; i++) {
      temp.resize(nbViewsObj.at(i),0,false,false);
      int j;
      for(j=0;j<nbViewsObj.at(i);j++) {
        temp.at(j)=j;
      }
      for(j=0;j<nbNeurObj;j++) {
        k=int((double(rand())/RAND_MAX)*(nbViewsObj.at(i)-j));
        randInd[i][j]=temp[k];
        temp[k]=temp[nbViewsObj[i]-j-1];
      }

      sorter.apply(randInd.getRow(i));
    }

    ivector p(nbObj,0);
    ivector s(nbObj,0);

    // Now go through the training patterns once and for the chosen indices
    // copy the pattern into weights and assign a classID to the output neuron

    int j=0;
    for (i=0; i<trainIDs.size(); i++) {
      if (s[trainIDs[i]]==randInd[trainIDs[i]][p[trainIDs[i]]]) {
        weights[j]=features[i];
        outID[j++]=trainIDs[i];
        p[trainIDs[i]]++;
      }
      s[trainIDs[i]]++;
      if (p[trainIDs[i]]==nbNeurObj) {
        i+=nbViewsObj[trainIDs[i]]-randInd[trainIDs[i]][p[trainIDs[i]]-1]-1;
      }
    }

    return true;
  }


  lvq::maxDistInitFunctor::maxDistInitFunctor(const dmatrix& theFeatures,
                                              const ivector& theTrainIDs,
                                              const ivector& theNbViewsObj,
                                              const int& theNbObj,
                                              const int& theNbNeurObj,
                                              const eNormType& theNorm)
  : stdLayer::initFunctor(),features(theFeatures), trainIDs(theTrainIDs),
    nbViewsObj(theNbViewsObj), nbObj(theNbObj), nbNeurObj(theNbNeurObj),
    norm(theNorm) {
  }


  bool lvq::maxDistInitFunctor::operator()(dmatrix& weights, ivector& outID){

    int farthest=0; //feature vector farthest form others of the same
                    // class already selected

    int views=nbViewsObj.maximum();

    dmatrix distance;
    dvector absvalue;
    dvector cumDist;

    dmatrix clsVectors;
    dvector tmp;

    for(int obj=0; obj<nbObj; obj++) {

      views = nbViewsObj[obj];

      clsVectors=classVectors(features, trainIDs, views, obj);

      distance.resize(views,views,0.);
      absvalue.resize(views,views,0.);
      cumDist.resize(views,0.);

      //Find feature vector with lowest absolute value
      //(gotta start somewhere :-)
      int i;
      for(i=0; i<views; i++) {
        tmp=clsVectors.getRow(i);
        absvalue[i]=tmp.dot(tmp);
      }

      farthest=absvalue.getIndexOfMinimum();
      weights.setRow(obj*nbNeurObj, clsVectors.getRow(farthest));
      outID[obj*nbNeurObj]=obj;

      //Find the distances between prototypes the same class
      if (norm==L1distance) {
        l1Distance<double> l1norm;
        for(i=0; i<views; i++) {
          for(int j=i+1; j<views; j++) {
            distance[i][j]=l1norm.apply(clsVectors[i],clsVectors[j]);
            distance[j][i]=distance[i][j];
          }
        }
      } else {
        l2Distance<double> l2norm;

        for(i=0; i<views; i++) {
          for(int j=i+1; j<views; j++) {
            distance[i][j]=l2norm.apply(clsVectors[i],clsVectors[j]);
            distance[j][i]=distance[i][j];
          }
        }
      }

      //Always find the feature vector with the greatest mean (geometric)
      //distance to the already selected prototypes of each class

      cumDist=distance[farthest];
      for(i=1; i<nbNeurObj; i++) {
        farthest=cumDist.getIndexOfMaximum();
        weights.setRow(obj*nbNeurObj+i, clsVectors.getRow(farthest));
        outID[obj*nbNeurObj+i]=obj;
        cumDist.emultiply(distance[farthest]);
      }

    }


    return true;
  }

  // --------------------------------------------------
  // lvq
  // --------------------------------------------------

  /*
   * constructor
   */
  lvq::lvq()
    : supervisedInstanceClassifier(), lvqLayer("LVQ-Layer"), distProp(0), gaussAct(0),
      sizeIn(1), nbObj(1) {
//      trainStat(""), testStat(""),

    // default parameters
    parameters par;
    setParameters(par);

    // default output object
    outTemplate=outputTemplate();
  }

  lvq::lvq(const lvq& other)
    : supervisedInstanceClassifier(), lvqLayer("LVQ-Layer"),
      distProp(0),
      gaussAct(0), sizeIn(1),
      nbObj(1) {
//  , trainStat(""), testStat("")
    copy(other);
  }

  lvq::~lvq() {
    delete distProp;
    distProp = 0;

    delete gaussAct;
    gaussAct = 0;
  }

  bool lvq::train(const dmatrix& input,
                  const ivector& ids) {

    assert(ids.size()==input.rows());

    const parameters& param = getParameters();
//      output& nnoutput = getOutput();

    int i;
    char buf[1024];

    // initialze progress object

    if (validProgressObject()) {
      getProgressObject().reset();
      getProgressObject().setTitle("LVQ: Training");
      getProgressObject().setMaxSteps(param.nbPresentations1+
                                      param.nbPresentations2+
                                      4);

      getProgressObject().step("Initializing...");
    }

    sizeIn=input.columns();

    // extract usefull information from the input data

    // how many objects are in the ids vector?
    realToIntern.clear();

    int counter;
    std::map<int,int>::iterator mit;
    std::vector<int> nbViewsObjTmp;
    std::vector<int> internToRealTmp;

    // initialize the tables realToIntern, internToReal and nbViewsObj
    counter = 0;

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
    nbObj=counter;

    // initialize the member trainID, which contains the internal
    // class id for the respective input vector
    trainID.resize(ids.size(),0,false,false);

    for (i=0;i<ids.size();i++) {
      trainID.at(i) = realToIntern[ids.at(i)];
    }

    // resize the lvq layer
    lvqLayer.setSize(sizeIn,nbObj*param.nbNeuronsPerClass);

    // initialize nnoutput

    // resize the new output object
//      nnoutput.resize(nbObj*param.nbNeuronsPerClass,nbObj);
//      nnoutput.setClassifierName("LVQ");

    sigma.resize(nbObj*param.nbNeuronsPerClass,0.);

    features.copy(input);

    // index vector
    ivector index(ids.size(),0);

    // initialize index vector
    for(i=0;i<index.size();i++) {
      index[i]=i;
    }

    if (validProgressObject()) {
      getProgressObject().step("Initializing weights");
    }

    /*
     * create the initialization functor
     */
    stdLayer::initFunctor* lvqInit = 0;

    if (param.initType == parameters::LvqRand) {
      lvqInit = new randInitFunctor(features,
                                    trainID,
                                    nbViewsObj,
                                    nbObj,
                                    param.nbNeuronsPerClass);
    } else if (param.initType == parameters::LvqMaxDist) {
      lvqInit = new maxDistInitFunctor(features,
                                       trainID,
                                       nbViewsObj,
                                       nbObj,
                                       param.nbNeuronsPerClass,
                                       param.norm);
    } else {
      throw invalidParametersException(getTypeName());
    }

#   ifdef _DEBUG
    cout << "lvq::train: init weights\n";
#   endif

    lvqLayer.initWeights(*lvqInit);

    ivector trainStatIDs;

    // if the user wants training statistics ...

//      if (param.doStatistics) {
//        std::string name, extension, trainfile, testfile;
//        name.assign(param.statisticsFilename,0,
//                    param.statisticsFilename.rfind("."));
//        extension.assign(param.statisticsFilename,
//                         param.statisticsFilename.rfind("."),
//                         param.statisticsFilename.size());
//        trainfile=name+"_train"+extension;
//        testfile=name+"_test"+extension;

//        trainStat=classifyStatFunctor(trainfile.c_str());
//        testStat=classifyStatFunctor(testfile.c_str());

//        testStat.writeHeader();

//        trainStatIDs.resize(features.rows(),0);
//        int i;
//        for(i=0;i<features.rows();i++) {
//          trainStatIDs[i]=internToReal[trainID[i]];
//        }

//        ivector outID(lvqLayer.getOutID());


//        //TODO init outTemplate!

//        for(i=nbObj*param.nbNeuronsPerClass-1;i>=0;i--) {
//          nnoutput.getOutputVector()[i].objects.clear();
//          objectProb objProb;
//          objProb.id = internToReal[outID[i]];
//          objProb.prob =  1;
//          nnoutput.getOutputVector()[i].objects.push_back(objProb);
//        }
//      }

#   ifdef _DEBUG
    cout << "lvq::train: start training\n";
#   endif

    delete distProp;
    distProp = new stdLayer::distancePropFunctor(linearAct,param.norm);

//      if (param.doStatistics) {
//        statistics(testStat,testIDs,testFeatures,false);
//      }

    if (param.nbPresentations1!=0) {
      lvq::trainFunctor* trainer = 0;
      if (param.flagOlvq1) {
        trainer = new olvq1TrainFunctor(*distProp);
      }
      else {
        trainer = new lvq1TrainFunctor(*distProp);
      }

      trainer->setLearnRate(param.learnRate1);

      for(int k=0;k<param.nbPresentations1;k++) {
        // each time training patterns are presented in a different order
        scramble<int> scrambler;
        scrambler.apply(index);

        if (validProgressObject()) {
          sprintf(buf,"Training (O)LVQ1 step %i",k);
          getProgressObject().step(buf);
        }

        for(int i=0;i<index.size();i++)              {
          lvqLayer.train(features[index[i]],trainID[index[i]],*trainer);
        }

//          if (param.doStatistics)     {
//            statistics(testStat,testIDs,testFeatures,true);
//          }
      }
      delete trainer;
      trainer=0;
    }

    if (param.nbPresentations2!=0) {
      lvq::trainFunctor* trainer = 0;
      if (param.flagOlvq3) {
        trainer = new olvq3TrainFunctor(*distProp);
      } else {
        trainer = new lvq3TrainFunctor(*distProp);
      }

      trainer->setLearnFactor(param.learnRateFactor);
      trainer->setWindowSize(param.windowSize);
      trainer->setLearnRate(param.learnRate2);

      for(int k=0;k<param.nbPresentations2;k++) {
        scramble<int> scrambler;
        scrambler.apply(index);

        if (validProgressObject()) {
          sprintf(buf,"Training (O)LVQ3 step %i",k);
          getProgressObject().step(buf);
        }

        for(int i=0;i<index.size();i++) {
          lvqLayer.train(features[index[i]],trainID[index[i]],*trainer);
        }

//          if (param.doStatistics) {
//            statistics(testStat,testIDs,testFeatures,true);
//          }
      }
      delete trainer;
      trainer = 0;
    }

    lvqLayer.findSigmas(param.nbNeuronsPerClass, param.sigmaFactor,sigma);

    delete gaussAct;
    gaussAct = new stdLayer::gaussActFunctor(0.0,sigma);

    delete distProp;
    distProp = new stdLayer::distancePropFunctor(*gaussAct,param.norm);

    getProgressObject().step("Calculating probabilities");

    // Calculate object probabilities
    if (param.doTrain2) {
      ivector train2InternIds(train2IDs.size(),0);
      for(int i=0;i<train2InternIds.size();i++) {
        train2InternIds[i]=realToIntern[train2IDs[i]];
      }
      calcObjProbs(train2InternIds, train2Features, *distProp);
    } else {
      calcObjProbs(trainID, features, *distProp);
    }

    return true;
  }

  bool lvq::classify(const dvector& feature, outputVector& result) const {
//      const parameters& param = getParameters();

    dvector outStat;

    lvqLayer.propagate(feature,*distProp,outStat);

    double numerator=1./outStat.sumOfElements();
    outStat.multiply(numerator);

    return outTemplate.apply(outStat, result);
  }

#ifdef _INCLUDE_DEPRECATED

  // TODO: save is not ready!!!!!

//    bool lvq::save(std::ostream& outfile) {
//      const parameters& param = getParameters();
//      const output& nnoutput = getOutput();

//      //std::ofstream outfile;
//      //outfile.open(fname,std::ios::out);

//      if (!outfile.good()) {
//        if (validProgressObject()) {
//          std::string display = "Saving information FAILED!!!";
//          getProgressObject().step(display);
//        }
//        return false;
//      }

//      if (validProgressObject()) {
//        std::string display="Saving network information";
//        getProgressObject().step(display);
//      }

//      outfile << "LVQ Net\n"
//              << "========\n\n"
//              << "Input neurons: " << sizeIn << "\n"
//              << "Object classes: " << nbObj<< "\n"
//              << "Neurons per class: " << param.nbNeuronsPerClass << "\n"
//              << "Norm used: "
//              << (param.norm==L1distance ? "L1" : "L2") <<"\n";

//      if (param.nbPresentations1 != 0) {
//        outfile << "Trainingsteps " << ((param.flagOlvq1) ? "O":"") << "LVQ1: "
//                << param.nbPresentations1 << "\n";
//      }

//      if (param.nbPresentations2 != 0) {
//        outfile << "Trainingsteps " << ((param.flagOlvq3) ? "O":"") << "LVQ3: "
//                << param.nbPresentations2 << "\n"
//                << "\tWindow parameter: " << param.windowSize << "\n"
//                << "\tModification of learnrate: " << param.learnRateFactor
//                << "\n";
//      }

//      outfile << "\nOutputfunction sigmas:\n";

//      //outfile << sigma;

//      lispStreamHandler lsh(outfile);
//      lti::write(lsh,sigma);

//      outfile << "\n\n";

//      lvqLayer.save(outfile);

//      outfile <<"\n\nNNOutput:\n";
//      nnoutput.writeAscii(outfile);

//      //    outfile.close();

//      return true;
//    }

//    // TODO: load is not ready!!

//    bool lvq::load(std::istream& infile) {
//      static const int LineLen=1024;
//      parameters& param = getParameters();
//      output& nnoutput = getOutput();

//      int tlen;
//      char tmpstr[LineLen+1];

//      //std::ifstream infile;
//      //infile.open(fname,std::ios::in);
//      if (!infile.good()) {
//        return false;
//      }

//      tlen=strlen("Input neurons: ");

//      do {
//        infile.getline(tmpstr,LineLen);
//      } while(strncmp(tmpstr, "Input neurons: ", tlen)!=0);

//      sizeIn=atoi(&tmpstr[tlen]);
//      infile.getline(tmpstr,LineLen);
//      tlen=strlen("Object classes: ");
//      nbObj=atoi(&tmpstr[tlen]);
//      infile.getline(tmpstr,LineLen);
//      tlen=strlen("Neurons per class: ");
//      param.nbNeuronsPerClass=atoi(&tmpstr[tlen]);
//      infile.getline(tmpstr,LineLen);
//      tlen=strlen("Norm used: L");

//      int dummy=atoi(&tmpstr[tlen]);
//      param.norm= (dummy==1 ? L1distance : L2distance);
//      sigma.resize(nbObj*param.nbNeuronsPerClass,0.);
//      tlen=strlen("Outputfunction sigmas:\n");
//      do {
//        infile.getline(tmpstr,LineLen);
//      } while(strncmp(tmpstr, "Outputfunction sigmas:", tlen)!=0);
//      //infile >> sigma;

//      lispStreamHandler lsh(infile);
//      lti::read(lsh,sigma);

//      lvqLayer.load(infile);

//      tlen=strlen("NNOutput:");

//      do {
//        infile.getline(tmpstr,LineLen);
//      } while(strncmp(tmpstr, "NNOutput:", tlen)!=0);

//      nnoutput.readAscii(infile);

//      // infile.close();

//      param.nbPresentations1=0;
//      param.nbPresentations2=0;

//      delete gaussAct;
//      gaussAct = new stdLayer::gaussActFunctor(0.,sigma);

//      delete distProp;
//      distProp = new stdLayer::distancePropFunctor(*gaussAct,param.norm);

//      return true;
//    }

#endif // deprecated

  void lvq::setTestSet(const dmatrix& input,
                       const ivector& ids) {


    testIDs.copy(ids);
    testFeatures.copy(input);
  }

  void lvq::setTrainSet2(const dmatrix& input,
                         const ivector& ids) {

    train2IDs.copy(ids);
    train2Features.copy(input);

  }

//    void lvq::statistics(classifyStatFunctor& stat,
//                         const ivector& ids,
//                         const dmatrix& feats,
//                         const bool saveMe) {

//      const parameters& param = getParameters();
//      output& nnoutput = getOutput();
//      int i;

//      lvqLayer.findSigmas(param.nbNeuronsPerClass, param.sigmaFactor,sigma);

//      delete gaussAct;
//      gaussAct = new stdLayer::gaussActFunctor(0.,sigma);

//      delete distProp;
//      distProp = new stdLayer::distancePropFunctor(*gaussAct,param.norm);


//      for (i=0; i<feats.rows(); i++) {
//        classify(feats[i]);
//        stat(ids[i],nnoutput);
//      }

//      stat.writeStat();

//      bool yesReallySave = (param.correctVs3Best && stat.isMaxCorrect())
//                            ||(!param.correctVs3Best && stat.isMax3Best());

//      if (param.saveBest && saveMe && yesReallySave) {
//        // Calculate object probabilities
//        if (param.doTrain2) {
//          ivector train2InternIds(train2IDs.size(),0);
//          // first convert the real IDs to the internal ones
//          for(i=0;i<train2InternIds.size();i++) {
//            train2InternIds[i]=realToIntern[train2IDs[i]];
//          }
//          calcObjProbs(train2InternIds, train2Features, *distProp);
//        } else {
//          calcObjProbs(trainID, features, *distProp);
//        }

//  //        std::ofstream outfile(param.netFilename.c_str());
//  //        save(outfile);
//  //        outfile.close();

//        // set NNOutput ObjProbs back to 1
//        ivector outID(lvqLayer.getOutID());

//        for(i=0;i<nbObj*param.nbNeuronsPerClass;i++) {
//          nnoutput.getOutputVector()[i].objects.clear();
//          objectProb objProb;
//          objProb.id = internToReal[outID[i]];
//          objProb.prob =  1;
//          nnoutput.getOutputVector()[i].objects.push_back(objProb);
//        }

//      }

//      delete distProp;
//      distProp = new stdLayer::distancePropFunctor(linearAct);
//      stat.reset();
//    }

  void lvq::calcObjProbs(const ivector& internIds,
                         const dmatrix& feats,
                         stdLayer::distancePropFunctor& distProp) {

    const parameters& param = getParameters();

    imatrix clCount(nbObj*param.nbNeuronsPerClass,nbObj,0);
    ivector outID(lvqLayer.getOutID());
    int i,j,k;
    outputVector outV;
    bool b;

    for(i=0;i<feats.rows();i++) {
      b=classify(feats[i], outV);
      if (b) {
        clCount[outV.maxId()][internIds[i]]++;
      }
    }

    double rowsum;
    int rowsize;
    outTemplate=outputTemplate(nbObj*param.nbNeuronsPerClass);
    for(i=0;i<nbObj*param.nbNeuronsPerClass;i++) {
      rowsum=clCount[i].sumOfElements();
      outputVector rowV;
      if (rowsum!=0) {
        rowsize=0;
        for (j=0;j<nbObj;j++) {
          if (clCount[i][j]!=0) {
            rowsize++;
          }
        }
        rowV=outputVector(rowsize);
        for(j=0,k=0;j<nbObj;j++) {
          if (clCount[i][j]!=0) {
            rowV.setPair(k++, internToReal[j], clCount[i][j]/rowsum);
          }
        }
      } else {
        rowV=outputVector(1);
        rowV.setPair(0,internToReal[outID[i]],1.);
      }
    }
  }

  // return parameters
  const lvq::parameters& lvq::getParameters() const {
    const parameters* par =
      dynamic_cast<const lvq::parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // return parameters
//    lvq::parameters& lvq::getParameters() {
//      parameters* par =
//        dynamic_cast<lvq::parameters*>(&classifier::getParameters());
//      if(isNull(par)) {
//        throw invalidParametersException(getTypeName());
//      }
//      return *par;
//    }

  lvq& lvq::copy(const lvq& other) {
    delete progressBox;
    delete distProp;
    delete gaussAct;

    progressBox = other.getProgressObject().clone();
    distProp = new stdLayer::distancePropFunctor(*other.distProp);
    gaussAct = new stdLayer::gaussActFunctor(*other.gaussAct);

//      trainStat = other.trainStat;
//      testStat = other.testStat;

    sizeIn = other.sizeIn;
    nbObj  = other.nbObj;

    features.copy(other.features);
    sigma.copy(other.sigma);

    realToIntern = other.realToIntern;
    trainID = other.trainID;
    train2Features = other.train2Features;
    train2IDs = other.train2IDs;

    testFeatures = other.testFeatures;
    testIDs = other.testIDs;

    return *this;
  }

  classifier* lvq::clone() const {
    return new lvq(*this);
  }

}
