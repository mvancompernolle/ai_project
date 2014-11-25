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


/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiCurvatureFeature.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 5.12.2001
 * revisions ..: $Id: ltiCurvatureFeature.cpp,v 1.12 2006/09/05 10:08:34 ltilib Exp $
 */


#include "ltiCurvatureFeature.h"
#include "ltiBilinearInterpolator.h"
#include "ltiGaussKernels.h"
#include "ltiConvolution.h"

namespace lti {
  // --------------------------------------------------
  // curvatureFeature::parameters
  // --------------------------------------------------

  // default constructor
  curvatureFeature::parameters::parameters()
    : globalFeatureExtractor::parameters() {
    gaussianVariance = float(-1);
    deltaLength = float(0.50);
    relevanceThreshold = float(0.0);
    gaussianSize = int(3);
    size = int(64);
    normalize = bool(true);
    lowerLimit = static_cast<float>(-Pi/2.0);
    upperLimit = static_cast<float>(Pi/2.0);
  }

  // copy constructor
  curvatureFeature::parameters::parameters(const parameters& other)
    : globalFeatureExtractor::parameters()  {
    copy(other);
  }

  // destructor
  curvatureFeature::parameters::~parameters() {
  }

  // get type name
  const char* curvatureFeature::parameters::getTypeName() const {
    return "curvatureFeature::parameters";
  }

  // copy member

  curvatureFeature::parameters&
    curvatureFeature::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    globalFeatureExtractor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    globalFeatureExtractor::parameters& (globalFeatureExtractor::parameters::* p_copy)
      (const globalFeatureExtractor::parameters&) =
      globalFeatureExtractor::parameters::copy;
    (this->*p_copy)(other);
# endif

    gaussianVariance = other.gaussianVariance;
    deltaLength = other.deltaLength;
    relevanceThreshold = other.relevanceThreshold;
    gaussianSize = other.gaussianSize;
    size = other.size;
    normalize = other.normalize;
    lowerLimit = other.lowerLimit;
    upperLimit = other.upperLimit;
    return *this;
  }

  // alias for copy member
  curvatureFeature::parameters&
    curvatureFeature::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* curvatureFeature::parameters::clone() const {
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
  bool curvatureFeature::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool curvatureFeature::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"gaussianVariance",gaussianVariance);
      lti::write(handler,"deltaLength",deltaLength);
      lti::write(handler,"relevanceThreshold",relevanceThreshold);
      lti::write(handler,"gaussianSize",gaussianSize);
      lti::write(handler,"size",size);
      lti::write(handler,"normalize",normalize);
      lti::write(handler,"lowerLimit",lowerLimit);
      lti::write(handler,"upperLimit",upperLimit);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::write(handler,false);
