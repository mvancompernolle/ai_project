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
 * file .......: ltiChrominanceMapMasking.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 14.2.2004
 * revisions ..: $Id: ltiChrominanceMapMasking.cpp,v 1.5 2006/09/05 10:05:13 ltilib Exp $
 */

#include "ltiRGBPixel.h"
#include "ltiChrominanceMapMasking.h"
#include "ltiALLFunctor.h"
#include <fstream>

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // chrominanceMapMasking::parameters
  // --------------------------------------------------

  // default constructor
  chrominanceMapMasking::parameters::parameters()
    : modifier::parameters() {
 
    blackLuminance = ubyte(50);
    whiteLuminance = ubyte(180);
    achromaticLabel = int(0);
    whiteLabel = int(100);
    greyLabel  = int(101);
    blackLabel = int(102);
    shrinkFactor = int(1);
  }

  // copy constructor
  chrominanceMapMasking::parameters::parameters(const parameters& other)
    : modifier::parameters() {
    copy(other);
  }

  // destructor
  chrominanceMapMasking::parameters::~parameters() {
  }

  // get type name
  const char* chrominanceMapMasking::parameters::getTypeName() const {
    return "chrominanceMapMasking::parameters";
  }

  // copy member

  chrominanceMapMasking::parameters&
    chrominanceMapMasking::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    modifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    modifier::parameters& (modifier::parameters::* p_copy)
      (const modifier::parameters&) =
      modifier::parameters::copy;
    (this->*p_copy)(other);
# endif

    blackLuminance = other.blackLuminance;
    whiteLuminance = other.whiteLuminance;
    achromaticLabel = other.achromaticLabel;
    whiteLabel = other.whiteLabel;
    greyLabel = other.greyLabel;
    blackLabel = other.blackLabel;
    shrinkFactor = other.shrinkFactor;

    return *this;
  }

  // alias for copy member
  chrominanceMapMasking::parameters&
    chrominanceMapMasking::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* chrominanceMapMasking::parameters::clone() const {
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
  bool chrominanceMapMasking::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool chrominanceMapMasking::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"blackLuminance",blackLuminance);
      lti::write(handler,"whiteLuminance",whiteLuminance);
      lti::write(handler,"achromaticLabel",achromaticLabel);
      lti::write(handler,"whiteLabel",whiteLabel);
      lti::write(handler,"greyLabel",greyLabel);
      lti::write(handler,"blackLabel",blackLabel);
      lti::write(handler,"shrinkFactor",shrinkFactor);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::write(handler,false);
