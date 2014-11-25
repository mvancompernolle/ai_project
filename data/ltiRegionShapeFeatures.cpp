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
 * file .......: ltiRegionShapeFeatures.cpp
 * authors ....: Axel Berner, Pablo Alvarado, Jens Rietzschel
 * organization: LTI, RWTH Aachen
 * creation ...: 19.6.2002
 * revisions ..: $Id: ltiRegionShapeFeatures.cpp,v 1.13 2006/09/05 10:28:42 ltilib Exp $
 */



#include "ltiRegionShapeFeatures.h"
#include "ltiGeometricTransform.h"
#include "ltiPolarToCartesian.h"
#include "ltiCartesianToPolar.h"

//#define _LTI_DEBUG 4

#ifdef _LTI_DEBUG
#include "ltiViewer.h"
#include <cstdio> //getchar
#endif


namespace lti {
  // --------------------------------------------------
  // regionShapeFeatures::parameters
  // --------------------------------------------------

  // default constructor
  regionShapeFeatures::parameters::parameters()
    : globalFeatureExtractor::parameters() {

    size = point(12,3); //angular freq , radial freq.
    maskSize = 65;//129;//257; //best2^n + 1
    polar = true;
    basisFunctionSet = MPEG7;
  }

  // copy constructor
  regionShapeFeatures::parameters::parameters(const parameters& other)
    : globalFeatureExtractor::parameters()  {
    copy(other);
  }

  // destructor
  regionShapeFeatures::parameters::~parameters() {
  }

  // get type name
  const char* regionShapeFeatures::parameters::getTypeName() const {
    return "regionShapeFeatures::parameters";
  }

  // copy member

  regionShapeFeatures::parameters&
    regionShapeFeatures::parameters::copy(const parameters& other) {
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

    size = other.size;
    maskSize = other.maskSize;
    polar = other.polar;
    basisFunctionSet = other.basisFunctionSet;

    return *this;
  }

