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
 * file .......: ltiColorModelSelector.cpp
 * authors ....: Axel Berner, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 15.2.2002
 * revisions ..: $Id: ltiColorModelSelector.cpp,v 1.13 2006/09/05 10:06:37 ltilib Exp $
 */

#include "ltiGaussDist.h"
#include "ltiProbabilityMap.h"
#include <fstream>
#include <ltiLispStreamHandler.h>

#include "ltiColorModelSelector.h"

namespace lti {
  // --------------------------------------------------
  // colorModelSelector::parameters
  // --------------------------------------------------

  // default constructor
  colorModelSelector::parameters::parameters()
    : transform::parameters() {

    window = rectangle(0,0,10000,10000);
    colorHistFile = "../skin/skin-32-32-32.hist";
    nonColorHistFile = "../skin/nonskin-32-32-32.hist";
    pcaDataPoints = 500000;
  }

  // copy constructor
  colorModelSelector::parameters::parameters(const parameters& other)
    : transform::parameters()  {
    copy(other);
  }

  // destructor
  colorModelSelector::parameters::~parameters() {
  }

  // get type name
  const char* colorModelSelector::parameters::getTypeName() const {
    return "colorModelSelector::parameters";
  }

  // copy member

  colorModelSelector::parameters&
    colorModelSelector::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    transform::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    transform::parameters& (transform::parameters::* p_copy)
      (const transform::parameters&) =
      transform::parameters::copy;
    (this->*p_copy)(other);
# endif

    window = other.window;
    colorHistFile = other.colorHistFile;
    nonColorHistFile = other.nonColorHistFile;
    pcaDataPoints = other.pcaDataPoints;
    mapParameters.copy(other.mapParameters);

    return *this;
  }

  // alias for copy member
  colorModelSelector::parameters&
    colorModelSelector::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* colorModelSelector::parameters::clone() const {
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
  bool colorModelSelector::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool colorModelSelector::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"window",window);
      lti::write(handler,"colorHistFile",colorHistFile);
      lti::write(handler,"nonColorHistFile",nonColorHistFile);
      lti::write(handler,"pcaDataPoints",pcaDataPoints);
      lti::write(handler,"mapParameters",mapParameters);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::write(handler,false);
# else
    bool (transform::parameters::* p_writeMS)(ioHandler&,const bool) const =
      transform::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool colorModelSelector::parameters::write(ioHandler& handler,
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
  bool colorModelSelector::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool colorModelSelector::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"window",window);
      lti::read(handler,"colorHistFile",colorHistFile);
      lti::read(handler,"nonColorHistFile",nonColorHistFile);
      lti::read(handler,"pcaDataPoints",pcaDataPoints);
      lti::read(handler,"mapParameters",mapParameters);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::read(handler,false);
