/*
 * Copyright (C) 2004, 2005, 2006
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
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiTransformEstimatorFactory.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 18.4.2004
 * revisions ..: $Id: ltiTransformEstimatorFactory.cpp,v 1.4 2006/02/08 11:57:40 ltilib Exp $
 */

#include "ltiTransformEstimatorFactory.h"
#include "ltiConfig.h"
#include "ltiHomography8DofEstimator.h"

namespace lti {
  // --------------------------------------------------
  // static members
  // --------------------------------------------------

  struct instancesTransformEstimators {
    static homography8DofEstimator homoEstimator;
  };

  homography8DofEstimator instancesTransformEstimators::homoEstimator;

  const transformEstimator *const 
  transformEstimatorFactory::transformEstimators[] = {
    /*
    new homography8DofEstimator,
    */
    &instancesTransformEstimators::homoEstimator,
    0
  };

  objectFactory<transformEstimator>
  transformEstimatorFactory::factory(transformEstimators);

  // --------------------------------------------------
  // transformEstimatorFactory
  // --------------------------------------------------

  // default constructor
  transformEstimatorFactory::transformEstimatorFactory() {
  }

  // destructor
  transformEstimatorFactory::~transformEstimatorFactory() {
  }

  // returns the name of this type
  const char*
  transformEstimatorFactory::getTypeName() const {
    return "transformEstimatorFactory";
  }

  transformEstimator* transformEstimatorFactory
  ::newInstance(const char *name) const {
    return factory.newInstance(name);
  }

  transformEstimator* transformEstimatorFactory
  ::newInstance(eTransformEstimatorType name) const {

    std::string str;
    switch(name) {
    case Homography8DofEstimator:
      str = "lti::homography8DofEstimator";
      break;
    case Homography9DofEstimator:
      str = "lti::homography9DofEstimator";
      break;
    case AffineEstimator:
      str = "lti::affineEstimator";
      break;
    case SimilarityEstimator:
      str = "lti::similarityEstimator";
      break;
    default:
      str = "NoTransformEstimator";
      break;
    }
    return newInstance(str);
  }
  
}
