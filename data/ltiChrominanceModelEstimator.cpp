/*
 * Copyright (C) 2004, 2005, 2006
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
 * file .......: ltiChrominanceModelEstimator.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 8.2.2004
 * revisions ..: $Id: ltiChrominanceModelEstimator.cpp,v 1.6 2007/11/28 20:50:51 goenner Exp $
 */

#include "ltiMacroSymbols.h"

// only for compilers different than VC++ 6.0 available
#ifndef _LTI_MSC_6

#include "ltiChrominanceModelEstimator.h"
#include <iostream>
#include <fstream>

#include "ltiTypes.h"
#include "ltiLispStreamHandler.h"

namespace lti {
  // --------------------------------------------------
  // chrominanceModelEstimator::parameters
  // --------------------------------------------------

  // default constructor
  chrominanceModelEstimator::parameters::parameters()
    : statisticsFunctor::parameters(),
      checker ( overlappingSets2D() ) {

    checkIsObject = bool(false);
    autoIsObjectSize = bool(true);
    shrinkFactor = int(0);
    numberOfModels = int(0);
    interval = int (100);
    decay = float(.7);
    minHits = int(1000);
  }

  // copy constructor
  chrominanceModelEstimator::parameters::parameters(const parameters& other)
    : statisticsFunctor::parameters(),
      checker ( overlappingSets2D() ) {
    copy(other);
  }

  // destructor
  chrominanceModelEstimator::parameters::~parameters() {
  }

  // get type name
  const char* chrominanceModelEstimator::parameters::getTypeName() const {
    return "chrominanceModelEstimator::parameters";
  }

  // copy member

  chrominanceModelEstimator::parameters&
    chrominanceModelEstimator::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    statisticsFunctor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    statisticsFunctor::parameters& (statisticsFunctor::parameters::* p_copy)
      (const statisticsFunctor::parameters&) =
      statisticsFunctor::parameters::copy;
    (this->*p_copy)(other);
# endif
 
    checkIsObject = other.checkIsObject;
    autoIsObjectSize = other.autoIsObjectSize;
    shrinkFactor = other.shrinkFactor;
    numberOfModels = other.numberOfModels;
    interval = other.interval;
    decay = other.decay;
    minHits = other.minHits;
    checker = other.checker;

    return *this;
  }

  // alias for copy member
  chrominanceModelEstimator::parameters&
    chrominanceModelEstimator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* chrominanceModelEstimator::parameters::clone() const {
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
  bool chrominanceModelEstimator::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool chrominanceModelEstimator::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"checkIsObject",checkIsObject);
      lti::write(handler,"autoIsObjectSize",autoIsObjectSize);
      lti::write(handler,"shrinkFactor",shrinkFactor);
      lti::write(handler,"numberOfModels",numberOfModels);
      lti::write(handler,"interval",interval);
      lti::write(handler,"decay",decay);
      lti::write(handler,"minHits",minHits);
      lti::write(handler,"checker",checker);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && statisticsFunctor::parameters::write(handler,false);
