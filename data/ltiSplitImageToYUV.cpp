/*
 * Copyright (C) 2002, 2003, 2004, 2005, 2006
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

 
/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiSplitImageToYUV.cpp
 * authors ....: Volker Schmirgel
 * organization: LTI, RWTH Aachen
 * creation ...: 17.12.2002
 * revisions ..: $Id: ltiSplitImageToYUV.cpp,v 1.5 2006/02/08 11:54:57 ltilib Exp $
 */

#include "ltiSplitImageToYUV.h"

namespace lti {

  // --------------------------------------------------
  // splitImageToYUV
  // --------------------------------------------------

  // default constructor
  splitImageToYUV::splitImageToYUV()
    : splitImage(){
  }

  // copy constructor
  splitImageToYUV::splitImageToYUV(const splitImageToYUV& other) {
    copy(other);
  }

  // destructor
  splitImageToYUV::~splitImageToYUV() {
  }

  // returns the name of this type
  const char* splitImageToYUV::getTypeName() const {
    return "splitImageToYUV";
  }

  // copy member
  splitImageToYUV& splitImageToYUV::copy(const splitImageToYUV& other) {
    splitImage::copy(other);

    return (*this);
  }

  // alias for copy member
  splitImageToYUV&
  splitImageToYUV::operator=(const splitImageToYUV& other) {
    return (copy(other));
  }


  // clone member
  functor* splitImageToYUV::clone() const {
    return (new splitImageToYUV(*this));
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // split image into float channels
  bool splitImageToYUV::apply(const image& img,
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
  bool splitImageToYUV::apply(const image& img,
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
