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


/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiBinomialDistribution.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 26.3.2002
 * revisions ..: $Id: ltiBinomialDistribution.cpp,v 1.9 2006/09/05 10:38:36 ltilib Exp $
 */

#include "ltiMath.h"
#include "ltiBinomialDistribution.h"

namespace lti {
  // --------------------------------------------------
  // binomialDistribution::parameters
  // --------------------------------------------------

  // default constructor
  binomialDistribution::parameters::parameters()
    : discreteRandomDistribution::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    sampleSize = 1;
    events = 0;
    confidence = 0.95;
    accuracy = 0.0001;
    baseProbability = 0.5;
  }

  // copy constructor
  binomialDistribution::parameters::parameters(const parameters& other)
    : discreteRandomDistribution::parameters()  {
    copy(other);
  }

  // destructor
  binomialDistribution::parameters::~parameters() {
  }

  // get type name
  const char* binomialDistribution::parameters::getTypeName() const {
    return "binomialDistribution::parameters";
  }

  // copy member

  binomialDistribution::parameters&
    binomialDistribution::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    discreteRandomDistribution::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    discreteRandomDistribution::parameters& (discreteRandomDistribution::parameters::* p_copy)
      (const discreteRandomDistribution::parameters&) =
      discreteRandomDistribution::parameters::copy;
    (this->*p_copy)(other);
# endif


      sampleSize = other.sampleSize;
      events = other.events;
      confidence = other.confidence;
      accuracy = other.accuracy;
      baseProbability = other.baseProbability;

    return *this;
  }

  // alias for copy member
  binomialDistribution::parameters&
    binomialDistribution::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* binomialDistribution::parameters::clone() const {
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
  bool binomialDistribution::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool binomialDistribution::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"sampleSize",sampleSize);
      lti::write(handler,"events",events);
      lti::write(handler,"confidence",confidence);
      lti::write(handler,"accuracy",accuracy);
      lti::write(handler,"baseProbability",baseProbability);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && discreteRandomDistribution::parameters::write(handler,false);
# else
    bool (discreteRandomDistribution::parameters::* p_writeMS)(ioHandler&,const bool) const =
      discreteRandomDistribution::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool binomialDistribution::parameters::write(ioHandler& handler,
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
  bool binomialDistribution::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool binomialDistribution::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"sampleSize",sampleSize);
      lti::read(handler,"events",events);
      lti::read(handler,"confidence",confidence);
      lti::read(handler,"accuracy",accuracy);
      lti::read(handler,"baseProbability",baseProbability);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && discreteRandomDistribution::parameters::read(handler,false);
