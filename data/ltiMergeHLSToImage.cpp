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
 * file .......: ltiMergeHSLToImage.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiMergeHLSToImage.cpp,v 1.4 2006/02/08 11:28:28 ltilib Exp $
 */

#include "ltiMergeHLSToImage.h"

namespace lti {


  // merge HLS channels to image

  // return the name of this type
  const char* mergeHLSToImage::getTypeName() const {
    return "mergeHLSToImage";
  }

  // returns a pointer to a clone of the functor.
  functor* mergeHLSToImage::clone() const {
    return (new mergeHLSToImage(*this));
  }

  // merge float channels
  bool mergeHLSToImage::apply(const matrix<float>& c1,
                              const matrix<float>& c2,
                              const matrix<float>& c3,
                              image& img) const {
    int xSize,ySize;
    point p;              // coordinates
    int i;                // auxiliary for transformation
    float f,t,r,q,v;      // auxiliary for transformation
    float H, L, S;        // channels


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
    // L = c2
    // S = c3


    for (p.y=0;p.y<ySize;p.y++)
      for (p.x=0;p.x<xSize;p.x++) {

	H = c1.at(p);
	L = c2.at(p);
	S = c3.at(p);

	if (S==0) {
	  img.at(p).set(static_cast<ubyte>(L*255.0f),
                        static_cast<ubyte>(L*255.0f),
                        static_cast<ubyte>(L*255.0f),
                        0);
	}
	else {
	  i = static_cast<int>(H*6.0f);
	  f = (H*6.0f)-i;
	  if (L<=0.5f) {
	    v = L*(1.0f+S);
	    r = L*(1.0f-S);
	    q = L*(1.0f+S*(1.0f-2.0f*f));
	    t = 2*L-q;
	  }
	  else {
	    v = L*(1.0f-S)+S;
	    r = L*(1.0f+S)-S;
	    q = S*(1.0f-L)*(1.0f-2*f)+L;
	    t = 2*L-q;
	  }
          
	  switch (i) {
            case 0:
            case 6:
              img.at(p).set(static_cast<ubyte>(255.0f*v),
                            static_cast<ubyte>(255.0f*t),
                            static_cast<ubyte>(255.0f*r),
                            0);
              break;
            case 1:
              img.at(p).set(static_cast<ubyte>(255.0f*q),
                            static_cast<ubyte>(255.0f*v),
                            static_cast<ubyte>(255.0f*r),
                            0);
              break;
            case 2:
              img.at(p).set(static_cast<ubyte>(255.0f*r),
                            static_cast<ubyte>(255.0f*v),
                            static_cast<ubyte>(255.0f*t),
                            0);
              break;
            case 3:
              img.at(p).set(static_cast<ubyte>(255.0f*r),
                            static_cast<ubyte>(255.0f*q),
                            static_cast<ubyte>(255.0f*v),
                            0);
              break;
            case 4:
              img.at(p).set(static_cast<ubyte>(255.0f*t),
                            static_cast<ubyte>(255.0f*r),
                            static_cast<ubyte>(255.0f*v),
                            0);
              break;
            case 5:
              img.at(p).set(static_cast<ubyte>(255.0f*v),
                            static_cast<ubyte>(255.0f*r),
                            static_cast<ubyte>(255.0f*q),
                            0);
              break;
	  };
	};
        
      }
    
    return true;
  };
  

  // merge 8-bit channels channels
  bool mergeHLSToImage::apply(const channel8& c1,
                              const channel8& c2,
                              const channel8& c3,
                              image& img) const {
    int xSize,ySize;
    point p;
    int i;               // auxiliary for transformation
    float f,t,r,q,v;      // auxiliary for transformation
    float H, L, S;


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
    // L = c2
    // S = c3


    for (p.y=0;p.y<ySize;p.y++)
      for (p.x=0;p.x<xSize;p.x++) {

	H = static_cast<float>(c1.at(p))/255.0f;  // normalize
	L = static_cast<float>(c2.at(p))/255.0f;
	S = static_cast<float>(c3.at(p))/255.0f;

	if (S==0) {
	  img.at(p).set(static_cast<ubyte>(L*255.0f),
                        static_cast<ubyte>(L*255.0f),
                        static_cast<ubyte>(L*255.0f),
                        0);
 	}
	else {
	  i = static_cast<int>(H*6.0f);
	  f = (H*6.0f)-i;
	  if (L<=0.5f) {
	    v = L*(1.0f+S);
	    r = L*(1.0f-S);
	    q = L*(1.0f+S*(1.0f-2.0f*f));
	    t = 2*L-q;
	  }
	  else {
	    v = L*(1.0f-S)+S;
	    r = L*(1.0f+S)-S;
	    q = S*(1.0f-L)*(1.0f-2*f)+L;
	    t = 2*L-q;
	  }
          
	  switch (i) {
            case 0:
            case 6:
              img.at(p).set(static_cast<ubyte>(255.0f*v),
                            static_cast<ubyte>(255.0f*t),
                            static_cast<ubyte>(255.0f*r),
                            0);
              break;
            case 1:
              img.at(p).set(static_cast<ubyte>(255.0f*q),
                            static_cast<ubyte>(255.0f*v),
                            static_cast<ubyte>(255.0f*r),
                            0);
              break;
            case 2:
              img.at(p).set(static_cast<ubyte>(255.0f*r),
                            static_cast<ubyte>(255.0f*v),
                            static_cast<ubyte>(255.0f*t),
                            0);
              break;
            case 3:
              img.at(p).set(static_cast<ubyte>(255.0f*r),
                            static_cast<ubyte>(255.0f*q),
                            static_cast<ubyte>(255.0f*v),
                            0);
              break;
            case 4:
              img.at(p).set(static_cast<ubyte>(255.0f*t),
                            static_cast<ubyte>(255.0f*r),
                            static_cast<ubyte>(255.0f*v),
                            0);
              break;
            case 5:
              img.at(p).set(static_cast<ubyte>(255.0f*v),
                            static_cast<ubyte>(255.0f*r),
                            static_cast<ubyte>(255.0f*q),
                            0);
              break;
	  };
	};

      }

    return true;
  };

