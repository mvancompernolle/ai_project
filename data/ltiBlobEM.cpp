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
 * file .......: ltiBlobEM.cpp
 * authors ....: Suat Akyol
 * organization: LTI, RWTH Aachen
 * creation ...: 27.5.2002
 * revisions ..: $Id: ltiBlobEM.cpp,v 1.9 2006/09/05 10:03:07 ltilib Exp $
 */

// include files
#include "ltiBlobEM.h"
#include "ltiMatrixInversion.h"
#include "ltiUniformDist.h"

#include <limits>
#include <iomanip>
#include <sstream>

#ifdef _LTI_MSC_6
#define isnan _isnan
#endif

#ifdef _LTI_GNUC_2
namespace std {
  typedef ios ios_base;
}
#endif

using std::endl;

namespace lti {

  // maps arbitrary orientations to range -Pi/2 to Pi/2
  void mapAngle(double& ang) {

    ang += - lti::Pi*static_cast<int>(ang/(lti::Pi));

    if (ang>lti::Pi/2) {
      ang -= lti::Pi;
    }
    else if (ang<-lti::Pi/2) {
      ang += lti::Pi;
    }
  }

  // maps arbitrary orientations to range -Pi/2 to Pi/2
  void show(const blobEM::gaussEllipse& ellipse, std::ostream& ostr) {

    static const int wd = 15; // width for fields

    ostr << "Properties:" << endl;
    ostr.setf(std::ios_base::left);
    ostr << std::setw(wd) << "center" << " = " << ellipse.center << endl;
    ostr.setf(std::ios_base::left);
    ostr << std::setw(wd) << "angle" << " = " << ellipse.angle << endl;
    ostr.setf(std::ios_base::left);
    ostr << std::setw(wd) << "lambda1" << " = " << ellipse.lambda1 << endl;
    ostr.setf(std::ios_base::left);
    ostr << std::setw(wd) << "lambda2" << " = " << ellipse.lambda2 << endl;
    ostr << endl;
    ostr << "Constraining:" << endl;
    if (ellipse.constrainCenter) {
      ostr.setf(std::ios_base::left);
      ostr << std::setw(wd) << "center" << " = " << ellipse.centerTolerance
           << endl;
    }
    if (ellipse.constrainAngle) {
      ostr.setf(std::ios_base::left);
      ostr << std::setw(wd) << "angle" << " = " << ellipse.angleTolerance
           << endl;
    }
    if (ellipse.constrainArea) {
      ostr.setf(std::ios_base::left);
      ostr << std::setw(wd) << "area" << " = " << ellipse.areaTolerance
           << endl;
    }
    if (ellipse.constrainShape) {
      ostr.setf(std::ios_base::left);
      ostr << std::setw(wd) << "shape" << " = " << ellipse.shapeTolerance
           << endl;
    }

    ostr << endl;
  }

  // --------------------------------------------------
  // blobEM::parameters
  // --------------------------------------------------

  const double blobEM::LAMBDA = 1;
  const double blobEM::ALPHAEPSILON = 1e-3;


  // default constructor
  blobEM::parameters::parameters()
    : functor::parameters() {
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    maxIterations = int(100);
    convergenceThreshold = double(1e-6);
  }

  // copy constructor
  blobEM::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  blobEM::parameters::~parameters() {
  }

  // get type name
  const char* blobEM::parameters::getTypeName() const {
    return "blobEM::parameters";
  }

  // copy member

  blobEM::parameters&
    blobEM::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters& (functor::parameters::* p_copy)
      (const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
# endif

      maxIterations = other.maxIterations;
      convergenceThreshold = other.convergenceThreshold;

    return *this;
  }

  // alias for copy member
  blobEM::parameters&
    blobEM::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* blobEM::parameters::clone() const {
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
  bool blobEM::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool blobEM::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"maxIterations",maxIterations);
      lti::write(handler,"convergenceThreshold",convergenceThreshold);
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
  bool blobEM::parameters::write(ioHandler& handler,
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
  bool blobEM::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool blobEM::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"maxIterations",maxIterations);
      lti::read(handler,"convergenceThreshold",convergenceThreshold);
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
  bool blobEM::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // blobEM
  // --------------------------------------------------

