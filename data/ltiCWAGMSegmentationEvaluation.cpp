/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiCWAGMSegmentationEvaluation.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 25.11.2003
 * revisions ..: $Id: ltiCWAGMSegmentationEvaluation.cpp,v 1.4 2008/12/17 14:10:25 alvarado Exp $
 */

#include "ltiCWAGMSegmentationEvaluation.h"

#undef _LTI_DEBUG
// #define _LTI_DEBUG 2
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#include "ltiViewer.h"
#include "ltiDraw.h"
#endif


namespace lti {


  // --------------------------------------------------
  // cwagmSegmentationEvaluation::parameters
  // --------------------------------------------------

  // default constructor
  cwagmSegmentationEvaluation::parameters::parameters()
    : segmentationEvaluation::parameters() {
    
    //
    // Preprocessing
    //

    minValues.medianParam.kernelSize = 1;    
    maxValues.medianParam.kernelSize = 7;

    //
    // General split configuration
    //

    minValues.colorSplitter = "lti::splitImageToRGB";
    maxValues.colorSplitter = "lti::splitImageToXYZ";

    minValues.colorContrastParam.kernelType = 
      colorContrastGradient::parameters::Optimal;
    maxValues.colorContrastParam.kernelType =
      colorContrastGradient::parameters::Kirsch;

    minValues.colorContrastParam.contrastFormat = 
      colorContrastGradient::parameters::MDD;
    maxValues.colorContrastParam.contrastFormat =
      colorContrastGradient::parameters::Maximum;

    //
    // Watershed configuration
    //

    minValues.watershedParam.neighborhood8 = false;
    maxValues.watershedParam.neighborhood8 = true;

    minValues.minProbForWatershedThreshold = 0.0;
    maxValues.minProbForWatershedThreshold = 1.0;

    minValues.harisRegionMergeParam.mergeThreshold = 0;
    maxValues.harisRegionMergeParam.mergeThreshold = 25;

    minValues.harisRegionMergeParam.minRegionNumber = 1;
    maxValues.harisRegionMergeParam.minRegionNumber = 25;

 }

  // copy constructor
  cwagmSegmentationEvaluation::parameters::parameters(const parameters& other)
    : segmentationEvaluation::parameters() {
    copy(other);
  }

  // destructor
  cwagmSegmentationEvaluation::parameters::~parameters() {
  }

  // get type name
  const char* cwagmSegmentationEvaluation::parameters::getTypeName() const {
    return "cwagmSegmentationEvaluation::parameters";
  }

  // copy member

  cwagmSegmentationEvaluation::parameters&
  cwagmSegmentationEvaluation::parameters::copy(const parameters& other) {

    segmentationEvaluation::parameters::copy(other);

    minValues = other.minValues;
    maxValues = other.maxValues;

    return *this;
  }

