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
 * file .......: ltiHarrisCorners.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 31.10.2002
 * revisions ..: $Id: ltiHarrisCorners.cpp,v 1.16 2006/09/05 10:14:57 ltilib Exp $
 */

#include "ltiMacroSymbols.h"

#ifdef _LTI_MSC_6
#pragma warning( disable : 4786 )
#endif

#include "ltiHarrisCorners.h"
#include "ltiConvolution.h"
#include "ltiLinearKernels.h"
#include "ltiGaussKernels.h"
#include "ltiLocalMaxima.h"

namespace lti {
  // --------------------------------------------------
  // harrisCorners::parameters
  // --------------------------------------------------

  // default constructor
  harrisCorners::parameters::parameters()
    : cornerDetector::parameters(), localMaximaParameters(), 
      gradientFunctorParameters() {
    variance = float(-1);
    kernelSize = int(7);
    maximumCorners = int(300);
    scale = float(0.04f);
    gradientFunctorParameters.format = gradientFunctor::parameters::Cartesic;
    gradientFunctorParameters.kernelType = gradientFunctor::parameters::Harris;
  }

  // copy constructor
  harrisCorners::parameters::parameters(const parameters& other)
    : cornerDetector::parameters(), localMaximaParameters(), 
      gradientFunctorParameters() {
    copy(other);
  }

  // destructor
  harrisCorners::parameters::~parameters() {
  }

  // get type name
  const char* harrisCorners::parameters::getTypeName() const {
    return "harrisCorners::parameters";
  }

  // copy member

  harrisCorners::parameters&
    harrisCorners::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    cornerDetector::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    cornerDetector::parameters& (cornerDetector::parameters::* p_copy)
      (const cornerDetector::parameters&) =
      cornerDetector::parameters::copy;
    (this->*p_copy)(other);
# endif


    variance = other.variance;
    kernelSize = other.kernelSize;
    maximumCorners = other.maximumCorners;
    scale = other.scale;
    localMaximaParameters.copy(other.localMaximaParameters);
    gradientFunctorParameters.copy(other.gradientFunctorParameters);

    return *this;
  }

  // alias for copy member
  harrisCorners::parameters&
    harrisCorners::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* harrisCorners::parameters::clone() const {
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
  bool harrisCorners::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool harrisCorners::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"variance",variance);
      lti::write(handler,"kernelSize",kernelSize);
      lti::write(handler,"maximumCorners",maximumCorners);
      lti::write(handler,"scale",scale);
      lti::write(handler,"localMaximaParameters",localMaximaParameters);
      lti::write(handler,"gradientFunctorParameters",
                 gradientFunctorParameters);
      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && cornerDetector::parameters::write(handler,false);
# else
    bool (cornerDetector::parameters::* p_writeMS)(ioHandler&,const bool) const =
      cornerDetector::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool harrisCorners::parameters::write(ioHandler& handler,
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
  bool harrisCorners::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool harrisCorners::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"variance",variance);
      lti::read(handler,"kernelSize",kernelSize);
      lti::read(handler,"maximumCorners",maximumCorners);
      lti::read(handler,"scale",scale);
      lti::read(handler,"localMaximaParameters",localMaximaParameters);
      lti::read(handler,"gradientFunctorParameters",
                 gradientFunctorParameters);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && cornerDetector::parameters::read(handler,false);
