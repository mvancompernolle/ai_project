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
 * file .......: ltiGaussDist.cpp
 * authors ....: Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 21.04.99
 * revisions ..: $Id: ltiGaussDist.cpp,v 1.4 2006/02/08 12:23:09 ltilib Exp $
 */

#include "ltiMath.h"
#include "ltiGaussDist.h"

namespace lti {
  // construction
  gaussianDistribution::gaussianDistribution(const double mean,
                                             const double stdDeviation)
    : continuousRandomDistribution() {
    parameters tmpParam;
    tmpParam.sigma = stdDeviation;
    tmpParam.mu = mean;
    setParameters(tmpParam);
    phase = 0;
    V1 = 0.0;
    V2 = 0.0;
    S = 0.0;
  }

  gaussianDistribution::gaussianDistribution(const parameters& theParams)
    : continuousRandomDistribution() {
    setParameters(theParams);
    phase = 0;
    V1 = 0.0;
    V2 = 0.0;
    S = 0.0;
  }

  // returns the current parameters
  const gaussianDistribution::parameters&
  gaussianDistribution::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0) throw invalidParametersException(getTypeName());
    return *params;
  }

  // draws a number from the distribution
  double gaussianDistribution::draw() const {
    const parameters& tmpParam = getParameters();
    return draw(tmpParam.mu,tmpParam.sigma);
  }

  // draws a number from the distribution
  // see for example: Press, Vetterling, Teukolsky, Flannery
  //                  Numerical Recipes in C, 2nd edition
  //                  Cambridge University Press, 1992
  //                  p. 289
  double gaussianDistribution::draw(const double& average,
                                    const double& stdDeviation) const {

    double X;

    if(phase == 0) {
      do {
        V1 = 2.0 * continuousRandomDistribution::draw() - 1.0;
        V2 = 2.0 * continuousRandomDistribution::draw() - 1.0;
        S = V1 * V1 + V2 * V2;
      } while(S >= 1.0 || S == 0.0);

      X = V1 * sqrt(-2.0 * log(S) / S);
    } else {
      X = V2 * sqrt(-2.0 * log(S) / S);
    }

    phase = 1 - phase;

    return (average + stdDeviation * X);
  }

  /*
   * return a gaussian distributed random number.
   * @param average mean value of the distribution
   * @param stdDeviation std. deviation of the distribution
   * @param result value from the distribution
   * @return true if successful, false otherwise.
   */
  bool gaussianDistribution::apply(const double& average,
                                   const double& stdDeviation,
                                   double& result) const {
    result = draw(average,stdDeviation);
    return true;
  }

  // copy data of "other" functor
  gaussianDistribution&
  gaussianDistribution::copy(const gaussianDistribution& other) {

    continuousRandomDistribution::copy(other);

    setParameters(other.getParameters());
    V1=other.V1;
    V2=other.V2;
    S=other.S;
    phase=other.phase;

    return (*this);
  }
}
