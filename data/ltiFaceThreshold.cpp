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
 * file .......: ltiFaceThreshold.cpp
 * authors ....: Thorsten Dick
 * organization: LTI, RWTH Aachen
 * creation ...: 17.12.2003
 * revisions ..: $Id: ltiFaceThreshold.cpp,v 1.12 2008/08/17 22:20:12 alvarado Exp $
 */

#include "ltiFaceThreshold.h"
#include <climits>

namespace lti {

  // #########################################################################
  // faceThreshold::parameters
  // #########################################################################

  // default constructor
  faceThreshold::parameters::parameters() 
    : functor::parameters() {    
    // computation mode parameters
    computationMode    = int (0);
    thresholdStep      = int (5);
    gaussKernelSize    = int (0);
    interpolationStep  = float (1.0f);
    optModePrecision   = int (2);
    // blob ranking parameters
    compactnessExponent          = int (2);
    mainAxisOrientationExponent  = int (2);
    dRatioExponent               = int (2);
    dRatioOptRatio               = float (1.5f);
    dRatioWidth                  = int (2);
    hDistExponent                = int (2);
    hDistExpectedPosition        = int (50);
    hDistWidth                   = int (1);
    vDistExponent                = int (3);
    vDistExpectedPosition        = int (30);
    vDistWidth                   = int (3);
    sizeExponent                 = int (3);
    sizeMin                      = float (0.5f);
    sizeMax                      = float (5.5f);
    thresholdProbabilityExponent = int (2);
  }

  // copy constructor
  faceThreshold::parameters::parameters(const parameters& other) {
    copy(other);
  }
  
  // destructor
  faceThreshold::parameters::~parameters() {    
  }

  // get type name
  const char* faceThreshold::parameters::getTypeName() const {
    return "faceThreshold::parameters";
  }
  