# else
    bool (cornerDetector::parameters::* p_readMS)(ioHandler&,const bool) =
      cornerDetector::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool harrisCorners::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // harrisCorners
  // --------------------------------------------------

  // default constructor
  harrisCorners::harrisCorners()
    : cornerDetector(), gradient() {

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  harrisCorners::harrisCorners(const parameters& par)
    : cornerDetector(), gradient() {
    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  harrisCorners::harrisCorners(const harrisCorners& other)
    : cornerDetector(), gradient() {
    copy(other);
  }

  // destructor
  harrisCorners::~harrisCorners() {
  }

  // returns the name of this type
  const char* harrisCorners::getTypeName() const {
    return "harrisCorners";
  }

  // copy member
  harrisCorners&
  harrisCorners::copy(const harrisCorners& other) {
    cornerDetector::copy(other);
    return (*this);
  }

  // alias for copy member
  harrisCorners& harrisCorners::operator=(const harrisCorners& other) {
    return (copy(other));
  }


  // clone member
  functor* harrisCorners::clone() const {
    return new harrisCorners(*this);
  }

  // return parameters
  const harrisCorners::parameters&
    harrisCorners::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool harrisCorners::updateParameters() {
    
    bool b = true;

    gradientFunctor::parameters gradP = 
      getParameters().gradientFunctorParameters;

    gradP.format = gradientFunctor::parameters::Cartesic;

    b = b && gradient.setParameters(gradP);

    return b;
  }

  /*
   * read the functor parameters
   */
  bool harrisCorners::read(ioHandler& handler, const bool complete) {

    bool b = cornerDetector::read(handler, complete);

    gradientFunctor::parameters gradP = 
      getParameters().gradientFunctorParameters;

    gradP.format = gradientFunctor::parameters::Cartesic;

    b = b && gradient.setParameters(gradP);

    return b;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On place apply for type channel8!
  bool harrisCorners::apply(channel8& srcdest) const {
    return cornerDetector::apply(srcdest);
  };

  // On place apply for type channel!
  bool harrisCorners::apply(channel& srcdest) const {
    return cornerDetector::apply(srcdest);
  };

  // On copy apply for type channel8!
  bool harrisCorners::apply(const channel8& src,channel8& dest) const {
    pointList pts;
    if (apply(src,pts)) {
      const parameters& par = getParameters();
      pointList::iterator it;
      dest.resize(src.size(),par.noCornerValue,false,true);
      for (it=pts.begin();it!=pts.end();++it) {
        dest.at(*it)=par.cornerValue;
      }
      return true;
    }

    return false;
  };

  // On copy apply for type channel!
  bool harrisCorners::apply(const channel& src,channel& dest) const {
    const parameters& param = getParameters();

    channel gx,gy,fxy,tmp;
    float maxCornerness;

    pointList cornerMax;

    if (!gradient.apply(src,gx,gy)) {
      appendStatusString(gradient);
      dest.clear();
      return false;
    }
    getSecondOrder(gx,gy,fxy);
    getCornerness(gx,fxy,gy,param.scale,tmp,maxCornerness);
    findCornerMaxima(tmp,dest,cornerMax);

    return true;
  };

  /*
   * operates on a copy of the given %parameters.
   * @param src channel8 with the source data.
   * @param dest list of corners
   * @return true if apply successful or false otherwise.
   */
  bool harrisCorners::apply(const channel8& src,pointList& dest) const {
    channel tmp;
    tmp.castFrom(src);
    return apply(tmp,dest);
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src channel with the source data.
   * @param dest list of corners
   * @return true if apply successful or false otherwise.
   */
  bool harrisCorners::apply(const channel& src,pointList& dest) const {
    const parameters& param = getParameters();

    channel gx,gy,fxy,tmp,d;
    float maxCornerness;

    if (!gradient.apply(src,gx,gy)) {
      appendStatusString(gradient);
      dest.clear();
      return false;
    }
    getSecondOrder(gx,gy,fxy);
    getCornerness(gx,fxy,gy,param.scale,tmp,maxCornerness);
    return findCornerMaxima(tmp,d,dest);
  };

  /*
   * operates on a copy of the given %parameters.
   * @param src channel with the source data.
   * @param dest list of corners
   * @return true if apply successful or false otherwise.
   */
  bool harrisCorners::apply(const channel& src,
                            channel& cornerness,
                            float& maxCornerness) const {
    const parameters& param = getParameters();

    channel gx,gy,fxy,d;

    if (!gradient.apply(src,gx,gy)) {
      appendStatusString(gradient);
      cornerness.clear();
      maxCornerness = 0.0f;
      return false;
    }
    getSecondOrder(gx,gy,fxy);
    return getCornerness(gx,fxy,gy,param.scale,cornerness,maxCornerness);
  };

  bool harrisCorners::apply(const channel& src,
                            channel& cornerness,
                            float& maxCornerness,
                            pointList& dest) const {
    const parameters& param = getParameters();

    channel gx,gy,fxy,d;

    if (!gradient.apply(src,gx,gy)) {
      appendStatusString(gradient);
      dest.clear();
      return false;
    }
    getSecondOrder(gx,gy,fxy);
    getCornerness(gx,fxy,gy,param.scale,cornerness,maxCornerness);
    return findCornerMaxima(cornerness,d,dest);
  };

  /*
   * compute the second order.
   */
  bool harrisCorners::getSecondOrder(channel& gx,
                                     channel& gy,
                                     channel& fxy) const {

    const parameters& par = getParameters();

    fxy.resize(gx.size(),false,false);
    gaussKernel2D<float> gk(par.kernelSize,par.variance);
    convolution filter;
    convolution::parameters filterPar;
    filterPar.boundaryType = lti::Constant;
    filterPar.setKernel(gk);
    filter.setParameters(filterPar);

    channel::iterator igx=gx.begin();
    channel::iterator igxend=gx.end();
    channel::iterator igy=gy.begin();
    channel::iterator ifxy=fxy.begin();
    float tx, ty;

    while (igx!=igxend) {
      tx=(*igx);
      ty=(*igy);
      (*igx)=tx*tx;
      (*igy)=ty*ty;
      (*ifxy)=tx*ty;
      ++igx; ++igy; ++ifxy;
    }

    return (filter.apply(gx) && filter.apply(gy) && filter.apply(fxy));
  }


  /*
   * compute cornerness
   */
  bool harrisCorners::getCornerness(const channel& fxx,
                                    const channel& fxy,
                                    const channel& fyy,
                                    const float scale,
                                    channel& cornerness,
                                    float& maxCornerness) const {
    // we can assume that all channels are connected, but try it out if not
    if ((fxx.getMode() != channel::Connected) ||
        (fxy.getMode() != channel::Connected) ||
        (fyy.getMode() != channel::Connected)) {
      setStatusString("Channels not contigous in getCornerness");
      return false;
    }
    
    if (fxx.empty() || fxy.empty() || fyy.empty()) {
      cornerness.clear();
      maxCornerness = 0.0f;
      return false;
    }
    
    int i;
    const int end = fxx.rows()*fxx.columns();
    const float *const pfxx = &fxx.at(0);
    const float *const pfxy = &fxy.at(0);
    const float *const pfyy = &fyy.at(0);

    cornerness.resize(fxx.size(),0,false,false);
    float* pcor = &cornerness.at(0);

    float det,trace,txx,txy,tyy,c;
    float maxc = 0.0f;

    for (i=0;i<end;++i) {
      txx=pfxx[i];
      txy=pfxy[i];
      tyy=pfyy[i];
      det=txx*tyy - txy*txy;
      trace=txx+tyy;
      c = det-scale*trace*trace;
      pcor[i]=c;
      if (c>maxc) {
        maxc=c;
      }
    }

    maxCornerness = maxc;
    return true;
  }

  /*
   * find corners with maximal cornerness
   */
  bool harrisCorners::findCornerMaxima(const channel& cornerness,
                                             channel& cornersOnly,
                                             pointList& cornerMax) const {
    if (cornerness.empty()) {
      cornersOnly.clear();
      cornerMax.clear();
      return true;
    }

    const parameters& par = getParameters();

    const float corner = par.cornerValue/255.0f;
    const float noCorner = par.noCornerValue/255.0f;

    localMaxima<float> lmax;
    localMaxima<float>::parameters lmaxPar(par.localMaximaParameters);
    lmaxPar.noMaxValue = noCorner;
    lmaxPar.maxNumber = par.maximumCorners;
    lmax.setParameters(lmaxPar);

    if (lmax.apply(cornerness,cornersOnly,cornerMax)) {
      pointList::iterator it;
      int i;
      for (it=cornerMax.begin(),i=0;
           (it!=cornerMax.end());
           ++it) {
        cornersOnly.at(*it) = corner;
      }

      for (;it!=cornerMax.end();++it) {
        cornersOnly.at(*it) = noCorner;
      }

      return true;
    }
    return false;
  }

}
