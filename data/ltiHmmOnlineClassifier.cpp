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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltihmmOnlineClassifier.cpp
 * authors ....: Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 09.07.2002
 * revisions ..: $Id: ltiHmmOnlineClassifier.cpp,v 1.8 2006/09/05 09:58:41 ltilib Exp $
 */

#include "ltiObject.h"
#include "ltiArray.h"
#include "ltiHmmOnlineClassifier.h"

namespace lti {

  // --------------------------------------------------
  // hmmOnlineClassifier::parameters
  // --------------------------------------------------

  // default constructor
  hmmOnlineClassifier::parameters::parameters()
    : hmmClassifier::parameters() {

    pruningThreshold = 1000;
    maxActiveHypotheses = 1000;
    numberOfBuckets = 100;
    automaticTraceBack = 100;
  }

  // copy constructor
  hmmOnlineClassifier::parameters::parameters(const parameters& other)
    : hmmClassifier::parameters() {
    copy(other);
  }

  // destructor
  hmmOnlineClassifier::parameters::~parameters() {
  }

  // get type name
  const char* hmmOnlineClassifier::parameters::getTypeName() const {
    return "hmmOnlineClassifier::parameters";
  }

  // copy member
  hmmOnlineClassifier::parameters&
    hmmOnlineClassifier::parameters::copy(const parameters& other) {

# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    hmmClassifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    hmmClassifier::parameters& (hmmClassifier::parameters::* p_copy)
      (const hmmClassifier::parameters&) =
      hmmClassifier::parameters::copy;
    (this->*p_copy)(other);
# endif


    pruningThreshold = other.pruningThreshold;
    maxActiveHypotheses = other.maxActiveHypotheses;
    numberOfBuckets = other.numberOfBuckets;
    automaticTraceBack = other.automaticTraceBack;


    return *this;
  }

