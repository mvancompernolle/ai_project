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
 * file .......: ltiManualCrispDecisionTree.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 25.2.2002
 * revisions ..: $Id: ltiManualCrispDecisionTree.cpp,v 1.7 2006/09/05 09:59:35 ltilib Exp $
 */

#include "ltiObject.h"

#include <queue>
#include <stack>
#include <vector>

#include "ltiManualCrispDecisionTree.h"

namespace lti {
  // --------------------------------------------------
  // manualCrispDecisionTree::parameters
  // --------------------------------------------------

  // default constructor
  manualCrispDecisionTree::parameters::parameters()
    : crispDecisionTree::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

  }

  // copy constructor
  manualCrispDecisionTree::parameters::parameters(const parameters& other)
    : crispDecisionTree::parameters()  {
    copy(other);
  }

  // destructor
  manualCrispDecisionTree::parameters::~parameters() {
  }

  // get type name
  const char* manualCrispDecisionTree::parameters::getTypeName() const {
    return "manualCrispDecisionTree::parameters";
  }

  // copy member

  manualCrispDecisionTree::parameters&
    manualCrispDecisionTree::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    crispDecisionTree::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    crispDecisionTree::parameters& (crispDecisionTree::parameters::* p_copy)
      (const crispDecisionTree::parameters&) =
      crispDecisionTree::parameters::copy;
    (this->*p_copy)(other);
# endif


    return *this;
  }

  // alias for copy member
  manualCrispDecisionTree::parameters&
    manualCrispDecisionTree::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  classifier::parameters* manualCrispDecisionTree::parameters::clone() const {
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
  bool manualCrispDecisionTree::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool manualCrispDecisionTree::parameters::writeMS(ioHandler& handler,
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
    b = b && crispDecisionTree::parameters::write(handler,false);
# else
    bool (crispDecisionTree::parameters::* p_writeMS)(ioHandler&,const bool) const =
      crispDecisionTree::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool manualCrispDecisionTree::parameters::write(ioHandler& handler,
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
  bool manualCrispDecisionTree::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool manualCrispDecisionTree::parameters::readMS(ioHandler& handler,
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
    b = b && crispDecisionTree::parameters::read(handler,false);
# else
    bool (crispDecisionTree::parameters::* p_readMS)(ioHandler&,const bool) =
      crispDecisionTree::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool manualCrispDecisionTree::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // manualCrispDecisionTree
  // --------------------------------------------------

  // default constructor
  manualCrispDecisionTree::manualCrispDecisionTree()
    : crispDecisionTree(){

    //TODO: comment the attributes of your classifier
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // copy constructor
  manualCrispDecisionTree::manualCrispDecisionTree(const manualCrispDecisionTree& other)
    : crispDecisionTree()  {
    copy(other);
  }

  // destructor
  manualCrispDecisionTree::~manualCrispDecisionTree() {
  }

  // returns the name of this type
  const char* manualCrispDecisionTree::getTypeName() const {
    return "manualCrispDecisionTree";
  }

  // copy member
  manualCrispDecisionTree&
    manualCrispDecisionTree::copy(const manualCrispDecisionTree& other) {
      crispDecisionTree::copy(other);

    //TODO: comment the attributes of your classifier
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  manualCrispDecisionTree&
    manualCrispDecisionTree::operator=(const manualCrispDecisionTree& other) {
    return (copy(other));
  }


  // clone member
  classifier* manualCrispDecisionTree::clone() const {
    return new manualCrispDecisionTree(*this);
  }

  // return parameters
  const manualCrispDecisionTree::parameters&
    manualCrispDecisionTree::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The train-methods!
  // -------------------------------------------------------------------

  // train
  bool manualCrispDecisionTree::train(const dmatrix& data,
                                      const ivector& ids) {

    bool b=true;

    // need a stack for construction of tree
    int i=1; // init to one because first row is root, done outside loop
    int j;
    // collects the given ids of the nodes.
    std::vector<int> tIds;
    crispNode* node=0;
    crispNode* newNode=0;
    std::stack<crispNode*> st;
    numberOfLeafs=0;

    // find the first value in the current data row that is unequal to zero
    // and create a new crisp node with univariate decision function and
    // the, for now, the given id.
    const dvector& actData = data.getRow(0);
    for (j=0; j<actData.size(); j++) {
      if (actData.at(j)!=0.) {
        break;
      }
    }
    if (j<actData.size()) {
      node = new crispNode(j, data.at(0,j), ids.at(0));
    } else {
      node = new crispNode(ids.at(0));
    }
    // the first node is the root node
    root=node;
    st.push(node);

    while (!st.empty()) {
      // get a node from the stack and pop it.
      node=st.top();
      st.pop();

      // if the node's id is not -1, ie it is a leaf, get the id
      // set by the user, push into tIds and assign correct pre-order
      // id to the node
      if (node->getId()!=-1) {
        tIds.push_back(node->getId());
        node->setId(numberOfLeafs++);
        // the node is an inner node. a new node needs to be created and
        // inserted in the tree. First find the value != 0 in the current
        // row of data, then construct the node.
      } else {
        const dvector& actData = data.getRow(i);
        for (j=0; j<actData.size(); j++) {
          if (actData.at(j)!=0.) {
            break;
          }
        }
        if (j<actData.size()) {
          newNode = new crispNode(j, data.at(i,j), ids.at(i));
        } else {
          newNode = new crispNode(ids.at(i));
        }
        i++;
        // if the new node is a left child also push the current node
        // on the stack since it needs to get a right node later.
        if (node->getLeftChild()==0) {
          node->setLeftChild(newNode);
          st.push(node);
        } else {
          node->setRightChild(newNode);
        }
        st.push(newNode);
      }
    }

    // when the stack is empty, all data should have been used
    if (i!=ids.size()) {
      b=false;
      setStatusString("manualCrispDecisionTree: Error during training");
    }

    // create the outputTemplate of the decision tree using the ids
    // supplied by the user.
    ivector vec(tIds);
    outTemplate=classifier::outputTemplate(vec);

    return b;

  }


}
