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
 * file .......: ltiOverlappingSets2D.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 8.2.2004
 * revisions ..: $Id: ltiOverlappingSets2D.cpp,v 1.5 2006/09/05 10:24:49 ltilib Exp $
 */

#include "ltiMacroSymbols.h"

// only for compilers different than VC++ 6.0 available
#ifndef _LTI_MSC_6

#include "ltiRGBPixel.h"
#include "ltiOverlappingSets2D.h"
#include <limits>
#include <fstream>
#include "ltiALLFunctor.h"
#include "ltiSplitImageTorgI.h"
#include "ltiScaling.h"

namespace lti {
  // --------------------------------------------------
  // overlappingSets2D::parameters
  // --------------------------------------------------

  // default constructor
  overlappingSets2D::parameters::parameters()
    : functor::parameters() {
    discardThreshold = int(0);
    size = ipoint(256,256);
  }

  // copy constructor
  overlappingSets2D::parameters::parameters(const parameters& other)
    : functor::parameters() {
    copy(other);
  }

  // destructor
  overlappingSets2D::parameters::~parameters() {
  }

  // get type name
  const char* overlappingSets2D::parameters::getTypeName() const {
    return "overlappingSets2D::parameters";
  }

  // copy member

  overlappingSets2D::parameters&
    overlappingSets2D::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters& (functor::parameters::* p_copy)
      (const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
# endif
    
      discardThreshold = other.discardThreshold;
      size = other.size;
    return *this;
  }