  // alias for copy member
  hmmOnlineClassifier::parameters&
    hmmOnlineClassifier::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  classifier::parameters* hmmOnlineClassifier::parameters::clone() const {
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
  bool hmmOnlineClassifier::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool hmmOnlineClassifier::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler, "pruningThreshold", pruningThreshold);
      lti::write(handler, "maxActiveHypotheses", maxActiveHypotheses);
      lti::write(handler, "numberOfBuckets", numberOfBuckets);
      lti::write(handler, "automaticTraceBack", automaticTraceBack);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && hmmClassifier::parameters::write(handler,false);
# else
    bool (hmmClassifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      hmmClassifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool hmmOnlineClassifier::parameters::write(ioHandler& handler,
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
  bool hmmOnlineClassifier::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool hmmOnlineClassifier::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler, "pruningThreshold", pruningThreshold);
      lti::read(handler, "maxActiveHypotheses", maxActiveHypotheses);
      lti::read(handler, "numberOfBuckets", numberOfBuckets);
      lti::read(handler, "automaticTraceBack", automaticTraceBack);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && hmmClassifier::parameters::read(handler,false);
# else
    bool (hmmClassifier::parameters::* p_readMS)(ioHandler&,const bool) =
      hmmClassifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool hmmOnlineClassifier::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif



  // --------------------------------------------------
  // hmmOnlineClassifier::wordHypothesis
  // --------------------------------------------------

  // default constructor
  hmmOnlineClassifier::wordHypothesis::wordHypothesis() : ioObject() {
    modelID = -1;
    wordLength = 1;
    resultVector.clear();
  }

  // copy constructor
  hmmOnlineClassifier::wordHypothesis::wordHypothesis(const hmmOnlineClassifier::wordHypothesis& other)
    : ioObject() {
    copy(other);
  }

  // destructor
  hmmOnlineClassifier::wordHypothesis::~wordHypothesis() {
  }

// copy constructor
  hmmOnlineClassifier::wordHypothesis& hmmOnlineClassifier::wordHypothesis::copy(const hmmOnlineClassifier::wordHypothesis& other) {
  // copy data from other hmmOnlineClassifier
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    ioObject::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    ioObject& (ioObject::* p_copy)
      (const ioObject&) =
      ioObject::copy;
    (this->*p_copy)(other);
# endif

    modelID = other.modelID;
    wordLength = other.wordLength;
    resultVector = other.resultVector;

    return *this;
  }

  // assigment operator (alias for copy(other)).
  hmmOnlineClassifier::wordHypothesis& hmmOnlineClassifier::wordHypothesis::operator=(const hmmOnlineClassifier::wordHypothesis& other) {
    return copy(other);
  }

  // get type name
  const char* hmmOnlineClassifier::wordHypothesis::getTypeName() const {
    return "hmmOnlineClassifier::wordHypothesis";
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */

# ifndef _LTI_MSC_6
  bool hmmOnlineClassifier::wordHypothesis::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool hmmOnlineClassifier::wordHypothesis::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler, "modelID", modelID);
      lti::write(handler, "wordLength", wordLength);
      lti::write(handler, "resultVector", resultVector);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && ioObject::write(handler,false);
# else
    bool (ioObject::* p_writeMS)(ioHandler&,const bool) const =
      ioObject::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool hmmOnlineClassifier::wordHypothesis::write(ioHandler& handler,
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
  bool hmmOnlineClassifier::wordHypothesis::read(ioHandler& handler,
                                        const bool complete)
# else
  bool hmmOnlineClassifier::wordHypothesis::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler, "modelID", modelID);
      lti::read(handler, "wordLength", wordLength);
      lti::read(handler, "resultVector", resultVector);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && ioObject::read(handler,false);
# else
    bool (ioObject::* p_readMS)(ioHandler&,const bool) =
      ioObject::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool hmmOnlineClassifier::wordHypothesis::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif



  // --------------------------------------------------
  // hmmOnlineClassifier::wordEndHypothesis
  // --------------------------------------------------

  // default constructor
  hmmOnlineClassifier::wordEndHypothesis::wordEndHypothesis()
    : ioObject() {
    modelID = -1;
    wordLength = 1;
    score = static_cast<float>(hiddenMarkovModel::INFINITE_NUMBER);
  }

  // copy constructor
  hmmOnlineClassifier::wordEndHypothesis::wordEndHypothesis(const hmmOnlineClassifier::wordEndHypothesis& other) 
    : ioObject() {
    copy(other);
  }

  // destructor
  hmmOnlineClassifier::wordEndHypothesis::~wordEndHypothesis() {
  }

// copy constructor
  hmmOnlineClassifier::wordEndHypothesis& hmmOnlineClassifier::wordEndHypothesis::copy(const hmmOnlineClassifier::wordEndHypothesis& other) {
  // copy data from other hmmOnlineClassifier
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    ioObject::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    ioObject& (ioObject::* p_copy)
      (const ioObject&) =
      ioObject::copy;
    (this->*p_copy)(other);
# endif

    modelID = other.modelID;
    wordLength = other.wordLength;
    score = other.score;

    return *this;
  }

  // assigment operator (alias for copy(other)).
  hmmOnlineClassifier::wordEndHypothesis& hmmOnlineClassifier::wordEndHypothesis::operator=(const hmmOnlineClassifier::wordEndHypothesis& other) {
    return copy(other);
  }

  // get type name
  const char* hmmOnlineClassifier::wordEndHypothesis::getTypeName() const {
    return "hmmOnlineClassifier::wordEndHypothesis";
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */

# ifndef _LTI_MSC_6
  bool hmmOnlineClassifier::wordEndHypothesis::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool hmmOnlineClassifier::wordEndHypothesis::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler, "modelID", modelID);
      lti::write(handler, "wordLength", wordLength);
      lti::write(handler, "score", score);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && ioObject::write(handler,false);
# else
    bool (ioObject::* p_writeMS)(ioHandler&,const bool) const =
      ioObject::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool hmmOnlineClassifier::wordEndHypothesis::write(ioHandler& handler,
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
  bool hmmOnlineClassifier::wordEndHypothesis::read(ioHandler& handler,
                                        const bool complete)
# else
  bool hmmOnlineClassifier::wordEndHypothesis::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler, "modelID", modelID);
      lti::read(handler, "wordLength", wordLength);
      lti::read(handler, "score", score);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && ioObject::read(handler,false);
# else
    bool (ioObject::* p_readMS)(ioHandler&,const bool) =
      ioObject::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool hmmOnlineClassifier::wordEndHypothesis::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif



  // --------------------------------------------------
  // hmmOnlineClassifier
  // --------------------------------------------------


  // constructor
  hmmOnlineClassifier::hmmOnlineClassifier()
    : hmmClassifier() {

    parameters theParameters;
    setParameters(theParameters);
  }

  // copy constructor
  hmmOnlineClassifier::hmmOnlineClassifier(const hmmOnlineClassifier& other)
    : hmmClassifier(other)  {
    copy(other);
  }

  // destructor
  hmmOnlineClassifier::~hmmOnlineClassifier() {
  }

  // returns the name of this type
  const char* hmmOnlineClassifier::getTypeName() const {
    return "hmmOnlineClassifier";
  }

  // copy member
  hmmOnlineClassifier&
    hmmOnlineClassifier::copy(const hmmOnlineClassifier& other) {

    hmmClassifier::copy(other);
    return (*this);
  }

  // alias for copy member
  hmmOnlineClassifier&
    hmmOnlineClassifier::operator=(const hmmOnlineClassifier& other) {
    return (copy(other));
  }

  // return pointer to copy of classifier
  classifier* hmmOnlineClassifier::clone() const {
    return new hmmOnlineClassifier(*this);
  }

  // return parameters
  const hmmOnlineClassifier::parameters&
  hmmOnlineClassifier::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }




