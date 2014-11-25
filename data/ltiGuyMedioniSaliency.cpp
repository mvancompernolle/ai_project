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
 * file .......: ltiGuyMedioniSaliency.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 19.5.2003
 * revisions ..: $Id: ltiGuyMedioniSaliency.cpp,v 1.12 2006/09/05 10:14:47 ltilib Exp $
 */


#include "ltiGuyMedioniSaliency.h"
#include "ltiConstants.h"
#include <limits>

#undef _LTI_DEBUG
// #define _LTI_DEBUG 1
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#include "ltiViewer.h"
#endif

namespace lti {
  // --------------------------------------------------
  // guyMedioniSaliency::parameters
  // --------------------------------------------------

  // default constructor
  guyMedioniSaliency::parameters::parameters()
    : saliency::parameters() {
    
    highCurvatureDecay = float(2.85f);
    proximityDecay     = float(0.003f);
    threshold          = float(0.01f); // 1% necessary to be considered
    fieldThreshold     = float(0.1f);
    maxFieldRadius     = int(128);
  }

  // copy constructor
  guyMedioniSaliency::parameters::parameters(const parameters& other)
    : saliency::parameters() {
    copy(other);
  }

  // destructor
  guyMedioniSaliency::parameters::~parameters() {
  }

  // get type name
  const char* guyMedioniSaliency::parameters::getTypeName() const {
    return "guyMedioniSaliency::parameters";
  }

  // copy member

  guyMedioniSaliency::parameters&
  guyMedioniSaliency::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    saliency::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    saliency::parameters& (saliency::parameters::* p_copy)
      (const saliency::parameters&) =
      saliency::parameters::copy;
    (this->*p_copy)(other);
# endif

    highCurvatureDecay = other.highCurvatureDecay;
    proximityDecay = other.proximityDecay;
    threshold = other.threshold;
    fieldThreshold = other.fieldThreshold;
    maxFieldRadius = other.maxFieldRadius;

