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


/*--------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiRegionGraphMeans.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 25.10.2003
 * revisions ..: $Id: ltiRegionGraphMeans.cpp,v 1.3 2006/02/08 11:44:35 ltilib Exp $
 */

#include "ltiMacroSymbols.h"

// only for compilers different than VC++ 6.0 available
#ifndef _LTI_MSC_6

#include "ltiMath.h"
#include "ltiRegionGraphMeans.h"
#include "ltiRegionGraphMeans_template.h"
#include <limits>

namespace lti {
  // --------------------------------------------------
  // regionGraphEdge
  // --------------------------------------------------
  float regionGraphColorMeanDistance::operator()(const node_type& a,
                                                 const node_type& b,
                                                 const int& data) const {
    return static_cast<float>(sqrt(distanceSqr(a.computeMean(),
                                               b.computeMean())));
  }

  float regionGraphScalarMeanDistance::operator()(const node_type& a,
                                                  const node_type& b,
                                                  const int& data) const {
    return abs(a.computeMean()-b.computeMean());
  }

  float regionGraphColorHarisDistance::operator()(const node_type& a,
                                                  const node_type& b,
                                                  const int& data) const {
    const float l1 = static_cast<float>(a.size());
    const float l2 = static_cast<float>(b.size());
    // fixed:
    return static_cast<float>(l1*l2*(distanceSqr(a.computeMean(),
                                                 b.computeMean()))/(l1+l2));

    // this is buggy:
    //return l1*l2*sqrt(distanceSqr(a.computeMean(),b.computeMean()))/(l1+l2);
  }

  float regionGraphScalarHarisDistance::operator()(const node_type& a,
                                                   const node_type& b,
                                                   const int& data) const {
    const float l1 = static_cast<float>(a.size());
    const float l2 = static_cast<float>(b.size());
    return l1*l2*sqr(a.computeMean()-b.computeMean())/(l1+l2);
  }

  // explicit instantiation
  template class regionGraphMeansNode<float>;
  template class regionGraphMeansNode< trgbPixel<float> >;
  
  template class regionGraphColor<regionGraphColorMeanDistance>;
  template class regionGraphColor<regionGraphColorHarisDistance>;
  template class regionGraphGray<regionGraphScalarMeanDistance>;
  template class regionGraphGray<regionGraphScalarHarisDistance>;

}
#endif
