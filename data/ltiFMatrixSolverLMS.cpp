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
 * file .......: fundamentalMatrixSolverLMS.cpp
 * authors ....: Frederik Lange
 * organization: LTI, RWTH Aachen
 * creation ...: 7.1.2002
 * revisions ..: $Id: ltiFMatrixSolverLMS.cpp,v 1.10 2006/09/05 10:11:16 ltilib Exp $
 */

#include "ltiFMatrixSolverLMS.h"
#include "ltiPointList.h"
#include "ltiSVD.h"
#include "ltiSort.h"
#include <limits>

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1 // if you want to see the debug info
#include "ltiDebug.h"

namespace lti {

  // 2D-vector definition for usage in distance computation
  // between epipolar line and point (homographic)
  template<class valType>
  class vec2D {
  public:
    vec2D(){};
    ~vec2D(){};
    vec2D(valType x, valType y, valType w = 1)
    {
      v[0] = x;
      v[1] = y;
      v[2] = 1.0 / w;
    }

    valType scalarProd(vec2D other)
    {
      valType ret = 0;
      int i;
      for (i = 0; i < 2; i++)
	{
	  ret += (v[2]*v[i])*(other.v[2]*other.v[i]);
	}
      return ret;

    }

    vec2D add(vec2D other)
    {
      vec2D ret;
      ret.v[0] = (v[2]*v[0])+(other.v[2]*other.v[0]);
      ret.v[1] = (v[2]*v[1])+(other.v[2]*other.v[1]);
      ret.v[2] = 1;
      return ret;
    }

    vec2D sub(vec2D other)
    {
      vec2D ret;
      ret.v[0] = (v[2]*v[0])-(other.v[2]*other.v[0]);
      ret.v[1] = (v[2]*v[1])-(other.v[2]*other.v[1]);
      ret.v[2] = 1;
      return ret;
    }

    void scale(valType s)
    {
      v[0] *= s;
      v[1] *= s;
      //v[2] *= s;
    }

    valType length()
    {
      valType ret;
      ret = (v[0]*v[0]*v[2]*v[2])+(v[1]*v[1]*v[2]*v[2]);
      ret = sqrt(ret);
      return ret;
    }


    valType getX(){return v[2]*v[0];};
    valType getY(){return v[2]*v[1];};

    void print() {
      std::cout << "2DVec: " << v[0] << " " << v[1] << " " << v[2] << std::endl;
    }


    valType
    v[3];


  };

  typedef vec2D<double> v2;



  // --------------------------------------------------
  // fundamentalMatrixSolverLMS::parameters
  // --------------------------------------------------

  // default constructor
  fundamentalMatrixSolverLMS::parameters::parameters()
    : transform::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    numTrials = int(400);
    threshold = double(0.000001);    // same as not actiavated
  }

  // copy constructor
  fundamentalMatrixSolverLMS::parameters::parameters(const parameters& other)
    : transform::parameters()  {
    copy(other);
  }

  // destructor
  fundamentalMatrixSolverLMS::parameters::~parameters() {
  }

  // get type name
  const char* fundamentalMatrixSolverLMS::parameters::getTypeName() const {
    return "fundamentalMatrixSolverLMS::parameters";
  }

  // copy member

  fundamentalMatrixSolverLMS::parameters&
  fundamentalMatrixSolverLMS::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    transform::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    transform::parameters& (transform::parameters::* p_copy)
      (const transform::parameters&) =
      transform::parameters::copy;
    (this->*p_copy)(other);
