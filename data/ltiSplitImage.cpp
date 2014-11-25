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
 * file .......: ltiSplitImg.cpp
 * authors ....: Pablo Alvarado, Stefan Syberichs, Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiSplitImage.cpp,v 1.5 2006/02/07 19:41:54 ltilib Exp $
 */

#include "ltiSplitImage.h"
#include "ltiException.h"
#include "ltiConstants.h"

namespace lti {
  //--- SplitImage classes

  // returns the name of this type
  const char* splitImage::getTypeName() const {
    return "splitImage";
  }

  // returns the minimum of three integers
  int splitImage::minimum(const int a,const int b,const int c) const {
    return ((a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c));
  }

  // returns the maximum of three integers
  int splitImage::maximum(const int a,const int b, const int c) const {
    return ((a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c));
  }


  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImage::getFirst(const image& img, channel& c1) const {
    channel c2,c3;
    return apply(img,c1,c2,c3);
  }

  /*
   * Returns the first of the three channels into which the image is split.
   */
  bool splitImage::getFirst(const image& img, channel8& c1) const {
    channel8 c2,c3;
    return apply(img,c1,c2,c3);
  }

  /*
   * Returns the second of the three channels into which the image is split.
   */
  bool splitImage::getSecond(const image& img, channel& c2) const {
    channel c1,c3;
    return apply(img,c1,c2,c3);
  }

  /*
   * Returns the second of the three channels into which the image is split.
   */
  bool splitImage::getSecond(const image& img, channel8& c2) const {
    channel8 c1,c3;
    return apply(img,c1,c2,c3);
  }

  /*
   * Returns the third of the three channels into which the image is split.
   */
  bool splitImage::getThird(const image& img, channel& c3) const {
    channel c1,c2;
    return apply(img,c1,c2,c3);
  }

  /*
   * Returns the third of the three channels into which the image is split.
   */
  bool splitImage::getThird(const image& img, channel8& c3) const {
    channel8 c1,c2;
    return apply(img,c1,c2,c3);
  }


  splitImage& splitImage::copy(const splitImage& other) {
    functor::copy(other);

    return *this;
  }

  splitImage& splitImage::operator=(const splitImage& other) {
    return copy(other);
  }
  

} // end of namespace lti