# else
    bool (modifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      modifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool chrominanceMapMasking::parameters::write(ioHandler& handler,
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
  bool chrominanceMapMasking::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool chrominanceMapMasking::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
 
      lti::read(handler,"blackLuminance",blackLuminance);
      lti::read(handler,"whiteLuminance",whiteLuminance);
      lti::read(handler,"achromaticLabel",achromaticLabel);
      lti::read(handler,"whiteLabel",whiteLabel);
      lti::read(handler,"greyLabel",greyLabel);
      lti::read(handler,"blackLabel",blackLabel);
      lti::read(handler,"shrinkFactor",shrinkFactor);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::read(handler,false);
# else
    bool (modifier::parameters::* p_readMS)(ioHandler&,const bool) =
      modifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool chrominanceMapMasking::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // map handling
  // --------------------------------------------------

  bool chrominanceMapMasking::setChrominanceMap(const channel8& map) {

    if ( map.size().x == 0 ||
	 map.size().y == 0 ||
	 map.size().x != map.size().y ) {
      return false;
    }

    if ( m_destroyChrominance && notNull(m_chrominanceMap) ) {
      _lti_debug("setCM: destroying ");
      delete m_chrominanceMap;
      m_chrominanceMap = 0;
    }
    m_destroyChrominance = true;
    m_chrominanceMap = new channel8(map);
    
    setShift();
    return true;
  }

  bool chrominanceMapMasking::useExternalChrominanceMap(const channel8* map) {
    
    if ( map->size().x == 0 ||
	 map->size().y == 0 ||
	 map->size().x != map->size().y ) {
      return false;
    }

    if ( m_destroyChrominance && notNull(m_chrominanceMap) ) {
      _lti_debug("useCM: destroying ");
      delete m_chrominanceMap;
      m_chrominanceMap = 0;
    }
    m_destroyChrominance = false;
    m_chrominanceMap = map;
    
    setShift();
    return true;
  }

  bool chrominanceMapMasking::setProbabilityMap(const channel& map) {
     
    if ( map.size().x == 0 ||
	 map.size().y == 0 ||
	 map.size().x != map.size().y ) {
      return false;
    }

    if ( m_destroyProbability && notNull(m_probabilityMap) ) {
      delete m_probabilityMap;
      m_probabilityMap = 0;
    }
    m_destroyProbability = true;
    m_probabilityMap = new channel(map);
    return true;
  }
  
  bool chrominanceMapMasking::useExternalProbabilityMap(const channel* map) {
    
    if ( map->size().x == 0 ||
	 map->size().y == 0 ||
	 map->size().x != map->size().y ) {
      return false;
    }

    if ( m_destroyProbability && notNull(m_probabilityMap) ) {
      delete m_probabilityMap;
      m_probabilityMap = 0;
    }
    m_destroyProbability = false;
    m_probabilityMap = map;
    return true;
  }

  const channel8& chrominanceMapMasking::getChrominanceMap() const {
    if ( !notNull(m_chrominanceMap) ) {
      throw exception("Invalid chrominance map");
    }
    return *m_chrominanceMap;
  }

  const channel& chrominanceMapMasking::getProbabilityMap() const {
    if ( !notNull(m_probabilityMap) ) {
      throw exception("Invalid chrominance map");
    }
    return *m_probabilityMap;
  }

  // --------------------------------------------------

  bool chrominanceMapMasking::isChrominanceMapValid() const {
    return ( notNull(m_chrominanceMap) &&
	     m_chrominanceMap->size().x != 0 &&
	     m_chrominanceMap->size().y != 0 &&
	     m_chrominanceMap->size().x == m_chrominanceMap->size().y );
  }
  
  bool chrominanceMapMasking::isProbabilityMapValid() const {
    return ( notNull(m_probabilityMap) &&
	     m_probabilityMap->size().x != 0 &&
	     m_probabilityMap->size().y != 0 &&
	     m_probabilityMap->size().x == m_probabilityMap->size().y );
  }
  
  bool chrominanceMapMasking::areMapsConsistent() const {
    return ( isChrominanceMapValid() &&
	     isProbabilityMapValid() &&
	     m_chrominanceMap->size() == m_probabilityMap->size() );
  }

  // --------------------------------------------------
  
  void chrominanceMapMasking::setShift() {
    
    const int scale ( 256 / m_chrominanceMap->size().x );
    int base = 1;
    m_shift = 0;
    while ( base <= scale ) {
      base *= 2;
      m_shift++;
    }
    m_shift--;
  }

  int chrominanceMapMasking::getShift() const {
    return m_shift;
  }

  // --------------------------------------------------

  bool chrominanceMapMasking
  ::readChrominanceMap(const std::string& filename,
		       const std::vector<rgbPixel>& colors) {

    std::ifstream infile(filename.c_str());
    if (!infile.is_open()) {
      setStatusString("Cannot open file");
      return false;
    }

    lti::image img;
    lti::loadImage loader;
    loader.load(filename, img);
    if ( img.size().x == 0 ||
	 img.size().y == 0 ||
	 img.size().x != img.size().y ) {
      return false;
    }
  
    channel8 tmp ( img.size() );
    image::iterator    tit ( img.begin() );
    channel8::iterator it  ( tmp.begin() );
    channel8::iterator end ( tmp.end()   ); 
    for ( ; it != end; ++it,++tit ) { 
      int index ( 0 );
      std::vector<rgbPixel>::const_iterator cit  ( colors.begin() );
      std::vector<rgbPixel>::const_iterator cend ( colors.end() );
      for ( ; cit!=cend; ++cit, ++index ) {
	if ( *tit == *cit ) *it = index;
      }
    }
    if ( m_destroyChrominance && notNull(m_chrominanceMap) ) { 
      delete m_chrominanceMap;
      m_chrominanceMap = 0;
    }
    m_destroyChrominance = true,
    m_chrominanceMap = new channel8(true,tmp,0,tmp.rows(),0,tmp.columns());
    setShift();

    _lti_debug("readCM: destroyChrom "<<m_destroyChrominance<<" size "
	       << m_chrominanceMap->size());
    return true;
  }

  // --------------------------------------------------
  // chrominanceMapMasking
  // --------------------------------------------------

  // default constructor
  chrominanceMapMasking::chrominanceMapMasking()
    : modifier(){

    m_probabilityMap = 0;
    m_chrominanceMap = 0;
    m_destroyProbability = bool(false);
    m_destroyChrominance = bool(false);
    m_shift = int(0);

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  chrominanceMapMasking::chrominanceMapMasking(const parameters& par)
    : modifier() {

    m_probabilityMap = 0;
    m_chrominanceMap = 0;
    m_destroyProbability = bool(false);
    m_destroyChrominance = bool(false);
    m_shift = int(0);

    // set the given parameters
    setParameters(par);
  }

  // copy constructor
  chrominanceMapMasking
  ::chrominanceMapMasking(const chrominanceMapMasking& other)
    : modifier() {

    m_probabilityMap = 0;
    m_chrominanceMap = 0;
    m_destroyProbability = bool(false);
    m_destroyChrominance = bool(false);
    m_shift = int(0);

    copy(other);
  }

  // destructor
  chrominanceMapMasking::~chrominanceMapMasking() {
    if ( m_destroyProbability && notNull(m_probabilityMap) ) {
      delete m_probabilityMap;
      m_probabilityMap = 0;
    }
    if ( m_destroyChrominance && notNull(m_chrominanceMap) ) {
      delete m_chrominanceMap;
      m_chrominanceMap = 0;
    }
  }

  // returns the name of this type
  const char* chrominanceMapMasking::getTypeName() const {
    return "chrominanceMapMasking";
  }

  // copy member
  chrominanceMapMasking&
    chrominanceMapMasking::copy(const chrominanceMapMasking& other) {
      modifier::copy(other);
 
      if ( m_destroyProbability && notNull(m_probabilityMap) ) {
	delete m_probabilityMap;
	m_probabilityMap = 0;
      }
      if ( m_destroyChrominance && notNull(m_chrominanceMap) ) {
	delete m_chrominanceMap;
	m_chrominanceMap = 0;
      }
      m_destroyProbability = true; //deep copy
      if ( m_destroyProbability && notNull(other.m_probabilityMap)) {
	m_probabilityMap = new channel(*other.m_probabilityMap);
      }
      m_destroyChrominance = true; //deep copy
      if ( m_destroyChrominance && notNull(other.m_chrominanceMap)) {
	m_chrominanceMap = new channel8(*other.m_chrominanceMap);
      }  

      m_shift = other.m_shift;
      return (*this);
  }

  // alias for copy member
  chrominanceMapMasking&
    chrominanceMapMasking::operator=(const chrominanceMapMasking& other) {
    return (copy(other));
  }

  // clone member
  functor* chrominanceMapMasking::clone() const {
    return new chrominanceMapMasking(*this);
  }

  // return parameters
  const chrominanceMapMasking::parameters&
    chrominanceMapMasking::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  bool chrominanceMapMasking::checkInput(const channel8& y,
					 const channel8& u,
					 const channel8& v) {
    if ( y.size() == u.size() &&
	 u.size() == v.size() ) {
      return true;
    } else {
      setStatusString("apply: input must have same size");
      return false;
    }
  }

  bool chrominanceMapMasking::checkInput(const channel8& y,
					 const channel8& u) {
    if ( y.size() == u.size() ) {
      return true;
    } else {
      setStatusString("apply: input must have same size");
      return false;
    }
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool chrominanceMapMasking::apply(const channel8& y,
				    const channel8& u,
				    const channel8& v,
				    channel8& dest) {
    
    if ( !checkInput(y,u,v) ) {
      return false;
    }
    const parameters& par = getParameters();
    if ( !isChrominanceMapValid() ) {
      setStatusString("apply: Invalid chrominance map");
      return false;
    }
    _lti_debug(" chromMask size " << m_chrominanceMap->size() 
	       << " shift " << getShift() );
    const int off   ( par.shrinkFactor > 0 ?
		      par.shrinkFactor :
		      1 );
    const ipoint newSize ( fpoint(y.size()) / float(off) + fpoint(.5,.5) );
    dest.resize( newSize );
    channel8::iterator dit     ( dest.begin() );

    const int shift ( getShift() );

    int row ( 0 );
    const int maxRow ( y.size().y );
    for ( ; row<maxRow; row+=off ) {
      vector<ubyte>::const_iterator yit  ( y.getRow(row).begin() );
      vector<ubyte>::const_iterator uit  ( u.getRow(row).begin() );
      vector<ubyte>::const_iterator vit  ( v.getRow(row).begin() );
      vector<ubyte>::const_iterator vend ( v.getRow(row).end()   );
      for ( ; vit < vend; vit+=off, yit+=off, uit+=off, ++dit ) {
	*dit = m_chrominanceMap->at(*vit>>shift, *uit>>shift);
	if ( *dit ==  par.achromaticLabel ) {
	  if ( *yit > par.whiteLuminance ) {
	    *dit = par.whiteLabel;
	  } else if ( *yit < par.blackLuminance ) {
	    *dit = par.blackLabel;
	  } else {
	    *dit = par.greyLabel;
	  }
	}
      }
    }
    return true;
  }

  bool chrominanceMapMasking::apply(const channel8& y,
				    const channel8& u,
				    const channel8& v,
				    channel8& dest, channel& prob) {

    if ( !checkInput(y,u,v) ) {
      return false;
    }
    const parameters& par = getParameters();
    if ( !areMapsConsistent() ) {
      setStatusString("apply: Inconsistent chrominance and probability map");
      return false;
    }
    const int off   ( par.shrinkFactor > 0 ?
		      par.shrinkFactor :
		      1 );
    const ipoint newSize ( fpoint(y.size()) / float(off) + fpoint(.5,.5) );
    dest.resize( newSize );
    channel8::iterator dit     ( dest.begin() );
    prob.resize( newSize );
    channel::iterator pit     ( prob.begin() );

    const int shift ( getShift() );
 
    int row ( 0 );
    const int maxRow ( y.size().y );
    for ( ; row<maxRow; row+=off ) {
      vector<ubyte>::const_iterator yit  ( y.getRow(row).begin() );
      vector<ubyte>::const_iterator uit  ( u.getRow(row).begin() );
      vector<ubyte>::const_iterator vit  ( v.getRow(row).begin() );
      vector<ubyte>::const_iterator vend ( v.getRow(row).end()   );
      for ( ; vit < vend; vit+=off, yit+=off, uit+=off, ++dit, ++pit ) {
	*pit = m_probabilityMap->at(*vit>>shift, *uit>>shift);
	*dit = m_chrominanceMap->at(*vit>>shift, *uit>>shift);
	if ( *dit ==  par.achromaticLabel ) {
	  if ( *yit > par.whiteLuminance ) {
	    *dit = par.whiteLabel;
	  } else if ( *yit < par.blackLuminance ) {
	    *dit = par.blackLabel;
	  } else {
	    *dit = par.greyLabel;
	  }
	}
      }
    }
    return true;
  }

  bool chrominanceMapMasking
  ::apply(const channel8& u, const channel8& v, channel8& dest) {

   if ( !checkInput(u,v) ) {
      return false;
    }
    const parameters& par = getParameters();
    if ( !isChrominanceMapValid() ) {
      setStatusString("apply: Invalid chrominance map");
      return false;
    }
    
    const int off   ( par.shrinkFactor > 0 ?
		      par.shrinkFactor :
		      1 );
    const ipoint newSize ( fpoint(u.size()) / float(off) + fpoint(.5,.5) );
    dest.resize( newSize );
    channel8::iterator dit     ( dest.begin() );
 
    const int shift ( getShift() );
 
    int row ( 0 );
    const int maxRow ( u.size().y );
    for ( ; row<maxRow; row+=off ) {
      vector<ubyte>::const_iterator uit  ( u.getRow(row).begin() );
      vector<ubyte>::const_iterator vit  ( v.getRow(row).begin() );
      vector<ubyte>::const_iterator vend ( v.getRow(row).end()   );
      for ( ; vit < vend; vit+=off, uit+=off, ++dit ) {
	*dit = m_chrominanceMap->at(*vit>>shift, *uit>>shift);
      }
    }
    return true;
  }

  bool chrominanceMapMasking
  ::apply(const channel8& u, const channel8& v,
	  channel8& dest, channel& prob) {

  if ( !checkInput(u,v) ) {
      return false;
    }
    const parameters& par = getParameters();
    if ( !isChrominanceMapValid() ) {
      setStatusString("apply: Invalid chrominance map");
      return false;
    }
    
    const int off   ( par.shrinkFactor > 0 ?
		      par.shrinkFactor :
		      1 );
    const ipoint newSize ( fpoint(u.size()) / float(off) + fpoint(.5,.5) );
    dest.resize( newSize );
    channel8::iterator dit     ( dest.begin() );
    prob.resize( newSize );
    channel::iterator pit     ( prob.begin() );

    const int shift ( getShift() );
 
    int row ( 0 );
    const int maxRow ( u.size().y );
    for ( ; row<maxRow; row+=off ) {
      vector<ubyte>::const_iterator uit  ( u.getRow(row).begin() );
      vector<ubyte>::const_iterator vit  ( v.getRow(row).begin() );
      vector<ubyte>::const_iterator vend ( v.getRow(row).end()   );
      for ( ; vit < vend; vit+=off, uit+=off, ++dit ) {
	*pit = m_probabilityMap->at(*vit>>shift, *uit>>shift);
	*dit = m_chrominanceMap->at(*vit>>shift, *uit>>shift);
      }
    }
    return true;
  }

}
#include "ltiUndebug.h"
