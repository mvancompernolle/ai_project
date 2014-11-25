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
 * file .......: ltiHomographyEstimatorFactory.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 18.4.2004
 * revisions ..: $Id: ltiHomographyEstimatorFactory.cpp,v 1.4 2006/02/08 11:18:00 ltilib Exp $
 */

#include "ltiHomographyEstimatorFactory.h"
#include "ltiConfig.h"
#include "ltiHomography8DofEstimator.h"

namespace lti {
  // --------------------------------------------------
  // static members
  // --------------------------------------------------

  const homographyEstimatorBase *const 
  homographyEstimatorFactory::homographyEstimators[] = {
    new homography8DofEstimator,
    0
  };

  objectFactory<homographyEstimatorBase>
  homographyEstimatorFactory::factory(homographyEstimators);

  // --------------------------------------------------
  // homographyEstimatorFactory
  // --------------------------------------------------

  // default constructor
  homographyEstimatorFactory::homographyEstimatorFactory() {
  }

  // destructor
  homographyEstimatorFactory::~homographyEstimatorFactory() {
  }

  // returns the name of this type
  const char*
  homographyEstimatorFactory::getTypeName() const {
    return "homographyEstimatorFactory";
  }

  homographyEstimatorBase* homographyEstimatorFactory
  ::newInstance(const char *name) const {
    return factory.newInstance(name);
  }

  homographyEstimatorBase* homographyEstimatorFactory
  ::newInstance(eTransformEstimatorType& name) const {

    //please add estimators derived from homographyEstimatorBase only
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
