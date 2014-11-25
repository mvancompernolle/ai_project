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
 * file .......: ltiFeatureSaliencyIK.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 30.10.2000
 * revisions ..: $Id: ltiFeatureSaliencyIK.cpp,v 1.11 2006/09/05 10:12:44 ltilib Exp $
 */

#include "ltiFeatureSaliencyIK.h"
#include "ltiGaussianPyramid.h"
#include "ltiGaborPyramid.h"

#ifdef _DEBUG
//#define DEBUG_FS 1

#ifdef DEBUG_FS
#  include <cstdio>
#  include "ltiViewer.h"
#endif

#endif
namespace lti {
  // --------------------------------------------------
  // featureSaliencyIK::parameters
  // --------------------------------------------------

  // default constructor
  featureSaliencyIK::parameters::parameters()
    : saliency::parameters() {

    scales = int(8);
    mapScale = int(0);
    colorThreshold = double(0.05);
    minScale = 1;
    maxScale = 3;
    minDeltaScale = 3;
    maxDeltaScale = 4;
    gaborKernelSize = 9;
    smoothingIterations = 3;
  }

  // copy constructor
  featureSaliencyIK::parameters::parameters(const parameters& other)
    : saliency::parameters()  {
    copy(other);
  }

  // destructor
  featureSaliencyIK::parameters::~parameters() {
  }

  // get type name
  const char* featureSaliencyIK::parameters::getTypeName() const {
    return "featureSaliencyIK::parameters";
  }

  // copy member

  featureSaliencyIK::parameters&
    featureSaliencyIK::parameters::copy(const parameters& other) {
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

    scales = other.scales;
    mapScale = other.mapScale;
    colorThreshold = other.colorThreshold;
    minScale = other.minScale;
    maxScale = other.maxScale;
    minDeltaScale = other.minDeltaScale;
    maxDeltaScale = other.maxDeltaScale;
    gaborKernelSize = other.gaborKernelSize;
    smoothingIterations = other.smoothingIterations;

    return *this;
  }

