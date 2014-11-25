/*
 * Copyright (C) 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiEdgeDetectorFactory.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 30.10.2002
 * revisions ..: $Id: ltiEdgeDetectorFactory.cpp,v 1.3 2006/02/08 11:02:35 ltilib Exp $
 */

#include "ltiEdgeDetectorFactory.h"
#include "ltiConfig.h"
#include "ltiCannyEdges.h"
#ifdef HAVE_SUSAN
#include "ltiSusanEdges.h"
#endif

namespace lti {
  // --------------------------------------------------
  // static members
  // --------------------------------------------------

  const edgeDetector *const edgeDetectorFactory::edgeDetectors[] = {
    new cannyEdges,
#ifdef HAVE_SUSAN
    new susanEdges,
#endif
    0
  };

  objectFactory<edgeDetector>
  edgeDetectorFactory::factory(edgeDetectors);

  // --------------------------------------------------
  // edgeDetectorFactory
  // --------------------------------------------------

  // default constructor
  edgeDetectorFactory::edgeDetectorFactory() {
  }

  // destructor
  edgeDetectorFactory::~edgeDetectorFactory() {
  }

  // returns the name of this type
  const char*
  edgeDetectorFactory::getTypeName() const {
    return "edgeDetectorFactory";
  }

  edgeDetector* edgeDetectorFactory::newInstance(const char *name) const {
    return factory.newInstance(name);
  }

}
