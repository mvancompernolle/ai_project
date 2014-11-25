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
 * file .......: ltiKalmanFilter.cpp
 * authors ....: Joerg Zieren
 * organization: LTI, RWTH Aachen
 * creation ...: 26.3.2001
 * revisions ..: $Id: ltiKalmanFilter.cpp,v 1.8 2006/09/05 10:18:43 ltilib Exp $
 */

#include "ltiKalmanFilter.h"
#include "ltiTypes.h"

namespace lti {
  // --------------------------------------------------
  // kalmanFilter::parameters
  // --------------------------------------------------

  // default constructor
  kalmanFilter::parameters::parameters() : functor::parameters() {
    // The parameters consist of vectors and matrices, using their
    // default constructors.  Thus they are all initialized to have 0
    // elements.
  }

  // set all matrices and vectors to their correct size, given the
  // number of system, measurement and control dimensions
  kalmanFilter::parameters::parameters(int systemDimension,
                                       int measurementDimension,
                                       int controlDimension)
    : functor::parameters() {

    initialSystemState.resize(systemDimension);
    dynamicsMatrix.resize(systemDimension,systemDimension);
    controlMatrix.resize(systemDimension,controlDimension);
    measurementMatrix.resize(measurementDimension,systemDimension);
    measurementNoiseCovariance.resize(measurementDimension,
                                      measurementDimension);
    processNoiseCovariance.resize(systemDimension,systemDimension);
    initialErrorCovariance.resize(systemDimension,systemDimension);
  }

  // copy constructor
  kalmanFilter::parameters::parameters(const parameters& other) : functor::parameters() {
    copy(other);
  }

  // destructor
  kalmanFilter::parameters::~parameters() {
    // all members are automatic variables
  }

  // get type name
  const char* kalmanFilter::parameters::getTypeName() const {
    return "kalmanFilter::parameters";
  }

  // copy member
  kalmanFilter::parameters&
  kalmanFilter::parameters::copy(const kalmanFilter::parameters& other) {
    initialSystemState = other.initialSystemState;
    dynamicsMatrix = other.dynamicsMatrix;
    controlMatrix = other.controlMatrix;
    measurementMatrix = other.measurementMatrix;
    measurementNoiseCovariance = other.measurementNoiseCovariance;
    processNoiseCovariance = other.processNoiseCovariance;
    initialErrorCovariance = other.initialErrorCovariance;
    return *this;
  }

  // alias for copy member
  kalmanFilter::parameters&
  kalmanFilter::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* kalmanFilter::parameters::clone() const {
    return new parameters(*this);
  }

