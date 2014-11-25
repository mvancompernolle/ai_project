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
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiAxOGDFeature.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 21.5.2001
 * revisions ..: $Id: ltiAxOGDFeature.cpp,v 1.16 2006/09/05 10:02:34 ltilib Exp $
 */


#include "ltiObject.h"
#include "ltiBilinearInterpolator.h"
#include "ltiAxOGDFeature.h"
#include "ltiOgdFilter.h"
#include "ltiGaussianPyramid.h"
#include "ltiGaussKernels.h"
#include "ltiDownsampling.h"
#include "ltiConvolution.h"
#include "ltiSquareConvolution.h"
#include "ltiSplitImageToRGB.h"
#include <set>

//#define _LTI_DEBUG 1

#ifdef _LTI_DEBUG

#include <iostream>

using std::cout;
using std::endl;

#endif


namespace lti {
  // --------------------------------------------------
  // axOGDFeature::parameters
  // --------------------------------------------------

  // default constructor
  axOGDFeature::parameters::parameters()
    : localFeatureExtractor::parameters(),
      globalFeatureExtractor::parameters() {

    gaussian = bool(true);
    windowSize = 13;
    windowVariance = -1.0;
    ogdVariance = double(2.0);
    voices = bool(true);
    ogdSize = int(13);
    ogdOrder = int(2);
    levels = int(4);
    considerPhase = bool(false);
    computeEdgeOrientation = bool(true);
  }

  // copy constructor
  axOGDFeature::parameters::parameters(const parameters& other)
    : localFeatureExtractor::parameters(),
      globalFeatureExtractor::parameters() {
    copy(other);
  }

  // destructor
  axOGDFeature::parameters::~parameters() {
  }

  // get type name
  const char* axOGDFeature::parameters::getTypeName() const {
    return "axOGDFeature::parameters";
  }

  // copy member

  axOGDFeature::parameters&
    axOGDFeature::parameters::copy(const parameters& other) {
// We need to use the old workaround here for .NET, as well, since the compiler
// has runtime problems with calling the second copy function.
#if defined(_LTI_MSC_6) || defined(_LTI_MSC_DOT_NET_2003)

    // We have to use this workaround as MS Visual C++ 6 is not able to compile the correct code below.
    // Conditional on that, copy may not be virtual.
    localFeatureExtractor::parameters& 
      (localFeatureExtractor::parameters::* p_copy)
      (const localFeatureExtractor::parameters&) =
      localFeatureExtractor::parameters::copy;
    (this->*p_copy)(other);

    globalFeatureExtractor::parameters& 
      (globalFeatureExtractor::parameters::* p_copy2)
      (const globalFeatureExtractor::parameters&) =
      globalFeatureExtractor::parameters::copy;
    (this->*p_copy2)(other);
# else
    // MS Visual C++ 6 is not able to compile this...
    localFeatureExtractor::parameters::copy(other);
    globalFeatureExtractor::parameters::copy(other);
# endif

    gaussian = other.gaussian;
    windowSize = other.windowSize;
    windowVariance = other.windowVariance;
    ogdVariance = other.ogdVariance;
    voices = other.voices;
    ogdSize = other.ogdSize;
    ogdOrder = other.ogdOrder;
    levels = other.levels;
    considerPhase=other.considerPhase;
    computeEdgeOrientation = other.computeEdgeOrientation;

    return *this;
  }

  // alias for copy member
  axOGDFeature::parameters&
    axOGDFeature::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* axOGDFeature::parameters::clone() const {
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
  bool axOGDFeature::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool axOGDFeature::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"ogdOrder",ogdOrder);
      lti::write(handler,"ogdSize",ogdSize);
      lti::write(handler,"ogdVariance",ogdVariance);
      lti::write(handler,"levels",levels);
      lti::write(handler,"voices",voices);
      lti::write(handler,"considerPhase",considerPhase);
      lti::write(handler,"gaussian",gaussian);
      lti::write(handler,"windowSize",windowSize);
      lti::write(handler,"windowVariance",windowVariance);
      lti::write(handler,"computeEdgeOrientation",computeEdgeOrientation);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && localFeatureExtractor::parameters::write(handler,false);
    b = b && globalFeatureExtractor::parameters::write(handler,false);
# else
    bool (localFeatureExtractor::parameters::* p_writeMS)
      (ioHandler&,const bool) const =
      localFeatureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
    bool (globalFeatureExtractor::parameters::* p_writeMS2)
      (ioHandler&,const bool) const =
      globalFeatureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS2)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool axOGDFeature::parameters::write(ioHandler& handler,
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
  bool axOGDFeature::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool axOGDFeature::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"ogdOrder",ogdOrder);
      lti::read(handler,"ogdSize",ogdSize);
      lti::read(handler,"ogdVariance",ogdVariance);
      lti::read(handler,"levels",levels);
      lti::read(handler,"voices",voices);
      lti::read(handler,"considerPhase",considerPhase);
      lti::read(handler,"gaussian",gaussian);
      lti::read(handler,"windowSize",windowSize);
      lti::read(handler,"windowVariance",windowVariance);
      lti::read(handler,"computeEdgeOrientation",computeEdgeOrientation);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && localFeatureExtractor::parameters::read(handler,false);
    b = b && globalFeatureExtractor::parameters::read(handler,false);