# else
    bool (transform::parameters::* p_readMS)(ioHandler&,const bool) =
      transform::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool colorModelSelector::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // colorModelSelector
  // --------------------------------------------------

  // default constructor
  colorModelSelector::colorModelSelector()
    : transform(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  colorModelSelector::colorModelSelector(const colorModelSelector& other)
    : transform()  {
    copy(other);
  }

  // destructor
  colorModelSelector::~colorModelSelector() {
  }

  // returns the name of this type
  const char* colorModelSelector::getTypeName() const {
    return "colorModelSelector";
  }

  // copy member
  colorModelSelector&
  colorModelSelector::copy(const colorModelSelector& other) {
    transform::copy(other);
    modelList.clear();
    std::list<namedModel>::const_iterator it = other.modelList.begin();
    while(it != other.modelList.end()) {
      modelList.push_back(*it);
      it++;
    }
    return (*this);
  };

  // alias for copy member
  colorModelSelector&
  colorModelSelector::operator=(const colorModelSelector& other) {
    return (copy(other));
  };

  bool colorModelSelector::read(ioHandler& handler,
			  const bool complete) {
    bool b = true;
    if (complete)
      b = handler.readBegin();

    parameters tmpParam;

    b = b && lti::read(handler,"parameters",tmpParam);
    setParameters(tmpParam);

    int size;
    b = b && handler.read("size",size);

    std::string str;

    int level = handler.getLevel();

    b = b && handler.readBegin();
    b = b && handler.readSymbol(str);

    if (str == "data") {
      b = b && handler.readKeyValueSeparator();
      b = b && handler.readBegin();
      modelList.clear();
      if (size > 0) {
        namedModel tmp;
        for (int i=0;i<size-1;++i) {
          b = b && tmp.read(handler);
          modelList.push_back(tmp);
          b = b && handler.readKeyValueSeparator();
        }
        b = b && tmp.read(handler);
        modelList.push_back(tmp);
      }
    }

    // read all the next end tokens... and ensure consistency
    while (handler.readEnd() && (handler.getLevel() > level));

    if (complete) {
      b = b && handler.readEnd();
    }
    return b;
  };

  bool colorModelSelector::write(lti::ioHandler& handler,
			   const bool complete) const {
    bool b = true;
    if (complete)
      b = handler.writeBegin();

    b = b && lti::write(handler,"parameters",getParameters());

    const int size = modelList.size();
    b = b && handler.write("size",size);

    b = b && handler.writeBegin();
    b = b && handler.writeSymbol(std::string("data"));
    b = b && handler.writeKeyValueSeparator();
    b = b && handler.writeBegin();
    if (size > 0) {
      // begin from one to ensure that the last element of the container
      // will NOT be written yet!
      int i;
      std::list<namedModel>::const_iterator it;
      for(i=1,it=modelList.begin();i<size;++it,++i) {
        b = b && (*it).write(handler);
        b = b && handler.writeDataSeparator();
      }
      b = b && (*it).write(handler);
    }
    b = b && handler.writeEnd();
    b = b && handler.writeEnd();

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  };

  // clone member
  functor* colorModelSelector::clone() const {
    return new colorModelSelector(*this);
  };

  // return parameters
  const colorModelSelector::parameters&
    colorModelSelector::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool
  colorModelSelector::addColorModel(const std::string& name,
                                    const std::string& colorHistFileName,
                                    const std::string& nonColorHistFileName) {
    // get color histogram
    thistogram<double> colorHist;
    if(colorHistFileName.empty()) { // get default color model
      std::ifstream in(getParameters().colorHistFile.c_str());
      if (in) {
        lispStreamHandler lsh(in);
        if (!colorHist.read(lsh)) {
          std::string info("Wrong default File: "
                           +getParameters().colorHistFile);
          setStatusString(info.c_str());
          return false;
        }
      } else {
        std::string info("Default File: "
                         +getParameters().colorHistFile+" not found");
        setStatusString(info.c_str());
        return false;
      }
    } else { //read from colorHistFile
      std::ifstream in(colorHistFileName.c_str());
      if (in) {
        lispStreamHandler lsh(in);
        if (!colorHist.read(lsh)) {
          std::string info("Wrong file: " + colorHistFileName);
          setStatusString(info.c_str());
          return false;
        }
      } else {
        std::string info("File: "+colorHistFileName+" not found");
        setStatusString(info.c_str());
        return false;
      }
    }

    // get noncolor histogram
    thistogram<double> nonColorHist;
    if(nonColorHistFileName.empty()) { // get default nonColor model
      std::ifstream in(getParameters().nonColorHistFile.c_str());
      if (in) {
        lispStreamHandler lsh(in);
        if (!nonColorHist.read(lsh)) {
          std::string info("Wrong default File: "
                           +getParameters().nonColorHistFile);
          setStatusString(info.c_str());
          return false;
        }
      } else {
        std::string info("Default File: "
                         +getParameters().nonColorHistFile+" not found");
        setStatusString(info.c_str());
        return false;
      }
    } else { //read from nonColorHistFile
      std::ifstream in(nonColorHistFileName.c_str());
      if (in) {
        lispStreamHandler lsh(in);
        if (!nonColorHist.read(lsh)) {
          std::string info("Wrong file: " + nonColorHistFileName);
          setStatusString(info.c_str());
          return false;
        }
      } else {
        std::string info("File: "+nonColorHistFileName+" not found");
        setStatusString(info.c_str());
        return false;
      }
    }
    return addColorModel(name,colorHist,nonColorHist);
  };

  bool
  colorModelSelector::addColorModel(const std::string& name,
                                    const thistogram<double>& colorHist,
                                    const thistogram<double>& nonColorHist) {

    probabilityMap::parameters colorMapPar(getParameters().mapParameters);
    colorMapPar.setObjectColorModel(colorHist);
    colorMapPar.setNonObjectColorModel(nonColorHist);

    probabilityMap colorMap;
    colorMap.setParameters(colorMapPar);

    return addColorModel(name,colorMap);
  };

  bool colorModelSelector::addColorModel(const std::string& name,
                                         const probabilityMap& colorMap) {

    namedModel tmpModel;
    modelList.push_back(tmpModel);

    modelList.back().name = name;
    modelList.back().setColorMap(colorMap);

    return true;
  };

  bool
  colorModelSelector::updateModels(const probabilityMap::parameters& nAtt) {
    std::list<namedModel>::iterator it;
    for (it=modelList.begin();it!=modelList.end();++it) {
      if (!(*it).getColorMap().setParametersKeepingHistograms(nAtt)) {
        setStatusString("Wrong parameters in internal model");
        return false;
      }
    }

    return true;
  }

  bool colorModelSelector::clearColorModels() {
    modelList.clear();
    return modelList.empty();
  };

  bool colorModelSelector::removeColorModel(const std::string& theName){
    bool nameFound = false;
    std::list<namedModel>::iterator it = modelList.begin();
    while(it != modelList.end()) {
      if((*it).name == theName) {
	it = modelList.erase(it);
	nameFound = true;
      } else {
	it++;
      }
    }
    return nameFound;
  };

  bool colorModelSelector::initPCA() {
    std::list<namedModel>::iterator it = modelList.begin();
    bool b = true;
    while(it != modelList.end()) {
      if (!(*it).pcaInitialized()) {
        b = (*it).computePCA() && b;
      }
      it++;
    }
    return b;

  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool colorModelSelector::apply(const image& img,
                                 const probabilityMap* &skinMap,
                                 const principalComponents<float>* &pca) {
    return apply(img,getParameters().window,skinMap,pca);
  }

  bool colorModelSelector::apply(const image& img,
                                 const rectangle& window,
                                 const probabilityMap* &skinMap,
                                 const principalComponents<float>* &pca) {
    skinMap = 0;
    pca = 0;
    if(modelList.empty()) {
      setStatusString("No skin model available. Add one or more " \
                      " with addSkinModel(..)");
      return false;
    }

    // calculate for all models, one by one the skin probability map
    // and choose the best suitable (highest accumulated skin probability)
    std::list<namedModel>::iterator it = modelList.begin();
    image cutImg;
    cutImg.copy(img,window);
    channel skinProb;
    double prob,maxProb=0.0;
    while(it != modelList.end()) {
      (*it).getColorMap().apply(cutImg,skinProb);
      prob = skinProb.sumOfElements(); //accumulate all prob's
      if (prob > maxProb) {
        maxProb = prob;     // keep the highest
	skinMap = &((*it).getColorMap());

        if (!(*it).pcaInitialized()) {
          (*it).computePCA();
        }

	pca = &((*it).getPCA());
      }
      it++;
    }
    return true;
  };

  bool colorModelSelector::apply(const image& img,
                                 probabilityMap& skinMap,
                                 principalComponents<float>& pca) {

    const probabilityMap* bestSkinMap = 0;
    const principalComponents<float>* bestPca = 0;

    if (apply(img,getParameters().window,bestSkinMap,bestPca) ) {
      skinMap.copy(*bestSkinMap);
      pca.copy(*bestPca);
      return true;
    }

    return false;
  };

  bool colorModelSelector::apply(const image& img,
                                 const rectangle& window,
                                 probabilityMap& skinMap,
                                 principalComponents<float>& pca)  {

    const probabilityMap* bestSkinMap = 0;
    const principalComponents<float>* bestPca = 0;
    if (apply(img,window,bestSkinMap,bestPca) ) {
      skinMap.copy(*bestSkinMap);
      pca.copy(*bestPca);
      return true;
    }

    return false;
  };

  bool colorModelSelector::apply(const image& img,
                                 const probabilityMap* &skinMap) const {
    return apply(img,getParameters().window,skinMap);
  }

  bool colorModelSelector::apply(const image& img,
                                 const probabilityMap* &skinMap,
                                 std::string& modelName) const {
    return apply(img,getParameters().window,skinMap,modelName);
  }


  bool colorModelSelector::apply(const image& img,
                                 const rectangle& window,
                                 const probabilityMap* &pColorMap) const {
    std::string tmp;
    return apply(img,window,pColorMap,tmp);
  }

  bool colorModelSelector::apply(const image& img,
                                 const rectangle& window,
                                 const probabilityMap* &pColorMap,
                                 std::string& modelName) const {
    pColorMap = 0;
    if(modelList.empty()) {
      setStatusString("No color model available. Add one or more: addColorModel(..)");
      return false;
    }

    // calculate for all models, one by one the color probability map
    // and choose the best suitable (highest accumulated color probability)
    std::list<namedModel>::const_iterator it = modelList.begin();
    std::list<namedModel>::const_iterator itMax = modelList.end();
    image cutImg;
    cutImg.copy(img,window);
    channel colorProb;
    double prob,maxProb=0.0;
    while(it != modelList.end()) {
      (*it).getColorMap().apply(cutImg,colorProb);
      prob = colorProb.sumOfElements(); //accumulate all prob's
      if (prob > maxProb) {
        itMax = it;
        maxProb = prob;     // keep the highest
      }
      it++;
    }

    if (itMax != modelList.end()) {
      modelName = (*itMax).name;
      pColorMap = &((*itMax).getColorMap());
    }
    return true;
  };

  bool colorModelSelector::apply(const image& img,
                                 const rectangle& window,
                                 const probabilityMap* &skinMap,
                                 const principalComponents<float>* &pca,
                                 std::string& modelName) {
    skinMap = 0;
    pca = 0;
    modelName = "";
    if(modelList.empty()) {
      setStatusString("No skin model available. Add one or more " \
                      " with addSkinModel(..)");
      return false;
    }

    // calculate for all models, one by one the skin probability map
    // and choose the best suitable (highest accumulated skin probability)
    std::list<namedModel>::iterator it = modelList.begin();
    std::list<namedModel>::iterator itMax = modelList.end();

    image cutImg;
    cutImg.copy(img,window);
    channel skinProb;
    double prob,maxProb=0.0;
    while(it != modelList.end()) {
      (*it).getColorMap().apply(cutImg,skinProb);
      prob = skinProb.sumOfElements(); //accumulate all prob's
      if (prob > maxProb) {
        maxProb = prob;     // keep the highest
        itMax = it;
      }
      it++;
    }

    if (itMax != modelList.end()) {
      modelName = (*itMax).name;
      skinMap = &((*itMax).getColorMap());

      if (!(*itMax).pcaInitialized()) {
        (*itMax).computePCA();
      }

      pca = &((*itMax).getPCA());
    }

    return true;
  };


  bool colorModelSelector::apply(const image& img,
                                 probabilityMap& skinMap) const {

    const probabilityMap* bestSkinMap = 0;

    if (apply(img,getParameters().window,bestSkinMap) ) {
      skinMap.copy(*bestSkinMap);
      return true;
    }

    return false;
  };

  bool colorModelSelector::apply(const image& img,
                                 const rectangle& window,
                                 probabilityMap& skinMap) const {

    const probabilityMap* bestSkinMap = 0;
    if (apply(img,window,bestSkinMap) ) {
      skinMap.copy(*bestSkinMap);
      return true;
    }

    return false;
  };


  // -----------------
  // -   namedModel  -
  // -----------------

  colorModelSelector::namedModel::namedModel() {
    name = "";
    pcaComputed = false;
  }

  bool colorModelSelector::namedModel::pcaInitialized() const {
    return pcaComputed;
  }

  bool colorModelSelector::namedModel::computePCA() {
    // check, among other things, that the color model is three-dimensional
    if (!colorMap.getParameters().isObjectColorModelValid()) {
      return false;
    }

    // first create a list of vectors for training
    const thistogram<double>& skinHisto
      = colorMap.getParameters().getObjectColorModel();

    const int cells0 = skinHisto.getLastCell()[0];
    const int cells1 = skinHisto.getLastCell()[1];
    const int cells2 = skinHisto.getLastCell()[2];

    const double n = skinHisto.getNumberOfEntries();

    const float cellWidthH0 = 1.0f/cells0;
    const float cellWidthH1 = 1.0f/cells1;
    const float cellWidthH2 = 1.0f/cells2;

    matrix<double> cov(3,3,0.0f);
    vector<double> mean(3,0.0f);
    vector<double> data(3);
    double r,g,b,mr,mg,mb;
    double m;
    ivector idx(3,0);

    // compute covariance and mean from histogram

    for (idx.at(0)=0;     idx.at(0)<=cells0; ++idx.at(0)) {
      r = cellWidthH0*(idx.at(0) + 0.5);
      for (idx.at(1)=0;   idx.at(1)<=cells1; ++idx.at(1)) {
        g = cellWidthH1*(idx.at(1) + 0.5);

	for (idx.at(2)=0; idx.at(2)<=cells2; ++idx.at(2)) {        
          b = cellWidthH2*(idx.at(2) + 0.5);
 
          m = skinHisto.at(idx)/n;

          mean.at(0) += (mr = m*r);
          mean.at(1) += (mg = m*g);
          mean.at(2) += (mb = m*b);

          cov.at(0,0) += mr*r; cov.at(0,1) += mr*g; cov.at(0,2) += mr*b;
          cov.at(1,0) += mg*r; cov.at(1,1) += mg*g; cov.at(1,2) += mg*b;
          cov.at(2,0) += mb*r; cov.at(2,1) += mb*g; cov.at(2,2) += mb*b;
        }
      }
    }

    r = mean.at(0);
    g = mean.at(1);
    b = mean.at(2);

    // compute the covariance as E(xx') - mean mean'
    cov.at(0,0) -= r*r; cov.at(0,1) -= r*g; cov.at(0,2) -= r*b;
    cov.at(1,0) -= g*r; cov.at(1,1) -= g*g; cov.at(1,2) -= g*b;
    cov.at(2,0) -= b*r; cov.at(2,1) -= b*g; cov.at(2,2) -= b*b;

    // now, indicate the pca functor to use the computed mean and covariance
    // now, we can do the whitening transformation
    principalComponents<float>::parameters pcaParam;

    pcaParam.resultDim = 3;
    pcaParam.autoDim = false;
    pcaParam.useCorrelation = false;
    pcaParam.whitening = true;
    pca.setParameters(pcaParam);

    matrix<float> fCovar;
    vector<float> fMean;
    fCovar.castFrom(cov);
    fMean.castFrom(mean);

    pca.setCovarianceAndMean(fCovar,fMean);  // prepare trans-matrix

    pcaComputed = true;

    return true;

  }

  void
  colorModelSelector::namedModel::setColorMap(const probabilityMap& colMap) {
    pcaComputed = false;
    colorMap.copy(colMap);
  }

  const probabilityMap& colorModelSelector::namedModel::getColorMap() const {
    return colorMap;
  }

  probabilityMap& colorModelSelector::namedModel::getColorMap() {
    return colorMap;
  }

  principalComponents<float>&
  colorModelSelector::namedModel::getPCA() {
    return pca;
  }

  bool colorModelSelector::namedModel::read(ioHandler& handler,
                                            const bool complete) {
    bool b = true;
    if (complete)
      b = handler.readBegin();

    b = b && lti::read(handler,"name",name);
    b = b && lti::read(handler,"colorMap",colorMap);
    b = b && lti::read(handler,"pca",pca);
    b = b && lti::read(handler,"pcaComputed",pcaComputed);

    if (complete)
      b = b && handler.readEnd();

    return b;
  };

  bool colorModelSelector::namedModel::write(lti::ioHandler& handler,
                                       const bool complete) const {
    bool b = true;
    if (complete)
      b = handler.writeBegin();

    b = b && lti::write(handler,"name",name);
    b = b && lti::write(handler,"colorMap",colorMap);
    b = b && lti::write(handler,"pca",pca);
    b = b && lti::write(handler,"pcaComputed",pcaComputed);

    if (complete)
      b = b && handler.writeEnd();

    return b;
  };

  colorModelSelector::namedModel&
  colorModelSelector::namedModel::copy(const namedModel& other) {
    name = other.name;
    colorMap = other.colorMap;
    pca = other.pca;
    pcaComputed = other.pcaComputed;
    return *this;
  }

  colorModelSelector::namedModel&
  colorModelSelector::namedModel::operator=(const namedModel& other) {
    return copy(other);
  }



}
