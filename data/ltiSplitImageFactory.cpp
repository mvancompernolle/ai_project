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


/* -----------------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiSplitImageFactory.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 30.10.2002
 * revisions ..: $Id: ltiSplitImageFactory.cpp,v 1.9 2007/01/05 04:38:30 alvarado Exp $
 */

#include "ltiSplitImageFactory.h"
#include "ltiConfig.h"

#include "ltiSplitImageToCIELuv.h"
#include "ltiSplitImageToHSI.h"
#include "ltiSplitImageToHLS.h"
#include "ltiSplitImageToHSV.h"
#include "ltiSplitImageToOCP.h"
#include "ltiSplitImageToRGB.h"
#include "ltiSplitImageTorgI.h"
#include "ltiSplitImageToxyY.h"
#include "ltiSplitImageToXYZ.h"
#include "ltiSplitImageToYIQ.h"
#include "ltiSplitImageToGSC.h"
#include "ltiSplitImageToGSL.h"
#include "ltiSplitImageToYUV.h"
#include "ltiSplitImageToYCbCr.h"
#include "ltiSplitImageToYPbPr.h"

namespace lti {
  // --------------------------------------------------
  // static members
  // --------------------------------------------------
  
  // Static objects to avoid memory leak.
  //
  // (These will be destroyed by the default destruction routines set by the
  // compiler at the end of the application)
  // 
  // Note that the traditional "new splitClass" as component of the splitImages
  // array would create one object in the heap, which would never be deleted.
  // The approach with the static objects is better, since the compiler will
  // take care of their destruction.
  //
  // The method used here is more oriented towards a real "singleton" pattern.
  struct instances {
    // the static objects
    static splitImageToCIELuv CIELuv;
    static splitImageToHSI    HSI;
    static splitImageToHLS    HLS;
    static splitImageToHSV    HSV;
    static splitImageToOCP    OCP;
    static splitImageToRGB    RGB;
    static splitImageTorgI    rgI;
    static splitImageToxyY    xyY;
    static splitImageToXYZ    XYZ;
    static splitImageToYIQ    YIQ;
    static splitImageToGSC    GSC;
    static splitImageToGSL    GSL;
    static splitImageToYUV    YUV;
    static splitImageToYCbCr  YCbCr;
    static splitImageToYPbPr  YPbPr;
  };

  // the real instantiation
  splitImageToCIELuv instances::CIELuv;
  splitImageToHSI    instances::HSI;
  splitImageToHLS    instances::HLS;
  splitImageToHSV    instances::HSV;
  splitImageToOCP    instances::OCP;
  splitImageToRGB    instances::RGB;
  splitImageTorgI    instances::rgI;
  splitImageToxyY    instances::xyY;
  splitImageToXYZ    instances::XYZ;
  splitImageToYIQ    instances::YIQ;
  splitImageToGSC    instances::GSC;
  splitImageToGSL    instances::GSL;
  splitImageToYUV    instances::YUV;
  splitImageToYCbCr  instances::YCbCr;
  splitImageToYPbPr  instances::YPbPr;

  // array to the objects
  const splitImage *const splitImageFactory::splitImages[] = {
    &instances::CIELuv,
    &instances::HSI,
    &instances::HLS,
    &instances::HSV,
    &instances::OCP,
    &instances::RGB,
    &instances::rgI,
    &instances::xyY,
    &instances::XYZ,
    &instances::YIQ,
    &instances::GSC,
    &instances::GSL,
    &instances::YUV,
    &instances::YCbCr,
    &instances::YPbPr,
    0
  };
      
  objectFactory<splitImage> splitImageFactory::factory(splitImages);

  // --------------------------------------------------
  // splitImageFactory
  // --------------------------------------------------

  // default constructor
  splitImageFactory::splitImageFactory() {
  }

  // destructor
  splitImageFactory::~splitImageFactory() {
  }

  // returns the name of this type
  const char*
  splitImageFactory::getTypeName() const {
    return "splitImageFactory";
  }

  splitImage* splitImageFactory::newInstance(const char *name) const {
    return factory.newInstance(name);
  }

}
