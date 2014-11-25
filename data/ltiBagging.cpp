/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiBagging.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 24.2.2003
 * revisions ..: $Id: ltiBagging.cpp,v 1.8 2006/09/05 09:56:17 ltilib Exp $
 */

#include "ltiBagging.h"
#include "ltiUniformDist.h"
#include "ltiRbf.h"

#include "ltiProgressInfo.h"

namespace lti {
  // --------------------------------------------------
  // bagging::parameters
  // --------------------------------------------------

  // default constructor
  bagging::parameters::parameters()
    : classifier::parameters() {
    nbClassifiers = 10;
    nbTrainingVectors = 100;
    useClassifier = "lti::rbf";
    classifierParam = new rbf::parameters();

    // the both parameter classes don't have to be initialized, because the
    // the standard parameters are used

  }

  // copy constructor
  bagging::parameters::parameters(const parameters& other)
    : classifier::parameters() {
    copy(other);
  }

  // destructor
  bagging::parameters::~parameters() {
  }

  // get type name
  const char* bagging::parameters::getTypeName() const {
    return "bagging::parameters";
  }

  // copy member

  bagging::parameters&
    bagging::parameters::copy(const parameters& other) {
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

    nbClassifiers = other.nbClassifiers;
    nbTrainingVectors = other.nbTrainingVectors;
    useClassifier = other.useClassifier;
    combineParam = other.combineParam;
    classifierParam = other.classifierParam;
    return *this;
  }

