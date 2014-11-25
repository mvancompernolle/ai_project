/*
 * Copyright (C) 2003, 2004, 2005, 2006
 * Lehrstuhl fuer Technische Informatik, RWTH-Aachen, Germany
 *
 * This file is art of the LTI-Computer Vision Library (LTI-Lib)
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
 * file .......: ltiChrominanceMapEstimator.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 21.9.2003
 * revisions ..: $Id: ltiChrominanceMapEstimator.cpp,v 1.8 2006/09/05 10:05:05 ltilib Exp $
 */

#include "ltiMacroSymbols.h"

// only for compilers different than VC++ 6.0 available
#ifndef _LTI_MSC_6

#include "ltiRGBPixel.h"
#include "ltiChrominanceMapEstimator.h"

#include <iostream>
#include <limits>
#include "ltiTypes.h"
#include "ltiLispStreamHandler.h"
#include "ltiALLFunctor.h"

namespace lti {
  // --------------------------------------------------
  // chrominanceMapEstimator::parameters
  // --------------------------------------------------

  // default constructor
  chrominanceMapEstimator::parameters::parameters(int dim)
    : chrominanceModelEstimator::parameters() {
    
    numberOfModels = dim;
    if ( numberOfModels != 0 ) {
      objectProbabilities = fvector(numberOfModels, 1.f/float(numberOfModels));
    } else {
      objectProbabilities = fvector(0);
    }
    autoObjectProbabilities = bool(true);
    defaultProbability = float(.0001);
    excludeBackground = bool(false);
  }

  // copy constructor
  chrominanceMapEstimator::parameters::parameters(const parameters& other)
    : chrominanceModelEstimator::parameters() {
    copy(other);
  }

  // destructor
  chrominanceMapEstimator::parameters::~parameters() {
  }

  // get type name
  const char* chrominanceMapEstimator::parameters::getTypeName() const {
    return "chrominanceMapEstimator::parameters";
  }

  // copy member

  chrominanceMapEstimator::parameters&
    chrominanceMapEstimator::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    chrominanceModelEstimator::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    chrominanceModelEstimator::parameters& 
      (chrominanceModelEstimator::parameters::* p_copy)
      (const chrominanceModelEstimator::parameters&) =
      chrominanceModelEstimator::parameters::copy;
    (this->*p_copy)(other);
# endif

      objectProbabilities = other.objectProbabilities;
      autoObjectProbabilities = other.autoObjectProbabilities;
      defaultProbability = other.defaultProbability;
      excludeBackground = other.excludeBackground;

