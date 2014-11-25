/*
 * Copyright (C) 2004, 2005, 2006
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
 * file .......: ltiLocalFeatureExtractor.cpp
 * authors ....: Arnd Hannemann
 * organization: LTI, RWTH Aachen
 * creation ...: 5.4.2004
 * revisions ..: $Id: ltiLocalFeatureExtractor.cpp,v 1.9 2006/09/05 10:20:48 ltilib Exp $
 */

#include "ltiLocalFeatureExtractor.h"
#include "ltiConstants.h"
#include "ltiSplitImageToRGB.h"

#undef _LTI_DEBUG
// #define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {

  // function which concatenates a vector to another
  inline void concat(dvector& srcdest, const dvector& second) {
    int offset = srcdest.size();
    srcdest.resize(offset+second.size(),0.0,true,false);
    dvector::const_iterator itt;
    dvector::iterator itd;
    for (itd=srcdest.begin()+offset, itt=second.begin();
         itd!=srcdest.end();itd++,itt++) {
      *itd = *itt;
    }
  }

  // --------------------------------------------------
  // localFeatureExtractor::parameters
  // --------------------------------------------------

  // default constructor
  localFeatureExtractor::parameters::parameters()
#ifndef _LTI_MSC_6
    : featureExtractor::parameters() 
#endif
  {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters
    
  }

  // copy constructor
  localFeatureExtractor::parameters::parameters(const parameters& other)
#ifndef _LTI_MSC_6
    : featureExtractor::parameters() 
