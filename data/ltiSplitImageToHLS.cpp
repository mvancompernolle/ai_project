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
 * file .......: ltiSplitImageToHLS.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiSplitImageToHLS.cpp,v 1.4 2006/02/08 11:52:59 ltilib Exp $
 */

#include "ltiSplitImageToHLS.h"
#include "ltiConstants.h"

namespace lti {

  // ========= splitImageToHLS ===============


  // split image to HLS channels
  // returns the name of this type
  const char* splitImageToHLS::getTypename() const {
    return "splitImageToHLS";
  }

  // returns a pointer to a clone of the functor
  functor* splitImageToHLS::clone() const {
    return (new splitImageToHLS(*this));
  }

  // split image into float channels
  bool splitImageToHLS::apply(const image& img,
                              channel& c1,
                              channel& c2,
                              channel& c3) const {
    point p;      // coordinates
    rgbPixel pix;   // single Pixel Element in RGB-values...

    // make the channels size of source image...
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    int mini,maxi, delta;
    float H,L,S;

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {

        // take pixel at position p
        pix = img.at(p);

        maxi = maximum(pix.getRed(), pix.getGreen(), pix.getBlue());
        mini = minimum(pix.getRed(), pix.getGreen(), pix.getBlue());


        // see Gonzales & Woods for magic numbers
        delta = maxi-mini;
        L = (float)(maxi + mini);

        if(delta == 0) {
          H = 0;
          L = (L/255.0f)/2;
          S = 0;

        }
        else {
          if(L<256.0f)
            S = delta / L;
          else
            S = delta/(510.0f-L);

          if (pix.getRed()==maxi)
            H = (pix.getGreen()-pix.getBlue())/(delta*6.0f);
          else if (pix.getGreen()==maxi)
            H = 0.333333f + (pix.getBlue()-pix.getRed())/(delta*6.0f);
          else
            H = 0.666667f + (pix.getRed()-pix.getGreen())/(delta*6.0f);
          if (H<0)
            H = H + 1.0f;

          L = (L/255.0f)/2;

        }

        c1.at(p) = H;
        c2.at(p) = L;
        c3.at(p) = S;
      } // loop
    return true;
  }


  // split image into 8-bit channels
  bool splitImageToHLS::apply(const image& img,
                              channel8& c1,
                              channel8& c2,
                              channel8& c3) const {
    point p;        // coordinates
    rgbPixel pix;   // single Pixel Element in RGB-values...

    // make the channels size of source image...
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    int mini,maxi, delta;
    float H,L,S;

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {

        // take pixel at position p
        pix = img.at(p);

        maxi = maximum(pix.getRed(), pix.getGreen(), pix.getBlue());
        mini = minimum(pix.getRed(), pix.getGreen(), pix.getBlue());

        // see Gonzales & Woods for magic numbers 

        delta = maxi-mini;
        L = (float)(maxi + mini);

        if(delta == 0) {
          H = 0;
          L = (L/255.0f)/2;
          S = 0;

        }
        else {
          if(L<256.0f)
            S = delta / L;
          else
            S = delta/(510.0f-L);

          if (pix.getRed()==maxi)
            H = (pix.getGreen()-pix.getBlue())/(delta*6.0f);
          else if (pix.getGreen()==maxi)
            H = 0.333333f + (pix.getBlue()-pix.getRed())/(delta*6.0f);
          else
            H = 0.666667f + (pix.getRed()-pix.getGreen())/(delta*6.0f);
          if (H<0)
            H = H + 1.0f;

          L = (L/255.0f)/2;
        }

        c1.at(p) = (ubyte)(H*255.0f);
        c2.at(p) = (ubyte)(L*255.0f);
        c3.at(p) = (ubyte)(S*255.0f);
      } // loop

    return true;
  }

  bool splitImageToHLS::apply(const rgbPixel& pixel,
                              float& H,
                              float& L,
                              float& S) const {


    const ubyte& red=pixel.getRed();
    const ubyte& green=pixel.getGreen();
    const ubyte& blue=pixel.getBlue();

    int mini,maxi, delta;
    maxi = maximum(red, green, blue);
    mini = minimum(red, green, blue);


    // see Gonzales & Woods for magic numbers

    delta = maxi-mini;
    L = (float)(maxi + mini);

    if(delta == 0) {
      H = 0;
      L = (L/255.0f)/2;
      S = 0;
    } else {
      if(L<256.0f) {
        S = delta / L;
      } else {
        S = delta/(510.0f-L);
      }

      if (red==maxi) {
        H = (green-blue)/(delta*6.0f);
      } else if (green==maxi) {
        H = 0.333333f + (blue-red)/(delta*6.0f);
      } else {
        H = 0.666667f + (red-green)/(delta*6.0f);
      }

      if (H<0) {
        H = H + 1.0f;
      }

      L = (L/255.0f)/2;

    }
    return true;
  }

  bool splitImageToHLS::apply(const rgbPixel& pixel,
                              ubyte& H,
                              ubyte& L,
                              ubyte& S) const {

    const ubyte& red=pixel.getRed();
    const ubyte& green=pixel.getGreen();
    const ubyte& blue=pixel.getBlue();

    int mini,maxi, delta;
    float h,l,s;

    maxi = maximum(red, green, blue);
    mini = minimum(red, green, blue);


    // see Gonzales & Woods for magic numbers
    delta = maxi-mini;
    l = (float)(maxi + mini);

    if(delta == 0) {
      h = 0;
      s = 0;
    } else {
      if(l<256.0f) {
        s = delta / l;
      } else {
        s = delta/(510.0f-l);
      }

      if (red==maxi) {
        h = (green-blue)/(delta*6.0f);
      } else if (green==maxi) {
        h = 0.333333f + (blue-red)/(delta*6.0f);
      } else {
        h = 0.666667f + (red-green)/(delta*6.0f);
      }

      if (h<0) {
        h = h + 1.0f;
      }

    }
    H = (ubyte)(h*255.0f);
    L = (ubyte)(l/2.0f);
    S = (ubyte)(s*255.0f);

    return true;
  }


} // end of namespace lti
