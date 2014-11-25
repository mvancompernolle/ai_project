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
 * file .......: ltiprobabilityMapBaseBase.cpp
 * authors ....: Benjamin Winkler, Florian Bley
 * organization: LTI, RWTH Aachen
 * creation ...: 30.1.2001
 * revisions ..: $Id: ltiProbabilityMapBase.cpp,v 1.11 2006/09/05 10:25:56 ltilib Exp $
 */

#include "ltiProbabilityMapBase.h"
#include "ltiSquareConvolution.h"
#include "ltiConvolution.h"
#include "ltiGaussKernels.h"

namespace lti {
  // --------------------------------------------------
  // probabilityMapBase::parameters
  // --------------------------------------------------

  // default constructor
  probabilityMapBase::parameters::parameters()
    : transform::parameters(), objectColorModel() {
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    objectProbability = 0.5;

    nonObjectColorModel = 0;
    objectColorModel = 0;
    ownModels = true;

    iterations = 1;
    gaussian = false;
    windowSize = 5;
    variance = -1;


  }

  // copy constructor
  probabilityMapBase::parameters::parameters(const parameters& other)
    : transform::parameters(), ownModels (true), nonObjectColorModel(0), objectColorModel(0)  {
    copy(other);
  }

  // destructor
  probabilityMapBase::parameters::~parameters() {
    if (ownModels) {
      delete nonObjectColorModel;
      delete objectColorModel;
      nonObjectColorModel = 0;
      objectColorModel = 0;
    }
  }

  // get type name
  const char* probabilityMapBase::parameters::getTypeName() const {
    return "probabilityMapBase::parameters";
  }

  // copy member

  probabilityMapBase::parameters&
  probabilityMapBase::parameters::copy(const parameters& other) {
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

    // just copy the references
    objectProbability = other.objectProbability;

    if (ownModels) {
      delete nonObjectColorModel;
      delete objectColorModel;
      nonObjectColorModel = 0;
      objectColorModel = 0;
    }

    ownModels = other.ownModels;

    if (other.ownModels) {
      if (other.isNonObjectColorModelValid()) {
        nonObjectColorModel =
          new thistogram<double>(*other.nonObjectColorModel);
      }
      if (other.isObjectColorModelValid()) {
        objectColorModel = new thistogram<double>(*other.objectColorModel);
      }
    } else {
      nonObjectColorModel = other.nonObjectColorModel;
      objectColorModel = other.objectColorModel;
    }

    iterations = other.iterations;
    gaussian   = other.gaussian;
    windowSize = other.windowSize;
    variance   = other.variance;

    return *this;
  }

  probabilityMapBase::parameters& probabilityMapBase
  ::parameters::copyAllButHistograms(const parameters& other) {

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

    // just copy the references
    objectProbability = other.objectProbability;
    iterations = other.iterations;
    gaussian   = other.gaussian;
    windowSize = other.windowSize;
    variance   = other.variance;

    return *this;
  }


