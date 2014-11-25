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
 * file .......: ltiSOFM.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 12.8.2002
 * revisions ..: $Id: ltiSOFM.cpp,v 1.7 2006/09/05 10:00:56 ltilib Exp $
 */

#include "ltiSOFM.h"

namespace lti {
  // --------------------------------------------------
  // SOFM::parameters
  // --------------------------------------------------

  // default constructor
  SOFM::parameters::parameters()
    : unsupervisedClassifier::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters


    initType = Random;
    metricType = L2;
    orderLearnMax = double();
    convergenceLearnMax = double();
    stepsOrdering = int();
    stepsConvergence = int();
    orderNeighborThresh = double();
    orderNeighborMin = double();
    orderLearnMin = double();
    convergenceLearnMin = double();
    orderNeighborMax = double();
  }

  // copy constructor
  SOFM::parameters::parameters(const parameters& other)
    : unsupervisedClassifier::parameters()  {
    copy(other);
  }

  // destructor
  SOFM::parameters::~parameters() {
  }

  // get type name
  const char* SOFM::parameters::getTypeName() const {
    return "SOFM::parameters";
  }

  // copy member

  SOFM::parameters&
  SOFM::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    unsupervisedClassifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    unsupervisedClassifier::parameters& (unsupervisedClassifier::parameters::* p_copy)
      (const unsupervisedClassifier::parameters&) =
      unsupervisedClassifier::parameters::copy;
    (this->*p_copy)(other);
