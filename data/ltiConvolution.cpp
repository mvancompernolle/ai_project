/*
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiConvolution_template.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 17.11.99
 * revisions ..: $Id: ltiConvolution.cpp,v 1.12 2006/09/05 10:07:38 ltilib Exp $
 */

#include "ltiConvolution.h"
#include "ltiMinimizeBasis.h"
#include "ltiConvolutionHelper.h"

#include <list>

namespace lti {

  // --------------------------------------------------
  // convolution::parameters
  // --------------------------------------------------

  // default constructor

  convolution::parameters::parameters()
    : filter::parameters(), kernel(0) {
  }

  // copy constructor

  convolution::parameters::parameters(const parameters& other)
    : filter::parameters()  {
    kernel = 0;
    copy(other);
  }

  // destructor

  convolution::parameters::~parameters() {
    delete kernel;
  }

  // get type name
  const char* convolution::parameters::getTypeName() const {
    return "convolution::parameters";
  }

  // copy member

  convolution::parameters&
    convolution::parameters::copy(const parameters& other) {
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

    delete kernel;
    kernel = 0;

    if (notNull(other.kernel)) {
      kernel = other.kernel->clone();
    }

    return *this;
  }

  // clone member

  functor::parameters* convolution::parameters::clone() const {
    return new parameters(*this);
  }

  const mathObject& convolution::parameters::getKernel() const {
    if (isNull(kernel)) {
      throw invalidParametersException(getTypeName());
    }
    return *kernel;
  }