  // alias for copy member
  regionShapeFeatures::parameters&
    regionShapeFeatures::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* regionShapeFeatures::parameters::clone() const {
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
  bool regionShapeFeatures::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool regionShapeFeatures::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"size",size);
      lti::write(handler,"maskSize",maskSize);
      lti::write(handler,"polar",polar);
      if (basisFunctionSet == OrthoRadial) {
        lti::write(handler,"basisFunctionSet","OrthoRadial");
      } else {
        lti::write(handler,"basisFunctionSet","MPEG7");
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
  bool regionShapeFeatures::parameters::write(ioHandler& handler,
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
  bool regionShapeFeatures::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool regionShapeFeatures::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"size",size);
      lti::read(handler,"maskSize",maskSize);
      lti::read(handler,"polar",polar);
      std::string str;
      lti::read(handler,"basisFunctionSet",str);
      if (str == "MPEG7") {
        basisFunctionSet = MPEG7;
      } else {
        basisFunctionSet = OrthoRadial;
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
  bool regionShapeFeatures::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // regionShapeFeatures
  // --------------------------------------------------

  // default constructor
  regionShapeFeatures::regionShapeFeatures()
    : globalFeatureExtractor() {

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  regionShapeFeatures::regionShapeFeatures(const parameters& par)
    : globalFeatureExtractor() {

    // set the default parameters
    setParameters(par);
  }

  // copy constructor
  regionShapeFeatures::regionShapeFeatures(const regionShapeFeatures& other)
    : globalFeatureExtractor()  {
    copy(other);
  }

  // destructor
  regionShapeFeatures::~regionShapeFeatures() {
  }

  // returns the name of this type
  const char* regionShapeFeatures::getTypeName() const {
    return "regionShapeFeatures";
  }

  // copy member
  regionShapeFeatures&
    regionShapeFeatures::copy(const regionShapeFeatures& other) {
    globalFeatureExtractor::copy(other);
    lutR = other.lutR;
    lutI = other.lutI;
    radLut.copy(other.radLut);

    return (*this);
  }

  // alias for copy member
  regionShapeFeatures&
    regionShapeFeatures::operator=(const regionShapeFeatures& other) {
    return (copy(other));
  }


  // clone member
  functor* regionShapeFeatures::clone() const {
    return new regionShapeFeatures(*this);
  }

  bool regionShapeFeatures::updateParameters() {
    return calcLUT();
  }

  // return parameters
  const regionShapeFeatures::parameters&
    regionShapeFeatures::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool regionShapeFeatures::calcLUT() {

    const parameters& param = getParameters();
    const point size = param.size;
    const int lutSize = param.maskSize;
    const int lutSize2 = (lutSize-1)/2;

    if (lutSize < 3) {
      setStatusString("Desired mask size is too small");
      return false;
    }

    if ((size.x < 1) || (size.y < 1)) {
      setStatusString("Invalid descriptor size: at least 1x1 required.");
      return false;
    }

    // check if it is necessary to recompute the LUT
    if ((lutSize == radLut.columns()) &&
        (size.y == static_cast<int>(lutR.size())) &&
        (size.x == static_cast<int>(lutR[0].size())) &&
        (lastBasisFunctionSet == param.basisFunctionSet)) {
      // nothing to be done.  (no changes in the parameters)
      return true;
    }

    int x,y,dx,dy;
    int m,n;
    float angle;
    double Rnm=0;
    float radius,rho;
    static const float pi2 = static_cast<float>(2.0*Pi);
    lastBasisFunctionSet = param.basisFunctionSet;

    //inverted radius map
    radLut.resize(lutSize,lutSize,0.0,false,false);

    for(y=-lutSize2,dy=0;dy<lutSize;++y,++dy) {
      for(x=-lutSize2,dx=0;dx<lutSize;++x,++dx) {
        radLut.at(dy,dx) = sqrt(static_cast<float>(x*x+y*y));
      }
    }

    if(param.basisFunctionSet == parameters::Zernike){

      if (abs(size.x)>size.y){
      setStatusString("Zernike condition violated: |m|<=n");
      return false;
      }

        // simulate a matrix<channel>[n][m] with size.y = n and size.x = m
      lutR.resize(size.y);
      lutI.resize(size.y);
      vecNM.resize(size.y);

      for(n=0;n<size.y;n++) {
        lutR[n].resize(size.x);
        lutI[n].resize(size.x);
        vecNM[n].resize(size.x);
        for(m=0;m<size.x;m++){
          if(abs(n-m)%2 == 0)vecNM[n][m]=true;
          else vecNM[n][m]=false;
        }
      }

      float sina, cosa;
      for(n=0;n<size.y;n++) {
        for(m=0;m<size.x;m++){
          channel& actLutR = lutR[n][m];
          channel& actLutI = lutI[n][m];

          actLutR.resize(lutSize,lutSize,0.0f,false,false);
          actLutI.resize(lutSize,lutSize,0.0f,false,false);

          if(vecNM[n][m]==true){
            for(dy=0,y=-lutSize2;dy<lutSize;++y,++dy) {
              for(dx=0,x=-lutSize2;dx<lutSize;++x,++dx) {
                radius = radLut.at(dy,dx);
                Rnm=0;rho=(radius/lutSize2);
                // only the circle of radius lutSize2 required
                if (radius <= lutSize2) {

                  for(int s=0;s<=((n-m)/2);s++){

                    Rnm+=double(pow(-1.0,s)*fac(n-s)*pow(double(rho),n-2*s)/
                                (fac(s)*fac((n+m)/2-s)*fac((n-m)/2-s)));
                  }
                  angle = atan2(float(y),float(x));
                  sincos(angle*m, sina, cosa);
                  actLutR.at(dy,dx) = static_cast<float>(Rnm*cosa);
                  actLutI.at(dy,dx) = static_cast<float>(Rnm*sina);
                } else {
                  actLutR.at(dy,dx) = 0;
                  actLutI.at(dy,dx) = 0;
                }
              }
            }
          }
        }
      }
    }
    else{  //MPEG7 or OrthoRadial

      // simulate a matrix<channel>[n][m] with size.y = n and size.x = m
      lutR.resize(size.y);
      lutI.resize(size.y);
      for(n=0;n<size.y;++n) {
        lutR[n].resize(size.x);
        lutI[n].resize(size.x);
      }

      // fill the LUT with the basis functions

      // first fill the first row, with zero radial frequency (n = 0) and
      // different angular frequencies:  A(theta,m) = exp(j*m*theta)/(2*Pi)

      float sina, cosa;
      for(m=0;m<size.x;++m) {
        channel& actLutR = lutR[0][m];
        channel& actLutI = lutI[0][m];

        actLutR.resize(lutSize,lutSize,0.0f,false,false);
        actLutI.resize(lutSize,lutSize,0.0f,false,false);

        for(dy=0,y=-lutSize2;dy<lutSize;++y,++dy) {
          for(dx=0,x=-lutSize2;dx<lutSize;++x,++dx) {
            radius = radLut.at(dy,dx);
            // only the circle of radius lutSize2 required
            if (radius <= lutSize2) {
              angle = atan2(float(y),float(x));
              sincos(angle*m, sina, cosa);
              actLutR.at(dy,dx) = static_cast<float>(cosa/pi2);
              actLutI.at(dy,dx) = static_cast<float>(sina/pi2);
            } else {
              actLutR.at(dy,dx) = 0;
              actLutI.at(dy,dx) = 0;
            }
          }
        }
      }

      // now fill the first column, with zero angular frequency (m=0) and
      // different radial frecuencies:
      // R(radius,n) = 1 if n=0, 2cos(n*Pi*radius) if n!=0
      float temp;
      float cst;

      if (param.basisFunctionSet == parameters::MPEG7) {
        cst = static_cast<float>(Pi);
      } else {
        cst = static_cast<float>(2.0*Pi);
      }

      for(n=1;n<size.y;++n) {
        channel& actLutR = lutR[n][0];
        channel& actLutI = lutI[n][0];

        actLutR.resize(lutSize,lutSize,0.0f,false,false);
        actLutI.resize(lutSize,lutSize,0.0f,false,true);

        for(y=-lutSize2,dy=0;dy<lutSize;++y,++dy) {
          for(x=-lutSize2,dx=0;dx<lutSize;++x,++dx) {
            radius = radLut.at(dy,dx);
            // only circle of radius lutSize2 required:
            if (radius <= lutSize2) {
              temp = cst * radius *  n / lutSize2;
              actLutR.at(dy,dx) = 2.0f*cos(temp);
            }
            else {
              actLutR.at(dy,dx) = 0;
            }
          }
        }
      }

      // all other elements can be expressed as a product of the first row
      // and first column.  m,n != 0
      for(n=1;n<size.y;++n) {
        for(m=1;m<size.x;++m) {
          lutR[n][m].emultiply(lutR[n][0],lutR[0][m]);
          lutI[n][m].emultiply(lutR[n][0],lutI[0][m]);
        }
      }

      // now we need to rescale the first column of the real part
      // (which should also be multiplied by A_0(theta)
      for(n=1;n<size.y;++n) {
        lutR[n][0].multiply(lutR[0][0].at(lutSize2,lutSize2));
      }
    }


#   ifdef _LTI_DEBUG 
#   if _LTI_DEBUG > 2

    // show the basis functions:
    static channel canvasR,canvasI;
    static viewer viewR("Basis Functions (Real)");
    static viewer viewI("Basis Functions (Imag)");
    const int sx = size.x*lutSize + (size.x-1);
    const int sy = size.y*lutSize + (size.y-1);

    canvasR.resize(sy,sx,0,false,true);
    canvasI.resize(sy,sx,0,false,true);

    for (n=0;n<size.y;++n) {
      for (m=0;m<size.x;++m) {
        canvasR.fill(lutR[n][m],n*(lutSize+1),m*(lutSize+1),
                     n*(lutSize+1)+lutSize-1,m*(lutSize+1)+lutSize-1);
        canvasI.fill(lutI[n][m],n*(lutSize+1),m*(lutSize+1),
                     n*(lutSize+1)+lutSize-1,m*(lutSize+1)+lutSize-1);
      }
    }

    viewR.show(canvasR);
    viewI.show(canvasI);

#   if _LTI_DEBUG > 3
    static channel ortho;
    static viewer viewo("Orthogonality");
    // testOrthogonality(ortho);
    // viewo.show(ortho);
#   endif

#   endif
#   endif

    return true;
  };

  bool regionShapeFeatures::testOrthogonality(channel& ortho) const {
    const parameters& param = getParameters();
    int n,m,np,mp,i,j;
    const int nm = param.size.x * param.size.y;
    ortho.resize(nm,nm,0,false,false);
    float re,im;
    for (j=0;j<nm;++j) {
      n = j / param.size.x;
      m = j % param.size.x;
      for (i=0;i<nm;++i) {
        np = i / param.size.x;
        mp = i % param.size.x;
        dot(lutR[n][m],lutI[n][m],
            lutR[np][mp],lutI[np][mp],
            re,im);
        ortho.at(j,i) = sqrt(re*re+im*im);
      }
    }

    return true;
  }

  float regionShapeFeatures::binarize(const float& x) {
    return (x<0.5f) ? 0.0f : 1.0f;
  }

  void regionShapeFeatures::resizeMask(const channel8& src8,
                                             channel& ssrc) const {

    // computer center of mass
    int x,y,sum(0);
    rectangle bbox;
    bbox.ul = src8.size();
    bbox.br = point(0,0);
    point centerOfMass(0,0);
    for (y=0;y<src8.rows();++y) {
      for (x=0;x<src8.columns();++x) {
        if (src8.at(y,x) != 0) {
          centerOfMass.x+=x;
          centerOfMass.y+=y;
          ++sum;
          bbox.ul.x = min(bbox.ul.x,x);
          bbox.ul.y = min(bbox.ul.y,y);
          bbox.br.x = max(bbox.br.x,x);
          bbox.br.y = max(bbox.br.y,y);
        }
      }
    }

    centerOfMass=point(iround(float(centerOfMass.x)/float(sum)),
                       iround(float(centerOfMass.y)/float(sum)));

    //search max distance between center and all object pixels
    // for scale factor
    int maxRad2(0);
    point maxP;
    int r2;
    for (y=bbox.ul.y;y<bbox.br.y;++y) {
      for (x=bbox.ul.x;x<bbox.br.x;++x) {
        if (src8.at(y,x) != 0) {
          r2 = centerOfMass.distanceSqr(point(x,y));
          if (r2>maxRad2) {
            maxRad2 = r2;
            maxP = point(x,y);
          }
        }
      }
    }

    const int lutSize = getParameters().maskSize;
    const double scale = double(lutSize)/(2.0*sqrt(maxRad2));
    const tpoint<double> shiftVec(double(lutSize)/2-scale*centerOfMass.x,
                                  double(lutSize)/2-scale*centerOfMass.y);

    //shift and scale src
    channel cutOutSrc;
    cutOutSrc.castFrom(src8);
    cutOutSrc.resize(max(lutSize,cutOutSrc.rows()),
                     max(lutSize,cutOutSrc.columns()),
                     0,true,true); //force min size

    geometricTransform gt;
    geometricTransform::parameters gtParam;
    gtParam.scale(scale); //transformation depends on order!
    gtParam.shift(shiftVec);
    gt.setParameters(gtParam);

    gt.apply(cutOutSrc,ssrc);

    ssrc.resize(lutSize,lutSize,0,true,true);
    ssrc.apply(binarize); // binarize the channel

  };

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  void regionShapeFeatures::dot(const channel& reA, const channel& imA,
                                const channel& reB, const channel& imB,
                                       float& real,        float& imag) const {

    channel::const_iterator rait,iait,rbit,ibit,eit;
    float re(0),im(0);

    for(rait=reA.begin(),iait=imA.begin(),
        rbit=reB.begin(),ibit=imB.begin(),eit=reA.end();
        rait != eit;
        ++rait,++iait,++rbit,++ibit) {

      re += ((*rait * *rbit) - (*iait * *ibit));
      im += ((*rait * *ibit) + (*iait * *rbit));
    }

    real = re;
    imag = im;
  }

  // On place apply for type channel8!
  bool regionShapeFeatures::apply(const channel8& src,
                                  channel& dest1,
                                  channel& dest2) const {

    if(src.empty()) {
      setStatusString("image empty");
      dest1.clear();
      dest2.clear();
      return false;
    }

    channel ssrc; //scaled source
    resizeMask(src,ssrc);

    const parameters& param = getParameters();
    const point size = param.size;
    float factor=1;


    channel& coffR = dest1; // Real Part
    channel& coffI = dest2; // Imagine Part


    coffR.resize(size,0.0f,false,true);
    coffI.resize(size,0.0f,false,true);

    channel coffMag(size,0.0f); //Magnitude
    channel coffPha(size,0.0f); //Phase

    channel::iterator itSrc,itEnd;
    channel::const_iterator itLutR,itLutI;

    const float norm  = 1.0f/lutR[0][0].sumOfElements();
    channel temp;
    int m,n;
    for(n=0;n<size.y;++n) {
      if (param.basisFunctionSet==parameters::Zernike) {
        factor=static_cast<float>((n+1)/Pi);
      }
      for(m=0;m<size.x;++m) {
        float coffRTemp = 0.0;
        float coffITemp = 0.0;
        itSrc = ssrc.begin();
        itEnd = ssrc.end();
        itLutR = lutR[n][m].begin();
        itLutI = lutI[n][m].begin();
        for(;itSrc != itEnd;
            ++itSrc,++itLutR,++itLutI) {
          coffRTemp += *itSrc * *itLutR;
          coffITemp -= *itSrc * *itLutI;
        }

        coffR.at(n,m) = coffRTemp*norm*factor;
        coffI.at(n,m) = coffITemp*norm*factor;

        if (param.polar) {
          // magnitude and phase
          coffMag.at(n,m) = sqrt(coffR.at(n,m)*coffR.at(n,m)
                                 +coffI.at(n,m)*coffI.at(n,m));
          coffPha.at(n,m) = atan2(coffI.at(n,m),coffR.at(n,m));
        }
      }
    }

    if (param.polar) {
      dest1.copy(coffMag);
      dest2.copy(coffPha);
    }

    return true;

  };

  // On place apply for type channel8!
  bool regionShapeFeatures::apply(const channel8& src,
                                  channel& dest) const {
    channel dest2;
    return apply(src,dest,dest2);
  }

  bool regionShapeFeatures::apply(const channel8& src,
                                        dvector& dest1,
                                        dvector& dest2) const {
    channel c1,c2;
    if (apply(src,c1,c2)) {
      channel::const_iterator it,eit;
      dvector::iterator vit;
      dest1.resize(c1.rows()*c1.columns(),0.0,false,false);
      for (it=c1.begin(),eit=c1.end(),vit=dest1.begin();it!=eit;++it,++vit) {
        *vit = *it;
      }

      dest2.resize(c2.rows()*c2.columns(),0.0,false,false);
      for (it=c2.begin(),eit=c2.end(),vit=dest2.begin();it!=eit;++it,++vit) {
        *vit = *it;
      }
      return true;
    }
    return false;
  }

  bool regionShapeFeatures::apply(const channel8& src,
                                        dvector& dest) const {
    channel c1;
    if (apply(src,c1)) {
      channel::const_iterator it,eit;
      dvector::iterator vit;
      dest.resize(c1.rows()*c1.columns(),0.0,false,false);
      for (it=c1.begin(),eit=c1.end(),vit=dest.begin();it!=eit;++it,++vit) {
        *vit = *it;
      }
      return true;
    }
    return false;
  }

  bool regionShapeFeatures::getBasisFunction(const int n, const int m,
                                             channel& real,
                                             channel& imag) const {
    if (lutR.empty()) {
      setStatusString("No valid parameters set yet");
      return false;
    }

    if ((n >= static_cast<int>(lutR.size())) ||
        (m >= static_cast<int>(lutR[0].size()))) {
      setStatusString("Invalid indices");
      return false;
    }

    real.copy(lutR[n][m]);
    imag.copy(lutI[n][m]);

    return true;
  }


  bool regionShapeFeatures::reconstruct(const channel& coef1,
                                        const channel& coef2,
                                        channel& shapeRe,
                                        channel& shapeIm) const {
    const parameters& param = getParameters();

    if (coef1.empty() || coef2.empty()) {
      setStatusString("Coefficient channels empty");
      shapeRe.clear();
      shapeIm.clear();
      return false;
    }

    if (coef1.size() != coef2.size()) {
      setStatusString("coefficient channel must have the same size");
      return false;
    }

    channel re,im;

    if (param.polar) {
      polarToCartesian<float> ptc;
      ptc.apply(coef1,coef2,re,im);
    } else {
      re.copy(coef1);
      im.copy(coef2);
    }

    shapeRe.resize(param.maskSize,param.maskSize,0,false,true);
    shapeIm.resize(param.maskSize,param.maskSize,0,false,true);

    int n,m;

    for (n=0;n<param.size.y;++n) {
      for (m=0;m<param.size.x;++m) {
          // the Zernike condition for a valid coefficient is n-m even
          if((param.basisFunctionSet!=parameters::Zernike) ||(abs(n-m)%2 == 0)) {
            shapeRe.addScaled(re.at(n,m),lutR[n][m]);
            shapeRe.addScaled(-im.at(n,m),lutI[n][m]);
            shapeIm.addScaled(im.at(n,m),lutR[n][m]);
            shapeIm.addScaled(re.at(n,m),lutI[n][m]);
          }
      }
    }

    static const float pi2 = static_cast<float>(2.0f*Pi);
    shapeIm.multiply(pi2);
    shapeRe.multiply(pi2);

    if (param.polar) {
      cartesianToPolar<float> ctp;
      ctp.apply(shapeRe,shapeIm);
    }

    return true;
  }


  double regionShapeFeatures::fac(int arg){

    double result=1;
    if (arg<2) {
      return 1;
    }

    while(arg>1){

      result*=arg;
      arg--;
    }

    return result;
  }
}
