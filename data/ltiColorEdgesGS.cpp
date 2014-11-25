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
 * file .......: ltiColorEdgesGS.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 17.10.2003
 * revisions ..: $Id: ltiColorEdgesGS.cpp,v 1.13 2006/09/05 10:06:18 ltilib Exp $
 */

#include "ltiColorEdgesGS.h"
#include "ltiArctanLUT.h"
#include "ltiGradientKernels.h"
#include "ltiOgdKernels.h"
#include "ltiConvolution.h"

#undef _LTI_DEBUG
// #define _LTI_DEBUG 2
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#include "ltiViewer.h"
#endif


namespace lti {
  // --------------------------------------------------
  // colorEdgesGS::parameters
  // --------------------------------------------------

  // default constructor
  colorEdgesGS::parameters::parameters()
    : edgeDetector::parameters() {

    shadowEdgeValue     = ubyte(128);
    highlightEdgeValue  = ubyte(192);
    noiseStdDeviation   = trgbPixel<float>(1.5f,1.5f,1.5f);
    gradient.format     = gradientFunctor::parameters::Cartesic;
    gradient.kernelType = gradientFunctor::parameters::Sobel;
    uncertaintyFactor = 3.0f;
  }

  // copy constructor
  colorEdgesGS::parameters::parameters(const parameters& other)
    : edgeDetector::parameters() {
    copy(other);
  }

  // destructor
  colorEdgesGS::parameters::~parameters() {
  }

  // get type name
  const char* colorEdgesGS::parameters::getTypeName() const {
    return "colorEdgesGS::parameters";
  }

  // copy member

  colorEdgesGS::parameters&
    colorEdgesGS::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    edgeDetector::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    edgeDetector::parameters& (edgeDetector::parameters::* p_copy)
      (const edgeDetector::parameters&) =
      edgeDetector::parameters::copy;
    (this->*p_copy)(other);
# endif

    
    shadowEdgeValue = other.shadowEdgeValue;
    highlightEdgeValue = other.highlightEdgeValue;
    noiseStdDeviation = other.noiseStdDeviation;
    gradient.copy(other.gradient);
    uncertaintyFactor = other.uncertaintyFactor;

    return *this;
  }

  // alias for copy member
  colorEdgesGS::parameters&
    colorEdgesGS::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* colorEdgesGS::parameters::clone() const {
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
  bool colorEdgesGS::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool colorEdgesGS::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"shadowEdgeValue",shadowEdgeValue);
      lti::write(handler,"highlightEdgeValue",highlightEdgeValue);
      lti::write(handler,"noiseStdDeviation",noiseStdDeviation);
      lti::write(handler,"gradient",gradient);
      lti::write(handler,"uncertaintyFactor",uncertaintyFactor);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && edgeDetector::parameters::write(handler,false);
# else
    bool (edgeDetector::parameters::* p_writeMS)(ioHandler&,const bool) const =
      edgeDetector::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool colorEdgesGS::parameters::write(ioHandler& handler,
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
  bool colorEdgesGS::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool colorEdgesGS::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"shadowEdgeValue",shadowEdgeValue);
      lti::read(handler,"highlightEdgeValue",highlightEdgeValue);
      lti::read(handler,"noiseStdDeviation",noiseStdDeviation);
      lti::read(handler,"gradient",gradient);
      lti::read(handler,"uncertaintyFactor",uncertaintyFactor);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && edgeDetector::parameters::read(handler,false);
