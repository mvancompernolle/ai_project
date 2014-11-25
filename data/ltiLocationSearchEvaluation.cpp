/*
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003
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
 * file .......: ltiLocationSearchEvaluation.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 25.11.2003
 * revisions ..: $Id: ltiLocationSearchEvaluation.cpp,v 1.1 2006/03/10 02:53:11 alvarado Exp $
 */

#include "ltiRGBPixel.h"
#include "ltiLocationSearchEvaluation.h"
#include "ltiKdTree.h"
#include "ltiLocation.h"
#include "ltiSerialVectorStats.h"
#include "ltiGeometricTransform.h"
#include "ltiLoadImageList.h"
#include "ltiALLFunctor.h"
#include "ltiTimer.h"
#include "ltiConstants.h"
#include "ltiL2Distance.h"
#include <cstdio>

#undef _LTI_DEBUG
#define _LTI_DEBUG 1
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#define _LTI_SAVE_PARAM 1
#include "ltiLispStreamHandler.h"
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>
#include "ltiViewer.h"
#include "ltiDraw.h"
#endif


namespace lti {


  // --------------------------------------------------
  // locationSearchEvaluation::parameters
  // --------------------------------------------------

  // default constructor
  locationSearchEvaluation::parameters::parameters()
    : paretoFront::parameters() {

    numberOfLocations = false;
    locationsReciprocal = false;
    occupiedLevels = false;
    time = true;
    scaleRepeatability = false;
    rotationRepeatability = false;
    rotationScaleRepeatability = true;

    scaleStable = false;
    rotationStable = false;
    rotationScaleStable = false;

    images = std::string("images.txt");
    postfix = std::string("_premask");

    minValues.numLevels=2;
    minValues.upsampleFirstLevel=0;
    minValues.pyramidParameters.factor=0.5f;
    minValues.pyramidParameters.gaussian=false;
    minValues.pyramidParameters.interpolatorType = BilinearInterpolator;
    minValues.interpolateMaxima = false;
    minValues.checkInterLevelMaxima = false;
    minValues.smoothingGaussianVariance = 0;
    minValues.spatialMaximumNeighborhoodSize = 3;
    minValues.peakSelectionMode = 
      pyramidLocationSearch::parameters::GradientMagnitude;
    minValues.doGLevelDistance = 1;
    minValues.doGKernelVariance = 1;
    minValues.saliencyThreshold.resize(1,0.0,false,true);
    minValues.edgenessThreshold = 2;    

    minValues.densityFilter = false;
    minValues.densityKernelRadius = 3;
    minValues.densityThreshold.resize(1,0.0,false,true);

    minValues.orientationWindowRadius = 2;
    minValues.orientationSigmaFactor = 0.5f;
    minValues.averageOrientation = false;
 
    maxValues.numLevels=30;
    maxValues.upsampleFirstLevel=3;
    maxValues.pyramidParameters.factor=pow(0.5,1.0/8.0);
    maxValues.pyramidParameters.gaussian=true;
    maxValues.pyramidParameters.interpolatorType = BiquadraticInterpolator;
    maxValues.interpolateMaxima = true;
    minValues.checkInterLevelMaxima = true;
    maxValues.smoothingGaussianVariance = 5;
    maxValues.spatialMaximumNeighborhoodSize = 7;
    maxValues.peakSelectionMode = 
      pyramidLocationSearch::parameters::DoG;
    maxValues.doGLevelDistance = 4;
    maxValues.doGKernelVariance = 4;
    maxValues.saliencyThreshold.resize(1,0.99,false,true);
    maxValues.edgenessThreshold = 10;    

    maxValues.densityFilter = true;
    maxValues.densityKernelRadius = 64;
    maxValues.densityThreshold.resize(1,1.0,false,true);

    maxValues.orientationWindowRadius = 5;
    maxValues.orientationSigmaFactor = 6;
    maxValues.averageOrientation = true;

    stepAngle = degToRad(10.0f);
    stepScaling = sqrt(2.0f);
    firstScaling = 0.25f;
    lastScaling = 2.0f;
    firstAngle = 0.0f;
    lastAngle = static_cast<float>(2*Pi);

    considerAngle = true;
    scaleTolerance = 1.0f/sqrt(2.0f);
    positionTolerance = 1.5f;
    angleTolerance = degToRad(15);
  }

  // copy constructor
  locationSearchEvaluation::parameters::parameters(const parameters& other)
    : paretoFront::parameters() {
    copy(other);
  }

  // destructor
  locationSearchEvaluation::parameters::~parameters() {
  }

  // get type name
  const char* locationSearchEvaluation::parameters::getTypeName() const {
    return "locationSearchEvaluation::parameters";
  }

  // copy member

  locationSearchEvaluation::parameters&
    locationSearchEvaluation::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    paretoFront::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    paretoFront::parameters& (paretoFront::parameters::* p_copy)
      (const paretoFront::parameters&) =
      paretoFront::parameters::copy;
    (this->*p_copy)(other);
# endif

    numberOfLocations = other.numberOfLocations;
    locationsReciprocal = other.locationsReciprocal;
    occupiedLevels = other.occupiedLevels;
    time = other.time;

    scaleRepeatability = other.scaleRepeatability;
    rotationRepeatability = other.rotationRepeatability;
    rotationScaleRepeatability = other.rotationScaleRepeatability;
    
    scaleStable = other.scaleStable;
    rotationStable = other.rotationStable;
    rotationScaleStable = other.rotationScaleStable;

    images          = other.images;
    postfix         = other.postfix;
    minValues.copy(other.minValues);
    maxValues.copy(other.maxValues);
    stepAngle       = other.stepAngle;
    stepScaling     = other.stepScaling;
    firstScaling    = other.firstScaling;
    lastScaling     = other.lastScaling;
    firstAngle      = other.firstAngle;
    lastAngle       = other.lastAngle;

    considerAngle   = other.considerAngle;
    scaleTolerance  = other.scaleTolerance;
    positionTolerance = other.positionTolerance;
    angleTolerance  = other.angleTolerance;

    return *this;
  }

  // alias for copy member
  locationSearchEvaluation::parameters&
  locationSearchEvaluation::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* locationSearchEvaluation::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was succeful
   */
