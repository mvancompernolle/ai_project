/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiClassifierFactory.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 3.4.2003
 * revisions ..: $Id: ltiSupervisedInstanceClassifierFactory.cpp,v 1.3 2006/02/07 18:27:03 ltilib Exp $
 */

#include "ltiSupervisedInstanceClassifierFactory.h"

// include all possible supervised instance classifiers
#include "ltiManualCrispDecisionTree.h"
#include "ltiLvq.h"
#include "ltiMLP.h"
#include "ltiRbf.h"
#include "ltiSHClassifier.h"
#include "ltiSVM.h"

namespace lti {
  // --------------------------------------------------
  // static members
  // --------------------------------------------------

    const supervisedInstanceClassifier 
    *const supervisedInstanceClassifierFactory::classifiers[] = {new rbf(), 
            new MLP(), new lvq(), new shClassifier(), new svm(), 
            new manualCrispDecisionTree(), 0 };

  objectFactory<supervisedInstanceClassifier>
  supervisedInstanceClassifierFactory::factory(classifiers);

  // --------------------------------------------------
  // supervisedInstanceClassifierFactory
  // --------------------------------------------------

  // default constructor
  supervisedInstanceClassifierFactory::supervisedInstanceClassifierFactory() {
  }

  // destructor
  supervisedInstanceClassifierFactory::~supervisedInstanceClassifierFactory() {
  }

  // returns the name of this type
  const char*
  supervisedInstanceClassifierFactory::getTypeName() const {
    return "supervisedInstanceClassifierFactory";
  }

  supervisedInstanceClassifier* 
  supervisedInstanceClassifierFactory::newInstance(const char *name) const {
    return factory.newInstance(name);
  }

}
