/*
 * Copyright (C) 2004, 2005, 2006
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


/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiHessianFunctor.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 12.3.2004
 * revisions ..: $Id: ltiHessianFunctor.cpp,v 1.9 2006/09/05 10:15:23 ltilib Exp $
 */

//TODO: include files
#include "ltiHessianFunctor.h"
#include "ltiHessianKernels.h"
#include "ltiSecondDerivativeKernels.h"
#include "ltiOgdKernels.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 4
#include "ltiDebug.h"


namespace lti {
  // --------------------------------------------------
  // hessianFunctor::parameters
  // --------------------------------------------------

  // default constructor
  hessianFunctor::parameters::parameters()
    : transform::parameters() {
    
    kernelSize = 3;
    kernelType = Hessian;
    kernelVariance = -1.f;
  }
  
  // copy constructor
  hessianFunctor::parameters::parameters(const parameters& other)
    : transform::parameters() {
    copy(other);
  }

  // destructor
  hessianFunctor::parameters::~parameters() {
  }

  // get type name
  const char* hessianFunctor::parameters::getTypeName() const {
    return "hessianFunctor::parameters";
  }

  // copy member

  hessianFunctor::parameters&
  hessianFunctor::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    transform::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    transform::parameters& (transform::parameters::* p_copy)
      (const transform::parameters&) =
      transform::parameters::copy;
    (this->*p_copy)(other);
# endif

    
    kernelType = other.kernelType;
    kernelSize = other.kernelSize;
    kernelVariance = other.kernelVariance;