    return *this;
  }

  // alias for copy member
  chrominanceMapEstimator::parameters&
    chrominanceMapEstimator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* chrominanceMapEstimator::parameters::clone() const {
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
  bool chrominanceMapEstimator::parameters::write(ioHandler& handler,
						 const bool complete) const
# else
  bool chrominanceMapEstimator::parameters::writeMS(ioHandler& handler,
						   const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"objectProbabilities",objectProbabilities);
      lti::write(handler,"autoObjectProbabilities",autoObjectProbabilities);
      lti::write(handler,"defaultProbability",defaultProbability);
      lti::write(handler,"excludeBackground",excludeBackground);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && chrominanceModelEstimator::parameters::write(handler,false);
# else
    bool (chrominanceModelEstimator::parameters::* p_writeMS)
      (ioHandler&,const bool) const =
      chrominanceModelEstimator::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool chrominanceMapEstimator::parameters::write(ioHandler& handler,
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
  bool chrominanceMapEstimator::parameters::read(ioHandler& handler,
						const bool complete)
# else
  bool chrominanceMapEstimator::parameters::readMS(ioHandler& handler,
						  const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"objectProbabilities",objectProbabilities);
      lti::read(handler,"autoObjectProbabilities",autoObjectProbabilities);
      lti::read(handler,"defaultProbability",defaultProbability);
      lti::read(handler,"excludeBackground",excludeBackground);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && chrominanceModelEstimator::parameters::read(handler,false);
# else
    bool (chrominanceModelEstimator::parameters::* p_readMS)
      (ioHandler&,const bool) =
      chrominanceModelEstimator::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool chrominanceMapEstimator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // chrominanceMapEstimator
  // --------------------------------------------------

  // default constructor
  chrominanceMapEstimator::chrominanceMapEstimator()
    : chrominanceModelEstimator() {

    m_constructMap = true;
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  chrominanceMapEstimator::chrominanceMapEstimator(const parameters& par)
      : chrominanceModelEstimator() 
  {
      m_constructMap = true;
      setParameters(par);
  }


  // copy constructor
  chrominanceMapEstimator
  ::chrominanceMapEstimator(const chrominanceMapEstimator& other)
    : chrominanceModelEstimator() {
    copy(other);
  }

  // destructor
  chrominanceMapEstimator::~chrominanceMapEstimator() {
  }

  // returns the name of this type
  const char* chrominanceMapEstimator::getTypeName() const {
    return "chrominanceMapEstimator";
  }

  // copy member
  chrominanceMapEstimator&
    chrominanceMapEstimator::copy(const chrominanceMapEstimator& other) {
      chrominanceModelEstimator::copy(other);
    
      m_colorMap = other.m_colorMap;
      m_probabilityMap = other.m_probabilityMap;
      m_constructMap = other.m_constructMap;
      return (*this);
  }

  // alias for copy member
  chrominanceMapEstimator& chrominanceMapEstimator
  ::operator=(const chrominanceMapEstimator& other) {
    return (copy(other));
  }


  // clone member
  functor* chrominanceMapEstimator::clone() const {
    return new chrominanceMapEstimator(*this);
  }

  // return parameters
  const chrominanceMapEstimator::parameters&
    chrominanceMapEstimator::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // return writable parameters
  chrominanceMapEstimator::parameters& 
    chrominanceMapEstimator::getWritableParameters() {
    parameters* par = 
      dynamic_cast<parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw functor::invalidParametersException();
    }
    return *par;
  }

  // -------------------------------------------------------------------

  bool chrominanceMapEstimator::setParameters(const parameters& theParams) 
  {
      if ( !theParams.autoObjectProbabilities &&
	   theParams.objectProbabilities.size() != theParams.numberOfModels ) 
      {
	  setStatusString("setParameters: wrong number of models");
	  if ( m_constructMap ) {
	      throw exception(getStatusString());
	  }
	  return false;
      }

    if ( !chrominanceModelEstimator::setParameters(theParams) ) {
      if ( m_constructMap ) {
	throw exception(getStatusString());
	return false;
      }
    }
  
    //reset m_constructMap -> we don't need it anymore
    m_constructMap = false;

    //check whether the sum of the object probabilities is one
    parameters& par = getWritableParameters(); 
    float probSum ( 0 );
    int i ( 0 );
    if ( !par.autoObjectProbabilities ) {
      for ( i=0; i < par.numberOfModels; i++ ) {
	probSum += max(par.objectProbabilities[i],
		       -par.objectProbabilities[i]); 
      }
      if ( probSum < 0 ) {
	probSum *= -1;
      }
    }
    if ( probSum < std::numeric_limits<float>::epsilon() ||
	 par.autoObjectProbabilities ) {
      par.autoObjectProbabilities = true;
      const float val ( 1.f / float(par.numberOfModels) );
      if ( par.objectProbabilities.size() != par.numberOfModels ) {
	par.objectProbabilities.resize(par.numberOfModels, val, false, true);
      } else {
	  for ( i = 0; i < par.numberOfModels; i++ ) {
	  par.objectProbabilities[i] = val;
	}
      }

    } else if ( probSum > 1.f + std::numeric_limits<float>::epsilon() ||
		probSum < 1.f - std::numeric_limits<float>::epsilon() ) {
      //normalize
      for ( i = 0; i < par.objectProbabilities.size(); i++ ) {
	par.objectProbabilities[i] /= probSum;
      }
    }

    //rescale the map to be estimated
    if ( m_numEntries != m_colorMap.columns() ||
	 m_numEntries != m_colorMap.rows() ) {
      m_colorMap.resize(m_numEntries, m_numEntries);
      m_probabilityMap.resize(m_numEntries, m_numEntries);
    }
    
    return true;
  }

  // -------------------------------------------------------------------

   const channel8& chrominanceMapEstimator::getChrominanceMap() const {
    return m_colorMap;
  }
  
  const channel& chrominanceMapEstimator::getProbabilityMap() const {
    return m_probabilityMap;
  }

  // -------------------------------------------------------------------

  bool chrominanceMapEstimator::update() {

    const parameters& par = getParameters();
    if ( par.excludeBackground ) {
      return updateExclude();
    }
  
    return updateAll();
  }
  
  // -------------------------------------------------------------------

  void chrominanceMapEstimator
  ::initUpdate(std::vector<histogram2D::const_iterator>& histIts,
	       dvector& objNumEntries) const {

    dvector::iterator dit ( objNumEntries.begin() );
    std::vector<histogram2D>::const_iterator hend ( m_chromHistos.end() );
    std::vector<histogram2D>::const_iterator hit  ( m_chromHistos.begin() );
    for ( ; hit!=hend; ++dit, ++hit ) {
      //avoid division by 0
      *dit =  max( 1., (*hit).getNumberOfEntries() );
      histIts.push_back((*hit).begin());
    }
  }

  // -------------------------------------------------------------------

  bool chrominanceMapEstimator::updateExclude() {

    //get number of entries in the histograms
    const parameters& par = getParameters(); 
    const overlappingSets2D& checker = par.checker;

    dvector objNumEntries ( par.numberOfModels );
    std::vector<histogram2D::const_iterator> histIts;
    initUpdate(histIts, objNumEntries);
 
    int v ( 0 );
    const int numEntries ( m_colorMap.size().x );
    std::vector<histogram2D::const_iterator>::iterator cend ( histIts.end() );
    channel8::iterator objIt       ( m_colorMap.begin() );
    channel::iterator      probIt ( m_probabilityMap.begin() );
    for ( ; v < numEntries; ++v ) {
      int u ( 0 );
      for ( ; u < numEntries; ++u, ++objIt, ++probIt ) {
     
	*probIt = 0.f;
	*objIt = 0;                      //default: background
	
	//only consider values that belong to interesting objects
	if ( checker.doDiscard(u,v) ) {
	  //increase the histogram iterators
	  std::vector<histogram2D::const_iterator>::iterator
	    cit ( histIts.begin() );
	  for ( ; cit!= cend; ++cit ) {
	    ++(*cit);
	  }
	  continue;
	}
	//compute P(u,v,o) for all objects o
	//and the sum
	double probSum ( 0. );
	dvector andProbs(par.numberOfModels, 0.);
	dvector::iterator ait ( andProbs.begin() );
	fvector::const_iterator oit ( par.objectProbabilities.begin() );
	dvector::const_iterator nit ( objNumEntries.begin() );
	std::vector<histogram2D::const_iterator>::iterator
	  cit ( histIts.begin() );
	for ( ; cit!= cend; ++cit,++ait,++oit,++nit ) {
	  //avoid classifying unlikely objects ( with prob. close to one )
	  *ait = (*(*cit)) / (*nit) * (*oit);
	  ++(*cit);
	  if ( *ait < par.defaultProbability ) {
	    *ait = par.defaultProbability;
	  }
	  probSum += *ait;
	}
	
	//get the best object
	dvector::iterator aend ( andProbs.end() );
	int i ( 0 );
	for ( ait = andProbs.begin(); ait!= aend; ++ait,++i ) {
	  float tmp ( static_cast<float>((*ait) / probSum) ); //histo is double
	  if ( tmp > *probIt ) {
	    *probIt = tmp;   //this is a channel (float)
	    *objIt = i;
	  }
	}
      }
    }
    
    return true;
  }

  // -------------------------------------------------------------------

  //compute the color and probability map
  bool chrominanceMapEstimator::updateAll() {

    //get number of entries in the histograms
    const parameters& par = getParameters();
    dvector objNumEntries ( par.numberOfModels );
    std::vector<histogram2D::const_iterator> histIts;
    initUpdate(histIts, objNumEntries);

    std::vector<histogram2D::const_iterator>::iterator cend ( histIts.end() );
    channel8::iterator objEnd      ( m_colorMap.end()   );
    channel8::iterator objIt       ( m_colorMap.begin() );
    channel::iterator      probIt ( m_probabilityMap.begin() );
    for ( ; objIt!=objEnd; ++objIt, ++probIt ) {

      //compute P(u,v,o) for all objects o
      //and the sum
      double probSum ( 0. );
      dvector andProbs(par.numberOfModels, 0.);
      dvector::iterator ait ( andProbs.begin() );
      fvector::const_iterator oit ( par.objectProbabilities.begin() );
      dvector::const_iterator nit ( objNumEntries.begin() );
      std::vector<histogram2D::const_iterator>::iterator
	cit ( histIts.begin() );
      for ( ; cit!= cend; ++cit,++ait,++oit,++nit ) {
	//avoid classifying unlikely objects ( with prob. close to one )
	*ait = (*(*cit)) / (*nit) * (*oit);
	++(*cit);
	if ( *ait < par.defaultProbability ) {
	  *ait = par.defaultProbability;
	}
	probSum += *ait;
      }
      
      //get the best object
      *probIt = 0.f;
      *objIt = 0;                      //default: background
      dvector::iterator aend ( andProbs.end() );
      int i ( 0 );
      for ( ait = andProbs.begin(); ait!= aend; ++ait,++i ) {
	float tmp ( static_cast<float>((*ait) / probSum) ); //histo is double
	if ( tmp > *probIt ) {
	  *probIt = tmp;   //this is a channel (float)
	  *objIt = i;
	}
      }
    }

    return true;
  }

  // -------------------------------------------------------------------

  bool chrominanceMapEstimator
  ::writeChrominanceMap(const std::string& prefix,
			const vector<rgbPixel>& colors) const {
    
    std::string name(prefix + "_objects.bmp");
    lti::saveImage mySaver;
    
    image img ( m_colorMap.size() );
    image::iterator end ( img.end() );
    image::iterator it ( img.begin() );
    channel8::const_iterator mIt ( m_colorMap.begin() );
    int colSize ( colors.size() );
    for ( ; it != end; ++it,++mIt) {
      if ( *mIt < colSize ) 
	*it = colors[*mIt];
    }
    bool success ( mySaver.save(name,img) );

    name = prefix + "_prob.bmp";
    success &= mySaver.save(name,m_probabilityMap);

    return true;
  }

}//namespace

#endif
