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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiRealInvFFT.cpp
 * authors ....: Stefan Syberichs
 * organization: LTI, RWTH Aachen
 * comment.....: inverse FFT routines based on modified code by Takuya Ooura
 *               with kind permission ooura@kurims.kyoto-u.ac.jp or
 *               ooura@mmm.t.u-tokyo.ac.jp)
 * creation ...: 28.01.2000
 * revisions ..: $Id: ltiRealInvFFT.cpp,v 1.5 2006/02/08 11:44:00 ltilib Exp $
 */

#include "ltiRealInvFFT.h"
#include "ltiVector.h"
#include "ltiImage.h"
#include "ltiFFTinit.h"
#include "ltiException.h"

#include <cstdio>

namespace lti {

  /////////////////////////////////////////////////////////////////////
  // realInvFFT
  /////////////////////////////////////////////////////////////////////

  // returns the current parameters
  const realInvFFT::parameters& realInvFFT::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if (params==0) throw invalidParametersException(getTypeName());
    return *params;
  }

  realInvFFT::realInvFFT(void) {
    parameters defaultParam;
    setParameters(defaultParam);
  }

  realInvFFT::~realInvFFT(void) {
  }

  /// returns the name of this type
  const char* realInvFFT::getTypeName() const {
    return "realInvFFT";
  }

  /// returns a pointer to a clone of the functor.
  functor* realInvFFT::clone() const {
     return (new realInvFFT(*this));
  }

  /// apply Real Inv FFT to real vectors
  //  size of output is N !
  void realInvFFT::apply(const vector<float>& realInput,
		         const vector<float>& imagInput,
		               vector<float>& realOutput) const{

    const parameters& param = getParameters();

    int ldn, originalSize;
    int  k ;
    lti::vector<float> help;

    // buffers needed by rdft2d
    int *ip,  n, m;
    float  *w;

    if(realInput.size() != imagInput.size() ) {
      throw exception("realInvFFT::Input vector sizes do not match !");
    }

    // size must follow size = 2^n+1
    originalSize = realInput.size();

    // input data is just one half of the original spectrum
    // the original data are twice the size

    // n is always a power of 2
    ldn = static_cast<int>(ceil(log(double(originalSize-1))/log(2.0)));
    n = 1 << ldn; // n = 2^ldn

    // vector size 2x in space-domain
    realOutput.resize(n*2, 0.0, false, true);

    // if input data polar:
    if(param.mode == realInvFFT::parameters::Polar) {

      realOutput[0] = realInput[0]*cos(imagInput[0]);
      
      float sina, cosa;

      for (k =1; k < n; k++){
        const int k2 = 2*k;
        sincos(imagInput[k], sina, cosa);
        realOutput[k2]   = realInput[k]*cosa;
	realOutput[k2+1] = realInput[k]*sina;
      }

      realOutput[1] = realInput[n]*cos(imagInput[n]);

    } else { // Cartesic
      realOutput[0] = realInput[0];

      for (k =1; k < n; k++){
        const int k2 = k << 1; // 2*k
        realOutput[k2] = realInput[k];
	realOutput[k2+1] = imagInput[k];
      }

      realOutput[1] = realInput[n];
    }

    // initialize fft
    ip = fft::alloc_1d<int>(2 + (int) sqrt(2*n + 0.5));
    m = 2*n * 5 / 4 + (2*n) / 4;
    w = fft::alloc_1d<float>(m);
    ip[0] = 0;

    // inverse FFT
    fft::vrdft(2*n, -1, realOutput,  ip, w);

    // delete arrays
    fft::free_1d<float>(w);
    fft::free_1d<int>(ip);

  } //apply for vectors

  /// apply Real FFT to real vectors
  //  size of output is N/2+1 !
  void realInvFFT::apply(const vector<double>& realInput,
		         const vector<double>& imagInput,
		               vector<double>& realOutput) const{

    const parameters& param = getParameters();

    int ldn, originalSize;
    int  k ;
    lti::vector<double> help;

    // buffers needed by rdft2d
    int *ip,  n, m;
    double  *w;

    if(realInput.size() != imagInput.size() ) {
      throw exception("realInvFFT::Input vector sizes do not match !");
    }

    // size must follow size = 2^n
    originalSize = realInput.size();

    // input data is just one half of the original spectrum
    // the original data are twice the size

    // n is always a power of 2
    ldn = static_cast<int>(ceil(log(double(originalSize-1))/log(2.0)));
    n = 1 << ldn; // n = 2^ldn

    // vector size 2x in space-domain
    realOutput.resize(n*2, 0.0, false, true);

    // if input data polar:
    if(param.mode == realInvFFT::parameters::Polar) {

      realOutput[0] = realInput[0]*cos(imagInput[0]);
      
      double sina, cosa;

      for (k =1; k < n; k++){
        const int k2 = 2*k;
        sincos(realInput[k], sina, cosa);
        realOutput[k2]   = realInput[k]*cosa;
	realOutput[k2+1] = realInput[k]*sina;
      }

      realOutput[1] = realInput[n]*cos(imagInput[n]);

    } else { // Cartesic
      realOutput[0] = realInput[0];

      for (k =1; k < n; k++){
        const int k2 = 2*k;
        realOutput[k2] = realInput[k];
	realOutput[k2+1] = imagInput[k];
      }

      realOutput[1] = realInput[n];
    }

    // initialize fft
    ip = fft::alloc_1d<int>(2 + (int) sqrt(2*n + 0.5));
    m = 2*n * 5 / 4 + (2*n) / 4;
    w = fft::alloc_1d<double>(m);
    ip[0] = 0;

    // inverse FFT
    fft::vrdft(2*n, -1, realOutput,  ip, w);

    // delete arrays
    fft::free_1d<double>(w);
    fft::free_1d<int>(ip);

  } //apply for vectors


  /// apply Real FFT to real matrix<float>s
  void realInvFFT::apply(const matrix<float> & realInput,
                         const matrix<float> & imagInput,
		               matrix<float> & realOutput) const{

    int originalXsize, originalYsize, ldn;

    // buffers needed by rdft2d
    int *ip, n1, n2, n, j1, j2;
    float *t, *w;
    lti::matrix<float> help;

    if (realInput.rows() != imagInput.rows() ||
        realInput.columns() != imagInput.columns()) {
      throw exception("realInvFFT::Input matrix<float>s sizes do not match !");
    }

    // size must be always be 2^n
    originalXsize = realInput.columns();
    originalYsize = realInput.rows();

    // n1 und n2 are always powers of 2
    ldn = static_cast<int>(ceil(log(double(originalYsize))/log(2.0)));
    n1   = 1 << ldn; // n = 2^ldn

    ldn = static_cast<int>(ceil(log(double(originalXsize))/log(2.0)));
    n2   = 1 << ldn; // n = 2^ldn

    realOutput.resize(n1, n2, 0.0, false, true);

    const int n1h = n1/2;
    const int n2h = n2/2;

    // for polar input data...
    if (getParameters().mode == realInvFFT::parameters::Polar) {
      
      float sina, cosa;
      
      for(j1=1; j1<n1h; j1++){
        const vector<float>& reIn = realInput.getRow(j1);
        const vector<float>& imIn = imagInput.getRow(j1);
        vector<float>& out = realOutput[j1];
        
        for(j2=1; j2<n2h;j2++){
          const int j22 = j2 << 1; // x2
          sincos(imIn[j2], sina, cosa);
          out[j22]   = reIn[j2]*cosa;
          out[j22+1] = reIn[j2]*sina;
        }

        sincos(imIn[0], sina, cosa);
        out[0] = reIn[0]*cosa;
        out[1] = reIn[0]*sina;
        sincos(-imIn[n2h], sina, cosa);
        realOutput[n1-j1][1] = reIn[n2h]*cosa;
        realOutput[n1-j1][0] = reIn[n2h]*sina;
      }
      
      for(; j1<n1; j1++){
        const vector<float>& reIn = realInput.getRow(j1);
        const vector<float>& imIn = imagInput.getRow(j1);
        vector<float>& out = realOutput[j1];
        for(j2=1; j2<n2h;j2++){
          const int j22 = j2 << 1; // x2
          sincos(imIn[j2], sina, cosa);
          out[j22]   = reIn[j2]*cosa;
          out[j22+1] = reIn[j2]*sina;
        }
      }
      
      for(j2=1; j2<n2h;j2++){
        const int j22 = j2 << 1; // x2
        sincos(imagInput[0][j2], sina, cosa);
        realOutput[0][j22]   = realInput[0][j2]*cosa;
        realOutput[0][j22+1] = realInput[0][j2]*sina;
        // -I[0][n2-j2],
      }

      realOutput[0][0] = realInput[0][0]*cos(imagInput[0][0]);
      realOutput[0][1] = realInput[0][n2h]*cos(imagInput[0][n2h]);
      realOutput[n1h][0] = realInput[n1h][0]*cos(imagInput[n1h][0]);
      realOutput[n1h][1] = realInput[n1h][n2h]*cos(imagInput[n1h][0]);

    } else { //Cartesic

      for(j1=1; j1<n1h; j1++){
        const vector<float>& reIn = realInput.getRow(j1);
        const vector<float>& imIn = imagInput.getRow(j1);
        vector<float>& out = realOutput[j1];

        for(j2=1; j2<n2h;j2++){
          const int j22 = j2 << 1; // x2
          out[j22] = reIn[j2];// = R[n1-j1][n2-j2],
          out[j22+1] = imIn[j2];// = -I[n1-j1][n2-j2],
        }

        out[0] = reIn[0];// = realInput[n1-j1][0],
        out[1] = imIn[0];// = -imagInput[n1-j1][0],
        realOutput[n1-j1][1] = reIn[n2h];//=realInput[n1-j1][n2h],
        realOutput[n1-j1][0] = -imIn[n2h];//=imagInput[n1-j1][n2h],
      }

      for(; j1<n1; j1++){
        const vector<float>& reIn = realInput.getRow(j1);
        const vector<float>& imIn = imagInput.getRow(j1);
        vector<float>& out = realOutput[j1];
        for(j2=1; j2<n2h;j2++){
          const int j22 = j2 << 1; // x2
          out[j22] = reIn[j2];// = R[n1-j1][n2-j2],
          out[j22+1] = imIn[j2];// = -I[n1-j1][n2-j2],
        }
      }

      for(j2=1; j2<n2h;j2++){
        const int j22 = j2 << 1; // x2
        realOutput[0][j22] = realInput[0][j2];// = R[0][n2-j2],
        realOutput[0][j22+1] = imagInput[0][j2];// = -I[0][n2-j2],
      }

      realOutput[0][0] = realInput[0][0];
      realOutput[0][1] = realInput[0][n2h];
      realOutput[n1h][0] = realInput[n1h][0];
      realOutput[n1h][1] = realInput[n1h][n2h];
    }

    ////////////////////////////////////////////////////////////////////
    // initialize fft

    t = fft::alloc_1d<float>(2 * n1);
    n = max(n1, n2 / 2);
    ip = fft::alloc_1d<int>(2 + (int) sqrt(n + 0.5));
    n = max(n1 * 5 / 4, n2 * 5 / 4) + n2 / 4;
    w = fft::alloc_1d<float>(n);
    ip[0] = 0; // flag to indicate "initialization required!"

    // inverse FFT
    fft::rdft2d(n1, n2, -1, realOutput, t, ip, w);

    // delete arrays
    fft::free_1d<float>(w);
    fft::free_1d<int>(ip);
    fft::free_1d<float>(t);
  } //apply for matrix<float>s
} // namespace lti
