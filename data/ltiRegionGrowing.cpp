/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiRegionGrowing.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 28.9.2000
 * revisions ..: $Id: ltiRegionGrowing.cpp,v 1.14 2006/09/05 10:28:13 ltilib Exp $
 */

#include "ltiObject.h"
#include "ltiMath.h"
#include "ltiConfig.h"
#include "ltiRegionGrowing.h"
#include "ltiSplitImageToRGB.h"
#include "ltiSplitImageToHSV.h"
#include "ltiSplitImageTorgI.h"
#include "ltiConvolution.h"
#include "ltiGaussKernels.h"
#include "ltiDownsampling.h"

#ifdef HAVE_SUSAN
#include "ltiSusanEdges.h"
#else
#include "ltiCannyEdges.h"
#endif

#define USE_VIEWER 0

#if USE_VIEWER
#include "ltiViewer.h"
#include <cstdio>
#include <iostream>

using std::cout;
using std::endl;
#endif

// required for MS VC++, which defines max and min as old-fashioned macros:
#undef max
#undef min

namespace lti {
  // --------------------------------------------------
  // regionGrowing::parameters
  // --------------------------------------------------

  // default constructor
  regionGrowing::parameters::parameters()
    : segmentation::parameters() {

    useGaussKernel = bool(false);
    localStatisticsKernelSize = int(5);
    localStatisticsKernelVariance = double(-1);
    smoothingKernelSize = int(3);
    smoothingThreshold = float(0.6);

    backgroundType = Dark;

    // the background patch will be take from the bottom-left corner
    patchPosition.copy(trectangle<float>(0.0f,0.9f,0.1f,1.0f));
    mode = eMode(UseGivenThresholds);
    scaleFactor = int(2);
    averageThreshold = float(0.05); // 5% difference
    averageThresholds = trgbPixel<float>(0.05f,0.05f,0.05f);
    edgesThreshold = float(0.05); // 5%

    // generate default seed points ((0,0),(0,1),(1,0),(1,1))
    seedPoints.resize(4,tpoint<float>(0,0));
    seedPoints.at(1).y = 1.0f;
    seedPoints.at(2).x = 1.0f;
    seedPoints.at(3).x = 1.0f;
    seedPoints.at(3).y = 1.0f;
  }

  // copy constructor
  regionGrowing::parameters::parameters(const parameters& other)
    : segmentation::parameters()  {
    copy(other);
  }

  // destructor
  regionGrowing::parameters::~parameters() {
  }

  // get type name
  const char* regionGrowing::parameters::getTypeName() const {
    return "regionGrowing::parameters";
  }

  // copy member

  regionGrowing::parameters&
    regionGrowing::parameters::copy(const parameters& other) {
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

    useGaussKernel = other.useGaussKernel;
    localStatisticsKernelSize = other.localStatisticsKernelSize;
    localStatisticsKernelVariance = other.localStatisticsKernelVariance;
    smoothingKernelSize = other.smoothingKernelSize;
    smoothingThreshold = other.smoothingThreshold;
    backgroundType = other.backgroundType;
    patchPosition.copy(other.patchPosition);
    mode = other.mode;
    scaleFactor = other.scaleFactor;
    averageThreshold = other.averageThreshold;
    averageThresholds.copy(other.averageThresholds);
    edgesThreshold = other.edgesThreshold;
    seedPoints.copy(other.seedPoints);

    return *this;
  }

  // clone member
  functor::parameters* regionGrowing::parameters::clone() const {
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
  bool regionGrowing::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool regionGrowing::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"useGaussKernel",useGaussKernel);
      lti::write(handler,"localStatisticsKernelSize",
                 localStatisticsKernelSize);
      lti::write(handler,"localStatisticsKernelVariance",
                 localStatisticsKernelVariance);
      lti::write(handler,"smoothingKernelSize",smoothingKernelSize);
      lti::write(handler,"smoothingThreshold",smoothingThreshold);

      switch(backgroundType) {
          case Medium:
            lti::write(handler,"backgroundType","Medium");
            break;
          case Light:
            lti::write(handler,"backgroundType","Light");
            break;
          default:
            // assume Dark
            lti::write(handler,"backgroundType","Dark");
            break;
      }

