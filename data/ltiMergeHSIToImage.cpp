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
 * file .......: ltiMergeHSIToImage.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiMergeHSIToImage.cpp,v 1.4 2006/02/08 11:28:44 ltilib Exp $
 */

#include "ltiMergeHSIToImage.h"

namespace lti {

  // merge HSI channels to image


  // return the name of this type
  const char* mergeHSIToImage::getTypeName() const {
    return "mergeHSIToImage";
  }

  // returns a pointer to a clone of the functor.
  functor* mergeHSIToImage::clone() const {
    return (new mergeHSIToImage(*this));
  }

  ubyte mergeHSIToImage::DeHSI[120][256];

  // Constructor
  mergeHSIToImage::mergeHSIToImage(void) {

    double s,h,alpha, beta;

    for (s = 0; s < 256; s++) {
      for (h=0;h<120;h++) {
	alpha = (h*Pi)/180.0;
	beta = s*cos(alpha) / cos((Pi/3.0)-alpha);
	beta = (255.0+beta)/3.0;
	DeHSI[(int)h][(int)s] = (ubyte) (beta+0.5);
      }
    }
  }

  // merge float channels
  bool mergeHSIToImage::apply(const matrix<float>& c1,
                              const matrix<float>& c2,
                              const matrix<float>& c3,
                              image& img) const {
    int xSize,ySize;
    point p;            // coordinates
    int h;
    float r,g,b;        // channels

    xSize=c1.columns();
    ySize=c1.rows();
    if ((c2.columns()!=xSize)||
	(c2.rows()!=ySize)||
	(c3.columns()!=xSize)||
	(c3.rows()!=ySize)) {
      setStatusString("sizes of channels do not match");
      return false;
    }

    img.resize(ySize,xSize,rgbPixel(),false,false);

    // H = c1
    // S = c2
    // I = c3

    for (p.y=0;p.y<ySize;p.y++)
      for (p.x=0;p.x<xSize;p.x++) {

	h = (int)(c1.at(p)*360.0f);

	if(h<120) {
	  r = DeHSI[h][(int)(c2.at(p)*255.0f)];
	  b = (float) (((1.0f-c2.at(p))*255.0f)/3.0f);
	  g = 255-r-b;

	}
	else if (h<240) {
	  r = (float) (((1.0f-c2.at(p))*255.0f)/3.0f);
	  g = DeHSI[h-120][(int)(c2.at(p)*255.0f)];
	  b = 255-r-g;
	}
	else if (h<360) {
	  g = (float) (((1.0f-c2.at(p))*255.0f)/3.0f);
	  b = DeHSI[h-240][(int)(c2.at(p)*255.0f)];
	  r = 255-g-b;
	}
	else {
	  r = DeHSI[h-360][(int)(c2.at(p)*255.0f)];
	  b = (float) (((1.0f-c2.at(p))*255.0f)/3.0f);
	  g=255-r-b;
	}


	r = (r*3*c3.at(p));
	g = (g*3*c3.at(p));
	b = (b*3*c3.at(p));

	if (r>=256.0f) {
	  g = g*255.0f/r;
	  b = b*255.0f/r;
	  r = 255.0f;
	}

	if (g>=256.0f) {
	  r = r*255.0f/g;
	  b = b*255.0f/g;
	  g = 255.0f;
	}

	if (b>=256.0f) {
	  r = r*255.0f/b;
	  g = g*255.0f/b;
	  b = 255.0f;
	}

	img.at(p).set(static_cast<ubyte>(r),
                      static_cast<ubyte>(g),
                      static_cast<ubyte>(b),
                      0);
      }

    return true;
  }

