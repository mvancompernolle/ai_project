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
 * file .......: ltiQmfEnergy.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 17.5.2001
 * revisions ..: $Id: ltiQmfEnergy.cpp,v 1.13 2006/09/05 10:27:03 ltilib Exp $
 */


#include "ltiQmfEnergy.h"
#include "ltiSplitImageToRGB.h"
#include "ltiConvolution.h"
#include "ltiGaussKernels.h"
#include "ltiUpsampling.h"
#include "ltiDownsampling.h"

//#define _LTI_DEBUG

#ifdef _LTI_DEBUG
#include <iostream>
#include <cstdio>
#include "ltiViewer.h"

using std::cout;
using std::endl;

#endif // _LTI_DEBUG

namespace lti {

#ifdef _LTI_DEBUG
  static lti::viewer theViewer("QmfEnergy");
#endif // ifdef _LTI_DEBUG

  // --------------------------------------------------
  // qmfEnergy::parameters
  // --------------------------------------------------

  // default constructor
  qmfEnergy::parameters::parameters()
    : globalFeatureExtractor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    squareRootEnergy = bool(true);
    upsample = bool(false);
    powerFilterSize = int(3);
    powerFilterVariance = double(-1);
    fusionFilterVariance = double(3);
    fusionFilterSize = int(-1);