# ifndef _LTI_MSC_6
  bool locationSearchEvaluation::parameters::write(ioHandler& handler,
                                         const bool& complete) const
# else
  bool locationSearchEvaluation::parameters::writeMS(ioHandler& handler,
                                           const bool& complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"numberOfLocations",numberOfLocations);
      lti::write(handler,"locationsReciprocal",locationsReciprocal);
      lti::write(handler,"occupiedLevels",occupiedLevels);
      lti::write(handler,"time",time);
      lti::write(handler,"scaleRepeatability",scaleRepeatability);
      lti::write(handler,"rotationRepeatability",rotationRepeatability);
      lti::write(handler,"rotationScaleRepeatability",
                 rotationScaleRepeatability);

      lti::write(handler,"scaleStable",scaleStable);
      lti::write(handler,"rotationStable",rotationStable);
      lti::write(handler,"rotationScaleStable",
                 rotationScaleStable);
      
      lti::write(handler,"images",images);
      lti::write(handler,"postfix",postfix);
      lti::write(handler,"minValues",minValues);
      lti::write(handler,"maxValues",maxValues);
      lti::write(handler,"firstScaling",firstScaling);
      lti::write(handler,"lastScaling",lastScaling);
      lti::write(handler,"stepScaling",stepScaling);
      lti::write(handler,"firstAngle",firstAngle);
      lti::write(handler,"lastAngle",lastAngle);
      lti::write(handler,"stepAngle",stepAngle);
      lti::write(handler,"considerAngle",considerAngle);
      lti::write(handler,"scaleTolerance",scaleTolerance);
      lti::write(handler,"positionTolerance",positionTolerance);
      lti::write(handler,"angleTolerance",angleTolerance);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && paretoFront::parameters::write(handler,false);
