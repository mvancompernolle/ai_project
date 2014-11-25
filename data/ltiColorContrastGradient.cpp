/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiColorContrastGradient.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 1.6.2003
 * revisions ..: $Id: ltiColorContrastGradient.cpp,v 1.19 2006/09/05 10:06:07 ltilib Exp $
 */

#include "ltiColorContrastGradient.h"
#include "ltiSplitImageToRGB.h"
#include "ltiArctanLUT.h"
#include "ltiGaussKernels.h"
#include "ltiGradientKernels.h"
#include "ltiConvolution.h"
#include "ltiOgdFilter.h"
#include  <limits>

namespace lti {
  // --------------------------------------------------
  // colorContrastGradient::parameters
  // --------------------------------------------------

  // default constructor
  colorContrastGradient::parameters::parameters()
    : gradientFunctor::parameters() {
    contrastFormat = Contrast;
  }

  // copy constructor
  colorContrastGradient::parameters::parameters(const parameters& other)
    : gradientFunctor::parameters() {
    copy(other);
  }

  // destructor
  colorContrastGradient::parameters::~parameters() {
  }

  // get type name
  const char* colorContrastGradient::parameters::getTypeName() const {
    return "colorContrastGradient::parameters";
  }

  // copy member

  colorContrastGradient::parameters&
    colorContrastGradient::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    gradientFunctor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    gradientFunctor::parameters& (gradientFunctor::parameters::* p_copy)
      (const gradientFunctor::parameters&) =
      gradientFunctor::parameters::copy;
    (this->*p_copy)(other);
# endif

    contrastFormat = other.contrastFormat;

