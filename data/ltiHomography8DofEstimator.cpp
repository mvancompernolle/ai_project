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
 * file .......: ltiHomography8DofEstimator.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 14.4.2004
 * revisions ..: $Id: ltiHomography8DofEstimator.cpp,v 1.14 2006/09/05 10:16:25 ltilib Exp $
 */

#include "ltiHomography8DofEstimator.h"

#include "ltiMath.h"
#include <limits>

#undef _LTI_DEBUG
//#define _LTI_DEBUG 3

#include "ltiDebug.h"


#if defined(_LTI_DEBUG)
using std::cout;
using std::endl;
using std::flush;
#endif

namespace lti {
  // --------------------------------------------------
  // homography8DofEstimator::parameters
  // --------------------------------------------------

  // default constructor
  homography8DofEstimator::parameters::parameters()
    : homographyEstimatorBase::parameters() {
  }

  // copy constructor
  homography8DofEstimator::parameters::parameters(const parameters& other)
    : homographyEstimatorBase::parameters() {
    copy(other);
  }

  // destructor
  homography8DofEstimator::parameters::~parameters() {
  }

  // get type name
  const char* homography8DofEstimator::parameters::getTypeName() const {
    return "homography8DofEstimator::parameters";
  }

  // copy member

  homography8DofEstimator::parameters&
    homography8DofEstimator::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    homographyEstimatorBase::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    homographyEstimatorBase::parameters& 
      (homographyEstimatorBase::parameters::* p_copy)
      (const homographyEstimatorBase::parameters&) =
      homographyEstimatorBase::parameters::copy;
    (this->*p_copy)(other);
# endif

