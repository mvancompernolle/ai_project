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
 * file .......: ltiShiftInvariance.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 31.5.2001
 * revisions ..: $Id: ltiShiftInvariance.cpp,v 1.11 2006/09/05 10:31:00 ltilib Exp $
 */


#include "ltiShiftInvariance.h"
#include "ltiRealFFT.h"
#include "ltiRealInvFFT.h"
#include "ltiConvolution.h"
#include "ltiLinearKernels.h"

//#define _LTI_DEBUG

#ifdef _LTI_DEBUG
// ------> DEBUG
#include "ltiViewer.h"
#include <iostream>
// ------> DEBUG
#endif

namespace lti {
  // --------------------------------------------------
  // shiftInvariance::parameters
  // --------------------------------------------------

  // default constructor
  shiftInvariance::parameters::parameters()
    : globalFeatureExtractor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    mode = Momentum;
  }

  // copy constructor
  shiftInvariance::parameters::parameters(const parameters& other)
    : globalFeatureExtractor::parameters()  {
    copy(other);
  }

  // destructor
  shiftInvariance::parameters::~parameters() {
  }

  // get type name
  const char* shiftInvariance::parameters::getTypeName() const {
    return "shiftInvariance::parameters";
  }

  // copy member

  shiftInvariance::parameters&
    shiftInvariance::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    globalFeatureExtractor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    globalFeatureExtractor::parameters& (globalFeatureExtractor::parameters::* p_copy)
      (const globalFeatureExtractor::parameters&) =
      globalFeatureExtractor::parameters::copy;
    (this->*p_copy)(other);