    return *this;
  }

  // alias for copy member
  colorContrastGradient::parameters&
    colorContrastGradient::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* colorContrastGradient::parameters::clone() const {
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
  bool colorContrastGradient::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool colorContrastGradient::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      switch(contrastFormat){
        case MDD:
          lti::write(handler,"contrastFormat","MDD");
          break;
        case Contrast:
          lti::write(handler,"contrastFormat","Contrast");
          break;
        case Maximum:
          lti::write(handler,"contrastFormat","Maximum");
          break;
        default:
          lti::write(handler,"contrastFormat","Contrast");
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && gradientFunctor::parameters::write(handler,false);
# else
    bool (gradientFunctor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      gradientFunctor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool colorContrastGradient::parameters::write(ioHandler& handler,
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
  bool colorContrastGradient::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool colorContrastGradient::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string str;
      lti::read(handler,"contrastFormat",str);
      if (str == "MDD") {
        contrastFormat = MDD;
      } else if (str == "Contrast") {
        contrastFormat = Contrast;
      } else if (str == "Maximum") {
        contrastFormat = Maximum;
      } else {
        contrastFormat = Contrast;
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && gradientFunctor::parameters::read(handler,false);
# else
    bool (gradientFunctor::parameters::* p_readMS)(ioHandler&,const bool) =
      gradientFunctor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool colorContrastGradient::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // colorContrastGradient
  // --------------------------------------------------

  // default constructor
  colorContrastGradient::colorContrastGradient() 
    : gradientFunctor(false) {
    parameters defParam;
    setParameters(defParam);
  }

  // default constructor
  colorContrastGradient::colorContrastGradient(const parameters::eOutputFormat& f,
                                               const int gradKernelSize)
    : gradientFunctor(false) {

    parameters defaultParameters;

    defaultParameters.format=f;
    defaultParameters.gradientKernelSize = gradKernelSize;

    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  colorContrastGradient::colorContrastGradient(const parameters& par)
    : gradientFunctor(par) {
  }


  // copy constructor
  colorContrastGradient::colorContrastGradient(const colorContrastGradient& other)
    : gradientFunctor() {
    copy(other);
  }

  // destructor
  colorContrastGradient::~colorContrastGradient() {
  }

  // returns the name of this type
  const char* colorContrastGradient::getTypeName() const {
    return "colorContrastGradient";
  }

  // copy member
  colorContrastGradient&
  colorContrastGradient::copy(const colorContrastGradient& other) {
    gradientFunctor::copy(other);

    return (*this);
  }

  // alias for copy member
  colorContrastGradient&
  colorContrastGradient::operator=(const colorContrastGradient& other) {
    return (copy(other));
  }


  // clone member
  functor* colorContrastGradient::clone() const {
    return new colorContrastGradient(*this);
  }

  // return parameters
  const colorContrastGradient::parameters&
  colorContrastGradient::getParameters() const {
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
  
  // On copy apply for type image!
  bool colorContrastGradient::apply(const image& src,
                                    channel& xOrMag,
                                    channel& yOrArg) const {
    splitImageToRGB splitter;
    channel c1,c2,c3;

    splitter.apply(src,c1,c2,c3);
    float maxmag;
    return apply(c1,c2,c3,xOrMag,yOrArg,maxmag);
  };

  bool colorContrastGradient::apply(const channel& c1,
                                    const channel& c2,
                                    const channel& c3,
                                    channel& xOrMag,
                                    channel& yOrArg) const {
    float maxmag;
    return apply(c1,c2,c3,xOrMag,yOrArg,maxmag);
  }

  bool colorContrastGradient::apply(const channel& c1,
                                    const channel& c2,
                                    const channel& c3,
                                    channel& xOrMag,
                                    channel& yOrArg,
                                    float& maxmag) const {

    if (getParameters().format == parameters::Polar) {
      return computeGradientPolar(c1,c2,c3,xOrMag,yOrArg,maxmag);
    } else {
      return computeColorGradientCart(c1,c2,c3,xOrMag,yOrArg,maxmag);      
    }
  }


  bool colorContrastGradient::apply(const channel& c1,
                                    const channel& c2,
                                    channel& xOrMag,
                                    channel& yOrArg,
                                    float& maxmag) const {

    if (getParameters().format == parameters::Polar) {
      return computeGradientPolar(c1,c2,xOrMag,yOrArg,maxmag);
    } else {
      return computeColorGradientCart(c1,c2,xOrMag,yOrArg,maxmag);      
    }
  }

  bool colorContrastGradient::apply(const channel& c1,
                                    channel& xOrMag,
                                    channel& yOrArg) const {
    return gradientFunctor::apply(c1,xOrMag,yOrArg);
  }

  bool colorContrastGradient::apply(const channel8& c1,
                                    channel& xOrMag,
                                    channel& yOrArg) const {
    return gradientFunctor::apply(c1,xOrMag,yOrArg);
  }

  bool colorContrastGradient::apply(const channel& c1,
                                    const channel& c2,
                                    const channel& c3,
                                    channel& lambdaMin,
                                    channel& lambdaMax,
                                    channel& orientation,
                                    float& maxmag) const {

    return computeGradientPolar(c1,c2,c3,lambdaMin,lambdaMax,orientation,
                                maxmag);
  }
    
  inline float 
  colorContrastGradient::magSqr(const float a,const float b) const {
    return (a*a+b*b);
  }

  /*
   * differentiate the image in X and Y directions
   * @param c1 first channel
   * @param c2 second channel
   * @param c3 third channel
   * @param gradAbs the absolute values of the gradient
   * @param orientation the orientation map of the gradient image
   * @return true if successful, false otherwise.
   */
  bool colorContrastGradient::computeGradientPolar(const channel& c1,
                                                   const channel& c2,
                                                   const channel& c3,
                                                   channel& gradAbs,
                                                   channel& orientation,
                                                   float& maxVal) const {
    
    // compute the gradients
    channel dx1,dx2,dx3,dy1,dy2,dy3;

    // compute the partial derivatives of each channel respect to x and y
    computeGradientCart(c1,dx1,dy1);
    computeGradientCart(c2,dx2,dy2);
    computeGradientCart(c3,dx3,dy3);

    // resize the result
    gradAbs.resize(c1.size(),float(),false,false);
    orientation.resize(c1.size(),float(),false,false);

    int x,y;
    float E,F,G,EpG,EmG,a;
    trgbPixel<float> dfdx(0,0,0),dfdy(0,0,0);
    maxVal=0.0f;
    
    switch (getParameters().contrastFormat) {
      case parameters::Contrast: {
        // Determine the direction and amplitude of highest contrast.
        
        for (y=0;y<c1.rows();++y) {
          for (x=0;x<c1.columns();++x) {
            // get both derivative vectors
            dfdx.set(dx1.at(y,x),dx2.at(y,x),dx3.at(y,x));
            dfdy.set(dy1.at(y,x),dy2.at(y,x),dy3.at(y,x));
            
            // components of the tensor
            E=dfdx.dot(dfdx);
            F=dfdx.dot(dfdy);
            G=dfdy.dot(dfdy);
            
            // the vector of maximal contrast is given by the eigenvalues of
            // the tensor matrix lambda = (E+G) +/- sqrt((E-G)^2+4F^2)/2;
            EpG=E+G;
            if (EpG<=std::numeric_limits<float>::epsilon()) {
              gradAbs.at(y,x)=0.0f;
              orientation.at(y,x)=0.0f;
            } else {
              EmG=E-G;
              // best contrast obtained with lambda_max - lambda_min
              maxVal=max((gradAbs.at(y,x)=pow(EmG*EmG+4.0f*F*F,0.25f)),maxVal);
              a = 0.5f*atan2(2.0f*F,EmG);
              orientation.at(y,x)=a;
            }
          }
        }
      } break;
      case parameters::MDD: {
        for (y=0;y<c1.rows();++y) {
          for (x=0;x<c1.columns();++x) {
            // get both derivative vectors
            dfdx.set(dx1.at(y,x),dx2.at(y,x),dx3.at(y,x));
            dfdy.set(dy1.at(y,x),dy2.at(y,x),dy3.at(y,x));
            
            // components of the tensor
            E=dfdx.dot(dfdx);
            F=dfdx.dot(dfdy);
            G=dfdy.dot(dfdy);
            
            // the vector of maximal contrast is given by the eigenvalues of
            // the tensor matrix lambda = (E+G) +/- sqrt((E-G)^2+4F^2)/2;
            EpG=E+G;
            if (EpG<=std::numeric_limits<float>::epsilon()) {
              gradAbs.at(y,x)=0.0f;
              orientation.at(y,x)=0.0f;
            } else {
              EmG=E-G;
              a = sqrt(EmG*EmG+4.0f*F*F);
              // maximum directional derivative -> lambda_max
              maxVal=max((gradAbs.at(y,x)=sqrt((EpG + a)*0.5f)),maxVal);
              a = 0.5f*atan2(2.0f*F,EmG);
              orientation.at(y,x)=a;
            }
          }
        }
      } break;
      case parameters::Maximum: {
        float mr,mg,mb;
        for (y=0;y<c1.rows();++y) {
          for (x=0;x<c1.columns();++x) {
            mr=magSqr(dx1.at(y,x),dy1.at(y,x));
            mg=magSqr(dx2.at(y,x),dy2.at(y,x));
            mb=magSqr(dx3.at(y,x),dy3.at(y,x));
            if (mr > mg) {
              if (mr > mb) {
                maxVal=max((gradAbs.at(y,x)=sqrt(mr)),maxVal);
                orientation.at(y,x) = atan2(dy1.at(y,x),dx1.at(y,x));
              } else {
                maxVal=max((gradAbs.at(y,x)=sqrt(mb)),maxVal);
                orientation.at(y,x) = atan2(dy3.at(y,x),dx3.at(y,x));
              }
            } else {
              if (mg > mb) {
                maxVal=max((gradAbs.at(y,x)=sqrt(mg)),maxVal);
                orientation.at(y,x) = atan2(dy2.at(y,x),dx2.at(y,x));
              } else {
                maxVal=max((gradAbs.at(y,x)=sqrt(mb)),maxVal);
                orientation.at(y,x) = atan2(dy3.at(y,x),dx3.at(y,x));
              }             
            }
          }
        }        
      } break;
      default:
        setStatusString("Unknown contrastFormat type.");
        return false;
    }

    if (getParameters().kernelType == parameters::Roberts) {
      // we still need to fix the angle, which is shifted 45 degrees:
      orientation.add(static_cast<float>(Pi/4.0));
    }
    
    return true;
  }

  /*
   * differentiate the image in X and Y directions
   * @param c1 first channel
   * @param c2 second channel
   * @param gradAbs the absolute values of the gradient
   * @param orientation the orientation map of the gradient image
   * @return true if successful, false otherwise.
   */
  bool colorContrastGradient::computeGradientPolar(const channel& c1,
                                                   const channel& c2,
                                                   channel& gradAbs,
                                                   channel& orientation,
                                                   float& maxVal) const {
    
    // compute the gradients
    channel dx1,dx2,dy1,dy2;

    // compute the partial derivatives of each channel respect to x and y
    computeGradientCart(c1,dx1,dy1);
    computeGradientCart(c2,dx2,dy2);

    // resize the result
    gradAbs.resize(c1.size(),float(),false,false);
    orientation.resize(c1.size(),float(),false,false);

    // Determine the direction and amplitude of highest contrast.
    int x,y;
    float E,F,G,EpG,EmG,a;
    tpoint<float> dfdx(0,0),dfdy(0,0);
    maxVal=0.0f;

    switch (getParameters().contrastFormat) {
      case parameters::Contrast: {
        for (y=0;y<c1.rows();++y) {
          for (x=0;x<c1.columns();++x) {
            // get both derivative vectors
            dfdx.set(dx1.at(y,x),dx2.at(y,x));
            dfdy.set(dy1.at(y,x),dy2.at(y,x));
            
            // components of the tensor
            E=dfdx.dot(dfdx);
            F=dfdx.dot(dfdy);
            G=dfdy.dot(dfdy);
            
            // the vector of maximal contrast is given by the eigenvalues of
            // the tensor matrix lambda = (E+G) +/- sqrt((E-G)^2+4F^2)/2;
            EpG=E+G;
            if (EpG<=std::numeric_limits<float>::epsilon()) {
              gradAbs.at(y,x)=0.0f;
              orientation.at(y,x)=0.0f;
            } else {
              EmG=E-G;
              // best contrast obtained with lambda_max - lambda_min
              maxVal=max((gradAbs.at(y,x)=pow(EmG*EmG+4.0f*F*F,0.25f)),maxVal);
              a = 0.5f*atan2(2.0f*F,EmG);
              orientation.at(y,x)=a;
            }
          }
        }
      } break;
      case parameters::MDD: {
        for (y=0;y<c1.rows();++y) {
          for (x=0;x<c1.columns();++x) {
            // get both derivative vectors
            dfdx.set(dx1.at(y,x),dx2.at(y,x));
            dfdy.set(dy1.at(y,x),dy2.at(y,x));
            
            // components of the tensor
            E=dfdx.dot(dfdx);
            F=dfdx.dot(dfdy);
            G=dfdy.dot(dfdy);
            
            // the vector of maximal contrast is given by the eigenvalues of
            // the tensor matrix lambda = (E+G) +/- sqrt((E-G)^2+4F^2)/2;
            EpG=E+G;
            if (EpG<=std::numeric_limits<float>::epsilon()) {
              gradAbs.at(y,x)=0.0f;
              orientation.at(y,x)=0.0f;
            } else {
              EmG=E-G;
              a = sqrt(EmG*EmG+4.0f*F*F);
              // maximum directional derivative -> lambda_max
              maxVal=max((gradAbs.at(y,x)=sqrt((EpG + a)*0.5f)),maxVal);
              a = 0.5f*atan2(2.0f*F,EmG);
              orientation.at(y,x)=a;
            }
          }
        }
      } break;
      case parameters::Maximum: {
        float mr,mg;
        for (y=0;y<c1.rows();++y) {
          for (x=0;x<c1.columns();++x) {
            mr=magSqr(dx1.at(y,x),dy1.at(y,x));
            mg=magSqr(dx2.at(y,x),dy2.at(y,x));
            if (mr > mg) {
              maxVal=max((gradAbs.at(y,x)=sqrt(mr)),maxVal);
              orientation.at(y,x) = atan2(dy1.at(y,x),dx1.at(y,x));
            } else {
              maxVal=max((gradAbs.at(y,x)=sqrt(mg)),maxVal);
              orientation.at(y,x) = atan2(dy2.at(y,x),dx2.at(y,x));
            }
          }
        }
      } break;
      default:
        setStatusString("Unknown contrastFormat type.");
        return false;
    }


    if (getParameters().kernelType == parameters::Roberts) {
      // we still need to fix the angle, which is shifted 45 degrees:
      orientation.add(static_cast<float>(Pi/4.0));
    }
    
    return true;
  }


  /*
   * differentiate the image in X and Y directions
   *
   * The lambdaMax-lambdaMin corresponds usually to the highest contrast.
   *
   * @param c1 first channel
   * @param c2 second channel
   * @param c3 third channel
   * @param lambdaMin minimum eigenvalue
   * @param lambdaMax maximum eigenvalue
   * @param orientation the orientation map of the gradient image
   * @return true if successful, false otherwise.
   */
  bool colorContrastGradient::computeGradientPolar(const channel& c1,
                                                   const channel& c2,
                                                   const channel& c3,
                                                   channel& lambdaMin,
                                                   channel& lambdaMax,
                                                   channel& orientation,
                                                   float& maxVal) const {
    
    // compute the gradients
    channel dx1,dx2,dx3, dy1,dy2,dy3;

    // compute the partial derivatives of each channel respect to x and y
    computeGradientCart(c1,dx1,dy1);
    computeGradientCart(c2,dx2,dy2);
    computeGradientCart(c3,dx3,dy3);

    // resize the result
    lambdaMin.resize(c1.size(),float(),false,false);
    lambdaMax.resize(c1.size(),float(),false,false);
    orientation.resize(c1.size(),float(),false,false);

    // Determine the direction and amplitude of highest contrast.
    int x,y;
    float E,F,G,EpG,EmG,a;
    trgbPixel<float> dfdx(0,0,0),dfdy(0,0,0);
    maxVal=0.0f;

    for (y=0;y<c1.rows();++y) {
      for (x=0;x<c1.columns();++x) {
        // get both derivative vectors
        dfdx.set(dx1.at(y,x),dx2.at(y,x),dx3.at(y,x));
        dfdy.set(dy1.at(y,x),dy2.at(y,x),dy3.at(y,x));

        // components of the tensor
        E=dfdx.dot(dfdx);
        F=dfdx.dot(dfdy);
        G=dfdy.dot(dfdy);

        // the vector of maximal contrast is given by the eigenvalues of
        // the tensor matrix lambda = (E+G) +/- sqrt((E-G)^2+4F^2)/2;
        EpG=E+G;
        if (EpG <= std::numeric_limits<float>::epsilon()) {
          lambdaMin.at(y,x)=lambdaMax.at(y,x)=orientation.at(y,x)=0.0f;
        } else {
          EmG=E-G;
          a = sqrt(EmG*EmG+4.0f*F*F);
          lambdaMin.at(y,x) = (EpG - a)*0.5f;
          lambdaMax.at(y,x) = (EpG + a)*0.5f;

          // best contrast obtained with lambda_max - lambda_min
          maxVal=max(a,maxVal);
          a = 0.5f*atan2(2.0f*F,EmG);
          orientation.at(y,x)=a;
        }
      }
    }

    if (getParameters().kernelType == parameters::Roberts) {
      // we still need to fix the angle, which is shifted 45 degrees:
      orientation.add(static_cast<float>(Pi/4.0));
    }
    
    return true;
  }


  bool colorContrastGradient::computeColorGradientCart(const channel& c1,
                                                       const channel& c2,
                                                       const channel& c3,
                                                       channel& xchnl,
                                                       channel& ychnl,
                                                       float& maxVal) const {
    
    // compute the gradients
    channel dx1,dx2,dx3,dy1,dy2,dy3;

    // compute the partial derivatives of each channel respect to x and y
    computeGradientCart(c1,dx1,dy1);
    computeGradientCart(c2,dx2,dy2);
    computeGradientCart(c3,dx3,dy3);

    // resize the result
    xchnl.resize(c1.size(),float(),false,false);
    ychnl.resize(c1.size(),float(),false,false);

    // Determine the direction and amplitude of highest contrast.

    int x,y;
    float E,F,G,EpG,EmG,mag2;
    trgbPixel<float> dfdx(0,0,0),dfdy(0,0,0);
    maxVal=0.0f;

    switch (getParameters().contrastFormat) {
      case parameters::Contrast: {

        for (y=0;y<c1.rows();++y) {
          for (x=0;x<c1.columns();++x) {
            // get both derivative vectors
            dfdx.set(dx1.at(y,x),dx2.at(y,x),dx3.at(y,x));
            dfdy.set(dy1.at(y,x),dy2.at(y,x),dy3.at(y,x));
            
            // components of the tensor
            E=dfdx.dot(dfdx);
            F=dfdx.dot(dfdy);
            G=dfdy.dot(dfdy);
            
            // the vector of maximal contrast is given by the eigenvalues of
            // the tensor matrix lambda = (E+G) +/- sqrt((E-G)^2+4F^2)/2;
            EpG=E+G;
            if (EpG<=std::numeric_limits<float>::epsilon()) {
              xchnl.at(y,x)=0.0f;
              ychnl.at(y,x)=0.0f;
            } else {
              EmG=E-G;
              mag2 = sqrt(EmG*EmG+4.0f*F*F);
              
              // we know:
              // angle = 0.5 arctan(2F/(E-G))
              // so that 2*angle = arctan(2F/E-G)
              // if we assume a vector [ E-G , 2F ] -> its magnitude: mag2
              // cos(2*angle) = (E-G)/mag
              // sin(2*angle) = 2F/mag
              
              // with cos(2*angle) = 2*cos^2(angle) - 1 and
              //      sin(2*angle) = 2*sin(angle)*cos(angle)
              
              // and x = mag * cos(alpha)
              //     y = mag * sin(alpha) we finally get

              xchnl.at(y,x)=sqrt((mag2 + EmG)/2.0f);
              ychnl.at(y,x)=sqrt((mag2 - EmG)/2.0f);
              
              maxVal=max(mag2,maxVal);
            }
          }
        }
      } break;
      case parameters::MDD: {
        for (y=0;y<c1.rows();++y) {
          for (x=0;x<c1.columns();++x) {
            // get both derivative vectors
            dfdx.set(dx1.at(y,x),dx2.at(y,x),dx3.at(y,x));
            dfdy.set(dy1.at(y,x),dy2.at(y,x),dy3.at(y,x));
            
            // components of the tensor
            E=dfdx.dot(dfdx);
            F=dfdx.dot(dfdy);
            G=dfdy.dot(dfdy);
            
            // the vector of maximal contrast is given by the eigenvalues of
            // the tensor matrix lambda = (E+G) +/- sqrt((E-G)^2+4F^2)/2;
            EpG=E+G;
            if (EpG<=std::numeric_limits<float>::epsilon()) {
              xchnl.at(y,x)=0.0f;
              ychnl.at(y,x)=0.0f;
            } else {
              EmG=E-G;
              mag2 = sqrt(EmG*EmG+4.0f*F*F);
              
              // we know:
              // angle = 0.5 arctan(2F/(E-G))
              // so that 2*angle = arctan(2F/E-G)
              // if we assume a vector [ E-G , 2F ] -> its magnitude: mag2
              // cos(2*angle) = (E-G)/mag2
              // sin(2*angle) = 2F/mag2
              
              // with cos(2*angle) = 2*cos^2(angle) - 1 and
              //      sin(2*angle) = 2*sin(angle)*cos(angle)
              
              // and mag = sqrt(mag2)
              //     x = mag * cos(alpha)
              //     y = mag * sin(alpha) we finally get

              xchnl.at(y,x)=sqrt((mag2+EpG)*(mag2+EmG)/(4*mag2));
              ychnl.at(y,x)=sqrt((mag2+EpG)*(mag2-EmG)/(4*mag2));
             
              maxVal=max(mag2,maxVal);
            }
          }
        }
      } break;
      case parameters::Maximum: {
        float mr,mg,mb;
        for (y=0;y<c1.rows();++y) {
          for (x=0;x<c1.columns();++x) {
            mr=magSqr(dx1.at(y,x),dy1.at(y,x));
            mg=magSqr(dx2.at(y,x),dy2.at(y,x));
            mb=magSqr(dx3.at(y,x),dy3.at(y,x));
            if (mr > mg) {
              if (mr > mb) {
                xchnl.at(y,x)=dx1.at(y,x);
                ychnl.at(y,x)=dy1.at(y,x);
                maxVal=max(maxVal,mr);
              } else {
                xchnl.at(y,x)=dx3.at(y,x);
                ychnl.at(y,x)=dy3.at(y,x);
                maxVal=max(maxVal,mb);
              }
            } else {
              if (mg > mb) {
                xchnl.at(y,x)=dx2.at(y,x);
                ychnl.at(y,x)=dy2.at(y,x);
                maxVal=max(maxVal,mg);
              } else {
                xchnl.at(y,x)=dx3.at(y,x);
                ychnl.at(y,x)=dy3.at(y,x);
                maxVal=max(maxVal,mb);
              }
            }
          }
        }
      } break;
      default:
        setStatusString("Unknown contrastFormat type.");
        return false;
    }

    maxVal=sqrt(maxVal);

    return true;
  }

  // cartesian bi-spectral
  
  bool colorContrastGradient::computeColorGradientCart(const channel& c1,
                                                       const channel& c2,
                                                       channel& xchnl,
                                                       channel& ychnl,
                                                       float& maxVal) const {
    
    // compute the gradients
    channel dx1,dx2,dy1,dy2;

    // compute the partial derivatives of each channel respect to x and y
    computeGradientCart(c1,dx1,dy1);
    computeGradientCart(c2,dx2,dy2);

    // resize the result
    xchnl.resize(c1.size(),float(),false,false);
    ychnl.resize(c1.size(),float(),false,false);

    // Determine the direction and amplitude of highest contrast.

    int x,y;
    float E,F,G,EpG,EmG,mag2;
    tpoint<float> dfdx(0,0),dfdy(0,0);
    maxVal=0.0f;

    switch (getParameters().contrastFormat) {
      case parameters::Contrast: {

        for (y=0;y<c1.rows();++y) {
          for (x=0;x<c1.columns();++x) {
            // get both derivative vectors
            dfdx.set(dx1.at(y,x),dx2.at(y,x));
            dfdy.set(dy1.at(y,x),dy2.at(y,x));
            
            // components of the tensor
            E=dfdx.dot(dfdx);
            F=dfdx.dot(dfdy);
            G=dfdy.dot(dfdy);
            
            // the vector of maximal contrast is given by the eigenvalues of
            // the tensor matrix lambda = (E+G) +/- sqrt((E-G)^2+4F^2)/2;
            EpG=E+G;
            if (EpG<=std::numeric_limits<float>::epsilon()) {
              xchnl.at(y,x)=0.0f;
              ychnl.at(y,x)=0.0f;
            } else {
              EmG=E-G;
              mag2 = sqrt(EmG*EmG+4.0f*F*F);
              
              // best contrast obtained with lambda_max - lambda_min
              xchnl.at(y,x)=sqrt((mag2 + EmG)/2.0f);
              ychnl.at(y,x)=sqrt((mag2 - EmG)/2.0f);
              
              maxVal=max(mag2,maxVal);
            }
          }
        }
      } break;
      case parameters::MDD: {
        for (y=0;y<c1.rows();++y) {
          for (x=0;x<c1.columns();++x) {
            // get both derivative vectors
            dfdx.set(dx1.at(y,x),dx2.at(y,x));
            dfdy.set(dy1.at(y,x),dy2.at(y,x));
            
            // components of the tensor
            E=dfdx.dot(dfdx);
            F=dfdx.dot(dfdy);
            G=dfdy.dot(dfdy);
            
            // the vector of maximal contrast is given by the eigenvalues of
            // the tensor matrix lambda = (E+G) +/- sqrt((E-G)^2+4F^2)/2;
            EpG=E+G;
            if (EpG<=std::numeric_limits<float>::epsilon()) {
              xchnl.at(y,x)=0.0f;
              ychnl.at(y,x)=0.0f;
            } else {
              EmG=E-G;
              mag2 = sqrt(EmG*EmG+4.0f*F*F);
              
              // we know:
              // angle = 0.5 arctan(2F/(E-G))
              // so that 2*angle = arctan(2F/E-G)
              // if we assume a vector [ E-G , 2F ] -> its magnitude: mag2
              // cos(2*angle) = (E-G)/mag2
              // sin(2*angle) = 2F/mag2
              
              // with cos(2*angle) = 2*cos^2(angle) - 1 and
              //      sin(2*angle) = 2*sin(angle)*cos(angle)
              
              // and mag = sqrt(mag2)
              //     x = mag * cos(alpha)
              //     y = mag * sin(alpha) we finally get

              xchnl.at(y,x)=sqrt((mag2+EpG)*(mag2+EmG)/(4*mag2));
              ychnl.at(y,x)=sqrt((mag2+EpG)*(mag2-EmG)/(4*mag2));
             
              maxVal=max(mag2,maxVal);
            }
          }
        }
      } break;
      case parameters::Maximum: {
        float mr,mg;
        for (y=0;y<c1.rows();++y) {
          for (x=0;x<c1.columns();++x) {
            mr=magSqr(dx1.at(y,x),dy1.at(y,x));
            mg=magSqr(dx2.at(y,x),dy2.at(y,x));
            if (mr > mg) {
              xchnl.at(y,x)=dx1.at(y,x);
              ychnl.at(y,x)=dy1.at(y,x);
              maxVal=max(maxVal,mr);
            } else {
              xchnl.at(y,x)=dx2.at(y,x);
              ychnl.at(y,x)=dy2.at(y,x);
              maxVal=max(maxVal,mg);
            }
          }
        }        

      } break;
      default:
        setStatusString("Unknown contrastFormat type.");
        return false;
    }

    
    maxVal=sqrt(maxVal);

    return true;
  }

}
