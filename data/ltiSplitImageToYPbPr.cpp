/*
 * Copyright (C) 2002, 2003, 2004, 2005, 2006,2007
 * Department of Electronics, ITCR, Costa Rica
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

 
/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiSplitImageToYPbPr.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 04.01.2007
 * revisions ..: $Id: ltiSplitImageToYPbPr.cpp,v 1.1 2007/01/05 04:38:04 alvarado Exp $
 */

#include "ltiSplitImageToYPbPr.h"

namespace lti {

  // --------------------------------------------------
  // splitImageToYPbPr
  // --------------------------------------------------

  // default constructor
  splitImageToYPbPr::splitImageToYPbPr()
    : splitImage(){
  }

  // copy constructor
  splitImageToYPbPr::splitImageToYPbPr(const splitImageToYPbPr& other) {
    copy(other);
  }

  // destructor
  splitImageToYPbPr::~splitImageToYPbPr() {
  }

  // returns the name of this type
  const char* splitImageToYPbPr::getTypeName() const {
    return "splitImageToYPbPr";
  }

  // copy member
  splitImageToYPbPr& splitImageToYPbPr::copy(const splitImageToYPbPr& other) {
    splitImage::copy(other);

    return (*this);
  }

  // alias for copy member
  splitImageToYPbPr&
  splitImageToYPbPr::operator=(const splitImageToYPbPr& other) {
    return (copy(other));
  }


  // clone member
  functor* splitImageToYPbPr::clone() const {
    return (new splitImageToYPbPr(*this));
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // split image into float channels
  bool splitImageToYPbPr::apply(const image& img,
                                channel& c1,
                                channel& c2,
                                channel& c3) const {

    // make the channels size of source image...
    // Todo: don't initialize
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    const int numRows (img.rows());
    const int numColumns (img.columns());
    int i;

    for(i=0; i<numRows; i++) {
      const vector<rgbPixel>& srcVct = img.getRow(i);
      vector<float>& c1Vct = c1.getRow(i);
      vector<float>& c2Vct = c2.getRow(i);
      vector<float>& c3Vct = c3.getRow(i);

      int j;
      for(j=0; j<numColumns; j++) {
	// take pixel at position j
	rgbPixel pix ( srcVct.at(j) );  // single Pixel Element in RGB-values...

	apply(pix, c1Vct.at(j), c2Vct.at(j), c3Vct.at(j));

      } // loopcd
    }
    return true;
  }


  // split image into 8-bit channels
  // N.B.: when casting the transformation result to unsigned shorts
  // (8-bit channel) major rounding errors will occur.
  // As a result, the merging operation might produce negative
  // values or values > 1,  which are truncated subsequently.
  // When accurate Y, U, and V channels are required, prefer float channels!
  bool splitImageToYPbPr::apply(const image& img,
                                channel8& c1,
                                channel8& c2,
                                channel8& c3) const {
    
    // make the channels size of source image...
    c1.resize(img.rows(),img.columns(),0,false,false);
    c2.resize(img.rows(),img.columns(),0,false,false);
    c3.resize(img.rows(),img.columns(),0,false,false);

    const int numRows (img.rows());
    const int numColumns (img.columns());
    int i;

    for(i=0; i<numRows; i++) {
      const vector<rgbPixel>& srcVct = img.getRow(i);
      vector<ubyte>& c1Vct = c1.getRow(i);
      vector<ubyte>& c2Vct = c2.getRow(i);
      vector<ubyte>& c3Vct = c3.getRow(i);

      int j;
      for(j=0; j<numColumns; j++) {
	// take pixel at position j
	rgbPixel pix ( srcVct.at(j) );  // single Pixel Element in RGB-values...

	apply(pix, c1Vct.at(j), c2Vct.at(j), c3Vct.at(j));

      } // loopcd
    }
    return true;
  }
}
