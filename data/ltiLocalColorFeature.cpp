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


/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiLocalColorFeature.cpp
 * authors ....: Axel Berner, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 17.1.2002
 * revisions ..: $Id: ltiLocalColorFeature.cpp,v 1.18 2006/09/05 10:20:38 ltilib Exp $
 */

#include "ltiLocalColorFeature.h"
#include "ltiArctanLUT.h"

//#define _LTI_DEBUG

namespace lti {
  // --------------------------------------------------
  // localColorFeature::parameters
  // --------------------------------------------------

  // default constructor
  localColorFeature::parameters::parameters()
    : localFeatureExtractor::parameters() {
    slices = 6;
    overlap = true;
    maskRadius = 7;
    normalize = false;
  }

  // copy constructor
  localColorFeature::parameters::parameters(const parameters& other)
    : localFeatureExtractor::parameters()  {
    copy(other);
  }

  // destructor
  localColorFeature::parameters::~parameters() {
  }

  // get type name
  const char* localColorFeature::parameters::getTypeName() const {
    return "localColorFeature::parameters";
  }

  // copy member

  localColorFeature::parameters&
    localColorFeature::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    localFeatureExtractor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    localFeatureExtractor::parameters& (localFeatureExtractor::parameters::* p_copy)
      (const localFeatureExtractor::parameters&) =
      localFeatureExtractor::parameters::copy;
    (this->*p_copy)(other);
# endif

    slices = other.slices;
    overlap = other.overlap;
    maskRadius = other.maskRadius;
    normalize = other.normalize;

