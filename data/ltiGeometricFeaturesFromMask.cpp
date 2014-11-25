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
 * file .......: ltiGeometricFeaturesFromMask.cpp
 * authors ....: Dorothee Finck
 * organization: LTI, RWTH Aachen
 * creation ...: 11.6.2003
 * revisions ..: $Id: ltiGeometricFeaturesFromMask.cpp,v 1.13 2007/12/02 00:20:22 alvarado Exp $
 */

//TODO: include files
#include "ltiGeometricFeaturesFromMask.h"
#include "ltiSort.h"

//#define _LTI_DEBUG 0
#if defined(_LTI_DEBUG)
using std::cout;
using std::endl;
#endif

namespace lti {
  // --------------------------------------------------
  // geometricFeaturesFromMask::parameters
  // --------------------------------------------------

  // default constructor
  geometricFeaturesFromMask::parameters::parameters()
    : fastRelabeling::parameters() {

    nBest = int(0); 
    minimumMergedObjectSize = 0;
    minimumDistance = point(0,0);
    merge = false;
  }

  // copy constructor
  geometricFeaturesFromMask::parameters::parameters(const parameters& other)
    : fastRelabeling::parameters() {
    copy(other);
  }

  // destructor
  geometricFeaturesFromMask::parameters::~parameters() {
  }

  // get type name
  const char* geometricFeaturesFromMask::parameters::getTypeName() const {
    return "geometricFeaturesFromMask::parameters";
  }

  // copy member

  geometricFeaturesFromMask::parameters&
    geometricFeaturesFromMask::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    fastRelabeling::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    fastRelabeling::parameters& (fastRelabeling::parameters::* p_copy)
      (const fastRelabeling::parameters&) =
      fastRelabeling::parameters::copy;
    (this->*p_copy)(other);
# endif
    
      nBest = other.nBest;
      minimumMergedObjectSize = other. minimumMergedObjectSize;
      minimumDistance = other.minimumDistance;
      merge = other.merge;
    return *this;
  }

  // alias for copy member
  geometricFeaturesFromMask::parameters&
    geometricFeaturesFromMask::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* geometricFeaturesFromMask::parameters::clone() const {
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
  bool geometricFeaturesFromMask::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool geometricFeaturesFromMask::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"nBest",nBest);
      lti::write(handler,"minimumMergedObjectSize",minimumMergedObjectSize);
      lti::write(handler,"minimumDistance",minimumDistance);
      lti::write(handler,"merge",merge);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && fastRelabeling::parameters::write(handler,false);
# else
    bool (fastRelabeling::parameters::* p_writeMS)(ioHandler&,const bool) const =
      fastRelabeling::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool geometricFeaturesFromMask::parameters::write(ioHandler& handler,
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
  bool geometricFeaturesFromMask::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool geometricFeaturesFromMask::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"nBest",nBest);
      lti::read(handler,"minimumMergedObjectSize",minimumMergedObjectSize);
      lti::read(handler,"minimumDistance",minimumDistance);
      lti::read(handler,"merge",merge);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && fastRelabeling::parameters::read(handler,false);
