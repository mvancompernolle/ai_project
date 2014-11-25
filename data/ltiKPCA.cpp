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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiKPCA.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 06.06.02
 * revisions ..: $Id: ltiKPCA.cpp,v 1.12 2006/09/05 10:40:30 ltilib Exp $
 */

#include "ltiObject.h"
#include "ltiMath.h"

#include "ltiEigenSystem.h"
#include "ltiMatrixInversion.h"
#include "ltiMeansFunctor.h"
#include "ltiVarianceFunctor.h"

#include "ltiKPCA.h"
#include "ltiLinearKernel.h"
#include "ltiSigmoidKernel.h"
#include "ltiRadialKernel.h"
#include "ltiPolynomialKernel.h"

// Under MS-VC++ min/max are defined as macros, and we need the lti::min
// lti::max functions.  So let's undefine the silly macros here:
# ifdef _LTI_MSC_6
#   undef min
#   undef max
# endif

#undef _LTI_DEBUG
// #define _LTI_DEBUG 2
#include "ltiDebug.h"

namespace lti {

  struct instancesKPCAKernels {
    static linearKernel     instanceLinearKernel;
    static sigmoidKernel    instanceSigmoidKernel;
    static radialKernel     instanceRadialKernel;
    static polynomialKernel instancePolynomialKernel;
  };

  linearKernel      instancesKPCAKernels::instanceLinearKernel;
  sigmoidKernel     instancesKPCAKernels::instanceSigmoidKernel;
  radialKernel      instancesKPCAKernels::instanceRadialKernel;
  polynomialKernel  instancesKPCAKernels::instancePolynomialKernel;

  // initialise list of possible kernels
  const kernelFunctor<double>* kernelPCA::parameters::kernelArray[] = {
    &instancesKPCAKernels::instanceLinearKernel,
    &instancesKPCAKernels::instanceSigmoidKernel,
    &instancesKPCAKernels::instanceRadialKernel,
    &instancesKPCAKernels::instancePolynomialKernel,
    /*
    new linearKernel,
    new sigmoidKernel,
    new radialKernel,
    new polynomialKernel,
    */
    0
  };

  // initialise kernel factory
  objectFactory< kernelFunctor<double> >
  kernelPCA::parameters::kfa(kernelPCA::parameters::kernelArray);


  // --------------------------------------------------
  // kernelPCA::parameters
  // --------------------------------------------------
  // default constructor
  kernelPCA::parameters::parameters()
    : linearAlgebraFunctor::parameters() {

    resultDim = 3;
    autoDim = false;
    eigen=new jacobi<double>;
    whitening = false;
    relevance=100000.0;

    kernel=new linearKernel;
    destroyKernel=true;
  }

  // copy constructor
  kernelPCA::parameters::parameters(const parameters& other)
    : linearAlgebraFunctor::parameters(), kernel(0),eigen(0)  {
    copy(other);
  }

  // destructor
  kernelPCA::parameters::~parameters() {
    delete eigen;

    if (notNull(kernel) && destroyKernel) {
      delete kernel;
      kernel=0;
    }
  }

  void kernelPCA::parameters::setKernel(const kernelFunctor<double>& k) {
    if (notNull(kernel) && destroyKernel) {
      delete kernel;
    }
    kernel=dynamic_cast<kernelFunctor<double>*>(k.clone());
    destroyKernel=true;
  }

  void kernelPCA::parameters::attachKernel(kernelFunctor<double>* k) {
    if (notNull(kernel) && destroyKernel) {
      delete kernel;
    }
    kernel=k;
    destroyKernel=true;
  }

  void kernelPCA::parameters::useExternalKernel(kernelFunctor<double>* k) {
    if (notNull(kernel) && destroyKernel) {
      delete kernel;
    }
    kernel=k;
    destroyKernel=false;
  }

  kernelFunctor<double>*
  kernelPCA::parameters::createKernel(const std::string& name) const {
    return kfa.newInstance(name);
  }

  // get type name
  const char* kernelPCA::parameters::getTypeName() const {
    return "kernelPCA::parameters";
  }

  // copy member