# endif


    numTrials = other.numTrials;
    threshold = other.threshold;

    return *this;
  }

  // alias for copy member
  fundamentalMatrixSolverLMS::parameters&
  fundamentalMatrixSolverLMS::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* fundamentalMatrixSolverLMS::parameters::clone() const {
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
  bool fundamentalMatrixSolverLMS::parameters::write(ioHandler& handler,
					   const bool complete) const
# else
    bool fundamentalMatrixSolverLMS::parameters::writeMS(ioHandler& handler,
					       const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"numTrials",numTrials);
      lti::write(handler,"threshold",threshold);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::write(handler,false);
# else
    bool (transform::parameters::* p_writeMS)(ioHandler&,const bool) const =
      transform::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool fundamentalMatrixSolverLMS::parameters::write(ioHandler& handler,
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
  bool fundamentalMatrixSolverLMS::parameters::read(ioHandler& handler,
					  const bool complete)
# else
    bool fundamentalMatrixSolverLMS::parameters::readMS(ioHandler& handler,
					      const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"numTrials",numTrials);
      lti::read(handler,"threshold",threshold);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::read(handler,false);
# else
    bool (transform::parameters::* p_readMS)(ioHandler&,const bool) =
      transform::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool fundamentalMatrixSolverLMS::parameters::read(ioHandler& handler,
					  const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // fundamentalMatrixSolverLMS
  // --------------------------------------------------

  // default constructor
  fundamentalMatrixSolverLMS::fundamentalMatrixSolverLMS()
    : transform(){

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
  fundamentalMatrixSolverLMS::fundamentalMatrixSolverLMS(const fundamentalMatrixSolverLMS& other)
    : transform()  {
    copy(other);
  }

  // destructor
  fundamentalMatrixSolverLMS::~fundamentalMatrixSolverLMS() {
  }

  // returns the name of this type
  const char* fundamentalMatrixSolverLMS::getTypeName() const {
    return "fundamentalMatrixSolverLMS";
  }

  // copy member
  fundamentalMatrixSolverLMS&
  fundamentalMatrixSolverLMS::copy(const fundamentalMatrixSolverLMS& other) {
    transform::copy(other);

    return (*this);
  }

  // alias for copy member
  fundamentalMatrixSolverLMS&
  fundamentalMatrixSolverLMS::operator=(const fundamentalMatrixSolverLMS& other) {
    return (copy(other));
  }


  // clone member
  functor* fundamentalMatrixSolverLMS::clone() const {
    return new fundamentalMatrixSolverLMS(*this);
  }

  // return parameters
  const fundamentalMatrixSolverLMS::parameters&
  fundamentalMatrixSolverLMS::getParameters() const {
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


  // integer version of apply
  bool fundamentalMatrixSolverLMS::apply(const pointList& l1,
			       const pointList& l2,
			       matrix<double>& fMatrix) const {

    tpointList<double> tl1, tl2;

    tpointList<int>::const_iterator i1,i2;

    if (l1.size() != l2.size()) {
      setStatusString("Point lists dont have the same size");
      return false;
    }

    tpoint<double> tmpPt;

    i2 = l2.begin();
    for (i1 = l1.begin(); i1 != l1.end(); i1++) {
      tmpPt.x = (double)(*i1).x;
      tmpPt.y = (double)(*i1).y;

      tl1.push_back(tmpPt);

      tmpPt.x = (double)(*i2).x;
      tmpPt.y = (double)(*i2).y;

      tl2.push_back(tmpPt);

      i2++;
    }


    return apply(tl1,tl2, fMatrix);  // use the floatingpoint apply

  }



  // floatingpoint version of apply
  bool fundamentalMatrixSolverLMS::apply(const tpointList<double>& l1,
			       const tpointList<double>& l2,
			       matrix<double>& fMatrix) const {
    int i,j,k;


  if (l1.size() != l2.size()) {
      setStatusString("Point lists dont have the same size");
      return false;
  }


#ifdef _LTI_DEBUG
    sort<int> sorterInt;
    _lti_debug("start fundSolver" << std::endl);
#endif

    parameters params = getParameters();

    sort<double> sorter;

    // holds the points in a N x 9 matrix
    matrix<double> pointMatrix;

    matrix<double> momentMatrix;
    matrix<double> fundMatrix;
    matrix<double> fundMatrixTmp;
    vector<int> select;
    vector<double> residual;

    buildPointMatrix(l1, l2, pointMatrix);

    int numSamples = pointMatrix.rows();

    double minRes = 99999.9;
    double res;

    // eight points are enough to estimate the matrix maybe we use
    // more and throw some out with the leverage factor (influence of
    // a sample on the result, later)
    static const int numS(8);

    _lti_debug("Num Samples: " << numSamples << endl);

    for (i = 0; i < params.numTrials; i++) {

      buildMomentMatrixRandom(pointMatrix, numS, momentMatrix, select);

#ifdef _LTI_DEBUG
      sorterInt.apply(select);
      _lti_debug("Selected points:" << endl);
      for (j = 0; j < numS; j++) {
	_lti_debug(select[j] << " ");
      }
      _lti_debug(endl);
#endif

      calcFundMat(momentMatrix, fundMatrixTmp);
      calcResidual(pointMatrix, fundMatrixTmp, residual);
      sorter.apply(residual);
      res = residual[numSamples/2];    // the median of the residuals

      _lti_debug("Iteration " << i << ": median of residual:" << res << endl);

      if (res < minRes) {    // swaping the result

	minRes = res;
	fundMatrix = fundMatrixTmp;

	if (minRes < params.threshold) {
	  break;      // limit is reached, no further trials needed
	}
      }

    }


    // resulting matrix may not be singular, so make it singular by
    // using svd and set smallest eigenvalue to zero
    // the fundamental matrix must be of rank 2 !!!
    singularValueDecomp<double> decomp;
    matrix<double> u,v;
    vector<double> w;

    decomp.apply(fundMatrix,u,w,v);

    double ssv = w[0];

    j = 0;
    for (i = 0;i < 3; i++) {
      if (w[i] <= ssv) {
	ssv = w[i];
	j = i;
      }
    }

    w[j] = 0;  // for det(..) = 0

    matrix<double> tmp;
    tmp.resize(3,3,0);

    tmp[0][0] = w[0];
    tmp[1][1] = w[1];
    tmp[2][2] = w[2];

    matrix<double> tmp2;
    tmp2.resize(3,3,0);

    for (i = 0; i < 3; i++) {
      for (j = 0; j < 3; j++) {
	tmp2[i][j] = 0.0;
	for (k = 0; k < 3; k++) {
	  // recognize the transposed v
	  tmp2[i][j] += tmp[i][k] * v[j][k];
	}
      }
    }

    matrix<double> tmp3;
    tmp3.resize(3,3,0);

    for (i = 0; i < 3; i++) {
      for (j = 0; j < 3; j++) {
	tmp3[i][j] = 0.0;
	for (k = 0; k < 3; k++) {
	  tmp3[i][j] += u[i][k] * tmp2[k][j];
	}
      }
    }


    fundMatrix = tmp3;
    fMatrix = fundMatrix;

    _lti_debug("end fundSolver" << endl);
    return true;

  }


  void fundamentalMatrixSolverLMS::buildPointMatrix(
                                        const tpointList<double>& l1,
					                              const tpointList<double>& l2,
								                        matrix<double> &pointMatrix) const {
    int i;

    tpointList<double>::const_iterator
      iterA,
      iterB;

    iterA = l1.begin();
    iterB = l2.begin();
    i = 0;


    pointMatrix.resize(l1.size(), 9);    // we have <numpoints>x9

    i = 0;
    while (iterA != l1.end()) {
  	  pointMatrix[i][0] = (*iterA).x * (*iterB).x;  //x*x'
	    pointMatrix[i][1] = (*iterA).x * (*iterB).y;  //x*y'
	    pointMatrix[i][2] = (*iterA).x;               //x
	    pointMatrix[i][3] = (*iterA).y * (*iterB).x;  //y*x'
	    pointMatrix[i][4] = (*iterA).y * (*iterB).y;  //y*y'
	    pointMatrix[i][5] = (*iterA).y;               //y
	    pointMatrix[i][6] =              (*iterB).x;  //  x'
	    pointMatrix[i][7] =              (*iterB).y;  //  y'
	    pointMatrix[i][8] = 1;                         // 1

	    i++;
	    iterA++;
	    iterB++;
    }
  }

  // the standart method for using all samples to build the moment-matrix
  void fundamentalMatrixSolverLMS::buildMomentMatrix(  const matrix<double> &pointMatrix,
                                                       matrix<double> &momentMatrix) const {
    int i,j,k;

    momentMatrix.resize(9, 9,0);    // the moment matrix, initialized to zero


    // m = pointmat * pointmat^T
    for (i = 0; i < 9; i++) {
	    for (j = 0; j < 9; j++) {
	      momentMatrix[i][j] = 0.0;
	      for (k = 0; k < pointMatrix.rows(); k++) {
		      momentMatrix[i][j] += pointMatrix[k][i] * pointMatrix[k][j];  // ATTENTION!!!
	      }
	    }
    }


    _lti_debug("Buld moment matrix end" << endl);
  }


  // building the moment-matrix with "numsamples" random samples
  void fundamentalMatrixSolverLMS::buildMomentMatrixRandom( const matrix<double> &pointMatrix,
						  const int numSamples,
						  matrix<double> &momentMatrix,
						  vector<int> &select) const {
    int i,j,k;

    momentMatrix.resize(9, 9,0);    // the moment matrix, initialized to zero

    select.resize(numSamples);

    // choose the samples randomly and ensure they are not two times in the output
    i = 0;
    while(i < numSamples)
      {

	select[i] = (int) ( (((double)rand()/RAND_MAX)) * ((double)pointMatrix.rows()) );

	k = 0;
	for (j = 0; j < i; j++ )
	  {
	    if( select[i] == select[j] )
	      {
		k = 1;
		break;
	      }
	  }
	if (k == 0)
	  {
	    i++;
	  }
      }  // while

    int ind;
    // m = pointmat * pointmat^T
    for (i = 0; i < 9; i++)
      {
	for (j = 0; j < 9; j++)
	  {
	    momentMatrix[i][j] = 0.0;
	    for (k = 0; k < numSamples; k++)
	      {
		ind = select[k];
		momentMatrix[i][j] += pointMatrix[ind][i] * pointMatrix[ind][j];
	      }
	  }
      }
    _lti_debug("Build moment matrix with randomsamples end" << endl);
  }



  // uses standart singular value decomposition
  void fundamentalMatrixSolverLMS::calcFundMat(const matrix<double> &momentMatrix, matrix<double> &fundMatrix) const {

    singularValueDecomp<double> decomp;
    /*
      w: eigenvalues
      u: eigenvectors
      v: left hand sigular vectors

    */

    //_lti_debug("calc eigenvalues" << endl);
    matrix<double> u,v;
    vector<double> w;

    fundMatrix.resize(3,3,1);



    // standart singular value decomposition
    if (!decomp.apply(momentMatrix,u,w,v)) {
	    setStatusString("error in calc of svd!");
	    return;
    }

    int i;
#ifdef _LTI_DEBUG

    _lti_debug("the eigenvalues are:" << endl);
    for (i = 0; i < w.size() ;i++) {
    	_lti_debug(i << " : " << w[i] << endl);
    }

    int j;

    _lti_debug("the eigenvectors are:" << endl);
    for (i = 0; i < u.rows() ;i++) {
    	for (j = 0; j < u.columns(); j++) {
	      _lti_debug(" " << u[j][i]);
	    }
	    _lti_debug(endl);
    }
#endif

    // the fundamental matrix is one of the eigenvectors
    // the u-matrix is transposed, so use the last column

    for (i = 0; i < 9 ;i++) {
    	fundMatrix[i%3][i/3] = u[i][u.rows()-1];
    }
  }

  double fundamentalMatrixSolverLMS::calcResidual(const matrix<double> &pointMatrix,
					const matrix<double> &fundMatrix,
					vector<double> &resid) const {

    resid.resize(pointMatrix.rows());

    int i;
//    double la,lb,lc,da,db;
    double ret = 0;

    for (i = 0; i < pointMatrix.rows(); i++) {
      int k,l;
      double tmpVec[3];
      double vecA[3], vecB[3];

      // get the points back out of the pointMatrix
      vecA[0] = pointMatrix[i][2];
      vecA[1] = pointMatrix[i][5];
      vecA[2] = 1;
      vecB[0] = pointMatrix[i][6];
      vecB[1] = pointMatrix[i][7];
      vecB[2] = 1;

      // actaully we calculate the geometrical distance between the
      // line and the point
      for (k = 0; k < 3; k++) {
      	tmpVec[k] = 0;
	      for (l = 0; l < 3; l++)  {
	        tmpVec[k] += fundMatrix[k][l] * vecA[l];  // tmpVec is the line
	      }
      }

      v2 x(vecB[0], vecB[1]); // the point
      v2 p(0.0, - tmpVec[2]/tmpVec[1],1.0);  // at y-axis, startpoint of line
      v2 d(1.0,(-tmpVec[0])/tmpVec[1],1.0);  // direction of line

      v2 s(-d.v[1],d.v[0],d.v[2]);   // orthogonal

      v2 z;
      z = p.sub(x); // p - x


      double dX_1;
      if (fabs(d.getX()) >= std::numeric_limits<double>::epsilon()) {
      	dX_1 = 1.0 / d.getX();
      }
      else {
      	dX_1 = std::numeric_limits<double>::max();
      }

      double tmpI;
      tmpI = (s.getX()*d.getY()*dX_1) - s.getY();

      double tmpR;
      tmpR = z.getY() - (z.getX()*d.getY()*dX_1);

      double tmpI_1;
      double b;

      if (fabs(tmpI) >= std::numeric_limits<double>::epsilon())	{
      	tmpI_1 = 1.0 / tmpI;
      	b = tmpR*tmpI_1;
      }
      else {
      	tmpI_1 = std::numeric_limits<double>::max();
      	b = tmpR / std::numeric_limits<double>::max();
      }

  //    double a = (z.getX() + b*s.getX())*dX_1;

      s.scale(b);
      double len = s.length();

      resid[i] = len;

      ret += resid[i];
    }
    return ret;
  }

}