# else
    bool (localFeatureExtractor::parameters::* p_readMS)
      (ioHandler&,const bool) =
      localFeatureExtractor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
    bool (globalFeatureExtractor::parameters::* p_readMS2)
      (ioHandler&,const bool) =
      globalFeatureExtractor::parameters::readMS;
    b = b && (this->*p_readMS2)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool axOGDFeature::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // axOGDFeature
  // --------------------------------------------------

  // default constructor
  axOGDFeature::axOGDFeature()
    : localFeatureExtractor(),globalFeatureExtractor(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  axOGDFeature::axOGDFeature(const axOGDFeature& other)
    : localFeatureExtractor(),globalFeatureExtractor() {
    copy(other);
  }

  // destructor
  axOGDFeature::~axOGDFeature() {
  }

  // returns the name of this type
  const char* axOGDFeature::getTypeName() const {
    return "axOGDFeature";
  }

  // copy member
  axOGDFeature& axOGDFeature::copy(const axOGDFeature& other) {
    localFeatureExtractor::copy(other);
    globalFeatureExtractor::copy(other);
    return (*this);
  }

  // clone member
  functor* axOGDFeature::clone() const {
    return new axOGDFeature(*this);
  }

  // return parameters
  const axOGDFeature::parameters&
    axOGDFeature::getParameters() const {
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

  bool axOGDFeature::apply(const image& img,dvector& dest) const {
    splitImageToRGB splitter;
    channel r,g,b;
    splitter.apply(img,r,g,b);
    dvector tmp;
    if (apply(r,dest)) {
      dest.resize(dest.size()*3,0,true,false);

      if (apply(g,tmp)) {
        dest.fill(tmp,tmp.size());
        if (apply(b,tmp)) {
          dest.fill(tmp,2*tmp.size());
          return true;
        }
      }
    }

    dest.clear();
    return false;
  }

  // On copy apply for type channel!
  bool axOGDFeature::apply(const channel& src,dvector& dest) const {

    if (src.empty()) {
      dest.clear();
      setStatusString("source channel empty");
      return false;
    }

    const parameters& param = getParameters();

    gaussianPyramid<channel> thePyramid(param.levels);
    thePyramid.generate(src);

    int n(0),i,voiceFactor,aFactor;
    double eFactor(1.0);

    if (param.considerPhase) {
      n = (1+2*param.ogdOrder); // a0,a1,theta1,[a2,theta2]
    } else {
      n = (param.ogdOrder+1);   // a0,a1,[a2]
    }

    if (param.voices) {
      eFactor = double(1<<(param.ogdOrder-1));
      aFactor = (2*param.levels);
      n = n*aFactor;
      voiceFactor = 2; // step in the final vector
    } else {
      aFactor = (param.levels);
      n = n*aFactor;
      voiceFactor = 1; // no voices: each element
    }

    dest.resize(n,0.0,false,false);

    switch (param.ogdOrder) {
        case 1: { // 1st order ogd
          channel p11,p12,p22;
          double a0,a1,theta1;
          for (i=0;i<param.levels;++i) {
            generatePowerBasisOgd1(thePyramid.at(i),false,
                                   p11,p12,p22);
            globalOgd1(p11,p12,p22,a0,a1,theta1);
            dest.at(i*voiceFactor) = a0;
            dest.at(i*voiceFactor+aFactor) = a1;
            if (param.considerPhase) {
              dest.at(i*voiceFactor+2*aFactor) = theta1;
            }

            if (param.voices) {
              generatePowerBasisOgd1(thePyramid.at(i),true,
                                     p11,p12,p22);
              globalOgd1(p11,p12,p22,a0,a1,theta1);
              dest.at(i*voiceFactor+1) = a0*eFactor;
              dest.at(i*voiceFactor+aFactor+1) = a1*eFactor;
              if (param.considerPhase) {
                dest.at(i*voiceFactor+2*aFactor+1) = theta1;
              }
            }
          }
        } break;
        case 2: { // 2nd order ogd
          channel p11,p12,p13,p22,p23,p33;
          double a0,a1,a2,theta1,theta2;

          for (i=0;i<param.levels;++i) {
            generatePowerBasisOgd2(thePyramid.at(i),false,
                                   p11,p12,p13,p22,p23,p33);
            globalOgd2(p11,p12,p13,p22,p23,p33,a0,a1,a2,theta1,theta2);
            dest.at(i*voiceFactor) = a0;
            dest.at(i*voiceFactor+aFactor) = a1;
            dest.at(i*voiceFactor+2*aFactor) = a2;

            if (param.considerPhase) {
              dest.at(i*voiceFactor+3*aFactor) = theta1;
              dest.at(i*voiceFactor+4*aFactor) = theta2;
            }

            if (param.voices) {
              generatePowerBasisOgd2(thePyramid.at(i),true,
                                     p11,p12,p13,p22,p23,p33);
              globalOgd2(p11,p12,p13,p22,p23,p33,a0,a1,a2,theta1,theta2);
              dest.at(i*voiceFactor+1) = a0*eFactor;
              dest.at(i*voiceFactor+aFactor+1) = a1*eFactor;
              dest.at(i*voiceFactor+2*aFactor+1) = a2*eFactor;

              if (param.considerPhase) {
                dest.at(i*voiceFactor+3*aFactor+1) = theta1;
                dest.at(i*voiceFactor+4*aFactor+1) = theta2;
              }
            }
          }
        } break;
        default: {
          setStatusString("OGD order not supported!");
          return false;
        }
    }

    return true;
  };

  // On copy apply for type channel!
  bool axOGDFeature::apply(const channel& src,
                           channel& a0,channel& a1,channel& theta1) const {
    return apply(src,false,0,a0,a1,theta1);
  }
  // On copy apply for type channel!
  bool axOGDFeature::apply(const channel& src,
                           const bool& voices,
                           const int& level,
                           channel& a0,channel& a1,channel& theta1) const {

    if (src.empty()) {
      a0.clear();
      a1.clear();
      theta1.clear();
      setStatusString("source channel empty");
      return false;
    }

    const parameters& param = getParameters();

    channel p11,p12,p22;

    generatePowerBasisOgd1(src,voices,p11,p12,p22);

    const double levelFactor = (1 << level);
    int kernelSize = static_cast<int>(0.5+param.windowSize/levelFactor);

    if ((kernelSize % 2) == 0) {
      kernelSize++;
    }

    if (param.gaussian) {

      double kernelVariance = param.windowVariance/(levelFactor*levelFactor);
      gaussKernel2D<float> k(kernelSize,kernelVariance);

      convolution conv;
      convolution::parameters convPar;
      convPar.boundaryType = lti::Mirror;
      convPar.setKernel(k);
      conv.setParameters(convPar);

      conv.apply(p11);
      conv.apply(p12);
      conv.apply(p22);

    } else {
      squareConvolution<float> conv;
      squareConvolution<float>::parameters convPar;
      convPar.boundaryType = lti::Mirror;
      conv.setParameters(convPar);

      conv.apply(p11);
      conv.apply(p12);
      conv.apply(p22);
    }

    int x,y;
    double e11(0),e12(0),e22(0);

    a0.resize(p11.size(),0.0f,false,false);
    a1.resize(p11.size(),0.0f,false,false);
    theta1.resize(p11.size(),0.0f,false,false);

    const int sizex=a0.columns();
    const int sizey=a0.rows();

    if (param.computeEdgeOrientation) {

      for (y=0;y<sizey;++y) {
        for (x=0;x<sizex;++x) {

          e11 = p11.at(y,x);
          e12 = p12.at(y,x);
          e22 = p22.at(y,x);

          a0.at(y,x) = static_cast<float>((e11+e22)/2.0);
          a1.at(y,x) = static_cast<float>(0.5*sqrt(e11*e11+e22*e22+4*e12*e12-2*e11*e22));
          theta1.at(y,x) = static_cast<float>(atan2((2*e12),(e11-e22))/2.0);
        }
      }

    } else {

      for (y=0;y<sizey;++y) {
        for (x=0;x<sizex;++x) {

          e11 = p11.at(y,x);
          e12 = p12.at(y,x);
          e22 = p22.at(y,x);

          a0.at(y,x) = static_cast<float>((e11+e22)/2.0);
          a1.at(y,x) = static_cast<float>(0.5*sqrt(e11*e11+e22*e22+4*e12*e12-2*e11*e22));
          theta1.at(y,x) = static_cast<float>(atan2((2*e12),(e11-e22)));
        }
      }
    }

    return true;
  };

  // On copy apply for type channel!
  bool axOGDFeature::apply(const channel& src,
                           channel& a0,channel& a1,channel& a2,
                           channel& theta1,channel& theta2) const {
    return apply(src,false,0,a0,a1,a2,theta1,theta2);
  }

  // On copy apply for type channel!
  bool axOGDFeature::apply(const channel& src,
                           const bool& voices,
                           const int& level,
                           channel& a0,channel& a1,channel& a2,
                           channel& theta1,channel& theta2) const {

    if (src.empty()) {
      a0.clear();
      a1.clear();
      a2.clear();
      theta1.clear();
      theta2.clear();
      setStatusString("source channel empty");
      return false;
    }

    const parameters& param = getParameters();

    channel p11,p12,p13,p22,p23,p33;

    generatePowerBasisOgd2(src,voices,p11,p12,p13,p22,p23,p33);

    const double levelFactor = (1 << level);
    int kernelSize = static_cast<int>(0.5+param.windowSize/levelFactor);

    if ((kernelSize % 2) == 0) {
      kernelSize++;
    }

    if (param.gaussian) {

      double kernelVariance = param.windowVariance/(levelFactor*levelFactor);
      gaussKernel2D<float> k(kernelSize,kernelVariance);

      convolution conv;
      convolution::parameters convPar;
      convPar.boundaryType = lti::Mirror;
      convPar.setKernel(k);
      conv.setParameters(convPar);

      conv.apply(p11);
      conv.apply(p12);
      conv.apply(p13);
      conv.apply(p22);
      conv.apply(p23);
      conv.apply(p33);

    } else {

      squareConvolution<float> conv;
      squareConvolution<float>::parameters convPar;
      convPar.boundaryType = lti::Mirror;

      int tmp = param.windowSize/2;
      if ((tmp % 2) == 0) {
        ++tmp; // size of the kernel must be odd
      }

      tmp /= 2;

      convPar.kernelSize = rectangle(-tmp,-tmp,tmp,tmp);
      convPar.kernelNorm = static_cast<float>(1.0/((4.0*tmp*(tmp+1.0))+1.0));

      conv.setParameters(convPar);

      conv.apply(p11);
      conv.apply(p12);
      conv.apply(p13);
      conv.apply(p22);
      conv.apply(p23);
      conv.apply(p33);
    }

    int x,y;
    double e11(0),e12(0),e13(0),e22(0),e23(0),e33(0);

    a0.resize(p11.size(),0.0f,false,false);
    a1.resize(p11.size(),0.0f,false,false);
    a2.resize(p11.size(),0.0f,false,false);
    theta1.resize(p11.size(),0.0f,false,false);
    theta2.resize(p11.size(),0.0f,false,false);

    const int sizex=a0.columns();
    const int sizey=a0.rows();

    double e11pe22;
    double e12p2e33;
    double e11me22h;
    double e13pe23;
    double e13me23h;
    double tmp;

    if (param.computeEdgeOrientation) {

      for (y=0;y<sizey;++y) {
        for (x=0;x<sizex;++x) {
          e11 = p11.at(y,x);
          e12 = p12.at(y,x);
          e13 = p13.at(y,x);
          e22 = p22.at(y,x);
          e23 = p23.at(y,x);
          e33 = p33.at(y,x);


          // some temporary variables
          e11pe22  = e11+e22;
          e12p2e33 = e12+2.0*e33;
          e11me22h = (e11-e22)/2.0;
          e13pe23  = e13 + e23;
          e13me23h = (e13 - e23)/2.0;

          a0.at(y,x) = static_cast<float>((3.0*(e11pe22)+2.0*e12p2e33)/8.0);
          a1.at(y,x) = static_cast<float>(sqrt(e11me22h*e11me22h + e13pe23*e13pe23));
          tmp = (e11pe22/8.0)-(e12p2e33/4.0);
          a2.at(y,x) = static_cast<float>(sqrt(tmp*tmp+e13me23h*e13me23h));
          theta1.at(y,x) = static_cast<float>(atan2(e13pe23,e11me22h)/2.0);
          theta2.at(y,x) = static_cast<float>(atan2(8*e13me23h,(e11pe22-2*e12p2e33))/4.0);
        }
      }
    } else {

      for (y=0;y<sizey;++y) {
        for (x=0;x<sizex;++x) {
          e11 = p11.at(y,x);
          e12 = p12.at(y,x);
          e13 = p13.at(y,x);
          e22 = p22.at(y,x);
          e23 = p23.at(y,x);
          e33 = p33.at(y,x);


          // some temporary variables
          e11pe22  = e11+e22;
          e12p2e33 = e12+2.0*e33;
          e11me22h = (e11-e22)/2.0;
          e13pe23  = e13 + e23;
          e13me23h = (e13 - e23)/2.0;

          a0.at(y,x) = static_cast<float>((3.0*(e11pe22)+2.0*e12p2e33)/8.0);
          a1.at(y,x) = static_cast<float>(sqrt(e11me22h*e11me22h + e13pe23*e13pe23));
          tmp = (e11pe22/8.0)-(e12p2e33/4.0);
          a2.at(y,x) = static_cast<float>(sqrt(tmp*tmp+e13me23h*e13me23h));
          theta1.at(y,x) = static_cast<float>(atan2(e13pe23,e11me22h));
          theta2.at(y,x) = static_cast<float>(atan2(e13me23h,2*(e11pe22-2*e12p2e33)));
        }
      }
    }

    return true;
  };

  // On copy apply for type channel!
  bool axOGDFeature::apply(const channel& center,
                           const channel& sround,
                           dvector& dest) const {

    if (center.empty() || sround.empty()) {
      dest.clear();
      setStatusString("source channel empty");
      return false;
    }

    if (center.size() != sround.size()) {
      dest.clear();
      setStatusString("center and surround channels have different sizes");
      return false;
    }

    const parameters& param = getParameters();

    gaussianPyramid<channel> centers(param.levels);
    gaussianPyramid<channel> surrounds(param.levels);

    centers.generate(center);
    surrounds.generate(sround);

    int n(0),i,voiceFactor,aFactor,cFactor,order;
    double eFactor(1.0);
    // order = param.ogdOrder;
    order = 1; // by now, only ogd first order supported!

    if (param.considerPhase) {
      cFactor = 1+2*order;
      n = 3*cFactor; // a0,a1,theta1,[a2,theta2],as0,as1,thetas1,...
    } else {
      cFactor = 1+order;
      n = 3*cFactor;   // a0,a1,[a2],as0,as1,[as2],ao0,ao1,[ao2]
    }

    if (param.voices) {
      eFactor = double(1<<(order-1));
      aFactor = (2*param.levels);
      n *= aFactor;
      cFactor *= aFactor;
      voiceFactor = 2; // step in the final vector
    } else {
      aFactor = (param.levels);
      n *= aFactor;
      cFactor *= aFactor;
      voiceFactor = 1; // no voices: each element
    }

    dest.resize(n,0.0,false,false);

    switch (order) {
        case 1: { // 1st order ogd
          double ac0,ac1,thetac1;
          double as0,as1,thetas1;
          double ao0,ao1,thetao1;
          for (i=0;i<param.levels;++i) {
            globalOgd1(centers.at(i),surrounds.at(i),false,
                       ac0,ac1,thetac1,
                       as0,as1,thetas1,
                       ao0,ao1,thetao1);
            dest.at(i*voiceFactor) = ac0;
            dest.at(i*voiceFactor+aFactor) = ac1;

            dest.at(i*voiceFactor+cFactor) = as0;
            dest.at(i*voiceFactor+aFactor+cFactor) = as1;

            dest.at(i*voiceFactor+2*cFactor) = ao0;
            dest.at(i*voiceFactor+aFactor+2*cFactor) = ao1;

            if (param.considerPhase) {
              dest.at(i*voiceFactor+2*aFactor) = thetac1;
              dest.at(i*voiceFactor+2*aFactor+cFactor) = thetas1;
              dest.at(i*voiceFactor+2*aFactor+2*cFactor) = thetao1;
            }

            if (param.voices) {
              globalOgd1(centers.at(i),surrounds.at(i),true,
                         ac0,ac1,thetac1,
                         as0,as1,thetas1,
                         ao0,ao1,thetao1);

              dest.at(i*voiceFactor+1) = ac0*eFactor;
              dest.at(i*voiceFactor+aFactor+1) = ac1*eFactor;

              dest.at(i*voiceFactor+1+cFactor) = as0*eFactor;
              dest.at(i*voiceFactor+aFactor+1+cFactor) = as1*eFactor;

              dest.at(i*voiceFactor+1+2*cFactor) = ao0*eFactor;
              dest.at(i*voiceFactor+aFactor+1+2*cFactor) = ao1*eFactor;

              if (param.considerPhase) {
                dest.at(i*voiceFactor+2*aFactor+1) = thetac1;
                dest.at(i*voiceFactor+2*aFactor+1+cFactor) = thetas1;
                dest.at(i*voiceFactor+2*aFactor+1+2*cFactor) = thetao1;
              }
            }
          }
        } break;
        default: {
          setStatusString("OGD order not supported!");
          return false;
        }
    }

    return true;
  };


  void axOGDFeature::generatePowerBasisOgd1(const channel& chnl,
                                            const bool voice,
                                            channel& p11,
                                            channel& p12,
                                            channel& p22) const {
    ogdFilter ogd;
    ogdFilter::parameters ogdPar;

    const parameters& param = getParameters();

    // set the ogd filter parameters

    ogdPar.boundaryType = lti::Mirror;
    ogdPar.order = 1;

    if (voice) {
      ogdPar.size = static_cast<int>(param.ogdSize * sqrt(2.0));
      ogdPar.variance = 2.0*param.ogdVariance; // sqrt(2)*stdDeviation
    } else {
      ogdPar.size = param.ogdSize;
      ogdPar.variance = param.ogdVariance;
    }

    if ((ogdPar.size%2) == 0) {
      ogdPar.size++;
    }

    ogd.setParameters(ogdPar);

    // calculate the basis channels

    ogd.generateBasisOgd1(chnl,p11,p22);

    p12.resize(p11.size(),0.0f,false,false);

    // generate the basis power channels
    channel::iterator it11,it22,it11e;
    channel::iterator it12;

    for (it11=p11.begin(),it11e=p11.end(),
           it12=p12.begin(),
           it22=p22.begin();
         it11 != it11e;
         ++it11,++it12,++it22) {

      (*it12) = (*it11)*(*it22);
      (*it11) = (*it11)*(*it11);
      (*it22) = (*it22)*(*it22);
    }
  }

  void axOGDFeature::generatePowerBasisOgd1(const channel& center,
                                            const channel& sround,
                                            const bool voice,
                                            channel& pc1,
                                            channel& pc2,
                                            channel& ps1,
                                            channel& ps2) const {
    ogdFilter ogd;
    ogdFilter::parameters ogdPar;

    downsampling downsampler;
    downsampling::parameters downPar;

    const parameters& param = getParameters();

    // set the ogd filter parameters
    downPar.boundaryType = lti::Constant;
    downsampler.setParameters(downPar);

    ogdPar.boundaryType = lti::Mirror;
    ogdPar.order = 1;

    if (voice) {
      ogdPar.size = static_cast<int>(param.ogdSize * sqrt(2.0));
      ogdPar.variance = 2.0*param.ogdVariance; // sqrt(2)*stdDeviation
    } else {
      ogdPar.size = param.ogdSize;
      ogdPar.variance = param.ogdVariance;
    }

    if ((ogdPar.size%2) == 0) {
      ogdPar.size++;
    }

    ogd.setParameters(ogdPar);

    // calculate the basis channels
    channel srnd2;

    // center
    ogd.generateBasisOgd1(center,pc1,pc2);

    downsampler.apply(pc1);
    downsampler.apply(pc2);

    // surround

    downsampler.apply(sround,srnd2); // use ps12 as temporary channel

    ogd.generateBasisOgd1(srnd2,ps1,ps2);
  }

  void axOGDFeature::generatePowerBasisOgd2(const channel& chnl,
                                            const bool voice,
                                            channel& p11,
                                            channel& p12,
                                            channel& p13,
                                            channel& p22,
                                            channel& p23,
                                            channel& p33) const {
    ogdFilter ogd;
    ogdFilter::parameters ogdPar;

    const parameters& param = getParameters();

    // set the ogd filter parameters

    ogdPar.boundaryType = lti::Mirror;
    ogdPar.order = 2;

    if (voice) {
      ogdPar.size = static_cast<int>(param.ogdSize * sqrt(2.0));
      ogdPar.variance = 2.0*param.ogdVariance; // sqrt(2)*stdDeviation
    } else {
      ogdPar.size = param.ogdSize;
      ogdPar.variance = param.ogdVariance;
    }

    if ((ogdPar.size%2) == 0) {
      ogdPar.size++;
    }

    ogd.setParameters(ogdPar);

    // calculate the basis channels

    ogd.generateBasisOgd2(chnl,p11,p22,p33);

    p12.resize(p11.size(),0.0f,false,false);
    p13.resize(p11.size(),0.0f,false,false);
    p23.resize(p11.size(),0.0f,false,false);

    // generate the basis power channels
    channel::iterator it11,it22,it33,it11e;
    channel::iterator it12,it13,it23;

    for (it11=p11.begin(),it11e=p11.end(),
           it12=p12.begin(),it13=p13.begin(),
           it22=p22.begin(),it23=p23.begin(),
           it33=p33.begin();
         it11 != it11e;
         ++it11,++it12,++it13,++it22,++it23,++it33) {

      (*it12) = (*it11)*(*it22);
      (*it13) = (*it11)*(*it33);
      (*it23) = (*it22)*(*it33);

      (*it11) = (*it11)*(*it11);
      (*it22) = (*it22)*(*it22);
      (*it33) = (*it33)*(*it33);
    }
  }

  void axOGDFeature::globalOgd1(const channel& p11,
                                const channel& p12,
                                const channel& p22,
                                double& a0,
                                double& a1,
                                double& theta1) const {

    double e11(0),e12(0),e22(0);

    // generate the global energy
    channel::const_iterator it11,it22,it11e;
    channel::const_iterator it12;

    for (it11=p11.begin(),it11e=p11.end(),
           it12=p12.begin(),
           it22=p22.begin();
         it11 != it11e;
         ++it11,++it12,++it22) {

      e11+=(*it11);
      e12+=(*it12);
      e22+=(*it22);
    }

    a0 = (e11+e22)/2.0;
    a1 = 0.5*sqrt(e11*e11+e22*e22+4*e12*e12-2*e11*e22);
    theta1 = atan2((2*e12),(e11-e22));
  }


  /*
   * calculate the local descriptors using the first order ogd basis
   * power channels
   */
  void axOGDFeature::localOgd1(const point& pos,
                               const imatrix& ioPts,
                               const channel& p11,
                               const channel& p12,
                               const channel& p22,
                               double& a0,
                               double& a1,
                               double& theta1) const {

    double e11(0),e12(0),e22(0);

    int x,y,lastx,lasty;

    int lr = (ioPts.rows()-1)/2;
    for (y=max(0,pos.y-lr),lasty=min(pos.y+lr+1,p11.rows());
         y<lasty;
         ++y) {
      for (x=max(0,ioPts.at(y-pos.y+lr,0)+pos.x),
             lastx=min(ioPts.at(y-pos.y+lr,1)+pos.x+1,p11.columns());
           x<lastx;
           ++x) {
        e11+=p11.at(y,x);
        e12+=p12.at(y,x);
        e22+=p22.at(y,x);
      }
    }

    a0 = (e11+e22)/2.0;
    a1 = 0.5*sqrt(e11*e11+e22*e22+4*e12*e12-2*e11*e22);
    theta1 = atan2((2*e12),(e11-e22));
  }

  /**
   * calculate the global descriptors using the first order ogd basis
   * power channels for the opponent color feature
   */
  void axOGDFeature::globalOgd1(const channel& center,
                                const channel& sround,
                                const bool voice,
                                double& ac0,
                                double& ac1,
                                double& thetac1,
                                double& as0,
                                double& as1,
                                double& thetas1,
                                double& ao0,
                                double& ao1,
                                double& thetao1) const {

    channel pc1,pc2,ps1,ps2;
    // generate the basis power channels
    generatePowerBasisOgd1(center,sround,voice,pc1,pc2,ps1,ps2);

    int y,x;
    const int sizey = pc1.rows();
    const int sizex = pc1.columns();
    float c1,c2,s1,s2;
    double ec11(0),ec12(0),ec22(0);
    double es11(0),es12(0),es22(0);
    double eo11(0),eo12(0),eo22(0);

    for (y=0;y<sizey;++y) {
      for (x=0;x<sizex;++x) {

        c1 = pc1.at(y,x);
        c2 = pc2.at(y,x);
        s1 = ps1.at(y,x);
        s2 = ps2.at(y,x);

        ec11 += c1*c1;
        ec12 += c1*c2;
        ec22 += c2*c2;

        es11 += s1*s1;
        es12 += s1*s2;
        es22 += s2*s2;

        eo11 += c1*s1;
        eo12 += c1*s2 + c2*s1;
        eo22 += c2*s2;
      }
    }

    ac0 = (ec11+ec22)/2.0;
    ac1 = 0.5*sqrt(ec11*ec11+ec22*ec22+4*ec12*ec12-2*ec11*ec22);
    thetac1 = atan2((2*ec12),(ec11-ec22));

    as0 = (es11+es22)/2.0;
    as1 = 0.5*sqrt(es11*es11+es22*es22+4*es12*es12-2*es11*es22);
    thetas1 = atan2((2*es12),(es11-es22));

    ao0 = (eo11+eo22);
    ao1 = 0.5*sqrt(eo11*eo11+eo22*eo22-2*eo11*eo22+eo12*eo12);
    thetao1 = atan2(eo12,eo11-eo22);
  }

  /**
   * calculate the global descriptors using the first order ogd basis
   * power channels for the opponent color feature
   */
  void axOGDFeature::localOgd1(const point& pos,
                               const imatrix& ioPts,
                               const channel& pc1,
                               const channel& pc2,
                               const channel& ps1,
                               const channel& ps2,
                               double& ac0,
                               double& ac1,
                               double& thetac1,
                               double& as0,
                               double& as1,
                               double& thetas1,
                               double& ao0,
                               double& ao1,
                               double& thetao1) const {

    int y,x,lastx,lasty;
    float c1,c2,s1,s2;
    double ec11(0),ec12(0),ec22(0);
    double es11(0),es12(0),es22(0);
    double eo11(0),eo12(0),eo22(0);

    int lr = (ioPts.rows()-1)/2;
    for (y=max(0,pos.y-lr),lasty=min(pos.y+lr+1,pc1.rows());
         y<lasty;
         ++y) {
      for (x=max(0,ioPts.at(y-pos.y+lr,0)+pos.x),
             lastx=min(ioPts.at(y-pos.y+lr,1)+pos.x+1,pc1.columns());
           x<lastx;
           ++x) {

        c1 = pc1.at(y,x);
        c2 = pc2.at(y,x);
        s1 = ps1.at(y,x);
        s2 = ps2.at(y,x);

        ec11 += c1*c1;
        ec12 += c1*c2;
        ec22 += c2*c2;

        es11 += s1*s1;
        es12 += s1*s2;
        es22 += s2*s2;

        eo11 += c1*s1;
        eo12 += c1*s2 + c2*s1;
        eo22 += c2*s2;
      }
    }

    ac0 = (ec11+ec22)/2.0;
    ac1 = 0.5*sqrt(ec11*ec11+ec22*ec22+4*ec12*ec12-2*ec11*ec22);
    thetac1 = atan2((2*ec12),(ec11-ec22));

    as0 = (es11+es22)/2.0;
    as1 = 0.5*sqrt(es11*es11+es22*es22+4*es12*es12-2*es11*es22);
    thetas1 = atan2((2*es12),(es11-es22));

    ao0 = (eo11+eo22);
    ao1 = 0.5*sqrt(eo11*eo11+eo22*eo22-2*eo11*eo22+eo12*eo12);
    thetao1 = atan2(eo12,eo11-eo22);
  }



  void axOGDFeature::globalOgd2(const channel& p11,
                                const channel& p12,
                                const channel& p13,
                                const channel& p22,
                                const channel& p23,
                                const channel& p33,
                                double& a0,
                                double& a1,
                                double& a2,
                                double& theta1,
                                double& theta2) const {

    double e11(0),e12(0),e13(0),e22(0),e23(0),e33(0);

    // generate the basis power channels
    channel::const_iterator it11,it22,it33,it11e;
    channel::const_iterator it12,it13,it23;

    // calculate the energies for each power channel
    for (it11=p11.begin(),it11e=p11.end(),
           it12=p12.begin(),it13=p13.begin(),
           it22=p22.begin(),it23=p23.begin(),
           it33=p33.begin();
         it11 != it11e;
         ++it11,++it12,++it13,++it22,++it23,++it33) {

      e11 += (*it11);
      e12 += (*it12);
      e13 += (*it13);

      e22 += (*it22);
      e23 += (*it23);
      e33 += (*it33);
    }

    // some temporary variables
    double e11pe22  = e11+e22;
    double e12p2e33 = e12+2.0*e33;
    double e11me22h = (e11-e22)/2.0;
    double e13pe23  = e13 + e23;
    double e13me23h = (e13 - e23)/2.0;

    a0 = (3.0*(e11pe22)+2.0*e12p2e33)/8.0;
    a1 = sqrt(e11me22h*e11me22h + e13pe23*e13pe23);
    double tmp = (e11pe22/8.0)-(e12p2e33/4.0);
    a2 = sqrt(tmp*tmp+e13me23h*e13me23h);
    theta1 = atan2(e13pe23,e11me22h);
    theta2 = atan2(8*e13me23h,(e11pe22-2*e12p2e33));
  }

  //
  void axOGDFeature::localOgd2(const point& pos,
                               const imatrix& ioPts,
                               const channel& p11,
                               const channel& p12,
                               const channel& p13,
                               const channel& p22,
                               const channel& p23,
                               const channel& p33,
                               double& a0,
                               double& a1,
                               double& a2,
                               double& theta1,
                               double& theta2) const {

    double e11(0),e12(0),e13(0),e22(0),e23(0),e33(0);

    int x,y,lastx,lasty;

    int lr = (ioPts.rows()-1)/2;
    for (y=max(0,pos.y-lr),lasty=min(pos.y+lr+1,p11.rows());
         y<lasty;
         ++y) {
      for (x=max(0,ioPts.at(y-pos.y+lr,0)+pos.x),
             lastx=min(ioPts.at(y-pos.y+lr,1)+pos.x+1,p11.columns());
           x<lastx;
           ++x) {

        e11 += p11.at(y,x);
        e12 += p12.at(y,x);
        e13 += p13.at(y,x);

        e22 += p22.at(y,x);
        e23 += p23.at(y,x);
        e33 += p33.at(y,x);
      }
    }

    // some temporary variables
    double e11pe22  = e11+e22;
    double e12p2e33 = e12+2.0*e33;
    double e11me22h = (e11-e22)/2.0;
    double e13pe23  = e13 + e23;
    double e13me23h = (e13 - e23)/2.0;

    a0 = (3.0*(e11pe22)+2.0*e12p2e33)/8.0;
    a1 = sqrt(e11me22h*e11me22h + e13pe23*e13pe23);
    double tmp = (e11pe22/8.0)-(e12p2e33/4.0);
    a2 = sqrt(tmp*tmp+e13me23h*e13me23h);
    theta1 = atan2(e13pe23,e11me22h);
    theta2 = atan2(8*e13me23h,(e11pe22-2*e12p2e33));
  }


  /*
   * Indicates which channel to use in the local feature extraction.
   * Some precalculation will be done, and using the
   * apply(const point&, dvector&) member, the feature vector of a specific
   * position can be computed in a faster way.
   *
   * After calling this method the corresponding monochromatic features will
   * be extracted.
   */
  bool axOGDFeature::use(const channel& src) {

    const parameters& param = getParameters();

    numLevels = param.levels;
    ogdOrder = param.ogdOrder;

    if (src.empty()) {
      setStatusString("input data empty");
      theChannels.clear();
      return false;
    }

    int n(0),i,voiceFactor,aFactor;
    double eFactor(1.0);

    gaussianPyramid<channel> thePyramid(param.levels);
    thePyramid.generate(src);

    if (param.considerPhase) {
      n = (1+2*param.ogdOrder); // a0,a1,theta1,[a2,theta2]
    } else {
      n = (param.ogdOrder+1);   // a0,a1,[a2]
    }

    if (param.voices) {
      eFactor = double(1<<(param.ogdOrder-1));
      aFactor = (2*param.levels);
      n = n*aFactor;
      voiceFactor = 2; // step in the final vector
    } else {
      aFactor = (param.levels);
      n = n*aFactor;
      voiceFactor = 1; // no voices: each element
    }

    theChannels.resize(n);

    switch (param.ogdOrder) {
        case 1: { // 1st order ogd
          for (i=0;i<param.levels;++i) {
            if (param.considerPhase) {
              apply(thePyramid.at(i),false,i,
                    theChannels[i*voiceFactor],            // a0
                    theChannels[i*voiceFactor+aFactor],    // a1
                    theChannels[i*voiceFactor+2*aFactor]); // theta1
            } else {
              channel theta1;
              apply(thePyramid.at(i),false,i,
                    theChannels[i*voiceFactor],         // a0
                    theChannels[i*voiceFactor+aFactor], // a1
                    theta1);                     // theta1
            }

            if (param.voices) {
              if (param.considerPhase) {
                apply(thePyramid.at(i),true,i,
                      theChannels[i*voiceFactor+1],            // a0
                      theChannels[i*voiceFactor+aFactor+1],    // a1
                      theChannels[i*voiceFactor+2*aFactor+1]); // theta1
              } else {
                channel theta1;
                apply(thePyramid.at(i),true,i,
                      theChannels[i*voiceFactor+1],         // a0
                      theChannels[i*voiceFactor+aFactor+1], // a1
                      theta1);                       // theta1
              }

              theChannels[i*voiceFactor+1].multiply(static_cast<float>(eFactor));
              theChannels[i*voiceFactor+aFactor+1].multiply(static_cast<float>(eFactor));
            }
          }
        } break;
        case 2: { // 2nd order ogd

          for (i=0;i<param.levels;++i) {
            if (param.considerPhase) {
              apply(thePyramid.at(i),false,i,
                    theChannels[i*voiceFactor],            // a0
                    theChannels[i*voiceFactor+aFactor],    // a1
                    theChannels[i*voiceFactor+2*aFactor],  // a2
                    theChannels[i*voiceFactor+3*aFactor],  // theta1
                    theChannels[i*voiceFactor+4*aFactor]); // theta2
            } else {
              channel theta1,theta2;
              apply(thePyramid.at(i),false,i,
                    theChannels[i*voiceFactor],            // a0
                    theChannels[i*voiceFactor+aFactor],    // a1
                    theChannels[i*voiceFactor+2*aFactor],  // a2
                    theta1,theta2);                 // theta1
            }


            if (param.voices) {
              if (param.considerPhase) {
                apply(thePyramid.at(i),true,i,
                      theChannels[i*voiceFactor+1],            // a0
                      theChannels[i*voiceFactor+aFactor+1],    // a1
                      theChannels[i*voiceFactor+2*aFactor+1],  // a2
                      theChannels[i*voiceFactor+3*aFactor+1],  // theta1
                      theChannels[i*voiceFactor+4*aFactor+1]); // theta2
              } else {
                channel theta1,theta2;
                apply(thePyramid.at(i),true,i,
                      theChannels[i*voiceFactor+1],            // a0
                      theChannels[i*voiceFactor+aFactor+1],    // a1
                      theChannels[i*voiceFactor+2*aFactor+1],  // a2
                      theta1,theta2);                 // theta1
              }

              theChannels[i*voiceFactor+1].multiply(static_cast<float>(eFactor));
              theChannels[i*voiceFactor+aFactor+1].multiply(static_cast<float>(eFactor));
              theChannels[i*voiceFactor+2*aFactor+1].multiply(static_cast<float>(eFactor));
            }
          }
        } break;
        default: {
          setStatusString("OGD order not supported!");
          return false;
        }
    }

    return true;
  }

  /*
   * Indicates which channels to use in the local feature extraction.
   * Some precalculation will be done, and using the
   * apply(const point&, dvector&) member, the feature vector of a specific
   * position can be computed in a faster way.
   *
   * After calling this method the corresponding opponent-color features will
   * be extracted.
   */
  bool axOGDFeature::use(const channel& center,const channel& surround) {
    return true;
  }

  /*
   * extract the local feature at the position pos and leave the result
   * in the given vector.  It depends on the last used "use" member, if
   * the feature vector is an "opponent-color" feature or a
   * monochromatic feature.
   *
   * @param pos the position at the given image.
   * @param dest the feature vector.
   * @return true if sucessful, false otherwise
   */
  bool axOGDFeature::apply(const point& pos,dvector& dest) const {
    const parameters& param = getParameters();

    bilinearInterpolator<float> bilinear;


    if (theChannels.size() == 0) {
      setStatusString("No default image found.  Try using 'use()'");
      dest.clear();
      return false;
    }

    if ((numLevels != param.levels) ||
        (ogdOrder != param.ogdOrder)) {
      setStatusString("Parameters changed.  Please call 'use()' again!");
      return false;
    }

    point currentSize = theChannels[0].size();
    double factor = 1.0;
    int i;

    dest.resize(theChannels.size(),0.0,false,false);

    for (i=0;i<dest.size();++i) {
      if (currentSize != theChannels[i].size()) {
        currentSize = theChannels[i].size();
        factor*=2.0;
      }

      dest.at(i) = bilinear.apply(theChannels[i],
                                  static_cast<float>(pos.y/factor),
                                  static_cast<float>(pos.x/factor));
    }

    return true;
  }

  // location feature extraction
  /*
   * extracts the OGD features for each given location.  The radius
   * of each location is used to determine which scale should be used
   * in the generation of each feature vector.
   * @param src original channel to be analyzed.
   * @param locs the locations that need to be analyzed.
   * @param dest the list of feature vectors.  This list will have the same
   *             size as locs.  The feature vector at position p will
   *             correspond to the location at position p.
   * @return true if successful, of false otherwise.
   */
  bool axOGDFeature::apply(const channel& src,
                           const std::list<location>& locs,
                           std::list<dvector>& dest) {



    if (src.empty()) {
      dest.clear();
      setStatusString("source channel empty");
      return false;
    }

    const parameters& param = getParameters();

    std::map<float,int> radToIdx;
    std::map<float,int> radToScale;
    std::vector<imatrix> ioPts;

    // sort the locations
    std::list<location> slocs(locs);
    slocs.sort();

    const int levels = analyzeLocations(slocs,radToIdx,radToScale,ioPts);

    const unsigned int numLocs = slocs.size();

    // check size of output data and fix it if not ok
    if (dest.size() < numLocs) {
      // how many new data required?
      const int newelem = numLocs - dest.size();
      dvector tmp;
      for (int i=0;i<newelem;++i) {
        dest.push_back(tmp);
      }
    } else if (dest.size() > numLocs) {
      // how many new data required?
      const int newelem = dest.size() - numLocs;

      for (int i=0;i<newelem;++i) {
        dest.pop_back();
      }
    }

    // generate the pyramid with the required number of levels
    gaussianPyramid<channel> thePyramid(levels);
    thePyramid.generate(src);

    int n(0),i;

    if (param.considerPhase) {
      n = (1+2*param.ogdOrder); // a0,a1,theta1,[a2,theta2]
    } else {
      n = (param.ogdOrder+1);   // a0,a1,[a2]
    }

    switch (param.ogdOrder) {
      case 1: { // 1st order ogd
        channel p11,p12,p22;
        double a0,a1,theta1;
        float lastRadius = -1;
        float sfactor(1); // scale factor
        int actualIo(0);
        point pos;
        std::list<location>::const_iterator lit;
        std::list<dvector>::iterator vit;

        for (lit=slocs.begin(),vit=dest.begin();
             lit!=slocs.end();
             ++lit,++vit) {
          if (lastRadius != (*lit).radius) {
            lastRadius = (*lit).radius;
            i = radToScale[lastRadius];
            generatePowerBasisOgd1(thePyramid.at(i),
                                   radToIdx[lastRadius] == 1,
                                   p11,p12,p22);
            sfactor = static_cast<float>(1<<i); // 2^i
            actualIo = radToIdx[lastRadius];

          }
          pos = point(static_cast<int>(0.5+(*lit).position.x/sfactor),
                      static_cast<int>(0.5+(*lit).position.y/sfactor));
          localOgd1(pos,ioPts[actualIo],p11,p12,p22,a0,a1,theta1);

          (*vit).resize(n);
          (*vit).at(0) = a0;
          (*vit).at(1) = a1;
          if (param.considerPhase) {
            theta1 = theta1 - (2.0 * (*lit).angle);
            while (theta1<0) theta1+=Pi;
            while (theta1>Pi) theta1-=Pi;
            (*vit).at(2) = theta1;
          }
        }
      } break;
      case 2: { // 2nd order ogd
        channel p11,p12,p13,p22,p23,p33;
        double a0,a1,a2,theta1,theta2;
        float lastRadius = -1;
        float sfactor(1); // scale factor
        int actualIo(0);
        point pos;
        std::list<location>::const_iterator lit;
        std::list<dvector>::iterator vit;

        for (lit=slocs.begin(),vit=dest.begin();
             lit!=slocs.end();
             ++lit,++vit) {
          if (lastRadius != (*lit).radius) {
            lastRadius = (*lit).radius;
            i = radToScale[lastRadius];
            generatePowerBasisOgd2(thePyramid.at(i),
                                   radToIdx[lastRadius] == 1,
                                   p11,p12,p13,p22,p23,p33);
            sfactor = static_cast<float>(1<<i); // 2^i
            actualIo = radToIdx[lastRadius];

          }
          pos = point(static_cast<int>(0.5+(*lit).position.x/sfactor),
                      static_cast<int>(0.5+(*lit).position.y/sfactor));

          localOgd2(pos,ioPts[actualIo],p11,p12,p13,p22,p23,p33,
                    a0,a1,a2,theta1,theta2);

          (*vit).resize(n);
          (*vit).at(0) = a0;
          (*vit).at(1) = a1;
          (*vit).at(2) = a2;
          if (param.considerPhase) {
            theta1 = theta1 - (2.0 * (*lit).angle);
            while (theta1<0) theta1+=Pi;
            while (theta1>Pi) theta1-=Pi;
            (*vit).at(3) = theta1;

            theta2 = theta2 - (4.0 * (*lit).angle) ; //TODO: is /4 ok?
            while (theta2<0) theta2+=Pi;
            while (theta2>Pi) theta2-=Pi;
            (*vit).at(4) = theta2;
          }
        }
      } break;

      default: {
        setStatusString("OGD order not supported!");
        return false;
      }
    }

    return true;
  }

  /*
   * extracts the OGD features for each given location.  The image will
   * be splitted in its red, green and blue components, and the feature
   * vectors will be concatenated.
   * The radius of each location is used to determine which scale
   * should be used in the generation of each feature vector.
   * @param src original channel to be analyzed.
   * @param locs the locations that need to be analyzed.
   * @param dest the list of feature vectors.  This list will have the same
   *             size as locs.  The feature vector at position p will
   *             correspond to the location at position p.
   * @return true if successful, of false otherwise.
   */
  bool axOGDFeature::apply(const image& src,
                           const std::list<location>& locs,
                           std::list<dvector>& dest) {

    splitImageToRGB splitter;
    channel r,g,b;
    splitter.apply(src,r,g,b);

    std::list<dvector> dr,dg,db;
    std::list<dvector>::const_iterator rit,git,bit;
    dvector tmp;

    dest.clear();

    if (apply(r,locs,dr) &&
        apply(g,locs,dg) &&
        apply(b,locs,db)) {
      const int size = (*(dr.begin())).size();

      for (rit=dr.begin(),git=dg.begin(),bit=db.begin();
           rit!=dr.end();
           ++rit,++git,++bit) {
        dest.push_back(tmp);
        dest.back().resize(3*size);
        dest.back().fill(*rit,0);
        dest.back().fill(*git,size);
        dest.back().fill(*bit,2*size);
      }

    }

    return true;
  }

  /*
   * extracts the opponent color OGD features for each given
   * location.  The radius of each location is used to determine
   * which scale should be used in the generation of each feature
   * vector.
   *
   * @param center original channel considered as "center"
   * @param surround channel considered as surround.  This must have
   *                 the same size as center.
   * @param locs the locations that need to be analyzed.
   * @param dest the list of feature vectors.  This list will have the same
   *             size as locs.  The feature vector at position p will
   *             correspond to the location at position p.
   * @return true if successful, of false otherwise.
   */
  bool axOGDFeature::apply(const channel& center,
                           const channel& surround,
                           const std::list<location>& locs,
                           std::list<dvector>& dest) {

    if (center.size() != surround.size()) {
      dest.clear();
      setStatusString("source channels must have the same size");
      return false;
    } else if (center.empty()) {
      dest.clear();
      setStatusString("source channels are empty");
      return false;
    }

    const parameters& param = getParameters();

    std::map<float,int> radToIdx;
    std::map<float,int> radToScale;
    std::vector<imatrix> ioPts;

    // sort the locations
    std::list<location> slocs(locs);
    slocs.sort();

    const int levels = analyzeLocations(slocs,radToIdx,radToScale,ioPts);

    const unsigned int numLocs = slocs.size();

    // check size of output data and fix it if not ok
    if (dest.size() < numLocs) {
      // how many new data required?
      const int newelem = numLocs - dest.size();
      dvector tmp;
      for (int i=0;i<newelem;++i) {
        dest.push_back(tmp);
      }
    } else if (dest.size() > numLocs) {
      // how many new data required?
      const int newelem = dest.size() - numLocs;

      for (int i=0;i<newelem;++i) {
        dest.pop_back();
      }
    }

    // generate the pyramid with the required number of levels
    gaussianPyramid<channel> centers(levels);
    gaussianPyramid<channel> surrounds(levels);

    centers.generate(center);
    surrounds.generate(surround);

    int n(0),i;

    if (param.considerPhase) {
      n = 3*(1+2*param.ogdOrder); // a0,a1,theta1,[a2,theta2]
    } else {
      n = 3*(param.ogdOrder+1);   // a0,a1,[a2]
    }

    switch (param.ogdOrder) {
      case 1: { // 1st order ogd
        channel pc1,pc2,ps1,ps2;
        double ac0,ac1,thetac1,as0,as1,thetas1,ao0,ao1,thetao1;
        float lastRadius = -1;
        float sfactor(1); // scale factor
        int actualIo(0);
        point pos;
        std::list<location>::const_iterator lit;
        std::list<dvector>::iterator vit;

        for (lit=slocs.begin(),vit=dest.begin();
             lit!=slocs.end();
             ++lit,++vit) {
          if (lastRadius != (*lit).radius) {
            lastRadius = (*lit).radius;
            i = radToScale[lastRadius];
            generatePowerBasisOgd1(centers.at(i),
                                   surrounds.at(i),
                                   radToIdx[lastRadius] == 1,
                                   pc1,pc2,ps1,ps2);
            sfactor = static_cast<float>(1<<i); // 2^i
            actualIo = radToIdx[lastRadius];

          }
          pos = point(static_cast<int>(0.5+(*lit).position.x/sfactor),
                      static_cast<int>(0.5+(*lit).position.y/sfactor));

          localOgd1(pos,ioPts[actualIo],
                    pc1,pc2,ps1,ps2,
                    ac0,ac1,thetac1,
                    as0,as1,thetas1,
                    ao0,ao1,thetao1);

          (*vit).resize(n);
          (*vit).at(0) = ac0;
          (*vit).at(1) = ac1;
          (*vit).at(2) = as0;
          (*vit).at(3) = as1;
          (*vit).at(4) = ao0;
          (*vit).at(5) = ao1;
          if (param.considerPhase) {
            thetac1 = thetac1-(2.0* (*lit).angle);
            thetas1 = thetas1-(2.0* (*lit).angle);
            thetao1 = thetao1-(2.0* (*lit).angle);

            while (thetac1<0) thetac1 += Pi;
            while (thetas1<0) thetas1 += Pi;
            while (thetao1<0) thetao1 += Pi;
            while (thetac1>Pi) thetac1 -= Pi;
            while (thetas1>Pi) thetas1 -= Pi;
            while (thetao1>Pi) thetao1 -= Pi;

            (*vit).at(6) = thetac1;
            (*vit).at(7) = thetas1;
            (*vit).at(8) = thetao1;
          }
        }
      } break;
      default: {
        setStatusString("OGD order not supported!");
        return false;
      }
    }

    return true;
  }

  /*
   * check the location list for used radii, and initialize
   * the region io-points required.
   *
   */
  int axOGDFeature::analyzeLocations(const std::list<location>& locs,
                                      std::map<float,int>& radToIdx,
                                      std::map<float,int>& radToScale,
                                      std::vector<imatrix>& ioPts ) {

    std::set<float> radii;
    std::set<float>::const_iterator rit;
    std::list<location>::const_iterator lit;
    int i = 0;
    int levels = 0;

    for (lit=locs.begin();lit!=locs.end();++lit) {
      radii.insert((*lit).radius);
    }

    float factor;
    float lastRadius;

    // compute the factor between the radii:
    // usually will be 2 or sqrt(2)
    rit=radii.begin();
    lastRadius = (*rit);
    ++rit;
    if (rit!=radii.end()) {
      factor = (*rit)/lastRadius;

      for (;rit!=radii.end();++rit) {
        factor = min(factor,(*rit)/lastRadius);
        lastRadius = *rit;
      }
    } else {
      static const float sqrt2= sqrt(2.0f);
      factor = sqrt2;
    }

    // each i-th levels (voices included)
    // that are required from the ogd-pyramid.
    // i is computed this way:
    i = static_cast<int>(round(2.0*log(factor)/log(2.0)));

    if ((i%2) == 0) {
      // an even "i" means only one circle is required, because
      // the voices are NOT considered
      ioPts.resize(1);
      getRadialIoPoints(*radii.begin(),ioPts[0]);
    } else {
      ioPts.resize(2);
      getRadialIoPoints(*radii.begin(),ioPts[0]);
      getRadialIoPoints(sqrt(2.0)*(*radii.begin()),ioPts[1]);
    }

    radToIdx.clear();
    radToScale.clear();

    for (rit=radii.begin(),lastRadius=*rit;rit!=radii.end();++rit) {
      i = static_cast<int>(round(2.0*log((*rit)/lastRadius)/log(2.0)));

      radToIdx[*rit] = (i & 0x01);
      radToScale[*rit] = (i/2);
      levels = max(levels,1+(i/2));
    }

    return levels;
  };

  /*
   * get the io points for the radial location of the given radius.
   * This is used by the apply methods which analyse only given
   * locations, to take only the necessary data into consideration.
   */
  void axOGDFeature::getRadialIoPoints(const double& radius,
                                             imatrix& ioPts) {

    const int lr = static_cast<int>(radius-1.0+0.5);
    const int size = 1+2*lr;
    ioPts.resize(size,2,-1);

    // the position at the middle has the complete radius
    ioPts.at(lr,1) = lr;
    int x,y;
    const int sqLr = static_cast<int>(0.5+double(lr)/sqrt(2.0));

    // the region between 90° and 45°
    for (x=0;x<=sqLr;++x) {
      y = lr - static_cast<int>(sqrt(lr*lr - x*x) + 0.5);
      ioPts.at(y,1)=max(ioPts.at(y,1),x);
    }

    // now between 45° and 0°
    for (y=sqLr;y>0;--y) {
      x = static_cast<int>(sqrt(lr*lr - y*y) + 0.5);
      ioPts.at(lr-y,1)=max(ioPts.at(lr-y,1),x);
    }

    // mirror the first quadrant into the fourth quadrant
    for (y=size-1;y>lr;--y) {
      ioPts.at(y,1) = ioPts.at(size-y-1,1);
    }

    // the second and third quadrant
    for (y=0;y<size;++y) {
      ioPts.at(y,0) = -ioPts.at(y,1);
    }
  }
}
