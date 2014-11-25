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
 * file .......: ltiWhiteningSegmentation.cpp
 * authors ....: Axel Berner
 * organization: LTI, RWTH Aachen
 * creation ...: 14.2.2002
 * revisions ..: $Id: ltiWhiteningSegmentation.cpp,v 1.10 2006/09/05 10:33:45 ltilib Exp $
 */

#include "ltiKMeansSegmentation.h"
#include "ltiObjectsFromMask.h"

#include "ltiWhiteningSegmentation.h"

namespace lti {
  // --------------------------------------------------
  // whiteningSegmentation::parameters
  // --------------------------------------------------

  // default constructor
  whiteningSegmentation::parameters::parameters()
    : segmentation::parameters() {

    quantNormal.kernelSize = 5;
    quantNormal.quantParameters.numberOfColors = 16;
    quantNormal.smoothFilter = kMeansSegmentation::parameters::KNearest;

    quantTransformed.kernelSize = 5;
    quantTransformed.quantParameters.numberOfColors = 15;
    quantTransformed.smoothFilter = kMeansSegmentation::parameters::KNearest;

    minRegionSize = 12;
  }

  // copy constructor
  whiteningSegmentation::parameters::parameters(const parameters& other)
    : segmentation::parameters()  {
    copy(other);
  }

  // destructor
  whiteningSegmentation::parameters::~parameters() {
  }

  // get type name
  const char* whiteningSegmentation::parameters::getTypeName() const {
    return "whiteningSegmentation::parameters";
  }

  // copy member

  whiteningSegmentation::parameters&
    whiteningSegmentation::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    segmentation::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    segmentation::parameters& (segmentation::parameters::* p_copy)
      (const segmentation::parameters&) =
      segmentation::parameters::copy;
    (this->*p_copy)(other);
# endif

    quantNormal.copy(other.quantNormal);
    quantTransformed.copy(other.quantTransformed);
    minRegionSize = other.minRegionSize;

