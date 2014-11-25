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
 * file .......: ltiAxLocalRegions.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 7.1.2002
 * revisions ..: $Id: ltiAxLocalRegions.cpp,v 1.11 2006/09/05 10:02:23 ltilib Exp $
 */

#ifdef _LTI_MSC_6
#pragma warning ( disable : 4786 )
#endif

#include "ltiAxLocalRegions.h"
#include "ltiMaximumFilter.h"
#include "ltiConvolution.h"
#include "ltiGaussKernels.h"
#include "ltiGradientKernels.h"

//#define _LTI_DEBUG 2


#ifdef _LTI_DEBUG
#include <iostream>
#include <ltiViewer.h>
#endif


namespace lti {
  // --------------------------------------------------
  // axLocalRegions::parameters
  // --------------------------------------------------

  // default constructor
  axLocalRegions::parameters::parameters()
    : transform::parameters() {

    orientationWindowFactor = double(3);
    binsInOrientationHistogram = int(32);
    firstLevel = int(2);
    lastLevel = int(6);
    saliencyThreshold.resize(1,0.1f,false,true);
    firstMaximumFilterSize = int(5);
    maximumFilterAdaption = float(1);
  }

  // copy constructor
  axLocalRegions::parameters::parameters(const parameters& other)
    : transform::parameters()  {
    copy(other);
  }

  // destructor
  axLocalRegions::parameters::~parameters() {
  }

  // get type name
  const char* axLocalRegions::parameters::getTypeName() const {
    return "axLocalRegions::parameters";
  }

  // copy member

  axLocalRegions::parameters&
    axLocalRegions::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    transform::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    transform::parameters& (transform::parameters::* p_copy)
      (const transform::parameters&) =
      transform::parameters::copy;
    (this->*p_copy)(other);
# endif

    orientationWindowFactor = other.orientationWindowFactor;
    binsInOrientationHistogram = other.binsInOrientationHistogram;
    firstLevel = other.firstLevel;
    lastLevel = other.lastLevel;
    saliencyThreshold.copy(other.saliencyThreshold);
    firstMaximumFilterSize = other.firstMaximumFilterSize;
    maximumFilterAdaption = other.maximumFilterAdaption;

    return *this;
  }

  // alias for copy member
  axLocalRegions::parameters&
    axLocalRegions::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* axLocalRegions::parameters::clone() const {
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
  bool axLocalRegions::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool axLocalRegions::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"orientationWindowFactor",orientationWindowFactor);
      lti::write(handler,"binsInOrientationHistogram",
                 binsInOrientationHistogram);
      lti::write(handler,"firstLevel",firstLevel);
      lti::write(handler,"lastLevel",lastLevel);
      lti::write(handler,"saliencyThreshold",saliencyThreshold);
      lti::write(handler,"firstMaximumFilterSize",firstMaximumFilterSize);
      lti::write(handler,"maximumFilterAdaption",maximumFilterAdaption);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::write(handler,false);
# else
    bool (transform::parameters::* p_writeMS)(ioHandler&,const bool) const =
      transform::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool axLocalRegions::parameters::write(ioHandler& handler,
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
  bool axLocalRegions::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool axLocalRegions::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"orientationWindowFactor",orientationWindowFactor);
      lti::read(handler,"binsInOrientationHistogram",
                binsInOrientationHistogram);
      lti::read(handler,"firstLevel",firstLevel);
      lti::read(handler,"lastLevel",lastLevel);
      lti::read(handler,"saliencyThreshold",saliencyThreshold);
      lti::read(handler,"firstMaximumFilterSize",firstMaximumFilterSize);
      lti::read(handler,"maximumFilterAdaption",maximumFilterAdaption);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::read(handler,false);
