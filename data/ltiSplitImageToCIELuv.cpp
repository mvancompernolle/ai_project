/*
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006
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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiSplitImageToCIELuv.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiSplitImageToCIELuv.cpp,v 1.6 2006/02/08 11:52:08 ltilib Exp $
 */

#include "ltiSplitImageToCIELuv.h"
#include "ltiConstants.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {

  // ========= splitImageToCIELuv ===============

  // -------------------------------------------------
  // the CIE L* u* v* color space
  // -------------------------------------------------
  // static member initialization
  const float* splitImageToCIELuv::lut = 0;
  const int splitImageToCIELuv::lutSize = 4096;

  // default constructor
  splitImageToCIELuv::splitImageToCIELuv() {
    if (isNull(lut)) {
      _lti_debug("building LUT, size is " << lutSize << "\n");

      // the singleton approach: just one static member, this way the
      // compiler will take care of deleting the static object at the end
      // of the application:
      static float lutData[lutSize];     
      static const float cb = 1.0f/3.0f;
      static const float t=static_cast<float>(lutSize-1);
      float y;

      _lti_debug("t is " << t << "\n");
      _lti_debug("cb is " << cb << "\n");
      for (int i=0; i<lutSize; ++i) {
        y = static_cast<float>(i)/t;
        if (y<=0.008856f) {
          lutData[i] = 903.3f*y;
        } else {
          lutData[i] = 116.0f*pow(y,cb)-16.0f;
        }
      }

      lut = lutData; // provide access from the whole class to the static
                     // member.
    }
  }

  // split image to OCP channels (opponent colour system after Pomierskki)
  // returns the name of this type
  const char* splitImageToCIELuv::getTypename() const {
    return "splitImageToCIELuv";
  }

  // returns a pointer to a clone of the functor
  functor* splitImageToCIELuv::clone() const {
    return (new splitImageToCIELuv(*this));
  }

  // split image into float channels
  bool splitImageToCIELuv::apply(const image& img,
                                 channel& c1,
                                 channel& c2,
                                 channel& c3) const {

    point p;           // coordinates
    float X,Y,Z;
    float up,vp,L,t;
    static const float un = 0.2009f;
    static const float vn = 0.4610f;
    static const int lutLastIndex = lutSize - 1;

    // make the channels size of source image...
    c1.resize(img.size(),0,false,false);
    c2.resize(img.size(),0,false,false);
    c3.resize(img.size(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++) {
      for (p.x=0;p.x<img.columns();p.x++) {

        // take pixel at position p
        const rgbPixel& pix = img.at(p); // single Pixel (RGB-values)

        // see Gonzales & Woods for explanation of magic numbers
        X = (((float)(pix.getRed()))   * 0.412453f +
             ((float)(pix.getGreen())) * 0.357580f +
             ((float)(pix.getBlue()))  * 0.180423f )/255.0f;   // X
        Y = (((float)(pix.getRed()))   * 0.212671f +
             ((float)(pix.getGreen())) * 0.715160f +
             ((float)(pix.getBlue()))  * 0.072169f )/255.0f;   // Y
        Z = (((float)(pix.getRed()))   * 0.019334f +
             ((float)(pix.getGreen())) * 0.119193f +
             ((float)(pix.getBlue()))  * 0.950227f )/255.0f;   // Z

        // Y has now values between 0 and 1.
        // Luminance between 0 and 100
        L  = lut[static_cast<int>(0.5f+Y*lutLastIndex)];

        // normalized chrominance values
        t = (X+15*Y+3*Z);
        if (t == 0) {
          up = 0;
          vp = 0;
        } else {
          up = 4*X/t;
          vp = 9*Y/t;
        }

        // normalize to be from 0 to 1
        c1.at(p) = L/100.0f;
        t = 0.13f*L;
        c2.at(p) = t*(up-un);
        c3.at(p) = t*(vp-vn);
      } // loop
    }
    return true;
  }

  // split Image to 8-bit channels
  // channels RG and BY arwe shifted up to obtain positive values !
  // for more accuracy use float channels
  bool splitImageToCIELuv::apply(const image& img,
                                 channel8& c1,
                                 channel8& c2,
                                 channel8& c3) const {

    point p;           // coordinates
    float X,Y,Z;
    float up,vp,L,t;
    static const float un = 0.2009f;
    static const float vn = 0.4610f;
    static const int lutLastIndex = lutSize - 1;

    // make the channels size of source image...
    c1.resize(img.size(),0,false,false);
    c2.resize(img.size(),0,false,false);
    c3.resize(img.size(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++) {
      for (p.x=0;p.x<img.columns();p.x++) {

        // take pixel at position p
        const rgbPixel& pix = img.at(p); // single Pixel (RGB-values)

        // see Gonzales & Woods for explanation of magic numbers
        X = (((float)(pix.getRed()))   * 0.412453f +
             ((float)(pix.getGreen())) * 0.357580f +
             ((float)(pix.getBlue()))  * 0.180423f)/255.0f;  // X
        Y = (((float)(pix.getRed())) * 0.212671f +
             ((float)(pix.getGreen())) * 0.715160f +
             ((float)(pix.getBlue())) * 0.072169f)/255.0f;   // Y
        Z = (((float)(pix.getRed())) * 0.019334f +
             ((float)(pix.getGreen())) * 0.119193f +
             ((float)(pix.getBlue())) * 0.950227f)/255.0f;   // Z

        //
        L  = lut[static_cast<int>(0.5f+Y*lutLastIndex)];
        t = (X+15*Y+3*Z);
        if (t == 0) {
          up = 0;
          vp = 0;
        } else {
          up = 4*X/t;
          vp = 9*Y/t;
        }


        c1.at(p) = static_cast<ubyte>(L*2.55f);
        t = (13.0f*2.55f)*L; 
        c2.at(p) = static_cast<ubyte>(t*(up-un));
        c3.at(p) = static_cast<ubyte>(t*(vp-vn));
      } // loop
    }
    return true;
  }

  bool splitImageToCIELuv::apply(const rgbPixel& pixel,
                                 float& c1,
                                 float& c2,
                                 float& c3) const {

    const ubyte& red=pixel.getRed();
    const ubyte& green=pixel.getGreen();
    const ubyte& blue=pixel.getBlue();

    float X,Y,Z;
    float up,vp,L,t;
    static const float un = 0.2009f;
    static const float vn = 0.4610f;
    static const int lutLastIndex = lutSize - 1;


    // see Gonzales & Woods for explanation of magic numbers
    X = (((float)(red))   * 0.412453f +
         ((float)(green)) * 0.357580f +
         ((float)(blue))  * 0.180423f )/255.0f;   // X
    Y = (((float)(red))   * 0.212671f +
         ((float)(green)) * 0.715160f +
         ((float)(blue))  * 0.072169f )/255.0f;   // Y
    Z = (((float)(red))   * 0.019334f +
         ((float)(green)) * 0.119193f +
         ((float)(blue))  * 0.950227f )/255.0f;   // Z


    //
    L  = lut[static_cast<int>(0.5f+Y*lutLastIndex)];
    t = (X+15*Y+3*Z);
    if (t == 0) {
      up = 0;
      vp = 0;
    } else {
      up = 4*X/t;
      vp = 9*Y/t;
    }

    c1 = L/100.0f;
    t = 0.13f*L;
    c2 = t*(up-un);
    c3 = t*(vp-vn);

    return true;
  }

  bool splitImageToCIELuv::apply(const rgbPixel& pixel,
                                 ubyte& c1,
                                 ubyte& c2,
                                 ubyte& c3) const {

    const ubyte& red=pixel.getRed();
    const ubyte& green=pixel.getGreen();
    const ubyte& blue=pixel.getBlue();

    float X,Y,Z;
    float up,vp,L,t;
    static const float un = 0.2009f;
    static const float vn = 0.4610f;
    static const float a = 116.0f/255.0f;
    static const float b = 16.0f/255.0f;
    static const int k = 255*13;
    static const int lutLastIndex = lutSize - 1;

    // see Gonzales & Woods for explanation of magic numbers
    X = (((float)(red))   * 0.412453f +
         ((float)(green)) * 0.357580f +
         ((float)(blue))  * 0.180423f)/255.0f;  // X
    Y = (((float)(red)) * 0.212671f +
         ((float)(green)) * 0.715160f +
         ((float)(blue)) * 0.072169f)/255.0f;   // Y
    Z = (((float)(red)) * 0.019334f +
         ((float)(green)) * 0.119193f +
         ((float)(blue)) * 0.950227f)/255.0f;   // Z

    //
    L  = a*lut[static_cast<int>(0.5f+Y*lutLastIndex)]-b;
    t = (X+15*Y+3*Z);
    up = 4*X/t;
    vp = 9*Y/t;
    t = k*L;

    c1 = static_cast<ubyte>(L*255);
    c2 = static_cast<ubyte>(t*(up-un));
    c3 = static_cast<ubyte>(t*(vp-vn));
    return true;
  }

} // end of namespace lti