  // alias for copy member
  cwagmSegmentationEvaluation::parameters&
  cwagmSegmentationEvaluation::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* cwagmSegmentationEvaluation::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was succeful
   */
  bool 
  cwagmSegmentationEvaluation::parameters::write(ioHandler& handler,
                                                 const bool& complete) const {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"minValues",minValues);
      lti::write(handler,"maxValues",maxValues);
    }

    b = b && segmentationEvaluation::parameters::write(handler,false);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

  /*
   * read the parameters from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if read was succeful
   */
  bool cwagmSegmentationEvaluation::parameters::read(ioHandler& handler,
                                                     const bool& complete) {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"minValues",minValues);
      lti::read(handler,"maxValues",maxValues);
    }

    b = b && segmentationEvaluation::parameters::read(handler,false);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

  // --------------------------------------------------
  // cwagmSegmentationEvaluation
  // --------------------------------------------------

  // default constructor
  cwagmSegmentationEvaluation::cwagmSegmentationEvaluation()
    : segmentationEvaluation() {
  }

  // default constructor
  cwagmSegmentationEvaluation::
  cwagmSegmentationEvaluation(const parameters& par)
    : segmentationEvaluation(par) {
  }


  // copy constructor
  cwagmSegmentationEvaluation::cwagmSegmentationEvaluation(
                                      const cwagmSegmentationEvaluation& other
                                     ) : segmentationEvaluation(other) {
  }

  // destructor
  cwagmSegmentationEvaluation::~cwagmSegmentationEvaluation() {
  }

  // returns the name of this type
  const char* cwagmSegmentationEvaluation::getTypeName() const {
    return "cwagmSegmentationEvaluation";
  }

  /*
   * Returns a pointer to a clone of this functor.
   */
  functor* cwagmSegmentationEvaluation::clone() const {
    return new cwagmSegmentationEvaluation(*this);
  }

  // return parameters
  const cwagmSegmentationEvaluation::parameters& 
  cwagmSegmentationEvaluation::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  //
  // global private enum, with the number of pixels per parameter
  // it is global to define this constants for all method that need them,
  //
  struct cwagmSegmentationEvaluation::bits {
    enum {
      MedianKernel                 = 2,
      ColorSplitter                = 3,
      GradientType                 = 3,
      ContrastFormat               = 2,  // __ 12 Bits => (MR 13.08)

      WatershedNeighborhood        = 1,
      WatershedThreshold           = 8, // real threshold (usually 255)
      WatershedMinProbThreshold    = 8, // flood factor
      WatershedHarisMerge          = 16,// merge threshold
      WatershedHarisMergeMode      = 1, // fast/optimal
      WatershedHarisMinNumRegions  = 12, // __ 42 Bits => (MR 3.74)
                                         // (both groups: 54 => 2.9
    };

    static int total() {
      return (      MedianKernel                 +
                    ColorSplitter                +
                    GradientType                 +
                    ContrastFormat               +
                    
                    WatershedNeighborhood        +
                    WatershedThreshold           +
                    WatershedMinProbThreshold    +
                    WatershedHarisMerge          +
                    WatershedHarisMergeMode      +
                    WatershedHarisMinNumRegions   ); // 132 bits?
    }
  };


  bool cwagmSegmentationEvaluation::chromosomeToPhenotype(
                                        const chromosome& genotype,
                                        functor::parameters& phenotype) const {
    
    const parameters& par = getParameters();
    cwagmSegmentation::parameters* phen = 
      dynamic_cast<cwagmSegmentation::parameters*>(&phenotype);
    
    if (isNull(phen)) {
      return false;
    }

    int pos=0;
    int ires,theMin,theMax;
    double dres;

    // median kernel size
    pos = binToInt(genotype,pos,bits::MedianKernel,
                   (par.minValues.medianParam.kernelSize-1)/2,
                   (par.maxValues.medianParam.kernelSize-1)/2,ires);
    phen->medianParam.kernelSize = (ires*2+1);
    
    // color splitter
    pos = binToInt(genotype,pos,bits::ColorSplitter,0,7,ires);
   
    if (par.minValues.colorSplitter == par.maxValues.colorSplitter) {
      phen->colorSplitter = par.minValues.colorSplitter;
    } else {
      if (par.minValues.colorSplitter.find("XYZ") != std::string::npos){
        theMin=1;
      } else if (par.minValues.colorSplitter.find("xyY") != std::string::npos){
        theMin=2;
      } else if (par.minValues.colorSplitter.find("Luv") != std::string::npos){
        theMin=3;
      } else if (par.minValues.colorSplitter.find("rgI") != std::string::npos){
        theMin=4;
      } else if (par.minValues.colorSplitter.find("YUV") != std::string::npos){
        theMin=5;
      } else if (par.minValues.colorSplitter.find("YIQ") != std::string::npos){
        theMin=6;
      } else if (par.minValues.colorSplitter.find("OCP") != std::string::npos){
        theMin=7;
      } else { // (par.minValues.colorSplitter.find("RGB")!=std::string::npos)
        theMin=0; // RGB
      } 

      if (par.maxValues.colorSplitter.find("XYZ") != std::string::npos){
        theMax=1;
      } else if (par.maxValues.colorSplitter.find("xyY") != std::string::npos){
        theMax=2;
      } else if (par.maxValues.colorSplitter.find("Luv") != std::string::npos){
        theMax=3;
      } else if (par.maxValues.colorSplitter.find("rgI") != std::string::npos){
        theMax=4;
      } else if (par.maxValues.colorSplitter.find("YUV") != std::string::npos){
        theMax=5;
      } else if (par.maxValues.colorSplitter.find("YIQ") != std::string::npos){
        theMax=6;
      } else if (par.maxValues.colorSplitter.find("OCP") != std::string::npos){
        theMax=7;
      } else { // (par.maxValues.colorSplitter.find("RGB")!=std::string::npos)
        theMax=0; // RGB
      } 

      ires = max(theMin,min(theMax,ires));

      switch (ires) {
        case 0:
          phen->colorSplitter = "lti::splitImageToRGB";
          break;
        case 1:
          phen->colorSplitter = "lti::splitImageToXYZ";
          break;
        case 2:
          phen->colorSplitter = "lti::splitImageToxyY";
          break;
        case 3:
          phen->colorSplitter = "lti::splitImageToLuv";
          break;
        case 4:
          phen->colorSplitter = "lti::splitImageTorgI";
          break;
        case 5:
          phen->colorSplitter = "lti::splitImageToYUV";
          break;
        case 6:
          phen->colorSplitter = "lti::splitImageToYIQ";
          break;
        case 7:
          phen->colorSplitter = "lti::splitImageToOCP";
          break;
        default:
          phen->colorSplitter = "lti::splitImageToXYZ";
          break;
      }
    }
    
    // gradient kernel type
    pos = binToInt(genotype,pos,bits::GradientType,0,7,ires);
    if (par.minValues.colorContrastParam.kernelType == 
        par.maxValues.colorContrastParam.kernelType) {
      phen->colorContrastParam.kernelType =
        par.minValues.colorContrastParam.kernelType;
    } else {
      theMin = static_cast<int>(par.minValues.colorContrastParam.kernelType);
      theMax = static_cast<int>(par.maxValues.colorContrastParam.kernelType);
      minmax(theMin,theMax);
      ires = max(theMin,min(theMax,ires));

      phen->colorContrastParam.kernelType = 
        static_cast<gradientFunctor::parameters::eKernelType>(ires);
    }

    // gradient contrast format
    pos = binToInt(genotype,pos,bits::ContrastFormat,0,3,ires);
    if (par.minValues.colorContrastParam.contrastFormat == 
        par.maxValues.colorContrastParam.contrastFormat) {
      phen->colorContrastParam.contrastFormat =
        par.minValues.colorContrastParam.contrastFormat;
    } else {
      theMin=static_cast<int>(par.minValues.colorContrastParam.contrastFormat);
      theMax=static_cast<int>(par.maxValues.colorContrastParam.contrastFormat);
      minmax(theMin,theMax);
      ires = max(theMin,min(theMax,ires));

      phen->colorContrastParam.contrastFormat = 
        static_cast<colorContrastGradient::parameters::eContrastFormat>(ires);
    }

    // watershed configuration

    // neighborhood
    pos = binToInt(genotype,pos,bits::WatershedNeighborhood,
                   (par.minValues.watershedParam.neighborhood8) ? 1 : 0,
                   (par.maxValues.watershedParam.neighborhood8) ? 1 : 0,
                   ires);

    phen->watershedParam.neighborhood8 = (ires != 0);

    // WatershedThreshold
    pos = binToInt(genotype,pos,bits::WatershedThreshold,
                   par.minValues.watershedParam.threshold,
                   par.maxValues.watershedParam.threshold,ires);

    phen->watershedParam.threshold = static_cast<ubyte>(ires);

    // minProbForWatershedThreshold
    pos = binToDouble(genotype,pos,bits::WatershedMinProbThreshold,
                      par.minValues.minProbForWatershedThreshold,
                      par.maxValues.minProbForWatershedThreshold,dres);

    phen->minProbForWatershedThreshold = static_cast<float>(dres);

    // harisRegionMergeParam.mergeThreshold
    pos = binToDouble(genotype,pos,bits::WatershedHarisMerge,
                      par.minValues.harisRegionMergeParam.mergeThreshold,
                      par.maxValues.harisRegionMergeParam.mergeThreshold,dres);

    phen->harisRegionMergeParam.mergeThreshold = static_cast<float>(dres);

    // harisRegionMergeParam.mergeMode
    pos = binToInt(genotype,pos,bits::WatershedHarisMergeMode,
               static_cast<int>(par.minValues.harisRegionMergeParam.mergeMode),
               static_cast<int>(par.maxValues.harisRegionMergeParam.mergeMode),
                   ires);

    phen->harisRegionMergeParam.mergeMode =
      static_cast<regionGraphColorHaris::parameters::eMergeMode>(ires);

    // harisRegionMergeParam.minRegionNumber
    pos = binToDouble(genotype,pos,bits::WatershedHarisMinNumRegions,
                      par.minValues.harisRegionMergeParam.minRegionNumber,
                      par.maxValues.harisRegionMergeParam.minRegionNumber,
                      dres);

    phen->harisRegionMergeParam.minRegionNumber = iround(dres);

    // check in debug modus that everything is there!
    assert(pos == bits::total());

    return true;
  }

  bool cwagmSegmentationEvaluation::phenotypeToChromosome(
                                         const functor::parameters& phenotype,
                                         chromosome& genotype) const {
    
    genotype.resize(bits::total());

    const parameters& par = getParameters();
    const cwagmSegmentation::parameters* phen =
      dynamic_cast<const cwagmSegmentation::parameters*>(&phenotype);

    if (isNull(phen)) {
      return false;
    }

    int pos=0;
    int ires;
    
    // median kernel size
    pos = intToBin((phen->medianParam.kernelSize-1)/2,pos,bits::MedianKernel,
                   (par.minValues.medianParam.kernelSize-1)/2,
                   (par.maxValues.medianParam.kernelSize-1)/2,
                   genotype);

    // color splitter
    if (phen->colorSplitter.find("XYZ") != std::string::npos) {
      ires=1;
    } else if (phen->colorSplitter.find("xyY") != std::string::npos) {
      ires=2;
    } else if (phen->colorSplitter.find("Luv") != std::string::npos) {
      ires=3;
    } else if (phen->colorSplitter.find("rgI") != std::string::npos) {
      ires=4;
    } else if (phen->colorSplitter.find("YUV") != std::string::npos) {
      ires=5;
    } else if (phen->colorSplitter.find("YIQ") != std::string::npos) {
      ires=6;
    } else if (phen->colorSplitter.find("OCP") != std::string::npos) {
      ires=7;
    } else { // (phen->colorSplitter.find("RGB")!=std::string::npos)
      ires=0; // RGB
    } 
    
    pos = intToBin(ires,pos,bits::ColorSplitter,0,7,genotype);

    // gradient kernel type
    ires = static_cast<int>(phen->colorContrastParam.kernelType);
    pos = intToBin(ires,pos,bits::GradientType,0,7,genotype);

    // gradient contrast format
    ires = static_cast<int>(phen->colorContrastParam.contrastFormat);
    pos = intToBin(ires,pos,bits::ContrastFormat,0,3,genotype);

    // watershed
    
    // neighborhood
    pos = intToBin((phen->watershedParam.neighborhood8) ? 1 : 0,
                   pos,bits::WatershedNeighborhood,
                   (par.minValues.watershedParam.neighborhood8) ? 1 : 0,
                   (par.maxValues.watershedParam.neighborhood8) ? 1 : 0,
                   genotype);

    // WatershedThreshold
    pos = intToBin(phen->watershedParam.threshold,
                   pos,bits::WatershedThreshold,
                   par.minValues.watershedParam.threshold,
                   par.maxValues.watershedParam.threshold,genotype);

    // minProbForWatershedThreshold
    pos = doubleToBin(phen->minProbForWatershedThreshold,
                      pos,bits::WatershedMinProbThreshold,
                      par.minValues.minProbForWatershedThreshold,
                      par.maxValues.minProbForWatershedThreshold,genotype);

    // harisRegionMergeParam.mergeThreshold
    pos = doubleToBin(phen->harisRegionMergeParam.mergeThreshold,
                      pos,bits::WatershedHarisMerge,
                      par.minValues.harisRegionMergeParam.mergeThreshold,
                      par.maxValues.harisRegionMergeParam.mergeThreshold,
                      genotype);

    // neighborhood
    pos = intToBin(static_cast<int>(phen->harisRegionMergeParam.mergeMode),
                   pos,bits::WatershedHarisMergeMode,
               static_cast<int>(par.minValues.harisRegionMergeParam.mergeMode),
               static_cast<int>(par.maxValues.harisRegionMergeParam.mergeMode),
                   genotype);

    // harisRegionMergeParam.minRegionNumber
    pos = doubleToBin(double(phen->harisRegionMergeParam.minRegionNumber),
                      pos,bits::WatershedHarisMinNumRegions,
                      par.minValues.harisRegionMergeParam.minRegionNumber,
                      par.maxValues.harisRegionMergeParam.minRegionNumber,
                      genotype);

    assert (pos == bits::total());
    
    return true;
    
  }

  int cwagmSegmentationEvaluation::getChromosomeSize() const {
    return bits::total();
  }

  bool cwagmSegmentationEvaluation::init(const functor::parameters& param) {
    const cwagmSegmentation::parameters* parPtr = 
      dynamic_cast<const cwagmSegmentation::parameters*>(&param);

    if (isNull(parPtr)) {
      _lti_debug("Error initializing segmentation (wrong parameters?)\n");
      return false;
    }

    return segmenter.setParameters(param);
  }

  bool cwagmSegmentationEvaluation::segment(const image& img,
                                            const imatrix& prevMask,
                                            imatrix& mask,
                                            channel& certainty) {
    ivector sizes;
    if (!segmenter.apply(img,mask,sizes)) {
      _lti_debug("Error in segmenter: " << segmenter.getStatusString() <<
                 std::endl);
      setStatusString(segmenter.getStatusString());
      return false;
    }
    certainty.clear(); // no certainty computation in this kind of functors.
    return true;
  }

  functor::parameters* cwagmSegmentationEvaluation::getInstanceOfParam() const{
    return getParameters().minValues.clone();
  }

}
