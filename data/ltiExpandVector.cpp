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
 * file .......: ltiExpandVector.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 22.3.2001
 * revisions ..: $Id: ltiExpandVector.cpp,v 1.10 2006/09/05 10:10:46 ltilib Exp $
 */

#include "ltiExpandVector.h"
#include "ltiMath.h"

namespace lti {
  double expandVector::intPow2m(double x) {
    if (x>=0.0) {
      return (double(1 << int(x)) - 1.0);
    } else {
      return ((1.0/(1 << int(-x))) - 1.0) ;
    }
  }

  // --------------------------------------------------
  // expandVector::parameters
  // --------------------------------------------------

  // default constructor
  expandVector::parameters::parameters()
    : functor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    function = expandVector::intPow2m;
  }

  // copy constructor
  expandVector::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  expandVector::parameters::~parameters() {
  }

  // get type name
  const char* expandVector::parameters::getTypeName() const {
    return "expandVector::parameters";
  }

  // copy member

  expandVector::parameters&
    expandVector::parameters::copy(const parameters& other) {
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

    function = other.function;

    return *this;
  }

  // alias for copy member
  expandVector::parameters&
    expandVector::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* expandVector::parameters::clone() const {
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
  bool expandVector::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool expandVector::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    // TODO: to store the pointer, a static global map with function names
    // should be done, where all posible functions could be inscribed...
    if (b) {
      if (function == expandVector::intPow2m) {
        lti::write(handler,"function","intPow2m");
      } else {
        lti::write(handler,"function","unknown");
      }
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
  bool expandVector::parameters::write(ioHandler& handler,
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
  bool expandVector::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool expandVector::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string tmp;
      lti::read(handler,"function",tmp);
      function = expandVector::intPow2m;
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
  bool expandVector::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // expandVector
  // --------------------------------------------------

  // default constructor
  expandVector::expandVector()
    : functor() {
    parameters defaultParam;
    setParameters(defaultParam);
  }

  /**
   * constructor, with the wished default mapping function
   */
  expandVector::expandVector(double (*function)(double)) {
    parameters param;
    param.function = function;
    setParameters(param);
  }


  // copy constructor
  expandVector::expandVector(const expandVector& other)
    : functor()  {
    copy(other);
  }

  // destructor
  expandVector::~expandVector() {
  }

  // returns the name of this type
  const char* expandVector::getTypeName() const {
    return "expandVector";
  }

  // copy member
  expandVector&
    expandVector::copy(const expandVector& other) {
      functor::copy(other);
    return (*this);
  }

  // clone member
  functor* expandVector::clone() const {
    return new expandVector(*this);
  }

  // return parameters
  const expandVector::parameters&
    expandVector::getParameters() const {
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

  /**
   * short cut for apply and parameters.  This function "draw" the
   * input vector on the output vector, using the given function,
   * i.e.  out(g(x)) = in(x).  Between the samples g(x) and g(x+1)
   * linear interpolation will be used.
   * @param in input vector
   * @param out output vector
   * @param g mapping function
   */
  vector<double>& expandVector::draw(const vector<double>& in,
                                     vector<double>& out,
                                     double (*g)(double)) const {

    if (in.size()<1) {
      out.clear();
      return out;
    }

    out.resize(int(1.5+max((*g)(0),(*g)(in.lastIdx()))),0,false,false);

    int i,j;
    int firstj,lastj;
    double acc,step;

    for (i=0;i<in.lastIdx();++i) {
      acc = in.at(i);
      firstj = int((*g)(i)+0.5);
      lastj = int((*g)(i+1)+0.5);
      if (lastj>firstj) {
        step = (in.at(i+1)-acc)/(lastj-firstj);

        for (j=firstj;j<lastj;++j) {
          out.at(j)=acc;
          acc+=step;
        }

      } else {
        out.at(firstj) = acc;
      }
    }

    out.at(out.lastIdx()) = in.at(in.lastIdx());

    return out;
  }

  /**
   * short cut for apply and parameters.  This function "draw" the
   * input vector on the output vector, using the given function,
   * i.e.  out(g(x)) = in(x).  Between the samples g(x) and g(x+1)
   * linear interpolation will be used.
   * @param in input vector
   * @param out output vector
   * @param g mapping function
   */
  vector<float>& expandVector::draw(const vector<float>& in,
                                    vector<float>& out,
                                    double (*g)(double)) const {

    if (in.size()<1) {
      out.clear();
      return out;
    }

    out.resize(int(1.5+max((*g)(0),(*g)(in.lastIdx()))),0,false,false);

    int i,j;
    int firstj,lastj;
    float acc,step;

    for (i=0;i<in.lastIdx();++i) {
      acc = in.at(i);
      firstj = int((*g)(i)+0.5);
      lastj = int((*g)(i+1)+0.5);
      if (lastj>firstj) {
        step = (in.at(i+1)-acc)/(lastj-firstj);

        for (j=firstj;j<lastj;++j) {
          out.at(j)=acc;
          acc+=step;
        }

      } else {
        out.at(firstj) = acc;
      }
    }

    out.at(out.lastIdx()) = in.at(in.lastIdx());

    return out;

  }

  // On place apply for type vector<float>!
  vector<float>& expandVector::apply(vector<float>& srcdest) const {

    const parameters& param = getParameters();
    vector<float> temp;

    draw(srcdest,temp,param.function);
    temp.detach(srcdest);

    return srcdest;
  };

  // On place apply for type vector<double>!
  vector<double>& expandVector::apply(vector<double>& srcdest) const {

    const parameters& param = getParameters();
    vector<double> temp;

    draw(srcdest,temp,param.function);
    temp.detach(srcdest);

    return srcdest;
  };

  // On copy apply for type vector<float>!
  vector<float>& expandVector::apply(const vector<float>& src,
                                           vector<float>& dest) const {

    const parameters& param = getParameters();

    draw(src,dest,param.function);

    return dest;
  };

  // On copy apply for type vector<double>!
  vector<double>& expandVector::apply(const vector<double>& src,
                                      vector<double>& dest) const {
    const parameters& param = getParameters();

    draw(src,dest,param.function);

    return dest;
  };

}