  // write method
  bool hmmOnlineClassifier::write(ioHandler &handler, const bool complete) const {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    lti::write(handler, "activeHypotheses", activeHypotheses);
    lti::write(handler, "traceBackField", traceBackField);
    lti::write(handler, "bestHypothesis", bestHypothesis);
    lti::write(handler, "overallActiveHypotheses", overallActiveHypotheses);

    // write out internal partial traceback information
    lti::write(handler, "timestepCounter", timestepCounter);
    lti::write(handler, "partialWordHyp", partialWordHyp);
    lti::write(handler, "partialWordLengths", partialWordLengths);
    lti::write(handler, "partialWordEndScores", partialWordEndScores);

    // partial zeugs noch rausschreiben!

    b = b && hmmClassifier::write(handler,false);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }




  // read method
  bool hmmOnlineClassifier::read(ioHandler &handler, const bool complete) {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    lti::read(handler, "activeHypotheses", activeHypotheses);
    lti::read(handler, "traceBackField", traceBackField);
    lti::read(handler, "bestHypothesis", bestHypothesis);
    lti::read(handler, "overallActiveHypotheses", overallActiveHypotheses);

    // read internal partial traceback information
    lti::read(handler, "timestepCounter", timestepCounter);
    lti::read(handler, "partialWordHyp", partialWordHyp);
    lti::read(handler, "partialWordLengths", partialWordLengths);
    lti::read(handler, "partialWordEndScores", partialWordEndScores);

    b = b && hmmClassifier::read(handler,false);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }


  //
  // hmmOnlineClassifier training and classification methods:
  //



  // delete all models
  void hmmOnlineClassifier::reset() {
    hmmClassifier::reset();

    initialize();
  }





  // delete traceback field and active hypotheses
  void hmmOnlineClassifier::initialize() {
    activeHypotheses.clear();
    traceBackField.clear();
    overallActiveHypotheses = 0;

    bestHypothesis = wordHypothesis();

    // clear partial traceback
    timestepCounter = 0;
    partialWordHyp.clear();
    partialWordLengths.clear();
    partialWordEndScores.clear();

  }






  // prune one model hypothesis, return number of active hypotheses
  // adjust minScore and maxScore
  int hmmOnlineClassifier::pruneModelHypothesis(dvector &resultVector, float &minScore, float &maxScore) const {

    int returnValue = 0;

    // new minimum score? new valid maximum score?
    float temp;
    int d;
    for (d=0; d<resultVector.size(); ++d) {
      temp = static_cast<float>(resultVector[d]);
      if (temp < minScore)
        minScore = temp;
      if (temp > maxScore && temp < hiddenMarkovModel::INFINITE_NUMBER)
        maxScore = temp;
    }

    // no score-pruning: all hypotheses valid
    if (pruningThreshold < 0)
      return resultVector.size();

    // delete invalid hypotheses
    for (d=0; d<resultVector.size(); ++d)
      if (resultVector[d] > minScore + pruningThreshold)
        resultVector[d] = hiddenMarkovModel::INFINITE_NUMBER;
      else
        ++returnValue;

    return returnValue;
  }







