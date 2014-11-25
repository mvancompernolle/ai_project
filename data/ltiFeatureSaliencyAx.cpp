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
 * file .......: ltiFeatureSaliencyAx.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 23.2.2001
 * revisions ..: $Id: ltiFeatureSaliencyAx.cpp,v 1.14 2006/09/05 10:12:34 ltilib Exp $
 */

#ifdef _DEBUG
// #define _DEBUG_SALIENCY_AX 1
#endif

#include "ltiFeatureSaliencyAx.h"
#include "ltiKullbackContrast.h"
#include "ltiUpsampling.h"
#include "ltiGaussianPyramid.h"
#include "ltiSplitImageToRGB.h"
#include "ltiSplitImageTorgI.h"
#include "ltiConvolution.h"
#include "ltiKullbackContrast.h"
#include "ltiSquareConvolution.h"
#include "ltiQmf.h"
#include "ltiQmfKernels.h"

#ifdef _DEBUG_SALIENCY_AX

#include "ltiExpandVector.h"
#include "ltiViewer.h"
#include <iostream>

using std::cout;
using std::endl;

#endif

namespace lti {
  // --------------------------------------------------
  // featureSaliencyAx::parameters
  // --------------------------------------------------

  // default constructor
  featureSaliencyAx::parameters::parameters()
    : saliency::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    levels = int(256);
    doIntensity   = false;
    doColor       = true;
    doOrientation = false;
    useHistogram = true;
  }

  // copy constructor
  featureSaliencyAx::parameters::parameters(const parameters& other)
    : saliency::parameters()  {
    copy(other);
  }

  // destructor
  featureSaliencyAx::parameters::~parameters() {
  }

  // get type name
  const char* featureSaliencyAx::parameters::getTypeName() const {
    return "featureSaliencyAx::parameters";
  }

  // copy member

  featureSaliencyAx::parameters&
    featureSaliencyAx::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    saliency::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    saliency::parameters& (saliency::parameters::* p_copy)
      (const saliency::parameters&) =
      saliency::parameters::copy;
    (this->*p_copy)(other);
