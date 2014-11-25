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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiRealFT.cpp
 * authors ....: Stefan Syberichs
 * organization: LTI, RWTH Aachen
 * comment.....: FFT class based on modified code by Takuya Ooura
 *               with kind permission ooura@kurims.kyoto-u.ac.jp or
 *               ooura@mmm.t.u-tokyo.ac.jp)
 * creation ...: 06.12.99
 * revisions ..: $Id: ltiRealFFT.cpp,v 1.8 2006/09/05 10:27:54 ltilib Exp $
 */

#include "ltiRealFFT.h"
#include "ltiVector.h"
#include "ltiImage.h"
#include "ltiFFTinit.h"

#include <cstdio>

namespace lti {

  // ////////////////////////////////////////////////////////////
  //  RealFFT Parameters
  // ////////////////////////////////////////////////////////////

  realFFT::parameters::parameters()
    : transform::parameters(), mode(Cartesic)  {
  }

  realFFT::parameters& realFFT::parameters::copy(const parameters& other) {

#ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    transform::parameters::copy(other);
#else
    // ...so we have to use this workaround.
    // Conditional on that copy may not be virtual.
    transform::parameters& (transform::parameters::* p_copy)
      (const transform::parameters&) = transform::parameters::copy;
    (this->*p_copy)(other);
#endif
    mode = other.mode;
    return (*this);
  }

  functor::parameters* realFFT::parameters::clone() const  {
    realFFT::parameters* newParam = new realFFT::parameters;
    newParam->copy(*this);
    return newParam;
  }

