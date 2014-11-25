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
 * file .......: ltiLinearKernels.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 28.10.00
 * revisions ..: $Id: ltiLinearKernels.cpp,v 1.6 2006/09/01 22:13:36 alvarado Exp $
 */

#include "ltiMacroSymbols.h"
#ifdef _LTI_MSC_6
// a weird warning from MSVC++ is a false alarm!
#pragma warning(disable:4660)
#endif

#include "ltiLinearKernels.h"
#include "ltiLinearKernels_template.h"
#include "ltiComplex.h"

namespace lti {

  // specialization for unsigned bytes
  template <>
  kernel2D<ubyte>& kernel2D<ubyte>::outerProduct(const kernel1D<ubyte>& k1,
                                                 const kernel1D<ubyte>& k2) {

    kernel1D<int> ik1;
    kernel1D<int> ik2;
    matrix<int> mk;

    int minNorm = min(k1.getNorm(),k2.getNorm());
    int maxNorm = max(k1.getNorm(),k2.getNorm());

    ik1.castFrom(k1);
    ik2.castFrom(k2);

    mk.outerProduct(static_cast<const vector<int> >(ik1),
                    static_cast<const vector<int> >(ik2));

    mk.divide(minNorm);
    matrix<ubyte>::castFrom(mk);

    offset.y = k1.getOffset();
    offset.x = k2.getOffset();

    norm = maxNorm;
    return (*this);
  }

  // explicit instantiations

  template class kernel1D<ubyte>;
  template class kernel1D<int>;
  template class kernel1D<float>;
  template class kernel1D<double>;



  template class kernel2D<ubyte>;
  template class kernel2D<int>;
  template class kernel2D<float>;
  template class kernel2D<double>;

  template class sepKernel<ubyte>;
  template class sepKernel<int>;
  template class sepKernel<float>;
  template class sepKernel<double>;

  template class kernel1D<complex<float> >;
  template class kernel2D<complex<float> >;
  template class sepKernel<complex<float> >;

}
