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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiHiddenMarkovModel.cpp
 * authors ....: Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 14.12.01
 * revisions ..: $Id: ltiHiddenMarkovModel.cpp,v 1.8 2006/09/05 10:39:54 ltilib Exp $
 */

#include "ltiObject.h"
#include "ltiHiddenMarkovModel.h"

namespace lti {

  // --------------------------------------------------
  // hiddenMarkovModel::singleDensity
  // --------------------------------------------------

  // default constructor
  hiddenMarkovModel::singleDensity::singleDensity() : ioObject() {
    weightScore = 0.0;
  }

  // copy constructor
  hiddenMarkovModel::singleDensity::singleDensity(const singleDensity& other) 
    : ioObject() {
    copy(other);
  }

  // destructor
  hiddenMarkovModel::singleDensity::~singleDensity() {
  }

// copy constructor
  hiddenMarkovModel::singleDensity& hiddenMarkovModel::singleDensity::copy(const hiddenMarkovModel::singleDensity& other) {
  // copy data from other hiddenMarkovModel
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

    weightScore = other.weightScore;
    mean = other.mean;
    scalingFactor = other.scalingFactor;

    return *this;
  }

  // assigment operator (alias for copy(other)).
  hiddenMarkovModel::singleDensity& hiddenMarkovModel::singleDensity::operator=(const hiddenMarkovModel::singleDensity& other) {
    return copy(other);
  }

  // get type name
  const char* hiddenMarkovModel::singleDensity::getTypeName() const {
    return "hiddenMarkovModel::singleDensity";
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */

# ifndef _LTI_MSC_6
  bool hiddenMarkovModel::singleDensity::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool hiddenMarkovModel::singleDensity::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler, "weightScore", weightScore);
      lti::write(handler, "mean", mean);
      lti::write(handler, "scalingFactor", scalingFactor);

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
  bool hiddenMarkovModel::singleDensity::write(ioHandler& handler,
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
  bool hiddenMarkovModel::singleDensity::read(ioHandler& handler,
                                        const bool complete)
# else
  bool hiddenMarkovModel::singleDensity::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler, "weightScore", weightScore);
      lti::read(handler, "mean", mean);
      lti::read(handler, "scalingFactor", scalingFactor);
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
  bool hiddenMarkovModel::singleDensity::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // hiddenMarkovModel
  // --------------------------------------------------

  // default constructor
  hiddenMarkovModel::hiddenMarkovModel() : ioObject() {

    name = "";
    states.clear();

    scoreFunction = laplaceScore;

    initialScore.clear();
    transitionScore.clear();

    featureDimension = 0;
    emissionScoreWeight = 1.0;

    featureWeights.clear();
  }

  // copy constructor
  hiddenMarkovModel::hiddenMarkovModel(const hiddenMarkovModel& other) 
    : ioObject() {
    copy(other);
  }

  // destructor
  hiddenMarkovModel::~hiddenMarkovModel() {
  }

  // copy data from other hiddenMarkovModel
  hiddenMarkovModel& hiddenMarkovModel::copy(const hiddenMarkovModel& other) {
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


    name = other.name;
    scoreFunction = other.scoreFunction;
    featureDimension = other.featureDimension;
    states = other.states;
    initialScore = other.initialScore;
    transitionScore = other.transitionScore;
    emissionScoreWeight = other.emissionScoreWeight;
    featureWeights = other.featureWeights;


    return *this;
  }

  // assigment operator (alias for copy(other)).
  hiddenMarkovModel& hiddenMarkovModel::operator=(const hiddenMarkovModel& other) {
    return copy(other);
  }

  // get type name
  const char* hiddenMarkovModel::getTypeName() const {
    return "hiddenMarkovModel";
  }

  int hiddenMarkovModel::getNumberOfStates() const {
    return states.size();
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */

# ifndef _LTI_MSC_6
  bool hiddenMarkovModel::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool hiddenMarkovModel::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler, "name", name);

      switch (scoreFunction) {
      case gaussScore:
        lti::write(handler, "scoreFunction", "gaussScore");
        break;

      case laplaceScore:
        lti::write(handler, "scoreFunction", "laplaceScore");
        break;

      default:
        lti::write(handler, "scoreFunction", "unknown");
      }

      lti::write(handler, "featureDimension", featureDimension);
      lti::write(handler, "states", states);

      lti::write(handler, "initialScore", initialScore);
      lti::write(handler, "transitionScore", transitionScore);

      lti::write(handler, "emissionScoreWeight", emissionScoreWeight);
      lti::write(handler, "featureWeights", featureWeights);

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
  bool hiddenMarkovModel::write(ioHandler& handler,
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
  bool hiddenMarkovModel::read(ioHandler& handler,
                                        const bool complete)
# else
  bool hiddenMarkovModel::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler, "name", name);

      std::string scoreDescription;
      lti::read(handler, "scoreFunction", scoreDescription);
      if (scoreDescription == "gaussScore")
        scoreFunction = gaussScore;
      else if (scoreDescription == "laplaceScore")
        scoreFunction = laplaceScore;
      else
        b = false;

      lti::read(handler, "featureDimension", featureDimension);
      lti::read(handler, "states", states);

      lti::read(handler, "initialScore", initialScore);
      lti::read(handler, "transitionScore", transitionScore);

      lti::read(handler, "emissionScoreWeight", emissionScoreWeight);
      lti::read(handler, "featureWeights", featureWeights);

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
  bool hiddenMarkovModel::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif



  // this number is considered to be infinite (score equivalent to a probability of 0.0)
  const double hiddenMarkovModel::INFINITE_NUMBER = 1.0e+35;



  // read single hidden markov model state
  bool read(ioHandler& handler, hiddenMarkovModel::singleDensity& p, const bool complete) {
    return p.read(handler,complete);
  }

  // write single hidden markov model state
  bool write(ioHandler& handler, const hiddenMarkovModel::singleDensity& p, const bool complete) {
    return p.write(handler,complete);
  }


  // read hidden markov model
  bool read(ioHandler& handler, hiddenMarkovModel& p, const bool complete) {
    return p.read(handler,complete);
  }

  // read hidden markov model
  bool write(ioHandler& handler, const hiddenMarkovModel& p, const bool complete) {
    return p.write(handler,complete);
  }


}
