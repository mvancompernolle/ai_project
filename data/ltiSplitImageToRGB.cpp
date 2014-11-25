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
 * file .......: ltiSplitImageToRGB.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiSplitImageToRGB.cpp,v 1.4 2006/02/08 11:54:05 ltilib Exp $
 */

#include "ltiSplitImageToRGB.h"
#include "ltiConstants.h"

namespace lti {

  // ========= splitImageToRGB ===============


  // split image into RGB channels
  // returns the name of this type
  const char* splitImageToRGB::getTypeName() const {
    return "splitImageToRGB";
  }

  // returns a pointer to a clone of the functor.
  functor* splitImageToRGB::clone() const {
    return (new splitImageToRGB(*this));
  }

  bool splitImageToRGB::apply(const image& img,
                              channel& c1,
                              channel& c2,
                              channel& c3) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    channel::iterator rit,git,bit;

    for (p.y=0,rit=c1.begin(),git=c2.begin(),bit=c3.begin();
         p.y<img.rows();
         p.y++) {
      const vector<rgbPixel>& theRow = img.getRow(p.y);
      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;rit++,git++,bit++,imgIt++) {
        const rgbPixel& pix = (*imgIt);

        *rit = float(pix.getRed())/255.0f;
        *git = float(pix.getGreen())/255.0f;
        *bit = float(pix.getBlue())/255.0f;
      }
    }

    return true;
  }

  bool splitImageToRGB::apply(const image& img,
                              channel& c1,
                              channel& c2,
                              channel& c3,
                              channel& c4) const {
    point p;

    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);
    c4.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++) {
      for (p.x=0;p.x<img.columns();p.x++) {
        const rgbPixel& pix = img.at(p);
        c1.at(p) = float(pix.getRed())/255.0f;
        c2.at(p) = float(pix.getGreen())/255.0f;
        c3.at(p) = float(pix.getBlue())/255.0f;
        c4.at(p) = float(pix.getDummy())/255.0f;
      }
    }

    return true;
  }

  bool splitImageToRGB::apply(const image& img,
                              channel8& c1,
                              channel8& c2,
                              channel8& c3) const {
    point p;
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        const rgbPixel& pix = img.at(p);

        c1.at(p) = pix.getRed();
        c2.at(p) = pix.getGreen();
        c3.at(p) = pix.getBlue();
      }
    return true;
  }

  bool splitImageToRGB::apply(const image& img,
                              channel8& c1,
                              channel8& c2,
                              channel8& c3,
                              channel8& c4) const {
    point p;

    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);
    c4.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++) {
      for (p.x=0;p.x<img.columns();p.x++) {
        const rgbPixel& pix = img.at(p);
        c1.at(p) = pix.getRed();
        c2.at(p) = pix.getGreen();
        c3.at(p) = pix.getBlue();
        c4.at(p) = pix.getDummy();
      }
    }

    return true;
  }

  bool splitImageToRGB::apply(const rgbPixel& pixel,
                              float& c1,
                              float& c2,
                              float& c3) const {

    c1=float(pixel.getRed())/255.0f;
    c2=float(pixel.getGreen())/255.0f;
    c3=float(pixel.getBlue())/255.0f;
    return true;
  }

  bool splitImageToRGB::apply(const rgbPixel& pixel,
                              ubyte& c1,
                              ubyte& c2,
                              ubyte& c3) const {

    c1=pixel.getRed();
    c2=pixel.getGreen();
    c3=pixel.getBlue();
    return true;
  }

  bool splitImageToRGB::getRed(const image& img,
                               channel& red) const {
    point p;
    red.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        red.at(p)=float(img.at(p).getRed())/255.0f;
      }
    return true;
  }

  bool splitImageToRGB::getRed(const image& img,
                               channel8& red) const {
    point p;
    red.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        red.at(p)=img.at(p).getRed();
      }
    return true;
  }

  bool splitImageToRGB::getGreen(const image& img,
                                 channel& green) const {
    point p;
    green.resize(img.rows(),img.columns(),0,false,false);
    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        green.at(p)=float(img.at(p).getGreen())/255.0f;
      }
    return true;
  }

  bool splitImageToRGB::getGreen(const image& img,
                                 channel8& green) const {
    point p;
    green.resize(img.rows(),img.columns(),0,false,false);
    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        green.at(p)=img.at(p).getGreen();
      }
    return true;
  }

  bool splitImageToRGB::getBlue(const image& img,
                                channel& blue) const {
    point p;
    blue.resize(img.rows(),img.columns(),0,false,false);
    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        blue.at(p)=float(img.at(p).getBlue())/255.0f;
      }
    return true;
  }

  bool splitImageToRGB::getBlue(const image& img,
                                channel8& blue) const {
    point p;
    blue.resize(img.rows(),img.columns(),0,false,false);
    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        blue.at(p)=img.at(p).getBlue();
      }

    return true;
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToRGB::getFirst(const image& img, channel& c1) const {
    return getRed(img,c1);
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToRGB::getFirst(const image& img, channel8& c1) const {
    return getRed(img,c1);
  }

  /*
   * Returns the second of the three channels into which the image is split.
   */
  bool splitImageToRGB::getSecond(const image& img, channel& c2) const {
    return getGreen(img,c2);
  }

  /*
   * Returns the second of the three channels into which the image is split.
   */
  bool splitImageToRGB::getSecond(const image& img, channel8& c2) const {
    return getGreen(img,c2);
  }

  /*
   * Returns the third of the three channels into which the image is split.
   */
  bool splitImageToRGB::getThird(const image& img, channel& c2) const {
    return getBlue(img,c2);
  }

  /*
   * Returns the third of the three channels into which the image is split.
   */
  bool splitImageToRGB::getThird(const image& img, channel8& c2) const {
    return getBlue(img,c2);
  }


} // end of namespace lti
