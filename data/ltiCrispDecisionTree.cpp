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
 * file .......: ltiCrispDecisionTree.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 20.2.2002
 * revisions ..: $Id: ltiCrispDecisionTree.cpp,v 1.8 2006/09/05 09:57:39 ltilib Exp $
 */

// include "ltiObject.h" first, to avoid unnecessary Visual C++ warnings
#include "ltiObject.h"

#include <stack>
#include "ltiMath.h"
#include "ltiClassName.h"

#include "ltiCrispDecisionTree.h"

namespace lti {
  // --------------------------------------------------
  // crispDecisionTree::parameters
  // --------------------------------------------------

  // default constructor
  crispDecisionTree::parameters::parameters()
    : decisionTree::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

  }

  // copy constructor
  crispDecisionTree::parameters::parameters(const parameters& other)
    : decisionTree::parameters()  {
    copy(other);
  }

  // destructor
  crispDecisionTree::parameters::~parameters() {
  }

  // get type name
  const char* crispDecisionTree::parameters::getTypeName() const {
    return "crispDecisionTree::parameters";
  }

  // copy member

  crispDecisionTree::parameters&
    crispDecisionTree::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    decisionTree::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    decisionTree::parameters& (decisionTree::parameters::* p_copy)
      (const decisionTree::parameters&) =
      decisionTree::parameters::copy;
    (this->*p_copy)(other);
# endif



    return *this;
  }

  // alias for copy member
  crispDecisionTree::parameters&
    crispDecisionTree::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  classifier::parameters* crispDecisionTree::parameters::clone() const {
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
  bool crispDecisionTree::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool crispDecisionTree::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && decisionTree::parameters::write(handler,false);
# else
    bool (decisionTree::parameters::* p_writeMS)(ioHandler&,const bool) const =
      decisionTree::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool crispDecisionTree::parameters::write(ioHandler& handler,
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
  bool crispDecisionTree::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool crispDecisionTree::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && decisionTree::parameters::read(handler,false);
# else
    bool (decisionTree::parameters::* p_readMS)(ioHandler&,const bool) =
      decisionTree::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool crispDecisionTree::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif



  // -----------------------------------------------------
  // crispDecisionTree::univariateCrispDecisionFunction
  // -----------------------------------------------------

  // default constructor
  crispDecisionTree::crispDecisionFunction::crispDecisionFunction() 
    : ioObject() {
  }

  // copy constructor
  crispDecisionTree::crispDecisionFunction::crispDecisionFunction
  (const crispDecisionFunction& other) : ioObject() {
    copy(other);
  }

  // destructor
  crispDecisionTree::crispDecisionFunction::~crispDecisionFunction() {
  }

  // returns name of this type
  const char* crispDecisionTree::crispDecisionFunction::getTypeName() const {
    return "crispDecisionTree::crispDecisionFunction";
  }

  // copy the contents of a crispDecisionFunction object
  crispDecisionTree::crispDecisionFunction&
  crispDecisionTree::crispDecisionFunction::copy
  (const crispDecisionFunction& other) {
    return *this;
  }

  // copy the contents of a crispDecisionFunction object
  crispDecisionTree::crispDecisionFunction&
  crispDecisionTree::crispDecisionFunction::operator=
  (const crispDecisionFunction& other) {
    return copy(other);
  }

  bool crispDecisionTree::crispDecisionFunction::write(ioHandler& handler, const bool complete) const {

    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }


    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;

  }

  bool crispDecisionTree::crispDecisionFunction::read(ioHandler& handler, const bool complete) {

    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }


    if (complete) {
      b = b && handler.readEnd();
    }

    return b;

  }

# ifdef _LTI_MSC_6
  bool crispDecisionTree::crispDecisionFunction::readMS(ioHandler& handler,
                                                        const bool complete) {
    return true;
  };

  bool crispDecisionTree::crispDecisionFunction::writeMS(ioHandler& handler,
                                                         const bool complete) const {
    return true;
  };
