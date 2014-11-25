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
 * file .......: ltiHistogramRGBL.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 14.5.2001
 * revisions ..: $Id: ltiHistogramRGBL.cpp,v 1.10 2006/09/05 10:15:49 ltilib Exp $
 */

#include "ltiHistogramRGBL.h"
#include "ltiGaussKernels.h"
#include "ltiConvolution.h"

namespace lti {
  // --------------------------------------------------
  // histogramRGBL::parameters
  // --------------------------------------------------

  // default constructor
  histogramRGBL::parameters::parameters()
    : globalFeatureExtractor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    smooth = bool(true);
    normalize = bool(true);
    cells = int(32);
    considerAllData = bool(false);
    ignoreValue = rgbPixel(0,0,0);
    kernel = gaussKernel1D<double>(3);
  }

  // copy constructor
  histogramRGBL::parameters::parameters(const parameters& other)
    : globalFeatureExtractor::parameters()  {
    copy(other);
  }

  // destructor
  histogramRGBL::parameters::~parameters() {
  }

  // get type name
  const char* histogramRGBL::parameters::getTypeName() const {
    return "histogramRGBL::parameters";
  }

  // copy member

  histogramRGBL::parameters&
    histogramRGBL::parameters::copy(const parameters& other) {
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

    smooth = other.smooth;
    normalize = other.normalize;
    cells = other.cells;
    considerAllData = other.considerAllData;
    ignoreValue = other.ignoreValue;
    kernel.copy(other.kernel);

    return *this;
  }

  // alias for copy member
  histogramRGBL::parameters&
    histogramRGBL::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* histogramRGBL::parameters::clone() const {
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
  bool histogramRGBL::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool histogramRGBL::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"smooth",smooth);
      lti::write(handler,"normalize",normalize);
      lti::write(handler,"cells",cells);
      lti::write(handler,"considerAllData",considerAllData);
      lti::write(handler,"ignoreValue",ignoreValue);
      lti::write(handler,"kernel",kernel);
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
  bool histogramRGBL::parameters::write(ioHandler& handler,
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
  bool histogramRGBL::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool histogramRGBL::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"smooth",smooth);
      lti::read(handler,"normalize",normalize);
      lti::read(handler,"cells",cells);
      lti::read(handler,"considerAllData",considerAllData);
      lti::read(handler,"ignoreValue",ignoreValue);
      lti::read(handler,"kernel",kernel);
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
  bool histogramRGBL::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // histogramRGBL
  // --------------------------------------------------

  // default constructor
  histogramRGBL::histogramRGBL()
    : globalFeatureExtractor(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  histogramRGBL::histogramRGBL(const histogramRGBL& other)
    : globalFeatureExtractor()  {
    copy(other);
  }

  // destructor
  histogramRGBL::~histogramRGBL() {
  }

  // returns the name of this type
  const char* histogramRGBL::getTypeName() const {
    return "histogramRGBL";
  }

  // copy member
  histogramRGBL&
    histogramRGBL::copy(const histogramRGBL& other) {
      globalFeatureExtractor::copy(other);
    return (*this);
  }

  // clone member
  functor* histogramRGBL::clone() const {
    return new histogramRGBL(*this);
  }

  // return parameters
  const histogramRGBL::parameters&
    histogramRGBL::getParameters() const {
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
  bool histogramRGBL::apply(const image& src,dvector& dest) const {

    if (src.empty()) {
      dest.clear();
      setStatusString("input channel empty");
      return false;
    }

    const parameters& param = getParameters();

    int theMin(0),theMax(255);

    const int lastIdx = param.cells-1;

    const float m = float(lastIdx)/(theMax-theMin);
    int y,r,g,b,l;
    int idx;
    int entries;

    vector<rgbPixel>::const_iterator it,eit;

    dest.resize(4*param.cells,0.0,false,true); // initialize with 0
    dvector theR(param.cells,0.0);
    dvector theG(param.cells,0.0);
    dvector theB(param.cells,0.0);
    dvector theL(param.cells,0.0);

    entries = 0;

    // if b too small, it's possible to calculate everything faster...

    // check if the ignore value
    if (param.considerAllData) {
      for (y=0;y<src.rows();++y) {
        const vector<rgbPixel>& vct = src.getRow(y);
        for (it=vct.begin(),eit=vct.end();it!=eit;++it) {
          r = (*it).getRed();
          g = (*it).getGreen();
          b = (*it).getBlue();
          l = (min(r,g,b)+max(r,g,b))/2;

          idx = static_cast<int>(r*m);
          theR.at(idx)++;
          idx = static_cast<int>(g*m);
          theG.at(idx)++;
          idx = static_cast<int>(b*m);
          theB.at(idx)++;
          idx = static_cast<int>(l*m);
          theL.at(idx)++;

          entries++;
        }
      }
    } else {
      for (y=0;y<src.rows();++y) {
        const vector<rgbPixel>& vct = src.getRow(y);
        for (it=vct.begin(),eit=vct.end();it!=eit;++it) {
          if ((*it) != param.ignoreValue) {
            r = (*it).getRed();
            g = (*it).getGreen();
            b = (*it).getBlue();
            l = (min(r,g,b)+max(r,g,b))/2;

            idx = static_cast<int>(r*m);
            theR.at(idx)++;
            idx = static_cast<int>(g*m);
            theG.at(idx)++;
            idx = static_cast<int>(b*m);
            theB.at(idx)++;
            idx = static_cast<int>(l*m);
            theL.at(idx)++;

            entries++;
          }
        }
      }
    }

    if (param.smooth) {
      convolution convolver;
      convolution::parameters cpar;
      cpar.boundaryType = lti::Mirror;
      cpar.setKernel(param.kernel);
      convolver.setParameters(cpar);

      matrix<double> tmp;
      tmp.useExternData(4,param.cells,&dest.at(0));

      convolver.apply(theR,tmp.getRow(0));
      convolver.apply(theG,tmp.getRow(1));
      convolver.apply(theB,tmp.getRow(2));
      convolver.apply(theL,tmp.getRow(3));

    } else {
      dest.fill(theR,0);
      dest.fill(theG,param.cells);
      dest.fill(theB,2*param.cells);
      dest.fill(theL,3*param.cells);
    }

    if (param.normalize) {
      if (entries > 0) {
        dest.divide(entries);
      }
    }

    return true;

  };

}
