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


/*-------------------------------------------------------------------------
 * project ....: lti library
 * file .......: ltiPolygonPoints.cpp
 * classes ....: lti::PolygonPoints
 * description.: class for description of an objects contour
 *              as a linear polygon
 * authors ....: Ruediger Weiler
 * organization: LTI, RWTH Aachen
 * creation ...: 06.12.2000
 * revisions ..: $Id: ltiPolygonPoints.cpp,v 1.5 2006/02/08 11:39:26 ltilib Exp $
 */

#include "ltiMacroSymbols.h"
#ifdef _LTI_MSC_6
// a weird warning from MSVC++ is a false alarm!
#pragma warning(disable:4660)
#endif

#include "ltiObject.h"
#include "ltiPolygonPoints.h"
#include "ltiPolygonPoints_template.h"

namespace lti {
  // explicit instantiations
  template class tpolygonPoints<float>;
  template class tpolygonPoints<double>;
  template class tpolygonPoints<int>;
} // end of namespace lti::
