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
 * file .......: ltiMultivariateGaussian.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 14.5.2002
 * revisions ..: $Id: ltiMultivariateGaussian.cpp,v 1.8 2006/09/05 10:41:03 ltilib Exp $
 */

//TODO: include files
#include "ltiMultivariateGaussian.h"

namespace lti {
  // --------------------------------------------------
  // multivariateGaussian::parameters
  // --------------------------------------------------

  // default constructor
  multivariateGaussian::parameters::parameters()
    : continuousRandomDistribution::parameters() {

    centre = dvector();
    covarianceMatrix = dmatrix();
  }

  // copy constructor
  multivariateGaussian::parameters::parameters(const parameters& other)
    : continuousRandomDistribution::parameters()  {
    copy(other);
  }

  // destructor
  multivariateGaussian::parameters::~parameters() {
  }

  // get type name
  const char* multivariateGaussian::parameters::getTypeName() const {
    return "multivariateGaussian::parameters";
  }

  // copy member

  multivariateGaussian::parameters&
    multivariateGaussian::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    continuousRandomDistribution::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    continuousRandomDistribution::parameters& (continuousRandomDistribution::parameters::* p_copy)
      (const continuousRandomDistribution::parameters&) =
      continuousRandomDistribution::parameters::copy;
    (this->*p_copy)(other);
# endif


      centre = other.centre;
      covarianceMatrix = other.covarianceMatrix;

    return *this;
  }

  // alias for copy member
  multivariateGaussian::parameters&
    multivariateGaussian::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* multivariateGaussian::parameters::clone() const {
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
  bool multivariateGaussian::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool multivariateGaussian::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"centre",centre);
      lti::write(handler,"covarianceMatrix",covarianceMatrix);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && continuousRandomDistribution::parameters::write(handler,false);
# else
    bool (continuousRandomDistribution::parameters::* p_writeMS)(ioHandler&,const bool) const =
      continuousRandomDistribution::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool multivariateGaussian::parameters::write(ioHandler& handler,
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
  bool multivariateGaussian::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool multivariateGaussian::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"centre",centre);
      lti::read(handler,"covarianceMatrix",covarianceMatrix);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && continuousRandomDistribution::parameters::read(handler,false);