# else
    bool (fastRelabeling::parameters::* p_readMS)(ioHandler&,const bool) =
      fastRelabeling::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool geometricFeaturesFromMask::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // geometricFeaturesFromMask
  // --------------------------------------------------

  // default constructor
  geometricFeaturesFromMask::geometricFeaturesFromMask()
    : fastRelabeling(){

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  geometricFeaturesFromMask::geometricFeaturesFromMask(const parameters& par)
    : fastRelabeling() {

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  geometricFeaturesFromMask::geometricFeaturesFromMask(const geometricFeaturesFromMask& other)
    : fastRelabeling() {
    copy(other);
  }

  // destructor
  geometricFeaturesFromMask::~geometricFeaturesFromMask() {
  }

  // returns the name of this type
  const char* geometricFeaturesFromMask::getTypeName() const {
    return "geometricFeaturesFromMask";
  }

  // copy member
  geometricFeaturesFromMask&
    geometricFeaturesFromMask::copy(const geometricFeaturesFromMask& other) {
      fastRelabeling::copy(other);

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  geometricFeaturesFromMask&
    geometricFeaturesFromMask::operator=(const geometricFeaturesFromMask& other) {
    return (copy(other));
  }


  // clone member
  functor* geometricFeaturesFromMask::clone() const {
    return new geometricFeaturesFromMask(*this);
  }

  // return parameters
  const geometricFeaturesFromMask::parameters&
    geometricFeaturesFromMask::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // some helpers
  // -------------------------------------------------------------------
  bool geometricFeaturesFromMask::initRectangleVector(const int numLab,
						     const point& size,
						     std::vector<rectangle>& objects) const {

    rectangle object;
    object.ul = size;
    if ( numLab > 0 ) {
      objects.clear();
      objects.reserve(numLab);
      int i (0);
      for (; i < numLab ; i++) {
	objects.push_back(object);
      }
    } else {
      objects.resize(0);
      return false;
    }
    
    return true;
  }

  bool geometricFeaturesFromMask::
  initGeomFeature0Vector(const int numLab, const point& size,
			 std::vector<geometricFeatureGroup0>& objects) const {

    geometricFeatureGroup0 object;
    object.minX = size;
    object.minY = size;
    if ( numLab > 0 ) {
      objects.clear();
      objects.reserve(numLab);
      int i (0);
      for (; i < numLab ; i++) {
	objects.push_back(object);
      }
    } else {
      objects.resize(0);
      return false;
    }
    
    return true;
  }

  bool geometricFeaturesFromMask::computeEquivLabels(const channel8& src, imatrix& dest,
						     ivector& objSize, ivector& equivLabels,
						     int& numObjects) const {

    {
      ivector tmpEquivLabels;
      //this apply returns a partially labeled dest -> more efficient
      if ( !fastRelabeling::apply(src, dest, tmpEquivLabels, numObjects) )
	return false;
      //get the area, relabels the image
      if ( !fastRelabeling::relabelWithArea(tmpEquivLabels, numObjects, dest, objSize) )
	return false;
      //the background is counted as an object, we do not want this!!!
      objSize[0] = 0;
    }
    
    const parameters& par = getParameters(); 
    //sort labels, but do not relabel the image, process nBest
    //and suppress small objects
    if ( par.sortSize ) {
      if ( !sortLabels(par.minimumObjectSize, objSize, equivLabels ) ) {
	return false;
      }
      if ( par.nBest > 0 && par.nBest < numObjects) {
	numObjects = par.nBest;
      }
      //suppress small objects
      if (par.minimumObjectSize > 1) {
	//suppress small objects
	const int numEquivLabels ( equivLabels.size() );
	int i ( 1 );  // label 0 has artificial size 0
	while ( i < numEquivLabels
		&& objSize[i] >= par.minimumObjectSize ) {
	  i++;
	}
	if ( i <= numObjects ) { //consider n-best
	  numObjects = i-1;     // 0 is no Object;
	}
	for (i=0; i < numEquivLabels; i++) {
	  if ( equivLabels[i] > numObjects ) {
	    equivLabels[i] = 0;
	  }
	}
      }
    } else {
      equivLabels.resize(numObjects, 0, false, false);
      int i (0);
      for (; i<numObjects; i++ ) {
	equivLabels[i] = i;
      }
      //suppress small objects
      if (par.minimumObjectSize > 1) {
	//suppress small objects
	int nextLabel ( 0 );
	const int numEquivLabels ( equivLabels.size() );
	for ( i = 0; i < numEquivLabels; i++ ) {
	  const int label ( equivLabels[i] );
	  if ( objSize[label] < par.minimumObjectSize ) {
	    equivLabels[i] = 0;
	    numObjects--;
	  } else {
	    //no gaps
	    nextLabel++;
	    equivLabels[i] = nextLabel;
	  }
	}
      } else {
	numObjects--; //0 is no object
      }
    }

    return true;
  }

  //----------------
  // merge functions
  //----------------
  bool geometricFeaturesFromMask::merge(std::vector<rectangle>& src) const {

    //init
    std::vector<bool> del(src.size(), false);
    parameters par ( getParameters() );

    //merge
    int i1 ( 0 );
    const int end ( src.size() );
    for ( ; i1 < end; ++i1 ) {
      if ( del[i1] ) continue;
      int i2 ( i1 + 1 );
      int last ( i1 );
      for (; i2 < end; ++i2 ) {
        rectangle& rect = src[i2];
        rectangle& tmp = src [last];
        if ( rect.isClose(tmp, par.minimumDistance) ) {
          del[last] = true;
          rect.join(tmp);
          last = i2;
        }
      }
    }

    //delete all 1st entries that have been merged
    std::vector<rectangle>::iterator it = src.begin();
    std::vector<rectangle>::iterator nextIt = it;
    std::vector<rectangle>::iterator endIt = src.end();
    std::vector<bool>::iterator delIt = del.begin();
    while ( nextIt != endIt ) {
      if ( *delIt ) {
        do {
          ++nextIt; ++delIt;
        } while ( nextIt != endIt && *delIt );
        if ( nextIt != endIt ) {
	        *it = *nextIt;
        }
      } else {
        *it = *nextIt;
      }
      ++it; ++nextIt;
      ++delIt;
    }
    src.erase(it, endIt);

    return true;
  }

  bool geometricFeaturesFromMask::merge(std::vector<geometricFeatureGroup0>& src) const {

    //init
    std::vector<bool> del(src.size(), false);
    parameters par = getParameters();
  
    //merge
    int i1 ( 0 );
    const int end ( src.size() );
    for ( ; i1 < end; ++i1 ) {
      if ( del[i1] ) continue;
      //cout << " i1 " << i1 << " " << src[i1].area << "****" << endl;
      int i2 ( i1 + 1 );
      int last ( i1 );
      for (; i2 < end; ++i2 ) {
	if ( del[i2] ) continue;
	if ( src[last].isClose(src[i2], par.minimumDistance) ) {
	  if ( src[last].minX.x < src[i2].minX.x ) src[i2].minX = src[last].minX;
	  if ( src[last].minY.y < src[i2].minY.y ) src[i2].minY = src[last].minY;
	  if ( src[last].maxX.x > src[i2].maxX.x ) src[i2].maxX = src[last].maxX;
	  if ( src[last].maxY.y > src[i2].maxY.y ) src[i2].maxY = src[last].maxY;
	  src[i2].cog = src[i2].cog + src[last].cog;
	  src[i2].area += src[last].area;
	  //cout << " i2 " << i2 <<  " i1 " << i1 << " " << src[i2].area << endl;
	  del[last] = true;
	  last =i2;
	} // else { cout << " --!!!!!-- "; }
      }
    }
    
    //delete all 1st entries that have been merged
    std::vector<geometricFeatureGroup0>::iterator it ( src.begin() );
    std::vector<geometricFeatureGroup0>::iterator nextIt ( it );
    std::vector<geometricFeatureGroup0>::iterator endIt ( src.end() );
    std::vector<bool>::iterator delIt ( del.begin() );
    while ( nextIt != endIt ) {
      if ( *delIt ) {
	do {
	  ++nextIt; ++delIt;
	} while ( nextIt != endIt && *delIt );
	if ( nextIt != endIt ) {
	  *it = *nextIt;
	}

      } else {
	*it = *nextIt;
      }
      ++it; ++nextIt;
      ++delIt;
    }
    src.erase(it, endIt);
 
    return true;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool geometricFeaturesFromMask::apply(const channel8 & src, imatrix& dest,
					std::vector<rectangle>& objects) const {
    
    int numObjects;
    ivector objSize;
    ivector equivLabels;
    
    if ( !computeEquivLabels( src, dest, objSize, equivLabels, numObjects )
	 || !initRectangleVector( numObjects, dest.size(), objects ) ) {
      return false;
    }
    const int numRows (dest.rows());      
    int i (0);
    for(; i<numRows; i++) {
      int column (0);
      ivector& vct =  dest.getRow(i);
      ivector::iterator end ( vct.end() );
      ivector::iterator it ( vct.begin() );
      for (; it!=end; ++it) {
	int index ( equivLabels[*it] );
	*it = index; //relabel the image
	if( index > 0 ) {
	  index--;
	  if ( index < numObjects) {
	    if(i < objects[index].ul.y) objects[index].ul.y = i;
	    if(i > objects[index].br.y) objects[index].br.y = i;
	    
	    if(column < objects[index].ul.x) objects[index].ul.x = column;
	    if(column > objects[index].br.x) objects[index].br.x = column;
	  }
	}
	column++;
      }
    }
  
    const parameters& par = getParameters();
    if ( par.merge ) {
      merge(objects);
    }
    
    return true;
  }

  bool geometricFeaturesFromMask::apply(const channel8 & src, 
					std::vector<rectangle>& objects) const {

    int numObjects;
    ivector objSize;
    ivector equivLabels;
    imatrix dest;
    if ( !computeEquivLabels( src, dest, objSize, equivLabels, numObjects )
	 || !initRectangleVector( numObjects, dest.size(), objects ) ) {
      return false;
    }

    const int numRows (dest.rows());      
    int i (0);
    for(; i<numRows; i++) {
      int column (0);
      ivector& vct =  dest.getRow(i);
      ivector::iterator end ( vct.end() );
      ivector::iterator it ( vct.begin() );
      for (; it!=end; ++it) {
	int index ( equivLabels[*it] );
	if( index > 0 ) {
	  index--;
	  if ( index < numObjects) {
	    if(i < objects[index].ul.y) objects[index].ul.y = i;
	    if(i > objects[index].br.y) objects[index].br.y = i;
	    
	    if(column < objects[index].ul.x) objects[index].ul.x = column;
	    if(column > objects[index].br.x) objects[index].br.x = column;
	  }
	}
	column++;
      }
    }

    const parameters& par = getParameters();
    if ( par.merge ) {
      merge(objects);
    }
    
    return true;
  }

  bool geometricFeaturesFromMask::apply(const channel8 & src, imatrix& dest,
					std::vector<geometricFeatureGroup0>& objects) const
  {
    int i;
    int numObjects;
    ivector objSize;
    ivector equivLabels;
    if ( !computeEquivLabels( src, dest, objSize, equivLabels, numObjects )
	 || !initGeomFeature0Vector( numObjects, dest.size(), objects ) ) {
      return false;
    }

    const int numRows (dest.rows());      
    for(i=0; i<numRows; i++) {
      int column (0);
      ivector& vct =  dest.getRow(i);
      ivector::iterator end ( vct.end() );
      ivector::iterator it ( vct.begin() );
      for (; it!=end; ++it) {
	int index ( equivLabels[*it] );
	*it = index; //relabel the image
	if( index > 0 ) {
	  index--;
	  if ( index < numObjects)
	    objects[index].addObjectPoint(column, i);
	}
	column++;
      }
    }
    
    const parameters& par = getParameters();
    if ( par.merge ) {
      merge(objects);
      sortMerged(objects);
    }
    
    for (i=0; i < numObjects; i++) {
      if ( !objects[i].completeComputation() ) {return false;}
    }

    return true;
  }

  bool geometricFeaturesFromMask::apply(const channel8 & src, 
					std::vector<geometricFeatureGroup0>& objects) const {

    int i;
    int numObjects;
    ivector objSize;
    ivector equivLabels;
    imatrix dest;
    if ( !computeEquivLabels( src, dest, objSize, equivLabels, numObjects )
        || !initGeomFeature0Vector( numObjects, dest.size(), objects ) ) {
      return false;
    }

    const int numRows (dest.rows());      
    for(i=0; i<numRows; i++) {
      int column (0);
      ivector& vct =  dest.getRow(i);
      ivector::iterator end ( vct.end() );
      ivector::iterator it ( vct.begin() );
      for (; it!=end; ++it) {
        int index ( equivLabels[*it] );
        if( index > 0 ) {
          index--;
          if ( index < numObjects) {
            objects[index].addObjectPoint(column, i);
          }
        }
        column++;
      }
    }

    const parameters& par = getParameters();
    if ( par.merge ) {
      merge(objects);
      sortMerged(objects);
    }
    
    for (i=0; i < numObjects; i++) {
      if (! objects[i].completeComputation() ) {return false;}

    }
    return true;
  }


  bool geometricFeaturesFromMask
  ::sortMerged(std::vector<geometricFeatureGroup0>& objects) const {

    const parameters& par = getParameters();
    if ( par.minimumMergedObjectSize > 1 ) {
      std::vector<geometricFeatureGroup0>::iterator oEnd ( objects.end() );
      std::vector<geometricFeatureGroup0>::iterator it ( objects.begin() );
      std::vector<geometricFeatureGroup0>::iterator nextIt ( objects.begin() );
      for (; it != oEnd; ++it) {
        if ( (*it).area >= par.minimumMergedObjectSize ) {
          if (nextIt != it ) {
            ++nextIt;
          } else {
            *nextIt = *it;
            ++nextIt;
          }
        }
      }
      //delete remainder
      objects.erase(nextIt, oEnd);
    }
    
    //sort again
    if ( par.sortSize ) {
      
      const int numObjects ( objects.size() );
      ivector objectSize ( numObjects );
      ivector newIndex ( numObjects );
      int i ( 0 );
      for (; i<numObjects; i++ ) {
        objectSize[i] = objects[i].area;
        newIndex[i] = i;
      }
      //inefficient,
      // because the ltilib does not provide
      // a sort(vector<T> key, std::vector<U> elems)
      sort2<int, int> sorter(true);
      sorter.apply(objectSize, newIndex);
      
      //reorder the std::vector -> the inefficient part !!!
      ivector newIndex2 ( numObjects );
      for ( i=0; i<numObjects; i++ ) {
        newIndex2[i] = i;
      }
      for ( i=0; i<numObjects; i++ ) {
        geometricFeatureGroup0 tmp;
        const int k = newIndex2[newIndex[i]];
        if ( k != i ) {
          tmp = objects[i];
          objects[i] = objects[k];
          objects[k] = tmp;
          newIndex2[i] = k;
          newIndex2[k] = i;
        }
      }
    }

    return true;
  }

} 
