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
 * file .......: ltiPoissonDist.cpp
 * authors ....: Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 21.04.99
 * revisions ..: $Id: ltiPoissonDist.cpp,v 1.4 2006/02/08 12:39:08 ltilib Exp $
 */

#include "ltiMath.h"
#include "ltiPoissonDist.h"

namespace lti {
  // construction
  poissonDistribution::poissonDistribution(const double mean)
    : continuousRandomDistribution() {
    parameters tmpParam;
    tmpParam.mu = mean;
    setParameters(tmpParam);
    sq = .0;
    alxm = .0;
    g = .0;
    oldm = -1.0;
  }

  poissonDistribution::poissonDistribution(const parameters& theParams)
    : continuousRandomDistribution() {
    setParameters(theParams);
    sq = .0;
    alxm = .0;
    g = .0;
    oldm = -1.0;
  }

  // returns the current parameters
  const poissonDistribution::parameters&
    poissonDistribution::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0) throw invalidParametersException(getTypeName());
    return *params;
  }

  // draws a number from the distribution
  // see for example: Press, Vetterling, Teukolsky, Flannery
  //                  Numerical Recipes in C, 2nd edition
  //                  Cambridge University Press, 1992
  //                  p. 294
  double poissonDistribution::draw() const {
    const parameters& tmpParam = getParameters();
    double em,t,y;

    if(tmpParam.mu<12.0) {
      if(tmpParam.mu!=oldm) {
	oldm=tmpParam.mu;
	g=exp(-tmpParam.mu);
      }
      em=-1;
      t=1.0;
      do {
	em++;
	t*=continuousRandomDistribution::draw();
      }
      while(t>g);
    } else {
      if(tmpParam.mu!=oldm) {
	oldm=tmpParam.mu;
	sq=sqrt(2.0*tmpParam.mu);
	alxm=log(tmpParam.mu);
	g=tmpParam.mu*alxm-lnGamma(tmpParam.mu+1.0);
      }
      do {
	do {
	  y=tan(Pi*continuousRandomDistribution::draw());
	  em=sq*y+tmpParam.mu;
	} while(em<0.0);
	em=floor(em);
	t=0.9*(1.0+y*y)*exp(em*alxm-lnGamma(em+1.0)-g);
      } while(continuousRandomDistribution::draw()>t);
    }
    return em;
  }

  // copy data of "other" functor
  poissonDistribution&
    poissonDistribution::copy(const poissonDistribution& other) {
    continuousRandomDistribution::copy(other);
    sq=other.sq;
    alxm=other.alxm;
    g=other.g;
    oldm=other.g;

    return (*this);
  }
}
