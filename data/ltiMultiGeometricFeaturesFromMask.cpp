/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiMultiGeometricFeaturesFromMask.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 31.8.2003
 * revisions ..: $Id: ltiMultiGeometricFeaturesFromMask.cpp,v 1.8 2006/09/05 10:22:54 ltilib Exp $
 */

#include "ltiMultiGeometricFeaturesFromMask.h"

#if defined(_LTI_DEBUG)
using std::cout; 
using std::flush; 
using std::endl;
#endif

namespace lti {
  // --------------------------------------------------
  // multiGeometricFeaturesFromMask::parameters
  // --------------------------------------------------

  // default constructor
  multiGeometricFeaturesFromMask::parameters::parameters()
    : geometricFeaturesFromMask::parameters() {
    
    mergeEntries.resize(0);
    maskValues.resize(0);
    manualPostProcess = false;
  }

  // copy constructor
  multiGeometricFeaturesFromMask::parameters::
  parameters(const parameters& other)
    : geometricFeaturesFromMask::parameters() {
    copy(other);
  }

  // destructor
  multiGeometricFeaturesFromMask::parameters::~parameters() {
  }

  // get type name
  const char* multiGeometricFeaturesFromMask::parameters::getTypeName() const {
    return "multiGeometricFeaturesFromMask::parameters";
  }

  // copy member

  multiGeometricFeaturesFromMask::parameters&
    multiGeometricFeaturesFromMask::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    geometricFeaturesFromMask::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    geometricFeaturesFromMask::parameters&
	(geometricFeaturesFromMask::parameters::* p_copy)
	(const geometricFeaturesFromMask::parameters&) =
	geometricFeaturesFromMask::parameters::copy;
    (this->*p_copy)(other);
# endif