# else
    bool (transform::parameters::* p_readMS)(ioHandler&,const bool) =
      transform::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool axLocalRegions::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // axLocalRegions
  // --------------------------------------------------

  // default constructor
  axLocalRegions::axLocalRegions()
    : transform(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  axLocalRegions::axLocalRegions(const axLocalRegions& other)
    : transform()  {
    copy(other);
  }

  // destructor
  axLocalRegions::~axLocalRegions() {
  }

  // returns the name of this type
  const char* axLocalRegions::getTypeName() const {
    return "axLocalRegions";
  }

  // copy member
  axLocalRegions& axLocalRegions::copy(const axLocalRegions& other) {
    transform::copy(other);

    return (*this);
  }

  // alias for copy member
  axLocalRegions& axLocalRegions::operator=(const axLocalRegions& other) {
    return (copy(other));
  }


  // clone member
  functor* axLocalRegions::clone() const {
    return new axLocalRegions(*this);
  }

  // return parameters
  const axLocalRegions::parameters&
    axLocalRegions::getParameters() const {
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

  inline bool axLocalRegions::even(const int i) const {
    return ((i&0x01) == 0);
  }

  inline double axLocalRegions::gaussian(const float dy,
                                         const float dx,
                                         const double& s2) const {
    // prefix factor
    return exp(-((dx*dx + dy*dy)/(2.0*s2)));
  }

  // On copy apply for type channel!
  bool axLocalRegions::apply(const channel& src,
                             std::list<location>& locs) const {


    const parameters& param = getParameters();
    const int firstLevel = max(1,param.firstLevel);
    const int lastLevel = max(firstLevel,param.lastLevel);
    const int levels = max(3,lastLevel+1);

    // gaussian kernels
    static const int gaussKernelSize = 5;

    gaussKernel2D<float> gk2(gaussKernelSize,2);
    gaussKernel2D<float> gk4(gaussKernelSize+2,4); // greater variance, more
                                                   // size.
    pyramid<channel> pyr(levels);
    int asize,csize;
    int i;

    float mxKernelSize = static_cast<float>(param.firstMaximumFilterSize);
    int iMxKernelSize = max(3,iround(mxKernelSize));
    if (even(iMxKernelSize)) {
      iMxKernelSize++;
    }

    channel b,c;

    convolution convolver;
    maximumFilter<float> mxConvolver(iMxKernelSize);

    convolution::parameters convParam;
    convParam.boundaryType = lti::Constant;
    convParam.setKernel(gk2);
    convolver.setParameters(convParam);

    if (even(levels)) {
    // even number of levels
      asize = csize = levels/2;
    } else {
      // odd number of levels
      asize = (levels+1)/2;
      csize = levels/2;
    }

    // auxiliar pyramids
    gaussianPyramid<channel> apyr(asize,gaussKernelSize,4);
    gaussianPyramid<channel> bpyr(asize,gaussKernelSize,4);
    gaussianPyramid<channel> cpyr(csize,gaussKernelSize,4);

    // A-Pyramid: original images
    apyr.generate(src);

    // B-Pyramid: voices with variance = 2
    convolver.apply(src,b);
    bpyr.generate(b);

    // C-Pyramid equivalent to A-Pyramid, but shifted one level up
    convolver.setKernel(gk4);
    convolver.apply(src,c);
    cpyr.generate(c);

    // build DoG pyramid
    for (i=firstLevel;i<lastLevel;++i) {
      if (even(i)) {
        // the even levels are between A and B pyramids
        pyr.at(i).apply(apyr.at(i/2),bpyr.at(i/2),absdiff);
      } else {
        // the odd levels are between B and C pyramids
        pyr.at(i).apply(bpyr.at(i/2),cpyr.at(i/2),absdiff);
      }
    }

    return pyramidToLocs(src,pyr,locs);
  };

  // On copy apply for type channel!
  bool axLocalRegions::apply(const channel& src,
                             std::map<float,std::list<location> >& locs) const{
    std::list<location> olocs;
    locs.clear();

    if (apply(src,olocs)) {
      std::list<location>::const_iterator it;
      std::list<location>* currentLocs = 0;
      float radius = -1.0f;
      it = olocs.begin();
      while (it!=olocs.end()) {
        if (radius != (*it).radius) {
          // get the list of locations for the given radius, and if it does not
          // exist, just create it!
          radius = (*it).radius;
          currentLocs = &locs[radius];
        }
        currentLocs->push_back(*it);
        ++it;
      }
      return true;
    }

    return false;
  };

  // On copy apply for type channel!
  bool axLocalRegions::pyramidToLocs(const channel& src,
                                     const pyramid<channel>& pyr,
                                           std::list<location>& locs) const {


    const parameters& param = getParameters();
    const int firstLevel = max(1,param.firstLevel);
    const int lastLevel = max(firstLevel,param.lastLevel);

    int i,x,y;
    channel a,b,surround;

    float mxKernelSize = static_cast<float>(param.firstMaximumFilterSize);
    int iMxKernelSize = max(3,iround(mxKernelSize));
    if (even(iMxKernelSize)) {
      iMxKernelSize++;
    }

    convolution convolver;
    maximumFilter<float> mxConvolver(iMxKernelSize);

    convolution::parameters convParam;
    convParam.boundaryType = lti::Constant;
    convolver.setParameters(convParam);

    // pyramids now ready!
    locs.clear();
    location loc;
    int factor = 1 << (firstLevel/2);
    float thresh;
    const int stSize = param.saliencyThreshold.size()-1;

    // search for the maxima, beginning with the level 1 of the pyramid!
    for (i=firstLevel;i<=lastLevel;++i) {
      const channel& chnl = pyr.at(i);
      const int lasty = chnl.lastRow();
      const int lastx = chnl.lastColumn();
      const float radius = static_cast<float>(pow(2.,1.+(double(i)/2.0)));

      mxConvolver.setSquareKernel(iMxKernelSize);
      mxConvolver.apply(chnl,surround);

      mxKernelSize *= param.maximumFilterAdaption;
      iMxKernelSize = max(3,iround(mxKernelSize));
      if (even(iMxKernelSize)) {
        iMxKernelSize++;
      }

      if (i>stSize) {
        thresh = surround.maximum()*param.saliencyThreshold.at(stSize);
      } else {
        thresh = surround.maximum()*param.saliencyThreshold.at(i);
      }

      for (y=1;y<lasty;++y) {
        for (x=1;x<lastx;++x) {
          // check if it is a maximum here
          if ((chnl.at(y,x) >= surround.at(y,x)) &&
              (chnl.at(y,x) > thresh)) {

            loc.position = tpoint<float>(static_cast<float>(x*factor),
                                         static_cast<float>(y*factor));
            loc.radius = radius;

            // and store the result
            locs.push_back(loc);
          }
        }
      }

      if (!even(i)) {
        factor*=2;
      }
    }

    // compute orientation for each location
    gradientKernelX<float> gx(3);
    gradientKernelY<float> gy(3);

    convolver.setKernel(gx);
    convolver.apply(src,a); // a is x component

    convolver.setKernel(gy);
    convolver.apply(src,b); // b is y component

    float wndLastX,wndLastY,w,accx,accy;
    std::list<location>::iterator it;
    for (it=locs.begin();it!=locs.end();++it) {
      const float r = 3*(*it).radius;
      const float r2n = (r*r)/9;
      wndLastX = min(static_cast<float>(src.columns()),
                     static_cast<float>((*it).position.x+r+1.5));
      wndLastY = min(static_cast<float>(src.rows()),
                     static_cast<float>((*it).position.y+r+1.5));
      accx=accy=0;
      for (y=max(0,static_cast<int>((*it).position.y-r+0.5));y<wndLastY;++y) {
        for (x=max(0,static_cast<int>((*it).position.x-r+0.5));
             x<wndLastX;
             ++x) {
          w = static_cast<float>(gaussian(y-(*it).position.y,x-(*it).position.x,r2n));
          accx+=a.at(y,x)*w;
          accy+=b.at(y,x)*w;
        }
      }
      (*it).angle = atan2(accy,accx);
    }

    return true;
  };


}
