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
 * file .......: ltiContinuousRandDist.cpp
 * authors ....: Thomas Rusert, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 21.04.99
 * revisions ..: $Id: ltiContinuousRandDist.cpp,v 1.6 2006/02/08 12:14:55 ltilib Exp $
 */

#include <cstdlib>
#include <ctime>
#include "ltiContinuousRandDist.h"

namespace lti  {

  // default constructor: initialization using system time

  continuousRandomDistribution::continuousRandomDistribution(bool reInit)
    : randomDistribution(reInit) {
  }

  // constructor: user-defined initialization
  continuousRandomDistribution::continuousRandomDistribution(const unsigned int theValue) 
    : randomDistribution(theValue) {
  }

  // destructor
  continuousRandomDistribution::~continuousRandomDistribution() {
  }

  continuousRandomDistribution&
  continuousRandomDistribution::copy(const continuousRandomDistribution& other)
  {
    // nothing to be copied!

    return (*this);
  }

}
