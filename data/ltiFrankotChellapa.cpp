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
 * file .......: ltiFrankotChellapa.cpp
 * authors ....: Frederik Lange
 * organization: LTI, RWTH Aachen
 * creation ...: 4.4.2001
 * revisions ..: $Id: ltiFrankotChellapa.cpp,v 1.8 2006/09/05 10:13:25 ltilib Exp $
 */

#ifdef _DEBUG
#define _DEBUG_FRANKOT_CHELLAPA 0
#endif

#include "ltiFrankotChellapa.h"

#include "ltiConvolution.h"
#include "ltiLinearKernels.h"
#include "ltiDownsampling.h"
#include "ltiGaussKernels.h"
#include "ltiTransform.h"
#include "ltiRealFFT.h"
#include "ltiRealInvFFT.h"
#include "ltiGradientKernels.h"

namespace lti {


  // --------------------------------------------------
  // frankotChellapa::parameters
  // --------------------------------------------------

  // default constructor
  frankotChellapa::parameters::parameters()
    : modifier::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    scale = int(0.4);
    kernelSize = int(5);
  }

  // copy constructor
  frankotChellapa::parameters::parameters(const parameters& other)
    : modifier::parameters()  {
    copy(other);
  }

  // destructor
  frankotChellapa::parameters::~parameters() {
  }

  // get type name
  const char* frankotChellapa::parameters::getTypeName() const {
    return "frankotChellapa::parameters";
  }

  // copy member

  frankotChellapa::parameters&
    frankotChellapa::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    modifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    modifier::parameters& (modifier::parameters::* p_copy)
      (const modifier::parameters&) =
      modifier::parameters::copy;
    (this->*p_copy)(other);
