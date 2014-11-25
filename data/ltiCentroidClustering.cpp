/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiCentroidClustering.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 29.08.2001
 * revisions ..: $Id: ltiCentroidClustering.cpp,v 1.4 2006/02/07 18:12:21 ltilib Exp $
 */

#include "ltiCentroidClustering.h"
#include "ltiUnsupervisedClassifier.h"
#include "ltiL1Distance.h"
#include "ltiL2Distance.h"

namespace lti {

  // *** implementation of centroidClustering

  centroidClustering::centroidClustering()
    : clustering(), centroids() {
  }

  centroidClustering::centroidClustering(const centroidClustering& other)
    : clustering(other)  {
    copy(other);
  }

  centroidClustering::~centroidClustering() {
  }

  const char* centroidClustering::getTypeName() const {
    return "centroidClustering";
  }

  centroidClustering& centroidClustering::copy(const centroidClustering& other) {

    clustering::copy(other);

    centroids.copy(other.centroids);

    return (*this);

  }

  // return parameters
  const centroidClustering::parameters&
  centroidClustering::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  const dmatrix& centroidClustering::getCentroids() const {

    return centroids;
  }

  bool centroidClustering::classify(const dvector& feature,
                                    classifier::outputVector& result) const {

    dvector dists;
    l2Distance<double> distFunc;

    distFunc.apply(centroids, feature, dists);
    double s=dists.sumOfElements();
    dists.divide(s);
    dists.apply(probabilize);
    s=dists.sumOfElements();
    dists.divide(s);

    bool rc=outTemplate.apply(dists, result);
    if (!rc) {
      setStatusString("The outputTemplate used returned an error.\nMost likely your setting of parameters::multipleMode does not match the information available in the outputTemplate.");
    }
    return rc;
  }

  bool centroidClustering::train(const dmatrix& input, ivector& ids) {

    return clustering::train(input, ids);
  }


  bool centroidClustering::write(ioHandler& handler,const bool complete) const {
    bool b(true);

    if (complete) {
      b = handler.writeBegin();
    }

    // write the standard data (output and parameters)
    b = b && clustering::write(handler,false);

    b = b && lti::write(handler,"centroids",centroids);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

  bool centroidClustering::read(ioHandler& handler,const bool complete) {
    bool b(true);

    if (complete) {
      b = handler.readBegin();
    }


    // read the standard data (output and parameters)
    b = b && clustering::read(handler,false);

    b = b && lti::read(handler,"centroids",centroids);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;

  }


  double centroidClustering::probabilize(const double& d) {

    return 1.-d;
  }

}
