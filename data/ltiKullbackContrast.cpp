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
 * file .......: ltiKullbackContrast.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 16.3.2001
 * revisions ..: $Id: ltiKullbackContrast.cpp,v 1.10 2006/09/05 10:19:30 ltilib Exp $
 */

#include "ltiKullbackContrast.h"
#include "ltiUpsampling.h"
#include "ltiConvolution.h"
#include "ltiGaussianPyramid.h"
#include "ltiSquareConvolution.h"
#include "ltiMath.h"

//  #include "ltiViewer.h"
//  #include <cstdio>

namespace lti {
  // --------------------------------------------------
  // kullbackContrast::parameters
  // --------------------------------------------------

  // default constructor
  kullbackContrast::parameters::parameters()
    : statisticsFunctor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    resolutions = int(7);
    useGaussian = true;
    gaussianSize = int(5);
    gaussianVariance = 1.0;
    useHistogram = false;
    histogramSize = 256;
  }

  // copy constructor
  kullbackContrast::parameters::parameters(const parameters& other)
    : statisticsFunctor::parameters()  {
    copy(other);
  }

  // destructor
  kullbackContrast::parameters::~parameters() {
  }

  // get type name
  const char* kullbackContrast::parameters::getTypeName() const {
    return "kullbackContrast::parameters";
  }

  // copy member

  kullbackContrast::parameters&
    kullbackContrast::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    statisticsFunctor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    statisticsFunctor::parameters& (statisticsFunctor::parameters::* p_copy)
      (const statisticsFunctor::parameters&) =
      statisticsFunctor::parameters::copy;
    (this->*p_copy)(other);
