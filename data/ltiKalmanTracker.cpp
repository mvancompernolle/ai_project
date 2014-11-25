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
 * file .......: ltiKalmanTracker.cpp
 * authors ....: Joerg Zieren
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.2001
 * revisions ..: $Id: ltiKalmanTracker.cpp,v 1.10 2006/09/05 10:19:00 ltilib Exp $
 */

#include "ltiKalmanTracker.h"
#include "ltiTypes.h"

namespace lti {
  // constructor for n-dim state vector
  // default constructor if optional argument is omitted
  kalmanTracker::parameters::parameters(const int& stateDimensions)
      : functor::parameters() {
    switch (stateDimensions) {
    case 6:
      initialErrorCovariance.resize(6,6);
      initialSystemState.resize(6);
      measurementNoiseCovariance.resize(2,2);
      processNoiseCovariance.resize(6,6);

      initialErrorCovariance.setIdentity(1.0f);
      initialSystemState.fill(0.0f);
      measurementNoiseCovariance.fill(0.0f);
      processNoiseCovariance.fill(0.0f);
      autoInitialize = false;
      break;
    default: // all other values -> 4-dim.
      initialErrorCovariance.resize(4,4);
      initialSystemState.resize(4);
      measurementNoiseCovariance.resize(2,2);
      processNoiseCovariance.resize(4,4);

      initialErrorCovariance.setIdentity(1.0f);
      initialSystemState.fill(0.0f);
      measurementNoiseCovariance.fill(0.0f);
      processNoiseCovariance.fill(0.0f);
      autoInitialize = false;
      break;
    }
  }

  // copy constructor
  kalmanTracker::parameters::parameters(const parameters& other) : functor::parameters() {
    copy(other);
  }

  // destructor
  kalmanTracker::parameters::~parameters() {
  }

  const char* kalmanTracker::parameters::getTypeName() const {
    return "kalmanTracker::parameters";
  }

  kalmanTracker::parameters& kalmanTracker::parameters::copy(
    const parameters& other) {
#ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
#else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    lti::functor::parameters& (lti::functor::parameters::* p_copy)
      (const lti::functor::parameters&) =
      lti::functor::parameters::copy;
    (this->*p_copy)(other);
#endif
    initialErrorCovariance = other.initialErrorCovariance;
    initialSystemState = other.initialSystemState;
    measurementNoiseCovariance = other.measurementNoiseCovariance;
    processNoiseCovariance = other.processNoiseCovariance;
    autoInitialize = other.autoInitialize;
    return *this;
  }

