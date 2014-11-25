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
 * file .......: ltiCsPresegmentation.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 8.11.2001
 * revisions ..: $Id: ltiCsPresegmentation.cpp,v 1.9 2006/09/05 10:08:25 ltilib Exp $
 */

#include "ltiCsPresegmentation.h"
#include "ltiUsePalette.h"
#include "ltiKMColorQuantization.h"
#include "ltiMedianFilter.h"
#include "ltiKNearestNeighFilter.h"
#include "ltiVector.h"
#include "ltiSerialStatsFunctor.h"

#undef _LTI_DEBUG
// #define _LTI_DEBUG 4
#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // csPresegmentation::parameters
  // --------------------------------------------------

  // static members
  const int csPresegmentation::parameters::All    = 0x0F;
  const int csPresegmentation::parameters::Top    = 0x01;
  const int csPresegmentation::parameters::Bottom = 0x02;
  const int csPresegmentation::parameters::Left   = 0x04;
  const int csPresegmentation::parameters::Right  = 0x08;


  // default constructor
  csPresegmentation::parameters::parameters()
    : segmentation::parameters() {

    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    smoothingKernelSize = int(5);
    quantParameters.numberOfColors = int(12);
    quantParameters.thresholdDeltaPalette = 0.5f;
    quantParameters.maximalNumberOfIterations = 50;
    useAlwaysNewPalette = bool(false);
    borderSize = int(16);
    borderParts = All;
    forceBorderToBackground = bool(false);
    labelObjects = bool(false);
    backgroundTolerance = 9.0f;
    filterType = Median;
  }

  // copy constructor
  csPresegmentation::parameters::parameters(const parameters& other)
    : segmentation::parameters()  {
    copy(other);
  }

  // destructor
  csPresegmentation::parameters::~parameters() {
  }

  // get type name
  const char* csPresegmentation::parameters::getTypeName() const {
    return "csPresegmentation::parameters";
  }

  // copy member

  csPresegmentation::parameters&
    csPresegmentation::parameters::copy(const parameters& other) {
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


    smoothingKernelSize = other.smoothingKernelSize;
    quantParameters = other.quantParameters;
    useAlwaysNewPalette = other.useAlwaysNewPalette;
    borderSize = other.borderSize;
    borderParts = other.borderParts;
    forceBorderToBackground = other.forceBorderToBackground;
    labelObjects = other.labelObjects;
    backgroundTolerance = other.backgroundTolerance;
    filterType = other.filterType;

    return *this;
  }

  // alias for copy member
  csPresegmentation::parameters&
    csPresegmentation::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* csPresegmentation::parameters::clone() const {
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
  bool csPresegmentation::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool csPresegmentation::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"smoothingKernelSize",smoothingKernelSize);
      lti::write(handler,"quantParameters",quantParameters);
      lti::write(handler,"useAlwaysNewPalette",useAlwaysNewPalette);
      lti::write(handler,"borderSize",borderSize);
      lti::write(handler,"borderParts",borderParts);
      lti::write(handler,"forceBorderToBackground",forceBorderToBackground);
      lti::write(handler,"labelObjects",labelObjects);
      lti::write(handler,"backgroundTolerance",backgroundTolerance);

      if (filterType == Median) {
        lti::write(handler,"filterType","Median");
      } else if (filterType == KNearest) {
        lti::write(handler,"filterType","KNearest");
      } else {
        lti::write(handler,"filterType","Nothing");
      }

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
  bool csPresegmentation::parameters::write(ioHandler& handler,
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
  bool csPresegmentation::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool csPresegmentation::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"smoothingKernelSize",smoothingKernelSize);
      lti::read(handler,"quantParameters",quantParameters);
      lti::read(handler,"useAlwaysNewPalette",useAlwaysNewPalette);
      lti::read(handler,"borderSize",borderSize);
      lti::read(handler,"borderParts",borderParts);
      lti::read(handler,"forceBorderToBackground",forceBorderToBackground);
      lti::read(handler,"labelObjects",labelObjects);
      lti::read(handler,"backgroundTolerance",backgroundTolerance);

      std::string tmp;
      lti::read(handler,"filterType",tmp);

      if (tmp == "Median") {
        filterType = Median;
      } else if (tmp == "KNearest") {
        filterType = KNearest;
      } else {
        filterType = Nothing;
      }
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
  bool csPresegmentation::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // csPresegmentation
  // --------------------------------------------------

  // default constructor
  csPresegmentation::csPresegmentation()
    : segmentation(){

    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // copy constructor
  csPresegmentation::csPresegmentation(const csPresegmentation& other)
    : segmentation()  {
    copy(other);
  }

  // destructor
  csPresegmentation::~csPresegmentation() {
  }

  // returns the name of this type
  const char* csPresegmentation::getTypeName() const {
    return "csPresegmentation";
  }

  // copy member
  csPresegmentation& csPresegmentation::copy(const csPresegmentation& other) {
    segmentation::copy(other);

    return (*this);
  }

  // alias for copy member
  csPresegmentation&
  csPresegmentation::operator=(const csPresegmentation& other) {
    return (copy(other));
  }


  // clone member
  functor* csPresegmentation::clone() const {
    return new csPresegmentation(*this);
  }

  // return parameters
  const csPresegmentation::parameters&
    csPresegmentation::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool csPresegmentation::validBackground(const rgbPixel& p,
                                          const trgbPixel<float>& mean,
                                          const trgbPixel<float>& var,
                                          const float& tolerance) const {
    trgbPixel<float> tmp(p);
    tmp.subtract(mean);
    tmp.multiply(tmp);

    return ((tmp.red <= tolerance*var.red) &&
            (tmp.green <= tolerance*var.green) &&
            (tmp.blue <= tolerance*var.blue));
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On copy apply for type image!
  bool csPresegmentation::apply(const image& src,
                                channel8& mask) {

    const parameters& param = getParameters();
    const float tolerance = param.backgroundTolerance;
    int i,x,xr,y,n;
    bool changed;

    // initialize color quantization functor
    kMColorQuantization quant;
    quant.setParameters(param.quantParameters);

    if ((param.quantParameters.numberOfColors != lastPalette.size()) ||
        (param.useAlwaysNewPalette)) {
      lastPalette.clear();
    }

    // color quantization
    quant.apply(src,mask,lastPalette);

    // initialize median filter functor

    if (param.filterType == parameters::Median) {
      medianFilter medianFilter;
      medianFilter::parameters medianPar;
      medianPar.kernelSize = param.smoothingKernelSize;
      medianPar.boundaryType = lti::Constant;
      medianFilter.setParameters(medianPar);
      medianFilter.apply(mask);
    } else if (param.filterType == parameters::KNearest) {
      kNearestNeighFilter knnFilter;
      kNearestNeighFilter::parameters knnPar;
      knnPar.kernelSize = param.smoothingKernelSize;
      knnPar.boundaryType = lti::Constant;
      knnFilter.setParameters(knnPar);
      knnFilter.apply(mask);
    }

    // get the entries in the palette which belong to the background
    const point border = point(min(param.borderSize,mask.columns()/2),
                               min(param.borderSize,mask.rows()/2));


    ivector histo(lastPalette.size(),0);

    // create a histogram of the number of entries in the border per
    // color

    // the upper border
    if ((param.borderParts & param.Top) != 0) {
      for (y=0;y<border.y;++y) {
        for (x=0;x<mask.columns();++x) {
          histo.at(mask.at(y,x))++;
        }
      }
    }

    // the bottom border
    if ((param.borderParts & param.Bottom) != 0) {
      for (y=mask.rows()-border.y;y<mask.rows();++y) {
        for (x=0;x<mask.columns();++x) {
          histo.at(mask.at(y,x))++;
        }
      }
    }

    const int lastY = mask.rows()-border.y;

    // the left border
    if ((param.borderParts & param.Left) != 0) {
      // can we do both borders at the same time?
      if ((param.borderParts & param.Right) != 0) {
        // both borders
        for (y=border.y;y<lastY;++y) {
          for (x=0,xr=mask.columns()-border.x;x<border.x;++x,++xr) {
            histo.at(mask.at(y,x))++;
            histo.at(mask.at(y,xr))++;
          }
        }
      } else {
        // only left border
        for (y=border.y;y<lastY;++y) {
          for (x=0;x<border.x;++x) {
            histo.at(mask.at(y,x))++;
          }
        }
      }
    } else if ((param.borderParts & param.Left) != 0) {
      // only right border
      for (y=border.y;y<lastY;++y) {
        for (x=mask.columns()-border.x;x<mask.columns();++x) {
          histo.at(mask.at(y,x))++;
        }
      }
    }

    //
    ivector flags(lastPalette.size(),int(1));
    serialStatsFunctor<float> statsRed,statsGreen,statsBlue;
    serialStatsFunctor<float>::parameters statsPar;
    statsPar.type = serialStatsFunctor<float>::parameters::Maxlikely;
    statsRed.setParameters(statsPar);
    statsGreen.setParameters(statsPar);
    statsBlue.setParameters(statsPar);

    // initial values chosen in a way that will force all
    // pixels at the boundary to be background (the first iteration)
    trgbPixel<float> mean(0,0,0),var(70000,70000,70000);

    i = 0;

    do {
      _lti_debug3("Iteration " << i << ": \n\r");

      statsRed.reset();
      statsGreen.reset();
      statsBlue.reset();

      changed = false;
      x = 0;

      while (x<histo.size()) {
        const lti::rgbPixel& p = lastPalette.at(x);

        _lti_debug3("Entry " << x << " " << p);

        n = histo.at(x);

        if (n>0) {
          if (validBackground(p,mean,var,tolerance)) {

            statsRed.consider(p.getRed(),n);
            statsGreen.consider(p.getGreen(),n);
            statsBlue.consider(p.getBlue(),n);

            changed = changed || (flags.at(x) != 0);
            flags.at(x) = 0;
            _lti_debug3("  --> Background");
          } else {
            changed = changed || (flags.at(x) != 1);
            flags.at(x) = 1;
            _lti_debug3("  --> Object");
          }
        }

        _lti_debug3("\n\r");
        ++x;

      }

      statsRed.apply(mean.red,var.red);
      statsGreen.apply(mean.green,var.green);
      statsBlue.apply(mean.blue,var.blue);

      _lti_debug3(" --------------------------------------------\n\r");
      _lti_debug3(" Avrg/Var " << mean << " / " << var << "\n\r");

      ++i;

    } while (!param.forceBorderToBackground && changed);

    if (param.labelObjects) {
      for (n=1,i=0;i<flags.size();++i) {
        if (flags.at(i) != 0) {
          flags.at(i)=n;
          n++;
        }
      }
    } else {
      for (i=0;i<flags.size();++i) {
        if (flags.at(i) != 0) {
          flags.at(i)=255;
        }
      }
    }

    // mask background as perfect black...
    lti::channel8::iterator it,eit;

    for (it=mask.begin(),eit=mask.end();
         it != eit;
         ++it) {
      (*it) = flags.at(*it);
    }

    return true;
  };



}