  // merge float channels
  bool mergeHLSToImage::apply(const float& H,
                              const float& L,
                              const float& S,
                              rgbPixel& pixel) const {
    int i;               // auxiliary for transformation
    float f,t,r,q,v;      // auxiliary for transformation

    if (S==0) {
      pixel.set(static_cast<ubyte>(L*255.0f),
                static_cast<ubyte>(L*255.0f),
                static_cast<ubyte>(L*255.0f),
                0);
    }
    else {
      i = (int) (H*6.0f);
      f = (H*6.0f)-i;
      if (L<=0.5f) {
	v = L*(1.0f+S);
	r = L*(1.0f-S);
	q = L*(1.0f+S*(1.0f-2.0f*f));
	t = 2*L-q;
      }
      else {
	v = L*(1.0f-S)+S;
	r = L*(1.0f+S)-S;
	q = S*(1.0f-L)*(1.0f-2*f)+L;
	t = 2*L-q;
      }

      switch (i) {
	case 0:
	case 6:
	  pixel.set(static_cast<ubyte>(255.0f*v),
                    static_cast<ubyte>(255.0f*t),
                    static_cast<ubyte>(255.0f*r),
                    0);
	  break;
	case 1:
	  pixel.set(static_cast<ubyte>(255.0f*q),
                    static_cast<ubyte>(255.0f*v),
                    static_cast<ubyte>(255.0f*r),
                    0);
	  break;
	case 2:
	  pixel.set(static_cast<ubyte>(255.0f*r),
                    static_cast<ubyte>(255.0f*v),
                    static_cast<ubyte>(255.0f*t),
                    0);
	  break;
	case 3:
	  pixel.set(static_cast<ubyte>(255.0f*r),
                    static_cast<ubyte>(255.0f*q),
                    static_cast<ubyte>(255.0f*v),
                    0);
	  break;
	case 4:
	  pixel.set(static_cast<ubyte>(255.0f*t),
                    static_cast<ubyte>(255.0f*r),
                    static_cast<ubyte>(255.0f*v),
                    0);
	  break;
	case 5:
	  pixel.set(static_cast<ubyte>(255.0f*v),
                    static_cast<ubyte>(255.0f*r),
                    static_cast<ubyte>(255.0f*q),
                    0);
	  break;
      }
    }

    return true;
  };


  // merge 8-bit channels channels
  bool mergeHLSToImage::apply(const ubyte& c1,
                              const ubyte& c2,
                              const ubyte& c3,
                              rgbPixel& pixel) const {
    int i;                 // auxiliary for transformation
    float f,t,r,q,v;       // auxiliary for transformation
    float H, L, S;

    H = float(c1)/255.0f;  // normalize
    L = float(c2)/255.0f;
    S = float(c3)/255.0f;

    if (S==0) {
      pixel.set(static_cast<ubyte>(L*255.0f),
                static_cast<ubyte>(L*255.0f),
                static_cast<ubyte>(L*255.0f),
                0);
    }
    else {
      i = (int) (H*6.0f);
      f = (H*6.0f)-i;
      if (L<=0.5f) {
	v = L*(1.0f+S);
	r = L*(1.0f-S);
	q = L*(1.0f+S*(1.0f-2.0f*f));
	t = 2*L-q;
      }
      else {
	v = L*(1.0f-S)+S;
	r = L*(1.0f+S)-S;
	q = S*(1.0f-L)*(1.0f-2*f)+L;
	t = 2*L-q;
      }
      switch (i) {
	case 0:
	case 6:
	  pixel.set(static_cast<ubyte>(255.0f*v),
                    static_cast<ubyte>(255.0f*t),
                    static_cast<ubyte>(255.0f*r),
                    0);
	  break;
	case 1:
	  pixel.set(static_cast<ubyte>(255.0f*q),
                    static_cast<ubyte>(255.0f*v),
                    static_cast<ubyte>(255.0f*r),
                    0);
	  break;
	case 2:
	  pixel.set(static_cast<ubyte>(255.0f*r),
                    static_cast<ubyte>(255.0f*v),
                    static_cast<ubyte>(255.0f*t),
                    0);
	  break;
	case 3:
	  pixel.set(static_cast<ubyte>(255.0f*r),
                    static_cast<ubyte>(255.0f*q),
                    static_cast<ubyte>(255.0f*v),
                    0);
	  break;
	case 4:
	  pixel.set(static_cast<ubyte>(255.0f*t),
                    static_cast<ubyte>(255.0f*r),
                    static_cast<ubyte>(255.0f*v),
                    0);
	  break;
	case 5:
	  pixel.set(static_cast<ubyte>(255.0f*v),
                    static_cast<ubyte>(255.0f*r),
                    static_cast<ubyte>(255.0f*q),
                    0);
	  break;
      }
    }

    return true;
  };


} // end of namespace