  kalmanTracker::parameters& kalmanTracker::parameters::operator=(
    const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* kalmanTracker::parameters::clone() const {
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
  bool kalmanTracker::parameters::write(ioHandler& handler, const bool complete) const
# else
  bool kalmanTracker::parameters::writeMS(ioHandler& handler, const bool complete) const
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
        && measurementNoiseCovariance.write(handler,false)
        && processNoiseCovariance.write(handler,false)
        && initialErrorCovariance.write(handler,false)
        && handler.write(autoInitialize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::write(handler,false);
# else
    bool (lti::functor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      lti::functor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool kalmanTracker::parameters::write(ioHandler& handler, const bool complete) const {
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
  bool kalmanTracker::parameters::read(ioHandler& handler, const bool complete)
# else
  bool kalmanTracker::parameters::readMS(ioHandler& handler, const bool complete)
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
        && measurementNoiseCovariance.read(handler,false)
        && processNoiseCovariance.read(handler,false)
        && initialErrorCovariance.read(handler,false)
        && handler.read(autoInitialize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::read(handler,false);
# else
    bool (lti::functor::parameters::* p_readMS)(ioHandler&,const bool) =
      lti::functor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool kalmanTracker::parameters::read(ioHandler& handler, const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // ------------------------------------- kalmanTracker ----------------------

  kalmanTracker::kalmanTracker(const int& stateDimension) : functor() {
    if (stateDimension == 6) {
      measurement.resize(2);
      initialized = false;

      parameters params(6);
      setParameters(params);
    } else {
      measurement.resize(2);
      initialized = false;

      parameters defaultParameters;
      setParameters(defaultParameters);
    }
  }

  kalmanTracker::kalmanTracker(const kalmanTracker& other) : functor() {
    measurement.resize(2);
    initialized = false;

    copy(other);
  }

  kalmanTracker::~kalmanTracker() {
  }

  // perform measurement then time update using the given
  // x and y measurements + store x/y prediction for next time
  // step (i.e. the a priori system state estimate).
  bool kalmanTracker::apply(const float& xMeasurement,
                            const float& yMeasurement,
                            float& xPredictionApriori,
                            float& yPredictionApriori) {
    bool b =
      performMeasurementUpdate(xMeasurement,yMeasurement)
      && performTimeUpdate();

    prediction = kalman.getEstimate();

    // read x/y coordinate prediction results from vector
    xPredictionApriori = prediction[0];
    yPredictionApriori = prediction[2];

    return b;
  }

  // perform only time update
  bool kalmanTracker::apply(float& xPredictionApriori,
                            float& yPredictionApriori) {
    bool b = performTimeUpdate();
    prediction = kalman.getEstimate();

    // read x/y coordinate prediction results from vector
    xPredictionApriori = prediction[0];
    yPredictionApriori = prediction[2];

    return b;
  }


  const char* kalmanTracker::getTypeName(void) const {
    return "kalmanTracker";
  }

  // copy member
  kalmanTracker& kalmanTracker::copy(const kalmanTracker& other) {
    setParameters(other.getParameters());
    return (*this);
  }

  // clone member
  functor* kalmanTracker::clone(void) const {
    return new kalmanTracker(*this);
  }

  // return parameters
  const kalmanTracker::parameters& kalmanTracker::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // set the parameters for this Kalman filter
  bool kalmanTracker::updateParameters() {
    // initialize with the values provided in the parameters object
    reset();
    return true;
  }

  // set a new measurement noise covariance
  void kalmanTracker::setMeasurementNoiseCovariance(const matrix<float>& r) {
    kalmanFilter::parameters kalmanParams(kalman.getParameters());
    kalmanParams.measurementNoiseCovariance = r;
    kalman.setParameters(kalmanParams);
  }

  void kalmanTracker::reset(void) {

    const parameters& params = getParameters();

    // configure kalmanFilter
    if (getSystemStateDimensions() == 6) {
      // assume constant acceleration. state_vector=(x,v_x, y,v_y)
      kalmanFilter::parameters filterParams(6,2,1);
      float dynamics[36] = {
        1,1,0,0, 0.5,0,
        0,1,0,0, 1,  0,
        0,0,1,1, 0,0.5,
        0,0,0,1, 0,  1,
        0,0,0,0, 1,  0,
        0,0,0,0, 0,  1};
      float measurement[12] = {1,0,0,0,0,0,
                               0,0,1,0,0,0};
      filterParams.dynamicsMatrix.fill(dynamics);
      filterParams.initialErrorCovariance = params.initialErrorCovariance;
      filterParams.initialSystemState = params.initialSystemState;
      filterParams.measurementMatrix.fill(measurement);
      filterParams.measurementNoiseCovariance =
        params.measurementNoiseCovariance;
      filterParams.processNoiseCovariance = params.processNoiseCovariance;
      kalman.setParameters(filterParams);
      kalman.reset();
      initialized = false;
    } else {
      kalmanFilter::parameters filterParams(4,2,1);
      // assume constant velocity. state_vector=(x,v_x, y,v_y, a_x,a_y)
      float dynamics[16] = {1,1,0,0,
                            0,1,0,0,
                            0,0,1,1,
                            0,0,0,1};
      float measurement[8] = {1,0,0,0,
                              0,0,1,0};
      filterParams.dynamicsMatrix.fill(dynamics);
      filterParams.initialErrorCovariance = params.initialErrorCovariance;
      filterParams.initialSystemState = params.initialSystemState;
      filterParams.measurementMatrix.fill(measurement);
      filterParams.measurementNoiseCovariance =
        params.measurementNoiseCovariance;
      filterParams.processNoiseCovariance = params.processNoiseCovariance;
      kalman.setParameters(filterParams);
      kalman.reset();
      initialized = false;
    }
  }

  bool kalmanTracker::isInitialized() const {
    return initialized;
  }

  const matrix<float>& kalmanTracker::getErrorCovarianceApriori() const {
    return kalman.getErrorCovarianceApriori();
  }

  const matrix<float>& kalmanTracker::getErrorCovarianceAposteriori() const {
    return kalman.getErrorCovarianceAposteriori();
  }

  const vector<float>& kalmanTracker::getEstimate() const {
    return kalman.getEstimate();
  }

  const vector<float>& kalmanTracker::getEstimateAposteriori() const {
    return kalman.getEstimateAposteriori();
  }

  bool kalmanTracker::performTimeUpdate() {
    initialized = true;
    return kalman.apply();
  }

  bool kalmanTracker::performMeasurementUpdate(const float& xMeasurement,
    const float& yMeasurement) {
    // wrap the floats in a vector
    measurement[0] = xMeasurement;
    measurement[1] = yMeasurement;

    // is this the first measurement & should the system state be
    // initialized to this measurement?
    if (!isInitialized()) {
      kalmanFilter::parameters filterParams(kalman.getParameters());
      if (getParameters().autoInitialize) { // initialize from first measurement
        filterParams.initialSystemState.fill(0.0f);
        filterParams.initialSystemState[0] = xMeasurement;
        filterParams.initialSystemState[2] = yMeasurement;
      } // else: initialSystemState remains what it was
      kalman.setParameters(filterParams);
      kalman.reset(); // parameters' initialSystemState -> filter's current state
      initialized = true;
    }

    return kalman.applyMeasurementUpdate(measurement);
  }

  int kalmanTracker::getSystemStateDimensions() const {
    return getParameters().initialSystemState.size();
  }
}
