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

/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiHmmViterbiPathSearch.cpp
 * authors ....: Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 12.9.2001
 * revisions ..: $Id: ltiHmmViterbiPathSearch.cpp,v 1.4 2006/02/07 20:19:01 ltilib Exp $
 */

//TODO: include files
#include "ltiObject.h"
#include "ltiHmmViterbiPathSearch.h"

namespace lti {


  // --------------------------------------------------
  // hmmViterbiPathSearch
  // --------------------------------------------------

  // default constructor
  hmmViterbiPathSearch::hmmViterbiPathSearch() {
    registerStateSelection = false;
    registerDensitySelection = false;
    registerEmissionScore = false;
    reset();
  }

  // copy constructor
  hmmViterbiPathSearch::hmmViterbiPathSearch(const hmmViterbiPathSearch& other) {
    registerStateSelection = false;
    registerDensitySelection = false;
    registerEmissionScore = false;
    copy(other);
  }

  // destructor
  hmmViterbiPathSearch::~hmmViterbiPathSearch() {
  }

  // returns the name of this type
  const char* hmmViterbiPathSearch::getTypeName() const {
    return "hmmViterbiPathSearch";
  }

  // copy member
  hmmViterbiPathSearch& hmmViterbiPathSearch::copy(const hmmViterbiPathSearch& other) {
    return (*this);
  }




  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------


  // run viterbi alorithm, return score matrix
  double hmmViterbiPathSearch::apply(const hiddenMarkovModel &model, const observationSequence &srcData) {

    // clear logged data
    reset();

    stateCount = model.states.size();
    int frameCount = srcData.size();

    // empty sequence?
    if (!frameCount)
      return hiddenMarkovModel::INFINITE_NUMBER;

    dvector resultScores(stateCount, 0.0);

    if (registerStateSelection)
      stateMatrix.resize(frameCount - 1, stateCount, -1, false, true);

    if (registerDensitySelection)
      densityMatrix.resize(frameCount, stateCount, 0, false, true);

    if (registerEmissionScore)
      emissionScoreMatrix.resize(frameCount, stateCount, 0.0, false, true);

    ivector emptyIntVector(0);
    dvector emptyDoubleVector(0);

    double currentScore = hiddenMarkovModel::INFINITE_NUMBER;

    for (int frame = 0; frame < frameCount; ++frame) {

      ivector &stateVector = (registerStateSelection && frame>0 ? stateMatrix[frame-1] : emptyIntVector);
      ivector &densityVector = (registerDensitySelection ? densityMatrix[frame] : emptyIntVector);
      dvector &scoreVector = (registerEmissionScore ? emissionScoreMatrix[frame] : emptyDoubleVector);

      if (frame == 0)
        currentScore = initialStep(model, srcData[frame], resultScores, densityVector, scoreVector);
      else
        currentScore = subsequentStep(model, srcData[frame], resultScores, stateVector, densityVector, scoreVector);
    }

		// limit score
		if (currentScore > hiddenMarkovModel::INFINITE_NUMBER)
			currentScore = hiddenMarkovModel::INFINITE_NUMBER;

    return currentScore;
  }



  // calculate score for sequence and given path.
  double hmmViterbiPathSearch::evaluatePath(const hiddenMarkovModel &model, const sequence<dvector> &srcData,
                                       const ivector &path) {

    ivector densPath(0);
    evaluatePath(model, srcData, path, densPath);

    // clear logged data
    reset();

    int frameCount = srcData.size();

    if (registerDensitySelection)
      densityPath.resize(frameCount);

    if (registerEmissionScore)
      emissionScorePath.resize(frameCount);


    double score = hiddenMarkovModel::INFINITE_NUMBER;

    // calculate score
    if (!path.empty()) {

      for (int element = 0; element < path.size(); element++) {

        // out of bounds?
        if (path[element] < 0 || path[element] >= static_cast<int>(model.featureDimension))
          throw exception("invalid state number in given path");

        // first step: only initial score
        if (element == 0)
          score = model.initialScore[path[element]];
        // another step: previous score plus transition score
        else
          score += model.transitionScore[path[element-1]][path[element]];

        int dens;
        // score must be calculated, when density or emission score is to be logged
        if (registerDensitySelection || registerEmissionScore) {
          double densScore = getStateScoreAndDensity(model, srcData[element], model.states[path[element]], dens);

          if (registerDensitySelection)
            densityPath[element] = dens;

          if (registerEmissionScore)
            emissionScorePath[element] = densScore;

          if (score<hiddenMarkovModel::INFINITE_NUMBER)
            score += densScore;
        }
        else {
          // skip calculation if already infinite score
          if (score<hiddenMarkovModel::INFINITE_NUMBER)
            score += getStateScoreAndDensity(model, srcData[element], model.states[path[element]], dens);
        }

      }
    }

    // limit score to "infinite"
    if (score>hiddenMarkovModel::INFINITE_NUMBER)
      score = hiddenMarkovModel::INFINITE_NUMBER;

    return score;
  }


