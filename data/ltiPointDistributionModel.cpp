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

/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiPointDistributionModel.cpp
 * authors ....: Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 12.6.2001
 * revisions ..: $Id: ltiPointDistributionModel.cpp,v 1.7 2006/09/05 10:25:21 ltilib Exp $
 */

//TODO: include files
#include "ltiPointDistributionModel.h"

namespace lti {

  // default constructor
  pointDistributionModel::pointDistributionModel() : ioObject() {
    meanShape.clear();
    eigenVectorMatrix.clear();
    varianceVector.clear();
  }

  // copy constructor
  pointDistributionModel::pointDistributionModel(const
                                                 pointDistributionModel& other)
    : ioObject() {
    copy(other);
  }

  // destructor
  pointDistributionModel::~pointDistributionModel() {
  }

  // copy data from other ioObject
  pointDistributionModel& pointDistributionModel::copy(const pointDistributionModel& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    ioObject::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    ioObject& (ioObject::* p_copy)
      (const ioObject&) =
      ioObject::copy;
    (this->*p_copy)(other);
# endif


    meanShape = other.meanShape;
    eigenVectorMatrix = other.eigenVectorMatrix;
    varianceVector = other.varianceVector;

    return *this;
  }

  // assigment operator (alias for copy(other)).
  pointDistributionModel& pointDistributionModel::operator=(const pointDistributionModel& other) {
    return copy(other);
  }

  // get type name
  const char* pointDistributionModel::getTypeName() const {
    return "pointDistributionModel";
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */

# ifndef _LTI_MSC_6
  bool pointDistributionModel::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool pointDistributionModel::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler, "meanShape", meanShape);
      lti::write(handler, "eigenVectorMatrix", eigenVectorMatrix);
      lti::write(handler, "varianceVector", varianceVector);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && ioObject::write(handler,false);
# else
    bool (ioObject::* p_writeMS)(ioHandler&,const bool) const =
      ioObject::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool pointDistributionModel::write(ioHandler& handler,
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
  bool pointDistributionModel::read(ioHandler& handler,
                                        const bool complete)
# else
  bool pointDistributionModel::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler, "meanShape", meanShape);
      lti::read(handler, "eigenVectorMatrix", eigenVectorMatrix);
      lti::read(handler, "varianceVector", varianceVector);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && ioObject::read(handler,false);
# else
    bool (ioObject::* p_readMS)(ioHandler&,const bool) =
      ioObject::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool pointDistributionModel::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // read point distribution model
  bool read(ioHandler& handler, pointDistributionModel& p, const bool complete) {
    return p.read(handler,complete);
  }

  // read point distribution model
  bool write(ioHandler& handler, const pointDistributionModel& p, const bool complete) {
    return p.write(handler,complete);
  }

}