# endif

    mode = other.mode;

    return *this;
  }

  // alias for copy member
  shiftInvariance::parameters&
    shiftInvariance::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* shiftInvariance::parameters::clone() const {
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
  bool shiftInvariance::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool shiftInvariance::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      switch(mode) {
        case Fourier:
          lti::write(handler,"mode","Fourier");
          break;
        default:
          lti::write(handler,"mode","Momentum");
          break;
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::write(handler,false);
# else
    bool (globalFeatureExtractor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      globalFeatureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool shiftInvariance::parameters::write(ioHandler& handler,
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
  bool shiftInvariance::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool shiftInvariance::parameters::readMS(ioHandler& handler,
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
      if (str == "Fourier") {
        mode = Fourier;
      } else {
        mode = Momentum;
      }

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::read(handler,false);
# else
    bool (globalFeatureExtractor::parameters::* p_readMS)(ioHandler&,const bool) =
      globalFeatureExtractor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool shiftInvariance::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // shiftInvariance
  // --------------------------------------------------

  // default constructor
  shiftInvariance::shiftInvariance()
    : globalFeatureExtractor(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  shiftInvariance::shiftInvariance(const shiftInvariance& other)
    : globalFeatureExtractor()  {
    copy(other);
  }

  // destructor
  shiftInvariance::~shiftInvariance() {
  }

  // returns the name of this type
  const char* shiftInvariance::getTypeName() const {
    return "shiftInvariance";
  }

  // copy member
  shiftInvariance&
    shiftInvariance::copy(const shiftInvariance& other) {
      globalFeatureExtractor::copy(other);
    return (*this);
  }

  // clone member
  functor* shiftInvariance::clone() const {
    return new shiftInvariance(*this);
  }

  // return parameters
  const shiftInvariance::parameters&
    shiftInvariance::getParameters() const {
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


  // On place apply for type dvector!
  bool shiftInvariance::apply(dvector& srcdest) const {

    dvector tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
    }

    return false;
  };

  // On copy apply for type dvector!
  bool shiftInvariance::apply(const dvector& src,dvector& dest) const {
    int shft;
    const parameters& param = getParameters();
    switch(param.mode) {
        case parameters::Fourier:
          return direct(src,dest,shft);
          break;
        default:
          return momentum(src,dest,shft);
          break;
    }
    return false;
  }

  // On copy apply for type dvector!
  bool shiftInvariance::apply(const dvector& src,
                                    dvector& dest,
                                    int& shft) const {
    const parameters& param = getParameters();
    switch(param.mode) {
        case parameters::Fourier:
          return direct(src,dest,shft);
          break;
        default:
          return momentum(src,dest,shft);
          break;
    }

    return false;
  }

  // On copy apply for type dvector!
  bool shiftInvariance::momentum(const dvector& src,
                                 dvector& dest,
                                 int& shft) const {
    double x,y,sina, cosa;
    int i;
    const int n(src.size());
    const double fac = 2.0*Pi / static_cast<double>(n);
    x = 0;
    y = 0;
    for (i=0;i<n;++i) {
      sincos(static_cast<double>(i)*fac, sina, cosa); 
      x+=cosa*src.at(i);
      y+=sina*src.at(i);
    }

    if ((x == 0) || (y==0)) {
      i = 0;
    } else {
      i = static_cast<int>(0.5+(atan2(y,x)*n)/(2*Pi));
    }

    shift(src,i,dest);

    return true;;
  }

  // On copy apply for type dvector!
  bool shiftInvariance::direct(const dvector& src,
                               dvector& dest,
                               int& shft) const {


    dvector mag,arg,argrec,darg,d2arg;

    // FFT the vector
    realFFT fft;
    realFFT::parameters fftParam;

    fftParam.mode = realFFT::parameters::Polar;
    fft.setParameters(fftParam);

    // Kernel for the first derivative
    kernel1D<double> fdKern(-1,1,0.0);
    fdKern.at(-1) = 0.5;
    fdKern.at(+1) = -0.5;

    // Kernel for the second derivative
    kernel1D<double> sdKern(-1,1,1.0);
    sdKern.at(0)=-2.0;

    convolution conv;
    convolution::parameters convPar;
    convPar.boundaryType =  lti::NoBoundary;

    fft.apply(src,mag,arg);
    fixArg(mag,arg);
    const int n2 = arg.lastIdx();

    // calculate first derivative
    convPar.setKernel(fdKern);
    conv.setParameters(convPar);

    conv.apply(arg,darg);

    // fix the border
    darg.at(0) = arg.at(1);
    darg.at(n2) = -arg.at(n2-1);

    // calculate the second derivatives
    convPar.setKernel(sdKern);
    conv.setParameters(convPar);

    conv.apply(arg,d2arg);

    // fix the boundaries
    d2arg.at(0) = -2*arg.at(0);
    d2arg.at(n2) = -2*arg.at(n2);

    int x;
    int n;

    double acc = 0;
    double sum = 0;
    double sum2 = 0;
    double dtmp;
    double avrg,stddev;
    double theMin,theMax;
    double cst;

    argrec.resize(arg.size(),0,false,false);

    // calculate a first approximation for the constant:
    for (x=0,n=0;x<darg.lastIdx();++x) {
      acc += d2arg.at(x);
      dtmp = darg.at(x) - acc;
      sum+=dtmp;
      sum2+=(dtmp*dtmp);
      argrec.at(x) = dtmp;
      n++;
    }

    argrec.at(x)=0;
    avrg = sum/n;
    stddev = sqrt((sum2/n)-avrg*avrg);

    theMin = avrg-0.5*stddev;
    theMax = avrg+0.5*stddev;

    // recalculate the avrg considering only the points similar to the
    // old average:
    for (x=0,n=0,sum=0;x<argrec.lastIdx();++x) {
      dtmp = argrec.at(x);
      if ((theMin <= dtmp) &&
          (dtmp <= theMax)) {
        sum += dtmp;
        n++;
      }
    }

    if (n == 0) {
      cst = avrg;
    } else {
      cst = sum/n;
    }

    // the shift value:
    shft = static_cast<int>((cst*x/Pi)+0.5);
    shift(src,shft,dest);

#   ifdef _LTI_DEBUG
    // ----->> DEBUG
    static viewer vdfder("int-der");
    vdfder.show(argrec);

    cout << "cst: " <<  cst << " avrg: " << avrg << " stddev: " << stddev
         << " shift: " << shft << endl;
    // ----->> DEBUG
#   endif

    return true;
  };

  // On copy apply for type dvector!
  bool shiftInvariance::arctan(const dvector& src,
                                     dvector& dest,
                                     int& shft) const {

    dvector mag,arg,argcos,argsin,dcos,dsin,d2cos,d2sin,argrec,darg,d2arg;
    dvector dargo,id2argo;

    // FFT the vector
    realFFT fft;
    realFFT::parameters fftParam;

    fftParam.mode = realFFT::parameters::Polar;
    fft.setParameters(fftParam);

    // Kernel for the first derivative
    kernel1D<double> fdKern(-1,1,0.0);
    fdKern.at(-1) = 0.5;
    fdKern.at(+1) = -0.5;

    // Kernel for the second derivative
    kernel1D<double> sdKern(-1,1,1.0);
    sdKern.at(0)=-2.0;

    convolution conv;
    convolution::parameters convPar;
    convPar.boundaryType =  lti::NoBoundary;

    // FFT of the source vector:
    fft.apply(src,mag,arg);

    // fix the arg to avoid discontinuities due to periodicity of the signal
    fixArg(mag,arg);

    double acc = 0;
    const int n2 = arg.lastIdx();

    // calculate the derivatives indirectly:
    argcos.copy(arg);
    argsin.copy(arg);
    argcos.apply(cos);
    argsin.apply(sin);

    // calculate first derivative
    convPar.setKernel(fdKern);
    conv.setParameters(convPar);

    conv.apply(argcos,dcos);
    conv.apply(argsin,dsin);

    conv.apply(arg,dargo); // also directly calculated to check some things

    // fix the border
    dcos.at(0) = argcos.at(1);
    dcos.at(n2) = -argcos.at(n2-1);

    dsin.at(0) = argsin.at(1);
    dsin.at(n2) = -argsin.at(n2-1);

    dargo.at(0) = arg.at(1);
    dargo.at(n2) = -arg.at(n2-1);

    // calculate the second derivatives:
    convPar.setKernel(sdKern);
    conv.setParameters(convPar);

    conv.apply(argcos,d2cos);
    conv.apply(argsin,d2sin);

    conv.apply(arg,id2argo);

    // fix the boundaries
    d2cos.at(0) = -2*argcos.at(0);
    d2cos.at(n2) = -2*argcos.at(n2);

    d2sin.at(0) = -2*argsin.at(0);
    d2sin.at(n2) = -2*argsin.at(n2);

    id2argo.at(0) = -2*arg.at(0);
    id2argo.at(n2) = -2*arg.at(n2);

    // now calculate the first and the integrated second derivative of
    // the argument
    argrec.resize(mag.size(),0.0,false,false);
    darg.resize(mag.size(),0.0,false,false);

    acc = 0;
    double sum = 0;
    double sum2 = 0;
    double dtmp;
    double acco = 0;
    double sumo = 0;
    double sum2o = 0;

    int x;
    for (x=0;x<darg.lastIdx();++x) {
      // first derivative
      darg.at(x) = ((dsin.at(x)*argcos.at(x))-(dcos.at(x)*argsin.at(x)));

      // acc == integral of the second derivative:
      acc += ((d2sin.at(x)*argcos.at(x))-(d2cos.at(x)*argsin.at(x)));

      acco += id2argo.at(x);

      // this will help to calculate the shift constant:
      dtmp = darg.at(x) - acc;

      argrec.at(x) = dtmp;
      sum += dtmp;
      sum2 += (dtmp*dtmp);

      dtmp = dargo.at(x)-acco;
      id2argo.at(x) = dtmp;

      sumo += dtmp;
      sum2o += (dtmp*dtmp);
    }

    // calculate the constant value (slope of the phase displacement
    // added as a result to the shift
    double avrg,stddev;
    double avrgo,stddevo;
    int n = darg.size()-1;
    int no = darg.size()-1;
    double cst,theMin,theMax;
    double csto,theMino,theMaxo;

    avrg = sum/n;
    stddev = sqrt((sum2o/n)-avrg*avrg);

    theMin = avrg-0.5*stddev;
    theMax = avrg+0.5*stddev;

    avrgo = sumo/no;
    stddevo = sqrt((sum2o/no)-avrgo*avrgo);

    theMino = avrgo-0.5*stddevo;
    theMaxo = avrgo+0.5*stddevo;

    // recalculate the avrg considering only the point similar to the
    // old average:
    for (x=0,n=0,no=0,sum=0,sumo=0;x<argrec.lastIdx();++x) {
      dtmp = argrec.at(x);
      if ((theMin <= dtmp) &&
          (dtmp <= theMax)) {
        sum += dtmp;
        n++;
      }

      dtmp = id2argo.at(x);
      if ((theMino <= dtmp) &&
          (dtmp <= theMaxo)) {
        sumo += dtmp;
        no++;
      }

    }

    if (n == 0) {
      cst = avrg;
    } else {
      cst = sum/n;
    }

    if (no == 0) {
      csto = avrgo;
    } else {
      csto = sumo/no;
    }

    // the question of the century: why is cst a sine function of the
    // displacement?
    if (cst>1.0) {
      cst=Pi/2;
    } else if (cst<-1.0) {
      cst=-Pi/2;
    } else {
      cst = asin(cst);
    }

    if (cst > 0) {
      if (abs(Pi-cst-csto) < abs(cst-csto)) {
        cst = Pi - cst;
      }
    } else {
      if (abs(-Pi-cst-csto) < abs(cst-csto)) {
        cst = -Pi - cst;
      }
    }

    // the shift factor
    shft = static_cast<int>(0.5+(cst*x/Pi));
    shift(src,shft,dest);

#ifdef _LTI_DEBUG
    // ----->> DEBUG
    static viewer vdarg("der-int");
    vdarg.show(argrec);

    cout << "cst: " <<  cst << " avrg: " << avrg << " stddev: " << stddev
         << " shift: " << shft << endl;
    // ----->> DEBUG
#endif

    return true;
  };


  double shiftInvariance::normAngle(const double& x) const {
    static const double Pi2 = 2.0*Pi;
    double frac = (x/Pi2);
    frac = (frac-int(frac))*Pi2;
    if (frac<0) {
      frac+=Pi2;
    }
    return frac;
  }

  /**
   * consider discontinuities in the magnitud to interpolate
   * missing phase elements
   */
  bool shiftInvariance::fixArg(const dvector& mag,dvector& arg) const {
    double maxMag = mag.maximum();
    double theMax = maxMag*1.0e-8;
    double theMin = -theMax;
    double tmp;
    int x,i,idx;
    bool needsFix = false;
    static const double Pi2 = 2.0*Pi;

    ivector indices(mag.size(),0);

    // find which intervals need interpolation
    for (x=1,i=1,idx=0;x<mag.lastIdx();++x) {
      tmp = mag.at(x);

      if ((theMin <= tmp) &&
          (tmp <= theMax)) {
        indices.at(idx)++;
        needsFix = true;
      } else {
        idx = x;
      }
    }

    if (!needsFix) {
      return true;
    }

    // interpolate
    double m;
    x=0;
    while (x<indices.size()) {
      if (indices.at(x) > 0) {
        idx = indices.at(x)+x+1;
        tmp=arg.at(x);



        m = (arg.at(idx)-tmp);

        // TODO: if the interval is too big, an some other conditions are
        //       met (?), this interval should be fixed,
        //       (this is a false interpolation):
        // Preliminary test:


        if (m>Pi) {
          m-=(Pi2);
        } else if (m<-Pi) {
          m+=Pi2;
        }

        m /= (idx-x);

        i=x+1;
        do {
          tmp+=m;
          arg.at(i) = tmp;
          ++i;
        } while (i<idx);
        x = i;
      }
      else {
        ++x;
      }
    }

    return true;
  }

  /**
   * cyclic shift of the given vector
   */
  bool shiftInvariance::shift(const dvector& in,
                              const int& shft,
                              dvector& out) const {

    out.resize(in.size(),0.0,false,false);
    int x,y;
    x=0;
    y=((-shft)%in.size());

    if (y<0) {
      y+=in.size();
    }

    for (x=0;x<in.size();++x,++y) {
      if (y>=out.size()) {
        y=0;
      }
      out.at(y)=in.at(x);
    }

    return true;
  }


}
