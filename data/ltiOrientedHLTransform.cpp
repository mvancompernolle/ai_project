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
 * file .......: ltiOrientedHLTransform.cpp
 * authors ....: Henning Luepschen
 * organization: LTI, RWTH Aachen
 * creation ...: 02.10.2002
 * revisions ..: $Id: ltiOrientedHLTransform.cpp,v 1.14 2006/09/05 10:24:36 ltilib Exp $
 */

#include "ltiOrientedHLTransform.h"
#include "ltiMath.h"

#include "ltiPointList.h"
#include "ltiSort.h"
#include "ltiGaussianPyramid.h"
#include "ltiMaximumFilter.h"
#include "ltiDownsampling.h"
#include "ltiOrientationMap.h"
#include "ltiCartesianToPolar.h"
#include "ltiGaussKernels.h"

#include <limits>

// #define _LTI_DEBUG 3

#ifdef _LTI_DEBUG
#include "ltiViewer.h"
#include <iostream>
using std::cout;
using std::endl;
#endif

namespace lti {
  // --------------------------------------------------
  // orientedHLTransform::parameters
  // --------------------------------------------------

  // default constructor
  orientedHLTransform::parameters::parameters()
    : transform::parameters() {

    transformationArea = trectangle<int>(0,0,
                                         std::numeric_limits<int>::max(),
                                         std::numeric_limits<int>::max());
    baseValue = int(0);
    accuracy = 180;
    range = 10;
    accumulationMode = Classic;
  }

  // copy constructor
  orientedHLTransform::parameters::parameters(const parameters& other)
    : transform::parameters()  {
    copy(other);
  }

  // destructor
  orientedHLTransform::parameters::~parameters() {
  }

  // get type name
  const char* orientedHLTransform::parameters::getTypeName() const {
    return "orientedHLTransform::parameters";
  }

  // copy member

  orientedHLTransform::parameters&
  orientedHLTransform::parameters::copy(const parameters& other) {
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

    transformationArea = other.transformationArea;
    baseValue = other.baseValue;
    accuracy = other.accuracy;
    range = other.range;
    accumulationMode = other.accumulationMode;

    return *this;
  }