# endif

    initType = other.initType;
    metricType = other.metricType;
    orderLearnMax = other.orderLearnMax;
    convergenceLearnMax = other.convergenceLearnMax;
    stepsOrdering = other.stepsOrdering;
    stepsConvergence = other.stepsConvergence;
    orderNeighborThresh = other.orderNeighborThresh;
    orderNeighborMin = other.orderNeighborMin;
    orderLearnMin = other.orderLearnMin;
    convergenceLearnMin = other.convergenceLearnMin;
    orderNeighborMax = other.orderNeighborMax;

    return *this;
  }

  // alias for copy member
  SOFM::parameters&
  SOFM::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  classifier::parameters* SOFM::parameters::clone() const {
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
  bool SOFM::parameters::write(ioHandler& handler,
                               const bool complete) const
# else
    bool SOFM::parameters::writeMS(ioHandler& handler,
                                   const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      switch(initType) {
      case Random:
        lti::write(handler,"initType","Random");
        break;
      case Linear:
        lti::write(handler,"initType","Linear");
        break;
      default:
        b = false;
        lti::write(handler,"initType","Random");
      }

      switch(metricType) {
      case L1:
        lti::write(handler,"metricType","L1");
        break;
      case L2:
        lti::write(handler,"metricType","L2");
        break;
      case Dot:
        lti::write(handler,"metricType","Dot");
        break;
      default:
        b = false;
        lti::write(handler,"metricType","L2");
      }

      lti::write(handler,"orderLearnMax",orderLearnMax);
      lti::write(handler,"convergenceLearnMax",convergenceLearnMax);
      lti::write(handler,"stepsOrdering",stepsOrdering);
      lti::write(handler,"stepsConvergence",stepsConvergence);
      lti::write(handler,"orderNeighborThresh",orderNeighborThresh);
      lti::write(handler,"orderNeighborMin",orderNeighborMin);
      lti::write(handler,"orderLearnMin",orderLearnMin);
      lti::write(handler,"convergenceLearnMin",convergenceLearnMin);
      lti::write(handler,"orderNeighborMax",orderNeighborMax);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && unsupervisedClassifier::parameters::write(handler,false);
# else
    bool (unsupervisedClassifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      unsupervisedClassifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool SOFM::parameters::write(ioHandler& handler,
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
  bool SOFM::parameters::read(ioHandler& handler,
                              const bool complete)
# else
    bool SOFM::parameters::readMS(ioHandler& handler,
                                  const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      std::string str;

      lti::read(handler,"initType",str);
      if (str=="Random") {
        initType = Random;
      } else if (str=="Linear") {
        initType = Linear;
      } else {
        b=false;
        initType=Random;
      }

      lti::read(handler,"metricType",str);
      if (str=="L1") {
        metricType=L1;
      } else if (str=="L2") {
        metricType=L2;
      } else if (str=="Dot") {
        metricType=Dot;
      } else {
        b=false;
        metricType=L2;
      }

      lti::read(handler,"orderLearnMax",orderLearnMax);
      lti::read(handler,"convergenceLearnMax",convergenceLearnMax);
      lti::read(handler,"stepsOrdering",stepsOrdering);
      lti::read(handler,"stepsConvergence",stepsConvergence);
      lti::read(handler,"orderNeighborThresh",orderNeighborThresh);
      lti::read(handler,"orderNeighborMin",orderNeighborMin);
      lti::read(handler,"orderLearnMin",orderLearnMin);
      lti::read(handler,"convergenceLearnMin",convergenceLearnMin);
      lti::read(handler,"orderNeighborMax",orderNeighborMax);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && unsupervisedClassifier::parameters::read(handler,false);
# else
    bool (unsupervisedClassifier::parameters::* p_readMS)(ioHandler&,const bool) =
      unsupervisedClassifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool SOFM::parameters::read(ioHandler& handler,
                              const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // SOFM
  // --------------------------------------------------

  // default constructor
  SOFM::SOFM()
    : unsupervisedClassifier(), grid(0,0) {

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
  SOFM::SOFM(const SOFM& other)
    : unsupervisedClassifier()  {
    copy(other);
  }

  // destructor
  SOFM::~SOFM() {
  }

  // returns the name of this type
  const char* SOFM::getTypeName() const {
    return "SOFM";
  }

  // copy member
  SOFM&
  SOFM::copy(const SOFM& other) {
    unsupervisedClassifier::copy(other);

    //TODO: comment the attributes of your classifier
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  SOFM&
  SOFM::operator=(const SOFM& other) {
    return (copy(other));
  }


  // return parameters
  const SOFM::parameters&
  SOFM::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool SOFM::write(ioHandler& handler,const bool complete) const {
    bool b(true);

    if (complete) {
      b = handler.writeBegin();
    }

    // write the standard data (output and parameters)
    unsupervisedClassifier::write(handler,false);

    if (b) {
      lti::write(handler,"grid",grid);
    }

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

  bool SOFM::read(ioHandler& handler,const bool complete) {
    bool b(true);

    if (complete) {
      b = handler.readBegin();
    }

    // read the standard data (output and parameters)
    unsupervisedClassifier::read(handler,false);

    if (b) {
      lti::read(handler,"grid",grid);
    }

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

  // -------------------------------------------------------------------
  // The train-methods!
  // -------------------------------------------------------------------

  // Calls the same method of the superclass.
  bool SOFM::train(const dmatrix& input, ivector& ids) {

    return unsupervisedClassifier::train(input, ids);
  }


  // set learn-rates etc.
  bool SOFM::setLearnRates(const int& nbData) {
    const parameters& p=getParameters();

    double so=p.stepsOrdering*nbData;
    double sc=p.stepsConvergence*nbData;

    lrOrder=p.orderLearnMax;
    lrOrderDelta=(p.orderLearnMax-p.orderLearnMin)/so;

    if (p.orderNeighborThresh>1) return false;
    else if (p.orderNeighborThresh<=0) return false;
    sigma=p.orderNeighborMax/sqrt(-2*log(p.orderNeighborThresh));
    double sigmaEnd=p.orderNeighborMin/sqrt(-2*log(p.orderNeighborThresh));
    sigmaDelta=(sigma-sigmaEnd)/so;

    lrConvergeB=(sc*p.convergenceLearnMin)/
      (p.convergenceLearnMax-p.convergenceLearnMin);
    lrConvergeA=p.convergenceLearnMax*lrConvergeB;

    return true;
  }

}