# else
    bool (edgeDetector::parameters::* p_readMS)(ioHandler&,const bool) =
      edgeDetector::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool colorEdgesGS::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  bool colorEdgesGS::parameters::measureNoise(const image& img,
                                              const rectangle& window) {
    int minX,maxX,minY,maxY;
    minX = max(0,min(window.ul.x,window.br.x));
    minY = max(0,min(window.ul.y,window.br.y));

    maxX = min(img.lastColumn(),max(window.ul.x,window.br.x));
    maxY = min(img.lastRow(),max(window.ul.y,window.br.y));

    int y,x;
    trgbPixel<float> sum(0,0,0),sum2(0,0,0);
    trgbPixel<float> c;
    int n = 0;

    for (y=minY;y<=maxY;++y) {
      for (x=minX;x<=maxX;++x) {
        c.copy(img.at(y,x));
        sum.add(c);
        sum2.add(trgbPixel<float>(c.red*c.red,c.green*c.green,c.blue*c.blue));
        n++;
      }
    }
    sum.divide(static_cast<float>(n));
    sum2.divide(static_cast<float>(n));
    noiseStdDeviation.set(sqrtrect(sum2.red   - sum.red*sum.red),
                          sqrtrect(sum2.green - sum.green*sum.green),
                          sqrtrect(sum2.blue  - sum.blue*sum.blue));
    
    return true;
  }

  // --------------------------------------------------
  // colorEdgesGS
  // --------------------------------------------------

  // default constructor
  colorEdgesGS::colorEdgesGS()
    : edgeDetector(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  colorEdgesGS::colorEdgesGS(const parameters& par)
    : edgeDetector() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  colorEdgesGS::colorEdgesGS(const colorEdgesGS& other)
    : edgeDetector() {
    copy(other);
  }

  // destructor
  colorEdgesGS::~colorEdgesGS() {
  }

  // returns the name of this type
  const char* colorEdgesGS::getTypeName() const {
    return "colorEdgesGS";
  }

  // copy member
  colorEdgesGS&
  colorEdgesGS::copy(const colorEdgesGS& other) {
    edgeDetector::copy(other);
      
    return (*this);
  }

  // alias for copy member
  colorEdgesGS&
    colorEdgesGS::operator=(const colorEdgesGS& other) {
    return (copy(other));
  }


  // clone member
  functor* colorEdgesGS::clone() const {
    return new colorEdgesGS(*this);
  }

  // return parameters
  const colorEdgesGS::parameters& colorEdgesGS::getParameters() const {
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

  bool colorEdgesGS::apply(channel8 &srcdest) const {
    image tmp;
    tmp.castFrom(srcdest);
    return apply(tmp,srcdest);
  }

  bool colorEdgesGS::apply(channel &srcdest) const {
    image tmp;
    channel8 chnl8;

    tmp.castFrom(srcdest);
    if (apply(tmp,chnl8)) {
      srcdest.castFrom(chnl8);
      return true;
    }
    
    return false;
  }

  bool colorEdgesGS::apply(const channel8 &src, channel8 &dest) const {
    image tmp;
    tmp.castFrom(src);
    return apply(tmp,dest);    
  }

  bool colorEdgesGS::apply(const channel &src, channel &dest) const {
    image tmp;
    channel8 chnl8;

    tmp.castFrom(src);
    if (apply(tmp,chnl8)) {
      dest.castFrom(chnl8);
      return true;
    }
    
    return false;
  } 

  bool colorEdgesGS::apply(const channel &src, channel8 &dest) const {
    image tmp;
    tmp.castFrom(src);
    return apply(tmp,dest);
  } 

  bool colorEdgesGS::apply (image &srcdest) const {
    image tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }
    return false;
  }

  bool colorEdgesGS::apply (const image &src, image &dest) const {
    const parameters& par = getParameters();

    const rgbPixel back = rgbPixel(par.noEdgeValue,
                                   par.noEdgeValue,
                                   par.noEdgeValue);

    const rgbPixel mate = rgbPixel(par.edgeValue,
                                   par.edgeValue,
                                   par.edgeValue);

    const rgbPixel shad = rgbPixel(par.shadowEdgeValue,0,0);
    const rgbPixel high = rgbPixel(0,par.highlightEdgeValue,0);
    
    channel8 mask;
    if (apply(src,mask)) {
      dest.resize(src.size(),rgbPixel(),false,false);
      channel8::const_iterator mit;
      image::iterator it,eit;
      eit=dest.end();

      for (it=dest.begin(),mit=mask.begin();it!=eit;++it,++mit) {
        if ((*mit) == par.noEdgeValue) {
          (*it)=back;
        } else if ((*mit) == par.edgeValue) {
          (*it)=mate;
        } else if ((*mit) == par.shadowEdgeValue) {
          (*it)=shad;
        } else if ((*mit) == par.highlightEdgeValue) {
          (*it)=high;
        } else {
          // this shouldn't happend, but to avoid uninitialized memory:
          (*it) = lti::Black;
        }
      }
      return true;
    }

    return false;
  }

  bool colorEdgesGS::apply (const image &src, channel8 &dest) const {
    if (src.empty()) {
      dest.clear();
      return true;
    }

    const parameters& par = getParameters();

    // step one: determine the components for the three color spaces
    //           rgb, c1c2 and o1o2 and the standard deviations sigma_c1,
    //           sigma_c2, sigma_o1, sigma_o2, the standard deviations sigma_r,
    //           sigma_g and sigma_b are provided in the parameters

    channel red,green,blue,o1,o2,c1,c2; // channels where the
                                        // components are stored
    channel stdDevC1,stdDevC2;          // channels for standard
                                        // deviations of components c1
                                        // and c2
    float   stdDevO1,stdDevO2;          // standard deviations for
                                        // components o1 and o2, note
                                        // that they are scalar
    float   stdDevR,stdDevG,stdDevB;    // standard deviations for
                                        // components r,g,b
    float   varR,varG,varB;             // variances of the r,g and b
                                        // channel to speed up
                                        // calculations

    // initialize all channels
    red.resize(src.size(),float(),false,false);
    green.resize(src.size(),float(),false,false);
    blue.resize(src.size(),float(),false,false);
    o1.resize(src.size(),float(),false,false);
    o2.resize(src.size(),float(),false,false);
    c1.resize(src.size(),float(),false,false);
    c2.resize(src.size(),float(),false,false);

    stdDevC1.resize(src.size(),float(),false,false);
    stdDevC2.resize(src.size(),float(),false,false);

    // calculate standard deviations and variances for r,g and b channel
    stdDevR = par.noiseStdDeviation.getRed() / 255.0f;
    stdDevG = par.noiseStdDeviation.getGreen() / 255.0f;
    stdDevB = par.noiseStdDeviation.getBlue() / 255.0f;
    varR = stdDevR * stdDevR;
    varG = stdDevG * stdDevG;
    varB = stdDevB * stdDevB;
    // calculate standard deviations and variances for o1,o2 channel
    stdDevO1 = 0.5f * sqrt(varG + varR);
    stdDevO2 = 0.25f * sqrt(4.0f * varB + varG + varR);

    arctanLUT atan2;      // object for fast calculation of arctan
    ubyte     r,g,b;      // pixel values for red, green, and blue component
    float     fr,fg,fb;   // normalized values for red, green, and
                          // blue component
    int y,x;

    // run through source image and calculate all color components and
    // standard deviations for c1 and c2
    for (y=0;y<src.rows();++y) {
      for (x=0;x<src.columns();++x) {
        const rgbPixel& px = src.at(y,x);
        r=px.getRed();
        g=px.getGreen();
        b=px.getBlue();
        // initialize red, green, and blue component channels with
        // normalized values
        fr = red.at(y,x)   = float(r)/255.0f;
        fg = green.at(y,x) = float(g)/255.0f;
        fb = blue.at(y,x)  = float(b)/255.0f;
        // calculate c1 and c2 component (it is much better with the ubytes!)
        c1.at(y,x) = atan2(r,b);
        c2.at(y,x) = atan2(g,b);
        // calculate o1 and o2 component
        o1.at(y,x) = 0.5f*(fr-fg);
        o2.at(y,x) = 0.5f*fb - 0.25f*(fr+fg);
        // calculate standard deviations for c1 and c2 channel
        if (b==0) {
          if (r == 0) {
            stdDevC1.at(y,x) = 1.0f;
          } else {
            stdDevC1.at(y,x) = sqrt(fr*fr*varB)/(fr*fr);
          }
          if (g == 0) {
            stdDevC2.at(y,x) = 1.0f;
          } else {
            stdDevC2.at(y,x) = sqrt(fg*fg*varB)/(fg*fg);
          }
        } else {
          stdDevC1.at(y,x) = sqrt(fr*fr*varB + fb*fb*varR)/(fr*fr + fb*fb);
          stdDevC2.at(y,x) = sqrt(fg*fg*varB + fb*fb*varG)/(fg*fg + fb*fb);
        }
      }
    }

    // step two: the moduli of the gradients of all three color spaces
    //           have to be calculated and their uncertainties (standard
    //           deviations) respectively
    channel gradRGB,gradO1O2,gradC1C2;       // channels for the
                                             // moduli of gradients
    channel stdDevRGB,stdDevO1O2,stdDevC1C2; // channels for the
                                             // standard deviations

    getGradients(red,green,blue,          o1,o2,             c1,c2,
                 stdDevR,stdDevG,stdDevB, stdDevO1,stdDevO2, stdDevC1,stdDevC2,
                 gradRGB,stdDevRGB,       gradO1O2,stdDevO1O2,gradC1C2,
                                                              stdDevC1C2);

    // initialize output image
    dest.resize(src.size(),ubyte(),false,false);
    
    // step three: the edges have to be classified into material,
    // highlight, shadow and no edge
    bool drgb,dcs,dos;
    for (y=0;y<gradRGB.rows();++y) {
      for (x=0;x<gradRGB.columns();++x) {
        if (gradRGB.at(y,x) < stdDevRGB.at(y,x)) {
          gradRGB.at(y,x)=0.0f;
          drgb = false;
        } else {
          if (stdDevRGB.at(y,x) == 0.0f) {
            // no variation => no edge!
            gradRGB.at(y,x)=0.0f;
            drgb = false;            
          } else {
            drgb = true;
          }
        }

        if (gradO1O2.at(y,x) < stdDevO1O2.at(y,x)) {
          gradO1O2.at(y,x)=0.0f;
          dos = false;
        } else {
          dos = true;
        }

        if (gradC1C2.at(y,x) < stdDevC1C2.at(y,x)) {
          gradC1C2.at(y,x)=0.0f;
          dcs = false;
        } else {
          dcs = true;
        }

        if (drgb) {
          if (!dcs) {
            dest.at(y,x) = par.shadowEdgeValue;
          } else if (dcs && !dos) {
            dest.at(y,x) = par.highlightEdgeValue;
          } else {
            dest.at(y,x) = par.edgeValue;
          }
        } else {
          dest.at(y,x) = par.noEdgeValue;
        }
      }
    }
    
#ifdef _LTI_DEBUG 
    static viewer v1("rgb gradient");
    static viewer v2("rgb uncertainty");
    static viewer v3("o1o2 gradient");
    static viewer v4("o1o2 uncertainty");
    static viewer v5("c1c2 gradient");
    static viewer v6("c1c2 uncertainty");
    static viewer ve("edges");

    ve.show(dest);

    v1.show(gradRGB);
    v2.show(stdDevRGB);

    v3.show(gradO1O2);
    v4.show(stdDevO1O2);

    v5.show(gradC1C2);
    v6.show(stdDevC1C2);
    
#endif

    return true;
  }


  bool colorEdgesGS::getGradients(const channel& red,
                                  const channel& green,
                                  const channel& blue,
                                  const channel& o1,
                                  const channel& o2,
                                  const channel& c1,
                                  const channel& c2,
                                  const float& stdDevR,
                                  const float& stdDevG,
                                  const float& stdDevB,
                                  const float& stdDevO1,
                                  const float& stdDevO2,
                                  const channel& stdDevC1,
                                  const channel& stdDevC2,
                                  channel& gradRGB,
                                  channel& stdDevRGB,
                                  channel& gradO1O2,
                                  channel& stdDevO1O2,
                                  channel& gradC1C2,
                                  channel& stdDevC1C2) const {
    
    const parameters& par = getParameters();

    // initialize all output channels
    gradRGB.resize(red.size(),float(),false,false);
    stdDevRGB.resize(red.size(),float(),false,false);
    gradO1O2.resize(o1.size(),float(),false,false);
    stdDevO1O2.resize(o1.size(),float(),false,false);
    gradC1C2.resize(c1.size(),float(),false,false);
    stdDevC1C2.resize(c1.size(),float(),false,false);

    int y,x;                             // variables for channel scan
    channel gx1,gy1,gx2,gy2,gx3,gy3;     // channels two hold the
                                         // gradient information for
                                         // each component and
                                         // direction
    float fgx1,fgx2,fgx3,fgy1,fgy2,fgy3; // help variables containing
                                         // specific values of the
                                         // channels above
    float stdDevNum,stdDevDen;           // help variables containing
                                         // numerator and denumerator
                                         // for uncertainty
                                         // calculation

    // parameters for the gradient computation
    gradientFunctor::parameters gradPar(par.gradient);
    // ensure the cartesic mode
    gradPar.format = gradientFunctor::parameters::Cartesic;
    gradientFunctor grad(gradPar);

    convolution conv; // the convolution object

    // -----------------------------------------
    // the RGB gradient modulus and uncertainty
    // -----------------------------------------

//----------------------------------------------
    sepKernel<float> ux,uy;
    constructUncertaintyKernels(ux,uy);

    kernel2D<float> ux2,uy2;
    ux2.castFrom(ux);
    uy2.castFrom(uy);

    const float uvx = ux2.sumOfElements();
    const float uvy = uy2.sumOfElements();

    // these are the sigma_(dci/dx) and sigma_(dci/dy) for i = r,g and b
    const float sdrx = stdDevR*uvx;
    const float sdry = stdDevR*uvy;
    const float sdgx = stdDevG*uvx;
    const float sdgy = stdDevG*uvy;
    const float sdbx = stdDevB*uvx;
    const float sdby = stdDevB*uvy;
//----------------------------------------------

    // calculate gradients
    grad.apply(red,gx1,gy1);
    grad.apply(green,gx2,gy2);
    grad.apply(blue,gx3,gy3);

    // determine modulus
    for (y=0;y<gradRGB.rows();++y) {
      for (x=0;x<gradRGB.columns();++x) {
        fgx1 = gx1.at(y,x);
        fgy1 = gy1.at(y,x);
        fgx2 = gx2.at(y,x);
        fgy2 = gy2.at(y,x);
        fgx3 = gx3.at(y,x);
        fgy3 = gy3.at(y,x);

        gradRGB.at(y,x) = sqrt(fgx1*fgx1 + fgx2*fgx2 + fgx3*fgx3 +
                               fgy1*fgy1 + fgy2*fgy2 + fgy3*fgy3);

        stdDevNum = abs(fgx1)*sdrx + abs(fgx2)*sdgx + abs(fgx3)*sdbx +
                    abs(fgy1)*sdry + abs(fgy2)*sdgy + abs(fgy3)*sdby;
        stdDevDen = gradRGB.at(y,x);// sqrt(abs(fgx1) + abs(fgx2) + abs(fgx3) 
                                    // + abs(fgy1) + abs(fgy2) + abs(fgy3));

        stdDevRGB.at(y,x) = (stdDevDen != 0.0) ? 
                         par.uncertaintyFactor*stdDevNum/stdDevDen : stdDevDen;
      }
    }

    // -----------------------------------------
    // the o1o2 gradient modulus and uncertainty
    // -----------------------------------------

//----------------------------------------------
    // these are the sigma_(dci/dx) and sigma_(dci/dy) for i = r,g and b
    const float sdo1x = stdDevO1*uvx;
    const float sdo1y = stdDevO1*uvy;

    const float sdo2x = stdDevO2*uvx;
    const float sdo2y = stdDevO2*uvy;
//----------------------------------------------

    // calculate gradients
    grad.apply(o1,gx1,gy1);
    grad.apply(o2,gx2,gy2);

    // determine modulus
    for (y=0;y<gradO1O2.rows();++y) {
      for (x=0;x<gradO1O2.columns();++x) {
        fgx1 = gx1.at(y,x);
        fgy1 = gy1.at(y,x);
        fgx2 = gx2.at(y,x);
        fgy2 = gy2.at(y,x);

        gradO1O2.at(y,x) = sqrt(fgx1*fgx1 + fgx2*fgx2 +
                                fgy1*fgy1 + fgy2*fgy2);

        stdDevNum = abs(fgx1)*sdo1x + abs(fgx2)*sdo2x +
                    abs(fgy1)*sdo1y + abs(fgy2)*sdo2y;
        stdDevDen = gradO1O2.at(y,x); // sqrt(abs(fgx1) + abs(fgx2) 
                                      // + abs(fgy1) + abs(fgy2));

        stdDevO1O2.at(y,x) = (stdDevDen != 0.0) ? 
                         par.uncertaintyFactor*stdDevNum/stdDevDen : stdDevDen;
      }
    }

    // -----------------------------------------
    // the c1c2 gradient modulus and uncertainty
    // -----------------------------------------

//----------------------------------------------
    // these are the sigma_(dci/dx) and sigma_(dci/dy) for i = r,g and b
    convolution convPar(ux,convolution::parameters::Constant);
    channel uc1x,uc1y,uc2x,uc2y;

    convPar.apply(stdDevC1,uc1x);
    convPar.apply(stdDevC2,uc2x);

    convPar.setKernel(uy);
    convPar.apply(stdDevC1,uc1y);
    convPar.apply(stdDevC2,uc2y);
//----------------------------------------------
    // calculate gradients
    grad.apply(c1,gx1,gy1);
    grad.apply(c2,gx2,gy2);

    // determine modulus
    for (y=0;y<gradC1C2.rows();++y) {
      for (x=0;x<gradC1C2.columns();++x) {
        fgx1 = gx1.at(y,x);
        fgy1 = gy1.at(y,x);
        fgx2 = gx2.at(y,x);
        fgy2 = gy2.at(y,x);

        gradC1C2.at(y,x) = sqrt(fgx1*fgx1 + fgx2*fgx2 +
                                fgy1*fgy1 + fgy2*fgy2);

        stdDevNum = abs(fgx1)*uc1x.at(y,x) + abs(fgx2)*uc2x.at(y,x) +
                    abs(fgy1)*uc1y.at(y,x) + abs(fgy2)*uc2y.at(y,x);
        stdDevDen = gradC1C2.at(y,x); // sqrt(abs(fgx1) + abs(fgx2) 
                                      // + abs(fgy1) + abs(fgy2));

        stdDevC1C2.at(y,x) = (stdDevDen != 0.0) ? 
                         par.uncertaintyFactor*stdDevNum/stdDevDen : stdDevDen;
      }
    }

    return true;
  }

  bool 
  colorEdgesGS::constructUncertaintyKernels(sepKernel<float>& filterX,
                                            sepKernel<float>& filterY) const {
    const parameters& par = getParameters();
    const gradientFunctor::parameters& gradPar = par.gradient;
    
    switch (gradPar.kernelType) {
      case gradientFunctor::parameters::Optimal: {
        gradientKernelX<float> gx(gradPar.gradientKernelSize);
        gradientKernelY<float> gy(gradPar.gradientKernelSize);
        
        filterX.apply(gx,abs);
        filterY.apply(gy,abs);
      } break;
      case gradientFunctor::parameters::OGD: {
        ogd1Kernel<float> gx(gradPar.gradientKernelSize,
                             gradPar.ogdVariance,
                             0.0);
        ogd1Kernel<float> gy(gradPar.gradientKernelSize,
                             gradPar.ogdVariance,
                             Pi/2.0);

        filterX.apply(gx,abs);
        filterY.apply(gy,abs);
      } break;
      case gradientFunctor::parameters::Sobel: {
        sobelKernelX<float> gx(true);
        sobelKernelY<float> gy(true);

        filterX.apply(gx,abs);
        filterY.apply(gy,abs);       
      } break;
      default: {
        // the other kernels: difference, Kirsch or Robinson are or
        // not precise enough or simply not separable.  We use just the 
        // Prewitt kernel instead:
        prewittKernelX<float> gx(true);
        prewittKernelY<float> gy(true);

        filterX.apply(gx,abs);
        filterY.apply(gy,abs);       
      } break;
    }
    
    return true;
  }

}
