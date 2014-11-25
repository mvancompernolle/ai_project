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
 * file .......: ltiSplitImageToYIQ.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiSplitImageToYIQ.cpp,v 1.4 2006/02/08 11:54:40 ltilib Exp $
 */

#include "ltiSplitImageToYIQ.h"
#include "ltiConstants.h"

namespace lti {

  // ========= splitImageToYIQ ===============

  // split image to YIQ channels
  // returns the name of this type
  const char* splitImageToYIQ::getTypename() const {
    return "splitImageToYIQ";
  }

  // returns a pointer to a clone of the functor
  functor* splitImageToYIQ::clone() const {
    return (new splitImageToYIQ(*this));
  }

  // split image into float channels
  bool splitImageToYIQ::apply(const image& img,
                              channel& c1,
                              channel& c2,
                              channel& c3) const {

    point p;      // coordinates
    rgbPixel pix;   // single Pixel Element in RGB-values...

    // make the channels size of source image...
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        // take pixel at position p
        pix = img.at(p);

  // see Gonzales & Woods for explanation of magic numbers
  // channels I (inphase) and Q (quadrature) can be negative!
  // The Y, I and Q channel are here  - unlike to the other colour space
  // transforms - NOT normalized in order not to loose significant
  // information.
  // When normalized channels are required, this has to be done by
  // using a separate functor...

        c1.at(p) = ( ((float)(pix.getRed())) *0.299f +
                     ((float)(pix.getGreen())) *0.587f +
                     ((float)(pix.getBlue())) *0.114f)/255.0f;       // Y
        c2.at(p) = ((( ((float)(pix.getRed())) *0.500f -
                       ((float)(pix.getGreen())) *0.231f -
                       ((float)(pix.getBlue())) *0.269f)))/255.0f;   // I
        c3.at(p) = ((( ((float)(pix.getRed())) *0.203f -
                       ((float)(pix.getGreen())) *0.500f +
                       ((float)(pix.getBlue())) *0.297f)))/255.0f;   // Q

        // Y range: 0..1
        // I range: -0.5..0.5
        // Q range: -0.5..0.5

      } // loop
    return true;
  }


  // split image into 8-bit channels
  bool splitImageToYIQ::apply(const image& img,
                              channel8& c1,
                              channel8& c2,
                              channel8& c3) const {
    point p;      // coordinates
    rgbPixel pix;   // single Pixel Element in RGB-values...

    // make the channels size of source image...
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {

        // take pixel at position p
        pix = img.at(p);

        // see Gonzales & Woods for explanation of magic numbers
        // channels I (inphase) and Q (quadrature) can be negative!
        // The Y, I and Q channel are here  - unlike to the other colour space
        // transforms - NOT normalized in order not to loose significant
        // information.
        // When normalized channels are required, this has to be done by
        // using a separate functor...

        c1.at(p) = (ubyte)((((float)(pix.getRed()))*0.299f +
                            ((float)(pix.getGreen()))*0.587f +
                            ((float)(pix.getBlue()))*0.114f));   // Y
        c2.at(p) = (ubyte)((((float)(pix.getRed()))*0.500f -
                            ((float)(pix.getGreen()))*0.231f -
                            ((float)(pix.getBlue()))*0.269f));   // I
        c3.at(p) = (ubyte)((((float)(pix.getRed()))*0.203f -
                            ((float)(pix.getGreen()))*0.500f +
                            ((float)(pix.getBlue()))*0.297f));   // Q

      } // loop
    return true;
  }


  bool splitImageToYIQ::apply(const rgbPixel& pixel,
                              float& Y,
                              float& I,
                              float& Q) const {

    const ubyte& red=pixel.getRed();
    const ubyte& green=pixel.getGreen();
    const ubyte& blue=pixel.getBlue();

    // see Gonzales & Woods for explanation of magic numbers
    // channels I (inphase) and Q (quadrature) can be negative!
    // The Y, I and Q channel are here  - unlike to the other colour space
    // transforms - NOT normalized in order not to loose significant
    // information.
    // When normalized channels are required, this has to be done by
    // using a separate functor...

    Y  = (((float)(red)) *0.299f +
          ((float)(green)) *0.587f +
          ((float)(blue)) *0.114f)/255.0f;     // Y
    I =  ((((float)(red)) *0.500f -
           ((float)(green)) *0.231f -
           ((float)(blue)) *0.269f))/255.0f;   // I
    Q =  ((((float)(red)) *0.203f -
           ((float)(green)) *0.500f +
           ((float)(blue)) *0.297f))/255.0f;   // Q

    // Y range: 0..1
    // I range: -0.5..0.5
    // Q range: -0.5..0.5

    return true;
  }

  bool splitImageToYIQ::apply(const rgbPixel& pixel,
                              ubyte& Y,
                              ubyte& I,
                              ubyte& Q) const {

    const ubyte& red=pixel.getRed();
    const ubyte& green=pixel.getGreen();
    const ubyte& blue=pixel.getBlue();

    // see Gonzales & Woods for explanation of magic numbers
    // channels I (inphase) and Q (quadrature) can be negative!
    // The Y, I and Q channel are here  - unlike to the other colour space
    // transforms - NOT normalized in order not to loose significant
    // information.
    // When normalized channels are required, this has to be done by
    // using a separate functor...

    Y = (ubyte)(((float)(red))*0.299f +
                ((float)(green))*0.587f +
                ((float)(blue))*0.114f);   // Y
    I = (ubyte)(((float)(red))*0.500f -
                ((float)(green))*0.231f -
                ((float)(blue))*0.269f);   // I
    Q = (ubyte)(((float)(red))*0.203f -
                ((float)(green))*0.500f +
                ((float)(blue))*0.297f);   // Q
    return true;
  }

} // end of namespace lti