  // perform online classification
  bool hmmOnlineClassifier::classify(const dvector& observation) {

    if (theModels.size() == 0) {
      setStatusString("classifier is untrained");
      return false;
    }

    hmmViterbiPathSearch theViterbiPathSearch;

    pruningThreshold = getParameters().pruningThreshold;

    // initialize with values that will be immediately overwritten
    float minScore = static_cast<float>(hiddenMarkovModel::INFINITE_NUMBER);
    float maxScore = - minScore;



    //
    // (1) expand best hypothesis first to get a reliable pruning value.
    //

    // the best hypothesis is very likely to be the successor of the current best hypothesis.
    if (bestHypothesis.modelID >= 0) {

      theViterbiPathSearch.subsequentStep(theModels[bestHypothesis.modelID], observation, bestHypothesis.resultVector);

      // find new minimum score
      for (int d=0; d<bestHypothesis.resultVector.size(); ++d)
        if (bestHypothesis.resultVector[d] < minScore)
          minScore = static_cast<float>(bestHypothesis.resultVector[d]);

      ++bestHypothesis.wordLength;
    }


    // find score of best word end hypothesis
    float startingScore = 0.0f;
    if (traceBackField.size())
      startingScore = traceBackField.back().score;

    // penalty-score: -ln(1/models)
    // might be replaced by a simple language model (unigram, bigram)
    startingScore += log(static_cast<float>(theModels.size()));



    //
    // (2) start new models
    //

    // only start models, if word transition is possible
    if (startingScore < hiddenMarkovModel::INFINITE_NUMBER) {

      // start with new instance for all given models
      std::map<int, hiddenMarkovModel>::const_iterator it = theModels.begin();
      while (!(it == theModels.end())) {
        wordHypothesis newHypothesis;

        newHypothesis.modelID = (*it).first;
        newHypothesis.wordLength = 0;
        newHypothesis.resultVector.resize((*it).second.states.size(), startingScore, false, true);

        activeHypotheses.push_back(newHypothesis);

        ++it;
      }
    }

    // the best word end hypothesis will be saved here
    wordEndHypothesis bestWordEnd;
    bestWordEnd.wordLength = traceBackField.size() + 1;



    // (3) perform viterbi step on all active models

    overallActiveHypotheses = 0;
    std::list<wordHypothesis>::iterator hypothesisIterator = activeHypotheses.begin();
    while (!(hypothesisIterator == activeHypotheses.end())) {
      dvector &resultVector = (*hypothesisIterator).resultVector;

      // process observation, i.e. perform viterbi step
      if ((*hypothesisIterator).wordLength == 0)
        theViterbiPathSearch.initialStep(theModels[(*hypothesisIterator).modelID],
                                         observation, resultVector);
      else
        theViterbiPathSearch.subsequentStep(theModels[(*hypothesisIterator).modelID],
                                            observation, resultVector);

      // prune after expansion, adjust minScore
      float lastMinScore = minScore;
      int active = pruneModelHypothesis(resultVector, minScore, maxScore);

      // no active states in this model?
      if (active == 0) {
        hypothesisIterator = activeHypotheses.erase(hypothesisIterator);
        continue;
      }

      overallActiveHypotheses += active;

      // increase word length
      ++((*hypothesisIterator).wordLength);

      // best active word hypothesis?
      if (lastMinScore != minScore)
        bestHypothesis = *hypothesisIterator;

      ++hypothesisIterator;
    }




    //
    // (4) pruning
    //

    // perform histogram pruning, if necessary (i.e. adjust pruningThreshold if too many hypotheses active)

    int maxActiveHypotheses = getParameters().maxActiveHypotheses;
    if (maxActiveHypotheses > 0 && maxScore != minScore) {      // avoid division by zero (extremely unlikely)

      if (overallActiveHypotheses > maxActiveHypotheses) {

        // perform bucket sort
        int bucketCount = getParameters().numberOfBuckets;
        ivector buckets(bucketCount + 1, 0);
        int destinationBucket;

        // iterate through all active hypotheses
        hypothesisIterator = activeHypotheses.begin();
        while (!(hypothesisIterator == activeHypotheses.end())) {

          dvector &currentScores = (*hypothesisIterator).resultVector;

          float stateScore;
          // iterate through all states
          for (int s=0; s<currentScores.size(); ++s) {

            stateScore = static_cast<float>(currentScores[s]);

            // skip inactive states
            if (stateScore > maxScore)
              continue;

            destinationBucket = int((bucketCount * (stateScore - minScore)) / (maxScore - minScore));

            // increase bucket counter
            ++buckets[destinationBucket];
          }

          ++hypothesisIterator;
        }

        // determine new pruning threshold
        int hypCount = 0;

        // increase threshold, until maxActiveHypotheses is reached
        for (int b = 0; b < buckets.size(); ++b) {
          pruningThreshold = b * (maxScore - minScore) / float(bucketCount);

          hypCount += buckets[b];
          if (hypCount > maxActiveHypotheses)
            break;
        }

      }
    }



    // prune all hypotheses again (with the new pruneThreshold), find best word end hypothesis
    overallActiveHypotheses = 0;
    hypothesisIterator = activeHypotheses.begin();
    while (!(hypothesisIterator == activeHypotheses.end())) {
      dvector &resultVector = (*hypothesisIterator).resultVector;

      // throw away bad hypotheses, delete empty hypothesis vectors
      int active = pruneModelHypothesis(resultVector, minScore, maxScore);
      if (!active) {
        hypothesisIterator = activeHypotheses.erase(hypothesisIterator);
        continue;
      }

      overallActiveHypotheses += active;

      // best word end?
      float thisEndScore = static_cast<float>(resultVector[resultVector.lastIdx()]);
      if (thisEndScore <= bestWordEnd.score) {

        // if equal scores, take longer word (to avoid repetitions of silence- or trash-models)
        if (thisEndScore < bestWordEnd.score || (*hypothesisIterator).wordLength > bestWordEnd.wordLength) {
          bestWordEnd.modelID = (*hypothesisIterator).modelID;
          bestWordEnd.score = thisEndScore;
          bestWordEnd.wordLength = (*hypothesisIterator).wordLength;
        }
      }


      ++hypothesisIterator;
    }


    // add traceback field entry etc.
    traceBackField.push_back(bestWordEnd);


    if (getParameters().automaticTraceBack > 0) {
      ++timestepCounter;
      if (timestepCounter >= getParameters().automaticTraceBack) {

        // perform partial traceback and save the resulting vectors
        ivector wordLength;
        dvector wordEndScores;

        partialWordHyp = partialTraceBack(wordLength, wordEndScores);
        partialWordLengths = wordLength;
        partialWordEndScores = wordEndScores;

        timestepCounter = 0;
      }
    }

    return true;
  }







