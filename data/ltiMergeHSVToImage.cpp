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
 * file .......: ltiMergeHSVToImage.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiMergeHSVToImage.cpp,v 1.4 2006/02/08 11:29:01 ltilib Exp $
 */

#include "ltiMergeHSVToImage.h"

namespace lti {

  // merge HSV channels to Image

  // return the name of this type
  const char* mergeHSVToImage::getTypeName() const {
    return "mergeHSVToImage";
  }

  // returns a pointer to a clone of the functor.
  functor* mergeHSVToImage::clone() const {
    return (new mergeHSVToImage(*this));
  }

  // merge float channels
  bool mergeHSVToImage::apply(const matrix<float>& c1,
                              const matrix<float>& c2,
                              const matrix<float>& c3,
                              image& img) const {
    int xSize,ySize;
    point p;
    int i,r,q,t,v; // auxiliary for transformation
    float f;       // auxiliary for transformation
    float H, S, V; // channels

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

	H = c1.at(p);
	S = c2.at(p);
	V = c3.at(p);

	if (c2.at(p)==0) {
          img.at(p).set(static_cast<ubyte>(V*255.0f),
                        static_cast<ubyte>(V*255.0f),
                        static_cast<ubyte>(V*255.0f),
                        0);
	}
	else {
	  i = static_cast<int>(H*6);
	  f = H*6-i;
	  r =static_cast<int>((V*(1.0f-S))*255.0f);
	  q =static_cast<int>((V*(1.0f-(S*f)))*255.0f);
	  t =static_cast<int>((V*(1.0f-(S*(1.0f-f))))*255.0f);
	  v =static_cast<int>(V*255.0f);

	  switch (i) {
	  case 0:
	  case 6: 	
            img.at(p).set(v,t,r,0);
	    break;
	  case 1:
            img.at(p).set(q,v,r,0);
	    break;

	  case 2:
            img.at(p).set(r,v,t,0);
	    break;

	  case 3:
            img.at(p).set(r,q,v,0);
	    break;

	  case 4:
            img.at(p).set(t,r,v,0);
	    break;

	  case 5:
            img.at(p).set(v,r,q,0);
	    break;
	  }
	}
      }

    return true;
  };


  // merge 8-bit channels channels
  bool mergeHSVToImage::apply(const channel8& c1,
                              const channel8& c2,
                              const channel8& c3,
                              image& img) const {
    int xSize,ySize;
    point p;
    int i,r,q,t,v;     // auxiliary for transformation
    float f;           // auxuliary for transformation
    float H, S, V;     // channels

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

	H = static_cast<float>(c1.at(p))/255.0f;
	S = static_cast<float>(c2.at(p))/255.0f;
	V = static_cast<float>(c3.at(p))/255.0f;

	if (S==0) {
	  img.at(p).set(static_cast<ubyte>(V*255.0f),
                        static_cast<ubyte>(V*255.0f),
                        static_cast<ubyte>(V*255.0f),
                        0);

	}
	else {
	  i = static_cast<int>(H*6);
	  f = H*6-i;
	  r = static_cast<int>((V*(1.0f-S))*255.0f);
	  q = static_cast<int>((V*(1.0f-(S*f)))*255.0f);
	  t = static_cast<int>((V*(1.0f-(S*(1.0f-f))))*255.0f);
	  v = static_cast<int>(V*255.0f);

	  switch (i) {
	  case 0:
	  case 6: 	
            img.at(p).set(v,t,r,0);
	    break;

	  case 1:
            img.at(p).set(q,v,r,0);
	    break;

	  case 2:
            img.at(p).set(r,v,t,0);
	    break;

	  case 3:
            img.at(p).set(r,q,v,0);
	    break;

	  case 4:
            img.at(p).set(t,r,v,0);
	    break;

          case 5:
            img.at(p).set(v,r,q,0);
	    break;
	  }
	}
      }

    return true;
  };

  // merge float values
  bool mergeHSVToImage::apply(const float& H,
                              const float& S,
                              const float& V,
                              rgbPixel& pixel) const {

    int i,r,q,t,v; // auxiliary for transformation
    float f;       // auxiliary for transformation

    if (S==0) {
      pixel.set(static_cast<ubyte>(V*255.0f),
                static_cast<ubyte>(V*255.0f),
                static_cast<ubyte>(V*255.0f),
                0);
    }
    else {
      i = static_cast<int>(H*6);
      f = H*6-i;
      r = static_cast<int>((V*(1.0f-S))*255.0f);
      q = static_cast<int>((V*(1.0f-(S*f)))*255.0f);
      t = static_cast<int>((V*(1.0f-(S*(1.0f-f))))*255.0f);
      v = static_cast<int>(V*255.0f);

      switch (i) {
	case 0:
	case 6:
          pixel.set(v,t,r,0);
	  break;
	case 1:
          pixel.set(q,v,r,0);
	  break;
	case 2:
          pixel.set(r,v,t,0);
	  break;
	case 3:
          pixel.set(r,q,v,0);
	  break;
	case 4:
          pixel.set(t,r,v,0);
	  break;
	case 5:
          pixel.set(v,r,q,0);
	  break;
      }
    }

    return true;
  }

  // merge 8-bit values
  bool mergeHSVToImage::apply(const ubyte& c1,
                              const ubyte& c2,
                              const ubyte& c3,
                              rgbPixel& pixel) const {

    int i,r,q,t,v;     // auxiliary for transformation
    float f;           // auxuliary for transformation
    float H, S, V;     // channels

    H = float(c1)/255.0f;
    S = float(c2)/255.0f;
    V = float(c3)/255.0f;

    if (S==0) {
      pixel.set(static_cast<ubyte>(V*255.0f),
                static_cast<ubyte>(V*255.0f),
                static_cast<ubyte>(V*255.0f),
                0);
    }
    else {
      i = static_cast<int>(H*6);
      f = H*6-i;
      r = static_cast<int>((V*(1.0f-S))*255.0f);
      q = static_cast<int>((V*(1.0f-(S*f)))*255.0f);
      t = static_cast<int>((V*(1.0f-(S*(1.0f-f))))*255.0f);
      v = static_cast<int>(V*255.0f);

      switch (i) {
	case 0:
	case 6:
          pixel.set(v,t,r,0);
	  break;
	case 1:
          pixel.set(q,v,r,0);
	  break;
	case 2:
          pixel.set(r,v,t,0);
	  break;
	case 3:
          pixel.set(r,q,v,0);
	  break;
	case 4:
          pixel.set(t,r,v,0);
	  break;
	case 5:
          pixel.set(v,r,q,0);
	  break;
      }
    }

    return true;
  };

} // end of namespace
