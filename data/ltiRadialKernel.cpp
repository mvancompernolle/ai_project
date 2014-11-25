/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiRadialKernel.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 05.11.2001
 * revisions ..: $Id: ltiRadialKernel.cpp,v 1.7 2006/09/05 10:01:30 ltilib Exp $
 */

#include "ltiRadialKernel.h"
#include "ltiL1Distance.h"
#include "ltiL2Distance.h"

namespace lti {
  // --------------------------------------------------
  // kernelFunctor::parameters
  // --------------------------------------------------

  // default constructor

  radialKernel::parameters::parameters()
    : kernelFunctor<double>::parameters() {

    sigmasq=1.0;
  }

  radialKernel::parameters::parameters(const double& sigma)
    : kernelFunctor<double>::parameters() {

    sigmasq=sigma;
  }

  // copy constructor

  radialKernel::parameters::parameters(const parameters& other)
    : kernelFunctor<double>::parameters() {
    copy(other);
  }

  // get type name
   const char*
  radialKernel::parameters::getTypeName() const {
    return "radialKernel::parameters";
  }

  // copy member


  radialKernel::parameters&
  radialKernel::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters&
      (functor::parameters::* p_copy)
      (const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
# endif

    sigmasq=other.sigmasq;
    return *this;
  }

  // clone member
   functor::parameters*
  radialKernel::parameters::clone() const {
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
  bool radialKernel::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool radialKernel::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      b=b && lti::write(handler, "sigmasq", sigmasq);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && kernelFunctor<double>::parameters::write(handler,false);
# else
    bool (kernelFunctor<double>::parameters::* p_writeMS)(ioHandler&,const bool) const =
      kernelFunctor<double>::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool radialKernel::parameters::write(ioHandler& handler,
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
  bool radialKernel::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool radialKernel::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      b=b && lti::read(handler, "sigmasq", sigmasq);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && kernelFunctor<double>::parameters::read(handler,false);
# else
    bool (kernelFunctor<double>::parameters::* p_readMS)(ioHandler&,const bool) =
      kernelFunctor<double>::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool radialKernel::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif



  // --------------------------------------------------
  // radialKernel
  // --------------------------------------------------

  // default constructor

  radialKernel::radialKernel()
    : kernelFunctor<double>(){
    parameters def;
    setParameters(def);
  }

  // default constructor

  radialKernel::radialKernel(const double& sigma)
    : kernelFunctor<double>() {
    parameters def(sigma);
    setParameters(def);
  }

  // copy constructor

  radialKernel::radialKernel(const radialKernel& other) 
    : kernelFunctor<double>() {
    copy(other);
  }

  // returns the name of this type
   const char*
  radialKernel::getTypeName() const {
    return "radialKernel";
  }

  // clone member
   functor*
  radialKernel::clone() const {
    return new radialKernel(*this);
  }

  // return parameters
  const radialKernel::parameters&
  radialKernel::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&kernelFunctor<double>::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  double radialKernel::apply(const vector<double>& a,
                             const vector<double>& b) const {
    double t=l2.apply(a,b);

    return exp(-(t*t)/getParameters().sigmasq);
  }


}