  // calculate score for sequence and given path and density selection.
  double hmmViterbiPathSearch::evaluatePath(const hiddenMarkovModel &model, const sequence<dvector> &srcData,
                                       const ivector &path, const ivector &densPath) {

    // clear logged data
    reset();

    int frameCount = srcData.size();

    if (registerEmissionScore)
      emissionScorePath.resize(frameCount);


    double score = hiddenMarkovModel::INFINITE_NUMBER;

    // calculate score
    if (!path.empty()) {

      for (int element = 0; element < path.size(); element++) {

        // out of bounds?
        if (path[element] < 0 || path[element] >= static_cast<int>(model.featureDimension))
          throw exception("invalid state number in given path");

        // first step: only initial score
        if (element == 0)
          score = model.initialScore[path[element]];
        // another step: previous score plus transition score
        else
          score += model.transitionScore[path[element-1]][path[element]];

        // score must be calculated, when emission score is to be logged
        if (registerEmissionScore) {
          emissionScorePath[element] = getDensityScore(model, srcData[element], model.states[path[element]][densPath[element]]);


          if (score<hiddenMarkovModel::INFINITE_NUMBER)
            score += emissionScorePath[element];
        }
        else {
          // skip calculation if already infinite score
          if (score<hiddenMarkovModel::INFINITE_NUMBER)
            score += getDensityScore(model, srcData[element], model.states[path[element]][densPath[element]]);
        }

      }
    }

    // limit score to "infinite"
    if (score>hiddenMarkovModel::INFINITE_NUMBER)
      score = hiddenMarkovModel::INFINITE_NUMBER;

    return score;
  }




  // find out lowest possible score of the given model for a sequence of the specified length;
  double hmmViterbiPathSearch::minimumPathScore(const hiddenMarkovModel &model, int pathLength) const {

    // empty model
    if (!model.states.size())
      return hiddenMarkovModel::INFINITE_NUMBER;

    // find lowest possible score of all states
    dvector lowestStateScores(model.states.size(), hiddenMarkovModel::INFINITE_NUMBER);

    for (int s = 0; s < static_cast<int>(model.states.size()); ++s) {

      // find lowest score of all densities in this state
      for (int d = 0; d < static_cast<int>(model.states[s].size()); ++d) {
        const dvector &currentFrame = model.states[s][d].mean;

        int dens;
        float score = 
          static_cast<float>(getStateScoreAndDensity(model, 
                             currentFrame, model.states[s], dens));
        if (score < lowestStateScores[s])
          lowestStateScores[s] = score;
      }
    }




    // perform viterbi search with lowest scores for each state
    dvector resultVector(model.states.size(), 0.0f);
    ivector predecessors;

    for (int t = 0; t < pathLength; ++t) {
      if (t == 0)
        initialTransition(model, resultVector);
      else
        subsequentTransition(model, resultVector, predecessors);
      resultVector += lowestStateScores;
    }

    // return last state's score
    return resultVector[resultVector.lastIdx()];
  }



