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
 * file .......: ltiSplitImageToHSV.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiSplitImageToHSV.cpp,v 1.4 2006/02/08 11:53:32 ltilib Exp $
 */

#include "ltiSplitImageToHSV.h"
#include "ltiConstants.h"

namespace lti {

  // ========= splitImageToHSV ===============

  // split image into HSV channels
  // returns the name of this type
  const char* splitImageToHSV::getTypename() const {
    return "splitImageToHSV";
  }

  // returns a pointer to a clone of the functor
  functor* splitImageToHSV::clone() const {
    return (new splitImageToHSV(*this));
  }


  bool splitImageToHSV::apply(const image& img,
                              channel& c1,
                              channel& c2,
                              channel& c3) const {
    point p;        // coordinates
    rgbPixel pix;   // single Pixel Element in RGB-values...

    // make the channels size of source image...
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    float mini,delta;
    float H,S,V;          // channels

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        // take pixel at position p
        pix = img.at(p);

        V = (float) maximum(pix.getRed(),pix.getGreen(),pix.getBlue());
        mini = (float) minimum(pix.getRed(),pix.getGreen(),pix.getBlue());
        if (V!=0) {
          S = 1.0f - (mini/V);
          if (S==0) {
            H=0;
          }
          else {
            delta = (V-mini)*6.0f;
            if (pix.getRed()==V)
              H = (pix.getGreen()-pix.getBlue())/delta;
            else if (pix.getGreen()==V)
              H = 0.333333f + (pix.getBlue()-pix.getRed())/delta;
            else
              H = 0.666667f + (pix.getRed()-pix.getGreen())/delta;
            if (H<0)
              H = H + 1.0f;
          }
        }
        else {
          S = 1.0f;
          H = 0;
        }

        V /= 255.0f ;  // V zwischen 0 und 1

        c1.at(p) = H;
        c2.at(p) = S;
        c3.at(p) = V;
      } // loop
    return true;
  }

  bool splitImageToHSV::apply(const image& img,
                              channel8& c1,
                              channel8& c2,
                              channel8& c3) const {

    point p;        // coordinates
    rgbPixel pix;   // single Pixel Element in RGB-values...

    // make the channels size of source image...
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    float mini,delta;
    float H,S,V;

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {

        // take image-pixel at position p
        pix = img.at(p);

        V = (float) maximum(pix.getRed(),pix.getGreen(),pix.getBlue());
        mini = (float) minimum(pix.getRed(),pix.getGreen(),pix.getBlue());
        if (V!=0) {
          S = 1.0f - (mini/V);
          if (S==0) {
            H=0;
          }
          else {
            delta = (V-mini)*6.0f;
            if (pix.getRed()==V)
              H = (pix.getGreen()-pix.getBlue())/delta;
            else if (pix.getGreen()==V)
              H = 0.333333f + (pix.getBlue()-pix.getRed())/delta;
            else
              H = 0.666667f + (pix.getRed()-pix.getGreen())/delta;
            if (H<0)
              H = H + 1.0f;
          }
        }
        else {
          S = 1.0f;
          H = 0;
        }

        c1.at(p) = (ubyte) (H*255.0f);
        c2.at(p) = (ubyte) (S*255.0f);
        c3.at(p) = (ubyte) V;
      }// loop
    return true;
  }

  bool splitImageToHSV::apply(const rgbPixel& pixel,
                              float& H,
                              float& S,
                              float& V) const {

    float mini,delta;
    const ubyte& red=pixel.getRed();
    const ubyte& green=pixel.getGreen();
    const ubyte& blue=pixel.getBlue();


    V = (float) maximum(red,green,blue);
    mini = (float) minimum(red,green,blue);
    if (V!=0) {
      S = 1.0f - (mini/V);
      if (S==0) {
        H=0;
      } else {
        delta = (V-mini)*6.0f;
        if (red==V) {
          H = (green-blue)/delta;
        } else if (green==V) {
          H = 0.333333f + (blue-red)/delta;
        } else {
          H = 0.666667f + (red-green)/delta;
        }
        if (H<0) {
          H = H + 1.0f;
        }
      }
    }
    else {
      S = 1.0f;
      H = 0;
    }

    V /= 255.0f ;  // V between 0 and 1

    return true;
  }

  bool splitImageToHSV::apply(const rgbPixel& pixel,
                              ubyte& H,
                              ubyte& S,
                              ubyte& V) const {


    float mini,delta;
    float h,s,v;

    const ubyte& red=pixel.getRed();
    const ubyte& green=pixel.getGreen();
    const ubyte& blue=pixel.getBlue();

    v = (float) maximum(red,green,blue);
    mini = (float) minimum(red,green,blue);
    if (v!=0) {
      s = 1.0f - (mini/v);
      if (s==0) {
        h=0;
      } else {
        delta = (v-mini)*6.0f;
        if (red==v) {
          h = (green-blue)/delta;
        } else if (green==v) {
          h = 0.333333f + (blue-red)/delta;
        } else {
          h = 0.666667f + (red-green)/delta;
        }
        if (h<0) {
          h = h + 1.0f;
        }
      }
    } else {
      s = 1.0f;
      h = 0;
    }

    H = (ubyte) (h*255.0f);
    S = (ubyte) (s*255.0f);
    V = (ubyte) v;

    return true;
  }

  bool splitImageToHSV::getValue(const image& img,
                                 channel8& value) const {

    value.resize(img.size(),0,false,false);
    image::const_iterator cit,e;
    channel8::iterator it;

    for (cit=img.begin(),e=img.end(),it=value.begin();
         cit!=e;
         cit++,it++) {
      const rgbPixel& tmp = (*cit);
      (*it)=max(tmp.getRed(),tmp.getGreen(),tmp.getBlue());
    }
    return true;
  }

  bool splitImageToHSV::getValue(const image& img,
                                 channel& value) const {

    value.resize(img.size(),0,false,false);
    image::const_iterator cit,e;
    channel::iterator it;

    for (cit=img.begin(),e=img.end(),it=value.begin();
         cit!=e;
         cit++,it++) {
      const rgbPixel& tmp = (*cit);
      (*it)=static_cast<channel::value_type>(max(tmp.getRed(),
                                                 tmp.getGreen(),
                                                 tmp.getBlue()))/255.0f;
    }
    return true;
  }


  /*
   * Returns the third of the three channels into which the image is split.
   */
  bool splitImageToHSV::getThird(const image& img, channel& c2) const {
    return getValue(img,c2);
  }

  /*
   * Returns the third of the three channels into which the image is split.
   */
  bool splitImageToHSV::getThird(const image& img, channel8& c2) const {
    return getValue(img,c2);
  }


} // end of namespace lti
