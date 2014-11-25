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
 * file .......: ltiGammaDist.cpp
 * authors ....: Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 21.04.99
 * revisions ..: $Id: ltiGammaDist.cpp,v 1.4 2006/02/08 12:22:49 ltilib Exp $
 */

#include <limits>
#include "ltiMath.h"
#include "ltiGammaDist.h"

namespace lti {
  // construction
  gammaDistribution::gammaDistribution(const int theOrder)
    : continuousRandomDistribution() {
    parameters tmpParam;
    tmpParam.order = theOrder;
    setParameters(tmpParam);
  }

  gammaDistribution::gammaDistribution(const parameters& theParams)
    : continuousRandomDistribution() {
    setParameters(theParams);
  }

  // returns the current parameters
  const gammaDistribution::parameters&
  gammaDistribution::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0) throw invalidParametersException(getTypeName());
    return *params;
  }

  // draws a number from the distribution
  // see for example: Press, Vetterling, Teukolsky, Flannery
  //                  Numerical Recipes in C, 2nd edition
  //                  Cambridge University Press, 1992
  //                  p. 292
  double gammaDistribution::draw() const {
    const parameters& tmpParam = getParameters();
    int j;
    double am,e,s,v1,v2,x,y;

    if(tmpParam.order<6) {
      x=1.0;
      for(j=1;j<=tmpParam.order;j++) {
        do {
          v1 = continuousRandomDistribution::draw();
        } while(v1==0.0);
        x *= v1;
      }
      x = -log(x);
    } else {
      do {
        do {
          do {
            do {
              v1=2.0*continuousRandomDistribution::draw()-1.0;
            } while(fabs(v1) < std::numeric_limits<double>::epsilon());
            v2=2.0*continuousRandomDistribution::draw()-1.0;
          } while((v1*v1+v2*v2)>1.0);
          y=v2/v1;
          am=double(tmpParam.order)-1.0;
          s=sqrt(2.0*am+1.0);
          x=s*y+am;
        } while(x<=0.0);
        e=(1.0+y*y)*exp(am*log(x/am)-s*y);
      } while(continuousRandomDistribution::draw()>e);
    }
    return x;
  }
}
