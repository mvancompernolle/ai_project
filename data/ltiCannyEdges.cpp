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
 * file .......: ltiCannyEdges.cpp
 * authors ....: Christian Harte
 * organization: LTI, RWTH Aachen
 * creation ...: 19.7.2002
 * revisions ..: $Id: ltiCannyEdges.cpp,v 1.16 2006/09/05 10:04:31 ltilib Exp $
 */

#include "ltiCannyEdges.h"
#include "ltiGaussKernels.h"
#include "ltiGradientKernels.h"
#include "ltiConvolution.h"
#include "ltiSplitImageToRGB.h"
#include "ltiMaximumFilter.h"
#include "ltiConstants.h"
#include "ltiColorContrastGradient.h"

#include <limits>

#undef _LTI_DEBUG
//#define _LTI_DEBUG 2
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#include "ltiViewer.h"
#endif

namespace lti {
  // --------------------------------------------------
  // cannyEdges::parameters
  // --------------------------------------------------

  // default constructor
  cannyEdges::parameters::parameters()
    : edgeDetector::parameters() {

    variance = float(1.0f);
    kernelSize = 7;
    thresholdMin = float(0.5f);
    thresholdMax = float(0.04f);
    gradientParameters.kernelType = gradientFunctor::parameters::Difference;
    gradientParameters.format     = gradientFunctor::parameters::Polar;
  }

  // copy constructor
  cannyEdges::parameters::parameters(const parameters& other)
    : edgeDetector::parameters()  {
    copy(other);
  }

  // destructor
  cannyEdges::parameters::~parameters() {
  }

  // get type name
  const char* cannyEdges::parameters::getTypeName() const {
    return "cannyEdges::parameters";
  }

  // copy member

  cannyEdges::parameters&
    cannyEdges::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    edgeDetector::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    edgeDetector::parameters& (edgeDetector::parameters::* p_copy)
      (const edgeDetector::parameters&) =
      edgeDetector::parameters::copy;
    (this->*p_copy)(other);
# endif

