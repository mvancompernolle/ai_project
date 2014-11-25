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
 * file .......: ltiFilledUpsampling.cpp
 * authors ....: Pablo Alvarado, Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 12.5.2000
 * revisions ..: $Id: ltiFilledUpsampling.cpp,v 1.10 2006/09/05 10:12:54 ltilib Exp $
 */

#include "ltiFilledUpsampling.h"
#include "ltiConvolution.h"
#include "ltiGaussKernels.h"

namespace lti {


  // --------------------------------------------------
  // filledUpsampling::parameters
  // --------------------------------------------------

  // default constructor
  filledUpsampling::parameters::parameters()
    : filter::parameters() {

    kernel=0;
    factor = point(2,2);
  }

  // copy constructor
  filledUpsampling::parameters::parameters(const parameters& other)
    : filter::parameters()  {
    kernel = 0;
    copy(other);
  }

  // destructor
  filledUpsampling::parameters::~parameters() {
  }

  // get type name
  const char* filledUpsampling::parameters::getTypeName() const {
    return "filledUpsampling::parameters";
  }

  // copy member

  filledUpsampling::parameters&
    filledUpsampling::parameters::copy(const parameters& other) {
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

  const mathObject& filledUpsampling::parameters::getKernel() const {
    if (isNull(kernel)) {
      // filter for not filtering at all
      static const kernel1D<channel::value_type> tmp(0,0,1.0f);
      static const sepKernel<channel::value_type> tmpSep(tmp);
      return tmpSep;
    }
    return *kernel;
  }

  void filledUpsampling::parameters::setKernel(const mathObject& aKernel) {
    delete kernel;
    kernel = 0;
    kernel = aKernel.clone();
  }

  // clone member
  functor::parameters* filledUpsampling::parameters::clone() const {
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
  bool filledUpsampling::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool filledUpsampling::parameters::writeMS(ioHandler& handler,
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
  bool filledUpsampling::parameters::write(ioHandler& handler,
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
  bool filledUpsampling::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool filledUpsampling::parameters::readMS(ioHandler& handler,
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
        // assume float (maybe to be change in the near future ;-)
        kernel1D<float> kern;
        lti::write(handler,"kernel",kern);
        setKernel(kern);
      } else if (str == "kernel2D") {
        // assume float (maybe to be change in the near future ;-)
        kernel2D<float> kern;
        lti::write(handler,"kernel",kern);
        setKernel(kern);
      } else if (str == "sepKernel") {
        // assume float (maybe to be change in the near future ;-)
        sepKernel<float> kern;
        lti::write(handler,"kernel",kern);
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
  bool filledUpsampling::parameters::read(ioHandler& handler,
                                          const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // filledUpsampling
  // --------------------------------------------------


  // private class
  /*
   * upsampling of a vector
   */
  template <class T>
  bool lti::filledUpsampling::genericUpsampler<T>::apply(const int& factor,
                                                   const vector<T>& src,
                                                   vector<T>& dest) {

    if (factor < 1)
      return false;

    if (src.empty()) {
      dest.clear();
      return true;
    }

    const int newsize = src.size()*factor;
    if (newsize != dest.size()) {
      dest.resize(src.size()*factor,T(),false,false);
    }

    int i;
    const int size = src.size();
    T val;

    // some pointer arithmetic required to do things faster:
    T* ptr = &dest.at(0);  // the position at the dest vector
    T* ptrEnd;             // the end of the actual pixel

    for (i=0;i<size;++i) {
      val = src.at(i);
      // write the val in the next "factor" pixels
      for (ptrEnd=ptr+factor;ptr<ptrEnd;++ptr) {
        *ptr = val;
      }
    }

    return true;
  }

  /*
   * upsampling of a matrix
   */
  template <class T>
  bool lti::filledUpsampling::genericUpsampler<T>::apply(const point& factor,
                                                   const matrix<T>& src,
                                                   matrix<T>& dest) {

    if ((factor.x < 1) || (factor.y < 1)) {
      return false;
    }

    if (src.empty()) {
      dest.clear();
      return true;
    }

    int i,j,k,row;

    dest.resize(src.rows()*factor.y,src.columns()*factor.x,T(),false,false);

    for (i=0,j=0;i<src.rows();++i) {
      apply(factor.x,src.getRow(i),dest.getRow(j));
      for (row=j,k=row+factor.y,++j;j<k;++j) {
        dest.getRow(j).copy(dest.getRow(row));
      }
    }

    return true;
  }

  // default constructor
  filledUpsampling::filledUpsampling()
    : filter() {
    parameters tmp;
    setParameters(tmp);
  }

  // default constructor
  filledUpsampling::filledUpsampling(const point& factor)
    : filter() {
    parameters tmp;
    tmp.factor = factor;
    setParameters(tmp);
  }

  // square upsampling constructor
  filledUpsampling::filledUpsampling(const int& factor)
    : filter() {
    parameters tmp;
    tmp.factor = point(factor,factor);
    setParameters(tmp);
  }


  // copy constructor
  filledUpsampling::filledUpsampling(const filledUpsampling& other)
    : filter()  {
    copy(other);
  }

  // destructor
  filledUpsampling::~filledUpsampling() {
  }

  // returns the name of this type
  const char* filledUpsampling::getTypeName() const {
    return "filledUpsampling";
  }

  // copy member
  filledUpsampling&
    filledUpsampling::copy(const filledUpsampling& other) {
      filter::copy(other);
    return (*this);
  }

  // clone member
  functor* filledUpsampling::clone() const {
    return new filledUpsampling(*this);
  }

  // return parameters
  const filledUpsampling::parameters&
    filledUpsampling::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  void filledUpsampling::setKernel(const mathObject& aKernel) {
    parameters tmpParam(getParameters());
    tmpParam.setKernel(aKernel);
    setParameters(tmpParam);
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On place apply for type channel8!
  bool filledUpsampling::apply(channel8& srcdest) const {
    channel8 tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }
    return false;
  };

  // On place apply for type channel!
  bool filledUpsampling::apply(channel& srcdest) const {
    channel tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type channel!
  bool filledUpsampling::apply(image& srcdest) const {
    image tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type dmatrix
  bool filledUpsampling::apply(dmatrix& srcdest) const {
    dmatrix tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type channel!
  bool filledUpsampling::apply(imatrix& srcdest) const {
    imatrix tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type vector<channel8::value_type>!
  bool filledUpsampling::apply(vector<channel8::value_type>& srcdest) const {

    vector<channel8::value_type> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On place apply for type vector<channel::value_type>!
  bool filledUpsampling::apply(vector<channel::value_type>& srcdest) const {

    vector<channel::value_type> tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On copy apply for type channel8!
  bool filledUpsampling::apply(const channel8& src,channel8& dest) const {

    const parameters& param = getParameters();
    bool result;
    genericUpsampler<channel8::value_type> upsampler;


    if (notNull(param.kernel)) {
      channel8 tmp;

      result = upsampler.apply(param.factor,src,tmp);

      convolution conv;
      convolution::parameters paramc;
      paramc.setKernel(param.getKernel());
      conv.setParameters(paramc);
      conv.apply(tmp,dest);
    } else {
      result = upsampler.apply(param.factor,src,dest);
    }

    return result;
  };

  // On copy apply for type channel!
  bool filledUpsampling::apply(const channel& src,channel& dest) const {
    const parameters& param = getParameters();
    bool result;
    genericUpsampler<channel::value_type> upsampler;


    if (notNull(param.kernel)) {
      channel tmp;

      result = upsampler.apply(param.factor,src,tmp);

      convolution conv;
      convolution::parameters paramc;
      paramc.setKernel(param.getKernel());
      conv.setParameters(paramc);
      conv.apply(tmp,dest);
    } else {
      result = upsampler.apply(param.factor,src,dest);
    }

    return result;
  };

  // On copy apply for type channel!
  bool filledUpsampling::apply(const image& src,image& dest) const {
    const parameters& param = getParameters();
    bool result;
    genericUpsampler<image::value_type> upsampler;

    result = upsampler.apply(param.factor,src,dest);

    if (notNull(param.kernel)) {
      setStatusString("Filled Upsampling cannot filter color images yet");
      result = false;
    }

    return result;
  };

  // On copy apply for type dmatrix!
  bool filledUpsampling::apply(const dmatrix& src,dmatrix& dest) const {
    const parameters& param = getParameters();
    bool result;
    genericUpsampler<double> upsampler;

//     if (notNull(param.kernel)) {
//       imatrix tmp;

//       result = upsampler.apply(param.factor,src,tmp);

//       convolution conv;
//       convolution::parameters paramc;
//       paramc.setKernel(param.getKernel());
//       conv.setParameters(paramc);
//       conv.apply(tmp,dest);
//     } else {
    result = upsampler.apply(param.factor,src,dest);
//     }

    return result;
  };

  // On copy apply for type imatrix!
  bool filledUpsampling::apply(const imatrix& src,imatrix& dest) const {
    const parameters& param = getParameters();
    bool result;
    genericUpsampler<imatrix::value_type> upsampler;


//     if (notNull(param.kernel)) {
//       imatrix tmp;

//       result = upsampler.apply(param.factor,src,tmp);

//       convolution conv;
//       convolution::parameters paramc;
//       paramc.setKernel(param.getKernel());
//       conv.setParameters(paramc);
//       conv.apply(tmp,dest);
//     } else {
      result = upsampler.apply(param.factor,src,dest);
//     }

    return result;
  };


  // On copy apply for type vector<channel8::value_type>!
  bool filledUpsampling::apply(const vector<channel8::value_type>& src,
                                    vector<channel8::value_type>& dest) const {

    const parameters& param = getParameters();
    bool result;
    genericUpsampler<channel8::value_type> upsampler;


    if (notNull(param.kernel)) {
      vector<channel8::value_type> tmp;

      result = upsampler.apply(param.factor.x,src,tmp);

      convolution conv;
      convolution::parameters paramc;
      paramc.setKernel(param.getKernel());
      conv.setParameters(paramc);
      conv.apply(tmp,dest);
    } else {
      result = upsampler.apply(param.factor.x,src,dest);
    }

    return true;
  };

  // On copy apply for type vector<channel::value_type>!
  bool filledUpsampling::apply(const vector<channel::value_type>& src,
                                     vector<channel::value_type>& dest) const {

    const parameters& param = getParameters();
    bool result;
    genericUpsampler<channel::value_type> upsampler;


    if (notNull(param.kernel)) {
      vector<channel::value_type> tmp;

      result = upsampler.apply(param.factor.x,src,tmp);

      convolution conv;
      convolution::parameters paramc;
      paramc.setKernel(param.getKernel());
      conv.setParameters(paramc);
      conv.apply(tmp,dest);
    } else {
      result = upsampler.apply(param.factor.x,src,dest);
    }

    return true;
  };

}
