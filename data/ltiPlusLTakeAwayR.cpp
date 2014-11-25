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
 * file .......: ltiPlusLTakeAwayR.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 8.7.2002
 * revisions ..: $Id: ltiPlusLTakeAwayR.cpp,v 1.9 2006/09/05 09:59:44 ltilib Exp $
 */

#include "ltiPlusLTakeAwayR.h"
#include "ltiBhattacharyyaDistOfSubset.h"
#include "ltiGenericVector.h"
#include <map>
#include <algorithm>
#include <list>

namespace lti {
  // --------------------------------------------------
  // plusLTakeAwayR::parameters
  // --------------------------------------------------

  // default constructor
  plusLTakeAwayR::parameters::parameters() 
    : featureSelector::parameters() {    
    
    r = 1;
    l = 0;
//     cVParameter = crossValidator::parameters();
     usedCostFunction = new bhattacharyyaDistOfSubset();
  }

  // copy constructor
  plusLTakeAwayR::parameters::parameters(const parameters& other) {
    copy(other);
  }
  
  // destructor
  plusLTakeAwayR::parameters::~parameters() {    
  }

  // get type name
  const char* plusLTakeAwayR::parameters::getTypeName() const {
    return "plusLTakeAwayR::parameters";
  }
  
  // copy member

  plusLTakeAwayR::parameters& 
    plusLTakeAwayR::parameters::copy(const parameters& other) {
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
    
    
      r = other.r;
//        nbOfSplits = other.nbOfSplits;
      l = other.l;
//       cVParameter = other.cVParameter;
      usedCostFunction = other.usedCostFunction;

    return *this;
  }

  // alias for copy member
  plusLTakeAwayR::parameters& 
    plusLTakeAwayR::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* plusLTakeAwayR::parameters::clone() const {
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
  bool plusLTakeAwayR::parameters::write(ioHandler& handler,
                                         const bool complete) const 
# else
  bool plusLTakeAwayR::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const 
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"r",r);
      lti::write(handler,"l",l);
      usedCostFunction->write(handler,false);
//       if (costFunction==bhattacharyyaDistance) {
//         lti::write(handler,"costFunction","bhattacharyyaDistance");
//       }
//       else if (costFunction==mahalanobisDistance ){
//         lti::write(handler,"costFunction","mahalanobisDistance");
//       } else {
//         lti::write(handler,"costFunction","crossValidation");
//       } 
//       cVParameter.write(handler,false);
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
  bool plusLTakeAwayR::parameters::write(ioHandler& handler,
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
  bool plusLTakeAwayR::parameters::read(ioHandler& handler,
                                        const bool complete) 
# else
  bool plusLTakeAwayR::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"r",r);
      lti::read(handler,"l",l);
      usedCostFunction->read(handler,false);
//       std::string str;
//       lti::read(handler,"costFunction",str);
//       if (str=="bhattacharyyaDistance") {
//         costFunction=bhattacharyyaDistance;
//       }
//       else if (str=="mahalanobisDistance" ){
//         costFunction=mahalanobisDistance;
//       } else {
//         costFunction=crossValidation;
//       } 

//       cVParameter.read(handler,false);
//       lti::read(handler,"classify",str);
//       const supervisedInstanceClassifier* types[] = { new rbf, new lvq, 
//                    new svm, new shClassifier, new manualCrispDecisionTree, 0 };
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
  bool plusLTakeAwayR::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // plusLTakeAwayR
  // --------------------------------------------------