  // find out lowest possible score of the given model for a given state sequence and (optional) a given density sequence
  double hmmViterbiPathSearch::minimumPathScore(const hiddenMarkovModel &model, const ivector &stateSelection, const ivector &densitySelection) const {

    // empty model
    if (!model.states.size())
      return hiddenMarkovModel::INFINITE_NUMBER;

    if (densitySelection.size() > 0)
      if (densitySelection.size() != stateSelection.size())
        throw exception("different size of state and density selection vector");

    double score = 0.0f;
    for (int t=0; t < stateSelection.size(); ++t) {

      // add transition score
      if (t==0)
        score += model.initialScore[stateSelection[t]];
      else
        score += model.transitionScore[stateSelection[t-1]][stateSelection[t]];

      const hiddenMarkovModel::mixtureDensity& state = model.states[stateSelection[t]];


      // calculate emission score
      double minScore = hiddenMarkovModel::INFINITE_NUMBER;

      // density selection path given?
      if (densitySelection.size()) {
        const hiddenMarkovModel::singleDensity& dens = state[densitySelection[t]];

        // calculate score for mean frame (=lowest score)
        minScore = dens.weightScore + getDensityScore(model, dens.mean, dens);
      }
      else {

        // find best density center
        for (int d = 0; d < static_cast<int>(state.size()); ++d) {
          const hiddenMarkovModel::singleDensity& dens = state[d];

          // calculate score for mean frame (=lowest score)
          double densScore = dens.weightScore + getDensityScore(model, dens.mean, dens);
          if (densScore < minScore)
            minScore = densScore;
        }
      }

      score += minScore;
    }

    return score;
  }






  // -------------------------------------------------------------------
  // The high level score functions!
  // -------------------------------------------------------------------



  //
  // add initial transition vector to result
  // adding is done to allow for later implementation of observation chain recognition
  //
  void hmmViterbiPathSearch::initialTransition(const hiddenMarkovModel &model, dvector &result) const {
    result += model.initialScore;
  }

  //
  // calculate best transitions
  //
  void hmmViterbiPathSearch::subsequentTransition(const hiddenMarkovModel &model, dvector &result, ivector &predecessors) const {

    // save last scores
    dvector lastScores = result;

    int stateCount = model.states.size();

    // valid predecessor (destination) vector? iff so, save best predecessing states
    bool writePredecessor = (predecessors.size() == stateCount);

    // process all states
    for (int thisState = 0; thisState < stateCount; ++thisState) {

      // find predecessing state that minimizes total score
      double minimalScore = hiddenMarkovModel::INFINITE_NUMBER;
      int minimalState = 0;
      for (int lastState = 0; lastState < stateCount; ++lastState) {

        // stepscore = last score plus transition score
        double stepScore = lastScores[lastState];
        stepScore += model.transitionScore[lastState][thisState];

        // minimal score of all predecessor states
        if ((stepScore <= minimalScore)) {

          minimalState = lastState;
          minimalScore = stepScore;
        }
      }

      // save best predecessing (!) state
      if (writePredecessor)
        predecessors[thisState] = minimalState;

//      if (registerStateSelection)
//        stateMatrix[frame - 1][thisState] = minimalState;

      result[thisState] = minimalScore;
    }
  }

  //
  // calculate emission score for given frame and add it to the result vector
  //
  void hmmViterbiPathSearch::addEmissionScore(const hiddenMarkovModel &model, const dvector &frame,
                                              dvector &result, ivector &densitySelection,
                                              dvector &scoreSelection) const {
      // check size of feature vector
      // (lti::vector::size returns signed int!)
      if (frame.size() != static_cast<int>(model.featureDimension))
        throw exception("wrong size of feature vector");

      int dens;
      double densScore;

      int stateCount = model.states.size();

      bool writeDensitySelection = (densitySelection.size() == stateCount);
      bool writeEmissionScore = (scoreSelection.size() == stateCount);

      // process all states
      for (int thisState = 0; thisState < stateCount; ++thisState) {

        // this distinction is done to omit emission score calculation, if
        // score is already higher than INFINITE_NUMBER
        if (writeDensitySelection || writeEmissionScore) {
          densScore = getStateScoreAndDensity(model, frame, model.states[thisState], dens);

          if (writeEmissionScore)
            scoreSelection[thisState] = densScore;

          if (writeDensitySelection)
            densitySelection[thisState] = dens;

          if (result[thisState] < hiddenMarkovModel::INFINITE_NUMBER)
            result[thisState] += densScore;

        }
        // avoid emission score calculation if possible:
        else if (result[thisState] < hiddenMarkovModel::INFINITE_NUMBER)
          result[thisState] += getStateScoreAndDensity(model, frame, model.states[thisState], dens);
      }
  }