# endif


  // -----------------------------------------------------
  // crispDecisionTree::univariateCrispDecisionFunction
  // -----------------------------------------------------

  // default constructor
  crispDecisionTree::univariateCrispDecisionFunction::univariateCrispDecisionFunction() :
    crispDecisionTree::crispDecisionFunction(), dimension(0), threshold(0) {
  }

  // conveniance constructor
  crispDecisionTree::univariateCrispDecisionFunction::univariateCrispDecisionFunction(const int& dim, const double& thresh) :
    crispDecisionTree::crispDecisionFunction(),
    dimension(dim), threshold(thresh) {
  }

  // copy constructor
  crispDecisionTree::univariateCrispDecisionFunction::univariateCrispDecisionFunction(
    const crispDecisionTree::univariateCrispDecisionFunction& other)
    : crispDecisionTree::crispDecisionFunction() {
    copy(other);
  }

  // destructor
  crispDecisionTree::univariateCrispDecisionFunction::~univariateCrispDecisionFunction() {
  }

  // returns name of this type
  const char* crispDecisionTree::univariateCrispDecisionFunction::getTypeName() const {
    return "crispDecisionTree::univariateCrispDecisionFunction";
  }

  // copy member
  crispDecisionTree::univariateCrispDecisionFunction&
    crispDecisionTree::univariateCrispDecisionFunction::copy(const univariateCrispDecisionFunction& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    crispDecisionTree::crispDecisionFunction::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    crispDecisionTree::crispDecisionFunction& (crispDecisionTree::crispDecisionFunction::* p_copy)
    (const crispDecisionTree::crispDecisionFunction&) =
      crispDecisionTree::crispDecisionFunction::copy;
    (this->*p_copy)(other);
# endif

    dimension=other.dimension;
    threshold=other.threshold;

    return *this;
  }

  // copy the contents of a univariateCrispDecisionFunction object
  crispDecisionTree::univariateCrispDecisionFunction&
  crispDecisionTree::univariateCrispDecisionFunction::operator=
  (const univariateCrispDecisionFunction& other) {
    return copy(other);
  }

  // returns a pointer to a clone of the univariateCrispDecisionFunction
  crispDecisionTree::crispDecisionFunction*
  crispDecisionTree::univariateCrispDecisionFunction::clone() const {
    return new univariateCrispDecisionFunction(*this);
  }

  // Evaluate the condition implemented in the cdf.
  bool
  crispDecisionTree::univariateCrispDecisionFunction::apply(const dvector& data)
    const {

    return data.at(dimension)<threshold;

  }

  // Sets the condition to be tested.
  void
  crispDecisionTree::univariateCrispDecisionFunction::setCondition
  (const int& dim, const double& thresh) {

    dimension=dim;
    threshold=thresh;
  }

  /*
   * write the univariateCrispDecisionFunction in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool crispDecisionTree::univariateCrispDecisionFunction::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool crispDecisionTree::univariateCrispDecisionFunction::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      b = b && lti::write(handler, "dim", dimension);
      b = b && lti::write(handler, "thresh", threshold);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && crispDecisionTree::crispDecisionFunction::write(handler,false);
# else
    bool (crispDecisionTree::crispDecisionFunction::* p_writeMS)(ioHandler&,const bool) const =
      crispDecisionTree::crispDecisionFunction::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool crispDecisionTree::univariateCrispDecisionFunction::write(ioHandler& handler,
                                         const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /*
   * read the univariateCrispDecisionFunction from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool crispDecisionTree::univariateCrispDecisionFunction::read(ioHandler& handler,
                                        const bool complete)
# else
  bool crispDecisionTree::univariateCrispDecisionFunction::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      b = b && lti::read(handler, "dim", dimension);
      b = b && lti::read(handler, "thresh", threshold);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && crispDecisionTree::crispDecisionFunction::read(handler,false);
# else
    bool (crispDecisionTree::crispDecisionFunction::* p_readMS)(ioHandler&,const bool) =
      crispDecisionTree::crispDecisionFunction::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool crispDecisionTree::univariateCrispDecisionFunction::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // -----------------------------------------------------
  // crispDecisionTree::multivariateCrispDecisionFunction
  // -----------------------------------------------------

  // default constructor
  crispDecisionTree::multivariateCrispDecisionFunction::multivariateCrispDecisionFunction() :
  crispDecisionTree::crispDecisionFunction(), weights(0), threshold(0) {
  }

  // conveniance constructor
  crispDecisionTree::multivariateCrispDecisionFunction::multivariateCrispDecisionFunction(const dvector& w, const double& thresh) :
    crispDecisionTree::crispDecisionFunction(), weights(w), threshold(thresh) {
  }

  // copy constructor
  crispDecisionTree::multivariateCrispDecisionFunction::multivariateCrispDecisionFunction
    (const crispDecisionTree::multivariateCrispDecisionFunction& other)
      : crispDecisionTree::crispDecisionFunction() {
    copy(other);
  }

  // destructor
  crispDecisionTree::multivariateCrispDecisionFunction::~multivariateCrispDecisionFunction() {
  }

  // returns name of this type
  const char* crispDecisionTree::multivariateCrispDecisionFunction::getTypeName() const {
    return "crispDecisionTree::multivariateCrispDecisionFunction";
  }

  // copy member
  crispDecisionTree::multivariateCrispDecisionFunction&
    crispDecisionTree::multivariateCrispDecisionFunction::copy(const multivariateCrispDecisionFunction& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    crispDecisionTree::crispDecisionFunction::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    crispDecisionTree::crispDecisionFunction& (crispDecisionTree::crispDecisionFunction::* p_copy)
      (const crispDecisionTree::crispDecisionFunction&) =
      crispDecisionTree::crispDecisionFunction::copy;
    (this->*p_copy)(other);
# endif

    weights.copy(other.weights);
    threshold=other.threshold;

    return *this;
  }

  // copy the contents of a multivariateCrispDecisionFunction object
  crispDecisionTree::multivariateCrispDecisionFunction&
  crispDecisionTree::multivariateCrispDecisionFunction::operator=
  (const multivariateCrispDecisionFunction& other) {
    return copy(other);
  }

  // returns a pointer to a clone of the multivariateCrispDecisionFunction
  crispDecisionTree::crispDecisionFunction*
  crispDecisionTree::multivariateCrispDecisionFunction::clone() const {
    return new multivariateCrispDecisionFunction(*this);
  }

  // Evaluate the condition implemented in the cdf.
  bool
  crispDecisionTree::multivariateCrispDecisionFunction::apply(const dvector& data)
    const {

    dvector tmp(data);
    return tmp.dot(weights) < threshold;
  }

  // Sets the condition to be tested.
  void
  crispDecisionTree::multivariateCrispDecisionFunction::setCondition
  (const dvector& w, const double& thresh) {

    weights.copy(w);
    threshold=thresh;
  }

  /*
   * write the multivariateCrispDecisionFunction in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool crispDecisionTree::multivariateCrispDecisionFunction::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool crispDecisionTree::multivariateCrispDecisionFunction::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      b = b && lti::write(handler, "weights", weights);
      b = b && lti::write(handler, "thresh", threshold);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && crispDecisionTree::crispDecisionFunction::write(handler,false);
# else
    bool (crispDecisionTree::crispDecisionFunction::* p_writeMS)(ioHandler&,const bool) const =
      crispDecisionTree::crispDecisionFunction::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool crispDecisionTree::multivariateCrispDecisionFunction::write(ioHandler& handler,
                                         const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /*
   * read the multivariateCrispDecisionFunction from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool crispDecisionTree::multivariateCrispDecisionFunction::read(ioHandler& handler,
                                        const bool complete)
# else
  bool crispDecisionTree::multivariateCrispDecisionFunction::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      b = b && lti::read(handler, "weights", weights);
      b = b && lti::read(handler, "thresh", threshold);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && crispDecisionTree::crispDecisionFunction::read(handler,false);
# else
    bool (crispDecisionTree::crispDecisionFunction::* p_readMS)(ioHandler&,const bool) =
      crispDecisionTree::crispDecisionFunction::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool crispDecisionTree::multivariateCrispDecisionFunction::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // crispDecisionTree::crispNode
  // --------------------------------------------------

  const crispDecisionTree::crispDecisionFunction*
  crispDecisionTree::crispNode::cdfArray[]={
    new crispDecisionTree::univariateCrispDecisionFunction(),
    new crispDecisionTree::multivariateCrispDecisionFunction(),
    NULL
  };

  const objectFactory<crispDecisionTree::crispDecisionFunction>
  crispDecisionTree::crispNode::crispDecisionFunctionFactory
  (crispDecisionTree::crispNode::cdfArray);

  // default constructor
  crispDecisionTree::crispNode::crispNode()
    : decisionTree::dtNode(), decisionF(0) {

  }

  // constructor
  crispDecisionTree::crispNode::crispNode(const int& theId)
    : decisionTree::dtNode(theId), decisionF(0) {

  }

  // conveniance constructor: univariate decision function
  crispDecisionTree::crispNode::crispNode(const int& dim,
                                          const double& thresh,
                                          const int& theId)
    : decisionTree::dtNode(theId) {

    decisionF=new univariateCrispDecisionFunction(dim, thresh);
  }

  // conveniance constructor: multivariate decision function
  crispDecisionTree::crispNode::crispNode(const dvector& w,
                                          const double& thresh,
                                          const int& theId)
    : decisionTree::dtNode(theId) {

    decisionF=new multivariateCrispDecisionFunction(w, thresh);
  }

  // copy constructor
  crispDecisionTree::crispNode::crispNode(const crispNode& other)
    : decisionTree::dtNode()  {
    copy(other);
  }

  // destructor
  crispDecisionTree::crispNode::~crispNode() {
  }

  // get type name
  const char* crispDecisionTree::crispNode::getTypeName() const {
    return "crispDecisionTree::crispNode";
  }

  // copy member

  crispDecisionTree::crispNode&
    crispDecisionTree::crispNode::copy(const crispNode& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    decisionTree::dtNode::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    decisionTree::dtNode& (decisionTree::dtNode::* p_copy)
      (const decisionTree::dtNode&) =
      decisionTree::dtNode::copy;
    (this->*p_copy)(other);
# endif

    if (other.decisionF==0) {
      decisionF=0;
    } else {
      decisionF=other.decisionF->clone();
    }

    return *this;
  }

  // alias for copy member
  crispDecisionTree::crispNode&
    crispDecisionTree::crispNode::operator=(const crispNode& other) {
    return copy(other);
  }

  // clone member
  decisionTree::dtNode* crispDecisionTree::crispNode::clone() const {
    return new crispNode(*this);
  }


  // Sets the crispDecisionFunction for this node
  void crispDecisionTree::crispNode::setDecisionFunction
  (const crispDecisionFunction* cdf) {

    decisionF=cdf->clone();
  }

  // Gets the crispDecisionFunction this node uses.
  const crispDecisionTree::crispDecisionFunction*
  crispDecisionTree::crispNode::getDecisionFunction
  (const crispDecisionFunction* cdf) {
    return decisionF;
  }

  // Propagates the given value through the node. If the
  // crispDecisionFunction is true for this value, the left
  // child node is returned, else the right.
  const crispDecisionTree::crispNode*
  crispDecisionTree::crispNode::propagate(const dvector& data) const {

    if (isLeaf()) {
      return 0;
    }

    if (decisionF->apply(data)) {
      return dynamic_cast<crispDecisionTree::crispNode*>(left);
    } else {
      return dynamic_cast<crispDecisionTree::crispNode*>(right);
    }

  }

  /*
   * write the crispNode in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool crispDecisionTree::crispNode::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool crispDecisionTree::crispNode::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (decisionF!=0) {
      std::string str;
      className cn;
      cn.get(decisionF, str);

      b = b && lti::write(handler, "dfType", str);
      b = b && decisionF->write(handler, true);
    } else {
      b = b && lti::write(handler, "dfType", "none");
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && decisionTree::dtNode::write(handler,false);
# else
    bool (decisionTree::dtNode::* p_writeMS)(ioHandler&,const bool) const =
      decisionTree::dtNode::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool crispDecisionTree::crispNode::write(ioHandler& handler,
                                         const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /*
   * read the crispNode from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool crispDecisionTree::crispNode::read(ioHandler& handler,
                                        const bool complete)
# else
  bool crispDecisionTree::crispNode::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      std::string str;
      b = b && lti::read(handler, "dfType", str);
      if (str!="none") {
        decisionF=crispDecisionTree::crispNode::crispDecisionFunctionFactory.newInstance(str);
        b = b && decisionF->read(handler, true);
      } else {
        decisionF=0;
      }

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && decisionTree::dtNode::read(handler,false);
# else
    bool (decisionTree::dtNode::* p_readMS)(ioHandler&,const bool) =
      decisionTree::dtNode::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool crispDecisionTree::crispNode::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // crispDecisionTree
  // --------------------------------------------------


  // default constructor
  crispDecisionTree::crispDecisionTree()
    : decisionTree(), root(0) {

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // copy constructor
  crispDecisionTree::crispDecisionTree(const crispDecisionTree& other)
    : decisionTree()  {
    copy(other);
  }

  // destructor
  crispDecisionTree::~crispDecisionTree() {
  }

  // returns the name of this type
  const char* crispDecisionTree::getTypeName() const {
    return "crispDecisionTree";
  }

  // copy member
  crispDecisionTree&
    crispDecisionTree::copy(const crispDecisionTree& other) {
      decisionTree::copy(other);

      root=new crispDecisionTree::crispNode();
      root->copy(*other.root);

      return (*this);
  }

  // alias for copy member
  crispDecisionTree&
    crispDecisionTree::operator=(const crispDecisionTree& other) {
    return (copy(other));
  }


  // return parameters
  const crispDecisionTree::parameters&
    crispDecisionTree::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // classify
  bool
  crispDecisionTree::classify(const dvector& feature,
                              outputVector& result) const {

    const crispNode* node=root;
    while (!node->isLeaf()) {
      node=node->propagate(feature);
    }

    int winner=node->getId();
    dvector tResult(numberOfLeafs, 0.);
    tResult.at(winner)=1.;

    return outTemplate.apply(tResult, result);
  }

  bool
  crispDecisionTree::write(ioHandler& handler,
                           const bool complete) const {

    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    // write paren class
    b = b && decisionTree::write(handler, false);

    // need a stack to write the tree
    // also have to keep track of how many  children have been written
    // per node with the levelcount
    std::stack<const dtNode*> st;
    st.push(root);
    const dtNode* node;
    int level=0;
    std::vector<int> levelcount(10);

    std::string str;
    className cn;

    while (!st.empty()) {
      node=st.top();
      st.pop();

      // This is the case, when a node has one child only.
      // The not existent child gets pushed on the stack as 0.
      // It is written as a node of type nil and counted as a child.
      if (node==0) {
        b = b && handler.writeBegin();
        b = b && lti::write(handler, "nodeType", "nil");
        b = b && handler.writeEnd();
        levelcount[level]++;
        while (levelcount[level]==2) {
          b = b && handler.writeEnd();
          levelcount[level]=0;
          level--;
          levelcount[level]++;
        }
        continue;
      }

      // If a node is not a leaf push both children on the stack
      // even if one of them doesn't exist, push the 0 pointer
      // see above
      if (!node->isLeaf()) {
        st.push(node->getRightChild());
        st.push(node->getLeftChild());
      }

      // write the actual node: first the type, then the data
      b = b && handler.writeBegin();
      cn.get(node, str);
      b = b && lti::write(handler, "nodeType", str);
      b = b && node->write(handler, true);

      // If a node is a leaf it will be closed and one child of that level
      // is finished. Otherwise, the next node processed will be a child of
      // the current node. Thus, the level is incremented, reserving space
      // if necessary and the counter for the next level is set to 0.
      if (node->isLeaf()) {
        b = b && handler.writeEnd();
        levelcount[level]++;
      } else {
        level++;
        levelcount.reserve(level+1);
        levelcount[level]=0;
      }

      // If two nodes have been written on one level:
      // - reset counter
      // - go back one level
      // - increment counter on that level for one child is done
      // This obviously needs to be done until a node is found where there are
      // children left to be written.
      while (levelcount[level]==2) {
        b = b && handler.writeEnd();
        levelcount[level]=0;
        level--;
        levelcount[level]++;
      }
    }

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;

  }

  // read
  bool
  crispDecisionTree::read(ioHandler& handler, const bool complete) {

    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    // read parent class
    b = b && decisionTree::read(handler, false);

    // need a stack to read a tree
    std::stack<dtNode*> st;
    dtNode* node;
    dtNode* child;
    std::string str;

    // the dummyNode is needed for nodes who have only one child.
    // If this is the case for the left child a dummy node is inserted
    // as left child, since the method recognizes whether a left or right
    // child needs to be read by checking the left child for 0. The dummyNode
    // is deleted later.
    crispNode* dummyNode=new crispNode();


    // read the root node
    b = b && handler.readBegin();
    b = b && lti::read(handler, "nodeType", str);
    node = nodeFactory.newInstance(str);
    node->read(handler, true);
    if (str=="lti::crispDecisionTree::crispNode") {
      root=dynamic_cast<crispNode*>(node);
    } else {
      b=false;
    }
    st.push(root);

    while (!st.empty()) {
      node=st.top();

      // if an opening parenthesis is found the next node is a child
      // of the current node
      if (handler.tryBegin()) {
        b = b && lti::read(handler, "nodeType", str);
        // if the node type is "nil", there is no child on that child
        // for left children a dummyNode needs to be set temporarily.
        // see docu above
        if (str=="nil") {
          if (node->getLeftChild()==0) {
            node->setLeftChild(dummyNode);
            handler.readEnd();
          } else {
            handler.readEnd();
          }
          continue;
        }

        // now read the actual node
        // check on which side it belongs and push it on the stack
        child = nodeFactory.newInstance(str);
        child->read(handler, true);
        if (node->getLeftChild()==0) {
          node->setLeftChild(child);
        } else {
          node->setRightChild(child);
        }
        st.push(child);

        // if a closing parenthesis is read, this node and its children is
        // finished. Pop it from the stack and if a dummyNode was used
        // replace is by zero.
      } else if (handler.tryEnd()) {
        st.pop();
        if (node->getLeftChild()==dummyNode) {
          node->setLeftChild(0);
          std::cerr << "replaced left node with 0\n\n";
        }

        // if no parenthesis is read the file is corrupted.
      } else {
        b = false;
        std::cerr << "error\n\n";
      }
    }

    if (complete) {
      b = b && handler.readEnd();
    }

    delete dummyNode;

    return b;

  }


}
