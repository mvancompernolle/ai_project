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
 * file .......: ltiProbabilityMap.cpp
 * authors ....: Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 30.1.2001
 * revisions ..: $Id: ltiProbabilityMap.cpp,v 1.16 2006/02/08 11:39:55 ltilib Exp $
 */

#include "ltiProbabilityMap.h"
#include "ltiSquareConvolution.h"
#include "ltiConvolution.h"
#include "ltiGaussKernels.h"

namespace lti {

  // --------------------------------------------------
  // probabilityMap::parameters
  // --------------------------------------------------

  // default constructor
  probabilityMap::parameters::parameters()
    : probabilityMapBase::parameters() {
  }

  // copy constructor
  probabilityMap::parameters::parameters(const parameters& other)
    : probabilityMapBase::parameters(other) {
  }

  // destructor
  probabilityMap::parameters::~parameters() {
  }

  // get type name
  const char* probabilityMap::parameters::getTypeName() const {
    return "probabilityMap::parameters";
  }

  // copy member
  probabilityMap::parameters&
  probabilityMap::parameters::copy(const parameters& other) {
#ifndef _LTI_MSC_6
    probabilityMapBase::parameters::copy(other);
#else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    probabilityMapBase::parameters& (probabilityMapBase::parameters::* p_copy)
      (const probabilityMapBase::parameters&) =
      probabilityMapBase::parameters::copy;
    (this->*p_copy)(other);
#endif
    return *this;
  }

  // alias for copy member
  probabilityMap::parameters&
    probabilityMap::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* probabilityMap::parameters::clone() const {
    return new parameters(*this);
  }

  bool probabilityMap::parameters::isObjectColorModelValid() const {
    return (notNull(objectColorModel) &&
            (objectColorModel->dimensions() == 3));
  }

  bool probabilityMap::parameters::isNonObjectColorModelValid() const {
    return (notNull(nonObjectColorModel) &&
            (nonObjectColorModel->dimensions() == 3));
  }

  // --------------------------------------------------
  // probabilityMap
  // --------------------------------------------------

  // default constructor
  probabilityMap::probabilityMap()
    : probabilityMapBase() {

    probabilityHistogram.clear();
  }

  probabilityMap::probabilityMap(const parameters& theParams)
    : probabilityMapBase()
  {
    probabilityHistogram.clear();
    setParameters(theParams);
  }

  // copy constructor
  probabilityMap::probabilityMap(const probabilityMap& other)
    : probabilityMapBase()  {

    probabilityHistogram.clear();

    copy(other);
  }

  // destructor
  probabilityMap::~probabilityMap() {
  }

  // returns the name of this type
  const char* probabilityMap::getTypeName() const {
    return "probabilityMap";
  }

  // copy member
  probabilityMap&
    probabilityMap::copy(const probabilityMap& other) {
    probabilityMapBase::copy(other);

    return (*this);
  }

  // copy member
  probabilityMap&
    probabilityMap::operator=(const probabilityMap& other) {
    return copy(other);
  }

  // clone member
  functor* probabilityMap::clone() const {
    return new probabilityMap(*this);
  }

