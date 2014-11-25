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
 * file .......: ltiPyramidLocationSearch.cpp
 * authors ....: Frederik Lange, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 18.03.2003
 * revisions ..: $Id: ltiPyramidLocationSearch.cpp,v 1.5 2006/09/05 10:26:25 ltilib Exp $
 */

#include "ltiPyramidLocationSearch.h"

#include "ltiConvolution.h"
#include "ltiGaussKernels.h"
#include "ltiGradientKernels.h"

#include "ltiMaximumFilter.h"
#include "ltiMatrixInversion.h"
#include "ltiMeansFunctor.h"
#include "ltiHarrisCorners.h"
#include "ltiLaplacianKernel.h"
#include "ltiArray.h"
#include "ltiGaussian.h"
#include "ltiSort.h"
#include "ltiScaling.h"
#include "ltiArctanLUT.h"

#include <limits>

#undef _LTI_DEBUG
// #define _LTI_DEBUG 3
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#include "ltiViewer.h"
#include "ltiTimer.h"
#endif

namespace lti {

  // --------------------------------------------------
  // pyramidLocationSearch::parameters
  // --------------------------------------------------

  // default constructor
  pyramidLocationSearch::parameters::parameters() 
    : transform::parameters() {    

    numLevels = int(15);
    upsampleFirstLevel = 1;

    pyramidParameters.factor = static_cast<float>(pow(0.5,1.0/3.0));
    pyramidParameters.interpolatorType = BilinearInterpolator;
    pyramidParameters.gaussian = true;
    
    interpolateMaxima = true;
    checkInterLevelMaxima = false;
    smoothingGaussianVariance = 2.56f;
    locationsAtLevelZero = false;
    
    spatialMaximumNeighborhoodSize = int(3);
    peakSelectionMode = DoG;
    doGLevelDistance = 1;
    doGKernelVariance = 1.6f*1.6f;
    doGKernelSize = -1;
    saliencyThreshold.resize(1,float(0.1f),false,true);
    edgenessThreshold = 10.0f;

    densityFilter = false;
    densityKernelRadius = int(3);
    densityThreshold.resize(1,float(0.75f),false,true);
   
    orientationWindowRadius = int(3);
    orientationSigmaFactor = int(2);
    binsInOrientationHistogram = int(36);
    multiOrientationThreshold = float(0.8f);
    averageOrientation = false;

    locationRelativeRadius = float(7.0f);

  }

  // copy constructor
  pyramidLocationSearch::parameters::parameters(const parameters& other) {
    copy(other);
  }

  // destructor
  pyramidLocationSearch::parameters::~parameters() {
  }

  // get type name
  const char* pyramidLocationSearch::parameters::getTypeName() const {
    return "pyramidLocationSearch::parameters";
  }

  // copy member

  pyramidLocationSearch::parameters&
  pyramidLocationSearch::parameters::copy(const parameters& other) {
# if !defined(_LTI_MSC_6)
    // MS Visual C++ 6 is not able to compile this...
    transform::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    transform::parameters& (transform::parameters::* p_copy)
      (const transform::parameters&) =
      transform::parameters::copy;
    (this->*p_copy)(other);
# endif

    numLevels = other.numLevels;
    upsampleFirstLevel = other.upsampleFirstLevel;
    pyramidParameters.copy(other.pyramidParameters);
    interpolateMaxima = other.interpolateMaxima;
    checkInterLevelMaxima = other.checkInterLevelMaxima;
    smoothingGaussianVariance = other.smoothingGaussianVariance;
    locationsAtLevelZero = other.locationsAtLevelZero;

    spatialMaximumNeighborhoodSize = other.spatialMaximumNeighborhoodSize;
    peakSelectionMode = other.peakSelectionMode;

    doGLevelDistance = other.doGLevelDistance;
    doGKernelVariance = other.doGKernelVariance;
    doGKernelSize = other.doGKernelSize;

    saliencyThreshold = other.saliencyThreshold;
    edgenessThreshold = other.edgenessThreshold;

    densityFilter = other.densityFilter;
    densityKernelRadius = other.densityKernelRadius;
    densityThreshold = other.densityThreshold;


    orientationWindowRadius = other.orientationWindowRadius;
    orientationSigmaFactor = other.orientationSigmaFactor;
    binsInOrientationHistogram = other.binsInOrientationHistogram;
    multiOrientationThreshold = other.multiOrientationThreshold;
    averageOrientation = other.averageOrientation;

    locationRelativeRadius = other.locationRelativeRadius;

    return *this;
  }

