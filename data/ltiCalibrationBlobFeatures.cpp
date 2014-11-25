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
 * file .......: ltiCalibrationBlobFeatures.cpp
 * authors ....: Dorothee Finck
 * organization: LTI, RWTH Aachen
 * creation ...: 28.11.2002
 * revisions ..: $Id: ltiCalibrationBlobFeatures.cpp,v 1.9 2006/09/05 10:04:08 ltilib Exp $
 */

#include "ltiCalibrationBlobFeatures.h"
#include "ltiSplitImageTorgI.h"
#include "ltiLispStreamHandler.h"
#include "ltiSort.h"

namespace lti {
  // --------------------------------------------------
  // calibrationBlobFeatures::parameters
  // --------------------------------------------------

  // default constructor
  calibrationBlobFeatures::parameters::parameters() 
    : modifier::parameters() {    

    blobThreshold = int(100);
    minimumBlobSize = 4;
    numberOfBlobs.x = 9;
    numberOfBlobs.y = 9;
    xOrdering = true;
  }

  // copy constructor
  calibrationBlobFeatures::parameters::parameters(const parameters& other) {
    copy(other);
  }
  
  // destructor
  calibrationBlobFeatures::parameters::~parameters() {    
  }

  // get type name
  const char* calibrationBlobFeatures::parameters::getTypeName() const {
    return "calibrationBlobFeatures::parameters";
  }
  
  // copy member

  calibrationBlobFeatures::parameters& 
    calibrationBlobFeatures::parameters::copy(const parameters& other) {
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
    
    
    blobThreshold = other.blobThreshold;
    minimumBlobSize = other.minimumBlobSize;
    numberOfBlobs = other.numberOfBlobs;
    xOrdering = other.xOrdering;

    return *this;
  }

  // alias for copy member
  calibrationBlobFeatures::parameters& 
    calibrationBlobFeatures::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* calibrationBlobFeatures::parameters::clone() const {
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
  bool calibrationBlobFeatures::parameters::write(ioHandler& handler,
                                         const bool complete) const 
# else
  bool calibrationBlobFeatures::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const 
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"blobThreshold",blobThreshold);
      lti::write(handler,"minimumBlobSize",minimumBlobSize);
      lti::write(handler,"numberOfBlobs", numberOfBlobs);
      lti::write(handler,"xOrdering", xOrdering);
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
  bool calibrationBlobFeatures::parameters::write(ioHandler& handler,
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
  bool calibrationBlobFeatures::parameters::read(ioHandler& handler,
                                        const bool complete) 
# else
  bool calibrationBlobFeatures::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"blobThreshold",blobThreshold);
      lti::read(handler,"minimumBlobSize",minimumBlobSize);
      lti::read(handler,"numberOfBlobs",numberOfBlobs);
      lti::read(handler,"xOrdering", xOrdering);
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
  bool calibrationBlobFeatures::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // calibrationBlobFeatures
  // --------------------------------------------------

  // default constructor
  calibrationBlobFeatures::calibrationBlobFeatures()
    : modifier(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  calibrationBlobFeatures::calibrationBlobFeatures(const parameters& par)
    : modifier() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  calibrationBlobFeatures::calibrationBlobFeatures(const calibrationBlobFeatures& other) {
    copy(other);
  }

  // destructor
  calibrationBlobFeatures::~calibrationBlobFeatures() {
  }

  // returns the name of this type
  const char* calibrationBlobFeatures::getTypeName() const {
    return "calibrationBlobFeatures";
  }

  // copy member
  calibrationBlobFeatures&
    calibrationBlobFeatures::copy(const calibrationBlobFeatures& other) {
      modifier::copy(other);

    return (*this);
  }

  // alias for copy member
  calibrationBlobFeatures&
    calibrationBlobFeatures::operator=(const calibrationBlobFeatures& other) {
    return (copy(other));
  }


  // clone member
  functor* calibrationBlobFeatures::clone() const {
    return new calibrationBlobFeatures(*this);
  }

  // return parameters
  const calibrationBlobFeatures::parameters&
    calibrationBlobFeatures::getParameters() const {
    const parameters* par = 
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException();
    }
    return *par;
  }
  
  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // 
   bool calibrationBlobFeatures::apply(const image& scr, matrix<point>& dest){
    lti::splitImageTorgI splitter;
    lti::channel8 chnl8;
    splitter.getIntensity(scr,chnl8);
    apply(chnl8,dest);

    return false;
   };
 
  //
  bool calibrationBlobFeatures::apply(const channel8& src, matrix<point>& dest){

    int i;

    parameters par ( getParameters() );

    geometricFeaturesFromMask::parameters gfmPar;
    gfmPar.minThreshold = 0;
    gfmPar.maxThreshold = par.blobThreshold;
    gfmPar.fourNeighborhood = false;
    gfmPar.assumeLabeledMask = false;
    gfmPar.minimumObjectSize = par.minimumBlobSize;
    gfmPar.sortSize = false;
   
    std::vector<geometricFeatureGroup0> objects;
    geometricFeaturesFromMask geomFeatures ( gfmPar );
    geomFeatures.apply(src, objects);

    const int numObjects ( objects.size() );
    if ( numObjects != par.numberOfBlobs.x * par.numberOfBlobs.y )
      return false;

    //sort the objects by their x-coordinate
    //afterwards each nine-tupel will correspond to one row
    sort2<float, int> sorter;
    ivector reindex ( numObjects );

    if ( par.xOrdering ) {
      {
	vector<float> xCoords ( numObjects );
	for (i=0; i<numObjects; i++ ) {
	  xCoords[i] = objects[i].cog.x;
	  reindex[i] = i;
	}
	sorter.apply(xCoords, reindex);
      }
      
      //start filling the destination matrix
      int index ( 0 );
      dest.resize(par.numberOfBlobs, point(), false, true);
      int column (0);
      for(; column<par.numberOfBlobs.x; column++) { //for
	
	//sort each nine-tupel by their y-coordinate
	vector<float> yCoords ( par.numberOfBlobs.y );
	ivector reindex2 ( par.numberOfBlobs.y );
	for (i=0; i<par.numberOfBlobs.y; i++ ) {
	  yCoords[i] = objects[ reindex[index] ].cog.y;
	  reindex2[i] = reindex[index++];
	}
	sorter.apply(yCoords, reindex2);
	
	for (i=0; i<par.numberOfBlobs.y; i++ )
	  dest.at(i, column) = objects[ reindex2[i] ].cog;
      }
    }

    else {
     {
       vector<float> yCoords ( numObjects );
	for (i=0; i<numObjects; i++ ) {
	  yCoords[i] = objects[i].cog.y;
	  reindex[i] = i;
	}
	sorter.apply(yCoords, reindex);
      }
      
      //start filling the destination matrix
      int index ( 0 );
      dest.resize(par.numberOfBlobs, point(), false, true);
      int row ( 0 );
      for(; row<par.numberOfBlobs.y; row++) { //for
	
	//sort each nine-tupel by their y-coordinate
	vector<float> xCoords ( par.numberOfBlobs.x );
	ivector reindex2 ( par.numberOfBlobs.x );
	for (i=0; i<par.numberOfBlobs.x; i++ ) {
	  xCoords[i] = objects[ reindex[index] ].cog.x;
	  reindex2[i] = reindex[index++];
	}
	sorter.apply(xCoords, reindex2);
	
	for (i=0; i<par.numberOfBlobs.x; i++ )
	  dest.at(row, i) = objects[ reindex2[i] ].cog;
      }
    }
 
    return false;
  };

}