  // perform partial traceback
  ivector hmmOnlineClassifier::partialTraceBack(ivector &wordLength, dvector &wordEndScores) {
    wordLength.clear();
    wordEndScores.clear();

    if (!activeHypotheses.size())
      return ivector();


    //
    // find fixed begining of word sequence
    //

    // find latest collective starting point for all active hypotheses
    int foremostWordStart =  findCollectiveStartingPoint();

    // no fixed beginning available
    if (foremostWordStart < 0)
      return ivector();


    // trace back from given starting point
    ivector returnVector;
    traceBack(foremostWordStart, returnVector, wordLength, wordEndScores);


    //
    // delete the first elements of the trace back fields
    //

    float lastScore = traceBackField[foremostWordStart].score;
    int redundantElements = foremostWordStart + 1;

    for (int i = redundantElements;
         i < static_cast<int>(traceBackField.size());
         ++i) {
      traceBackField[i - redundantElements] = traceBackField[i];
    }

    traceBackField.resize(traceBackField.size() - redundantElements);


    //
    // reduce all scores (not necessary, but could help avoiding too large scores)
    //

    reduceScores(lastScore);


    //
    // consider internal partial-traceback information (i.e. if automaticTraceback is enabled)
    //

    appendTracebackVectors(returnVector, wordLength, wordEndScores);


    return returnVector;
  }







  // perform partial traceback
  ivector hmmOnlineClassifier::partialTraceBack() {
    ivector wordLength;
    dvector wordEndScores;

    return partialTraceBack(wordLength, wordEndScores);
  }