      mergeEntries = other.mergeEntries;
      maskValues = other.maskValues;
      manualPostProcess = other.manualPostProcess;
    return *this;
  }

  // alias for copy member
  multiGeometricFeaturesFromMask::parameters&
    multiGeometricFeaturesFromMask::parameters::
  operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* multiGeometricFeaturesFromMask::
  parameters::clone() const {
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
  bool multiGeometricFeaturesFromMask::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool multiGeometricFeaturesFromMask::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
  
      lti::write(handler,"mergeEntries",mergeEntries);
      lti::write(handler,"maskValues",maskValues);
      lti::write(handler,"manualPostProcess",manualPostProcess);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && geometricFeaturesFromMask::parameters::write(handler,false);
# else
    bool (geometricFeaturesFromMask::parameters::* p_writeMS)
	(ioHandler&,const bool) const =
	geometricFeaturesFromMask::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool multiGeometricFeaturesFromMask::parameters::write(ioHandler& handler,
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
  bool multiGeometricFeaturesFromMask::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool multiGeometricFeaturesFromMask::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
  
      lti::read(handler,"mergeEntries",mergeEntries);
      lti::read(handler,"maskValues",maskValues);
      lti::read(handler,"manualPostProcess",manualPostProcess);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && geometricFeaturesFromMask::parameters::read(handler,false);
# else
    bool (geometricFeaturesFromMask::parameters::* p_readMS)
	(ioHandler&,const bool) =
	geometricFeaturesFromMask::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool multiGeometricFeaturesFromMask::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // multiGeometricFeaturesFromMask
  // --------------------------------------------------

  // default constructor
  multiGeometricFeaturesFromMask::multiGeometricFeaturesFromMask()
    : geometricFeaturesFromMask(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  multiGeometricFeaturesFromMask
  ::multiGeometricFeaturesFromMask(const parameters& par)
    : geometricFeaturesFromMask() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  multiGeometricFeaturesFromMask
  ::multiGeometricFeaturesFromMask(const multiGeometricFeaturesFromMask& other)
    : geometricFeaturesFromMask() {
    copy(other);
  }

  // destructor
  multiGeometricFeaturesFromMask::~multiGeometricFeaturesFromMask() {
  }

  // returns the name of this type
  const char* multiGeometricFeaturesFromMask::getTypeName() const {
    return "multiGeometricFeaturesFromMask";
  }

  // copy member
  multiGeometricFeaturesFromMask& multiGeometricFeaturesFromMask::
  copy(const multiGeometricFeaturesFromMask& other) {
    geometricFeaturesFromMask::copy(other);

    return (*this);
  }

  // alias for copy member
  multiGeometricFeaturesFromMask&
    multiGeometricFeaturesFromMask::
  operator=(const multiGeometricFeaturesFromMask& other) {
    return (copy(other));
  }


  // clone member
  functor* multiGeometricFeaturesFromMask::clone() const {
    return new multiGeometricFeaturesFromMask(*this);
  }

  // return parameters
  const multiGeometricFeaturesFromMask::parameters&
    multiGeometricFeaturesFromMask::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool multiGeometricFeaturesFromMask::updateParameters() {
    
    const parameters& par = getParameters();
    
    if ( par.maskValues.size() != par.mergeEntries.size() ) {
      setStatusString ("maskValues and mergeEntries must be of the same size");
      return false;
    }
    
    return true;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool multiGeometricFeaturesFromMask
  ::apply(const channel8& src, imatrix& dest,
	  std::vector< std::vector<rectangle> >& objects) const {

    int numAllObjects ( 0 );
    ivector equivLabels;
    ivector objSize;
    if ( !computeEquivLabels(src, dest, objSize, equivLabels, numAllObjects) )
    {
      return false;
    }

    const parameters& par = getParameters();
    int numMaskValues ( par.maskValues.size() );
    if ( numMaskValues > 0 ) {
      objects.clear();
      objects.resize(numMaskValues);
      int i ( 0 );
      const int num ( numAllObjects / numMaskValues ); //coarse approx.
      for ( ; i < numMaskValues; i++ ) {
	objects[i].reserve(num);
      }
    } else {
      return false;
    }
 
    //table of the object kind and and the index within the kind
    // the background ( label 0 ) is not included here
    ivector maskValueTable(numAllObjects, -1);  //-1 -> UNDEFINED
    ivector objectTable(numAllObjects, -1);     //-1 -> UNDEFINED

    const int numRows (dest.rows());
    int row ( 0 );
    for(; row<numRows; row++) {
      int column (0);
      ivector& destVct = dest.getRow(row);
      const vector<ubyte>& srcVct  = src.getRow(row);
      ivector::const_iterator end ( destVct.end() );
      ivector::const_iterator destIt ( destVct.begin() );
      vector<ubyte>::const_iterator srcIt ( srcVct.begin() );
      for (; destIt!=end; ++destIt) {
	const int label ( equivLabels[*destIt] );
	const int label_1 ( label -1 );
	if( label > 0 ) {
	  int  kind ( maskValueTable[label_1] );
	  int index ( objectTable[label_1] );
	  
	  if ( index > -1 ) {
	    //add point to known object
	    rectangle& rect = objects[kind][index] ;
	    if ( column < rect.ul.y ) {
	      rect.ul.y = column; 
	    } else if ( column > rect.br.y ) {
	      rect.br.y = column;
	    }
	    if ( row < rect.ul.x ) {
	      rect.ul.x = row;
	    } else if ( row > rect.br.x ) {
	      rect.br.x = row;
	    }
	    
	  } else if ( index == -1 ) {
	    //we found a new object
	    //determine the kind
	    const int maskValue = *srcIt;
	    int k ( 0 );
	    while ( k < numMaskValues) {
	      if ( par.maskValues[k] == maskValue ) {
		kind = k;
		k = numMaskValues;
	      }
	      k++;
	    }
	    
	    if ( kind < 0 ) {
	      maskValueTable[label_1] = -2; //don't consider this object
	      
	    } else {
	      maskValueTable[label_1] = kind;
	      
	      //init the object
	      objectTable[label_1] = objects[kind].size();
	      point pt ( column, row );
	      rectangle object ( pt, pt );
	      (objects[kind]).push_back(object);
	    }
	  }
	}
	column++;
	++srcIt;
      }
    }
    
    if ( par.manualPostProcess ) {
      return true;
    } else {
      return postProcess(objects);
    }
 }

  bool multiGeometricFeaturesFromMask
  ::apply(const channel8 & src, 
	  std::vector<std::vector<rectangle> >& objects) const {

    imatrix dest;
    return apply (src, dest, objects);
  }
 

  bool multiGeometricFeaturesFromMask
  ::apply(const channel8& src, imatrix& dest,
	  std::vector< std::vector<geometricFeatureGroup0> >& objects) const {

    int numAllObjects ( 0 );
    ivector equivLabels;
    ivector objSize;
    if ( !computeEquivLabels(src, dest, objSize, equivLabels, numAllObjects) )
    {
      return false;
    }

    const parameters& par = getParameters();
    int numMaskValues ( par.maskValues.size() );
    if ( numMaskValues > 0 ) {
      objects.clear();
      objects.resize(numMaskValues);
      int i ( 0 );
      const int num ( numAllObjects / numMaskValues ); //coarse approx.
      for ( ; i < numMaskValues; i++ ) {
	objects[i].reserve(num);
      }
    } else {
      return false;
    }
 
    //table of the object kind and and the index within the kind
    // the background ( label 0 ) is not included here
    ivector maskValueTable(numAllObjects, -1);  //-1 -> UNDEFINED
    ivector objectTable(numAllObjects, -1);     //-1 -> UNDEFINED

    const int numRows (dest.rows());
    int row ( 0 );
    for(; row<numRows; row++) {
      int column (0);
      ivector& destVct = dest.getRow(row);
      const vector<ubyte>& srcVct  = src.getRow(row);
      ivector::const_iterator end ( destVct.end() );
      ivector::const_iterator destIt ( destVct.begin() );
      vector<ubyte>::const_iterator srcIt ( srcVct.begin() );
      for (; destIt!=end; ++destIt) {
	const int label ( equivLabels[*destIt] );
	const int label_1 ( label -1 );
	if( label > 0 ) {
	  int  kind ( maskValueTable[label_1] );
	  int index ( objectTable[label_1] );
	  
	  if ( index > -1 ) {
	    //add point to known object
	    (objects[kind])[index].addObjectPoint(column, row);
	    
	  } else if ( index == -1 ) {
	    //we found a new object
	    //determine the kind
	    const int maskValue = *srcIt;
	    int k ( 0 );
	    while ( k < numMaskValues) {
	      if ( par.maskValues[k] == maskValue ) {
          kind = k;
          k = numMaskValues;
	      }
	      k++;
	    }
	    
	    if ( kind < 0 ) {
	      maskValueTable[label_1] = -2; //don't consider this object
	      
	    } else {
	      maskValueTable[label_1] = kind;
	      
	      //init the object
	      objectTable[label_1] = objects[kind].size();
	      geometricFeatureGroup0 object;
	      point pt ( column, row );
	      object.minX = pt;
	      object.maxX = pt;
	      object.minY = pt;
	      object.maxY = pt;
	      object.area = 1;
	      object.cog = pt;
	      (objects[kind]).push_back(object);
	    }
	  }
	}
	column++;
	++srcIt;
      }
    }
    
    if ( par.manualPostProcess ) {
      return true;
    } else {
      return postProcess(objects);
    }
  }

  bool multiGeometricFeaturesFromMask
  ::apply(const channel8& src, 
	  std::vector< std::vector<geometricFeatureGroup0> >& objects) const {

    imatrix dest;
    return apply (src, dest, objects);
  }

  //----------------
  // postprocessing
  //----------------
  bool multiGeometricFeaturesFromMask
  ::postProcess(std::vector< std::vector<rectangle> >& objects) const {

    const parameters& par = getParameters();
    std::vector<std::vector<rectangle> >::iterator allIt ( objects.begin() );
    std::vector<std::vector<rectangle> >::iterator allEnd ( objects.end() );
    vector<bool>::const_iterator mergeIt ( par.mergeEntries.begin() );
    for (; allIt != allEnd; ++allIt) {
      
      //merge
      if ( *mergeIt ) {
	merge(*allIt);
      }
      ++mergeIt;
    }
    
    return true;
  }

  bool multiGeometricFeaturesFromMask::
  postProcess(std::vector<std::vector<geometricFeatureGroup0> >& objects) const
  { 
    const parameters& par = getParameters();

    //postprocessing:
    // - merge if requested
    // - ensure that all objects of a single kind are sorted
    
    std::vector<std::vector<geometricFeatureGroup0> >::iterator 
	allIt ( objects.begin() );
    std::vector<std::vector<geometricFeatureGroup0> >::iterator 
	allEnd ( objects.end() );
    vector<bool>::const_iterator mergeIt ( par.mergeEntries.begin() );
    for (; allIt != allEnd; ++allIt) {
      
      //merge
      if ( *mergeIt ) {
	merge(*allIt);
      }
      ++mergeIt;

      sortMerged(*allIt);
      
      //update COG
      std::vector<geometricFeatureGroup0>::iterator oEnd ( (*allIt).end() );
      std::vector<geometricFeatureGroup0>::iterator it ( (*allIt).begin() );
      for ( ; it != oEnd; ++it) {
	if (! (*it).completeComputation() ) return false;
      }
    }
    
    return true;
  }

}//namespace
