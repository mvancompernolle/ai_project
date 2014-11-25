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
 * file .......: ltiSplitImageToHSI.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiSplitImageToHSI.cpp,v 1.4 2006/02/08 11:53:15 ltilib Exp $
 */

#include "ltiSplitImageToHSI.h"
#include "ltiConstants.h"
#include "ltiTriMatrix.h"

namespace lti {

  // ========= splitImageToHSI ===============


  // split image into HSI channels
  // returns the name of this type
  const char* splitImageToHSI::getTypeName() const {
    return "splitImageToHSI";
  }

  // returns a pointer to a clone of the functor.
  functor* splitImageToHSI::clone() const {
    return (new splitImageToHSI(*this));
  }

  // static member initialization
  triMatrix<int>* splitImageToHSI::HueHSI = 0;
  bool           splitImageToHSI::hueInitialized = false;

  // Constructor
  // initializes triangular array for RGB->HSI-Conversion
  splitImageToHSI::splitImageToHSI(void) {
    double r,g;
    double alpha,beta;

    if (!hueInitialized) {
      hueInitialized = true;
      // a singleton object:  
      static triMatrix<int> theHueHSI(512);
      HueHSI = &theHueHSI;

      const int maxVal = HueHSI->size()-1;

      // See Gonzalez/Woods, page 232ff for a description of the algorithm
      //   H = acos(.5*((R-G)+(R-B))/sqrt((R-G)*(R-G)+(R-B)*(G-B)))
      // This equation is equivalent to (with r = R*maxVal/(3I) and so on...)
      // to
      //   H = acos(0.5*(3*r-maxVal)/sqrt(3(r^2+rg+g^2)-maxVal(3r+3g-maxVal)))
      for (r = 0;r<HueHSI->size();r++) {
        for (g = 0;g<HueHSI->size()-r;g++) {

          alpha = 0.5*(3*r-maxVal);
          alpha = alpha / sqrt(3*(r*r+r*g+g*g) - maxVal*(3*r+3*g-maxVal));

          beta = acos(alpha);
          beta = (18000.0 * beta) / Pi; // 180 degrees -> 18000.0

          if (maxVal>(2.0*g+r)) {
            beta = 36000.0-beta;
          }

          HueHSI->at(int(r),int(g)) = (unsigned short) (beta);
        } // g
      } // r
    }
  }