  // consistency check
  bool kalmanFilter::parameters::consistent() const {
    return ((dynamicsMatrix.rows() == initialSystemState.size())
      && (dynamicsMatrix.columns() == initialSystemState.size())
      && (initialErrorCovariance.size() == dynamicsMatrix.size())
      && (processNoiseCovariance.size() == dynamicsMatrix.size())
      && (controlMatrix.rows() == initialSystemState.size())
      && (measurementMatrix.columns() == initialSystemState.size())
      && (measurementNoiseCovariance.rows() == measurementMatrix.rows())
      && (measurementNoiseCovariance.columns() == measurementMatrix.rows()));
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool kalmanFilter::parameters::write(ioHandler& handler,
                                       const bool complete) const
# else
  bool kalmanFilter::parameters::writeMS(ioHandler& handler,
                                         const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      // this expression is evaluated from left to right,
      // so the first failure causes the following write
      // operations not to be carried out
      b = b && initialSystemState.write(handler,false)
        && dynamicsMatrix.write(handler,false)
        && controlMatrix.write(handler,false)
        && measurementMatrix.write(handler,false)
        && measurementNoiseCovariance.write(handler,false)
        && processNoiseCovariance.write(handler,false)
        && initialErrorCovariance.write(handler,false);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::write(handler,false);
# else
    bool (functor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      functor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool kalmanFilter::parameters::write(ioHandler& handler,
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
  bool kalmanFilter::parameters::read(ioHandler& handler,
                                      const bool complete)
# else
  bool kalmanFilter::parameters::readMS(ioHandler& handler,
                                        const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      // this expression is evaluated from left to right,
      // so the first failure causes the following read
      // operations not to be carried out
      b = b && initialSystemState.read(handler,false)
        && dynamicsMatrix.read(handler,false)
        && controlMatrix.read(handler,false)
        && measurementMatrix.read(handler,false)
        && measurementNoiseCovariance.read(handler,false)
        && processNoiseCovariance.read(handler,false)
        && initialErrorCovariance.read(handler,false);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::read(handler,false);
# else
    bool (functor::parameters::* p_readMS)(ioHandler&,const bool) =
      functor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool kalmanFilter::parameters::read(ioHandler& handler, const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // kalmanFilter
  // --------------------------------------------------

  // default constructor
  kalmanFilter::kalmanFilter() : functor() {
    parameters defaultParameters;
    setParameters(defaultParameters);

    reset();
  }

  // copy constructor
  kalmanFilter::kalmanFilter(const kalmanFilter& other) : functor() {
    copy(other);
  }

  // destructor
  kalmanFilter::~kalmanFilter() {
  }

  // returns the name of this type
  const char* kalmanFilter::getTypeName() const {
    return "kalmanFilter";
  }

  // copy member
  kalmanFilter& kalmanFilter::copy(const kalmanFilter& other) {
    setParameters(other.getParameters());

    // copy internal state
    systemStateApriori         = other.systemStateApriori;
    systemStateAposteriori     = other.systemStateAposteriori;
    kalmanGainMatrix           = other.kalmanGainMatrix;
    errorCovarianceApriori     = other.errorCovarianceApriori;
    errorCovarianceAposteriori = other.errorCovarianceAposteriori;

    return (*this);
  }

  // clone member
  functor* kalmanFilter::clone() const {
    return new kalmanFilter(*this);
  }

  // return parameters
  const kalmanFilter::parameters& kalmanFilter::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // perform measurement update without control input
  bool kalmanFilter::apply(const lti::vector<float>& measurement) {
    return measurementUpdate(measurement) && timeUpdate();
  }

  // perform measurement update with control input
  bool kalmanFilter::apply(const lti::vector<float>& measurement,
                           const lti::vector<float>& control) {
    return measurementUpdate(measurement) && timeUpdate(control);
  }

  // perform time update
  bool kalmanFilter::apply() {
    return timeUpdate();
  }

  // perform measurement update
  bool kalmanFilter::applyMeasurementUpdate(
    const lti::vector<float>& measurement) {
    return measurementUpdate(measurement);
  }

  // perform time update with control input
  bool kalmanFilter::applyTimeUpdate(
    const lti::vector<float>& control) {
    return timeUpdate(control);
  }

  // perform time update without control input
  bool kalmanFilter::applyTimeUpdate() {
    return timeUpdate();
  }

  // perform time update (with control input)
  bool kalmanFilter::timeUpdate(const lti::vector<float>& controlVector) {

    const parameters& params = getParameters();

    if (params.controlMatrix.columns() != controlVector.size()) {
      return false;
    }

    // The time update and measurement update equations implemented here
    // can be found in the tech report mentioned in the header file.

    // update system state
    systemStateApriori = systemStateAposteriori;
    params.dynamicsMatrix.multiply(systemStateApriori);
    params.controlMatrix.multiply(controlVector,tempVector);
    systemStateApriori.add(tempVector);

    // update error covariance
    errorCovarianceApriori.multiply(params.dynamicsMatrix,
                                    errorCovarianceAposteriori);
    tempMatrix = params.dynamicsMatrix;
    tempMatrix.transpose();
    errorCovarianceApriori *= tempMatrix;
    errorCovarianceApriori += params.processNoiseCovariance;

    return true;
  }

  // perform time update (no control input)
  bool kalmanFilter::timeUpdate() {
    const parameters& params = getParameters();

    // The time update and measurement update equations implemented here
    // can be found in the tech report mentioned in the header file.

    // update system state
    systemStateApriori = systemStateAposteriori;
    params.dynamicsMatrix.multiply(systemStateApriori);

    // update error covariance
    errorCovarianceApriori.multiply(params.dynamicsMatrix,
                                    errorCovarianceAposteriori);
    tempMatrix = params.dynamicsMatrix;
    tempMatrix.transpose();
    errorCovarianceApriori *= tempMatrix;
    errorCovarianceApriori += params.processNoiseCovariance;

    return true;
  }

  // perform measurement update
  bool kalmanFilter::measurementUpdate(const lti::vector<float>& measurementVector) {
    const parameters& params = getParameters();

    if (params.measurementMatrix.rows() != measurementVector.size()) {
      return false;
    }
    // update kalman gain
    tempMatrix = params.measurementMatrix;
    tempMatrix.transpose();
    kalmanGainMatrix.multiply(errorCovarianceApriori,tempMatrix);

    tempMatrix2.multiply(params.measurementMatrix,errorCovarianceApriori);
    tempMatrix2 *= tempMatrix;
    tempMatrix2.add(params.measurementNoiseCovariance);
    myMatrixInvertor.apply(tempMatrix2);

    kalmanGainMatrix *= tempMatrix2;

    // update system state
    params.measurementMatrix.multiply(systemStateApriori,tempVector);
    tempVector2.subtract(measurementVector,tempVector);
    kalmanGainMatrix.multiply(tempVector2);
    systemStateAposteriori.add(systemStateApriori,tempVector2);

    // update error covariance
    tempMatrix.multiply(kalmanGainMatrix,params.measurementMatrix);
    tempMatrix *= errorCovarianceApriori;
    errorCovarianceAposteriori.subtract(errorCovarianceApriori,tempMatrix);

    return true;
  }

  const vector<float>& kalmanFilter::getEstimate() const {
    return systemStateApriori;
  }

  const vector<float>& kalmanFilter::getEstimateAposteriori() const {
    return systemStateAposteriori;
  }

  const matrix<float>& kalmanFilter::getErrorCovarianceAposteriori() const {
    return errorCovarianceAposteriori;
  }

  const matrix<float>& kalmanFilter::getErrorCovarianceApriori() const {
    return errorCovarianceApriori;
  }

  void kalmanFilter::reset() {

    const parameters& params = getParameters();

    systemStateApriori = params.initialSystemState;
    systemStateAposteriori = params.initialSystemState;
    errorCovarianceApriori = params.initialErrorCovariance;
    errorCovarianceAposteriori = params.initialErrorCovariance;
  }

}
