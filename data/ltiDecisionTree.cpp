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
 * file .......: ltiDecisionTree.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 19.2.2002
 * revisions ..: $Id: ltiDecisionTree.cpp,v 1.7 2006/09/05 09:57:59 ltilib Exp $
 */

#include "ltiObject.h"

#include <queue>
#include <stack>

#include "ltiDecisionTree.h"
#include "ltiCrispDecisionTree.h"

namespace lti {


  // --------------------------------------------------
  // decisionTree::parameters
  // --------------------------------------------------

  // default constructor
  decisionTree::parameters::parameters()
    : supervisedInstanceClassifier::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

  }

  // copy constructor
  decisionTree::parameters::parameters(const parameters& other)
    : supervisedInstanceClassifier::parameters()  {
    copy(other);
  }

  // destructor
  decisionTree::parameters::~parameters() {
  }

  // get type name
  const char* decisionTree::parameters::getTypeName() const {
    return "decisionTree::parameters";
  }

  // copy member

  decisionTree::parameters&
    decisionTree::parameters::copy(const parameters& other) {
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



    return *this;
  }

  // alias for copy member
  decisionTree::parameters&
    decisionTree::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  classifier::parameters* decisionTree::parameters::clone() const {
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
  bool decisionTree::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool decisionTree::parameters::writeMS(ioHandler& handler,
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
  bool decisionTree::parameters::write(ioHandler& handler,
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
  bool decisionTree::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool decisionTree::parameters::readMS(ioHandler& handler,
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
  bool decisionTree::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // decisionTree::dtNode
  // --------------------------------------------------

  // default constructor
  decisionTree::dtNode::dtNode()
    : object() {
    left=0;
    right=0;
    id=-1;
  }

  // Constructor
  decisionTree::dtNode::dtNode(const int& theId)
    : object(), id(theId) {
    left=0;
    right=0;
  }

  // copy constructor
  decisionTree::dtNode::dtNode(const dtNode& other)
    : object()  {
    copy(other);
  }

  // destructor
  decisionTree::dtNode::~dtNode() {
    delete left;
    delete right;
    left=0;
    right=0;
  }

  // get type name
  const char* decisionTree::dtNode::getTypeName() const {
    return "decisionTree::dtNode";
  }

  // copy member

  decisionTree::dtNode&
  decisionTree::dtNode::copy(const dtNode& other) {

    if (other.left==0) {
      left=0;
    } else {
      left=other.left->clone();
    }

    if (other.right==0) {
      right=0;
    } else {
      right=other.right->clone();
    }

    id=other.id;

    return *this;
  }

  // alias for copy member
  decisionTree::dtNode&
    decisionTree::dtNode::operator=(const dtNode& other) {
    return copy(other);
  }

  // clone member
  decisionTree::dtNode* decisionTree::dtNode::clone() const {
    return new dtNode(*this);
  }

  decisionTree::dtNode* decisionTree::dtNode::setLeftChild(dtNode* newLeft) {
    decisionTree::dtNode* old=left;
    left=newLeft;
    return old;
  }

  decisionTree::dtNode* decisionTree::dtNode::setRightChild(dtNode* newRight) {
    decisionTree::dtNode* old=right;
    right=newRight;
    return old;
  }

  const decisionTree::dtNode* decisionTree::dtNode::getLeftChild() const {
    return left;
  }

  const decisionTree::dtNode* decisionTree::dtNode::getRightChild() const {
    return right;
  }


  // is this node a leaf?
  bool decisionTree::dtNode::isLeaf() const {
    return (left==0 && right==0);
  }

  // set id
  void decisionTree::dtNode::setId(const int& theId) {
    id=theId;
  }

  // get id
  int decisionTree::dtNode::getId() const {
    return id;
  }

  /*
   * write the dtNode in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
  bool decisionTree::dtNode::write(ioHandler& handler,
                                   const bool complete) const
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      b = b && lti::write(handler, "id", id);

//        if (left!=0) {
//          b = b && lti::write(handler, "nodeType", left->getTypeName());
//          b = b && left->write(handler, complete);
//        } else {
//          b = b && lti::write(handler, "nodeType", "nil");
//        }


//        if (right!=0) {
//          b = b && lti::write(handler, "nodeType", right->getTypeName());
//          b = b && right->write(handler, complete);
//        } else {
//          b = b && lti::write(handler, "nodeType", "nil");
//        }

    }


    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }


  /*
   * read the dtNode from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if write was successful
   */
  bool decisionTree::dtNode::read(ioHandler& handler,
                                        const bool complete)
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      b = b && lti::read(handler, "id", id);

//        std::string str;

//        b = b && lti::read(handler, "nodeType", str);
//        if (str!="nil") {
//          left=nodeFactory.newInstance(str);
//          b = b && left->read(handler, complete);
//        }

//        b = b && lti::read(handler, "nodeType", str);
//        if (str!="nil") {
//          right=nodeFactory.newInstance(str);
//          b = b && right->read(handler, complete);
//        }

    }


    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool decisionTree::dtNode::readMS(ioHandler& handler,
                                    const bool complete) {
    return true;
  };

  bool decisionTree::dtNode::writeMS(ioHandler& handler,
                                     const bool complete) const {
    return true;
  };
# endif

  // assign ids to leaf nodes
  int decisionTree::dtNode::assignIds() {

    std::stack<decisionTree::dtNode*> st;
    int idCount=0;
    decisionTree::dtNode* actNode=this;
    decisionTree::dtNode* actChild=0;


    st.push(actNode);

    while (!st.empty()) {

      actNode=st.top();
      if (actNode->isLeaf()) {
        actNode->setId(idCount++);
      } else {
        actChild=actNode->right;
        if (actChild!=0) {
          st.push(actChild);
        }
        actChild=actNode->left;
        if (actChild!=0) {
          st.push(actChild);
        }
      }
      st.pop();
    }

    return idCount;

  }

  // --------------------------------------------------
  // decisionTree
  // --------------------------------------------------


  const decisionTree::dtNode* decisionTree::nodeArray[]={
    new crispDecisionTree::crispNode(),
    NULL
  };

  const objectFactory<decisionTree::dtNode>
  decisionTree::nodeFactory(decisionTree::nodeArray);

  // default constructor
  decisionTree::decisionTree()
    : supervisedInstanceClassifier(), numberOfLeafs(0) {

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // copy constructor
  decisionTree::decisionTree(const decisionTree& other)
    : supervisedInstanceClassifier()  {
    copy(other);
  }

  // destructor
  decisionTree::~decisionTree() {
  }

  // returns the name of this type
  const char* decisionTree::getTypeName() const {
    return "decisionTree";
  }

  // copy member
  decisionTree&
    decisionTree::copy(const decisionTree& other) {
      supervisedInstanceClassifier::copy(other);

      numberOfLeafs=other.numberOfLeafs;

    return (*this);
  }

  // alias for copy member
  decisionTree&
    decisionTree::operator=(const decisionTree& other) {
    return (copy(other));
  }

  // return parameters
  const decisionTree::parameters&
    decisionTree::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // write
  bool
  decisionTree::write(ioHandler& handler, const bool complete) const {

    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    b = b && supervisedInstanceClassifier::write(handler, false);

    b = b && lti::write(handler, "numberOfLeafs", numberOfLeafs);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;

  }

  // read
  bool
  decisionTree::read(ioHandler& handler, const bool complete) {

    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    b = b && supervisedInstanceClassifier::read(handler, false);

    b = b && lti::read(handler, "numberOfLeafs", numberOfLeafs);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;

  }

}