# else
    bool (paretoFront::parameters::* p_writeMS)(ioHandler&,const bool&) const =
      paretoFront::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool locationSearchEvaluation::parameters::write(ioHandler& handler,
                                         const bool& complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /*
   * read the parameters from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if read was succeful
   */
# ifndef _LTI_MSC_6
  bool locationSearchEvaluation::parameters::read(ioHandler& handler,
                                        const bool& complete)
# else
  bool locationSearchEvaluation::parameters::readMS(ioHandler& handler,
                                          const bool& complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"numberOfLocations",numberOfLocations);
      lti::read(handler,"locationsReciprocal",locationsReciprocal);
      lti::read(handler,"occupiedLevels",occupiedLevels);
      lti::read(handler,"time",time);
      lti::read(handler,"scaleRepeatability",scaleRepeatability);
      lti::read(handler,"rotationRepeatability",rotationRepeatability);
      lti::read(handler,"rotationScaleRepeatability",
                rotationScaleRepeatability);
      lti::read(handler,"scaleStable",scaleStable);
      lti::read(handler,"rotationStable",rotationStable);
      lti::read(handler,"rotationScaleStable",
                rotationScaleStable);

      lti::read(handler,"images",images);
      lti::read(handler,"postfix",postfix);
      lti::read(handler,"minValues",minValues);
      lti::read(handler,"maxValues",maxValues);
      lti::read(handler,"firstScaling",firstScaling);
      lti::read(handler,"lastScaling",lastScaling);
      lti::read(handler,"stepScaling",stepScaling);

      lti::read(handler,"firstAngle",firstAngle);
      lti::read(handler,"lastAngle",lastAngle);
      lti::read(handler,"stepAngle",stepAngle);
      
      lti::read(handler,"considerAngle",considerAngle);
      lti::read(handler,"scaleTolerance",scaleTolerance);
      lti::read(handler,"positionTolerance",positionTolerance);
      lti::read(handler,"angleTolerance",angleTolerance);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && paretoFront::parameters::read(handler,false);
# else
    bool (paretoFront::parameters::* p_readMS)(ioHandler&,const bool&) =
      paretoFront::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool locationSearchEvaluation::parameters::read(ioHandler& handler,
                                        const bool& complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif



  // --------------------------------------------------
  // locationSearchEvaluation
  // --------------------------------------------------

  // default constructor
  locationSearchEvaluation::locationSearchEvaluation()
    : paretoFront() {
  }

  // default constructor
  locationSearchEvaluation::locationSearchEvaluation(const parameters& par)
    : paretoFront(par) {
  }


  // copy constructor
  locationSearchEvaluation::locationSearchEvaluation(
                 const locationSearchEvaluation& other) : paretoFront(other) {
  }

  // destructor
  locationSearchEvaluation::~locationSearchEvaluation() {
  }

  // returns the name of this type
  const char* locationSearchEvaluation::getTypeName() const {
    return "locationSearchEvaluation";
  }

  functor* locationSearchEvaluation::clone() const {
    return new locationSearchEvaluation(*this);
  }

  // return parameters
  const locationSearchEvaluation::parameters& 
  locationSearchEvaluation::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  std::string locationSearchEvaluation::getMaskName(const std::string& imgName,
                                                    const std::string& postfix
                                                    ) const {
    std::string file(imgName);
    std::string::size_type pos = file.rfind('.');
    if (pos != std::string::npos) {
      file.insert(pos,postfix);
    } else {
      file+=postfix;
    }
    return file;
  }

  bool locationSearchEvaluation::setParameters(const functor::parameters& par){
    if (paretoFront::setParameters(par)) {
      const parameters& param = getParameters();

      // initialize loader
      loadImageList loader(param.images);
      loadImage iloader;

      image img;
      channel8 mask;
      palette pal;
      int i;
      bool error = false;
      std::string file;
      channel emptyChannel;
      channel8 emptyMask;

      // the channels to be read (imageData) are a class attribute:
      imageData.clear();
      imageData.reserve(loader.size());

      maskData.clear();
      maskData.reserve(loader.size());

      i=0;
      while (loader.hasNext()) {
        file = loader.getNextFileName();
        if (loader.apply(img)) {
          // read the image
          imageData.push_back(emptyChannel);
          imageData[i].castFrom(img);

          maskData.push_back(emptyMask);
          // read the mask
          if (iloader.load(getMaskName(file,param.postfix),mask,pal) &&
              (mask.size() == img.size())) {
            mask.detach(maskData[i]);
          }
          
          i++;
        } else {
          if (!error) {
            setStatusString("Following images could not be read:\n");
            error = true;
          }

          file += "\n  ";
          appendStatusString(file.c_str());
        }
      }
      
      if ( !error && (i==0) ) {
        setStatusString("No images could be found.  Maybe wrong path?");
        return false;
      }

      return !error;
    }

    return false;
  }

  //
  // global private enum, with the number of pixels per parameter
  // it is global to define this constants for all method that need them,
  //
  struct locationSearchEvaluation::bits {
    enum {
      NumLevels=5,
      UpsampleFirstLevel=2,
      Factor=3,
      Gaussian=1,
      InterpolatorType=1,
      InterpolateMaxima=1,
      CheckILMaxima=1,
      SmoothingGaussianVariance=8,
      SpatialMaximumNeighborhoodSize=2,
      PeakSelectionMode=2,
      SaliencyThreshold=8,
      EdgenessThreshold=8,
      DensityFilter=1,
      DensityKernelRadius=8,
      DensityThreshold=8,
      OrientationWindowRadius=2,
      SigmaFactor=8,
      MultiOrienThreshold=8,
      AverageOrientation=1,
      DoGLevelDistance=2,
      DoGKernelVariance=8
    };

    static int total() {
      return (NumLevels                      +
              UpsampleFirstLevel             +
              Factor                         +
              Gaussian                       +
              InterpolatorType               +
              InterpolateMaxima              +
              CheckILMaxima                  +
              SmoothingGaussianVariance      +
              SpatialMaximumNeighborhoodSize +
              PeakSelectionMode              +
              SaliencyThreshold              +
              EdgenessThreshold              +
              DensityFilter                  +
              DensityKernelRadius            +
              DensityThreshold               +
              OrientationWindowRadius        +
              SigmaFactor                    +
              MultiOrienThreshold            +
              AverageOrientation             +
              DoGLevelDistance               +
              DoGKernelVariance); // 70 bits
    }
    
  };

  /*
   * Convert a binary-chain representation of a chromosome to a valid
   * parameter object.
   */
  bool locationSearchEvaluation::chromosomeToPhenotype(
                                        const chromosome& genotype,
                                        functor::parameters& phenotype) const {
    
    const parameters& par = getParameters();
    pyramidLocationSearch::parameters* phen = 
      dynamic_cast<pyramidLocationSearch::parameters*>(&phenotype);

    if (isNull(phen)) {
      return false;
    }

    int pos=0;
    int ires,theMin,theMax;
    uint32 ures;
    double dres;

    // numLevels
    pos = binToInt(genotype,pos,bits::NumLevels,
                   par.minValues.numLevels,par.maxValues.numLevels,ires);
    phen->numLevels = ires;

    // upsampleFirstLevel
    pos = binToInt(genotype,pos,bits::UpsampleFirstLevel,
                   par.minValues.upsampleFirstLevel,
                   par.maxValues.upsampleFirstLevel,ires);
    phen->upsampleFirstLevel = ires;
    
    // factor
    theMin = iround(-log(2.0)/log(par.minValues.pyramidParameters.factor));
    theMax = iround(-log(2.0)/log(par.maxValues.pyramidParameters.factor));
    minmax(theMin,theMax);

    pos = binToInt(genotype,pos,bits::Factor,theMin,theMax,ires);
    phen->pyramidParameters.factor = pow(0.5f,1.0f/ires);

    // gaussian
    if (par.minValues.pyramidParameters.gaussian !=
        par.maxValues.pyramidParameters.gaussian) {
      phen->pyramidParameters.gaussian = genotype[pos];
    } else {
      phen->pyramidParameters.gaussian =
        par.minValues.pyramidParameters.gaussian;
    }
    pos++;

    // interpolator type
    if (par.minValues.pyramidParameters.interpolatorType !=
        par.maxValues.pyramidParameters.interpolatorType) {
      phen->pyramidParameters.interpolatorType = 
        (genotype[pos]) ? BilinearInterpolator : BiquadraticInterpolator;
    } else {
      phen->pyramidParameters.interpolatorType = 
        par.minValues.pyramidParameters.interpolatorType;
    }
    pos++;

    // interpolateMaxima
    if (par.minValues.interpolateMaxima !=
        par.maxValues.interpolateMaxima) {
      phen->interpolateMaxima = genotype[pos];
    } else {
      phen->interpolateMaxima = par.minValues.interpolateMaxima;
    }
    pos++;

    // checkInterLevelMaxima
    if (par.minValues.checkInterLevelMaxima !=
        par.maxValues.checkInterLevelMaxima) {
      phen->checkInterLevelMaxima = genotype[pos];
    } else {
      phen->checkInterLevelMaxima = par.minValues.checkInterLevelMaxima;
    }
    pos++;

    // smoothingGaussianVariance
    pos = binToDouble(genotype,pos,bits::SmoothingGaussianVariance,
                      par.minValues.smoothingGaussianVariance,
                      par.maxValues.smoothingGaussianVariance,
                      dres);
    phen->smoothingGaussianVariance = static_cast<float>(dres);

    // spatialMaximumNeighborhoodSize
    pos = binToInt(genotype,pos,bits::SpatialMaximumNeighborhoodSize,
                   par.minValues.spatialMaximumNeighborhoodSize,
                   par.maxValues.spatialMaximumNeighborhoodSize,
                   ires);
    phen->spatialMaximumNeighborhoodSize = 1 + 2*(ires/2); // ensure odd

    // peakSelectionMode
    pos = binToUInt(genotype,pos,bits::PeakSelectionMode,ures);

    // restrict the previous values according to the min/max values
    switch (par.minValues.peakSelectionMode) {
      case pyramidLocationSearch::parameters::GradientMagnitude:
        theMin = 0;
        break;
      case pyramidLocationSearch::parameters::Laplacian:
        theMin = 1;
        break;
      case pyramidLocationSearch::parameters::Harris:
        theMin = 2;
        break;
      case pyramidLocationSearch::parameters::DoG:
        theMin = 3;
        break;
    }

    switch (par.maxValues.peakSelectionMode) {
      case pyramidLocationSearch::parameters::GradientMagnitude:
        theMax = 0;
        break;
      case pyramidLocationSearch::parameters::Laplacian:
        theMax = 1;
        break;
      case pyramidLocationSearch::parameters::Harris:
        theMax = 2;
        break;
      case pyramidLocationSearch::parameters::DoG:
        theMax = 3;
        break;
    }

    minmax(theMin,theMax); // ensure that these names are what they say!
    
    ires = theMax-theMin+1;
    ures = static_cast<uint32>((ures-theMin)%ires + theMin);
    
    switch (ures) {
      case 0:
        phen->peakSelectionMode = 
          pyramidLocationSearch::parameters::GradientMagnitude;
        break;
      case 1:
        phen->peakSelectionMode = 
          pyramidLocationSearch::parameters::Laplacian;
        break;
      case 2:
        phen->peakSelectionMode = 
          pyramidLocationSearch::parameters::Harris;
        break;
      case 3:
        phen->peakSelectionMode = 
          pyramidLocationSearch::parameters::DoG;
        break;        
    }
    

    // saliencyThreshold
    pos = binToDouble(genotype,pos,
                      bits::SaliencyThreshold,
                      par.minValues.saliencyThreshold.at(0),
                      par.maxValues.saliencyThreshold.at(0),
                      dres);
    phen->saliencyThreshold.resize(1,static_cast<float>(dres),false,true);

    // edgenessThreshold
    pos = binToDouble(genotype,pos,bits::EdgenessThreshold,
                      par.minValues.edgenessThreshold,
                      par.maxValues.edgenessThreshold,
                      dres);
    phen->edgenessThreshold = static_cast<float>(dres);

    // densityFilter
    if (par.minValues.densityFilter !=
        par.maxValues.densityFilter) {
      phen->densityFilter = genotype[pos];
    } else {
      phen->densityFilter = par.minValues.densityFilter;
    }
    pos++;

    // densityKernelRadius
    pos = binToInt(genotype,pos,bits::DensityKernelRadius,
                   par.minValues.densityKernelRadius,
                   par.maxValues.densityKernelRadius,ires);
    phen->densityKernelRadius = ires;
    
    // densityThreshold
    pos = binToDouble(genotype,pos,
                      bits::DensityThreshold,
                      par.minValues.densityThreshold.at(0),
                      par.maxValues.densityThreshold.at(0),
                      dres);
    phen->densityThreshold.resize(1,static_cast<float>(dres),false,true);

    // orientationRadius
    pos = binToInt(genotype,pos,bits::OrientationWindowRadius,
                   par.minValues.orientationWindowRadius,
                   par.maxValues.orientationWindowRadius,ires);
    phen->orientationWindowRadius = ires;

    // orientation sigma factor
    pos = binToDouble(genotype,pos,bits::SigmaFactor,
                      par.minValues.orientationSigmaFactor,
                      par.maxValues.orientationSigmaFactor,
                      dres);
    phen->orientationSigmaFactor = static_cast<float>(dres);

    // multi orientation threshold
    pos = binToDouble(genotype,pos,bits::MultiOrienThreshold,
                      par.minValues.multiOrientationThreshold,
                      par.maxValues.multiOrientationThreshold,
                      dres);
    phen->multiOrientationThreshold = static_cast<float>(dres);

    // orientation average 
    if (par.minValues.averageOrientation != par.maxValues.averageOrientation) {
      phen->averageOrientation = genotype[pos];
    } else {
      phen->averageOrientation = par.minValues.averageOrientation;
    }
    pos++;

    // DoG level distance
    pos = binToInt(genotype,pos,bits::DoGLevelDistance,
                   par.minValues.doGLevelDistance,
                   par.maxValues.doGLevelDistance,ires);
    phen->doGLevelDistance = ires;
    
    // DoG kernel variance
    pos = binToDouble(genotype,pos,bits::DoGKernelVariance,
                      par.minValues.doGKernelVariance,
                      par.maxValues.doGKernelVariance,
                      dres);
    phen->doGKernelVariance = static_cast<float>(dres);

    assert (pos == bits::total());

    return true;
  }

  /*
   * Return a fresh allocated parameters for the evaluated functor, which is
   * equivalent to the given genotype.
   */
  functor::parameters* 
  locationSearchEvaluation::chromosomeToPhenotype(const chromosome& genotype
                                                  ) const {

    pyramidLocationSearch::parameters par;
    chromosomeToPhenotype(genotype,par);

    return par.clone();
  }

  /*
   * Convert a valid parameters object (phenotype) into binary-chain
   * representation of a chromosome.
   */
  bool locationSearchEvaluation::phenotypeToChromosome(
                                          const functor::parameters& phenotype,
                                          chromosome& genotype) const {

    genotype.resize(bits::total());

    const parameters& par = getParameters();
    const pyramidLocationSearch::parameters* phen = 
      dynamic_cast<const pyramidLocationSearch::parameters*>(&phenotype);

    if (isNull(phen)) {
      return false;
    }

    int pos=0;
    int ires,theMin,theMax;
    uint32 ures(0);

    // numLevels
    pos = intToBin(phen->numLevels,pos,bits::NumLevels,
                   par.minValues.numLevels,par.maxValues.numLevels,genotype);

    // upsampleFirstLevel
    pos = intToBin(phen->upsampleFirstLevel,pos,bits::UpsampleFirstLevel,
                   par.minValues.upsampleFirstLevel,
                   par.maxValues.upsampleFirstLevel,genotype);
    // factor
    theMin = iround(-log(2.0)/log(par.minValues.pyramidParameters.factor));
    theMax = iround(-log(2.0)/log(par.maxValues.pyramidParameters.factor));
    ires   = iround(-log(2.0)/log(phen->pyramidParameters.factor));
    minmax(theMin,theMax);

    pos = intToBin(ires,pos,bits::Factor,theMin,theMax,genotype);

    // gaussian
    if (par.minValues.pyramidParameters.gaussian !=
        par.maxValues.pyramidParameters.gaussian) {
      genotype[pos] = phen->pyramidParameters.gaussian;
    } else {
      genotype[pos] = par.minValues.pyramidParameters.gaussian;
    }
    pos++;

    // interpolator type
    if (par.minValues.pyramidParameters.interpolatorType !=
        par.maxValues.pyramidParameters.interpolatorType) {
      genotype[pos] = (phen->pyramidParameters.interpolatorType ==
                       BiquadraticInterpolator);
    } else {
      genotype[pos] = (par.minValues.pyramidParameters.interpolatorType == 
                       BiquadraticInterpolator);
    }
    pos++;

    // interpolateMaxima
    if (par.minValues.interpolateMaxima !=
        par.maxValues.interpolateMaxima) {
      genotype[pos] = phen->interpolateMaxima;
    } else {
      genotype[pos] = par.minValues.interpolateMaxima;
    }
    pos++;

    // checkInterLevelMaxima
    if (par.minValues.checkInterLevelMaxima !=
        par.maxValues.checkInterLevelMaxima) {
      genotype[pos] = phen->checkInterLevelMaxima;
    } else {
      genotype[pos] = par.minValues.checkInterLevelMaxima;
    }
    pos++;

    // smoothingGaussianVariance
    pos = doubleToBin(phen->smoothingGaussianVariance,
                      pos,bits::SmoothingGaussianVariance,
                      par.minValues.smoothingGaussianVariance,
                      par.maxValues.smoothingGaussianVariance,
                      genotype);

    // spatialMaximumNeighborhoodSize
    pos = intToBin(phen->spatialMaximumNeighborhoodSize,
                   pos,bits::SpatialMaximumNeighborhoodSize,
                   par.minValues.spatialMaximumNeighborhoodSize,
                   par.maxValues.spatialMaximumNeighborhoodSize,
                   genotype);

    // peakSelectionMode
    switch (phen->peakSelectionMode) {
      case pyramidLocationSearch::parameters::GradientMagnitude:
        ures = 0;
        break;
      case pyramidLocationSearch::parameters::Laplacian:
        ures = 1;
        break;
      case pyramidLocationSearch::parameters::Harris:
        ures = 2;
        break;
      case pyramidLocationSearch::parameters::DoG:
        ures = 3;
        break;
    }
    pos = uintToBin(ures,pos,bits::PeakSelectionMode,genotype);
    
    // saliencyThreshold
    pos = doubleToBin(phen->saliencyThreshold.at(0),
                      pos,bits::SaliencyThreshold,
                      par.minValues.saliencyThreshold.at(0),
                      par.maxValues.saliencyThreshold.at(0),
                      genotype);
    
    // edgenessThreshold
    pos = doubleToBin(phen->edgenessThreshold,pos,bits::EdgenessThreshold,
                      par.minValues.edgenessThreshold,
                      par.maxValues.edgenessThreshold,
                      genotype);

    
    // densityFilter
    if (par.minValues.densityFilter !=
        par.maxValues.densityFilter) {
      genotype[pos] = phen->densityFilter;
    } else {
      genotype[pos] = par.minValues.densityFilter;
    }
    pos++;

    // densityKernelRadius
    pos = intToBin(phen->densityKernelRadius,
                   pos,bits::DensityKernelRadius,
                   par.minValues.densityKernelRadius,
                   par.maxValues.densityKernelRadius,genotype);

    // densityThreshold
    pos = doubleToBin(phen->densityThreshold.at(0),
                      pos,bits::DensityThreshold,
                      par.minValues.densityThreshold.at(0),
                      par.maxValues.densityThreshold.at(0),
                      genotype);

    // orientationRadius
    pos = intToBin(phen->orientationWindowRadius,
                   pos,bits::OrientationWindowRadius,
                   par.minValues.orientationWindowRadius,
                   par.maxValues.orientationWindowRadius,genotype);

    // orientationSigmaFactor
    pos = doubleToBin(phen->orientationSigmaFactor,pos,bits::SigmaFactor,
                      par.minValues.orientationSigmaFactor,
                      par.maxValues.orientationSigmaFactor,
                      genotype);
    
    // multiOrientationThreshold
    pos = doubleToBin(phen->multiOrientationThreshold,pos,
                      bits::MultiOrienThreshold,
                      par.minValues.multiOrientationThreshold,
                      par.maxValues.multiOrientationThreshold,
                      genotype);

    // averageOrientation
    if (par.minValues.averageOrientation != par.maxValues.averageOrientation) {
      genotype[pos] = phen->averageOrientation;
    } else {
      genotype[pos] = par.minValues.averageOrientation;
    }
    pos++;

    // DoG level distance
    pos = intToBin(phen->doGLevelDistance,
                   pos,bits::DoGLevelDistance,
                   par.minValues.doGLevelDistance,
                   par.maxValues.doGLevelDistance,genotype);

    // DoG kernel variance
    pos = doubleToBin(phen->doGKernelVariance,
                      pos,bits::DoGKernelVariance,
                      par.minValues.doGKernelVariance,
                      par.maxValues.doGKernelVariance,genotype);

    assert (pos == bits::total());

    return true;
  }
   
  /*
   * Return the length in bits for a chromosome.
   *
   * This method needs to be reimplemented, in order to get some 
   * default implementations to work.
   */
  int locationSearchEvaluation::getChromosomeSize() const {
    return bits::total();
  }

  // -----------------------------------------------------------------------
  // Static member initialization
  // -----------------------------------------------------------------------
  const int locationSearchEvaluation::totalFitnessDimensionality = 16;

  /*
   * Evaluate Chromosome
   *
   * This method is one of the most important ones for the pareto evaluation.
   * Its task is to produce a multidimensional fitness measure for a given
   * chromosome.
   *
   * It returns true if the evaluation was successful, of false if the
   * phenotype represents some invalid parameterization.  It is highly 
   * recomended that the mutation and crossover methods are reimplemented to 
   * avoid invalid parameterizations.
   */
  bool locationSearchEvaluation::evaluateChromosome(
                                                const chromosome& individual,
                                                dvector& fitness) {
    
    pyramidLocationSearch::parameters plsPar;
    chromosomeToPhenotype(individual,plsPar);
    dmatrix mfitness;

#ifdef _LTI_DEBUG
#ifdef _LTI_SAVE_PARAM
    // for debug purposes, if the system crash the given file will have
    // the parameters currently under evaluation, so that it can be possible
    // to isolate the case:
    char buffer[128];
    sprintf(buffer,"lti_lseval_dbg_%i.txt",getpid());
    std::ofstream _dbg_out(buffer);
    lispStreamHandler lsh(_dbg_out);
    plsPar.write(lsh);
    _dbg_out << std::endl;
    _dbg_out.close();
#endif
#endif  

    if (evaluate(plsPar,mfitness,true)) {
      const parameters& par = getParameters();
      if (fitness.size() != par.fitnessSpaceDimensionality) {
        fitness.resize(par.fitnessSpaceDimensionality,double(),false,false);
      }

      int j;
      j = 0;

      if ((par.numberOfLocations) &&
          (j<par.fitnessSpaceDimensionality)) {
        fitness.at(j)=mfitness.at(0,IdxNumberOfLocations);
        j++;
      } 

      if ((par.locationsReciprocal) &&
          (j<par.fitnessSpaceDimensionality)) {
        fitness.at(j)=mfitness.at(0,IdxLocationsRecip);
        j++;
      } 

      if ((par.occupiedLevels) &&
          (j<par.fitnessSpaceDimensionality)) {
        fitness.at(j)=mfitness.at(0,IdxOccupiedLevels);
        j++;
      }

      if ((par.time) &&
          (j<par.fitnessSpaceDimensionality)) {
        fitness.at(j)=mfitness.at(0,IdxTime);
        j++;
      }

      if ((par.scaleRepeatability) && 
          (j<par.fitnessSpaceDimensionality)) {
        if (par.considerAngle) {
          fitness.at(j)=mfitness.at(0,IdxScalePosRep);
        } else {
          fitness.at(j)=mfitness.at(0,IdxScaleARep);
        }
        j++;
      }

      if ((par.rotationRepeatability) && 
          (j<par.fitnessSpaceDimensionality)) {
        if (par.considerAngle) {
          fitness.at(j)=mfitness.at(0,IdxRotationPosRep);
        } else {
          fitness.at(j)=mfitness.at(0,IdxRotationARep);
        }
        j++;
      }

      if ((par.rotationScaleRepeatability) && 
          (j<par.fitnessSpaceDimensionality)) {
        if (par.considerAngle) {
          fitness.at(j)=mfitness.at(0,IdxRSPosRep);
        } else {
          fitness.at(j)=mfitness.at(0,IdxRSARep);
        }
        j++;
      }

      // 
      if ((par.scaleStable) && 
          (j<par.fitnessSpaceDimensionality)) {
        if (par.considerAngle) {
          fitness.at(j)=mfitness.at(0,IdxScalePosStable);
        } else {
          fitness.at(j)=mfitness.at(0,IdxScaleAStable);
        }
        j++;
      }

      if ((par.rotationStable) && 
          (j<par.fitnessSpaceDimensionality)) {
        if (par.considerAngle) {
          fitness.at(j)=mfitness.at(0,IdxRotationPosStable);
        } else {
          fitness.at(j)=mfitness.at(0,IdxRotationAStable);
        }
        j++;
      }

      if ((par.rotationScaleStable) && 
          (j<par.fitnessSpaceDimensionality)) {
        if (par.considerAngle) {
          fitness.at(j)=mfitness.at(0,IdxRSPosStable);
        } else {
          fitness.at(j)=mfitness.at(0,IdxRSAStable);
        }
        j++;
      }

    } else {
      return false;
    }

    return true;
  }

  
  bool locationSearchEvaluation::evaluate(
                                const pyramidLocationSearch::parameters& param,
                                dmatrix& fitness,
                                const bool onlyMeanAndVariance) {
    // const parameters& par = getParameters();

    image img;
    channel chnl;

    // set the location parameters only once
    locationSearcher.setParameters(param);

    const int size = static_cast<int>(imageData.size());

    if (size == 0) {
      // the statusString were initialize while setting the parameters!
      return false;
    }

    // assume all images are ok
    fitness.resize(size,totalFitnessDimensionality);

    int i=0; // i counts the number of successful readed images
    char buffer[255];

    // for all channels read in setParameters()
    while (i<size) {
      if (validProgressObject(2)) {
        sprintf(buffer,"Image %i/%i",i+1,size);
        getProgressObject().substep(2,buffer);        
      }
      evaluate(imageData[i],maskData[i],fitness.getRow(i));
      ++i;
    }

    // compute the statistics if desired
    if (onlyMeanAndVariance) {
      serialVectorStats<double> stats;
      stats.considerRows(fitness);
      dmatrix mstats(false,2,fitness.columns());
      stats.getMean(mstats.getRow(0));
      stats.getVariance(mstats.getRow(1));
      mstats.detach(fitness);
    }

    return true;
  }

  bool locationSearchEvaluation::evaluate(const channel& chnl,
                                const pyramidLocationSearch::parameters& param,
                                      dvector& fitness) {
    fitness.resize(totalFitnessDimensionality,0.0,false,true);
    channel8 emptyMask;
    locationSearcher.setParameters(param);
    return evaluate(chnl,emptyMask,fitness);
  }



  bool locationSearchEvaluation::evaluate(const channel& chnl,
                                          const channel8& mask,
                                          dvector& fitness) {

    // original locations
    std::vector<location> olocs;

    // locations of the transformed channel
    std::list<location> tlocs;
    int tlocsSize;

    // back transformed locations
    std::vector<location> blocs;

    // transformed channel
    channel tchnl;

    // the geometric transform functor to scale and rotate the images.
    geometricTransform gt;
    geometricTransform::parameters gtPar;

    gtPar.keepDimensions = false;
    gtPar.boundaryType = Constant;
   
    const parameters& par = getParameters();

    int i,j;
    point offset,pt;
    timer chronos;
    float angle,scale;
    float fa,ta,fs,ts,sa,ss;
    std::vector<location>::iterator vit;
    std::list<location>::iterator lit;
    char buffer[256];

    // Complete interval check
    const float firstAngle = (par.firstAngle > constants<float>::TwoPi()) ?
      degToRad(par.firstAngle) : par.firstAngle;
    const float lastAngle = (par.lastAngle > constants<float>::TwoPi()) ?
      degToRad(par.lastAngle) : par.lastAngle;
    const float stepAngle = (par.stepAngle > constants<float>::TwoPi()) ?
      degToRad(par.stepAngle) : par.stepAngle;

    minmax(firstAngle,lastAngle,fa,ta);
    minmax(par.firstScaling,par.lastScaling,fs,ts);
    sa = (stepAngle > 0) ? stepAngle : -stepAngle;
    ss = (par.stepScaling > 1) ? par.stepScaling : 1.0f/par.stepScaling;

    // avoid infinite loops
    if (sa == 0.0f) {
      sa = (fa == ta) ? 1.0 : (ta-fa);
    }

    // avoid infinite loops
    if (ss == 1.0f) {
      ss = (fs == ts) ? 2.0f : ts/fs;
    }
   
    // temporary map with the locations sorted per level
    std::map<int,std::list<location> > olocsMap;

    // compute the locations for the original image
    // (the parameters were already set by the caller method)
    int totalLocs;
    chronos.start();
    locationSearcher.apply(chnl,olocsMap,totalLocs);
    chronos.stop();

    // convert the map into a vector.
    std::map<int,std::list<location> >::iterator mit;
    olocs.clear();
    olocs.reserve(totalLocs);

    int lastOccLevel = 0;

    if (mask.empty()) {
      // mask empty => consider everything!

      // for each level
      for (mit = olocsMap.begin();mit!=olocsMap.end();++mit) {
        if (!((*mit).second.empty())) { // if level is not empty
          // insert all elements of that level in the "original locs" list
          olocs.insert(olocs.end(),
                       (*mit).second.begin(),
                       (*mit).second.end());
          lastOccLevel=(*mit).first;
        }
      }
    } else {
      // use only those locations within the masked subset!

      // for each level
      for (mit = olocsMap.begin();mit!=olocsMap.end();++mit) {
        if (!((*mit).second.empty())) { // if level is not empty
          // insert all elements of that level in the "original locs" list

          std::list<location>& levelList = (*mit).second;
          for (lit = levelList.begin();lit!=levelList.end();++lit) {
            pt.castFrom((*lit).position);
            if ((static_cast<uint32>(pt.x) <
                 static_cast<uint32>(mask.columns())) &&
                (static_cast<uint32>(pt.y) < 
                 static_cast<uint32>(mask.rows())) &&
                (mask.at(pt) != 0)) {
              // insert location since mask is not zero at the position
              olocs.push_back(*lit);
            }
          }

          lastOccLevel=(*mit).first;
        }
      }
      
    }

    // To efficiently search for the nearest locations use a kd-tree,
    // where the locations are going to be searched by their position in
    // the coordinates of the original image.
    // the data corresponds to the index of the location in the vector of
    // locations
    kdTree<tpoint<float>,int> otree;

    for (i=0,vit=olocs.begin();vit!=olocs.end();++vit,++i) {
      otree.add((*vit).position,i);
    }

    otree.build(32); // use a bucket of 32 (more efficient)

    // insert the first statistics in the results vector
    fitness.at(IdxTime) = 1000000.0/chronos.getTime(); // locs per second
    fitness.at(IdxNumberOfLocations) = static_cast<double>(olocs.size());
    fitness.at(IdxLocationsRecip) = 1.0/(1.0+olocs.size());
    fitness.at(IdxOccupiedLevels) = double(lastOccLevel+1.0)/
      locationSearcher.getChannelPyramid().size();

    // now to the expensive evaluation of rotation and scale invariance...

    hMatrix3D<float> matT;
    location loc;

    unsigned int oldCapacity;
    int posStable,rotStable;
    int totalPosStable,totalRotStable,totalPosUnstable,totalRotUnstable;
    double total;
    int n;

#if defined _LTI_DEBUG && (_LTI_DEBUG > 2)
    static viewer viewo("Original");
    static viewer viewt("Transformed");
    static viewer viewb("Back-transformed");

    image canvas;
    canvas.castFrom(chnl);
    draw<rgbPixel> painter;
    painter.use(canvas);
    painter.setColor(rgbPixel(255,128,64));
    for (unsigned int ii=0;ii<olocs.size();++ii) {
      painter.set(olocs[ii]);
    }

    viewo.show(canvas);
#endif


    for (j=0;j<3;++j) {
      if ( (j==0) &&
           !par.rotationScaleRepeatability ) {
        // not necessary to compute the first stuff
        
        // fix the intervals for the next "scale only" test
        fa=ta=0.0f;

        continue;
      }
      if ( (j==1) &&
           !par.scaleRepeatability ) {
        // not necessary to compute the scaling stuff

        // fix the intervals for the next "rotation only" test
        minmax(firstAngle,lastAngle,fa,ta);
        fs=ts=1.0f;

        continue;
      }

      if ( (j==2) &&
           !par.rotationRepeatability ) {
        // not necessary to compute the scaling stuff
        continue;
      }

      totalPosStable = totalPosUnstable = 0;
      totalRotStable = totalRotUnstable = 0;
      n = 0;

      // scale/rotate image
      for (scale=fs;scale<=ts;scale*=ss) {
        if (validProgressObject(3)) {
          sprintf(buffer,"Scale %f",scale);
          getProgressObject().substep(3,buffer);        
        }

        for (angle=fa;angle<=ta;angle+=sa) {
          if (validProgressObject(4)) {
            sprintf(buffer,"Rotation %f",radToDeg(angle));
            getProgressObject().substep(4,buffer);        
          }

          // debug info
          _lti_debug3("  scale=" << scale << 
                      " \tangle=" << radToDeg(angle) << std::endl);

          // set desired rotation and scale
          gtPar.clear();
          gtPar.rotate(angle);
          gtPar.scale(scale);
          gt.setParameters(gtPar);
          
          // rotate the image itself
          gt.apply(chnl,tchnl,offset);
          
          // create the locations for the transformed channel
          locationSearcher.apply(tchnl,tlocs);
          
          // map the transformed locations into the original coordinate system,
          // to simplify searching for an equivalent match.
          // revert the offset process:
          matT.unit();
          matT.translate(hPoint3D<float>(static_cast<float>(-offset.x),
                                         static_cast<float>(-offset.y),
                                         0.0f));
          matT.multiply(gtPar.transMatrix);
          gtPar.transMatrix.invert(matT);
          gt.setParameters(gtPar);
          
          // transform all new locations to the coordinate system of the
          // original image
          tlocsSize = 0;
          oldCapacity = blocs.capacity();
          blocs.clear(); // keep capacity!
          blocs.reserve(oldCapacity);

          for (lit=tlocs.begin();lit!=tlocs.end();++lit) {
            gt.apply((*lit).position,loc.position);
            loc.angle = (*lit).angle - angle;
            loc.radius = (*lit).radius/scale;
            blocs.push_back(loc);
            tlocsSize++;
          }
          
          compare(otree,olocs,blocs,posStable,rotStable);

          totalPosStable += posStable;
          totalRotStable += rotStable;
          totalPosUnstable += max(0,totalLocs - posStable) +
                              (tlocsSize - posStable);
          totalRotUnstable += max(0,totalLocs - rotStable) +
                              (tlocsSize - rotStable);
          n++; // number of test images until now
#if defined _LTI_DEBUG && (_LTI_DEBUG > 2)
          canvas.castFrom(tchnl);
          painter.set(tlocs);
          viewt.show(canvas);
          canvas.castFrom(chnl);
          std::list<location> listOfLocs;
          std::vector<location>::const_iterator vlit;
          for (vlit=blocs.begin();vlit!=blocs.end();++vlit) {
            listOfLocs.push_back(*vlit);
          }
          painter.set(listOfLocs);
          viewb.show(canvas);
#  if _LTI_DEBUG > 3
          viewo.waitKey();
#  endif
#endif

        }
      }

      switch(j) {
        case 0:
          total = double(totalPosStable+totalPosUnstable);
          if (total == 0.0) {
            total = 1.0; // avoid division by 0
          }
          fitness.at(IdxRSPosRep) = double(totalPosStable)/total;
          // mean over n test images:
          fitness.at(IdxRSPosStable) = double(totalPosStable)/n; 
          
          total = double(totalRotStable+totalRotUnstable);
          if (total == 0.0) {
            total = 1.0; // avoid division by 0
          }
          fitness.at(IdxRSARep) = double(totalRotStable)/total;
          fitness.at(IdxRSAStable) = double(totalRotStable)/n;

          // fix the intervals for the next "scale only" text
          fa=ta=0.0f;
          break;
        case 1:
          total = double(totalPosStable+totalPosUnstable);
          if (total == 0.0) {
            total = 1.0; // avoid division by 0
          }
          fitness.at(IdxScalePosRep) = double(totalPosStable)/total;
          fitness.at(IdxScalePosStable) = double(totalPosStable)/n;

          total = double(totalRotStable+totalRotUnstable);
          if (total == 0.0) {
            total = 1.0; // avoid division by 0
          }
          fitness.at(IdxScaleARep) = double(totalRotStable)/total;
          fitness.at(IdxScaleAStable) = double(totalRotStable)/n;

          // fix the intervals for the next "rotation only" test
          minmax(firstAngle,lastAngle,fa,ta);
          fs=ts=1.0f;
          break;
        case 2:
          total = double(totalPosStable+totalPosUnstable);
          if (total == 0.0) {
            total = 1.0; // avoid division by 0
          }
          fitness.at(IdxRotationPosRep) = double(totalPosStable)/total;
          fitness.at(IdxRotationPosStable) = double(totalPosStable)/n;

          total = double(totalRotStable+totalRotUnstable);
          if (total == 0.0) {
            total = 1.0; // avoid division by 0
          }
          fitness.at(IdxRotationARep)    = double(totalRotStable)/total;
          fitness.at(IdxRotationAStable) = double(totalRotStable)/n;

          break;
        default:
          assert(false); // we shouldn't come here at all!
          break;
      }
    }
    
    return true;
  }

  bool locationSearchEvaluation::compare(const kdTree<tpoint<float>,
                                                      int>& otree,
                                         const std::vector<location>& olocs,
                                         const std::vector<location>& blocs,
                                         int& posStable,
                                         int& rotStable) const {
    
    const parameters& par = getParameters();

    // degrees or radians?
    const float angleTolerance =
      (par.angleTolerance > constants<float>::TwoPi()) ?
      degToRad(par.angleTolerance) : par.angleTolerance;

    typedef std::multimap<float,kdTree<tpoint<float>,int>::element*> mmap_type;

    mmap_type candidates;
    mmap_type::iterator mit;

    unsigned int ui;
    const float locationRelativeRadius =
      locationSearcher.getParameters().locationRelativeRadius;

    float minRad,maxRad;
    bool match;
    posStable = rotStable = 0;
    const unsigned int bsize = blocs.size();

    vector<ubyte> oflags(olocs.size(),ubyte(1));

    float smallestAngularError;
    int bestMatch;

    // for each element of the back-transformed locations
    for (ui=0;ui<bsize;++ui) {
      otree.searchWithin(blocs[ui].position,
                         par.positionTolerance*(blocs[ui].radius/
                                                locationRelativeRadius),
                         candidates);

      // from all candidates, we need only one!
      mit=candidates.begin();
      match=false;
      smallestAngularError = constants<float>::TwoPi();
      bestMatch = -1;
      while (mit!=candidates.end()) {
        // the elements in candidates are all within the positional tolerance
        // we need only to check if the radius is good enough.
        if (oflags[(*mit).second->data] == 0) {
          // location already used
          ++mit;
          continue;
        }
        minmax(blocs[ui].radius,olocs[(*mit).second->data].radius,
               minRad,maxRad);
        if (minRad > maxRad*par.scaleTolerance) {
          match=true;
          // check if also the angles are all right
          minRad = abs(blocs[ui].angle - olocs[(*mit).second->data].angle);
          if (minRad > static_cast<float>(Pi)) {
            minRad = static_cast<float>(2.0*Pi) - minRad;
          }

          if (minRad < smallestAngularError) {
            smallestAngularError = minRad;
            bestMatch = (*mit).second->data;
          }
        }
        ++mit;
      }

      if (match) {
        posStable++;
        oflags[bestMatch] = 0; // mark best match as used        
      }

      if (smallestAngularError <= angleTolerance) {
        rotStable++;
      }
    }

    return true;
  }

}

