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
 * file .......: ltiErosion.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 19.7.2000
 * revisions ..: $Id: ltiErosion.cpp,v 1.10 2006/09/05 10:10:35 ltilib Exp $
 */

#include "ltiErosion.h"
#include "ltiConvolutionHelper.h"

namespace lti {

  // specialization for <ubyte, int>
  template<>
  ubyte lti::erosion::accumulatorGray<ubyte, int>::getResult() const {
    return (state < 0 ? ubyte(0) : static_cast<ubyte>(state));
  }


  // --------------------------------------------------
  // erosion::parameters
  // --------------------------------------------------

  // default constructor
  erosion::parameters::parameters()
    : morphology::parameters(), mode(Binary), kernel(0) {

  }

  // copy constructor
  erosion::parameters::parameters(const parameters& other)
    : morphology::parameters()  {
    kernel = 0;
    copy(other);
  }

  // destructor
  erosion::parameters::~parameters() {
    delete kernel;
  }

  // get type name
  const char* erosion::parameters::getTypeName() const {
    return "erosion::parameters";
  }

  // copy member

  erosion::parameters&
    erosion::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    morphology::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    morphology::parameters& (morphology::parameters::* p_copy)
      (const morphology::parameters&) =
      morphology::parameters::copy;
    (this->*p_copy)(other);
# endif

    delete kernel;
    kernel = 0;

    if (notNull(other.kernel)) {
      kernel =other.kernel->clone();
    }

    mode = other.mode;