# else
    bool (continuousRandomDistribution::parameters::* p_readMS)(ioHandler&,const bool) =
      continuousRandomDistribution::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool multivariateGaussian::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // multivariateGaussian
  // --------------------------------------------------

  // default constructor
  multivariateGaussian::multivariateGaussian()
    : continuousRandomDistribution(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

    gaussFunc = new gaussianDistribution();
    eigenFunc = new jacobi<double>();
    detFunc = new luDecomposition<double>();

  }

  // copy constructor
  multivariateGaussian::multivariateGaussian(const multivariateGaussian& other)
    : continuousRandomDistribution()  {
    copy(other);
  }

  // destructor
  multivariateGaussian::~multivariateGaussian() {
      delete gaussFunc;
      delete eigenFunc;
      delete detFunc;
  }

  // returns the name of this type
  const char* multivariateGaussian::getTypeName() const {
    return "multivariateGaussian";
  }

  // copy member
  multivariateGaussian&
    multivariateGaussian::copy(const multivariateGaussian& other) {
      continuousRandomDistribution::copy(other);

    return (*this);
  }

  // alias for copy member
  multivariateGaussian&
    multivariateGaussian::operator=(const multivariateGaussian& other) {
    return (copy(other));
  }


  // clone member
  functor* multivariateGaussian::clone() const {
    return new multivariateGaussian(*this);
  }

  // return parameters
  const multivariateGaussian::parameters&
    multivariateGaussian::getParameters() const {
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


 bool multivariateGaussian::apply(const dvector& mean,
                                  const dmatrix& covarianceMatrix,
                                  dvector& dest) const {
    bool ok=true;
    int dimensionality=mean.size();
    int i;
    double det=detFunc->det(covarianceMatrix);
    if (det <= 0) {
      setStatusString("the determinant of your covariance Matrix is <= 0");
      return false;
    }
    dvector tmp(dimensionality);
    dmatrix tmpMatrix(dimensionality,dimensionality);
    dvector eigenValues(dimensionality);
    dmatrix eigenVectors(dimensionality,dimensionality);
    // if the dest vector has a wrong dimensionality, fit it.
    if (dest.size()!=dimensionality) {
      dest.resize(dimensionality,0.0,false);
    }
    // get eigenvalues and eigenvectors
    eigenFunc->apply(covarianceMatrix, eigenValues, eigenVectors);
    // draw one point from the distribution
    for (i=0; i<dimensionality; i++) {
        tmp[i]=gaussFunc->draw(0,sqrt(eigenValues[i]));
    }
    // copy covarianceMatrix
    tmpMatrix.copy(eigenVectors);
    dest=tmpMatrix.multiply(tmp);
    dest.add(mean);
    return ok;
  };

  bool multivariateGaussian::apply(const dvector& mean,
                                   const dmatrix& covarianceMatrix,
                                   const int& number,
                                   dmatrix& dest) const {
    bool ok=true;
    int i,j;
    int dim=mean.size();
    // if determinant is <= 0, the computation is not possible
    double det=detFunc->det(covarianceMatrix);
    dvector tmp(dim);
    dvector tVec(dim);
    dmatrix tmpMatrix(dim,dim);
    dvector eigenValues(dim);
    dmatrix eigenVectors(dim,dim);
    if (det <= 0) {
      setStatusString("the determinant of your covariance Matrix is <= 0");
      return false;
    }
    // if the dest vector has a wrong dimensionality, fit it.
    if (dest.columns()!=dim) {
      dest.resize(number,dim,0.0,false);
    }
    // get eigenvalues and eigenvectors
    eigenFunc->apply(covarianceMatrix, eigenValues, eigenVectors);

    for (i=0; i<number; i++) {
      for (j=0; j<dim; j++) {
        tmp[j]=gaussFunc->draw(0,sqrt(eigenValues[j]));
      }
      // copy covarianceMatrix
      tmpMatrix.copy(eigenVectors);
      tVec=tmpMatrix.multiply(tmp);
      tVec.add(mean);
      dest.setRow(i,tVec);
    }
    return ok;
  };

 bool multivariateGaussian::apply(std::list<dvector>& mean,
                                  std::list<dmatrix>& covarianceMatrix,
                                  std::list<int>& numbers,
                                  std::list<dmatrix>& dest) const {
     std::list<dvector>::iterator meanIter;
     std::list<dmatrix>::iterator varIter;
     std::list<int>::iterator nbIter;
     dmatrix tmp;
     varIter = covarianceMatrix.begin();
     nbIter = numbers.begin();
     for(meanIter = mean.begin(); meanIter != mean.end(); meanIter++) {
         tmp.resize((*nbIter),(*meanIter).size());
         apply(*meanIter,*varIter,*nbIter,tmp);
         dest.push_back(tmp);
         varIter++;
         nbIter++;
     }
     return true;
 };

 bool multivariateGaussian::apply(std::list<dvector>& mean,
                                  std::list<dmatrix>& covarianceMatrix,
                                  std::list<int>& numbers,
                                  dmatrix& dest) const {
   std::list<dvector>::iterator meanIt;
   std::list<dmatrix>::iterator dmatrixIt;
   std::list<int>::iterator nbIt;
   std::list<dmatrix> tmp2;
   dmatrix tmp;
   int total=0;
   dmatrixIt = covarianceMatrix.begin();
   nbIt = numbers.begin();
   for(meanIt = mean.begin(); meanIt != mean.end(); meanIt++) {
     tmp.resize((*nbIt),(*meanIt).size());
     apply(*meanIt,*dmatrixIt,*nbIt,tmp);
     tmp2.push_back(tmp);
     total+=(*nbIt);
     dmatrixIt++;
     nbIt++;
   }
   dest.resize(total,(*mean.begin()).size());
   int counter=0;
   int i;
   for (dmatrixIt = tmp2.begin(); dmatrixIt != tmp2.end(); dmatrixIt++) {
     for (i = 0; i < (*dmatrixIt).rows(); i++) {
       dest.setRow(counter,(*dmatrixIt).getRow(i));
       counter++;
     }
   }
   return true;

 };


    /**
     * only reimplemented because method is virtual in parent
     * class.
     */
  double multivariateGaussian::draw() const {
      return 0;
  };


  dmatrix multivariateGaussian::draw(const int& number) const {
    const parameters& p=getParameters();
    bool ok;
    int dimensionality=p.centre.size();
    dmatrix tmp(number,dimensionality);
    ok=apply(p.centre, p.covarianceMatrix,number,tmp);
    if (!ok) tmp.resize(0,0);
    return tmp;
  };

}