  // find last collective starting point for all active hypotheses
  int hmmOnlineClassifier::findCollectiveStartingPoint() const {
    std::vector<bool> wordStart(traceBackField.size() + 1, false);

    int foremostWordStart=traceBackField.size();

    // find starting points for active hypotheses, remember foremost starting point
    std::list<wordHypothesis>::const_iterator hypothesisIterator = activeHypotheses.begin();
    while (!(hypothesisIterator == activeHypotheses.end())) {
      int wordStartIndex = traceBackField.size() - (*hypothesisIterator).wordLength - 1;
      // mark starting points for active hypotheses
      wordStart[wordStartIndex + 1] = true;

      if (wordStartIndex < foremostWordStart)
        foremostWordStart = wordStartIndex;

      ++hypothesisIterator;
    }

    // jump back until the one collective starting point for all hypotheses is found
    for (int i = traceBackField.size() - 1; (i > foremostWordStart) && (foremostWordStart >= 0); --i) {
      if (wordStart[i + 1]) {
        int newStart = i - traceBackField[i].wordLength;

        wordStart[newStart + 1] = true;
        if (newStart < foremostWordStart)
          foremostWordStart = newStart;
      }
    }

    return foremostWordStart;
  }



  // full traceback
  ivector hmmOnlineClassifier::fullTraceBack(ivector &wordLength, dvector &wordEndScores) const {
    ivector returnVector;
    wordLength.clear();
    wordEndScores.clear();

    traceBack(traceBackField.size() - 1, returnVector, wordLength, wordEndScores);

    // consider internal partial-traceback information
    appendTracebackVectors(returnVector, wordLength, wordEndScores);

    return returnVector;
  }






  // full traceback
  ivector hmmOnlineClassifier::fullTraceBack() const {
    ivector wordLength;
    dvector wordEndScores;

    return fullTraceBack(wordLength, wordEndScores);
  }






  // best active hypothesis traceback (last hypothesis is not necessarily in a word-end state)
  ivector hmmOnlineClassifier::bestHypothesisTraceBack(int &innerState, ivector &wordLength, dvector &wordEndScores) const {

    // find best hypothesis
    wordEndHypothesis lastHyp;
    lastHyp.modelID = bestHypothesis.modelID;
    lastHyp.wordLength = bestHypothesis.wordLength;

    // find the best hypothesis (in the hypothesis vector)
    for (int d = 0; d < bestHypothesis.resultVector.size(); ++d)
      if (lastHyp.score >= bestHypothesis.resultVector[d]) {
        lastHyp.score = static_cast<float>(bestHypothesis.resultVector[d]);
        innerState = d;
      }


    // perform trace back steps from the last word end
    int traceBackIndex = traceBackField.size() - lastHyp.wordLength - 1;
    ivector returnVector;
    wordLength.clear();
    wordEndScores.clear();
    traceBack(traceBackIndex, returnVector, wordLength, wordEndScores);

    // append information about active hypothesis
    returnVector.resize(returnVector.size() + 1, lastHyp.modelID);
    wordLength.resize(wordLength.size() + 1, lastHyp.wordLength);
    wordEndScores.resize(wordEndScores.size() + 1, lastHyp.score - traceBackField[traceBackIndex].score);


    // consider internal partial-traceback information
    appendTracebackVectors(returnVector, wordLength, wordEndScores);

    return returnVector;
  }





  // best active hypothesis traceback
  ivector hmmOnlineClassifier::bestHypothesisTraceBack() const {
    int innerState;
    ivector wordLength;
    dvector wordEndScores;

    return bestHypothesisTraceBack(innerState, wordLength, wordEndScores);
  }







  // trace back from given position.
  // append vectors!
  void hmmOnlineClassifier::traceBack(int traceBackIndex, ivector &wordIDs, ivector &wordLength, dvector &wordEndScores) const {
    std::list<wordEndHypothesis> wordList;

    // assume word end, trace back
    int i;
    for (i = traceBackIndex; i >= 0; i -= wordList.front().wordLength)
      wordList.push_front(traceBackField[i]);


    int wordCount = wordList.size();

    // prepare return values
    int wordIDsIndex = wordIDs.size();
    int wordLengthIndex = wordLength.size();
    int wordEndScoresIndex = wordEndScores.size();

    wordIDs.resize(wordIDsIndex + wordCount);
    wordLength.resize(wordLengthIndex + wordCount);
    wordEndScores.resize(wordEndScoresIndex + wordCount);

    // copy sequence information to vectors
    std::list<wordEndHypothesis>::const_iterator wordListIterator = wordList.begin();
    for (i = 0; i < wordCount; ++i, ++wordListIterator) {
      wordIDs[wordIDsIndex + i] = (*wordListIterator).modelID;
      wordLength[wordLengthIndex + i] = (*wordListIterator).wordLength;
      wordEndScores[wordEndScoresIndex + i] = (*wordListIterator).score;
    }

    // relative word end scores
    for (i = wordEndScores.lastIdx(); i > wordEndScoresIndex; --i)
      wordEndScores[i] -= wordEndScores[i-1];
  }




