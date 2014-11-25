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
 * file .......: ltiSplitImageTorgI.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiSplitImageTorgI.cpp,v 1.4 2006/02/08 11:55:14 ltilib Exp $
 */

#include "ltiSplitImageTorgI.h"
#include "ltiConstants.h"

namespace lti {

  // ========= splitImageTorgI ===============

  // split image into rgI channels
  // returns the name of this type
  const char* splitImageTorgI::getTypeName() const {

    return "splitImageTorgI";
  }

  // returns a pointer to a clone of the functor.
  functor* splitImageTorgI::clone() const {
    return (new splitImageTorgI(*this));
  }

  bool splitImageTorgI::apply(const image& img,
                              channel& c1,
                              channel& c2,
                              channel& c3) const {
    float i;
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++) {
      const vector<rgbPixel>& theRow = img.getRow(p.y);
      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;p.x++,imgIt++) {
        const rgbPixel& pix = *imgIt;

        i = (float(pix.getRed()) + float(pix.getGreen()) +
             float(pix.getBlue()));
        if (i>0) { // to avoid division by 0
          c1.at(p) = float(pix.getRed())/i;
          c2.at(p) = float(pix.getGreen())/i;
          c3.at(p) = i/(765.0f); // 765 = 3*255;
        } else {
          c1.at(p) = 0.0f;
          c2.at(p) = 0.0f;
          c3.at(p) = 0.0f;
        }
      }
    }

    return true;
  }

  bool splitImageTorgI::apply(const image& img,
                              channel8& c1,
                              channel8& c2,
                              channel8& c3) const {
    int i;
    point p;
    rgbPixel pix;
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        pix = img.at(p);

        i = int(pix.getRed()) + int(pix.getGreen()) + int(pix.getBlue());
        if (i!=0) { // to avoid division by 0
          c1.at(p) = (ubyte)(int(pix.getRed())*255/i);
          c2.at(p) = (ubyte)(int(pix.getGreen())*255/i);
          c3.at(p) = (ubyte)(i/3);
        } else {
          c1.at(p) = 0;
          c2.at(p) = 0;
          c3.at(p) = 0;
        }
      }
    return true;
  }

  bool splitImageTorgI::apply(const rgbPixel& pixel,
                              float& r,
                              float& g,
                              float& I) const {

    float sum = (float(pixel.getRed()) + float(pixel.getGreen()) +
                 float(pixel.getBlue()));

    if (sum>0) {
      r=float(pixel.getRed())/sum;
      g=float(pixel.getGreen())/sum;
      I=sum/765.0f;
    } else {
      r=0.0f;
      g=0.0f;
      I=0.0f;
    }
    return true;
  }

  bool splitImageTorgI::apply(const rgbPixel& pixel,
                              ubyte& r,
                              ubyte& g,
                              ubyte& I) const {

    int sum = int(pixel.getRed())+int(pixel.getGreen())+int(pixel.getBlue());

    if (sum!=0) { // to avoid division by 0
      r = (ubyte)(int(pixel.getRed())*255/sum);
      g = (ubyte)(int(pixel.getGreen())*255/sum);
      I = (ubyte)(sum/3);
    } else {
      r=0;
      g=0;
      I=0;
    }
    return true;
  }

  bool splitImageTorgI::getR(const image& img,
                             channel& r) const {
    point p;
    float i;
    rgbPixel pix;
    r.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        pix = img.at(p);
        i = (float(pix.getRed()) + float(pix.getGreen()) +
             float(pix.getBlue()));
        if (i>0) {
          r.at(p) = float(pix.getRed())/i;
        } else {
          r.at(p) = 0.0f;
        }
      }
    return true;
  }

  bool splitImageTorgI::getR(const image& img,
                             channel8& r) const {
    point p;
    int i;
    rgbPixel pix;
    r.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        pix = img.at(p);
        i = int(pix.getRed()) + int(pix.getGreen()) + int(pix.getBlue());
        if (i!=0) {
          r.at(p) = (ubyte)(int(pix.getRed())*255/i);
        } else {
          r.at(p) = 0;
        }
      }
    return true;
  }

  bool splitImageTorgI::getG(const image& img,
                             channel& g) const {
    point p;
    float i;
    rgbPixel pix;

    g.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        pix = img.at(p);
        i = (float(pix.getRed()) + float(pix.getGreen()) +
             float(pix.getBlue()));
        if (i>0) {
          g.at(p) = float(pix.getGreen())/i;
        } else {
          g.at(p) = 0.0f;
        }
      }
    return true;
  }

  bool splitImageTorgI::getG(const image& img,
                             channel8& g) const {
    point p;
    int i;
    rgbPixel pix;

    g.resize(img.rows(),img.columns(),0,false,false);
    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        pix = img.at(p);
        i = int(pix.getRed()) + int(pix.getGreen()) + int(pix.getBlue());
        if (i!=0) {
          g.at(p) = (ubyte)(int(pix.getRed())*255/i);
        } else {
          g.at(p) = 0;
        }
      }
    return true;
  }

  bool splitImageTorgI::getIntensity(const image& img,
                                     channel& intensity) const {
    int y;
    intensity.resize(img.rows(),img.columns(),0,false,false);

    if ((intensity.getMode() == channel::Connected) &&
        (img.getMode() == image::Connected)) {
      image::const_iterator iit,eit;
      channel::iterator it;

      for (iit=img.begin(),eit=img.end(),it=intensity.begin();
           iit != eit;
           ++iit,++it) {
        const rgbPixel& pix = *iit;
        *it = (float(pix.getRed()) + float(pix.getGreen()) +
               float(pix.getBlue()))/765.0f; // 255*3 = 765
      }
    } else {
      vector<channel::value_type>::iterator it;
      vector<rgbPixel>::const_iterator iit,eit;

      for (y=0;y<img.rows();y++) {
        const vector<rgbPixel>& imgVct = img.getRow(y);
        vector<channel::value_type>& intVct = intensity.getRow(y);

        for (iit=imgVct.begin(),eit=imgVct.end(),it=intVct.begin();
             iit!=eit;
             ++iit,++it) {
          const rgbPixel& pix = *iit;
          *it = (float(pix.getRed()) + float(pix.getGreen()) +
                 float(pix.getBlue()))/765.0f; // 255*3 = 765
        }
      }
    }
    return true;
  }

  bool splitImageTorgI::getIntensity(const image& img,
                                     channel8& intensity) const {

    int y;
    intensity.resize(img.rows(),img.columns(),0,false,false);

    if ((intensity.getMode() == channel8::Connected) &&
        (img.getMode() == image::Connected)) {
      image::const_iterator iit,eit;
      channel8::iterator it;

      for (iit=img.begin(),eit=img.end(),it=intensity.begin();
           iit != eit;
           ++iit,++it) {
        const rgbPixel& pix = *iit;
        *it = ubyte((int(pix.getRed()) + int(pix.getGreen()) +
                     int(pix.getBlue()))/3);
      }
    } else {
      vector<channel8::value_type>::iterator it;
      vector<rgbPixel>::const_iterator iit,eit;

      for (y=0;y<img.rows();y++) {
        const vector<rgbPixel>& imgVct = img.getRow(y);
        vector<channel8::value_type>& intVct = intensity.getRow(y);

        for (iit=imgVct.begin(),eit=imgVct.end(),it=intVct.begin();
             iit!=eit;
             ++iit,++it) {
          const rgbPixel& pix = *iit;
          *it = ubyte((int(pix.getRed()) + int(pix.getGreen()) +
                       int(pix.getBlue()))/3);
        }
      }
    }

    return true;
  }


  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageTorgI::getFirst(const image& img, channel& c1) const {
    return getR(img,c1);
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageTorgI::getFirst(const image& img, channel8& c1) const {
    return getR(img,c1);
  }

  /*
   * Returns the second of the three channels into which the image is split.
   */
  bool splitImageTorgI::getSecond(const image& img, channel& c2) const {
    return getG(img,c2);
  }

  /*
   * Returns the second of the three channels into which the image is split.
   */
  bool splitImageTorgI::getSecond(const image& img, channel8& c2) const {
    return getG(img,c2);
  }

  /*
   * Returns the third of the three channels into which the image is split.
   */
  bool splitImageTorgI::getThird(const image& img, channel& c2) const {
    return getIntensity(img,c2);
  }

  /*
   * Returns the third of the three channels into which the image is split.
   */
  bool splitImageTorgI::getThird(const image& img, channel8& c2) const {
    return getIntensity(img,c2);
  }

} // end of namespace lti
