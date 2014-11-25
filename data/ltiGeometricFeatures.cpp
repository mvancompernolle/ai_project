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
 * file .......: ltiGeometricFeatures.cpp
 * authors ....: Ulle Canzler
 * organization: LTI, RWTH Aachen
 * creation ...: 11.1.2001
 * revisions ..: $Id: ltiGeometricFeatures.cpp,v 1.16 2006/09/05 10:13:46 ltilib Exp $
 */

#include "ltiMacroSymbols.h"

#ifdef _LTI_MSC_6
#pragma warning( disable : 4786 )
#endif

//include files
#include "ltiGeometricFeatures.h"
#include "ltiObject.h"
#include "ltiObjectsFromMask.h" // for chain codes
#include "ltiException.h"

#include <set>
#include <limits>

namespace lti {
  // --------------------------------------------------
  // geometricFeatures::parameters
  // --------------------------------------------------

  // direction array
  const int geometricFeatures::direction[3][3] =
    {{objectsFromMask::NW,objectsFromMask::W,objectsFromMask::SW},
    {objectsFromMask::N,objectsFromMask::Invalid,objectsFromMask::S},
    {objectsFromMask::NE,objectsFromMask::E,objectsFromMask::SE}};

  // outerBoundaryLength array (Canzler Codes)
  const int geometricFeatures::outerBoundaryLength[8][8] =
    {{1,2,2,3,3,-1,-1,1}, // 0
    {1,2,2,3,3,4,-1,1},   // 1
    {-1,1,1,2,2,3,3,-1},  // 2
    {-1,1,1,2,2,3,3,4},   // 3
    {3,-1,-1,1,1,2,2,3},  // 4
    {3,4,-1,1,1,2,2,3},   // 5
    {2,3,3,-1,-1,1,1,2},  // 6
    {2,3,3,4,-1,1,1,2}};  // 7

  // centralBoundaryLength array
  const double geometricFeatures::centralBoundaryLength[3][3] =
    {{sqrt(2.0f),1,sqrt(2.0f)}, // 0
    {1,-1,1},                   // 1
    {sqrt(2.0f),1,sqrt(2.0f)}}; // 2

  // default constructor
  geometricFeatures::parameters::parameters()
    : globalFeatureExtractor::parameters() {
    //Initialize parameter values!

    calcFeatureGroup1 = true;
    calcFeatureGroup2 = true;
    calcFeatureGroup3 = true;
    calcFeatureGroup4 = true;

    boundaryDefinition = CentralBoundary;
  }

  // copy constructor
  geometricFeatures::parameters::parameters(const parameters& other)
    : globalFeatureExtractor::parameters()  {
    copy(other);
  }

  // destructor
  geometricFeatures::parameters::~parameters() {
  }

  // get type name
  const char* geometricFeatures::parameters::getTypeName() const {
    return "geometricFeatures::parameters";
  }

  // copy member

  geometricFeatures::parameters&
    geometricFeatures::parameters::copy(const parameters& other) {
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

      calcFeatureGroup1 = other.calcFeatureGroup1;
      calcFeatureGroup2 = other.calcFeatureGroup2;
      calcFeatureGroup3 = other.calcFeatureGroup3;
      calcFeatureGroup4 = other.calcFeatureGroup4;
      boundaryDefinition = other.boundaryDefinition;

    return *this;
  }