  // channels are float images...
  bool splitImageToHSI::apply(const image& img,
                              channel& c1,
                              channel& c2,
                              channel& c3) const {
    float mini;   // minimum of R,G,B
    int r;        // for channel red
    int g;        // for channel green
    int I3;       // threesome Intensity
    point p;      // coordinates
    rgbPixel pix;   // single Pixel Element in RGB-values...

    // make the channels size of source image...
    c1.resize(img.rows(),img.columns(),0,false,false);  // Hue
    c2.resize(img.rows(),img.columns(),0,false,false);  // Saturation
    c3.resize(img.rows(),img.columns(),0,false,false);  // Intensity

    static const int maxVal = HueHSI->size()-1;


    // after Gonzales & Woods:
    // I between 0 and 1
    // H         0 and 360
    // S     0 and 1
    // with I = (R+G+B)/3
    //      H = acos(.5*((R-G)+(R-B))/sqrt((R-G)*(R-G)+(R-B)*(G-B)))
    //      S = 1 - min(R,G,B)/I
    // note:
    // Triangular matrix HueHSI -> 0..36000.0

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        // take pixel at position p
        pix = img.at(p);

        I3 = (pix.getRed() + pix.getGreen() + pix.getBlue());
        if(I3 > 0) {

          r = (maxVal*pix.getRed())/I3;
          g = (maxVal*pix.getGreen())/I3;
          mini= (float) minimum(pix.getRed(), pix.getGreen(), pix.getBlue());

          c1.at(p) = ((float)HueHSI->at(r,g))/36000.0f; // H: range 0..1
          c2.at(p) = 1.0f - ((3.0f*mini)/I3);           // S
          c3.at(p) = (I3/765.0f); // 765 = 255.0f*3    // I
        }
        else {
          c1.at(p) = 0;        // H
          c2.at(p) = 0;        // S
          c3.at(p) = 0;        // I
        }
      }// loops
    return true;
  }


  // channels are 8-bit grey value images
  bool splitImageToHSI::apply(const image& img,
                              channel8& c1,
                              channel8& c2,
                              channel8& c3) const {
    float mini;   // minimum of R,G,B
    int r;        // for channel red
    int g;        // for channel green
    int I3;       // threesome Intensity
    point p;      // coordinates
    rgbPixel pix; // single Pixel Element in RGB-values...

    // make the channels size of source image...
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    static const int maxVal = HueHSI->size()-1;

    // after Gonzales & Woods:
    // I between 0 and 1
    // H         0 and 360
    // S     0 and 1
    // with I = (R+G+B)/3
    //      H = acos(.5*((R-G)+(R-B))/sqrt((R-G)*(R-G)+(R-B)*(G-B)))
    //      S = 1 - min(R,G,B)/I
    // note:
    // Triangular matrix HueHSI -> 0..36000.0

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {
        // take pixel at position p
        pix = img.at(p);

        I3 = (pix.getRed() + pix.getGreen() + pix.getBlue());
        if(I3 > 0) {

          r = (maxVal*pix.getRed())/I3;
          g = (maxVal*pix.getGreen())/I3;
          mini= (float) minimum(pix.getRed(), pix.getGreen(), pix.getBlue());

          c1.at(p) = (ubyte)((HueHSI->at(r,g)*255)/36000);// H
          c2.at(p) = (ubyte)(255 - ((765*mini)/I3));     // S
          c3.at(p) = (ubyte)(I3/3);                      // I
        }
        else {
          c1.at(p) = 0;      // H
          c2.at(p) = 0;      // S
          c3.at(p) = 0;      // I
        }

      }// loops
    return true;
  }

  bool splitImageToHSI::apply(const rgbPixel& pixel,
                              float& H,
                              float& S,
                              float& I) const {

    float mini;   // minimum of R,G,B
    int r;        // for channel red
    int g;        // for channel green
    int I3;       // threesome Intensity

    static const int maxVal = HueHSI->size()-1;


    // after Gonzales & Woods:
    // I between 0 and 1
    // H         0 and 360
    // S     0 and 1
    // with I = (R+G+B)/3
    //      H = acos(.5*((R-G)+(R-B))/sqrt((R-G)*(R-G)+(R-B)*(G-B)))
    //      S = 1 - min(R,G,B)/I
    // note:
    // Triangular matrix HueHSI -> 0..36000.0

    I3 = (pixel.getRed() + pixel.getGreen() + pixel.getBlue());
    if(I3 > 0) {

      r = (maxVal*pixel.getRed())/I3;
      g = (maxVal*pixel.getGreen())/I3;
      mini= (float) minimum(pixel.getRed(), pixel.getGreen(), pixel.getBlue());

      H = ((float)HueHSI->at(r,g))/36000.0f; // H: range 0..1
      S= 1.0f - ((3.0f*mini)/I3);           // S
      I = (I3/765.0f); // 765 = 255.0f*3    // I
    }
    else {
      H=0.0f;
      S=0.0f;
      I=0.0f;
    }
    return true;
  }

  bool splitImageToHSI::apply(const rgbPixel& pixel,
                              ubyte& H,
                              ubyte& S,
                              ubyte& I) const {

    float mini;   // minimum of R,G,B
    int r;        // for channel red
    int g;        // for channel green
    int I3;       // threesome Intensity

    static const int maxVal = HueHSI->size()-1;

    // after Gonzales & Woods:
    // I between 0 and 1
    // H         0 and 360
    // S     0 and 1
    // with I = (R+G+B)/3
    //      H = acos(.5*((R-G)+(R-B))/sqrt((R-G)*(R-G)+(R-B)*(G-B)))
    //      S = 1 - min(R,G,B)/I
    // note:
    // Triangular matrix HueHSI -> 0..36000.0

    I3 = (pixel.getRed() + pixel.getGreen() + pixel.getBlue());
    if(I3 > 0) {

      r = (maxVal*pixel.getRed())/I3;
      g = (maxVal*pixel.getGreen())/I3;
      mini= (float) minimum(pixel.getRed(), pixel.getGreen(), pixel.getBlue());

      H = (ubyte)((HueHSI->at(r,g)*255)/36000);// H
      S = (ubyte)(255 - ((765*mini)/I3));     // S
      I = (ubyte)(I3/3);                      // I
    }
    else {
      H=0;
      S=0;
      I=0;
    }
    return true;
  }

  /**
   * return the hue of the image.  If you need also the saturation and
   * the intensity please use the apply methods, which are much faster!
   */
  bool splitImageToHSI::getHue(const image& img, channel& hue) const {

    int r;          // for channel red
    int g;          // for channel green
    int I3;   // threesome Intensity
    point p;    // coordinates

    // make the channels size of source image...
    hue.resize(img.size(),0,false,false);  // Hue

    static const int maxVal = HueHSI->size()-1;

    // after Gonzales & Woods:
    // I between 0 and 1
    // H         0 and 360
    // S     0 and 1
    // with I = (R+G+B)/3
    //      H = acos(.5*((R-G)+(R-B))/sqrt((R-G)*(R-G)+(R-B)*(G-B)))
    //      S = 1 - min(R,G,B)/I
    // note:
    // Triangular matrix HueHSI -> 0..36000.0

    for (p.y=0;p.y<img.rows();p.y++) {
      for (p.x=0;p.x<img.columns();p.x++) {
        // take pixel at position p
        const rgbPixel& pix = img.at(p);

        I3 = (pix.getRed() + pix.getGreen() + pix.getBlue());

        if(I3 > 0) {

          r = (maxVal*pix.getRed())/I3;
          g = (maxVal*pix.getGreen()) /I3;

          hue.at(p) = ((float)HueHSI->at(r,g))/36000.0f; // H
        }
        else {
          hue.at(p) = 0;        // H
        }
      }// loops
    }
    return true;
  }

  /**
   * return the hue of the image.  If you need also the saturation and
   * the intensity please use the apply methods, which are much faster!
   */
  bool splitImageToHSI::getHue(const image& img, channel8& hue) const {
    int r;    // for channel red
    int g;    // for channel green
    int I3;   // threesome Intensity
    point p;    // coordinates

    // make the channels size of source image...
    hue.resize(img.rows(),img.columns(),0,false,false);

    static const int maxVal = HueHSI->size()-1;

    // after Gonzales & Woods:
    // I between 0 and 1
    // H         0 and 360
    // S     0 and 1
    // with I = (R+G+B)/3
    //      H = acos(.5*((R-G)+(R-B))/sqrt((R-G)*(R-G)+(R-B)*(G-B)))
    //      S = 1 - min(R,G,B)/I
    // note:
    // Triangular matrix HueHSI -> 0..36000.0

    static const float hueTo255 = 255.0f/36000.0f;

    for (p.y=0;p.y<img.rows();p.y++) {
      for (p.x=0;p.x<img.columns();p.x++) {
        // take pixel at position p
        const rgbPixel& pix = img.at(p);

        I3 = (pix.getRed() + pix.getGreen() + pix.getBlue());
        if(I3 > 0) {

          r = (maxVal*pix.getRed())/I3;
          g = (maxVal*pix.getGreen()) /I3;

          hue.at(p) = (ubyte)(((float)HueHSI->at(r,g))*hueTo255);// H
        }
        else {
          hue.at(p) = 0;        // H
        }

      }// loops
    }
    return true;
  }

  /**
   * return the saturation of the image.  If you need also the hue and
   * the intensity please use the apply methods, which are much faster!
   */
  bool splitImageToHSI::getSaturation(const image& img,
                                      channel& saturation) const {
    int y;
    int mini;
    int I3;
    saturation.resize(img.rows(),img.columns(),0,false,false);

    if ((saturation.getMode() == channel::Connected) &&
        (img.getMode() == image::Connected)) {
      image::const_iterator iit,eit;
      channel::iterator it;

      for (iit=img.begin(),eit=img.end(),it=saturation.begin();
           iit != eit;
           ++iit,++it) {
        const rgbPixel& pix = *iit;

        I3 = pix.getRed() + pix.getGreen() + pix.getBlue();
        if (I3 > 0) {
          mini = minimum(pix.getRed(),pix.getGreen(),pix.getBlue());
          *it = 1.0f - ((3.0f*mini)/I3);
        } else {
          *it = 0;
        }
      }
    } else {
      vector<channel::value_type>::iterator it;
      vector<rgbPixel>::const_iterator iit,eit;

      for (y=0;y<img.rows();y++) {
        const vector<rgbPixel>& imgVct = img.getRow(y);
        vector<channel::value_type>& intVct = saturation.getRow(y);

        for (iit=imgVct.begin(),eit=imgVct.end(),it=intVct.begin();
             iit!=eit;
             ++iit,++it) {
          const rgbPixel& pix = *iit;

          I3 = pix.getRed() + pix.getGreen()+pix.getBlue();
          if (I3 > 0) {
            mini = minimum(pix.getRed(),pix.getGreen(),pix.getBlue());
            *it = 1.0f - ((3.0f*mini)/I3);
          } else {
            *it = 0;
          }
        }
      }
    }
    return true;
  }

  /**
   * return the saturation of the image.  If you need also the hue and
   * the saturation please use the apply methods, which are much faster!
   */
  bool splitImageToHSI::getSaturation(const image& img,
                                      channel8& saturation) const {

    int y;
    int mini;
    int I3;

    saturation.resize(img.rows(),img.columns(),0,false,false);

    if ((saturation.getMode() == channel8::Connected) &&
        (img.getMode() == image::Connected)) {
      image::const_iterator iit,eit;
      channel8::iterator it;

      for (iit=img.begin(),eit=img.end(),it=saturation.begin();
           iit != eit;
           ++iit,++it) {

        const rgbPixel& pix = *iit;

        I3 = pix.getRed() + pix.getGreen() + pix.getBlue();
        if (I3 > 0) {
          mini = minimum(pix.getRed(),pix.getGreen(),pix.getBlue());
          *it = 255 - (ubyte)((765*mini)/I3);
        } else {
          *it = 0;
        }
      }
    } else {
      vector<channel8::value_type>::iterator it;
      vector<rgbPixel>::const_iterator iit,eit;

      for (y=0;y<img.rows();y++) {
        const vector<rgbPixel>& imgVct = img.getRow(y);
        vector<channel8::value_type>& intVct = saturation.getRow(y);

        for (iit=imgVct.begin(),eit=imgVct.end(),it=intVct.begin();
             iit!=eit;
             ++iit,++it) {
          const rgbPixel& pix = *iit;

          I3 = pix.getRed() + pix.getGreen() + pix.getBlue();

          if (I3 > 0) {
            mini = minimum(pix.getRed(),pix.getGreen(),pix.getBlue());                    *it = 255 - (ubyte)((765*mini)/I3);
          }
        }
      }
    }
    return true;
  }

  /**
   * return the intensity of the image.  If you need also the hue and
   * the intensity please use the apply methods, which are much faster!
   */
  bool splitImageToHSI::getIntensity(const image& img,
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

  /**
   * return the intensity of the image.  If you need also the hue and
   * the intensity please use the apply methods, which are much faster!
   */
  bool splitImageToHSI::getIntensity(const image& img,
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
  bool splitImageToHSI::getFirst(const image& img, channel& c1) const {
    return getHue(img,c1);
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImageToHSI::getFirst(const image& img, channel8& c1) const {
    return getHue(img,c1);
  }

  /*
   * Returns the second of the three channels into which the image is split.
   */
  bool splitImageToHSI::getSecond(const image& img, channel& c2) const {
    return getSaturation(img,c2);
  }

  /*
   * Returns the second of the three channels into which the image is split.
   */
  bool splitImageToHSI::getSecond(const image& img, channel8& c2) const {
    return getSaturation(img,c2);
  }

  /*
   * Returns the third of the three channels into which the image is split.
   */
  bool splitImageToHSI::getThird(const image& img, channel& c2) const {
    return getIntensity(img,c2);
  }

  /*
   * Returns the third of the three channels into which the image is split.
   */
  bool splitImageToHSI::getThird(const image& img, channel8& c2) const {
    return getIntensity(img,c2);
  }

} // end of namespace lti