  // default constructor
  plusLTakeAwayR::plusLTakeAwayR()
    : featureSelector(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // copy constructor
  plusLTakeAwayR::plusLTakeAwayR(const plusLTakeAwayR& other) {
    copy(other);
  }

  // destructor
  plusLTakeAwayR::~plusLTakeAwayR() {
  }

  // returns the name of this type
  const char* plusLTakeAwayR::getTypeName() const {
    return "plusLTakeAwayR";
  }

  // copy member
  plusLTakeAwayR& plusLTakeAwayR::copy(const plusLTakeAwayR& other) {
    featureSelector::copy(other);

    return (*this);
  }

  // alias for copy member
  plusLTakeAwayR&
    plusLTakeAwayR::operator=(const plusLTakeAwayR& other) {
    return (copy(other));
  }


  // clone member
  functor* plusLTakeAwayR::clone() const {
    return new plusLTakeAwayR(*this);
  }

  // return parameters
  const plusLTakeAwayR::parameters&
    plusLTakeAwayR::getParameters() const {
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
  
    bool plusLTakeAwayR::apply(const dmatrix& src,const ivector& srcIds, 
                               dmatrix& dest) const {
    parameters param=getParameters();
    int r=param.r;
    int l=param.l;
    int nbFeatures=src.columns();
    bool ok=true;
    int i,j;
    genericVector<bool> inDest(nbFeatures);
    ivector posInSrc(nbFeatures,-1);//saves the position in src of the inserted
    // feature to mark it as not used if this feature is deleted later
    dvector regRate(nbFeatures);  // the recognition rates after the insertion 
    // aof a new feature
    unsigned int dim=src.columns();

    std::list<int> in;
    std::list<int> out;

    // set start condition
    if (l==r) { 
      setStatusString("no valid choice of l and r");
      return false;
    }
    if (param.nbFeatures>=src.columns()) { // if nbFeature=the resulting 
                                           // number of features
        setStatusString("nbFeatures is equal or bigger than the number of features in the source data");
        dest.resize(src.rows(),src.columns());
        dest=src;
        return false;
    }
    if (l>r) { // if l>r search "bottom up"
        dest.resize(src.rows(),0);
        inDest.fill(false);
        in.clear();
        for (i=0; i<static_cast<int>(dim); i++) {
            out.push_back(i);
        }

    } else { // if l<r search "top dowm"
        dest.resize(src.rows(),src.columns());
        dest=src;
        inDest.fill(true);

        for (i=0; i<static_cast<int>(dim); i++) {
            in.push_back(i);
        }
        out.clear();

        for (i=0; i<posInSrc.size(); i++) {
            posInSrc.at(i)=i;
        }
    }

    // initalize the cost function
    costFunction *cF=param.usedCostFunction;
    cF->setSrc(src,srcIds);

    std::map<double,int> cFValues;
    std::list<int>::iterator listIt;
    double value;
    bool terminate=false;

    while (!terminate) {
        // STEP 1: Insert l features
      for (i=0; i<l; i++) {
        if (in.size()<dim && !terminate) {
          list<int> tmp(in);
          j=0;
          cFValues.clear();
          for (listIt=out.begin(); listIt!=out.end(); listIt++) {
            // test all features that are not already included
            tmp.push_back((*listIt));
            cF->apply(tmp,value);
            tmp.pop_back();
            cFValues[value]=(*listIt);
          }
          // find the element with the biggest value and included the
          // corresponding feature
          int inc=(*std::max_element(cFValues.begin(),cFValues.end())).second;
          in.push_back(inc);
          out.erase(std::find(out.begin(),out.end(),inc));
        }
        if ( param.nbFeatures==int(in.size()) )
            terminate=true;
      }

      // STEP 2: delete r features
      for (i=0; i<r; i++) {
        if (in.size()>0 && !terminate) {
          cFValues.clear();
          // for all elements in the include list do the following:
          // 1. remove the current element
          // 2. compute the cost Function
          // 3. the case where the cost function reaches its maximum, is the 
          //    case where the most unrelevant feature was excluded. 
          int tmp;
          for (listIt=in.begin(); listIt!=in.end(); listIt++) {
            tmp=(*listIt); // the feature that is deleted
            listIt=in.erase(listIt);
            cF->apply(in,value);
            in.insert(listIt,tmp); // stelle alten zustand wieder her
            listIt--;
            cFValues[value]=tmp;
          }
          int inc=(*std::max_element(cFValues.begin(),cFValues.end())).second;
          in.erase(std::find(in.begin(),in.end(),inc));
          out.push_back(inc);
        }
        if ( param.nbFeatures==int(in.size()) )
          terminate=true;
      }
/*      for (i=0; i<l; i++) {
        if (dest.columns()<src.columns() && !terminate) {
          regRate.fill(-1.0); // set all recognition rates to zero
          // add space for one extra feature
          dest.resize(src.rows(),dest.columns()+1); 
          for (j=0; j<inDest.size(); j++) {
            if (!inDest.at(j)) { // if feature is not already in dest
              dest.setColumn(dest.columns()-1,src.getColumnCopy(j));
              cV.apply(dest,srcIds,regRate.at(j));
              
            } 
          }
          // search for maximum in regRate; all possibilities not tested are -1
          featureToInsert=regRate.getIndexOfMaximum();
          dest.setColumn(dest.columns()-1,src.getColumnCopy(featureToInsert));
          inDest.at(featureToInsert)=true; // mark feature as inserted
  1        posInSrc.at(dest.columns()-1)=featureToInsert;

          if (param.nbFeatures==dest.columns()) // the predetermined number 
              // of features is reached -> terminate
            terminate=true;
        }
      }
      // STEP 2: delete r features
      for (i=0; i<r; i++) {
        if (dest.columns()>0 && !terminate) {
          dmatrix tmp(dest);
          tmp.resize(tmp.rows(),tmp.columns()-1); //cut off last element
          // fill with more than the maximum recognition rate
          regRate.fill(-1.0); 
          cV.apply(tmp,srcIds,regRate.at(tmp.columns()));
          for (j=0; j<tmp.columns(); j++) {
          // set last column of dest to the next column in tmp and
            tmp.setColumn(j,dest.getColumnCopy(dest.columns()-1));
            cV.apply(dest,srcIds,regRate.at(j));
          // restore old value at this place in tmp
            tmp.setColumn(j,dest.getColumnCopy(j));
          }
          featureToDelete=regRate.getIndexOfMaximum();
          // delete least significant feature
          dest.setColumn(featureToDelete,dest.getColumnCopy(dest.columns()-1));
          //mark feature as deleted
          inDest.at(posInSrc.at(featureToDelete))=false;
          posInSrc.at(featureToDelete)=posInSrc.at(dest.columns()-1);
          posInSrc.at(dest.columns()-1)=-1;
          dest.resize(dest.rows(),dest.columns()-1);

          // true if the termination criterion is reached
          terminate = (param.nbFeatures==dest.columns());
        }
      }
*/
    }

    dest.resize(src.rows(),param.nbFeatures);
    i=0;
    // fuege alle features aus "in" in dest ein.
    for (listIt=in.begin(); listIt!=in.end(); listIt++) {
      dest.setColumn(i,src.getColumnCopy((*listIt)));
      i++;
    }

    return ok; 
  };



}
