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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiHmmClassifier.cpp
 * authors ....: Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 12.04.2002
 * revisions ..: $Id: ltiHmmClassifier.cpp,v 1.12 2006/09/05 09:58:30 ltilib Exp $
 */

#include <cstdio>
#include <limits>
#include "ltiObject.h"
#include "ltiHmmClassifier.h"

namespace lti {

  // --------------------------------------------------
  // hmmClassifier::parameters
  // --------------------------------------------------

  // default constructor
  hmmClassifier::parameters::parameters() 
    : classifier::parameters() {
    hmmTrainingParameters = hmmTrainer::parameters();
    defaultModel = hiddenMarkovModel();
    mappingFunction = exponential;
  }

  // copy constructor
  hmmClassifier::parameters::parameters(const parameters& other)
    : classifier::parameters() {
    copy(other);
  }

  // destructor
  hmmClassifier::parameters::~parameters() {
  }

  // get type name
  const char* hmmClassifier::parameters::getTypeName() const {
    return "hmmClassifier::parameters";
  }

  // copy member
  hmmClassifier::parameters&
    hmmClassifier::parameters::copy(const parameters& other) {

# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    supervisedSequenceClassifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    supervisedSequenceClassifier::parameters& (supervisedSequenceClassifier::parameters::* p_copy)
      (const supervisedSequenceClassifier::parameters&) =
      supervisedSequenceClassifier::parameters::copy;
    (this->*p_copy)(other);
# endif


      hmmTrainingParameters = other.hmmTrainingParameters;
      defaultModel = other.defaultModel;
      mappingFunction = other.mappingFunction;

    return *this;
  }

  // alias for copy member
  hmmClassifier::parameters&
    hmmClassifier::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  classifier::parameters* hmmClassifier::parameters::clone() const {
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
  bool hmmClassifier::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool hmmClassifier::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler, "hmmTrainingParameters", hmmTrainingParameters);
      lti::write(handler, "defaultModel", defaultModel);

      switch(mappingFunction) {
      case exponential:
        lti::write(handler,"mappingFunction","exponential");
        break;
      case linear:
        lti::write(handler,"mappingFunction","linear");
        break;
      case none:
        lti::write(handler,"mappingFunction","none");
        break;
      default:
        b = false;
        lti::write(handler,"mappingFunction","exponential");
      }

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && supervisedSequenceClassifier::parameters::write(handler,false);
# else
    bool (supervisedSequenceClassifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      supervisedSequenceClassifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool hmmClassifier::parameters::write(ioHandler& handler,
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
  bool hmmClassifier::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool hmmClassifier::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler, "hmmTrainingParameters", hmmTrainingParameters);
      lti::read(handler, "defaultModel", defaultModel);

      std::string str;

      lti::read(handler, "mappingFunction", str);
      if (str=="exponential") {
        mappingFunction = exponential;
      } else if (str=="linear") {
        mappingFunction = linear;
      } else if (str=="none") {
        mappingFunction = none;
      } else {
//        b=false;
        mappingFunction=exponential;
      }

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && supervisedSequenceClassifier::parameters::read(handler,false);
# else
    bool (supervisedSequenceClassifier::parameters::* p_readMS)(ioHandler&,const bool) =
      supervisedSequenceClassifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool hmmClassifier::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif



  // --------------------------------------------------
  // hmmClassifier
  // --------------------------------------------------


  // constructor
  hmmClassifier::hmmClassifier()
    : supervisedSequenceClassifier() {

    parameters theParameters;
    setParameters(theParameters);

    theModels.clear();
    featureDimension = 0;
  }

  // copy constructor
  hmmClassifier::hmmClassifier(const hmmClassifier& other)
    : supervisedSequenceClassifier(other)  {
    copy(other);
  }

  // destructor
  hmmClassifier::~hmmClassifier() {
  }

  // returns the name of this type
  const char* hmmClassifier::getTypeName() const {
    return "hmmClassifier";
  }

  int hmmClassifier::getShortestModelLength() const {
    int shortest = -1;
    int i;
    std::map<int, hiddenMarkovModel>::const_iterator it = theModels.begin();
    while (it != theModels.end()) {
      i = it->second.getNumberOfStates();
      if (shortest < 0 || shortest > i)
        shortest = i;
      ++it;
    }
    return shortest;
  }

  // copy member
  hmmClassifier&
    hmmClassifier::copy(const hmmClassifier& other) {

    supervisedSequenceClassifier::copy(other);
    return (*this);
  }

  // alias for copy member
  hmmClassifier&
    hmmClassifier::operator=(const hmmClassifier& other) {
    return (copy(other));
  }

  // return pointer to copy of classifier
  classifier* hmmClassifier::clone() const {
    return new hmmClassifier(*this);
  }

  // return parameters
  const hmmClassifier::parameters&
  hmmClassifier::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }


  // write method
  bool hmmClassifier::write(ioHandler &handler, const bool complete) const {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    lti::write(handler, "featureDimension", featureDimension);
    lti::write(handler, "theModels", theModels);

    b = b && supervisedSequenceClassifier::write(handler,false);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

  // read method
  bool hmmClassifier::read(ioHandler &handler, const bool complete) {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    lti::read(handler, "featureDimension", featureDimension);
    lti::read(handler, "theModels", theModels);

    b = b && supervisedSequenceClassifier::read(handler,false);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }


  //
  // hmmClassifier training and classification methods:
  //


  // query model ids
  ivector hmmClassifier::getIDs() {
    std::map<int, hiddenMarkovModel>::iterator it = theModels.begin();

    ivector modelIDs(theModels.size());
    for (int i=0; i < modelIDs.size(); ++i, ++it)
      modelIDs[i] = (*it).first;

    return modelIDs;
  }


  // delete all models
  void hmmClassifier::reset() {
    theModels.clear();
    featureDimension = 0;

    // clear output template
    outputTemplate theOutputTemplate;
    setOutputTemplate(theOutputTemplate);
  }



  // get single model
  hiddenMarkovModel hmmClassifier::getHMM(int id) const {
    std::map<int, hiddenMarkovModel>::const_iterator it = theModels.find(id);

    // no such model found
    if (it == theModels.end())
      return getParameters().defaultModel;

    return (*it).second;
  }



  // set single model
  bool hmmClassifier::setHMM(hiddenMarkovModel &model, int id) {
    if (featureDimension == 0)
      featureDimension = model.featureDimension;

    // catch wrong feature dimension
    if (model.featureDimension != featureDimension) {
      setStatusString("feature dimensions don't match");
      return false;
    }

    // catch 0-state model
    if (model.states.size() == 0) {
      setStatusString("empty model");
      return false;
    }

    // copy model (create, if appropriate)
    theModels[id] = model;

    return true;
  }



  // delete single model
  bool hmmClassifier::deleteHMM(int id) {
    std::map<int, hiddenMarkovModel>::iterator it = theModels.find(id);

    // no such model found
    if (it == theModels.end()) {
      setStatusString("can't find model");
      return false;
    }

    theModels.erase(it);

    if (theModels.size() == 0)
      featureDimension = 0;

    return true;
  }


  // set the name of a single model
  void hmmClassifier::setName(int id, const std::string &theName) {
    std::map<int, hiddenMarkovModel>::iterator it = theModels.find(id);

    if (!(it == theModels.end()))
      (*it).second.name = theName;
  }


  // generate map index -> names
  void hmmClassifier::generateIndexNameMap(std::map<int, std::string> &theMap) const {
    theMap.clear();

    std::map<int, hiddenMarkovModel>::const_iterator it;

    for (it = theModels.begin(); !(it == theModels.end()); ++it)
      theMap[(*it).first] = (*it).second.name;
  }


  // train a single model
  bool hmmClassifier::train(std::vector< sequence<dvector> > &input, int id) {
    // prepare training
    hmmTrainer theTrainer;
    hmmTrainer::parameters hmmTrainingParameters = getParameters().hmmTrainingParameters;
    theTrainer.setParameters(hmmTrainingParameters);
    hiddenMarkovModel theModel = getParameters().defaultModel;

    // train model
    theTrainer.consider(input);
    theTrainer.apply(theModel);

    // set model
    return setHMM(theModel, id);
  }


  // classify a sequence with limited models - models given by name and ID
  //with reduced theValues
	bool hmmClassifier::reducedClassify(const sequence<dvector>& observations, outputVector& result, std::map<int, std::string> activeWordsMap) {
    hmmViterbiPathSearch theViterbiPathSearch;

	//build reduced list of theModels = selectedModels
	std::map<int, hiddenMarkovModel> selectedModels;
  int modelNr = 0;
  std::map<int, hiddenMarkovModel>::const_iterator it;
  std::map<int, std::string>::const_iterator testIt;
  
	for (it = theModels.begin(); !(it == theModels.end()); ++it) {
			testIt = activeWordsMap.find(it->first);
			if (testIt!=activeWordsMap.end())
			{
				selectedModels.insert(std::make_pair((*it).first, (*it).second));
			}
		}

    ivector theIDs(selectedModels.size());
    dvector theValues(selectedModels.size());

    // save extreme scores for linear mapping
    double lowestScore=std::numeric_limits<double>::max();
    double highestScore=std::numeric_limits<double>::min();

    // no internal models present
    if (theModels.size() == 0) {
      setStatusString("classifier is untrained");
      return false;
    }

    modelNr = 0;

    // evaluate training data on all models
		for (it = selectedModels.begin(); !(it == selectedModels.end()); ++it, ++modelNr) {
				theIDs[modelNr] = (*it).first;
				theValues[modelNr] = theViterbiPathSearch.apply((*it).second, observations);

				// save best (=lowest) and worst (=highest) recognition score
				if (modelNr == 0 || theValues[modelNr] < lowestScore)
					lowestScore = theValues[modelNr];
				if (modelNr == 0 || theValues[modelNr] > highestScore)
					highestScore = theValues[modelNr];
			}


    float scoreDiff = static_cast<float>(highestScore - lowestScore);

    const parameters& p = getParameters();

    switch (p.mappingFunction) {
    case parameters::exponential:
      
      // map scores to [0...1], so larger values represent better recognition results (=lower scores)
      // step 1: scale [highest...lowest] to (0..1]
      for (modelNr = 0; modelNr < theValues.size(); ++modelNr)
        theValues[modelNr] = 1.0f / (theValues[modelNr] - lowestScore + 1.0f);

      if (scoreDiff != 0.0f) {
        // step 2: scale (0..1] to [0..1]
        for (modelNr = 0; modelNr < theValues.size(); ++modelNr) {
          theValues[modelNr] -= 1.0f / (scoreDiff + 1.0f);
          theValues[modelNr] *= (scoreDiff + 1.0f) / scoreDiff;
        }
      }
    
      break;

    case parameters::linear:

      // map scores linear to [0...1], so larger values represent better recognition results (=lower scores)
      for (modelNr = 0; modelNr < theValues.size(); ++modelNr)
        theValues[modelNr] = (highestScore-theValues[modelNr]) / scoreDiff;

      break;

    case parameters::none:

      // do not map scores
      break;
    }

		lti::classifier::outputTemplate reducedOutpTempl;
		reducedOutpTempl.setIds(theIDs);
		return reducedOutpTempl.apply(theValues, result);
	}

  // classify a sequence
  bool hmmClassifier::classify(const sequence<dvector>& observations, outputVector& result) {
    hmmViterbiPathSearch theViterbiPathSearch;

    ivector theIDs(theModels.size());
    dvector theValues(theModels.size());

    // save extreme scores for linear mapping
    double lowestScore=std::numeric_limits<double>::max();
    double highestScore=std::numeric_limits<double>::min();

    // no internal models present
    if (theModels.size() == 0) {
      setStatusString("classifier is untrained");
      return false;
    }

    int modelNr = 0;
    std::map<int, hiddenMarkovModel>::const_iterator it;

    // evaluate training data on all models
    for (it = theModels.begin(); !(it == theModels.end()); ++it, ++modelNr) {

      theIDs[modelNr] = (*it).first;
      theValues[modelNr] = theViterbiPathSearch.apply((*it).second, observations);

      // save best (=lowest) and worst (=highest) recognition score
      if (modelNr == 0 || theValues[modelNr] < lowestScore)
        lowestScore = theValues[modelNr];
      if (modelNr == 0 || theValues[modelNr] > highestScore)
        highestScore = theValues[modelNr];
    }

    float scoreDiff = static_cast<float>(highestScore - lowestScore);

    const parameters& p = getParameters();

    switch (p.mappingFunction) {
    case parameters::exponential:
      
      // map scores to [0...1], so larger values represent better recognition results (=lower scores)
      // step 1: scale [highest...lowest] to (0..1]
      for (modelNr = 0; modelNr < theValues.size(); ++modelNr)
        theValues[modelNr] = 1.0f / (theValues[modelNr] - lowestScore + 1.0f);

      if (scoreDiff != 0.0f) {
        // step 2: scale (0..1] to [0..1]
        for (modelNr = 0; modelNr < theValues.size(); ++modelNr) {
          theValues[modelNr] -= 1.0f / (scoreDiff + 1.0f);
          theValues[modelNr] *= (scoreDiff + 1.0f) / scoreDiff;
        }
      }
    
      break;

    case parameters::linear:

      // map scores linear to [0...1], so larger values represent better recognition results (=lower scores)
      for (modelNr = 0; modelNr < theValues.size(); ++modelNr)
        theValues[modelNr] = (highestScore-theValues[modelNr]) / scoreDiff;

      break;

    case parameters::none:

      // do not map scores
      break;
    }

    // apply outputTemplate on results
    return outTemplate.apply(theValues, result);
  }



  // train all ids, create output template
  bool hmmClassifier::train(const std::vector< sequence<dvector> > &input, const ivector& ids) {

    // observation size and id vector size don't match
    if (static_cast<int>(input.size()) != ids.size()) {
      setStatusString("size of input and ids don't match");
      return false;
    }

    // track the status of the training process.
    bool b=true;

    // clear all models first
    reset();

    // prepare training
    hmmTrainer theTrainer;
    hmmTrainer::parameters hmmTrainingParameters = getParameters().hmmTrainingParameters;
    theTrainer.setParameters(hmmTrainingParameters);

    int numberOfIDs = 0;

    if (validProgressObject()) {
      getProgressObject().reset();
      getProgressObject().setTitle("Training models");
      getProgressObject().setMaxSteps(input.size());
    }

    // iterate on all training sequences
    for (unsigned int seq = 0; seq < input.size(); ++seq) {

      int currentID = ids[seq];

      // create new model if necessary
      if (theModels.find(currentID) == theModels.end()) {
        ++numberOfIDs;

        // collect all sequences of the current id
        for (unsigned int actSeq = seq; actSeq < input.size(); ++actSeq) {
          if (ids[actSeq] == currentID) {
            theTrainer.consider(input[actSeq]);
            if (validProgressObject()) {
              char buffer[40];
              sprintf(buffer,"Model %d",currentID);
              std::string tmp=buffer;
              getProgressObject().step(tmp);
            }
          }
        }

        // train model
        hiddenMarkovModel theModel = getParameters().defaultModel;
        theTrainer.apply(theModel);

        // set model
        b = b && setHMM(theModel, currentID);

        // clear training sequences
        theTrainer.reset();
      }
    }

    ivector theIDs(theModels.size());
    std::map<int, hiddenMarkovModel>::const_iterator it = theModels.begin();

    // collect ids of all models
    for (unsigned int modelNr = 0; modelNr < theModels.size(); ++it, ++modelNr)
      theIDs[modelNr] = (*it).first;

    outTemplate = outputTemplate(theIDs);
//    outTemplate.setIds(theIDs);

    return b;
  }



  // write function for simplified use
  bool write(ioHandler &handler, const hmmClassifier::parameters &p, const bool complete) {
    return p.write(handler, complete);
  }

  // read function for simplified use
  bool read(ioHandler &handler, hmmClassifier::parameters &p, const bool complete) {
    return p.read(handler, complete);
  }


  // write function for simplified use
  bool write(ioHandler &handler, const hmmClassifier &p, const bool complete) {
    return p.write(handler, complete);
  }

  // read function for simplified use
  bool read(ioHandler &handler, hmmClassifier &p, const bool complete) {
    return p.read(handler, complete);
  }

}