    variance = other.variance;
    kernelSize = other.kernelSize;
    thresholdMin = other.thresholdMin;
    thresholdMax = other.thresholdMax;
    gradientParameters = other.gradientParameters;
    return *this;
  }

  // alias for copy member
  cannyEdges::parameters&
    cannyEdges::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* cannyEdges::parameters::clone() const {
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
  bool cannyEdges::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool cannyEdges::parameters::writeMS(ioHandler& handler,
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
      lti::write(handler,"thresholdMin",thresholdMin);
      lti::write(handler,"thresholdMax",thresholdMax);
      lti::write(handler,"gradientParameters",gradientParameters);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && edgeDetector::parameters::write(handler,false);
# else
    bool (edgeDetector::parameters::* p_writeMS)(ioHandler&,const bool) const =
      edgeDetector::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool cannyEdges::parameters::write(ioHandler& handler,
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
  bool cannyEdges::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool cannyEdges::parameters::readMS(ioHandler& handler,
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
      lti::read(handler,"thresholdMin",thresholdMin);
      lti::read(handler,"thresholdMax",thresholdMax);
      lti::read(handler,"gradientParameters",gradientParameters);
      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && edgeDetector::parameters::read(handler,false);
# else
    bool (edgeDetector::parameters::* p_readMS)(ioHandler&,const bool) =
      edgeDetector::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool cannyEdges::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // cannyEdges
  // --------------------------------------------------

  /*
   * convert a pair of values in cartesic system into a pair in
   * polar system
   */
  inline void cannyEdges::cartToPolar(const int dx,const int dy,
                                      float& mag,float& angle) const {
    static const float factor = 1.0f/255.0f;
    mag = factor*sqrt(float(dx*dx+dy*dy));
    angle = atan2(dy,dx);
  }

  // default constructor
  cannyEdges::cannyEdges()
    : edgeDetector(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // default constructor
  cannyEdges::cannyEdges(const parameters& thePars)
    : edgeDetector() {

    // set the default parameters
    setParameters(thePars);
  }


  // copy constructor
  cannyEdges::cannyEdges(const cannyEdges& other)
    : edgeDetector() {

    copy(other);
  }

  // destructor
  cannyEdges::~cannyEdges() {
  }

  // returns the name of this type
  const char* cannyEdges::getTypeName() const {
    return "cannyEdges";
  }

  // copy member
  cannyEdges& cannyEdges::copy(const cannyEdges& other) {
    edgeDetector::copy(other);

    return (*this);
  }

  // alias for copy member
  cannyEdges&
    cannyEdges::operator=(const cannyEdges& other) {
    return (copy(other));
  }


  // clone member
  functor* cannyEdges::clone() const {
    return new cannyEdges(*this);
  }

  // return parameters
  const cannyEdges::parameters&
  cannyEdges::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // set parameters
  bool cannyEdges::updateParameters() {
    const parameters& par = getParameters();
    nonMaximaSuppression::parameters nmsPar;
    nmsPar.thresholdMin = par.thresholdMin;
    nmsPar.thresholdMax = (par.thresholdMax<=1.0) ? par.thresholdMax :
      par.thresholdMax/255.0f;
    nmsPar.edgeValue    = par.edgeValue;
    nmsPar.background   = par.noEdgeValue;
    nmsPar.checkAngles  = false;
    
    colorContrastGradient::parameters gradPar(par.gradientParameters);
    gradPar.format = colorContrastGradient::parameters::Polar;
    
    return (grad.setParameters(gradPar) &&
            suppressor.setParameters(nmsPar));
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------


  // On place apply for type channel8!
  bool cannyEdges::apply(channel8& srcdest) const {
    return edgeDetector::apply(srcdest);
  }

  // On place apply for type channel!
  bool cannyEdges::apply(channel& srcdest) const {
    return edgeDetector::apply(srcdest);
  }

  // On copy apply for type channel8!
  bool cannyEdges::apply(const channel8& src,channel8& dest) const {
    channel orient;
    return apply(src,dest,orient);
  }

  // On copy apply for type channel8!
  bool cannyEdges::apply(const channel8& src,
                               channel8& dest,
                               channel& orientation) const {

    const parameters& param = getParameters();
    if (src.empty()) {
      setStatusString("input image empty");
      return(false);
    }

    dest.resize(src.size(),ubyte(),false,false);

    // temporary images
    channel8 chanSmoothed; // low-pass filtered image (noise reduction)
    channel gradAbs;       // magnitude of gradient

    gradAbs.resize(src.size(),0.0f,false,false);
    orientation.resize(src.size(),0.0f,false,false);

    _lti_debug("Gauss Filter");
    const channel8* smoothPtr = 0;

    // apply Gauss filter to smooth image, only if desired
    if (param.kernelSize > 0) {
      gaussKernel2D<ubyte> kernel(param.kernelSize,param.variance);
      
      convolution::parameters filterParam;       // parameters
      filterParam.setKernel(kernel);             // use the gauss kernel
      filterParam.boundaryType = lti::Constant;
      convolution filter(filterParam);           // convolution operator
      
      // filter the channel 
      filter.apply(src,chanSmoothed);
      smoothPtr = &chanSmoothed;
    } else {
      smoothPtr = &src;
    }

    _lti_debug("Differentiation and result in polar coordinates");

    // compute the gradient
    if (grad.apply(*smoothPtr,gradAbs,orientation)) {

#     ifdef _LTI_DEBUG
      static viewer viewAbs("Abs"),viewArg("Arg");
      viewAbs.show(gradAbs);
      viewArg.show(orientation);
#     endif      

      // if gradient ok, then non-maxima suppression and hystheresis
      return suppressor.apply(gradAbs,orientation,dest);
    }
   
    return false;
  };

  // On copy apply for type channel8!
  bool cannyEdges::apply(const channel& src,channel8& dest) const {
    channel8 tmp;
    tmp.castFrom(src);
    return apply(tmp,dest);
  }

  // On copy apply for type channel8!
  bool cannyEdges::apply(const channel& src,channel8& dest,channel& o) const {
    channel8 tmp;
    tmp.castFrom(src);
    return apply(tmp,dest,o);
  }


  // On copy apply for type channel!
  bool cannyEdges::apply(const channel& src,channel& dest) const {
    channel8 tmp;
    if (apply(src,tmp)) {
      dest.castFrom(tmp);
      return true;
    }
    return false;
  };

  // On copy apply for type channel!
  bool cannyEdges::apply(const image& src,channel8& dest) const {
    splitImageToRGB splitter;
    channel r,g,b,o;
    splitter.apply(src,r,g,b);
    
    return apply(r,g,b,dest,o);
  };

  // On copy apply for type channel!
  bool cannyEdges::apply(const image& src,channel8& dest,channel& ori) const {
    splitImageToRGB splitter;
    channel r,g,b;
    splitter.apply(src,r,g,b);
    
    return apply(r,g,b,dest,ori);
  };

  // On copy apply for type channel!
  bool cannyEdges::apply(const image& src,image& dest) const {
    return edgeDetector::apply(src,dest);
  };

  // On copy apply for type channel!
  bool cannyEdges::apply(image& srcdest) const {
    return edgeDetector::apply(srcdest);
  };

  bool cannyEdges::apply(const channel& c1,
                         const channel& c2,
                         const channel& c3,
                         channel8& dest,
                         channel& orientation) const {

    const parameters& param = getParameters();

    if ((c1.size() != c2.size()) ||
        (c1.size() != c3.size())) {
      setStatusString("Input channels have different sizes");
      return false;
    }

    if (c1.empty()) {
      setStatusString("input image empty");
      return(false);
    }    

    dest.resize(c1.size(),ubyte(),false,false);

    // temporary images
    channel c1s,c2s,c3s; // low-pass filtered image (noise reduction)
    channel gradAbs;       // magnitude of gradient
    const channel *c1p,*c2p,*c3p;
    c1p=c2p=c3p=0;

    gradAbs.resize(c1.size(),0.0f,false,false);
    orientation.resize(c1.size(),0.0f,false,false);

    _lti_debug("Gauss Filter");

    // apply gauss filter to smooth image, only if desired
    if (param.kernelSize > 0) {
      
      gaussKernel2D<ubyte> kernel(param.kernelSize,param.variance);
      
      convolution::parameters filterParam;       // parameters
      filterParam.setKernel(kernel);             // use the gauss kernel
      filterParam.boundaryType = lti::Constant;
      convolution filter(filterParam);           // convolution operator
      
      // filter the channel 
      filter.apply(c1,c1s);
      filter.apply(c2,c2s);
      filter.apply(c3,c3s);
      c1p=&c1s;
      c2p=&c2s;
      c3p=&c3s;
    } else {
      c1p=&c1;
      c2p=&c2;
      c3p=&c3;
    }

    _lti_debug("Differentiation and result in polar coordinates");

    float maxVal;
    
    // compute color contrast gradient
    if (grad.apply(*c1p,*c2p,*c3p,gradAbs,orientation,maxVal)) {

#     ifdef _LTI_DEBUG
      static viewer viewAbs("Abs Color"),viewArg("Arg");
      viewAbs.show(gradAbs);
      viewArg.show(orientation);
#     endif      

      // non-maxima suppression
      return suppressor.apply(gradAbs,orientation,dest,maxVal);
    }   
    
    return false;
  }

}