    histogramParam.cells = 32;
    histogramParam.smooth = false;
    histogramParam.mode = histograming1D::parameters::Interval;
    histogramParam.intervalLow = 0;
    histogramParam.intervalHigh = 0.3f;
    histogramParam.ignoreValue = 0;
    histogramParam.normalize = true;
  }

  // copy constructor
  qmfEnergy::parameters::parameters(const parameters& other)
    : globalFeatureExtractor::parameters()  {
    copy(other);
  }

  // destructor
  qmfEnergy::parameters::~parameters() {
  }

  // get type name
  const char* qmfEnergy::parameters::getTypeName() const {
    return "qmfEnergy::parameters";
  }

  // copy member

  qmfEnergy::parameters&
    qmfEnergy::parameters::copy(const parameters& other) {
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


    squareRootEnergy = other.squareRootEnergy;
    upsample = other.upsample;
    powerFilterSize = other.powerFilterSize;
    powerFilterVariance = other.powerFilterVariance;
    fusionFilterVariance = other.fusionFilterVariance;
    fusionFilterSize = other.fusionFilterSize;
    qmfParam.copy(other.qmfParam);
    histogramParam.copy(other.histogramParam);

    return *this;
  }

  // alias for copy member
  qmfEnergy::parameters&
    qmfEnergy::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* qmfEnergy::parameters::clone() const {
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
  bool qmfEnergy::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool qmfEnergy::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"squareRootEnergy",squareRootEnergy);
      lti::write(handler,"upsample",upsample);
      lti::write(handler,"powerFilterSize",powerFilterSize);
      lti::write(handler,"powerFilterVariance",powerFilterVariance);
      lti::write(handler,"fusionFilterVariance",fusionFilterVariance);
      lti::write(handler,"fusionFilterSize",fusionFilterSize);
      lti::write(handler,"qmfParam",qmfParam);
      lti::write(handler,"histogramParam",histogramParam);
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
  bool qmfEnergy::parameters::write(ioHandler& handler,
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
  bool qmfEnergy::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool qmfEnergy::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"squareRootEnergy",squareRootEnergy);
      lti::read(handler,"upsample",upsample);
      lti::read(handler,"powerFilterSize",powerFilterSize);
      lti::read(handler,"powerFilterVariance",powerFilterVariance);
      lti::read(handler,"fusionFilterVariance",fusionFilterVariance);
      lti::read(handler,"fusionFilterSize",fusionFilterSize);
      lti::read(handler,"qmfParam",qmfParam);
      lti::read(handler,"histogramParam",histogramParam);
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
  bool qmfEnergy::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // qmfEnergy
  // --------------------------------------------------

  // default constructor
  qmfEnergy::qmfEnergy()
    : globalFeatureExtractor(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  qmfEnergy::qmfEnergy(const parameters& par)
    : globalFeatureExtractor(){
    // set the default parameters
    setParameters(par);
  }

  // copy constructor
  qmfEnergy::qmfEnergy(const qmfEnergy& other)
    : globalFeatureExtractor()  {
    copy(other);
  }

  // destructor
  qmfEnergy::~qmfEnergy() {
  }

  // returns the name of this type
  const char* qmfEnergy::getTypeName() const {
    return "qmfEnergy";
  }

  // copy member
  qmfEnergy&
    qmfEnergy::copy(const qmfEnergy& other) {
      globalFeatureExtractor::copy(other);
    return (*this);
  }

  // clone member
  functor* qmfEnergy::clone() const {
    return new qmfEnergy(*this);
  }

  // return parameters
  const qmfEnergy::parameters&
    qmfEnergy::getParameters() const {
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


  // On place apply for type image!
  bool qmfEnergy::apply(const image& src,dvector& feature) const {

    if (src.empty()) {
      feature.clear();
      setStatusString("source image empty");
      return false;
    }

    splitImageToRGB splitter;
    channel r,g,b; // the color channels
    dvector hr,hg,hb; // the histograms

    splitter.apply(src,r,g,b);

    apply(r,hr);
    apply(g,hg);
    apply(b,hb);

    feature.resize(3*hr.size(),0.0,false,false);
    feature.fill(hr);
    feature.fill(hg,hr.size());
    feature.fill(hb,2*hr.size());

    return true;
  };

  // On place apply for type channel!
  bool qmfEnergy::apply(channel& srcdest) const {
    bool result;
    channel tmp;

    result = apply(srcdest,tmp);
    tmp.detach(srcdest);

    return result;
  };

  // On copy apply for type image!
  bool qmfEnergy::apply(const channel& src,channel& dest) const {

    if (src.empty()) {
      dest.clear();
      setStatusString("input channel empty");
      return false;
    }

    const parameters& param = getParameters();

    if (param.qmfParam.levels < 2) {
      setStatusString("Too few levels, set at least 2");
      dest.clear();
      return false;
    }

    std::vector<channel> chnls;
    int i,idx(0);
    channel paddedsrc;

    // be sure that the channel has the right size!
    padChannel(param.qmfParam.levels,src,paddedsrc);

#ifdef _LTI_DEBUG
    theViewer.show(paddedsrc);
    getchar();
#endif // ifdef _LTI_DEBUG

    // wavelet transform and each subband in its own channel and
    // energy computation
    getChannels(paddedsrc,chnls);

    // convolution functor to integrate energy:
    convolution convolver;
    convolution::parameters convParam;
    convParam.boundaryType = lti::Constant;

    // should integrate?
    if (param.powerFilterSize>1) {
      // gaussian kernel for the power (local energy integrator)
      gaussKernel2D<channel::value_type> gk(param.powerFilterSize,
                                            param.powerFilterVariance);

      convParam.setKernel(gk);
      convolver.setParameters(convParam);

      for (i = 0;i<static_cast<int>(chnls.size());++i) {
        convolver.apply(chnls[i]); // low-pass filter all channels!
      }
    }

    // channel fusion: upsampling or downsampling?

    if (param.upsample) {
      upsampling upsampler;
      bool firstTime = true;
      for (i=(param.qmfParam.levels-2);i>=0;--i) {
        idx = i*3;
        chnls[idx+1].add(chnls[idx+2]);
        chnls[idx].add(chnls[idx+1]);
        if (!firstTime) {
          chnls[idx].add(chnls[idx+3]);
          firstTime = false;
        }

        upsampler.apply(chnls[idx]);
      }

      chnls[idx].detach(dest);

    } else {
      downsampling downsampler;
      for (i=0;i<(param.qmfParam.levels-1);++i) {
        idx = i*3;
        chnls[idx+1].add(chnls[idx+2]);
        chnls[idx].add(chnls[idx+1]);
        if (i>0) {
          chnls[idx].add(chnls[idx-3]);
        }

        downsampler.apply(chnls[idx]);
      }
      chnls[idx].detach(dest);
    }

    // should smooth energy channel?
    if (param.fusionFilterSize>1) {
      // gaussian kernel for the power (local energy integrator)
      gaussKernel2D<channel::value_type> gk(param.fusionFilterSize,
                                            param.fusionFilterVariance);

      convParam.setKernel(gk);
      convolver.setParameters(convParam);

      convolver.apply(dest);
    }

    // if non-linearity required:
    if (param.squareRootEnergy) {
      dest.apply(sqrt);
    }

    return true;
  };


  void qmfEnergy::getChannels(const channel& chnl,
                              std::vector<channel>& chnls) const {


    const parameters& param = getParameters();
    const int levels = param.qmfParam.levels-1;

    if (levels <= 0) {
      chnls.clear();
      return;
    }

    int i;

    channel wl;
    qmf filterBank;

    filterBank.setParameters(param.qmfParam);
    filterBank.apply(chnl,wl);

    // Calculate the power as the square of each element
    wl.apply(sqr); // power (after integration: energy)

#ifdef _LTI_DEBUG
    theViewer.show(wl);
    getchar();
#endif // ifdef _LTI_DEBUG

    // there are 3 channels per level, and the low-pass level
    // remains ignored!
    chnls.resize(3*levels);
    const vector<point>& lim = filterBank.getLastPartitioning();
    point first,last,size;

    for (i=0;i<levels;++i) {
      first = lim.at(i+1);
      last = lim.at(i);
      size.x = max(first.x+1,last.x-first.x);
      size.y = max(first.y+1,last.y-first.y);
      const int idx = i*3;
      int y,lastIdx,newIdx;

      channel& chnl1 = chnls[idx];
      channel& chnl2 = chnls[idx+1];
      channel& chnl3 = chnls[idx+2];

      // make all channels in a subband have the same size!
      chnl1.resize(size,0.0f,false,false);
      chnl2.resize(size,0.0f,false,false);
      chnl3.resize(size,0.0f,false,false);

      // fill the channels with the respective subband
      chnl1.fill(wl,0,0,size.y,size.x,0,first.x+1);
      chnl2.fill(wl,0,0,size.y,size.x,first.y+1,first.x+1);
      chnl3.fill(wl,0,0,size.y,size.x,first.y+1,0);

      // fix the borders if their sizes are not correct
      if ((first.x+1) > (last.x-first.x)) {
        // bands 1 and 2 need to be fixed
        newIdx = last.x-first.x;
        lastIdx = newIdx-1;
        for (y=0;y<size.y;++y) {
          chnl1.at(y,newIdx) = chnl1.at(y,lastIdx);
          chnl2.at(y,newIdx) = chnl2.at(y,lastIdx);
        }
      } else if ((first.x+1) < (last.x-first.x)) {
        // band 3 needs to be fixed
        newIdx = first.x+2;
        lastIdx = newIdx-1;
        for (y=0;y<size.y;++y) {
          chnl3.at(y,newIdx) = chnl3.at(y,lastIdx);
        }
      }

      if ((first.y+1) > (last.y-first.y)) {
        // bands 2 and 3 need to be fixed
        newIdx = last.y-first.y;
        lastIdx = newIdx-1;
        chnl2.getRow(newIdx).copy(chnl2.getRow(lastIdx));
        chnl3.getRow(newIdx).copy(chnl3.getRow(lastIdx));
      } else if ((first.y+1) < (last.y-first.y)) {
        // bands 1
        newIdx = first.y+2;
        lastIdx = newIdx-1;
        chnl1.getRow(newIdx).copy(chnl2.getRow(lastIdx));
      }
    }
  }

  void qmfEnergy::padChannel(const int& levels,
                             const channel& chnl,
                             channel& padchnl) const {
    int mult,x,y,last;
    mult = (1 << levels);

    point newsize,start,end;

    newsize.x = ((chnl.columns()+mult-1)/mult)*mult;
    newsize.y = ((chnl.rows()+mult-1)/mult)*mult;

    if (newsize == chnl.size()) {
      // correct size: nothing needs to be done
      padchnl.copy(chnl);
      return;
    }

    start.x = (newsize.x-chnl.columns())/2;
    start.y = (newsize.y-chnl.rows())/2;
    end.y = start.y+chnl.rows();
    end.x = start.x+chnl.columns();

    padchnl.resize(newsize,0.0f,false,false);
    padchnl.fill(chnl,start,end);

    // first generate the left and right borders
    if (newsize.x != chnl.columns()) {
      for (y=start.y;y<end.y;++y) {
        for (x=0;x<start.x;++x) {
          padchnl.at(y,x)=padchnl.at(y,start.x);
        }
        last = end.x-1;
        for (x=end.x;x<newsize.x;++x) {
          padchnl.at(y,x)=padchnl.at(y,last);
        }
      }
    }

    // now do the top
    for (y=0;y<start.y;++y) {
      padchnl.getRow(y).copy(padchnl.getRow(start.y));
    }

    last = end.y-1;
    // and the buttom...
    for (y=end.y;y<newsize.y;++y) {
      padchnl.getRow(y).copy(padchnl.getRow(last));
    }

  }

  // On copy apply for type channel!
  bool qmfEnergy::apply(const channel& src,dvector& dest) const {

    channel tmp;
    if (apply(src,tmp)) {
      const parameters& param = getParameters();
      histograming1D histo;
      histo.setParameters(param.histogramParam);
      return histo.apply(tmp,dest);
    }

    return false;
  };

}
