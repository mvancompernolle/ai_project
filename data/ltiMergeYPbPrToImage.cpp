/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007
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

 
/*--------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiMergeYPbPrToImage.h
 * authors ....: Pablo Alvarado
 * organization: Electronics Department, ITCR
 * creation ...: 04.01.2007
 * revisions ..: $Id: ltiMergeYPbPrToImage.cpp,v 1.1 2007/01/05 04:38:04 alvarado Exp $
 */

// merge YPbPr channels to image

#include "ltiMergeYPbPrToImage.h"


namespace lti {
  /*
   * look up tables to accelerate conversion YUV -> RGB
   * only used in the quick-mode
   */
  const int* mergeYPbPrToImage::lutY = 0;
  const int* mergeYPbPrToImage::lutUg = 0;
  const int* mergeYPbPrToImage::lutUb = 0;
  const int* mergeYPbPrToImage::lutVg = 0;
  const int* mergeYPbPrToImage::lutVr = 0;

  // Constructor  
  mergeYPbPrToImage::mergeYPbPrToImage(void) {
    initializeLUTs();
  }

  // destructor
  mergeYPbPrToImage::~mergeYPbPrToImage() {

  }

  // return the name of this type
  const char* mergeYPbPrToImage::getTypeName() const {
    return "mergeYPbPrToImage";
  }

  // copy member
  mergeYPbPrToImage& mergeYPbPrToImage::copy(const mergeYPbPrToImage& other) {
    mergeImage::copy(other);
    
    return (*this);
  }

  // alias for copy member
  mergeYPbPrToImage&
    mergeYPbPrToImage::operator=(const mergeYPbPrToImage& other) {
    return (copy(other));
  }

  // returns a pointer to a clone of the functor.
  functor* mergeYPbPrToImage::clone() const {
    return (new mergeYPbPrToImage(*this));
  }




  // merge float channels
  bool mergeYPbPrToImage::apply(const matrix<float>& c1,
                                const matrix<float>& c2,
                                const matrix<float>& c3,
                                image& img) const {
 
    const int xSize=c1.columns();
    const int ySize=c1.rows();
    if ((c2.columns()!=xSize)||
	(c2.rows()!=ySize)||
	(c3.columns()!=xSize)||
	(c3.rows()!=ySize)) {
      setStatusString("sizes of channels do not match");
      return false;
    }

    img.resize(ySize,xSize,rgbPixel(),false,false);

    // Y = c1
    // U = c2
    // V = c3

    const int numRows (img.rows());
    const int numColumns (img.columns());
    int i;
    for(i=0; i<numRows; i++) {
      vector<rgbPixel>& imgVct = img.getRow(i);
      const vector<float>& c1Vct = c1.getRow(i);
      const vector<float>& c2Vct = c2.getRow(i);
      const vector<float>& c3Vct = c3.getRow(i);

      int j;
      for(j=0; j<numColumns; j++) {
	// take pixel at position j
	apply(c1Vct.at(j), c2Vct.at(j), c3Vct.at(j), imgVct.at(j));

      } // loopcd
    }
    return true;
  }



  // merge 8-bit-channels
  bool mergeYPbPrToImage::apply(const channel8& c1,
                                const channel8& c2,
                                const channel8& c3,
                                image& img) const {
 
    //point p;            // coordinates
    //rgbPixel pix;       // pixel structure

    const int xSize=c1.columns();
    const int ySize=c1.rows();
    if ((c2.columns()!=xSize) || (c2.rows()!=ySize) ||
	(c3.columns()!=xSize) || (c3.rows()!=ySize)) {
      setStatusString("sizes of channels do not match");
      return false;
    }

    img.resize(ySize,xSize,rgbPixel(),false,false);

    // Y = c1
    // U = c2
    // V = c3

    const int numRows (img.rows());
    const int numColumns (img.columns());
    int i;
    for(i=0; i<numRows; i++) {
      vector<rgbPixel>& imgVct = img.getRow(i);
      const vector<ubyte>& c1Vct = c1.getRow(i);
      const vector<ubyte>& c2Vct = c2.getRow(i);
      const vector<ubyte>& c3Vct = c3.getRow(i);

      int j;
      for(j=0; j<numColumns; j++) {
	// take pixel at position j
	apply(c1Vct.at(j), c2Vct.at(j), c3Vct.at(j), imgVct.at(j));

      } // loopcd
    }

    return true;
  }

  void mergeYPbPrToImage::initializeLUTs() {
    if (lutUg == 0) {
      int i;

      // The following code is the standard conversion:
      //
      //     const int Y =  static_cast<int>(c1)*65536 + 32768;
      //     const int U = (static_cast<int>(c2)-128);
      //     const int V = (static_cast<int>(c3)-128);
      
      //     pixel.set(clip((Y              +  91881 * V)>>16),
      //               clip((Y -  22553 * U -  46802 * V)>>16),
      //               clip((Y + 116130 * U             )>>16),
      //               0);


      /*
       * look up tables to accelerate conversion YUV -> RGB
       */
      static int lutYTmp[256];
      static int lutUgTmp[256];
      static int lutUbTmp[256];
      static int lutVgTmp[256];
      static int lutVrTmp[256];

      for (i=0;i<256;++i) {
        lutYTmp[i]  =  i*65536 + 32768; // partial computation of Y
        lutVrTmp[i] = (i-128)*91881;        // for Red computation
        lutUgTmp[i] = (i-128)*(-22553);      // for Green computation
        lutVgTmp[i] = (i-128)*(-46802);      // for Green computation
        lutUbTmp[i] = (i-128)*116130;        // for blue computation
      }

      // Give external access to internal static attributes.
      lutY  = lutYTmp;
      lutUg = lutUgTmp;
      lutUb = lutUbTmp;
      lutVg = lutVgTmp;
      lutVr = lutVrTmp;

    }
  }
}