  //
  // perform initial transition and add emission score
  // return last entry of result vector
  //
  double hmmViterbiPathSearch::initialStep(const hiddenMarkovModel &model, const dvector &frame, dvector &result,
                                          ivector &densitySelection, dvector &scoreSelection) const {
    initialTransition(model, result);
    addEmissionScore(model, frame, result, densitySelection, scoreSelection);

    return result[result.lastIdx()];
  }

  // perform initial transition and add emission score
  double hmmViterbiPathSearch::initialStep(const hiddenMarkovModel &model, const dvector &frame, dvector &result) const {
    ivector densitySelection;
    dvector scoreSelection;

    return initialStep(model, frame, result, densitySelection, scoreSelection);
  }

  //
  // perform subsequent transition and add emission score
  // return last entry of result vector
  //
  double hmmViterbiPathSearch::subsequentStep(const hiddenMarkovModel &model, const dvector &frame, dvector &result,
                                             ivector &stateSelection, ivector &densitySelection, dvector &scoreSelection) const {
    subsequentTransition(model, result, stateSelection);
    addEmissionScore(model, frame, result, densitySelection, scoreSelection);

    return result[result.lastIdx()];
  }

  // perform subsequent transition and add emission score
  double hmmViterbiPathSearch::subsequentStep(const hiddenMarkovModel &model, const dvector &frame, dvector &result) const {
    ivector densitySelection;
    ivector stateSelection;
    dvector scoreSelection;

    return subsequentStep(model, frame, result, stateSelection, densitySelection, scoreSelection);
  }


  // -------------------------------------------------------------------
  // The log methods!
  // -------------------------------------------------------------------


  // turns logging of state selection on and off.
  void hmmViterbiPathSearch::logStateSelection(bool enable) {
    registerStateSelection = enable;
  }

  // returns matrix of the best states leading to the minimal score.
  const matrix<int>& hmmViterbiPathSearch::getBackpointerMatrix() const {
    return stateMatrix;
  }

  // returns chosen path of the last run leading to the minimal score
  const vector<int>& hmmViterbiPathSearch::getStatePath() {
    // create path, if it doesn't exist yet
    if (statePath.empty() /*&& !stateMatrix.empty()*/) {
      int frame = stateMatrix.rows();
      statePath.resize(frame + 1);

      // path always ends with the last state
      statePath[frame] = stateCount - 1;

      while (--frame >= 0)
        statePath[frame] = stateMatrix[frame][statePath[frame+1]];
    }

    return statePath;
  }


  // turns logging of density selection matrix on and off.
  void hmmViterbiPathSearch::logDensitySelection(bool enable) {
    registerDensitySelection = enable;
  }

  // returns matrix of the densities used in the last pass.
  const matrix<int>& hmmViterbiPathSearch::getDensitySelectionMatrix() const {
    return densityMatrix;
  }

  // returns densities selected for the chosen path.
  const vector<int>& hmmViterbiPathSearch::getDensitySelectionPath() {
    if (densityPath.empty() && !densityMatrix.empty()) {
      // create state path if it doesn' exist yet
      getStatePath();

      // create density path
      densityPath.resize(statePath.size());
      for (int fr=0; fr < statePath.size(); ++fr)
        densityPath[fr] = densityMatrix[fr][statePath[fr]];
    }

    return densityPath;
  }


  // turns logging of emission score matrix on and off.
  void hmmViterbiPathSearch::logEmissionScore(bool enable) {
    registerEmissionScore = enable;
  }

  // returns matrix of the emission scores generated in the last pass.
  const matrix<double>& hmmViterbiPathSearch::getEmissionScoreMatrix() const {
    return emissionScoreMatrix;
  }

