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
 * file .......: ltiSimilarityMatrix.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 18.3.2002
 * revisions ..: $Id: ltiSimilarityMatrix.cpp,v 1.9 2006/09/05 10:31:13 ltilib Exp $
 */

#include "ltiSimilarityMatrix.h"
#include <string>

namespace lti {
  // --------------------------------------------------
  // similarityMatrix::parameters
  // --------------------------------------------------

  // default constructor
  similarityMatrix::parameters::parameters()
    : functor::parameters() {
    mode = BorderBased;
    distFunction = similarityMatrix::chromaticity2;
  }

  // copy constructor
  similarityMatrix::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  similarityMatrix::parameters::~parameters() {
  }

  // get type name
  const char* similarityMatrix::parameters::getTypeName() const {
    return "similarityMatrix::parameters";
  }

  // copy member

  similarityMatrix::parameters&
  similarityMatrix::parameters::copy(const parameters& other) {
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

    mode = other.mode;
    distFunction = other.distFunction;

    return *this;
  }

  // alias for copy member
  similarityMatrix::parameters&
    similarityMatrix::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* similarityMatrix::parameters::clone() const {
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
  bool similarityMatrix::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool similarityMatrix::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      switch (mode) {
          case BorderBased:
            lti::write(handler,"mode","BorderBased");
            break;
          case AreaBased:
            lti::write(handler,"mode","AreaBased");
            break;
          default:
            lti::write(handler,"mode","Unknown");
            break;
      }

      if (distFunction == similarityMatrix::euclideanRGB2) {
        lti::write(handler,"distFunction","euclideanRBG2");
      } else if (distFunction == similarityMatrix::euclideanLLL2) {
        lti::write(handler,"distFunction","euclideanLLL2");
      } else if (distFunction == similarityMatrix::chromaticity2) {
        lti::write(handler,"distFunction","chromaticity2");
      } else if (distFunction == similarityMatrix::hueSaturation) {
        lti::write(handler,"distFunction","hueSaturation");
      } else if (distFunction == similarityMatrix::hue) {
        lti::write(handler,"distFunction","hue");
      } else if (distFunction == similarityMatrix::neighbor) {
        lti::write(handler,"distFunction","neighbor");
      } else {
        lti::write(handler,"distFunction","unknown");
      }

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
  bool similarityMatrix::parameters::write(ioHandler& handler,
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
  bool similarityMatrix::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool similarityMatrix::parameters::readMS(ioHandler& handler,
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
      if (str == "BorderBased") {
        mode = BorderBased;
      } else {
        mode = AreaBased;
      }

      lti::read(handler,"distFunction",str);
      if (str == "euclideanRGB2") {
        distFunction = similarityMatrix::euclideanRGB2;
      } else if (str == "euclideanLLL2") {
        distFunction = similarityMatrix::euclideanLLL2;
      } else if (str == "chromaticity2") {
        distFunction = similarityMatrix::chromaticity2;
      } else if (str == "hueSaturation") {
        distFunction = similarityMatrix::hueSaturation;
      } else if (str == "hue") {
        distFunction = similarityMatrix::hue;
      } else if (str == "neighbor") {
        distFunction = similarityMatrix::neighbor;
      } else {
        distFunction = similarityMatrix::euclideanRGB2;
      }
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
  bool similarityMatrix::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // similarityMatrix
  // --------------------------------------------------

  // default constructor
  similarityMatrix::similarityMatrix()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // copy constructor
  similarityMatrix::similarityMatrix(const similarityMatrix& other)
    : functor()  {
    copy(other);
  }

  // destructor
  similarityMatrix::~similarityMatrix() {
  }

  // returns the name of this type
  const char* similarityMatrix::getTypeName() const {
    return "similarityMatrix";
  }

  // copy member
  similarityMatrix& similarityMatrix::copy(const similarityMatrix& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  similarityMatrix&
    similarityMatrix::operator=(const similarityMatrix& other) {
    return (copy(other));
  }


  // clone member
  functor* similarityMatrix::clone() const {
    return new similarityMatrix(*this);
  }

  // return parameters
  const similarityMatrix::parameters&
    similarityMatrix::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The distance functions
  // -------------------------------------------------------------------

  double similarityMatrix::euclideanLLL2(const rgbPixel& a,
                                         const rgbPixel& b) {
    int ra = a.getRed();
    int ga = a.getGreen();
    int ba = a.getBlue();
    int rb = b.getRed();
    int gb = b.getGreen();
    int bb = b.getBlue();

    int l1a = (ra-ga)*(ra-ga);
    int l2a = (ra-ba)*(ra-ga);
    int l3a = (ga-ba)*(ga-ba);
    int l1b = (rb-gb)*(rb-gb);
    int l2b = (rb-bb)*(rb-gb);
    int l3b = (gb-bb)*(gb-bb);

    return
      (l1a-l1b)*(l1a-l1b)+
      (l2a-l2b)*(l2a-l2b)+
      (l3a-l3b)*(l3a-l3b);
  }

  double similarityMatrix::euclideanRGB2(const rgbPixel& a,
                                         const rgbPixel& b) {
    return double(a.distanceSqr(b));
  }

  double similarityMatrix::chromaticity2(const rgbPixel& a,
                                         const rgbPixel& b) {

    double Ia = (double(a.getRed()) + a.getGreen() + a.getBlue());
    double Ib = (double(b.getRed()) + b.getGreen() + b.getBlue());

    double ra = double(a.getRed())/Ia;
    double rb = double(b.getRed())/Ib;

    double ga = double(a.getGreen())/Ia;
    double gb = double(b.getGreen())/Ib;

    double dr = (ra-rb)*255.0;
    double dg = (ga-gb)*255.0;

    double d = dr*dr + dg*dg;

    return d;
  }

  double similarityMatrix::hueSaturation(const rgbPixel& a,
                                         const rgbPixel& b) {
    double Ha,Sa,Va,Hb,Sb,Vb;          // channels
    double mini,delta;
    double dh,ds;

    // compute hue, value and saturation for pixel a

    Va = max(a.getRed(),a.getGreen(),a.getBlue());
    mini = min(a.getRed(),a.getGreen(),a.getBlue());
    if (Va!=0) {
      Sa = 1.0f - (mini/Va);
      if (Sa==0) {
        Ha=0;
      }
      else {
        delta = (Va-mini)*6.0f;
        if (a.getRed()==Va)
          Ha = (a.getGreen()-a.getBlue())/delta;
        else if (a.getGreen()==Va)
          Ha = 0.333333f + (a.getBlue()-a.getRed())/delta;
        else
          Ha = 0.666667f + (a.getRed()-a.getGreen())/delta;
        if (Ha<0)
          Ha = Ha + 1.0f;
      }
    }
    else {
      Sa = 1.0f;
      Ha = 0;
    }

    Va /= 255.0f ;  // V zwischen 0 und 1

    // compute hue, value and saturation for pixel b

    Vb = max(b.getRed(),b.getGreen(),b.getBlue());
    mini = min(b.getRed(),b.getGreen(),b.getBlue());
    if (Vb!=0) {
      Sb = 1.0f - (mini/Vb);
      if (Sb==0) {
        Hb=0;
      }
      else {
        delta = (Vb-mini)*6.0f;
        if (b.getRed()==Vb)
          Hb = (b.getGreen()-b.getBlue())/delta;
        else if (b.getGreen()==Vb)
          Hb = 0.333333f + (b.getBlue()-b.getRed())/delta;
        else
          Hb = 0.666667f + (b.getRed()-b.getGreen())/delta;
        if (Ha<0)
          Hb = Hb + 1.0f;
      }
    }
    else {
      Sb = 1.0f;
      Hb = 0;
    }

    Vb /= 255.0f ;  // V zwischen 0 und 1

    // compute the distance
    dh = abs(Ha-Hb);
    if (dh > 0.5)
      dh = 1.0 - dh;
    ds = (Sa-Sb);

    dh *=255.0;
    ds *=255.0;

    return dh*dh/*+ds*ds*/;

  }


  double similarityMatrix::hue(const rgbPixel& a,
                               const rgbPixel& b) {
    double Ha,Sa,Va,Hb,Sb,Vb;          // channels
    double mini,delta;
    double dh;

    // compute hue, value and saturation for pixel a

    Va = max(a.getRed(),a.getGreen(),a.getBlue());
    mini = min(a.getRed(),a.getGreen(),a.getBlue());
    if (Va!=0) {
      Sa = 1.0f - (mini/Va);
      if (Sa==0) {
        Ha=0;
      }
      else {
        delta = (Va-mini)*6.0f;
        if (a.getRed()==Va)
          Ha = (a.getGreen()-a.getBlue())/delta;
        else if (a.getGreen()==Va)
          Ha = 0.333333f + (a.getBlue()-a.getRed())/delta;
        else
          Ha = 0.666667f + (a.getRed()-a.getGreen())/delta;
        if (Ha<0)
          Ha = Ha + 1.0f;
      }
    }
    else {
      Sa = 1.0f;
      Ha = 0;
    }

    Va /= 255.0f ;  // V zwischen 0 und 1

    // compute hue, value and saturation for pixel b

    Vb = max(b.getRed(),b.getGreen(),b.getBlue());
    mini = min(b.getRed(),b.getGreen(),b.getBlue());
    if (Vb!=0) {
      Sb = 1.0f - (mini/Vb);
      if (Sb==0) {
        Hb=0;
      }
      else {
        delta = (Vb-mini)*6.0f;
        if (b.getRed()==Vb)
          Hb = (b.getGreen()-b.getBlue())/delta;
        else if (b.getGreen()==Vb)
          Hb = 0.333333f + (b.getBlue()-b.getRed())/delta;
        else
          Hb = 0.666667f + (b.getRed()-b.getGreen())/delta;
        if (Ha<0)
          Hb = Hb + 1.0f;
      }
    }
    else {
      Sb = 1.0f;
      Hb = 0;
    }

    Vb /= 255.0f ;  // V zwischen 0 und 1

    // compute the distance
    dh = abs(Ha-Hb);
    if (dh > 0.5)
      dh = 1.0 - dh;

    dh *=255.0;

    return dh*dh;

  }

  double similarityMatrix::neighbor(const rgbPixel& a,
				    const rgbPixel& b) {
    return 1.0;
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  /*
   * operates on the given %parameter.
   * @param img original image.  All points in the list of areaPoints must
   *        lie inside the image, or an exception will be thrown.
   * @param lapts list of area points.
   * @param colSimMat color similarity matrix
   * @param geoSimMat geometry similarity measure
   * @return true if apply successful or false otherwise.
   */
  bool similarityMatrix::apply(const image& img,
                               const int& nObj,
			       const imatrix& lmask,
                               dmatrix& colSimMat,
                               dmatrix& geoSimMat) const {

    dmatrix borderMat;
    dmatrix areaMat;
    borderMat.resize(nObj+1,nObj+1,0,false,false);
    areaMat.resize(nObj+1,nObj+1,0,false,false);
    geoSimMat.resize(nObj+1,nObj+1,0,false,false);

    similarity(img,lmask,borderMat,areaMat,geoSimMat);

    if (getParameters().mode == parameters::BorderBased) {
      borderMat.detach(colSimMat);
    } else {
      areaMat.detach(colSimMat);
    }

    return true;
  }

  /*
   * operates on the given %parameter.
   * @param img original image.  All points in the list of areaPoints must
   *        lie inside the image, or an exception will be thrown.
   * @param lapts list of area points.
   * @param colSimMat color similarity matrix
   * @param geoSimMat geometry similarity measure
   * @return true if apply successful or false otherwise.
   */
  bool similarityMatrix::apply(const image& img,
                               const std::list<areaPoints>& lapts,
                               dmatrix& colSimMat,
                               dmatrix& geoSimMat,
                               imatrix& lmask) const {

    lmask.resize(img.size(),0,false,false);

    const int nObj = lapts.size();

    dmatrix borderMat;
    dmatrix areaMat;

    borderMat.resize(nObj+1,nObj+1,0,false,false);
    areaMat.resize(nObj+1,nObj+1,0,false,false);
    geoSimMat.resize(nObj+1,nObj+1,0,false,false);

    createMask(lapts,lmask);
    similarity(img,lmask,borderMat,areaMat,geoSimMat);

    const parameters& param = getParameters();

    if (param.mode == parameters::BorderBased) {
      borderMat.detach(colSimMat);
    } else {
      areaMat.detach(colSimMat);
    }

    return true;
  }

  /*
   * create a mask from the area points
   */
  void similarityMatrix::createMask(const std::list<areaPoints>& apts,
                                    imatrix& mask) const {

    std::list<areaPoints>::const_iterator iterLstAreaPointLists;
    areaPoints::const_iterator iterAreaPoints,endIterAreaPoints;

    int label = 1;

    mask.fill(0);

    for (iterLstAreaPointLists = apts.begin();
         iterLstAreaPointLists != apts.end();
         ++iterLstAreaPointLists) {

      for (iterAreaPoints = (*iterLstAreaPointLists).begin(),
             endIterAreaPoints = (*iterLstAreaPointLists).end();
           iterAreaPoints != endIterAreaPoints;
           ++iterAreaPoints) {
        mask.at(*iterAreaPoints) = label;
      }

      label++;
    }

  }


  /*
   * create the border similarity matrix.
   * The size of the similarity matrix must be set before calling this
   * method
   */
  void similarityMatrix::similarity(const image& img,
                                    const imatrix& mask,
                                    dmatrix& colBorderSimMat,
                                    dmatrix& colAreaSimMat,
                                    dmatrix& geoSimMat) const {

    int y,x,xx,yy;
    int a,b;
    rgbPixel px;
    const int lasty = mask.lastRow();
    const int lastx = mask.lastColumn();
    double d;

    double (*f)(const rgbPixel&,
                const rgbPixel&) = getParameters().distFunction;

    // this matrix how many pixels were considered in the borders
    imatrix mij(colBorderSimMat.size(),0);
    vector< trgbPixel<float> > pal(colBorderSimMat.rows(),
                                   trgbPixel<float>(0.0f,0.0f,0.0f));
    ivector objPix(colBorderSimMat.rows(),0);
    colBorderSimMat.fill(0.0);
    colAreaSimMat.fill(0.0);
    geoSimMat.fill(0.0);

    for (y=0,yy=1;y<lasty;++y,++yy) {
      for (x=0,xx=1;x<lastx;++x,++xx) {
        a = mask.at(y,x);
        objPix.at(a)++;
        px = img.at(y,x);
        pal.at(a)+=px;

        b = mask.at(y,xx);
        if (b != a) {
          d = (*f)(px,img.at(y,xx));
          colBorderSimMat.at(a,b) += d;
          colBorderSimMat.at(b,a) += d;
	  mij.at(a,b)++;
          mij.at(b,a)++;
        }

        b = mask.at(yy,x);
        if (b != a) {
          d = (*f)(px,img.at(yy,x));
          colBorderSimMat.at(a,b) += d;
          colBorderSimMat.at(b,a) += d;
          mij.at(a,b)++;
          mij.at(b,a)++;
        }
      }

      a = mask.at(y,x);
      objPix.at(a)++;
    }

    for (x=0,xx=1;x<lastx;++x,++xx) {
      a = mask.at(y,x);
      objPix.at(a)++;
      px = img.at(y,x);
      pal.at(a) += px;

      b = mask.at(y,xx);
      if (b != a) {
        d = (*f)(px,img.at(y,xx));
	colBorderSimMat.at(a,b) += d;
        colBorderSimMat.at(b,a) += d;
	mij.at(a,b)++;
        mij.at(b,a)++;
      }
    }
    a = mask.at(y,x);
    objPix.at(a)++;
    pal.at(a) += img.at(y,x);

    //average color per pal-entry
    for (a=0;a<pal.size();++a) {
      pal.at(a).divide(static_cast<float>(objPix.at(a)));
    }

    for (y=0;y<mij.rows();++y) {
      double area = sqrt(objPix.at(y));
      for (x=0;x<mij.columns();++x) {
        if (mij.at(x,y) != 0) {
          if (colBorderSimMat.at(y,x) != 0) {
	    // border measure
            colBorderSimMat.at(y,x) = (mij.at(y,x)/
                                       colBorderSimMat.at(y,x));
	    // area measure
            colAreaSimMat.at(y,x) = 1.0/(*f)(pal.at(y).getRGBPixel(),
                                             pal.at(x).getRGBPixel());
          } else {
            colBorderSimMat.at(y,x) = 0.0;
            colAreaSimMat.at(y,x) = 0.0;
          }
	  // geometric measure
          geoSimMat.at(y,x) = mij.at(y,x)/area;
        }
      }
    }
  };


  /*
   * computes for the given mask which labels are used in neighbor objects
   * for the given label.
   * @param nObj number of objects in the mask.  Is expected here in order to
   *             reduce its computation from the mask.  This value should be
   *             equal to the maximum value in the mask plus one.
   * @param lmask label mask computed from the area points
   * @param label label for which the neighbors need to be found
   * @param neighborLabels resulting vector
   */
  bool similarityMatrix::neighbors(const int& nObj,
                                   const imatrix& mask,
                                   const int& label,
                                   ivector& neighborLabels) const {

    int y,x,xx,yy;
    int a,b;


    neighborLabels.resize(nObj+1,0,false,true);

    const int lasty = mask.lastRow();
    const int lastx = mask.lastColumn();

    for (y=0,yy=1;y<lasty;++y,++yy) {
      for (x=0,xx=1;x<lastx;++x,++xx) {
        a = mask.at(y,x);

        if (a==label) {

          b = mask.at(y,xx);
          if (b != a) {
            neighborLabels.at(b)++;
          }

          b = mask.at(yy,x);
          if (b != a) {
            neighborLabels.at(b)++;
          }
        }
      }

      a = mask.at(y,x);
      if (a == label) {
        b = mask.at(y,x-1);
        if (b != a) {
          neighborLabels.at(b)++;
        }
        b = mask.at(yy,x);
        if (b != a) {
          neighborLabels.at(b)++;
        }
      }
    }

    yy = lasty-1;
    for (x=0,xx=1;x<lastx;++x,++xx) {
      a = mask.at(y,x);

      if (a==label) {
        b = mask.at(y,xx);
        if (b != a) {
          neighborLabels.at(b)++;
        }

        b = mask.at(yy,x);
        if (b != a) {
          neighborLabels.at(b)++;
        }
      }
    }

    a = mask.at(y,x);
    if (a == label) {
      b = mask.at(y,x-1);
      if (b != a) {
        neighborLabels.at(b)++;
      }
    }

    return true;
  }


}