  // alias for copy member
  bagging::parameters&
    bagging::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  classifier::parameters* bagging::parameters::clone() const {
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
  bool bagging::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool bagging::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"nbClassifiers",nbClassifiers);
      lti::write(handler,"nbTrainingVectors",nbTrainingVectors);
      lti::write(handler,"combineParam",combineParam);
      lti::write(handler,"useClassifier",useClassifier);

//       switch(useClassifier) {
//         case Rbf:
//           lti::write(handler,"useClassifier","Rbf");
//           break;
//         case MLP:
//           lti::write(handler,"useClassifier","MLP");
//           break;
//         case ManualCrispDecisionTree:
//           lti::write(handler,"useClassifier","ManualCrispDecisionTree");
//           break;
//         case Svm:
//           lti::write(handler,"useClassifier","Svm");
//           break;
//         case Lvq:
//           lti::write(handler,"useClassifier","Lvq");
//           break;
//         case ShClassifier:
//           lti::write(handler,"useClassifier","ShClassifier");
//           break;
//       }
      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && classifier::parameters::write(handler,false);
# else
    bool (classifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      classifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool bagging::parameters::write(ioHandler& handler,
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
  bool bagging::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool bagging::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }
    if (b) {
      lti::read(handler,"nbClassifiers",nbClassifiers);
      lti::read(handler,"nbTrainingVectors",nbTrainingVectors);
      lti::read(handler,"combineParam",combineParam);
      lti::read(handler,"useClassifier",useClassifier);
      // std::string str;
//       lti::read(handler,"useClassifier",str);
//       if (str=="MLP") {
//           useClassifier=MLP;
//       } else if (str=="ManualCrispDecisionTree") {
//           useClassifier=ManualCrispDecisionTree;
//       } else if (str=="Svm") {
//           useClassifier=Svm;
//       } else if (str=="ShClassifier") {
//           useClassifier=ShClassifier;
//       } else if (str=="Lvq") {
//           useClassifier=Lvq;
//       } else {
//           useClassifier=Rbf;
//       }
      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && classifier::parameters::read(handler,false);
# else
    bool (classifier::parameters::* p_readMS)(ioHandler&,const bool) =
      classifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool bagging::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // bagging
  // --------------------------------------------------

  // default constructor
  bagging::bagging()
    : classifier(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

    ensemble = new std::list<supervisedInstanceClassifier*>();

//     const char* names[] = { "Rbf", "MLP", "Lvq", "ShClassifier",
//                             "manualCrispDecisionTree", "Svm", 0};
//     const supervisedInstanceClassifier* 
//         prototypes[]={new rbf(), new MLP(), new lvq(), new shClassifier(),
//                       new manualCrispDecisionTree(), new svm(), 0};
//     factory = 
//         new objectFactory<supervisedInstanceClassifier>(names,prototypes);
  }

  // default constructor
  bagging::bagging(const parameters& par)
    : classifier() {

    // set the given parameters
    setParameters(par);

    ensemble = new std::list<supervisedInstanceClassifier*>();

//     const char* names[] = { "Rbf", "MLP", "Lvq", "ShClassifier",
//                             "manualCrispDecisionTree", "Svm", 0};

//     const supervisedInstanceClassifier* 
//         prototypes[]={new rbf(), new MLP(), new lvq(), new shClassifier(),
//                       new manualCrispDecisionTree(), new svm(), 0};

//     factory = 
//         new objectFactory<supervisedInstanceClassifier>(names,prototypes);
  }


  // copy constructor
  bagging::bagging(const bagging& other)
    : classifier() {
    copy(other);
  }

  // destructor
  bagging::~bagging() {
    std::list<supervisedInstanceClassifier*>::iterator it;
    const std::list<supervisedInstanceClassifier*>::iterator 
        it2=ensemble->end();
    for (it=ensemble->begin(); it!=it2; it++) {
      delete (*it);
    }
  }

  // returns the name of this type
  const char* bagging::getTypeName() const {
    return "bagging";
  }

  // copy member
  bagging&
    bagging::copy(const bagging& other) {
      classifier::copy(other);

    return (*this);
  }

  // alias for copy member
  bagging&
    bagging::operator=(const bagging& other) {
    return (copy(other));
  }


  // clone member
  classifier* bagging::clone() const {
    return new bagging(*this);
  }

  // return parameters
  const bagging::parameters&
    bagging::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool bagging::train(const dmatrix &input, const ivector& ids) {
    parameters p=getParameters();
    int nbPoints=input.rows();
    int nbClassifiers=p.nbClassifiers;
    int nbTrainingVectors=p.nbTrainingVectors;
    bool ok=true;
    supervisedInstanceClassifier* tmpClassifier;

    // random number generator between 0 and nbPoints
    uniformDistribution rnd(0.0,double(nbPoints)+0.999999);
    int i,j,k;
    dmatrix tmp(nbTrainingVectors,input.columns());
    ivector tmpIds(nbTrainingVectors);

    if (validProgressObject()) {
      getProgressObject().reset();
      getProgressObject().setTitle("bagging: training");
      getProgressObject().setMaxSteps(nbClassifiers);
    }
    for (k=0; k<nbClassifiers; k++) {
        // generate training data for classifier number k
      for (i=0; i<nbTrainingVectors; i++) {
        j=static_cast<int>(rnd.draw());
        tmp.setRow(i,input.getRow(j));
        tmpIds.at(i)=ids.at(j);
      }
      tmpClassifier = factory.newInstance(p.useClassifier);
      // i training point in train matrix
      if (validProgressObject()) {
        getProgressObject().step("       Training classifier for ensemble.\n");
        tmpClassifier->setProgressObject((getProgressObject()));
      }
      tmpClassifier->setParameters(*(p.classifierParam));
      ok= ok && tmpClassifier->train(tmp,tmpIds);
      ensemble->push_back(tmpClassifier);
    }
    // all objects in the ensemble are trained now
    return ok;
  };

  bool bagging::classify(const dvector& feature,outputVector& result) const {
      std::list<outputVector> results;
      outputVector* tmp;
      bool ok=true;
      std::list<supervisedInstanceClassifier*>::iterator it;

      for (it=ensemble->begin(); it!=ensemble->end(); it++) {
          tmp = new outputVector;
          ok = ok && (*it)->classify(feature,(*tmp));
          results.push_back((*tmp));
      } 

      combination combiner;
      combiner.setParameters(getParameters().combineParam);

      ok = ok && combiner.apply(results,result);
         
    return ok;
  };

  bool bagging::read(ioHandler& handler,const bool complete) {
    bool b(true);
    if(complete) {
      b = handler.readBegin();
    }
    // read the standard data (output and parameters)
    classifier::read(handler,false);

    int nbClassifiers=getParameters().nbClassifiers;
    int i;
    supervisedInstanceClassifier *c;
//     std::string str;
//     switch (getParameters().useClassifier) {
//       case parameters::Rbf:
//         str="Rbf";
//         break;
//       case parameters::ManualCrispDecisionTree:
//         str="manualCrispDecisionTree";
//         break;
//       case parameters::Lvq:
//         str="Lvq";
//         break;
//       case parameters::MLP:
//         str="MLP";
//         break;
//       case parameters::ShClassifier:
//         str="ShClassifier";
//         break;
//       case parameters::Svm:
//         str="Svm";
//         break;
//       default:
//         break;
//     }

    if (b) {
      for (i=0; i<nbClassifiers; i++) {
        c = factory.newInstance(getParameters().useClassifier);
        c->read(handler,false);
        ensemble->push_back(c);
      }
    }

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

  bool bagging::write(ioHandler& handler,const bool complete) const {
    bool b(true);

    if (complete) {
      b = handler.writeBegin();
    }

    // write the standard data (output and parameters)
    classifier::write(handler,false);

    std::list<supervisedInstanceClassifier*>::iterator it;
    if (b) {
      for (it=ensemble->begin(); it!=ensemble->end(); it++) {
        (*it)->write(handler,false);
      }
    }

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

}
