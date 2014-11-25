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
 * file .......: ltiCombination.cpp
 * authors ....: Pablo Alvarado, Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 5.9.2001
 * revisions ..: $Id: ltiCombination.cpp,v 1.9 2006/09/05 09:57:19 ltilib Exp $
 */

#include "ltiObject.h"
#include "ltiCombination.h"

namespace lti {
  // --------------------------------------------------
  // combination::parameters
  // --------------------------------------------------

  // default constructor
  combination::parameters::parameters()
    : functor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    combinationStrategy = Linear;
    normalize           = true;
    normalizeInputs     = false;
    useWeights          = false;
    weights             = dvector(0);
  }

  // copy constructor
  combination::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  combination::parameters::~parameters() {
  }

  // get type name
  const char* combination::parameters::getTypeName() const {
    return "combination::parameters";
  }

  // copy member

  combination::parameters&
    combination::parameters::copy(const parameters& other) {
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


    combinationStrategy  = other.combinationStrategy;
    normalize            = other.normalize;
    normalizeInputs      = other.normalizeInputs;
    useWeights           = other.useWeights;
    weights              = other.weights;

    return *this;
  }

  // alias for copy member
  combination::parameters&
    combination::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* combination::parameters::clone() const {
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
  bool combination::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool combination::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      switch(combinationStrategy) {
      case Linear:
        lti::write(handler,"combinationStrategy","Linear");
        break;
      case ProbabilisticBordaCount:
        lti::write(handler,"combinationStrategy","ProbabilisticBordaCount");
        break;
      default:
        lti::write(handler,"combinationStrategy","ERROR: mode unknown");
        break;
      }

      lti::write(handler,"normalize",normalize);
      lti::write(handler,"normalizeInputs",normalizeInputs);
      lti::write(handler,"useWeights",useWeights);

      if (useWeights) {
        lti::write(handler,"weights",weights);
      }
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
  bool combination::parameters::write(ioHandler& handler,
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
  bool combination::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool combination::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string str;

      lti::read(handler,"combinationStrategy",str);
      if (str == "Linear") {
        combinationStrategy = Linear;
      } else if (str == "ProbabilisticBordaCount") {
        combinationStrategy = ProbabilisticBordaCount;
      } else {
        combinationStrategy = Linear;  // default value on unknown mode
      }

      lti::read(handler,"normalize",normalize);
      lti::read(handler,"normalizeInputs",normalizeInputs);
      lti::read(handler,"useWeights",useWeights);

      if (useWeights) {
        lti::read(handler,"weights",weights);
      }

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
  bool combination::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // combination
  // --------------------------------------------------

  // default constructor
  combination::combination()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  combination::combination(const combination& other)
    : functor()  {
    copy(other);
  }

  // destructor
  combination::~combination() {
  }

  // returns the name of this type
  const char* combination::getTypeName() const {
    return "combination";
  }

  // copy member
  combination& combination::copy(const combination& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  combination&
    combination::operator=(const combination& other) {
    return (copy(other));
  }


  // clone member
  functor* combination::clone() const {
    return new combination(*this);
  }

  // return parameters
  const combination::parameters&
    combination::getParameters() const {
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


  bool combination::apply(const std::list<classifier::outputVector>& src,
                          classifier::outputVector& dest) const {

    bool b=true;

    // If src is empty there is nothing to combine
    if (src.empty()) {
      dest=classifier::outputVector();
      setStatusString("Empty input-list in combination");
      return false;
    }

    // To use weighted combination call other apply method
    // If useWeights is true but weights is empty or 0 return false but
    // continue with processing as if useWeights was false
    const parameters& param = getParameters();
    if (param.useWeights) {
      if (!param.weights.empty()) {
        return apply(src, param.weights, dest);
      } else {
        dest=classifier::outputVector();
        setStatusString
          ("Combination: Empty weights vector although useWeights is true");
        b=false;
      }
    }


    const std::list<classifier::outputVector>* inputs=0;
    std::list<classifier::outputVector> tlist;
    if (param.normalizeInputs) {
      classifier::outputVector tvec;
      std::list<classifier::outputVector>::const_iterator it=src.begin();
      while (it!=src.end()) {
        tvec=*it++;
        tvec.makeProbDistribution();
        tlist.push_back(tvec);
      }
      inputs=&tlist;
    } else {
      inputs=&src;
    }

    parameters::eCombinationStrategy mode = param.combinationStrategy;

    switch(mode) {

    case parameters::Linear:
      b = sumOfVectors(*inputs, dest) && b;
      break;

    case parameters::ProbabilisticBordaCount:
      b = probBordaCount(*inputs, dest) && b;
      break;

    default:
      b = sumOfVectors(*inputs, dest) && b;
    }

    if (param.normalize) {
      dest.makeProbDistribution();
    }

    return b;
  }

  bool combination::apply(const std::list<classifier::outputVector>& src,
                          const dvector& weights,
                          classifier::outputVector& dest) const {

    bool b=true;

    // If src is empty there is nothing to combine
    if (src.empty()) {
      dest=classifier::outputVector();
      setStatusString("Empty input-list in combination");
      return false;
    }

    const parameters& param = getParameters();
    parameters::eCombinationStrategy mode = param.combinationStrategy;

    switch(mode) {

    case parameters::Linear:
      b = sumOfVectors(src, weights, dest) && b;
      break;

    case parameters::ProbabilisticBordaCount:
      b = probBordaCount(src, weights, dest) && b;
      break;

    default:
      b = sumOfVectors(src, weights, dest) && b;
    }

    if (param.normalize) {
      dest.makeProbDistribution();
    }

    return b;
  }

  // -------------------------------------------------------------------
  // The combination-methods!
  // -------------------------------------------------------------------


  bool combination::sumOfVectors(const std::list<classifier::outputVector> src,
                                 classifier::outputVector& dest) const {

#ifdef _DEBUG
    // Checks whether it is possible to combine the vectors in src.
    // This is quite time consuming and thus only executed when in debug mode
    if (!noMultAndCompatible(src)) {
      return false;
    }
#endif

    std::list<classifier::outputVector>::const_iterator it;

    it=src.begin();
    dest=*it;
    ++it;

    while (it!=src.end()) {
      dest.add(*it);
      ++it;
    }

    return true;

  }

  bool combination::sumOfVectors(const std::list<classifier::outputVector> src,
                                 const dvector& weights,
                                 classifier::outputVector& dest) const {

#ifdef _DEBUG
    // Checks whether it is possible to combine the vectors in src.
    // This is quite time consuming and thus only executed when in debug mode
    if (!noMultAndCompatible(src)) {
      dest=classifier::outputVector();
      return false;
    }
    if (src.size()!=(unsigned int)weights.size()) {
      setStatusString("Combination: Unequal number of vectors and weights");
      dest=classifier::outputVector();
      return false;
    }
#endif

    std::list<classifier::outputVector>::const_iterator it;

    it=src.begin();
    dest=*it;
    ++it;
    dest.mul(weights.at(0));
    int i=1;
    while (it!=src.end()) {
      dest.addScaled(*it, weights.at(i));
      ++it;
      ++i;
    }

    return true;
  }


  bool
  combination::probBordaCount(const std::list<classifier::outputVector> src,
                              classifier::outputVector& dest) const {

#ifdef _DEBUG
    // Checks whether it is possible to combine the vectors in src.
    // This is quite time consuming and thus only executed when in debug mode
    if (!noMultAndCompatible(src)) {
      dest=classifier::outputVector();
      return false;
    }
#endif

    int i;
    std::list<classifier::outputVector>::const_iterator it;
    it=src.begin();

    dest=classifier::outputVector(*it);
    dest.setValues(dvector(it->size()));
    it++;
    while (it!=src.end()) {

      int    curr;
      double currPbc;
      double currP;
      double resP;
      double prevPbc=0.;
      double prevP;
      it->getValueByPosition(0, prevP);
      double d=0.f;
      for (i=1; i<it->size(); i++, d++) {
        it->getPair(i, curr, currP);
        dest.getValueById(curr, resP);
        currPbc=d*(currP-prevP)+prevPbc;
        dest.setValueByPosition(curr, resP + currPbc);
        prevPbc=currPbc;
        prevP=currP;
      }
      it++;
    }
    return true;
  }

  bool
  combination::probBordaCount(const std::list<classifier::outputVector> src,
                              const dvector& weights,
                              classifier::outputVector& dest) const {

#ifdef _DEBUG
    // Checks whether it is possible to combine the vectors in src.
    // This is quite time consuming and thus only executed when in debug mode
    if (!noMultAndCompatible(src)) {
      dest=classifier::outputVector();
      return false;
    }
    if (src.size()!=(unsigned int)weights.size()) {
      setStatusString("Combination: Unequal number of vectors and weights");
      dest=classifier::outputVector();
      return false;
    }
#endif

    int i, count;
    std::list<classifier::outputVector>::const_iterator it;
    it=src.begin();

    dest=classifier::outputVector(*it);
    dest.setValues(dvector(it->size()));

    it++;
    count=0;
    while (it!=src.end()) {
      int    curr;
      double currPbc;
      double currP;
      double resP;
      double prevPbc=0.;
      double prevP;
      it->getValueByPosition(0,prevP);
      double d=0.f;
      for (i=1; i<it->size(); i++, d++) {
        it->getPair(i, curr, currP);
        dest.getValueById(curr, resP);
        currPbc=d*(currP-prevP)+prevPbc;
        dest.setValueByPosition(curr, resP + weights[count++]*currPbc);
        prevPbc=currPbc;
        prevP=currP;
      }
    }
    return true;
  }


  // -------------------------------------------------------------------
  // tools!
  // -------------------------------------------------------------------

  bool combination::noMultAndCompatible
  (const std::list<classifier::outputVector> src) const {

    std::list<classifier::outputVector>::const_iterator it;
    it=src.begin();
    classifier::outputVector first=*it++;
    if (!first.noMultipleIds()) {
      return false;
    }
    for (; it!=src.end(); it++) {
      if (!it->noMultipleIds()) {
        setStatusString("Combination: One vector in the list contained a label more than once.");
        return false;
      }
      if (!it->compatible(first)) {
        setStatusString("Combination: Two vectors of the list are not compatible");
        return false;
      }
    }
    return true;
  }

}
