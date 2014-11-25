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
 * file .......: ltiGrayWorldNormalization.cpp
 * authors ....: Suat Akyol, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 8.11.2000
 * revisions ..: $Id: ltiGrayWorldNormalization.cpp,v 1.13 2006/09/05 10:14:39 ltilib Exp $
 */

#include "ltiGrayWorldNormalization.h"
#include "ltiSplitImageToRGB.h"
#include "ltiMergeRGBToImage.h"
#include "ltiSerialStatsFunctor.h"
#include "ltiSquareConvolution.h"

namespace lti {
  // --------------------------------------------------
  // grayWorldNormalization::parameters
  // --------------------------------------------------

  // default constructor
  grayWorldNormalization::parameters::parameters()
    : colorNormalizationBase::parameters() {
    // Initialize parameter values!
    kappa = 3.0f;
    allowOverflow = false;
    localNormalization = false;
    windowSize = 64;
    ignoreColorFlag = false;
    ignoreColor = rgbPixel(0,0,0);
    gamma = 1.0f;
    nu = 1.0f;
  }

  // copy constructor
  grayWorldNormalization::parameters::parameters(const parameters& other)
    : colorNormalizationBase::parameters()  {
    copy(other);
  }

  // destructor
  grayWorldNormalization::parameters::~parameters() {
  }

  // get type name
  const char* grayWorldNormalization::parameters::getTypeName() const {
    return "grayWorldNormalization::parameters";
  }

  // copy member

  grayWorldNormalization::parameters&
    grayWorldNormalization::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    colorNormalizationBase::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    colorNormalizationBase::parameters& (colorNormalizationBase::parameters::* p_copy)
      (const colorNormalizationBase::parameters&) =
      colorNormalizationBase::parameters::copy;
    (this->*p_copy)(other);
# endif

    kappa = other.kappa;
    allowOverflow = other.allowOverflow;
    localNormalization = other.localNormalization;
    windowSize = other.windowSize;
    ignoreColorFlag = other.ignoreColorFlag;
    ignoreColor = other.ignoreColor;
    gamma = other.gamma;
    nu = other.nu;

