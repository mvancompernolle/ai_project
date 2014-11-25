/*
 * Copyright (C) 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiColorModelEstimator.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 12.3.2002
 * revisions ..: $Id: ltiColorModelEstimator.cpp,v 1.13 2006/09/05 10:06:26 ltilib Exp $
 */

#include "ltiColorModelEstimator.h"

namespace lti {
  // --------------------------------------------------
  // colorModelEstimator::parameters
  // --------------------------------------------------

  // default constructor
  colorModelEstimator::parameters::parameters()
    : statisticsFunctor::parameters() {

    histogramDimensions = rgbPixel(32,32,32);
  }

  // copy constructor
  colorModelEstimator::parameters::parameters(const parameters& other)
    : statisticsFunctor::parameters()  {
    copy(other);
  }

  // destructor
  colorModelEstimator::parameters::~parameters() {
  }

  // get type name
  const char* colorModelEstimator::parameters::getTypeName() const {
    return "colorModelEstimator::parameters";
  }

  // copy member

  colorModelEstimator::parameters&
    colorModelEstimator::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    statisticsFunctor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    statisticsFunctor::parameters& (statisticsFunctor::parameters::* p_copy)
      (const statisticsFunctor::parameters&) =
      statisticsFunctor::parameters::copy;
    (this->*p_copy)(other);
# endif


      histogramDimensions = other.histogramDimensions;

    return *this;
  }

  // alias for copy member
  colorModelEstimator::parameters&
    colorModelEstimator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* colorModelEstimator::parameters::clone() const {
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
  bool colorModelEstimator::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool colorModelEstimator::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"histogramDimensions",histogramDimensions);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && statisticsFunctor::parameters::write(handler,false);
