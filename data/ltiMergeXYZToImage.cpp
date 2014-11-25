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
 * file .......: ltiMergeXYZToImage.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiMergeXYZToImage.cpp,v 1.4 2006/02/08 11:29:51 ltilib Exp $
 */

#include "ltiMergeXYZToImage.h"

namespace lti {

  // ------------------------------------------
  //       merge XYZ channels to image
  // ------------------------------------------

  // return the name of this type
  const char* mergeXYZToImage::getTypeName() const {
    return "mergeXYZToImage";
  }

  // returns a pointer to a clone of the functor.
  functor* mergeXYZToImage::clone() const {
    return (new mergeXYZToImage(*this));
  }

  // merge float channels
  bool mergeXYZToImage::apply(const matrix<float>& c1,
                              const matrix<float>& c2,
                              const matrix<float>& c3,
                              image& img) const {
    int xSize,ySize;
    point p;              // coordinates
    float r,g,b;          // unnormed RGB channels
    float X,Y,Z;

    xSize=c1.columns();
    ySize=c1.rows();
    if ((c2.columns()!=xSize)||
	(c2.rows()!=ySize)||
	(c3.columns()!=xSize)||
	(c3.rows()!=ySize)) {
      setStatusString("sizes of channels do not match");
      return false;    }

    img.resize(ySize,xSize,rgbPixel(),false,false);

    for (p.y=0;p.y<ySize;p.y++)
      for (p.x=0;p.x<xSize;p.x++) {

	X = c1.at(p);
	Y = c2.at(p);
	Z = c3.at(p);

	// for explanation of magic numbers see Gonzales & Woods

	r =  X*3.240479f - Y*1.537150f - Z*0.498535f;
	g = -X*0.969256f + Y*1.875992f + Z*0.041556f;
	b =  X*0.055648f - Y*0.204043f + Z*1.057311f;

	img.at(p).set(static_cast<ubyte>(r*255.0f),
                      static_cast<ubyte>(g*255.0f),
                      static_cast<ubyte>(b*255.0f),
                      0);
      }

    return true;
  };


  // merge 8-bit channels channels
  bool mergeXYZToImage::apply(const channel8& c1,
                              const channel8& c2,
                              const channel8& c3,
                              image& img) const {
    int xSize,ySize;
    point p;              // coordinates
    float r,g,b;          // unnormed RGB channels
    float X,Y,Z;

    xSize=c1.columns();
    ySize=c1.rows();
    if ((c2.columns()!=xSize)||
	(c2.rows()!=ySize)||
	(c3.columns()!=xSize)||
	(c3.rows()!=ySize)) {
      setStatusString("sizes of channels do not match");
      return false;    }

    img.resize(ySize,xSize,rgbPixel(),false,false);

    for (p.y=0;p.y<ySize;p.y++)
      for (p.x=0;p.x<xSize;p.x++) {

	X = ((float)c1.at(p))/255.0f;
	Y = ((float)c2.at(p))/255.0f;
	Z = ((float)c3.at(p))/255.0f;

	// for explanation of magic numbers see Gonzales & Woods

	r =  X*3.240479f - Y*1.537150f - Z*0.498535f;
	g = -X*0.969256f + Y*1.875992f + Z*0.041556f;
	b =  X*0.055648f - Y*0.204043f + Z*1.057311f;

	// To correct the 8-bit-split operations' rounding error :
	// limit the range of r, g and b to values between 0 and 1
	// the resulting error in the merged image is practically
	// invisible.

	if (r<0)
	  r =0;
	else if (r>1)
	  r = 1;

	if(g<0)
	  g=0;
	else if(g>1)
	  g=1;

	if(b<0)
	  b = 0;
	else if(b>1)
	  b=1;

	img.at(p).set(static_cast<ubyte>(r*255.0f),
                      static_cast<ubyte>(g*255.0f),
                      static_cast<ubyte>(b*255.0f),
                      0);
      }

    return true;
  };


  // merge float values
  bool mergeXYZToImage::apply(const float& X,
                              const float& Y,
                              const float& Z,
                              rgbPixel& pixel) const {
    float r,g,b;          // unnormed RGB channels

    // for explanation of magic numbers see Gonzales & Woods

    r =  X*3.240479f - Y*1.537150f - Z*0.498535f;
    g = -X*0.969256f + Y*1.875992f + Z*0.041556f;
    b =  X*0.055648f - Y*0.204043f + Z*1.057311f;

    pixel.set(static_cast<ubyte>(r*255.0f),
              static_cast<ubyte>(g*255.0f),
              static_cast<ubyte>(b*255.0f),
              0);

    return true;
  };


  // merge 8-bit values
  bool mergeXYZToImage::apply(const ubyte& c1,
                              const ubyte& c2,
                              const ubyte& c3,
                              rgbPixel& pixel) const {

    float r,g,b;          // unnormed RGB channels
    float X,Y,Z;

    X = float(c1)/255.0f;
    Y = float(c2)/255.0f;
    Z = float(c3)/255.0f;

    // for explanation of magic numbers see Gonzales & Woods

    r =  X*3.240479f - Y*1.537150f - Z*0.498535f;
    g = -X*0.969256f + Y*1.875992f + Z*0.041556f;
    b =  X*0.055648f - Y*0.204043f + Z*1.057311f;

    // To correct the 8-bit-split operations' rounding error :
    // limit the range of r, g and b to values between 0 and 1
    // the resulting error in the merged image is practically
    // invisible.

    if (r<0)
      r =0;
    else if (r>1)
      r = 1;

    if(g<0)
      g=0;
    else if(g>1)
      g=1;

    if(b<0)
      b = 0;
    else if(b>1)
      b=1;

    pixel.set(static_cast<ubyte>(r*255.0f),
              static_cast<ubyte>(g*255.0f),
              static_cast<ubyte>(b*255.0f),
              0);

    return true;
  };



} // end of namespace
