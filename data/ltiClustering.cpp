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
 * file .......: ltiClustering.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 29.08.2001
 * revisions ..: $Id: ltiClustering.cpp,v 1.7 2006/09/05 09:56:59 ltilib Exp $
 */

#include "ltiClustering.h"
#include "ltiUnsupervisedClassifier.h"

namespace lti {


  // *** implementation of clustering::parameters

  clustering::parameters::parameters()
    : unsupervisedClassifier::parameters() {
    clusterMode=batch;
  }

  clustering::parameters::parameters(const clustering::parameters& other)
    : unsupervisedClassifier::parameters(other) {
    copy(other);
  }

  clustering::parameters::~parameters() {
  }

  const char* clustering::parameters::getTypeName() const {
    return "clustering::parameters";
  }

  clustering::parameters&
  clustering::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    unsupervisedClassifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    unsupervisedClassifier::parameters&
      (unsupervisedClassifier::parameters::* p_copy)
      (const unsupervisedClassifier::parameters&) =
      unsupervisedClassifier::parameters::copy;
    (this->*p_copy)(other);
# endif

    clusterMode     = other.clusterMode;

    return *this;
  }

  // clone member
  classifier::parameters* clustering::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool clustering::parameters::write(ioHandler& handler,
                                     const bool complete) const
# else
    bool clustering::parameters::writeMS(ioHandler& handler,
                                         const bool complete) const
# endif
  {

    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      switch (clusterMode) {
      case batch:
        b = b && lti::write(handler,"clusterMode","batch");
        break;
      case sequential:
        b = b && lti::write(handler,"clusterMode","sequential");
        break;
      case online:
        b = b && lti::write(handler,"clusterMode","online");
        break;
      case miniBatch:
        b = b && lti::write(handler,"clusterMode","miniBatch");
        break;
      default:
        b = b && lti::write(handler,"clusterMode","batch");
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && unsupervisedClassifier::parameters::write(handler,false);
# else
    bool (unsupervisedClassifier::parameters::* p_writeMS)
      (ioHandler&, const bool) const =
      unsupervisedClassifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool clustering::parameters::write(ioHandler& handler,
                                     const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /*
   * read the parameters from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool clustering::parameters::read(ioHandler& handler,
                                    const bool complete)
# else
  bool clustering::parameters::readMS(ioHandler& handler,
                                      const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      std::string str;
      b = b && lti::read(handler,"clusterMode",str);

      if (str == "batch") {
        clusterMode = batch;
      } else if (str == "sequential") {
        clusterMode = sequential;
      } else if (str == "online") {
        clusterMode = online;
      } else if (str == "miniBatch") {
        clusterMode = miniBatch;
      } else {
        clusterMode = batch;
      }

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && unsupervisedClassifier::parameters::read(handler,false);
# else
    bool (unsupervisedClassifier::parameters::* p_readMS)
      (ioHandler&,const bool) =
      unsupervisedClassifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool clustering::parameters::read(ioHandler& handler,
                                    const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif



  // *** implementation of clustering

  clustering::clustering() : unsupervisedClassifier()  {

    // default parameters object
    parameters p;
    setParameters(p);

    outTemplate=outputTemplate();

  }

  clustering::clustering(const clustering& other)
    : unsupervisedClassifier(other) {}

  clustering::~clustering() {
  }

  const char* clustering::getTypeName() const {
    return "clustering";
  }

  clustering& clustering::copy(const clustering& other) {

    unsupervisedClassifier::copy(other);
    return (*this);
  }

  // return parameters
  const clustering::parameters& clustering::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool clustering::train(const dmatrix& input, ivector& ids) {

    return unsupervisedClassifier::train(input, ids);
  }



}
