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
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiDraw2DDistribution.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 20.12.2002
 * revisions ..: $Id: ltiDraw2DDistribution.cpp,v 1.5 2006/02/07 18:46:21 ltilib Exp $
 */

#include "ltiMacroSymbols.h"

#ifdef _LTI_MSC_6
// a weird warning from MSVC++ is a false alarm!
#pragma warning(disable:4660)
#pragma warning(disable:4661)
#endif

#include "ltiDraw2DDistribution.h"
#include "ltiDraw2DDistribution_template.h"

namespace lti {

  // specializations

  template<>
  lti::rgbPixel draw2DDistribution<lti::rgbPixel>::parameters::getDefaultAxisColor() {
    return rgbPixel(255,255,255);
  }

  template<>
  float draw2DDistribution<float>::parameters::getDefaultAxisColor() {
    return 1.0f;
  }

  template<>
  double draw2DDistribution<double>::parameters::getDefaultAxisColor() {
    return 1.0;
  }

  template<>
  lti::ubyte draw2DDistribution<lti::ubyte>::parameters::getDefaultAxisColor() {
    return 255;
  }

  template<>
  int draw2DDistribution<int>::parameters::getDefaultAxisColor() {
    return 32768;
  }




  // explicit instantiations
  template class draw2DDistribution<float>;     // channels
  template class draw2DDistribution<ubyte>;     // channel8
  template class draw2DDistribution<rgbPixel>;  // image
  template class draw2DDistribution<double>;    // matrices of doubles
  template class draw2DDistribution<int>;       // matrices of integers
} // namespace lti