  // default constructor
  blobEM::blobEM()
    : functor(){

    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    reset();

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // copy constructor
  blobEM::blobEM(const blobEM& other)
    : functor()  {
    copy(other);
  }

  // destructor
  blobEM::~blobEM() {
  }

  // returns the name of this type
  const char* blobEM::getTypeName() const {
    return "blobEM";
  }

  // copy member
  blobEM&
    blobEM::copy(const blobEM& other) {
      functor::copy(other);

    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    m_alphas = other.m_alphas;
    m_centers = other.m_centers;
    m_covariances = other.m_covariances;

    m_iterations = other.m_iterations;
    m_M = other.m_M;
    m_N = other.m_N;
    m_xi = other.m_xi;
    m_yi = other.m_yi;
    m_Ni = other.m_Ni;
    m_ellipses = other.m_ellipses;
    m_Q = other.m_Q;
    m_QDiff = other.m_QDiff;

    return (*this);
  }

  // alias for copy member
  blobEM&
    blobEM::operator=(const blobEM& other) {
    return (copy(other));
  }


  // clone member
  functor* blobEM::clone() const {
    return new blobEM(*this);
  }

  // return parameters
  const blobEM::parameters&
    blobEM::getParameters() const {
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

  // apply for type channel8!
  bool blobEM::apply(const channel8& src,
                     std::vector<gaussEllipse>& elem) {

    const parameters& param = getParameters();
    if (!initialize(src, elem)) {

      setStatusString("Couldn't initialize!");
      return false;
    }

    double lastQ = 0;
    m_Q = - std::numeric_limits<double>::max();
    m_QDiff = std::numeric_limits<double>::max();

    while ( ( (m_QDiff > fabs(lastQ*param.convergenceThreshold)) ||
              (param.convergenceThreshold <= 0.0) )
           && (getIterations() < param.maxIterations) ) {

      lastQ = m_Q;
      m_Q = iterate();
      m_QDiff = m_Q - lastQ;
    }

    // compute and copy ellipses into elem
    std::vector<gaussEllipse> tmpElem;
    if (!computeEllipses(tmpElem)) {

      return false;
    }
    elem = tmpElem;
    return true;
  }

  bool blobEM::apply(const channel8& src, const int& components) {

    const parameters& param = getParameters();
    if (!initialize(src, components)) {

      setStatusString("Couldn't initialize!");
      return false;
    }

    double lastQ = 0;
    m_Q = - std::numeric_limits<double>::max();
    m_QDiff = std::numeric_limits<double>::max();

    while ( ( (m_QDiff > fabs(lastQ*param.convergenceThreshold)) ||
              (param.convergenceThreshold <= 0.0) )
           && (getIterations() < param.maxIterations) ) {

      lastQ = m_Q;
      m_Q = iterate();
      m_QDiff = m_Q - lastQ;
    }

    return true;
  }

  bool blobEM::initialize(const channel8& src, const int& components) {

    reset();
    for (int row=0; row<src.rows(); row++) {

      for (int col=0; col<src.columns(); col++) {

        int val = src.at(row,col);
        if (val!=0) {
          m_xi.push_back(col);
          m_yi.push_back(row);
          m_Ni.push_back(val);
          m_N += val;
        }
      }
    }
    if (m_N<=0) {

      setStatusString("No Data!");
      return false;
    }

    std::vector<gaussEllipse> elem;
    double maxFact = sqrt(src.size().x * src.size().y);
    lti::uniformDistribution rnd;

    for (int i=0; i<components; i++) {

      gaussEllipse theEllipse;

      // randomly draw centers from data
      int rdIdx = lti::iround((m_xi.size()-1)*rnd.draw());
      theEllipse.center.x = m_xi[rdIdx];
      theEllipse.center.y = m_yi[rdIdx];

      // random lambdas, but make sure lambda1 is larger
      theEllipse.lambda1 = 1+maxFact*rnd.draw();
      theEllipse.lambda2 = 1+maxFact*rnd.draw();
      if (theEllipse.lambda2>theEllipse.lambda1) {
        double temp = theEllipse.lambda1;
        theEllipse.lambda1 = theEllipse.lambda2;
        theEllipse.lambda2 = temp;
      }
      theEllipse.angle   = lti::Pi*rnd.draw() - lti::Pi/2;

      elem.push_back(theEllipse);
    }


    std::vector<gaussEllipse>::const_iterator it;
    lti::matrix<double> cov;

    for (it=elem.begin();it!=elem.end();it++) {

      if ((*it).to2x2Covariance(cov)) {

        m_centers.push_back((*it).center);
        m_covariances.push_back(cov);
        m_ellipses.push_back((*it));
      }
    }

    m_M = m_centers.size();
    if ( m_centers.size()==m_covariances.size() ) {

      m_alphas.resize(m_M,1/double(m_M));
    } else {

      reset();
    }

    return true;
  }


  bool blobEM::initialize(const channel8& src,
                          const std::vector<gaussEllipse>& elem) {
    reset();
    for (int row=0; row<src.rows(); row++) {

      for (int col=0; col<src.columns(); col++) {

        int val = src.at(row,col);
        if (val!=0) {
          m_xi.push_back(col);
          m_yi.push_back(row);
          m_Ni.push_back(val);
          m_N += val;
        }
      }
    }
    if (m_N<=0) {

      reset();
      setStatusString("No Data!");
      return false;
    }

    std::vector<gaussEllipse>::const_iterator it;
    lti::matrix<double> cov;

    for (it=elem.begin();it!=elem.end();it++) {

      if ((*it).to2x2Covariance(cov)) {

        m_centers.push_back((*it).center);
        m_covariances.push_back(cov);
        m_ellipses.push_back((*it));
      }
    }

    m_M = m_centers.size();
    if ( m_centers.size()==m_covariances.size() ) {

      m_alphas.resize(m_M,1/double(m_M));
    } else {

      setStatusString("Dimension error!");
      reset();
      return false;
    }

    return true;
  }

  double blobEM::iterate() {

    m_iterations++;
    return singleEMStep();
  }

  bool blobEM::
    computeEllipses(std::vector<gaussEllipse>& ellipses) {

    ellipses.clear();
    blobEM::gaussEllipse el;

    bool success = true;
    for (int l=0; l<m_M; l++) {

      el = m_ellipses[l];
      if (!el.from2x2Covariance(m_covariances[l],false)) {
        success &= false;
      }
      el.center = m_centers[l];
      ellipses.push_back(el);
    }

    return success;
  }


  void blobEM::reset() {

    m_alphas.clear();
    m_centers.clear();
    m_covariances.clear();

    m_iterations = 0;
    m_M = 0;
    m_N = 0;
    m_xi.clear();
    m_yi.clear();
    m_Ni.clear();
    m_ellipses.clear();
    m_Q = - std::numeric_limits<double>::max();
    m_QDiff = 0;
  }

  const int& blobEM::getIterations() const {

    return m_iterations;
  }

  const int& blobEM::getM() const {

    return m_M;
  }

  const int& blobEM::getN() const {

    return m_N;
  }

  const std::vector<double>& blobEM::getAlphas() const {

    return m_alphas;
  }

  const std::vector<tpoint<double> >& blobEM::getCenters() const {

    return m_centers;
  }

  const std::vector<matrix<double> >& blobEM::getCovariances() const {

    return m_covariances;
  }

  const double& blobEM::getQ() const {

    return m_Q;
  }

  const double& blobEM::getQDiff() const {

    return m_QDiff;
  }


  double blobEM::singleEMStep() {

    if (m_N<=0) {
      return 0; // error
    }
    double doubleN = static_cast<double>(m_N);

    int i, l;
    int dim = m_Ni.size();  // is NOT N !!!
    int components = m_alphas.size();

    // prepare a matrix containing the p(l|x_i).
    // rows stand for the l-th mixture component
    // columns stand for the i-th data vector
    lti::matrix<double> p_l_xi(components, dim, double(0));
    lti::vector<double> rowSum(components);

    // compute all p(l|x_i)
    gauss2DPDF pdf;
    for (l=0; l<components; l++) {

      pdf.set2x2CovarianceMatrix(m_covariances[l]);
      for (i=0; i<dim; i++) {

        // p(l|x_i) = p(x_i|l)*p(l) / p(x_i)
        // with
        // p(x_i) = sum_i[p(x_i|l)*p(l)]  // can not be computed before end of loop
        p_l_xi.at(l,i) = m_alphas[l]*pdf.evaluate(m_xi[i],m_yi[i],m_centers[l]);
      }
    }

    rowSum.fill(static_cast<double>(0));
    for (i=0; i<dim; i++) {

      double csum = 0;
      for (l=0; l<components; l++) {
        csum += p_l_xi.at(l,i);
      }

      for (l=0; l<components; l++) {
        p_l_xi.at(l,i) *= m_Ni[i]/csum;
        rowSum[l] += p_l_xi.at(l,i);
      }

    }

    // re-estimate alphas and centers and covariances
    for (l=0; l<components; l++) {

      m_alphas[l] = rowSum[l]/doubleN;

      double sum_x_pl[2] = {0,0};
      double prob, tempX, tempY;
      for (i=0; i<dim; i++) {

        prob = p_l_xi.at(l,i);

        sum_x_pl[0] += m_xi[i]*prob;
        sum_x_pl[1] += m_yi[i]*prob;
      }

      // re-estimate center
      tempX = sum_x_pl[0]/rowSum[l];
      tempY = sum_x_pl[1]/rowSum[l];
      // if impossible, then randomly assign
      // one data point as center
      if ( isnan(tempX) || isnan(tempY) ) {
        lti::uniformDistribution rnd;
        int rdIdx = lti::iround((m_xi.size()-1)*rnd.draw());
        m_centers[l].x = m_xi[rdIdx];
        m_centers[l].y = m_yi[rdIdx];
      } else {
        m_centers[l].x = tempX;
        m_centers[l].y = tempY;
      }

      // limit, if constrained! lambda1,lambda2,angle are not affected
      if (m_ellipses[l].constrainCenter) {
        gaussEllipse el1, el2;

        el1.center = m_ellipses[l].center;
        el1.lambda1 = m_ellipses[l].lambda1;
        el1.lambda2 = m_ellipses[l].lambda2;
        el1.constrainCenter = m_ellipses[l].constrainCenter;
        el1.centerTolerance = m_ellipses[l].centerTolerance;

        el2.center = m_centers[l];

        el1.fromEllipse(el2);

        m_centers[l] = el1.center;
      }

      // re-estimate covariances and use regularizer
      double sum_pl_x_xT[4] = {0,0,0,0};
      double x_mx, y_my, temp;
      for (i=0; i<dim; i++) {

        prob = p_l_xi.at(l,i);
        x_mx = m_xi[i] - m_centers[l].x;
        y_my = m_yi[i] - m_centers[l].y;

        sum_pl_x_xT[0] += x_mx * x_mx * prob;
        temp = x_mx * y_my * prob;

        sum_pl_x_xT[1] += temp;
        sum_pl_x_xT[2] += temp;
        sum_pl_x_xT[3] += y_my * y_my * prob;

      }

      m_covariances[l].at(0,0) = (sum_pl_x_xT[0]+LAMBDA)/(rowSum[l]+1);
      m_covariances[l].at(0,1) = sum_pl_x_xT[1]/(rowSum[l]+1);
      m_covariances[l].at(1,0) = sum_pl_x_xT[2]/(rowSum[l]+1);
      m_covariances[l].at(1,1) = (sum_pl_x_xT[3]+LAMBDA)/(rowSum[l]+1);

      // limit covariances, if constrained! center is not affected!
      if ( m_ellipses[l].constrainShape ||
           m_ellipses[l].constrainArea  ||
           m_ellipses[l].constrainAngle) {

        // prepare temporary ellipse from current covariance
        gaussEllipse refEllipse = m_ellipses[l];

        refEllipse.from2x2Covariance(m_covariances[l]);

        // transform ellipse back into 2x2 covariance matrix
        refEllipse.to2x2Covariance(m_covariances[l]);
      }
    }

    for (l=0; l<components; l++) {
      m_alphas[l] = (m_alphas[l]+ALPHAEPSILON)/(1+ALPHAEPSILON*components);
    }


    double Q_val = 0;

    // compute Q value
    for (l=0; l<components; l++) {

      pdf.set2x2CovarianceMatrix(m_covariances[l]);
      for (i=0; i<dim; i++) {

        Q_val += p_l_xi.at(l,i)*log(m_alphas[l]*pdf.evaluate(m_xi[i],m_yi[i],m_centers[l]));
      }
    }

    m_QDiff = Q_val - m_Q;
    m_Q = Q_val;
    return m_Q;
  }


  /////////////////////////////////////////////////////////////////////////////
  // blobEM::gauss2DPDF
  /////////////////////////////////////////////////////////////////////////////

  const lti::point blobEM::gauss2DPDF::m_validSize(2,2);

  void blobEM::gauss2DPDF::
    reset() {
      m_covar.resize(2,2);
      m_covar.setIdentity();
      m_covarInverse.resize(2,2);
      m_covarInverse.setIdentity();
      m_det = 1.0;
  }

  double blobEM::gauss2DPDF::
    evaluate(int x, int y, const tpoint<double>& cen) const {

    if (m_det <= std::numeric_limits<double>::epsilon()) {
      return std::numeric_limits<double>::max();
    }

    const double ic11 = m_covarInverse.at(0,0);
    const double ic12 = m_covarInverse.at(0,1);
    const double ic21 = m_covarInverse.at(1,0);
    const double ic22 = m_covarInverse.at(1,1);

    const double x_mx = x - cen.x;
    const double y_my = y - cen.y;

    double K = ( x_mx*(x_mx*ic11 + y_my*ic21) + y_my*(x_mx*ic12 + y_my*ic22) );

    return 1/(2*Pi*sqrt(m_det)) * exp(-0.5 * K);
  }

  bool blobEM::gauss2DPDF::
    set2x2CovarianceMatrix(const lti::matrix<double>& covar) {

    if (covar.size() != m_validSize) {
      reset();
      return false;
    }

    m_covar = covar;
    lti::matrixInversion<double> invertor;
    if (!invertor.apply(m_covar,m_covarInverse)) {
      reset();
      return false;
    }

    m_det = m_covar.at(0,0)*m_covar.at(1,1) - m_covar.at(1,0)*m_covar.at(0,1);

    if (m_det<0) {
      m_det=0;
    }

    return true;
  }


  /////////////////////////////////////////////////////////////////////////////
  // blobEM::gaussEllipse
  /////////////////////////////////////////////////////////////////////////////

  blobEM::gaussEllipse::
    gaussEllipse() {

    center  = lti::tpoint<double>(0,0);
    lambda1 = 1;
    lambda2 = 1;
    angle   = 0;

    constrainCenter = false;
    constrainShape = false;
    constrainArea = false;
    constrainAngle = false;

    centerTolerance = 0.0;
    shapeTolerance = 0.0;
    areaTolerance = 0.0;
    angleTolerance = 0.0;

  }

  blobEM::gaussEllipse::
    gaussEllipse(lti::tpoint<double> cen,
                 double l1,
                 double l2,
                 double ang) {
    center  = cen;
    lambda1 = l1;
    lambda2 = l2;
    angle   = ang;

    constrainCenter = false;
    constrainShape = false;
    constrainArea = false;
    constrainAngle = false;

    centerTolerance = 0.0;
    shapeTolerance = 0.0;
    areaTolerance = 0.0;
    angleTolerance = 0.0;

    mapAngle(angle);
  }

  blobEM::gaussEllipse::
    ~gaussEllipse() {

  }

  bool blobEM::gaussEllipse::
    from2x2Covariance(const lti::matrix<double>& cov, const bool& constraints) {

    gaussEllipse tmp;

    double l1;
    double l2;
    lti::vector<double> e1;
    lti::vector<double> e2;

    bool success = true;
    if (!pca2x2(cov,l1,l2,e1,e2)) {

      tmp.lambda1 = 1;
      tmp.lambda2 = 1;
      tmp.angle = 0;
      tmp.center = this->center;  // can not be estimated from covar.matrix
      success = false;
    } else {

      tmp.lambda1 = l1;
      tmp.lambda2 = l2;
      tmp.angle = atan2(e1[1],e1[0]);
      tmp.center = this->center;  // can not be estimated from covar.matrix
    }

    fromEllipse(tmp, constraints);

    return success;
  }

  bool blobEM::gaussEllipse::
    fromEllipse(const gaussEllipse& other, const bool& constraints) {

    bool success = true;

    if (!constraints) {
      center = other.center;
      lambda1 = other.lambda1;
      lambda2 = other.lambda2;
      angle = other.angle;
      mapAngle(angle);
      return true;
    }

    // center computation
    if (!constrainCenter) {
      center = other.center;
    } else {

      double maxDistSq = centerTolerance*centerTolerance;//fabs(sqrt(lambda1*lambda2)*centerTolerance*centerTolerance);

      lti::tpoint<double> diff = other.center;
      diff.subtract(center);

      double distSq = diff.absSqr();

      if (distSq > std::numeric_limits<double>::epsilon()) {
        // do nothing
        if (distSq > maxDistSq) {

          double norm = sqrt(maxDistSq/distSq);
          if (!isnan(norm)) {
            center = center + diff*norm;
          }
        } else {
          center = other.center;
        }
      }
    }

    // covariances computation
    if (!constrainShape && !constrainArea) {
      lambda1 = other.lambda1;
      lambda2 = other.lambda2;
    }
    else {

      double origLambda1 = lambda1;
      double origLambda2 = lambda2;

      // keep area within limits
      if (constrainArea) {

        // take size (lambda1/lambda2=const), but don't change area

        double areaNorm = sqrt(lambda1*lambda2 / (other.lambda1*other.lambda2));
        double areaLimit = sqrt(1+areaTolerance);

        if (areaNorm >= areaLimit) {
          areaNorm *= areaLimit;
        }
        else if (areaNorm < 1/areaLimit) {
          areaNorm *= 1/areaLimit;
        }

        if (!isnan(areaNorm)) {

          lambda1 = other.lambda1*areaNorm;
          lambda2 = other.lambda2*areaNorm;
        } else {
          success = false;
        }
      }


      // keep shape within limits
      if (constrainShape) {

        // for size normation use lambda1*lambda2, since these could be adapted by
        // area constraining
        double norm = sqrt(lambda1*lambda2 / (origLambda1*origLambda2));

        // for shape limitation use origLambda1 and origLambda2
        if (!isnan(norm)) {

          origLambda1 *= norm;
          origLambda2 *= norm;
        } else {
          success = false;
        }

        lambda1 = origLambda1;
        lambda2 = origLambda2;
      }


    }

    // angle computation
    if (!constrainAngle) {
      angle = other.angle;
    } else {

      double angleDiff = other.angle - angle;
      double angleSign;
      // ensure  -Pi/2 < angleDiff < +Pi/2
      mapAngle(angleDiff);

      if (angleDiff>=0) {
        angleSign = +1.0;
      } else {
        angleSign = -1.0;
      }

      if (fabs(angleDiff)>angleTolerance) {
        angle = angle + angleTolerance*angleSign;
      } else {
        angle = angle + angleDiff;
      }

    }

    mapAngle(angle);

    return success;
  }

  bool blobEM::gaussEllipse::
    to2x2Covariance(lti::matrix<double>& cov) const {

    cov.resize(2,2,double(0),false,true);

    double cosA, sinA;
    sincos(angle, sinA, cosA);

    // use C = V * L * V^T formula
    cov.at(0,0) = cosA*cosA*lambda1 + sinA*sinA*lambda2;
    cov.at(1,0) = cosA*sinA*lambda1 - sinA*cosA*lambda2;
    cov.at(0,1) = cov.at(1,0);
    cov.at(1,1) = sinA*sinA*lambda1 + cosA*cosA*lambda2;

    return true;
  }

  bool blobEM::gaussEllipse::
    pca2x2(const lti::matrix<double>& cov,
           double& lambda1,
           double& lambda2,
           lti::vector<double>& e1,
           lti::vector<double>& e2) {

    lambda1 = 0;
    lambda2 = 0;
    e1.resize(2,0,false,true);
    e2.resize(2,0,false,true);

    double c11 = cov.at(0,0);
    double c12 = cov.at(0,1);
    double c21 = cov.at(1,0);
    double c22 = cov.at(1,1);

    static const double epsilon = std::numeric_limits<double>::epsilon();
    //static const double infinity = std::numeric_limits<double>::infinity();


    // solve quadratic equation lambda^2 + p*lambda + q = 0
    double pHalf = - (c11 + c22)/2;
    double q = c11*c22 - c12*c21;

    double temp = pHalf*pHalf - q;

    if (temp<0) {
      // numeric problem
      lambda1 = - pHalf;
      lambda2 = - pHalf;
    } else {
      temp = sqrt(temp);

      lambda1 = - pHalf + temp;
      lambda2 = - pHalf - temp;
    }

    if (lambda2>lambda1) {

      temp = lambda1;
      lambda1 = lambda2;
      lambda2 = lambda1;
    }

    double norm;

    if (fabs(c11 - lambda1) > epsilon) {
      temp = - c12 / (c11 - lambda1);
      norm = sqrt(temp*temp +1);
      e1[0] = temp / norm;
      e1[1] = 1 / norm;
    } else {
      e1[0] = 1;
      e1[1] = 0;
    }

    if (fabs(c11 - lambda2) > epsilon) {
      temp = - c12 / (c11 - lambda2);
      norm = sqrt(temp*temp +1);
      e2[0] = temp / norm;
      e2[1] = 1 / norm;
    } else {
      e2[0] =   e1[1];
      e2[1] = - e1[0];
    }

    return true;
  }

  /*
   * write a blobEM::ellipse into the given stream handler
   * @param handler ioHandler where the ellipse should be written.
   * @param el ellipse
   * @param complete if true (default), begin and end tokens will be written
   *                 around the object.
   * @return true if successful, false otherwise
   */
  bool write(ioHandler& handler,
             const blobEM::gaussEllipse& el,
             const bool complete) {
    bool b = true;

    if (complete) {
      b = handler.writeBegin();
    }

    b = b && lti::write(handler,"center",el.center);
    b = b && lti::write(handler,"lambda1",el.lambda1);
    b = b && lti::write(handler,"lambda2",el.lambda2);
    b = b && lti::write(handler,"angle",el.angle);
    b = b && lti::write(handler,"constrainCenter",el.constrainCenter);
    b = b && lti::write(handler,"constrainShape",el.constrainShape);
    b = b && lti::write(handler,"constrainArea",el.constrainArea);
    b = b && lti::write(handler,"constrainAngle",el.constrainAngle);
    b = b && lti::write(handler,"centerTolerance",el.centerTolerance);
    b = b && lti::write(handler,"shapeTolerance",el.shapeTolerance);
    b = b && lti::write(handler,"areaTolerance",el.areaTolerance);
    b = b && lti::write(handler,"angleTolerance",el.angleTolerance);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

  /*
   * read a blobEM::ellipse from the given stream handler
   * @param handler ioHandler where the ellipse should be read from.
   * @param el ellipse will be left here
   * @param complete if true (default), begin and end tokens will be read too.
   * @return true if successful, false otherwise
   */
  bool read(ioHandler& handler,
            blobEM::gaussEllipse& el,
            const bool complete) {

    bool b = true;

    if (complete) {
      b = handler.readBegin();
    }

    b = b && lti::read(handler,"center",el.center);
    b = b && lti::read(handler,"lambda1",el.lambda1);
    b = b && lti::read(handler,"lambda2",el.lambda2);
    b = b && lti::read(handler,"angle",el.angle);
    b = b && lti::read(handler,"constrainCenter",el.constrainCenter);
    b = b && lti::read(handler,"constrainShape",el.constrainShape);
    b = b && lti::read(handler,"constrainArea",el.constrainArea);
    b = b && lti::read(handler,"constrainAngle",el.constrainAngle);
    b = b && lti::read(handler,"centerTolerance",el.centerTolerance);
    b = b && lti::read(handler,"shapeTolerance",el.shapeTolerance);
    b = b && lti::read(handler,"areaTolerance",el.areaTolerance);
    b = b && lti::read(handler,"angleTolerance",el.angleTolerance);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }


}
