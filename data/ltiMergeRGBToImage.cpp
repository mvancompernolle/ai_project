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
 * file .......: ltiMergeRGBToImage.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiMergeRGBToImage.cpp,v 1.4 2006/02/08 11:29:35 ltilib Exp $
 */

#include "ltiMergeRGBToImage.h"

namespace lti {

  // merge RGB channels to image

  // returns the name of this type
  const char* mergeRGBToImage::getTypeName() const {
    return "mergeRGBToImage";
  }

  // returns a pointer to a clone of the functor.
  functor* mergeRGBToImage::clone() const {
    return (new mergeRGBToImage(*this));
  }

  bool mergeRGBToImage::apply(const matrix<float>& c1,
                              const matrix<float>& c2,
                              const matrix<float>& c3,
                              image& img) const {
    int xSize,ySize;
    point p;               // coordinates

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

    for (p.y=0;p.y<ySize;p.y++)
      for (p.x=0;p.x<xSize;p.x++) {
	img.at(p).set(static_cast<ubyte>(c1.at(p)*255.0f),
                      static_cast<ubyte>(c2.at(p)*255.0f),
                      static_cast<ubyte>(c3.at(p)*255.0f),
                      0);
      }

    return true;
  }

  bool mergeRGBToImage::apply(const matrix<float>& c1,
                              const matrix<float>& c2,
                              const matrix<float>& c3,
                              const matrix<float>& c4,
                              image& img) const {
    int xSize,ySize;
    point p;               // coordinates

    xSize=c1.columns();
    ySize=c1.rows();
    if ((c2.columns()!=xSize) || (c2.rows()!=ySize) ||
	(c3.columns()!=xSize) || (c3.rows()!=ySize) ||
        (c4.columns()!=xSize) || (c4.rows()!=ySize)) {
      setStatusString("sizes of channels do not match");
      return false;
    }

    img.resize(ySize,xSize,rgbPixel(),false,false);

    for (p.y=0;p.y<ySize;p.y++)
      for (p.x=0;p.x<xSize;p.x++) {
	img.at(p).set(static_cast<ubyte>(c1.at(p)*255.0f),
                      static_cast<ubyte>(c2.at(p)*255.0f),
                      static_cast<ubyte>(c3.at(p)*255.0f),
                      static_cast<ubyte>(c4.at(p)*255.0f));
      }

    return true;
  }

  bool mergeRGBToImage::apply(const channel8& c1,
                              const channel8& c2,
                              const channel8& c3,
                              image& img) const {
    int xSize,ySize;
    point p;            // coordinates

    xSize=c1.columns();
    ySize=c1.rows();
    if((c2.columns()!=xSize) || (c2.rows()!=ySize)||
       (c3.columns()!=xSize) || (c3.rows()!=ySize)) {
      setStatusString("sizes of channels do not match");
      return false;
    }

    img.resize(ySize,xSize,rgbPixel(),false,false);

    for (p.y=0;p.y<ySize;p.y++) {
      for (p.x=0;p.x<xSize;p.x++) {
	img.at(p).set(c1.at(p),c2.at(p),c3.at(p),0);
      }
    }

    return true;
  }

  bool mergeRGBToImage::apply(const channel8& c1,
                              const channel8& c2,
                              const channel8& c3,
                              const channel8& c4,
                              image& img) const {
    int xSize,ySize;
    point p;            // coordinates

    xSize=c1.columns();
    ySize=c1.rows();
    if((c2.columns()!=xSize) || (c2.rows()!=ySize) ||
       (c3.columns()!=xSize) || (c3.rows()!=ySize) ||
       (c4.columns()!=xSize) || (c4.rows()!=ySize)) {
      setStatusString("sizes of channels do not match");
      return false;
    }

    img.resize(ySize,xSize,rgbPixel(),false,false);

    for (p.y=0;p.y<ySize;p.y++) {
      for (p.x=0;p.x<xSize;p.x++) {
	img.at(p).set(c1.at(p),c2.at(p),c3.at(p),c4.at(p));
      }
    }

    return true;
  }

  bool mergeRGBToImage::apply(const float& red,
			      const float& green,
			      const float& blue,
			      rgbPixel& pixel) const {

    pixel.set(static_cast<ubyte>(red  *255.0f),
              static_cast<ubyte>(green*255.0f),
              static_cast<ubyte>(blue *255.0f),
              0);

    return true;
  };

  bool mergeRGBToImage::apply(const ubyte& red,
			      const ubyte& green,
			      const ubyte& blue,
			      rgbPixel& pixel) const{

    pixel.set(red,green,blue,0);
    return true;
  };


} // end of namespace