  const char* realFFT::parameters::getTypeName() const {
    return "realFFT::parameters";
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool realFFT::parameters::write(ioHandler& handler,
                              const bool complete) const
# else
  bool realFFT::parameters::writeMS(ioHandler& handler,
                                const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      if (mode == Polar) {
        lti::write(handler,"mode","Polar");
      } else {
        lti::write(handler,"mode","Cartesic");
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::write(handler,false);
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
  bool realFFT::parameters::write(ioHandler& handler,
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
  bool realFFT::parameters::read(ioHandler& handler,
                                 const bool complete)
# else
  bool realFFT::parameters::readMS(ioHandler& handler,
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
      if (str == "Polar") {
        mode = Polar;
      } else {
        mode = Cartesic;
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::read(handler,false);
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
  bool realFFT::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------


  // ///////////////////////////////////////////////////////////////////
  //  realFFT
  // ///////////////////////////////////////////////////////////////////

  // returns the current parameters
  const realFFT::parameters& realFFT::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0) throw invalidParametersException(getTypeName());
    return *params;
  }

  realFFT::realFFT(void) {
    parameters defaultParam;
    setParameters(defaultParam);
  }

  realFFT::~realFFT(void) {
  }

  /// returns the name of this type
  const char* realFFT::getTypeName() const {
    return "realFFT";
  }

  /// returns a pointer to a clone of the functor.
  functor* realFFT::clone() const {
     return (new realFFT(*this));
  }

  /// apply real FFT to real vectors
  //  size of output is N/2+1 !
  void realFFT::apply(const vector<float>& realInput,
                            vector<float>& realOutput,
                            vector<float>& imagOutput) const{
    int originalSize, ldn;
    int *ip,  n, m, k;
    float  *w;
    lti::vector<float> help;

    // size must have a size with the size = 2^n
    originalSize = realInput.size();

    ldn = static_cast<int>(ceil(log(double(originalSize))/log(2.0)));
    n   = 1 << ldn;
    const int n2h = n/2;

    if (n!=originalSize) {
      help.resize(n, 0.0, false, true);
      help.fill(realInput, (n-originalSize)/2, n);
    } else {
      help.copy(realInput);
    }

    // initialize fft

    // ip: work area for bit reversal
    ip = fft::alloc_1d<int>(2 + (int) sqrt(n + 0.5));
    m = n * 5 / 4 + n / 4;

    // w: cos/sin table
    w = fft::alloc_1d<float>(m);
    ip[0] = 0; // flag to indicate initialization

    //resize output data
    realOutput.resize(1+n2h,0.0f,false,false);
    imagOutput.resize(1+n2h,0.0f,false,false);

    // FFT
    fft::vrdft(n, 1, help, ip, w); // FFT

    const float factor = 2.0f/n;
    // generate output data

    if(getParameters().mode == realFFT::parameters::Polar) {
      // POLAR
      realOutput[0] = help[0]*factor;
      imagOutput[0] = 0.0f;

      for(k=1; k<n2h; k++) {
        const int k2 = k << 1; // k*2
        const float& even = help[k2];
        const float& odd = help[k2+1];

        realOutput[k] = sqrt(even*even+odd*odd)*factor;
        imagOutput[k] = atan2(odd,even);
      }

      realOutput[n2h] = abs(help[1]*factor);
      imagOutput[n2h] = (help[1] < 0) ? static_cast<float>(Pi) : 0.0f;
    } else {
      // CARTESIC
      realOutput[0] = help[0]*factor;
      imagOutput[0] = 0.0f;

      for(k=1; k<n2h; k++){
        const int k2 = k << 1; // k*2
        realOutput[k] = help[k2]*factor;
	imagOutput[k] = help[k2+1]*factor;
      }

      realOutput[n2h] = help[1]*factor;
      imagOutput[n2h] = 0.0f;
    }

    // delete memory in the heap!

    fft::free_1d<float>(w);
    fft::free_1d<int>(ip);

  } //apply for vectors

  /// apply real FFT to real vectors
  //  size of output is N/2+1 !
  void realFFT::apply(const vector<double>& realInput,
                            vector<double>& realOutput,
                            vector<double>& imagOutput) const{

    int originalSize, ldn;
    int *ip,  n, m, k;
    double  *w;
    lti::vector<double> help;

    // size must have a size with the size = 2^n
    originalSize = realInput.size();

    ldn = static_cast<int>(ceil(log(double(originalSize))/log(2.0)));
    n   = 1 << ldn;
    const int n2h = n/2;

    if (n!=originalSize) {
      help.resize(n, 0.0, false, true);
      help.fill(realInput, (n-originalSize)/2, n);
    } else {
      help.copy(realInput);
    }

    // initialize fft

    // ip: work area for bit reversal
    ip = fft::alloc_1d<int>(2 + (int) sqrt(n + 0.5));
    m = n * 5 / 4 + n / 4;

    // w: cos/sin table
    w = fft::alloc_1d<double>(m);
    ip[0] = 0; // flag to indicate initialization

    //resize output data
    realOutput.resize(1+n2h,0.0f,false,false);
    imagOutput.resize(1+n2h,0.0f,false,false);

    // FFT
    fft::vrdft(n, 1, help, ip, w); // FFT

    const double factor = 2.0f/n;
    // generate output data

    if(getParameters().mode == realFFT::parameters::Polar) {
      // POLAR
      realOutput[0] = help[0]*factor;
      imagOutput[0] = 0.0f;

      for(k=1; k<n2h; k++) {
        const int k2 = k << 1; // k*2
        const double& even = help[k2];
        const double& odd = help[k2+1];

        realOutput[k] = sqrt(even*even+odd*odd)*factor;
        imagOutput[k] = atan2(odd,even);
      }

      realOutput[n2h] = abs(help[1]*factor);
      imagOutput[n2h] = (help[1]<0) ? Pi : 0.0f;
    } else {
      // CARTESIC
      realOutput[0] = help[0]*factor;
      imagOutput[0] = 0.0f;

      for(k=1; k<n2h; k++){
        const int k2 = k << 1; // k*2
        realOutput[k] = help[k2]*factor;
	imagOutput[k] = help[k2+1]*factor;
      }

      realOutput[n2h] = help[1]*factor;
      imagOutput[n2h] = 0.0f;
    }

    // delete memory in the heap!

    fft::free_1d<double>(w);
    fft::free_1d<int>(ip);

  } //apply for vectors


  /// apply real FFT to real matrix<float>s
  void realFFT::apply(const matrix<float> & realInput,
                            matrix<float> & realOutput,
		            matrix<float> & imagOutput) const{

    int originalXsize, originalYsize, ldn;
    int  k1, k2 ;

    // buffers needed by rdft2d
    int *ip, n1, n2, n;
    float *t, *w;
    lti::matrix<float> help;

    // size muss immer in der Form size = 2^n
    originalXsize = realInput.columns();
    originalYsize = realInput.rows();

    // n1 und n2 sind stets Zweierpotenzen
    // entweder die original-Bildgroesse oder die naechst hoehere Zweierpotenz

    ldn = static_cast<int>(ceil(log(double(originalYsize))/log(2.0)));
    n1   = 1 << ldn; // n = 2^ldn

    ldn = static_cast<int>(ceil(log(double(originalXsize))/log(2.0)));
    n2   = 1 << ldn; // n = 2^ldn

    if ((n1 != help.rows()) || (n2 != help.columns())) {
      help.resize(n1, n2, 0.0f, false, true);
      help.fill(realInput,
                (n1-originalYsize)/2, (n2-originalXsize)/2,
                n1, n2);
    } else {
      help.copy(realInput);
    }

    // initialize fft
    const int n2h = n2/2;
    const int n1h = n1/2;

    t = fft::alloc_1d<float>(2 * n1);

    n = lti::max(n1, n2h);
    ip = fft::alloc_1d<int>(2 + (int) sqrt(n + 0.5));
    n = lti::max(n1 * 5 / 4, n2 * 5 / 4) + n2 / 4;
    w = fft::alloc_1d<float>(n);
    ip[0] = 0;

    // resize output buffer
    realOutput.resize(n1, n2,0.0f,false,false);
    imagOutput.resize(n1, n2,0.0f,false,false);

    // FFT
    fft::rdft2d(n1, n2, 1, help, t, ip, w); // FFT

    const float factor = 2.0f/(n1*n2);

    // generate output matrices
    if(getParameters().mode == realFFT::parameters::Polar) {
      float im,re;

      for(k1=1; k1 < n1h; k1++) {
        const int mn1 = n1-k1;

        const vector<float>& h = help.getRow(k1);
        vector<float>& reOut = realOutput.getRow(k1);
        vector<float>& imOut = imagOutput.getRow(k1);
        vector<float>& creOut = realOutput.getRow(mn1);
        vector<float>& cimOut = imagOutput.getRow(mn1);

        for (k2 = 1; k2 < n2h; k2++) {
          const int k22 = (k2 << 1); // x2
          re = h[k22];
          im = h[k22+1];

          creOut[n2-k2] = reOut[k2] = factor*sqrt(re*re+im*im);
          cimOut[n2-k2] = -(imOut[k2] = atan2(im,re));
        }

        re = h[0];
        im = h[1];
        reOut[0]  = creOut[0] = factor*sqrt(re*re+im*im);
        cimOut[0] = -(imOut[0] = atan2(im,re));

        re = help[mn1][1];
        im = help[mn1][0];
        reOut[n2h] = creOut[n2h] = factor*sqrt(re*re+im*im);
        imOut[n2h] = -(cimOut[n2h] = atan2(re,im)); // !!!!!
      }

      for(; k1 < n1; k1++) {
        const int mn1 = n1-k1;

        const vector<float>& h = help.getRow(k1);
        vector<float>& reOut = realOutput.getRow(k1);
        vector<float>& imOut = imagOutput.getRow(k1);
        vector<float>& creOut = realOutput.getRow(mn1);
        vector<float>& cimOut = imagOutput.getRow(mn1);

        for(k2 = 1; k2 < n2h; k2++) {
          const int k22 = (k2 << 1); // x2

          re = h[k22];
          im = h[k22+1];

          creOut[n2-k2] = reOut[k2] = factor*sqrt(re*re+im*im);
          cimOut[n2-k2] = -(imOut[k2] = atan2(im,re));
        }
      }

      for(k2=1; k2 < n2h; k2++){
        const int k22 = (k2 << 1); // x2

        re = help[0][k22];
        im = help[0][k22+1];
        realOutput[0][k2] = realOutput[0][n2-k2] = factor*sqrt(re*re+im*im);
        imagOutput[0][n2-k2] = -(imagOutput[0][k2] = atan2(im,re));
      }

      realOutput[0][0] = abs(factor*help[0][0]);
      imagOutput[0][0] = (help[0][0]<0) ? static_cast<float>(Pi) : 0.0f;

      realOutput[0][n2h] = abs(factor*help[0][1]);
      imagOutput[0][n2h] = (help[0][1]<0) ? static_cast<float>(Pi) : 0.0f;

      realOutput[n1h][0] = abs(factor*help[n1h][0]);
      imagOutput[n1h][0] = (help[n1h][0]<0) ? static_cast<float>(Pi) : 0.0f;

      realOutput[n1h][n2h] = abs(factor*help[n1h][1]);
      imagOutput[n1h][n2h] = (help[n1h][1]<0) ? static_cast<float>(Pi) : 0.0f;

    } else { //Cartesic

      // the first rows
      for(k1=1; k1 < n1h; k1++){
        const int mn1 = n1-k1;
        const vector<float>& h = help.getRow(k1);
        vector<float>& reOut = realOutput.getRow(k1);
        vector<float>& imOut = imagOutput.getRow(k1);
        vector<float>& creOut = realOutput.getRow(mn1);
        vector<float>& cimOut = imagOutput.getRow(mn1);

        for(k2 = 1; k2 < n2h; k2++) {
          const int k22 = (k2 << 1); // x2

          creOut[n2-k2] = reOut[k2] = factor*h[k22];
          cimOut[n2-k2] = -(imOut[k2] = factor*h[k22+1]);
        }

        reOut[0]  = creOut[0] = factor*h[0];
        cimOut[0] = -(imOut[0] = factor*h[1]);
        reOut[n2h] = creOut[n2h] = factor*help[mn1][1];
        imOut[n2h] = -(cimOut[n2h] = factor*help[mn1][0]);
      }


      for(; k1 < n1; k1++) {
        const int mn1 = n1-k1;

        const vector<float>& h = help.getRow(k1);
        vector<float>& reOut = realOutput.getRow(k1);
        vector<float>& imOut = imagOutput.getRow(k1);
        vector<float>& creOut = realOutput.getRow(mn1);
        vector<float>& cimOut = imagOutput.getRow(mn1);

        for(k2 = 1; k2 < n2/2; k2++) {
          const int k22 = (k2 << 1); // x2

          creOut[n2-k2] = reOut[k2] = factor*h[k22];
          cimOut[n2-k2] = -(imOut[k2] = factor*h[k22+1]);

        }
      }

      for(k2=1; k2 < n2h; k2++){
        const int k22 = (k2 << 1); // x2

        realOutput[0][k2] = realOutput[0][n2-k2] = factor*help[0][k22];
        imagOutput[0][n2-k2] = -(imagOutput[0][k2] = factor*help[0][k22+1]);
      }

      realOutput[0][0] = factor*help[0][0];
      imagOutput[0][0] = 0;

      realOutput[0][n2h] = factor*help[0][1];
      imagOutput[0][n2h] = 0;

      realOutput[n1h][0] = factor*help[n1h][0];
      imagOutput[n1h][0] = 0;

      realOutput[n1h][n2h] = factor*help[n1h][1];
      imagOutput[n1h][n2h] = 0;

    } // end else if polar

    // Delete arrays

    fft::free_1d<float>(w);
    fft::free_1d<int>(ip);
    fft::free_1d<float>(t);

  } //apply for matrix<float>s



} // namespace lti
