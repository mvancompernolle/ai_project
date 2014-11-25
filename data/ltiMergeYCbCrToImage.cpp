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
 * file .......: ltiSplitImageToYCbCr.h
 * authors ....: Pablo Alvarado
 * organization: Electronics Department, ITCR
 * creation ...: 04.01.2007
 * revisions ..: $Id: ltiMergeYCbCrToImage.cpp,v 1.1 2007/01/05 03:29:42 alvarado Exp $
 */

// merge YCbCr channels to image

#include "ltiMergeYCbCrToImage.h"


namespace lti {
  /*
   * look up tables to accelerate conversion YUV -> RGB
   * only used in the quick-mode
   */
  const int* mergeYCbCrToImage::lutY = 0;
  const int* mergeYCbCrToImage::lutUg = 0;
  const int* mergeYCbCrToImage::lutUb = 0;
  const int* mergeYCbCrToImage::lutVg = 0;
  const int* mergeYCbCrToImage::lutVr = 0;

  // Constructor  
  mergeYCbCrToImage::mergeYCbCrToImage(void) {
    initializeLUTs();
  }

  // destructor
  mergeYCbCrToImage::~mergeYCbCrToImage() {

  }

  // return the name of this type
  const char* mergeYCbCrToImage::getTypeName() const {
    return "mergeYCbCrToImage";
  }

  // copy member
  mergeYCbCrToImage& mergeYCbCrToImage::copy(const mergeYCbCrToImage& other) {
    mergeImage::copy(other);
    
    return (*this);
  }

  // alias for copy member
  mergeYCbCrToImage&
    mergeYCbCrToImage::operator=(const mergeYCbCrToImage& other) {
    return (copy(other));
  }

  // returns a pointer to a clone of the functor.
  functor* mergeYCbCrToImage::clone() const {
    return (new mergeYCbCrToImage(*this));
  }




  // merge float channels
  bool mergeYCbCrToImage::apply(const matrix<float>& c1,
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
  bool mergeYCbCrToImage::apply(const channel8& c1,
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

  void mergeYCbCrToImage::initializeLUTs() {
    if (lutUg == 0) {
      int i;

      // The following code is the standard conversion:
      //
      //     const int Y = (static_cast<int>(c1)- 16)*76309 + 32768;
      //     const int U = (static_cast<int>(c2)-128);
      //     const int V = (static_cast<int>(c3)-128);
      
      //     pixel.set(clip((Y              + 104597 * V)>>16),
      //               clip((Y -  25675 * U -  53279 * V)>>16),
      //               clip((Y + 132201 * U             )>>16),
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
        lutYTmp[i]  = (i- 16)*76309 + 32768; // partial computation of Y
        lutVrTmp[i] = (i-128)*104597;        // for Red computation
        lutUgTmp[i] = (i-128)*(-25675);      // for Green computation
        lutVgTmp[i] = (i-128)*(-53279);      // for Green computation
        lutUbTmp[i] = (i-128)*132201;        // for blue computation
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
