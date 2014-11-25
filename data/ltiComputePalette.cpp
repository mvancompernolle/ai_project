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
 * file .......: ltiComputePalette.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 11.3.2002
 * revisions ..: $Id: ltiComputePalette.cpp,v 1.17 2006/09/22 17:02:03 ltilib Exp $
 */

#include "ltiComputePalette.h"

#undef _LTI_DEBUG
// #define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // computePalette::parameters
  // --------------------------------------------------

  // default constructor
  computePalette::parameters::parameters()
    : functor::parameters() {

  }

  // copy constructor
  computePalette::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  computePalette::parameters::~parameters() {
  }

  // get type name
  const char* computePalette::parameters::getTypeName() const {
    return "computePalette::parameters";
  }

  // copy member

  computePalette::parameters&
    computePalette::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters& (functor::parameters::* p_copy)
      (const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
# endif



    return *this;
  }

  // alias for copy member
  computePalette::parameters&
    computePalette::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* computePalette::parameters::clone() const {
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
  bool computePalette::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool computePalette::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::write(handler,false);
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
  bool computePalette::parameters::write(ioHandler& handler,
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
  bool computePalette::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool computePalette::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::read(handler,false);
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
  bool computePalette::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // computePalette
  // --------------------------------------------------

  // default constructor
  computePalette::computePalette()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // copy constructor
  computePalette::computePalette(const computePalette& other)
    : functor()  {
    copy(other);
  }

  // destructor
  computePalette::~computePalette() {
  }

  // returns the name of this type
  const char* computePalette::getTypeName() const {
    return "computePalette";
  }

  // copy member
  computePalette& computePalette::copy(const computePalette& other) {
    functor::copy(other);
    
    return (*this);
  }

  // alias for copy member
  computePalette&
    computePalette::operator=(const computePalette& other) {
    return (copy(other));
  }


  // clone member
  functor* computePalette::clone() const {
    return new computePalette(*this);
  }

  // return parameters
  const computePalette::parameters&
    computePalette::getParameters() const {
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


  /**
   * operates on a copy of the given %parameters.
   * @param src image with the source data.
   * @param mask mask with palette entry indices.
   * @param dest computed palette.
   * @return true if apply successful or false otherwise.
   */
  bool computePalette::apply(const image& src,
                             const channel8& mask,
                             palette& dest) const {
    vector<trgbPixel<float> > stdDev;
    return apply(src,mask,dest,stdDev);
  }

  /**
   * operates on a copy of the given %parameters.
   * @param src image with the source data.
   * @param mask mask with palette entry indices.
   * @param dest computed palette.
   * @return true if apply successful or false otherwise.
   */
  bool computePalette::apply(const image& src,
                             const matrix<int>& mask,
                             palette& dest) const {
    vector<trgbPixel<float> > stdDev;
    return apply(src,mask,dest,stdDev);
  }

  bool computePalette::apply(const image& src,
                             const matrix<int>& mask,
                             palette& dest,
                             vector<trgbPixel<float> >& palStdDev) const {
    vector<int> n;
    return apply(src,mask,dest,palStdDev,n);
  }

  bool computePalette::apply(const image& src,
                             const matrix<int>& mask,
                             vector<trgbPixel<float> >& means,
                             ivector& n,
                             const int numLabels) const {

    if (src.size() != mask.size()) {
      setStatusString("Labeled mask and image must have the same size");
      return false;
    }

    const int labels = (numLabels < 0) ? mask.maximum()+1 : numLabels;
    means.resize(labels,trgbPixel<float>(0,0,0),false,true);
    n.resize(labels,0,false,true);
    
    int y,x,l;
    for (y=0;y<src.rows();++y) {
      for (x=0;x<src.columns();++x) {
        l = mask.at(y,x);
        means.at(l)+=src.at(y,x);
        n.at(l)++;
      }
    }

    // compute the mean values
    for (l=0;l<labels;++l) {
      if (n.at(l)>0) {
        means.at(l)/=static_cast<float>(n.at(l));
      }
    }

    return true;
  }

  bool computePalette::apply(const image& src,
                             const channel8& mask,
                             palette& dest,
                             vector<trgbPixel<float> >& palStdDev) const {
    vector<int> n;
    return apply(src,mask,dest,palStdDev,n);
  }

  /**
   * operates on a copy of the given %parameters.
   * @param src image with the source data.
   * @param mask mask with palette entry indices.
   * @param dest computed palette (mean value of all elements with the
   *                               same index).
   * @param palStdDev standard deviation for each palette entry.
   * @return true if apply successful or false otherwise.
   */
  bool computePalette::apply(const image& src,
                             const channel8& mask,
                             palette& dest,
                             vector<trgbPixel<float> >& palStdDev,
                             vector<int>& n) const {

    if (src.size() != mask.size()) {
      setStatusString("Source image and index mask have different sizes");
      return false;
    }

    vector<trgbPixel<float> > sum2(256,trgbPixel<float>(0,0,0));
    vector<trgbPixel<float> >  sum(256,trgbPixel<float>(0,0,0));
    n.resize(256,0,false,true);

    int y,i,m,maxi(0);
    vector<rgbPixel>::const_iterator sit,eit;
    vector<channel8::value_type>::const_iterator cit;
    trgbPixel<float> px;

    for (y=0;y<src.rows();y++) {
      const vector<rgbPixel>& vct=src.getRow(y);
      const vector<channel8::value_type>& idx=mask.getRow(y);
      for (sit=vct.begin(),eit=vct.end(),cit=idx.begin();
           sit!=eit;
           ++sit,++cit) {
        i = *cit;
        maxi=max(i,maxi);
        n.at(i)++;
        px.copy(*sit);
        sum.at(i).add(px);
        sum2.at(i).add(px*px);
      }
    }

    maxi++;
    dest.resize(maxi,Black,false,false);
    palStdDev.resize(maxi,trgbPixel<float>(),false,false);

    for (i=0;i<maxi;++i) {
      m = n.at(i);
      const trgbPixel<float>& s = sum.at(i);
      const trgbPixel<float>& s2 = sum2.at(i);

      dest.at(i).setRed(static_cast<ubyte>(s.red/m));
      dest.at(i).setGreen(static_cast<ubyte>(s.green/m));
      dest.at(i).setBlue(static_cast<ubyte>(s.blue/m));
      dest.at(i).setAlpha(0);

      if (m>1) {
        palStdDev.at(i).setRed(sqrt((s2.red-(s.red*s.red/m))/(m-1)));
        palStdDev.at(i).setGreen(sqrt((s2.green-(s.green*s.green/m))/(m-1)));
        palStdDev.at(i).setBlue(sqrt((s2.blue - (s.blue*s.blue/m))/(m-1)));
      } else {
        palStdDev.at(i) = trgbPixel<float>(0,0,0);
      }
    }

    n.resize(maxi,0,true,true);

    return true;
  }

  /**
   * operates on a copy of the given %parameters.
   * @param src image with the source data.
   * @param mask mask with palette entry indices.
   * @param dest computed palette (mean value of all elements with the
   *                               same index).
   * @param palStdDev standard deviation for each palette entry.
   * @return true if apply successful or false otherwise.
   */
  bool computePalette::apply(const image& src,
                             const matrix<int>& mask,
                             palette& dest,
                             vector<trgbPixel<float> >& palStdDev,
                             vector<int>& n) const {
    if (src.size() != mask.size()) {
      setStatusString("Source image and index mask have different sizes");
      return false;
    }

    int y,i,m,maxi;

    maxi = mask.maximum()+1;

    vector<trgbPixel<float> > sum2(maxi,trgbPixel<float>(0,0,0));
    vector<trgbPixel<float> >  sum(maxi,trgbPixel<float>(0,0,0));
    n.resize(maxi,0,false,true);

    vector<rgbPixel>::const_iterator sit,eit;
    vector<int>::const_iterator cit;
    trgbPixel<float> px;

    for (y=0;y<src.rows();y++) {
      const vector<rgbPixel>& vct=src.getRow(y);
      const vector<int>& idx=mask.getRow(y);
      for (sit=vct.begin(),eit=vct.end(),cit=idx.begin();
           sit!=eit;
           ++sit,++cit) {
        i = *cit;
        n.at(i)++;
        px.copy(*sit);
        sum.at(i).add(px);
        sum2.at(i).add(px*px);
      }
    }

    dest.resize(maxi,Black,false,false);
    palStdDev.resize(maxi,trgbPixel<float>(),false,false);

    for (i=0;i<maxi;++i) {
      m = n.at(i);
      const trgbPixel<float>& s = sum.at(i);
      const trgbPixel<float>& s2 = sum2.at(i);

      dest.at(i).setRed(static_cast<ubyte>(s.red/m));
      dest.at(i).setGreen(static_cast<ubyte>(s.green/m));
      dest.at(i).setBlue(static_cast<ubyte>(s.blue/m));
      dest.at(i).setAlpha(0);

      if (m>1) {
        palStdDev.at(i).setRed(sqrt((s2.red-(s.red*s.red/m))/(m-1)));
        palStdDev.at(i).setGreen(sqrt((s2.green-(s.green*s.green/m))/(m-1)));
        palStdDev.at(i).setBlue(sqrt((s2.blue - (s.blue*s.blue/m))/(m-1)));
      } else {
        palStdDev.at(i) = trgbPixel<float>(0,0,0);
      }
    }

    return true;
  }

  //

  /**
   * this apply will generate multivariate statistics for each
   * entry in the palette. It takes more time than the other
   * ones.
   * @param src image with the source data.
   * @param mask mask with palette entry indices.
   * @param dest computed palette (mean value of all elements with the
   *                               same index).  Note that this apply
   *                               will compute with floats)
   * @param palCovar covariance matrices for each palette entry.
   * @param n number of pixels found for each entry
   * @return true if apply successful or false otherwise.
   */
  bool computePalette::apply(const image& src,
                             const matrix<int>& mask,
                             vector<trgbPixel<float> >& dest,
                             std::vector<matrix<float> >& palCovar,
                             vector<int>& n) const {

    if (src.size() != mask.size()) {
      setStatusString("Source image and index mask have different sizes");
      return false;
    }

    int y,i,m,maxi;

    maxi = mask.maximum()+1;

    palCovar.resize(maxi);
    dest.resize(maxi,trgbPixel<float>(),false,false);
    n.resize(maxi,0,false,true);

    matrix<double> tmpCovar(3,3,0.0f);
    std::vector<matrix<double> > sumOuter(maxi,tmpCovar);
    vector<trgbPixel<float> > sum(maxi,trgbPixel<float>(0.0f,0.0f,0.0f));

    vector<rgbPixel>::const_iterator sit,eit;
    vector<int>::const_iterator cit;
    matrix<double>::iterator omit;

    trgbPixel<float> px;

    for (y=0;y<src.rows();y++) {
      const vector<rgbPixel>& vct=src.getRow(y);
      const vector<int>& idx=mask.getRow(y);
      for (sit=vct.begin(),eit=vct.end(),cit=idx.begin();
           sit!=eit;
           ++sit,++cit) {
        i = *cit;
        n.at(i)++;
        px.copy(*sit);
        sum.at(i).add(px);

        // outer product of color is symmetric => consider only the upper
        // triangular matrix, and when asking for the model compute the
        // complete one...
        omit = sumOuter[i].begin();

        (*omit) += px.red*px.red;
        ++omit;
        (*omit) += px.red*px.green;
        ++omit;
        (*omit) += px.red*px.blue;
        omit+=2;
        (*omit) += px.green*px.green;
        ++omit;
        (*omit) += px.green*px.blue;
        omit+=3;
        (*omit) += px.blue*px.blue;
      }
    }

    for (i=0;i<maxi;++i) {
      m = n.at(i);
      if (m>0) {
        dest.at(i).divide(sum.at(i),static_cast<float>(m));
        const trgbPixel<float>& s = dest.at(i);
        outer(s,s,tmpCovar);

        sumOuter[i].at(1,0)=sumOuter[i].at(0,1);
        sumOuter[i].at(2,0)=sumOuter[i].at(0,2);
        sumOuter[i].at(2,1)=sumOuter[i].at(1,2);

        sumOuter[i].addScaled(static_cast<double>(-m),tmpCovar);
        if (m>1) {
          sumOuter[i].divide(static_cast<double>(m-1));
          palCovar[i].castFrom(sumOuter[i]);
        } else {
          palCovar[i].resize(3,3,0.0f,false,true); // fill with zeros
        }

      } else {
        palCovar[i].castFrom(sumOuter[i]); // copy a 3x3 zero matrix
      }
    }

    return true;
  }

  /**
   * this apply will generate multivariate statistics for each
   * entry in the palette.  It takes more time than the other
   * ones.  The image will be assumed to be composed by the given three
   * channels.
   *
   * @param ch1 first channel of the image
   * @param ch2 second channel of the image
   * @param ch3 third channel of the image
   * @param mask mask with palette entry indices.
   * @param dest computed palette (mean value of all elements with the
   *                               same index).  Note that this apply
   *                               will compute with floats)
   * @param palCovar covariance matrices for each palette entry.
   * @param n number of pixels found for each entry
   * @return true if apply successful or false otherwise.
   */
  bool computePalette::apply(const channel& ch1,
                             const channel& ch2,
                             const channel& ch3,
                             const matrix<int>& mask,
                             vector<trgbPixel<float> >& dest,
                             std::vector<matrix<float> >& palCovar,
                             vector<int>& n) const {

    if (ch1.size() != mask.size()) {
      setStatusString("Source channels and index mask have different sizes");
      return false;
    }

    if ((ch1.size() != ch2.size()) ||
        (ch1.size() != ch3.size())) {
      setStatusString("Source channels must have the same size.");
      return false;
    }

    int y,x,i,m,maxi;

    maxi = mask.maximum()+1;

    palCovar.resize(maxi);
    dest.resize(maxi,trgbPixel<float>(),false,false);
    n.resize(maxi,0,false,true);

    matrix<double> tmpCovar(3,3,0.0f);
    std::vector<matrix<double> > sumOuter(maxi,tmpCovar);
    vector<trgbPixel<float> > sum(maxi,trgbPixel<float>(0.0f,0.0f,0.0f));

    matrix<double>::iterator omit;
    trgbPixel<float> px;

    for (y=0;y<ch1.rows();++y) {
      for (x=0;x<ch1.columns();++x) {
        i = mask.at(y,x);
        n.at(i)++;
        px.set(ch1.at(y,x),ch2.at(y,x),ch3.at(y,x));
        sum.at(i).add(px);

        // outer product of color is symmetric => consider only the upper
        // triangular matrix, and when asking for the model compute the
        // complete one...
        omit = sumOuter[i].begin();

        (*omit) += px.red*px.red;
        ++omit;
        (*omit) += px.red*px.green;
        ++omit;
        (*omit) += px.red*px.blue;
        omit+=2;
        (*omit) += px.green*px.green;
        ++omit;
        (*omit) += px.green*px.blue;
        omit+=3;
        (*omit) += px.blue*px.blue;
      }
    }

    for (i=0;i<maxi;++i) {
      m = n.at(i);
      if (m>0) {
        dest.at(i).divide(sum.at(i),static_cast<float>(m));
        const trgbPixel<float>& s = dest.at(i);
        outer(s,s,tmpCovar);

        sumOuter[i].at(1,0)=sumOuter[i].at(0,1);
        sumOuter[i].at(2,0)=sumOuter[i].at(0,2);
        sumOuter[i].at(2,1)=sumOuter[i].at(1,2);

        sumOuter[i].addScaled(static_cast<double>(-m),tmpCovar);
        if (m>1) {
          sumOuter[i].divide(static_cast<double>(m-1));
          palCovar[i].castFrom(sumOuter[i]);
        } else {
          palCovar[i].resize(3,3,0.0f,false,true); // fill with zeros
        }

      } else {
        palCovar[i].castFrom(sumOuter[i]); // copy a 3x3 zero matrix
      }
    }

    return true;
  }

  /**
   * this apply will generate multivariate statistics for each
   * entry in the palette.  It takes more time than the other
   * ones.  The image will be assumed to be composed by the given three
   * channels.
   *
   * @param ch1 first channel of the image
   * @param ch2 second channel of the image
   * @param ch3 third channel of the image
   * @param mask mask with palette entry indices.
   * @param dest computed palette (mean value of all elements with the
   *                               same index).  Note that this apply
   *                               will compute with floats)
   * @param palCovar covariance matrices for each palette entry.
   * @param n number of pixels found for each entry
   * @return true if apply successful or false otherwise.
   */
  bool computePalette::apply(const channel& ch1,
                             const channel& ch2,
                             const channel& ch3,
                             const matrix<int>& mask,
                             vector<trgbPixel<float> >& dest,
                             vector<int>& n) const {

    if (ch1.size() != mask.size()) {
      setStatusString("Source channels and index mask have different sizes");
      return false;
    }

    if ((ch1.size() != ch2.size()) ||
        (ch1.size() != ch3.size())) {
      setStatusString("Source channels must have the same size.");
      return false;
    }

    int y,x,i,m,maxi;

    maxi = mask.maximum()+1;

    dest.resize(maxi,trgbPixel<float>(0.0f,0.0f,0.0f),false,true);
    n.resize(maxi,0,false,true);

    trgbPixel<float> px;

    for (y=0;y<ch1.rows();++y) {
      for (x=0;x<ch1.columns();++x) {
        i = mask.at(y,x);
        n.at(i)++;
        px.set(ch1.at(y,x),ch2.at(y,x),ch3.at(y,x));
        dest.at(i).add(px);
      }
    }
    
    for (i=0;i<maxi;++i) {
      m = n.at(i);
      if (m>0) {
        dest.at(i).divide(static_cast<float>(m));
      }
    }

    return true;
  }



  /**
   * This apply method computes the mean values and variances of the
   * regions indicated by the mask for the given channel.
   *
   * @param ch channel for which the region statistics will be computed
   * @param mask labeled mask indicating the regions
   * @param means mean values for each region
   * @param vars variances for each region
   * @param n number of pixels per region
   * @return true if successful, false otherwise.
   */
  bool computePalette::apply(const channel& ch,
                             const matrix<int>& mask,
                             vector<float>& means,
                             vector<float>& vars,
                             vector<int>& n) const {
    
    if (ch.size() != mask.size()) {
      setStatusString("Channel and mask have different sizes");
      return false;
    }
    
    int y,x,i,m;

    const int maxi = mask.maximum()+1;

    vars.resize(maxi,float(0),false,true);
    means.resize(maxi,float(0),false,true);
    n.resize(maxi,0,false,true);

    for (y=0;y<ch.rows();++y) {
      for (x=0;x<ch.columns();++x) {
        i = mask.at(y,x);
        const float v = ch.at(y,x);

        n.at(i)++;
        means.at(i)+=v;
        vars.at(i)+=(v*v);
      }
    }

    for (i=0;i<maxi;++i) {
      m = n.at(i);
      if (m>0) {
        const float avr = (means.at(i)/=m);
        vars.at(i)      = (vars.at(i)/m) - (avr*avr);
      } 
    }

    return true;
  }


  /**
   * compute the mean value and covariance matrix for just one
   * label in the mask.
   *
   * @param src image with the source data.
   * @param mask mask with palette entry indices.
   * @param label label to be analyzed.
   * @param mean mean color of the given label.
   * @param covar covariance matrix for the given label.
   * @param n number of pixels found with the given label.
   * @return true if apply successful or false otherwise.
   */
  bool computePalette::apply(const image& src,
                             const matrix<int>& mask,
                             const int label,
                             trgbPixel<float>& mean,
                             matrix<float>& covar,
                             int& n) const {

    if (src.size() != mask.size()) {
      setStatusString("Source image and index mask have different sizes");
      return false;
    }

    int y,m,maxi;

    maxi = mask.maximum()+1;

    matrix<double> tmpCovar(3,3,0.0f);
    covar.castFrom(tmpCovar);
    matrix<double> sumOuter(tmpCovar);

    trgbPixel<float> sum(0.0f,0.0f,0.0f);
    mean.copy(sum);
    n = 0;

    vector<rgbPixel>::const_iterator sit,eit;
    vector<int>::const_iterator cit;
    matrix<double>::iterator omit;

    trgbPixel<float> px;

    for (y=0;y<src.rows();y++) {
      const vector<rgbPixel>& vct=src.getRow(y);
      const vector<int>& idx=mask.getRow(y);
      for (sit=vct.begin(),eit=vct.end(),cit=idx.begin();
           sit!=eit;
           ++sit,++cit) {
        if (*cit == label) {
          n++;
          px.copy(*sit);
          sum.add(px);

          // outer product of color is symmetric => consider only the upper
          // triangular matrix, and when asking for the model compute the
          // complete one...
          omit = sumOuter.begin();

          (*omit) += px.red*px.red;
          ++omit;
          (*omit) += px.red*px.green;
          ++omit;
          (*omit) += px.red*px.blue;
          omit+=2;
          (*omit) += px.green*px.green;
          ++omit;
          (*omit) += px.green*px.blue;
          omit+=3;
          (*omit) += px.blue*px.blue;
        }
      }
    }

    m = n;
    if (m>0) {
      mean.divide(sum,static_cast<float>(m));
      const trgbPixel<float>& s = mean;
      outer(s,s,tmpCovar);

      sumOuter.at(1,0)=sumOuter.at(0,1);
      sumOuter.at(2,0)=sumOuter.at(0,2);
      sumOuter.at(2,1)=sumOuter.at(1,2);

      sumOuter.addScaled(static_cast<double>(-m),tmpCovar);
      if (m>1) {
        sumOuter.divide(static_cast<double>(m-1));
        covar.castFrom(sumOuter);
      } else {
        covar.resize(3,3,0.0f,false,true); // fill with zeros
      }

    } else {
      covar.castFrom(sumOuter); // copy a 3x3 zero matrix
    }

    return true;
  }

  /*
   * outer product of two colors
   */
  void computePalette::outer(const trgbPixel<float>& a,
                             const trgbPixel<float>& b,
                                   matrix<double>& mat) const {
    matrix<double>::iterator it = mat.begin();
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
