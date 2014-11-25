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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiArray.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 30.05.2000
 * revisions ..: $Id: ltiArray.cpp,v 1.8 2006/09/01 22:14:08 alvarado Exp $
 */

#include "ltiMacroSymbols.h"
#ifdef _LTI_MSC_6
// a weird warning from MSVC++ is a false alarm!
#pragma warning(disable:4660)
#pragma warning(disable:4800) // warnings for matrix<bool> disabled
#pragma warning(disable:4804) // warnings for matrix<bool> disabled
#endif

#include "ltiArray.h"
#include "ltiArray_template.h"
#include "ltiPoint.h"
#include "ltiRGBPixel.h"
#include "ltiTypes.h"
#include "ltiComplex.h"

namespace lti {
  // explicit instantiations

  template class array<ubyte>;
  template class array<byte>;
  template class array<int>;
  template class array<unsigned int>;
  template class array<float>;
  template class array<double>;
  template class array<rgbPixel>;
  template class array<point>;
  template class array<complex<float> >;
}