  // alias for copy member
  probabilityMapBase::parameters&
    probabilityMapBase::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool probabilityMapBase::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool probabilityMapBase::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"objectProbability",objectProbability);

      thistogram<double> dummy;

      if (ownModels && notNull(nonObjectColorModel)) {
        lti::write(handler,"nonObjectColorModel", *nonObjectColorModel);
      } else {
        lti::write(handler,"nonObjectColorModel", dummy);
      }

      if (ownModels && notNull(objectColorModel)) {
        lti::write(handler,"objectColorModel", *objectColorModel);
      } else {
        lti::write(handler,"objectColorModel", dummy);
      }

      lti::write(handler,"iterations",iterations);
      lti::write(handler,"gaussian",gaussian);
      lti::write(handler,"windowSize",windowSize);
      lti::write(handler,"variance",variance);
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
  bool probabilityMapBase::parameters::write(ioHandler& handler,
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
  bool probabilityMapBase::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool probabilityMapBase::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      if (ownModels) {
        delete objectColorModel;
        delete nonObjectColorModel;
        objectColorModel = 0;
        nonObjectColorModel = 0;
      }

      ownModels = true;
      thistogram<double>* tmpObjectColorModel = new thistogram<double>();
      thistogram<double>* tmpNonObjectColorModel = new thistogram<double>();

      lti::read(handler,"objectProbability",objectProbability);
      lti::read(handler,"nonObjectColorModel", *tmpNonObjectColorModel);
      lti::read(handler,"objectColorModel", *tmpObjectColorModel);

      objectColorModel = tmpObjectColorModel;
      nonObjectColorModel = tmpNonObjectColorModel;

      lti::read(handler,"iterations",iterations);
      lti::read(handler,"gaussian",gaussian);
      lti::read(handler,"windowSize",windowSize);
      lti::read(handler,"variance",variance);
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
  bool probabilityMapBase::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  /*
   * set object color model
   */
  void
  probabilityMapBase::parameters::setObjectColorModel(const thistogram<double>&
                                                  objModel) {
    if (ownModels) {
      delete objectColorModel;
      objectColorModel = new thistogram<double>(objModel);
    } else {
      objectColorModel = &objModel;
    }
  }

  /*
   * get object color model
   */
  const thistogram<double>&
  probabilityMapBase::parameters::getObjectColorModel() const {
    return *objectColorModel;
  }

  /*
   * set non-object color model
   */
  void
  probabilityMapBase::parameters::setNonObjectColorModel(const thistogram<double>&
                                                     nonObjModel) {
    if (ownModels) {
      delete nonObjectColorModel;
      nonObjectColorModel = new thistogram<double>(nonObjModel);
    } else {
      nonObjectColorModel = &nonObjModel;
    }
  }

  /*
   * get non-object color model
   */
  const thistogram<double>&
  probabilityMapBase::parameters::getNonObjectColorModel() const {
    return *nonObjectColorModel;
  }

  // --------------------------------------------------
  // probabilityMapBase
  // --------------------------------------------------

  // default constructor
  probabilityMapBase::probabilityMapBase()
    : transform() {

    probabilityHistogram.clear();
  }

  // copy constructor
  probabilityMapBase::probabilityMapBase(const probabilityMapBase& other)
    : transform()  {

    probabilityHistogram.clear();

    copy(other);
  }

  // destructor
  probabilityMapBase::~probabilityMapBase() {
  }

  // returns the name of this type
  const char* probabilityMapBase::getTypeName() const {
    return "probabilityMapBase";
  }

  // copy member
  probabilityMapBase&
    probabilityMapBase::copy(const probabilityMapBase& other) {
    transform::copy(other);

    // after copying successfuly the parameters, the state
    // of the functors must be updated
    if (validParameters())
      generate();

    return (*this);
  }

  // copy member
  probabilityMapBase&
    probabilityMapBase::operator=(const probabilityMapBase& other) {
    return copy(other);
  }

  // clone member
//   functor* probabilityMapBase::clone() const {
//     return new probabilityMapBase(*this);
//   }

  // return parameters
  const probabilityMapBase::parameters&
    probabilityMapBase::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  /*
   * setParameters needs to be overloaded to avoid reloading the
   * histograms every time apply is called.
   */
  bool probabilityMapBase::updateParameters() {
    return generate();
  }

  /**
   * setParameters needs to be overloaded to avoid reloading the
   * histograms every time apply is called.
   */
  bool probabilityMapBase
  ::setParametersKeepingHistograms(const parameters &theParams) {
    // get a read-writable copy of the parameters

    parameters* par = dynamic_cast<parameters*>(&functor::getParameters());
    if(isNull(par)) {
      setStatusString("Invalid parameters type or parameters not set yet");
      return false;
    }
    
    par->copyAllButHistograms(theParams);

    return true;
  }

  /**
   * generate lookup tables for faster element access
   */
  void probabilityMapBase::generateLookupTable(const ivector &dimensions) {

    int numberOfDim ( dimensions.size() );
    lookupTable.resize(numberOfDim, 256);

    for (int dimCounter = 0; dimCounter < numberOfDim; dimCounter++) {
      vector<ubyte> &row = lookupTable.getRow(dimCounter);
      for (int elemCounter = 0; elemCounter < 256; elemCounter++) {
        // map to 0 .. (dimensions-1)
        row[elemCounter] = elemCounter * dimensions[dimCounter] / 256;
      }
    }
  }


  /*
   * generate map from 2 histograms
   */
  bool probabilityMapBase::generate(const thistogram<double> &objectModel,
				    const thistogram<double> &nonObjectModel) {

    double objectProbability = getParameters().objectProbability;
    ivector histogramSize = objectModel.cellsPerDimension();

    // check for same size
    if (!(nonObjectModel.cellsPerDimension() == histogramSize)) {
      setStatusString("probabilityMapBase: histograms must have same size");
      return false;
    }

    // initialize probability histogram
    probabilityHistogram.resize(histogramSize.size(), histogramSize);

    // scan all 3 histograms
    thistogram<double>::const_iterator objectIt    = objectModel.begin();
    thistogram<double>::const_iterator nonObjectIt = nonObjectModel.begin();
    thistogram<double>::iterator probabilityIt = probabilityHistogram.begin();

    double objNumEntries = objectModel.getNumberOfEntries();
    double nonObjNumEntries = nonObjectModel.getNumberOfEntries();

    // avoid division by zero
    if (objNumEntries == 0.0) {
      objNumEntries = 1.0;  // no difference since all entries are 0.0
    }

    // avoid division by zero
    if (nonObjNumEntries == 0.0) {
      nonObjNumEntries = 1.0;  // no difference since all entries are 0.0
    }

    double relObjProb;
    double relNonObjProb;
    const double nonObjectProbability = (1.0-objectProbability);

    while (objectIt != objectModel.end()) {

      relObjProb = (*objectIt) * objectProbability / objNumEntries;
      relNonObjProb = (*nonObjectIt) * nonObjectProbability / nonObjNumEntries;

      // assume non-object if no entries are given
      if ((relObjProb == 0) && (relNonObjProb == 0)) {
        (*probabilityIt) = 0.0;
      }
      else {
        // bayes
        (*probabilityIt) = relObjProb / (relObjProb + relNonObjProb);
      }

      objectIt++;
      nonObjectIt++;
      probabilityIt++;
    }

    return true;
  }

  /*
   * generate map of 1 histogram and an equipartition representing the
   * non object model
   */
  bool probabilityMapBase::generate(const thistogram<double> &objectModel) {

    double objectProbability = getParameters().objectProbability;
    ivector histogramSize = objectModel.cellsPerDimension();

    // calculate constant non object value = 1.0 / numberOfCells
    // (equal distribution assumed)
    double constNonObjectValue = 1.0;
    ivector::const_iterator vecEntry = histogramSize.begin();
    while (!(vecEntry == histogramSize.end())) {
      constNonObjectValue /= (*(vecEntry++));
    }

    // scan both histograms
    thistogram<double>::const_iterator objectIt = objectModel.begin();
    thistogram<double>::iterator probabilityIt  = probabilityHistogram.begin();

    double objNumEntries = objectModel.getNumberOfEntries();

    // avoid division by zero
    if (objNumEntries == 0.0) {
      objNumEntries = 1.0;  // no difference since all entries are 0.0
    }

    double relNonObjProb = constNonObjectValue * (1.0 - objectProbability);
    double relObjProb;
    while (objectIt != objectModel.end()) {

      relObjProb = (*objectIt) * objectProbability / objNumEntries;

      // bayes
      (*probabilityIt) = relObjProb / (relObjProb + relNonObjProb);

      objectIt++;
      probabilityIt++;
    }

    return true;
  }

  /**
   * generate probability map
   */
  bool probabilityMapBase::generate() {

    const parameters& param = getParameters();

    if (!param.isObjectColorModelValid()) {
      setStatusString("probabilityMapBase: no object color model specified");
      return false;
    }

    const thistogram<double>&
      objectModel = getParameters().getObjectColorModel();

    probabilityHistogram.clear();

    // initialize probability histogram
    ivector histogramSize ( objectModel.cellsPerDimension() );
    probabilityHistogram.resize(histogramSize.size(), histogramSize);

    generateLookupTable(histogramSize);

    // generate map
    if (!param.isNonObjectColorModelValid()) {  //ToDo: probably cases matrix overflow
      // constant non-object model

      return generate(objectModel);
    } else {
      const thistogram<double>&
        nonObjectModel = getParameters().getNonObjectColorModel();
      return generate(objectModel, nonObjectModel);
    }

    return false;
  }

}