  // alias for copy member
  pyramidLocationSearch::parameters&
    pyramidLocationSearch::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* pyramidLocationSearch::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# if !defined(_LTI_MSC_6)
  bool pyramidLocationSearch::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool pyramidLocationSearch::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"numLevels",numLevels);
      lti::write(handler,"upsampleFirstLevel",upsampleFirstLevel);
      lti::write(handler,"pyramidParameters",pyramidParameters);
      lti::write(handler,"interpolateMaxima",interpolateMaxima);
      lti::write(handler,"checkInterLevelMaxima",checkInterLevelMaxima);
      lti::write(handler,"smoothingGaussianVariance",
                 smoothingGaussianVariance);
      lti::write(handler,"locationsAtLevelZero",locationsAtLevelZero);

      lti::write(handler,"spatialMaximumNeighborhoodSize",
                 spatialMaximumNeighborhoodSize);
      switch (peakSelectionMode) {
        case GradientMagnitude:
          lti::write(handler,"peakSelectionMode","GradientMagnitude");
          break;
        case Laplacian:
          lti::write(handler,"peakSelectionMode","Laplacian");
          break;
        case Harris:
          lti::write(handler,"peakSelectionMode","Harris");
          break;
        case DoG:
          lti::write(handler,"peakSelectionMode","DoG");
          break;
        default:
          lti::write(handler,"peakSelectionMode","GradientMagnitude");
      }

      lti::write(handler,"doGLevelDistance",doGLevelDistance);
      lti::write(handler,"doGKernelVariance",doGKernelVariance);
      lti::write(handler,"doGKernelSize",doGKernelSize);

      lti::write(handler,"saliencyThreshold",saliencyThreshold);
      lti::write(handler,"edgenessThreshold",edgenessThreshold);

      lti::write(handler,"densityFilter",densityFilter);
      lti::write(handler,"densityKernelRadius",densityKernelRadius);
      lti::write(handler,"densityThreshold",densityThreshold);

      lti::write(handler,"orientationWindowRadius",orientationWindowRadius);
      lti::write(handler,"orientationSigmaFactor",orientationSigmaFactor);
      lti::write(handler,"binsInOrientationHistogram",
                 binsInOrientationHistogram);
      lti::write(handler,"multiOrientationThreshold",
                 multiOrientationThreshold);
      lti::write(handler,"averageOrientation",averageOrientation);

      lti::write(handler,"locationRelativeRadius",locationRelativeRadius);
      
    }

# if !defined(_LTI_MSC_6)
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::write(handler,false);
# else
    bool (transform::parameters::* p_writeMS)(ioHandler&,const bool) const =
      transform::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# if defined(_LTI_MSC_6)
  bool pyramidLocationSearch::parameters::write(ioHandler& handler,
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
# if !defined(_LTI_MSC_6)
  bool pyramidLocationSearch::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool pyramidLocationSearch::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      b = lti::read(handler,"numLevels",numLevels) && b;
      b = lti::read(handler,"upsampleFirstLevel",upsampleFirstLevel) && b;
      b = lti::read(handler,"pyramidParameters",pyramidParameters) && b;
      b = lti::read(handler,"interpolateMaxima",interpolateMaxima) && b;
      b = lti::read(handler,"checkInterLevelMaxima",checkInterLevelMaxima)&& b;
      b = lti::read(handler,"smoothingGaussianVariance",
                             smoothingGaussianVariance) && b;
      b = lti::read(handler,"locationsAtLevelZero",locationsAtLevelZero) && b;

      b = lti::read(handler,"spatialMaximumNeighborhoodSize",
                             spatialMaximumNeighborhoodSize) && b;

      std::string str;
      b = lti::read(handler,"peakSelectionMode",str) && b;
      if (str == "GradientMagnitude") {
        peakSelectionMode=GradientMagnitude;
      } else if (str == "Laplacian") {
        peakSelectionMode=Laplacian;        
      } else if (str == "Harris") {
        peakSelectionMode=Harris;        
      } else if (str == "DoG") {
        peakSelectionMode=DoG;
      } else {
        peakSelectionMode=GradientMagnitude;
        handler.appendStatusString("Unknown symbol for peakSelectionMode '" +
                                   str + "'");
        b=false;
      }

      b = lti::read(handler,"doGLevelDistance",doGLevelDistance) && b;
      b = lti::read(handler,"doGKernelVariance",doGKernelVariance) && b;
      b = lti::read(handler,"doGKernelSize",doGKernelSize) && b;

      b = lti::read(handler,"saliencyThreshold",saliencyThreshold) && b;
      b = lti::read(handler,"edgenessThreshold",edgenessThreshold) && b;

      b = lti::read(handler,"densityFilter",densityFilter) && b;
      b = lti::read(handler,"densityKernelRadius",densityKernelRadius) && b;
      b = lti::read(handler,"densityThreshold",densityThreshold) && b;

      b = lti::read(handler,"orientationWindowRadius",
                             orientationWindowRadius) && b;
      b = lti::read(handler,"orientationSigmaFactor",
                             orientationSigmaFactor) && b;
      b = lti::read(handler,"binsInOrientationHistogram",
                             binsInOrientationHistogram) && b;
      b = lti::read(handler,"multiOrientationThreshold",
                             multiOrientationThreshold) && b;
      b = lti::read(handler,"averageOrientation",averageOrientation) && b;

      b = lti::read(handler,"locationRelativeRadius",
                             locationRelativeRadius) && b;      
    }

