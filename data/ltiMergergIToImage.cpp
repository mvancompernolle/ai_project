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
 * file .......: ltiMergergIToImage.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiMergergIToImage.cpp,v 1.7 2006/02/08 11:30:51 ltilib Exp $
 */

#include "ltiMergergIToImage.h"

namespace lti {

  // merge rgI channels to image

  // returns the name of this type
  const char* mergergIToImage::getTypeName() const {
    return "mergergIToImage";
  }

  // returns a pointer to a clone of the functor.
  functor* mergergIToImage::clone() const {
    return (new mergergIToImage(*this));
  }

  bool mergergIToImage::apply(const matrix<float>& c1,
                              const matrix<float>& c2,
                              const matrix<float>& c3,
                              image& img) const {
    int xSize,ySize;
    float i;
    point p;            // coordinates

    xSize=c1.columns();
    ySize=c1.rows();
    if ((c2.columns()!=xSize) || (c2.rows()!=ySize) ||
        (c3.columns()!=xSize) || (c3.rows()!=ySize)) {
      setStatusString("sizes of channels do not match");
      return false;
    }

    img.resize(ySize,xSize,rgbPixel(),false,false);

    for (p.y=0;p.y<ySize;p.y++) {
      for (p.x=0;p.x<xSize;p.x++) {
				i	= c3.at(p)*765.0f; // 765 = 3*255
				const float x = c1.at(p)*i;
				const float y = c2.at(p)*i;
				const ubyte r = static_cast<ubyte>(lti::max(0,lti::min(static_cast<int>(x),255) ));
				const ubyte g = static_cast<ubyte>(lti::max(0,lti::min(static_cast<int>(y),255) ));
				const ubyte b = static_cast<ubyte>(lti::max(0,lti::min(static_cast<int>(i-x-y),255)));
				img.at(p).set(r, g, b, 0);
      }
    }
    return true;
  }

  bool mergergIToImage::apply(const channel8& c1,
                              const channel8& c2,
                              const channel8& c3,
                              image& img) const {

    int xSize,ySize;
    point p;          // coordinates

    // check the size of the three channels
    xSize=c1.columns();
    ySize=c1.rows();
    if ((c2.columns()!=xSize) || (c2.rows()!=ySize) ||
        (c3.columns()!=xSize) || (c3.rows()!=ySize)) {
      setStatusString("sizes of channels do not match");
      return false; // if sizes don't match
    }

    // new merged image
    img.resize(ySize,xSize,rgbPixel(),false,false);

    for (p.y=0;p.y<ySize;p.y++)
      for (p.x=0;p.x<xSize;p.x++) {
        const int i = 3*c3.at(p);
        const int x = c1.at(p)*i/255;
        const int y = c2.at(p)*i/255;
				const ubyte r = static_cast<ubyte>(lti::max(0,lti::min(x,255) ));
        const ubyte g = static_cast<ubyte>(lti::max(0,lti::min(y,255) ));
				const ubyte b = static_cast<ubyte>(lti::max(0,lti::min(i-x-y,255)));
        img.at(p).set(r, g, b, 0);
      }
    return true;
  }

  bool mergergIToImage::apply(const float& c1,
                              const float& c2,
                              const float& c3,
                              rgbPixel& pixel) const {

		const float i	= c3*765.0f; // 765 = 3*255
		const float x = c1*i;
		const float y = c2*i;
		const ubyte r = static_cast<ubyte>(lti::max(0,lti::min(static_cast<int>(x),255) ));
		const ubyte g = static_cast<ubyte>(lti::max(0,lti::min(static_cast<int>(y),255) ));
		const ubyte b = static_cast<ubyte>(lti::max(0,lti::min(static_cast<int>(i-x-y),255)));
    pixel.set(r, g, b, 0);
    return true;
  }

  bool mergergIToImage::apply(const ubyte& c1,
                              const ubyte& c2,
                              const ubyte& c3,
                              rgbPixel& pixel) const {

    const int i = c3*3;
    const int x = c1*i/255;
    const int y = c2*i/255;
		const ubyte r = static_cast<ubyte>(lti::max(0,lti::min(x,255) ));
    const ubyte g = static_cast<ubyte>(lti::max(0,lti::min(y,255) ));
		const ubyte b = static_cast<ubyte>(lti::max(0,lti::min(i-x-y,255)));
    pixel.set(r, g, b, 0);
    return true;
  }


} // end of namespace
