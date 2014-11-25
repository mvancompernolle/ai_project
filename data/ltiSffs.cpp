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
 * file .......: ltiSffs.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 11.7.2002
 * revisions ..: $Id: ltiSffs.cpp,v 1.12 2006/09/05 10:01:53 ltilib Exp $
 */

#include "ltiSffs.h"
#include "ltiSequentialForwardSearch.h"
#include <list>
#include "ltiBhattacharyyaDistOfSubset.h"

namespace lti {
  // --------------------------------------------------
  // sffs::parameters
  // --------------------------------------------------

  // default constructor
  sffs::parameters::parameters() 
    : featureSelector::parameters() {    

      usedCostFunction = new bhattacharyyaDistOfSubset();
  }

  // copy constructor
  sffs::parameters::parameters(const parameters& other) {
    copy(other);
  }
  
  // destructor
  sffs::parameters::~parameters() {    
  }

  // get type name
  const char* sffs::parameters::getTypeName() const {
    return "sffs::parameters";
  }
  
  // copy member

  sffs::parameters& 
  sffs::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    featureSelector::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    featureSelector::parameters& (featureSelector::parameters::* p_copy)
      (const featureSelector::parameters&) = 
      featureSelector::parameters::copy;
    (this->*p_copy)(other);
# endif
    
    
//     classifier = other.classifier;
    usedCostFunction  = other.usedCostFunction;
    return *this;
  }

  // alias for copy member
  sffs::parameters& 
  sffs::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* sffs::parameters::clone() const {
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
  bool sffs::parameters::write(ioHandler& handler,
                               const bool complete) const 
# else
    bool sffs::parameters::writeMS(ioHandler& handler,
                                   const bool complete) const 
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      usedCostFunction->write(handler,false);      
//       lti::write(handler,"classifier",classifier->getTypeName());
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to 
    // compile...
    b = b && featureSelector::parameters::write(handler,false);
# else
    bool (featureSelector::parameters::* p_writeMS)(ioHandler&,const bool) const = 
      featureSelector::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool sffs::parameters::write(ioHandler& handler,
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
  bool sffs::parameters::read(ioHandler& handler,
                              const bool complete) 
# else
    bool sffs::parameters::readMS(ioHandler& handler,
                                  const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      usedCostFunction->read(handler,false);
//       std::string str;
//       lti::read(handler,"classifier",str);
//       const supervisedInstanceClassifier* types[] = { new rbf, new lvq, 
//                                                       new svm, new shClassifier, new manualCrispDecisionTree, 0 };
//       objectFactory<supervisedInstanceClassifier> objFac(types);
//       classifier = objFac.newInstance(str);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to 
    // compile...
    b = b && featureSelector::parameters::read(handler,false);
# else
    bool (featureSelector::parameters::* p_readMS)(ioHandler&,const bool) = 
      featureSelector::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool sffs::parameters::read(ioHandler& handler,
                              const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // sffs
  // --------------------------------------------------

  // default constructor
  sffs::sffs()
    : featureSelector(){

    //TODO: comment the attributes of your functor
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
  sffs::sffs(const sffs& other) {
    copy(other);
  }

  // destructor
  sffs::~sffs() {
  }

  // returns the name of this type
  const char* sffs::getTypeName() const {
    return "sffs";
  }

  // copy member
  sffs& sffs::copy(const sffs& other) {
    featureSelector::copy(other);

    return (*this);
  }

  // alias for copy member
  sffs&
  sffs::operator=(const sffs& other) {
    return (copy(other));
  }


  // clone member
  functor* sffs::clone() const {
    return new sffs(*this);
  }

  // return parameters
  const sffs::parameters&
  sffs::getParameters() const {
    const parameters* par = 
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException();
    }
    return *par;
  }
  
  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  

  bool sffs::apply(const dmatrix& src,const ivector& srcIds, 
                   dmatrix& dest) const {
    bool ok=true;
    dest.clear();
    parameters param=getParameters();
    // initialize cross validator

    costFunction *cF;
    cF = param.usedCostFunction;
    cF->setSrc(src,srcIds);

    int featureToInsert(0),featureToDelete(0),i;
    double oldRate,newRate;
    bool doInclude=true;
    bool terminate=false;
    int nbFeatures=src.columns();
    std::list<int> in,out;
    std::list<int>::iterator it;
    std::map<double,int> values;
    double value;
    for (i=0; i<nbFeatures; i++) {
      out.push_back(i);
    }
    ivector posInSrc(nbFeatures,-1);//saves the position in src of the inserted
    // feature to mark it as not used if this feature is deleted later
    dvector regRate(nbFeatures);  // the recognition rates after the insertion 
                                  // of a new feature
    if (param.nbFeatures<2) {
      setStatusString("You will have to choose at least two features. Set nbFeatures=2");
      return false;
    }

    // add the first best two features; do 2 steps sfs
    for (i=0; i<2; i++ ) {
      if (dest.columns()<src.columns() && !terminate) {
        // add space for one extra feature
        for (it=out.begin(); it!=out.end(); it++) {
          in.push_back(*it);
          cF->apply(in,value);
          values[value]=*it;
          in.pop_back();
        }
        // search for maximum in regRate; all possibilities not tested are -1
        in.push_back((--values.end())->second);
        out.remove((--values.end())->second);
      }
    }
    cF->apply(in,oldRate);
    while (!terminate) {
      // STEP 1: include the best possible feature
      if (static_cast<int>(in.size())<src.columns() && 
          !terminate && doInclude) {
        values.clear();
        for (it=out.begin(); it!=out.end(); it++) {
          in.push_back(*it);
          cF->apply(in,value);
          values[value]=*it;
          in.pop_back();
        }
        featureToInsert=(--values.end())->second;
        in.push_back(featureToInsert);
        out.remove(featureToInsert);
      }
      // STEP 2: conditional exclusion
      if (in.size()>0 && !terminate) {
        values.clear();
        for (it=in.begin(); it!=in.end(); it++) {
          int tmp=*it;
          it=in.erase(it);
          cF->apply(in,value);
          values[value]=tmp;
          in.insert(it,tmp);
          it--;
        }
        featureToDelete=(--values.end())->second;

        // if the least significant feature is equal to the most significant
        // feature that was included in step 1, leave feature and 
        // include the next one
        if (featureToDelete==featureToInsert) {
          doInclude=true;
        } else {    // delete this feature and compute new recognition rate

          // if the feature to delete is not the last feature in dest,
          // change the feature against the last feature in dest and delete
          // the last column in dest, otherwise if the feature to delete 
          // is equal to the last feature in dest nothing will be done, 
          // because this is already the lacking feature in temp
          cF->apply(in,newRate);
          // if recognition rate without least significant feature is better 
          // than with this feature delete it
          if (newRate>oldRate) { 

            in.remove(featureToDelete);
            out.push_back(featureToDelete);
            // search for another least significant feature before 
            // including the next one
            doInclude=false;
            oldRate=newRate;
          } else {
            doInclude=true;
          }
          // if only two features left, include the next one
          if (dest.columns()<=2) {
            doInclude=true;    
          }
        }          
      } // end of exclusion
      // test if the predetermined number of features is reached
      terminate=(param.nbFeatures==static_cast<int>(in.size()));
    } // while (!terminate)

    // Now fill dest
    const int sz = static_cast<int>(in.size());
    dest.resize(src.rows(), sz, 0., false, false);
    ivector idvec(false, sz);
    std::list<int>::const_iterator lit = in.begin();
    for (i=0; i < sz; ++i) {
      idvec.at(i)=*lit;
      ++lit;
    }
    for (i=0; i < src.rows(); ++i) {
      const dvector& svec = src.getRow(i);
      dvector& dvec = dest.getRow(i);
      for (int j=0; j < sz; ++j) {
        dvec.at(j) = svec.at(idvec.at(j));
      }
    }

    return ok;
  };


}
