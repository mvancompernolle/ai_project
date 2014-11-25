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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiDrawBase.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 16.01.2003
 * revisions ..: $Id: ltiDrawBase.cpp,v 1.4 2006/02/07 18:47:33 ltilib Exp $
 */

#include "ltiMacroSymbols.h"
#ifdef _LTI_MSC_6
// a weird warning from MSVC++ is a false alarm!
#pragma warning(disable:4660)
#pragma warning(disable:4661)
#endif

#include "ltiDrawBase.h"
#include "ltiDrawBase_template.h"
#include "ltiCMYKColor.h"

namespace lti {

  // explicit instantiations

  template class drawBase<float>;     // channels
  template class drawBase<ubyte>;     // channel8
  template class drawBase<rgbPixel>;  // image
  template class drawBase<double>;    // matrices of doubles
  template class drawBase<int>;       // matrices of integers
  template class drawBase<cmykColor>; // for EPS drawing in CMYK
} // namespace lti