  // return parameters
  const probabilityMap::parameters&
    probabilityMap::getParameters() const {
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
  bool probabilityMap::read(ioHandler& handler, const bool complete) {
    parameters param;
    param.read(handler,complete);
    return setParameters(param);
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // return probability channel
  bool probabilityMap::apply(const image& src,channel& dest) const{

    const parameters& param = getParameters();

    // the color model MUST have 3 dimensions!
    if (probabilityHistogram.dimensions() == 3) {
      // resize probability channel
      dest.resize(src.size());

      ivector theBin(3);

      // compute first iteration
      int y;
      vector<rgbPixel>::const_iterator srcIterator,eit;
      vector<channel::value_type>::iterator destIterator;
      for (y=0;y<src.rows();++y) {
        srcIterator = src.getRow(y).begin();
        eit = src.getRow(y).end();
        destIterator = dest.getRow(y).begin();
        while (srcIterator != eit) {

	  theBin[0] = lookupTable[0][(*srcIterator).getRed()];
	  theBin[1] = lookupTable[1][(*srcIterator).getGreen()];
	  theBin[2] = lookupTable[2][(*srcIterator).getBlue()];
          (*destIterator)=static_cast<float>(probabilityHistogram.at(theBin));

          srcIterator++;
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
            computeMap(src,dest);
          }
        } else {
          squareConvolution<float> convolver;
          squareConvolution<float>::parameters convParam;

          convParam.boundaryType = lti::Mirror;
          convParam.initSquare(param.windowSize);

          convolver.setParameters(convParam);

          for (i=1;i<param.iterations;++i) {
            convolver.apply(dest);
            computeMap(src,dest);
          }
        }
      }

      return true;
    }

    setStatusString("probabilityMap: no models loaded");
    return false;
  }


  bool probabilityMap::apply(const image& src,channel& dest, const channel &apriori) const{
    channel probChannel(apriori);
    const parameters& param = getParameters();
    bool returnValue=true;
    // the color model MUST have 3 dimensions!
    if (probabilityHistogram.dimensions() == 3) {
      // resize probability channel
      dest.resize(src.size());    
      if (probChannel.size()!=src.size()) {
        probChannel.resize(src.size(),param.objectProbability);
        returnValue=false;
        setStatusString("probabilityMap: apriori channel has wrong size");
      }
        
      
                      
      // compute iterations
      if (param.iterations > 0) {
        int i;

        if (param.gaussian) {
          gaussKernel2D<float> gk(param.windowSize,param.variance);
          convolution convolver;
          convolution::parameters convParam;

          convParam.boundaryType = lti::Mirror;
          convParam.setKernel(gk);
          convolver.setParameters(convParam);

          for (i=0;i<param.iterations;++i) {
            convolver.apply(probChannel);
            computeMap(src,probChannel);
          }
        } else {
          squareConvolution<float> convolver;
          squareConvolution<float>::parameters convParam;

          convParam.boundaryType = lti::Mirror;
          convParam.initSquare(param.windowSize);

          convolver.setParameters(convParam);

          for (i=0;i<param.iterations;++i) {
            convolver.apply(probChannel);
            computeMap(src,probChannel);
          }
        }
      }      
      dest=probChannel;
      return returnValue;
    }

    setStatusString("probabilityMap: no models loaded");
    return false;    
  }

  /*
   * compute the second and up iterations of a probability map
   * using the given apriori probabilites per pixel.
   */
  void probabilityMap::computeMap(const image& img,
                                        channel& aPrioriDest) const{

    int y;
    vector<rgbPixel>::const_iterator srcIterator,eit;
    vector<channel::value_type>::iterator destIterator;

    const parameters& param = getParameters();

    const thistogram<double>& objModel = param.getObjectColorModel();
    const thistogram<double>& nonObjModel = param.getNonObjectColorModel();

    float relObjProb;
    float relNonObjProb;

    ivector theBin(3);

    for (y=0;y<img.rows();++y) {
      srcIterator = img.getRow(y).begin();
      eit = img.getRow(y).end();
      destIterator = aPrioriDest.getRow(y).begin();

      while (srcIterator != eit) {
        theBin[0] = lookupTable[0][(*srcIterator).getRed()];
        theBin[1] = lookupTable[1][(*srcIterator).getGreen()];
        theBin[2] = lookupTable[2][(*srcIterator).getBlue()];

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

        srcIterator++;
        destIterator++;
      }
    }
  }

  // return probability value of an rgb pixel
  float probabilityMap::apply(const rgbPixel &src,ivector& theBin) const {

    assert((probabilityHistogram.dimensions() == 3) &&
           (theBin.size() == 3));

    theBin[0] = lookupTable[0][src.getRed()];
    theBin[1] = lookupTable[1][src.getGreen()];
    theBin[2] = lookupTable[2][src.getBlue()];

    return static_cast<float>(probabilityHistogram.at(theBin));
  }

  // return probability value of an rgb pixel
  float probabilityMap::apply(const rgbPixel &src) const{

    assert (probabilityHistogram.dimensions() == 3);

    ivector theBin(3);

    theBin[0] = lookupTable[0][src.getRed()];
    theBin[1] = lookupTable[1][src.getGreen()];
    theBin[2] = lookupTable[2][src.getBlue()];

    return static_cast<float>(probabilityHistogram.at(theBin));
  }
 
} // of namespace