# else
    bool (statisticsFunctor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      statisticsFunctor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool chrominanceModelEstimator::parameters::write(ioHandler& handler,
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
   * @return true if read was successful
   */
# ifndef _LTI_MSC_6
  bool chrominanceModelEstimator::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool chrominanceModelEstimator::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"checkIsObject",checkIsObject);
      lti::read(handler,"autoIsObjectSize",autoIsObjectSize);
      lti::read(handler,"shrinkFactor",shrinkFactor);
      lti::read(handler,"numberOfModels",numberOfModels);
      lti::read(handler,"interval",interval);
      lti::read(handler,"decay",decay);
      lti::read(handler,"minHits",minHits);
      lti::read(handler,"checker",checker);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && statisticsFunctor::parameters::read(handler,false);
# else
    bool (statisticsFunctor::parameters::* p_readMS)(ioHandler&,const bool) =
      statisticsFunctor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool chrominanceModelEstimator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

 
  // --------------------------------------------------
  // chrominanceModelEstimator
  // --------------------------------------------------

  // default constructor
  chrominanceModelEstimator::chrominanceModelEstimator()
    : statisticsFunctor(),
      m_chromHistos ( std::vector<histogram2D>(0) ),
      m_count ( ivector(0) ) {
    
    m_maxEntries = 256;
    m_numEntries = 256;
    m_constructModel = true;
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);    
  }

  // default constructor
  chrominanceModelEstimator::chrominanceModelEstimator(const parameters& par)
    : statisticsFunctor(),
      m_chromHistos ( std::vector<histogram2D>(0) ),
      m_count ( ivector(0) ) {

    m_maxEntries = 256;
    m_numEntries = 256;
    m_constructModel = true;
    // set the given parameters
    setParameters(par);
  }

  // copy constructor
  chrominanceModelEstimator
  ::chrominanceModelEstimator(const chrominanceModelEstimator& other)
    : statisticsFunctor() {
    copy(other);
  }

  // destructor
  chrominanceModelEstimator::~chrominanceModelEstimator() {
  }

  // returns the name of this type
  const char* chrominanceModelEstimator::getTypeName() const {
    return "chrominanceModelEstimator";
  }

  // copy member
  chrominanceModelEstimator&
    chrominanceModelEstimator::copy(const chrominanceModelEstimator& other) {
      statisticsFunctor::copy(other);
     
      m_chromHistos = other.m_chromHistos;
      m_count = other.m_count;
      m_maxEntries = other.m_maxEntries;
      m_numEntries = other.m_numEntries;
      m_constructModel = other.m_constructModel;

      return (*this);
  }

  // alias for copy member
  chrominanceModelEstimator& chrominanceModelEstimator
  ::operator=(const chrominanceModelEstimator& other) {
    return (copy(other));
  }


  // clone member
  functor* chrominanceModelEstimator::clone() const {
    return new chrominanceModelEstimator(*this);
  }

  // return parameters
  const chrominanceModelEstimator::parameters&
    chrominanceModelEstimator::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // return parameters
  chrominanceModelEstimator::parameters&
    chrominanceModelEstimator::getWritableParameters() {
    parameters* par =
      dynamic_cast<parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The methods!
  // -------------------------------------------------------------------
  bool chrominanceModelEstimator::setParameters(const parameters& theParams) {
 
   //set the scale factor
    int i ( 0 );
    m_numEntries = m_maxEntries;
    for ( ; i<theParams.shrinkFactor; i++ ) {
      m_numEntries /= 2;
    }
 
    if ( theParams.checkIsObject ) {
      //check validity of checker
      if ( !theParams.checker.isInitialized() ) {
	setStatusString ("setParameters: checker is not initialized");
	if ( m_constructModel ) {
	  throw exception(getStatusString());
	}
	return false;
      } else {
	//make sure that the checker supports the right size
	if ( theParams.checker.getParameters().size.x < m_numEntries ||
	     theParams.checker.getParameters().size.y < m_numEntries ) {
	  setStatusString ("setParameters: checker is too small");
	  if ( m_constructModel ) {
	    throw exception(getStatusString());
	  }
	  return false;
	}
      }
    }
    bool success ( true );
    
    if ( theParams.autoIsObjectSize ) {
      if ( theParams.checker.getParameters().size.x < m_numEntries ||
	   theParams.checker.getParameters().size.y < m_numEntries ) {
	setStatusString("setParameters: isObjectTable is too small");
	return false;
      }
    }

    if ( !m_constructModel ) {
      const parameters& oldPar = getParameters();
      if ( oldPar.numberOfModels != theParams.numberOfModels ) {
	//adjust the number Of histograms
	m_chromHistos.clear();
	m_count.resize(theParams.numberOfModels, 0);
	int i (0);
	for ( ; i < theParams.numberOfModels; i++ ) {
	  m_count[i] = 0;  //to be on the safe side
	  m_chromHistos.push_back(histogram2D(m_numEntries, m_numEntries));
	}

      } else {    
	// adjust the histogram size
	std::vector<histogram2D>::iterator end ( m_chromHistos.end() );
	std::vector<histogram2D>::iterator it  ( m_chromHistos.begin() );
	for ( ; it!=end; ++it ) {
	  success &= rescale(*it, m_numEntries);
	}
      }

    } else {
      //initialize the histograms
      m_chromHistos.clear();
      m_count.resize(theParams.numberOfModels, 0);
      int i ( 0 );
      for ( ; i < theParams.numberOfModels; i++ ) {
	m_count[i] = 0;  //to be on the safe side
	m_chromHistos.push_back(histogram2D(m_numEntries, m_numEntries));
      }
      m_constructModel = false;
    }

    //must be done after getting oldPar
    success &= statisticsFunctor::setParameters(theParams);

    //adjust the size of the is=bjectTable
    if ( theParams.autoIsObjectSize ) {
      parameters& wpar = getWritableParameters();
      if ( wpar.checker.isInitialized() ) {
	overlappingSets2D::parameters cpar ( wpar.checker.getParameters() );
	if ( cpar.size != ipoint(m_numEntries,m_numEntries) ) {
	  cpar.size = ipoint(m_numEntries,m_numEntries);
	  wpar.checker.setParameters(cpar);
	}
      }
    }

    return success;
  }
  
  bool chrominanceModelEstimator
  ::setChrominanceModels(const std::vector<histogram2D>& theHistos) {
    if ( !checkHistograms(theHistos) ) {
      return false;
    }

    bool success ( true );
    m_chromHistos.clear();
 
    std::vector<histogram2D>::const_iterator end ( theHistos.end() );
    std::vector<histogram2D>::const_iterator it  ( theHistos.begin() );
    for ( ; it!=end; ++it ) {
      histogram2D hist;
      success &= rescale(*it, hist, m_numEntries);
      m_chromHistos.push_back(hist);
    }
    
    return success;
  }
  
  const std::vector<histogram2D>& chrominanceModelEstimator
  ::getChrominanceModels() const {
    return m_chromHistos;
  }
 
  // -------------------------------------------------------------------
 
  bool chrominanceModelEstimator::consider(const channel8& uChan,
					  const channel8& vChan,
					  const channel8& mask) {
    
    const parameters& par = getParameters();
    ivector count ( par.numberOfModels, 1 );
    return consider(uChan, vChan, mask, count);
  }
   
  // -------------------------------------------------------------------
 
  bool chrominanceModelEstimator::consider(const channel8& uChan,
                                           const channel8& vChan,
                                           const bool trainForeground) {
    
    const parameters& par = getParameters();
    ivector count ( par.numberOfModels, 1 );
    return consider(uChan, vChan, trainForeground, count);
  }
   
  // -------------------------------------------------------------------
 
  bool chrominanceModelEstimator::consider(const channel8& uChan,
                                           const channel8& vChan,
                                           const int index) {
    
    const parameters& par = getParameters();
    ivector count ( par.numberOfModels, 1 );
    return consider(uChan, vChan, index, count);
  }
   
  // -------------------------------------------------------------------
 
  bool chrominanceModelEstimator::increaseCount(const ivector& count) {
 
    if ( count.size() != m_count.size() ) {
       setStatusString("consider: count has wrong size");
      return false;
    }

    //increase the count und check for decay
    m_count += count;
    const parameters& par = getParameters();
    if ( par.interval > 0 ) {
      ivector::iterator end ( m_count.end()   );
      ivector::iterator it  ( m_count.begin() );
      std::vector<histogram2D>::iterator hit ( m_chromHistos.begin() );
      for ( ; it!=end; ++it, ++hit ) {
	if ( *it == par.interval ) {
	  scaleValues(*hit);
	  *it = 0;
	}
      }
    }

    return true;
  }

  // -------------------------------------------------------------------
 
  bool chrominanceModelEstimator::consider (const channel8& uChan,
					   const channel8& vChan,
					   const channel8& mask,
					   const ivector& count) {
 
    //The values in the mask fit to the indices of the histo-vectors.
    //The background (default object) should have index 0.
    //Any pixels that shall not be considered at all must have a value
    //bigger than the number of models.
    const point& size = mask.size();
    if ( size != uChan.size() ||
	 size != vChan.size() ) {
      setStatusString("consider: input is of different size");
      return false;
    }

    if ( !increaseCount(count) ) {
      return false;
    }

    const parameters& par = getParameters();

    const overlappingSets2D& checker = par.checker;
    const int numRows ( mask.rows() );
    int row ( 0 );
    for(; row<numRows; row++) {
      const vector<ubyte>& maskVct = mask.getRow(row);
      const vector<ubyte>& uVct    = uChan.getRow(row);
      const vector<ubyte>& vVct    = vChan.getRow(row);
      
      vector<ubyte>::const_iterator end    ( maskVct.end()   );
      vector<ubyte>::const_iterator maskIt ( maskVct.begin() );
      vector<ubyte>::const_iterator    uIt (    uVct.begin() );
      vector<ubyte>::const_iterator    vIt (    vVct.begin() );
      
      for ( ; maskIt != end; ++maskIt,++uIt,++vIt ) {
	if ( *maskIt >= par.numberOfModels ) {
	  continue;
	}
	const ubyte u ( *uIt>>par.shrinkFactor );
	const ubyte v ( *vIt>>par.shrinkFactor );
	if ( par.checkIsObject &&
	     !checker.isContained(*maskIt, u, v) ) {
	  continue;
	}
	//consider the value pair
	(m_chromHistos[*maskIt]).put(v, u);
      }
    }
    
    return true;
  }

  // -------------------------------------------------------------------
 
  bool chrominanceModelEstimator::consider (const channel8& uChan,
                                            const channel8& vChan,
                                            const bool trainForeground,
                                            const ivector& count) {
 
    if ( uChan.size() != vChan.size() ) {
      setStatusString("consider: input is of different size");
      return false;
    }

    if ( !increaseCount(count) ) {
      return false;
    }

    const parameters& par = getParameters();
    if ( par.checkIsObject && !par.checker.isInitialized() ) {
      
      setStatusString ("consider: checker is not initialized");
      return false;
    }

    if ( par.numberOfModels < 2 ) {
      
      setStatusString("consider: please provide at least 2 models");
      return false;
    }

    const int numLabels = count.size();

    const overlappingSets2D& checker = par.checker;

    const int numRows ( uChan.rows() );
    int row ( 0 );
    for(; row<numRows; row++) {

      const vector<ubyte>& uVct    = uChan.getRow(row);
      const vector<ubyte>& vVct    = vChan.getRow(row);
      
      vector<ubyte>::const_iterator end ( uVct.end()   );
      vector<ubyte>::const_iterator uIt ( uVct.begin() );
      vector<ubyte>::const_iterator vIt ( vVct.begin() );
      
      if ( trainForeground ) {
        
        for ( ; uIt != end; ++uIt,++vIt ) {
          const ubyte u ( *uIt>>par.shrinkFactor );
          const ubyte v ( *vIt>>par.shrinkFactor );
          
          //no background
          for ( int i = 1; i < numLabels; ++i ) {
            if ( par.checkIsObject &&
                 !checker.isContained(i, u, v) ) {
              continue;
            }
            
            //consider the value pair
            (m_chromHistos[i]).put(v, u);
          }
        }

      } else {
        
         for ( ; uIt != end; ++uIt,++vIt ) {
          const ubyte u ( *uIt>>par.shrinkFactor );
          const ubyte v ( *vIt>>par.shrinkFactor );
     
          if ( par.checkIsObject &&
               !checker.isContained(0, u, v) ) {
            continue;
          }
            
          //consider the value pair
          (m_chromHistos[0]).put(v, u);
        }
      }
    }
 
    return true;
  }

  // -------------------------------------------------------------------
 
  bool chrominanceModelEstimator::consider (const channel8& uChan,
                                            const channel8& vChan,
                                            const int index,
                                            const ivector& count) {

    if ( uChan.size() != vChan.size() ) {
      setStatusString("consider: input is of different size");
      return false;
    }

    if ( !increaseCount(count) ) {
      return false;
    }

    const parameters& par = getParameters();
    if ( par.checkIsObject && !par.checker.isInitialized() ) {
      
      setStatusString ("consider: checker is not initialized");
      return false;
    }

    if ( par.numberOfModels < 2 ) {
      
      setStatusString("consider: please provide at least 2 models");
      return false;
    }

    if ( index >= par.numberOfModels ) {
      
      setStatusString("consider: please provide a valid index");
      return false;
    }

    const overlappingSets2D& checker = par.checker;

    const int numRows ( uChan.rows() );
    int row ( 0 );
    for(; row<numRows; row++) {

      const vector<ubyte>& uVct    = uChan.getRow(row);
      const vector<ubyte>& vVct    = vChan.getRow(row);
      
      vector<ubyte>::const_iterator end ( uVct.end()   );
      vector<ubyte>::const_iterator uIt ( uVct.begin() );
      vector<ubyte>::const_iterator vIt ( vVct.begin() );
   
      for ( ; uIt != end; ++uIt,++vIt ) {
        const ubyte u ( *uIt>>par.shrinkFactor );
        const ubyte v ( *vIt>>par.shrinkFactor );
        
        if ( par.checkIsObject &&
             !checker.isContained(index, u, v) ) {
          continue;
        }
        
        //consider the value pair
        (m_chromHistos[index]).put(v, u);
      }
    }
 
    return true;
  }

  // -------------------------------------------------------------------

  bool chrominanceModelEstimator
  ::checkHistograms(const std::vector<histogram2D>& theHistos) {

    const parameters& par = getParameters();
    const int numHistos ( theHistos.size() );
    if ( numHistos != par.numberOfModels ) {
      setStatusString("update: wrong number of histograms");
      return false;
    }

    std::vector<histogram2D>::const_iterator hend ( theHistos.end() );
    std::vector<histogram2D>::const_iterator hit  ( theHistos.begin() );
    const int numEntries ( (*hit).cellsInDimension(0) );
    for ( ; hit!=hend; ++hit ) {
      if ( (*hit).dimensions() != 2 ) {
	setStatusString("update: a histogram has wrong dimension");
	return false;
      }
      if ( (*hit).cellsInDimension(0) != numEntries &&
	   (*hit).cellsInDimension(1) != numEntries) {
	setStatusString("update: a histogram has wrong size");
	return false;
      }
    }

    return true;
  }
  
  // -------------------------------------------------------------------

  bool chrominanceModelEstimator::scaleValues(histogram2D& theHisto) {
    
    const parameters& par = getParameters();
    if ( theHisto.maximum() < par.minHits ) {
      return false;
    }
    histogram2D::iterator it ( theHisto.begin() );    
    histogram2D::iterator end ( theHisto.end() );
    for ( ; it != end; ++it ) {
      //scale the entries
      //Never delete an entry completely!!! This way chrominances
      //carved out from the background will survive and objects
      //cannot be deleted completely.
      *it = max( histogram2D::value_type(1),
		 static_cast<histogram2D::value_type>(*it * par.decay) );
    }
    (theHisto).updateNumberOfEntries();
  
    return true;
  }

  // -------------------------------------------------------------------

  bool chrominanceModelEstimator
  ::writeChrominanceModels(const std::string& prefix,
			  const std::vector<std::string>& fileNames) const {

    const unsigned int size ( fileNames.size() );
    if ( size == 0 ) {
      return false;
    }
    if ( size < m_chromHistos.size() ) {
      setStatusString("writeChrominanceModels: Too few file names");
      return false;
    }
 
    unsigned int i ( 0 );
    bool success ( true );
    for ( ; i<size; i++) {
      std::string tmpStr (prefix + fileNames[i]);
      std::ofstream outfile(tmpStr.c_str());
      if (!outfile.is_open()) {
	setStatusString
	  ("writeChrominanceModels: Could not write all histograms");
	success = false;
      } else {
	lti::lispStreamHandler fileHistOut(outfile);
	success &= m_chromHistos[i].write(fileHistOut);
      }
    }

    return success;
  }

  bool chrominanceModelEstimator
  ::readChrominanceModels(const std::string& prefix,
			 const std::vector<std::string>& fileNames) {

    const unsigned int size ( fileNames.size() );
    if ( size == 0 ) {
      return false;
    }
    if ( size < m_chromHistos.size() ) {
      setStatusString("readChrominanceModels: Too few file names");
      return false;
    }

    unsigned int i ( 0 );
    bool success ( true );
    for ( ; i<size; i++) {
      //check if the input file exists and can be read
      std::string tmpStr (prefix + fileNames[i]);
      std::ifstream infile(tmpStr.c_str());
      if (!infile.is_open()) {
	setStatusString("readHistograms: Could not read all histograms");
	success = false;
      } else {
	lti::lispStreamHandler fileHistIn(infile);
	success &= m_chromHistos[i].read(fileHistIn);
	if ( m_chromHistos[i].dimensions() != 2 ) {
	  m_chromHistos[i].resize(m_numEntries, m_numEntries);
	  setStatusString("readHistograms: no 2D histogram");
	  success = false;
	}
	//rescale the histogram to fit the parameters
	if ( !rescale ( m_chromHistos[i], m_numEntries ) ) {
	  setStatusString("readHistograms: rescale error");
	  m_chromHistos[i].resize(m_numEntries, m_numEntries);
	  success = false;
	}
      }
    }
   
    return success;
  }

  // -------------------------------------------------------------------
  bool chrominanceModelEstimator::rescale(histogram2D& theHisto,
					 const int cellsInDimension) {
    histogram2D tmpHisto ( theHisto );
    return rescale(tmpHisto, theHisto, cellsInDimension);
  }

  bool chrominanceModelEstimator::rescale(const histogram2D& theHisto, 
					 histogram2D& destHisto,
					 const int cellsInDimension) {
 
    //there is no remainder when dividing scales by definition
    const int oldScale ( m_maxEntries / theHisto.cellsInDimension(0) );
    const int newScale  ( m_maxEntries / cellsInDimension );
    //the number of entries must always be smaller than m_maxEntries
    if ( newScale == oldScale ) {
      destHisto.copy(theHisto);
      return true;
    } else if ( newScale < 1 ) {
      setStatusString("rescale: no true multiples");
      return false;
    }

    const int deltaScale ( newScale / oldScale );   
    if ( deltaScale == 0 ) {
      upsample( theHisto, destHisto, oldScale / newScale );
      
    } else {
      downsample( theHisto, destHisto, deltaScale );
    }

    return true;
  }

  bool chrominanceModelEstimator::upsample(const histogram2D& theHisto, 
					  histogram2D& destHisto,
					  const int factor) {

    if ( factor == 1 ) {
      destHisto.copy(theHisto);
      return true;
    }
 
    ivector tmp ( theHisto.cellsPerDimension() );
    const int oldRows ( tmp.at(0) );
    const int oldCols ( tmp.at(1) );
    tmp *= factor;
    destHisto.resize( tmp.at(0), tmp.at(1) );
      
    //the downsampling class does not work here
    //kernel2D<float> theKernel (factor, factor, 1.f);
    //downsampling sampler(theKernel);
    //sampler.apply(other.theHistoMatrix, theHistoMatrix);
    
    //so we do it ourself
    histogram2D::iterator newRowIt ( destHisto.begin() );
    histogram2D::const_iterator oldIt ( theHisto.begin() );

    int rows ( 0 );
    for ( ; rows < oldRows; ++rows ) {

      std::vector<histogram2D::iterator> colIts ( factor );
      {
	std::vector<histogram2D::iterator>::iterator it ( colIts.begin() );
	for ( ; it!= colIts.end(); ++it ) {
	  *it = newRowIt;
	  newRowIt += tmp.at(1); //jump to next 'row'
	}
      }
      int cols ( 0 );
      for ( ; cols < oldCols; ++cols ) {	
	int j ( 0 );
	for ( ; j < factor; ++j) {
	  int i ( 0 );
	  for ( ; i < factor; ++i) {
	    *(colIts[j]) = *oldIt / factor;
	    ++(colIts[j]);
	  }
	}
	++oldIt;
      }
    }
    
    destHisto.updateNumberOfEntries();

    return true;
  }

  bool chrominanceModelEstimator::downsample(const histogram2D& theHisto,
					    histogram2D& destHisto,
					    const int factor) {

    if ( factor == 1 ) {
      destHisto.copy(theHisto);
      return true;
    }

    ivector tmp ( theHisto.cellsPerDimension() );
    const int oldCols ( tmp.at(1) );
    tmp /= factor;
    destHisto.resize( tmp.at(0), tmp.at(1) );
      
    //the downsampling class does not work here
    //kernel2D<float> theKernel (factor, factor, 1.f);
    //downsampling sampler(theKernel);
    //sampler.apply(other.theHistoMatrix, theHistoMatrix);
    
    //so we do it ourself
    histogram2D::iterator newRowIt ( destHisto.begin() );
    histogram2D::const_iterator oldRowIt ( theHisto.begin() );

    int rows ( 0 );
    for ( ; rows < tmp.at(0); ++rows ) {
      
      histogram2D::iterator newIt ( newRowIt );
      
      std::vector<histogram2D::const_iterator> colIts ( factor );
      {
	std::vector<histogram2D::const_iterator>::iterator it (colIts.begin());
	for ( ; it!= colIts.end(); ++it ) {
	  *it = oldRowIt;
	  oldRowIt += oldCols; //jump to next 'row'
	}
      }
      int cols ( 0 );
      for ( ; cols < tmp.at(1); ++cols ) {
	histogram2D::value_type sum ( 0 );
	int j ( 0 );
	for ( ; j < factor; ++j) {
	  int i ( 0 );
	  for ( ; i < factor; ++i) {
	    sum += *(colIts[j]);
	      ++(colIts[j]);
	  }
	}
	*newIt = sum;
	++newIt;
      }
      newRowIt += tmp.at(1); //jump to next 'row'
    }
    
    destHisto.updateNumberOfEntries();

    return true;
  }

}

#endif