  // alias for copy member
  orientedHLTransform::parameters&
  orientedHLTransform::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* orientedHLTransform::parameters::clone() const {
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
  bool orientedHLTransform::parameters::write(ioHandler& handler,
                                              const bool complete) const
# else
    bool orientedHLTransform::parameters::writeMS(ioHandler& handler,
                                                  const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"transformationArea",transformationArea);
      lti::write(handler,"baseValue",baseValue);
      lti::write(handler,"accuracy",accuracy);
      lti::write(handler,"range",range);
      switch(accumulationMode) {
        case Classic:
          lti::write(handler,"accumulationMode","Classic");
          break;
        case Gradient:
          lti::write(handler,"accumulationMode","Gradient");
          break;
        default:
          lti::write(handler,"accumulationMode","Classic");
          break;
      }

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
  bool orientedHLTransform::parameters::write(ioHandler& handler,
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
  bool orientedHLTransform::parameters::read(ioHandler& handler,
                                             const bool complete)
# else
    bool orientedHLTransform::parameters::readMS(ioHandler& handler,
                                                 const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"transformationArea",transformationArea);
      lti::read(handler,"baseValue",baseValue);
      lti::read(handler,"accuracy",accuracy);
      lti::read(handler,"range",range);
      std::string str;
      lti::read(handler,"accumulationMode",str);
      if (str == "Classic") {
        accumulationMode = Classic;
      } else if (str == "Gradient") {
        accumulationMode = Gradient;
      } else {
        accumulationMode = Classic;
      }
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
  bool orientedHLTransform::parameters::read(ioHandler& handler,
                                             const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // orientedHLTransform
  // --------------------------------------------------

  // default constructor
  orientedHLTransform::orientedHLTransform()
    : transform(){

    m_pCosinus = 0;
    m_pSinus = 0;
    m_iAccuracy = 0;  // will be set to 180 in setParameters

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // copy constructor
  orientedHLTransform::orientedHLTransform(const orientedHLTransform& other)
    : transform()  {

    // To ensure that delete[] in createSinCosTables() works correctly.
    m_pSinus = 0;
    m_pCosinus = 0;
    m_iAccuracy = 0;

    copy(other);
  }

  // destructor
  orientedHLTransform::~orientedHLTransform() {
    delete[] m_pCosinus;
    delete[] m_pSinus;
  }

  // returns the name of this type
  const char* orientedHLTransform::getTypeName() const {
    return "orientedHLTransform";
  }

  // copy member
  orientedHLTransform&
  orientedHLTransform::copy(const orientedHLTransform& other) {
    transform::copy(other);

    return (*this);
  }

  // alias for copy member
  orientedHLTransform&
  orientedHLTransform::operator=(const orientedHLTransform& other) {
    return (copy(other));
  }

  // clone member
  functor* orientedHLTransform::clone() const {
    return new orientedHLTransform(*this);
  }

  // return parameters
  const orientedHLTransform::parameters&
  orientedHLTransform::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // set parameters AND create sinus/cosinus tables if necessary
  bool orientedHLTransform::updateParameters() {

    // we need to check the accuracy parameter :   
    const parameters& localPar = getParameters();

    if( localPar.accuracy != 45 && localPar.accuracy != 90 &&
	(localPar.accuracy%180)!=0 ) {
      setStatusString("Invalid accuracy (not 45 or 90)");
      return false;
    }

    if (localPar.accuracy != m_iAccuracy) {
      m_iAccuracy = localPar.accuracy;

      // now create sinus/cosinus integer tables.
      createSinCosTable(m_iAccuracy);
    }
    return true;
  }

  // we need to insert a "very private" class  here, in order to cope with
  // a MS-VC++ bug
  //
  // this is a template class, which does the real job, but even if we would
  // like to have all methods as template methods of the real functor, visual
  // doesn't like it.
  template<class T>
  class houghHelper {
  public:
    houghHelper(const int accuracy,
                const T theBaseValue,
                const orientedHLTransform::parameters& par,
                const int* cosinus,
                const int* sinus)
      : m_pCosinus(cosinus),m_pSinus(sinus),
        baseValue(theBaseValue),
        m_iAccuracy(accuracy),
        params(par) {
    };

    // general method
    bool hough(const matrix<T>& src,
               const channel& angle_src,
               channel32& dest) const;


  protected:
    // Classic: accumulate 1 if src.at(y,x) > baseValue
    bool hough1(const matrix<T>& src,
                const channel& angle_src,
                channel32& dest) const;

    // Gradient: accumulate src.at(y,x) if it is > than baseValue
    bool houghG(const matrix<T>& src,
                const channel& angle_src,
                channel32& dest) const;

    // cast method between given value in src and the value_type of the
    // accumulator
    inline channel32::value_type getAccVal(const T val) const;

    /**
     * Pointer to cosinus integer table. We want to speed
     * the transformation up.
     */
    const int* m_pCosinus;

    /**
     * Pointer to sinus integer table. We want to speed
     * the transformation up.
     */
    const int* m_pSinus;


    /**
     * base value from the parameters as attribute to avoid parameters casting
     * in isEdge specializations
     */
    T baseValue;

    /**
     * accuracy value
     */
    int m_iAccuracy;

    /**
     * all other parameters
     */
    const orientedHLTransform::parameters& params;
  };

  // cast method between given value in src and the value_type of the
  // accumulator
  template<class T>
  inline channel32::value_type houghHelper<T>::getAccVal(const T val) const {
    return static_cast<channel32::value_type>(val);
  }

  template<>
  inline channel32::value_type
  houghHelper<float>::getAccVal(const float val) const {
    return static_cast<channel32::value_type>(val*255);
  }

  template<>
  inline channel32::value_type
  houghHelper<rgbPixel>::getAccVal(const rgbPixel val) const {
    return (static_cast<channel32::value_type>(val.getRed()) +
            static_cast<channel32::value_type>(val.getGreen()) +
            static_cast<channel32::value_type>(val.getBlue()))/3;
  }

  template<class T>
  bool houghHelper<T>::hough(const matrix<T>& src,
                             const channel& angle_src,
                             channel32& dest) const {
    switch (params.accumulationMode) {
      case orientedHLTransform::parameters::Classic:
        return hough1(src,angle_src,dest);
        break;
      case orientedHLTransform::parameters::Gradient:
        return houghG(src,angle_src,dest);
        break;
      default:
        return hough1(src,angle_src,dest);
    }
    return false;
  }

  /**
   * This method implements the hough transform, depending on the
   * image type being analysed.
   *
   * The implementation code must be here due to a MS-VC++ bug, that
   * produces an error C2893 if the code is where it should be.
   */
  template<class T>
  bool houghHelper<T>::houghG(const matrix<T>& src,
                              const channel& angle_src,
                              channel32& dest) const {

    int iY, iX;   // X and Y in source.
    int y,x;      // iY and iX shifted
    int iAngle;   // angle in transformation accumulator
    int iAxisIntercept;  // axis intercept in transformation accu
    int iDistance, iOldY, iCounter; // used for Bresenham's algorithm
    int iMaxAI;    // 1/2 of maximum axis intercept

    // Variables used for orientation
    int iGradientAngle;    // Angle determined by the gradient edge filter
    int aiAngleRange[2][2];
    int iRangeCnt;

    // variables used for speeding up
    int iTmp, iTmp2;

    // Check size.
    const int minx = max(0,params.transformationArea.ul.x);
    const int maxx = min(src.lastColumn(),params.transformationArea.br.x);

    const int miny = max(0,params.transformationArea.ul.y);
    const int maxy = min(src.lastRow(),params.transformationArea.br.y);

    const int iDimX = maxx-minx+1;
    const int iDimY = maxy-miny+1;

    const int range = params.range;

    // maximum radii from the origin to the borders of the region analysed
    const int midX = (iDimX+1)/2;
    const int midY = (iDimY+1)/2;

    // position of the origin
    const int x0 = midX + minx;
    const int y0 = midY + miny;

    // constant required to convert from grad to deg
    static const float gtd = static_cast<float>(180.0/Pi);

    // the transformation accumulator will have the size:
    // [m_iAccuracy,2*iMaxAI]
    iMaxAI = static_cast<int>(sqrt(midX*midX + midY*midY)/10+1)*10;
    dest.resize(2*iMaxAI, m_iAccuracy, 0, false, true);

    // In order to be used correctly with sinus/cosinus values from the table
    iMaxAI *= 8192;

    // The angle range values must be multiplied with this factor
    const float fFactor = m_iAccuracy / 180.0f;
    channel32::value_type val;
    iOldY = 0;

    // scan whole source
    for( iY=miny; iY<=maxy; iY++ ) {
      // with a relatively high probability, there will be points at
      // the most lines, so we can compute the shifted y just once per line.
      y = iY-y0;

      for( iX=minx; iX<=maxx; iX++ ) {
        // Only consider points having a value that is
        // greater than the base value (usually white
        // lines on black background)
        if (src.at(iY,iX) > baseValue) {
          val = getAccVal(src.at(iY,iX));
          // compute the shifted x just when necessary:
          x = iX-x0;

          // Radians -> Degrees
          iGradientAngle = static_cast<int>(angle_src.at(iY,iX) * gtd);

          // We have lines. We only need values between 0 and 180
          if(iGradientAngle<0) {
            iGradientAngle += 180;
          } else if (iGradientAngle >= 180) {
            iGradientAngle -= 180;
          }

          // Determine the angle range in which the sinusoid will be drawn.
          if( iGradientAngle < range ) {
            aiAngleRange[0][0] = 0;
            aiAngleRange[1][0] = static_cast<int>((180 - range +
                                                   iGradientAngle)*fFactor);
            aiAngleRange[1][1] = static_cast<int>(180*fFactor);
          }
          else {
            aiAngleRange[0][0] = static_cast<int>((iGradientAngle-
                                                   range)*fFactor);
            aiAngleRange[1][0] = 0;
            aiAngleRange[1][1] = 0;
          }

          if( iGradientAngle > 180-range ) {
            aiAngleRange[0][1] = static_cast<int>(180*fFactor);
            aiAngleRange[1][1] = static_cast<int>((iGradientAngle-
                                                   (180-range))*fFactor);
          }
          else {
            aiAngleRange[0][1] = static_cast<int>((iGradientAngle+
                                                   range)*fFactor);
          }

          iRangeCnt = 0;

          while( iRangeCnt<2 && aiAngleRange[iRangeCnt][1] != 0 ) {

            iAngle=aiAngleRange[iRangeCnt][0];
            iAxisIntercept = (x*m_pCosinus[iAngle] +
                              y*m_pSinus[iAngle] + iMaxAI) / 8192;
            iOldY = iAxisIntercept;

            for( iAngle++;
                 iAngle<aiAngleRange[iRangeCnt][1];
                 iAngle++) {

              // determine axis intercept for given angle iAngle using
              // the fast sinus/cosinus integer tables. Add iMaxAI in
              // order to have only positive values (thus integer
              // rounding will be correct).
              iAxisIntercept = (x*m_pCosinus[iAngle] +
                                y*m_pSinus[iAngle] + iMaxAI) / 8192;

              // Now accumulate point in the transformation
              // accumulator using a fast form of Bresenham's line
              // algorithm.  See also:
              // http://graphics.cs.ucdavis.edu/GraphicsNotes/
              // Bresenhams-Algorithm/Bresenhams-Algorithm.html
              iDistance=iAxisIntercept-iOldY;
              iTmp2=iAngle-1;
              if (iDistance<=1 && iDistance>=-1) {
                dest.at(iOldY,iTmp2)+=val;
              }
              else {
                if (iDistance>0) {
                  iTmp=iOldY+iDistance;

                  for (iCounter=iOldY; iCounter<iTmp; iCounter++) {
                    dest.at(iCounter,iTmp2)+=val;
                  }
                }
                else {
                  iTmp=iAxisIntercept+1-iDistance;

                  for (iCounter=iAxisIntercept+1;
                       iCounter<iTmp;
                       iCounter++) {
                    dest.at(iCounter,iTmp2)+=val;
                  }
                }
              }

              iOldY = iAxisIntercept;
            }

            // the last column must not remain empty!
            if (iAngle >= m_iAccuracy) {
              iAxisIntercept = (-x*m_pCosinus[0]
                                -y*m_pSinus[0] + iMaxAI) / 8192;
            } else {
              iAxisIntercept = (x*m_pCosinus[iAngle] +
                                y*m_pSinus[iAngle] + iMaxAI) / 8192;
            }

            iDistance=iAxisIntercept-iOldY;
            iTmp2=iAngle-1;

            if (iDistance<=1 && iDistance>=-1) {
              dest.at(iOldY,iTmp2)+=val;
            }
            else {
              if (iDistance>0) {
                iTmp=iOldY+iDistance;

                for (iCounter=iOldY; iCounter<iTmp; iCounter++) {
                  dest.at(iCounter,iTmp2)+=val;
                }
              }
              else {
                iTmp=iAxisIntercept+1-iDistance;

                for (iCounter=iAxisIntercept+1;
                     iCounter<iTmp;
                     iCounter++) {
                  dest.at(iCounter,iTmp2)+=val;
                }
              }
            }
            // end of last column count

            iRangeCnt++;
          }
        }
      }
    }

    return true;
  };

  /**
   * This method implements the hough transform, depending on the
   * image type being analysed.
   *
   * The implementation code must be here due to a MS-VC++ bug, that
   * produces an error C2893 if the code is where it should be.
   */
  template<class T>
  bool houghHelper<T>::hough1(const matrix<T>& src,
                              const channel& angle_src,
                              channel32& dest) const {

    int iY, iX;   // X and Y in source.
    int y,x;      // iY and iX shifted
    int iAngle;   // angle in transformation accumulator
    int iAxisIntercept;  // axis intercept in transformation accu
    int iDistance, iOldY, iCounter; // used for Bresenham's algorithm
    int iMaxAI;    // 1/2 of maximum axis intercept

    // Variables used for orientation
    int iGradientAngle;    // Angle determined by the gradient edge filter
    int aiAngleRange[2][2];
    int iRangeCnt;

    // variables used for speeding up
    int iTmp, iTmp2;

    // Check size.
    const int minx = max(0,params.transformationArea.ul.x);
    const int maxx = min(src.lastColumn(),params.transformationArea.br.x);

    const int miny = max(0,params.transformationArea.ul.y);
    const int maxy = min(src.lastRow(),params.transformationArea.br.y);

    const int iDimX = maxx-minx+1;
    const int iDimY = maxy-miny+1;

    const int range = params.range;

    // maximum radii from the origin to the borders of the region analysed
    const int midX = (iDimX+1)/2;
    const int midY = (iDimY+1)/2;

    // position of the origin
    const int x0 = midX + minx;
    const int y0 = midY + miny;

    // constant required to convert from grad to deg
    const float gtd = static_cast<float>(180.0/Pi);

    // the transformation accumulator will have the size:
    // [m_iAccuracy,2*iMaxAI]
    iMaxAI = static_cast<int>(sqrt(midX*midX + midY*midY)/10+1)*10;
    dest.resize(2*iMaxAI, m_iAccuracy, 0, false, true);

    // In order to be used correctly with sinus/cosinus values from the table
    iMaxAI *= 8192;

    // The angle range values must be multiplied with this factor
    const float fFactor = m_iAccuracy / 180.0f;
    iOldY = 0;

    // scan whole source
    for( iY=miny; iY<=maxy; iY++ ) {
      // with a relatively high probability, there will be points at
      // the most lines, so we can compute the shifted y just once per line.
      y = iY-y0;

      for( iX=minx; iX<=maxx; iX++ ) {
        // Only consider points having a value that is
        // greater than the base value (usually white
        // lines on black background)
        if (src.at(iY,iX) > baseValue) {
          // compute the shifted x just when necessary:
          x = iX-x0;

          // Radians -> Degrees
          iGradientAngle = static_cast<int>(angle_src.at(iY,iX) * gtd);

          // We have lines. We only need values between 0 and 180
          if(iGradientAngle<0) {
            iGradientAngle += 180;
          } else if (iGradientAngle>=180) {
            iGradientAngle -= 180;
          }

          // Determine the angle range in which the sinusoid will be drawn.
          if( iGradientAngle < range ) {
            aiAngleRange[0][0] = 0;
            aiAngleRange[1][0] = static_cast<int>((180 - range +
                                                   iGradientAngle)*fFactor);
            aiAngleRange[1][1] = static_cast<int>(180*fFactor);
          }
          else {
            aiAngleRange[0][0] = static_cast<int>((iGradientAngle-
                                                   range)*fFactor);
            aiAngleRange[1][0] = 0;
            aiAngleRange[1][1] = 0;
          }

          if( iGradientAngle > 180-range ) {
            aiAngleRange[0][1] = static_cast<int>(180*fFactor);
            aiAngleRange[1][1] = static_cast<int>((iGradientAngle-
                                                   (180-range))*fFactor);
          }
          else {
            aiAngleRange[0][1] = static_cast<int>((iGradientAngle+
                                                   range)*fFactor);
          }

          iRangeCnt = 0;

          while( iRangeCnt<2 && aiAngleRange[iRangeCnt][1] != 0 ) {

            iAngle=aiAngleRange[iRangeCnt][0];
            iAxisIntercept = (x*m_pCosinus[iAngle] +
                              y*m_pSinus[iAngle] + iMaxAI) / 8192;
            iOldY = iAxisIntercept;

            for( iAngle++;
                 iAngle<aiAngleRange[iRangeCnt][1];
                 iAngle++) {

              // determine axis intercept for given angle iAngle using
              // the fast sinus/cosinus integer tables. Add iMaxAI in
              // order to have only positive values (thus integer
              // rounding will be correct).
              iAxisIntercept = (x*m_pCosinus[iAngle] +
                                y*m_pSinus[iAngle] + iMaxAI) / 8192;

              // Now accumulate point in the transformation
              // accumulator using a fast form of Bresenham's line
              // algorithm.  See also:
              // http://graphics.cs.ucdavis.edu/GraphicsNotes/
              // Bresenhams-Algorithm/Bresenhams-Algorithm.html
              iDistance=iAxisIntercept-iOldY;
              iTmp2=iAngle-1;

              if (iDistance<=1 && iDistance>=-1) {
                dest.at(iOldY,iTmp2)++;
              }
              else {
                if (iDistance>0) {
                  iTmp=iOldY+iDistance;

                  for (iCounter=iOldY; iCounter<iTmp; iCounter++) {
                    dest.at(iCounter,iTmp2)++;
                  }
                }
                else {
                  iTmp=iAxisIntercept+1-iDistance;

                  for (iCounter=iAxisIntercept+1;
                       iCounter<iTmp;
                       iCounter++) {
                    dest.at(iCounter,iTmp2)++;
                  }
                }
              }

              iOldY = iAxisIntercept;
            }

            // the last column must not remain empty!
            if (iAngle >= m_iAccuracy) {
              iAxisIntercept = (-x*m_pCosinus[0]
                                -y*m_pSinus[0] + iMaxAI) / 8192;
            } else {
              iAxisIntercept = (x*m_pCosinus[iAngle] +
                                y*m_pSinus[iAngle] + iMaxAI) / 8192;
            }

            iDistance=iAxisIntercept-iOldY;
            iTmp2=iAngle-1;

            if (iDistance<=1 && iDistance>=-1) {
              dest.at(iOldY,iTmp2)++;
            }
            else {
              if (iDistance>0) {
                iTmp=iOldY+iDistance;

                for (iCounter=iOldY; iCounter<iTmp; iCounter++) {
                  dest.at(iCounter,iTmp2)++;
                }
              }
              else {
                iTmp=iAxisIntercept+1-iDistance;

                for (iCounter=iAxisIntercept+1;
                     iCounter<iTmp;
                     iCounter++) {
                  dest.at(iCounter,iTmp2)++;
                }
              }
            }
            // end of last column count

            iRangeCnt++;
          }
        }
      }
    }

    return true;
  };

  // On copy apply for type channel8!
  bool orientedHLTransform::apply(const channel& src,
                                  const channel& angle_src,
                                  channel32& dest) const {

    const parameters& par = getParameters();
    houghHelper<float> helper(m_iAccuracy,
                              float(par.baseValue)/255.0f,
                              par,
                              m_pCosinus,m_pSinus);
    return helper.hough(src,angle_src,dest);
  };

  // On copy apply for type channel8!
  bool orientedHLTransform::apply(const channel8& src,
                                  const channel& angle_src,
                                  channel32& dest) const {

    const parameters& par = getParameters();
    houghHelper<ubyte> helper(m_iAccuracy,
                              static_cast<ubyte>(par.baseValue),
                              par,
                              m_pCosinus,m_pSinus);
    return helper.hough(src,angle_src,dest);
  };

  // On copy apply for type image!
  bool orientedHLTransform::apply(const image& src, const channel& angle_src,
                                  channel32& dest) const {

    const parameters& par = getParameters();
    const ubyte gray=static_cast<ubyte>(par.baseValue);
    houghHelper<rgbPixel> helper(m_iAccuracy,
                                 rgbPixel(gray,gray,gray),
                                 par,
                                 m_pCosinus,m_pSinus);
    return helper.hough(src,angle_src,dest);
  };

  // On copy apply for type channel32!
  bool orientedHLTransform::apply(const channel32& src,
                                  const channel& angle_src,
                                  channel32& dest) const {

    const parameters& par = getParameters();
    houghHelper<channel32::value_type>
      helper(m_iAccuracy,
             static_cast<channel32::value_type>(par.baseValue),
             par,
             m_pCosinus,
             m_pSinus);
    return helper.hough(src,angle_src,dest);
  };

  // short cut for accuracy (new sinus/cosinus tables may be created)
  int orientedHLTransform::getAccuracy() const {
    return m_iAccuracy;
  }

  bool orientedHLTransform::setAccuracy(const int iAccuracy) {

    parameters tmp(getParameters());
    tmp.accuracy = iAccuracy;
    return setParameters(tmp);
  }

  // Create sinus/cosinus integer table to speed things up.
  void orientedHLTransform::createSinCosTable(const int iSize) {

    delete[] m_pCosinus;
    delete[] m_pSinus;

    m_pCosinus = new int[iSize];
    m_pSinus   = new int[iSize];

    // create integer tables with values between (-8192) and 8192.
    // The +1.0 is used to round correctly (we need positive values).
    double sina, cosa;
    for(int i=0; i<iSize; i++) {
      sincos(i*Pi/iSize, sina, cosa);
      m_pCosinus[i] = static_cast<int>((cosa+1.0)*8192 + 0.5) - 8192;
      m_pSinus[i]   = static_cast<int>((sina+1.0)*8192 + 0.5) - 8192;
    }
  }

  /**
   * This method is used to get the list of points in the parameter
   * space, sorted by their hough transformation value, with the
   * maxima at the beginning.
   *
   * The given threshold value determines if a pixel in the transformation
   * should be considered or not.
   *
   */
  bool
  orientedHLTransform::getHoughSpacePoints(const channel32& hough,
                                           vector<tpoint<float> >& pos,
                                           const float stdDevFactor,
                                           const float hystheresis) const {

    pointList stack;
    float maxPts,accMax,acc2Max;
    int y,x,i,j;
    float v;

    channel houghf,premax;
    houghf.castFrom(hough);

    // maximum filter
    maximumFilter<float> mxf;
    maximumFilter<float>::parameters mxfPar;
    mxfPar.kernelSize = rectangle(-1,-1,1,1);
    mxfPar.boundaryType = lti::Mirror;
    mxf.setParameters(mxfPar);

    mxf.apply(houghf,premax);

    maxPts=accMax=acc2Max=0.0f;
    for (y=0;y<premax.rows();++y) {
      const vector<float>& hv = houghf.getRow(y);
      const vector<float>& pm = premax.getRow(y);
      for (x=0;x<premax.columns();++x) {
        v = hv.at(x);
        if ((v>0) && (pm.at(x) <= v)) {
          maxPts++;
          accMax  += v;
          acc2Max += v*v;
          stack.push_back(point(x,y));
        }
      }
    }

    // now, erase all points under the average + 1x std. deviation
    const float avrg = accMax/maxPts;
    const float stdd = sqrt((acc2Max/maxPts) - (avrg*avrg));
    const float threshold = max(avrg + stdDevFactor*stdd,0.0f);

    pointList::iterator it;
    it = stack.begin();
    while (it != stack.end()) {
      if (houghf.at(*it) > threshold) {
        ++it;
      } else {
        it = stack.erase(it);
        maxPts--;
      }
    };

    // the rest of points need to be sorted before we use a gradient
    // climb algorithm.
    vector<point> pts;
    ivector vals;

    pts.resize(static_cast<int>(maxPts),point(),false,false);
    vals.resize(static_cast<int>(maxPts),int(),false,false);

    it = stack.begin();
    for (it=stack.begin(),i=0;it!=stack.end();++it,++i) {
      pts.at(i)=*it;
      vals.at(i)=static_cast<int>(houghf.at(*it));
    }

    // sort the vectors using sort2 functor (Descending order -> true)
    sort2< int , point > sorter(true);
    sorter.apply(vals,pts);

    // now, do region growing from the smallest maxima to the biggest ones
    float actualMax,actualThresh;
    static const int deltax[] = {1,1,0,-1,-1,-1,0,1};
    static const int deltay[] = {0,-1,-1,-1,0,1,1,1};
    point p,q,actualMaxPos;
    pointList localMax;

    i=pts.lastIdx();
    stack.clear();
    while (i>=0) {

      actualMaxPos = p = pts.at(i);
      actualMax = houghf.at(p);

      if (actualMax > 0) {
        actualThresh = actualMax*hystheresis;
        stack.push_back(p);

        while (!stack.empty()) {
          // get next point to be evaluated
          p = stack.back();
          stack.pop_back();
          v = houghf.at(p);
          houghf.at(p) = -1; // flag: already evaluated

          if (v > actualMax) {
            actualMax = v;
            actualMaxPos = p;
          }

          for (j=0;j<8;j++) {
            q.x = p.x+deltax[j];
            q.y = p.y+deltay[j];

            if (q.x < 0) {
              q.x += houghf.columns();
              q.y = houghf.rows() - q.y;
            } else if (q.x >= houghf.columns()) {
              q.x -= houghf.columns();
              q.y = houghf.rows() - q.y;
            }

            if (houghf.at(q) > actualThresh) {
              stack.push_back(q);
            } else {
              houghf.at(q) = -1;
            }
          }
        }

        // the surviving maxima is a local maxima!
        localMax.push_front(actualMaxPos);
      }

      i--;
    }

    pos.resize(localMax.size(),tpoint<float>(),false,false);

    const float angFactor = static_cast<float>(Pi/hough.columns());
    const float r0 = static_cast<float>(round(hough.rows()/2));

    for (i=0,it=localMax.begin();i<pos.size();++i,++it) {
      pos.at(i).x=(*it).y-r0; // radius
      pos.at(i).y=(*it).x*angFactor;
    }

#ifdef _LTI_DEBUG
#if _LTI_DEBUG > 2
    cout << endl;
    cout << localMax.size() << "/" << premax.rows()*premax.columns()
         << " maxima found" << endl;
    cout << avrg << " average value" << endl;
    cout << stdd << " std. deviation" << endl;

    static viewer view1("1");
    static viewer view2("2");
    static viewer view3("3");
    static viewer view4("4");

    view1.show(premax);
    view3.show(houghf);
#endif
#endif
    return true;
  }

  /**
   * Given a radius and an angle, compute the two points at the border of
   * the image
   */
  bool
  orientedHLTransform::getSegmentPoints(const point& imageSize,
                                        const float radius,
                                        const float angle,
                                        point& a,
                                        point& b,
                                        const point& origin) const {

    point c;
    if (origin.x < 0) {
      c.x = imageSize.x/2;
    } else {
      c.x = origin.x;
    }

    if (origin.y < 0) {
      c.y = imageSize.y/2;
    } else {
      c.y = origin.y;
    }

    float stmp, ctmp;
    sincos(angle, stmp, ctmp);
    const float cosa = ctmp;
    const float sina = stmp;

    if ((angle > Pi/4.0) && (angle < (3.0*Pi/4.0))) {

      a.x=0;
      a.y=iround((radius - (cosa*(a.x-c.x)))/sina) + c.y;

      if (a.y<0) {
        a.y = 0;
        a.x = iround((radius - (sina*(a.y-c.y)))/cosa) + c.x;
      } else if (a.y>=imageSize.y) {
        a.y = imageSize.y-1;
        a.x = iround((radius - (sina*(a.y-c.y)))/cosa) + c.x;
      }

      b.x=imageSize.x-1;
      b.y=iround((radius - (cosa*(b.x-c.x)))/sina) + c.y;

      if (b.y<0) {
        b.y = 0;
        b.x = iround((radius - (sina*(b.y-c.y)))/cosa) + c.x;
      } else if (b.y>=imageSize.y) {
        b.y = imageSize.y-1;
        b.x = iround((radius - (sina*(b.y-c.y)))/cosa) + c.x;
      }

    } else {

      a.y = 0;
      a.x = iround((radius - (sina*(a.y-c.y)))/cosa) + c.x;

      if (a.x<0) {
        a.x = 0;
        a.y = iround((radius - (cosa*(a.x-c.x)))/sina) + c.y;
      } else if (a.x>=imageSize.x) {
        a.x = imageSize.x-1;
        a.y = iround((radius - (cosa*(a.x-c.x)))/sina) + c.y;
      }

      b.y = imageSize.y-1;
      b.x = iround((radius - (sina*(b.y-c.y)))/cosa) + c.x;

      if (b.x<0) {
        b.x = 0;
        b.y = iround((radius - (cosa*(b.x-c.x)))/sina) + c.y;
      } else if (b.x>=imageSize.x) {
        b.x = imageSize.x-1;
        b.y = iround((radius - (cosa*(b.x-c.x)))/sina) + c.y;
      }
    }

    return true;
  }

}
