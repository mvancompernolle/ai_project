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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiKMeansSegmentation.cpp
 * authors ....: Axel Berner
 * organization: LTI, RWTH Aachen
 * creation ...: 3.2.2002
 * revisions ..: $Id: ltiKMeansSegmentation.cpp,v 1.11 2006/09/05 10:18:06 ltilib Exp $
 */

#include "ltiMedianFilter.h"
#include "ltiKNearestNeighFilter.h"

#include "ltiKMeansSegmentation.h"

namespace lti {
  // --------------------------------------------------
  // kMeansSegmentation::parameters
  // --------------------------------------------------

  // default constructor
  kMeansSegmentation::parameters::parameters()
    : segmentation::parameters() {
    quantParameters.numberOfColors = 16;
    quantParameters.thresholdDeltaPalette = 1;
    smoothFilter = KNearest;
    kernelSize = 5;
  }

  kMeansSegmentation::parameters::parameters(const int numCols)
    : segmentation::parameters() {
    quantParameters.numberOfColors = numCols;
    quantParameters.thresholdDeltaPalette = 1;
    smoothFilter = KNearest;
    kernelSize = 5;
  }


  // copy constructor
  kMeansSegmentation::parameters::parameters(const parameters& other)
    : segmentation::parameters()  {
    copy(other);
  }

  // destructor
  kMeansSegmentation::parameters::~parameters() {
  }

  // get type name
  const char* kMeansSegmentation::parameters::getTypeName() const {
    return "kMeansSegmentation::parameters";
  }

  // copy member

  kMeansSegmentation::parameters&
    kMeansSegmentation::parameters::copy(const parameters& other) {
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

    quantParameters = other.quantParameters;
    smoothFilter = other.smoothFilter;
    kernelSize = other.kernelSize;

    return *this;
  }

  // alias for copy member
  kMeansSegmentation::parameters&
    kMeansSegmentation::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* kMeansSegmentation::parameters::clone() const {
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
  bool kMeansSegmentation::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool kMeansSegmentation::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"quantParameters",quantParameters);
      switch (smoothFilter) {
        case Nothing:
          lti::write(handler,"smoothFilter","Nothing");
          break;
        case Median:
          lti::write(handler,"smoothFilter","Median");
          break;
        case KNearest:
          lti::write(handler,"smoothFilter","KNearest");
          break;
        default:
          lti::write(handler,"smoothFilter","KNearest");
          break;
      }
      lti::write(handler,"kernelSize",kernelSize);
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
  bool kMeansSegmentation::parameters::write(ioHandler& handler,
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
  bool kMeansSegmentation::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool kMeansSegmentation::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"quantParameters",quantParameters);
      std::string str;
      lti::read(handler,"smoothFilter",str);
      if (str == "Nothing") {
        smoothFilter = Nothing;
      } else if (str == "Median") {
        smoothFilter = Median;
      } else if (str == "KNearest") {
        smoothFilter = KNearest;
      } else {
        smoothFilter = KNearest;
      }

      lti::read(handler,"kernelSize",kernelSize);
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
  bool kMeansSegmentation::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // kMeansSegmentation
  // --------------------------------------------------