# else
    bool (statisticsFunctor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      statisticsFunctor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool colorModelEstimator::parameters::write(ioHandler& handler,
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
  bool colorModelEstimator::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool colorModelEstimator::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"histogramDimensions",histogramDimensions);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && statisticsFunctor::parameters::read(handler,false);
# else
    bool (statisticsFunctor::parameters::* p_readMS)(ioHandler&,const bool) =
      statisticsFunctor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool colorModelEstimator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // colorModelEstimator
  // --------------------------------------------------

  // default constructor
  colorModelEstimator::colorModelEstimator()
    : statisticsFunctor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // copy constructor
  colorModelEstimator::colorModelEstimator(const colorModelEstimator& other)
    : statisticsFunctor()  {
    copy(other);
  }

  // destructor
  colorModelEstimator::~colorModelEstimator() {
  }

  // returns the name of this type
  const char* colorModelEstimator::getTypeName() const {
    return "colorModelEstimator";
  }

  // copy member
  colorModelEstimator&
  colorModelEstimator::copy(const colorModelEstimator& other) {
    statisticsFunctor::copy(other);
    
    theModel.copy(other.theModel);

    index.copy(other.index);
    hdimRed = other.hdimRed;
    hdimGreen = other.hdimGreen;
    hdimBlue = other.hdimBlue;

    lastIdxRed=hdimRed-1;
    lastIdxGreen=hdimGreen-1;
    lastIdxBlue=hdimBlue-1;

    return (*this);
  }

  // alias for copy member
  colorModelEstimator&
    colorModelEstimator::operator=(const colorModelEstimator& other) {
    return (copy(other));
  }


  // clone member
  functor* colorModelEstimator::clone() const {
    return new colorModelEstimator(*this);
  }

  /*
   * set functor's parameters.
   * This member makes a copy of <em>theParam</em>: the functor
   * will keep its own copy of the parameters!
   *
   * This overloading will initialize the size of the internal color model
   * @return true if successful, false otherwise
   */
  bool colorModelEstimator::updateParameters() {
    reset();
    return true;
  }


  // return parameters
  const colorModelEstimator::parameters&
    colorModelEstimator::getParameters() const {
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

  /*
   * reset the internal histogram to consider new elements.
   * The internal model will be empty after this.  The first "consider()"
   * will take the desired dimensions from the parameters and will initialize
   * the internal histogram with the proper size.
   */
  bool colorModelEstimator::reset() {
    const parameters& param = getParameters();
    hdimRed = param.histogramDimensions.getRed();
    hdimGreen = param.histogramDimensions.getGreen();
    hdimBlue = param.histogramDimensions.getBlue();

    lastIdxRed=hdimRed-1;
    lastIdxGreen=hdimGreen-1;
    lastIdxBlue=hdimBlue-1;

    theModel.clear();
    ivector dims(3);
    dims.at(0)=hdimRed;
    dims.at(1)=hdimGreen;
    dims.at(2)=hdimBlue;
    theModel.resize(3,dims);

    index.resize(3,0,false,true);

    n = 0;
    sumOuter.resize(3,3,0.0,false,true);
    sum = trgbPixel<double>(0.0,0.0,0.0);

    return true;
  }

  /*
   * consider all pixels in the image.  The first call to this member (or
   * after a reset()) will initialize the size of the internal model.
   * @param img image with colors to be considered
   * @return true if successful or false otherwise.
   */
  bool colorModelEstimator::consider(const rgbPixel& pix,
                                     const int times) {
    index.at(0)=static_cast<int>(pix.getRed()*hdimRed/256);
    index.at(1)=static_cast<int>(pix.getGreen()*hdimGreen/256);
    index.at(2)=static_cast<int>(pix.getBlue()*hdimBlue/256);
    theModel.put(index,times);

    ++n;
    trgbPixel<double> p(pix);
    sum.add(p);

    // outer product of color is symmetric => consider only the upper
    // triangular matrix, and when asking for the model compute the
    // complete one...
    omit = sumOuter.begin();

    (*omit) += p.red*p.red;
    ++omit;
    (*omit) += p.red*p.green;
    ++omit;
    (*omit) += p.red*p.blue;
    omit+=2;
    (*omit) += p.green*p.green;
    ++omit;
    (*omit) += p.green*p.blue;
    omit+=3;
    (*omit) += p.blue*p.blue;

    return true;
  }

  /*
   * consider all pixels in the image.  The first call to this member (or
   * after a reset()) will initialize the size of the internal model.
   * @param img image with colors to be considered
   * @return true if successful or false otherwise.
   */
  bool colorModelEstimator::smoothConsider(const rgbPixel& pix,
                                           const int times) {

    int fr,tr,fg,tg,fb,tb;
    double qr,qg,qb;
    int t,q;

    t=static_cast<int>(pix.getRed()*hdimRed);    
    fr = t/256;    
    q = (t - fr*256 - 128);
    if (q < 0) {
      tr = fr;
      fr = max(0,tr-1);
      qr = (q+256)/256.0;
    } else {
      tr = fr+1;
      if (tr==hdimRed) {
        tr--;
      }
      qr = q/256.0;
    }

    t=static_cast<int>(pix.getGreen()*hdimGreen);    
    fg = t/256;
    q = (t-fg*256 - 128);
    if (q < 0) {
      tg = fg;
      fg = max(0,tg-1);
      qg = (q+256)/256.0;
    } else {
      tg = fg+1;
      if (tg==hdimGreen) {
        tg--;
      }
      qg = q/256.0;
    }
    
    t=static_cast<int>(pix.getBlue()*hdimBlue);    
    fb = t/256;
    q = (t-fb*256 - 128);
    if (q < 0) {
      tb=fb;
      fb=max(0,tb-1);
      qb = (q+256)/256.0;
    } else {
      tb=fb+1;
      if (tb==hdimBlue) {
        tb--;
      }
      qb = q/256.0;
    }

    // now interpolate the eight values (using a grey code to change only one
    // index element at a time)
    index.at(0)=fr;
    index.at(1)=fg;
    index.at(2)=fb;
    theModel.put(index,times*(1.0-qr)*(1.0-qg)*(1.0-qb));
    
    index.at(0)=tr;
    theModel.put(index,times*qr*(1.0-qg)*(1.0-qb));
    
    index.at(1)=tg;
    theModel.put(index,times*qr*qg*(1.0-qb));

    index.at(0)=fr;
    theModel.put(index,times*(1.0-qr)*qg*(1.0-qb));

    index.at(2)=tb;
    theModel.put(index,times*(1.0-qr)*qg*qb);

    index.at(0)=tr;
    theModel.put(index,times*qr*qg*qb);
    
    index.at(1)=fg;
    theModel.put(index,times*qr*(1.0-qg)*qb);

    index.at(0)=fr;
    theModel.put(index,times*(1.0-qr)*(1.0-qg)*qb);
    
    // the gaussian model
    ++n;
    trgbPixel<double> p(pix);
    sum.add(p);

    // outer product of color is symmetric => consider only the upper
    // triangular matrix, and when asking for the model compute the
    // complete one...
    omit = sumOuter.begin();

    (*omit) += p.red*p.red;
    ++omit;
    (*omit) += p.red*p.green;
    ++omit;
    (*omit) += p.red*p.blue;
    omit+=2;
    (*omit) += p.green*p.green;
    ++omit;
    (*omit) += p.green*p.blue;
    omit+=3;
    (*omit) += p.blue*p.blue;

    return true;
  }


  /*
   * consider all pixels in the image.  The first call to this member (or
   * after a reset()) will initialize the size of the internal model.
   * @param img image with colors to be considered
   * @return true if successful or false otherwise.
   */
  bool colorModelEstimator::consider(const float& c1,
                                     const float& c2,
                                     const float& c3,
                                     const int times) {
    index.at(0)=iround(c1*lastIdxRed);
    index.at(1)=iround(c2*lastIdxGreen);
    index.at(2)=iround(c3*lastIdxBlue);
    theModel.put(index,times);

    ++n;
    trgbPixel<double> p(c1,c2,c3);
    sum.add(p);

    // outer product of color is symmetric => consider only the upper
    // triangular matrix, and when asking for the model compute the
    // complete one...
    omit = sumOuter.begin();

    (*omit) += p.red*p.red;
    ++omit;
    (*omit) += p.red*p.green;
    ++omit;
    (*omit) += p.red*p.blue;
    omit+=2;
    (*omit) += p.green*p.green;
    ++omit;
    (*omit) += p.green*p.blue;
    omit+=3;
    (*omit) += p.blue*p.blue;

    return true;
  }


  /*
   * consider a pixel for the model.
   * @param pal the color palette to be considered
   * @return true if successful or false otherwise.
   */
  bool colorModelEstimator::consider(const palette& pal) {
    palette::const_iterator it,eit;
    for (it=pal.begin(),eit=pal.end();it!=eit;++it) {
      consider(*it);
    }
    return true;
  }


  /*
   * consider all pixels in the image.  The first call to this member (or
   * after a reset()) will initialize the size of the internal model.
   * @param img image with colors to be considered
   * @return true if successful or false otherwise.
   */
  bool colorModelEstimator::consider(const image& img) {
    vector<image::value_type>::const_iterator it,eit;
    int y;

    for (y=0;y<img.rows();++y) {
      const vector<image::value_type>& vct=img.getRow(y);
      for (it=vct.begin(),eit=vct.end();it!=eit;++it) {
        consider(*it);
      }
    }

    return true;
  }

  /*
   * consider all pixels in the image.  The first call to this member (or
   * after a reset()) will initialize the size of the internal model.
   * @param img image with colors to be considered
   * @return true if successful or false otherwise.
   */
  bool colorModelEstimator::consider(const channel& c1,
                                     const channel& c2,
                                     const channel& c3) {

    if ((c1.size() != c2.size()) ||
        (c1.size() != c3.size())) {
      setStatusString("Channels have different sizes");
      return false;
    }

    int y,x;

    for (y=0;y<c1.rows();++y) {
      for (x=0;x<c1.columns();++x) {
        consider(c1.at(y,x),c2.at(y,x),c3.at(y,x));
      }
    }

    return true;
  }


  /*
   * consider all pixels in the image.  The first call to this member (or
   * after a reset()) will initialize the size of the internal model.
   * @param img image with colors to be considered
   * @return true if successful or false otherwise.
   */
  bool colorModelEstimator::consider(const image& img,
                                     const channel8& mask) {

    if (img.size() != mask.size()) {
      setStatusString("Image and mask must have the same size");
      return false;
    }

    vector<image::value_type>::const_iterator it,eit;
    vector<channel8::value_type>::const_iterator mit;
    int y;

    for (y=0;y<img.rows();++y) {
      const vector<image::value_type>& vct=img.getRow(y);
      for (it=vct.begin(),eit=vct.end(),mit=mask.getRow(y).begin();
           it!=eit;
           ++it,++mit) {
        if ((*mit) != 0) {
          consider(*it);
        }
      }
    }

    return true;
  }

  /*
   * consider all pixels in the image.  The first call to this member (or
   * after a reset()) will initialize the size of the internal model.
   * @param img image with colors to be considered
   * @return true if successful or false otherwise.
   */
  bool colorModelEstimator::consider(const image& img,
                                     const imatrix& mask) {

    if (img.size() != mask.size()) {
      setStatusString("Image and mask must have the same size");
      return false;
    }

    vector<image::value_type>::const_iterator it,eit;
    ivector::const_iterator mit;
    int y;

    for (y=0;y<img.rows();++y) {
      const vector<image::value_type>& vct=img.getRow(y);
      for (it=vct.begin(),eit=vct.end(),mit=mask.getRow(y).begin();
           it!=eit;
           ++it,++mit) {
        if ((*mit) != 0) {
          consider(*it);
        }
      }
    }

    return true;
  }

  bool colorModelEstimator::consider(const channel& c1,
                                     const channel& c2,
                                     const channel& c3,
                                     const channel8& mask) {

    if ((c1.size() != c2.size()) ||
        (c1.size() != c3.size()) ||
        (c1.size() != mask.size())) {
      setStatusString("Channels have different sizes");
      return false;
    }

    int y,x;

    for (y=0;y<c1.rows();++y) {
      for (x=0;x<c1.columns();++x) {
        if (mask.at(y,x) != 0)
        consider(c1.at(y,x),c2.at(y,x),c3.at(y,x));
      }
    }

    return true;
  }

  bool colorModelEstimator::consider(const channel& c1,
                                     const channel& c2,
                                     const channel& c3,
                                     const imatrix& mask) {

    if ((c1.size() != c2.size()) ||
        (c1.size() != c3.size()) ||
        (c1.size() != mask.size())) {
      setStatusString("Channels have different sizes");
      return false;
    }

    int y,x;

    for (y=0;y<c1.rows();++y) {
      for (x=0;x<c1.columns();++x) {
        if (mask.at(y,x) != 0)
        consider(c1.at(y,x),c2.at(y,x),c3.at(y,x));
      }
    }

    return true;
  }


  bool colorModelEstimator::consider(const image& img,
                                     const channel8& mask,
                                     const ubyte label) {

    if (img.size() != mask.size()) {
      setStatusString("Image and mask must have the same size");
      return false;
    }

    vector<image::value_type>::const_iterator it,eit;
    vector<channel8::value_type>::const_iterator mit;
    int y;

    for (y=0;y<img.rows();++y) {
      const vector<image::value_type>& vct=img.getRow(y);
      for (it=vct.begin(),eit=vct.end(),mit=mask.getRow(y).begin();
           it!=eit;
           ++it,++mit) {
        if ((*mit) == label) {
          consider(*it);
        }
      }
    }

    return true;
  }

  bool colorModelEstimator::consider(const image& img,
                                     const imatrix& mask,
                                     const int label) {

    if (img.size() != mask.size()) {
      setStatusString("Image and mask must have the same size");
      return false;
    }

    vector<image::value_type>::const_iterator it,eit;
    vector<int>::const_iterator mit;
    int y;

    for (y=0;y<img.rows();++y) {
      const vector<rgbPixel>& vct=img.getRow(y);
      for (it=vct.begin(),eit=vct.end(),mit=mask.getRow(y).begin();
           it!=eit;
           ++it,++mit) {
        if ((*mit) == label) {
          consider(*it);
        }
      }
    }

    return true;
  }


  /*
   * consider all the images in the given list
   *
   * @param imgs list of images to be considered
   * @return true if successful or false otherwise.
   */
  bool colorModelEstimator::consider(const std::list<image>& imgs) {
    std::list<image>::const_iterator it;
    for (it=imgs.begin();it!=imgs.end();++it) {
      consider(*it);
    }
    return true;
  }

  /*
   * consider all the images in the given list.  The mask list must have
   * the same size, and the corresponding images must also have the same
   * size.
   */
  bool colorModelEstimator::consider(const std::list<image>& imgs,
                                     const std::list<channel8>& masks) {
    std::list<image>::const_iterator it;
    std::list<channel8>::const_iterator mit;
    if (imgs.size() != masks.size()) {
      setStatusString("lists of masks and images have different sizes.");
      return false;
    }

    for (it=imgs.begin(),mit=masks.begin();it!=imgs.end();++it,++mit) {
      consider(*it,*mit);
    }

    return true;
  }

  /*
   * get a constant reference to the internal model
   */
  const thistogram<double>& colorModelEstimator::getColorModel() {
    return theModel;
  }

  /*
   * get a copy of the internal model in the given %parameter.
   * @param model the color model (as 3D histogram) computed until now
   * @return true if apply successful or false otherwise.
   */
  bool colorModelEstimator::apply(thistogram<double>& model) const {
    model.copy(theModel);
    return true;
  }

  /*
   * get the mean color and covariance matrix of the internal model
   * @param mean the mean color
   * @param covar covariance matrix of the color (3x3 matrix)
   * @return true if apply successful, false otherwise.
   */
  bool colorModelEstimator::apply(trgbPixel<double>& mean,
                                  dmatrix& covar) const {

    mean.copy(sum);
    if (n<2) {
      covar.resize(3,3,0.0,false,true);
      return true;
    }

    mean.divide(n);

    dmatrix a;
    a.resize(3,3,0,false,false);
    outer(mean,mean,a);

    covar.copy(sumOuter);
    // complete the outer product sum (till now only the upper triangular mat.)
    covar.at(1,0)=covar.at(0,1);
    covar.at(2,0)=covar.at(0,2);
    covar.at(2,1)=covar.at(1,2);

    covar.addScaled(-n,a);
    covar.divide(n-1);

    return true;
  }

  /*
   * outer product of two colors
   */
  void colorModelEstimator::outer(const trgbPixel<double>& a,
                                  const trgbPixel<double>& b,
                                  dmatrix& mat) const {
    dmatrix::iterator it = mat.begin();
    (*it)=a.red*b.red;
    ++it;
    (*it)=a.red*b.green;
    ++it;
    (*it)=a.red*b.blue;
    ++it;

    (*it)=a.green*b.red;
    ++it;
    (*it)=a.green*b.green;
    ++it;
    (*it)=a.green*b.blue;
    ++it;

    (*it)=a.blue*b.red;
    ++it;
    (*it)=a.blue*b.green;
    ++it;
    (*it)=a.blue*b.blue;
  }



}
