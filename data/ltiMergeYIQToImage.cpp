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
 * file .......: ltiMergeYIQToImage.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiMergeYIQToImage.cpp,v 1.4 2006/02/08 11:30:09 ltilib Exp $
 */

#include "ltiMergeYIQToImage.h"

namespace lti {


  // merge YIQ channels to image

  // return the name of this type
  const char* mergeYIQToImage::getTypeName() const {
    return "mergeYIQToImage";
  }

  // returns a pointer to a clone of the functor.
  functor* mergeYIQToImage::clone() const {
    return (new mergeYIQToImage(*this));
  }

  // merge float channels
  bool mergeYIQToImage::apply(const matrix<float>& c1,
                              const matrix<float>& c2,
                              const matrix<float>& c3,
                              image& img) const {
    int xSize,ySize;
    point p;              // coordinates
    float Y, I, Q;
    float r,g,b;          // unnormed RGB channels

    xSize=c1.columns();
    ySize=c1.rows();
    if((c2.columns()!=xSize)||
       (c2.rows()!=ySize)||
       (c3.columns()!=xSize)||
       (c3.rows()!=ySize)) {
      setStatusString("sizes of channels do not match");
      return false;    }

    img.resize(ySize,xSize,rgbPixel(),false,false);

    for (p.y=0;p.y<ySize;p.y++)
      for (p.x=0;p.x<xSize;p.x++) {

	Y = c1.at(p);
	I = c2.at(p);
	Q = c3.at(p);

	// for explanation of magic numbers see Gonzales & Woods

	// The Y, I and Q channel are - unlike to the other colour space
	// transforms - expected to be unnormalized,
	// i.e. just as they come from
	// the RGB->YIQ channel splitting. This is necessary for propper
	// merging back to RGB space.
	// When normalized channels are required, this has to be done by
	// using a separate functor...

	r = Y*1.0f + I*1.139f + Q*0.648f;
	g = Y*1.0f - I*0.323f - Q*0.677f;
	b = Y*1.0f - I*1.323f + Q*1.785f;

	img.at(p).set(static_cast<ubyte>(r*255.0f),
                      static_cast<ubyte>(g*255.0f),
                      static_cast<ubyte>(b*255.0f),
                      0);
      }

    return true;
  };


  // merge 8-bit channels channels
  bool mergeYIQToImage::apply(const channel8& c1,
                              const channel8& c2,
                              const channel8& c3,
                              image& img) const {
    int xSize,ySize;
    point p;              // coordinates
    float Y, I, Q,r, g, b;


    xSize=c1.columns();
    ySize=c1.rows();
    if ((c2.columns()!=xSize) || (c2.rows()!=ySize)||
	(c3.columns()!=xSize) || (c3.rows()!=ySize)) {
      setStatusString("sizes of channels do not match");
      return false;    }

    img.resize(ySize,xSize,rgbPixel(),false,false);

    for (p.y=0;p.y<ySize;p.y++)
      for (p.x=0;p.x<xSize;p.x++) {

	Y = static_cast<float>(c1.at(p))/255.0f;
	I = static_cast<float>(c2.at(p))/255.0f;
	Q = static_cast<float>(c3.at(p))/255.0f;


	// for explanation of magic numbers see Gonzales & Woods

	// The Y, I and Q channel are - unlike to the other colour space
	// transforms - expected to be unnormalized, i.e. just as they come
	// from the RGB->YIQ channel splitting. This is necessary for propper
	// merging back to RGB space.  When normalized channels are required,
	// this has to be done by using a separate functor...

	r = Y*1.0f + I*1.139f + Q*0.648f; // range: [0..2.787]
	g = Y*1.0f - I*0.323f - Q*0.677f; // range: [-1..1]
	b = Y*1.0f - I*1.323f + Q*1.785f; // range: [-1.323..2.785]

	img.at(p).set(static_cast<ubyte>(r*255.0f),
                      static_cast<ubyte>(g*255.0f),
                      static_cast<ubyte>(b*255.0f),
                      0);
      }

    return true;
  };



  // merge float values
  bool mergeYIQToImage::apply(const float& Y,
                              const float& I,
                              const float& Q,
                              rgbPixel& pixel) const {

    float r,g,b;          // unnormed RGB channels

    // for explanation of magic numbers see Gonzales & Woods

    // The Y, I and Q channel are - unlike to the other colour space
    // transforms - expected to be unnormalized,
    // i.e. just as they come from
    // the RGB->YIQ channel splitting. This is necessary for propper
    // merging back to RGB space.
    // When normalized channels are required, this has to be done by
    // using a separate functor...

    r = Y*1.0f + I*1.139f + Q*0.648f;
    g = Y*1.0f - I*0.323f - Q*0.677f;
    b = Y*1.0f - I*1.323f + Q*1.785f;

    pixel.set(static_cast<ubyte>(r*255.0f),
              static_cast<ubyte>(g*255.0f),
              static_cast<ubyte>(b*255.0f),
              0);

    return true;
  };


  // merge 8-bit values
  bool mergeYIQToImage::apply(const ubyte& c1,
                              const ubyte& c2,
                              const ubyte& c3,
                              rgbPixel& pixel) const {
    float Y, I, Q,r, g, b;

    Y = static_cast<float>(c1)/255.0f;
    I = static_cast<float>(c2)/255.0f;
    Q = static_cast<float>(c3)/255.0f;


    // for explanation of magic numbers see Gonzales & Woods

    // The Y, I and Q channel are - unlike to the other colour space
    // transforms - expected to be unnormalized, i.e. just as they come from
    // the RGB->YIQ channel splitting. This is necessary for propper
    // merging back to RGB space.
    // When normalized channels are required, this has to be done by using a
    // separate functor...

    r = Y*1.0f + I*1.139f + Q*0.648f; // range: [0..2.787]
    g = Y*1.0f - I*0.323f - Q*0.677f; // range: [-1..1]
    b = Y*1.0f - I*1.323f + Q*1.785f; // range: [-1.323..2.785]

    pixel.set(static_cast<ubyte>(r*255.0f),
              static_cast<ubyte>(g*255.0f),
              static_cast<ubyte>(b*255.0f),
              0);

    return true;
  };
  
} // end of namespace