# endif

    levels = other.levels;

    doIntensity   = other.doIntensity;
    doColor       = other.doColor;
    doOrientation = other.doOrientation;
    useHistogram  = other.useHistogram;

    return *this;
  }

  // alias for copy member
  featureSaliencyAx::parameters&
    featureSaliencyAx::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* featureSaliencyAx::parameters::clone() const {
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
  bool featureSaliencyAx::parameters::write(ioHandler& handler,
                                            const bool complete) const
# else
  bool featureSaliencyAx::parameters::writeMS(ioHandler& handler,
                                              const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"levels",levels);
      lti::write(handler,"doIntensity",doIntensity);
      lti::write(handler,"doOrientation",doOrientation);
      lti::write(handler,"doColor",doColor);
      lti::write(handler,"useHistogram",useHistogram);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && saliency::parameters::write(handler,false);
# else
    bool (saliency::parameters::* p_writeMS)(ioHandler&,const bool) const =
      saliency::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool featureSaliencyAx::parameters::write(ioHandler& handler,
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
  bool featureSaliencyAx::parameters::read(ioHandler& handler,
                                           const bool complete)
# else
  bool featureSaliencyAx::parameters::readMS(ioHandler& handler,
                                             const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"levels",levels);
      lti::read(handler,"doIntensity",doIntensity);
      lti::read(handler,"doOrientation",doOrientation);
      lti::read(handler,"doColor",doColor);
      lti::read(handler,"useHistogram",useHistogram);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && saliency::parameters::read(handler,false);
# else
    bool (saliency::parameters::* p_readMS)(ioHandler&,const bool) =
      saliency::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool featureSaliencyAx::parameters::read(ioHandler& handler,
                                           const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // featureSaliencyAx
  // --------------------------------------------------

  // default constructor
  featureSaliencyAx::featureSaliencyAx()
    : saliency() {
    parameters defaultPar;
    setParameters(defaultPar);
  }

  // copy constructor
  featureSaliencyAx::featureSaliencyAx(const featureSaliencyAx& other)
    : saliency()  {
    copy(other);
  }

  // destructor
  featureSaliencyAx::~featureSaliencyAx() {
  }

  // returns the name of this type
  const char* featureSaliencyAx::getTypeName() const {
    return "featureSaliencyAx";
  }

  // copy member
  featureSaliencyAx&
    featureSaliencyAx::copy(const featureSaliencyAx& other) {
      saliency::copy(other);
    return (*this);
  }

  // clone member
  functor* featureSaliencyAx::clone() const {
    return new featureSaliencyAx(*this);
  }

  // return parameters
  const featureSaliencyAx::parameters&
    featureSaliencyAx::getParameters() const {
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

  // On copy apply for type image!
  bool featureSaliencyAx::apply(const image& src,channel& dest) {

    channel ci,cc,co;
    return apply(src,ci,cc,co,dest);
  };

  const std::list<int>& featureSaliencyAx::getResolutions() const {
    return resolutions;
  }

  // On copy apply for type image!
  bool featureSaliencyAx::apply(const image& src,
                                channel& conspIntensity,
                                channel& conspColor,
                                channel& conspOrientation,
                                channel& dest) {

    if (src.empty()) {
      // error... empty source
      conspIntensity.clear();
      conspColor.clear();
      conspOrientation.clear();

      dest.clear();

      return false;
    }

    const parameters& param = getParameters();
    splitImageToRGB split2RGB;
    splitImageTorgI split2rgI;

    channel intensity;
    channel red,green,blue;

    vector<float> kullb;

    kullbackContrast contrast;

    kullbackContrast::parameters contPar;
    contPar.resolutions = 
      min(param.levels,static_cast<int>(ceil(log(double(min(src.rows(),
                                                            src.columns())))/
                                             log(2.0))));
    contPar.useHistogram = param.useHistogram;
    contPar.useGaussian = true;
    contrast.setParameters(contPar);

    // to ignore the shadows, for the analysis only the color information
    // will be considered.  The color information can be accumulated in a
    // red+green chromaticity channel
    split2rgI.getIntensity(src,intensity);

    // calculate a gaussian pyramid of the chromaticity, and then generate
    // the Kullback-Contrast to determine which resolutions are important
    contrast.apply(intensity,kullb);

#ifdef _DEBUG_SALIENCY_AX
    cout << "Kullb: " << kullb << endl;
    expandVector expander;
    vector<float> vct;
    static viewer view;
    expander.apply(kullb,vct);
    view.show(vct);

    cout << "Resolutions: ";
#endif

    // get the most important resolutions
    int i;

    resolutions.clear();
    resWithHighestKullback = 0;
    float hk = 0.0f;

    for (i=1;i<kullb.lastIdx();++i) {
      if (kullb.at(i) > hk) {
        hk = kullb.at(i);
        resWithHighestKullback = i;
      }

      if ((kullb.at(i-1)<kullb.at(i)) &&
          (kullb.at(i+1)<kullb.at(i))) {
        resolutions.push_back(i-1);

#       ifdef _DEBUG_SALIENCY_AX
        cout << i-1 << ", ";
#       endif

      }
    }

    if (kullb.at(i) > hk) {
      resWithHighestKullback = i;
    }

#   ifdef _DEBUG_SALIENCY_AX
    cout << i-1 << ")" << endl;
#   endif


    if (resolutions.empty()) {
      // there was no salient resolution...
      resolutions.push_back(1);

      // look for the most stable resolution
      int idx = 1;
      float intervall = abs(kullb.at(kullb.lastIdx()) - kullb.at(0));
      for (i=1;i<kullb.lastIdx();++i) {
        if (abs(kullb.at(i+1)-kullb.at(i)) < intervall) {
          idx = i;
          intervall = abs(kullb.at(i+1)-kullb.at(i));
        }
      }

      // ... and take the next resolution
      if (idx>1) {
        resolutions.push_back(idx+1);
      }

    }

    bool initialized = false;
    int numMaps = 0;

    // generate the intensity conspicuity map using this resolutions
    if (param.doIntensity) {
      intensityConsp(intensity,conspIntensity);
      dest.copy(conspIntensity);
      initialized = true;
      numMaps++;
    }

    // generate the color conspicuity map using r,g,b
    if (param.doColor) {
      split2RGB.apply(src,red,green,blue);
      colorConsp(red,green,blue,conspColor);
      if (initialized) {
        dest.add(conspColor);
      } else {
        dest.copy(conspColor);
        initialized = true;
      }
      numMaps++;
    }

    // generate the orientation conspicuity map using the intensity
    if (param.doOrientation) {
      orientationConsp(intensity,conspOrientation);
      if (initialized) {
        dest.add(conspOrientation);
      } else {
        dest.copy(conspOrientation);
      }
      numMaps++;
    }

    if (numMaps > 1) {
      normalize(dest);
    }

    return true;
  };

  void featureSaliencyAx::intensityConsp(const channel& intensity,
                                         channel& iconsp) const {

    gaussianPyramid<channel> pyr,pyr2;

#ifdef _DEBUG_SALIENCY_AX
    cout << "resolutions: (";

    std::list<int>::const_iterator dit;
    for (dit = resolutions.begin();dit!=resolutions.end();++dit) {
      cout << *dit << " ,";
    }

    cout << ")\n";
#endif

    pyr.resize((*resolutions.rbegin())+2,false);
    pyr2.resize(pyr.size()-1,false);

    squarePyramid(intensity,pyr);

    int i;

    for (i=0;i<pyr2.size();++i) {
      if (!pyr.at(i+1).empty()) {
        pyr2.at(i).useExternData(pyr.at(i+1).rows(),
                                 pyr.at(i+1).columns(),
                                 &pyr.at(i+1).at(0,0));
      }
    }

    std::list<int>::const_reverse_iterator it;
    bool firstOne = true;
    channel tmp;
    upsampling upsampler;
    upsampling::parameters upsampParam;

    upsampParam.boundaryType = lti::Constant;
    upsampler.setParameters(upsampParam);

    for (it=resolutions.rbegin();it!=resolutions.rend();++it) {

      if (firstOne) {
        iconsp.subtract(pyr.at(*it),pyr2.at(*it));
        iconsp.apply(abs);

        firstOne = false;
      } else {
        tmp.subtract(pyr.at(*it),pyr2.at(*it));
        tmp.apply(abs);

        while (iconsp.rows() < tmp.rows()) {
          upsampler.apply(iconsp);
        }

        if (iconsp.size() != tmp.size()) {
          iconsp.resize(tmp.size(),0.0f,true,false);
        }

        iconsp.add(tmp);
      }
    }

    normalize(iconsp);

    while (iconsp.rows() < pyr.at(0).rows()) {
      upsampler.apply(iconsp);
    }

    if (iconsp.size() != pyr.at(0).size()) {
      iconsp.resize(pyr.at(0).size(),0.0f,true,false);
    }
  }

  void featureSaliencyAx::colorConsp(const channel& red,
                                     const channel& green,
                                     const channel& blue,
                                     channel& cconsp) const {

    const int resol = 1 + *resolutions.rbegin();

    pyramid<channel> rpyr(resol),gpyr(resol+1),bpyr(resol),ypyr(resol+1);

    channel yellow;
    channel tmp;
    channel rg,by;

    yellow.add(red,green);
    yellow.divide(2.0f);

    // opponent color channels
    squarePyramid0(red,rpyr);
    squarePyramid1(green,gpyr);
    squarePyramid0(blue,bpyr);
    squarePyramid1(yellow,ypyr);

    // combine oc-channels
    bool firstOne = true;

    std::list<int>::const_iterator it;

    for (it=resolutions.begin();it!=resolutions.end();++it) {
      if (firstOne) {
        rg.subtract(rpyr.at(*it),gpyr.at(1+*it));
        rg.apply(abs);

        by.subtract(bpyr.at(*it),ypyr.at(1+*it));
        by.apply(abs);

        firstOne = false;
      } else {
        tmp.subtract(rpyr.at(*it),gpyr.at(1+*it));
        tmp.apply(abs);

        rg.add(tmp);

        tmp.subtract(bpyr.at(*it),ypyr.at(1+*it));
        tmp.apply(abs);

        by.add(tmp);
      }
    }

#if 0
    // apply "DoG" to look for salient regions and use for it the highest
    // resolution and the most "important" resolution

    squareConvolution<float> convolver;
    squareConvolution<float>::parameters convParam;
    convParam.boundaryType = squareConvolution<float>::parameters::Constant;

    int kernelSize,kidx;

    kernelSize = 1 << resWithHighestKullback;
    kidx = kernelSize/2;
    point kdim;

    // set-up kernel
    convParam.kernelSize = rectangle(-kidx,-kidx,kidx,kidx);
    kdim = convParam.kernelSize.getDimensions();
    convParam.kernelNorm = kdim.x*kdim.y;
    convolver.setParameters(convParam);

    // square-filter the image
    convolver.apply(rg,tmp);

    //
    rg.subtract(tmp);
    rg.apply(rectify);

    // square-filter the by image
    convolver.apply(by,tmp);

    by.subtract(tmp);
    by.apply(rectify);
#endif

    normalize(rg);
    normalize(by);

    cconsp.add(rg,by);

    normalize(cconsp);

  }

  void featureSaliencyAx::orientationConsp(const channel& intensity,
                                           channel& oconsp) const {

    // to simplify merging of different bands, copy the source channel into
    // an image with 2^n x 2^m pixels.

    const int n = *resolutions.rbegin();

    int i,j,x(0),y(0);
    channel wl;
    channel chnl;
    qmf wavelet;
    qmf::parameters wlParam;
    wlParam.boundaryType = lti::Constant;
    wlParam.levels = n;
    wlParam.kernel.copy(tap9Symmetric());
    wavelet.setParameters(wlParam);

    i = 1 << n; // (2^n)

    point p = intensity.size();

    if ((p.x % i) != 0) {
      p.x = (int(p.x/i)+1)*i;
    }

    if ((p.y % i) != 0) {
      p.y = (int(p.y/i)+1)*i;
    }

    if (p == intensity.size()) {
      wavelet.apply(intensity,wl);
    } else {
      chnl.resize(p,0.0f,false,false);

      x = (p.x-intensity.columns())/2;
      y = (p.y-intensity.rows())/2;

      chnl.fill(intensity,y,x,chnl.rows(),chnl.columns());

      // initialize border as constant
      const int lasty = y+intensity.rows();
      const int lastx = x+intensity.columns();
      const int cols = chnl.columns();
      const int rows = chnl.rows();

      // first the left and right sides
      for (j=y;j<lasty;++j) {

        float val = chnl.at(j,x);
        for (i=0;i<x;++i) {
          chnl.at(j,i) = val;
        }

        val = chnl.at(j,lastx-1);
        for (i=lastx;i<cols;++i) {
          chnl.at(j,i) = val;
        }
      }

      // now the upper and bottom frames
      const vector<float>& vcttop = chnl.getRow(y);
      for (j=0;j<y;++j) {
        chnl.getRow(j).fill(vcttop);
      }

      const vector<float>& vctbuttom = chnl.getRow(lasty-1);
      for (j=lasty;j<rows;++j) {
        chnl.getRow(j).fill(vctbuttom);
      }

      // now calculate the transform of the channel
      wavelet.apply(chnl,wl);
    }

    // calculate energy
    wl.apply(sqr);

    // the wavelet transform is in the wl channel, now try to generate
    // the energy image from it
    const vector<point>& partitioning = wavelet.getLastPartitioning();

    point delta;
    upsampling upsampler;
    upsampling::parameters uppar;
    uppar.boundaryType = lti::Constant;

    i=partitioning.size()-1;
    delta = partitioning.at(i-1)-partitioning.at(i);

    chnl.resize(delta,0,false,true);

    for (i=partitioning.size()-1;i>0;i--) {

      channel a(false,wl,
                partitioning.at(i).y+1,partitioning.at(i-1).y,
                partitioning.at(i).x+1,partitioning.at(i-1).x);

      channel b(false,wl,
                0,partitioning.at(i).y,
                partitioning.at(i).x+1,partitioning.at(i-1).x);

      channel c(false,wl,
                partitioning.at(i).y+1,partitioning.at(i-1).y,
                0,partitioning.at(i).x);

      normalize(a);
      normalize(b);
      normalize(c);

      chnl.add(a);
      chnl.add(b);
      chnl.add(c);

      normalize(chnl);

      upsampler.apply(chnl);
    }

    // ensure that the correct resolution is reconstructed
    while (chnl.rows() < wl.rows()) {
      upsampler.apply(chnl);
    }

    // take only the important data
    if (chnl.size() == intensity.size()) {
      chnl.detach(oconsp);
    } else {
      oconsp.resize(intensity.size(),0.0f,false,false);
      oconsp.fill(chnl,0,0,oconsp.rows(),oconsp.columns(),y,x);
    }

    normalize(oconsp);
  }

  /*
   * generate a pyramid using square filters
   */
  void featureSaliencyAx::squarePyramid(const channel& src,
                                        pyramid<channel>& pyr) const {

    squareConvolution<float> convolver;
    squareConvolution<float>::parameters convParam;
    convParam.boundaryType = lti::Constant;

    int i,kernelSize,kidx;
    kidx = 1;
    kernelSize = 2;
    pyr.at(0).copy(src);
    point kdim;

    // only the elements included in the list "resolution" are required

    std::list<int>::const_iterator it;
    it = resolutions.begin();
    bool lastWasInResolutions = false;

    if ((*it) == 0) {
      pyr.at(0).copy(src);
      lastWasInResolutions = true;
      ++it;
    }

    for (i=1; i<pyr.size(); ++i) {
      if (lastWasInResolutions || (*it) == i) {

        // set-up kernel
        convParam.kernelSize = rectangle(-kidx,-kidx,kidx,kidx);
        kdim = convParam.kernelSize.getDimensions();
        convParam.kernelNorm = static_cast<float>(kdim.x*kdim.y);
        convolver.setParameters(convParam);

        // square-filter the image
        convolver.apply(src,pyr.at(i));

        // update flags
        lastWasInResolutions = ((*it) == i);
        if (lastWasInResolutions) {
          ++it;
        }
      }

      // update next kernel values
      kidx = kernelSize;
      kernelSize *= 2;

    }

  }

  /*
   * generate a pyramid using square filters
   */
  void featureSaliencyAx::squarePyramid0(const channel& src,
                                         pyramid<channel>& pyr) const {

    squareConvolution<float> convolver;
    squareConvolution<float>::parameters convParam;
    convParam.boundaryType = lti::Constant;

    int i,kernelSize,kidx;
    kidx = 1;
    kernelSize = 2;
    pyr.at(0).copy(src);
    point kdim;

    // only the elements included in the list "resolution" are required

    std::list<int>::const_iterator it;
    it = resolutions.begin();

    if ((*it) == 0) {
      ++it;
    }

    for (i=1; i<pyr.size(); ++i) {
      if ((*it) == i) {

        // set-up kernel
        convParam.kernelSize = rectangle(-kidx,-kidx,kidx,kidx);
        kdim = convParam.kernelSize.getDimensions();
        convParam.kernelNorm = static_cast<float>(kdim.x*kdim.y);
        convolver.setParameters(convParam);

        // square-filter the image
        convolver.apply(src,pyr.at(i));

        ++it;

      }

      // update next kernel values
      kidx = kernelSize;
      kernelSize *= 2;

    }

  }

  /*
   * generate a pyramid using square filters
   */
  void featureSaliencyAx::squarePyramid1(const channel& src,
                                               pyramid<channel>& pyr) const {

    squareConvolution<float> convolver;
    squareConvolution<float>::parameters convParam;
    convParam.boundaryType = lti::Constant;

    int i,kernelSize,kidx;
    kidx = 1;
    kernelSize = 2;
    pyr.at(0).copy(src);
    point kdim;

    // only the elements included in the list "resolution" +1 are required

    std::list<int>::const_iterator it;
    it = resolutions.begin();

    for (i=1; i<pyr.size(); ++i) {
      if ((*it) == (i-1)) {

        // set-up kernel
        convParam.kernelSize = rectangle(-kidx,-kidx,kidx,kidx);
        kdim = convParam.kernelSize.getDimensions();
        convParam.kernelNorm = static_cast<float>(kdim.x*kdim.y);
        convolver.setParameters(convParam);

        // square-filter the image
        convolver.apply(src,pyr.at(i));

        ++it;

      }

      // update next kernel values
      kidx = kernelSize;
      kernelSize *= 2;

    }

  }

  void featureSaliencyAx::normalize(channel& chnl) const {

    float sum(0.0f);
    //    float sum2(0.0f);
    float theMax(0.0f);
    float maxAvrg,factor;
    int n(0);

    point p;
    point last(chnl.lastColumn(),chnl.lastRow());

    for (p.y=1;p.y<last.y;++p.y) {
      for (p.x=1;p.x<last.x;++p.x) {
        const float& l = chnl.at(p.y,p.x-1);
        const float& r = chnl.at(p.y,p.x+1);
        const float& t = chnl.at(p.y-1,p.x);
        const float& b = chnl.at(p.y+1,p.x);
        const float& c = chnl.at(p);
        if ((c >= l) &&
            (c >= r) &&
            (c >= t) &&
            (c >= b) &&
            !((l == r) && (t == b) && (l==b))) {

          sum+=c;
          //    sum2+=(c*c);

          theMax = max(c,theMax);
          ++n;
        }
      }
    }

    if (theMax != 0.0f) {
      maxAvrg = sum/n;
      factor = (1.0f-(maxAvrg/theMax));
      factor *= factor;
      factor /= theMax;

      maxAvrg = 0;
      if (chnl.getMode() == channel::Connected) {
        channel::iterator it,eit;
        eit = chnl.end();
        for (it=chnl.begin();it!=eit;++it) {
          if (*it >= maxAvrg) {
            (*it)*=factor;
          } else {
            (*it) = 0;
          }
        }
      } else {
        int y;
        vector<channel::value_type>::iterator it,eit;
        for (y=0;y<chnl.rows();++y) {
          vector<channel::value_type>& vct = chnl.getRow(y);
          for (it=vct.begin(),eit=vct.end();it!=eit;++it) {
            if (*it >= maxAvrg) {
              (*it)*=factor;
            } else {
              (*it) = 0;
            }
          }
        }
      }
    } else {
      chnl.fill(0.0f);
    }
  }

}
