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
 * file .......: ltiRandDist.cpp
 * authors ....: Thomas Rusert, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 21.04.99
 * revisions ..: $Id: ltiRandDist.cpp,v 1.5 2006/02/08 12:41:21 ltilib Exp $
 */

#include <cstdlib>
#include <ctime>
#include "ltiRandDist.h"

namespace lti  {

  bool randomDistribution::initialized = false;

  // default constructor: initialization using system time

  randomDistribution::randomDistribution(bool reInit) {
    if(reInit||(!initialized)) {
      init();
      initialized = true;
    }
  }

  // constructor: user-defined initialization

  randomDistribution::randomDistribution(const unsigned int theValue) {
    init(theValue);
    initialized = true;
  }

  // destructor
  randomDistribution::~randomDistribution() {
  }

  // re-initializes the random number generator using system time
  void randomDistribution::init() const {
    srand((unsigned)time(0));
  }

  // re-initializes the random number generator using the given value
  void randomDistribution::init(const unsigned int theValue) const {
    srand(theValue);
  }

  randomDistribution&
    randomDistribution::copy(const randomDistribution& other) {
    // nothing to be copied!

    return (*this);
  }

}
