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
 * file .......: ltiOgdFilter.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 25.5.2000
 * revisions ..: $Id: ltiOgdFilter.cpp,v 1.13 2006/09/05 10:23:40 ltilib Exp $
 */

#include "ltiOgdFilter.h"

namespace lti {
  // --------------------------------------------------
  // ogdFilter::parameters
  // --------------------------------------------------

  const double ogdFilter::useAngleOfParameters = -3e99;

  // default caonstructor
  ogdFilter::parameters::parameters()
    : filter::parameters() {

    order = 1;
    variance = 1.4;
    size = 5;
    angle = 0.0;
  }

  // copy constructor
  ogdFilter::parameters::parameters(const parameters& other)
    : filter::parameters()  {
    copy(other);
  }

  // destructor
  ogdFilter::parameters::~parameters() {
  }

  // get type name
  const char* ogdFilter::parameters::getTypeName() const {
    return "ogdFilter::parameters";
  }

  // copy member

  ogdFilter::parameters&
  ogdFilter::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    filter::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    filter::parameters& (filter::parameters::* p_copy)
      (const filter::parameters&) =
      filter::parameters::copy;
    (this->*p_copy)(other);
# endif

    order = other.order;
    variance = other.variance;
    size = other.size;
    angle = other.angle;

    return *this;
  }

  ogdFilter::parameters& 
  ogdFilter::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* ogdFilter::parameters::clone() const {
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
  bool ogdFilter::parameters::write(ioHandler& handler,
                                    const bool complete) const
# else
  bool ogdFilter::parameters::writeMS(ioHandler& handler,
                                      const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"order",order);
      lti::write(handler,"size",size);
      lti::write(handler,"variance",variance);
      lti::write(handler,"angle",angle);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && filter::parameters::write(handler,false);
# else
    bool (modifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      modifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool ogdFilter::parameters::write(ioHandler& handler,
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
  bool ogdFilter::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool ogdFilter::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"order",order);
      lti::read(handler,"size",size);
      lti::read(handler,"variance",variance);
      lti::read(handler,"angle",angle);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && filter::parameters::read(handler,false);
# else
    bool (modifier::parameters::* p_readMS)(ioHandler&,const bool) =
      modifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool ogdFilter::parameters::read(ioHandler& handler,
                                    const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // ogdFilter
  // --------------------------------------------------

  // default constructor
  ogdFilter::ogdFilter()
    : filter() {
    parameters par;
    setParameters(par);
  }

  // default constructor
  ogdFilter::ogdFilter(const parameters& par)
    : filter(){
    setParameters(par);
  }

  // copy constructor
  ogdFilter::ogdFilter(const ogdFilter& other)
    : filter()  {
    copy(other);
  }

  // destructor
  ogdFilter::~ogdFilter() {
  }

  // returns the name of this type
  const char* ogdFilter::getTypeName() const {
    return "ogdFilter";
  }

  // copy member
  ogdFilter& ogdFilter::copy(const ogdFilter& other) {
      filter::copy(other);
    return (*this);
  }

  // clone member
  functor* ogdFilter::clone() const {
    return new ogdFilter(*this);
  }

  // return parameters
  const ogdFilter::parameters&
    ogdFilter::getParameters() const {
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


  // On place apply for type channel!
  bool ogdFilter::apply(channel& srcdest) const {
    channel tmp;

    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }
    return false;
  };

  // On copy apply for type channel!
  bool ogdFilter::apply(const channel& src,channel& dest) const {
    const parameters& par = getParameters();
    convolution convolver;

    switch(par.order) {
      case 1: {
        ogd1Kernel<channel::value_type> k(par.size,par.variance,par.angle);
        convolution::parameters convParam;
        convParam.setKernel(k);
        convolver.setParameters(convParam);
      };
      break;

      case 2: {
        ogd2Kernel<channel::value_type> k(par.size,par.variance,par.angle);
        convolution::parameters convParam;
        convParam.setKernel(k);
        convolver.setParameters(convParam);
      };
      break;

      default:
        throw invalidParametersException(getTypeName());
    }

    convolver.apply(src,dest);

    return true;
  };

  bool ogdFilter::useBasisOgd1(const channel& basis0,
                               const channel& basis1,
                                     channel& dest,
                               const double& angle) {

    if ((basis0.rows() != basis1.rows()) ||
        (basis0.columns() != basis1.columns())) {
      throw lti::exception("Basis channels have not the same size");
    }

    point p;
    double cost,sint,a;

    if (angle == useAngleOfParameters) {
      a = getParameters().angle;
    } else {
      a = angle;
    }

    sincos(a, sint, cost);

    dest.resize(basis0.rows(),basis0.columns(),0.0f,false,false);

    for (p.y = 0;p.y < basis0.rows(); p.y++) {
      for (p.x = 0;p.x < basis0.columns(); p.x++) {
        dest.at(p) = static_cast<float>(cost*basis0.at(p) + sint*basis1.at(p));
      }
    }

    return true;
  }

  void ogdFilter::generateBasisOgd1(const channel& src,
                                          channel& basis0,
                                          channel& basis1) {


    const parameters& par = getParameters();
    convolution convolver;

    ogd1Kernel<channel::value_type> k;
    convolution::parameters convParam;
    convParam.boundaryType = par.boundaryType;

    k.generateBasisKernel(0,par.size,par.variance);

    convParam.setKernel(k);

    convolver.setParameters(convParam);

    convolver.apply(src,basis0);

    k.generateBasisKernel(1,par.size,par.variance);

    convParam.setKernel(k);
    convolver.setParameters(convParam);

    convolver.apply(src,basis1);

  }

  channel& ogdFilter::useBasisOgd2(const channel& basis0,
                                   const channel& basis1,
                                   const channel& basis2,
                                         channel& dest,
                                   const double& angle) {
    if ((basis0.rows() != basis1.rows()) ||
        (basis1.rows() != basis2.rows()) ||
        (basis0.columns() != basis1.columns()) ||
        (basis1.columns() != basis2.columns())) {
      throw lti::exception("Basis channels have not the same size");
    }

    point p;
    double cossin2,cos2,sin2,a;

    if (angle == useAngleOfParameters) {
      a = getParameters().angle;
    } else {
      a = angle;
    }

    sincos(a, sin2, cos2);
    cossin2 = cos2*sin2*2;
    cos2 *= cos2;
    sin2 *= sin2;

    dest.resize(basis0.rows(),basis0.columns(),0.0f,false,false);

    for (p.y = 0;p.y < basis0.rows(); p.y++) {
      for (p.x = 0;p.x < basis0.columns(); p.x++) {
        dest.at(p) = static_cast<float>(cos2*basis0.at(p) +
                                        sin2*basis1.at(p) +
                                        cossin2*basis2.at(p));
      }
    }

    return dest;
  }


  void ogdFilter::generateBasisOgd2(const channel& src,
                                          channel& basis0,
                                          channel& basis1,
                                          channel& basis2) {

    const parameters& par = getParameters();
    convolution convolver;

    ogd2Kernel<channel::value_type> k;
    convolution::parameters convParam;
    convParam.boundaryType = par.boundaryType;

    k.generateBasisKernel(0,par.size,par.variance);

    convParam.setKernel(k);
    convolver.setParameters(convParam);

    convolver.apply(src,basis0);

    k.generateBasisKernel(1,par.size,par.variance);

    convParam.setKernel(k);
    convolver.setParameters(convParam);

    convolver.apply(src,basis1);

    k.generateBasisKernel(2,par.size,par.variance);

    convParam.setKernel(k);
    convolver.setParameters(convParam);

    convolver.apply(src,basis2);
  }

}
