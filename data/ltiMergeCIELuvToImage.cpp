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
 * file .......: ltiMergeCIELuvToImage.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiMergeCIELuvToImage.cpp,v 1.5 2006/02/08 11:28:04 ltilib Exp $
 */

#include "ltiMergeCIELuvToImage.h"

namespace lti {

  // ------------------------------------------
  //       merge CIE-Luv channels to image
  // ------------------------------------------

  // return the name of this type
  const char* mergeCIELuvToImage::getTypeName() const {
    return "mergeCIELuvToImage";
  }

  // returns a pointer to a clone of the functor.
  functor* mergeCIELuvToImage::clone() const {
    return (new mergeCIELuvToImage(*this));
  }

  // merge float channels
  bool mergeCIELuvToImage::apply(const matrix<float>& c1,
                                 const matrix<float>& c2,
                                 const matrix<float>& c3,
                                 image& img) const {
    point p;                 // coordinates
    float up,vp,L,u,v,R,G,B; // unnormed RGB channels
    float X,Y,Z,t;

    static const float un = 0.2009f;
    static const float vn = 0.4610f;
    static const float a = 1.16f;
    static const float b = 0.16f;

    const int xSize = c1.columns();
    const int ySize = c1.rows();

    if ((c2.columns()!=xSize)||
	(c2.rows()!=ySize)||
	(c3.columns()!=xSize)||
	(c3.rows()!=ySize)) {
      setStatusString("sizes of channels do not match");
      return false;
    }

    img.resize(ySize,xSize,rgbPixel(),false,false);

    for (p.y=0;p.y<ySize;p.y++) {
      for (p.x=0;p.x<xSize;p.x++) {

	L = c1.at(p);
	u = c2.at(p);
	v = c3.at(p);

	// for explanation of magic numbers see Gonzales & Woods
        t = 13*L;
        up = (u/t)+un;
        vp = (v/t)+vn;

        t = ((L+b)/a);
        Y = (t*t*t);
        X = 2.25f*Y*up/vp; // -9*Y*up/((up-4)*vp - up*vp);
        Z = Y*(3.0f/vp - 5.0f) - X/3.0f; // (9*Y - 15*vp*Y - vp*X)/(3*vp);

        // assumes X,Y,Z are between 0 and 1 and R,G,B from 0 to 255
	R =  X*826.3227425f - Y*391.9736368f - Z*127.1267632f;
	G = -X*247.1600123f + Y*478.3774503f + Z*10.59676127f;
	B =  X*14.18989298f - Y*52.03054129f + Z*269.6143228f;

        
	img.at(p).set(static_cast<ubyte>(0.5f+R),
                      static_cast<ubyte>(0.5f+G),
                      static_cast<ubyte>(0.5f+B),
                      0);

      }
    }

    return true;
  };


  // merge 8-bit channels channels
  bool mergeCIELuvToImage::apply(const channel8& c1,
                                 const channel8& c2,
                                 const channel8& c3,
                                 image& img) const {
    img.clear();
    setStatusString("not implemented yet");
    return false;
  };

  // merge float values
  bool mergeCIELuvToImage::apply(const float& L,
                                 const float& u,
                                 const float& v,
                                 rgbPixel& pixel) const {

    float up,vp,R,G,B; // unnormed RGB channels
    float X,Y,Z,t;

    static const float un = 0.2009f;
    static const float vn = 0.4610f;
    static const float a = 1.16f;
    static const float b = 0.16f;

    // for explanation of magic numbers see Gonzales & Woods
    t = 13*L;
    up = (u/t)+un;
    vp = (v/t)+vn;

    t = ((L+b)/a);
    Y = (t*t*t);
    X = 2.25f*Y*up/vp; // -9*Y*up/((up-4)*vp - up*vp);
    Z = Y*(3.0f/vp - 5.0f) - X/3.0f; // (9*Y - 15*vp*Y - vp*X)/(3*vp);

    R =  X*826.3227425f - Y*391.9736368f - Z*127.1267632f;
    G = -X*247.1600123f + Y*478.3774503f + Z*10.59676127f;
    B =  X*14.18989298f - Y*52.03054129f + Z*269.6143228f;

    pixel.set(static_cast<ubyte>(0.5f+R),
              static_cast<ubyte>(0.5f+G),
              static_cast<ubyte>(0.5f+B),
              0);

    return true;
  };

  // merge 8-bit channels channels
  bool mergeCIELuvToImage::apply(const ubyte& c1,
                                 const ubyte& c2,
                                 const ubyte& c3,
                                 rgbPixel& pixel) const {
    pixel = lti::Black;
    setStatusString("not implemented yet");
    return false;
  };

} // end of namespace
