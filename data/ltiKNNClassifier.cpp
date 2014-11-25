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
 * file .......: ltikNNClassifier.cpp
 * authors ....: Jochen Wickel, Frederik Lange
 * organization: LTI, RWTH Aachen
 * creation ...: 30.10.2001
 * revisions ..: $Id: ltiKNNClassifier.cpp,v 1.15 2006/09/05 09:59:01 ltilib Exp $
 */

#include "ltiObject.h"
#include "ltiSTLIoInterface.h"
#include "ltiKNNClassifier.h"
#include "ltiL2Distance.h"

#include <limits>
#include <cstdio>

#undef _LTI_DEBUG
// #define _LTI_DEBUG 1
#include "ltiDebug.h"

// This stupid so-called C++ compiler from MS sucks!
#ifdef _LTI_MSC_6
#undef min
#undef max
#endif

namespace lti {

  // --------------------------------------------------
  // kNNClassifier static functions
  // --------------------------------------------------
  double kNNClassifier::linear(const double& r,const double& t) {
    return min(1.0,(r-1)/(t-1));
  }

  double kNNClassifier::exponential(const double& r,const double& t) {
    return (1.0 - exp(-(r-1.0)/t));
  }

  // --------------------------------------------------
  // kNNClassifier::parameters
  // --------------------------------------------------

  // default constructor
  kNNClassifier::parameters::parameters()
    : supervisedInstanceClassifier::parameters() {

    kNN = 1;
    normalizeData = true;
    normalizeOutput = true;
    useReliabilityMeasure = false;
    reliabilityMode = Linear;
    reliabilityThreshold = 10.0;
    maxUnreliableNeighborhood = 20;

    bestBinFirst = false;
    eMax = 100;
    bucketSize = 5;

  }

  // copy constructor
  kNNClassifier::parameters::parameters(const parameters& other) {
    copy(other);
  }

  // destructor
  kNNClassifier::parameters::~parameters() {
  }


  // get type name
  const char* kNNClassifier::parameters::getTypeName() const {
    return "kNNClassifier::parameters";
  }

  // copy member
  kNNClassifier::parameters&
  kNNClassifier::parameters::copy(const parameters& other) {
# if !defined(_LTI_MSC_6)
    // MS Visual C++ 6 is not able to compile this...
    supervisedInstanceClassifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    supervisedInstanceClassifier::parameters& 
      (supervisedInstanceClassifier::parameters::* p_copy)
      (const supervisedInstanceClassifier::parameters&) =
      supervisedInstanceClassifier::parameters::copy;
    (this->*p_copy)(other);
# endif

    kNN                   = other.kNN;
    normalizeData         = other.normalizeData;
    normalizeOutput       = other.normalizeOutput;
    useReliabilityMeasure = other.useReliabilityMeasure;
    reliabilityMode       = other.reliabilityMode;
    reliabilityThreshold  = other.reliabilityThreshold;
    maxUnreliableNeighborhood = other.maxUnreliableNeighborhood;
    bestBinFirst          = other.bestBinFirst;
    eMax                  = other.eMax;
    bucketSize            = other.bucketSize;
    return *this;
  }

  // alias for copy member
  kNNClassifier::parameters&
  kNNClassifier::parameters::operator=(const parameters& other) {
    copy(other);

    return *this;
  }

  // clone member
  classifier::parameters* kNNClassifier::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# if !defined(_LTI_MSC_6)
  bool kNNClassifier::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool kNNClassifier::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      b=b && lti::write(handler, "kNN",kNN);
      b=b && lti::write(handler, "normalizeData",normalizeData);
      b=b && lti::write(handler, "normalizeOutput",normalizeOutput);
      b=b && lti::write(handler, "useReliabilityMeasure",
                        useReliabilityMeasure);      

      switch(reliabilityMode) {
        case Linear:
          b=b && lti::write(handler, "reliabilityMode","Linear");
          break;
        case Exponential:
          b=b && lti::write(handler, "reliabilityMode","Exponential");
          break;
        default:
          b=b && lti::write(handler, "reliabilityMode","Linear");
          handler.setStatusString("Reliability mode unknown!");
          b=false;
      }