  // clone member
  functor::parameters* geometricFeatures::parameters::clone() const {
    return new parameters(*this);
  }

# ifndef _LTI_MSC_6
  bool geometricFeatures::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool geometricFeatures::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      b=b && lti::write(handler,"calcFeatureGroup1",calcFeatureGroup1);
      b=b && lti::write(handler,"calcFeatureGroup2",calcFeatureGroup2);
      b=b && lti::write(handler,"calcFeatureGroup3",calcFeatureGroup3);
      b=b && lti::write(handler,"calcFeatureGroup4",calcFeatureGroup4);
      switch (boundaryDefinition) {
        case CentralBoundary:
          b=b && lti::write(handler,"boundaryDefinition","CentralBoundary");
          break;
        case OuterBoundary:
          b=b && lti::write(handler,"boundaryDefinition","OuterBoundary");
          break;
        case Approximation:
          b=b && lti::write(handler,"boundaryDefinition","Approximation");
          break;
        default:
          b=false;
          lti::write(handler, "boundaryDefinition","unknown");
          break;
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::write(handler,false);
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
  bool geometricFeatures::parameters::write(ioHandler& handler,
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
  bool geometricFeatures::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool geometricFeatures::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      b=b && lti::read(handler,"calcFeatureGroup1",calcFeatureGroup1);
      b=b && lti::read(handler,"calcFeatureGroup2",calcFeatureGroup2);
      b=b && lti::read(handler,"calcFeatureGroup3",calcFeatureGroup3);
      b=b && lti::read(handler,"calcFeatureGroup4",calcFeatureGroup4);
      std::string tmp;
      b=b && lti::read(handler,"boundaryDefinition",tmp);
      if (tmp == "CentralBoundary") {
        boundaryDefinition=CentralBoundary;
      } else if (tmp == "OuterBoundary") {
        boundaryDefinition=OuterBoundary;
      } else if (tmp == "Approximation") {
        boundaryDefinition=Approximation;
      } else {
        b=false;
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::read(handler,false);
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
  bool geometricFeatures::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // geometricFeatures
  // --------------------------------------------------

  // default constructor
  geometricFeatures::geometricFeatures()
    : globalFeatureExtractor(){
    parameters param;
    setParameters(param);
  }

  // default constructor
  geometricFeatures::geometricFeatures(const parameters& par)
    : globalFeatureExtractor(){
    setParameters(par);
  }

  // copy constructor
  geometricFeatures::geometricFeatures(const geometricFeatures& other)
    : globalFeatureExtractor()  {
    copy(other);
  }

  // destructor
  geometricFeatures::~geometricFeatures() {
  }

  // returns the name of this type
  const char* geometricFeatures::getTypeName() const {
    return "geometricFeatures";
  }

  // copy member
  geometricFeatures&
  geometricFeatures::copy(const geometricFeatures& other) {
    globalFeatureExtractor::copy(other);
    return (*this);
  }

  // copy member
  geometricFeatures&
  geometricFeatures::operator=(const geometricFeatures& other) {
    return copy(other);
  }

  // clone member
  functor* geometricFeatures::clone() const {
    return new geometricFeatures(*this);
  }

  // return parameters
  const geometricFeatures::parameters&
    geometricFeatures::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }


  // *************************************************************************
  inline double geometricFeatures::logn( double x ) const {
    if (abs(x) > std::numeric_limits<double>::min()) {
      if (x>0.0) {
        return log(x);
      }
      else {
        return -log(-x);
      }
    }
    else {
      return log(std::numeric_limits<double>::min());
    }
  }

  inline double geometricFeatures::divide ( double numerator, 
                                            double denominator ) const {
    if( denominator != 0.0 ) {
      return (numerator/denominator);
    }
    else {
      return 0.0;
    }
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------
  // On place apply for type dvector!
  bool geometricFeatures::apply(const lti::areaPoints& ap,
                                dvector& features) const {

    // initialize vector
    features.resize(numberOfFeatures,0,false,true);
    if (ap.size()==0) {
      setStatusString("areaPoints are empty!");
      return false;
    }

    // get parameters
    const parameters& theParams = getParameters();
    parameters mParams = theParams;

    if (mParams.calcFeatureGroup4) mParams.calcFeatureGroup1=true;
    if (mParams.calcFeatureGroup3) mParams.calcFeatureGroup1=true;
    if (mParams.calcFeatureGroup2) mParams.calcFeatureGroup1=true;

    // initialize variables
    int i,x,y;
    double xCog=0,yCog=0; // center of gravity
    double m00,m10,m01,m11,m20,m02,m30,m03,m12,m21; // moments
    m00=m10=m01=m11=m20=m02=m30=m03=m12=m21=0;

    borderPoints bp;
    bp.castFrom(ap); // compute borderPoints!
    areaPoints::const_iterator itAp = ap.begin();
    borderPoints::const_iterator itBp = bp.begin();

    if (mParams.boundaryDefinition == parameters::OuterBoundary) {
      // bounding rectangle
      int xmin=itAp->x;
      int ymin=itAp->y;
      int xmax=0;
      int ymax=0;

      m00 = 0;

      // compute area size and COG
      for (; itAp != ap.end(); itAp++) {
        ++m00; // area == number of areaPoints in list
        x=(*itAp).x;
        y=(*itAp).y;
        xCog+=x; // part 1 of
        yCog+=y; // COG computation
        if (x < xmin) xmin=x;
        if (x > xmax) xmax=x;
        if (y < ymin) ymin=y;
        if (y > ymax) ymax=y;
      }
      xCog/=m00; // part 2 of
      yCog/=m00; // COG computation

      // compute boundary length
      int boundaryLength;
      if (bp.size() < 2) { // special case
        boundaryLength = 4;
      } else { // general case; at least two points!
        boundaryLength = 0;
        int dirBuf[2] = {lti::objectsFromMask::Invalid,
                         lti::objectsFromMask::Invalid};
        int dirIndex = 0; // alternating 2-int buffer index (0 <-> 1)
        lti::point step;

        // for each boundary contribution, three pixels are to be considered.
        // the first pixel is subsequent to the last, thus there are two
        // special cases: butlast-last-first, and last-first-second.
        // treat these now:
        lti::point firstStep, lastStep; // first-second and
                                        // butlast-last; last-first -> step
        step = bp.front() - bp.back();
        itBp = bp.begin();
        itBp++;
        firstStep = *itBp - bp.front();
        itBp = bp.end();
        itBp--;
        itBp--;
        lastStep = bp.back() - *itBp;
        // butlast-last-first
        dirBuf[0] = direction[lastStep.x+1][lastStep.y+1];
        dirBuf[1] = direction[step.x+1][step.y+1];
        boundaryLength += outerBoundaryLength[dirBuf[0]][dirBuf[1]];
        // last-first-second
        dirBuf[0] = direction[step.x+1][step.y+1];
        dirBuf[1] = direction[firstStep.x+1][firstStep.y+1];
        boundaryLength += outerBoundaryLength[dirBuf[0]][dirBuf[1]];

        // now treat non-special cases; we start with the second step,
        // i.e. second-third
        itBp = bp.begin();
        itBp++;
        borderPoints::const_iterator previous = itBp++;
        for (; itBp != bp.end(); previous = itBp++) {
          step = *itBp - *previous;
          dirBuf[dirIndex] = direction[step.x+1][step.y+1];
          boundaryLength +=
            outerBoundaryLength[dirBuf[1-dirIndex]][dirBuf[dirIndex]];
          dirIndex = 1 - dirIndex;
        }
      }

      // insert results into feature vector
      features[areasize]=m00;
      features[bordersize]=boundaryLength;
      features[xcog]=xCog;
      features[ycog]=yCog;
      features[geometricFeatures::xmin]=xmin;
      features[geometricFeatures::xmax]=xmax;
      features[geometricFeatures::ymin]=ymin;
      features[geometricFeatures::ymax]=ymax;

    } else if (mParams.boundaryDefinition == parameters::CentralBoundary) {
      int xmin=bp.front().x;
      int ymin=bp.front().y;
      int xmax=0;
      int ymax=0;

      lti::point step;
      int sum = 0;
      float boundaryLength = 0;
      lti::borderPoints::const_iterator previous;
      // compute areasize, boundarylength, COG
      if (bp.size() >= 2) {
        int xCog_int = 0, yCog_int = 0;
        // there is one special case: last-first; treat this first by
        // appropriate initialization
        itBp = bp.begin();
        previous = bp.end();
        previous--;
        do {
          sum += previous->x*itBp->y - itBp->x*previous->y;
          step = *itBp - *previous;
          boundaryLength +=
            static_cast<float>(centralBoundaryLength[step.x+1][step.y+1]);
          xCog_int += (previous->x*itBp->y - itBp->x*previous->y)*
                      (previous->x + itBp->x);
          yCog_int += (previous->x*itBp->y - itBp->x*previous->y)*
                      (previous->y + itBp->y);
          previous = itBp++;
        } while (itBp != bp.end());

        m00 = float(sum)/2.0f;
        xCog = float(xCog_int) / (6*m00);
        yCog = float(yCog_int) / (6*m00);
      } else { // can only be single pixel -> area = boundary = 0, COG
               // = center of that pixel
        m00 = 0;
        xCog = bp.front().x;
        yCog = bp.front().y;
      }

      // compute min,max
      for (itBp = bp.begin(); itBp != bp.end(); itBp++) {
        x = itBp->x;
        y = itBp->y;
        if (x < xmin) xmin=x;
        if (x > xmax) xmax=x;
        if (y < ymin) ymin=y;
        if (y > ymax) ymax=y;
      }

      // insert results into feature vector
      features[areasize]=m00;
      features[bordersize]=boundaryLength;
      features[xcog]=xCog;
      features[ycog]=yCog;
      features[geometricFeatures::xmin]=xmin;
      features[geometricFeatures::xmax]=xmax;
      features[geometricFeatures::ymin]=ymin;
      features[geometricFeatures::ymax]=ymax;
    } else if (mParams.boundaryDefinition == parameters::Approximation) {
      int xmin=itAp->x;
      int ymin=itAp->y;
      int xmax=0;
      int ymax=0;

      lti::point step;
      float boundaryLength = 0;
      lti::borderPoints::const_iterator lastPoint;
      m00 = ap.size();
      if (m00 == 1) { // treat special case of single pixel
        boundaryLength = 1; // somewhat arbitrary value (should be < 4)
      } else if (bp.size() >= 2) {
        // there is one special case: last-first; treat this now:
        step = bp.front() - bp.back();
        boundaryLength +=
          static_cast<float>(centralBoundaryLength[step.x+1][step.y+1]);
        // now for all non-special cases:
        itBp = bp.begin();
        lastPoint = itBp++;
        for (; itBp != bp.end(); lastPoint = itBp++) {
          step = *itBp - *lastPoint;
          boundaryLength +=
            static_cast<float>(centralBoundaryLength[step.x+1][step.y+1]);
        }
      }

      // compute COG,min,max like for OuterBoundary
      for (; itAp != ap.end(); itAp++) {
        x=(*itAp).x;
        y=(*itAp).y;
        xCog+=x; // part 1 of
        yCog+=y; // COG computation
        if (x < xmin) xmin=x;
        if (x > xmax) xmax=x;
        if (y < ymin) ymin=y;
        if (y > ymax) ymax=y;
      }
      xCog /= m00; // part 2 of
      yCog /= m00; // COG computation

      // insert results into feature vector
      features[areasize]=m00;
      features[bordersize]=boundaryLength;
      features[xcog]=xCog;
      features[ycog]=yCog;
      features[geometricFeatures::xmin]=xmin;
      features[geometricFeatures::xmax]=xmax;
      features[geometricFeatures::ymin]=ymin;
      features[geometricFeatures::ymax]=ymax;
    }

    // compute compactness from the above calculated features
    features[geometricFeatures::compactness]=
      4*Pi*divide(features[areasize],
                  features[bordersize]*features[bordersize]);

    //################### 1111111111111111111111111111111 ###################
    if (mParams.calcFeatureGroup1) {

      if (mParams.boundaryDefinition == parameters::CentralBoundary) {
        if (bp.size() >= 2) { // otherwise, all moments remain zero!
          double a10,a01,a11,a20,a02,a30,a03,a12,a21;
          a10=a01=a11=a20=a02=a30=a03=a12=a21=0;

          itBp = bp.begin();
          borderPoints::const_iterator previous = bp.end();
          previous--; // treat special case first (done in do loop)

          double xCog = features[xcog];
          double yCog = features[ycog];
          double area_pow1 = features[areasize];
          double area_pow1_5 = pow(area_pow1,double(1.5));
          double x_i, x_i1, y_i, y_i1;

          do {
            x_i1 = double(previous->x) - xCog;
            x_i = double(itBp->x) - xCog;
            y_i1 = double(previous->y) - yCog;
            y_i = double(itBp->y) - yCog;
            a10 += (x_i1*y_i - x_i*y_i1)*(x_i1 + x_i);
            a01 += (x_i1*y_i - x_i*y_i1)*(y_i1 + y_i);
            a20 += (x_i1*y_i - x_i*y_i1)*(x_i1*x_i1 + x_i1*x_i + x_i*x_i);
            a02 += (x_i1*y_i - x_i*y_i1)*(y_i1*y_i1 + y_i1*y_i + y_i*y_i);
            a11 += (x_i1*y_i - x_i*y_i1)*(2*x_i1*y_i1 + x_i1*y_i + x_i*y_i1 + 2*x_i*y_i);
            a12 += (x_i1*y_i - x_i*y_i1)*(3*x_i1*y_i1*y_i1 + 2*x_i1*y_i*y_i1 + x_i1*y_i*y_i + x_i*y_i1 + 2*x_i*y_i*y_i1 + 3*x_i*y_i*y_i);
            a21 += (x_i1*y_i - x_i*y_i1)*(3*y_i1*x_i1*x_i1 + 2*y_i1*x_i*x_i1 + y_i1*x_i*x_i + y_i*x_i1 + 2*y_i*x_i*x_i1 + 3*y_i*x_i*x_i);
            a30 += (x_i1*y_i - x_i*y_i1)*(x_i1*x_i1*x_i1 + x_i*x_i1*x_i1 + x_i*x_i*x_i1 + x_i*x_i*x_i);
            a03 += (x_i1*y_i - x_i*y_i1)*(y_i1*y_i1*y_i1 + y_i*y_i1*y_i1 + y_i*y_i*y_i1 + y_i*y_i*y_i);

            previous = itBp++;
          } while (itBp != bp.end());

          a10 /= 6*area_pow1;
          a01 /= 6*area_pow1;
          a20 /= 12*area_pow1;
          a11 /= 24*area_pow1;
          a02 /= 12*area_pow1;
          a12 /= 60*area_pow1;
          a21 /= 60*area_pow1;
          a30 /= 20*area_pow1;
          a03 /= 20*area_pow1;

          m02 = (a02 - a01*a01) / area_pow1;
          m20 = (a20 - a10*a10) / area_pow1;
          m11 = (a11 - a10*a01) / area_pow1;
          m12 = (2*(a10*a01*a01 - a01*a11) - a10*a02 + a12) / area_pow1_5;
          m21 = (2*(a01*a10*a10 - a10*a11) - a01*a20 + a21) / area_pow1_5;
          m30 = (2*a10*a10*a10 - 3*a10*a20 + a30) / area_pow1_5;
          m03 = (2*a01*a01*a01 - 3*a01*a02 + a03) / area_pow1_5;
        }
      } else if (mParams.boundaryDefinition == parameters::OuterBoundary
                  || mParams.boundaryDefinition == parameters::Approximation) {
        // calculate central unnormalized moments
        double m00pow2,m00pow2_5;
        m00pow2 = features[areasize]*features[areasize];
        m00pow2_5 = pow(features[areasize],2.5);
        itAp=ap.begin();
        for (i=0; i < ap.size(); i++) {
          x=(*itAp).x;
          y=(*itAp).y;
          m11+=(x-features[xcog])*(y-features[ycog]);
          m20+=(x-features[xcog])*(x-features[xcog]);
          m02+=(y-features[ycog])*(y-features[ycog]);
          m30+=(x-features[xcog])*(x-features[xcog])*(x-features[xcog]);
          m03+=(y-features[ycog])*(y-features[ycog])*(y-features[ycog]);
          m12+=(x-features[xcog])*(y-features[ycog])*(y-features[ycog]);
          m21+=(x-features[xcog])*(x-features[xcog])*(y-features[ycog]);
          itAp++;
        }
        // normalized central moments
        m02 = divide(m02,m00pow2);
        m03 = divide(m03,m00pow2_5);
        m11 = divide(m11,m00pow2);
        m12 = divide(m12,m00pow2_5);
        m20 = divide(m20,m00pow2);
        m21 = divide(m21,m00pow2_5);
        m30 = divide(m30,m00pow2_5);
      }
      // insert results into feature vector
      features[geometricFeatures::m02]=m02;
      features[geometricFeatures::m03]=m03;
      features[geometricFeatures::m11]=m11;
      features[geometricFeatures::m12]=m12;
      features[geometricFeatures::m20]=m20;
      features[geometricFeatures::m21]=m21;
      features[geometricFeatures::m30]=m30;

      // orientation and eccentricity can now also be calculated, as well as eigenvalues j1 and j2
      features[geometricFeatures::orientation]=0.5*atan2( 2*m11, m20 - m02 )*180/Pi;
      features[geometricFeatures::eccentricity]=
        divide( ((m20-m02)*(m20-m02) + 4*m11*m11), (m20+m02)*(m20+m02) );

      double temp = sqrt( (m20 - m02)*(m20 - m02) + 4*m11*m11 );
      // insert inertia into feature vector
      features[geometricFeatures::j1]=m00*0.5*((m20+m02) + temp);
      features[geometricFeatures::j2]=m00*0.5*((m20+m02) - temp);
    }
    //################### 22222222222222222222222 ###################
    if (mParams.calcFeatureGroup2) {
      // this group builds upon the moments calculated in group 1;
      // no differentiation between boundary types is required here.
      double rot1,rot2,rot3,rot4,rot5,rot6,rot7;
      rot1=rot2=rot3=rot4=rot5=rot6=rot7=0.0;

      // calculate moment invariants
      rot1 = -logn(m20 + m02);

      rot2 = -logn(pow((m20 - m02),2) + 4*pow(m11,2));

      rot3 = -logn(pow((m30 - 3*m12),2) + pow((3*m21 - m03),2));

      rot4 = -logn(pow((m30 + m12),2) + pow((m21 + m03),2));

      rot5 = -logn(  (m30-3*m12)*(m30+m12)*(   pow((m30+m12),2) - 3*pow((m21+m03),2) )
                   + (3*m21-m03)*(m21+m03)*( 3*pow((m30+m12),2) -   pow((m21+m03),2) ) );

      rot6 = -logn(  (m20-m02)*( pow((m30+m12),2) - pow((m21+m03),2) )
                   + 4*m11*(m30+m12)*(m21+m03) );

      rot7 = -logn(  (3*m21-m03)*(m30+m12)*(   pow((m30+m12),2) - 3*pow((m21+m03),2) )
                   - (m30-3*m12)*(m21+m03)*( 3*pow((m30+m12),2) -   pow((m21+m03),2) ));

      // insert results into feature vector
      features[geometricFeatures::hu1]=rot1;
      features[geometricFeatures::hu2]=rot2;
      features[geometricFeatures::hu3]=rot3;
      features[geometricFeatures::hu4]=rot4;
      features[geometricFeatures::hu5]=rot5;
      features[geometricFeatures::hu6]=rot6;
      features[geometricFeatures::hu7]=rot7;
    }
    //################### 33333333333333333333333333333 ###################
    if (mParams.calcFeatureGroup3) {
      // Empty for historic reasons
    }
    //################### 444444444444444444444444444 ###################
    if (mParams.calcFeatureGroup4) {
      double rmean,rmin,rmax,dleft,dright,dfront,drear,w1,r1,temporient,ftemp,orient;

      double sina, cosa;

      orient = features[geometricFeatures::orientation]*Pi/180.0f;

      // initialize variables
      x=(*itBp).x;
      y=(*itBp).y;
      dleft = dright = dfront = drear = 0.0;
      rmean = rmax = 0.0;
      rmin = sqrt( (y-features[ycog])*(y-features[ycog]) + (x-features[xcog])*(x-features[xcog]) );
      itBp=bp.begin();

      // calculate distance features
      for (i=0;i<bp.size();i++) {
        x=(*itBp).x;
        y=(*itBp).y;

        // 1.) calculate angle and distance
        w1 = atan2(y-features[ycog],x-features[xcog]);
        r1 = sqrt( (y-features[ycog])*(y-features[ycog]) + (x-features[xcog])*(x-features[xcog]) );
        rmean += r1;  // sum distances
        if (r1 > rmax) rmax = r1;
        if (r1 < rmin) rmin = r1;

        // 2.) compute angle relative to orientation
        temporient = w1 - orient;
        if (temporient <= -Pi) temporient += 2*Pi;
        if (temporient > +Pi)  temporient -= 2*Pi;

        // 3.) compute maximum distances along and perpendicular to main axis
        sincos(temporient, sina, cosa);
        if (temporient >= 0.0) {
          ftemp = fabs( r1 * sina );
          if ( ftemp > dleft ) {
            dleft = ftemp;
          }
        } else {
          ftemp = fabs( r1 * sina );
          if ( ftemp > dright ) {
            dright = ftemp;
          }
        }

        if ( -Pi/2 <= temporient && temporient <= Pi/2 ) {
          ftemp = fabs( r1 * cosa );
          if ( ftemp > drear ) {
            drear = ftemp;
          }
        } else {
          ftemp = fabs( r1 * cosa );
          if ( ftemp > dfront ) {
            dfront = ftemp;
          }
        }

        itBp++;
      }

      rmean = divide (rmean , bp.size());

      // insert results into feature vector
      features[geometricFeatures::rmin]=rmin;
      features[geometricFeatures::rmax]=rmax;
      features[geometricFeatures::rmean]=rmean;
      features[geometricFeatures::dleft]=dleft;
      features[geometricFeatures::dright]=dright;
      features[geometricFeatures::dfront]=dfront;
      features[geometricFeatures::drear]=drear;
    }

    return true;
  }

  // On place apply for type dvector!
  bool geometricFeatures::apply(const lti::borderPoints& bp,
                                dvector& features) const {

    // initialize vector
    features.resize(numberOfFeatures,0,false,true);
    if (bp.size()==0) {
      setStatusString("borderPoints are empty!");
      return false;
    }

    // get parameters
    const parameters& theParams = getParameters();
    parameters mParams = theParams;

    if (mParams.calcFeatureGroup4) mParams.calcFeatureGroup1=true;
    if (mParams.calcFeatureGroup3) mParams.calcFeatureGroup1=true;
    if (mParams.calcFeatureGroup2) mParams.calcFeatureGroup1=true;

    // initialize variables
    int i,x,y;
    double xCog=0,yCog=0; // center of gravity
    double m00,m10,m01,m11,m20,m02,m30,m03,m12,m21; // moments
    m00=m10=m01=m11=m20=m02=m30=m03=m12=m21=0;

    borderPoints::const_iterator itBp = bp.begin();

    int xmin=bp.front().x;
    int ymin=bp.front().y;
    int xmax=0;
    int ymax=0;

    lti::point step;
    int sum = 0;
    float boundaryLength = 0;
    lti::borderPoints::const_iterator previous;
    // compute areasize, boundarylength, COG
    if (bp.size() >= 2) {
      int xCog_int = 0, yCog_int = 0;
      // there is one special case: last-first; treat this first by appropriate initialization
      itBp = bp.begin();
      previous = bp.end();
      previous--;
      do {
        sum += previous->x*itBp->y - itBp->x*previous->y;
        step = *itBp - *previous;
        boundaryLength +=
          static_cast<float>(centralBoundaryLength[step.x+1][step.y+1]);
        xCog_int += (previous->x*itBp->y - itBp->x*previous->y)*
                    (previous->x + itBp->x);
        yCog_int += (previous->x*itBp->y - itBp->x*previous->y)*
                    (previous->y + itBp->y);
        previous = itBp++;
      } while (itBp != bp.end());

      m00 = float(sum)/2.0f;
      if (m00 == 0) { // object has no area -> compute COG as with OuterBoundary
        computeMasslessCOG(bp,xCog,yCog);
      } else {
        xCog = float(xCog_int) / (6*m00);
        yCog = float(yCog_int) / (6*m00);
      }
    } else { // can only be single pixel -> area = boundary = 0, COG = center of that pixel
      m00 = 0;
      xCog = bp.front().x;
      yCog = bp.front().y;
    }

    // compute min,max
    for (itBp = bp.begin(); itBp != bp.end(); itBp++) {
      x = itBp->x;
      y = itBp->y;
      if (x < xmin) xmin=x;
      if (x > xmax) xmax=x;
      if (y < ymin) ymin=y;
      if (y > ymax) ymax=y;
    }

    // insert results into feature vector
    features[areasize]=m00;
    features[bordersize]=boundaryLength;
    features[xcog]=xCog;
    features[ycog]=yCog;
    features[geometricFeatures::xmin]=xmin;
    features[geometricFeatures::xmax]=xmax;
    features[geometricFeatures::ymin]=ymin;
    features[geometricFeatures::ymax]=ymax;

    // compute compactness from the above calculated features
    features[geometricFeatures::compactness]=
      4*Pi*divide(features[areasize],features[bordersize]*features[bordersize]);

    //################### 1111111111111111111111111111111 ###################
    if (mParams.calcFeatureGroup1) {

      if (bp.size() >= 2) { // otherwise, all moments remain zero!
        double a10,a01,a11,a20,a02,a30,a03,a12,a21;
        a10=a01=a11=a20=a02=a30=a03=a12=a21=0;

        itBp = bp.begin();
        borderPoints::const_iterator previous = bp.end();
        previous--; // treat special case first (done in do loop)

        double xCog = features[xcog];
        double yCog = features[ycog];
        double area_pow1 = features[areasize];
        double area_pow1_5 = pow(area_pow1,double(1.5));
        double x_i, x_i1, y_i, y_i1;

        do {
          x_i1 = double(previous->x) - xCog;
          x_i = double(itBp->x) - xCog;
          y_i1 = double(previous->y) - yCog;
          y_i = double(itBp->y) - yCog;
          a10 += (x_i1*y_i - x_i*y_i1)*(x_i1 + x_i);
          a01 += (x_i1*y_i - x_i*y_i1)*(y_i1 + y_i);
          a20 += (x_i1*y_i - x_i*y_i1)*(x_i1*x_i1 + x_i1*x_i + x_i*x_i);
          a02 += (x_i1*y_i - x_i*y_i1)*(y_i1*y_i1 + y_i1*y_i + y_i*y_i);
          a11 += (x_i1*y_i - x_i*y_i1)*(2*x_i1*y_i1 + x_i1*y_i + x_i*y_i1 + 2*x_i*y_i);
          a12 += (x_i1*y_i - x_i*y_i1)*(3*x_i1*y_i1*y_i1 + 2*x_i1*y_i*y_i1 + x_i1*y_i*y_i + x_i*y_i1 + 2*x_i*y_i*y_i1 + 3*x_i*y_i*y_i);
          a21 += (x_i1*y_i - x_i*y_i1)*(3*y_i1*x_i1*x_i1 + 2*y_i1*x_i*x_i1 + y_i1*x_i*x_i + y_i*x_i1 + 2*y_i*x_i*x_i1 + 3*y_i*x_i*x_i);
          a30 += (x_i1*y_i - x_i*y_i1)*(x_i1*x_i1*x_i1 + x_i*x_i1*x_i1 + x_i*x_i*x_i1 + x_i*x_i*x_i);
          a03 += (x_i1*y_i - x_i*y_i1)*(y_i1*y_i1*y_i1 + y_i*y_i1*y_i1 + y_i*y_i*y_i1 + y_i*y_i*y_i);

          previous = itBp++;
        } while (itBp != bp.end());

        a10 /= 6*area_pow1;
        a01 /= 6*area_pow1;
        a20 /= 12*area_pow1;
        a11 /= 24*area_pow1;
        a02 /= 12*area_pow1;
        a12 /= 60*area_pow1;
        a21 /= 60*area_pow1;
        a30 /= 20*area_pow1;
        a03 /= 20*area_pow1;

        m02 = (a02 - a01*a01) / area_pow1;
        m20 = (a20 - a10*a10) / area_pow1;
        m11 = (a11 - a10*a01) / area_pow1;
        m12 = (2*(a10*a01*a01 - a01*a11) - a10*a02 + a12) / area_pow1_5;
        m21 = (2*(a01*a10*a10 - a10*a11) - a01*a20 + a21) / area_pow1_5;
        m30 = (2*a10*a10*a10 - 3*a10*a20 + a30) / area_pow1_5;
        m03 = (2*a01*a01*a01 - 3*a01*a02 + a03) / area_pow1_5;
      }
      // insert results into feature vector
      features[geometricFeatures::m02]=m02;
      features[geometricFeatures::m03]=m03;
      features[geometricFeatures::m11]=m11;
      features[geometricFeatures::m12]=m12;
      features[geometricFeatures::m20]=m20;
      features[geometricFeatures::m21]=m21;
      features[geometricFeatures::m30]=m30;

      // orientation and eccentricity can now also be calculated, as well as eigenvalues j1 and j2
      features[geometricFeatures::orientation]=0.5*atan2( 2*m11, m20 - m02 )*180/Pi;
      features[geometricFeatures::eccentricity]=
        divide( ((m20-m02)*(m20-m02) + 4*m11*m11), (m20+m02)*(m20+m02) );

      double temp = sqrt( (m20 - m02)*(m20 - m02) + 4*m11*m11 );
      // insert inertia into feature vector
      features[geometricFeatures::j1]=m00*0.5*((m20+m02) + temp);
      features[geometricFeatures::j2]=m00*0.5*((m20+m02) - temp);
    }
    //################### 22222222222222222222222 ###################
    if (mParams.calcFeatureGroup2) {
      // this group builds upon the moments calculated in group 1;
      // no differentiation between boundary types is required here.
      double rot1,rot2,rot3,rot4,rot5,rot6,rot7;
      rot1=rot2=rot3=rot4=rot5=rot6=rot7=0.0;

      // calculate moment invariants
      rot1 = -logn(m20 + m02);

      rot2 = -logn(pow((m20 - m02),2) + 4*pow(m11,2));

      rot3 = -logn(pow((m30 - 3*m12),2) + pow((3*m21 - m03),2));

      rot4 = -logn(pow((m30 + m12),2) + pow((m21 + m03),2));

      rot5 = -logn(  (m30-3*m12)*(m30+m12)*(   pow((m30+m12),2) - 3*pow((m21+m03),2) )
                   + (3*m21-m03)*(m21+m03)*( 3*pow((m30+m12),2) -   pow((m21+m03),2) ) );

      rot6 = -logn(  (m20-m02)*( pow((m30+m12),2) - pow((m21+m03),2) )
                   + 4*m11*(m30+m12)*(m21+m03) );

      rot7 = -logn(  (3*m21-m03)*(m30+m12)*(   pow((m30+m12),2) - 3*pow((m21+m03),2) )
                   - (m30-3*m12)*(m21+m03)*( 3*pow((m30+m12),2) -   pow((m21+m03),2) ));

      // insert results into feature vector
      features[geometricFeatures::hu1]=rot1;
      features[geometricFeatures::hu2]=rot2;
      features[geometricFeatures::hu3]=rot3;
      features[geometricFeatures::hu4]=rot4;
      features[geometricFeatures::hu5]=rot5;
      features[geometricFeatures::hu6]=rot6;
      features[geometricFeatures::hu7]=rot7;
    }
    //################### 33333333333333333333333333333 ###################
    if (mParams.calcFeatureGroup3) {
      // Empty for historic reasons
    }
    //################### 444444444444444444444444444 ###################
    if (mParams.calcFeatureGroup4) {
      double rmean,rmin,rmax,dleft,dright,dfront,drear,w1,r1,temporient,ftemp,orient;

      double sina, cosa;

      orient = features[geometricFeatures::orientation]*Pi/180.0f;

      // initialize variables
      x=(*itBp).x;
      y=(*itBp).y;
      dleft = dright = dfront = drear = 0.0;
      rmean = rmax = 0.0;
      rmin = sqrt( (y-features[ycog])*(y-features[ycog]) + (x-features[xcog])*(x-features[xcog]) );
      itBp=bp.begin();

      // calculate distance features
      for (i=0;i<bp.size();i++) {
        x=(*itBp).x;
        y=(*itBp).y;

        // 1.) calculate angle and distance
        w1 = atan2(y-features[ycog],x-features[xcog]);
        r1 = sqrt( (y-features[ycog])*(y-features[ycog]) + (x-features[xcog])*(x-features[xcog]) );
        rmean += r1;  // sum distances
        if (r1 > rmax) rmax = r1;
        if (r1 < rmin) rmin = r1;

        // 2.) compute angle relative to orientation
        temporient = w1 - orient;
        if (temporient <= -Pi) temporient += 2*Pi;
        if (temporient > +Pi)  temporient -= 2*Pi;

        // 3.) compute maximum distances along and perpendicular to main axis
        sincos(temporient, sina, cosa);
        if (temporient >= 0.0) {
          ftemp = fabs( r1 * sina );
          if ( ftemp > dleft ) {
            dleft = ftemp;
          }
        } else {
          ftemp = fabs( r1 * sina );
          if ( ftemp > dright ) {
            dright = ftemp;
          }
        }

        if ( -Pi/2 <= temporient && temporient <= Pi/2 ) {
          ftemp = fabs( r1 * cosa );
          if ( ftemp > drear ) {
            drear = ftemp;
          }
        } else {
          ftemp = fabs( r1 * cosa );
          if ( ftemp > dfront ) {
            dfront = ftemp;
          }
        }

        itBp++;
      }

      rmean = divide (rmean , bp.size());

      // insert results into feature vector
      features[geometricFeatures::rmin]=rmin;
      features[geometricFeatures::rmax]=rmax;
      features[geometricFeatures::rmean]=rmean;
      features[geometricFeatures::dleft]=dleft;
      features[geometricFeatures::dright]=dright;
      features[geometricFeatures::dfront]=dfront;
      features[geometricFeatures::drear]=drear;
    }

    return true;
  }

  // On place apply for type std::map!
  bool geometricFeatures::apply(const lti::areaPoints& AreaPointList,
                                std::map<std::string,double>& features) const {
    // initialize vector and variables
    dvector theVector;
    if (!apply(AreaPointList,theVector)) {
      return false;
    }
    return mapFeatures(theVector,features);
  }

  // On place apply for type std::map!
  bool geometricFeatures::apply(const lti::borderPoints& BorderPointList,
                                std::map<std::string,double>& features) const {
    dvector theVector;
    if (!apply(BorderPointList,theVector)) {
      return false;
    }
    return mapFeatures(theVector,features);
  }

  // On place apply for type std::map!
  bool geometricFeatures::apply(const lti::ioPoints& IOPointList,
                                std::map<std::string,double>& features) const {
    dvector theVector;
    if (!apply(IOPointList,theVector)) {
      return false;
    }
    return mapFeatures(theVector,features);
  }

  // On place apply for type dvector!
  bool geometricFeatures::apply(const lti::ioPoints& IOPointList,
                                dvector& features) const {
    // initialize vector and variables
    lti::borderPoints BorderPointList;
    lti::ioPoints::const_iterator iterIOPointList;
    lti::point pIn, pOut;
    iterIOPointList=IOPointList.begin();
    while(iterIOPointList != IOPointList.end()) {
      pIn.x=iterIOPointList->x;
      int y=iterIOPointList->y;
      iterIOPointList++;
      pOut.x=iterIOPointList->x;
      iterIOPointList++;
      for (int x=pIn.x; x <= pOut.x; x++)
        BorderPointList.push_back(lti::point(x,y));
    }

    if (!apply(BorderPointList,features)) {
      return false;
    }
    return true;
  }

  // Function for mapping vector to map
  bool 
  geometricFeatures::mapFeatures(dvector& theVector, 
                                 std::map<std::string,double>& theMap) const {
    
    theMap.clear();
    try {
      theMap["areasize"]=theVector[geometricFeatures::areasize];
      theMap["bordersize"]=theVector[geometricFeatures::bordersize];
      theMap["xcog"]=theVector[geometricFeatures::xcog];
      theMap["ycog"]=theVector[geometricFeatures::ycog];
      theMap["xmin"]=theVector[geometricFeatures::xmin];
      theMap["xmax"]=theVector[geometricFeatures::xmax];
      theMap["ymin"]=theVector[geometricFeatures::ymin];
      theMap["ymax"]=theVector[geometricFeatures::ymax];
      theMap["m02"]=theVector[geometricFeatures::m02];
      theMap["m03"]=theVector[geometricFeatures::m03];
      theMap["m11"]=theVector[geometricFeatures::m11];
      theMap["m12"]=theVector[geometricFeatures::m12];
      theMap["m20"]=theVector[geometricFeatures::m20];
      theMap["m21"]=theVector[geometricFeatures::m21];
      theMap["m30"]=theVector[geometricFeatures::m30];
      theMap["hu1"]=theVector[geometricFeatures::hu1];
      theMap["hu2"]=theVector[geometricFeatures::hu2];
      theMap["hu3"]=theVector[geometricFeatures::hu3];
      theMap["hu4"]=theVector[geometricFeatures::hu4];
      theMap["hu5"]=theVector[geometricFeatures::hu5];
      theMap["hu6"]=theVector[geometricFeatures::hu6];
      theMap["hu7"]=theVector[geometricFeatures::hu7];
      theMap["j1"]=theVector[geometricFeatures::j1];
      theMap["j2"]=theVector[geometricFeatures::j2];
      theMap["orientation"]=theVector[geometricFeatures::orientation];
      theMap["eccentricity"]=theVector[geometricFeatures::eccentricity];
      theMap["compactness"]=theVector[geometricFeatures::compactness];
      theMap["rmin"]=theVector[geometricFeatures::rmin];
      theMap["rmax"]=theVector[geometricFeatures::rmax];
      theMap["rmean"]=theVector[geometricFeatures::rmean];
      theMap["dleft"]=theVector[geometricFeatures::dleft];
      theMap["dright"]=theVector[geometricFeatures::dright];
      theMap["dfront"]=theVector[geometricFeatures::dfront];
      theMap["drear"]=theVector[geometricFeatures::drear];
    }
    catch (std::exception e) {
      return false;
    }

    return true;
  }

  float geometricFeatures::getLength(const borderPoints& bp,
    const parameters::eBoundaryDefinition& boundaryDefinition) const {
    if (boundaryDefinition == parameters::CentralBoundary) {
      if (bp.size() < 2) // single pixel -> boundaryLength is 0
        return 0;
      float boundaryLength = 0;
      borderPoints::const_iterator itBp;
      borderPoints::const_iterator previous;
      point step;
      // there is one special case: last-first; treat this first by appropriate initialization
      itBp = bp.begin();
      previous = bp.end();
      previous--;
      do {
        step = *itBp - *previous;
        boundaryLength += static_cast<float>(geometricFeatures::centralBoundaryLength[step.x+1][step.y+1]);
        previous = itBp++;
      } while (itBp != bp.end());
      return boundaryLength;
    } else if (boundaryDefinition == geometricFeatures::parameters::OuterBoundary) {
      if (bp.size() < 2) // special case
        return 4;
      borderPoints::const_iterator itBp;
      borderPoints::const_iterator previous;
      int boundaryLength = 0;
      int dirBuf[2] = {objectsFromMask::Invalid,objectsFromMask::Invalid};
      int dirIndex = 0; // alternating 2-int buffer index (0 <-> 1)
      lti::point step;

      // for each boundary contribution, three pixels are to be considered.
      // the first pixel is subsequent to the last, thus there are two
      // special cases: butlast-last-first, and last-first-second.
      // treat these now:

      // first-second and butlast-last; last-first -> step
      lti::point firstStep, lastStep; 
      step = bp.front() - bp.back();
      itBp = bp.begin();
      itBp++;
      firstStep = *itBp - bp.front();
      itBp = bp.end();
      itBp--;
      itBp--;
      lastStep = bp.back() - *itBp;
      // butlast-last-first
      dirBuf[0] = direction[lastStep.x+1][lastStep.y+1];
      dirBuf[1] = direction[step.x+1][step.y+1];
      boundaryLength += outerBoundaryLength[dirBuf[0]][dirBuf[1]];
      // last-first-second
      dirBuf[0] = direction[step.x+1][step.y+1];
      dirBuf[1] = direction[firstStep.x+1][firstStep.y+1];
      boundaryLength += outerBoundaryLength[dirBuf[0]][dirBuf[1]];

      // now treat non-special cases; 
      // we start with the second step, i.e. second-third
      itBp = bp.begin();
      itBp++;
      previous = itBp++;
      for (; itBp != bp.end(); previous = itBp++) {
        step = *itBp - *previous;
        dirBuf[dirIndex] = direction[step.x+1][step.y+1];
        boundaryLength 
          += outerBoundaryLength[dirBuf[1-dirIndex]][dirBuf[dirIndex]];
        dirIndex = 1 - dirIndex;
      }
      return static_cast<float>(boundaryLength);
    } else if (boundaryDefinition == geometricFeatures::parameters::Approximation) {
      float boundaryLength 
        = getLength(bp,geometricFeatures::parameters::CentralBoundary);
      // treat special case of single pixel
      return boundaryLength == 0 ? 1 : boundaryLength; 
    } else { // unknown boundary definition
      throw new exception("Unknown boundary definition: " +boundaryDefinition);
    }
  }

  void geometricFeatures::computeMasslessCOG(const borderPoints& bp,
                                             double& xCog,
                                             double& yCog) const {
    std::set<point> considered;
    std::pair<std::set<point>::iterator,bool> ret;
    borderPoints::const_iterator it;
    xCog = 0;
    yCog = 0;
    for (it = bp.begin(); it != bp.end(); ++it) {
      ret = considered.insert(*it);
      if (ret.second) { // was not yet considered
        xCog += it->x;
        yCog += it->y;
      }
    }
    xCog /= considered.size();
    yCog /= considered.size();
  }

}
