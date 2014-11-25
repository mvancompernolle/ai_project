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
 * file .......: ltiPolynomialKernel.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 05.11.2001
 * revisions ..: $Id: ltiPolynomialKernel.cpp,v 1.9 2006/09/05 09:59:54 ltilib Exp $
 */

#include "ltiPolynomialKernel.h"
#include "ltiMath.h"

namespace lti {
  // --------------------------------------------------
  // polynomialKernel::parameters
  // --------------------------------------------------

  // default constructor with standard degree of 2

  polynomialKernel::parameters::parameters()
    : kernelFunctor<double>::parameters() {
    d=2;
  }

  // constructor: degree determines the degree d of the polynomial kernel: \f$ K(A,B) = \left( 1 + A \cdot B \right)^d\f$

  polynomialKernel::parameters::parameters(const int degree)
    : kernelFunctor<double>::parameters() {
    d=degree;
  }

  // copy constructor

  polynomialKernel::parameters::parameters(const parameters& other) 
    : kernelFunctor<double>::parameters() {
    copy(other);
  }

  // get type name
   const char*
  polynomialKernel::parameters::getTypeName() const {
    return "polynomialKernel::parameters";
  }

  // copy member


  polynomialKernel::parameters&
  polynomialKernel::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    kernelFunctor<double>::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    kernelFunctor<double>::parameters&
      (kernelFunctor<double>::parameters::* p_copy)
      (const kernelFunctor<double>::parameters&) =
      kernelFunctor<double>::parameters::copy;
    (this->*p_copy)(other);
# endif

    d=other.d;

    return *this;
  }

  // clone member
  functor::parameters*
  polynomialKernel::parameters::clone() const {
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
  bool polynomialKernel::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool polynomialKernel::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      b=b && lti::write(handler, "d", d);
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
  bool polynomialKernel::parameters::write(ioHandler& handler,
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
  bool polynomialKernel::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool polynomialKernel::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      b=b && lti::read(handler, "d", d);
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
  bool polynomialKernel::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif




  // --------------------------------------------------
  // polynomialKernel
  // --------------------------------------------------

  // default constructor with standard degree of 2
  polynomialKernel::polynomialKernel()
    : kernelFunctor<double>() {
    parameters def;
    setParameters(def);
  }

  polynomialKernel::polynomialKernel(const int degree)
    : kernelFunctor<double>() {
    parameters def(degree);
    setParameters(def);
  }

  // copy constructor
  polynomialKernel::polynomialKernel(const polynomialKernel& other) 
    : kernelFunctor<double>() {
    copy(other);
  }

  // returns the name of this type
  const char*
  polynomialKernel::getTypeName() const {
    return "polynomialKernel";
  }

  // clone member
  functor*
  polynomialKernel::clone() const {
    return new polynomialKernel(*this);
  }

  // return parameters
  const polynomialKernel::parameters&
  polynomialKernel::getParameters() const {
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

  double polynomialKernel::apply(const vector<double>& a,
                                 const vector<double>& b) const {
    double t=a.dot(b)+1.0;

    return pow(t,getParameters().d);
  }


}
