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
 * file .......: ltiDownsampling.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 11.5.2000
 * revisions ..: $Id: ltiDownsampling.cpp,v 1.11 2006/09/05 10:09:32 ltilib Exp $
 */

#include "ltiDownsampling.h"
#include "ltiSplitImageToRGB.h"
#include "ltiMergeRGBToImage.h"

namespace lti {
  // --------------------------------------------------
  // downsampling::parameters
  // --------------------------------------------------

  // default constructor
  downsampling::parameters::parameters()
    : filter::parameters(), kernel(0) {
    sepKernel<channel::value_type>* tmp;
    tmp = new sepKernel<channel::value_type>(-1,1,0.25f);
    tmp->getRowFilter(0).at(0) = 0.5f;
    tmp->getColFilter(0).at(0) = 0.5f;

    kernel = tmp;
    factor = point(2,2);
  }

  // copy constructor
  downsampling::parameters::parameters(const parameters& other)
    : filter::parameters()  {
    kernel = 0;
    copy(other);
  }

  // destructor
  downsampling::parameters::~parameters() {
    delete kernel;
    kernel = 0;
  }

  // get type name
  const char* downsampling::parameters::getTypeName() const {
    return "downsampling::parameters";
  }

  // copy member
  downsampling::parameters&
    downsampling::parameters::copy(const parameters& other) {
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

    factor = other.factor;

    return *this;
  }

  const mathObject& downsampling::parameters::getKernel() const {
    if (isNull(kernel)) {
      throw invalidParametersException(getTypeName());
    }
    return *kernel;
  }

  void downsampling::parameters::setKernel(const mathObject& aKernel) {
    delete kernel;
    kernel = 0;
    kernel = aKernel.clone();
  }

  // clone member
  functor::parameters* downsampling::parameters::clone() const {
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
  bool downsampling::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool downsampling::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"factor",factor);
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
  bool downsampling::parameters::write(ioHandler& handler,
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
  bool downsampling::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool downsampling::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"factor",factor);
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
  bool downsampling::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // downsampling
  // --------------------------------------------------

  // default constructor
  downsampling::downsampling()
    : filter() {
    parameters tmp;
    setParameters(tmp);
  }

  // default constructor
  downsampling::downsampling(const parameters& par)
    : filter() {
    setParameters(par);
  }

  // constructor with a kernel
  downsampling::downsampling(const mathObject& aKernel)
    : filter() {
    parameters tmp;
    tmp.setKernel(aKernel);
    setParameters(tmp);
  }

  // copy constructor
  downsampling::downsampling(const downsampling& other)
    : filter()  {
    copy(other);
  }

  // destructor
  downsampling::~downsampling() {
  }

  // returns the name of this type
  const char* downsampling::getTypeName() const {
    return "downsampling";
  }

  // copy member
  downsampling& downsampling::copy(const downsampling& other) {
    filter::copy(other);
    return (*this);
  }

  // clone member
  functor* downsampling::clone() const {
    return new downsampling(*this);
  }

  // return parameters
  const downsampling::parameters&
    downsampling::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  void downsampling::setKernel(const mathObject& aKernel) {
    parameters tmp(getParameters());
    tmp.setKernel(aKernel);
    setParameters(tmp);
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On place apply for type channel8!
  bool downsampling::apply(channel8& srcdest) const {

    channel8 tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type channel!
  bool downsampling::apply(matrix<float>& srcdest) const {
    matrix<float> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type dmatrix!
  bool downsampling::apply(dmatrix& srcdest) const {
    dmatrix tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type channel!
  bool downsampling::apply(image& srcdest) const {

    if (srcdest.empty()) {
      setStatusString("input image empty");
      return false;
    }

    channel8 r,g,b;
    splitImageToRGB splitter;
    mergeRGBToImage merger;

    splitter.apply(srcdest,r,g,b);
    apply(r);
    apply(g);
    apply(b);

    return merger.apply(r,g,b,srcdest);
  };

  // On place apply for type vector<channel8::value_type>!
  bool downsampling::apply(vector<channel8::value_type>& srcdest) const {

    vector<channel8::value_type> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type vector<channel::value_type>!
  bool downsampling::apply(vector<channel::value_type>& srcdest) const {

    vector<channel::value_type> tmp;

    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);

      return true;
    }

    return false;
  };

  // On copy apply for type channel8!
  bool downsampling::apply(const channel8& src,channel8& dest) const {

    // there are two possibilities: kernel2D or sepKernel
    const parameters& param = getParameters();
    convolve2D<channel8::value_type,int> conv;
    if (!conv.setKernel(param.kernel)) {
      throw invalidParametersException(getTypeName());
    }

    if (conv.isKernelSeparable()) {
      conv.applySep(src,dest,param);
    } else {
      conv.apply(src,dest,param);
    }

    return true;
  };

  // On copy apply for type matrix<float>!
  bool downsampling::apply(const matrix<float>& src,
                                   matrix<float>& dest) const {

    const parameters& param = getParameters();
    convolve2D<float> conv;

    if (!conv.setKernel(param.kernel)) {
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
  bool downsampling::apply(const dmatrix& src, dmatrix& dest) const {

    const parameters& param = getParameters();
    convolve2D<double> conv;

    if (!conv.setKernel(param.kernel)) {
      throw invalidParametersException(getTypeName());
    }

    if (conv.isKernelSeparable()) {
      conv.applySep(src,dest,param);
    } else {
      conv.apply(src,dest,param);
    }

    return true;
  };

  // On place apply for type channel!
  bool downsampling::apply(const image& src, image& dest) const {

    if (src.empty()) {
      setStatusString("input image empty");
      dest.clear();
      return false;
    }

    channel8 r,g,b;
    splitImageToRGB splitter;
    mergeRGBToImage merger;

    splitter.apply(src,r,g,b);
    apply(r);
    apply(g);
    apply(b);

    return merger.apply(r,g,b,dest);
  };


  // On copy apply for type vector<channel8::value_type>!
  bool downsampling::apply(const vector<channel8::value_type>& src,
                                 vector<channel8::value_type>& dest) const {

    const parameters& param = getParameters();
    convolve1D<channel8::value_type,int> conv;
    if (!conv.setKernel(param.kernel)) {
      throw invalidParametersException(getTypeName());
    }

    conv.apply(src,dest,param);

    return true;
  };

  // On copy apply for type vector<matrix<float>::value_type>!
  bool downsampling::apply(const vector<channel::value_type>& src,
                                 vector<channel::value_type>& dest) const {

    const parameters& param = getParameters();
    convolve1D<channel::value_type,channel::value_type> conv;
    if (!conv.setKernel(param.kernel)) {
      throw invalidParametersException(getTypeName());
    }

    conv.apply(src,dest,param);

    return true;
  };

  /* accumulator (specialization for ubyte elements)
   */
  template<>
  void lti::downsampling::convolve<ubyte,int>::accumulate(const ubyte& filter,
                                                          const ubyte& src,
                                                           int& result) {
    result += static_cast<int>(filter)*static_cast<int>(src);
  }


  /* get result (specialization for integers)
   */
  template<>
  int lti::downsampling::convolve<int,int>::getResult(const int& result) {
    return (result/norm);
  }

  /* get result (specialization for ubyte)
  */
  template<>
  ubyte lti::downsampling::convolve<ubyte,int>::getResult(const int& result) {
    return (result/norm);
  }

}