      b=b && lti::write(handler, "reliabilityThreshold",reliabilityThreshold);
      b=b && lti::write(handler, "maxUnreliableNeighborhood",
                        maxUnreliableNeighborhood);

      b=b && lti::write(handler, "bestBinFirst",bestBinFirst);
      b=b && lti::write(handler, "eMax",eMax);
      b=b && lti::write(handler, "bucketSize",bucketSize);

    }

# if !defined(_LTI_MSC_6)
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

# if defined(_LTI_MSC_6)
  bool kNNClassifier::parameters::write(ioHandler& handler,
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
# if !defined(_LTI_MSC_6)
  bool kNNClassifier::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool kNNClassifier::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      b=lti::read(handler, "kNN",kNN) && b;

      b=lti::read(handler, "normalizeData",normalizeData) && b;
      b=lti::read(handler, "normalizeOutput",normalizeOutput) && b;

      b=lti::read(handler, "useReliabilityMeasure",useReliabilityMeasure) && b;

      std::string str;
      b=lti::read(handler,"reliabilityMode",str) && b;
      if (str == "Exponential") {
        reliabilityMode = Exponential;
      } else if (str == "Linear") {
        reliabilityMode = Linear;
      } else {
        reliabilityMode = Linear;
        handler.setStatusString("Reliability mode symbol not recognized:");
        handler.appendStatusString(str.c_str());
        b=false;
      }
      
      b=lti::read(handler, "reliabilityThreshold",reliabilityThreshold) && b;
      b=lti::read(handler, "maxUnreliableNeighborhood",
                  maxUnreliableNeighborhood) && b;

      b=lti::read(handler, "bestBinFirst",bestBinFirst) && b;
      b=lti::read(handler, "eMax",eMax) && b;
      b=lti::read(handler, "bucketSize",bucketSize) && b;

    }

