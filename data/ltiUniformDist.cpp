/*
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiUniformDist.cpp
 * authors ....: Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 19.05.99
 * revisions ..: $Id: ltiUniformDist.cpp,v 1.6 2006/02/08 12:51:29 ltilib Exp $
 */

#include "ltiUniformDist.h"

namespace lti {
  // construction
  uniformDistribution::uniformDistribution(const double& lower,
                                           const double& upper)
    : continuousRandomDistribution() {
    parameters tmpParam;
    tmpParam.lowerLimit = lower;
    tmpParam.upperLimit = upper;
    setParameters(tmpParam);
  }

  uniformDistribution::uniformDistribution(const double& lower,
                                           const double& upper,
                                           const unsigned int seed)
    : continuousRandomDistribution(seed) {
    parameters tmpParam;
    tmpParam.lowerLimit = lower;
    tmpParam.upperLimit = upper;
    setParameters(tmpParam);
  }

  uniformDistribution::uniformDistribution(const parameters& theParams)
    : continuousRandomDistribution() {
    setParameters(theParams);
  }

  // returns the current parameters
  const uniformDistribution::parameters&
  uniformDistribution::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0) throw invalidParametersException(getTypeName());
    return *params;
  }

  /**
   * set functor's parameters.
   * This member makes a copy of <em>theParam</em>: the functor
   * will keep its own copy of the parameters!
   * @return true if successful, false otherwise
   */
  bool uniformDistribution::updateParameters() {
    const parameters& param = getParameters();
    m = (param.upperLimit - param.lowerLimit)/(RAND_MAX+1.0);
    b = param.lowerLimit;
    return true;
  }

  // draws a number from the distribution
  double uniformDistribution::draw() const {
    return (m*rand() + b);
  }
}
