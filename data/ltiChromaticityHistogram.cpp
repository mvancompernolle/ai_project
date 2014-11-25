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
 * file .......: ltiChromaticityHistogram.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 12.10.2000
 * revisions ..: $Id: ltiChromaticityHistogram.cpp,v 1.13 2006/09/05 10:04:52 ltilib Exp $
 */

#include "ltiChromaticityHistogram.h"
#include "ltiSplitImageToRGB.h"
#include "ltiHistogram.h"
#include "ltiSquareConvolution.h"
#include "ltiConvolution.h"
#include "ltiGaussKernels.h"

namespace lti {
  // --------------------------------------------------
  // chromaticityHistogram::parameters
  // --------------------------------------------------

  // default constructor
  chromaticityHistogram::parameters::parameters()
    : globalFeatureExtractor::parameters() {

    verticalNeighbourhood = 0.5f;
    horizontalNeighbourhood = 0.5f;
    norm = eNormType(L2);
    smoothingKernelSize = int(3);
    greenCells = int(12);
    redCells = int(12);
    backgroundColor.set(0,0,0,0);
  }

  // copy constructor
  chromaticityHistogram::parameters::parameters(const parameters& other)
    : globalFeatureExtractor::parameters()  {
    copy(other);
  }

  // destructor
  chromaticityHistogram::parameters::~parameters() {
  }

  // get type name
  const char* chromaticityHistogram::parameters::getTypeName() const {
    return "chromaticityHistogram::parameters";
  }

  // copy member

  chromaticityHistogram::parameters&
    chromaticityHistogram::parameters::copy(const parameters& other) {
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

    verticalNeighbourhood = other.verticalNeighbourhood;
    horizontalNeighbourhood = other.horizontalNeighbourhood;
    norm = other.norm;
    smoothingKernelSize = other.smoothingKernelSize;
    greenCells = other.greenCells;
    redCells = other.redCells;
    backgroundColor = other.backgroundColor;

    return *this;
  }

  // clone member
  functor::parameters* chromaticityHistogram::parameters::clone() const {
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
  bool chromaticityHistogram::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool chromaticityHistogram::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"verticalNeighbourhood",verticalNeighbourhood);
      lti::write(handler,"horizontalNeighbourhood",horizontalNeighbourhood);
      switch (norm) {
          case NoNorm:
            lti::write(handler,"norm","NoNorm");
            break;
          case L1:
            lti::write(handler,"norm","L1");
            break;
          case L2:
            lti::write(handler,"norm","L2");
            break;
          default:
            lti::write(handler,"norm","L2");
      }