  // clone member
  functor::parameters* featureSaliencyIK::parameters::clone() const {
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
  bool featureSaliencyIK::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool featureSaliencyIK::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"scales",scales);
      lti::write(handler,"mapScale",mapScale);
      lti::write(handler,"colorThreshold",colorThreshold);
      lti::write(handler,"minScale",minScale);
      lti::write(handler,"maxScale",maxScale);
      lti::write(handler,"minDeltaScale",minDeltaScale);
      lti::write(handler,"maxDeltaScale",maxDeltaScale);
      lti::write(handler,"gaborKernelSize",gaborKernelSize);
      lti::write(handler,"smoothingIterations",smoothingIterations);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && saliency::parameters::write(handler,false);
# else
    bool (modifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      modifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool featureSaliencyIK::parameters::write(ioHandler& handler,
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
  bool featureSaliencyIK::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool featureSaliencyIK::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"scales",scales);
      lti::read(handler,"mapScale",mapScale);
      lti::read(handler,"colorThreshold",colorThreshold);
      lti::read(handler,"minScale",minScale);
      lti::read(handler,"maxScale",maxScale);
      lti::read(handler,"minDeltaScale",minDeltaScale);
      lti::read(handler,"maxDeltaScale",maxDeltaScale);
      lti::read(handler,"gaborKernelSize",gaborKernelSize);
      lti::read(handler,"smoothingIterations",smoothingIterations);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && saliency::parameters::read(handler,false);
# else
    bool (modifier::parameters::* p_readMS)(ioHandler&,const bool) =
      modifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool featureSaliencyIK::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // featureSaliencyIK
  // --------------------------------------------------

  // default constructor
  featureSaliencyIK::featureSaliencyIK()
    : saliency(){
    // default parameters
    parameters tmp;
    setParameters(tmp);
  }

  // copy constructor
  featureSaliencyIK::featureSaliencyIK(const featureSaliencyIK& other)
    : saliency()  {
    copy(other);
  }

  // destructor
  featureSaliencyIK::~featureSaliencyIK() {
  }

  // returns the name of this type
  const char* featureSaliencyIK::getTypeName() const {
    return "featureSaliencyIK";
  }

  // copy member
  featureSaliencyIK&
    featureSaliencyIK::copy(const featureSaliencyIK& other) {
      saliency::copy(other);
    return (*this);
  }

  // clone member
  functor* featureSaliencyIK::clone() const {
    return new featureSaliencyIK(*this);
  }

  // return parameters
  const featureSaliencyIK::parameters&
    featureSaliencyIK::getParameters() const {
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
  bool featureSaliencyIK::apply(const image& src,channel& dest) const {
    channel mapC,mapO;
    apply(src,dest,mapC,mapO);

    dest.add(mapC);
    dest.add(mapO);

    dest.divide(3.0f);

    return true;
  };


  bool featureSaliencyIK::apply(const image& src,
                                channel& conspIntensity,
                                channel& conspColor,
                                channel& conspOrientation) const {
    const parameters& param = getParameters();

    channel RG,BY,I;

    getColorChannels(src,RG,BY,I);

    const int& scales = param.scales;

    bool gaussian = true;
    int gaussSize = 3;

    gaussianPyramid<channel> pRG(scales,gaussSize,-1,gaussian);
    gaussianPyramid<channel> pBY(scales,gaussSize,-1,gaussian);
    gaussianPyramid<channel> pI(scales,gaussSize,-1,gaussian);

    pRG.generate(RG);
    pBY.generate(BY);
    pI.generate(I);

    gaborPyramid<channel> p00(scales,0,param.gaborKernelSize,0.698,gaussian);
    gaborPyramid<channel> p45(scales,45,param.gaborKernelSize,0.698,gaussian);
    gaborPyramid<channel> p90(scales,90,param.gaborKernelSize,0.698,gaussian);
    gaborPyramid<channel>
      p135(scales,135,param.gaborKernelSize,0.698,gaussian);

    p00.generate(I);
    p45.generate(I);
    p90.generate(I);
    p135.generate(I);

    conspicuityI(pI,conspIntensity);
    conspicuityC(pRG,pBY,conspColor);
    conspicuityO(p00,p45,p90,p135,conspOrientation);

    if (param.smoothingIterations > 0) {
      itNorm(param.smoothingIterations,conspIntensity);
      itNorm(param.smoothingIterations,conspColor);
      itNorm(param.smoothingIterations,conspOrientation);
    }

    return true;
  }

  /**
   * calculate the after Itti & Koch required channels:
   *
   * With
   * r', g' and b' the original color channels
   * I = (r'+g'+b')/3
   * r = r'/I
   * g = g'/I
   * b = b'/I
   *
   * the red, green, blue and yellow broadly tuned color channels
   * are defined as:
   *
   * R = r-(g+b)/2;         // negative values are set to zero!
   * G = g-(r+b)/2;         // negative values are set to zero!
   * B = b-(r+g)/2;         // negative values are set to zero!
   * Y = (r+g)/2-|r-g|/2-b; // negative values are set to zero!
   *
   */
  void featureSaliencyIK::getColorChannels(const image& img,
                                         channel& RG,
                                         channel& BY,
                                         channel& I) const {

    const parameters& param = getParameters();
    channel R,G,B,Y;

    // proper size
    R.resize(img.size(),0,false,false);
    G.resize(img.size(),0,false,false);
    B.resize(img.size(),0,false,false);
    Y.resize(img.size(),0,false,false);
    I.resize(img.size(),0,false,false);

    channel::iterator rit,git,bit,yit,iit;
    vector<rgbPixel>::const_iterator it,eit;

    int y;
    float r,g,b,pI,tmp,maxI;

    maxI = 0;
    for (y = 0,rit=R.begin(),git=G.begin(),bit=B.begin(),iit=I.begin();
         y<img.rows();
         y++) {
      const vector<rgbPixel>& vct=img.getRow(y);
      for (it=vct.begin(),eit=vct.end();
           it!=eit;
           ++it,++rit,++git,++bit,++iit) {
        (*rit) = r = static_cast<float>((*it).getRed());
        (*git) = g = static_cast<float>((*it).getGreen());
        (*bit) = b = static_cast<float>((*it).getBlue());
        (*iit) = (r+g+b);
        maxI = max(maxI,(*iit));
      }
    }

    float intThresh = maxI*float(param.colorThreshold);

    // or even not with max but with avrg/var

    for (y = 0,rit=R.begin(),git=G.begin(),bit=B.begin(),
           yit=Y.begin(),iit=I.begin();
         y<img.rows();
         y++) {
      const vector<rgbPixel>& vct=img.getRow(y);
      for (it=vct.begin(),eit=vct.end();
           it!=eit;
           ++it,++rit,++git,++bit,++yit,++iit) {

        r = (*rit);
        g = (*git);
        b = (*bit);
        pI = (*iit);
        (*iit)/=765.0f; //255*3.0f;

        if (pI<intThresh) {
          pI=1.0f;
          r=g=b=0;
        }

        // Red channel
        tmp = r - ((g+b)/2.0f);
        if (tmp>0.0f) {
          (*rit)=tmp/pI;
        } else {
          (*rit)=0.0f;
        }

        // Green channel
        tmp = g - ((r+b)/2.0f);
        if (tmp>0.0f) {
          (*git)=tmp/pI;
        } else {
          (*git)=0.0f;
        }

        // Blue channel
        tmp = b - ((r+g)/2.0f);
        if (tmp>0.0f) {
          (*bit)=tmp/pI;
        } else {
          (*bit)=0.0f;
        }

        // Yellow channel
        tmp = (((r>g)?(r-g):(g-r))/2.0f) - tmp;
        if (tmp>0.0f) {
          (*yit)=tmp/pI;
        } else {
          (*yit)=0.0f;
        }
      }
    }
    RG.subtract(R,G);
    BY.subtract(B,Y);
  };

  int featureSaliencyIK::getNumberPyramids() const {
    const parameters& param = getParameters();

    return (param.maxDeltaScale-param.minDeltaScale+1) +
      (param.maxScale-param.minScale);
  }

  int featureSaliencyIK::getIndex(const int& c,const int& s) const {
    const parameters& param = getParameters();

    return s-param.minDeltaScale-param.minScale;
  }

  void featureSaliencyIK::getLimits(const int& idx, int& from, int& to) const {
    const parameters& param = getParameters();

    // maximal number of elements per pyramid
    const int scales = param.maxScale-param.minScale+1;
    const int deltas = param.maxDeltaScale-param.minDeltaScale+1;

    const int diagMax = min(scales,deltas);

    // how many scales are used in the pyramid?
    int n;

    if (idx<deltas) {
      n = min(diagMax,idx+1);
    } else {
      n = min(diagMax,deltas+scales-1-idx);
    }

    // last scale
    if (idx<scales) {
      to = idx+param.minScale;
    } else {
      to = param.maxScale;
    }

    // first scale
    from = to-n+1;
  }

  void
  featureSaliencyIK::getUpscaledPyramids(const gaussianPyramid<channel>& src,
                          std::vector<gaussianPyramid<channel> >& usp) const {

    const parameters& param = getParameters();

    if ((param.maxScale+param.maxDeltaScale)>=param.scales) {
      throw
        exception("feature saliency error: maxScale+maxDeltaScale>=scales!");
    }

    usp.resize(0);
    usp.resize(getNumberPyramids());
    int c,s;


    c = param.minScale;
    const int from = c+param.minDeltaScale;
    const int to   = c+param.maxDeltaScale;
    for (s=from;s<=to;++s) {

#     ifdef DEBUG_FS
      printf("reconstructing (%d,%d) at usp[%d]\n",c,s,getIndex(c,s));
#     endif

      src.reconstruct(c,s,usp[getIndex(c,s)]);
    }

    for (++c;c<=param.maxScale;++c,++s) {

#     ifdef DEBUG_FS
      printf("reconstructing (%d,%d) at usp[%d]\n",c,s,getIndex(c,s));
#     endif

      src.reconstruct(c,s,usp[getIndex(c,s)]);
    }

  }

  void featureSaliencyIK::subtAndNorm(const channel& a,
                                      const channel& b,
                                      channel& result) const {
    // this is for internal use only!
    // this should always be valid!
    assert(a.size()==b.size());

    // the iterators
    channel::const_iterator ait,bit,eit;
    channel::iterator it;

    if (result.size() != a.size()) {
      result.resize(a.size(),0.0f,false,false);
    }

    float theMax,sum;

    sum=0;
    theMax=-1e30f;

    for (ait=a.begin(),bit=b.begin(),it = result.begin(),eit=a.end();
         ait!=eit;
         ++ait,++bit,++it) {
      if (*ait>*bit) {
        (*it) = ((*ait)-(*bit));
      } else {
        (*it) = ((*bit)-(*ait));
      }

      theMax = max(theMax,*it);
      sum+=(*it);
    }

    if (theMax != 0.0f) {
      float tmp;
      tmp = (theMax-(sum/(result.rows()*result.columns())))/theMax;

      result.multiply((tmp*tmp)/theMax);
    }
  }

  void featureSaliencyIK::addAndNorm(const channel& a,
                                   const channel& b,
                                   channel& result) const {
    // this is for internal use only!
    // this should always be valid!
    assert(a.size()==b.size());

    // the iterators
    channel::const_iterator ait,bit,eit;
    channel::iterator it;

    if (result.size() != a.size()) {
      result.resize(a.size(),0.0f,false,false);
    }

    float theMax,sum;

    sum=0;
    theMax=-1e+30f;

    for (ait=a.begin(),bit=b.begin(),it = result.begin(),eit=a.end();
         ait!=eit;
         ++ait,++bit,++it) {

      (*it) = ((*ait)+(*bit));
      if ((*it)<0) {
        (*it) = -(*it);
      }

      theMax = max(theMax,*it);
      sum+=(*it);
    }

    if (theMax != 0.0f) {
      float tmp;
      tmp = (theMax-(sum/(result.rows()*result.columns())))/theMax;
      result.multiply((tmp*tmp)/theMax);
    }
  }

  void featureSaliencyIK::normAndAcc(const channel& c,
                                   channel& acc) const {

    // this is for internal use only!
    // this should always be valid!
    assert(c.size()==acc.size());

    // the iterators
    channel::const_iterator cit,eit;
    float theMax,sum;

    sum=0;
    theMax=-1e+30f;

    for (cit=c.begin(),eit=c.end();
         cit!=eit;
         ++cit) {

      theMax = max(theMax,*cit);
      sum+=(*cit);
    }

    if (theMax != 0) {
      float tmp;
      tmp = (theMax-(sum/(acc.rows()*acc.columns())))/theMax;
      acc.addScaled((tmp*tmp)/theMax,c);
    }
  }

  void featureSaliencyIK::normalize(channel& chnl) const {
    // the iterators
    channel::const_iterator cit,eit;
    float theMax,sum;

    sum=0;
    theMax=-1e+30f;

    for (cit=chnl.begin(),eit=chnl.end();
         cit!=eit;
         ++cit) {

      theMax = max(theMax,*cit);
      sum+=(*cit);
    }

    if (theMax != 0) {
      float tmp;
      tmp = (theMax-(sum/(chnl.rows()*chnl.columns())))/theMax;

      chnl.multiply((tmp*tmp)/theMax);
    }
  }


  void featureSaliencyIK::conspicuityI(const gaussianPyramid<channel>& pyr,
                                     channel& mapI) const {

    const parameters& param = getParameters();

    std::vector<gaussianPyramid<channel> > usp; // upscaled pyramids

    getUpscaledPyramids(pyr,usp);

    int c,s;

#   ifdef DEBUG_FS
    static viewer vadd("add");
    static viewer a("a");
    static viewer b("b");
    static viewer vc("a-b");
#   endif

    for (c=param.minScale;c<=param.maxScale;++c) {
      const int to = c+param.maxDeltaScale;
      for (s=c+param.minDeltaScale;s<=to;++s) {

#       ifdef DEBUG_FS
        printf("Subtracting pyr.at(%d)-usp[%d].at(%d)\n",c,getIndex(c,s),c);
        a.show(pyr.at(c));
        b.show(usp[getIndex(c,s)].at(c));
#       endif

        subtAndNorm(pyr.at(c),
                    usp[getIndex(c,s)].at(c),
                    usp[getIndex(c,s)].at(c));

#       ifdef DEBUG_FS
        vc.show(usp[getIndex(c,s)].at(c));
        getchar();
#       endif


      }
    }

    // now all channels in usp must be (interscaled) added at the given
    // mapScale
    int i;

    channel tmp;
    mapI.resize(0,0,0,false,true);
    tmp.resize(pyr.at(param.mapScale).size(),0,false,false);
    for (i=0;i<int(usp.size());++i) {
#     ifdef DEBUG_FS
      printf("Adding pyramid %d\n",i);
      vadd.show(tmp);
#     endif

      add(usp[i],i,param.mapScale,tmp);

      if (mapI.size() == point(0,0)) {
        tmp.detach(mapI);
        tmp.resize(pyr.at(param.mapScale).size(),0,false,false);
      } else {
        mapI.add(tmp);
      }

#     ifdef DEBUG_FS
      getchar();
#     endif
    }

    normalize(mapI);

  }


  void featureSaliencyIK::conspicuityC(const gaussianPyramid<channel>& pRG,
                                     const gaussianPyramid<channel>& pBY,
                                     channel& mapI) const {

    const parameters& param = getParameters();

    std::vector<gaussianPyramid<channel> > uspRG; // upscaled pyramids
    std::vector<gaussianPyramid<channel> > uspBY; // upscaled pyramids

    getUpscaledPyramids(pRG,uspRG);
    getUpscaledPyramids(pBY,uspBY);

    int c,s;

    for (c=param.minScale;c<=param.maxScale;++c) {
      const int to = c+param.maxDeltaScale;
      for (s=c+param.minDeltaScale;s<=to;++s) {
        addAndNorm(pRG.at(c),
                   uspRG[getIndex(c,s)].at(c),
                   uspRG[getIndex(c,s)].at(c));
        addAndNorm(pBY.at(c),
                   uspBY[getIndex(c,s)].at(c),
                   uspBY[getIndex(c,s)].at(c));
      }
    }

    // now all channels in usp must be (interscaled) added at the given
    // mapScale
    int i;

    channel tmpRG,tmpBY;
    mapI.resize(0,0,0,false,true);
    tmpRG.resize(pRG.at(param.mapScale).size(),0,false,false);
    tmpBY.resize(pBY.at(param.mapScale).size(),0,false,false);
    for (i=0;i<int(uspRG.size());++i) {
      add(uspRG[i],i,param.mapScale,tmpRG);
      add(uspBY[i],i,param.mapScale,tmpBY);

      if (mapI.size() == point(0,0)) {
        tmpRG.detach(mapI);
        tmpRG.resize(pRG.at(param.mapScale).size(),0,false,false);
        mapI.add(tmpBY);
      } else {
        mapI.add(tmpRG);
        mapI.add(tmpBY);
      }
    }

    normalize(mapI);
  }


    /**
     * orientation conspicuity
     */
    void featureSaliencyIK::conspicuityO(const gaussianPyramid<channel>& p00,
                                       const gaussianPyramid<channel>& p45,
                                       const gaussianPyramid<channel>& p90,
                                       const gaussianPyramid<channel>& p135,
                                       channel& mapI) const {

    const parameters& param = getParameters();

    std::vector<gaussianPyramid<channel> > usp00;  // upscaled pyramids
    std::vector<gaussianPyramid<channel> > usp45;  // upscaled pyramids
    std::vector<gaussianPyramid<channel> > usp90;  // upscaled pyramids
    std::vector<gaussianPyramid<channel> > usp135; // upscaled pyramids

    getUpscaledPyramids(p00,usp00);
    getUpscaledPyramids(p45,usp45);
    getUpscaledPyramids(p90,usp90);
    getUpscaledPyramids(p135,usp135);

    int c,s;

    for (c=param.minScale;c<=param.maxScale;++c) {
      const int to = c+param.maxDeltaScale;
      for (s=c+param.minDeltaScale;s<=to;++s) {
        subtAndNorm(p00.at(c),
                   usp00[getIndex(c,s)].at(c),
                   usp00[getIndex(c,s)].at(c));

        subtAndNorm(p45.at(c),
                   usp45[getIndex(c,s)].at(c),
                   usp45[getIndex(c,s)].at(c));

        subtAndNorm(p90.at(c),
                   usp90[getIndex(c,s)].at(c),
                   usp90[getIndex(c,s)].at(c));

        subtAndNorm(p135.at(c),
                   usp135[getIndex(c,s)].at(c),
                   usp135[getIndex(c,s)].at(c));
      }
    }

    // now all channels in usp must be (interscaled) added at the given
    // mapScale
    int i;

    channel tmp00,tmp45,tmp90,tmp135;

    tmp00.resize(p00.at(param.mapScale).size(),0,false,false);
    tmp45.resize(p45.at(param.mapScale).size(),0,false,false);
    tmp90.resize(p90.at(param.mapScale).size(),0,false,false);
    tmp135.resize(p135.at(param.mapScale).size(),0,false,false);

    mapI.resize(tmp00.size(),0,false,true);

    for (i=0;i<int(usp00.size());++i) {
      add(usp00[i],i,param.mapScale,tmp00);
      add(usp45[i],i,param.mapScale,tmp45);
      add(usp90[i],i,param.mapScale,tmp90);
      add(usp135[i],i,param.mapScale,tmp135);

      normAndAcc(tmp00,mapI);
      normAndAcc(tmp45,mapI);
      normAndAcc(tmp90,mapI);
      normAndAcc(tmp135,mapI);
    }

    normalize(mapI);
  }


  void featureSaliencyIK::add(const gaussianPyramid<channel>& pyr,
                              const int& pyrIndex,
                              const int& scale,
                              channel& result) const {

    int from,to;
    int i;

    getLimits(pyrIndex,from,to);

#   ifdef DEBUG_FS
    printf("Adding pyramid(%d) elements from %d to %d\n",pyrIndex,from,to);
    static viewer view("adding...");
    static viewer vacc("acc");

#   endif


    if ((pyr.at(scale).size() == point(0,0)) ||
        (scale<from) || (scale>to)) {
#     ifdef DEBUG_FS
      printf("clear result\n");
#     endif

      result.fill(0.0f);
    } else {
      result.fill(pyr.at(scale));
#     ifdef DEBUG_FS
      printf("result filled with scale %d from pyramid %d!\n",scale,pyrIndex);
      view.show(result);
      getchar();
#     endif
    }

#   ifdef DEBUG_FS
    vacc.show(result);
#   endif


    // downsampling section!

    if (scale>from) {
      downsampling downsampler;
      downsampling::parameters dParam;

      gaussKernel2D<float> kern(3); // default gaussian kernel
      dParam.setKernel(kern);
      dParam.boundaryType = lti::Constant;
      dParam.factor = point(2,2);

      downsampler.setParameters(dParam);

      // following loop assumes that there are no empty elements between the
      // "borders" of the pyramid.
      channel acc;

      for (i=from;i<scale;++i) {
        if (pyr.at(i).size() != point(0,0)) {

#         ifdef DEBUG_FS
          printf("downsampling element %d\n",i);
          view.show(pyr.at(i));
#         endif

          if (acc.size() == point(0,0)) {
            downsampler.apply(pyr.at(i),acc);
          } else {
            acc.add(pyr.at(i));
            downsampler.apply(acc);
          }
        } else {
          if (acc.size() != point(0,0)) {
            downsampler.apply(acc);
          }
        }

#       ifdef DEBUG_FS
        vacc.show(acc);
        getchar();
#       endif

      }

      if (acc.size() != point(0,0)) {
        result.add(acc);

#       ifdef DEBUG_FS
        printf("adding element %d\n",i);
        vacc.show(result);
        getchar();
#       endif
      }
    }

    // upsampling section!

    if (scale<to) {

      // TODO: replace upsampling with reconstruct() of pyramid, to make
      //       use of the gaussian/square upsampling stuff...

      upsampling upsampler;
      upsampling::parameters uParam;

      kernel1D<float> kern1(-1,1,0.5f);
      kern1.at(0)=1.0f;

      sepKernel<float> skern(kern1);

      uParam.setKernel(skern);
      uParam.boundaryType = lti::Constant;
      uParam.factor = point(2,2);

      upsampler.setParameters(uParam);

      channel acc;

      for (i=to;i>scale;--i) {
        if (pyr.at(i).size() != point(0,0)) {

#         ifdef DEBUG_FS
          printf("upsampling element %d\n",i);
          view.show(pyr.at(i));
#         endif

          if (acc.size() == point(0,0)) {
            upsampler.apply(pyr.at(i),acc);
          } else {
            if (acc.size() != pyr.at(i).size()) {
              acc.resize(pyr.at(i).size(),0,true,false);
            }
            acc.add(pyr.at(i));
            upsampler.apply(acc);
          }
        } else {
          if (acc.size() != point(0,0)) {
            upsampler.apply(acc);
          }
        }

#       ifdef DEBUG_FS
        vacc.show(acc);
        getchar();
#       endif
      }

      // fix size after upsampling
      if (acc.size() != result.size()) {
        acc.resize(result.size(),0,true,false);
      }
      result.add(acc);
    }
  }

  void featureSaliencyIK::itNorm(const int& steps,channel& chnl) const {

    int dim = min(chnl.rows(),chnl.columns());

    gaussKernel1D<float> g1(static_cast<int>(0.1*dim),(0.25*dim)*(0.25*dim));
    gaussKernel1D<float> g2(static_cast<int>(0.1*dim),(0.02*dim)*(0.02*dim));

    g2.multiply(3.0f);

    g1.subtract(g2);
    g1.divide(2.0f);

    sepKernel<float> kern(g1);

    convolution conv;
    convolution::parameters cpar;

    cpar.boundaryType = lti::Constant;
    cpar.setKernel(kern);

    conv.setParameters(cpar);

    channel::iterator it,eit;
    channel tmp;

    for (int i = 0;i<steps;i++) {

      float maximum = chnl.maximum();

      conv.apply(chnl,tmp);
      tmp.add(chnl);
      tmp.divide(maximum);
      tmp.add(-0.02f);

      for (it=tmp.begin(),eit=tmp.end();
           it!=eit;
           ++it) {
        if ((*it) < 0) {
          (*it) = 0;
        }
      }

      tmp.detach(chnl);
    }

  }

}
