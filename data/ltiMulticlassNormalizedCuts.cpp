/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiMulticlassNormalizedCuts.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 10.10.2003
 * revisions ..: $Id: ltiMulticlassNormalizedCuts.cpp,v 1.8 2006/09/05 10:23:04 ltilib Exp $
 */

#include "ltiConfig.h"
#include "ltiMulticlassNormalizedCuts.h"
#include "ltiUniformDist.h"
#include <limits>

#ifdef HAVE_LAPACK
// #if 0
#define _LTI_FAST_LA
#else
#undef _LTI_FAST_LA
#endif

#ifdef _LTI_FAST_LA
#include "ltiFastEigenSystem.h"
#include "ltiFastSVD.h"
#else
#include "ltiEigenSystem.h"
#include "ltiSVD.h"
#endif

namespace lti {
  // --------------------------------------------------
  // multiclassNormalizedCuts::parameters
  // --------------------------------------------------

  // default constructor
  multiclassNormalizedCuts::parameters::parameters()
    : functor::parameters() {
    
    k = int(2);
  }

  // copy constructor
  multiclassNormalizedCuts::parameters::parameters(const parameters& other)
    : functor::parameters() {
    copy(other);
  }

  // destructor
  multiclassNormalizedCuts::parameters::~parameters() {
  }

  // get type name
  const char* multiclassNormalizedCuts::parameters::getTypeName() const {
    return "multiclassNormalizedCuts::parameters";
  }

  // copy member

  multiclassNormalizedCuts::parameters&
    multiclassNormalizedCuts::parameters::copy(const parameters& other) {
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
    
    k = other.k;

    return *this;
  }

  // alias for copy member
  multiclassNormalizedCuts::parameters&
    multiclassNormalizedCuts::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* multiclassNormalizedCuts::parameters::clone() const {
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
  bool multiclassNormalizedCuts::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool multiclassNormalizedCuts::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"k",k);
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
  bool multiclassNormalizedCuts::parameters::write(ioHandler& handler,
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
  bool multiclassNormalizedCuts::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool multiclassNormalizedCuts::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"k",k);
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
  bool multiclassNormalizedCuts::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // multiclassNormalizedCuts
  // --------------------------------------------------

  // default constructor
  multiclassNormalizedCuts::multiclassNormalizedCuts()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  multiclassNormalizedCuts::multiclassNormalizedCuts(const parameters& par)
    : functor() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  multiclassNormalizedCuts::multiclassNormalizedCuts(const multiclassNormalizedCuts& other)
    : functor() {
    copy(other);
  }

  // destructor
  multiclassNormalizedCuts::~multiclassNormalizedCuts() {
  }

  // returns the name of this type
  const char* multiclassNormalizedCuts::getTypeName() const {
    return "multiclassNormalizedCuts";
  }

