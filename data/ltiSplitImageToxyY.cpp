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
 * file .......: ltiSplitImageToxyY.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiSplitImageToxyY.cpp,v 1.4 2006/02/08 11:55:31 ltilib Exp $
 */

#include "ltiSplitImageToxyY.h"
#include "ltiConstants.h"

namespace lti {

  // ========= splitImageToxyY ===============

  // split image to xyY channels
  // returns the name of this type
  const char* splitImageToxyY::getTypename() const {
    return "splitImageToxyY";
  }

  // returns a pointer to a clone of the functor
  functor* splitImageToxyY::clone() const {
    return (new splitImageToxyY(*this));
  }

  // split image into float channels
  bool splitImageToxyY::apply(const image& img,
                              channel& c1,
                              channel& c2,
                              channel& c3) const {
    point p;             // coordinates
    rgbPixel pix;          // single Pixel Element in RGB-values...
    float Y;               // channels
    float X, XYZ;          // help variables

    // make the channels size of source image...
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        // take pixel at position p
        pix = img.at(p);

  // see Gonzales & Woods for explanation of magic numbers
        X   = (((float)(pix.getRed())) *0.412453f +
               ((float)(pix.getGreen())) *0.357580f +
               ((float)(pix.getBlue())) *0.180423f)/255.0f;   // x
        Y   = (((float)(pix.getRed())) *0.212671f +
               ((float)(pix.getGreen())) *0.715160f +
               ((float)(pix.getBlue())) *0.072169f)/255.0f;   // y
        XYZ = (((float)(pix.getRed())) *0.644458f +
               ((float)(pix.getGreen())) *1.191933f +
               ((float)(pix.getBlue())) *1.202819f)/255.0f;   // Y

        if (XYZ>0.0f) {
          c1.at(p) = X/XYZ;  // x
          c2.at(p) = Y/XYZ;  // y
        }
        else {
          c1.at(p) = 0;   // x
          c2.at(p) = 0;   // y
        }
        c3.at(p) = Y;     // Y
      } // loop
    return true;
  }
  

  // split image into 8-bit channels
  // N.B.: when casting the transformation result to unsigned shorts
  // (8-bit channel), major rounding errors will occur.
  // As a result, the merging operation might
  // produce negative values or values > 1,  which are truncated subsequently.
  // When accurate X, Y and Z channels are required, prefer float channels!
  bool splitImageToxyY::apply(const image& img,
                              channel8& c1,
                              channel8& c2,
                              channel8& c3) const {
    point p;       // coordinates
    rgbPixel pix;          // single Pixel Element in RGB-values...
    float Y;               // channels
    float X, XYZ;          // help variables

    // make the channels size of source image...
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        // take pixel at position p
        pix = img.at(p);

  // see Gonzales & Woods for explanation of magic numbers
        X   = (((float)(pix.getRed())) *0.412453f +
               ((float)(pix.getGreen())) *0.357580f +
               ((float)(pix.getBlue())) *0.180423f)/255.0f;   // x
        Y   = (((float)(pix.getRed())) *0.212671f +
               ((float)(pix.getGreen())) *0.715160f +
               ((float)(pix.getBlue())) *0.072169f)/255.0f;   // y
        XYZ = (((float)(pix.getRed())) *0.644458f +
               ((float)(pix.getGreen())) *1.191933f +
               ((float)(pix.getBlue())) *1.202819f)/255.0f;   // Y

        if (XYZ>0.0f) {
          c1.at(p) = (ubyte)(X/XYZ*255.0f);  // x
          c2.at(p) = (ubyte)(Y/XYZ*255.0f);  // y
        }
        else {
          c1.at(p) = 0;   // x
          c2.at(p) = 0;   // y
        }

        c3.at(p) = (ubyte)(Y*255.0f);     // Y
      } // loop
    return true;
  }

  bool splitImageToxyY::apply(const rgbPixel& pixel,
                              float& c1,
                              float& c2,
                              float& c3) const {

    const ubyte& red=pixel.getRed();
    const ubyte& green=pixel.getGreen();
    const ubyte& blue=pixel.getBlue();

    float Y;               // channels
    float X, XYZ;          // help variables

    // see Gonzales & Woods for explanation of magic numbers
    X   = (((float)(red)) *0.412453f +
           ((float)(green)) *0.357580f +
           ((float)(blue)) *0.180423f)/255.0f;   // x
    Y   = (((float)(red)) *0.212671f +
           ((float)(green)) *0.715160f +
           ((float)(blue)) *0.072169f)/255.0f;   // y
    XYZ = (((float)(red)) *0.644458f +
           ((float)(green)) *1.191933f +
           ((float)(blue)) *1.202819f)/255.0f;   // Y

    if (XYZ>0.0f) {
      c1 = X/XYZ;  // x
      c2 = Y/XYZ;  // y
    } else {
      c1 = 0;   // x
      c2 = 0;   // y
    }
    c3 = Y;     // Y
    return true;
  }

  bool splitImageToxyY::apply(const rgbPixel& pixel,
                              ubyte& c1,
                              ubyte& c2,
                              ubyte& c3) const {

    const ubyte& red=pixel.getRed();
    const ubyte& green=pixel.getGreen();
    const ubyte& blue=pixel.getBlue();

    float Y;               // channels
    float X, XYZ;          // help variables

    // see Gonzales & Woods for explanation of magic numbers
    X   = (((float)(red)) *0.412453f +
           ((float)(green)) *0.357580f +
           ((float)(blue)) *0.180423f)/255.0f;   // x
    Y   = (((float)(red)) *0.212671f +
           ((float)(green)) *0.715160f +
           ((float)(blue)) *0.072169f)/255.0f;   // y
    XYZ = (((float)(red)) *0.644458f +
           ((float)(green)) *1.191933f +
           ((float)(blue)) *1.202819f)/255.0f;   // Y

    if (XYZ>0.0f) {
      c1 = (ubyte)(X/XYZ*255.0f);  // x
      c2 = (ubyte)(Y/XYZ*255.0f);  // y
    } else {
      c1 = 0;   // x
      c2 = 0;   // y
    }

    c3 = (ubyte)(Y*255.0f);     // Y
    return true;
  }
} // end of namespace lti