    return *this;
  }

  // clone member
  functor::parameters* erosion::parameters::clone() const {
    return new parameters(*this);
  }

  const mathObject& erosion::parameters::getKernel() const {
    if (isNull(kernel)) {
      throw invalidParametersException(getTypeName());
    }
    return *kernel;
  }

  void erosion::parameters::setKernel(const mathObject& aKernel) {
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
  bool erosion::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool erosion::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      if (mode == Gray) {
        lti::write(handler,"mode","Gray");
      } else {
        lti::write(handler,"mode","Binary");
      }

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
    b = b && morphology::parameters::write(handler,false);
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
  bool erosion::parameters::write(ioHandler& handler,
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
  bool erosion::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool erosion::parameters::readMS(ioHandler& handler,
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
      if (str == "Gray") {
        mode = Gray;
      } else {
        mode = Binary;
      }

      lti::read(handler,"kernelType",str);
      if (str == "kernel1D") {
        // assume float (maybe to be change in the near future ;-)
        kernel1D<float> kern;
        lti::read(handler,"kernel",kern);
        setKernel(kern);
      } else if (str == "kernel2D") {
        // assume float (maybe to be change in the near future ;-)
        kernel2D<float> kern;
        lti::read(handler,"kernel",kern);
        setKernel(kern);
      } else if (str == "sepKernel") {
        // assume float (maybe to be change in the near future ;-)
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
    b = b && morphology::parameters::read(handler,false);
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
  bool erosion::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // erosion
  // --------------------------------------------------

  // default constructor
  erosion::erosion()
    : morphology() {
    parameters tmpParam;
    setParameters(tmpParam);
  }

  // default constructor
  erosion::erosion(const mathObject& aKernel)
    : morphology() {
    parameters tmpParam;
    tmpParam.setKernel(aKernel);
    setParameters(tmpParam);
  }


  // copy constructor
  erosion::erosion(const erosion& other)
    : morphology()  {
    copy(other);
  }

  // destructor
  erosion::~erosion() {
  }

  // returns the name of this type
  const char* erosion::getTypeName() const {
    return "erosion";
  }

  // copy member
  erosion& erosion::copy(const erosion& other) {
    morphology::copy(other);
    return (*this);
  }

  // clone member
  functor* erosion::clone() const {
    return new erosion(*this);
  }

  // return parameters
  const erosion::parameters& erosion::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  void erosion::setKernel(const mathObject& aKernel) {
    parameters tmpParam(getParameters());
    tmpParam.setKernel(aKernel);
    setParameters(tmpParam);
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On place apply for type channel!
  bool erosion::apply(channel& srcdest) const {
    channel tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type channel8!
  bool erosion::apply(channel8& srcdest) const {
    channel8 tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type fvector!
  bool erosion::apply(fvector& srcdest) const {

    vector<channel::value_type> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type vector<ubyte>!
  bool erosion::apply(vector<ubyte>& srcdest) const {

    vector<channel8::value_type> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On copy apply for type channel!
  bool erosion::apply(const channel& src,channel& dest) const {

    const parameters& param = getParameters();
    if (param.mode == parameters::Binary) {
      convHelper2D<channel::value_type, accumulatorBin<float,float> > conv;
      if (!conv.setKernel(&param.getKernel())) {
        throw invalidParametersException(getTypeName());
      }

      if (conv.isKernelSeparable()) {
        conv.applySep(src,dest,param);
      } else {
        conv.apply(src,dest,param);
      }
    } else {
      convHelper2D<channel::value_type,
                          accumulatorGray<float,float> > conv;
      if (!conv.setKernel(&param.getKernel())) {
        throw invalidParametersException(getTypeName());
      }

      if (conv.isKernelSeparable()) {
        conv.applySep(src,dest,param);
      } else {
        conv.apply(src,dest,param);
      }
    }

    return true;
  };

  // On copy apply for type channel8!
  bool erosion::apply(const channel8& src,channel8& dest) const {

    // there are two possibilities: kernel2D or sepKernel
    const parameters& param = getParameters();
    if (param.mode == parameters::Binary) {
      convHelper2D<channel8::value_type,
                          accumulatorBin<channel8::value_type,int> > conv;
      if (!conv.setKernel(&param.getKernel())) {
        throw invalidParametersException(getTypeName());
      }

      if (conv.isKernelSeparable()) {
        conv.applySep(src,dest,param);
      } else {
        conv.apply(src,dest,param);
      }
    } else {
      convHelper2D<channel8::value_type,
                          accumulatorGray<channel8::value_type,int> > conv;
      if (!conv.setKernel(&param.getKernel())) {
        throw invalidParametersException(getTypeName());
      }

      if (conv.isKernelSeparable()) {
        conv.applySep(src,dest,param);
      } else {
        conv.apply(src,dest,param);
      }
    }

    return true;
  };

  // On copy apply for type fvector!
  bool erosion::apply(const fvector& src,fvector& dest) const {

    const parameters& param = getParameters();

    if (param.mode == parameters::Binary) {
      convHelper1D<channel::value_type,
                          accumulatorBin<channel::value_type,
                                         channel::value_type> > conv;
      if (!conv.setKernel(&param.getKernel())) {
        throw invalidParametersException(getTypeName());
      }

      conv.apply(src,dest,param);
    } else {
      convHelper1D<channel::value_type,
                          accumulatorGray<channel::value_type,
                                          channel::value_type> > conv;
      if (!conv.setKernel(&param.getKernel())) {
        throw invalidParametersException(getTypeName());
      }

      conv.apply(src,dest,param);
    }

    return true;
  };

  // On copy apply for type vector<ubyte>!
  bool erosion::apply(const vector<ubyte>& src,
                                       vector<ubyte>& dest) const {

    const parameters& param = getParameters();

    if (param.mode == parameters::Binary) {
      convHelper1D<channel8::value_type,
                          accumulatorBin<channel8::value_type,
                                         int> > conv;
      if (!conv.setKernel(&param.getKernel())) {
        throw invalidParametersException(getTypeName());
      }

      conv.apply(src,dest,param);
    } else {
      convHelper1D<channel8::value_type,
                          accumulatorGray<channel8::value_type,
                                         int> > conv;
      if (!conv.setKernel(&param.getKernel())) {
        throw invalidParametersException(getTypeName());
      }

      conv.apply(src,dest,param);
    }

    return true;
  };

}
