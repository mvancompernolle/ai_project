/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiQmfKernels.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 28.10.00
 * revisions ..: $Id: ltiQmfKernels.cpp,v 1.4 2006/02/08 11:42:39 ltilib Exp $
 */

#include "ltiQmfKernels.h"
#include "ltiMath.h"

namespace lti {

  haarKernel::haarKernel()
    : kernel1D<float>(-1,0,float(1.0/sqrt(2.0))) {
  }

  daubechies16Kernel::daubechies16Kernel()
    : kernel1D<float>(-15,0,0.0f) {
    at(  0) = 0.054416f;
    at( -1) = 0.312872f;
    at( -2) = 0.675631f;
    at( -3) = 0.585355f;
    at( -4) =-0.015829f;
    at( -5) =-0.284016f;
    at( -6) = 0.000472f;
    at( -7) = 0.128747f;
    at( -8) =-0.017369f;
    at( -9) =-0.044088f;
    at(-10) = 0.013981f;
    at(-11) = 0.008746f;
    at(-12) =-0.004870f;
    at(-13) =-0.000392f;
    at(-14) = 0.000675f;
    at(-15) =-0.000117f;
  }

  battleLemarieKernel::battleLemarieKernel()
    : kernel1D<float>(-15,15,0.0f) {

    at( 0) = 0.766130f;
    at( 1) = 0.433923f;
    at( 2) =-0.050202f;
    at( 3) =-0.110037f;
    at( 4) = 0.032081f;
    at( 5) = 0.042068f;
    at( 6) =-0.017176f;
    at( 7) =-0.017982f;
    at( 8) = 0.008685f;
    at( 9) = 0.008201f;
    at(10) =-0.004354f;
    at(11) =-0.003882f;
    at(12) = 0.002187f;
    at(13) = 0.001882f;
    at(14) =-0.001104f;
    at(15) =-0.000927f;

    for (int i = -15;i<0;i++) {
      at(i) = at(-i);
    }
  }

  tap9Symmetric::tap9Symmetric()
    : kernel1D<float>(-4,4,0.0f) {

    at(4) = 0.02807382f;
    at(3) =-0.060944743f;
    at(2) =-0.073386624f;
    at(1) = 0.41472545f;
    at(0) = 0.7973934f;

    for (int i=-4;i<0;i++)
      at(i) = at(-i);
  }

}