  // copy member
  multiclassNormalizedCuts&
  multiclassNormalizedCuts::copy(const multiclassNormalizedCuts& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  multiclassNormalizedCuts&
  multiclassNormalizedCuts::operator=(const multiclassNormalizedCuts& other) {
    return (copy(other));
  }


  // clone member
  functor* multiclassNormalizedCuts::clone() const {
    return new multiclassNormalizedCuts(*this);
  }

  // return parameters
  const multiclassNormalizedCuts::parameters&
    multiclassNormalizedCuts::getParameters() const {
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
   * Normalized cut method
   */
  bool multiclassNormalizedCuts::kWayNormalizedCut(const dmatrix& weights,
                                                   const dvector& D,
                                                   const int k,
                                                         ivector& Xstar) const{

    if (D.size() != weights.rows()) {
      setStatusString("Invalid degrees vector.  D.size() != W.rows()");
      return false;
    }

    if (k<2) {
      setStatusString("Two few number of classes (must be greater 2)");
      return false;
    }

    const int n = (static_cast<int>(weights.rows()));
    int i,j;

    if (k>=n) {
      Xstar.resize(n,0,false,false);
      // partition is not necessary.  More classes than nodes!
      for (i=0;i<n;++i) {
        Xstar.at(i)=i;
      }
      return true;
    }

    // WARNING:
    // Please note that the implementation is optimized!
    // Don't try to find a 1 to 1 implementation of the math in the original
    // paper, because that stuff is better to be optimized like here to avoid
    // matrix multiplications (which are n^3).      

    // Degree and Weight matrices (notation kept from Shi and Malik's paper) 
    dvector isqrtD;
    dmatrix W(weights);

    // Step 1. Not required, because degree comes from outside

    double d;

    isqrtD.resize(n,0.0,false,false);

    // compute D^-0.5: easy because it is a diagonal matrix!
    for (i=0;i<n;++i) {
      d = D.at(i);
      if (d > 0.0) {
        isqrtD.at(i) = 1.0/sqrt(d);
      } else {
        isqrtD.at(i) = 0.0;
      }
    }
    
    // Step 2. compute eigensolution Zstar 
    //
    // now, we need the eigensolution for D^-0.5 W D^-0.5 V = V S
    // Remember:
    //   a matrix multiplied by a diagonal matrix => each _column_ scaled by
    //                                               corresponding diag value
    //   a diagonal matrix multiplied by a matrix => each _row_ scaled by
    //                                               corresponding diag value
    // much faster if we do it here in a hurry instead of calling the matrix
    // multiplication:
    
    for (j=0;j<W.rows();++j) {
      for (i=0;i<W.columns();++i) {
        W.at(j,i)*=(isqrtD.at(i)*isqrtD.at(j));
      }
    }

    dmatrix Vbar,Zstar,XstarT;

#ifdef _LTI_FAST_LA    
    // fast methods using LAPACK
    fastSVD<double>::parameters svdPar;
    svdPar.transposeU = true;
    fastSVD<double> svd(svdPar);

    fastEigenSystem<double>::parameters esPar;
    esPar.dimensions=k;
    fastEigenSystem<double> eigen(esPar);

#else
    // hmm, no LAPACK found :-( so use the slow methods:
    singularValueDecomp<double>::parameters svdPar;
    svdPar.sort = true;
    svdPar.transposeU = true;
    singularValueDecomp<double> svd(svdPar);

    jacobi<double>::parameters esPar;
    esPar.sort=true;
    esPar.dimensions=k;
    jacobi<double> eigen(esPar);
#endif

    // compute the first k eigenvectors and eigenvalues
    dvector eigVals;

    if (!eigen.apply(W,eigVals,Vbar)) {
      setStatusString("Eigensolution: ");
      appendStatusString(eigen.getStatusString());
      Xstar.clear();
      return false;
    }

    // following steps are not really necessary, since in Step 3. the rows
    // will be normalized, and thus, it is not necessary to normalize them 
    // here at all (They stay as comments for completeness)

    //     for (i=0;i<Vbar.rows();++i) {
    //       Vbar.getRow(i).multiply(isqrtD.at(i));
    //     }

    Vbar.detach(Zstar);  // ok,ok, to copy is not necessary, but I want to 
                         // keep the paper's notation a little bit!

    // Step 3. Normalize Zstar -> XstarT
    //
    // normalize Zstar by Xstar~=Diag(diag^0.5(Zstar * Zstar'))Zstar
    // which means normalize the magnitude of each row to one.
    XstarT.resize(Zstar.size(),0.0,false,false);
    for (j=0;j<Zstar.rows();++j) {
      d = sqrt(Zstar.getRow(j).dot(Zstar.getRow(j)));
      if (d > 0.0) {
        Zstar.getRow(j).divide(d);
      }
    }

    Zstar.detach(XstarT);

    // Step 4. Initialize Xstar by computing Rstar
    uniformDistribution rnd(0,k);
    
    dmatrix Rstar(k,k,0.0);
    i = static_cast<int>(rnd.draw());
    Rstar.getRow(0).copy(XstarT.getRow(i));

    dvector c(n,0.0);
    dvector vct;
    for (j=1;j<k;++j) {
      XstarT.multiply(Rstar.getRow(j-1),vct);
      vct.apply(abs);
      c.add(vct);
      i=vct.getIndexOfMinimum();
      Rstar.getRow(j).copy(XstarT.getRow(i));
    }

    // we have computed the transposed R, because it was much easier,
    // but now we must turn it right!
    Rstar.transpose();

    // Step 5. Initialize convergence monitoring parameter 
    double phiStarBar = 0.0;
    static const double machinePrec = std::numeric_limits<float>::epsilon();
    int maxIter = 100;

    // Step 6. Find the optimal discrete solution Xstar by
    Xstar.resize(n,0,false,false);
    dmatrix XT,P;
    P.resize(k,k,0.0,false,false);
    dmatrix U,UT;
    dvector omega;
    double phiBar = 0.0;

    while(maxIter>0) {
      XT.multiply(XstarT,Rstar);
      for (j=0;j<n;++j) {
        Xstar.at(j) = XT.getRow(j).getIndexOfMaximum();
      }

      // Step 7. Find the optimal orthogonal matrix Rstar
      P.fill(0.0);
      
      for (j=0;j<n;++j) {
        P.getRow(Xstar.at(j)).add(XstarT.getRow(j));
      }

      // now we need the Singular Value Decomposition (SVD) of matrix P
      if (!svd.apply(P,U,omega,UT)) {
        setStatusString("SVD computation: ");
        appendStatusString(svd.getStatusString());
        return false;
      }
      
      phiBar = omega.sumOfElements();
      
      if (abs(phiBar-phiStarBar) < machinePrec) {
        break;
      }

      phiStarBar=phiBar;
      Rstar.multiply(UT,U);

      maxIter--;
    }
    
    
    return true;
  }
 
  void multiclassNormalizedCuts::computeDegree(const dmatrix& W, 
                                                     dvector& D) const {
    D.resize(W.rows());
    int i;
    for (i=0;i<D.size();++i) {
      D.at(i)=W.getRow(i).sumOfElements();
    }
  }


  /*
   * Compute the k-way partitioning of a graph with affinity matrix
   * "weights".
   *
   * @param weights the affinity matrix containing the weights between each
   *                two nodes of the graph.  This square matrix must also be
   *                symmetric.  The elements must be positive or equal zero,
   *                and correspond to similarity metrics (not distance 
   *                metrics).  In the original paper is denoted with W
   * @param xstar   (X* in the paper) is a vector containing the label
   *                of each node.  The values will be between 0 and k-1 where
   *                k is the value given in the parameters object.
   * @return true if apply successful or false otherwise.
   */
  bool multiclassNormalizedCuts::apply(const dmatrix& weights,
                                       ivector& xstar) const {
    dvector D;
    computeDegree(weights,D);
    return kWayNormalizedCut(weights,D,getParameters().k,xstar);
  };
  
  /**
   * Compute the k-way partitioning of a graph with affinity matrix
   * "weights".
   *
   * @param k       number of classes to be found, i.e. k subgraphs will be
   *                detected.
   * @param weights the affinity matrix containing the weights between each
   *                two nodes of the graph.  This square matrix must also be
   *                symmetric.  The elements must be positive or equal zero,
   *                and correspond to similarity metrics (not distance 
   *                metrics).  In the original paper is denoted with W
   * @param xstar   (X* in the paper) is a vector containing the label
   *                of each node.  The values will be between 0 and k-1 where
   *                k is the value given as argument (the value k in the
   *                parameters object will  be ignored).
   * @return true if apply successful or false otherwise.
   */
  bool multiclassNormalizedCuts::apply(const int k,
                                       const dmatrix& weights,
                                       ivector& xstar) const {
    dvector D;
    computeDegree(weights,D);
    return kWayNormalizedCut(weights,D,k,xstar);
  }
  
  /**
   * Compute the k-way partitioning of a graph with affinity matrix
   * "weights".
   *
   * @param k       number of classes to be found, i.e. k subgraphs will be
   *                detected.
   * @param weights the affinity matrix containing the weights between each
   *                two nodes of the graph.  This square matrix must also be
   *                symmetric.  The elements must be positive or equal zero,
   *                and correspond to similarity metrics (not distance 
   *                metrics).  In the original paper is denoted with W
   * @param degree  the degree vector, defined as the sum of elements of each
   *                row of the weights.
   * @param xstar   (X* in the paper) is a vector containing the label
   *                of each node.  The values will be between 0 and k-1 where
   *                k is the value given as argument (the value k in the
   *                parameters object will  be ignored).
   * @return true if apply successful or false otherwise.
   */
  bool multiclassNormalizedCuts::apply(const int k,
                                       const dmatrix& weights,
                                       const dvector& degree,
                                       ivector& xstar) const {
    return kWayNormalizedCut(weights,degree,k,xstar);
  }
  
}