  // merge 8-bit-channels (quick and dirty, to be speed - optimized...)
  bool mergeHSIToImage::apply(const channel8& c1,
                              const channel8& c2,
                              const channel8& c3,
                              image& img) const {
    int xSize,ySize;
    point p;            // coordinates
    int h;
    float r,g,b;
    float H, S, I;          // channels

    xSize=c1.columns();
    ySize=c1.rows();
    if ((c2.columns()!=xSize)||
	(c2.rows()!=ySize)||
	(c3.columns()!=xSize)||
	(c3.rows()!=ySize)) {
      setStatusString("sizes of channels do not match");
      return false;
    }

    img.resize(ySize,xSize,rgbPixel(),false,false);

    // H = c1
    // S = c2
    // I = c3
    for (p.y=0;p.y<ySize;p.y++)
      for (p.x=0;p.x<xSize;p.x++) {

	H = ((float) c1.at(p))/255.0f;
	S = ((float) c2.at(p))/255.0f;
	I = ((float) c3.at(p))/255.0f;

	h = (int)(H*360.0f); // values: 0..360 when H is normalized to [0..1]

	if(h<120) {
	  r = DeHSI[h][(int)(S*255.0f)];
	  b = (float) (((1.0f-S)*255.0f)/3.0f);
	  g = 255-r-b;
	} else if (h<240) {
	  r = (float) (((1.0f-S)*255.0f)/3.0f);
	  g = DeHSI[h-120][(int)(S*255.0f)];
	  b = 255-r-g;
	} else if (h<360) {
	  g = (float) (((1.0f-S)*255.0f)/3.0f);
	  b = DeHSI[h-240][(int)(S*255.0f)];
	  r = 255-g-b;
	} else {
	  r = DeHSI[h-360][(int)(S*255.0f)];
	  b = (float) (((1.0f-S)*255.0f)/3.0f);
	  g=255-r-b;
	}

	r = (r*3*I);
	g = (g*3*I);
	b = (b*3*I);

	if (r>=256.0f) {
	  g = g*255.0f/r;
	  b = b*255.0f/r;
	  r = 255.0f;
	}

	if (g>=256.0f) {
	  r = r*255.0f/g;
	  b = b*255.0f/g;
	  g = 255.0f;
	}

	if (b>=256.0f) {
	  r = r*255.0f/b;
	  g = g*255.0f/b;
	  b = 255.0f;
	}

	img.at(p).set(static_cast<ubyte>(r),
                      static_cast<ubyte>(g),
                      static_cast<ubyte>(b),
                      0);
      }

    return true;
  }

  bool mergeHSIToImage::apply(const float& c1,
                              const float& c2,
                              const float& c3,
                              rgbPixel& pixel) const {
    int h;
    float r,g,b;        // channels

    // H = c1
    // S = c2
    // I = c3

    h = (int)(c1*360.0f);

    if(h<120) {
      r = DeHSI[h][(int)(c2*255.0f)];
      b = (float) (((1.0f-c2)*255.0f)/3.0f);
      g = 255-r-b;

    }
    else if (h<240) {
      r = (float) (((1.0f-c2)*255.0f)/3.0f);
      g = DeHSI[h-120][(int)(c2*255.0f)];
      b = 255-r-g;
    }
    else if (h<360) {
      g = (float) (((1.0f-c2)*255.0f)/3.0f);
      b = DeHSI[h-240][(int)(c2*255.0f)];
      r = 255-g-b;
    }
    else {
      r = DeHSI[h-360][(int)(c2*255.0f)];
      b = (float) (((1.0f-c2)*255.0f)/3.0f);
      g=255-r-b;
    }


    r = (r*3*c3);
    g = (g*3*c3);
    b = (b*3*c3);

    if (r>=256.0f) {
      g = g*255.0f/r;
      b = b*255.0f/r;
      r = 255.0f;
    }

    if (g>=256.0f) {
      r = r*255.0f/g;
      b = b*255.0f/g;
      g = 255.0f;
    }

    if (b>=256.0f) {
      r = r*255.0f/b;
      g = g*255.0f/b;
      b = 255.0f;
    }

    pixel.set(static_cast<ubyte>(r),
              static_cast<ubyte>(g),
              static_cast<ubyte>(b),
              0);

    return true;
  }

  // merge 8-bit-values (quick and dirty, to be speed - optimized...)
  bool mergeHSIToImage::apply(const ubyte& c1,
                              const ubyte& c2,
                              const ubyte& c3,
                              rgbPixel& pixel) const {
    int h;
    float r,g,b;
    float H, S, I;          // values

    H = float(c1)/255.0f;
    S = float(c2)/255.0f;
    I = float(c3)/255.0f;

    h = (int)(H*360.0f); // values: 0..360 when H is normalized to [0..1]

    if(h<120) {
      r = DeHSI[h][(int)(S*255.0f)];
      b = (float) (((1.0f-S)*255.0f)/3.0f);
      g = 255-r-b;
    } else if (h<240) {
      r = (float) (((1.0f-S)*255.0f)/3.0f);
      g = DeHSI[h-120][(int)(S*255.0f)];
      b = 255-r-g;
    } else if (h<360) {
      g = (float) (((1.0f-S)*255.0f)/3.0f);
      b = DeHSI[h-240][(int)(S*255.0f)];
      r = 255-g-b;
    } else {
      r = DeHSI[h-360][(int)(S*255.0f)];
      b = (float) (((1.0f-S)*255.0f)/3.0f);
      g=255-r-b;
    }

    r = (r*3*I);
    g = (g*3*I);
    b = (b*3*I);

    if (r>=256.0f) {
      g = g*255.0f/r;
      b = b*255.0f/r;
      r = 255.0f;
    }

    if (g>=256.0f) {
      r = r*255.0f/g;
      b = b*255.0f/g;
      g = 255.0f;
    }

    if (b>=256.0f) {
      r = r*255.0f/b;
      g = g*255.0f/b;
      b = 255.0f;
    }

    pixel.set(static_cast<ubyte>(r),
              static_cast<ubyte>(g),
              static_cast<ubyte>(b),
              0);

    return true;
  }
} // end of namespace
