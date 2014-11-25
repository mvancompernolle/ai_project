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
 * file .......: ltiMath.cpp
 * authors ....: Thomas Rusert, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 28.04.99
 * revisions ..: $Id: ltiMath.cpp,v 1.4 2006/02/08 12:33:35 ltilib Exp $
 */

#include "ltiMath.h"

namespace lti {

  double factorial(const int x) {
    static const int maxLut = 34;
    static const double lut[] = {1.0,        // 0!
                                 1.0,        // 1!
                                 2.0,        // 2!
                                 6.0,        // 3!
                                 24.0,       // 4!
                                 120.0,      // 5!
                                 720.0,      // 6!
                                 5040.0,     // 7!
                                 40320.0,    // 8!
                                 362880.0,   // 9!
                                 3628800.0,  // 10!
                                 39916800.0, // 11!
                                 479001600.0, // 12!
                                 6227020800.0, // 13!
                                 87178291200.0, // 14!
                                 1307674368000.0, // 15!
                                 20922789888000.0, // 16!
                                 355687428096000.0, // 17!
                                 6402373705728000.0, // 18!
                                 121645100408832000.0, // 19!
                                 2432902008176640000.0, // 20!
                                 51090942171709440000.0, // 21!
                                 1124000727777607680000.0, // 22!
                                 25852016738884976640000.0, // 23!
                                 620448401733239439360000.0, // 24!
                                 15511210043330985984000000.0, // 25!
                                 403291461126605635584000000.0, // 26!
                                 10888869450418352160768000000.0, // 27!
                                 304888344611713860501504000000.0, // 28!
                                 8841761993739701954543616000000.0, // 29!
                                 265252859812191058636308480000000.0, // 30!
                                 8222838654177922817725562880000000.0, // 31!
                                 263130836933693530167218012160000000.0, // 32!
                                 8683317618811886495518194401280000000.0 // 33!
    };

    if (x < 0) {
      return NaN;
    }
    if (x>=maxLut) {
      return exp(lnGamma(static_cast<double>(x+1)));
    }
    return lut[static_cast<int>(x)];
  }

  double lnFactorial(const int x) {
    static const int maxTableSize = 128;
    static double lut[maxTableSize]; // static arrays are automatically 
                                     // initialized with zero    
    if (x < 0) {
      return NaN;
    }
    if (x <= 1) {
      return 0.0;
    }
    if (x < maxTableSize) {
      return (lut[static_cast<int>(x)] != 0) ? lut[static_cast<int>(x)] :
        (lut[static_cast<int>(x)] = lnGamma(double(static_cast<int>(x)+1)));
    }
    return lnGamma(double(static_cast<int>(x)+1));
  }
                      
  double binomial(const int n,const int k) {
    return floor(0.5 + exp(lnFactorial(n)-lnFactorial(k)-lnFactorial(n-k)));
  }

  //the coefficients for the incomplete beta function
  double betacf(const double& a, const double& b, const double& x) {

    // ugly, but do not want to make betai a class on its ow
    const int maxIterations ( 100 );

    // A continued fraction representation of the beta function
    const double epsilon ( std::numeric_limits<double>::epsilon() );
    
    double am ( 1. );
    double bm ( 1. );
    
    const double qab ( a + b  );
    const double qap ( a + 1. );
    const double qam ( a - 1. );
    double bz = 1. - qab*x/qap;

    double az ( 1. );
    double aold ( 0. );

    int i ( 1 );     
    //while ( i<maxIterations && abs(az-aold)>=epsilon*abs(az) ) {
    while ( i<maxIterations && abs(az-aold)>epsilon ) {
      const double em ( static_cast<double>(i) );
      const double tem ( em+em );

      double d ( em*(b-em)*x / ( (qam+tem)* (a+tem) ) );
      const double ap ( az+d*am );
      const double bp ( bz+d*bm );
      d = -(a+em)*(qab+em)*x/((a+tem)*(qap+tem));
      const double app ( ap+d*az );
      const double bpp ( bp+d*bz );

      aold = az;
      if ( abs(bpp) < epsilon ) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      am = ap/bpp;
      bm = bp/bpp;
      az = app/bpp;

      bz = 1.;
      i++;
    }

    return az;
  }

}