# if !defined(_LTI_MSC_6)
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && supervisedInstanceClassifier::parameters::read(handler,false);
# else
    bool (supervisedInstanceClassifier::parameters::* p_readMS)
      (ioHandler&,const bool) =
      supervisedInstanceClassifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# if defined(_LTI_MSC_6)
  bool kNNClassifier::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // kNNClassifier
  // --------------------------------------------------

  // default constructor
  kNNClassifier::kNNClassifier() : supervisedInstanceClassifier() {

    clear();

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  kNNClassifier::kNNClassifier(const kNNClassifier& other) {
    copy(other);
  }

  // destructor
  kNNClassifier::~kNNClassifier() {
    databaseTree.clear();
  }

  // returns the name of this type
  const char* kNNClassifier::getTypeName() const {
    return "kNNClassifier";
  }

  // copy member
  kNNClassifier& kNNClassifier::copy(const kNNClassifier& other) {
    supervisedInstanceClassifier::copy(other);

    databaseTree=other.databaseTree;
    nClasses=other.nClasses;
    idMap=other.idMap;
    rIdMap=other.rIdMap;
    classWeight=other.classWeight;

    return (*this);
  }

  // alias for copy member
  kNNClassifier& kNNClassifier::operator=(const kNNClassifier& other) {
    return (copy(other));
  }


  // clone member
  classifier* kNNClassifier::clone() const {
    return new kNNClassifier(*this);
  }

  // return parameters
  const kNNClassifier::parameters&
  kNNClassifier::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException();
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The train-methods!
  // -------------------------------------------------------------------

  bool kNNClassifier::train(const dmatrix& input, 
                            const ivector& ids) {
    ivector pointIds(false,ids.size());
    for (int i=0;i<pointIds.size();++i) {
      pointIds.at(i)=i;
    }
    return train(input,ids,pointIds);
  }

  bool kNNClassifier::train(const dmatrix& input, 
                            const ivector& ids,
                            const ivector& pointIds) {
    int i,j,intId;

    if (input.rows() != ids.size()) {
      setStatusString("Incompatible data matrix size and ids vector size");
      return false;
    }

    if (input.rows() != pointIds.size()) {
      setStatusString("Incompatible data matrix size and number of ids for" \
                      " points");
      return false;
    }

    clear();

    // initialize the map attributes of the class
    buildIdMaps(ids);

    if (validProgressObject()) {
      getProgressObject().reset();
      getProgressObject().setTitle("k nearest neighbor training");
      getProgressObject().setMaxSteps(4);
    }

    if (validProgressObject()) {
      getProgressObject().step("Initialize data structure.");
    }

    // compute as weights the number of elements per class
    classWeight.clear();
    classWeight.resize(nClasses,0.0);

    if (validProgressObject()) {
      getProgressObject().step("Add samples to the database.");
    }

    // put each "point" (row of the matrix) as element in the kd-Tree
    for (i=0; i<input.rows(); ++i) {
      intId = idMap[ids[i]];
      // since no id for each point is specified, use just the row
      // index at the input matrix
      databaseTree.add(input[i], std::make_pair(intId,pointIds[i]) );
      classWeight[intId] += 1.0;
    }

    // and adjust the weights
    for (i = 0; i < nClasses; ++i) {
      j = iround(classWeight[i]);
      minPointsPerClass = min(minPointsPerClass,j);
      maxPointsPerClass = max(maxPointsPerClass,j);
      classWeight[i] = 1.0 / classWeight[i];
    }

    if (validProgressObject()) {
      getProgressObject().step("Create database index.");
    }

    build();

    if (validProgressObject()) {
      getProgressObject().step("Training ready.");
    }

    return true;
  }


  /*
   * Adds an object to this classifier. The id is determined automatically
   * and returned in the parameter.
   */
  bool kNNClassifier::trainObject(const dmatrix& input, int& id) {
    ivector pointIds(false,input.rows());
    for (int i=0;i<pointIds.size();++i) {
      pointIds.at(i)=i;
    }
    return trainObject(input,id,pointIds);
  }

  /*
   * Adds an object to this classifier. The id is determined automatically
   * and returned in the argument \a id.
   */
  bool kNNClassifier::trainObject(const dmatrix& input, 
                                  int& id,
                                  const ivector& pointIds) {

    if (pointIds.size() != input.rows()) {
      setStatusString("Incorrect number of point ids for the given input");
      return false;
    }

    // use as external id the maximal id used until now + 1
    // since the std::map is stored as sorted tree, its last element has
    // the biggest key.
    id = (*(idMap.rbegin())).first + 1;
    idMap[id]=nClasses;
    rIdMap[nClasses]=id;

    // insert the "points" (rows of the matrix) as points in the kd-Tree
    int j;
    for (j=0; j<input.rows(); ++j) {
      databaseTree.add(input[j], std::make_pair(nClasses,pointIds.at(j)));
    }
    classWeight.push_back(1.0/double(input.rows()));

    j = input.rows();
    minPointsPerClass = min(minPointsPerClass,j);
    maxPointsPerClass = max(maxPointsPerClass,j);

    nClasses++;

    return true;
  }

  /*
   * Adds an object to this classifier. The id is determined automatically
   * and returned in the parameter.
   */
  bool kNNClassifier::trainObjectId(const dmatrix& input,const int id) {
    ivector pointIds(false,input.rows());
    for (int i=0;i<pointIds.size();++i) {
      pointIds.at(i)=i;
    }
    return trainObjectId(input,id,pointIds);
  }

  /*
   * Adds an object to this classifier. The id is determined automatically
   * and returned in the parameter.
   */
  bool kNNClassifier::trainObjectId(const dmatrix& input,
                                    const int id,
                                    const ivector& pointIds) {

    // internal id in use
    unsigned int intId = nClasses;

    idMap_type::iterator it = idMap.find(id);
    // check if the class id has already been used
    if (it == idMap.end()) {
      // nope!  no such id used previously, this is a new class
      // use as external id the one given
      idMap[id]=intId; // from external to internal
      rIdMap[intId]=id; // from internal to external
      nClasses++;
    } else {
      // recover internal id
      intId = (*it).second;
    }

    // insert the "points" (rows of the matrix) as points in the kd-Tree
    int j;
    for (j=0; j<input.rows(); ++j) {
      databaseTree.add(input[j], std::make_pair(intId,pointIds.at(j)));
    }

    j = input.rows();
    double w = (j!=0) ? 1.0/double(j) : 0.0;

    if (classWeight.size() == intId) {
      classWeight.push_back(w);
    } else if (intId > classWeight.size()) {
      classWeight.resize(intId+1,0.0);
      classWeight[intId]=w;
    } else {
      // the internal id was already in use
      if (classWeight[intId] == 0.0) {
        // not really in use, so just update
        classWeight[intId]=w;
      } else {
        // how many elements belong to the class?
        int n = iround(1.0/classWeight[intId]);
        j+=n;
        classWeight[intId]= (j!=0) ? 1.0/double(j) : 0.0;        
      }
    }

    // the {min,max}PointsPerClass will be updated in the build() method

    return true;
  }

  void kNNClassifier::defineOutputTemplate() {

    ivector tids(rIdMap.size());
    unsigned int i=0;
    idMap_type::const_iterator it;
    for (i=0,it=rIdMap.begin(); i<rIdMap.size(); ++i,++it) {
      tids.at(i)=(*it).second;
    }

    outTemplate=outputTemplate(tids);
  }

  void kNNClassifier::buildIdMaps(const ivector& ids) {
    // remove old maps
    idMap.clear();
    rIdMap.clear();

    // create reverse id map
    int j=0;
    int i=0;
    for (i=0; i<ids.size(); ++i) {
      // id used by elements i already used?
      if (idMap.find(ids.at(i)) == idMap.end()) {
        // no => insert it in both maps
        _lti_debug("Mapping external id " << ids.at(i) << " to " << j 
                   << std::endl);
        rIdMap[j]=ids.at(i);
        idMap[ids.at(i)]=j;
        j++;
      }
    }

    // we know by now the real number of classes being used:
    nClasses=j;
  }



  // -------------------------------------------------------------------
  // The classify-method!
  // -------------------------------------------------------------------

  bool kNNClassifier::classify(const dvector& feature, 
                               outputVector& output) const {
    std::multimap<double,treeType::element*> resList;
    return classify(feature,output,resList);
  }

  bool kNNClassifier::classify(const dmatrix& features, 
                                     outputVector& output) const {

    dvector result(nClasses,0.0);

    const parameters& par = getParameters();

    const int n = features.rows();
    const int newKnn = min(par.kNN,databaseTree.size());

    int i;

    if (!par.useReliabilityMeasure) {  
      // don't use the reliability measure: simple kNN classifier
      std::list<treeType::element*> resList;
      std::list<treeType::element*>::iterator itr;

      for (i = 0; i < n; ++i) {
        // get the k nearest neighbors
        if (par.bestBinFirst) {
          databaseTree.searchBestBinFirst(newKnn,features[i],par.eMax,resList);
        } else {
          databaseTree.searchNearest(newKnn,features[i], resList);          
        }

        if (par.normalizeData) {
          for (itr = resList.begin(); itr != resList.end(); ++itr) {
            // each neighbor votes for its corresponding class (internal id)
            // in the weight obtained as the inverse of the number of samples
            // for that class.
            result[(*itr)->data.first] += classWeight[(*itr)->data.first];
          }
        } else {
          for (itr = resList.begin(); itr != resList.end(); ++itr) {
            // each neighbor votes for its corresponding class.
            result[(*itr)->data.first] += 1.0;
          }
        }
      }
    } else {
      // reliability consideration desired!

      // get the proper reliability function
      double (*reliability)(const double&, const double&) = 0;
      if (par.reliabilityMode == parameters::Exponential) {
        reliability = &exponential;
      } else {
        reliability = &linear;
      }

      std::multimap<double,treeType::element*> resList;
      std::multimap<double,treeType::element*>::iterator itr;

      // number of samples to be retrieved to ensure k elements and that
      // at least 1 element belongs to another class.
      const int newK = min(max(min(maxPointsPerClass+1,
                                   par.maxUnreliableNeighborhood),par.kNN),
                           databaseTree.size());

      double winner;    // distance to the winner sample.
      double nextOther; // distance to the first sample of a different class
                        // than the winner sample.
      bool foundOther;
      int j;

      // use the reliability measure, so kNN is one but not for access!!
      for (i = 0; i < n; ++i) {
        resList.clear(); 
        
        // ensure to get at least 1 element of another class
        if (par.bestBinFirst) {
          databaseTree.searchBestBinFirst(newK,features[i],par.eMax,resList);
        } else {
          databaseTree.searchNearest(newK,features[i], resList);          
        }

        itr = resList.begin();
        const int resultID = (*itr).second->data.first;    // the best hit

        // distance between winner and access point
        winner = ((*itr).first);
        
        // get the next point not belonging to the winner class
        itr++;
        while ((itr!=resList.end()) && 
               ((*itr).second->data.first == resultID)) {
          ++itr;
        }
        
        // this should always be true! except if the classifier has only one
        // single class (which doesn't really make sense):
        foundOther = (itr!=resList.end());
        if (foundOther) {
          nextOther = ((*itr).first);
        }


        if (foundOther) {    

          if (par.normalizeData) {
            for (j=0,itr = resList.begin(); j<newKnn; ++itr,++j) {
              // each neighbor votes for its corresponding class (internal id)
              // in the weight obtained as the inverse of the number of samples
              // for that class.
              if (winner>0.0) {
                result[(*itr).second->data.first] += 
                  classWeight[(*itr).second->data.first] *
                  (*reliability)(nextOther/winner,par.reliabilityThreshold);
              } else {
                result[(*itr).second->data.first] += 
                  classWeight[(*itr).second->data.first];
              }
            }
          } else {
            for (j=0,itr = resList.begin(); j<newKnn; ++itr,++j) {
              // each neighbor votes for its corresponding class.
              if (winner>0.0) {
                result[(*itr).second->data.first] += 
                  (*reliability)(nextOther/winner,par.reliabilityThreshold);
              } else {
                result[(*itr).second->data.first] += 1.0;
              }
            }
          }

        } else {
          // since no other class was found, we can assume an infinite 
          // difference between the classes.
          if (par.normalizeData) {
            for (j=0,itr = resList.begin(); j<newKnn; ++itr,++j) {
              // each neighbor votes for its corresponding class (internal id)
              // in the weight obtained as the inverse of the number of samples
              // for that class.
              result[(*itr).second->data.first] +=
                classWeight[(*itr).second->data.first];
            }
          } else {
            for (j=0,itr = resList.begin(); j<newKnn; ++itr,++j) {
              // each neighbor votes for its corresponding class.
              result[(*itr).second->data.first] += 1.0;
            }
          }
        }
      }
    }

    if (outTemplate.apply(result,output)) {
      output.setWinnerAtMax();
      if (par.normalizeOutput) {
        output.makeProbDistribution();
      } 
      return true;
    }
    return false;
  }

  //
  // Classify many vector separatelly
  //
  bool kNNClassifier::classify(const dmatrix& features, 
                                     dmatrix& result) const {

    const parameters& par = getParameters();

    const int n = features.rows();
    const int newKnn = min(par.kNN,databaseTree.size());

    result.resize(n,nClasses,0.0,false,true);

    int i;

    if (!par.useReliabilityMeasure) {
      // don't use the reliability measure: simple kNN classifier
      std::multimap<treeType::acc_type,treeType::element*> resList;
      std::multimap<treeType::acc_type,treeType::element*>::iterator itr;

      for (i = 0; i < n; ++i) {
        // get the k nearest neighbors
        if (par.bestBinFirst) {
          databaseTree.searchBestBinFirst(newKnn,features[i],par.eMax,resList);
        } else {
          databaseTree.searchNearest(newKnn,features[i], resList);          
        }

        if (par.normalizeData) {
          for (itr = resList.begin(); itr != resList.end(); ++itr) {
            // each neighbor votes for its corresponding class (internal id)
            // in the weight obtained as the inverse of the number of samples
            // for that class.
            result.at(i,(*itr).second->data.first) =
              classWeight[(*itr).second->data.first];
          }
        } else {
          for (itr = resList.begin(); itr != resList.end(); ++itr) {
            // each neighbor votes for its corresponding class.
            result.at(i,(*itr).second->data.first) = 1.0;
          }
        }
      }
    } else {
      // reliability consideration desired!

      // get the proper reliability function
      double (*reliability)(const double&, const double&) = 0;
      if (par.reliabilityMode == parameters::Exponential) {
        reliability = &exponential;
      } else {
        reliability = &linear;
      }

      std::multimap<double,treeType::element*> resList;
      std::multimap<double,treeType::element*>::iterator itr;

      // number of samples to be retrieved to ensure k elements and that
      // at least 1 element belongs to another class.
      const int newK = min(max(min(maxPointsPerClass+1,
                                   par.maxUnreliableNeighborhood),par.kNN),
                           databaseTree.size());

      double winner;    // distance to the winner sample.
      double nextOther; // distance to the first sample of a different class
                        // than the winner sample.
      bool foundOther;
      int j;

      // use the reliability measure, so kNN is one but not for access!!
      for (i = 0; i < n; ++i) {
        resList.clear(); 
        
        // ensure to get at least 1 element of another class
        if (par.bestBinFirst) {
          databaseTree.searchBestBinFirst(newK,features[i],par.eMax,resList);
        } else {
          databaseTree.searchNearest(newK,features[i], resList);          
        }

        itr = resList.begin();
        const int resultID = (*itr).second->data.first;    // the best hit

        // distance between winner and access point
        winner = ((*itr).first);
        
        // get the next point not belonging to the winner class
        itr++;
        while ((itr!=resList.end()) && 
               ((*itr).second->data.first == resultID)) {
          ++itr;
        }
        
        // this should always be true! except if the classifier has only one
        // single class (which doesn't really make sense):
        foundOther = (itr!=resList.end());
        if (foundOther) {
          nextOther = ((*itr).first);
        }


        if (foundOther) {    

          if (par.normalizeData) {
            for (j=0,itr = resList.begin(); j<newKnn; ++itr,++j) {
              // each neighbor votes for its corresponding class (internal id)
              // in the weight obtained as the inverse of the number of samples
              // for that class.
              if (winner>0.0) {
                result.at(i,(*itr).second->data.first) = 
                  classWeight[(*itr).second->data.first] *
                  (*reliability)(nextOther/winner,par.reliabilityThreshold);
              } else {
                result.at(i,(*itr).second->data.first) = 
                  classWeight[(*itr).second->data.first];
              }
            }
          } else {
            for (j=0,itr = resList.begin(); j<newKnn; ++itr,++j) {
              // each neighbor votes for its corresponding class.
              if (winner>0.0) {
                result.at(i,(*itr).second->data.first) = 
                  (*reliability)(nextOther/winner,par.reliabilityThreshold);
              } else {
                result.at(i,(*itr).second->data.first) = 1.0;
              }
            }
          }

        } else {
          // since no other class was found, we can assume an infinite 
          // difference between the classes.
          if (par.normalizeData) {
            for (j=0,itr = resList.begin(); j<newKnn; ++itr,++j) {
              // each neighbor votes for its corresponding class (internal id)
              // in the weight obtained as the inverse of the number of samples
              // for that class.
              result.at(i,(*itr).second->data.first) =
                classWeight[(*itr).second->data.first];
            }
          } else {
            for (j=0,itr = resList.begin(); j<newKnn; ++itr,++j) {
              // each neighbor votes for its corresponding class.
              result.at(i,(*itr).second->data.first) = 1.0;
            }
          }
        }
      }
    }

    return true;
  }


  // -------------------------------------------------------------------------

  kNNClassifier::pointInfo::pointInfo() 
    : point(0),classId(-1),pointId(-1),distance(0.0) {}

  bool kNNClassifier::nearest(const dvector& feature, 
                              pointInfo& nearestPoint) const {
    treeType::element* elem;
    if (databaseTree.searchNearest(feature,elem,nearestPoint.distance)) {

      nearestPoint.point = &(elem->point);
      nearestPoint.classId = elem->data.first;
      nearestPoint.pointId = elem->data.second;
      // fix the square of the distance to be the distance itself
      nearestPoint.distance = sqrt(nearestPoint.distance);

      return true;
    } 

    nearestPoint.point = 0;
    nearestPoint.classId = -1;
    nearestPoint.pointId = -1;
    nearestPoint.distance = std::numeric_limits<double>::max();

    setStatusString("No point found");    
    return false;
  }


  bool kNNClassifier::classify(const dvector& feature, 
                               outputVector& output,
                               std::vector<pointInfo>& points) const {

    std::multimap<double,treeType::element*> resList;
    if (classify(feature,output,resList)) {
      points.clear();
      points.reserve(getParameters().kNN);
      std::multimap<double,treeType::element*>::iterator it;
      pointInfo pinf;
      for (it=resList.begin();it!=resList.end();++it) {
        pinf.point    = &((*it).second->point);
        pinf.classId  = (*it).second->data.first;
        pinf.pointId  = (*it).second->data.second;
        pinf.distance = (*it).first;
        points.push_back(pinf);
      }

      return true;
    }
    return false;
  }

  bool kNNClassifier::classify(const dvector& feature, 
                               outputVector& output,
                     std::multimap<double,treeType::element*>& resList) const {

    _lti_debug("Classifying: " << feature << "\n");
    const parameters& par = getParameters();
    dvector result(nClasses,0.0);
    const int newKnn = min(par.kNN,databaseTree.size());

    resList.clear();
    std::multimap<double,treeType::element*>::iterator itr;
    int classId;

    if (!par.useReliabilityMeasure) {  
      // don't use the reliability measure: simple kNN classifier

      // get the k nearest neighbors
      if (par.bestBinFirst) {
        databaseTree.searchBestBinFirst(newKnn,feature,par.eMax,resList);
      } else {
        databaseTree.searchNearest(newKnn,feature, resList);          
      }
      
      if (par.normalizeData) {
        for (itr = resList.begin(); itr != resList.end(); ++itr) {
          // each neighbor votes for its corresponding class (internal id)
          // in the weight obtained as the inverse of the number of samples
          // for that class.
          classId = (*itr).second->data.first;
          result[classId] += classWeight[classId];
        }
      } else {
        for (itr = resList.begin(); itr != resList.end(); ++itr) {
          // each neighbor votes for its corresponding class.
          result[(*itr).second->data.first] += 1.0;
        }
      }
    } else {
      // reliability consideration desired!

      // get the proper reliability function
      double (*reliability)(const double&, const double&) = 0;
      int i;
      if (par.reliabilityMode == parameters::Exponential) {
        reliability = &exponential;
      } else {
        reliability = &linear;
      }

      // number of samples to be retrieved to ensure k elements and that
      // at least 1 element belongs to another class.
      const int newK = min(max(min(maxPointsPerClass+1,
                                   par.maxUnreliableNeighborhood),par.kNN),
                           databaseTree.size());

      double winner;    // distance to the winner sample.
      double nextOther; // distance to the first sample of a different class
                        // than the winner sample.
      bool foundOther;
      
      // ensure to get at least 1 element of another class
      if (par.bestBinFirst) {
        databaseTree.searchBestBinFirst(newK,feature,par.eMax,resList);
      } else {
        databaseTree.searchNearest(newK,feature, resList);          
      }

      itr = resList.begin();
      const int resultID = (*itr).second->data.first;    // the best hit
      
      // distance between winner and access point
      winner = (*itr).first;
        
      // get the next point not belonging to the winner class
      itr++;
      while ((itr!=resList.end()) && ((*itr).second->data.first == resultID)) {
        ++itr;
      }
      
      // this should always be true! except if the classifier has only one
      // single class (which doesn't really make sense):
      foundOther = (itr!=resList.end());
      if (foundOther) {
        nextOther = (*itr).first;
      }

      if (foundOther) {    
        
        if (par.normalizeData) {
          for (i=0,itr = resList.begin(); i<newKnn; ++itr,++i) {
            // each neighbor votes for its corresponding class (internal id)
            // in the weight obtained as the inverse of the number of samples
            // for that class.
            classId = (*itr).second->data.first;
            if (winner>0.0) {
              result[classId] += classWeight[classId] *
                (*reliability)(nextOther/winner,par.reliabilityThreshold);
            } else {
              result[classId] += classWeight[classId];
            }
          }
        } else {
          for (i=0,itr = resList.begin(); i<newKnn; ++itr,++i) {
            classId = (*itr).second->data.first;
            // each neighbor votes for its corresponding class.
            if (winner>0.0) {
              result[classId] += 
                (*reliability)(nextOther/winner,par.reliabilityThreshold);
            } else {
              result[classId] += 1.0;
            }
          }
        }

      } else {
        // since no other class was found, we can assume an infinite 
        // difference between the classes.
        if (par.normalizeData) {
          for (i=0,itr = resList.begin(); i<newKnn; ++itr,++i) {
            // each neighbor votes for its corresponding class (internal id)
            // in the weight obtained as the inverse of the number of samples
            // for that class.
            classId = (*itr).second->data.first;
            result[classId] += classWeight[classId];
          }
        } else {
          for (i=0,itr = resList.begin(); i<newKnn; ++itr,++i) {
            // each neighbor votes for its corresponding class.
            result[(*itr).second->data.first] += 1.0;
          }
        }
      }      
    }

    if (outTemplate.apply(result,output)) {
      output.setWinnerAtMax();
      if (par.normalizeOutput) {
        output.makeProbDistribution();
      }
      return true;
    }
    return false;

  }

  // -------------------------------------------------------------------------


  void kNNClassifier::clear() {
    databaseTree.clear();
    classWeight.clear();
    nClasses=0;
    idMap.clear();
    rIdMap.clear();

    minPointsPerClass = std::numeric_limits<int>::max();
    maxPointsPerClass = 0;
  }

  // parameter shortcuts

  //
  // IO methods
  //

  /*
   * write the classifier in the given ioHandler
   */
  bool kNNClassifier::write(ioHandler& handler,const bool complete) const {
    bool b=true;
    if (complete) {
      b=handler.writeBegin();
    }
    
    // save the parameters
    b = b && supervisedInstanceClassifier::write(handler,false);
    if (b) {
      // Write data
      b=b && lti::write(handler, "nClasses",nClasses);
      b=b && lti::write(handler, "idMap",idMap);
      b=b && lti::write(handler, "rIdMap",rIdMap);
      b=b && lti::write(handler, "classWeight",classWeight);
      b=b && lti::write(handler, "minPointsPerClass",minPointsPerClass);
      b=b && lti::write(handler, "maxPointsPerClass",maxPointsPerClass);
      b=b && databaseTree.write(handler);
    }

    if (complete) {
      b=handler.writeEnd();
    }
    return b;
  }

  /*
   * read the classifier from the given ioHandler
   */
  bool kNNClassifier::read(ioHandler& handler,const bool complete) {
    bool b=true;
    if (complete) {
      b=handler.readBegin();
    }
    b = b && supervisedInstanceClassifier::read(handler,false);    
    if (b) {     
      b=b && lti::read(handler, "nClasses",nClasses); 
      b=b && lti::read(handler, "idMap",idMap);
      b=b && lti::read(handler, "rIdMap",rIdMap);
      b=b && lti::read(handler, "classWeight",classWeight);      
      b=b && lti::read(handler, "minPointsPerClass",minPointsPerClass);
      b=b && lti::read(handler, "maxPointsPerClass",maxPointsPerClass);

      b=b && databaseTree.read(handler);

      defineOutputTemplate();
    } 
    if (complete) {
      b=b && handler.readEnd();
    }
    
    return b;
  }

  // do this if you add single data sets
  void kNNClassifier::build() {
    // the min number of elements per class can be wrong, so update it
    if (classWeight.size() > 0) {
      minPointsPerClass = maxPointsPerClass = iround(1.0/classWeight[0]);
    }
    for (unsigned int i = 1; i < classWeight.size(); ++i) {
      int j = iround(1.0/classWeight[i]);
      minPointsPerClass = min(minPointsPerClass,j);
      maxPointsPerClass = max(maxPointsPerClass,j);
    }
    
    defineOutputTemplate();
    databaseTree.build(getParameters().bucketSize);
  }

}
