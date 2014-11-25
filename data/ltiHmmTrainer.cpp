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
 * file .......: ltiHmmTrainer.cpp
 * authors ....: Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 7.1.2002
 * revisions ..: $Id: ltiHmmTrainer.cpp,v 1.9 2006/09/05 10:40:07 ltilib Exp $
 */

#include "ltiObject.h"
#include <fstream>
#include "ltiHmmTrainer.h"

namespace lti {
  // --------------------------------------------------
  // hmmTrainer::parameters
  // --------------------------------------------------

  // default constructor
  hmmTrainer::parameters::parameters()
    : functor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    maxIterations = 0;
    convergenceThreshold = 0.0;

    numberOfStates = parameters::minimumSequenceLength;
    scoreFunction = hiddenMarkovModel::laplaceScore;
    estimatorFunction = estimMaximumLikelihood;
    minimumJump = 0;
    maximumJump = 2;
    initialScore = dvector();

    maxDensities = 5;
    reassignmentIterations = 5;
    firstSplit = 0;
    pooling = noPooling;
    minFramesForSplit = 3;
    minScalingFactor = 0.1;

  }

  // copy constructor
  hmmTrainer::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  hmmTrainer::parameters::~parameters() {
  }

  // get type name
  const char* hmmTrainer::parameters::getTypeName() const {
    return "hmmTrainer::parameters";
  }

  // copy member

  hmmTrainer::parameters&
    hmmTrainer::parameters::copy(const parameters& other) {
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

      maxIterations = other.maxIterations;
      convergenceThreshold = other.convergenceThreshold;

      numberOfStates = other.numberOfStates;
      scoreFunction = other.scoreFunction;
      estimatorFunction = other.estimatorFunction;
      minimumJump = other.minimumJump;
      maximumJump = other.maximumJump;
      initialScore = other.initialScore;

      maxDensities = other.maxDensities;
      reassignmentIterations = other.reassignmentIterations;
      firstSplit = other.firstSplit;
      pooling = other.pooling;
      minFramesForSplit = other.minFramesForSplit;
      minScalingFactor = other.minScalingFactor;

    return *this;
  }

  // alias for copy member
  hmmTrainer::parameters&
    hmmTrainer::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* hmmTrainer::parameters::clone() const {
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
  bool hmmTrainer::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool hmmTrainer::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      // training options
      lti::write(handler, "maxIterations",maxIterations);
      lti::write(handler, "convergenceThreshold",convergenceThreshold);

      // model options
      if (numberOfStates == minimumSequenceLength)
        lti::write(handler, "numberOfStates", "minimumSequenceLength");
      else if (numberOfStates == averageSequenceLength)
        lti::write(handler, "numberOfStates", "averageSequenceLength");
      else
        lti::write(handler,"numberOfStates",numberOfStates);

      switch (scoreFunction) {
      case hiddenMarkovModel::gaussScore:
        lti::write(handler, "scoreFunction", "gaussScore");
        break;
      case hiddenMarkovModel::laplaceScore:
        lti::write(handler, "scoreFunction", "laplaceScore");
        break;
      default:
        throw exception("unknown score function!");
      }

      switch(estimatorFunction) {
      case estimMaximumLikelihood:
        lti::write(handler, "estimatorFunction", "MaximumLikelihood");
        break;
      case estimStandardDeviation:
        lti::write(handler, "estimatorFunction", "StandardDeviation");
        break;
      case estimMeanDeviation:
        lti::write(handler, "estimatorFunction", "MeanDeviation");
        break;
      case estimMeanDeviationRoot:
        lti::write(handler, "estimatorFunction", "MeanDeviationRoot");
        break;
      default:
        throw exception("unknown estimator function!");
      }

      lti::write(handler,"minimumJump",minimumJump);
      lti::write(handler,"maximumJump",maximumJump);
      lti::write(handler,"initialScore",initialScore);

      // density options
      lti::write(handler,"maxDensities",maxDensities);
      lti::write(handler,"reassignmentIterations",reassignmentIterations);
      lti::write(handler,"firstSplit",firstSplit);

      if (pooling == modelPooling)
        lti::write(handler,"pooling","modelPooling");
      else if (pooling == statePooling)
        lti::write(handler,"pooling","statePooling");
      else
        lti::write(handler,"pooling","noPooling");

      lti::write(handler,"minFramesForSplit",minFramesForSplit);
      lti::write(handler,"minScalingFactor",minScalingFactor);
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
  bool hmmTrainer::parameters::write(ioHandler& handler,
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
  bool hmmTrainer::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool hmmTrainer::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      // training options
      lti::read(handler, "maxIterations", maxIterations);
      lti::read(handler, "convergenceThreshold", convergenceThreshold);

      // model options
      std::string temp;
      lti::read(handler, "numberOfStates", temp);
      if (temp == "minimumSequenceLength")
        numberOfStates = parameters::minimumSequenceLength;
      else if (temp == "averageSequenceLength")
        numberOfStates = parameters::averageSequenceLength;
      else
        numberOfStates = atoi(temp.c_str());

      lti::read(handler, "scoreFunction", temp);
      if (temp == "gaussScore")
        scoreFunction = hiddenMarkovModel::gaussScore;
      else if (temp == "laplaceScore")
        scoreFunction = hiddenMarkovModel::laplaceScore;
      else
        throw exception("unknown score function!");

      lti::read(handler, "estimatorFunction", temp);
      if (temp == "MaximumLikelihood")
        estimatorFunction = estimMaximumLikelihood;
      else if (temp == "StandardDeviation")
        estimatorFunction = estimStandardDeviation;
      else if (temp == "MeanDeviation")
        estimatorFunction = estimMeanDeviation;
      else if (temp == "MeanDeviationRoot")
        estimatorFunction = estimMeanDeviationRoot;
      else
        throw exception("unknown estimator function!");
      lti::read(handler,"minimumJump",minimumJump);
      lti::read(handler,"maximumJump",maximumJump);
      lti::read(handler,"initialScore",initialScore);

      // density options
      lti::read(handler,"maxDensities",maxDensities);
      lti::read(handler,"reassignmentIterations",reassignmentIterations);
      lti::read(handler,"firstSplit",firstSplit);

      lti::read(handler, "pooling", temp);
      if (temp == "modelPooling")
        pooling = modelPooling;
      else if (temp == "statePooling")
        pooling = statePooling;
      else {
        pooling = noPooling;          // default
        b = !(temp == "noPooling");   // b=false, if temp != noPooling
      }

      lti::read(handler,"minFramesForSplit",minFramesForSplit);
      lti::read(handler,"minScalingFactor", minScalingFactor);
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
  bool hmmTrainer::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // hmmTrainer
  // --------------------------------------------------

  // default constructor
  hmmTrainer::hmmTrainer()
    : functor() {

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

    // clear data
    reset();

    // log all operations for viterbi path search
    viterbi.logStateSelection();
    viterbi.logDensitySelection();
    viterbi.logEmissionScore();

    scoreSequence.clear();

  }

  // copy constructor
  hmmTrainer::hmmTrainer(const hmmTrainer& other)
    : functor()  {
    copy(other);
  }

  // destructor
  hmmTrainer::~hmmTrainer() {
  }

  // returns the name of this type
  const char* hmmTrainer::getTypeName() const {
    return "hmmTrainer";
  }

  // copy member
  hmmTrainer&
    hmmTrainer::copy(const hmmTrainer& other) {
      functor::copy(other);

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    theTrainingData = other.theTrainingData;
    assignedFrames = other.assignedFrames;
    featureDimension = other.featureDimension;
    minSequenceLength = other.minSequenceLength;
    totalFramesInSequences = other.totalFramesInSequences;
    return (*this);
  }

  // alias for copy member
  hmmTrainer&
    hmmTrainer::operator=(const hmmTrainer& other) {
    return (copy(other));
  }


  // clone member
  functor* hmmTrainer::clone() const {
    return new hmmTrainer(*this);
  }

  // return parameters
  const hmmTrainer::parameters&
    hmmTrainer::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------


  //
  // discard all considered sequences
  //
  void hmmTrainer::reset() {
    theTrainingData.resize(0);
    minSequenceLength = 0;
    totalFramesInSequences = 0;
    featureDimension = 0;
    assignedFrames.clear();
  }




  //
  // consider this sequence for computing the hidden markov model
  //
  void hmmTrainer::consider(const sequence<dvector> &theSeq){

    int sequenceLength = theSeq.size();
    if (sequenceLength) {

      if (sequenceLength < minSequenceLength || minSequenceLength == 0)
        minSequenceLength = sequenceLength;

      totalFramesInSequences += sequenceLength;

      if (theTrainingData.size())
        featureDimension = theTrainingData[0][0].size();
      else
        featureDimension = theSeq[0].size();

      // check if frame sizes are correct
      for (int frame = 0; frame < theSeq.size(); ++frame)
        if (theSeq[frame].size() != featureDimension)
          throw exception("frames have different feature dimensions");

      theTrainingData.append(theSeq);
    }

  }




  //
  // consider this sequence for computing the hidden markov model
  //
  void
  hmmTrainer::consider(const std::vector< sequence<dvector> > &theSequences){
    for (unsigned int seq=0; seq<theSequences.size(); ++seq)
      consider(theSequences[seq]);
  }




  //
  // generate hidden markov model from training data
  //
  void hmmTrainer::apply(hiddenMarkovModel &model) {

    // check training sequences
    if (!theTrainingData.size())
      throw exception("no training data given");

    // copy and modify parameters
    copyParameters();

    // no transitions, yet
    transitionFrequency.resize(params.numberOfStates, params.numberOfStates, 0, false, true);

    model.initialScore = params.initialScore;
    featureWeights = model.featureWeights;
    initializeModel(model);         // copy parameters to model
    initializeAssignments();        // assign frames to states (w/ one density) in a linear way

    double lastScore = hiddenMarkovModel::INFINITE_NUMBER;
    hiddenMarkovModel lastModel;        // save last model, in case scores become worse

    // register first score (infinite)
    scoreSequence.clear();

    calculateDensities();
    createModel(model);

    int it = 0;
    while (it < params.maxIterations || params.maxIterations == 0) {
      it++;

      // split criteria
      bool doSplit = false;
      if (params.maxDensities > 1 && it >= params.firstSplit) {
        if (params.reassignmentIterations)
          if (((it - params.firstSplit) % params.reassignmentIterations) == 0)
            doSplit = true;
      }

      // copy and improve existing model
      lastModel = model;
      double actScore = improveModel(model, doSplit);

      // register score
      scoreSequence.resize(scoreSequence.size() + 1, actScore);

      // check model quality
      if (lastScore - actScore <= params.convergenceThreshold) {
        if (lastScore < actScore) {
          model = lastModel;           // last model was better
          scoreSequence.resize(scoreSequence.size() - 1);
        }
        break;
      }

      lastScore = actScore;
    }
  }




  //
  // improve given model
  //
  double hmmTrainer::improveModel(hiddenMarkovModel &model, bool split) {

    // no transitions, yet
    transitionFrequency.resize(params.numberOfStates, params.numberOfStates,
                               0, false, true);

    // resize assignments to model and state size
    assignedFrames.resize(model.states.size());
    for (unsigned int s=0; s<model.states.size(); ++s)
      assignedFrames[s].resize(model.states[s].size());

    clearAssignments();


    // evaluate model with training data, get average score
    double averageScore = evaluateModel(model);

    garbageCollection();          // throw away empty densities and states
    calculateDensities();         // calculate mean, scale, weight from the current assignments

    if (split) {
      // calculate score per emission
      normalizeEmissionScores();

      splitDensities();
    }

    createModel(model);

    return averageScore;
  }




  //
  // copy parameters and modify if necessary
  //
  void hmmTrainer::copyParameters() {
    params = getParameters();

    if (params.numberOfStates == parameters::averageSequenceLength)
      params.numberOfStates = totalFramesInSequences /theTrainingData.size();
    else if (params.numberOfStates == parameters::minimumSequenceLength)
      params.numberOfStates = minSequenceLength;
    else if (params.numberOfStates < 0)
      params.numberOfStates = minSequenceLength;

    // modify convergenceThreshold, if necessary
    if (params.convergenceThreshold < 0.0)
      params.convergenceThreshold = 0.0;

    // modify initial scores, if necessary
    // (lti::vector::size returns signed int!)
    if (params.initialScore.size() != params.numberOfStates){
      params.initialScore.resize(params.numberOfStates,
                                 hiddenMarkovModel::INFINITE_NUMBER,
                                 false, true);
      params.initialScore[0] = 0.0;
    }

    // swap jump widths, if necessary
    if (params.minimumJump > params.maximumJump) {
      int temp = params.minimumJump;
      params.minimumJump = params.maximumJump;
      params.maximumJump = temp;
    }

    // modify reassignmentIterations, if necessary
    if (params.reassignmentIterations < 1)
      params.reassignmentIterations = 1;

    if (params.firstSplit == 0)
      params.firstSplit = params.reassignmentIterations;

    // select maximum likelihood estimator
    if (params.estimatorFunction == parameters::estimMaximumLikelihood)
      switch (params.scoreFunction) {
      case hiddenMarkovModel::gaussScore:
        params.estimatorFunction = parameters::estimStandardDeviation;
        break;
      case hiddenMarkovModel::laplaceScore:
        params.estimatorFunction = parameters::estimMeanDeviation;
        break;
      default:
        throw exception("unknown estimator function!");
      }


  }




  //
  // assign frame to state/density in a linear fashion
  //
  void hmmTrainer::initializeAssignments() {
    assignedFrames.resize(params.numberOfStates);

    // initialize assignment fields (one empty density per state)
    for (int st=0; st<params.numberOfStates; ++st)
      assignedFrames[st].resize(1);
    clearAssignments();

    // assign frames to states linearly for all training sequences
    for (int seq=0; seq<theTrainingData.size(); ++seq) {
      double frameStep = theTrainingData[seq].size() / double(params.numberOfStates);
      double frameCounter = 0.0;
      int actFrame = 0;

      for (int st = 0; st<params.numberOfStates; ++st) {
        // step
        frameCounter += frameStep;

        // "overflow"?
        while (frameCounter >= 0.5) {
          assignedFrames[st][0].frames.push_back(&theTrainingData[seq][actFrame++]);
          frameCounter -= 1.0;
        }
      }
    }

    garbageCollection();          // throw away empty densities and states
  }




  //
  // clear assigned frames and scores for all states and densities
  //
  void hmmTrainer::clearAssignments() {
    for (unsigned int st = 0; st < assignedFrames.size(); ++st)
      for (unsigned int dens = 0; dens < assignedFrames[st].size(); ++dens) {
        // no frames assigned
        assignedFrames[st][dens].frames.clear();
        assignedFrames[st][dens].score = 0.0;
      }
  }




  //
  // initialize the model
  //
  void hmmTrainer::initializeModel(hiddenMarkovModel &model) {
    // model.initialScore = params.initialScore;
    model.featureDimension = featureDimension;
    model.scoreFunction = params.scoreFunction;
  }




  //
  // calculate transition matrix, i.e. do smoothing and -log(x)
  //
  void hmmTrainer::calculateTransitionMatrix(matrix<double> &transitionScore) {

    transitionScore.resize(transitionFrequency.size());

    for (int fromState = 0; fromState < params.numberOfStates; ++fromState) {

      int minState;
      int maxState;

      // jump out of range?
      if (fromState + params.minimumJump < 0)
        minState = 0;
      else
        minState = fromState + params.minimumJump;

      if (fromState + params.maximumJump < params.numberOfStates)
        maxState = fromState + params.maximumJump;
      else
        maxState = params.numberOfStates - 1;

      int transitionsThroughState = 0;
      int toState;

      // all valid jumps must have probability > 0
      for (toState = minState; toState <= maxState; ++toState) {
        // there are be better ways do this smoothing operation
        // -> subject to be changed (possibly via parameters)
        int &freq = transitionFrequency[fromState][toState];
        if (freq == 0)
          freq = 1;
        transitionsThroughState += freq;

        // simple alternative:
        // transitionsThroughState+=++transitionFrequency[fromState][toState];
      }

      // transition frequency -> transition probability -> transition score
      for (toState = 0; toState < params.numberOfStates; ++toState) {
        int &freq = transitionFrequency[fromState][toState];

        if (!freq)
          transitionScore[fromState][toState] =
            hiddenMarkovModel::INFINITE_NUMBER;
        else
          transitionScore[fromState][toState] =
            -log(freq / double(transitionsThroughState));
      }

    }

  }




  //
  // calculate transition matrix from transitions, copy densities
  //
  void hmmTrainer::createModel(hiddenMarkovModel &model) {

    model.initialScore = params.initialScore;
    calculateTransitionMatrix(model.transitionScore);

    // resize model to appropriate number of states
    model.states.resize(assignedFrames.size());

    // for all states
    for (unsigned int st = 0; st < assignedFrames.size(); ++st) {
      model.states[st].resize(assignedFrames[st].size());

      // copy density center
      for (unsigned int dens = 0; dens < assignedFrames[st].size(); ++dens)
        model.states[st][dens] = assignedFrames[st][dens].center;
    }
  }




  //
  // do viterbi path search for all training data
  // return average score
  //
  double hmmTrainer::evaluateModel(const hiddenMarkovModel &model) {
    double thisScore = 0.0;

    // run viterbi algorithm for all given training sequences
    // register transition scores and frame->density assignments
    for (int seq = 0; seq<theTrainingData.size(); ++seq)
      thisScore += considerTrainingSequence(model, theTrainingData[seq]);

    // average score
    thisScore /= double(theTrainingData.size());

    return thisScore;
  }




  //
  // calculate mean values, scales and weight scores
  //
  void hmmTrainer::calculateDensities() {

    resetScaleCalculation();

    unsigned int st;
    unsigned int dens;

    dvector tempVec;

    // iterate through all states
    for (st = 0; st < assignedFrames.size(); ++st) {
      stateAssignment &thisState = assignedFrames[st];

      int framesInState = 0;

      for (dens = 0; dens < thisState.size(); ++dens) {
        densityAssignment &thisDensity = thisState[dens];

        thisDensity.center.weightScore = thisDensity.frames.size();
        framesInState += thisDensity.frames.size();

        switch (params.pooling) {
        case parameters::noPooling:
          calculateThisDensity(thisDensity);
          break;

        case parameters::statePooling:
        case parameters::modelPooling:
          calculateDensityCenter(thisDensity.frames, thisDensity.center.mean);
          for (unsigned int frame = 0;
               frame < thisDensity.frames.size();
               ++frame)
            considerScale(thisDensity.center.mean,
                          *(thisDensity.frames[frame]));
          break;
        }
      }

      // calculate score weights for densities
      for (dens = 0; dens < thisState.size(); ++dens) {
        double &weight = thisState[dens].center.weightScore;
        weight = -log(weight / framesInState);
      }

      // state pooling
      if (params.pooling == parameters::statePooling) {
        calculateScale(tempVec);
        for (dens = 0; dens < thisState.size(); ++dens)
          thisState[dens].center.scalingFactor = tempVec;

        resetScaleCalculation();
      }
    }

    // model pooling
    if (params.pooling == parameters::modelPooling) {
      calculateScale(tempVec);
      for (st = 0; st < assignedFrames.size(); ++st)
        for (dens = 0; dens < assignedFrames[st].size(); ++dens)
          assignedFrames[st][dens].center.scalingFactor = tempVec;

      resetScaleCalculation();
    }
  }




  //
  // calculate density from the given density assignment
  //
  void hmmTrainer::calculateThisDensity(densityAssignment &theDensity) {

    int frameCount = theDensity.frames.size();

    // calculate mean
    calculateDensityCenter(theDensity.frames, theDensity.center.mean);

    // calculate scaling facator
    resetScaleCalculation();
    for (int frame = 0; frame < frameCount; ++frame)
      considerScale(theDensity.center.mean, *(theDensity.frames[frame]));

    calculateScale(theDensity.center.scalingFactor);
  }




  //
  // calculate density center
  //
  void hmmTrainer::calculateDensityCenter(const std::vector<const dvector *> &frames, dvector &mean) const {
    mean.resize(featureDimension, 0.0, false, true);

    int frameCount = frames.size();

    for (int frame = 0; frame < frameCount; ++frame) {
      const dvector &thisFrame = *(frames[frame]);

      // add all frames
      for (int dim=0; dim < featureDimension; ++dim)
        mean[dim] += thisFrame[dim];
    }

    // normalize mean
    for (int dim=0; dim < featureDimension; ++dim)
      mean[dim] = mean[dim] / double(frameCount);
  }




  //
  // do garbage collection: delete empty states and densities
  //
  void hmmTrainer::garbageCollection() {
    // check all states
    for (unsigned int st = 0; st < assignedFrames.size(); ++st) {

      // check all densities
      for (unsigned int dens = 0; dens < assignedFrames[st].size(); ++dens)
        if (!assignedFrames[st][dens].frames.size())
          deleteDensity(st, dens--);

      if (!assignedFrames[st].size())
        deleteState(st--);
    }

  }




  //
  // calculate transition scores from the viterbi search, reassign densities of the sequence
  // return achieved score
  //
  double hmmTrainer::considerTrainingSequence(const hiddenMarkovModel &currentModel, const sequence<dvector> &theSeq) {
    double modelScore = viterbi.apply(currentModel, theSeq);

    ivector lastPath = viterbi.getStatePath();
    ivector lastDens = viterbi.getDensitySelectionPath();
    dvector lastScores = viterbi.getEmissionScorePath();

    for (int frame = 0; frame < lastPath.size(); ++frame) {
      int theState = lastPath[frame];
      int theDensity = lastDens[frame];
      double theScore = lastScores[frame];

      // count transitions
      if (frame > 0)
        transitionFrequency[lastPath[frame - 1]][theState] += 1;

      // assign frame to selected density
      assignedFrames[theState][theDensity].frames.push_back(&theSeq[frame]);

      assignedFrames[theState][theDensity].score += theScore;
    }

    return modelScore;
  }




  //
  // check density scores and split if appropriate
  //
  bool hmmTrainer::splitDensities() {

    int numberOfDensities = 0;
    double averageScore = 0.0;

    bool newDensityCenters = false;

    unsigned int st;

    // calculate average emission score of all density centers
    for (st = 0; st < assignedFrames.size(); ++st) {
      numberOfDensities += assignedFrames[st].size();
      for (unsigned int dens = 0; dens < assignedFrames[st].size(); ++dens)
        averageScore += assignedFrames[st][dens].score;
    }

    averageScore /= double(numberOfDensities);

    // check all densities for split criterion
    for (st = 0; st < assignedFrames.size(); ++st) {
      stateAssignment &thisState = assignedFrames[st];

      int stateSize = thisState.size();

      for (int dens = 0; dens < stateSize; ++dens) {

        // no further splitting allowed
        if (static_cast<int>(thisState.size()) == params.maxDensities)
          break;

        densityAssignment &thisDensity = thisState[dens];
        // split criterion: emission score > average score
        if ((thisDensity.score > averageScore) &&
            (static_cast<int>(thisDensity.frames.size()) >=
             params.minFramesForSplit)) {
          densityAssignment newDensity;

          // split
          splitThisDensity(thisDensity, newDensity);

          // save new mixture density
          if (newDensity.frames.size()) {
            if (thisDensity.frames.size())
              thisState.push_back(newDensity);
            else
              thisDensity = newDensity;
          }

          newDensityCenters = true;
        }
      }
    }

    if (newDensityCenters)
      calculateDensities();   // only, if densities were splitted

    return newDensityCenters;
  }




  //
  // normalize emission scores
  //
  void hmmTrainer::normalizeEmissionScores() {

    for (unsigned int st = 0; st < assignedFrames.size(); ++st)
      for (unsigned int dens = 0; dens < assignedFrames[st].size(); ++dens)
        assignedFrames[st][dens].score /=
          double(assignedFrames[st][dens].frames.size());
  }




  //
  // split one density center by disturbance of the center
  //
  void hmmTrainer::splitThisDensity(densityAssignment &thisDensity, densityAssignment &newDensity) {

    // save old density assignments
    densityAssignment oldDensity = thisDensity;

    thisDensity.frames.clear();
    thisDensity.score = 0.0;

    newDensity.frames.clear();
    newDensity.score = 0.0;

    // reassign frames to the center with the lower score
    for (unsigned int frame = 0; frame < oldDensity.frames.size(); ++frame) {
      const dvector &thisFrame = *(oldDensity.frames[frame]);

      double leftScore = 0.0;
      double rightScore = 0.0;

      for (int dim = 0; dim < featureDimension; ++dim) {
        double dev = oldDensity.center.scalingFactor[dim];
        double featureWeight = (featureWeights.size() > dim ? featureWeights[dim] : 1.0f);

        if (dev > 0.0) {
          double rDiff = thisFrame[dim] - oldDensity.center.mean[dim];
          double lDiff = rDiff;

          rDiff += 1.0;
          lDiff -= 1.0;

          switch(params.scoreFunction) {
          case hiddenMarkovModel::gaussScore:
            lDiff *= lDiff;
            rDiff *= rDiff;
            break;
          case hiddenMarkovModel::laplaceScore:
            lDiff = abs(lDiff);
            rDiff = abs(rDiff);
            break;
          default:
            throw exception("score function not implemented (splitThisDensity)!");
            break;
          }

          leftScore += featureWeight * lDiff / dev;
          rightScore += featureWeight * rDiff / dev;
        }
      }

      // assign to new center
      if (rightScore < leftScore)
        newDensity.frames.push_back(&thisFrame);
      else
        thisDensity.frames.push_back(&thisFrame);
    }
  }




  //
  // delete state
  //
  void hmmTrainer::deleteState(const int stateNumber) {

    std::vector<stateAssignment>::iterator stateIt = assignedFrames.begin();

    // iterate through vector
    for (int st1 = 0; st1 < stateNumber; ++st1, ++stateIt);

    // delete state
    assignedFrames.erase(stateIt);

    params.numberOfStates = assignedFrames.size();

    // delete element from initialScore, delete row from transitionFrequency
    for (unsigned int st = stateNumber; st < assignedFrames.size(); ++st) {
      transitionFrequency[st] = transitionFrequency[st+1];
      params.initialScore[st] = params.initialScore[st+1];
    }

    // delete column from transitionFrequency
    for (int r = 0; r < params.numberOfStates; ++r)
      for (int c = stateNumber; c < params.numberOfStates; c++)
        transitionFrequency[r][c] = transitionFrequency[r][c+1];

    params.initialScore.resize(params.numberOfStates);
    transitionFrequency.resize(params.numberOfStates, params.numberOfStates);
  }




  //
  // delete density
  //
  void hmmTrainer::deleteDensity(const int stateNumber, const int densityNumber) {


    stateAssignment::iterator densIt = assignedFrames[stateNumber].begin();

    // iterate through vector
    for (int dens = 0; dens < densityNumber; ++dens, ++densIt);

    // delete density
    assignedFrames[stateNumber].erase(densIt);
  }




  //
  //reset scale calculation
  //
  void hmmTrainer::resetScaleCalculation() {
    scaleVector.resize(featureDimension, 0.0, false, true);
    scaleConsidered = 0;
  }




  //
  // scale calculation: take vector into consideration
  //
  void hmmTrainer::considerScale(const dvector &mean, dvector vec) {
    double diff;
    for (int dim = 0; dim < featureDimension; ++dim) {
      diff = vec[dim] - mean[dim];

      switch(params.estimatorFunction) {
      case parameters::estimStandardDeviation:
        scaleVector[dim] += diff*diff;
        break;
      case parameters::estimMeanDeviation:
      case parameters::estimMeanDeviationRoot:
        scaleVector[dim] += abs(diff);
        break;
      default:
        throw exception("estimator function not implemented (considerScale)!");
        break;
      }
    }
    ++scaleConsidered;
  }




  //
  // calculate scaling factor from considered vectors
  //
  void hmmTrainer::calculateScale(dvector &scalingFactor) {
    scalingFactor.resize(featureDimension, params.minScalingFactor, false, true);
    if (!scaleConsidered)
      return;

    for (int dim = 0; dim < featureDimension; ++dim) {

      switch(params.estimatorFunction) {
      case parameters::estimStandardDeviation:
        scalingFactor[dim] = sqrt(scaleVector[dim] / double(scaleConsidered));;
        break;
      case parameters::estimMeanDeviation:
        scalingFactor[dim] = scaleVector[dim] / double(scaleConsidered);
        break;
      case parameters::estimMeanDeviationRoot:
        scalingFactor[dim] = sqrt(scaleVector[dim] / double(scaleConsidered));
        break;
      default:
        throw exception("estimator function not implemented (calculateScale)!");
        break;
      }
      if (scalingFactor[dim] < params.minScalingFactor)
        scalingFactor[dim] = params.minScalingFactor;
    }
  }


}