  // reduce scores of all hypotheses by given value
  void hmmOnlineClassifier::reduceScores(float value) {

    // reduce scores within trace back field
    for (int i = 0; i < static_cast<int>(traceBackField.size()); ++i) {
      if (traceBackField[i].score < hiddenMarkovModel::INFINITE_NUMBER) {
        traceBackField[i].score -= value;
      }
    }

    // reduce scores of the best hypothesis' copy
    int d;
    for (d=0; d<bestHypothesis.resultVector.size(); ++d)
      if (bestHypothesis.resultVector[d] < hiddenMarkovModel::INFINITE_NUMBER)
        bestHypothesis.resultVector[d] -= value;

    // reduce scores of all active hypotheses
    std::list<wordHypothesis>::iterator hypothesisIterator = activeHypotheses.begin();
    for (hypothesisIterator = activeHypotheses.begin();
         !(hypothesisIterator == activeHypotheses.end());
         ++hypothesisIterator) {
      for (d=0; d<(*hypothesisIterator).resultVector.size(); ++d)
        if ((*hypothesisIterator).resultVector[d] < hiddenMarkovModel::INFINITE_NUMBER)
          (*hypothesisIterator).resultVector[d] -= value;
    }
  }




  // append internally saved vectors to the front of the given vectors (wordIDs = partialWordHyp + wordIDs etc.)
  void hmmOnlineClassifier::appendTracebackVectors(ivector &wordIDs, ivector &wordLength, dvector &wordEndScores) const {

    // concatenate word-id-hypotheses
    wordIDs.resize(wordIDs.size() + partialWordHyp.size());
    int i;
    for (i = wordIDs.lastIdx(); i >= partialWordHyp.size(); --i)
      wordIDs[i] = wordIDs[i - partialWordHyp.size()];

    for (i = 0; i < partialWordHyp.size(); ++i)
      wordIDs[i] = partialWordHyp[i];

    // concatenate word-length-hypotheses
    wordLength.resize(wordLength.size() + partialWordLengths.size());
    for (i = wordLength.lastIdx(); i >= partialWordLengths.size(); --i)
      wordLength[i] = wordLength[i - partialWordLengths.size()];

    for (i = 0; i < partialWordLengths.size(); ++i)
      wordLength[i] = partialWordLengths[i];

    // concatenate word-score-hypotheses
    wordEndScores.resize(wordEndScores.size() + partialWordEndScores.size());
    for (i = wordEndScores.lastIdx(); i >= partialWordEndScores.size(); --i)
      wordEndScores[i] = wordEndScores[i - partialWordEndScores.size()];

    for (i = 0; i < partialWordEndScores.size(); ++i)
      wordEndScores[i] = partialWordEndScores[i];


  }





  // calculate levenshtein-distance
  int hmmOnlineClassifier::wordErrorCount(const ivector &wordSequence1, const ivector &wordSequence2) const {
    array<int> currentDistance(-1, wordSequence1.lastIdx());

    int s;
    for (s = -1; s <= currentDistance.lastIdx(); ++s)
      currentDistance[s] = s + 1;

    for (int t=0; t<wordSequence2.size(); ++t) {
      array<int> lastDistance = currentDistance;
      for (s=0; s<wordSequence1.size(); ++s) {
        currentDistance[s] = min(lastDistance[s] + 1,                                           // insertion
                                 currentDistance[s-1] + 1,                                      // deletion
                                 lastDistance[s-1] + (wordSequence1[s] != wordSequence2[t]));   // skip or substitution
      }
    }

    return currentDistance[currentDistance.lastIdx()];
  }







  // write function for simplified use
  bool write(ioHandler &handler, const hmmOnlineClassifier::parameters &p, const bool complete) {
    return p.write(handler, complete);
  }

  // read function for simplified use
  bool read(ioHandler &handler, hmmOnlineClassifier::parameters &p, const bool complete) {
    return p.read(handler, complete);
  }


  // write function for simplified use
  bool write(ioHandler &handler, const hmmOnlineClassifier &p, const bool complete) {
    return p.write(handler, complete);
  }

  // read function for simplified use
  bool read(ioHandler &handler, hmmOnlineClassifier &p, const bool complete) {
    return p.read(handler, complete);
  }

}