    return *this;
  }

  // alias for copy member
  whiteningSegmentation::parameters&
    whiteningSegmentation::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* whiteningSegmentation::parameters::clone() const {
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
  bool whiteningSegmentation::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool whiteningSegmentation::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"quantNormal",quantNormal);
      lti::write(handler,"quantTransformed",quantTransformed);
      lti::write(handler,"minRegionSize",minRegionSize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && segmentation::parameters::write(handler,false);
# else
    bool (segmentation::parameters::* p_writeMS)(ioHandler&,const bool) const =
      segmentation::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool whiteningSegmentation::parameters::write(ioHandler& handler,
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
  bool whiteningSegmentation::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool whiteningSegmentation::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"quantNormal",quantNormal);
      lti::read(handler,"quantTransformed",quantTransformed);
      lti::read(handler,"minRegionSize",minRegionSize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && segmentation::parameters::read(handler,false);
# else
    bool (segmentation::parameters::* p_readMS)(ioHandler&,const bool) =
      segmentation::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool whiteningSegmentation::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // whiteningSegmentation
  // --------------------------------------------------

  // default constructor
  whiteningSegmentation::whiteningSegmentation()
    : segmentation(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  whiteningSegmentation::whiteningSegmentation(const whiteningSegmentation& o)
    : segmentation() {
    copy(o);
  }

  // destructor
  whiteningSegmentation::~whiteningSegmentation() {
  }

  // returns the name of this type
  const char* whiteningSegmentation::getTypeName() const {
    return "whiteningSegmentation";
  }

  // copy member
  whiteningSegmentation&
    whiteningSegmentation::copy(const whiteningSegmentation& other) {
      segmentation::copy(other);
    return (*this);
  }

  // alias for copy member
  whiteningSegmentation&
    whiteningSegmentation::operator=(const whiteningSegmentation& other) {
    return (copy(other));
  }


  // clone member
  functor* whiteningSegmentation::clone() const {
    return new whiteningSegmentation(*this);
  }

  // return parameters
  const whiteningSegmentation::parameters&
    whiteningSegmentation::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  void whiteningSegmentation::removeSmallRegions(
			       const image& src,
			       const int& thresh,
			       imatrix& imgMap) const{

    if(thresh == 1)
      return;

    std::list<lti::areaPoints> allList;
    lti::objectsFromMask getAreaList;
    lti::objectsFromMask::parameters paramOFM;
    paramOFM.assumeLabeledMask = true;
    getAreaList.setParameters(paramOFM);
    getAreaList.apply(imgMap, allList);
    lti::pointList freePoints;

    std::list<lti::areaPoints>::iterator itL = allList.begin();
    // for all areaPoints in allList...
    while(itL != allList.end()) {
      lti::areaPoints& listX = *itL;
      if(listX.size() < thresh) {
        // append the points in the freePoints list
        lti::areaPoints::iterator itP;
        for (itP=listX.begin();itP!=listX.end();itP++) {
          freePoints.push_back(*itP);
        }
      }
      itL++;
    }
    attachPoints(src,freePoints,imgMap);
  }

  void whiteningSegmentation::attachPoints(const image& src,
					   pointList& freePoints,
					   imatrix& imgMap) const{

    const int MASK = -1;
    const point neigh[4] = {lti::point(1,0),
			    lti::point(0,-1),
			    lti::point(-1,0),
			    lti::point(0,1)};

    lti::rectangle imgRect(lti::point(0,0),imgMap.size()+lti::point(-1,-1));

    //mask all points in the imgMap
    lti::pointList::iterator itPL = freePoints.begin();
    while (itPL != freePoints.end()) {
      imgMap.at(*itPL) = MASK;
      itPL++;
    }

    // try to attach the freePoints
    int minDiff,diff;
    bool change = false;
    int radius = 0;
    while(!freePoints.empty() && (radius < 255)) {

      if (!change)
        radius += 50;
      else
        change = false;

      lti::pointList::iterator itPL = freePoints.begin();

      while (itPL != freePoints.end()) {

        lti::point p=*itPL;
        lti::rgbPixel pColor(src.at(p));
        int next;
        lti::point nextp;
        minDiff = 3*radius*radius;

        for (next=0;next<4;next++) {
          nextp= p+neigh[next];
          if (imgRect.isInside(nextp) && imgMap.at(nextp)!=MASK) {
            diff = pColor.distanceSqr(src.at(nextp));
            if (diff < minDiff) {
              minDiff = diff;
              imgMap.at(p) = imgMap.at(nextp);
              change = true;
            }
          }
        }

        if (imgMap.at(p) != MASK)
          itPL = freePoints.erase(itPL);
        else
          itPL++;

      }

    }
  }

  // transform the img into the new color space
  bool
  whiteningSegmentation::transformImage(const principalComponents<float>& pca,
                                        const image& src,
                                        image& dest) const {
    if ( pca.getTransformMatrix().empty() ) {
      setStatusString("PCA is uninitialized");
      return false;
    }

    if (src.empty()) {
      setStatusString("Source image empty");
      return false;
    }

    dest.resize(src.size(),rgbPixel(),false,false);

    matrix<float> orgPat,transPat; //pattern
    orgPat.resize(src.rows()*src.columns(),3,float(0.0f),false,false);

    // "matrix" -> "vector"
    image::const_iterator cit;
    int i;
    for (i=0,cit=src.begin();i<orgPat.rows();++i,++cit) {
      orgPat.at(i,0) = float((*cit).getRed()  )/255.0f;
      orgPat.at(i,1) = float((*cit).getGreen())/255.0f;
      orgPat.at(i,2) = float((*cit).getBlue() )/255.0f;
    }

    // transform
    pca.transform(orgPat,transPat);

    // "vector" -> "matrix"
    image::iterator it;
    for (i=0,it=dest.begin();i<transPat.rows();++i,++it) {
      (*it).set(static_cast<lti::ubyte>(255*lti::sigmoid(transPat.at(i,0))),
                static_cast<lti::ubyte>(255*lti::sigmoid(transPat.at(i,1))),
                static_cast<lti::ubyte>(255*lti::sigmoid(transPat.at(i,2))),
                0);
    }

    return true;
  }

  // transform the img into the new color space
  bool
  whiteningSegmentation::transformImage(const drgbPixel& mean,
                                        const dmatrix& covar,
                                        const image& src,
                                        image& dest) const {
    // cast input data to the correct data types
    fvector fmean;
    fmatrix fcovar;
    fmean.resize(3,0.0f,false,false);
    fmean.at(0)=static_cast<float>(mean.red/255.0);
    fmean.at(1)=static_cast<float>(mean.green/255.0);
    fmean.at(2)=static_cast<float>(mean.blue/255.0);
    fcovar.castFrom(covar);
    fcovar.divide(255*255);

    // create a pca object

    // principal components functor used to transform the pixel colors
    lti::principalComponents<float> pca;
    lti::principalComponents<float>::parameters pcaPar;

    pcaPar.whitening = true; // whitening transform is a requirement!
    pcaPar.resultDim = 3;    // after the transform, each pixel must
                             // still have three dimensions!
    pcaPar.autoDim = false;  // do not try to reduce the dimensionality

    pca.setParameters(pcaPar); // specify to use the given parameters

    pca.setCovarianceAndMean(fcovar,fmean); // and the computed stats.

    bool res = transformImage(pca,src,dest);
    return res;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool
  whiteningSegmentation::apply(const image& src,
                               const drgbPixel& mean,
                               const dmatrix& covar,
                                     imatrix& dest) const {

    // cast input data to the correct data types
    fvector fmean;
    fmatrix fcovar;
    fmean.resize(3,0.0f,false,false);
    fmean.at(0)=static_cast<float>(mean.red/255.0);
    fmean.at(1)=static_cast<float>(mean.green/255.0);
    fmean.at(2)=static_cast<float>(mean.blue/255.0);
    fcovar.castFrom(covar);
    fcovar.divide(255*255);

    // create a pca object

    // principal components functor used to transform the pixel colors
    lti::principalComponents<float> pca;
    lti::principalComponents<float>::parameters pcaPar;

    pcaPar.whitening = true; // whitening transform is a requirement!
    pcaPar.resultDim = 3;    // after the transform, each pixel must
                             // still have three dimensions!
    pcaPar.autoDim = false;  // do not try to reduce the dimensionality

    pca.setParameters(pcaPar); // specify to use the given parameters

    pca.setCovarianceAndMean(fcovar,fmean); // and the computed stats.

    bool res = apply(src,pca,dest);
    return res;
  }


  // On copy apply for type image!
  bool
  whiteningSegmentation::apply(const image& src,
                               const principalComponents<float>& pca,
                               imatrix& dest) const {

    const parameters& par = getParameters();

    kMeansSegmentation kMSeg;

    image enh;              // enhanced image

    // whitening transformation of normal image
    if (!transformImage(pca,src,enh)) {
      return false;
    }

    // segmentate images
    imatrix nrmQuantMask,enhQuantMask;

    kMSeg.setParameters(par.quantNormal);
    kMSeg.apply(src,nrmQuantMask);

    kMSeg.setParameters(par.quantTransformed);
    kMSeg.apply(enh,enhQuantMask);

    // merge quantized masks
    dest.addScaled(1,enhQuantMask,
		   par.quantNormal.quantParameters.numberOfColors,
                   nrmQuantMask);

    // remove small regions
    removeSmallRegions(src,par.minRegionSize,dest);

    return true;
  };



}