      lti::write(handler,"patchPosition",patchPosition);

      switch(mode) {
          case GetThresholdsRelative:
            lti::write(handler,"mode","GetThresholdsRelative");
            break;
          case GetThresholdsAbsolute:
            lti::write(handler,"mode","GetThresholdsAbsolute");
            break;
          default:
            lti::write(handler,"mode","UseGivenThresholds");
            break;
      }

      lti::write(handler,"scaleFactor",scaleFactor);
      lti::write(handler,"averageThreshold",averageThreshold);
      lti::write(handler,"averageThresholds",averageThresholds);
      lti::write(handler,"edgesThreshold",edgesThreshold);
      lti::write(handler,"seedPoints",seedPoints);
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
  bool regionGrowing::parameters::write(ioHandler& handler,
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
  bool regionGrowing::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool regionGrowing::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string tmp;

      b=b && lti::read(handler,"useGaussKernel",useGaussKernel);
      b=b && lti::read(handler,"localStatisticsKernelSize",
                localStatisticsKernelSize);
      b=b && lti::read(handler,"localStatisticsKernelVariance",
                localStatisticsKernelVariance);
      b=b && lti::read(handler,"smoothingKernelSize",smoothingKernelSize);
      b=b && lti::read(handler,"smoothingThreshold",smoothingThreshold);

      b=b && lti::read(handler,"backgroundType",tmp);

      if (tmp == "Light") {
        backgroundType = Light;
      } else if (tmp == "Medium") {
        backgroundType = Medium;
      } else {
        backgroundType = Dark;
      }

      b=b && lti::read(handler,"patchPosition",patchPosition);
      b=b && lti::read(handler,"mode",tmp);
      if (tmp == "GetThresholdsAbsolute") {
        mode = GetThresholdsAbsolute;
      } else if (tmp == "GetThresholdsRelative") {
        mode = GetThresholdsRelative;
      } else {
        mode = UseGivenThresholds;
      }

      b=b && lti::read(handler,"scaleFactor",scaleFactor);
      b=b && lti::read(handler,"averageThreshold",averageThreshold);
      b=b && lti::read(handler,"averageThresholds",averageThresholds);
      b=b && lti::read(handler,"edgesThreshold",edgesThreshold);
      b=b && lti::read(handler,"seedPoints",seedPoints);
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
  bool regionGrowing::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // regionGrowing
  // --------------------------------------------------

  // default constructor
  regionGrowing::regionGrowing()
    : segmentation(){
    parameters tmp;
    setParameters(tmp);
  }

  // default constructor
  regionGrowing::regionGrowing(const parameters& par)
    : segmentation(){
    setParameters(par);
  }

  // copy constructor
  regionGrowing::regionGrowing(const regionGrowing& other)
    : segmentation()  {
    copy(other);
  }

  // destructor
  regionGrowing::~regionGrowing() {
  }

  // returns the name of this type
  const char* regionGrowing::getTypeName() const {
    return "regionGrowing";
  }

  // copy member
  regionGrowing&
    regionGrowing::copy(const regionGrowing& other) {
      segmentation::copy(other);
    return (*this);
  }

  // clone member
  functor* regionGrowing::clone() const {
    return new regionGrowing(*this);
  }

  // return parameters
  const regionGrowing::parameters&
    regionGrowing::getParameters() const {
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


  // On place apply for type channel8!
  channel8& regionGrowing::apply(channel8& srcdest) const {
    const parameters& param = getParameters();
    channel8 msk;
    ubyte background;

    switch (param.backgroundType) {
      case parameters::Dark:
        background = 0;
        break;
      case parameters::Medium:
        background = 128;
        break;
      case parameters::Light:
        background = 255;
        break;
      default:
        setStatusString("Unknown backgroundType in regionGrowing\n");
        srcdest.clear();
        return srcdest;
    };

    apply(srcdest,msk);
    mask(srcdest,msk,background);

    return srcdest;
  };

  // On place apply for type channel!
  channel& regionGrowing::apply(channel& srcdest) const {
    const parameters& param = getParameters();
    channel8 msk;
    float background;

    switch (param.backgroundType) {
      case parameters::Dark:
        background = 0.0f;
        break;
      case parameters::Medium:
        background = 0.5f;
        break;
      case parameters::Light:
        background = 1.0f;
        break;
      default:
        setStatusString("Unknown backgroundType in regionGrowing\n");
        srcdest.clear();
        return srcdest;
    };

    apply(srcdest,msk);
    mask(srcdest,msk,background);

    return srcdest;
  };

  // On place apply for type image!
  image& regionGrowing::apply(image& srcdest) const {
    const parameters& param = getParameters();
    channel8 msk;
    rgbPixel background;

    switch (param.backgroundType) {
      case parameters::Dark:
        background = rgbPixel(0,0,0);
        break;
      case parameters::Medium:
        background = rgbPixel(128,128,128);
        break;
      case parameters::Light:
        background = rgbPixel(255,255,255);
        break;
      default:
        setStatusString("Unknown backgroundType in regionGrowing\n");
        srcdest.clear();
        return srcdest;
    };

    apply(srcdest,msk);
    mask(srcdest,msk,background);

    return srcdest;
  };

  // On copy apply for type channel8!
  channel8& regionGrowing::apply(const channel8& src,channel8& dest) const {
    channel tmp;
    tmp.castFrom(src);
    return segmentate(tmp,dest);
  };

  // On copy apply for type channel!
  channel8& regionGrowing::apply(const channel& src,channel8& dest) const {

    return segmentate(src,dest);
  };

  // On copy apply for type image!
  channel8& regionGrowing::apply(const image& src,channel8& dest) const {
    return segmentate(src,dest);
  }

  // On copy apply for type image!
  channel8& regionGrowing::segmentate(const image& src,channel8& dest) const {

    if ((src.rows() <= 0) || (src.columns() <= 0)) {
      dest.resize(0,0,0,false,false);
      return dest;
    }

    const parameters& param = getParameters();

    const int lastCol = src.lastColumn();
    const int lastRow = src.lastRow();

    trgbPixel<float> averageThresholds;
    int edgesThreshold;

    // 1. calculate threshold values
    if (param.mode != parameters::UseGivenThresholds) {
      trgbPixel<float> stdDevThresholds;

      trectangle<float> tpatch = param.patchPosition;

      if (param.mode == parameters::GetThresholdsRelative) {
        tpatch.ul.x *= lastCol;
        tpatch.ul.y *= lastRow;
        tpatch.br.x *= lastCol;
        tpatch.br.y *= lastRow;
      }

      // check for valid patch ranges:
      if (tpatch.ul.x < 0) {
        tpatch.ul.x = 0;
      } else if (tpatch.ul.x > lastCol) {
        tpatch.ul.x = static_cast<float>(lastCol);
      }

      if (tpatch.br.x < 0) {
        tpatch.br.x = 0;
      } else if (tpatch.br.x > lastCol) {
        tpatch.br.x = static_cast<float>(lastCol);
      }

      if (tpatch.ul.y < 0) {
        tpatch.ul.y = 0;
      } else if (tpatch.ul.y > lastRow) {
        tpatch.ul.y = static_cast<float>(lastRow);
      }

      if (tpatch.br.y < 0) {
        tpatch.br.y = 0;
      } else if (tpatch.br.y > lastRow) {
        tpatch.br.y = static_cast<float>(lastRow);
      }

      rectangle patch; // after floating point stuff, just an integer
                       // valued result!

      patch.castFrom(tpatch);
      patch.ensureConsistency();

      // calculate average and std. deviation for the given patch
      point p;
      trgbPixel<int> sum(0,0,0);
      trgbPixel<int> sum2(0,0,0);
      trgbPixel<int> tmp;

      for (p.y=patch.ul.y;p.y<patch.br.y;p.y++) {
        for (p.x=patch.ul.x;p.x<patch.br.x;p.x++) {
          tmp = src.at(p);
          sum += tmp;
          tmp *= tmp;
          sum2 += tmp;
        }
      }

      int n = (1+patch.br.x-patch.ul.x)*(1+patch.br.y-patch.ul.y);

      averageThresholds.castFrom(sum);
      averageThresholds.divide(static_cast<float>(n));

      stdDevThresholds.castFrom(sum2);
      stdDevThresholds.divide(static_cast<float>(n));

      trgbPixel<float> tmpsum(averageThresholds);
      tmpsum.multiply(tmpsum);

      stdDevThresholds.subtract(tmpsum);
      stdDevThresholds.apply(sqroot);

      switch (param.backgroundType) {
        case parameters::Dark: {
          stdDevThresholds.multiply(3);

          averageThresholds.add(stdDevThresholds);

          edgesThreshold = int(lti::max(averageThresholds.red,
                                        averageThresholds.green,
                                        averageThresholds.blue));
        }
        break;
        case parameters::Medium: {
          stdDevThresholds.multiply(4);

          averageThresholds.copy(stdDevThresholds);

          edgesThreshold = int(averageThresholds.red+
                               averageThresholds.green+
                               averageThresholds.blue)/3;

        }
        break;
        case parameters::Light: {
          stdDevThresholds.multiply(3);

          averageThresholds = trgbPixel<float>(255,255,255) -
                              averageThresholds;

          averageThresholds.add(stdDevThresholds);

          edgesThreshold = int(lti::min(averageThresholds.red,
                                        averageThresholds.green,
                                        averageThresholds.blue));
        }
        break;
        default:
          // it is not possible to come here, but if things change...
          throw invalidParametersException(getTypeName());
      }

    } else {
      averageThresholds.castFrom(param.averageThresholds);
      averageThresholds.multiply(255);
      edgesThreshold = int(255.0f*param.edgesThreshold);
    }

    // 2. Calculate local average from the downsampled
    //    image, using a filter kernel generated with the user specified
    //    parameters.

    // create local region filter-kernel

    sepKernel<float> kernel;

    if (param.useGaussKernel) {
      gaussKernel2D<float> tmp(param.localStatisticsKernelSize,
                               param.localStatisticsKernelVariance);
      kernel.copy(tmp);
    } else {
      // construct a rectangular filter
      const int& size =  param.localStatisticsKernelSize;
      const int border = size/2;

      kernel1D<float> oneD(border-size+1,border,1.0f/float(size));

      kernel.setNumberOfPairs(1);

      kernel.getRowFilter(0).copy(oneD);
      kernel.getColFilter(0).copy(oneD);
    }

    // the original channels
    channel8 red,green,blue,intensity;
    // the average channels
    channel8 ared,agreen,ablue;

    splitImageToRGB splitter;
    splitter.apply(src,red,green,blue);

    // get an intensity channel for the edge detection
    switch (param.backgroundType) {
      case parameters::Dark: {
        splitImageToHSV intensitySplitter;
        intensitySplitter.getValue(src,intensity);
      };
      break;
      case parameters::Medium: {
        splitImageTorgI intensitySplitter;
        intensitySplitter.getIntensity(src,intensity);
      };
      break;
      case parameters::Light: {
        image::const_iterator cit,e;
        channel8::iterator it;
        for (cit=src.begin(),e=src.end(),it=intensity.begin();
             cit!=e;
             cit++,it++) {
          const rgbPixel& pix = (*cit);
          (*it) = min(pix.getRed(),pix.getGreen(),pix.getBlue());
        }
      }
      break;
      default:
        throw invalidParametersException(getTypeName());
    };

    convolution convolver;
    convolution::parameters convpar;
    convpar.setKernel(kernel);
    convpar.boundaryType = lti::Constant;
    convolver.setParameters(convpar);

    convolver.apply(red,ared);
    convolver.apply(green,agreen);
    convolver.apply(blue,ablue);

    // 3. edge detection

    // TODO:  this might change in the future to use the edgeDetectorFactory
    //        and a new parameter which indicates which edge detector should
    //        be used.

#   ifdef HAVE_SUSAN
    susanEdges edger;
    susanEdges::parameters edgeParams;
    // default parameters are: RGBmerge, threshold = 5%...
    edgeParams.threshold = edgesThreshold;
#   else
    cannyEdges edger;
    cannyEdges::parameters edgeParams;
    edgeParams.thresholdMax = edgesThreshold;
#   endif

    edger.setParameters(edgeParams);
    edger.apply(intensity);

    // 4. region grow algorithm

    static const ubyte background = 0;
    static const ubyte unknown = 128;
    static const ubyte object  = 255;

    // initilize mask with everything as "unknown"
    dest.resize(src.size(),unknown);

    channel8::const_iterator cit,ce;
    channel8::iterator it,e;

    // all edge points belong to the object
    for (cit=intensity.begin(),it=dest.begin(),ce=intensity.end();
         cit!=ce;
         cit++,it++) {
      if (*cit != 0) {
        (*it)=object;
      }
    }

    std::list<point> ptlist;

    point seed,tp;

    // add the user specified starting seed points
    int i;

    for (i=0;i<param.seedPoints.size();i++) {
      ptlist.push_back(point(int(float(lastCol)*param.seedPoints.at(i).x),
                             int(float(lastRow)*param.seedPoints.at(i).y)));

    }

    trgbPixel<int> avrgThresholds;
    averageThresholds.add(0.5f);
    avrgThresholds.castFrom(averageThresholds);

#if USE_VIEWER
    // ############# DEBUG

    static viewer view1;
    int vcounter = 0;

    // ############# END DEBUG
#endif

    // due to the edges, a N4 neighbourhood MUST be used.
    // with a N8 neighbourhood the edges would be skipped...

    static const int sizedelta = 4;
    static const int deltax[4] = { 0,-1,+1, 0};
    static const int deltay[4] = {-1, 0, 0,+1};

    // grow each pixel on the point list
    while (!ptlist.empty()) {
      // get first element on the list
      seed = ptlist.front();
      ptlist.pop_front();

      // if already evaluated do nothing
      if (dest.at(seed) == unknown) {
        dest.at(seed) = background;

        // check for the neighbours
        for (i=0;i<sizedelta;i++) {
          tp = seed+point(deltax[i],deltay[i]);

          if ((tp.x >= 0) && (tp.x <= lastCol) &&
              (tp.y >= 0) && (tp.y <= lastRow) &&
              (dest.at(tp)==unknown)) {

            // if pixel similar to the seed, insert it in the point list
            // to be also evaluated.  Otherwise just mark the point as object
            if (similar(red.at(tp),ared.at(seed),avrgThresholds.red) &&
                similar(green.at(tp),agreen.at(seed),avrgThresholds.green) &&
                similar(blue.at(tp),ablue.at(seed),avrgThresholds.blue)) {

              ptlist.push_back(tp);

            } else {

              dest.at(tp) = object;

            }
          }
        }

#if USE_VIEWER

        // ############# DEBUG
        if ((vcounter % 500) == 0) {
          view1.show(dest);
        }

        vcounter++;
        // ############# END DEBUG
#endif

      }

    }

#if USE_VIEWER
    // ############# DEBUG
    view1.show(dest);

    getchar();
    // ############# END DEBUG
#endif


    // consider all unknown points also as objects:

    for (it=dest.begin(),e=dest.end();it!=e;it++) {
      if (*it == unknown) {
        (*it)=object;
      }
    }

#   if USE_VIEWER
    // ############# DEBUG
    cout << "After making \"unknown\" part of the object" << endl;
    view1.show(dest);

    getchar();
    // ############# END DEBUG
#   endif

    // apply a smoothing filter if requested...
    if (param.smoothingKernelSize > 1) {
      convolver.setKernel(gaussKernel2D<float>(param.smoothingKernelSize));
      convolver.apply(dest);

#     if USE_VIEWER
      // ############# DEBUG
      cout << "After smoothing filter" << endl;
      view1.show(dest);

      getchar();
      // ############# END DEBUG
#     endif


      const ubyte thresh = ubyte(255.0*param.smoothingThreshold);

      for (it=dest.begin(),e=dest.end();it!=e;it++) {
        if (*it >= thresh) {
          (*it)=object;
        } else {
          (*it)=background;
        }
      }

    }

#   if USE_VIEWER
    // ############# DEBUG
    cout << "Result" << endl;
    view1.show(dest);

    getchar();
    // ############# END DEBUG
#   endif

    return dest;
  };

  // On copy apply for type image!
  channel8& regionGrowing::segmentate(const channel& src,
                                            channel8& dest) const {

    const parameters& param = getParameters();

    const int lastCol = src.lastColumn();
    const int lastRow = src.lastRow();

    float averageThreshold;
    float edgesThreshold;

    // 1. calculate threshold values
    if (param.mode != parameters::UseGivenThresholds) {
      float stdDevThreshold;

      trectangle<float> tpatch = param.patchPosition;

      if (param.mode == parameters::GetThresholdsRelative) {
        tpatch.ul.x *= lastCol;
        tpatch.ul.y *= lastRow;
        tpatch.br.x *= lastCol;
        tpatch.br.y *= lastRow;
      }

      // check for valid patch ranges:
      if (tpatch.ul.x < 0) {
        tpatch.ul.x = 0;
      } else if (tpatch.ul.x > lastCol) {
        tpatch.ul.x = static_cast<float>(lastCol);
      }

      if (tpatch.br.x < 0) {
        tpatch.br.x = 0;
      } else if (tpatch.br.x > lastCol) {
        tpatch.br.x = static_cast<float>(lastCol);
      }

      if (tpatch.ul.y < 0) {
        tpatch.ul.y = 0;
      } else if (tpatch.ul.y > lastRow) {
        tpatch.ul.y = static_cast<float>(lastRow);
      }

      if (tpatch.br.y < 0) {
        tpatch.br.y = 0;
      } else if (tpatch.br.y > lastRow) {
        tpatch.br.y = static_cast<float>(lastRow);
      }

      rectangle patch; // after floating point stuff, just an integer
                       // valued result!

      patch.castFrom(tpatch);
      patch.ensureConsistency();

      // calculate average and std. deviation for the given patch
      point p;
      float sum = 0;
      float sum2 = 0;
      float tmp;

      for (p.y=patch.ul.y;p.y<patch.br.y;p.y++) {
        for (p.x=patch.ul.x;p.x<patch.br.x;p.x++) {
          tmp = src.at(p);
          sum += tmp;
          tmp *= tmp;
          sum2 += tmp;
        }
      }

      int n = (1+patch.br.x-patch.ul.x)*(1+patch.br.y-patch.ul.y);

      averageThreshold = sum/float(n);
      stdDevThreshold = sum2/float(n);

      tmp = averageThreshold*averageThreshold;

      stdDevThreshold -=  tmp;
      stdDevThreshold = sqrt(stdDevThreshold);

      switch (param.backgroundType) {
        case parameters::Dark: {
          stdDevThreshold *= 3;
          averageThreshold += stdDevThreshold;
          edgesThreshold = averageThreshold;
        }
        break;
        case parameters::Medium: {
          stdDevThreshold *= 4;
          averageThreshold = stdDevThreshold;
          edgesThreshold = averageThreshold;
        }
        break;
        case parameters::Light: {
          stdDevThreshold *= 3;
          averageThreshold = 1.0f - averageThreshold + stdDevThreshold;
          edgesThreshold = averageThreshold;
        }
        break;
        default:
          // it is not possible to come here, but if things change...
          throw invalidParametersException(getTypeName());
      }

    } else {
      averageThreshold = param.averageThreshold;
      edgesThreshold = param.edgesThreshold;
    }

    // 2. Calculate local average from the downsampled
    //    image, using a filter kernel generated with the user specified
    //    parameters.

    // create local region filter-kernel

    sepKernel<float> kernel;

    if (param.useGaussKernel) {
      gaussKernel2D<float> tmp(param.localStatisticsKernelSize,
                               param.localStatisticsKernelVariance);
      kernel.copy(tmp);
    } else {
      // construct a rectangular filter
      const int& size =  param.localStatisticsKernelSize;
      const int border = size/2;

      kernel1D<float> oneD(border-size+1,border,1.0f/float(size));

      kernel.setNumberOfPairs(1);

      kernel.getRowFilter(0).copy(oneD);
      kernel.getColFilter(0).copy(oneD);
    }

    // the original channels
    channel8 edges;
    // the average channels
    channel aint;

    convolution convolver;
    convolution::parameters convpar;
    convpar.setKernel(kernel);
    convpar.boundaryType = lti::Constant;
    convolver.setParameters(convpar);

    convolver.apply(src,aint);

    // 3. edge detection
#   ifdef HAVE_SUSAN
    susanEdges edger;
    susanEdges::parameters edgeParams;
    // default parameters are: RGBmerge, threshold = 5%...
    edgeParams.threshold = int(0.5f+255.0f*edgesThreshold);
#   else
    cannyEdges edger;
    cannyEdges::parameters edgeParams;
    edgeParams.thresholdMax = edgesThreshold;
#   endif

    edger.setParameters(edgeParams);
    edges.castFrom(src);

    edger.apply(edges);

    // 4. region grow algorithm

    static const ubyte background = 0;
    static const ubyte unknown = 128;
    static const ubyte object  = 255;

    // initilize mask with everything as "unknown"
    dest.resize(src.size(),unknown);

    channel8::const_iterator cit,ce;
    channel8::iterator it,e;

    // all edge points belong to the object
    for (cit=edges.begin(),ce=edges.end(),it=dest.begin();
         cit!=ce;
         cit++,it++) {
      if (*cit != 0) {
        (*it)=object;
      }
    }

    std::list<point> ptlist;

    point seed,tp;

    // add the user specified starting seed points
    int i;

    for (i=0;i<param.seedPoints.size();i++) {
      ptlist.push_back(point(int(float(lastCol)*param.seedPoints.at(i).x),
                             int(float(lastRow)*param.seedPoints.at(i).y)));

    }

#if USE_VIEWER
    // ############# DEBUG

    static viewer view1;
    int vcounter = 0;

    // ############# END DEBUG
#endif

    static const int sizedelta = 4;
    static const int deltax[4] = { 0,-1,+1, 0};
    static const int deltay[4] = {-1, 0, 0,+1};

    // grow each pixel on the point list
    while (!ptlist.empty()) {
      // get first element on the list
      seed = ptlist.front();
      ptlist.pop_front();

      // if already evaluated do nothing
      if (dest.at(seed) == unknown) {
        dest.at(seed) = background;

        // check for the neighbours
        for (i=0;i<sizedelta;i++) {
          tp = seed+point(deltax[i],deltay[i]);

          if ((tp.x >= 0) && (tp.x <= lastCol) &&
              (tp.y >= 0) && (tp.y <= lastRow) &&
              (dest.at(tp)==unknown)) {

            // if pixel similar to the seed, insert it in the point list
            // to be also evaluated.  Otherwise just mark the point as object
            if (similar(src.at(tp),aint.at(seed),averageThreshold)) {

              ptlist.push_back(tp);

            } else {

              dest.at(tp) = object;

            }
          }
        }

#if USE_VIEWER

        // ############# DEBUG
        if ((vcounter % 300) == 0) {
          view1.show(dest);
        }

        vcounter++;
        // ############# END DEBUG
#endif

      }

    }

#if USE_VIEWER
    // ############# DEBUG
    view1.show(dest);

    getchar();
    // ############# END DEBUG
#endif


    // consider all unknown points also as objects:

    for (it=dest.begin(),e=dest.end();it!=e;it++) {
      if (*it == unknown) {
        (*it)=object;
      }
    }

    // apply a smoothing filter if requested...
    if (param.smoothingKernelSize > 1) {
      convolver.setKernel(gaussKernel2D<float>(param.smoothingKernelSize));
      convolver.apply(dest);

      const ubyte thresh = ubyte(255.0*param.smoothingThreshold);

      for (it=dest.begin(),e=dest.end();it!=e;it++) {
        if (*it >= thresh) {
          (*it)=object;
        } else {
          (*it)=background;
        }
      }

    }

    return dest;
  };


}