    return *this;
  }

  // alias for copy member
  localColorFeature::parameters&
    localColorFeature::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* localColorFeature::parameters::clone() const {
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
  bool localColorFeature::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool localColorFeature::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"slices",slices);
      lti::write(handler,"overlap",overlap);
      lti::write(handler,"maskRadius",maskRadius);
      lti::write(handler,"normalize",normalize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && localFeatureExtractor::parameters::write(handler,false);
# else
    bool (localFeatureExtractor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      localFeatureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool localColorFeature::parameters::write(ioHandler& handler,
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
  bool localColorFeature::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool localColorFeature::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"slices",slices);
      lti::read(handler,"overlap",overlap);
      lti::read(handler,"maskRadius",maskRadius);
      lti::read(handler,"normalize",normalize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && localFeatureExtractor::parameters::read(handler,false);
# else
    bool (localFeatureExtractor::parameters::* p_readMS)(ioHandler&,const bool) =
      localFeatureExtractor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool localColorFeature::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // localColorFeature
  // --------------------------------------------------

  // default constructor
  localColorFeature::localColorFeature()
    : localFeatureExtractor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // default constructor
  localColorFeature::localColorFeature(const parameters& par)
    : localFeatureExtractor(){

    // set the default parameters
    setParameters(par);

  }

  // copy constructor
  localColorFeature::localColorFeature(const localColorFeature& other)
    : localFeatureExtractor()  {
    copy(other);
  }

  // destructor
  localColorFeature::~localColorFeature() {
  }

  // returns the name of this type
  const char* localColorFeature::getTypeName() const {
    return "localColorFeature";
  }

  // copy member
  localColorFeature&
  localColorFeature::copy(const localColorFeature& other) {
    localFeatureExtractor::copy(other);

    normalMask.copy(other.normalMask);
    overlapMask.copy(other.overlapMask);

    return (*this);
  }

  // alias for copy member
  localColorFeature&
  localColorFeature::operator=(const localColorFeature& other) {
    return (copy(other));
  }


  // clone member
  functor* localColorFeature::clone() const {
    return new localColorFeature(*this);
  }

  // return parameters
  const localColorFeature::parameters&
  localColorFeature::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool localColorFeature::updateParameters() {
    bool ok=true;
    const parameters& par = getParameters();
    if ((par.slices < 1) || (par.slices > 120)) {
      setStatusString("Wrong number of slices (not in [1,120])");
      ok=false;
    }
    if ((par.maskRadius < 3) || (par.maskRadius > 255)) {
      setStatusString("Wrong radius for mask (not in [3,255])");
      ok=false;
    }
    normalMask.generate(par.maskRadius,0.0,par.slices);
    if (par.overlap) {
      overlapMask.generate(par.maskRadius,Pi/par.slices,par.slices);
    }
    return ok;
  }

  // -------------------------------------------------------------------
  // slice mask holder class
  // -------------------------------------------------------------------
  localColorFeature::sliceMask::sliceMask()
    : bilin(),mask(),ioPts(),slices(0),radius(0) {
  }

  localColorFeature::sliceMask::sliceMask(const int radius,
                                          const double angle,
                                          const int slices) {
    generate(radius,angle,slices);
  }

  localColorFeature::sliceMask::sliceMask(const sliceMask& other)
    : bilin(),mask(),ioPts(),slices(0),radius(0) {
    copy(other);
  }

  localColorFeature::sliceMask&
  localColorFeature::sliceMask::copy(const sliceMask& other) {
    bilin.copy(other.bilin);
    mask.copy(other.mask);
    ioPts.copy(other.ioPts);
    slices = other.slices;
    radius = other.radius;
    return *this;
  }

  localColorFeature::sliceMask&
  localColorFeature::sliceMask::operator=(const sliceMask& other) {
    return copy(other);
  }

  void localColorFeature::sliceMask::generate(const int radius,
                                              const double angle,
                                              const int slices) {

    this->slices = slices;
    this->radius = radius;

    if (radius < 1) {
      return;
    }

    if (slices < 1) {
      return;
    }

    // resize mask and ioPts
    mask.resize(-radius,-radius,radius,radius,0,false,true);
    ioPts.resize(-radius,radius,0,false,true);

    // create the io points and the mask
    int i,j,idx;
    float y2;
    float pa;

    // ensure an angle between 0 and 2*Pi
    static const float pi2 = static_cast<float>(2.0*Pi);
    float a = static_cast<float>(angle);
    while (a > pi2) {
      a -= pi2;
    }
    while (a < 0) {
      a += pi2;
    }

    const int r2 = radius*radius;

    arctanLUT atan2;

    // go through the y-axis
    for (i=mask.firstRow();i<=mask.lastRow();++i) {
      // the in and out points correspond to the borders of
      // a circle:
      y2 = static_cast<float>(i*(i+1)) + 0.25f; // == (i+0.5)^2
      const int o = (ioPts.at(i) = 
                     ((i <= 0) ? iround(sqrt(r2-y2)) : ioPts.at(-i)));

      // for all points between the in and out points
      for (j=-o;j<=o;++j) {
        // compute the angle of this point
        pa = atan2(i,j)-a;

        // fix the angle to be between 0 and 2*Pi
        while (pa < 0) {
          pa += pi2;
        }
        while (pa > pi2) {
          pa -= pi2;
        }

        // compute the index of the splice
        idx = iround(pa*slices/pi2) % slices;

        // and indicate the computation in the mask.
        mask.at(i,j)=idx;
      }
    }
  }

  bool localColorFeature::sliceMask::getMeans(const image& img,
                                              const location& loc,
                                              vector< trgbPixel<float> >& means
                                              ) const {

    means.resize(slices,trgbPixel<float>(0.0f,0.0f,0.0f),false,true);
    ivector n(slices,0);

    // we need a transformation from the location to the image space.
    
    float ctmp, stmp;
    sincos(loc.angle, stmp, ctmp);
    const float fac = loc.radius/radius;
    
    const float ssina = fac * stmp;
    const float scosa = fac * ctmp;

    const float maxx = static_cast<float>(img.lastColumn()) - 1.0f;
    const float maxy = static_cast<float>(img.lastRow()) - 1.0f;
    

    int y,x,i;
    float fy,fx;

    // interpolation of the boundary necessary
    // this is a robust but slow implementation
    for (y=-radius;y<=radius;++y) {
      const int o = ioPts.at(y);
      for (x=-o;x<=o;++x) {
        fx = x*scosa - y*ssina + loc.position.x;
        fy = x*ssina + y*scosa + loc.position.y;
        if ((fy >= 0) && (fy < maxy) &&
            (fx >= 0) && (fx < maxx)) {
          i = mask.at(y,x);
          means.at(i) += bilin.interpolateUnchk(img,fy,fx);
          n.at(i)++;
        }      
      }
    }

    // compute the mean values
    for (i=0;i<n.size();++i) {
      if (n.at(i) > 0) {
        means.at(i).divide(static_cast<float>(n.at(i)));
      }
    }

    return true;
  }

  const kernel2D<ubyte>& localColorFeature::sliceMask::getMask() const {
    return mask;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On copy apply for type std::list<location>!
  bool localColorFeature::apply(const lti::image& src,
                                const std::list<location>& locs,
                                std::list<dvector>& dest) {

    dest.resize(locs.size(),dvector());
    std::list<location>::const_iterator lit;
    std::list<dvector>::iterator it;
    for (lit=locs.begin(),it=dest.begin();lit!=locs.end();++lit,++it) {
      apply(src,(*lit),(*it));
    }
    return true;

  }

  // On copy apply for type std::list<location>!
  bool localColorFeature::apply(const lti::image& src,
                                const location& loc,
                                      dvector& dest) {

    const parameters& param = getParameters();
    int i;
    double acc(0.0);
    vector< trgbPixel<float> > means;

    if (param.overlap) {
      vector< trgbPixel<float> > omeans;
      normalMask.getMeans(src,loc,means);
      overlapMask.getMeans(src,loc,omeans);
      dest.resize(6*means.size(),0.0,false,false);
      const int g = 2*means.size();
      const int b = 2*g;
      int j;
      for (i=0;i<means.size();++i) {
        j = 2*i;
        acc += (dest.at(j)     =  means.at(i).red);
        acc += (dest.at(j+1)   = omeans.at(i).red);
        
        acc += (dest.at(j+g)   =  means.at(i).green);
        acc += (dest.at(j+g+1) = omeans.at(i).green);

        acc += (dest.at(j+b)   =  means.at(i).blue);
        acc += (dest.at(j+b+1) = omeans.at(i).blue);
      }      
    } else {
      // not overlaping slices
      normalMask.getMeans(src,loc,means);
      dest.resize(3*means.size(),0.0,false,false);
      const int g = means.size();
      const int b = 2*g;
      for (i=0;i<means.size();++i) {
        acc += (dest.at(i)   = means.at(i).red);
        acc += (dest.at(i+g) = means.at(i).green);
        acc += (dest.at(i+b) = means.at(i).blue);
      }
    }
    
    if (param.normalize) {
      dest.divide(acc);
    }

    return true;
  }

  const kernel2D<ubyte>& localColorFeature::getMask() const {
    return normalMask.getMask();
  }

}
