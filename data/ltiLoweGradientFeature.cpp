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
 * file .......: ltiLoweGradientFeature.cpp
 * authors ....: Frederik Lange, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 30.11.2003
 * revisions ..: $Id: ltiLoweGradientFeature.cpp,v 1.10 2006/09/05 10:21:24 ltilib Exp $
 */

#include "ltiLoweGradientFeature.h"

namespace lti {
  // --------------------------------------------------
  // loweGradientFeature::parameters
  // --------------------------------------------------

  // default constructor
  loweGradientFeature::parameters::parameters()
    : featureExtractor::parameters() {

    sigma = 3.5;
    locationPartition = 4;
    histogramPartition = 4;
    orientationBins = 8;
    pyramidLevels = int(15);
    // gradientParam = colorContrastGradient::parameters();
    // pyramidParam = scaleSpacePyramid::parameters();
    locationRelativeRadius = 7.0f;

    normalize = true;
    cutThreshold = 0.2f;
    
  }

  // copy constructor
  loweGradientFeature::parameters::parameters(const parameters& other)
    : featureExtractor::parameters() {
    copy(other);
  }

  // destructor
  loweGradientFeature::parameters::~parameters() {
  }

  // get type name
  const char* loweGradientFeature::parameters::getTypeName() const {
    return "loweGradientFeature::parameters";
  }

  // copy member

  loweGradientFeature::parameters&
  loweGradientFeature::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    featureExtractor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    featureExtractor::parameters& (featureExtractor::parameters::* p_copy)
      (const featureExtractor::parameters&) =
      featureExtractor::parameters::copy;
    (this->*p_copy)(other);
# endif

    sigma = other.sigma;
    locationPartition = other.locationPartition;
    histogramPartition = other.histogramPartition;
    orientationBins = other.orientationBins;
    gradientParam.copy(other.gradientParam);
    pyramidLevels = other.pyramidLevels;
    pyramidParam.copy(other.pyramidParam);
    locationRelativeRadius = other.locationRelativeRadius;
    normalize = other.normalize;
    cutThreshold = other.cutThreshold;

    return *this;
  }

  // alias for copy member
  loweGradientFeature::parameters&
    loweGradientFeature::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* loweGradientFeature::parameters::clone() const {
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
  bool loweGradientFeature::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool loweGradientFeature::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"sigma",sigma);
      lti::write(handler,"locationPartition",locationPartition);
      lti::write(handler,"histogramPartition",histogramPartition);
      lti::write(handler,"orientationBins",orientationBins);

      lti::write(handler,"gradientParam",gradientParam);
      lti::write(handler,"pyramidLevels",pyramidLevels);
      lti::write(handler,"pyramidParam",pyramidParam);
      lti::write(handler,"locationRelativeRadius",locationRelativeRadius);

      lti::write(handler,"normalize",normalize);
      lti::write(handler,"cutThreshold",cutThreshold);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && featureExtractor::parameters::write(handler,false);