    return *this;
  }

  // alias for copy member
  hessianFunctor::parameters&
  hessianFunctor::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* hessianFunctor::parameters::clone() const {
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
  bool hessianFunctor::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
    bool hessianFunctor::parameters::writeMS(ioHandler& handler,
                                             const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
          
      switch(kernelType) {
        case Hessian:
          lti::write(handler,"kernelType","Hessian");
          break;
        case Ando:
          lti::write(handler,"kernelType","Ando");
          break;
        case OGD2:
          lti::write(handler,"kernelType","OGD2");
          break;
        case Classic:
          lti::write(handler,"kernelType","Classic");
          break;
        case Sobel:
          lti::write(handler,"kernelType","Sobel");
          break;
        case Prewitt:
          lti::write(handler,"kernelType","Prewitt");
          break;
        case Robinson:
          lti::write(handler,"kernelType","Robinson");
          break;
        case Kirsch:
          lti::write(handler,"kernelType","Kirsch");
          break;
        case Harris:
          lti::write(handler,"kernelType","Harris");
          break;
        default:
          // unknown type, use the default
          lti::write(handler,"kernelType","Hessian");
          b = false;
          break;          
      }

      lti::write(handler,"kernelSize",kernelSize);
      lti::write(handler,"kernelVariance",kernelVariance);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::write(handler,false);
# else
    bool (transform::parameters::* p_writeMS)(ioHandler&,const bool) const =
      transform::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool hessianFunctor::parameters::write(ioHandler& handler,
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
   * @return true if read was successful
   */
# ifndef _LTI_MSC_6
  bool hessianFunctor::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
    bool hessianFunctor::parameters::readMS(ioHandler& handler,
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
      if (str == "Ando") {
        kernelType = Ando;
      } else if (str == "OGD") {
        kernelType = OGD2;
      } else if (str == "Classic") {
        kernelType = Classic;
      } else if (str == "Sobel") {        
        kernelType = Sobel;
      } else if (str == "Prewitt") {        
        kernelType = Prewitt;
      } else if (str == "Robinson") {        
        kernelType = Robinson;
      } else if (str == "Kirsch") {        
        kernelType = Kirsch;
      } else if (str == "Harris") {        
        kernelType = Harris;
      } else if (str == "Hessian") {        
        kernelType = Hessian;
      } else {
        kernelType = Hessian;
        handler.setStatusString("Unknown kernel type");
        b = false;
      }

      lti::read(handler,"kernelSize",kernelSize);
      lti::read(handler,"kernelVariance",kernelVariance);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::read(handler,false);
# else
    bool (transform::parameters::* p_readMS)(ioHandler&,const bool) =
      transform::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool hessianFunctor::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // hessianFunctor
  // --------------------------------------------------

  // default constructor
  hessianFunctor::hessianFunctor()
    : transform(), convXX(), convYY(), convXY() {

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  hessianFunctor::hessianFunctor(const parameters& par)
    : transform(), convXX(), convYY(), convXY() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  hessianFunctor::hessianFunctor(const hessianFunctor& other)
    : transform(), convXX(), convYY(), convXY() {
    copy(other);
  }

  // destructor
  hessianFunctor::~hessianFunctor() {
  }

  // returns the name of this type
  const char* hessianFunctor::getTypeName() const {
    return "hessianFunctor";
  }

  // copy member
  hessianFunctor&
  hessianFunctor::copy(const hessianFunctor& other) {
    transform::copy(other);

    convXX.copy(other.convXX);
    convYY.copy(other.convYY);
    convXY.copy(other.convXY);

    return (*this);
  }

  // alias for copy member
  hessianFunctor&
  hessianFunctor::operator=(const hessianFunctor& other) {
    return (copy(other));
  }


  // clone member
  functor* hessianFunctor::clone() const {
    return new hessianFunctor(*this);
  }

  // return parameters
  const hessianFunctor::parameters&
  hessianFunctor::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool hessianFunctor::updateParameters() {

    bool rc=true;
    const parameters& param=getParameters();

    convolution::parameters convPar;
    convPar.boundaryType = lti::Constant;

    switch (param.kernelType) {
      
      case parameters::Hessian:
        convPar.setKernel(hessianKernelXX());
        convXX.setParameters(convPar);
        convPar.setKernel(hessianKernelXY());
        convXY.setParameters(convPar);
        convPar.setKernel(hessianKernelYY());
        convYY.setParameters(convPar);
        break;
        
      case parameters::Ando: {
        int kernelSize;
        if (odd(param.kernelSize)) {
          kernelSize = min(9,max(3,param.kernelSize));
        } else {
          kernelSize = min(9,max(3,param.kernelSize-1));
        }
        convPar.setKernel(andoKernelXX<channel::value_type>
                          (kernelSize));
        convXX.setParameters(convPar);
        convPar.setKernel(andoKernelXY<channel::value_type>
                          (kernelSize));
        convXY.setParameters(convPar);
        convPar.setKernel(andoKernelYY<channel::value_type>
                          (kernelSize));
        convYY.setParameters(convPar);
        if (kernelSize!=param.kernelSize) {
          setStatusString("kernelSize ");
          appendStatusString(param.kernelSize);
          appendStatusString(" not supported for Ando kernel\n");
          rc = false;
        }
        break;
      }
        
      case parameters::OGD2: {
        ogd2Kernel<channel::value_type> ktmp;
        ktmp.generateBasisKernel(0, param.kernelSize,
                                 param.kernelVariance);
        convPar.setKernel(ktmp);
        _lti_debug4("ogd2kernelXX:\n" << ktmp.getRowFilter(0) 
                    << "\n" << ktmp.getColFilter(0));
        convXX.setParameters(convPar);
        ktmp.generateBasisKernel(2, param.kernelSize,
                                 param.kernelVariance);
        convPar.setKernel(ktmp);
        convXY.setParameters(convPar);
        ktmp.generateBasisKernel(1, param.kernelSize,
                                 param.kernelVariance);
        convPar.setKernel(ktmp);
        convYY.setParameters(convPar);
        break;
      }
        
      case parameters::Sobel:
        convPar.setKernel(sobelKernelXX<channel::value_type>());
        convXX.setParameters(convPar);
        convPar.setKernel(sobelKernelXY<channel::value_type>());
        convXY.setParameters(convPar);
        convPar.setKernel(sobelKernelYY<channel::value_type>());
        convYY.setParameters(convPar);
        break;
        
      case parameters::Prewitt:
        convPar.setKernel(prewittKernelXX<channel::value_type>());
        convXX.setParameters(convPar);
        convPar.setKernel(prewittKernelXY<channel::value_type>());
        convXY.setParameters(convPar);
        convPar.setKernel(prewittKernelYY<channel::value_type>());
        convYY.setParameters(convPar);
        break;
        
      case parameters::Robinson:
        convPar.setKernel(robinsonKernelXX<channel::value_type>());
        convXX.setParameters(convPar);
        convPar.setKernel(robinsonKernelXY<channel::value_type>());
        convXY.setParameters(convPar);
        convPar.setKernel(robinsonKernelYY<channel::value_type>());
        convYY.setParameters(convPar);
        break;
        
      case parameters::Kirsch:
        convPar.setKernel(kirschKernelXX<channel::value_type>());
        convXX.setParameters(convPar);
        convPar.setKernel(kirschKernelXY<channel::value_type>());
        convXY.setParameters(convPar);
        convPar.setKernel(kirschKernelYY<channel::value_type>());
        convYY.setParameters(convPar);
        break;
        
      case parameters::Harris:
        convPar.setKernel(harrisKernelXX<channel::value_type>());
        convXX.setParameters(convPar);
        convPar.setKernel(harrisKernelXY<channel::value_type>());
        convXY.setParameters(convPar);
        convPar.setKernel(harrisKernelYY<channel::value_type>());
        convYY.setParameters(convPar);
        break;
        
      case parameters::Classic:
        // no kernel needed: call special method
        break;
        
      default:
        // Hessian is the default
        convPar.setKernel(hessianKernelXX());
        convXX.setParameters(convPar);
        convPar.setKernel(hessianKernelXY());
        convXY.setParameters(convPar);
        convPar.setKernel(hessianKernelYY());
        convYY.setParameters(convPar);
        break;
        
    }
    return rc;
  }
  
  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool hessianFunctor::apply(const channel& src,
                             channel& xx, channel& xy, channel& yy) const {

    bool rc = true;

    const parameters& param = getParameters();
    
    switch (param.kernelType) {
      
      case parameters::Classic:
        //call specialized member functions
        return classicHessian(src,xx,xy,yy);
        break;

      case parameters::Hessian:
        //call spezialized member function for XY
        rc = rc && convXX.apply(src,xx);
        rc = rc && classicXY(src,xy);
        rc = rc && convYY.apply(src,yy);
        break;

      default:
        // not nice but faster than putting all possibilities
        // after all this has been checked in setParameters()
        rc = rc && convXX.apply(src,xx);
        rc = rc && convXY.apply(src,xy);
        rc = rc && convYY.apply(src,yy);
        break;
    }

    if (!rc) {
      xx.clear();
      xy.clear();
      yy.clear();
      appendStatusString(convXX);
      appendStatusString(convXY);
      appendStatusString(convYY);
    }

    return rc;

  };


  bool hessianFunctor::apply(const channel8& src,
                             channel& xx, channel& xy, channel& yy) const {

    channel tmp;
    tmp.castFrom(src);
    return apply(tmp,xx,xy,yy);
  };


  // -------------------------------------------------------------------
  // helper methods
  // -------------------------------------------------------------------

  bool hessianFunctor::classicHessian(const channel& src,
                                            channel& xx,    
                                            channel& xy,     
                                            channel& yy) const {

    if (src.columns() < 3) {
      setStatusString("width less than 3");
      xx.clear();
      xy.clear();
      yy.clear();
      return false;
    }
    if (src.rows() < 3) {
      setStatusString("height less than 3");
      xx.clear();
      xy.clear();
      yy.clear();
      return false;
    }

    if (src.getMode()!=channel::Connected) {
      setStatusString("src must be Connected");
      xx.clear();
      xy.clear();
      yy.clear();
      return false;
    }

    const int width  = src.columns();
    const int height = src.rows();

    xx.resize(height,width,0.f,false,false);
    xy.resize(height,width,0.f,false,false);
    yy.resize(height,width,0.f,false,false);
    
    float* fpxx = &xx.at(0,0);
    float* fpxy = &xy.at(0,0);
    float* fpyy = &yy.at(0,0);

    const float* fpSrc = &src.at(0,0);
    const float* rowy;
    const float* colx;

    float* pidxx;
    float* pidxy;
    float* pidyy;

    const int w1 = width-1;
    const int w2 = width-2;
    const int last = (height-1)*width; // index of begin of last row
    const int lastRow = -w1;           // offset from actual column pointer to
                                       // last row
    const int nextRow = width+1;       // offset from actual column pointer to
                                       // next row
    const int nextRow2 = width+2;      // offset from actual column pointer to
                                       // next row + 1

    // top-left corner
    fpxx[0]=(fpSrc[1]-fpSrc[0]);
    fpxy[0]=(fpSrc[0]-fpSrc[1]-fpSrc[width]+fpSrc[nextRow]);
    fpyy[0]=(fpSrc[width]-fpSrc[0]);

    // top
    pidxx = &fpxx[1];
    pidxy = &fpxy[1];
    pidyy = &fpyy[1];

    for (colx=&fpSrc[0],rowy=&fpSrc[w1];
         colx<rowy;
         ++colx,++pidxx,++pidxy,++pidyy) {
      *pidxx=(*colx - 2.f*colx[1] + colx[2]);
      *pidxy=(*colx - colx[2] - colx[width] + colx[nextRow2]);
      *pidyy=(colx[nextRow] - colx[1]);
    }

    // top-right corner
    fpxx[w1]=(fpSrc[w2]-fpSrc[w1]);
    fpxy[w1]=(fpSrc[w2]-fpSrc[w1]-fpSrc[w2+width]+fpSrc[w1+width]);
    fpyy[w1]=(fpSrc[w1]-fpSrc[w1+width]);

    // main loop (begin at coordinates (1,0)
    pidxx = &fpxx[width];
    pidxy = &fpxy[width];
    pidyy = &fpyy[width];

    const float *const rowEnd = &fpSrc[last];

    for (rowy=&fpSrc[width];
         rowy<rowEnd;
         rowy+=width) {

      // left side
      *pidxx=(rowy[1] - rowy[0]);
      *pidxy=(rowy[-width] - rowy[lastRow] - rowy[width] + rowy[nextRow]);
      *pidyy=(rowy[width] - 2.f*(*rowy) + rowy[-width]);

      ++pidxx;
      ++pidxy;
      ++pidyy;

      // middle
      const float *const colEnd = &rowy[w2];
      for (colx=rowy;
           colx<colEnd;
           ++colx,++pidxx,++pidxy,++pidyy) {
        *pidxx=(*colx - 2*colx[1] + colx[2]);
        *pidxy=(colx[-width] - colx[-w2] - colx[width] + colx[nextRow2]);
        *pidyy=(colx[lastRow] - 2.f*colx[1] + colx[nextRow]);
      }

      // right side
      *pidxx=(*colx - colx[1]);
      *pidxy=(colx[-width] - colx[lastRow] - colx[width] + colx[nextRow]);
      *pidyy=(colx[lastRow] - 2.f*colx[1] - colx[nextRow]);

      ++pidxx;
      ++pidxy;
      ++pidyy;
    }

    // bottom-left corner
    fpxx[last]=(fpSrc[last+1]-fpSrc[last]);
    fpxy[last]=(fpSrc[last+1]-fpSrc[last]);
    fpyy[last]=(fpSrc[last-width]-fpSrc[last-w1]-fpSrc[last]+fpSrc[last+1]);

    // bottom
    pidxx = &fpxx[last+1];
    pidxy = &fpxy[last+1];
    pidyy = &fpyy[last+1];

    const float *const colEnd = &rowEnd[w2];
    for (colx=rowEnd;
         colx<colEnd;
         ++colx,++pidxx,++pidxy,++pidyy) {
      *pidxx=(*colx - 2.f*colx[1] + colx[2]);
      *pidxy=(colx[-width] - colx[-w2] - *colx + colx[2]);
      *pidyy=(colx[lastRow] - colx[1]);
    }

    // bottom-right corner
    fpxx[last+w1]=(fpSrc[last+w2] - fpSrc[last+w1]);
    fpxy[last+w1]=(fpSrc[last-2]-fpSrc[last-1]-fpSrc[last+w2]+fpSrc[last+w1]);
    fpyy[last+w1]=(fpSrc[last-1] - fpSrc[last+w1]);

    return true;
  };

  bool hessianFunctor::classicXY(const channel& src, channel& xy) const {

    if (src.columns() < 3) {
      setStatusString("width less than 3");
      xy.clear();
      return false;
    }
    if (src.rows() < 3) {
      setStatusString("height less than 3");
      xy.clear();
      return false;
    }

    if (src.getMode()!=channel::Connected) {
      setStatusString("src must be Connected");
      xy.clear();
      return false;
    }

    const int width  = src.columns();
    const int height = src.rows();

    xy.resize(height,width,0.f,false,false);
    
    float* fpxy = &xy.at(0,0);

    const float* fpSrc = &src.at(0,0);
    const float* rowy;
    const float* colx;

    float* pidxy;

    const int w1 = width-1;
    const int w2 = width-2;
    const int last = (height-1)*width; // index of begin of last row
    const int lastRow = -w1;           // offset from actual column pointer to
                                       // last row
    const int nextRow = width+1;       // offset from actual column pointer to
                                       // next row
    const int nextRow2 = width+2;      // offset from actual column pointer to
                                       // next row + 1

    // top-left corner
    fpxy[0]=(fpSrc[0]-fpSrc[1]-fpSrc[width]+fpSrc[nextRow]);

    // top
    pidxy = &fpxy[1];

    for (colx=&fpSrc[0],rowy=&fpSrc[w1];
         colx<rowy;
         ++colx,++pidxy) {
      *pidxy=(*colx - colx[2] - colx[width] + colx[nextRow2]);
    }

    // top-right corner
    fpxy[w1]=(fpSrc[w2]-fpSrc[w1]-fpSrc[w2+width]+fpSrc[w1+width]);

    // main loop (begin at coordinates (1,0)
    pidxy = &fpxy[width];

    const float *const rowEnd = &fpSrc[last];

    for (rowy=&fpSrc[width];
         rowy<rowEnd;
         rowy+=width) {

      // left side
      *pidxy=(rowy[-width] - rowy[lastRow] - rowy[width] + rowy[nextRow]);

      ++pidxy;

      // middle
      const float *const colEnd = &rowy[w2];
      for (colx=rowy;
           colx<colEnd;
           ++colx,++pidxy) {
        *pidxy=(colx[-width] - colx[-w2] - colx[width] + colx[nextRow2]);
      }

      // right side
      *pidxy=(colx[-width] - colx[lastRow] - colx[width] + colx[nextRow]);

      ++pidxy;
    }

    // bottom-left corner
    fpxy[last]=(fpSrc[last+1]-fpSrc[last]);

    // bottom
    pidxy = &fpxy[last+1];

    const float *const colEnd = &rowEnd[w2];
    for (colx=rowEnd;
         colx<colEnd;
         ++colx,++pidxy) {
      *pidxy=(colx[-width] - colx[-w2] - *colx + colx[2]);
    }

    // bottom-right corner
    fpxy[last+w1]=(fpSrc[last-2]-fpSrc[last-1]-fpSrc[last+w2]+fpSrc[last+w1]);

    return true;
  };

}

#include "ltiUndebug.h"
