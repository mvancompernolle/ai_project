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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiProbabilityMap2D.cpp
 * authors ....: Florian Bley
 * organization: LTI, RWTH Aachen
 * creation ...: 26.3.2003
 * revisions ..: $Id: ltiProbabilityMap2D.cpp,v 1.8 2006/02/08 11:40:19 ltilib Exp $
 */

#include "ltiProbabilityMap2D.h"
#include "ltiSquareConvolution.h"
#include "ltiConvolution.h"
#include "ltiGaussKernels.h"

namespace lti {

  // --------------------------------------------------
  // probabilityMap2D::parameters
  // --------------------------------------------------

  // default constructor
  probabilityMap2D::parameters::parameters()
    : probabilityMapBase::parameters()
 {}

  // copy constructor
  probabilityMap2D::parameters::parameters(const parameters& other)
    : probabilityMapBase::parameters(other)
  {}

  // destructor
  probabilityMap2D::parameters::~parameters()
  {}

  // get type name
  const char* probabilityMap2D::parameters::getTypeName() const {
    return "probabilityMap2D::parameters";
  }

  // copy member
  probabilityMap2D::parameters&
    probabilityMap2D::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    probabilityMapBase::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    probabilityMapBase::parameters& (probabilityMapBase::parameters::* p_copy)
      (const probabilityMapBase::parameters&) =
      probabilityMapBase::parameters::copy;
    (this->*p_copy)(other);
# endif
    return *this;

  }

  // alias for copy member
  probabilityMap2D::parameters&
    probabilityMap2D::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* probabilityMap2D::parameters::clone() const {
    return new parameters(*this);
  }

  bool probabilityMap2D::parameters::isObjectColorModelValid() const {
    return (notNull(objectColorModel) &&
            (objectColorModel->dimensions() == 2));
  }

  bool probabilityMap2D::parameters::isNonObjectColorModelValid() const {
    return (notNull(nonObjectColorModel) &&
            (nonObjectColorModel->dimensions() == 2));
  }

  // --------------------------------------------------
  // probabilityMap
  // --------------------------------------------------

  // default constructor
  probabilityMap2D::probabilityMap2D()
    : probabilityMapBase() {

    probabilityHistogram.clear();
  }

  probabilityMap2D::probabilityMap2D(const parameters& theParams)
    : probabilityMapBase()
  {
    probabilityHistogram.clear();
    setParameters(theParams);
  }

  // copy constructor
  probabilityMap2D::probabilityMap2D(const probabilityMap2D& other)
    : probabilityMapBase()  {

    probabilityHistogram.clear();

    copy(other);
  }

  // destructor
  probabilityMap2D::~probabilityMap2D() {
  }

  // returns the name of this type
  const char* probabilityMap2D::getTypeName() const {
    return "probabilityMap2D";
  }

  // copy member
  probabilityMap2D&
    probabilityMap2D::copy(const probabilityMap2D& other) {
    probabilityMapBase::copy(other);

    return (*this);
  }

  // copy member
  probabilityMap2D&
    probabilityMap2D::operator=(const probabilityMap2D& other) {
    return copy(other);
  }

  // clone member
  functor* probabilityMap2D::clone() const {
    return new probabilityMap2D(*this);
  }

  // return parameters
  const probabilityMap2D::parameters&
    probabilityMap2D::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  /*
   * read the functor parameters
   */
  bool probabilityMap2D::read(ioHandler& handler, const bool complete) {
    parameters param;
    param.read(handler,complete);
    return setParameters(param);
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // return probability channel
  bool probabilityMap2D::apply(const channel8& src1, const channel8& src2, channel& dest) const {
      const parameters& param = getParameters();
      point chnl1_size = src1.size();
      point chnl2_size = src2.size();
      
      // size of src1 equals src2 ?
      if ( (chnl1_size.x != chnl2_size.x) || (chnl1_size.y != chnl2_size.y) ) {
          setStatusString("probabilityMap2D: channels do not match");
          return false;
      }
      
      // the color model MUST have 2 dimensions!
      if (probabilityHistogram.dimensions() == 2) {
          // resize probability channel
          dest.resize(src1.size());
          
          ivector theBin(2);
  
          // compute first iteration
	  int y;
	  vector<channel8::value_type>::const_iterator srcIterator1, eit1;
	  vector<channel8::value_type>::const_iterator srcIterator2, eit2;
	  vector<channel::value_type>::iterator destIterator;
	  for (y=0;y<src1.rows();++y) {
	    srcIterator1 = src1.getRow(y).begin();
	    eit1 = src1.getRow(y).end();
	    srcIterator2 = src2.getRow(y).begin();
	    eit2 = src2.getRow(y).end();

	    destIterator = dest.getRow(y).begin();
	    while (srcIterator1 != eit1) {
	      
	      theBin[0] = lookupTable[0][*srcIterator1];
	      theBin[1] = lookupTable[1][*srcIterator2];
	      (*destIterator)=static_cast<float>(probabilityHistogram.at(theBin));
	      
	      srcIterator1++;
	      srcIterator2++;
	      destIterator++;
	    }
	  }     

          // compute all other iterations
          if (param.iterations > 1) {
              int i;
              
              if (param.gaussian) {
                  gaussKernel2D<float> gk(param.windowSize,param.variance);
                  convolution convolver;
                  convolution::parameters convParam;
                  
                  convParam.boundaryType = lti::Mirror;
                  convParam.setKernel(gk);
                  convolver.setParameters(convParam);

                  for (i=1;i<param.iterations;++i) {
                    convolver.apply(dest);
                    computeMap(src1,src2,dest);
                  }
              } else {
                  squareConvolution<float> convolver;
                  squareConvolution<float>::parameters convParam;

                  convParam.boundaryType = lti::Mirror;
                  convParam.initSquare(param.windowSize);

                  convolver.setParameters(convParam);

                  for (i=1;i<param.iterations;++i) {
                    convolver.apply(dest);
                    computeMap(src1,src2,dest);
                  }
              }
          } // of (param.iterations > 1)

          return true;
          
      } // of (probabilityHistogram.dimensions() == 2)

      setStatusString("probabilityMap2D: no models loaded");
      return false;
  }

  /*
   * compute the second and up iterations of a probability map
   * using the given aPriori probabilites per pixel.
   */
  bool probabilityMap2D::computeMap(const channel8& src1, const channel8& src2,
				    channel& aPrioriDest) const {
 
    point chnl1_size = src1.size();
    point chnl2_size = src2.size();
      
      // size of src1 equals src2 ?
    if ( (chnl1_size.x != chnl2_size.x) || (chnl1_size.y != chnl2_size.y) ) {
      setStatusString("probabilityMap2D: channels do not match");
      return false;

    } else {
      int y;
      vector<channel8::value_type>::const_iterator srcIterator1, eit1;
      vector<channel8::value_type>::const_iterator srcIterator2, eit2;
      vector<channel::value_type>::iterator destIterator;
 
      const parameters& param = getParameters();
      const thistogram<double>& objModel = param.getObjectColorModel();
      const thistogram<double>& nonObjModel = param.getNonObjectColorModel();
      
      float relObjProb;
      float relNonObjProb;
      
      ivector theBin(2);
      
      for (y=0;y<src1.rows();++y) {
	srcIterator1 = src1.getRow(y).begin();
	eit1 = src1.getRow(y).end();
	srcIterator2 = src2.getRow(y).begin();
	eit2 = src2.getRow(y).end();

	destIterator = aPrioriDest.getRow(y).begin();

	while (srcIterator1 != eit1) {
	  theBin[0] = lookupTable[0][*srcIterator1];
	  theBin[1] = lookupTable[1][*srcIterator2];
	  
	  relObjProb = static_cast<float>(objModel.getProbability(theBin) *
					  (*destIterator));
	  relNonObjProb = static_cast<float>(nonObjModel.getProbability(theBin)*
					     (1.0f-(*destIterator)));

	  // assume non-object if no entries are given
	  if ((relObjProb == 0.0f) && (relNonObjProb == 0.0f)) {
	    (*destIterator) = 0.0f;
	  } else {
	    // bayes
	    (*destIterator) = relObjProb / (relObjProb + relNonObjProb);
	  }

	  srcIterator1++;
	  srcIterator2++;
	  destIterator++;
	}
      }
    }
    
    return true;
  }

  // return probability value of an rgb pixel
  float probabilityMap2D::apply(const ubyte &value1, const ubyte &value2, ivector& theBin) const {

    assert((probabilityHistogram.dimensions() == 2) &&
           (theBin.size() == 2));
    
    theBin[0] = lookupTable[0][static_cast<int>(value1)];
    theBin[1] = lookupTable[1][static_cast<int>(value2)];

    return static_cast<float>(probabilityHistogram.at(theBin));
  }

  // return probability value of an yuv pixel
  float probabilityMap2D::apply(const ubyte &value1, const ubyte &value2) const {

    assert (probabilityHistogram.dimensions() == 2);

    ivector theBin(2);
 
    theBin[0] = lookupTable[0][static_cast<int>(value1)];
    theBin[1] = lookupTable[1][static_cast<int>(value2)];
 
    return static_cast<float>(probabilityHistogram.at(theBin));
  }

} // of namespace