# else
    bool (featureExtractor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      featureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool loweGradientFeature::parameters::write(ioHandler& handler,
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
   * @return true if read was successful
   */
# ifndef _LTI_MSC_6
  bool loweGradientFeature::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool loweGradientFeature::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"sigma",sigma);
      lti::read(handler,"locationPartition",locationPartition);
      lti::read(handler,"histogramPartition",histogramPartition);
      lti::read(handler,"orientationBins",orientationBins);

      lti::read(handler,"gradientParam",gradientParam);
      lti::read(handler,"pyramidLevels",pyramidLevels);
      lti::read(handler,"pyramidParam",pyramidParam);
      lti::read(handler,"locationRelativeRadius",locationRelativeRadius);

      lti::read(handler,"normalize",normalize);
      lti::read(handler,"cutThreshold",cutThreshold);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && featureExtractor::parameters::read(handler,false);
# else
    bool (featureExtractor::parameters::* p_readMS)(ioHandler&,const bool) =
      featureExtractor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool loweGradientFeature::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // loweGradientFeature
  // --------------------------------------------------

  // default constructor
  loweGradientFeature::loweGradientFeature()
    : featureExtractor() {

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  loweGradientFeature::loweGradientFeature(const parameters& par)
    : featureExtractor() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  loweGradientFeature::loweGradientFeature(const loweGradientFeature& other)
    : featureExtractor() {
    copy(other);
  }

  // destructor
  loweGradientFeature::~loweGradientFeature() {
  }

  // returns the name of this type
  const char* loweGradientFeature::getTypeName() const {
    return "loweGradientFeature";
  }

  // copy member
  loweGradientFeature&
  loweGradientFeature::copy(const loweGradientFeature& other) {
    featureExtractor::copy(other);
    return (*this);
  }

  // alias for copy member
  loweGradientFeature&
  loweGradientFeature::operator=(const loweGradientFeature& other) {
    return (copy(other));
  }


  // clone member
  functor* loweGradientFeature::clone() const {
    return new loweGradientFeature(*this);
  }

  // return parameters
  const loweGradientFeature::parameters&
  loweGradientFeature::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // set parameters
  bool loweGradientFeature::updateParameters() {
    return initMasks();
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  /*
   * Extracts local features at the locations for a gray-valued input
   * channel.
   */
  bool loweGradientFeature::apply(const channel &src, 
                                  const std::list<location> &locs, 
                                  std::list<dvector> &vcts) {
    generate(src);
    return apply(locs,vcts);
  }
    
  /*
   * Extracts the color features at the loctions
   */
  bool loweGradientFeature::apply(const image &src,
                                  const std::list<location> &locs, 
                                  std::list<dvector> &vcts) {
    generate(src);
    return apply(locs,vcts);
  }
  
  /*
   * Extract features based on a previously generated pyramid set.
   *
   * The decision if you want color or gray features is taken at the time
   * of the pyramid generation.
   *
   * @see generate()
   */
  bool loweGradientFeature::apply(const std::list<location> &locs,
                                  std::list<dvector> &vcts) {

    vcts.clear();
    std::list<location>::const_iterator it;
    const dvector vct; // empty vector
    for (it=locs.begin();it!=locs.end();++it) {
      vcts.push_back(vct);
      apply(*it,vcts.back());
    }

    return true;
  }

  // only one feature vector
  bool loweGradientFeature::apply(const location& loc,
                                  dvector& vct) {
    
    const parameters& par = getParameters();

    // compute the level of the pyramid equivalent for the location
    const int level = iround(pyrMag.getScaleForRadius(loc.radius/
                                              (par.locationRelativeRadius)));

    if (level < 0 || level >= pyrMag.size()) {
      vct.resize(numHistograms*par.orientationBins,0.0,false,true);
      return false;
    }

    dmatrix hists;
    computeHistograms(level,
                      loc.position,
                      loc.angle,
                      hists);
    hists.detach(vct);

    // normalize the vector length

    int x;
    if (par.normalize) {
      const double norm = sqrt(vct.dot(vct));
      if (norm > 0.0f) {
        if (par.cutThreshold < 1.0f) {
          bool cut = false;
          // Cut all entries greater than the threshold
          const float thresh = static_cast<float>(norm*par.cutThreshold);
          for (x=0;x<vct.size();++x) {
            if (vct.at(x) > thresh) {
              // cut
              vct.at(x)=par.cutThreshold;
              cut = true;
            } else {
              // just scale
              vct.at(x) /= norm;
            }
          }
          if (cut) {
            // something changed, just renormalize
            vct.divide(sqrt(vct.dot(vct)));
          }
        } else {
          vct.divide(norm);
        }
      }
    }

    return true;
  }


  /*
   * Generate and internal pyramidal representation for the given color
   * image, which can be later analyzed with the respective apply method.
   */
  bool loweGradientFeature::generate(const image &src) {
    const parameters& par = getParameters();

    // generate the channels pyramid
    scaleSpacePyramid<image>::parameters ipar;
    
    ipar.automaticKernel = par.pyramidParam.automaticKernel;
    ipar.kernelSize      = par.pyramidParam.kernelSize;
    ipar.kernelVariance  = par.pyramidParam.kernelVariance;
    ipar.gaussian        = par.pyramidParam.gaussian;
    ipar.factor          = par.pyramidParam.factor;
    ipar.interpolatorType= par.pyramidParam.interpolatorType;

    scaleSpacePyramid<image> pyr(par.pyramidLevels,ipar);
    pyr.generate(src);
    
    // for each level of the channel pyramid generate the gradients
    pyrMag.resize(par.pyramidLevels);
    pyrArg.resize(par.pyramidLevels);
    pyrMag.setParameters(par.pyramidParam);
    pyrArg.setParameters(par.pyramidParam);

    colorContrastGradient ccg(par.gradientParam);

    int i;
    for (i=0;i<par.pyramidLevels;++i) {
      ccg.apply(pyr.at(i),pyrMag.at(i),pyrArg.at(i));
    }
    
    return true;
  }

  /*
   * Generate and internal pyramidal representation for the given gray
   * image, which can be later analyzed with the respective apply method.
   */
  bool loweGradientFeature::generate(const channel &src) {
    const parameters& par = getParameters();

    // generate the channels pyramid
    scaleSpacePyramid<channel> pyr(par.pyramidLevels,par.pyramidParam);
    pyr.generate(src);
    
    // for each level of the channel pyramid generate the gradients
    pyrMag.resize(par.pyramidLevels);
    pyrArg.resize(par.pyramidLevels);
    pyrMag.setParameters(par.pyramidParam);
    pyrArg.setParameters(par.pyramidParam);

    colorContrastGradient ccg(par.gradientParam);

    int i;
    for (i=0;i<par.pyramidLevels;++i) {
      ccg.apply(pyr.at(i),pyrMag.at(i),pyrArg.at(i));
    }
    
    return true;
  }

  // compute histograms
  bool loweGradientFeature::computeHistograms(const int level,
                                              const tpoint<float>& pos,
                                              const float angle,
                                              matrix<double>& hists) const {
    const parameters& par = getParameters();
    hists.resize(numHistograms,par.orientationBins,0.0,false,true);

    //2*pi
    const float twoPi=2.f*static_cast<float>(Pi);

    // we need a transformation from the location to the image space.
    float stmp, ctmp;
    sincos(angle, stmp, ctmp);
    const float ssina(stmp);
    const float scosa(ctmp);

    int y,x,i,ia;
    float fy,fx,cx,cy,a,w,fa;

    const float factor = static_cast<float>(par.orientationBins/(2.0*Pi));

    const uint32 maxx = pyrMag.at(0).lastColumn();
    const uint32 maxy = pyrMag.at(0).lastRow();
    
    for (y=0;y<hmask.rows();++y) {
      cy = y - maskCenter.y;
      const int out = ioPts.at(1,y);
      for (x=ioPts.at(0,y);x<=out;++x) {
        cx = x - maskCenter.x;

        fx = cx*scosa - cy*ssina + pos.x;
        fy = cx*ssina + cy*scosa + pos.y;

        if ((static_cast<uint32>(fx) > maxx) ||
            (static_cast<uint32>(fy) > maxy)) {
          continue;
        }

        i = hmask.at(y,x);
        w = pyrMag.bilinearAt(fy,fx,level)*weight.at(y,x);
        a = pyrArg.nearestAt(fy,fx,level); // nearest, to avoid
                                           // zero-Pi discontinuity problem

        //angle is relative to location angle
        a-=angle;
        while (a<0.f) a+=twoPi;
        while (a>twoPi) a-=twoPi;

        // compute which entries in the histogram need to be updated
        fa = a*factor;
        ia = static_cast<int>(fa);
        fa -= static_cast<float>(ia);
        
        hists.at(i,ia) += w*(1.0f-fa);
        hists.at(i,(ia+1)%hists.columns()) += w*fa;
      }
    }
    
    return true;
  }
                                                
  // initialize masks
  bool loweGradientFeature::initMasks() {
    const parameters& par = getParameters();
    const int m = par.locationPartition*par.histogramPartition;
    numHistograms = par.locationPartition*par.locationPartition;

    hmask.resize(m,m,0,false,false);
    weight.resize(m,m,0.0f,false,false);
    ioPts.resize(2,m,0);

    // center of the masks
    maskCenter.castFrom(hmask.size());
    maskCenter.subtract(tpoint<float>(1.0f,1.0f));
    maskCenter.divide(2.0f);
    
    int y,x,i,ix,iy;
    float fx,fy,d,s,y2;
    const float r2 = maskCenter.y*maskCenter.y;
    s = par.sigma*par.sigma*2.0f;
    for (y=0;y<m;++y) {
      fy = static_cast<float>(y)-maskCenter.y;

      // the in and out points correspond to the borders of a circle:
      y2 = (fy*(fy+1.0f)) + 0.25f; // == (i+0.5)^2

      ioPts.at(0,y) = (fy <= 0) ? iround(-sqrt(r2-y2) + maskCenter.x) : 
                                  ioPts.at(0,m-1-y);
      ioPts.at(1,y) = (fy <= 0) ? iround(sqrt(r2-y2) + maskCenter.x) : 
                                  ioPts.at(1,m-1-y);

      iy = y/par.histogramPartition; // == y*par.locationPartition/m;
      for (x=0;x<m;++x) {
        fx = static_cast<float>(x)-maskCenter.x;
        d  = fx*fx + fy*fy;
        ix = x/par.histogramPartition;
        i  = iy*par.locationPartition + ix;
             
        hmask.at(y,x)=i;
        weight.at(y,x)=exp(-d/s);
      }
    }

    return true;
  }

  const imatrix& loweGradientFeature::getHistogramMask() const {
    return hmask;
  }

  const matrix<float>& loweGradientFeature::getWeightMask() const {
    return weight;
  }

  const scaleSpacePyramid<channel>&
  loweGradientFeature::getMagnitudePyramid() const {
    return pyrMag;
  }

  const scaleSpacePyramid<channel>&
  loweGradientFeature::getAnglePyramid() const {
    return pyrArg;
  }


}
