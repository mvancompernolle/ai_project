/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiHessianKernels.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 17.11.2003
 * revisions ..: $Id: ltiHessianKernels.cpp,v 1.3 2006/02/08 11:15:26 ltilib Exp $
 */


#include "ltiHessianKernels.h"

namespace lti {
  // constructor
  hessianKernelXX::hessianKernelXX() : sepKernel<float>() {
    generate();
  }

  // generate filter
  void hessianKernelXX::generate() {
    setNumberOfPairs(1);

    getRowFilter(0).resize(-1,1,0.4f);
    getRowFilter(0).at(0) = -0.8f;

    getColFilter(0).resize(-1,1,0.25f);
    getColFilter(0).at(0) = 0.75f;
  }

  // constructor
  hessianKernelYY::hessianKernelYY() : sepKernel<float>() {
    generate();
  }

  // generate filter
  void hessianKernelYY::generate() {
    setNumberOfPairs(1);

    getColFilter(0).resize(-1,1,0.4f);
    getColFilter(0).at(0) = -0.8f;

    getRowFilter(0).resize(-1,1,0.25f);
    getRowFilter(0).at(0) = 0.75f;
  }

  // constructor
  hessianKernelXY::hessianKernelXY() : sepKernel<float>() {
    generate();
  }

  // generate filter
  void hessianKernelXY::generate() {
    setNumberOfPairs(1);

    getRowFilter(0).resize(-1,1,0.0f);
    getRowFilter(0).at(-1) = 0.5f;
    getRowFilter(0).at( 1) =-0.5f;

    getColFilter(0).resize(-1,1,0.0f);
    getColFilter(0).at(-1) = 0.25f;
    getColFilter(0).at( 1) =-0.25f;
  }
}