  // default constructor
  kMeansSegmentation::kMeansSegmentation()
    : segmentation(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // default constructor
  kMeansSegmentation::kMeansSegmentation(const parameters& par)
    : segmentation(){

    // set the parameters
    setParameters(par);

  }

  kMeansSegmentation::kMeansSegmentation(const int numCols)
    : segmentation(){

    // create an instance of the parameters with the default values
    parameters defaultParameters(numCols);
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  kMeansSegmentation::kMeansSegmentation(const kMeansSegmentation& other)
    : segmentation()  {
    copy(other);
  }

  // destructor
  kMeansSegmentation::~kMeansSegmentation() {
  }

  // returns the name of this type
  const char* kMeansSegmentation::getTypeName() const {
    return "kMeansSegmentation";
  }

  // copy member
  kMeansSegmentation&
    kMeansSegmentation::copy(const kMeansSegmentation& other) {
      segmentation::copy(other);

    return (*this);
  }

  // alias for copy member
  kMeansSegmentation&
    kMeansSegmentation::operator=(const kMeansSegmentation& other) {
    return (copy(other));
  }


  // clone member
  functor* kMeansSegmentation::clone() const {
    return new kMeansSegmentation(*this);
  }

  // return parameters
  const kMeansSegmentation::parameters&
    kMeansSegmentation::getParameters() const {
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

  bool kMeansSegmentation::apply(const image& src,
                                       matrix<int>& dest) const {
    palette pal;
    return apply(src,dest,pal);
  }

  // On copy apply for type matrix<int>!
  bool kMeansSegmentation::apply(const image& src,
                                       matrix<int>& dest,
                                       palette& pal) const {

    const parameters& param = getParameters();

    kMColorQuantization quant;
    quant.setParameters(param.quantParameters);
    quant.apply(src,dest,pal);

    // Smooth quantised image (dest)
    switch (param.smoothFilter) {
      case parameters::Nothing: {
        break;
      }
      case parameters::Median: {
        if(pal.size()>255) {
          setStatusString("Error: to many labels in quantised image for " \
                          "median-filter");
          return false;
        }
        channel8 tmpSrc,tmpDest;
        tmpSrc.castFrom(dest); // matrix<int> -> channel8
        medianFilter median;
        medianFilter::parameters medianPar;
        medianPar.kernelSize = param.kernelSize;
        medianPar.boundaryType = lti::Constant;
        median.setParameters(medianPar);
        median.apply(tmpSrc,tmpDest);
        dest.castFrom(tmpDest); // channel8 -> matrix<int>
        break;
      }
      case parameters::KNearest: {
        imatrix& tmpSrc = dest;
        imatrix tmpDest;
        kNearestNeighFilter mostF;
        kNearestNeighFilter::parameters mostPar;
        mostPar.kernelSize = param.kernelSize;
        mostPar.boundaryType = lti::Constant;
        mostF.setParameters(mostPar);
        mostF.apply(tmpSrc,tmpDest);
        tmpDest.detach(dest);
        break;
      }
      default: {
        setStatusString("Error: unknown filter selected");
        return false;
      }
    }
    return true;
  };

  // On copy apply for type matrix<int>!
  bool kMeansSegmentation::apply(const image& src,
                                 channel8& dest) const {
    palette pal;
    return apply(src,dest,pal);
  }


  // On copy apply for type matrix<int>!
  bool kMeansSegmentation::apply(const image& src,
                                 channel8& dest,
                                 palette& pal) const {

    const parameters& param = getParameters();

    if (param.quantParameters.numberOfColors>256) {
      setStatusString("Resulting mask type can represent only 256 colors");
      return false;
    }
    kMColorQuantization quant;
    quant.setParameters(param.quantParameters);

    // Smooth quantised image (dest)
    switch (param.smoothFilter) {
      case parameters::Nothing: {
        quant.apply(src,dest,pal);
        break;
      }
      case parameters::Median: {
        quant.apply(src,dest,pal);

        medianFilter median;
        medianFilter::parameters medianPar;
        medianPar.kernelSize = param.kernelSize;
        medianPar.boundaryType = lti::Constant;
        median.setParameters(medianPar);
        median.apply(dest);
        break;
      }
      case parameters::KNearest: {
        imatrix tmp;
        quant.apply(src,tmp,pal);

        kNearestNeighFilter mostF;
        kNearestNeighFilter::parameters mostPar;
        mostPar.kernelSize = param.kernelSize;
        mostPar.boundaryType = lti::Constant;
        mostF.setParameters(mostPar);
        mostF.apply(tmp);
        dest.castFrom(tmp);
        break;
      }
      default: {
        setStatusString("Error: unknown filter selected");
        return false;
      }
    }
    return true;
  }

}