# endif


      scale = other.scale;
      kernelSize = other.kernelSize;

    return *this;
  }

  // alias for copy member
  frankotChellapa::parameters&
    frankotChellapa::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* frankotChellapa::parameters::clone() const {
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
  bool frankotChellapa::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool frankotChellapa::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"scale",scale);
      lti::write(handler,"kernelSize",kernelSize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::write(handler,false);
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
  bool frankotChellapa::parameters::write(ioHandler& handler,
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
  bool frankotChellapa::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool frankotChellapa::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"scale",scale);
      lti::read(handler,"kernelSize",kernelSize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::read(handler,false);
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
  bool frankotChellapa::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // frankotChellapa
  // --------------------------------------------------

  // default constructor
  frankotChellapa::frankotChellapa()
    : modifier(){
    parameters param;
    setParameters(param);
  }

  // copy constructor
  frankotChellapa::frankotChellapa(const frankotChellapa& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  frankotChellapa::~frankotChellapa() {
  }

  // returns the name of this type
  const char* frankotChellapa::getTypeName() const {
    return "frankotChellapa";
  }

  // copy member
  frankotChellapa&
    frankotChellapa::copy(const frankotChellapa& other) {
      modifier::copy(other);
    return (*this);
  }

  // clone member
  functor* frankotChellapa::clone() const {
    return new frankotChellapa(*this);
  }

  // return parameters
  const frankotChellapa::parameters&
    frankotChellapa::getParameters() const {
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
  bool frankotChellapa::apply(channel& srcdest) {

    channel dest;

    if (apply(srcdest, dest)) {
      dest.detach(dest);
      return true;
    }
    return false;
  };

  // On copy apply for type channel!
  bool frankotChellapa::apply(const channel& src,channel& dest) {

    // calculate the gradients

    channel zMap,
      gradX,
      gradY,
      tmp_i;

    tmp_i = src;
    float max_intensity = tmp_i.maximum();

    const parameters& par = getParameters();

    gradX.resize(tmp_i.rows(), tmp_i.columns());
    gradY.resize(tmp_i.rows(), tmp_i.columns());

    createHGrad(tmp_i, gradX, par.kernelSize);
    createVGrad(tmp_i, gradY, par.kernelSize);

    float scale; // scale the gradients

    int i,j;

    zMap.resize(tmp_i.rows(), tmp_i.columns());

    channel fftXReal,
      fftXImg,
      fftYReal,
      fftYImg;

    realFFT fftFunc;
    realFFT::parameters realPars;

    int rx_ = tmp_i.columns();
    int ry_ = tmp_i.rows();

    for (i = 0; i < ry_; ++i)	{
      for (j = 0; j < rx_; ++j)	{
        gradX[i][j] *= 5.0 ;
        gradY[i][j] *= 5.0 ;

        if ((i == ry_ - 1)||(j == rx_ - 1)) {
          gradX[i][j] = 0;
          gradY[i][j] = 0;
        }
      }
    }

    fftFunc.apply(gradX, fftXReal, fftXImg);
    fftFunc.apply(gradY, fftYReal, fftYImg);
    channel h1,h2;

    h1.resize(tmp_i.rows(), tmp_i.columns());
    h2.resize(tmp_i.rows(), tmp_i.columns());

    int rx2 = tmp_i.columns() / 2;
    int ry2 = tmp_i.rows() / 2;

    for (i = 0; i < ry_; i++)	{
      for (j = 0; j < rx_; j++)	{
        if ( (i == 0) && (j == 0) ) {
          continue;
        }

        if ( (i < ry2) && ( j < rx2) ) {
          h1[i][j] = (((float)j) * fftXImg[i][j] +
                      ((float)i) * fftYImg[i][j]) /
            ((float)(i*i + j*j));
          h2[i][j] = (-((float)j) * fftXReal[i][j] -
                      ((float)i) * fftYReal[i][j]) /
            ((float)(i*i + j*j));
        }
        else if ( (i < ry2) && ( j >= rx2) ) {
          h1[i][j] = (((float)j-rx_) * fftXImg[i][j] +
                      ((float)i) * fftYImg[i][j]) /
            ((float)((i)*(i) + (j-rx_)*(j-rx_)));
          h2[i][j] = ( -((float)j-rx_) * fftXReal[i][j] -
                       ((float)i) * fftYReal[i][j]) /
            ((float)((i)*(i) + (j-rx_)*(j-rx_)));
        }
        else if ( (i >= ry2) && ( j < rx2) ) {
          h1[i][j] = (((float)j) * fftXImg[i][j] +
                      ((float)i-ry_) * fftYImg[i][j]) /
            ((float)((i-ry_)*(i-ry_) + (j)*(j)));
          h2[i][j] = ( -((float)j) * fftXReal[i][j] -
                       ((float)i-ry_) * fftYReal[i][j]) /
            ((float)((i-ry_)*(i-ry_) + (j)*(j)));
        }
        else if ( (i >= ry2) && ( j >= rx2) ) {
          h1[i][j] = (((float)j-rx_) * fftXImg[i][j] +
                      ((float)i-ry_) * fftYImg[i][j]) /
            ((float)((i-ry_)*(i-ry_) + (j-rx_)*(j-rx_)));
          h2[i][j] = ( -((float)j-rx_) * fftXReal[i][j] -
                       ((float)i-ry_) * fftYReal[i][j]) /
            ((float)((i-ry_)*(i-ry_) + (j-rx_)*(j-rx_)));
        }
      }
    }

    h1[0][0] = 0.0;
    h2[0][0] = 0.0;

    realInvFFT backFFT;
    realInvFFT::parameters invPars;

    backFFT.apply(h1,h2,zMap);

    scale = par.scale / max_intensity;

    //  	zMap.resize(ry_, rx_);

    for (i = 0; i < ry_; i++)	{
      for (j = 0; j < rx_; j++)	{
        zMap[i][j] *= 7.0;
        if ((i == ry_ - 1)||(j == rx_ - 1)) {
          zMap[i][j] = 0.0;
        }
      }
    }

    dest = zMap;

    return true;
  };

  void frankotChellapa::createVGrad(const channel &src,
                                          channel &dest, int kernelSize) {

    convolution convoluter_;

    gradientKernelY<float> y_kernel(kernelSize);
    convolution::parameters convoluter_parameters;

    // create and overgive the parameters
    convoluter_parameters.setKernel(y_kernel);
    convoluter_.setParameters(convoluter_parameters);

    // perform the gradient generation
    convoluter_.apply(src, dest);
  }

  // generation of the horizontal gradient
  void frankotChellapa::createHGrad(const channel &src,
                                          channel &dest, int kernelSize) {

    convolution convoluter_;

    gradientKernelX<float> x_kernel(kernelSize);
    convolution::parameters convoluter_parameters;

    // create and overgive the parameters
    convoluter_parameters.setKernel(x_kernel);
    convoluter_.setParameters(convoluter_parameters);

    // perform the gradient generation
    convoluter_.apply(src, dest);
  }

}