# else
    bool (globalFeatureExtractor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      globalFeatureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool curvatureFeature::parameters::write(ioHandler& handler,
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
  bool curvatureFeature::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool curvatureFeature::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"gaussianVariance",gaussianVariance);
      lti::read(handler,"deltaLength",deltaLength);
      lti::read(handler,"relevanceThreshold",relevanceThreshold);
      lti::read(handler,"gaussianSize",gaussianSize);
      lti::read(handler,"size",size);
      lti::read(handler,"normalize",normalize);
      lti::read(handler,"lowerLimit",lowerLimit);
      lti::read(handler,"upperLimit",upperLimit);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::read(handler,false);
# else
    bool (globalFeatureExtractor::parameters::* p_readMS)(ioHandler&,const bool) =
      globalFeatureExtractor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool curvatureFeature::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // curvatureFeature
  // --------------------------------------------------

  // default constructor
  curvatureFeature::curvatureFeature()
    : globalFeatureExtractor(){

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

  // copy constructor
  curvatureFeature::curvatureFeature(const curvatureFeature& other)
    : globalFeatureExtractor()  {
    copy(other);
  }

  // destructor
  curvatureFeature::~curvatureFeature() {
  }

  // returns the name of this type
  const char* curvatureFeature::getTypeName() const {
    return "curvatureFeature";
  }

  // copy member
  curvatureFeature& curvatureFeature::copy(const curvatureFeature& other) {
    globalFeatureExtractor::copy(other);

    return (*this);
  }

  // alias for copy member
  curvatureFeature&
    curvatureFeature::operator=(const curvatureFeature& other) {
    return (copy(other));
  }


  // clone member
  functor* curvatureFeature::clone() const {
    return new curvatureFeature(*this);
  }

  // return parameters
  const curvatureFeature::parameters&
    curvatureFeature::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool curvatureFeature::apply(const channel& orientation,
                               const channel& magnitude,
                               dvector& vct) const {
    channel dest;
    return apply(orientation,magnitude,dest,vct);
  }

  // apply for features
  bool curvatureFeature::apply(const channel& orientation,
                               const channel& magnitude,
                               channel& dest,
                               dvector& vct) const {


    if (!apply(orientation,magnitude,dest)) {
      return false;
    }

    // generate histogram
    const parameters& param = getParameters();

    int y;
    vector<channel::value_type>::const_iterator mit,mite,oit;
    const float m = (param.size-1)/(param.upperLimit-param.lowerLimit);
    const float b = -m*param.lowerLimit + 0.5f; // 0.5f for better round to int
    int idx;

    // initialize histogram with 0
    vct.resize(param.size,0.0,false,true);

    for (y=0;y<magnitude.rows();++y) {

      const vector<channel::value_type>& magv = magnitude.getRow(y);
      const vector<channel::value_type>& oriv = dest.getRow(y);

      for (mit=magv.begin(),mite=magv.end(),oit=oriv.begin();
           mit!=mite;
           ++mit,++oit) {
        idx = static_cast<int>(m*(*oit) + b);
        if ((*mit > param.relevanceThreshold) &&
            (idx >= 0) &&
            (idx < vct.size())) {
          vct.at(idx)+=(*mit);
        }
      }
    }

    if (param.normalize) {
      vct.divide(vct.sumOfElements());
    }

    return true;
  };

  // apply for curvature map
  bool curvatureFeature::apply(const channel& orientation,
                               const channel& magnitude,
                               channel& dest) const {

    if (magnitude.empty() || orientation.empty()) {
      dest.clear();
      setStatusString("input data empty");
      return false;
    }

    if (orientation.size() != magnitude.size()) {
      dest.clear();
      setStatusString("input channels must have the same size");
      return false;
    }

    const parameters& param = getParameters();

    // 1. separate the orientation in cos and sin channels
    channel cosine,sine;
    cosine.resize(orientation.size(),0,false,false);
    sine.resize(orientation.size(),0,false,false);

    int x,y;
    for (y=0;y<orientation.rows();++y) {
      for (x=0;x<orientation.columns();++x) {
        sincos(orientation.at(y,x), sine.at(y,x), cosine.at(y,x));
      }
    }

    // 2. Filter cosine/sine channels

    if (param.gaussianSize > 1) {
      gaussKernel2D<float> kern(param.gaussianSize,param.gaussianVariance);
      convolution filter(kern);
      filter.apply(cosine);
      filter.apply(sine);
    }

    // 3. compute curvature channel.

    dest.resize(orientation.size(),0,false,false);

    bilinearInterpolator<float> interpol;
    bilinearInterpolator<float>::parameters ipParam;
    ipParam.boundaryType = lti::Constant;
    float x1,y1,x2,y2,tx,ty;
    const float delta = param.deltaLength;
    const float ddelta = delta*2.0f;
    float cosa,cosb,sina,sinb;

    for (y=0;y<orientation.rows();++y) {
      for (x=0;x<orientation.columns();++x) {
        if (magnitude.at(y,x) > param.relevanceThreshold) {
          // the angles in sine and cosine are the angles of the
          // tangent to the pixel-direction.

          // we need now to determine the "neighbour" pixels in the
          // pixel contour:
          tx = -sine.at(y,x)*delta;
          ty = cosine.at(y,x)*delta;

          x1 = x + tx;
          y1 = y + ty;

          x2 = x - tx;
          y2 = y - ty;

          // now we compute the derivative as the difference divided
          // by length.

          // this is cos(angle.at(y1,x1) - angle.at(y2,x2))
          cosa = interpol.apply(cosine,y1,x1);
          cosb = interpol.apply(cosine,y2,x2);

          sina = interpol.apply(sine,y1,x1);
          sinb = interpol.apply(sine,y2,x2);

          tx = cosa*cosb + sina*sinb;
          ty = sina*cosb - cosa*sinb;

          dest.at(y,x) = atan2(ty,tx)/ddelta;
        } else {
          dest.at(y,x) = 0;
        }
      }
    }

    return true;
  };



}