    return *this;
  }

  // alias for copy member
  guyMedioniSaliency::parameters&
    guyMedioniSaliency::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* guyMedioniSaliency::parameters::clone() const {
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
  bool guyMedioniSaliency::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool guyMedioniSaliency::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {      
      lti::write(handler,"highCurvatureDecay",highCurvatureDecay);
      lti::write(handler,"proximityDecay",proximityDecay);
      lti::write(handler,"threshold",threshold);
      lti::write(handler,"fieldThreshold",fieldThreshold);
      lti::write(handler,"maxFieldRadius",maxFieldRadius);      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && saliency::parameters::write(handler,false);
# else
    bool (saliency::parameters::* p_writeMS)(ioHandler&,const bool) const =
      saliency::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool guyMedioniSaliency::parameters::write(ioHandler& handler,
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
  bool guyMedioniSaliency::parameters::read(ioHandler& handler,
                                            const bool complete)
# else
  bool guyMedioniSaliency::parameters::readMS(ioHandler& handler,
                                              const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {      
      lti::read(handler,"highCurvatureDecay",highCurvatureDecay);
      lti::read(handler,"proximityDecay",proximityDecay);
      lti::read(handler,"threshold",threshold);
      lti::read(handler,"fieldThreshold",fieldThreshold);
      lti::read(handler,"maxFieldRadius",maxFieldRadius);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && saliency::parameters::read(handler,false);
# else
    bool (saliency::parameters::* p_readMS)(ioHandler&,const bool) =
      saliency::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool guyMedioniSaliency::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // guyMedioniSaliency
  // --------------------------------------------------

  // default constructor
  guyMedioniSaliency::guyMedioniSaliency()
    : saliency() {

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  guyMedioniSaliency::guyMedioniSaliency(const parameters& par)
    : saliency() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  guyMedioniSaliency::guyMedioniSaliency(const guyMedioniSaliency& other)
    : saliency() {
    copy(other);
  }

  // destructor
  guyMedioniSaliency::~guyMedioniSaliency() {
  }

  // returns the name of this type
  const char* guyMedioniSaliency::getTypeName() const {
    return "guyMedioniSaliency";
  }

  // copy member
  guyMedioniSaliency&
    guyMedioniSaliency::copy(const guyMedioniSaliency& other) {
      saliency::copy(other);

    return (*this);
  }

  // alias for copy member
  guyMedioniSaliency&
    guyMedioniSaliency::operator=(const guyMedioniSaliency& other) {
    return (copy(other));
  }

  // clone member
  functor* guyMedioniSaliency::clone() const {
    return new guyMedioniSaliency(*this);
  }

  // return parameters
  const guyMedioniSaliency::parameters&
    guyMedioniSaliency::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool guyMedioniSaliency::updateParameters() {
    return generateExtensionField();
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  void guyMedioniSaliency::accumulate(const channel& edgeRelevance,
                                      const channel& orientation,
                                      const float threshold) {

    sumxy.resize(edgeRelevance.size(),0.0f,false,true);
    sumx2.resize(edgeRelevance.size(),0.0f,false,true);
    sumy2.resize(edgeRelevance.size(),0.0f,false,true);
    point p;
    for (p.y=0;p.y<edgeRelevance.rows();++p.y) {
      for (p.x=0;p.x<edgeRelevance.columns();++p.x) {
        if (edgeRelevance.at(p) >= threshold) {
          vote(p,orientation.at(p),edgeRelevance.at(p));
        }
      }
    }
  }

  void guyMedioniSaliency::getSaliency(channel& salLambdaMax,
                                       channel& salLambdaMin) {
    // "covariance" of each pixel
    float covarxx,covarxy,covaryy,tmp,tmp2;
    point p;

    for (p.y=0;p.y<salLambdaMax.rows();++p.y) {
      for (p.x=0;p.x<salLambdaMax.columns();++p.x) {

        // not exaclty the covariance, but something similar.
        covarxx = sumx2.at(p);
        covaryy = sumy2.at(p);

        tmp2 = (covarxx+covaryy);
        if (tmp2<=std::numeric_limits<float>::epsilon()) {
          // try to avoid expensive extra computations
          salLambdaMax.at(p) = 0.0f;
          salLambdaMin.at(p) = 0.0f;          
        } else {
          covarxy = sumxy.at(p);
          
          // compute eigenvalues
          tmp  = (covarxx-covaryy);
          tmp *= tmp;
          tmp += (4.0f*covarxy*covarxy);
          tmp  = sqrt(tmp);

          // both eigenvalues:
          salLambdaMax.at(p) = (tmp2+tmp)/2.0f;
          salLambdaMin.at(p) = (tmp2-tmp)/2.0f;
        }
      }
    }    
  }

  void guyMedioniSaliency::getSaliency(channel& sal) {

    // "covariance" of each pixel
    float covarxx,covarxy,covaryy,tmp,tmp2;
    point p;

    for (p.y=0;p.y<sal.rows();++p.y) {
      for (p.x=0;p.x<sal.columns();++p.x) {

        // not exaclty the covariance, but something similar.
        covarxx = sumx2.at(p);
        covaryy = sumy2.at(p);

        tmp2 = (covarxx+covaryy);
        if (tmp2<=std::numeric_limits<float>::epsilon()) {
          // try to avoid expensive extra computations
          sal.at(p) = 0.0f;
        } else {
          covarxy = sumxy.at(p);
          
          // compute eigenvalues
          tmp  = (covarxx-covaryy);
          tmp *= tmp;
          tmp += (4.0f*covarxy*covarxy);
          tmp  = sqrt(tmp);

          // both eigenvalues:
          sal.at(p) = tmp;
        }
      }
    }    
  }

  void guyMedioniSaliency::getSaliency(channel& sal,
                                       channel& angle,
                                       float& maxSaliency) {

    // "covariance" of each pixel
    float covarxx,covarxy,covaryy,tmp,tmp2,a;
    point p;
    maxSaliency=0.0f;

    for (p.y=0;p.y<sal.rows();++p.y) {
      for (p.x=0;p.x<sal.columns();++p.x) {

        // not exaclty the covariance, but something similar.
        covarxx = sumx2.at(p);
        covaryy = sumy2.at(p);

        tmp2 = (covarxx+covaryy);
        if (tmp2<=std::numeric_limits<float>::epsilon()) {
          // try to avoid expensive extra computations
          sal.at(p) = 0.0f;
          angle.at(p) = 0.0f;
        } else {
          covarxy = sumxy.at(p);
          
          // compute eigenvalues
          tmp2 = tmp  = (covaryy-covarxx);
          tmp *= tmp;
          tmp += (4.0f*covarxy*covarxy);
          tmp  = sqrt(tmp);

          // both eigenvalues:
          sal.at(p) = tmp;
          maxSaliency=max(maxSaliency,tmp);

          if (covarxy > std::numeric_limits<float>::epsilon()) {
            a = atan2((tmp2+tmp),(2.0f*covarxy));
          } else {
            if (covarxx > covaryy) {
              a = 0.0f;
            } else {
              a = constants<float>::HalfPi();
            }
          }

          // output angle must be perpendicular to edgel direction
          a -= constants<float>::HalfPi(); 

          // fix angle
          while (a<0) {
            a+=constants<float>::TwoPi();
          }
          while (a>constants<float>::TwoPi()) {
            a-=constants<float>::TwoPi();
          }

          angle.at(p)=a;

        }
      }
    }    
  }

  bool guyMedioniSaliency::apply(const channel& edgeRelevance,
                                 const channel& orientation,
                                 channel& salLambdaMax,
                                 channel& salLambdaMin) {

    const parameters& par = getParameters();    
    const float threshold = par.threshold*edgeRelevance.maximum();

    if (edgeRelevance.size() != orientation.size()) {
      setStatusString("Both input channels must have the same size");
      return false;
    }

    // initialize counters with 0
    salLambdaMax.resize(edgeRelevance.size(),float(),false,false);
    salLambdaMin.resize(edgeRelevance.size(),float(),false,false);

    if (edgeRelevance.empty()) {
      return true;
    }

    accumulate(edgeRelevance,orientation,threshold);
    getSaliency(salLambdaMax,salLambdaMin);

    return true;
  }
  
  bool guyMedioniSaliency::apply(const channel& edgeRelevance,
                                 const channel& orientation,
                                 channel& sal) {

    const parameters& par = getParameters();    
    const float threshold = par.threshold*edgeRelevance.maximum();

    if (edgeRelevance.size() != orientation.size()) {
      setStatusString("Both input channels must have the same size");
      return false;
    }

    // initialize counters with 0
    sal.resize(edgeRelevance.size(),float(),false,false);

    if (edgeRelevance.empty()) {
      return true;
    }

    accumulate(edgeRelevance,orientation,threshold);
    getSaliency(sal);

    return true;
  }

  bool guyMedioniSaliency::apply(const channel& edgeRelevance,
                                 const channel& orientation,
                                 channel& sal,
                                 channel& angle,
                                 float& maxSaliency) {

    const parameters& par = getParameters();    
    const float threshold = par.threshold*edgeRelevance.maximum();

    if (edgeRelevance.size() != orientation.size()) {
      setStatusString("Both input channels must have the same size");
      return false;
    }

    // initialize counters with 0
    sal.resize(edgeRelevance.size(),float(),false,false);
    angle.resize(edgeRelevance.size(),float(),false,false);

    if (edgeRelevance.empty()) {
      return true;
    }

    accumulate(edgeRelevance,orientation,threshold);
    getSaliency(sal,angle,maxSaliency);

    return true;
  }

  // ------------------------------------------------------------------------

  const matrix<float>& guyMedioniSaliency::getExtensionFieldX() const {
    return extensionFieldX;
  }

  const matrix<float>& guyMedioniSaliency::getExtensionFieldY() const {
    return extensionFieldY;   
  }

  const matrix<float>& guyMedioniSaliency::getExtensionFieldM() const {
    return extensionFieldM;
  }

  // ------------------------------------------------------------------------

  bool guyMedioniSaliency::generateExtensionField() {
    const parameters& par = getParameters();
    
    // 1st step. Check if required parameters are valid
    if ((par.fieldThreshold <= 0.0f) || (par.fieldThreshold >= 1.0f)) {
      setStatusString("Wrong fieldThreshold.  Must be in ]0,1[");
      return false;
    }
    if (par.proximityDecay <= 0.0f) {
      setStatusString("Wrong proximityDecay. Must be > 0.0f");
      return false;
    }
    if (par.highCurvatureDecay <= 0.0f) {
      setStatusString("Wrong highCurvatureDecay. Must be > 0.0f");
      return false;
    }

    // 2nd step. Determine the radius of the circle    
    int rad;
    // at this time 
    fieldThreshold = par.fieldThreshold;
    rad = min(iround(ceil(sqrt(-log(fieldThreshold)/par.proximityDecay))),
              par.maxFieldRadius);


    extFieldRadius = rad;
    rad=2*rad + 1;
    
    extensionFieldX.resize(rad,rad,0.0f,false,true);
    extensionFieldY.resize(rad,rad,0.0f,false,true);
    extensionFieldM.resize(rad,rad,0.0f,false,true);

    extFieldCenter.set(extFieldRadius,extFieldRadius);

    // 3rd step. Generate the io points
    int x,y;
    extFieldCircle.resize(2,extensionFieldX.rows(),int(),false,false);
    for (y=0;y<extFieldCircle.columns();++y) {
      x=y-extFieldCenter.y;      
      float r=static_cast<float>(sqrt(extFieldRadius*extFieldRadius - x*x));
      extFieldCircle.at(0,y)=iround(extFieldCenter.y - r);
      extFieldCircle.at(1,y)=iround(extFieldCenter.y + r);
    }

    // 4th step.  Generate the field
    
    const float A=par.proximityDecay;
    const float B=par.highCurvatureDecay;
    float tmp;

    for (y=0;y<extensionFieldX.rows();++y) {
      const int yp=y-extFieldCenter.y;
      const int end = extFieldCircle.at(1,y);
      for (x=extFieldCircle.at(0,y);x<=end;++x) {
        const int xp=x-extFieldCenter.x;

        // amplitude of the vector field
        tmp = (yp == 0) ? 0.0f : 
          ((xp==0) ? constants<float>::HalfPi() :
                     atan2(abs(static_cast<float>(yp)),
                           abs(static_cast<float>(xp))));
        float mag=exp( -A*xp*xp - B*tmp*tmp );

        // and the orientation
        if (yp==0) {
          extensionFieldX.at(y,x) = mag;
          extensionFieldY.at(y,x) = 0.0f;          
        } else {
          tmp = ((xp*xp)+(yp*yp))/(2.0f*yp);
          extensionFieldX.at(y,x) = mag*(1.0f-(static_cast<float>(yp)/tmp));

          // invert the y component to compensate the left-handed
          // image coordinate system
          extensionFieldY.at(y,x) = -mag*(static_cast<float>(xp)/tmp);
        }
        extensionFieldM.at(y,x) = mag;
      }
    }

    // initialize interpolators.
    ifieldX.setBoundaryType(lti::Zero);
    ifieldY.setBoundaryType(lti::Zero);

    ifieldX.use(extensionFieldX);
    ifieldY.use(extensionFieldY);

    return true;
  }

  // ------------------------------------------------------------------------

  void guyMedioniSaliency::vote(const point p,
                                const float angle,
                                const float weight) {

#ifdef _LTI_DEBUG
    static viewer viewm("Mag Field");
    static viewer viewa("Ang Field");
    static viewer viewsumx("SumX");
    static viewer viewsumy("SumY");
    _lti_debug("Vote at " << p << ", " << angle << " degrees, " << 
               ", " << weight << " weight." << std::endl);
    static channel fieldM,fieldA;
    fieldM.resize(n.size(),0.0f,false,true);
    fieldA.resize(n.size(),0.0f,false,true);
#endif    


    const int miny=max(0,p.y-extFieldRadius);
    const int maxy=min(sumx2.rows(),p.y+extFieldRadius+1);
    const float radius = static_cast<float>(extFieldRadius);
    int x,y;

    // TODO: sin and cos with LUT
    // const float cosa = cos(angle);
    // const float sina = sin(angle);

    // the input angle is perpendicular to the edge direction, so let's
    // add 90 degrees to it before computing the cos and sin.
    
    float ctmp, stmp;
    sincos(angle, stmp, ctmp);

    const float cosa = -stmp; // = cos(angle + Pi/2)
    const float sina =  ctmp; // = sin(angle + Pi/2)

    for (y=miny;y<maxy;++y) {
      const int yp   = y-(p.y-extFieldRadius);
      const int minx = max(0,extFieldCircle.at(0,yp) - extFieldRadius + p.x);
      const int maxx = min(sumx2.lastColumn(),
                           extFieldCircle.at(1,yp) - extFieldRadius + p.x);
      for (x=minx;x<=maxx;++x) {
        // compute the rotated coordinates to access the field
        const int dx = (x-p.x);
        const int dy = (y-p.y);

        const float xf = cosa*dx+sina*dy + radius;
        const float yf = cosa*dy-sina*dx + radius;

        // consider only those points in the field with enough magnitude.
        if (extensionFieldM.at(iround(yf),iround(xf)) >= fieldThreshold) {

          const float xt = ifieldX.interpolate(yf,xf);
          const float yt = ifieldY.interpolate(yf,xf);
          
          const float xval = weight*(cosa*xt+sina*yt);
          const float yval = weight*(cosa*yt-sina*xt);
          
          sumx2.at(y,x) += (xval*xval);
          sumy2.at(y,x) += (yval*yval);
          sumxy.at(y,x) += (xval*yval);

#ifdef _LTI_DEBUG
          // show the extension field used for this point, with magnitude and
          // phase.
          fieldM.at(y,x) = extensionFieldM.at(iround(yf),iround(xf));
          fieldA.at(y,x) = atan2(yval,xval);
#endif
        }
      }
    }

#ifdef _LTI_DEBUG
//     viewm.show(fieldM);
//     viewa.show(fieldA);
    viewsumx.show(sumx2);
    viewsumy.show(sumy2);
    
    // viewm.waitKey();
#endif

  }
}
