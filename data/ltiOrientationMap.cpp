/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiOrientationMap.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 26.5.2000
 * revisions ..: $Id: ltiOrientationMap.cpp,v 1.9 2006/09/05 10:24:25 ltilib Exp $
 */


#include "ltiOrientationMap.h"
#include "ltiConvolution.h"
#include "ltiGaussKernels.h"
#include "ltiGradientKernels.h"
#include "ltiOgdFilter.h"

namespace lti {
  // --------------------------------------------------
  // orientationMap::parameters
  // --------------------------------------------------

  // default constructor
  orientationMap::parameters::parameters()
    : transform::parameters() {

    mode = Gradient;

    size = 5;
    variance = 1.0;

    localFilterSize     = 7;
    localFilterVariance = 2;
  }

  // copy constructor
  orientationMap::parameters::parameters(const parameters& other)
    : transform::parameters()  {
    copy(other);
  }

  // destructor
  orientationMap::parameters::~parameters() {
  }

  // get type name
  const char* orientationMap::parameters::getTypeName() const {
    return "orientationMap::parameters";
  }

  // copy member
  orientationMap::parameters&
    orientationMap::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
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

    mode = other.mode;
    size = other.size;
    variance = other.variance;
    localFilterVariance = other.localFilterVariance;
    localFilterSize = other.localFilterSize;

    return *this;
  }

  orientationMap::parameters&
  orientationMap::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* orientationMap::parameters::clone() const {
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
  bool orientationMap::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool orientationMap::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      switch(mode) {
          case Ogd:
            lti::write(handler,"mode","Ogd");
            break;
          default:
            lti::write(handler,"mode","Gradient");
      }

      lti::write(handler,"size",size);
      lti::write(handler,"variance",variance);
      lti::write(handler,"localFilterSize",localFilterSize);
      lti::write(handler,"localFilterVariance",localFilterVariance);
    }

# ifndef _LTI_MSC_6
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

# ifdef _LTI_MSC_6
  bool orientationMap::parameters::write(ioHandler& handler,
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
  bool orientationMap::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool orientationMap::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string str;
      lti::read(handler,"mode",str);
      if (str == "Ogd") {
        mode = Ogd;
      } else {
        mode = Gradient;
      }

      lti::read(handler,"size",size);
      lti::read(handler,"variance",variance);
      lti::read(handler,"localFilterSize",localFilterSize);
      lti::read(handler,"localFilterVariance",localFilterVariance);
    }

# ifndef _LTI_MSC_6
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