    return *this;
  }

  // clone member
  functor::parameters* grayWorldNormalization::parameters::clone() const {
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
  bool grayWorldNormalization::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool grayWorldNormalization::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"kappa",kappa);
      lti::write(handler,"allowOverflow",allowOverflow);
      lti::write(handler,"localNormalization",localNormalization);
      lti::write(handler,"windowSize",windowSize);
      lti::write(handler,"ignoreColorFlag",ignoreColorFlag);
      lti::write(handler,"ignoreColor",ignoreColor);
      lti::write(handler,"gamma",gamma);
      lti::write(handler,"nu",nu);     
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && colorNormalizationBase::parameters::write(handler,false);
# else
    bool (colorNormalizationBase::parameters::* p_writeMS)(ioHandler&,const bool) const =
      colorNormalizationBase::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool grayWorldNormalization::parameters::write(ioHandler& handler,
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
  bool grayWorldNormalization::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool grayWorldNormalization::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      b = lti::read(handler,"kappa",kappa) && b;
      b = lti::read(handler,"allowOverflow",allowOverflow) && b;
      b = lti::read(handler,"localNormalization",localNormalization) && b;
      b = lti::read(handler,"windowSize",windowSize) && b;
      b = lti::read(handler,"ignoreColorFlag",ignoreColorFlag) && b;
      b = lti::read(handler,"ignoreColor",ignoreColor) && b;
      b = lti::read(handler,"gamma",gamma) && b;
      b = lti::read(handler,"nu",nu) && b;
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && colorNormalizationBase::parameters::read(handler,false);
# else
    bool (colorNormalizationBase::parameters::* p_readMS)(ioHandler&,const bool) =
      colorNormalizationBase::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool grayWorldNormalization::parameters::read(ioHandler& handler,
                                    const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // grayWorldNormalization
  // --------------------------------------------------

  // default constructor
  grayWorldNormalization::grayWorldNormalization()
    : colorNormalizationBase(){
    parameters param;
    setParameters(param);
  }

  // default constructor
  grayWorldNormalization::grayWorldNormalization(
                                const parameters& par)
    : colorNormalizationBase() {
    setParameters(par);
  }

  // copy constructor
  grayWorldNormalization::grayWorldNormalization(const grayWorldNormalization& other)
    : colorNormalizationBase()  {
    copy(other);
  }

  // destructor
  grayWorldNormalization::~grayWorldNormalization() {
  }

  // returns the name of this type
  const char* grayWorldNormalization::getTypeName() const {
    return "grayWorldNormalization";
  }

  // copy member
  grayWorldNormalization&
    grayWorldNormalization::copy(const grayWorldNormalization& other) {
      colorNormalizationBase::copy(other);
    return (*this);
  }

  // clone member
  functor* grayWorldNormalization::clone() const {
    return new grayWorldNormalization(*this);
  }

  // return parameters
  const grayWorldNormalization::parameters&
    grayWorldNormalization::getParameters() const {
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

  // On place apply for type image!
  bool grayWorldNormalization::apply(image& srcdest) const {

    if (srcdest.empty()) {
      setStatusString("input image empty");
      return false;
    }

    const parameters& param = getParameters();

    if (param.localNormalization) {
      lti::splitImageToRGB splitter;
      channel r,g,b;
      splitter.apply(srcdest,r,g,b);
      return apply(r,g,b,srcdest);
    } else {
      if (param.gamma == 0.0f) {
        // no normalization necessary
        return true;
      }

      trgbPixel<float> norm;

      computeNorm(srcdest,norm);
      normalize(srcdest,norm);
    }

    return true;
  };


  // On copy apply for type image!
  bool grayWorldNormalization::apply(const image& src,image& dest) const {

    if (src.empty()) {
      dest.clear();
      setStatusString("input image empty");
      return false;
    }

    const parameters& param = getParameters();

    if (param.localNormalization) {
      lti::splitImageToRGB splitter;
      channel r,g,b;
      splitter.apply(src,r,g,b);
      return apply(r,g,b,dest);
    } else {
      if (param.gamma == 0.0f) {
        // no normalization necessary
        dest.copy(src);
        return true;
      }

      trgbPixel<float> norm;

      computeNorm(src,norm);
      normalize(src,norm,dest);
    }

    return true;
  };

  bool grayWorldNormalization::apply(const channel& srcR,
                                     const channel& srcG,
                                     const channel& srcB,
                                     image& dest) const{
    lti::channel tempR;
    lti::channel tempG;
    lti::channel tempB;

    // apply normalization
    if (apply(srcR,srcG,srcB,tempR,tempG,tempB)) {
      // merge to image
      lti::mergeRGBToImage merger;
      merger.apply(tempR,tempG,tempB,dest);
      return true;
    }

    return false;
  };

  bool grayWorldNormalization::apply(const image& src,
                                     channel& chR,
                                     channel& chG,
                                     channel& chB) const{
    // Extract channels from image
    lti::splitImageToRGB splitter;
    splitter.apply(src, chR, chG, chB);
    // apply normalization
    return apply(chR,chG,chB);
  };

  bool grayWorldNormalization::apply(const channel& srcR,
                                     const channel& srcG,
                                     const channel& srcB,
                                     channel& chR,
                                     channel& chG,
                                     channel& chB) const{

    if ( (srcR.size() != srcG.size()) ||
         (srcG.size() != srcB.size()) ) {
      chR.clear();
      chG.clear();
      chB.clear();
      setStatusString("RGB channels have different sizes");
      return false;
    }

    // Get parameters
    const parameters& param = getParameters();

    if (param.localNormalization) {
      channel normR,normG,normB;
      computeNorm(srcR,srcG,srcB,normR,normG,normB);
      chR.edivide(srcR,normR);
      chG.edivide(srcG,normG);
      chB.edivide(srcB,normB);
    } else {
      if (param.gamma == 0.0f) {
        // no normalization desired
        chR.copy(srcR);
        chG.copy(srcG);
        chB.copy(srcB);
        return true;
      }

      // global invariance
      trgbPixel<float> norm;

      computeNorm(srcR,srcG,srcB,norm);

      // normalize
      if (param.allowOverflow) {
        chR.divide(srcR,norm.red);
        chG.divide(srcR,norm.green);
        chB.divide(srcB,norm.blue);
      } else {
        normalizeWithoutOverflow(srcR,norm.red,chR);
        normalizeWithoutOverflow(srcG,norm.green,chB);
        normalizeWithoutOverflow(srcG,norm.blue,chB);
      }
    }
    return true;
  };

  bool grayWorldNormalization::apply(channel& srcdestR,
                                     channel& srcdestG,
                                     channel& srcdestB) const {

    if ( (srcdestR.size() != srcdestG.size()) ||
         (srcdestG.size() != srcdestB.size()) ) {
      srcdestR.clear();
      srcdestG.clear();
      srcdestB.clear();
      setStatusString("RGB channels have different sizes");
      return false;
    }

    const parameters& param = getParameters();

    if (param.localNormalization) {
      channel normR,normG,normB;
      computeNorm(srcdestR,srcdestG,srcdestB,normR,normG,normB);
      srcdestR.edivide(normR);
      srcdestG.edivide(normG);
      srcdestB.edivide(normB);
    } else {
      if (param.gamma == 0.0f) {
        // no normalization desired
        return true;
      }

      trgbPixel<float> norm;

      computeNorm(srcdestR,srcdestG,srcdestB,norm);

      const int rows = srcdestR.rows();

      // Get parameters
      const parameters& params = getParameters();
      int i;

      lti::vector<lti::channel::value_type>::iterator itR,itG,itB;
      lti::vector<lti::channel::value_type>::iterator itEnd;

      if (params.allowOverflow) {
        for(i=0; i<rows; i++) {
          itR = srcdestR.getRow(i).begin();
          itG = srcdestG.getRow(i).begin();
          itB = srcdestB.getRow(i).begin();
          itEnd = srcdestR.getRow(i).end();

          while(itR!=itEnd) {
            (*itR)/=norm.red;
            (*itG)/=norm.green;
            (*itB)/=norm.blue;

            ++itR;++itG;++itB;
          }
        }

      } else {
        for(i=0; i<rows; i++) {
          itR = srcdestR.getRow(i).begin();
          itG = srcdestG.getRow(i).begin();
          itB = srcdestB.getRow(i).begin();
          itEnd = srcdestR.getRow(i).end();

          while(itR!=itEnd) {
            (*itR) = inChannelRange((*itR)/norm.red);
            (*itG) = inChannelRange((*itG)/norm.green);
            (*itB) = inChannelRange((*itB)/norm.blue);

            ++itR;++itG;++itB;
          }
        }
      }
    }
    return true;
  };

  /*
   * normalize image
   */
  void grayWorldNormalization::normalize(image& img,
                                         const trgbPixel<float>& norm) const {
    vector<image::value_type>::iterator it,eit;
    int y;
    float nr,ng,nb;

    nr = 255.0f/norm.red;
    ng = 255.0f/norm.green;
    nb = 255.0f/norm.blue;
    const parameters& param = getParameters();

    if (param.allowOverflow) {
      for (y=0;y<img.rows();y++) {
        for (it=img.getRow(y).begin(),eit=img.getRow(y).end();
             it!=eit;
             ++it) {
          rgbPixel& p = (*it);
          p.set(static_cast<ubyte>(nr*p.getRed()),
                static_cast<ubyte>(ng*p.getGreen()),
                static_cast<ubyte>(nb*p.getBlue()),
                0);
        }
      }
    } else {
      for (y=0;y<img.rows();y++) {
        for (it=img.getRow(y).begin(),eit=img.getRow(y).end();
             it!=eit;
             ++it) {
          rgbPixel& p = (*it);
          p.set(inChannel8Range(nr*p.getRed()),
                inChannel8Range(ng*p.getGreen()),
                inChannel8Range(nb*p.getBlue()),
                0);
        }
      }
    }
  }

  /*
   * normalize image
   */
  void grayWorldNormalization::normalize(const image& img,
                                         const trgbPixel<float>& norm,
                                         image& dest) const {

    dest.resize(img.size(),rgbPixel(),false,false);

    vector<image::value_type>::const_iterator cit,ceit;
    vector<image::value_type>::iterator it;

    int y;
    float nr,ng,nb;

    nr = 255.0f/norm.red;
    ng = 255.0f/norm.green;
    nb = 255.0f/norm.blue;

    const parameters& param = getParameters();

    if (param.allowOverflow) {
      for (y=0;y<img.rows();y++) {
        for (cit=img.getRow(y).begin(),ceit=img.getRow(y).end(),
               it=dest.getRow(y).begin();
             cit!=ceit;
             ++it,++cit) {
          const rgbPixel& p = (*cit);
          (*it).set(static_cast<ubyte>(nr*p.getRed()),
                    static_cast<ubyte>(ng*p.getGreen()),
                    static_cast<ubyte>(nb*p.getBlue()),
                    0);
        }
      }
    } else {
      for (y=0;y<img.rows();y++) {
        for (cit=img.getRow(y).begin(),ceit=img.getRow(y).end(),
               it=dest.getRow(y).begin();
             cit!=ceit;
             ++it,++cit) {
          const rgbPixel& p = (*cit);
          (*it).set(inChannel8Range(nr*p.getRed()),
                    inChannel8Range(ng*p.getGreen()),
                    inChannel8Range(nb*p.getBlue()),
                    0);
        }
      }
    }
  }


  void grayWorldNormalization::computeNorm(const channel& srcR,
                                           const channel& srcG,
                                           const channel& srcB,
                                           trgbPixel<float>& norm) const {

    trgbPixel<float> sum(0,0,0),sum2(0,0,0);

    const parameters& param = getParameters();
    int y,n;

    lti::vector<lti::channel::value_type>::const_iterator itR,itG,itB;
    lti::vector<lti::channel::value_type>::const_iterator itEnd;

    if (param.ignoreColorFlag) {
      n = 0;
      float r,g,b;
      r = float(param.ignoreColor.getRed())/255.0f;
      g = float(param.ignoreColor.getGreen())/255.0f;
      b = float(param.ignoreColor.getBlue())/255.0f;

      // first, compute the norm
      for (y=0;y<srcR.rows();y++) {
        for (itR = srcR.getRow(y).begin(),itEnd = srcR.getRow(y).end(),
               itG = srcG.getRow(y).begin(), itB = srcB.getRow(y).begin();
             itR != itEnd;
             ++itR,++itG,++itB) {

          if (((*itR) != r) || (*itG != g) || (*itB != b)) {
            sum.red += (*itR);
            sum.green += (*itG);
            sum.blue += (*itB);

            sum2.red += (*itR)*(*itR);
            sum2.green += (*itG)*(*itG);
            sum2.blue += (*itB)*(*itB);

            ++n;
          }
        }
      }
    } else {
      // first, compute the norm
      for (y=0;y<srcR.rows();y++) {
        for (itR = srcR.getRow(y).begin(),itEnd = srcR.getRow(y).end(),
               itG = srcG.getRow(y).begin(), itB = srcB.getRow(y).begin();
             itR != itEnd;
             ++itR,++itG,++itB) {
          sum.red += (*itR);
          sum.green += (*itG);
          sum.blue += (*itB);

          sum2.red += (*itR)*(*itR);
          sum2.green += (*itG)*(*itG);
          sum2.blue += (*itB)*(*itB);
        }
      }

      n = srcR.rows()*srcR.columns();
    }

    if (n>1) {
      // the average in norm
      norm.divide(sum,float(n));

      // compute the empirical std. deviation;
      trgbPixel<float> var;
      var.red   = sqrt((sum2.red   - (sum.red*sum.red)/n)/(n-1));
      var.green = sqrt((sum2.green - (sum.green*sum.green)/n)/(n-1));
      var.blue  = sqrt((sum2.blue  - (sum.blue*sum.blue)/n)/(n-1));

      // multiply the std. deviation with the given factor
      var.multiply(param.kappa);
      norm.multiply(param.nu);
      norm.add(var);

    } else {
      norm.copy(sum);
    }

    // now we have the best norm: norm = nu*avrg + kappa*stdDev
    // but the user may wish to use a "weaker" norm
    // We use gamma for this purpose: norm = gamma*norm + (1-gamma)

    norm.multiply(param.gamma);
    norm.add(1.0f-param.gamma);

  }

  void grayWorldNormalization::computeNorm(const image& img,
                                           trgbPixel<float>& norm) const {

    trgbPixel<float> sum(0,0,0),sum2(0,0,0);

    const parameters& param = getParameters();

    lti::vector<lti::image::value_type>::const_iterator it,itEnd;
    float r,g,b;
    int y,n;

    if (param.ignoreColorFlag) {
      const rgbPixel px = param.ignoreColor;
      n = 0;
      // first, compute the norm
      for (y=0;y<img.rows();y++) {
        for (it = img.getRow(y).begin(),itEnd = img.getRow(y).end();
             it != itEnd;
             ++it) {

          if ((*it) != px) {
            r = (*it).getRed();
            g = (*it).getGreen();
            b = (*it).getBlue();

            sum.red   += r;
            sum.green += g;
            sum.blue  += b;

            sum2.red += r*r;
            sum2.green += g*g;
            sum2.blue += b*b;

            ++n;
          }
        }
      }
    } else {
      // first, compute the norm
      for (y=0;y<img.rows();y++) {
        for (it = img.getRow(y).begin(),itEnd = img.getRow(y).end();
           it != itEnd;
             ++it) {
          r = (*it).getRed();
          g = (*it).getGreen();
          b = (*it).getBlue();

          sum.red   += r;
          sum.green += g;
          sum.blue  += b;

          sum2.red += r*r;
          sum2.green += g*g;
          sum2.blue += b*b;
        }
      }

      n = img.rows()*img.columns();
    }

    if (n>1) {
      // the average in norm
      norm.divide(sum,float(n));

      // compute the empirical std. deviation;
      trgbPixel<float> var;
      var.red   = sqrt((sum2.red   - (sum.red*sum.red)/n)/(n-1));
      var.green = sqrt((sum2.green - (sum.green*sum.green)/n)/(n-1));
      var.blue  = sqrt((sum2.blue  - (sum.blue*sum.blue)/n)/(n-1));

      // multiply the std. deviation with the given factor
      var.multiply(param.kappa);
      norm.multiply(param.nu);
      norm.add(var);
    } else {
      // the average in norm
      norm.copy(sum);
    }

    // now we have the best norm: norm = nu*avrg + kappa*stdDev
    // but the user may wish to use a "weaker" norm
    // We use gamma for this purpose: norm = gamma*norm + (1-gamma)

    norm.multiply(param.gamma);
    norm.add(255.0f*(1.0f-param.gamma));

  }

  void grayWorldNormalization::computeNorm(const channel& chnl,
                                           channel& norm) const {

    const parameters& param = getParameters();

    channel var;

    squareConvolution<float> convolver;
    squareConvolution<float>::parameters convParam;
    convParam.initSquare(param.windowSize);
    convParam.boundaryType = lti::Mirror;
    convolver.setParameters(convParam);

    // compute average and leave it in the "norm" channel
    convolver.apply(chnl,norm);

    // compute the average of the square
    var.emultiply(chnl,chnl);
    convolver.apply(var);

    // with variance = sqrt(
    int y;
    const float gamma = param.gamma;
    const float cgamma = 1.0f-param.gamma;

    vector<float>::iterator it,it2,eit;
    if (gamma < 1.0f) {
      for (y=0;y<norm.rows();++y) {
        for (it=norm.getRow(y).begin(),it2=var.getRow(y).begin(),
               eit=norm.getRow(y).end();
             it!=eit;
             ++it,++it2) {
          // compute the variance (var = avrg(x^2) - avrg(x)^2)
          (*it2) -= (*it)*(*it);
          (*it)*=param.nu;
          (*it)+=param.kappa*sqrt(*it2);

          // now make this norm weaker...
          (*it)*=gamma;
          (*it)+=cgamma;
        }
      }
    } else {
      for (y=0;y<norm.rows();++y) {
        for (it=norm.getRow(y).begin(),it2=var.getRow(y).begin(),
               eit=norm.getRow(y).end();
             it!=eit;
             ++it,++it2) {
          (*it2) -= (*it)*(*it);
          (*it)*=param.nu;
          (*it)+=param.kappa*sqrt(*it2);
        }
      }
    }
  }

  void grayWorldNormalization::computeNorm(const channel& red,
                                           const channel& green,
                                           const channel& blue,
                                           channel& normRed,
                                           channel& normGreen,
                                           channel& normBlue) const {
    computeNorm(red,normRed);
    computeNorm(green,normGreen);
    computeNorm(blue,normBlue);
  }


  /**
   * normalize avoiding overflows
   */
  void 
  grayWorldNormalization::normalizeWithoutOverflow(const channel& input,
                                                   const float& norm,
                                                   channel& output) const {

    output.resize(input.size(),0.0f,false,false);

    lti::vector<lti::channel::value_type>::const_iterator cit,citEnd;
    lti::vector<lti::channel::value_type>::iterator it;
    int y;

    // first, compute the norm
    for (y=0;y<input.rows();y++) {
      const vector<lti::channel::value_type> &vct = input.getRow(y);
      for (cit = vct.begin(),citEnd = vct.end(),it = output.getRow(y).begin();
           cit != citEnd;
           ++cit,++it) {
        (*it) = inChannelRange((*cit)/norm);
      }
    }
  }
}