# else
    bool (discreteRandomDistribution::parameters::* p_readMS)(ioHandler&,const bool) =
      discreteRandomDistribution::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool binomialDistribution::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // binomialDistribution
  // --------------------------------------------------

  // default constructor
  binomialDistribution::binomialDistribution()
    : discreteRandomDistribution() {

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // copy constructor
  binomialDistribution::binomialDistribution(const binomialDistribution& other)
    : discreteRandomDistribution()  {
    copy(other);
  }

  // destructor
  binomialDistribution::~binomialDistribution() {
  }

  // returns the name of this type
  const char* binomialDistribution::getTypeName() const {
    return "binomialDistribution";
  }

  // copy member
  binomialDistribution&
    binomialDistribution::copy(const binomialDistribution& other) {
      discreteRandomDistribution::copy(other);

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  binomialDistribution&
    binomialDistribution::operator=(const binomialDistribution& other) {
    return (copy(other));
  }


  // clone member
  functor* binomialDistribution::clone() const {
    return new binomialDistribution(*this);
  }

  // return parameters
  const binomialDistribution::parameters&
    binomialDistribution::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------


  /*
    Returns a binomial random number, i.e. zero or one. 1 is returned if
    the event occurs, 0 if it doesn't.
    @return binomial random number.
  */
  int binomialDistribution::draw() const {
    return (random()<getParameters().baseProbability?1:0);
  }

  /*
    Returns the probability for exactly k events with the given parameters.
    If k is out of bounds, zero is returned.
    @param k number of positive events
    @return probability of k events
  */
  double binomialDistribution::pdf(const int& k) const {
    return pdf(k,getParameters().baseProbability,getParameters().sampleSize);
  }

  /*
    Returns the probability for base probability p with the given
    parameters.
    If p is out of bounds it is truncated to 0 or 1.
    @param p base probability of an event
    @return probability of k events (paramters) with base probability p
  */
  double binomialDistribution::pdf(const double& p) const {
    return pdf(getParameters().events,p,getParameters().sampleSize);
  }

  /*
    Returns the probability for base probability p and number of events k
    with the given parameters.
    If p is out of bounds it is truncated to 0 or 1.
    @param p base probability of an event
    @return probability of k events (paramters) with base probability p
  */
  double binomialDistribution::pdf(const int& k, const double& p) const {
    return pdf(k,p,getParameters().sampleSize);
  }

  /*
    Returns the probability for base probability p and number of events k
    and number of samples n.
    If p is out of bounds it is truncated to 0 or 1.
    @param p base probability of an event
    @return probability of k events (paramters) with base probability p
  */
  double binomialDistribution::pdf(const int& k, const double& p,
                                   const int& n) const {

    if (p<0 || p>1) {
      return 0.;
    }
    if (k<0 || k>n) {
      return 0.;
    }

    double prob=pow(1.-p,n);
    int i;
    for (i=1; i<=k; i++) {
      prob*=1+((n+1) * p - i)/(i*(1-p));
    }

    return prob;

  }

  /*
    Returns the cumulated probability for greater equal k events
    with the given parameters.
    If k is out of bounds, zero is returned.
    @param k max number of positive events
    @return probability of up to k events
  */
  double binomialDistribution::cdf(const int& k) const {
   return cdf(k,getParameters().baseProbability,getParameters().sampleSize);
  }

  /*
    Returns the cumulated probability for greater equal k events as set
    in the parameters. The base probability in the argument is used.
    If p is out of bounds it is truncated to 0 or 1.
    @param p base probability of an event
    @return probability of up to k events (parameters) with base prob p.
  */
  double binomialDistribution::cdf(const double& p) const {
   return cdf(getParameters().events,p,getParameters().sampleSize);
  }

  /*
    Returns the cumulated probability for greater equal k events,
    base probability p, and sample size n.
    If k is out of bounds, zero is returned.
    If p is out of bounds it is truncated to 0 or 1.
    @param k max number of positive events
    @param p base probability of an event
    @param n number of samples
    @return probability of up to k events (parameters) with base prob p.
  */
  double binomialDistribution::cdf(const int& k, const double& p,
                                   const int& n) const {


    if (p<0 || p>1) {
      return 0.;
    }
    if (k<0 || k>n) {
      return 0.;
    }

    int i;
    double prob=0.;
    for (i=0; i<=k; i++) {
      prob+=pdf(i,p,n);
    }
    return prob;
  }

  /*
    Returns the quantile for a given confidence probability. For the
    number of samples and the base probability, the parameters are used.
    @param confProb confidence
    @return the quantile
  */
  int binomialDistribution::quantile(const double& confProb) const {

    const parameters& param=getParameters();
    int n=param.sampleSize;
    double prob=0.;

    int i=0;
    while (i<=n && prob<confProb) {
      prob+=pdf(i++);
    }

    return (i==0 ? 0 : i-1);

  }

  /*
    Returns an approximated upper bound for the base probability of an
    event given the number of events. The number of samples and the
    confidence probability are taken from the parameters.
      @param k number of positive events
      @return upper bound for the base probability
  */
  double binomialDistribution::upperBound(const int& k) const {

    return upperBound(k, getParameters().sampleSize);
  }

  /*
    Returns an approximated upper bound for the base probability of an
    event given the number of events and the number of samples. The
    confidence probability is taken from the parameters.
    @param k number of positive events
    @param n number of samples
    @return upper bound for the base probability
  */
  double binomialDistribution::upperBound(const int& k, const int& n) const {

    const parameters& param=getParameters();
    double conf=param.confidence;
    double p=0.5;
    double l=0.;
    double h=1.0;
    double prob=cdf(k,p,n);
    double lastProb=0.;
    double acc=param.accuracy;

    //cout << param.accuracy << endl;

    while (fabs(conf-prob)>acc) {
      if (prob < conf) {
        h=p;
        p=0.5*(l+p);
      } else {
        l=p;
        p=0.5*(p+h);
      }
      lastProb=prob;
      prob=cdf(k,p,n);
      if (fabs(lastProb-prob)<(acc/1000)) {
        return 2.;
      }
      //cout << p << "  " << prob << "  " << fabs(conf-prob) << endl;
    }
    return p;
  }

}