  kernelPCA::parameters&
  kernelPCA::parameters::copy(const parameters& other) {
#  ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    linearAlgebraFunctor::parameters::copy(other);
#  else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters&
      (functor::parameters::* p_copy)
      (const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
#  endif

    if (kernel != 0 && destroyKernel) {
      delete kernel;
      kernel=0;
    }

    kernel=dynamic_cast<kernelFunctor<double>*>(other.kernel->clone());
    destroyKernel=true;

    delete eigen;
    eigen = 0;

    resultDim=other.resultDim;
    autoDim=other.autoDim;
    whitening = other.whitening;
    relevance=other.relevance;
    eigen=dynamic_cast<eigenSystem<double>*>(other.eigen->clone());

    return *this;
  }

  // alias for copy member
  kernelPCA::parameters&
    kernelPCA::parameters::operator=(const parameters& other) {
    copy(other);

    return *this;
  }

  // clone member
  functor::parameters* kernelPCA::parameters::clone() const {
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
  bool kernelPCA::parameters::write(ioHandler& handler,
                                    const bool complete) const
# else
  bool kernelPCA::parameters::writeMS(ioHandler& handler,
                                      const bool complete) const
# endif
  {
    bool b=true;

    if (complete) {
      b=handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"resultDim",resultDim);
      lti::write(handler,"autoDim",autoDim);
      lti::write(handler,"whitening",whitening);
      lti::write(handler, "relevance", relevance);
      if (kernel != 0) {
        std::string kernelName;
        cn.get(kernel,kernelName);
        b=b && lti::write(handler, "haveKernel", true);
        b=b && lti::write(handler, "kernelType", kernelName);
        b=b && lti::write(handler, "kernelParam", kernel->getParameters());
      } else {
        b=b && lti::write(handler, "haveKernel", false);
      }
    }

#  ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not
    // able to compile...
    b = b && linearAlgebraFunctor::parameters::write(handler,false);
#  else
    bool
      (linearAlgebraFunctor::parameters::* p_writeMS)
      (ioHandler&,const bool) const =
      linearAlgebraFunctor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
#  endif

    if (complete) {
      b=b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool kernelPCA::parameters::write(ioHandler& handler,
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
  bool kernelPCA::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool kernelPCA::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b=true;

    if (complete) {
      b=handler.readBegin();
    }

    if (b) {
      lti::read(handler,"resultDim",resultDim);
      lti::read(handler,"autoDim",autoDim);
      lti::read(handler,"whitening",whitening);
      lti::read(handler,"relevance",relevance);

      std::string tmps;
      bool haveKernel;
      if (notNull(kernel) && destroyKernel) {
        delete kernel;
        kernel=0;
      }
      b=b && lti::read(handler, "haveKernel", haveKernel);
      if (haveKernel) {
        b=b && lti::read(handler, "kernelType", tmps);
        if (tmps != "unknown") {
          kernel=kfa.newInstance(tmps);
          kernelFunctor<double>::parameters* kp=
            dynamic_cast<kernelFunctor<double>::parameters*>
            (kernel->getParameters().clone());
          b=b && lti::read(handler, "kernelParam", *kp);
          kernel->setParameters(*kp);
          destroyKernel=true;
        } else {
          kernel=0;
          destroyKernel=false;
        }
      } else {
        kernel=0;
        destroyKernel=false;
      }
    }

#  ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not
    // able to compile...
    b = b && linearAlgebraFunctor::parameters::read(handler,false);
#  else
    bool (linearAlgebraFunctor::parameters::* p_readMS)
      (ioHandler&,const bool) =
      linearAlgebraFunctor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
#  endif

    if (complete) {
      b=b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool kernelPCA::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // kernelPCA
  // --------------------------------------------------

  // default constructor
  kernelPCA::kernelPCA()
    : linearAlgebraFunctor() {
    parameters tmp;
    setParameters(tmp);
  }

  // copy constructor
  kernelPCA::kernelPCA(const kernelPCA& oth)
    : linearAlgebraFunctor()  {
    copy(oth);
  }

  // destructor
  kernelPCA::~kernelPCA() {
  }

  // returns the name of this type
  const char* kernelPCA::getTypeName() const {
    return "kernelPCA";
  }

  // copy member
  kernelPCA& kernelPCA::copy(const kernelPCA& other) {
    linearAlgebraFunctor::copy(other);

    srcData.copy(other.srcData);
    Kunit.copy(other.Kunit);
    unitK.copy(other.unitK);
    unitKunit = other.unitKunit;
    orderedEigVec.copy(other.orderedEigVec);
    transformMatrix.copy(other.transformMatrix);
    eigValues.copy(other.eigValues);
    whiteScale.copy(other.whiteScale);
    usedDimensionality=other.usedDimensionality;

    return (*this);
  }

  // clone member
  functor* kernelPCA::clone() const {
    return new kernelPCA(*this);
  }

  // return parameters
  const kernelPCA::parameters&
    kernelPCA::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if (isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On copy apply for type dmatrix!
  bool kernelPCA::apply(const dmatrix& src,
                              dmatrix& dest) {
    return computeTransformMatrix(src) && transform(src,dest);
  }


  /*
   * compute kernel matrix
   */
  bool kernelPCA::computeKernelMatrix(const dmatrix& src,
                                            dmatrix& kmat) {
    int i,j(0);
    double val;
    const parameters& param = getParameters();
    const int n = src.rows();
    const kernelFunctor<double>& kernel = *param.kernel;

    // this can be a huge matrix! so catch all exceptions to detect if
    // it is possible to create it
    try {
      kmat.resize(n,n,0,false,false);
    } catch (lti::exception& exp) {
      // some error occured... let's report it
      setStatusString(exp.what());
      return false;
    }

    // compute the diagonal of the matrix
    for (i=0;i<n;++i) {
      kmat.at(i,i) = (kernel.apply(src.getRow(i),src.getRow(i)));
    }

    // and now the non diagonal elements
    for (i=0;i<n;++i) {
      for (j=i+1;j<n;++j) {
        val = (kernel.apply(src.getRow(i),src.getRow(j)));
        kmat.at(i,j) = val;
        kmat.at(j,i) = val;
      }
    }

    // centering in feature space
    // This implements (I-1/n) K (I-1/n), with 1 the one-matrix
    // This is the same as K - 1*K/n - K*1/n + 1*K*1/n^2

    // 1*K contains for each element the sum of the elements of the column
    //     of the element
    // K*1 contains for each element the sum of the elements of the row of
    //     the element

    unitK.resize(n,0,false,true);  // initialize with 0
    Kunit.resize(n,0,false,false); // initialization not required

    double sumRow,e;
    for (i=0;i<n;++i) {
      sumRow = 0;
      for (j=0;j<n;++j) {
        e = kmat.at(i,j);
        sumRow+=e;
        unitK.at(j)+=e;
      }
      Kunit.at(i) = sumRow;
    }

    unitK.divide(n);
    Kunit.divide(n);

    // the mean of all elements of the matrix
    unitKunit = unitK.sumOfElements()/n;

    // now center the data with the equation K - 1*K/n - K*1/n + 1*K*1/n^2
    for (i=0;i<n;++i) {
      for (j=0;j<n;++j) {
        kmat.at(i,j) += (unitKunit - (Kunit.at(i)+unitK.at(j)));
      }
    }

    return true;
  }

  /*
   * compute kernel matrix
   */
  bool kernelPCA::computeTestKernelMatrix(const dmatrix& src,
                                                dmatrix& kmat) const {
    int i,j(0);
    const parameters& param = getParameters();
    const int m = srcData.rows();
    const int n = src.rows();
    const kernelFunctor<double>& kernel = *param.kernel;

    // this can be a huge matrix! so catch all exceptions to detect if
    // it is possible to create it
    try {
      kmat.resize(n,m,0,false,false);
    } catch (lti::exception& exp) {
      // some error occured... let's report it
      setStatusString(exp.what());
      return false;
    }

    // and now the non diagonal elements
    for (i=0;i<n;++i) {
      for (j=0;j<m;++j) {
        kmat.at(i,j) = kernel.apply(src.getRow(i),srcData.getRow(j));
      }
    }

    // centering in feature space
    // This implements (I-1/n) K (I-1/n), with 1 the one-matrix
    // This is the same as K - 1*K/n - K*1/n + 1*K*1/n^2

    // 1*K contains for each element the sum of the elements of the column
    //     of the element
    // K*1 contains for each element the sum of the elements of the row of
    //     the element

    // now center the data with the equation K - 1*K/n - K*1/n + 1*K*1/n^2
    double sumOfRows;
    for (i=0;i<n;++i) {
      sumOfRows = kmat.getRow(i).sumOfElements()/m;
      for (j=0;j<m;++j) {
        kmat.at(i,j) += (unitKunit - (sumOfRows + unitK.at(j)));
      }
    }

    return true;
  }

  /*
   * compute kernel matrix
   */
  bool kernelPCA::computeTestKernelVector(const dvector& src,
                                                dvector& kvct) const {
    int j(0);
    const parameters& param = getParameters();
    const int m = srcData.rows();
    const kernelFunctor<double>& kernel = *param.kernel;

    // this can be a huge matrix! so catch all exceptions to detect if
    // it is possible to create it
    try {
      kvct.resize(m,0,false,false);
    } catch (lti::exception& exp) {
      // some error occured... let's report it
      setStatusString(exp.what());
      return false;
    }

    // and now the non diagonal elements
    for (j=0;j<m;++j) {
      kvct.at(j) = kernel.apply(src,srcData.getRow(j));
    }

    // centering in feature space
    // This implements (I-1/n) K (I-1/n), with 1 the one-matrix
    // This is the same as K - 1*K/n - K*1/n + 1*K*1/n^2

    // 1*K contains for each element the sum of the elements of the column
    //     of the element
    // K*1 contains for each element the sum of the elements of the row of
    //     the element

    // now center the data with the equation K - 1*K/n - K*1/n + 1*K*1/n^2
    double sumOfRows;
    sumOfRows = kvct.sumOfElements()/m;
    for (j=0;j<m;++j) {
      kvct.at(j) += (unitKunit - (sumOfRows + unitK.at(j)));
    }

    return true;
  }

  // On copy apply for type matrix!
  bool kernelPCA::computeTransformMatrix(const dmatrix& src) {
    const parameters& param = getParameters();

    // we need the source data for the data transformation later.
    srcData.copy(src);

    // compute the kernel matrix
    dmatrix kmat;
    computeKernelMatrix(srcData,kmat);

    double eval;
    int i,j;
    const int n = src.rows();

    eigenSystem<double> &eig = *param.eigen;

    _lti_debug("EIG is " << eig.getTypeName() << "\n");

    // set the sort-flag of the eigenvector functor
    if (!eig.getParameters().sort) {
      // need clone because we do not know the actual class of the
      // eigenvector instance
      eigenSystem<double>::parameters* ep=
        dynamic_cast<eigenSystem<double>::parameters*>
        (eig.getParameters().clone());
      ep->sort=true;
      eig.setParameters(*ep);
      delete ep;
    }

    // now compute eigenvectors of the kernel matrix
    if (!eig.apply(kmat,eigValues,orderedEigVec)) {
      _lti_debug("pX");
      setStatusString(eig.getStatusString());
      usedDimensionality=0;
      eigValues.resize(0);
      orderedEigVec.resize(0,0);
      return false;
    }

    int dim;
    // now norm the eigenvectors with the eigenvalues
    for (i=0;i<n;++i) {
      if ((eigValues.at(i)/eigValues.at(0)) <
          std::numeric_limits<double>::epsilon()) {
        eigValues.at(i) = double(0);
        for (j=0;j<orderedEigVec.rows();++j) {
          orderedEigVec.at(j,i) = 0.0;
        }
      } else {
        eval=sqrt(eigValues.at(i));
        for (j=0;j<orderedEigVec.rows();++j) {
          orderedEigVec.at(j,i)/=eval;
        }
      }
    }

    // the number of dimensions of the srdData is not relevant
//     dim = min(checkDim(),orderedEigVec.columns(),srcData.columns());
    dim = min(checkDim(),orderedEigVec.columns());
    if (dim <= 0) {
      setStatusString("Covariance matrix has rank 0");
      return false;
    }

    transformMatrix.copy(orderedEigVec,0,MaxInt32, 0,dim-1);

    if (param.whitening) {
      whiteScale.resize(dim,0.0,false,false);
      whiteScale.fill(eigValues);
      whiteScale.apply(sqrt);
    }

    return true;
  }

  bool kernelPCA::train(const dmatrix& src) {
    return computeTransformMatrix(src);
  }

  /*
   * Transforms a single vector according to a previously computed
   * transformation matrix.
   */
  bool kernelPCA::transform(const dvector& src,
                                  dvector& result) const {

    dvector tmp;
    computeTestKernelVector(src,tmp);
    transformMatrix.leftMultiply(tmp,result);

    if (getParameters().whitening) {
      result.edivide(whiteScale);
    }

    return true;
  }

  /*
   * Transform an entire matrix according to a previously computed
   * transformation matrix. Unfortunately, we must choose a name
   * different from apply.
   * @param src the data matrix
   * @param result the matrix which will receive the transformed data
   * @return a reference to <code>result</code>
   */
  bool kernelPCA::transform(const dmatrix &src,
                                  dmatrix& result) const {

    computeTestKernelMatrix(src,result);
    result.multiply(transformMatrix);

    if (getParameters().whitening) {
      for (int i=0; i<result.rows(); ++i) {
        result.getRow(i).edivide(whiteScale);
      }
    }

    return true;
  }

  bool kernelPCA::transform(dmatrix& srcdest) const {
    dmatrix tmp;
    if (transform(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  }

  bool kernelPCA::apply(dmatrix& srcdest) {
    dmatrix tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  }

  int kernelPCA::checkDim() {
    const parameters& p=getParameters();
    if (p.autoDim) {
      int n=0;
      if (eigValues.size() > 0) {
        lti::dvector::const_iterator i = eigValues.begin();
        // set the lowest accepted eigenvalue to the largest eigenvalue
        // divided by 1e6. This works only, if eigValues.at(0) is really
        // the maximum, i.e. if the eigenvalues are sorted descendingly
        // maybe 1e6 could be enlarged to something around 1e10
        double evLimit=abs(*i)/p.relevance;
        while (i != eigValues.end()) {
          if (abs(*i) > evLimit) {
            n++;
          }
          i++;
        }
        usedDimensionality=n;
      } else {
        usedDimensionality=0;
      }
    } else {
      usedDimensionality = p.resultDim;
    }
    return usedDimensionality;
  }

  void kernelPCA::setDimension(int n) {
    parameters p(getParameters());
    p.resultDim=n;
    setParameters(p);
  }

  bool kernelPCA::updateParameters() {
    bool boo=true;

    const parameters& p=getParameters();
    int dim = (p.autoDim) ? usedDimensionality : p.resultDim;

    if (dim <= 0) {
      dim=orderedEigVec.columns();
    }

    if (dim != transformMatrix.columns()) {
      transformMatrix.copy(orderedEigVec,0,MaxInt32,0,dim-1);

      if (p.whitening) {
        whiteScale.resize(dim,0.0,false,false);
        whiteScale.fill(eigValues);
        whiteScale.apply(sqrt);
      }
    }

    return boo;
  }

  bool kernelPCA::getEigenVectors(dmatrix& result) const {
    result.copy(orderedEigVec);
    return true;
  }

  bool kernelPCA::getEigenValues(dvector& result) const {
    result.copy(eigValues);
    return true;
  }

  const dmatrix&
  kernelPCA::getEigenVectors() const {
    return orderedEigVec;
  }

  const dvector&
  kernelPCA::getEigenValues() const {
    return eigValues;
  }

  bool kernelPCA::read(ioHandler& handler,
                                    const bool complete) {
    bool b=true;

    if (complete) {
      b=handler.readBegin();
    }

    if (b) {
      parameters param;
      lti::read(handler,"parameters",param);
      setParameters(param);

      lti::read(handler,"srcData",srcData);
      lti::read(handler,"Kunit",Kunit);
      lti::read(handler,"unitK",unitK);
      lti::read(handler,"unitKunit",unitKunit);
      lti::read(handler,"orderedEigVec",orderedEigVec);
      lti::read(handler,"eigValues",eigValues);
      lti::read(handler,"usedDimensionality",usedDimensionality);

      int dim=min(checkDim(),orderedEigVec.columns());

      transformMatrix.copy(orderedEigVec,0,MaxInt32,0,dim-1);

      if (getParameters().whitening) {
        whiteScale.resize(dim,0.0,false,false);
        whiteScale.fill(eigValues);
        whiteScale.apply(sqrt);
      }

      if (complete) {
        b=b && handler.readEnd();
      }
    }

    return b;
  }

  bool kernelPCA::write(ioHandler& handler,
                        const bool complete) const {
    bool b=true;

    if (complete) {
      b=handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"parameters",getParameters());

      lti::write(handler,"srcData",srcData);
      lti::write(handler,"Kunit",Kunit);
      lti::write(handler,"unitK",unitK);
      lti::write(handler,"unitKunit",unitKunit);
      lti::write(handler,"orderedEigVec",orderedEigVec);
      lti::write(handler,"eigValues",eigValues);
      lti::write(handler,"usedDimensionality",usedDimensionality);

      if (complete) {
        b=b && handler.writeEnd();
      }
    }
    return b;
  }

  bool read(ioHandler& handler,
            kernelPCA& kpca,
            const bool complete) {
    return kpca.read(handler,complete);
  }


  bool write(ioHandler& handler,
             const kernelPCA& kpca,
             const bool complete) {
    return kpca.write(handler,complete);
  }


}

#include "ltiUndebug.h"