      lti::write(handler,"smoothingKernelSize",smoothingKernelSize);
      lti::write(handler,"greenCells",greenCells);
      lti::write(handler,"redCells",redCells);
      lti::write(handler,"backgroundColor",backgroundColor);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::write(handler,false);
# else
    bool (functor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      functor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool chromaticityHistogram::parameters::write(ioHandler& handler,
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
  bool chromaticityHistogram::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool chromaticityHistogram::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"verticalNeighbourhood",verticalNeighbourhood);
      lti::read(handler,"horizontalNeighbourhood",horizontalNeighbourhood);

      std::string str;
      lti::read(handler,"norm",str);

      if (str == "NoNorm") {
        norm = NoNorm;
      } else if (str == "L1") {
        norm = L1;
      } else if (str == "L2") {
        norm = L2;
      } else {
        norm = L2;
      }

      lti::read(handler,"smoothingKernelSize",smoothingKernelSize);
      lti::read(handler,"greenCells",greenCells);
      lti::read(handler,"redCells",redCells);
      lti::read(handler,"backgroundColor",backgroundColor);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::read(handler,false);
# else
    bool (functor::parameters::* p_readMS)(ioHandler&,const bool) =
      functor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool chromaticityHistogram::parameters::read(ioHandler& handler,
                                               const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // chromaticityHistogram
  // --------------------------------------------------

  // default constructor
  chromaticityHistogram::chromaticityHistogram()
    : globalFeatureExtractor(){

    parameters tmp;
    setParameters(tmp); // default parameters
  }

  // copy constructor
  chromaticityHistogram::chromaticityHistogram(const chromaticityHistogram& other)
    : globalFeatureExtractor()  {
    copy(other);
  }

  // destructor
  chromaticityHistogram::~chromaticityHistogram() {
  }

  // returns the name of this type
  const char* chromaticityHistogram::getTypeName() const {
    return "chromaticityHistogram";
  }

  // copy member
  chromaticityHistogram&
    chromaticityHistogram::copy(const chromaticityHistogram& other) {
      globalFeatureExtractor::copy(other);
    return (*this);
  }

  // clone member
  functor* chromaticityHistogram::clone() const {
    return new chromaticityHistogram(*this);
  }

  // return parameters
  const chromaticityHistogram::parameters&
    chromaticityHistogram::getParameters() const {
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


  bool chromaticityHistogram::apply(const image& src,
                                          dvector& chrHist) const {

    if (src.empty()) {
      setStatusString("image empty");
      chrHist.clear();
      return false;
    }

    const parameters& param = getParameters();

    splitImageToRGB splitter;
    matrix<int> histo;

    // chromaticity channels
    channel r,g,b;

    splitter.apply(src,r,g,b);

    // normalization of chromaticity channels:
    point winSize;
    winSize.y = max(int(0.5f+param.verticalNeighbourhood*src.rows()),1);
    winSize.x = max(int(0.5f+param.horizontalNeighbourhood*src.columns()),1);
    // ensure an even number of elements in the neighbourhood
    winSize.y = (winSize.y/2)*2 + 1;
    winSize.x = (winSize.x/2)*2 + 1;

    switch(param.norm) {
      case parameters::L1: {

        squareConvolution<float,float> conv;
        squareConvolution<float,float>::parameters convParam;
        point dw;
        dw.x = winSize.x/2;
        dw.y = winSize.y/2;

        convParam.kernelNorm = static_cast<float>(winSize.x*winSize.y);
        convParam.kernelSize = rectangle(-dw.x,-dw.y,dw.x,dw.y);
        convParam.boundaryType = lti::Mirror;

        channel rnorm,gnorm,bnorm;

        conv.setParameters(convParam);

        conv.apply(r,rnorm);
        conv.apply(g,gnorm);
        conv.apply(b,bnorm);

        r.edivide(rnorm);
        g.edivide(gnorm);
        b.edivide(bnorm);

      };
      break;
      case parameters::L2: {
        squareConvolution<float,l2accumulator<float> > conv;
        squareConvolution<float,l2accumulator<float> >::parameters convParam;
        point dw;
        dw.x = winSize.x/2;
        dw.y = winSize.y/2;

        convParam.kernelNorm = static_cast<float>(winSize.x*winSize.y);
        convParam.kernelSize = rectangle(-dw.x,-dw.y,dw.x,dw.y);
        convParam.boundaryType = lti::Mirror;

        channel rnorm,gnorm,bnorm;

        conv.setParameters(convParam);

        conv.apply(r,rnorm);
        conv.apply(g,gnorm);
        conv.apply(b,bnorm);

        r.edivide(rnorm);
        g.edivide(gnorm);
        b.edivide(bnorm);
      }
      break;
      case parameters::NoNorm: {
      }
      break;
    }

    // use blue channel as intensity channel...
    // to calculate normalized chromaticity channels:

    channel::iterator rit,git;
    channel::iterator bit,eit;

    for (bit = b.begin(),eit=b.end(),rit=r.begin(),git=g.begin();
         bit!=eit;
         bit++,rit++,git++) {
      (*bit)+=((*rit)+(*git));
      if ((*bit) > 0) {
        (*git)/=(*bit);
        (*rit)/=(*bit);
      } else {
        (*git)=0;
        (*rit)=0;
      }
    }


    // calculate 2D histogram!
    histo.resize(param.redCells,param.greenCells,0,false);

    const float redCells   = 255.0f*float(param.redCells)/256.0f;
    const float greenCells = 255.0f*float(param.greenCells)/256.0f;
    int y;
    double n = 0.0;

    vector<rgbPixel>::const_iterator it,ieit;
    rit = r.begin();
    git = g.begin();
    for (y=0;y<src.rows();y++) {
      const vector<rgbPixel>& vct = src.getRow(y);
      for (it = vct.begin(),ieit=vct.end();it!=ieit;it++,rit++,git++) {
        if ((*it) != param.backgroundColor) {
          histo.at(int(redCells*(*rit)),int(greenCells*(*git)))++;
          n++;
        }
      }
    }

    // transform 2D histogram in a 1D vector

    int dim; // calculate dimensionality of the vector
    dim = 0;
    for (y = 0;y<param.redCells;y++) {
      dim+=param.greenCells - ((y*param.greenCells)/param.redCells);
    }

    chrHist.resize(dim,0,false,false);

    int step = +1;
    dvector::iterator dit;
    int x;

    if (n==0) ++n;

    for (dit=chrHist.begin(),y=0;y<param.redCells;y++,step*=-1) {
      dim = param.greenCells - ((y*param.greenCells)/param.redCells);
      if (step>0) {
        for (x=0;x<dim;x++,dit++) {
          (*dit)=(histo.at(y,x)/n);
        }
      } else {
        for (x=dim-1;x>=0;x--,dit++) {
          (*dit)=(histo.at(y,x)/n);
        }
      }
    }

    // smooth the histogram if necessary

    if (param.smoothingKernelSize > 1) {
      gaussKernel1D<double> gk(param.smoothingKernelSize);
      convolution conv1D;
      conv1D.setKernel(gk);
      conv1D.apply(chrHist);
    }

    return true;
  };

}