  // returns emission scores generated for the chosen path.
  const vector<double>& hmmViterbiPathSearch::getEmissionScorePath() {
    if (emissionScorePath.empty() && !emissionScoreMatrix.empty()) {
      // create state path if it doesn' exist yet
      getStatePath();

      // create emission score path
      emissionScorePath.resize(statePath.size());
      for (int fr=0; fr < statePath.size(); ++fr)
        emissionScorePath[fr] = emissionScoreMatrix[fr][statePath[fr]];
    }

    return (emissionScorePath);
  }


  //
  // private methods
  //

  // reset all logged data
  void hmmViterbiPathSearch::reset() {
    stateMatrix.clear();
    statePath.clear();
    densityMatrix.clear();
    densityPath.clear();
    emissionScoreMatrix.clear();
    emissionScorePath.clear();
  }


  // -------------------------------------------------------------------
  // The score methods!
  // -------------------------------------------------------------------

  /**
   * calculate score for given frame and density
   */
  double hmmViterbiPathSearch::getDensityScore(const hiddenMarkovModel &model, const dvector &frame, const hiddenMarkovModel::singleDensity &density) const {
    // pick score function
    switch (model.scoreFunction) {
    case hiddenMarkovModel::gaussScore:
      return gaussScore(frame, density.mean, density.scalingFactor, model.featureWeights);
      break;

    case hiddenMarkovModel::laplaceScore:
      return laplaceScore(frame, density.mean, density.scalingFactor, model.featureWeights);
      break;

    default:
      return hiddenMarkovModel::INFINITE_NUMBER;

    }

  }

  /**
   * calculate score for given frame and state, return best density
   */
  double hmmViterbiPathSearch::getStateScoreAndDensity(const hiddenMarkovModel &model, const dvector &frame, const hiddenMarkovModel::mixtureDensity &state, int &dens) const {
    double densityScore;
    double bestScore = hiddenMarkovModel::INFINITE_NUMBER;

    // process all densities within the given state
    dens = 0;
    int densityCount = state.size();

    for (int actDens = 0; actDens < densityCount; ++actDens) {
      densityScore = state[actDens].weightScore + getDensityScore(model, frame, state[actDens]);

      // found better matching density center
      if (densityScore < bestScore) {
        bestScore = densityScore;
        dens = actDens;
      }
    }

    return bestScore * model.emissionScoreWeight;
  }


  // gaussian Score (i.e. -ln(N(mean, deviation)), where N is the normal distribution)
  double hmmViterbiPathSearch::gaussScore(const dvector &frame, const dvector &mean, const dvector &deviation, const dvector &featureWeights) const {

    double help;
    double score = 0.0;

    int dimensions = frame.size();

    if ((dimensions != mean.size()) || (dimensions != deviation.size()))
      throw exception("different dimensions of frame and reference");


    // add scores for all dimensions
    for (int dim=0; dim<dimensions; ++dim) {
      double dev = deviation[dim];

      double featureWeight = (featureWeights.size() > dim ? featureWeights[dim] : 1.0f);

      help = (frame[dim] - mean[dim]) / dev;

      score += featureWeight * 0.5 * (help * help + log(2.0 * lti::Pi * dev * dev));
    }

    return score;
  }



  // laplace Score (i.e. -ln(L(mean, deviation)), where L is the laplace distribution)
  double hmmViterbiPathSearch::laplaceScore(const dvector &frame, const dvector &mean, const dvector &deviation, const dvector &featureWeights) const {

    double score = 0.0;

    int dimensions = frame.size();

    if ((dimensions != mean.size()) || (dimensions != deviation.size()))
      throw exception("different dimensions of frame and reference");

    // add scores for all dimensions
    for (int dim=0; dim<dimensions; ++dim) {
      double dev = deviation[dim];

      double featureWeight = (featureWeights.size() > dim ? featureWeights[dim] : 1.0f);

      score += featureWeight * (abs((frame[dim] - mean[dim]) / dev) + log(2.0 * dev));
    }

    return score;
  }

}