# ifdef _LTI_MSC_6
  bool orientationMap::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // orientationMap
  // --------------------------------------------------

  // default constructor
  orientationMap::orientationMap()
    : transform(){
    parameters p;
    setParameters(p);
  }

  // copy constructor
  orientationMap::orientationMap(const orientationMap& other)
    : transform()  {
    copy(other);
  }

  // destructor
  orientationMap::~orientationMap() {
  }

  // returns the name of this type
  const char* orientationMap::getTypeName() const {
    return "orientationMap";
  }

  // copy member
  orientationMap& orientationMap::copy(const orientationMap& other) {
    transform::copy(other);
    return (*this);
  }

  // clone member
  functor* orientationMap::clone() const {
    return new orientationMap(*this);
  }

  // return parameters
  const orientationMap::parameters&
    orientationMap::getParameters() const {
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

  // orientation maps for channels
  channel& orientationMap::ogdMap(const channel& src,
                                        channel& direction,
                                        channel& relevance) const {

    const parameters& par = getParameters();

    ogdFilter filter;
    convolution gauss;

    ogdFilter::parameters fparam;
    convolution::parameters gparam;

    float e0,e1;               // elements of the basis channels
    float e00,e01,e11;
    float A0,A1;

    point p;
    channel b0,b1;             // basis channels
    channel p00,p01,p11;       // power channels
    channel en00,en01,en11;    // energy channels

    // set the ogd filter parameters
    fparam.order = 1;
    fparam.size = par.size;
    fparam.variance = par.variance;

    filter.setParameters(fparam);

    // generate basis channels

    filter.generateBasisOgd1(src,b0,b1);

    p00.resize(b0.rows(),b0.columns(),0,false,false);
    p01.resize(b0.rows(),b0.columns(),0,false,false);
    p11.resize(b0.rows(),b0.columns(),0,false,false);

    for (p.y=0;p.y<b0.rows();p.y++) {
      for (p.x=0;p.x<b0.columns();p.x++) {
        e0 = b0.at(p);
        e1 = b1.at(p);

        p00.at(p) = e0*e0;
        p01.at(p) = e0*e1;
        p11.at(p) = e1*e1;
      }
    }

    // generate local energy

    // set the gauss filter parameters

    // generate a filter kernel
    gaussKernel2D<channel::value_type> gkernel(par.localFilterSize,
                                               par.localFilterVariance);
    gparam.setKernel(gkernel);  // which filter kernel should be used
    gauss.setParameters(gparam);// set the parameters

    gauss.apply(p00,en00);
    gauss.apply(p01,en01);
    gauss.apply(p11,en11);

    direction.resize(b0.rows(),b0.columns(),0,false,false);
    relevance.resize(b0.rows(),b0.columns(),0,false,false);

    static const float pi2=static_cast<float>(2.0*Pi);

    for (p.y=0;p.y<b0.rows();p.y++) {
      for (p.x=0;p.x<b0.columns();p.x++) {
        e00 = en00.at(p);
        e01 = en01.at(p);
        e11 = en11.at(p);

        A0 = (e00+e11)/2.0f;
        if (A0 > 0.0f) {
          A1 = 0.5f*sqrt(e00*e00+e11*e11+4*e01*e01-2*e00*e11);

          float ang = (atan2(2*e01,(e00-e11)))/2.0f;
          if (ang < 0) {
            ang += pi2;
          }

          if (ang > static_cast<float>(Pi)) {
            ang -= static_cast<float>(Pi);
          }

          direction.at(p) = ang;

          relevance.at(p) = A1;
        } else {
          direction.at(p) = 0;
          relevance.at(p) = 0;
        }
      }
    }

    return direction;
  };

  bool orientationMap::apply(const channel8& src,
                                        channel8& direction,
                                        channel8& relevance) const {

    channel s,d,r;
    s.castFrom(src);

    apply(s,d,r);

    direction.castFrom(d);
    relevance.castFrom(r);

    return true;
  };

  // orientation maps for channels
  channel& orientationMap::gradientMap(const channel& src,
                                             channel& direction,
                                             channel& relevance) const {
    const parameters& par = getParameters();

    gradientKernelX<channel::value_type> gx(par.size);
    gradientKernelY<channel::value_type> gy(par.size);

    convolution grad;
    convolution::parameters gradpar;

    gradpar.boundaryType = lti::Constant;
    gradpar.setKernel(gx);
    grad.setParameters(gradpar);

    grad.apply(src,direction);

    grad.setKernel(gy);
    grad.apply(src,relevance);
    vector<channel::value_type>::iterator itd,itr,ite;
    int y;
    channel::value_type mag,arg;
    for (y=0;y<src.rows();y++) {
      for (itd=direction.getRow(y).begin(),
             itr=relevance.getRow(y).begin(),
             ite=direction.getRow(y).end();
           itd!=ite;
           itd++,itr++) {
        mag = static_cast<float>(hypot(*itd,*itr));
        arg = static_cast<float>(atan2(*itr,*itd));
        *itr = mag;
        *itd = arg;
      }
    }

    return direction;
  }

  // orientation maps for channels
  bool orientationMap::apply(const channel& src,
                                   channel& direction,
                                   channel& relevance) const {
    const parameters& par = getParameters();

    if (par.mode == parameters::Gradient) {
      gradientMap(src,direction,relevance);
    } else {
      ogdMap(src,direction,relevance);
    }

    return true;
  }
}