  void convolution::parameters::setKernel(const mathObject& aKernel) {
    delete kernel;
    kernel = 0;
    kernel = aKernel.clone();
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool convolution::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool convolution::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      if (isNull(kernel)) {
        lti::write(handler,"kernelType","none");
      } else {
        lti::write(handler,"kernelType",kernel->getTypeName());
        lti::write(handler,"kernel",getKernel());
      }
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
  bool convolution::parameters::write(ioHandler& handler,
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
  bool convolution::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool convolution::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string str;
      lti::read(handler,"kernelType",str);
      if (str == "kernel1D") {
        // assume float (maybe to be changed in the near future ;-)
        kernel1D<float> kern;
        lti::read(handler,"kernel",kern);
        setKernel(kern);
      } else if (str == "kernel2D") {
        // assume float (maybe to be changed in the near future ;-)
        kernel2D<float> kern;
        lti::read(handler,"kernel",kern);
        setKernel(kern);
      } else if (str == "sepKernel") {
        // assume float (maybe to be changed in the near future ;-)
        sepKernel<float> kern;
        lti::read(handler,"kernel",kern);
        setKernel(kern);
      } else {
        delete kernel;
        kernel = 0;
      }
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
  bool convolution::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // convolution
  // --------------------------------------------------

  // default constructor
  convolution::convolution()
    : filter() {
    parameters tmpParam;
    setParameters(tmpParam);
  }

  convolution::convolution(const parameters& par)
    : filter() {
    setParameters(par);
  }

  // constructor to set filter kernel directly
  convolution::convolution(const mathObject& aKernel,
                           const eBoundaryType& boundary)
    : filter() {
    parameters tmpParam;
    tmpParam.setKernel(aKernel);
    tmpParam.boundaryType = boundary;
    setParameters(tmpParam);
  }

  // copy constructor
  convolution::convolution(const convolution& other)
    : filter()  {
    copy(other);
  }

  // destructor
  convolution::~convolution() {
  }

  // returns the name of this type
  const char* convolution::getTypeName() const {
    return "convolution";
  }

  // copy member
  convolution& convolution::copy(const convolution& other) {
    filter::copy(other);
    return (*this);
  }

  // clone member
  functor* convolution::clone() const {
    return new convolution(*this);
  }

  // return parameters
  const convolution::parameters& convolution::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  void convolution::setKernel(const mathObject& aKernel) {
    parameters tmpParam(getParameters());
    tmpParam.setKernel(aKernel);
    setParameters(tmpParam);
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On place apply for type channel8!
  bool convolution::apply(matrix<channel8::value_type>& srcdest) const {
    matrix<channel8::value_type> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }
    return false;
  };

  // On place apply for type channel!
  bool convolution::apply(matrix<channel::value_type>& srcdest) const {
    matrix<channel::value_type> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type channel!
  bool convolution::apply(dmatrix& srcdest) const {
    dmatrix tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };


  // On place apply for type vector<channel8::value_type>!
  bool convolution::apply(vector<channel8::value_type>& srcdest) const {
    vector<channel8::value_type> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type vector<channel::value_type>!
  bool convolution::apply(vector<channel::value_type>& srcdest) const {

    vector<channel::value_type> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type dvector!
  bool convolution::apply(dvector& srcdest) const {

    dvector tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On copy apply for type channel8!
  bool convolution::apply(const matrix<channel8::value_type>& src,
                           matrix<channel8::value_type>& dest) const {

    // there are two possibilities: kernel2D or sepKernel
    const parameters& param = getParameters();
    convHelper2D<channel8::value_type,
                 accumulator<channel8::value_type,int> > conv;

    if (src.empty()) {
      dest.clear();
      return true;
    }

	if (!conv.setKernel(&param.getKernel())) {
      throw invalidParametersException(getTypeName());
    }

    if (conv.isKernelSeparable()) {
      conv.applySep(src,dest,param);
    } else {
      conv.apply(src,dest,param);
    }

    return true;
  };

  // On copy apply for type channel!
  bool convolution::apply(const matrix<channel::value_type>& src,
                           matrix<channel::value_type>& dest) const {

    const parameters& param = getParameters();
    convHelper2D<channel::value_type,accumulator<channel::value_type> > conv;

    if (src.empty()) {
      dest.clear();
      return true;
    }

	if (!conv.setKernel(&param.getKernel())) {
      throw invalidParametersException(getTypeName());
    }

    if (conv.isKernelSeparable()) {
      conv.applySep(src,dest,param);
    } else {
      conv.apply(src,dest,param);
    }

    return true;
  };

  // On copy apply for type dmatrix!
  bool convolution::apply(const dmatrix& src,dmatrix& dest) const {

    const parameters& param = getParameters();
    convHelper2D<dmatrix::value_type,accumulator<dmatrix::value_type> > conv;

    if (src.empty()) {
      dest.clear();
      return true;
    }

	if (!conv.setKernel(&param.getKernel())) {
      throw invalidParametersException(getTypeName());
    }

    if (conv.isKernelSeparable()) {
      conv.applySep(src,dest,param);
    } else {
      conv.apply(src,dest,param);
    }

    return true;
  };


  // On copy apply for type vector<channel8::value_type>!
  bool convolution::apply(const vector<channel8::value_type>& src,
                                vector<channel8::value_type>& dest) const {

    const parameters& param = getParameters();
    convHelper1D<channel8::value_type,accumulator<int> > conv;

    if (src.empty()) {
      dest.clear();
      return true;
    }

    if (!conv.setKernel(&param.getKernel())) {
      throw invalidParametersException(getTypeName());
    }

    conv.apply(src,dest,param);

    return true;
  };

  // On copy apply for type vector<channel::value_type>!
  bool convolution::apply(const vector<channel::value_type>& src,
                                vector<channel::value_type>& dest) const {

    const parameters& param = getParameters();
    convHelper1D<channel::value_type,accumulator<channel::value_type> > conv;

    if (src.empty()) {
      dest.clear();
      return true;
    }

    if (!conv.setKernel(&param.getKernel())) {
      throw invalidParametersException(getTypeName());
    }

    conv.apply(src,dest,param);

    return true;
  };

  // On copy apply for type dvector!
  bool convolution::apply(const dvector& src,
                                dvector& dest) const {

    const parameters& param = getParameters();
    convHelper1D<double,accumulator<double> > conv;

    if (src.empty()) {
      dest.clear();
      return true;
    }

    if (!conv.setKernel(&param.getKernel())) {
      throw invalidParametersException(getTypeName());
    }

    conv.apply(src,dest,param);

    return true;
  };

  // call the upper classes masked method
  bool convolution::apply(const image& src,
                                image& dest) const {
    return filter::apply(src,dest);
  }

  // call the upper classes masked method
  bool convolution::apply(image& srcdest) const {
    return filter::apply(srcdest);    
  }

  // --------------------------------------------------
  // specializations!
  // --------------------------------------------------

  template<>
  void convolution::accumulator<ubyte,int>::accumulate(
      const ubyte& filter,
      const ubyte& src) {
    state += static_cast<int>(src)*static_cast<int>(filter);
  }

  /*
   * specialization for integers
   */
  template<>
  int convolution::accumulator<int,int>::getResult() const {
    return state/norm;
  }

  template<>
  ubyte convolution::accumulator<ubyte,int>::getResult() const {
    return static_cast<ubyte>(state/norm);
  }

}