    return *this;
  }

  // alias for copy member
  homography8DofEstimator::parameters&
    homography8DofEstimator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* homography8DofEstimator::parameters::clone() const {
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
  bool homography8DofEstimator::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool homography8DofEstimator::parameters::writeMS(ioHandler& handler,
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
    b = b && homographyEstimatorBase::parameters::write(handler,false);
# else
    bool (homographyEstimatorBase::parameters::* p_writeMS)
      (ioHandler&,const bool) const =
      homographyEstimatorBase::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool homography8DofEstimator::parameters::write(ioHandler& handler,
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
  bool homography8DofEstimator::parameters::read(ioHandler& handler,
                                                 const bool complete)
# else
  bool homography8DofEstimator::parameters::readMS(ioHandler& handler,
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
    b = b && homographyEstimatorBase::parameters::read(handler,false);
# else
    bool (homographyEstimatorBase::parameters::* p_readMS)
      (ioHandler&,const bool)=
      homographyEstimatorBase::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool homography8DofEstimator::parameters::read(ioHandler& handler,
                                                 const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // homography8DofEstimator
  // --------------------------------------------------

  // default constructor
  homography8DofEstimator::homography8DofEstimator()
    : homographyEstimatorBase(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  homography8DofEstimator::homography8DofEstimator(const parameters& par)
    : homographyEstimatorBase() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  homography8DofEstimator
  ::homography8DofEstimator(const homography8DofEstimator& other)
    : homographyEstimatorBase() {
    copy(other);
  }

  // destructor
  homography8DofEstimator::~homography8DofEstimator() {
  }

  // returns the name of this type
  const char* homography8DofEstimator::getTypeName() const {
    return "homography8DofEstimator";
  }

  // copy member
  homography8DofEstimator&
    homography8DofEstimator::copy(const homography8DofEstimator& other) {
      homographyEstimatorBase::copy(other);

    return (*this);
  }

  // alias for copy member
  homography8DofEstimator&
    homography8DofEstimator::operator=(const homography8DofEstimator& other) {
    return (copy(other));
  }


  // clone member
  functor* homography8DofEstimator::clone() const {
    return new homography8DofEstimator(*this);
  }

  // return parameters
  const homography8DofEstimator::parameters&
    homography8DofEstimator::getParameters() const {
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
  class hom8DofHelper {
  public:
    hom8DofHelper(const bool computeSqError) :
    m_computeSqError ( computeSqError ) {
    };

    ~hom8DofHelper() {
    };

    bool apply(const matrix<tpoint<U> >& src,
               vector<T>& dest) const;

    bool apply(const matrix<tpoint<U> >& src,
               vector<T>& dest, vector<T>& error) const;

    
    bool apply(const matrix<tpoint<U> >& src,
               vector<T>& dest,
               const ivector& indices,
               int numCorrespondences) const;
  
    bool apply(const matrix<tpoint<U> >& src,
               vector<T>& dest, vector<T>& error,
               const ivector& indices,
               int numCorrespondences) const;

   bool computeResidual(const matrix<tpoint<U> >& src,
                         const vector<T>& hom,
                         vector<T>& error) const;
    
    bool convert(const vector<T>& src,hMatrix3D<U>& dest) const;
  
    bool convert(const vector<T>& src, matrix<U>& dest) const;

    bool denormalize(vector<T>& srcdest,
                     const vector<tpoint<T> >& scale,
                     const vector<tpoint<T> >& shift) const;
    
  private:
    bool m_computeSqError;

    void initialize(const matrix<tpoint<U> >& src,
                    vector<T>& AtA11, matrix<T>& AtA31,
                    matrix<T>& AtA32, vector<T>& AtA33,
                    vector<T>& dest) const;

    void initialize(const matrix<tpoint<U> >& src, const ivector& indices,
                    const int numCorrespondences,
                    vector<T>& AtA11, matrix<T>& AtA31,
                    matrix<T>& AtA32, vector<T>& AtA33,
                    vector<T>& dest) const;

    bool compute(vector<T>& At_A11,
                 matrix<T>& At_A31,
                 matrix<T>& At_A32,
                 vector<T>& At_A33,
                 vector<T>& dest) const;

    //these are partly historic methods that also could be implemented
    // using (more) lti::classes (with the drawback of loosing efficiency)

    //code required for blockoriented cholesky factorization

    bool cholesky(int dim, vector<T>& src) const;

    bool invMultVec(int dim,
                    const vector<T>& UD,
                    typename vector<T>::iterator& vIt) const;

    bool invTMultVec(int dim, const vector<T>& UD,
                     typename vector<T>::iterator& vIt) const;

    void mMultMt(const matrix<T>& src,vector<T>& dest) const;
    
    bool blockCholesky(vector<T>& At_A11,
                       matrix<T>& At_A31,
                       matrix<T>& At_A32,
                       vector<T>& At_A33) const;

    //forward substitution
    bool cTimesYEqualsD(const vector<T>& At_A11, 
                        const matrix<T>& At_A31, 
                        const matrix<T>& At_A32,
                        const vector<T>& At_A33,
                        vector<T>& At_b) const;

    //backward substitution
    bool ctTimesXlsEqualsY(const vector<T>& At_A11, 
                           const matrix<T>& At_A31,
                           const matrix<T>& At_A32,
                           const vector<T>& At_A33,
                           vector<T>& At_b) const;
  };

  //------------------
  //the initialization
  //------------------

  template<class T, class U>
  inline void hom8DofHelper<T,U>::initialize(const matrix<tpoint<U> >& src,
                                             vector<T>& AtA11,
                                             matrix<T>& AtA31,
                                             matrix<T>& AtA32,
                                             vector<T>& AtA33,
                                             vector<T>& dest) const {

    //------ least squares problem: A * dest = b
    
    //---------------------------------------------------
    //references for fast access of the vector's elements
    //---------------------------------------------------
    //----------------
    //A_transponed * b
    //----------------
    dest.resize(8, T(0), false, true);
    typename vector<T>::iterator vit ( dest.begin() );
    T& d0 = *vit;
    T& d1 = *(++vit);
    T& d2 = *(++vit);
    T& d3 = *(++vit);
    T& d4 = *(++vit);
    T& d5 = *(++vit);
    T& d6 = *(++vit);
    T& d7 = *(++vit);
    //-------------
    //submatrix 1.1, 3x3, symectric, only triagonal part is computed
    //-------------
    AtA11.resize ( 6, T(0), false, true );
    vit = AtA11.begin();
    T& a11_0 = *vit;
    T& a11_1 = *(++vit);
    T& a11_2 = *(++vit);
    T& a11_3 = *(++vit);
    T& a11_4 = *(++vit);
    *(++vit) = static_cast<T>(src.columns());
    //-------------
    //submatrix 3.1, 2x3
    //-------------   
    AtA31.resize ( 2, 3, T(0), false, true );
    typename matrix<T>::iterator mit ( AtA31.begin() );
    T& a31_0 = *mit;
    T& a31_1 = *(++mit);
    T& a31_2 = *(++mit);
    T& a31_3 = *(++mit);
    T& a31_4 = *(++mit);
    T& a31_5 = *(++mit);
    //-------------
    //submatrix 3.2, 2x3
    //-------------   
    AtA32.resize( 2, 3, T(0), false, true );
    mit = AtA32.begin();
    T& a32_0 = *mit;
    T& a32_1 = *(++mit);
    T& a32_2 = *(++mit);
    T& a32_3 = *(++mit);
    T& a32_4 = *(++mit);
    T& a32_5 = *(++mit);
    //-------------
    //submatrix 3.3, 2x2
    //-------------   
    AtA33.resize( 3, T(0), false, true );
    vit = AtA33.begin();
    T& a33_0 = *vit;
    T& a33_1 = *(++vit);
    T& a33_2 = *(++vit);

    //--------------------
    //init all submatrices
    //--------------------

    const vector<tpoint<U> >& pts1 = src.getRow(0);
    const vector<tpoint<U> >& pts2 = src.getRow(1);
    typename vector<tpoint<U> >::const_iterator end ( pts1.end()   );
    typename vector<tpoint<U> >::const_iterator it1 ( pts1.begin() );
    typename vector<tpoint<U> >::const_iterator it2 ( pts2.begin() );
    
    for (; it1!=end; ++it1, ++it2) {
      const T x1 ( static_cast<T>((*it1).x) );
      const T y1 ( static_cast<T>((*it1).y) );
      const T sqX1 ( x1*x1 );
      const T sqY1 ( y1*y1 );
      const T x1Y1 ( x1*y1 );

      a11_0 += sqX1;
      a11_1 += x1Y1;
      a11_2 += x1;
      a11_3 += sqY1;
      a11_4 += y1;

      const T x2 ( static_cast<T>((*it2).x) );
      const T y2 ( static_cast<T>((*it2).y) );
      
      a31_0 -= sqX1 * x2;
      a31_1 -= x1Y1 * x2;
      a31_2 -= x1 * x2;
      a31_4 -= sqY1 * x2;
      a31_5 -= y1 * x2;
      d2 += x2;

      a32_0 -= sqX1 * y2;
      a32_1 -= x1Y1 * y2;
      a32_2 -= x1 * y2;
      a32_4 -= sqY1 * y2;
      a32_5 -= y1 * y2;
      d5 += y2;

      const T tmp ( ( x2 * x2 ) + ( y2 * y2 ) );
      a33_0 += sqX1 * tmp;
      a33_1 += x1Y1 * tmp;
      a33_2 += sqY1  * tmp;
      d6 -= x1  * tmp;
      d7 -= y1 * tmp;
    }
    a31_3 = a31_1;
    a32_3 = a32_1;
    d0 = -a31_2;
    d1 = -a31_5;
    d3 = -a32_2;
    d4 = -a32_5;

#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    cout << " At_A11 " << AtA11 << endl;
    cout << " At_A31 " << endl << AtA31 << endl;
    cout << " At_A32 " << endl << AtA32 << endl;
    cout << " At_A33 " << AtA33 << endl;
    cout << endl << " At_b: " << dest << endl;
#endif
}

  //initialization for 1st n points only
  template<class T, class U>
  inline void hom8DofHelper<T,U>::initialize(const matrix<tpoint<U> >& src,
                                             const ivector& indices,
                                             const int numCorrespondences,
                                             vector<T>& AtA11,
                                             matrix<T>& AtA31,
                                             matrix<T>& AtA32,
                                             vector<T>& AtA33,
                                             vector<T>& dest) const {
        
     //------ least squares problem: A * dest = b
    
    //---------------------------------------------------
    //references for fast access of the vector's elements
    //---------------------------------------------------
    //----------------
    //A_transponed * b
    //----------------
    dest.resize(8, T(0), false, true);
    typename vector<T>::iterator vit ( dest.begin() );
    T& d0 = *vit;
    T& d1 = *(++vit);
    T& d2 = *(++vit);
    T& d3 = *(++vit);
    T& d4 = *(++vit);
    T& d5 = *(++vit);
    T& d6 = *(++vit);
    T& d7 = *(++vit);
    //-------------
    //submatrix 1.1, 3x3, symectric, only triagonal part is computed
    //-------------
    AtA11.resize ( 6, T(0), false, true );
    vit = AtA11.begin();
    T& a11_0 = *vit;
    T& a11_1 = *(++vit);
    T& a11_2 = *(++vit);
    T& a11_3 = *(++vit);
    T& a11_4 = *(++vit);
    *(++vit) = static_cast<T>(src.columns());
    //-------------
    //submatrix 3.1, 2x3
    //-------------   
    AtA31.resize ( 2, 3, T(0), false, true );
    typename matrix<T>::iterator mit ( AtA31.begin() );
    T& a31_0 = *mit;
    T& a31_1 = *(++mit);
    T& a31_2 = *(++mit);
    T& a31_3 = *(++mit);
    T& a31_4 = *(++mit);
    T& a31_5 = *(++mit);
    //-------------
    //submatrix 3.2, 2x3
    //-------------   
    AtA32.resize( 2, 3, T(0), false, true );
    mit = AtA32.begin();
    T& a32_0 = *mit;
    T& a32_1 = *(++mit);
    T& a32_2 = *(++mit);
    T& a32_3 = *(++mit);
    T& a32_4 = *(++mit);
    T& a32_5 = *(++mit);
    //-------------
    //submatrix 3.3, 2x2
    //-------------   
    AtA33.resize( 3, T(0), false, true );
    vit = AtA33.begin();
    T& a33_0 = *vit;
    T& a33_1 = *(++vit);
    T& a33_2 = *(++vit);

    //--------------------
    //init all submatrices
    //--------------------

    const vector<tpoint<U> >& pts1 = src.getRow(0);
    const vector<tpoint<U> >& pts2 = src.getRow(1);
    typename ivector::const_iterator it  ( indices.begin() );
    typename ivector::const_iterator end ( it + numCorrespondences );
    for (; it!=end; ++it) {

      const tpoint<U>& pt1 = pts1.at(*it);
      const T x1 ( static_cast<T>(pt1.x) );
      const T y1 ( static_cast<T>(pt1.y) );
      const T sqX1 ( x1*x1 );
      const T sqY1 ( y1*y1 );
      const T x1Y1 ( x1*y1 );

      a11_0 += sqX1;
      a11_1 += x1Y1;
      a11_2 += x1;
      a11_3 += sqY1;
      a11_4 += y1;

      const tpoint<U>& pt2 = pts2.at(*it);
      const T x2 ( static_cast<T>(pt2.x) );
      const T y2 ( static_cast<T>(pt2.y) );

      a31_0 -= sqX1 * x2;
      a31_1 -= x1Y1 * x2;
      a31_2 -= x1 * x2;
      a31_4 -= sqY1 * x2;
      a31_5 -= y1 * x2;
      d2 += x2;

      a32_0 -= sqX1 * y2;
      a32_1 -= x1Y1 * y2;
      a32_2 -= x1 * y2;
      a32_4 -= sqY1 * y2;
      a32_5 -= y1 * y2;
      d5 += y2;

      const T tmp ( ( x2 * x2 ) + ( y2 * y2 ) );
      a33_0 += sqX1 * tmp;
      a33_1 += x1Y1 * tmp;
      a33_2 += sqY1  * tmp;
      d6 -= x1  * tmp;
      d7 -= y1 * tmp;
    }
    a31_3 = a31_1;
    a32_3 = a32_1;
    d0 = -a31_2;
    d1 = -a31_5;
    d3 = -a32_2;
    d4 = -a32_5;

#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
    cout << " index init " << endl;
    cout << " At_A11 " << AtA11 << endl;
    cout << " At_A31 " << endl << AtA31 << endl;
    cout << " At_A32 " << endl << AtA32 << endl;
    cout << " At_A33 " << AtA33 << endl;
    cout << endl << " At_b: " << dest << endl;
#endif
  }

  //the actual computation
  template<class T, class U>
  inline bool hom8DofHelper<T,U>::compute(vector<T>& At_A11,
                                          matrix<T>& At_A31,
                                          matrix<T>& At_A32,
                                          vector<T>& At_A33,
                                          vector<T>& dest) const {
    
    // cholesky factorisations of At_A, blockwise, At_A will be overwritten
    if ( !blockCholesky(At_A11,At_A31,At_A32,At_A33) ) {
      return false;
    }
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 2)
    cout << " ---- C33 : " << At_A33 << endl;
#endif

    // solve C * y = At_b, bockwise, At_b(dest) will be overwritten
    if ( !cTimesYEqualsD(At_A11,At_A31,At_A32,At_A33,dest) ) {
      return false;
    }
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 2)
    cout << " ---- y " << dest << endl;
#endif

    // solve Ct * xLS = y, blockwise, At_b(dest) will be overwritten
    if ( !ctTimesXlsEqualsY(At_A11,At_A31,At_A32,At_A33,dest) ) {
      return false;
    }
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 1)
    cout << " ---- xls " << dest << endl;
#endif

    return true;
  }

  template<class T, class U>
  inline bool hom8DofHelper<T,U>
  ::computeResidual(const matrix<tpoint<U> >& src,
                    const vector<T>& hom,
                    vector<T>& error) const { 
    
    if ( hom.size() != 8 ) {
      return false;
    }

    const int numElem = src.columns();
    error.resize(numElem, T(), false, false);
    typename vector<T>::iterator dit  ( error.begin() );
    typename vector<T>::iterator dend ( error.end() );
    typename vector<tpoint<U> >::const_iterator it1 ( src.getRow(0).begin() );
    typename vector<tpoint<U> >::const_iterator it2 ( src.getRow(1).begin() );

    T elemX, elemY;

    for ( ; dit!=dend; ++it1, ++it2, ++dit ) {

      const T x1 ( static_cast<T>((*it1).x) ); 
      const T y1 ( static_cast<T>((*it1).y) );

//       typename vector<T>::const_iterator it ( hom.begin() );
//       T elemX ( *it * x1 + *(++it) * y1 + *(++it) );
//       T elemY ( *(++it) * x1 + *(++it) * y1 + *(++it) );
//       const T denom ( *(++it) * x1 + *(++it) * y1 + T(1) );
      elemX = hom.at(0) * x1 + hom.at(1) * y1 + hom.at(2);
      elemY = hom.at(3) * x1 + hom.at(4) * y1 + hom.at(5);
      const T denom ( hom.at(6) * x1 + hom.at(7) * y1 + T(1) );
      elemX /= denom;
      elemY /= denom;
      //_lti_debug(" e1:  " << elemX << " " << elemY << " e2 " << *it2);
      const T x2 ( static_cast<T>((*it2).x) );
      const T y2 ( static_cast<T>((*it2).y) );
      elemX -= x2;
      elemY -= y2;
      *dit = elemX * elemX;
      *dit += elemY * elemY;
    }
    
    if ( !m_computeSqError ) {
      for ( dit = error.begin(); dit!=dend; ++dit ) {
        *dit = sqrt(*dit);
      }
    }
    _lti_debug(endl << " **** residual: " << error);
    return true;
  }

  template<class T, class U>  
  bool hom8DofHelper<T,U>::denormalize(vector<T>& srcdest,
                                       const vector<tpoint<T> >& scale,
                                       const vector<tpoint<T> >& shift) const {

    if ( srcdest.size() != 8 ) {
      return false;
    }
    
    // -- undo the normalization of the points to be transformed
    const tpoint<T>& otherScale = scale.at(0);
    const tpoint<T>& otherShift = shift.at(0);
    typename vector<T>::iterator hit ( srcdest.begin() );
    srcdest[2] += otherShift.x * (*hit) + otherShift.y * srcdest[1];
    *hit *= otherScale.x;
    *(++hit) *= otherScale.y;
    ++hit; //2
    ++hit; //3
    srcdest[5] += otherShift.x * (*hit) + otherShift.y * srcdest[4];
    *hit *= otherScale.x;       //3
    *(++hit) *= otherScale.y;   //4
    ++hit; //5
    ++hit; //6
    T hom9 = 1 + otherShift.x * (*hit) + otherShift.y * srcdest[7];
    *hit *= otherScale.x;
    *(++hit) *= otherScale.y;

    // -- undo normalization of the reference points 
    const tpoint<T>& refScale = scale.at(1);
    const tpoint<T>& refShift = shift.at(1);
    hit = srcdest.begin();               //0
    *hit -= srcdest[6] * refShift.x;
    *hit /= refScale.x * hom9;
    *(++hit) -= srcdest[7] * refShift.x; //1
    *hit    /= refScale.x * hom9;
    *(++hit) /= refScale.x;              //2
    *hit     -= refShift.x;
    *(++hit) -= srcdest[6] * refShift.y; //3
    *hit    /= refScale.y * hom9;
    *(++hit) -= srcdest[7] * refShift.y; //4
    *hit    /= refScale.y * hom9;
    *(++hit) /= refScale.y;              //5
    *hit     -= refShift.y;
    *(++hit) /= hom9;                    //6
    *(++hit) /= hom9;                    //7

    return true;
  }

  //ToDo: iterator arithmetic or use the ltilib class
  //overwrites src with its cholesky factorisation
  template<class T, class U>
  inline bool hom8DofHelper<T,U>::cholesky(int dim, vector<T>& src) const {
    //cout << " cholesky " << flush;
    //src is stored column by column, cholesky is computed column by column
    //for column j the columns 0 till j-1 are required
    static const T epsilon = std::numeric_limits<T>::epsilon();

    T* pv = &(src.at(0));
    int i,j,k;
    for (j=0;j<dim;j++) {
      int column = 0;             //the column of src
      for (k=0;k<j;k++) {
        column += k * (dim - k);        
        for(i=j;i<dim;i++)
        *(pv+i-j) = *(pv+i-j) - src[column+j] * src[column+i];
      }
      T diagEl = *pv;        //current diagonal element
      if ( diagEl < epsilon ) { return false; }
      diagEl = sqrt(diagEl);
      for(i=j;i<dim;i++) {
        *pv /= diagEl;
        pv++;
      }
    }
    // cout << " .. done " << endl << flush;
    return true;
  }
 
  //multiplies the inverse of a lower triagonal matrix with a vector
  //dimension: dim x dim, 
  //supply a start index into the "vector", i.e. row of a matrix
  template<class T, class U>
  inline bool hom8DofHelper<T,U>
  ::invMultVec(int dim,
               const vector<T>& UD,
               typename vector<T>::iterator& vIt) const {
    
    //cout << " invMultVec " << endl << sleep(1) << flush;
    static const T epsilon = std::numeric_limits<T>::epsilon();
    int i,j;
    for(i=0;i<dim;i++) {
     int column = 0;            //points to column
      for(j=0;j<i;j++) {
        column += j * (dim - j);
        vIt[i] -= vIt[j] * UD[column+i];
      }
      // column points to last element in the last column
      if (i==0) { 
        column = -1; 
      } else {
        column += i * (dim - i);
      }
      const T tmp = UD[column+1 ];
      if ( tmp < -epsilon || tmp > epsilon ) {
        vIt[i] /= tmp;
      } else {
        return false;
      }
    }
    
    return true;
  }

  //multiplies the inverse of an upper triagonal matrix with a vector
  //dimension: dim x dim, 
  //supply a start index into the "vector", i.e. row of a matrix
  template<class T, class U>
  inline bool hom8DofHelper<T,U>
  ::invTMultVec(int dim, const vector<T>& UD,
                typename vector<T>::iterator& vIt) const {
    //cout << " invTMultVec " << endl << sleep(1) << flush;
    static const T epsilon = std::numeric_limits<T>::epsilon();
    int i,j;
    int row = -1;    
    for(i=0;i<dim;i++) {
      row += dim-i;              // last element in last column
    }
    for(i=0;i<dim;i++) {
      row -= i;                     //last element in current row
      for(j=0;j<i;j++){
        vIt[dim-1-i] -= vIt[dim-1-j] * UD[row-j];
      }
      const T tmp = UD[row-i];
      if ( tmp < -epsilon || tmp > epsilon) {
        vIt[dim-1-i] /= tmp;
      } else {
        return false;
      }
    }

    return true;
  }


  //multiplies a 2x3 matrix with its transposed
  //result: triangular matrix
  template<class T, class U>
  inline void hom8DofHelper<T,U>::mMultMt(const matrix<T>& src,
                                          vector<T>& dest) const {
    //cout << " mMultMt " << endl << flush;
    typename vector<T>::iterator dIt ( dest.begin() );
    int i,j,k;
    for(i=0; i<2; i++) {
      for(j=0; j<=i; j++, ++dIt) {
        *dIt = 0.; 
        for(k=0; k<3; k++) {
          *dIt +=  src.at(i,k) * src.at(j,k);
        }
      }
    }
  }                   

  //cholesky factorisation of At_A
  //blockoriented from At_A1.1, At_A3.2, At_A3.2, At_A3.3
  //C2.1 is 0 and C2.2 = C1.1 -C ist lower cholesky factorisation of At_A
  //At_A will be overwritten
  template<class T, class U>
  inline bool hom8DofHelper<T,U>::blockCholesky(vector<T>& At_A11,
                                                matrix<T>& At_A31,
                                                matrix<T>& At_A32,
                                                vector<T>& At_A33) const {

    if ( !cholesky(3, At_A11) ) { return false; }

    //C3.1=At_A3.1*Ct1.1 -- 1st row of At_A3.1
    typename vector<T>::iterator it ( At_A31.getRow(0).begin() );
    if ( !invMultVec(3, At_A11, it) ) {
      return false;
    }
    it =  At_A31.getRow(1).begin();
    if ( !invMultVec(3, At_A11, it) ) {    //2nd row
      return false;
    }
      
    //C3.2=At_A3.2*Ct1.1 -- 1st row of At_A3.2
    it = At_A32.getRow(0).begin(); 
    if ( !invMultVec(3, At_A11, it) ) {
      return false;
    }
    it = At_A32.getRow(1).begin();
    if ( !invMultVec(3, At_A11, it) ) { //2nd row
      return false;
    }

    //intermediate symmetrical matrices
    vector<T> res1 (3);
    vector<T> res2 (3);
    mMultMt(At_A31,res1);               //H=At_A3.3-C3.1*Ct3.1-C3.2*Ct3.2 
    mMultMt(At_A32,res2);   
    int i (0);
    for(;i<3;i++) {
      At_A33[i] = At_A33[i] - res1[i] - res2[i];
    }
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 2)
    cout << " blockCholesky " << endl << flush;
    cout << " ---- C11 " << endl << At_A11 << endl;
    cout << " ---- C31 " << endl << At_A31 << endl;
    cout << " ---- C32 " << endl << At_A32 << endl;
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    cout << " ------- G31 " << res1 << endl;
    cout << " ------- G32 " << res2 << endl;
    cout << " ------- H " << At_A33 << endl;
#endif
#endif

    return cholesky(2, At_A33);                  // C3.3=cholesky(H)
  }

  //solve C * y = At_b, y overwrites At_b, blockwise
  template<class T, class U>
  inline bool hom8DofHelper<T,U>::cTimesYEqualsD(const vector<T>& At_A11, 
                                                 const matrix<T>& At_A31, 
                                                 const matrix<T>& At_A32,
                                                 const vector<T>& At_A33,
                                                 vector<T>& At_b) const {
    
    typename vector<T>::iterator it ( At_b.begin() );
    if ( !invMultVec(3, At_A11,it) ) {        // 0 - 2 values
      return false;      
    }

    it += 3;  //ToDo: Caution if invMultVec should modify vIt
    if ( !invMultVec(3, At_A11,it) ) {        // 3 - 5 values
      return false;
    }
 
    //C3.1 * At_b[0,1,2]
    vector<T> tmpRes1 ( 2 );
    {
      vector<T> startAtb( 3, &(At_b[0]) );    //Todo: a copy -> inefficient !!!
      At_A31.multiply(startAtb, tmpRes1);
    }

    //C3.2 * At_b[3,4,5]
    vector<T> tmpRes2 ( 2 );
    {
      vector<T> midAtb( 3, &(At_b[3]) );      //Todo: a copy -> inefficient !!!
      At_A32.multiply(midAtb, tmpRes2);
    }
 
    At_b[6] = At_b[6] - tmpRes1[0] - tmpRes2[0];
    At_b[7] = At_b[7] - tmpRes1[1] - tmpRes2[1];
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    cout << " computing cTimesYEqualsD " << endl << flush;
    cout << " ------ hy1 " << tmpRes1 << endl;
    cout << " ------ hy2" << tmpRes2 << endl;
    cout << " ------ yy1/2/h3 " << At_b << endl;
#endif

    it += 3;  //ToDo: Caution if invMultVec should modify vIt
    return invMultVec(2, At_A33,it);
  }

  //solve Ct * xLS = y, xLS overwrites At_b, bockwise
  template<class T, class U>
  inline bool hom8DofHelper<T,U>::ctTimesXlsEqualsY(const vector<T>& At_A11, 
                                                    const matrix<T>& At_A31,
                                                    const matrix<T>& At_A32,
                                                    const vector<T>& At_A33,
                                                    vector<T>& At_b) const {

    //compute xLS[6] and xLs[7]
    typename vector<T>::iterator it ( At_b.begin() + 6);
    if ( !invTMultVec(2,At_A33,it) ) {
      return false;
    }
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    cout << " computing ctTimesXlsEqualsY " << endl << flush;
    cout << " ------ xx3 " << At_b << endl;
#endif
    vector<T> tmpRes ( 3 );
    {
      vector<T> tailAtb( 2, &(At_b[6]) );    //Todo: a copy -> inefficient !!!
      At_A32.leftMultiply(tailAtb ,tmpRes);           //Ct3.2 * y[6,7]
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
      cout << " ------ hx2 " << tmpRes << endl;
#endif
      int i = 0;
      for(;i<3;i++) {
        At_b[3+i] -= tmpRes[i];                        //subtract from y[3,4,5]
      }
    }
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    cout << " ------ hxx2 " << At_b << endl;
#endif

    it -= 3;  //ToDo: Caution if invMultVec should modify vIt
    if ( !invTMultVec(3,At_A11,it) ) {                 //compute xLS[3,4,5]
      return false;
    }
    {
      vector<T> tailAtb( 2, &(At_b[6]) );    //Todo: a copy -> inefficient !!!
      At_A31.leftMultiply(tailAtb ,tmpRes);            //C3.1 * y[6,7]
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
      cout << " ------ hx1 " << tmpRes << endl;
#endif
      int i = 0;
      for(;i<3;i++){ 
        At_b[i] -= tmpRes[i];                          //subtract from x[0,1,2]
      }
    }
#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    cout << " ------ hxx1 " << At_b << endl;
#endif

    it -= 3;  //ToDo: Caution if invMultVec should modify vIt
    return invTMultVec(3,At_A11,it);                  //compute xLS[0,1,2]
  }


  template<class T, class U>
  inline bool hom8DofHelper<T,U>::apply(const matrix<tpoint<U> >& src,
                                        vector<T>& dest) const {
    
    vector<T> At_A11;
    matrix<T> At_A31;
    matrix<T> At_A32;
    vector<T> At_A33;
    initialize(src, At_A11, At_A31, At_A32, At_A33, dest);

    return compute(At_A11, At_A31, At_A32, At_A33, dest);
  }

  template<class T, class U>
  inline bool hom8DofHelper<T,U>::apply(const matrix<tpoint<U> >& src,
                                        vector<T>& dest,
                                        vector<T>& error) const {

    if ( !apply(src,dest) ) {
      return false;
    }
   
    return computeResidual(src, dest, error);
  }
  
  template<class T, class U>
  inline bool hom8DofHelper<T,U>::apply(const matrix<tpoint<U> >& src,
                                        vector<T>& dest,
                                        const ivector& indices,
                                        int numCorrespondences) const {

    vector<T> At_A11;
    matrix<T> At_A31;
    matrix<T> At_A32;
    vector<T> At_A33;
    initialize(src, indices, numCorrespondences,
               At_A11, At_A31, At_A32, At_A33, dest);

    if ( !compute(At_A11, At_A31, At_A32, At_A33, dest) ) {
      _lti_debug(" WARNING: Could not compute dest!\n");
      return false;
    }
    
    return true;
  }

  template<class T, class U>
  inline bool hom8DofHelper<T,U>::apply(const matrix<tpoint<U> >& src,
                                        vector<T>& dest,
                                        vector<T>& error,
                                        const ivector& indices,
                                        int numCorrespondences) const {
  
    if ( !apply(src,dest,indices,numCorrespondences) ) {
      return false;
    }
    return computeResidual(src, dest, error);
  }

  template<class T, class U>
  inline bool  hom8DofHelper<T,U>::convert(const vector<T>& src,
                                           hMatrix3D<U>& dest) const {
    
    if ( src.size() != 8 ) {
      return false;
    }

    typename vector<T>::const_iterator it ( src.begin() );
    //hMatrix3D<U>::iterator dit ( dest.begin() ); //does not exist
    //affine x
    dest.at(0,0) = static_cast<U>(*(it));   //0
    dest.at(0,1) = static_cast<U>(*(++it)); //1
    dest.at(0,2) = static_cast<U>(0);
    dest.at(0,3) = static_cast<U>(*(++it)); //3
  
    //affine y
    dest.at(1,0) = static_cast<U>(*(++it)); //0
    dest.at(1,1) = static_cast<U>(*(++it)); //1
    dest.at(1,2) = static_cast<U>(0);
    dest.at(1,3) = static_cast<U>(*(++it)); //3

    //the perspective denominator
    dest.at(2,0) = static_cast<U>(0);
    dest.at(2,1) = static_cast<U>(0);
    dest.at(2,2) = static_cast<U>(1);
    dest.at(2,3) = static_cast<U>(0);

    dest.at(3,0) = static_cast<U>(*(++it));   //0
    dest.at(3,1) = static_cast<U>(*(++it));   //1
    dest.at(3,2) = static_cast<U>(0);
    dest.at(3,3) = static_cast<U>(1);
    
    return true;
  }
  
  template<class T, class U>
  inline bool  hom8DofHelper<T,U>::convert(const vector<T>& src, 
                                           matrix<U>& dest) const {
    
    if ( src.size() != 8 ) {
      return false;
    }

    typename vector<T>::const_iterator it ( src.begin() );
    dest.resize(3,3,T(),false,false);
    typename matrix<U>::iterator dit ( dest.begin() );
    //affine x
    *dit  = static_cast<U>(*(it));      //0
    *(++dit) = static_cast<U>(*(++it)); //1
    *(++dit) = static_cast<U>(*(++it)); //3
  
    //affine y
    *(++dit) = static_cast<U>(*(++it));   //0
    *(++dit) = static_cast<U>(*(++it)); //1
    *(++dit) = static_cast<U>(*(++it)); //3

    //the perspective denominator
    *(++dit) = static_cast<U>(*(++it));   //0
    *(++dit) = static_cast<U>(*(++it)); //1
    *(++dit) = static_cast<U>(1);       //3
    
    return true;
  }


  // -------------------------------------------------------------------
  // Required information
  // -------------------------------------------------------------------
  int homography8DofEstimator::minNumberCorrespondences() const {
    return 4;
  }

  int homography8DofEstimator::minCorrespondenceDimension() const {
    return 2;
  }

  int homography8DofEstimator::maxCorrespondenceDimension() const {
    return 2;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool homography8DofEstimator::apply(const matrix<ipoint>& src,
                                      fvector& dest) const {

    const parameters& par = getParameters();
    hom8DofHelper<float,int>  help ( par.computeSqError );
    return help.apply(src,dest);
  }
  
  bool homography8DofEstimator::apply(const matrix<ipoint>& src,
                                      fvector& dest, fvector& error) const {

    const parameters& par = getParameters();
    hom8DofHelper<float,int> help ( par.computeSqError );
    return help.apply(src,dest,error);
  }
  
  
  bool homography8DofEstimator::apply(const matrix<ipoint>& src,
                                      fvector& dest,
                                      const ivector& indices,
                                      int numCorrespondences) const {
    
    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    hom8DofHelper<float,int> help ( par.computeSqError );
    return help.apply(src,dest,indices,numCorrespondences);
  }
  
  bool homography8DofEstimator::apply(const matrix<ipoint>& src,
                                      fvector& dest, fvector& error,
                                      const ivector& indices,
                                      int numCorrespondences) const {

    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    hom8DofHelper<float,int> help ( par.computeSqError );
    return help.apply(src,dest,error,indices,numCorrespondences);
  }
  
  bool homography8DofEstimator::apply(const matrix<fpoint>& src,
                                      fvector& dest) const {

    const parameters& par = getParameters();
    hom8DofHelper<float,float>  help ( par.computeSqError );
    return help.apply(src,dest);
  }
  
  bool homography8DofEstimator::apply(const matrix<fpoint>& src,
                                      fvector& dest, fvector& error) const {

    const parameters& par = getParameters();
    hom8DofHelper<float,float> help ( par.computeSqError );
    return help.apply(src,dest,error);
  }
  
  
  bool homography8DofEstimator::apply(const matrix<fpoint>& src,
                                      fvector& dest,
                                      const ivector& indices,
                                      int numCorrespondences) const {
    
    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    hom8DofHelper<float,float> help ( par.computeSqError );
    return help.apply(src,dest,indices,numCorrespondences);
  }
  
  bool homography8DofEstimator::apply(const matrix<fpoint>& src,
                                      fvector& dest, fvector& error,
                                      const ivector& indices,
                                      int numCorrespondences) const {

    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    hom8DofHelper<float,float> help ( par.computeSqError );
    return help.apply(src,dest,error,indices,numCorrespondences);
  }
  
  bool homography8DofEstimator::apply(const matrix<dpoint>& src,
                                      dvector& dest) const {
    
    const parameters& par = getParameters();
    hom8DofHelper<double,double>  help ( par.computeSqError );
    return help.apply(src,dest);
  }
  
  bool homography8DofEstimator::apply(const matrix<dpoint>& src,
                                      dvector& dest, dvector& error) const {
    
    const parameters& par = getParameters();
    hom8DofHelper<double,double>  help ( par.computeSqError );
    return help.apply(src,dest,error);
  }
  
  
  bool homography8DofEstimator::apply(const matrix<dpoint>& src,
                                      dvector& dest,
                                      const ivector& indices,
                                      int numCorrespondences) const {

    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    hom8DofHelper<double,double>  help ( par.computeSqError );
    return help.apply(src,dest,indices,numCorrespondences);
  }
  
  bool homography8DofEstimator::apply(const matrix<dpoint>& src,
                                      dvector& dest, dvector& error,
                                      const ivector& indices,
                                      int numCorrespondences) const {
    
    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    hom8DofHelper<double,double>  help ( par.computeSqError );
    return help.apply(src,dest,error,indices,numCorrespondences);
  }

  // -------------------------------------------------------------------
  // The getResidual-methods!
  // -------------------------------------------------------------------

  bool homography8DofEstimator
  ::computeResidual(const matrix<fpoint >& src,
                const fvector& transform, fvector& dest) const {
    const parameters& par = getParameters();
    hom8DofHelper<float,float>  help ( par.computeSqError );
    if ( !help.computeResidual(src,transform,dest) ) {
      setStatusString("Transform must have 9 elements.");
    }
    return true;
  }

  bool homography8DofEstimator
  ::computeResidual(const matrix<dpoint >& src,
                const dvector& transform, dvector& dest) const {
    const parameters& par = getParameters();
    hom8DofHelper<double,double>  help ( par.computeSqError );
    if ( !help.computeResidual(src,transform,dest) ) {
      setStatusString("Transform must have 9 elements.");
    }
    return true;
  }
  // -------------------------------------------------------------------
  // The denormalize-methods!
  // -------------------------------------------------------------------

  bool homography8DofEstimator::denormalize(fvector& srcdest,
                                            const vector<fpoint>& scale,
                                            const vector<fpoint>& shift) const{
 
    const parameters& par = getParameters();
    hom8DofHelper<float,float>  help ( par.computeSqError );
    if ( !help.denormalize(srcdest,scale,shift) ) {
      setStatusString("Transform must have 8 elements.");
    }
    return true;
  }

  bool homography8DofEstimator::denormalize(dvector& srcdest,
                                            const vector<dpoint>& scale,
                                            const vector<dpoint>& shift) const{
  
    const parameters& par = getParameters();
    hom8DofHelper<double,double>  help ( par.computeSqError );
    if ( !help.denormalize(srcdest,scale,shift) ) {
      setStatusString("Transform must have 8 elements.");
    }
    return true;
  }

  // -------------------------------------------------------------------
  // The convert-methods!
  // -------------------------------------------------------------------

  bool homography8DofEstimator::convert(const fvector& src,
                                        hMatrix3D<float>& dest) const {
    
    const parameters& par = getParameters();
    hom8DofHelper<float,float>  help ( par.computeSqError );
    if ( !help.convert(src,dest) ) {
      setStatusString("src must contain 8 elements");
      return false;
    }
    return true;
  }

  bool homography8DofEstimator::convert(const dvector& src,
                                        hMatrix3D<float>& dest) const {
    
    const parameters& par = getParameters();
    hom8DofHelper<double,float>  help ( par.computeSqError );
    if ( !help.convert(src,dest) ) {
      setStatusString("src must contain 8 elements");
      return false;
    }
    return true;
  }

  bool homography8DofEstimator::convert(const fvector& src,
                                        fmatrix& dest) const {
    
    const parameters& par = getParameters();
    hom8DofHelper<float,float>  help ( par.computeSqError );
    if ( !help.convert(src,dest) ) {
      setStatusString("src must contain 8 elements");
      return false;
    }
    return true;
  }

  bool homography8DofEstimator::convert(const dvector& src, 
                                        dmatrix& dest) const {
    
    const parameters& par = getParameters();
    hom8DofHelper<double,double>  help ( par.computeSqError );
    if ( !help.convert(src,dest) ) {
      setStatusString("src must contain 8 elements");
      return false;
    }
    return true;
  }
  
}

#include "ltiUndebug.h"