# if !defined(_LTI_MSC_6)
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::read(handler,false);
# else
    bool (transform::parameters::* p_readMS)(ioHandler&,const bool) =
      transform::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# if defined( _LTI_MSC_6)
  bool pyramidLocationSearch::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // pyramidLocationSearch
  // --------------------------------------------------

  // default constructor
  pyramidLocationSearch::pyramidLocationSearch()
    : transform(){

    gradientFunctor::parameters ccgPar;
    ccgPar.gradientKernelSize = 3;
    gradient.setParameters(ccgPar);

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  pyramidLocationSearch::pyramidLocationSearch(const parameters& par)
    : transform() {

    gradientFunctor::parameters ccgPar;
    ccgPar.gradientKernelSize = 3;
    gradient.setParameters(ccgPar);

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  pyramidLocationSearch::pyramidLocationSearch(const pyramidLocationSearch& other) {
    gradientFunctor::parameters ccgPar;
    ccgPar.gradientKernelSize = 3;
    gradient.setParameters(ccgPar);

    copy(other);
  }

  // destructor
  pyramidLocationSearch::~pyramidLocationSearch() {
  }

  // returns the name of this type
  const char* pyramidLocationSearch::getTypeName() const {
    return "pyramidLocationSearch";
  }

  // copy member
  pyramidLocationSearch&
  pyramidLocationSearch::copy(const pyramidLocationSearch& other) {
    transform::copy(other);

    return (*this);
  }

  // alias for copy member
  pyramidLocationSearch&
  pyramidLocationSearch::operator=(const pyramidLocationSearch& other) {
    return (copy(other));
  }


  // clone member
  functor* pyramidLocationSearch::clone() const {
    return new pyramidLocationSearch(*this);
  }

  // return parameters
  const pyramidLocationSearch::parameters&
  pyramidLocationSearch::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException();
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  /*
   * Compute the position of the locations, based on the pyramids
   */
  bool 
  pyramidLocationSearch::computePyramids(const channel& src) {
    const parameters& param = getParameters();

    // set the pyramid parameters and size
    iPyr.setParameters(param.pyramidParameters);
    lapPyr.setParameters(param.pyramidParameters);
    maxPyr.setParameters(param.pyramidParameters);
    argPyr.setParameters(param.pyramidParameters);
    magPyr.setParameters(param.pyramidParameters);

    iPyr.resize(param.numLevels);
    lapPyr.resize(param.numLevels);
    argPyr.resize(param.numLevels);
    magPyr.resize(param.numLevels);
    maxPyr.resize(param.numLevels);
    
    channel chnl,tmpChnl;
    
    // upsample the image if desired
    if (param.upsampleFirstLevel>0) {
      scaling::parameters scalingPar;
      scalingPar.interpolatorType = param.pyramidParameters.interpolatorType;
      scaling scaler(scalingPar);
      firstLevelFactor = 1.0/pow(param.pyramidParameters.factor,
                                 param.upsampleFirstLevel);
      scaler.apply(static_cast<float>(firstLevelFactor),src,chnl);
    } else {
      chnl.copy(src);
    }

    // if level smoothing is desired, do it here!
    int i;
    convolution convolver;
    convolution::parameters convolverParam;
    // it will be unnecessary to smooth the image with a Gaussian 
    // which variance is less than 1/36 = 0.027777... , since 
    // with it a half pixel away the kernel has reduced its value
    // to 1% of the the middle point and at the next sample it will
    // be even 6 sigma away 
    if (param.smoothingGaussianVariance > 0.0277777f) {
      const double var = param.smoothingGaussianVariance;
      gaussKernel2D<float> gaussK(1+2*iround(ceil(sqrt(-2.0*var*log(0.05)))),
                                    var);
      convolverParam.setKernel(gaussK);
      convolverParam.boundaryType = Constant;
      convolver.setParameters(convolverParam);

      convolver.apply(chnl);
    }

    // check if the DoG kernel has to be computed
    if (param.peakSelectionMode == parameters::DoG) {
      computeDoGKernel();
    }

    // generate the pyramid
    iPyr.generate(chnl);

    // determine the correct kernel size for maxima detection
    int mxNeighborhoodSize = max(3,
                                 min(7,param.spatialMaximumNeighborhoodSize));
    if (even(mxNeighborhoodSize)) {
      mxNeighborhoodSize++;
    }

    maximumFilter<float>::parameters maxParam(mxNeighborhoodSize);
    maxParam.boundaryType = Constant;
    maximumFilter<float> mxConvolver(maxParam);

    // generate laplacian pyramid and maximum pyramid
    for (i = 0; i < iPyr.size(); i++) {
      // The gradient is always used, to compute the orientation of the 
      // locations.
      applyMag(iPyr.at(i),magPyr.at(i),argPyr.at(i));
      switch(param.peakSelectionMode) {
        case parameters::GradientMagnitude:
          lapPyr.at(i).copy(magPyr.at(i));
          break;
        case parameters::Laplacian:
          applyLaplacian(iPyr.at(i), lapPyr.at(i));
          break;
        case parameters::Harris:         
          applyCornerness(iPyr.at(i), lapPyr.at(i));
          break;
        case parameters::DoG:
          applyDoG(iPyr.at(i),lapPyr.at(i));
          break;
        default:
          break;
      }

      // and get the maximum values at each level, but only if it is necessary
      if (param.spatialMaximumNeighborhoodSize > 3) {
        mxConvolver.apply(lapPyr.at(i), maxPyr.at(i)); // filter the maximum
                                                       // values for fast
                                                       // maxima search in NxN
      }
    }
    
    return true;
  }

  /*
   * Compute the position of the locations, based on the pyramids
   */
  bool pyramidLocationSearch::
  computeLocationsPosition(std::map< int, std::list<location> >& locMap) {
    const parameters& param = getParameters();

    // Now we need to detect the maxima at each level.
    // Only if one maxima is found at the level, a comparison with upper and
    // lower levels will be done to check if a candidate maxima can be a
    // valid location.
    const int offset = 1; // we will need always the upper and lower levels
    const int spatialOffset = min(1,param.spatialMaximumNeighborhoodSize/2);
    float row0,col0,r,c,s;
    location loc; // locations are to be writen here first
    int x,y,i;
    const int lastLevel = iPyr.size()-offset;
    float minSaliencyThreshold = 0.0f;
    const bool checkILMaxima = param.checkInterLevelMaxima;

    const float ra = max(1.0f,min(param.edgenessThreshold,
                                  sqrt(std::numeric_limits<float>::max())));
    const float edgenessThreshold = (ra+1)*(ra+1)/ra;

    
    /**
     * Assume always the smallest possible saliency as maximum
     */
    lapMax.resize(lapPyr.size(),-std::numeric_limits<float>::max(),false,true);

    // for each level in the pyramid
    for (i=offset;i<lastLevel;++i) {
      // list of locations for level i
      std::list<location> locs;
  
      // get the saliency threshold
      minSaliencyThreshold = (lapMax.at(i) = lapPyr.at(i).maximum()) * 
        param.saliencyThreshold.at(min(param.saliencyThreshold.lastIdx(),i));
      // the maxima will be equal ("or greater?") than the value in the
      // filtered maxima channel.  Assuming that the saliecy filters out 
      // all low-frequencies, there will be no risk that homogeneous regions
      // will also be detected as maxima.
      const int lastCol = lapPyr.at(i).columns()-spatialOffset;
      const int lastRow = lapPyr.at(i).rows()-spatialOffset;
      
      const channel& lap  = lapPyr.at(i);
      const channel& maxc = maxPyr.at(i);
      const bool checkMaxPyr = (param.spatialMaximumNeighborhoodSize > 3);
      for (y=spatialOffset;y<lastRow;++y) {
        for (x=spatialOffset;x<lastCol;++x) {
          if ((lap.at(y,x)  > minSaliencyThreshold)            &&
              lapPyr.checkMaximum(y,x,i)                       &&
              (!checkMaxPyr || (lap.at(y,x) >= maxc.at(y,x)) ) &&
              (!checkILMaxima || lapPyr.checkInterlevelMaximum(y,x,i)) &&
              (edgenessRatio(lapPyr.at(i),y,x) < edgenessThreshold)) {

            // that's it! this is definitively a location: it survived all
            // possible checks!
            // set the location in the location list.
            if (param.interpolateMaxima) {
              if (lapPyr.interpolateMaximum(y,x,i,r,c,s)) {
                lapPyr.mapToLevel0(i,r,c,row0,col0);
              
                loc.position.set(col0,row0);
                loc.radius = 
                  lapPyr.getRadiusForScale(s)*param.locationRelativeRadius;
                
                // remember this location
                locs.push_back(loc);
                
                _lti_debug4("                   " <<
                            "(" << x << ", " << y << "; " << i << ")" <<
                            " -> (" << c << ", " << r << "; " << s << ")" <<
                            " ; E: " << edgenessRatio(lapPyr.at(i),y,x) <<
                            std::endl);
              }
            } else {
              lapPyr.mapToLevel0(i,
                                 static_cast<float>(y),static_cast<float>(x),
                                 row0,col0);
              s = static_cast<float>(i);

              loc.position.set(col0,row0);
              loc.radius = 
                lapPyr.getRadiusForScale(s)*param.locationRelativeRadius;
              
              // remember this location
              locs.push_back(loc);
              
              _lti_debug4("                   " <<
                          "(" << x << ", " << y << "; " << i << ")" <<
                          " ; E: " << edgenessRatio(iPyr.at(i),y,x) <<
                          std::endl);
            }

          } // end of if maxima at (y,x)
        } // end of for x
      } // end of for y
      
      // save location of scale i into map
      std::list<location>& currentList = locMap[i];
      currentList.clear();
      currentList.splice(currentList.end(),locs);

    } // end of for i each scale (between offset and lastLevel)
    return true;
  }
  
  // compute the maximum using a quadratic approximation
  inline bool pyramidLocationSearch::qmaximum(const float ym1,
                                              const float y0,
                                              const float y1,
                                              float& x) const {
    const float tmp = 2*(ym1+y1-(2*y0));
    if (tmp >= 0.0f) {
      // is a minimum! nonsense!
      return false;
    }

    x = (ym1-y1)/tmp;
    return true;
  };

  // estimate the real angle value using quadratic interpolation
  float pyramidLocationSearch::getAngle(const vector<float>& angles,
                                        const int idx) const {
    const int im1 = (idx+angles.lastIdx())%angles.size(); // idx-1
    const int ip1 = (idx+1)%angles.size();                // idx+1

    // simple check for a maximum
    if ((angles.at(im1) > angles.at(idx)) ||
        (angles.at(ip1) > angles.at(idx))) {
      return -1.0; // flag for error!  Its not a maximum!
    }

    static const float fct = static_cast<float>(2.0*Pi);
    float x;
    if (qmaximum(angles.at(im1),angles.at(idx),angles.at(ip1),x)) {
      x+=(static_cast<float>(idx)+0.5f);
    } else {
      // not a maximum
      x=static_cast<float>(idx)+0.5f;
    }

    // ensure an angle between 0 and 2Pi
    x = fct*x/angles.size();
    if (x<0) {
      x+=fct;
    } else if (x>=fct) {
      x-=fct;
    }

    return x;
  }

  /*
   * Compute the orientation of each location
   */
  bool pyramidLocationSearch::
  computeLocationOrientation(std::map< int, std::list<location> >& locMap,
                             int& totalLocs) {

    
    // create two local LUT for fast computation of sinus and cosinus
    static const float* cosLUT = 0;
    static const float* sinLUT = 0;
    static const int lutSize = 512;
    static const float lutFactor = static_cast<float>(lutSize/(2*Pi));
    if (isNull(cosLUT)) {
      // the real singleton LUT (writable)
      static float theCosLUT[lutSize+2];
      static float theSinLUT[lutSize+2];

      int i;
      float a;
      for (i=0;i<lutSize+2;++i) {
        a = static_cast<float>(i)/lutFactor;
        theCosLUT[i]=cos(a);
        theSinLUT[i]=sin(a);
      }

      // now make the local singleton public in a read-only fashion
      cosLUT = theCosLUT;
      sinLUT = theSinLUT;
    }

    // stuff for the orientation computation
    arctanLUT atan2; // fast (but not so precise atan2 function)
    float avrCos;
    float avrSin;

    const parameters& param = getParameters();
    // we now need to compute for each location its orientation.

    // create first a Gaussian window, with which we can efficiently access
    // the weights for each pixel.
    
    const int r = param.orientationWindowRadius;
    // create first the LUT as a kernel2D (uninitialized)
    kernel2D<float> wnd(false,-r,-r,r,r);
    
    // we define that at the window radius we reach 2*sigma, meaning
    // that the next neighbor outside the window would get a negligible weight.
    const float sigma = static_cast<float>(r)/param.orientationSigmaFactor; 
    gaussian<float> g(0.0f,sigma*sigma);
    int x,y,fx,fy,tx,ty,cx,cy;
    float xf,yf;
    // fill the Gaussian window
    for (y=0;y<=r;++y) {
      for (x=0;x<=r;++x) {
        wnd.at(-y,-x)=wnd.at(-y,x)=wnd.at(y,-x)=wnd.at(y,x)=
          g(sqrt(static_cast<float>(x*x+y*y)));
      }
    }

#if defined(_LTI_DEBUG) && (_LTI_DEBUG >= 4)
    static viewer viewoww("Orient.Weight Wnd",2);
    viewoww.show(wnd);
    static int pause = 50000;
    passiveWait(pause);
    pause = 10;
#endif    

    vector<float> histo(false,param.binsInOrientationHistogram);
    vector<float> shisto; // used to sort the histogram bin values
    vector<int> idx;      // contain the indices used while sorting the bins
    vector<int> sidx;     // sorted indices

    // initialize the indices
    idx.resize(param.binsInOrientationHistogram,0,false,false);
    for (x=0;x<param.binsInOrientationHistogram;++x) {
      idx.at(x) = x;
    }

    sort2<float,int> sorter(true); // descending order sorter

    const float factor = static_cast<float>(param.binsInOrientationHistogram/
                                            (2.0*Pi));

    // now compute the orientation for each location following Lowe suggestions
    std::list<location> multilocs;
    std::list<location>::iterator it;
    std::list<location>::iterator itEnd;
    std::map< int, std::list<location> >::iterator mit=locMap.begin();
    std::map< int, std::list<location> >::iterator mitEnd=locMap.end();

    // shadow parameter
    const bool avrgOrient = param.averageOrientation;
    totalLocs = 0;

    // histogram linear interpolation
    int i;
    float f,vd,w,angle;

    //loop over levels
    for (;mit!=mitEnd; ++mit) {
      std::list<location>& locs=(*mit).second;
      it=locs.begin();
      itEnd=locs.end();
      const int level=(*mit).first;

      const channel& chnl = argPyr.at(level);
      const channel& mag  = magPyr.at(level);

      //loop over locations in current level
      for (;it!=itEnd; ++it) {
        // for each location
        ++totalLocs;

        lapPyr.mapToLevel(level,(*it).position.y,(*it).position.x,yf,xf);
        // center of the location
        cx = iround(xf);
        cy = iround(yf);
      
        // compute the histogram
        
        // valid window range
        fx = max(0,cx-r);
        fy = max(0,cy-r);
        tx = min(chnl.lastColumn(),cx+r);
        ty = min(chnl.lastRow(),cy+r);
        
        histo.fill(0.0f); // clear the histogram
        
        // accumulate with the window weights interpolating linearly between
        // bins.
        for (y=fy;y<=ty;++y) {
          for (x=fx;x<tx;++x) {
            vd = factor*chnl.at(y,x); // float histogram index
            i = static_cast<int>(vd); // integer histogram index
            f = vd - i;               // fractional part of index
            w = (wnd.at(y-cy,x-cx) *  // Gaussian weight
                 mag.at(y,x));        // gradient magnitude weight
            if (f<0.5f) {             // shared with lower cell
              histo.at(i) += w*(f+0.5f);
              if (i==0) {
                histo.at(histo.lastIdx()) += w*(0.5f-f);
              } else {
                histo.at(i-1) += w*(0.5f-f);
              }
            } else { // f >= 0.5      // shared with higher cell
              histo.at(i) += w*(1.5f-f);
              if (i>=histo.lastIdx()) {
                histo.at(0) += w*(f-0.5f);
              } else {
                histo.at(i+1) += w*(f-0.5f);
              }
            }
          }
        }
      
        // now search for the dominant orientation
        sorter.apply(histo,idx,shisto,sidx);
        const float thresh = param.multiOrientationThreshold * shisto.at(0);

        if (avrgOrient) {
          x = 0;
          f = 0.0f;
          w = 0.0f;
          avrCos = avrSin = 0.0f;
          while((x<sidx.size()) && ((w=shisto.at(x)) >= thresh)) {
            // use a simple angle reconstruction (the value in the middle)
            i = static_cast<int>(lutSize*(sidx.at(x)+0.5f)/sidx.size());
            
            // assert(static_cast<uint32>(i) < static_cast<uint32>(lutSize));
            avrCos += w*cosLUT[i];
            avrSin += w*sinLUT[i];
            f += w;
            ++x;            
          }

          if (f > 0.0f) {
            (*it).angle = atan2(avrSin/f,avrCos/f);
          } else {
            (*it).angle = getAngle(histo,sidx.at(0)); // get quadr. interp. ang
          }
        } else {
          // assign the "best" angle to the location (this is always a max)
          (*it).angle = getAngle(histo,sidx.at(0)); // this is the angle
          
          // check for further candidates
          x = 1;
          while ((x<sidx.size()) && (shisto.at(x) >= thresh)) {
            // get the maximum with a quadratic interpolation
            angle = getAngle(histo,sidx.at(x));
            if (angle >= 0.0f) { // only maxima! (negative means error!)
              location loc(*it);              
              loc.angle = angle;
              multilocs.push_back(loc);
              ++totalLocs;            
            }
            ++x;
          }
        }

#if defined(_LTI_DEBUG) && (_LTI_DEBUG >= 4)
        _lti_debug("Location position: " << (*it).position << std::endl);
        _lti_debug("     Winner angle:" << radToDeg((*it).angle) << std::endl);
        static viewer viewoh("Orient.Histo",2);
        viewoh.show(histo);
        viewoh.waitKey();
#endif    

        
      } // end loop over locations of one level
      
      // insert the multilocs list at the end of locs.
      locs.splice(locs.end(),multilocs);
      multilocs.clear();
    } // end loop over levels

    return true;
  }

  bool pyramidLocationSearch::apply(const channel& src,
                               std::map< int, std::list<location> >& locMap) {
    int totalLocs;
    return apply(src,locMap,totalLocs);
  }

  bool pyramidLocationSearch::apply(const channel& src,
                                  std::map< int, std::list<location> >& locMap,
                                    int& totalLocs) {

    //clear the list of locations
    locMap.clear();

    if ((computePyramids(src) &&
         computeLocationsPosition(locMap) &&
         densityFilter(locMap) &&
         computeLocationOrientation(locMap,totalLocs))) {

      const parameters& par = getParameters();

      if ((par.upsampleFirstLevel>0) && (!par.locationsAtLevelZero)) {
        
        const float factor = static_cast<float>(1.0/firstLevelFactor);

        // if upsample is active, then all locations have been computed
        // with respect to the upsampled original channel, and this
        // means that they need to be rescaled to the original one
        std::list<location>::iterator it;
        std::list<location>::iterator itEnd;
        std::map< int, std::list<location> >::iterator mit=locMap.begin();
        std::map< int, std::list<location> >::iterator mitEnd=locMap.end();
        for (; mit!=mitEnd; ++mit) {
          it=(*mit).second.begin();
          itEnd=(*mit).second.end();
          for (;it!=itEnd;++it) {
            ((*it).position).multiply(factor);
            (*it).radius*=factor;
          }
        }
      }

      return true;
    }

    return false;
    
  }
  
  // On copy apply for type channel!
  bool pyramidLocationSearch::apply(const channel& src,
                                    std::list<location>& locs) {
    int totalLocs;
    return apply(src,locs,totalLocs);
  }

  // On copy apply for type channel!
  bool pyramidLocationSearch::apply(const channel& src,
                                    std::list<location>& locs,
                                    int& totalLocs) {
    locs.clear();
    std::map< int, std::list<location> > locMap;

    if (!apply(src,locMap,totalLocs)) {
      return false;
    }
    
    std::map< int, std::list<location> >::iterator mit=locMap.begin();
    std::map< int, std::list<location> >::iterator mitEnd=locMap.end();

    for (; mit!=mitEnd; ++mit) {      
      // locs.insert(locs.end(), (*mit).second.begin(), (*mit).second.end());
      // move the lists efficiently from the maps to the list
      locs.splice(locs.end(),(*mit).second);
    }

    return true;
  }

  bool pyramidLocationSearch::apply(const channel& c1,
                                    const channel& c2,
                                    std::list<location>& locs) {

    int totalLocs;
    return apply(c1,c2,locs,totalLocs);
  }

  bool pyramidLocationSearch::apply(const channel& c1,
                                    const channel& c2,
                                    std::list<location>& locs,
                                    int& totalLocs) {
    if (c1.size() != c2.size()) {
      setStatusString("Input channels have different sizes");
      return false;
    }

    std::list<location> locs2;
    int l2;
    if (apply(c1,locs,totalLocs) && apply(c2,locs2,l2)) {
      locs.splice(locs.end(),locs2); // transfer data of locs2 at the end of
                                     // locs
      totalLocs+=l2;
      return true;
    }

    return false;
  }

  bool pyramidLocationSearch::apply(const channel& c1,
                                    const channel& c2,
                                    const channel& c3,
                                    std::list<location>& locs) {
    int totalLocs;
    return apply(c1,c2,c3,locs,totalLocs);
  }

  bool pyramidLocationSearch::apply(const channel& c1,
                                    const channel& c2,
                                    const channel& c3,
                                    std::list<location>& locs,
                                    int& totalLocs) {

    if ((c1.size() != c2.size()) ||
        (c2.size() != c3.size())) {
      setStatusString("Input channels have different sizes");
      return false;
    }

    std::list<location> locs2,locs3;
    int l2,l3;
    if (apply(c1,locs,totalLocs) && apply(c2,locs2,l2) && apply(c3,locs3,l3)) {
      locs.splice(locs.end(),locs2); // transfer data of locs2 and locs3 at
      locs.splice(locs.end(),locs3); // the end of locs
      totalLocs += (l2 + l3);
      return true;
    }

    return false;
  }
    
  void pyramidLocationSearch
  ::concatLocsMap(std::map<int,std::list<location> >& src1dest,
                  std::map<int,std::list<location> >& src2) const {
    std::map<int,std::list<location> >::iterator it,it2;
    // for each level of the second map
    for (it2=src2.begin();it2!=src2.end();++it2) {
      if ((*it2).second.empty()) {
        // empty list do not need to be inserted
        continue;
      }
      // find if the level has been used in the first map
      it = src1dest.find((*it2).first);
      if (it == src1dest.end()) {
        // hmm, the level wasn't used before, just insert the data
        std::list<location>& newList = src1dest[(*it2).first];
        newList.splice(newList.end(),(*it2).second);
      } else {
        // move the list from the second map to the first one
        (*it).second.splice((*it).second.end(),(*it2).second);
      }
    }

    src2.clear();
  }


  bool pyramidLocationSearch::apply(const channel& c1,
                                    const channel& c2,
                                std::map< int, std::list<location> >& locMap) {
    int totalLocs;
    return apply(c1,c2,locMap,totalLocs);
  }

  bool pyramidLocationSearch::apply(const channel& c1,
                                    const channel& c2,
                                  std::map< int, std::list<location> >& locMap,
                                    int& totalLocs) {

    if (c1.size() != c2.size()) {
      setStatusString("Input channels have different sizes");
      return false;
    }

    int l2;

    std::map< int, std::list<location> > locMap2;
    if (apply(c1,locMap,totalLocs) && apply(c2,locMap2,l2)) {
      concatLocsMap(locMap,locMap2);
      totalLocs += l2;
      return true;
    }

    return false;
  }

  bool pyramidLocationSearch::apply(const channel& c1,
                                    const channel& c2,
                                    const channel& c3,
                                std::map< int, std::list<location> >& locMap) {
    int totalLocs;
    return apply(c1,c2,c3,locMap,totalLocs);
  }

  bool pyramidLocationSearch::apply(const channel& c1,
                                    const channel& c2,
                                    const channel& c3,
                                  std::map< int, std::list<location> >& locMap,
                                    int& totalLocs) {

    if (c1.size() != c2.size()) {
      setStatusString("Input channels have different sizes");
      return false;
    }

    int l2,l3;
    std::map< int, std::list<location> > locMap2,locMap3;
    if (apply(c1,locMap,totalLocs) && 
        apply(c2,locMap2,l2) &&
        apply(c3,locMap3,l3)) {
      concatLocsMap(locMap,locMap2);
      concatLocsMap(locMap,locMap3);
      totalLocs += (l2+l3);
      return true;
    }

    return false;
  }

  // --------------------------------------------------------------------------

  //
  // Masked apply methods
  //



  // --------------------------------------------------------------------------

  //
  // Methods for saliency computation
  //

  void pyramidLocationSearch::applyCornerness(const channel& src, 
                                                    channel& dest) const {
    harrisCorners cd;
    harrisCorners::parameters params = cd.getParameters();

    params.variance = 1.5;
    params.kernelSize = 5;
    params.scale = 0.02f;

    cd.setParameters(params);
    float maxCornerness;

    // compute the cornerness.
    cd.apply(src,dest,maxCornerness);

    // get the absolute values of the gradient
    dest.apply(abs);
  }

  void pyramidLocationSearch::applyLaplacian(const channel& src,
                                                channel& dest) const {
    laplacianKernel laplacian(5);
    convolution::parameters convPar;

    convPar.setKernel(laplacian);
    convPar.boundaryType = Constant;

    convolution convolver(convPar);
    convolver.apply(src,dest);
    dest.apply(abs);
  }

  void pyramidLocationSearch::applyMag(const channel& src, 
                                    channel& mag,
                                    channel& arg) const {
    gradient.apply(src,mag,arg);
  }

  const scaleSpacePyramid<channel>& 
  pyramidLocationSearch::getSaliencyPyramid() const {
    return lapPyr;
  }

  const scaleSpacePyramid<channel>& 
  pyramidLocationSearch::getOrientationPyramid() const {
    return argPyr;
  }

  const scaleSpacePyramid<channel>& 
  pyramidLocationSearch::getChannelPyramid() const {
    return iPyr;
  }


  void pyramidLocationSearch::applyDoG(const channel& src,
                                    channel& dest) const {
    convolution::parameters convPar;

    convPar.setKernel(doGKernel);
    convPar.boundaryType = Constant;

    convolution convolver(convPar);
    convolver.apply(src,dest);
    dest.apply(abs);
  }

  bool pyramidLocationSearch::computeDoGKernel() {
    const parameters& par = getParameters();

    doGKernel.setNumberOfPairs(2);
    
    const double f = iPyr.getLevelScale(par.doGLevelDistance);

    // get the correct kernel size
    int size = par.doGKernelSize;
    if (size < 3) {
      size = 1+2*iround(sqrtrect(-2*par.doGKernelVariance*log(0.05))/f);
    }

    gaussKernel1D<float> gk;
    gk.generate(size,par.doGKernelVariance/(f*f));
    doGKernel.getColFilter(0).copy(gk);
    doGKernel.getRowFilter(0).copy(gk);

    size/=2;
    size = 1+2*iround(ceil(size*f));

    gk.generate(size,par.doGKernelVariance);
    doGKernel.getColFilter(1).copy(gk);
    gk.multiply(-1.0f);
    doGKernel.getRowFilter(1).copy(gk);

    return true;
  }

  float pyramidLocationSearch::edgenessRatio(const channel& chnl,
                                          const int y,
                                          const int x) const {

    // To compute the edgeness ratio or "principal curvatures" we use here
    // a slightly different approach than Lowe.  He uses the derivatives 
    // estimated from the image pixels to compute the coefficients of the
    // Hessian matrix.  In a few experiments this was not stable.
    //
    // Here we compute the coefficients from a LSE approximation of a 
    // quadratic surface passing through the point at (x,y) and minimizing
    // the error through the eight neighbor points.
    
    // Using SVD to minimize the error, we obtain a kernel of the form
    //
    // | a b a |
    // | c d c |
    // | a b a |

    static const float a =  0.1f;
    static const float b =  0.3f;
    static const float c = -0.2f;
    static const float d = -0.6f;
    
    const float lr = chnl.at(y,x-1) + chnl.at(y,x+1);
    const float ud = chnl.at(y-1,x) + chnl.at(y+1,x);
    const float e1 = chnl.at(y-1,x-1) + chnl.at(y+1,x+1);
    const float e2 = chnl.at(y-1,x+1) + chnl.at(y+1,x-1);
    const float dc = d*chnl.at(y,x) + a*(e1+e2);

    const float Dxx = (dc + b*lr + c*ud);
    const float Dyy = (dc + b*ud + c*lr);
    
    // the kernel Dxy has the form
    // |  e  0 -e  |
    // |  0  0  0  |
    // | -e  0  e  |
    
    static const float e = 0.125f;

    const float Dxy = e*(e1-e2);

    const float tr  = Dxx+Dyy;
    const float det = Dxx*Dyy - Dxy*Dxy;

    if (det < std::numeric_limits<float>::epsilon()) {
      return std::numeric_limits<float>::max();
    } else {
      return tr*tr/det;
    }
  }

  // -------------------------------------------------------------------------
  //                                 Density Filter
  // -------------------------------------------------------------------------
  // It is assumed that this function is called BEFORE the locations have
  // been updated to the use of upsamplingLevel position.
  bool pyramidLocationSearch::
  densityFilter(std::map<int,std::list<location> >& srcDest) const {
    const parameters& param = getParameters();

    if ((!param.densityFilter) || (param.densityKernelRadius < 1)) {
      // without a kernel we cannot compute the density
      return true;
    }

    if (param.densityThreshold.empty()) {
      setStatusString("Invalid density threshold parameter vector: empty");
      return false;
    }

    const int radius = param.densityKernelRadius;

    // initialize input-output points for the circle mask
    array<int> ioPts(-radius,radius,0);
    
    // create the io points and the mask of the circle window
    int i;
    const float r2 = sqr(static_cast<float>(radius));

    // go through the y-axis
    for (i=-radius;i<=radius;++i) {
      // the in and out points correspond to the borders of a circle:
      ioPts.at(i) = ((i <= 0) ?
                     iround(sqrt(r2-sqr(static_cast<float>(i)+0.5f))) :
                     ioPts.at(-i));
    }

    std::map<int,std::list<location> >::iterator mit;
    std::list<location>::const_iterator cit;
    std::list<location>::iterator it;
    float minThreshold;
    float val;
    fpoint pos;
    ipoint center;
    int x,y,tx,fy,ty;

    // create an empty, uninitialized mask just as memory holder
    channel maskHolder(false,lapPyr.at(0).size());
    // this will use the maskHolder memory (once allocated!)
    channel mask;

    _lti_if_debug3(static viewer viewm("Density",2.0));
    _lti_if_debug3(static viewer views("Saliency",2.0));
    _lti_if_debug2(int accepted(0));
    _lti_if_debug2(int rejected(0));
    _lti_debug3("Density Filter" << std::endl);

    // for each pyramid level with elements
    for (mit=srcDest.begin();mit!=srcDest.end();++mit) {
      const int level = (*mit).first;
      const std::list<location>& locs = (*mit).second;
      
      minThreshold = 
        param.densityThreshold.at(min(param.densityThreshold.lastIdx(),
                                      level));

      if (locs.empty() ||
          (minThreshold <= std::numeric_limits<float>::epsilon())) {
        // level without locations, or no filtering desired,
        // just go one with the next one.
        continue;
      }

      // fill the saliency mask
      mask.useExternData(lapPyr.at(level).rows(),
                         lapPyr.at(level).columns(),
                         &maskHolder.at(0));
      mask.fill(0.0f); // clear the saliency mask

      // for each location in the list of this level
      for (cit=locs.begin();cit!=locs.end();++cit) {
        // get the saliency value of the location. 
        
        // (*cit).position is in coordinates of the level 0.  This function
        // has to be called BEFORE the locations are updated to the upsampling
        // factor!!!
        val = lapPyr.nearestAt((*cit).position.y,(*cit).position.x,level);
        
        // we need the coordinates for the proper level
        lapPyr.mapToLevel(level,(*cit).position.y,(*cit).position.x,
                          pos.y,pos.x);

        // we have the center of the window now.
        center.castFrom(pos);

        // compute the proper windows limits
        fy=max(0,center.y-radius);
        ty=min(mask.lastRow(),center.y+radius);

        // fill the window
        for (y=fy;y<=ty;++y) {
          i=ioPts[y-center.y];
          tx=min(mask.lastColumn(),center.x+i);
          for (x=max(0,center.x-i);x<=tx;++x) {
            mask.at(y,x)=max(mask.at(y,x),val);
          }
        }
      }

      _lti_debug3("Density mask for level " << level << " with " <<
                  locs.size() << " locations." << std::endl);
      _lti_if_debug3(views.show(lapPyr.at(level)));
      _lti_if_debug3(viewm.show(mask));
      _lti_if_debug3(viewm.waitKey());
      
      std::list<location>& rwLocs = (*mit).second;

      // mask contains now the local saliencies.  We need now to filter
      // the locations
      // for each location in the list of this level
      it=rwLocs.begin();
      while(it!=rwLocs.end()) {
        // saliency value at the location
        val = lapPyr.nearestAt((*it).position.y,(*it).position.x,level);
        // we need the coordinates for the proper level
        lapPyr.mapToLevel(level,(*it).position.y,(*it).position.x,
                          pos.y,pos.x);

        // check if salient enough
        if (val >= minThreshold*mask.at(iround(pos.y),iround(pos.x))) {
          // Strong location! -> it survives
          it++;
          _lti_if_debug2(accepted++);
        } else {
          // sorry! too weak!  maybe next time :-)
          it=rwLocs.erase(it);
          _lti_if_debug2(rejected++);
        }
      } // end of while for each loc in rwLocs
    } // end of for each level    
    
    _lti_debug2("Accepted: " << accepted << "\nRejected: " << rejected);
    _lti_debug2("\nEnd of Density Filter" << std::endl;);

    return true;
  }

}