# endif


    resolutions = other.resolutions;
    useGaussian = other.useGaussian;
    gaussianSize = other.gaussianSize;
    gaussianVariance = other.gaussianVariance;
    useHistogram = other.useHistogram;
    histogramSize = other.histogramSize;

    return *this;
  }

  // alias for copy member
  kullbackContrast::parameters&
    kullbackContrast::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* kullbackContrast::parameters::clone() const {
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
  bool kullbackContrast::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool kullbackContrast::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"resolutions",resolutions);
      lti::write(handler,"useGaussian",useGaussian);
      lti::write(handler,"gaussianSize",gaussianSize);
      lti::write(handler,"gaussianVariance",gaussianVariance);
      lti::write(handler,"useHistogram",useHistogram);
      lti::write(handler,"histogramSize",histogramSize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && statisticsFunctor::parameters::write(handler,false);
# else
    bool (statisticsFunctor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      statisticsFunctor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool kullbackContrast::parameters::write(ioHandler& handler,
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
  bool kullbackContrast::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool kullbackContrast::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"resolutions",resolutions);
      lti::read(handler,"useGaussian",useGaussian);
      lti::read(handler,"gaussianSize",gaussianSize);
      lti::read(handler,"gaussianVariance",gaussianVariance);
      lti::read(handler,"useHistogram",useHistogram);
      lti::read(handler,"histogramSize",histogramSize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && statisticsFunctor::parameters::read(handler,false);
# else
    bool (statisticsFunctor::parameters::* p_readMS)(ioHandler&,const bool) =
      statisticsFunctor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool kullbackContrast::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // kullbackContrast
  // --------------------------------------------------

  // default constructor
  kullbackContrast::kullbackContrast()
    : statisticsFunctor() {
    parameters defaultParam;
    setParameters(defaultParam);
  }

  // copy constructor
  kullbackContrast::kullbackContrast(const kullbackContrast& other)
    : statisticsFunctor()  {
    copy(other);
  }

  // destructor
  kullbackContrast::~kullbackContrast() {
  }

  // returns the name of this type
  const char* kullbackContrast::getTypeName() const {
    return "kullbackContrast";
  }

  // copy member
  kullbackContrast&
    kullbackContrast::copy(const kullbackContrast& other) {
      statisticsFunctor::copy(other);
    return (*this);
  }

  // clone member
  functor* kullbackContrast::clone() const {
    return new kullbackContrast(*this);
  }

  // return parameters
  const kullbackContrast::parameters&
    kullbackContrast::getParameters() const {
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

  // On copy apply for type channel!
  vector<float>& kullbackContrast::apply(const channel& src,
                                               vector<float>& dest) const {
    gaussianPyramid<channel> pyram,pyram2;
    return apply(src,dest,pyram,pyram2);
  }

  // On copy apply for type channel!
  vector<float>&
  kullbackContrast::apply(const channel& src,
                          vector<float>& dest,
                          gaussianPyramid<channel>& pyr,
                          gaussianPyramid<channel>& pyr2) const {

    const parameters& param = getParameters();

    if (param.useGaussian) {
      // use gaussian Filter

      pyr.resize(param.resolutions+1,false);
      pyr.setKernelParameters(param.gaussianSize,
                              param.gaussianVariance);

      pyr2.resize(param.resolutions,false);
      pyr2.setKernelParameters(param.gaussianSize,
                               param.gaussianVariance);


      pyr.generate(src);
      dest.resize(pyr.size()-1);

      upsampling upsampler;
      upsampling::parameters upPar;
      upPar.boundaryType = lti::Constant;
      upsampler.setParameters(upPar);

      // generate probability distributions

      if (pyr.size() > 0) {
        int i,j;

        for (i=1;i<pyr.size();++i) {
          j = i-1;

          upsampler.apply(pyr.at(i),pyr2.at(j));
          if (pyr2.at(j).size() != pyr.at(j).size()) {
            pyr2.at(j).resize(pyr.at(j).size(),0.0f,true,false);
          }

          apply(pyr.at(j),pyr2.at(j),dest.at(j));
        }

      } else {
        dest.clear();
      }
    } else {
      // use square filter
      pyr.resize(param.resolutions+1,false);
      pyr2.resize(param.resolutions,false);

      dest.resize(pyr2.size());

      squareConvolution<float> convolver;
      squareConvolution<float>::parameters convParam;
      convParam.boundaryType = lti::Mirror;

      int i,j,kernelSize,kidx;
      kidx = 1;
      kernelSize = 2;
      pyr.at(0).copy(src);
      point kdim;

      for (i=1; i<pyr.size(); ++i) {
        j = i-1;

        // set-up kernel
        convParam.kernelSize = rectangle(-kidx,-kidx,kidx,kidx);
        kdim = convParam.kernelSize.getDimensions();
        convParam.kernelNorm = static_cast<float>(kdim.x*kdim.y);
        convolver.setParameters(convParam);

        // square-filter the image
        convolver.apply(pyr.at(0),pyr.at(i));
        pyr2.at(j).useExternData(pyr.at(i).rows(),
                                 pyr.at(i).columns(),
                                 &pyr.at(i).at(0,0));

        apply(pyr.at(j),pyr2.at(j),dest.at(j));

        // update next kernel values
        kidx = kernelSize;
        kernelSize *= 2;
      }

    }

    return dest;
  };

  // On copy apply for type vector<float>!
  float& kullbackContrast::apply(const vector<float>& q,
                                 const vector<float>& p,
                                 float& dest) const {

    vector<float>::const_iterator itp,itq,ite;
    float acc = 0;
    float pacc,qacc;
    int n;

    for (itp=p.begin(),itq=q.begin(),ite=p.end(),n=0,pacc=qacc=0.0f;
         itp!=ite;
         ++itp,++itq) {
      if (*itq > 0.0f) {
        n++;
        pacc += *itp;
        qacc += *itq;
        acc += (*itp)*log((*itp)/(*itq));
      }
    }
    dest = (acc/pacc) - log(pacc/qacc);

    return dest;
  };

  float& kullbackContrast::apply(const channel& q,
                                 const channel& p,
                                       float& dest) const {



    if (p.size() != q.size()) {
      return dest = -1.0f; // report error!
    }

    const parameters& param = getParameters();

    if (param.useHistogram) {
      // Histogram

      int hsize = param.histogramSize;

      if (hsize > q.rows()*q.columns()) {
        hsize = q.rows()*q.columns();
      }

      dvector histq(hsize,0.0),histp(hsize,0.0);

      makeHistogram(q,histq);
      makeHistogram(p,histp);

      int i;
      dest = 0;
      float probp,probq;
      for (i=0;i<hsize;++i) {
        probp = static_cast<float>(histp.at(i));
        probq = static_cast<float>(histq.at(i));
        if ((probp>0) && (probq>0)) {
          dest += probp*log(probp/probq);
        }
      }

    } else {

      // no histogram
      if ((p.getMode() == channel::Connected) &&
          (q.getMode() == channel::Connected)) {

        channel::const_iterator itp,itq,ite;
        float acc = 0;
        float pacc,qacc;
        int n;

        for (itp=p.begin(),itq=q.begin(),ite=p.end(),n=0,pacc=qacc=0.0f;
             itp!=ite;
             ++itp,++itq) {
          if (*itq > 0.0f) {
            n++;
            pacc += *itp;
            qacc += *itq;
            acc += (*itp)*log((*itp)/(*itq));
          }
        }
        dest = (acc/pacc) - log(pacc/qacc);
      } else {
        int y;
        float acc = 0;

        float pacc,qacc;
        int n;

        n= 0;
        pacc = qacc = 0.0f;

        for (y=0;y<p.rows();++y) {
          vector<channel::value_type>::const_iterator itp,itq,ite;
          for (itp=p.getRow(y).begin(),
                 itq=q.getRow(y).begin(),
                 ite=p.getRow(y).end();
               itp!=ite;
               ++itp,++itq) {
            if (*itq > 0.0f) {
              n++;
              pacc += *itp;
              qacc += *itq;
              acc += (*itp)*log((*itp)/(*itq));
            }
          }
        }
        dest = (acc/pacc) - log(pacc/qacc);
      }
    }

    return dest;
  }

  void kullbackContrast::makeHistogram(const channel& src,
                                       vector<double>& hist) const {

    const int hsize = hist.size();
    const int lastIdx = hsize-1;
    const int n = src.rows()*src.columns();
    hist.fill(0.0); // initialize with 0

    if ((src.getMode() == channel::Connected)) {

      channel::const_iterator it,ite;
      int idx;

      for (it=src.begin(),ite=src.end();
             it!=ite;
             ++it) {
        idx = static_cast<int>((*it)*hsize);
        if (idx<=0) {
          hist.at(idx)++;
        } else if (idx>=hsize) {
          hist.at(lastIdx)++;
        } else {
          hist.at(idx)++;
        }
      }

    } else {

      int y,idx;

      for (y=0;y<src.rows();++y) {
        vector<channel::value_type>::const_iterator it,ite;
        for (it=src.getRow(y).begin(),
               ite=src.getRow(y).end();
             it!=ite;
             ++it) {
          idx = static_cast<int>((*it)*hsize);
          if (idx<=0) {
            hist.at(idx)++;
          } else if (idx>=hsize) {
            hist.at(lastIdx)++;
          } else {
            hist.at(idx)++;
          }
        }
      }
    }

    hist.divide(float(n));
  }

}
