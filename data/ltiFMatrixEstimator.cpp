/*
 * Copyright (C) 2004, 2005, 2006
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
 * file .......: ltiFMatrixEstimator.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 14.4.2004
 * revisions ..: $Id: ltiFMatrixEstimator.cpp,v 1.7 2006/09/05 10:10:56 ltilib Exp $
 */

#include "ltiFMatrixEstimator.h"

#include "ltiUnifiedSVD.h"
#include "ltiHTypes.h"

#include "ltiMath.h"
#include <limits>

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1

#include "ltiDebug.h"


#if defined(_LTI_DEBUG)
using std::cout;
using std::endl;
using std::flush;
#endif

namespace lti {
  // --------------------------------------------------
  // fMatrixEstimator::parameters
  // --------------------------------------------------

  // default constructor
  fMatrixEstimator::parameters::parameters()
    : fMatrixEstimatorBase::parameters() {
  }

  // copy constructor
  fMatrixEstimator::parameters::parameters(const parameters& other)
    : fMatrixEstimatorBase::parameters() {
    copy(other);
  }

  // destructor
  fMatrixEstimator::parameters::~parameters() {
  }

  // get type name
  const char* fMatrixEstimator::parameters::getTypeName() const {
    return "fMatrixEstimator::parameters";
  }

  // copy member

  fMatrixEstimator::parameters&
    fMatrixEstimator::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    fMatrixEstimatorBase::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    fMatrixEstimatorBase::parameters& (fMatrixEstimatorBase::parameters::* p_copy)
      (const fMatrixEstimatorBase::parameters&) =
      fMatrixEstimatorBase::parameters::copy;
    (this->*p_copy)(other);
# endif

