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
 * file .......: ltiSplitImageToOCP.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiSplitImageToOCP.cpp,v 1.4 2006/02/08 11:53:49 ltilib Exp $
 */

#include "ltiSplitImageToOCP.h"
#include "ltiConstants.h"


namespace lti {

  // ========= splitImageToOPC ===============

  // split image to OCP channels (opponent colour system after Pomierskki)
  // returns the name of this type
  const char* splitImageToOCP::getTypename() const {
    return "splitImageToOCP";
  }

  // returns a pointer to a clone of the functor
  functor* splitImageToOCP::clone() const {
    return (new splitImageToOCP(*this));
  }

  // split image into float channels
  bool splitImageToOCP::apply(const image& img,
                              channel& c1,
                              channel& c2,
                              channel& c3) const {
    point p;           // coordinates

    // make the channels size of source image...
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {

        // take pixel at position p
        const rgbPixel& pix = img.at(p);

        // R-G
        // 0.5/255 = 0.00196078f
        c1.at(p) =  ( static_cast<float>(pix.getRed()) -
                      static_cast<float>(pix.getGreen()) ) * 0.00196078f;
        
        // B-Y
        // 0.25/255 = 0.000980392
        c2.at(p) =  ( static_cast<float>(pix.getRed()) +
                      static_cast<float>(pix.getGreen()) ) * (-0.000980392f) +
                      static_cast<float>(pix.getBlue()) * 0.00196078f;

        // W-B
        // 1/(3*255) = 0.00130719
        c3.at(p) =  ( static_cast<float>(pix.getRed())   +
                      static_cast<float>(pix.getGreen()) +
                      static_cast<float>(pix.getBlue()) ) * 0.00130719f;
      } // loop
    return true;
  }

  // split Image to 8-bit channels
  // channels RG and BY arwe shifted up to obtain positive values !
  // for more accuracy use float channels
  bool splitImageToOCP::apply(const image& img,
                              channel8& c1,
                              channel8& c2,
                              channel8& c3) const {
    point p;               // coordinates
    float RG, BY, WB;      // opponent colour channels

    // make the channels size of source image...
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    for (p.y=0;p.y<img.rows();p.y++)
      for (p.x=0;p.x<img.columns();p.x++) {

        // take pixel at position p
        const rgbPixel& pix = img.at(p);

        // R-G
        RG =  ( static_cast<float>(pix.getRed()) -
                static_cast<float>(pix.getGreen()) ) * 0.5f + 128.0f;
        
        // B-Y
        BY =  ( static_cast<float>(pix.getRed()) +
                static_cast<float>(pix.getGreen()) ) * (-0.25f) +
                static_cast<float>(pix.getBlue()) * 0.5f + 128.0f;

        // W-B
        WB =  ( static_cast<float>(pix.getRed())   +
                static_cast<float>(pix.getGreen()) +
                static_cast<float>(pix.getBlue()) ) * 0.3333333333333f;


        c1.at(p) = static_cast<ubyte>(RG);
        c2.at(p) = static_cast<ubyte>(BY);
        c3.at(p) = static_cast<ubyte>(WB);

      } // loop
    return true;
  }

  bool splitImageToOCP::apply(const rgbPixel& pix,
                              float& c1,
                              float& c2,
                              float& c3) const {


    // R-G
    // 0.5/255 = 0.00196078f
    c1 =  ( static_cast<float>(pix.getRed()) -
            static_cast<float>(pix.getGreen()) ) * 0.00196078f;
    
    // B-Y
    // 0.25/255 = 0.000980392
    c2 =  ( static_cast<float>(pix.getRed()) +
            static_cast<float>(pix.getGreen()) ) * (-0.000980392f) +
      static_cast<float>(pix.getBlue()) * 0.00196078f;
    
    // W-B
    // 1/(3*255) = 0.00130719
    c3 =  ( static_cast<float>(pix.getRed())   +
            static_cast<float>(pix.getGreen()) +
            static_cast<float>(pix.getBlue()) ) * 0.00130719f;
    return true;
  }

  bool splitImageToOCP::apply(const rgbPixel& pix,
                              ubyte& c1,
                              ubyte& c2,
                              ubyte& c3) const {

    float RG,BY,WB;
    // R-G
    RG =  ( static_cast<float>(pix.getRed()) -
            static_cast<float>(pix.getGreen()) ) * 0.5f + 128.0f;
    
    // B-Y
    BY =  ( static_cast<float>(pix.getRed()) +
            static_cast<float>(pix.getGreen()) ) * (-0.25f) +
            static_cast<float>(pix.getBlue()) * 0.5f + 128.0f;
    
    // W-B
    WB =  ( static_cast<float>(pix.getRed())   +
            static_cast<float>(pix.getGreen()) +
            static_cast<float>(pix.getBlue()) ) * 0.3333333333333f;
    
    
    c1 = static_cast<ubyte>(RG);
    c2 = static_cast<ubyte>(BY);
    c3 = static_cast<ubyte>(WB);

    return true;
  }
} // end of namespace lti