  // alias for copy member
  overlappingSets2D::parameters&
    overlappingSets2D::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* overlappingSets2D::parameters::clone() const {
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
  bool overlappingSets2D::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool overlappingSets2D::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"discardThreshold",discardThreshold);
      lti::write(handler,"size",size);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::write(handler,false);
# else
    bool (functor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      functor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool overlappingSets2D::parameters::write(ioHandler& handler,
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
  bool overlappingSets2D::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool overlappingSets2D::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"discardThreshold",discardThreshold);
      lti::read(handler,"size",size);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::read(handler,false);
# else
    bool (functor::parameters::* p_readMS)(ioHandler&,const bool) =
      functor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool overlappingSets2D::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // overlappingSets2D
  // --------------------------------------------------

  // default constructor
  overlappingSets2D::overlappingSets2D()
    : functor(),
      m_objectTable(0,0),
      m_id()  {
    // create an instance of the parameters with the default values
    m_discardThreshold = 0;
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  overlappingSets2D::overlappingSets2D(const parameters& par)
    : functor(),
      m_objectTable(0,0),
      m_id()  {
    m_discardThreshold = 0;
    // set the given parameters
    setParameters(par);
  }

  // copy constructor
  overlappingSets2D::overlappingSets2D(const overlappingSets2D& other)
    : functor() {
    copy(other);
  }

  // destructor
  overlappingSets2D::~overlappingSets2D() {
  }

  // returns the name of this type
  const char* overlappingSets2D::getTypeName() const {
    return "overlappingSets2D";
  }

  // copy member
  overlappingSets2D&
    overlappingSets2D::copy(const overlappingSets2D& other) {
      functor::copy(other);
      m_objectTable = other.m_objectTable;
      m_id = other.m_id;
      m_discardThreshold = other.m_discardThreshold;
    return (*this);
  }

  // alias for copy member
  overlappingSets2D&
    overlappingSets2D::operator=(const overlappingSets2D& other) {
    return (copy(other));
  }


  // clone member
  functor* overlappingSets2D::clone() const {
    return new overlappingSets2D(*this);
  }

  // return parameters
  const overlappingSets2D::parameters&
    overlappingSets2D::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------

  bool overlappingSets2D::setParameters(const parameters& theParams) {
    
    bool success ( true );
    success &= functor::setParameters(theParams);
    m_discardThreshold = theParams.discardThreshold;

    //ensure that the object table has the right size
    success &= resize();

    return success;
  }

  // -------------------------------------------------------------------
  // some more parameter methods
  // -------------------------------------------------------------------
  const imatrix& overlappingSets2D::getObjectTable() const {
    return m_objectTable;
  }

  // -------------------------------------------------------------------
  bool overlappingSets2D::initObjectTable(const std::vector<channel8>& src) {

    const parameters& par = getParameters();
    m_objectTable.resize(par.size, 0, false, true); //also init if same size
    m_id.clear();
 
    const int halfMax ( std::numeric_limits<int>::max() / 2 );
    int label ( 0 );
    int id ( 1 );
    std::vector<channel8>::const_iterator end ( src.end()   );
    std::vector<channel8>::const_iterator it  ( src.begin() );
    for ( ; it != end; ++it, ++label ) {//all input
      
      //update the id table
      m_id.push_back(id);
      imatrix::iterator dend       ( m_objectTable.end()    );
      imatrix::iterator  dit       ( m_objectTable.begin()  );
      channel8::const_iterator iit ( (*it).begin() );
      for ( ;dit!=dend; ++dit,++iit ) { //entire object-table
	if ( *iit > 0 ) { //if object
	  //update the isObject-table
	  *dit += id;
	}//if object
      }//entire object-table
      
      //create a new id
      if ( id >= halfMax ) {
	return false;
      }
      id *= 2;
    }//all input

#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
    viewer::parameters vpar;
    vpar.title = "object table";
    vpar.contrast = 1.5;
    vpar.brightness = .4;
    viewer viewIsOT(vpar);
    viewIsOT.show(m_objectTable);
    getchar();
#endif

    return true;
  }
  
  // -------------------------------------------------------------------
 
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
  bool overlappingSets2D
  ::getAllContainedSets(int value, std::list<int>& dest) {
 
    //get the largest multiple of 2 that is smaller than value
    int id ( 1 );
    while ( id < value ) {
      id *= 2;
    }
    if ( id > value ) {
      id = id>>1;
    }

    dest.clear();
    while ( value > 0 ) {
      dest.push_front(id);
      value -= id;
      id = 1;
      while ( id < value ) {
	id *= 2;
      }
      if ( id > value )  {
	id = id>>1;
      }
    }

    return true;
  }
#endif
  // -------------------------------------------------------------------

  bool overlappingSets2D::setObjectTable(const std::vector<channel8>& src) {
   
    const parameters& par = getParameters();
    //scale the input vector
    std::vector<channel8> scaledSrc;
    std::vector<channel8>::const_iterator end ( src.end()   );
    std::vector<channel8>::const_iterator it  ( src.begin() );
    for ( ; it!=end; ++it ) {
      const ipoint& srcSize = (*it).size();
      if ( par.size == srcSize ) {
	scaledSrc.push_back(*it);
      } else if ( srcSize.x == 0 || srcSize.y == 0 ) {
	//no image -> create empty image
	channel8 tmp ( par.size, ubyte(0) );
	scaledSrc.push_back(tmp);
      } else {
	//scale
	channel8 tmp;
	scale(*it, tmp);
	scaledSrc.push_back(tmp);
      }
    }
 
    return initObjectTable(scaledSrc);
  }

  bool overlappingSets2D::setObjectTable(const std::vector<image>& src) {
   
    const parameters& par = getParameters();
    //scale the input vector
    std::vector<channel8> scaledSrc;
    std::vector<image>::const_iterator end ( src.end()   );
    std::vector<image>::const_iterator it  ( src.begin() );
    for ( ; it!=end; ++it ) {
      const ipoint& srcSize = (*it).size();
      if ( srcSize.x == 0 || srcSize.y == 0 ) {
	//no image -> create empty image
	channel8 tmp ( par.size, ubyte(0) );
	scaledSrc.push_back(tmp);
      } else {
	channel8 tmp;
	splitImageTorgI splitter;
	splitter.getIntensity(*it, tmp);
	if ( par.size == srcSize ) {
	  scaledSrc.push_back(tmp);
	} else {
	  //scale
	  channel8 tmp;
	  splitImageTorgI splitter;
	  splitter.getIntensity(*it,tmp);
	  scale(tmp);
	  scaledSrc.push_back(tmp);
	}
      }
    }
 
    return initObjectTable(scaledSrc);
  }

  // -------------------------------------------------------------------
  
  bool overlappingSets2D::scale(const channel8& src, channel8& dest) const {

    const parameters& par = getParameters();
    scaling::parameters spar;
    spar.scale = tpoint<float>(par.size) / tpoint<float>(src.size());
    spar.interpolatorType = NearestNeighborInterpolator;
    scaling scaler(spar);
    return scaler.apply(src, dest);
  }

  bool overlappingSets2D::scale(channel8& src) const {
    const parameters& par = getParameters();
    scaling::parameters spar;
    spar.scale = tpoint<float>(par.size) / tpoint<float>(src.size());
    spar.interpolatorType = NearestNeighborInterpolator;
    scaling scaler(spar);
    return scaler.apply(src);
  }
    
  bool overlappingSets2D::resize() {
    
    const parameters& par = getParameters();
    const point& otSize = m_objectTable.size();
    //nothing to do if correct size or empty LUT
    if ( par.size == otSize || otSize.x == 0 || otSize.y == 0 ) {
      return true;
    } 
    scaling::parameters spar;
    spar.scale = tpoint<float>(par.size) / tpoint<float>(otSize);
    spar.interpolatorType = NearestNeighborInterpolator;
    scaling scaler(spar);
    return scaler.apply(m_objectTable);
  }

  // -------------------------------------------------------------------
 
  bool overlappingSets2D
  ::readObjectTable(const std::string& prefix,
		    const std::vector<std::string>& names) {
    
    const int size ( names.size() );
    if ( size == 0 ) {
      return false;
    }
  
    int i ( 0 );
    bool success ( true );
    std::vector<image> images;
    for ( ; i < size; i++ ) {
      //check if the input file exists and can be read
      std::string tmpStr (prefix + names[i]);
      std::ifstream infile(tmpStr.c_str());
      if (!infile.is_open()) {
	success = false;
      } else {
	lti::loadImage loader;
	lti::image img;
	success &= loader.load(tmpStr, img);
	images.push_back(img);
      }
    }

    return setObjectTable(images) && success;
  }

  // -------------------------------------------------------------------

  bool overlappingSets2D::isInitialized() const {

    return ( getObjectTable().size() != ipoint(0,0) );
  }

  // -------------------------------------------------------------------
  // The apply methods
  // -------------------------------------------------------------------
  
  bool overlappingSets2D::apply(int label, int u, int v,
				bool& contained) const {
    if ( !isInitialized() ) {
      setStatusString("overlappingSets2D: internal LUT is not initialized");
      return false;
    }
    contained = isContained(label,u,v);
    return true;
  }

 
  bool overlappingSets2D::apply(int u, int v, bool& discard) const {

    if ( !isInitialized() ) {
      setStatusString("overlappingSets2D: internal LUT is not initialized");
      return false;
    }
    discard = doDiscard(u,v);
    return true;
  }
}

#endif