#endif 
  {
    copy(other);
  }

  // destructor
  localFeatureExtractor::parameters::~parameters() {
  }

  // get type name
  const char* localFeatureExtractor::parameters::getTypeName() const {
    return "localFeatureExtractor::parameters";
  }

  // copy member

  localFeatureExtractor::parameters&
    localFeatureExtractor::parameters::copy(const parameters& other) {
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

    

    return *this;
  }

  // alias for copy member
  localFeatureExtractor::parameters&
    localFeatureExtractor::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* localFeatureExtractor::parameters::clone() const {
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
  bool localFeatureExtractor::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool localFeatureExtractor::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && featureExtractor::parameters::write(handler,false);
# else
    bool (featureExtractor::parameters::* p_writeMS)
      (ioHandler&,const bool) const =
      featureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool localFeatureExtractor::parameters::write(ioHandler& handler,
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
  bool localFeatureExtractor::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool localFeatureExtractor::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
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
  bool localFeatureExtractor::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // localFeatureExtractor
  // --------------------------------------------------

  // default constructor
  localFeatureExtractor::localFeatureExtractor()
    : featureExtractor(){

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  localFeatureExtractor::localFeatureExtractor(const parameters& par)
    : featureExtractor() {

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  localFeatureExtractor::
  localFeatureExtractor(const localFeatureExtractor& other)
    : featureExtractor() {
    copy(other);
  }

  // destructor
  localFeatureExtractor::~localFeatureExtractor() {
  }

  // returns the name of this type
  const char* localFeatureExtractor::getTypeName() const {
    return "localFeatureExtractor";
  }

  // copy member
  localFeatureExtractor&
  localFeatureExtractor::copy(const localFeatureExtractor& other) {
    featureExtractor::copy(other);

    return (*this);
  }

  // alias for copy member
  localFeatureExtractor&
    localFeatureExtractor::operator=(const localFeatureExtractor& other) {
    return (copy(other));
  }


  // clone member
  functor* localFeatureExtractor::clone() const {
    return new localFeatureExtractor(*this);
  }

  // return parameters
  const localFeatureExtractor::parameters&
    localFeatureExtractor::getParameters() const {
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


  bool localFeatureExtractor::apply(const lti::image& src,
                                           const std::list<location>& locs,
                                            std::list<dvector>& dest) const {
    bool rc = true;
    channel red,green,blue;
    splitImageToRGB splitter;
    splitter.apply(src,red,green,blue);
    std::list<location>::const_iterator it;
    dest.clear();
    dvector tmp;
    for (it = locs.begin(); ((it != locs.end()) && rc); it++) {
      dvector dest2;
      rc = rc && apply(red,*it,dest2); 
      rc = rc && apply(green,*it,tmp);
      concat(dest2,tmp);
      rc = rc && apply(red,*it,tmp);
      concat(dest2,tmp);
      dest.push_back(dest2);
    }
    return rc;
  };


  bool localFeatureExtractor::apply(const lti::channel8& src,
                                    const std::list<location>& locs,
                                    std::list<dvector>& dest) const { 
    channel tmp;
    tmp.castFrom(src);
    return apply(tmp,locs,dest);
  }


  bool localFeatureExtractor::apply(const std::list<channel>& src,
                                    const std::list<location>& locs,
                                    std::list<dvector>& dest) const {
    setStatusString("Method not implemented");
    return false;
  }

  bool localFeatureExtractor::apply(const lti::channel& src,
                                    const std::list<location>& locs,
                                    std::list<dvector>& dest) const {
    bool rc = true;
    std::list<location>::const_iterator it;
    dest.clear();
    dvector tmp;
    for (it = locs.begin(); ((it != locs.end()) && rc); it++) {
      rc = rc && apply(src,*it,tmp);
      dest.push_back(tmp);
    }
    return rc;
  };

  // splitting image into channels, apply on each channel and
  // concatenate the resulting vectors
  bool localFeatureExtractor::apply(const lti::channel& src,
                                    const location& loc,
                                    dvector& dest) const {
    setStatusString("Method not implemented");
    return false;
  }

  bool localFeatureExtractor::apply(const lti::image& src,
                                            const location& loc,
                                            dvector& dest) const {

    channel red,green,blue;
    splitImageToRGB splitter;
    splitter.apply(src,red,green,blue);
    bool rc = true;
    dvector tmp;
    rc = rc && apply(red,loc,dest); 
    rc = rc && apply(green,loc,tmp);
    concat(dest,tmp);
    rc = rc && apply(red,loc,tmp);
    concat(dest,tmp);
    return rc;
  }


  bool localFeatureExtractor::fixedRaySampling(const channel& chan, 
                                               const location& loc,
                                               fvector& dest,
                                               int numRays, int numCircles, 
                                               float scale,
                                               bool useOrientation,
                                               bool includeCenter,
                                               bool useCircleOrder) const {

    const float frad = loc.radius*scale;
    const int rad = iround(frad);
    const float angdelta = constants<float>::TwoPi() / numRays;
    const float raddelta = frad/ numCircles;
    const float locx = loc.position.x;
    const float locy = loc.position.y;

    // to determine if an unchecked interpolation could be used
    const int width = chan.size().x;
    const int height = chan.size().y;

    int offset = 0;
    if (includeCenter) {
      dest.resize(numRays*numCircles+1,0.f,false,false);
      dest[0] = interpol.interpolate(chan,locy,locx);
      offset = 1;
    } else {
      dest.resize(numRays*numCircles,0.f,false,false);
    }
    int i,j;
    float stmp, ctmp;
    if (!(useOrientation)) {
      if (useCircleOrder) {
        if ((locx-frad>=0) && (iround(locx)+rad<width) && 
            (locy-frad>=0) && (iround(locy)+rad<height)) {
          for (j = 0;j < numCircles;j++) {
            const int b = j*numRays+offset;          
            const float r = static_cast<float>(j+1)*raddelta;
            for (i = 0;i < numRays;i++) {
              const float phi = static_cast<float>(i)*angdelta;
              sincos(phi, stmp, ctmp);
              const float x = locx + r * ctmp;
              const float y = locy + r * stmp;
              dest[b+i] = interpol.interpolateUnchk(chan,y,x);
              _lti_debug("dest["<<j*numRays+i<<"] = interpolateUnchk("<<x
                         <<","<<y<<")\n");
            }
          } // for j
        } // interpolateUnchk
        else {
          for (j = 0;j < numCircles;j++) {
            const int b = j*numRays+offset;
            const float r = static_cast<float>(j+1)*raddelta;
            for (i = 0;i < numRays;i++) {
              const float phi = static_cast<float>(i)*angdelta;
              sincos(phi, stmp, ctmp);
              const float x = locx + r * ctmp;
              const float y = locy + r * stmp;
              dest[b+i] = interpol.interpolate(chan,y,x);
              _lti_debug("dest["<<j*numRays+i<<"] = interpolate("<<x
                         <<","<<y<<")\n");
            }
          } // for j
        }
      } // if (useCircleOrder) 
      else {
        if ((locx-frad>=0) && (iround(locx)+rad<width) && 
            (locy-frad>=0) && (iround(locy)+rad<height)) {
          for (j = 0;j < numRays;j++) {
            const float phi = static_cast<float>(j)*angdelta;
            const int b = j*numCircles+offset;
            for (i = 0;i < numCircles;i++) {
              const float r = static_cast<float>(i+1)*raddelta;
              sincos(phi, stmp, ctmp);
              const float x = locx + r * ctmp;
              const float y = locy + r * stmp;
              dest[b+i] = interpol.interpolateUnchk(chan,y,x);
              _lti_debug("dest["<<j*numRays+i<<
                         "] = interpolateUnchk("<<x<<","<<y<<")\n");
            }
          } // for j
        } else {
          for (j = 0;j < numRays;j++) {
            const float phi = static_cast<float>(j)*angdelta;
            const int b = j*numCircles+offset;
            for (i = 0;i < numCircles;i++) {
              const float r = static_cast<float>(i+1)*raddelta;
              sincos(phi, stmp, ctmp);
              const float x = locx + r * ctmp;
              const float y = locy + r * stmp;
              dest[b+i] = interpol.interpolate(chan,y,x);
              _lti_debug("dest["<<j*numRays+i<<"] = interpolateUnchk("
                         <<x<<","<<y<<")\n");
            }
          } // for j
        } // interpolate
      }
    } // (useOrientation!=true)
    else {
      if (useCircleOrder) {
        if ((locx-frad>=0) && (iround(locx)+rad<width) && 
            (locy-frad>=0) && (iround(locy)+rad<height)) {
          for (j = 0;j < numCircles;j++) {
            const int b = j*numRays+offset;
            const float r = static_cast<float>(j+1)*raddelta;
            for (i = 0;i < numRays;i++) {
              const float phi = static_cast<float>(i)*angdelta+loc.angle;
              sincos(phi, stmp, ctmp);
              const float x = locx + r * ctmp;
              const float y = locy + r * stmp;
              dest[b+i] = interpol.interpolateUnchk(chan,y,x);
              _lti_debug("dest["<<j*numRays+i<<"] = interpolateUnchk("<<x<<","
                         <<y<<")\n");
            }
          } // for j
        } // interpolateUnchk
        else {
          for (j = 0;j < numCircles;j++) {
          const int b = j*numRays+offset;
          for (i = 0;i < numRays;i++) {
            const float r = static_cast<float>(j+1)*raddelta;
            const float phi = static_cast<float>(i)*angdelta+loc.angle;
            sincos(phi, stmp, ctmp);
            const float x = locx + r * ctmp;
            const float y = locy + r * stmp;
            dest[b+i] = interpol.interpolate(chan,y,x);
            _lti_debug("dest["<<j*numRays+i<<"] = interpolate("<<x
                       <<","<<y<<")\n");
          }
          }
        } // interpolate
      } // if (useCircleOrder) 
      else {
        if ((locx-frad>=0) && (iround(locx)+rad<width) && 
            (locy-frad>=0) && (iround(locy)+rad<height)) {
          for (j = 0;j < numRays;j++) {
            const float phi = static_cast<float>(j)*angdelta+loc.angle;
            const int b = j*numCircles+offset;
            for (i = 0;i < numCircles;i++) {
              const float r = static_cast<float>(i+1)*raddelta;
              sincos(phi, stmp, ctmp);
              const float x = locx + r * ctmp;
              const float y = locy + r * stmp;
              dest[b+i] = interpol.interpolateUnchk(chan,y,x);
              _lti_debug("dest["<<j*numRays+i<<"] = interpolateUnchk("<<x
                         <<","<<y<<")\n");
            }
          } // for j
        } else {
          for (j = 0;j < numRays;j++) {
            const float phi = static_cast<float>(j)*angdelta+loc.angle;
            const int b = j*numCircles+offset;
            for (i = 0;i < numCircles;i++) {
              const float r = static_cast<float>(i+1)*raddelta;
              sincos(phi, stmp, ctmp);
              const float x = locx + r * ctmp;
              const float y = locy + r * stmp;
              dest[b+i] = interpol.interpolate(chan,y,x);
              _lti_debug("dest["<<j*numRays+i<<"] = interpolateUnchk("<<x
                         <<","<<y<<")\n");
            }
          } // for j
        } // interpolate
      }
    } // else
    return true;
  }

  bool localFeatureExtractor::fixedRaySampling(const channel& chan,
                                               const std::list<location>& loc,
                                               std::list<fvector>& dest,
                                               int numRays, int numCircles, 
                                               float scale,
                                               bool useOrientation,
                                               bool includeCenter,
                                               bool useCircleOrder) const {
    dest.clear();
    std::list<location>::const_iterator it;
    bool rc = true;
    for (it = loc.begin();((it != loc.end())&&rc); it++) {
      fvector tmp;
      rc = fixedRaySampling(chan,*it,tmp,
                            numRays,numCircles,
                            scale,useOrientation,includeCenter,useCircleOrder);
      dest.push_back(tmp);
    }
    return rc;
  }

  bool localFeatureExtractor::fixedGridSampling(const channel& chan, 
                                                const location& loc,
                                                fvector& dest,
                                                const array<int>& circPoints,
                                                float scale,
                                                bool useOrientation) const {
    const float frad = loc.radius*scale;
    const int rad = iround(frad);
    const float angle = loc.angle;
    const float x = loc.position.x;
    const float y = loc.position.y;
    int i;
    float ty,j,k;

//    interpol.use(chan);

    // calculate number of imagePoints
    int last = circPoints.lastIdx();
    int nbImagePoints = 0;
    for (i = circPoints.firstIdx();i<=last;i++) {
      nbImagePoints += circPoints.at(i)*2+1;
    }
    dest.resize(nbImagePoints,0.,false,false);
    fvector::iterator it=dest.begin();

    const int r = chan.size().x;
    const int c = chan.size().y;

    dest.resize(nbImagePoints,0.,false,false);
    if (useOrientation) {
      float stmp, ctmp;
      sincos(angle, stmp, ctmp);
      const float sina = stmp;
      const float cosa = ctmp;
      float tx,fi;

      if ((x-frad>=0) && (iround(x)+rad<c) && 
          (y-frad>=0) && (iround(y)+rad<r)) {
        for (i=-rad; i<=rad; ++i) {
          fi=static_cast<float>(i);
          k=static_cast<float>(circPoints.at(i));
          for (j=-k; j<=k; j+=1.) {
            tx=x+cosa*j-sina*fi;
            ty=y+sina*j+cosa*fi;
            (*it)=interpol.interpolateUnchk(chan,ty,tx);
//             std::cerr << "(" << iround(ty) << ", " << iround(tx) << ") ";
            ++it;
          } // for (j=...)
        }  // for (i=...)
      } // if
      else {
        for (i=-rad; i<=rad; ++i) {
          fi=static_cast<float>(i);
          k=static_cast<float>(circPoints.at(i));
          for (j=-k; j<=k; j+=1.) {
            tx=x+cosa*j-sina*fi;
            ty=y+sina*j+cosa*fi;
            (*it)=interpol.interpolate(chan,ty,tx);
            ++it;
          }
        }
      }
    } // useOrientation
    else {
      if ((x-frad>=0) && (iround(x)+rad<c) && 
          (y-frad>=0) && (iround(y)+rad<r)) {
        
        _lti_debug4("useOrientation=false  unchecked\n");
        
        for (i=-rad; i<=rad; ++i) {
          k=static_cast<float>(circPoints.at(i));
          ty=static_cast<float>(i)+y;
          for (j=x-k, k+=x; j<=k; j+=1.) {
            (*it)=interpol.interpolateUnchk(chan,ty,j);
            ++it;
          }
        }
        
      } else {

        _lti_debug4("useOrientation=false  checked\n");

        for (i=-rad; i<=rad; ++i) {
          k=static_cast<float>(circPoints.at(i));
          ty=static_cast<float>(i)+y;
          for (j=x-k, k+=x; j<=k; j+=1.) {
            (*it)=interpol.interpolate(chan,ty,j);
            ++it;
          }
          _lti_debug4("\n");
        }                
      }
    } // else (useOrientation==false)
    return true;    
  } // fixedGridSampling

  bool localFeatureExtractor::fixedGridSampling(const channel& chan, 
                                                const std::list<location>& loc,
                                                std::list<fvector>& dest,
                                                const array<int>& circPoints,
                                                float scale,
                                                bool useOrientation) const {
    dest.clear();
    std::list<location>::const_iterator it;
    bool rc = true;
    for (it = loc.begin();((it != loc.end())&&rc); it++) {
      fvector tmp;
      rc = fixedGridSampling(chan,*it,tmp,circPoints,scale,useOrientation);
      dest.push_back(tmp);
    }
    return rc;
  }

  bool 
  localFeatureExtractor::initCircle(int rad, array<int>& circPoints) const {

    int i,ip;
    float rad2=pow(static_cast<float>(rad),2);
    float p,tmp,fi;
    
    circPoints.resize(-rad,rad,0,false,true);

    _lti_debug4("circPoints before calc:\n" << circPoints << "\n");
    
    for (i=0; i<=rad; i++) {
      tmp=static_cast<float>(i);
      p=round(sqrt(rad2-tmp*tmp));
      if (p>circPoints.at(i)) {
        circPoints.at(i)=static_cast<int>(p);
        circPoints.at(-i)=static_cast<int>(p);
      }
      ip=static_cast<int>(p);
      fi=static_cast<float>(i);
      if ((ip<=rad) && (fi>circPoints.at(ip))) {
        circPoints.at(ip)=static_cast<int>(fi);
        circPoints.at(-ip)=static_cast<int>(fi);
      }
    }
#ifdef _LTI_DEBUG
#if _LTI_DEBUG==4
    for (i=rad; i>=-rad; i--) {
      int j;
      for (j=-rad; j<=rad; j++) {
        if (j<-circPoints.at(i)) {
          std::cerr << " ";
        } else if (j<=circPoints.at(i)) {
          std::cerr << "O";
        } else {
          std::cerr << " ";
        }
      }
      std::cerr << std::endl;
    }
    std::cerr << std::endl;
#endif
#endif

    return true;
  }

}

#include "ltiUndebug.h"
