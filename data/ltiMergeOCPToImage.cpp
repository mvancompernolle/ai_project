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
 * file .......: ltiMergeOCPToImage.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiMergeOCPToImage.cpp,v 1.4 2006/02/08 11:29:18 ltilib Exp $
 */

#include "ltiMergeOCPToImage.h"

namespace lti {

  // ------------------------------------------
  // merge OCP channels to image
  // ------------------------------------------

  // return the name of this type
  const char* mergeOCPToImage::getTypeName() const {
    return "mergeOCPToImage";
  }

  // returns a pointer to a clone of the functor.
  functor* mergeOCPToImage::clone() const {
    return (new mergeOCPToImage(*this));
  }

  // merge float channels
  bool mergeOCPToImage::apply(const matrix<float>& c1,
                              const matrix<float>& c2,
                              const matrix<float>& c3,
                              image& img) const {

    point p;              // coordinates
    float r,g,b;          // unnormed RGB channels
    float RG, BY, WB;     // opponent colour channels

    if ((c1.size() != c2.size()) || (c1.size() != c3.size())) {
      setStatusString("sizes of channels do not match");
      return false;
    }

    img.resize(c1.size(),rgbPixel(),false,false);

    for (p.y=0;p.y<img.rows();p.y++) {
      for (p.x=0;p.x<img.columns();p.x++) {

	RG = c1.at(p);
	BY = c2.at(p);
	WB = c3.at(p);

        b = BY*0.666666666667f;
        //
	r = WB + RG - b;
	g = WB - RG - b;
	b = WB + BY*1.3333333333333f;

	// truncate r,g and b if the value is not in intervall [0..1]
	// can happen due to rounding errors in split operation
	if (r<0.0f) {
          r=0.0f;
        } else if (r>1.0f) {
          r=1.0f;
        }

	if (g<0.0f) {
          g=0.0f;
        } else if (g>1.0f) {
          g=1.0f;
        }

	if (b<0.0f) {
          b=0.0f;
        } else if (b>1.0f) {
          b=1.0f;
        }

	img.at(p).set(static_cast<ubyte>(255.0f*r),
                      static_cast<ubyte>(255.0f*g),
                      static_cast<ubyte>(255.0f*b),
                      0);
      }
    }

    return true;
  };

  // merge 8-bit channels
  bool mergeOCPToImage::apply(const channel8& c1,
                              const channel8& c2,
                              const channel8& c3,
                              image& img) const {

    point p;              // coordinates
    float r,g,b;          // unnormed RGB channels
    float RG, BY, WB;     // opponent colour channels
    
    if ((c1.size() != c2.size()) || (c1.size() != c3.size())) {
      setStatusString("sizes of channels do not match");
      return false;
    }
    
    img.resize(c1.size(),rgbPixel(),false,false);

    for (p.y=0;p.y<img.rows();p.y++) {
      for (p.x=0;p.x<img.columns();p.x++) {

	RG = static_cast<float>(c1.at(p)) - 127.5f;
	BY = static_cast<float>(c2.at(p)) - 127.5f;
	WB = static_cast<float>(c3.at(p)) - 127.5f;
        
        b = BY*0.66666666666667f;
        //
	r = WB + RG - b;
	g = WB - RG - b;
	b = WB + BY*1.3333333333333f;

	// truncate r,g and b if the value is not in intervall [0..1]
	// can happen due to rounding errors in split operation
	if (r<0.0f) {
          r=0.0f;
        } else if (r>255.0f) {
          r=255.0f;
        }

	if (g<0.0f) {
          g=0.0f;
        } else if (g>255.0f) {
          g=255.0f;
        }

	if (b<0.0f) {
          b=0.0f;
        } else if (b>255.0f) {
          b=255.0f;
        }

	img.at(p).set(static_cast<ubyte>(r),
                      static_cast<ubyte>(g),
                      static_cast<ubyte>(b),
                      0);
      }
    }

    return true;
  };

  // merge float values
  bool mergeOCPToImage::apply(const float& RG,
                              const float& BY,
                              const float& WB,
                              rgbPixel& pixel) const {

    float r,g,b;          // unnormed RGB channels
    
    b = BY*0.666666666667f;
    //
    r = WB + RG - b;
    g = WB - RG - b;
    b = WB + BY*1.3333333333333f;
    
    // truncate r,g and b if the value is not in intervall [0..1]
    // can happen due to rounding errors in split operation
    if (r<0.0f) {
      r=0.0f;
    } else if (r>1.0f) {
      r=1.0f;
    }
    
    if (g<0.0f) {
      g=0.0f;
    } else if (g>1.0f) {
      g=1.0f;
    }
    
    if (b<0.0f) {
      b=0.0f;
    } else if (b>1.0f) {
      b=1.0f;
    }
    
    pixel.set(static_cast<ubyte>(255.0f*r),
              static_cast<ubyte>(255.0f*g),
              static_cast<ubyte>(255.0f*b),
              0);

    return true;
  };

  // merge 8-bit values
  bool mergeOCPToImage::apply(const ubyte& c1,
                              const ubyte& c2,
                              const ubyte& c3,
                              rgbPixel& pixel) const {

    float r,g,b;          // unnormed RGB channels
    float RG, BY, WB;     // opponent colour channels
    
    RG = static_cast<float>(c1) - 127.5f;
    BY = static_cast<float>(c2) - 127.5f;
    WB = static_cast<float>(c3) - 127.5f;
        
    b = BY*0.66666666666667f;
    //
    r = WB + RG - b;
    g = WB - RG - b;
    b = WB + BY*1.3333333333333f;

    // truncate r,g and b if the value is not in intervall [0..1]
    // can happen due to rounding errors in split operation
    if (r<0.0f) {
      r=0.0f;
    } else if (r>255.0f) {
      r=255.0f;
    }
    
    if (g<0.0f) {
      g=0.0f;
    } else if (g>255.0f) {
      g=255.0f;
    }
    
    if (b<0.0f) {
      b=0.0f;
    } else if (b>255.0f) {
      b=255.0f;
    }
    
    pixel.set(static_cast<ubyte>(r),
              static_cast<ubyte>(g),
              static_cast<ubyte>(b),
              0);

    return true;
  };

} // end of namespace