  // copy member
  faceThreshold::parameters& 
    faceThreshold::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters& (functor::parameters::* p_copy)
      (const functor::parameters&) = 
      functor::parameters::copy;
    (this->*p_copy)(other);
# endif
    computationMode   = other.computationMode;
    thresholdStep     = other.thresholdStep;
    gaussKernelSize   = other.gaussKernelSize;
    interpolationStep = other.interpolationStep;
    optModePrecision  = other.optModePrecision;
    compactnessExponent          = other.compactnessExponent;
    mainAxisOrientationExponent  = other.mainAxisOrientationExponent;
    dRatioExponent               = other.dRatioExponent;
    dRatioOptRatio               = other.dRatioOptRatio;
    dRatioWidth                  = other.dRatioWidth;
    hDistExponent                = other.hDistExponent;
    hDistExpectedPosition        = other.hDistExpectedPosition;
    hDistWidth                   = other.hDistWidth;
    vDistExponent                = other.vDistExponent;
    vDistExpectedPosition        = other.vDistExpectedPosition;
    vDistWidth                   = other.vDistWidth;
    sizeExponent                 = other.sizeExponent;
    sizeMin                      = other.sizeMin;
    sizeMax                      = other.sizeMax;
    thresholdProbabilityExponent = other.thresholdProbabilityExponent;
    return *this;
  }

  // alias for copy member
  faceThreshold::parameters& 
    faceThreshold::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* faceThreshold::parameters::clone() const {
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
  bool faceThreshold::parameters::write(ioHandler& handler,
                                         const bool complete) const 
# else
  bool faceThreshold::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const 
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }
    if (b) {
      lti::write(handler,"computationMode",       computationMode);
      lti::write(handler,"thresholdStep",         thresholdStep);
      lti::write(handler,"gaussKernelSize",       gaussKernelSize);
      lti::write(handler,"interpolationStep",     interpolationStep);
      lti::write(handler,"optModePrecision",      optModePrecision);
      lti::write(handler,"compactnessExponent",   compactnessExponent);
      lti::write(handler,"mainAxisOrientationExponent", 
        mainAxisOrientationExponent);
      lti::write(handler,"dRatioExponent",        dRatioExponent);
      lti::write(handler,"dRatioOptRatio",        dRatioOptRatio);
      lti::write(handler,"dRatioWidth",           dRatioWidth);
      lti::write(handler,"hDistExponent",         hDistExponent);
      lti::write(handler,"hDistExpectedPosition", hDistExpectedPosition);
      lti::write(handler,"hDistWidth",            hDistWidth);
      lti::write(handler,"vDistExponent",         vDistExponent);
      lti::write(handler,"vDistExpectedPosition", vDistExpectedPosition);
      lti::write(handler,"vDistWidth",            vDistWidth);
      lti::write(handler,"sizeExponent",          sizeExponent);
      lti::write(handler,"sizeMin",               sizeMin);
      lti::write(handler,"sizeMax",               sizeMax);
      lti::write(handler,"thresholdProbabilityExponent", 
        thresholdProbabilityExponent);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to 
    // compile...
    b = b && functor::parameters::write(handler,false);
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
  bool faceThreshold::parameters::write(ioHandler& handler,
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
  bool faceThreshold::parameters::read(ioHandler& handler,
                                        const bool complete) 
# else
  bool faceThreshold::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }
    if (b) {
      lti::read(handler,"computationMode",       computationMode);
      lti::read(handler,"thresholdStep",         thresholdStep);
      lti::read(handler,"gaussKernelSize",       gaussKernelSize);
      lti::read(handler,"interpolationStep",     interpolationStep);
      lti::read(handler,"optModePrecision",      optModePrecision);
      lti::read(handler,"compactnessExponent",   compactnessExponent);
      lti::read(handler,"mainAxisOrientationExponent", 
        mainAxisOrientationExponent);
      lti::read(handler,"dRatioExponent",        dRatioExponent);
      lti::read(handler,"dRatioOptRatio",        dRatioOptRatio);
      lti::read(handler,"dRatioWidth",           dRatioWidth);
      lti::read(handler,"hDistExponent",         hDistExponent);
      lti::read(handler,"hDistExpectedPosition", hDistExpectedPosition);
      lti::read(handler,"hDistWidth",            hDistWidth);
      lti::read(handler,"vDistExponent",         vDistExponent);
      lti::read(handler,"vDistExpectedPosition", vDistExpectedPosition);
      lti::read(handler,"vDistWidth",            vDistWidth);
      lti::read(handler,"sizeExponent",          sizeExponent);
      lti::read(handler,"sizeMin",               sizeMin);
      lti::read(handler,"sizeMax",               sizeMax);
      lti::read(handler,"thresholdProbabilityExponent", 
        thresholdProbabilityExponent);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to 
    // compile...
    b = b && functor::parameters::read(handler,false);
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
  bool faceThreshold::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // #########################################################################
  // faceThreshold
  // #########################################################################

  // default constructor
  faceThreshold::faceThreshold()
    : functor() {
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  faceThreshold::faceThreshold(const parameters& par)
    : functor() {
    // set the given parameters
    setParameters(par);
  }

  // copy constructor
  faceThreshold::faceThreshold(const faceThreshold& other) {
    copy(other);
  }

  // destructor
  faceThreshold::~faceThreshold() {
  }

  // returns the name of this type
  const char* faceThreshold::getTypeName() const {
    return "faceThreshold";
  }

  // copy member
  faceThreshold&
    faceThreshold::copy(const faceThreshold& other) {
      functor::copy(other);
    return (*this);
  }

  // alias for copy member
  faceThreshold&
    faceThreshold::operator=(const faceThreshold& other) {
    return (copy(other));
  }

  // clone member
  functor* faceThreshold::clone() const {
    return new faceThreshold(*this);
  }

  // return parameters
  const faceThreshold::parameters&
    faceThreshold::getParameters() const {
    const parameters* par = 
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException();
    }
    return *par;
  }
  
  // #########################################################################
  // The apply-methods
  // #########################################################################
  
  // --- apply version 1 -----------------------------------------------------
  bool faceThreshold::apply (const channel& input, 
                             float& threshold) const {
    channel8 mask;
    borderPoints faceBlob;
    bool success;
    int  result;
    mask.castFrom (input);
    success = apply(mask, result, faceBlob); // call of apply version 4
    threshold = (static_cast<float>(result)/255.0f);
    return success;
  };

  // --- apply version 2 -----------------------------------------------------
  bool faceThreshold::apply (const channel& input, 
                             float& threshold, 
                             borderPoints& faceBlob) const {
    channel8 mask;
    bool success;
    int  result;
    mask.castFrom (input);
    success = apply(mask, result, faceBlob); // call of apply version 4
    threshold = (static_cast<float>(result) / 255.0f);
    return success;
  };

  // --- apply version 3 -----------------------------------------------------
  bool faceThreshold::apply (const channel8& input, 
                             int& threshold) const {
    bool success;
    borderPoints faceBlob;
    success = apply(input, threshold, faceBlob); // call of apply version 4
    return success;
  }
  
  // --- apply version 4 -----------------------------------------------------
  bool faceThreshold::apply(const channel8& input, 
                            int& threshold, 
                            borderPoints& faceBlob) const {
    const parameters& params = getParameters ();
    channel8 mask = input;
    // smooth mask if wanted
    if (params.gaussKernelSize > 1) {
      convolution m_smoother;
      convolution::parameters smootherParams;
      smootherParams.boundaryType = convolution::parameters::Constant;
      smootherParams.setKernel (gaussKernel2D<float>
        (params.gaussKernelSize));
      m_smoother.setParameters (smootherParams);
      m_smoother.apply (mask);
    }
    // calling of the actual threshold computation method
    bool foundBlobs;
    if (params.computationMode == parameters::linearMode) {
      foundBlobs = linearThresholdSearch (mask, threshold, faceBlob);
    } else if (params.computationMode == parameters::timeOptimizedMode) {
      foundBlobs = optimizedThresholdSearch (mask, threshold, faceBlob);
    } else {
      threshold = 0;
      return false;
    }
    if (!foundBlobs)
      threshold = 0;
    return foundBlobs;
  };
  
  // #########################################################################
  // Protected methods
  // #########################################################################

  bool faceThreshold::linearThresholdSearch (const channel8 &mask, 
                                             int &faceThreshold, 
                                             borderPoints &faceBlob
                                            ) const {
    const parameters& params = getParameters();
    int score;
    int maxScore = INT_MIN;
    int threshold;
    bool foundBlob = false;
    static std::list<borderPoints>           blobList;
    static std::list<borderPoints>::iterator blobListIt;
    static geometricFeatures                 geoFeat;
    static std::map<std::string, double>          featureMap;
    static fastObjectsFromMask<channel8>     extractor;
    static fastObjectsFromMask<channel8>::parameters extractorParams;
    // use the border length of a quadrat with desired minimal area as 
    // an approximation for minimum number of border points
    extractorParams.minBorderPoints = 
      static_cast<int>(4*sqrt(params.sizeMin*mask.rows()*mask.columns()/100));

    // loop over thresholds
    for (threshold = params.thresholdStep; 
         threshold < 256; 
         threshold+= params.thresholdStep) {
      // extract blobs
      blobList.clear();
      extractorParams.threshold = static_cast<unsigned char>(threshold);
      extractor.setParameters (extractorParams);
      extractor.apply (mask, blobList);
      // if no blob was found -> continue with next threshold
      if (blobList.begin() == blobList.end())
        continue;
      foundBlob = true;
      // loop over all found blobs
      for (blobListIt = blobList.begin(); 
           blobListIt != blobList.end(); 
           ++blobListIt) {
        // interpolate blob, get features and rank
        interpolate (*blobListIt, mask.columns(), mask.rows());
        geoFeat.apply (*blobListIt, featureMap);
        getScoreForBlob (featureMap, mask.columns(), mask.rows(), 
          threshold, score);
        // found a new best blob?
        if (score > maxScore) {
          maxScore = score;
          faceThreshold = threshold;
          faceBlob = *blobListIt;
        }
      }
    }
   return foundBlob;
  }

  // -------------------------------------------------------------------------
  bool faceThreshold::optimizedThresholdSearch (const channel8 &mask, 
                                                int &faceThreshold, 
                                                borderPoints &faceBlob
                                               ) const {
    const parameters& params = getParameters();
    int score;
    int threshold;
    bool foundBlob = false;
    int t1 = INT_MIN;       // best threshold
    int t2 = INT_MIN;       // second best threshold
    int s1 = INT_MIN;       // score corresponding to best threshold
    int s2 = INT_MIN;       // score corresponding to second best threshold
    const int stepDiv = 16;
    int left  = stepDiv;    // start-threshold for first level of search
    int right = 255;        // end-threshold for first level of search
    int step  = stepDiv;    // check every step-th threshold
    bool tested[256];       // remember already tested threshold to 
                            // avoid redundancy
    static std::list<borderPoints>           blobList;
    static std::list<borderPoints>::iterator blobListIt;
    static std::map<std::string, double>          featureMap;
    static geometricFeatures                 geoFeat;
    static fastObjectsFromMask<channel8>     extractor;
    static fastObjectsFromMask<channel8>::parameters extractorParams;
    // use the border length of a quadrat with desired minimal area as 
    // an approximation for minimum number of border points
    extractorParams.minBorderPoints = 
      static_cast<int>(4*sqrt(params.sizeMin*mask.rows()*mask.columns()/100));
    for (threshold = 0; threshold < 256; ++threshold)
      tested[threshold] = false;
  
    // main loop
    while (step >= params.optModePrecision) {
      // check thresholds in current region (left, right)
      for (threshold = left; threshold <= right; threshold += step) {
        // to avoid redundant computation skip threshold if it has been 
        // checked already
        if (tested[threshold] == false) {
          // extract blobs
          blobList.clear();
          extractorParams.threshold = static_cast<unsigned char>(threshold);
          extractor.setParameters (extractorParams);
          extractor.apply (mask, blobList);
          if (blobList.begin() != blobList.end())
            foundBlob = true;
          // loop over all found blobs
          for (blobListIt = blobList.begin(); 
               blobListIt != blobList.end(); 
               ++blobListIt) {
            interpolate (*blobListIt, mask.columns(), mask.rows());
            geoFeat.apply (*blobListIt, featureMap);
            getScoreForBlob (featureMap, mask.columns(), mask.rows(), 
              threshold, score);
            // check whether a new best blob was found
            if (score > s2) {
              if (score < s1) { // second best score
                t2 = threshold;
                s2 = score;
              } else if (score > s1) { // highscore!
                t2 = t1;
                s2 = s1;
                t1 = threshold;
                s1 = score;
                faceBlob = *blobListIt;
              } else if (score == s1) { // exactly the best score has been 
                t1 = threshold;         // reached again
                s1 = score;
                faceBlob = *blobListIt;
              }
            } 
          } // end of loop over all found blobs
          tested[threshold] = true;
        } 
      } // end of for-loop over thresholds in current region
      // for next iteration: calculate the new search-region, which is 
      // determined by the thresholds that produced the best (t1 with s1) 
      // and the second best (t2 with s2) scores
      left  = ( (t1 < t2) ? (t1-2*step < 0 ? 0: t1-2*step) : 
        (t2-step < 0 ? 0 : t2-step) );
      right = ( (t1 < t2) ? (t2+step > 255 ? 255 : t2+step) : 
        (t1+2*step > 255 ? 255 : t1+2*step) );
      step  = ((right-left)/stepDiv >= step ? step-1 : 
        (right-left) / stepDiv);
    } // end of main loop
    faceThreshold = t1;
    return foundBlob;
  }

  // -------------------------------------------------------------------------
  // This method computes the score for the given features of the blob.
  // To cope with numerical problems (over/underflow) the range of each
  // resulting score is checked (and, if the value is impossible, it will be 
  // ignored), and furthermore the individual minimal scores are limited 
  // to INT_MIN/numOfCriteria.
  void faceThreshold::getScoreForBlob (std::map<std::string, double> 
                                         &featureMap, 
                                       const int &cols, 
                                       const int &rows, 
                                       const int &threshold,
                                       int &score) const {
    const parameters& params = getParameters();
    double a, b, c, expected, diff;
    int result;
    score = 0;
    // compactness
    if (params.compactnessExponent > 0) {
      result= static_cast<int>(100 - (100 * pow (
        (1.0f-featureMap["compactness"]), params.compactnessExponent)));
      if ((result<= 100) && (result>= 0))
        score+= result;
    }
    // orientation of main axis
    if (params.mainAxisOrientationExponent > 0) {
      result= static_cast<int>(100 * pow ( abs(static_cast<float>( 
        featureMap["orientation"]/90.0f)),params.mainAxisOrientationExponent));
      if ((result<= 100) && (result>= 0))
        score+= result;
    }
    // dRatio
    if (params.dRatioExponent >0) {
      a= featureMap["dfront"]+ featureMap["drear"];   // along main axis
      b= featureMap["dleft"] + featureMap["dright"];  // ortho to main axis
      c= abs( (b/a) - (1.0f/params.dRatioOptRatio) ); // ratio difference
      result= static_cast<int>(100 - (100 * pow(
        (params.dRatioWidth * c), params.dRatioExponent)));
      result= (result > 100 ? 0 : result);
      score+= (result < INT_MIN/numOfCriteria ? INT_MIN/numOfCriteria : 
        result);
    }
    // horizontal distance
    if (params.hDistExponent > 0) {
      expected = cols * params.hDistExpectedPosition / 100;
      diff = abs(featureMap["xcog"] - expected);
      result= static_cast<int>(100 - (100 * pow (
        (diff*params.hDistWidth*2/cols), params.hDistExponent)));
      result= (result > 100 ? 0 : result);
      score+= (result < INT_MIN/numOfCriteria ? INT_MIN/numOfCriteria: result);
    }
    // vertical distance
    if (params.vDistExponent > 0) {
      expected = rows * params.vDistExpectedPosition / 100;
      diff = abs(featureMap["ycog"] - expected);
      result= static_cast<int>(100 - (100 * pow (
        (diff*params.vDistWidth*2/rows), params.vDistExponent)));
      result= (result > 100 ? 0 : result);
      score+= (result < INT_MIN/numOfCriteria ? INT_MIN/numOfCriteria : 
        result);
    }
    // blob size
    if (params.sizeExponent > 0) {
      a = (cols * rows) * params.sizeMin / 100;
      b = (cols * rows) * params.sizeMax / 100;
      result= static_cast<int>(100 - (100 * pow (
        abs(2.0f*featureMap["areasize"]-a-b)/(b-a),params.sizeExponent)));
      result= (result > 100 ? 0 : result);
      score+= (result < INT_MIN/numOfCriteria ? INT_MIN/numOfCriteria : 
        result);
    }
    // threshold
    if (params.thresholdProbabilityExponent > 0) {
      result= static_cast<int> (100 * pow ( static_cast<float>(
        static_cast<float>(threshold)/255.0f), 
        (1.0f/params.thresholdProbabilityExponent)));
      if ((result<= 100) && (result>= 0))
        score+= result;
    }
  }
  
  // -------------------------------------------------------------------------
  void faceThreshold::interpolate (borderPoints &blob, 
                                   const int &cols, 
                                   const int &rows) const {
    const parameters& params = getParameters();
    if (params.interpolationStep <= 0)
      return;
    int max  = (cols > rows ? cols : rows);
    int step = static_cast<int>((static_cast<float>(
      (max*params.interpolationStep)/100)));
    if (step <= 1)
      return;
    polygonPoints poly;
    // convert blob into a polygon
    poly.approximate (blob, -1, step, true, false);
    // convert back to border points
    blob.castFrom (poly);
  }
}
