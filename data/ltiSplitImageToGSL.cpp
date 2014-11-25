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
 * file .......: ltiSplitImageToGSL.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiSplitImageToGSL.cpp,v 1.4 2006/02/08 11:52:42 ltilib Exp $
 */

#include "ltiSplitImageToGSL.h"

namespace lti {

  // ========= splitImageToGSL ===============


  // split image into GSL channels
  // returns the name of this type
  const char* splitImageToGSL::getTypeName() const {
    return "splitImageToGSL";
  }

  // returns a pointer to a clone of the functor.
  functor* splitImageToGSL::clone() const {
    return (new splitImageToGSL(*this));
  }

  bool splitImageToGSL::apply(const image& img,
                              channel& c1,
                              channel& c2,
                              channel& c3) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.size(),0,false,false);
    c2.resize(img.size(),0,false,false);
    c3.resize(img.size(),0,false,false);

    channel::iterator rit,git,bit;

    for (p.y=0,rit=c1.begin(),git=c2.begin(),bit=c3.begin();
         p.y<img.rows();
         p.y++) {
      const vector<rgbPixel>& theRow = img.getRow(p.y);
      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,git++,bit++,imgIt++) {

        const rgbPixel& pix = (*imgIt);

        const ubyte r=absdiff(pix.getRed(),pix.getGreen());
        const ubyte g=absdiff(pix.getRed(),pix.getBlue());
        const ubyte b=absdiff(pix.getGreen(),pix.getBlue());

        const float den = static_cast<float>(r)+g+b;
        if (den == 0.0f) {
          *rit = 0.0f;
          *git = 0.0f;
          *bit = 0.0f;          
        } else {
          *rit = r/den;
          *git = g/den;
          *bit = b/den;
        }
      }
    }

    return true;
  }

  bool splitImageToGSL::apply(const image& img,
                              channel8& c1,
                              channel8& c2,
                              channel8& c3) const {
    point p;

    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    channel8::iterator rit,git,bit;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    for (p.y=0,rit=c1.begin(),git=c2.begin(),bit=c3.begin();
         p.y<img.rows();
         p.y++) {
      const vector<rgbPixel>& theRow = img.getRow(p.y);
      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,git++,bit++,imgIt++) {
        const rgbPixel& pix = (*imgIt);

        const ubyte r=absdiff(pix.getRed(),pix.getGreen());
        const ubyte g=absdiff(pix.getRed(),pix.getBlue());
        const ubyte b=absdiff(pix.getGreen(),pix.getBlue());

        const int den = static_cast<int>(r)+g+b;
        if (den == 0) {
          *rit = 0;
          *git = 0;
          *bit = 0;          
        } else {
          *rit = 255*r/den;
          *git = 255*g/den;
          *bit = 255*b/den;
        }
      }
    }
    return true;
  }

  bool splitImageToGSL::apply(const rgbPixel& pix,
                              float& c1,
                              float& c2,
                              float& c3) const {
    
    const ubyte r=absdiff(pix.getRed(),pix.getGreen());
    const ubyte g=absdiff(pix.getRed(),pix.getBlue());
    const ubyte b=absdiff(pix.getGreen(),pix.getBlue());
    
    const float den = static_cast<float>(r)+g+b;
    if (den == 0.0f) {
      c1 = 0.0f;
      c2 = 0.0f;
      c3 = 0.0f;          
    } else {
      c1 = r/den;
      c2 = g/den;
      c3 = b/den;
    }
    return true;
  }

  bool splitImageToGSL::apply(const rgbPixel& pix,
                              ubyte& c1,
                              ubyte& c2,
                              ubyte& c3) const {

    const ubyte r=absdiff(pix.getRed(),pix.getGreen());
    const ubyte g=absdiff(pix.getRed(),pix.getBlue());
    const ubyte b=absdiff(pix.getGreen(),pix.getBlue());
    
    const int den = static_cast<int>(r)+g+b;
    if (den == 0) {
      c1 = 0;
      c2 = 0;
      c3 = 0;          
    } else {
      c1 = 255*r/den;
      c2 = 255*g/den;
      c3 = 255*b/den;
    }
    return true;
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToGSL::getFirst(const image& img, channel& c1) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.size(),0,false,false);

    channel::iterator rit;

    for (p.y=0,rit=c1.begin();
         p.y<img.rows();
         p.y++) {
      const vector<rgbPixel>& theRow = img.getRow(p.y);
      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,imgIt++) {

        const rgbPixel& pix = (*imgIt);

        const ubyte r=absdiff(pix.getRed(),pix.getGreen());
        const ubyte g=absdiff(pix.getRed(),pix.getBlue());
        const ubyte b=absdiff(pix.getGreen(),pix.getBlue());
        
        const float den = static_cast<float>(r)+g+b;
        if (den == 0.0f) {
          (*rit) = 0.0f;
        } else {
          (*rit) = r/den;
        }
      }
    }
    return true;
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToGSL::getFirst(const image& img, channel8& c1) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.size(),0,false,false);

    channel8::iterator rit;

    for (p.y=0,rit=c1.begin();
         p.y<img.rows();
         p.y++) {

      const vector<rgbPixel>& theRow = img.getRow(p.y);

      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,imgIt++) {

        const rgbPixel& pix = img.at(p);

        const ubyte r=absdiff(pix.getRed(),pix.getGreen());
        const ubyte g=absdiff(pix.getRed(),pix.getBlue());
        const ubyte b=absdiff(pix.getGreen(),pix.getBlue());

        const int den = static_cast<int>(r)+g+b;
        if (den == 0) {
          *rit = 0;
        } else {
          *rit = 255*r/den;
        }

      }
    }
    return true;
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToGSL::getSecond(const image& img, channel& c1) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.size(),0,false,false);

    channel::iterator rit;

    for (p.y=0,rit=c1.begin();
         p.y<img.rows();
         p.y++) {
      const vector<rgbPixel>& theRow = img.getRow(p.y);
      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,imgIt++) {

        const rgbPixel& pix = (*imgIt);

        const ubyte r=absdiff(pix.getRed(),pix.getGreen());
        const ubyte g=absdiff(pix.getRed(),pix.getBlue());
        const ubyte b=absdiff(pix.getGreen(),pix.getBlue());
        
        const float den = static_cast<float>(r)+g+b;
        if (den == 0.0f) {
          (*rit) = 0.0f;
        } else {
          (*rit) = g/den;
        }
      }
    }
    return true;
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToGSL::getSecond(const image& img, channel8& c1) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.size(),0,false,false);

    channel8::iterator rit;

    for (p.y=0,rit=c1.begin();
         p.y<img.rows();
         p.y++) {

      const vector<rgbPixel>& theRow = img.getRow(p.y);

      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,imgIt++) {

        const rgbPixel& pix = img.at(p);

        const ubyte r=absdiff(pix.getRed(),pix.getGreen());
        const ubyte g=absdiff(pix.getRed(),pix.getBlue());
        const ubyte b=absdiff(pix.getGreen(),pix.getBlue());

        const int den = static_cast<int>(r)+g+b;
        if (den == 0) {
          *rit = 0;
        } else {
          *rit = 255*g/den;
        }

      }
    }
    return true;
  }


  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToGSL::getThird(const image& img, channel& c1) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.size(),0,false,false);

    channel::iterator rit;

    for (p.y=0,rit=c1.begin();
         p.y<img.rows();
         p.y++) {
      const vector<rgbPixel>& theRow = img.getRow(p.y);
      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,imgIt++) {

        const rgbPixel& pix = (*imgIt);

        const ubyte r=absdiff(pix.getRed(),pix.getGreen());
        const ubyte g=absdiff(pix.getRed(),pix.getBlue());
        const ubyte b=absdiff(pix.getGreen(),pix.getBlue());
        
        const float den = static_cast<float>(r)+g+b;
        if (den == 0.0f) {
          (*rit) = 0.0f;
        } else {
          (*rit) = b/den;
        }
      }
    }
    return true;
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToGSL::getThird(const image& img, channel8& c1) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.size(),0,false,false);

    channel8::iterator rit;

    for (p.y=0,rit=c1.begin();
         p.y<img.rows();
         p.y++) {

      const vector<rgbPixel>& theRow = img.getRow(p.y);

      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,imgIt++) {

        const rgbPixel& pix = img.at(p);

        const ubyte r=absdiff(pix.getRed(),pix.getGreen());
        const ubyte g=absdiff(pix.getRed(),pix.getBlue());
        const ubyte b=absdiff(pix.getGreen(),pix.getBlue());

        const int den = static_cast<int>(r)+g+b;
        if (den == 0) {
          *rit = 0;
        } else {
          *rit = 255*b/den;
        }

      }
    }
    return true;
  }



} // end of namespace lti