    return *this;
  }

  // alias for copy member
  fMatrixEstimator::parameters&
    fMatrixEstimator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* fMatrixEstimator::parameters::clone() const {
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
  bool fMatrixEstimator::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool fMatrixEstimator::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && fMatrixEstimatorBase::parameters::write(handler,false);
# else
    bool (fMatrixEstimatorBase::parameters::* p_writeMS)(ioHandler&,const bool) const =
      fMatrixEstimatorBase::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool fMatrixEstimator::parameters::write(ioHandler& handler,
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
   * @return true if read was successful
   */
# ifndef _LTI_MSC_6
  bool fMatrixEstimator::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool fMatrixEstimator::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && fMatrixEstimatorBase::parameters::read(handler,false);
# else
    bool (fMatrixEstimatorBase::parameters::* p_readMS)(ioHandler&,const bool) =
      fMatrixEstimatorBase::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool fMatrixEstimator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // fMatrixEstimator
  // --------------------------------------------------

  // default constructor
  fMatrixEstimator::fMatrixEstimator()
    : fMatrixEstimatorBase(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  fMatrixEstimator::fMatrixEstimator(const parameters& par)
    : fMatrixEstimatorBase() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  fMatrixEstimator::fMatrixEstimator(const fMatrixEstimator& other)
    : fMatrixEstimatorBase() {
    copy(other);
  }

  // destructor
  fMatrixEstimator::~fMatrixEstimator() {
  }

  // returns the name of this type
  const char* fMatrixEstimator::getTypeName() const {
    return "fMatrixEstimator";
  }

  // copy member
  fMatrixEstimator&
    fMatrixEstimator::copy(const fMatrixEstimator& other) {
      fMatrixEstimatorBase::copy(other);

    return (*this);
  }

  // alias for copy member
  fMatrixEstimator&
    fMatrixEstimator::operator=(const fMatrixEstimator& other) {
    return (copy(other));
  }


  // clone member
  functor* fMatrixEstimator::clone() const {
    return new fMatrixEstimator(*this);
  }

  // return parameters
  const fMatrixEstimator::parameters&
    fMatrixEstimator::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // A very private helper
  // -------------------------------------------------------------------
  template<class T, class U>
  class fMatHelper : public fMatBaseHelper<T,U> {
  public:
    fMatHelper(const fMatrixEstimator::parameters& par) :
    fMatBaseHelper<T,U>(par) {
    };

    ~fMatHelper() {
    };

    bool apply(const matrix<tpoint<U> >& src,
               vector<T>& dest);

    bool apply(const matrix<tpoint<U> >& src,
               vector<T>& dest, vector<T>& error);

    
    bool apply(const matrix<tpoint<U> >& src,
               vector<T>& dest,
               const ivector& indices,
               const int numCorrespondences);
  
    bool apply(const matrix<tpoint<U> >& src,
               vector<T>& dest, vector<T>& error,
               const ivector& indices,
               const int numCorrespondences);

    bool computeResidual(const matrix<tpoint<U> >& src,
                         const vector<T>& transf,
                         vector<T>& error);

    bool denormalize(vector<T>& srcdest,
                     const vector<tpoint<T> >& scale,
                     const vector<tpoint<T> >& shift);

    bool convert(const vector<T>& src, matrix<U>& dest);

  private:

    bool compute(matrix<T>& AtA, vector<T>& dest);

    void multRowFT1(typename vector<T>::iterator& it,
                    const tpoint<T>& scale,
                    const tpoint<T>& shift) const;
  };

  //solve by singular value decomposition
  template<class T, class U>
  inline bool fMatHelper<T,U>::compute(matrix<T>& AtA,
                                       vector<T>& dest) {
    
    matrix<T> u;    //eigenvectors
    matrix<T> v;    //left hand singular vectors
    vector<T> w;    //eigenvalues
    typename unifiedSVD<T>::parameters svdPara;
    svdPara.sort = true;       //in descending order
    svdPara.transposeU = true; //faster; Ut is returned
    unifiedSVD<T> svd ( svdPara );
    if (!svd.apply(AtA,u,w,v)) {
      return false;
    }

#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    _lti_debug("the eigenvalues are:" << endl << w << endl);
    _lti_debug("the eigenvectors are:" << endl << u <<endl);
#endif

    //todo??: search for the smalest eigenvalue instead of sorting
    //        -> possibly faster

    //the solution is the eigenvector with least eigenvalue
    //the u-matrix is transposed, so use the last row
    dest.copy(u.getRow(u.lastRow()));
    return enforceSingularity(dest);
  }

  //--------
  // applies
  //--------

  template<class T, class U>
  inline bool fMatHelper<T,U>::apply(const matrix<tpoint<U> >& src,
                                     vector<T>& dest) {

    matrix<T> AtA;
    initialize(src, AtA);

    return compute
(AtA, dest);
  }

  template<class T, class U>
  inline bool fMatHelper<T,U>::apply(const matrix<tpoint<U> >& src,
                                     vector<T>& dest,
                                     vector<T>& error) {
    
    if ( !apply(src,dest) ) {
      return false;
    }
    
    return computeResidual(src, dest, error);
  }
  
  
  template<class T, class U>
  inline bool fMatHelper<T,U>::apply(const matrix<tpoint<U> >& src,
                                     vector<T>& dest,
                                     const ivector& indices,
                                     const int numCorrespondences) {
    
    //use the 1st numCorrespondences indices
    matrix<T> AtA;
    initialize(src, indices, numCorrespondences, AtA);

    return compute(AtA, dest);
  }

  template<class T, class U>
  inline bool fMatHelper<T,U>::apply(const matrix<tpoint<U> >& src,
                                     vector<T>& dest,
                                     vector<T>& error,
                                     const ivector& indices,
                                     const int numCorrespondences) {
  
    if ( !apply(src,dest,indices,numCorrespondences) ) {
      return false;
    }
    return computeResidual(src, dest, error);
  }

  template<class T, class U>
  inline bool fMatHelper<T,U>::computeResidual(const matrix<tpoint<U> >& src,
                                               const vector<T>& transf,
                                               vector<T>& error) { 

    if ( transf.size() != 9 ) {
      this->statusString += " transform must have 9 elements\n";
      return false;
    }
    
    const int numElem ( src.columns() );
    error.resize(numElem, T(), false, false);

    const matrix<T> fMat(3,3, &transf.at(0) ); //nasty copy, but const transf
    matrix<T> fMatT; 
    fMatT.transpose(fMat);

    hPoint2D<T> p1;
    hPoint2D<T> p2;

    const T epsilon ( std::numeric_limits<T>::epsilon() );
    typename vector<T>::iterator dit  ( error.begin() );
    typename vector<T>::iterator dend ( error.end() );
    typename vector<tpoint<U> >::const_iterator it1 ( src.getRow(0).begin() );
    typename vector<tpoint<U> >::const_iterator it2 ( src.getRow(1).begin() );
    
    if ( this->distanceMeasure_ == fMatrixEstimatorBase::parameters::Sampson) {

      //Sampson error:     sqr(p2 * F * p1) / ( J*Jt )
      // with: J*Jt = sqr(v_Fp1[0]) + sqr(v_Fp1[1])
      //             + sqr(v_Ftp2[0]) + sqr(v_Ftp2[1])
      //       Ft: transposed FMatrix
      //       v_Fp = F * p (a vector)
      for ( ; dit!=dend; ++it1, ++it2, ++dit ) {
        
        p1.x = static_cast<T>((*it1).x);
        p1.y = static_cast<T>((*it1).y);
        p1.h = T(1);
        
        p2.x = static_cast<T>((*it2).x);
        p2.y = static_cast<T>((*it2).y);
        p2.h = T(1);
        
        vector<T> Fp1 (false, 3);
        typename vector<T>::iterator vit ( Fp1.begin() );
        *vit = p1.dot(fMat.getRow(0));
        *(++vit) = p1.dot(fMat.getRow(1));
        *(++vit) = p1.dot(fMat.getRow(2));
        
        T denominator ( sqr(Fp1.at(0)) ); 
        denominator += sqr( Fp1.at(1) );
        denominator += sqr( p2.dot(fMatT.getRow(0)) );
        denominator += sqr( p2.dot(fMatT.getRow(1)) );

        if ( denominator > epsilon ) {
          const T nominator ( sqr(p2.dot(Fp1)) );
          *dit = nominator / denominator;
          
        } else {
          // x / 0 whith x!=0
          *dit = T(0); //undefined at the epipoles
        }
      }

    } else {
    
      //symmetric epipolar distance: (identical with residual)
      //   sqr(p2 * F * p1) * ( 1/denom1 + 1/denom2 )
      // with denom1 = v_Fp1[0] + v_Fp1[1]
      //      denom2 = v_Ftp2[0] + v_Ftp2[1]
      //       Ft: transposed FMatrix
      //       v_Fp = F * p (a vector)
      for ( ; dit!=dend; ++it1, ++it2, ++dit ) {

        p1.x = static_cast<T>((*it1).x);
        p1.y = static_cast<T>((*it1).y);
        p1.h = T(1);
        
        p2.x = static_cast<T>((*it2).x);
        p2.y = static_cast<T>((*it2).y);
        p2.h = T(1);

        vector<T> Fp1 (false, 3);
        typename vector<T>::iterator vit ( Fp1.begin() );
        *vit = p1.dot(fMat.getRow(0));
        *(++vit) = p1.dot(fMat.getRow(1));
        *(++vit) = p1.dot(fMat.getRow(2));
        
        T denom1 ( sqr(Fp1.at(0)) ); 
        denom1 += sqr( Fp1.at(1) );
        if ( denom1 > epsilon ) {
          denom1 = T(1) / denom1;
        } else {
          denom1 = T(0);
        }
        
        T denom2 ( sqr( p2.dot(fMatT.getRow(0)) ) );
        denom2 += sqr( p2.dot(fMatT.getRow(1)) );
        if ( denom2 > epsilon ) {
          denom2 = T(1) / denom1;
        } else {
          denom2 = T(0);
        }
        
        *dit = sqr(p2.dot(Fp1)) * ( denom1 + denom2 );  
      }
    }

    if ( !this->computeSqError_ ) {
      for ( dit = error.begin(); dit!=dend; ++dit ) {
        *dit = sqrt(*dit);
      }
    }
    
    return true;
  }

  template<class T, class U>  
  bool fMatHelper<T,U>::denormalize(vector<T>& srcdest,
                                    const vector<tpoint<T> >& scale,
                                    const vector<tpoint<T> >& shift) {

    if ( srcdest.size() != 9 ) {
      this->statusString += " transform must have 9 elements\n";
      return false;
    }
    
    // -- undo the normalization T_2_transposed * F * T_1
    typename vector<tpoint<T> >::const_iterator scaleIt ( scale.begin() );
    typename vector<tpoint<T> >::const_iterator shiftIt ( shift.begin() );

    //F * T_1
    typename vector<T>::iterator it ( srcdest.begin() );
    multRowFT1(it, *scaleIt, *shiftIt);
    multRowFT1(++it, *scaleIt, *shiftIt);
    multRowFT1(++it, *scaleIt, *shiftIt);
    _lti_debug(" F*T_1 " << endl << srcdest << endl );

    //T_2_transposed * F
    ++scaleIt;
    ++shiftIt;
    it = srcdest.begin();

    T sc = (*scaleIt).x;
    T tr = (*shiftIt).x;
    const T f1 = *it * tr;
    *it *= sc;
    const T f2 = *(++it) * tr;
    *(it) *= sc;
    const T f3 = *(++it) * tr;
    *(it) *= sc;
 
    sc = (*scaleIt).y;
    tr = (*shiftIt).y;
    const T f4 = *(++it) * tr;
    *(it) *= sc;
    const T f5 = *(++it) * tr;
    *(it) *= sc;
    const T f6 = *(++it) * tr;
    *(it) *= sc;
    
    *(++it) += f1 + f4;
    *(++it) += f2 + f5;
    *(++it) += f3 + f6;

    return true;
  }

  template<class T, class U>  
  inline void fMatHelper<T,U>
  ::multRowFT1(typename vector<T>::iterator& it,
               const tpoint<T>& scale,
               const tpoint<T>& shift) const {
    
    const T f1 = *it * shift.x;
    *it *= scale.x;
    const T f2 = *(++it) * shift.y;
    *(it) *= scale.y;
    *(++it) += f1 + f2;
  }

  template<class T, class U>
  inline bool  fMatHelper<T,U>::convert(const vector<T>& src, 
                                        matrix<U>& dest) {
    
    if ( src.size() != 9 ) {
      this->statusString += " transform must have 9 elements\n";
      return false;
    }

    typename vector<T>::const_iterator it ( src.begin() );
    dest.resize(3,3,U(),false,false);
    typename matrix<U>::iterator dit  ( dest.begin() );
    typename matrix<U>::iterator dend ( dest.end() );
    for ( ; dit!=dend; ++dit, ++it ) {
      *dit  = static_cast<U>(*(it));
    }
    
    return true;
  }

  // -------------------------------------------------------------------
  // Required information
  // -------------------------------------------------------------------
  int fMatrixEstimator::minNumberCorrespondences() const {
    return 8;
  }

  int fMatrixEstimator::minCorrespondenceDimension() const {
    return 2;
  }

  int fMatrixEstimator::maxCorrespondenceDimension() const {
    return 2;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool fMatrixEstimator::apply(const matrix<ipoint>& src,
                               fvector& dest) const {
    
    const parameters& par = getParameters();
    fMatHelper<float,int>  help ( par );
    return help.apply(src,dest);
  }
  
  bool fMatrixEstimator::apply(const matrix<ipoint>& src,
                               fvector& dest, fvector& error) const {

    const parameters& par = getParameters();
    fMatHelper<float,int> help ( par );
    return help.apply(src,dest,error);
  }
  
  
  bool fMatrixEstimator::apply(const matrix<ipoint>& src,
                                      fvector& dest,
                                      const ivector& indices,
                                      int numCorrespondences) const {
 
    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    fMatHelper<float,int> help ( par );
    return help.apply(src,dest,indices,numCorrespondences);
  }
  
  bool fMatrixEstimator::apply(const matrix<ipoint>& src,
                                      fvector& dest, fvector& error,
                                      const ivector& indices,
                                      int numCorrespondences) const {

    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    fMatHelper<float,int> help ( par );
    return help.apply(src,dest,error,indices,numCorrespondences);
  }
 
  bool fMatrixEstimator::apply(const matrix<fpoint>& src,
                                      fvector& dest) const {

    const parameters& par = getParameters();
    fMatHelper<float,float>  help ( par );
    return help.apply(src,dest);
  }
  
  bool fMatrixEstimator::apply(const matrix<fpoint>& src,
                                      fvector& dest, fvector& error) const {

    const parameters& par = getParameters();
    fMatHelper<float,float> help ( par );
    return help.apply(src,dest,error);
  }
  
  
  bool fMatrixEstimator::apply(const matrix<fpoint>& src,
                                      fvector& dest,
                                      const ivector& indices,
                                      int numCorrespondences) const {
    
    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    fMatHelper<float,float> help ( par );
    return help.apply(src,dest,indices,numCorrespondences);
  }
  
  bool fMatrixEstimator::apply(const matrix<fpoint>& src,
                                      fvector& dest, fvector& error,
                                      const ivector& indices,
                                      int numCorrespondences) const {

    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    fMatHelper<float,float> help ( par );
    return help.apply(src,dest,error,indices,numCorrespondences);
  }
  
  bool fMatrixEstimator::apply(const matrix<dpoint>& src,
                                      dvector& dest) const {
    
    const parameters& par = getParameters();
    fMatHelper<double,double>  help ( par );
    return help.apply(src,dest);
  }
  
  bool fMatrixEstimator::apply(const matrix<dpoint>& src,
                                      dvector& dest, dvector& error) const {
    
    const parameters& par = getParameters();
    fMatHelper<double,double>  help ( par );
    return help.apply(src,dest,error);
  }
  
  
  bool fMatrixEstimator::apply(const matrix<dpoint>& src,
                                      dvector& dest,
                                      const ivector& indices,
                                      int numCorrespondences) const {

    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    fMatHelper<double,double>  help ( par );
    return help.apply(src,dest,indices,numCorrespondences);
  }
  
  bool fMatrixEstimator::apply(const matrix<dpoint>& src,
                                      dvector& dest, dvector& error,
                                      const ivector& indices,
                                      int numCorrespondences) const {
    
    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    fMatHelper<double,double>  help ( par );
    return help.apply(src,dest,error,indices,numCorrespondences);
  }
  
  // -------------------------------------------------------------------
  // The getResidual-methods!
  // -------------------------------------------------------------------

  bool fMatrixEstimator
  ::computeResidual(const matrix<fpoint >& src,
                const fvector& transform, fvector& dest) const {
 
    const parameters& par = getParameters();
    fMatHelper<float,float>  help ( par );
    if ( !help.computeResidual(src,transform,dest) ) {
      setStatusString(help.statusString.c_str());
    }
    return true;
  }

  bool fMatrixEstimator
  ::computeResidual(const matrix<dpoint >& src,
                    const dvector& transform, dvector& dest) const {

    const parameters& par = getParameters();
    fMatHelper<double,double>  help ( par );
    if ( !help.computeResidual(src,transform,dest) ) {
      setStatusString(help.statusString.c_str());
    }
    return true;
  }

  // -------------------------------------------------------------------
  // The denormalize-methods!
  // -------------------------------------------------------------------

  bool fMatrixEstimator::denormalize(fvector& srcdest,
                                     const vector<fpoint>& scale,
                                     const vector<fpoint>& shift) const {
 
    const parameters& par = getParameters();
    fMatHelper<float,float>  help ( par );
    if ( !help.denormalize(srcdest,scale,shift) ) {
      setStatusString(help.statusString.c_str());
    }
    return true;
  }

  bool fMatrixEstimator::denormalize(dvector& srcdest,
                                     const vector<dpoint>& scale,
                                     const vector<dpoint>& shift) const {
  
    const parameters& par = getParameters();
    fMatHelper<double,double>  help ( par );
    if ( !help.denormalize(srcdest,scale,shift) ) {
      setStatusString(help.statusString.c_str());
    }
    return true;
  }

  // -------------------------------------------------------------------
  // The convert-methods!
  // -------------------------------------------------------------------

  bool fMatrixEstimator::convert(const fvector& src,
                                 fmatrix& dest) const {
    
    const parameters& par = getParameters();
    fMatHelper<float,float>  help ( par );
    if ( !help.convert(src,dest) ) {
      setStatusString(help.statusString.c_str());
      return false;
    }
    return true;
  }

  bool fMatrixEstimator::convert(const dvector& src, 
                                 dmatrix& dest) const {
    
    const parameters& par = getParameters();
    fMatHelper<double,double>  help ( par );
    if ( !help.convert(src,dest) ) {
      setStatusString(help.statusString.c_str());
      return false;
    }
    return true;
  }
  
}

#include "ltiUndebug.h"
