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
 * file .......: ltiSplitImageToGSC.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiSplitImageToGSC.cpp,v 1.5 2006/02/08 11:52:24 ltilib Exp $
 */

#include "ltiSplitImageToGSC.h"
#include "ltiConstants.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 2
#include "ltiDebug.h"

namespace lti {

  // ========= splitImageToGSC ===============


  // split image into GSC channels
  // returns the name of this type
  const char* splitImageToGSC::getTypeName() const {
    return "splitImageToGSC";
  }

  // returns a pointer to a clone of the functor.
  functor* splitImageToGSC::clone() const {
    return (new splitImageToGSC(*this));
  }

  bool splitImageToGSC::apply(const image& img,
                              channel& c1,
                              channel& c2,
                              channel& c3) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.size(),0,false,false);
    c2.resize(img.size(),0,false,false);
    c3.resize(img.size(),0,false,false);

    channel::iterator rit,git,bit;
    static const float factor = 2.0f/lti::constants<float>::Pi();

    for (p.y=0,rit=c1.begin(),git=c2.begin(),bit=c3.begin();
         p.y<img.rows();
         p.y++) {
      const vector<rgbPixel>& theRow = img.getRow(p.y);
      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,git++,bit++,imgIt++) {

        const rgbPixel& pix = (*imgIt);

        const ubyte r=pix.getRed();
        const ubyte g=pix.getGreen();
        const ubyte b=pix.getBlue();

        *rit = factor*atan2(r,max(g,b));
        *git = factor*atan2(g,max(r,b));
        *bit = factor*atan2(b,max(r,g));
      }
    }
    return true;
  }

  bool splitImageToGSC::apply(const image& img,
                              channel8& c1,
                              channel8& c2,
                              channel8& c3) const {
    point p;

    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    channel8::iterator rit,git,bit;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;
    static const float factor = 2.0f*255.0f/lti::constants<float>::Pi();

    for (p.y=0,rit=c1.begin(),git=c2.begin(),bit=c3.begin();
         p.y<img.rows();
         p.y++) {
      const vector<rgbPixel>& theRow = img.getRow(p.y);
      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,git++,bit++,imgIt++) {
        const rgbPixel& pix = (*imgIt);

        const ubyte r=pix.getRed();
        const ubyte g=pix.getGreen();
        const ubyte b=pix.getBlue();

        *rit = static_cast<ubyte>(factor*atan2(r,max(g,b)));
        *git = static_cast<ubyte>(factor*atan2(g,max(r,b)));
        *bit = static_cast<ubyte>(factor*atan2(b,max(r,g)));
      }
    }
    return true;
  }

  bool splitImageToGSC::apply(const rgbPixel& pixel,
                              float& c1,
                              float& c2,
                              float& c3) const {

    const ubyte r=pixel.getRed();
    const ubyte g=pixel.getGreen();
    const ubyte b=pixel.getBlue();

    static const float factor = 2.0f/lti::constants<float>::Pi();
    
    c1 = (factor*atan2(r,max(g,b)));
    c2 = (factor*atan2(g,max(r,b)));
    c3 = (factor*atan2(b,max(r,g)));

    return true;
  }

  bool splitImageToGSC::apply(const rgbPixel& pix,
                              ubyte& c1,
                              ubyte& c2,
                              ubyte& c3) const {

    const ubyte r=pix.getRed();
    const ubyte g=pix.getGreen();
    const ubyte b=pix.getBlue();
    
    static const float factor = 2.0f*255.0f/lti::constants<float>::Pi();

    c1 = static_cast<ubyte>(factor*atan2(r,max(g,b)));
    c2 = static_cast<ubyte>(factor*atan2(g,max(r,b)));
    c3 = static_cast<ubyte>(factor*atan2(b,max(r,g)));

    return true;
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToGSC::getFirst(const image& img, channel& c1) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.size(),0,false,false);

    channel::iterator rit;
    static const float factor = 2.0f/lti::constants<float>::Pi();

    for (p.y=0,rit=c1.begin();
         p.y<img.rows();
         p.y++) {
      const vector<rgbPixel>& theRow = img.getRow(p.y);
      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,imgIt++) {

        const rgbPixel& pix = (*imgIt);

        const ubyte r=pix.getRed();
        const ubyte g=pix.getGreen();
        const ubyte b=pix.getBlue();

        *rit = factor*atan2(r,max(g,b));
      }
    }

    return true;
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToGSC::getFirst(const image& img, channel8& c1) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.size(),0,false,false);

    channel8::iterator rit;
    static const float factor = 2.0f*255.0f/lti::constants<float>::Pi();

    for (p.y=0,rit=c1.begin();
         p.y<img.rows();
         p.y++) {

      const vector<rgbPixel>& theRow = img.getRow(p.y);

      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,imgIt++) {

        const rgbPixel& pix = (*imgIt);

        const ubyte r=pix.getRed();
        const ubyte g=pix.getGreen();
        const ubyte b=pix.getBlue();

        *rit = static_cast<ubyte>(factor*atan2(r,max(g,b)));
      }
    }

    return true;
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToGSC::getSecond(const image& img, channel& c1) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.size(),0,false,false);

    channel::iterator rit;
    static const float factor = 2.0f/lti::constants<float>::Pi();

    for (p.y=0,rit=c1.begin();
         p.y<img.rows();
         p.y++) {
      const vector<rgbPixel>& theRow = img.getRow(p.y);
      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,imgIt++) {

        const rgbPixel& pix = (*imgIt);

        const ubyte r=pix.getRed();
        const ubyte g=pix.getGreen();
        const ubyte b=pix.getBlue();

        *rit = factor*atan2(g,max(r,b));
      }
    }

    return true;
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToGSC::getSecond(const image& img, channel8& c1) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.size(),0,false,false);

    channel8::iterator rit;
    static const float factor = 2.0f*255.0f/lti::constants<float>::Pi();

    for (p.y=0,rit=c1.begin();
         p.y<img.rows();
         p.y++) {

      const vector<rgbPixel>& theRow = img.getRow(p.y);

      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,imgIt++) {

        const rgbPixel& pix = (*imgIt);

        const ubyte r=pix.getRed();
        const ubyte g=pix.getGreen();
        const ubyte b=pix.getBlue();

        *rit = static_cast<ubyte>(factor*atan2(g,max(r,b)));
      }
    }

    return true;
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToGSC::getThird(const image& img, channel& c1) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.size(),0,false,false);

    channel::iterator rit;
    static const float factor = 2.0f/lti::constants<float>::Pi();

    for (p.y=0,rit=c1.begin();
         p.y<img.rows();
         p.y++) {
      const vector<rgbPixel>& theRow = img.getRow(p.y);
      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,imgIt++) {

        const rgbPixel& pix = (*imgIt);

        const ubyte r=pix.getRed();
        const ubyte g=pix.getGreen();
        const ubyte b=pix.getBlue();

        *rit = factor*atan2(b,max(r,g));
      }
    }

    return true;
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToGSC::getThird(const image& img, channel8& c1) const {
    point p;
    vector<rgbPixel>::const_iterator imgIt,imgEIt;

    c1.resize(img.size(),0,false,false);

    channel8::iterator rit;
    static const float factor = 2.0f*255.0f/lti::constants<float>::Pi();

    for (p.y=0,rit=c1.begin();
         p.y<img.rows();
         p.y++) {

      const vector<rgbPixel>& theRow = img.getRow(p.y);

      for (p.x=0,imgIt=theRow.begin(),imgEIt=theRow.end();
           imgIt!=imgEIt;
           rit++,imgIt++) {

        const rgbPixel& pix = (*imgIt);

        const ubyte r=pix.getRed();
        const ubyte g=pix.getGreen();
        const ubyte b=pix.getBlue();

        *rit = static_cast<ubyte>(factor*atan2(b,max(r,g)));
      }
    }

    return true;
  }
} // end of namespace lti
