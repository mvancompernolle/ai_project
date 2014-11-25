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
 * file .......: ltiHomography9DofEstimator.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 14.4.2004
 * revisions ..: $Id: ltiHomography9DofEstimator.cpp,v 1.8 2006/09/05 10:16:42 ltilib Exp $
 */

#include "ltiHomography9DofEstimator.h"

#include "ltiUnifiedSVD.h"

#include "ltiMath.h"
#include <limits>

#undef _LTI_DEBUG
//#define _LTI_DEBUG 4

#include "ltiDebug.h"


#if defined(_LTI_DEBUG)
using std::cout;
using std::endl;
using std::flush;
#endif

namespace lti {
  // --------------------------------------------------
  // homography9DofEstimator::parameters
  // --------------------------------------------------

  // default constructor
  homography9DofEstimator::parameters::parameters()
    : homographyEstimatorBase::parameters() {
  }

  // copy constructor
  homography9DofEstimator::parameters::parameters(const parameters& other)
    : homographyEstimatorBase::parameters() {
    copy(other);
  }

  // destructor
  homography9DofEstimator::parameters::~parameters() {
  }

  // get type name
  const char* homography9DofEstimator::parameters::getTypeName() const {
    return "homography9DofEstimator::parameters";
  }

  // copy member

  homography9DofEstimator::parameters&
    homography9DofEstimator::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    homographyEstimatorBase::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    homographyEstimatorBase::parameters& (homographyEstimatorBase::parameters::* p_copy)
      (const homographyEstimatorBase::parameters&) =
      homographyEstimatorBase::parameters::copy;
    (this->*p_copy)(other);
# endif

    return *this;
  }

  // alias for copy member
  homography9DofEstimator::parameters&
    homography9DofEstimator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* homography9DofEstimator::parameters::clone() const {
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
  bool homography9DofEstimator::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool homography9DofEstimator::parameters::writeMS(ioHandler& handler,
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
    bool (homographyEstimatorBase::parameters::* p_writeMS)(ioHandler&,const bool) const =
      homographyEstimatorBase::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool homography9DofEstimator::parameters::write(ioHandler& handler,
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
  bool homography9DofEstimator::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool homography9DofEstimator::parameters::readMS(ioHandler& handler,
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
    bool (homographyEstimatorBase::parameters::* p_readMS)(ioHandler&,const bool) =
      homographyEstimatorBase::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool homography9DofEstimator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // homography9DofEstimator
  // --------------------------------------------------

  // default constructor
  homography9DofEstimator::homography9DofEstimator()
    : homographyEstimatorBase(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  homography9DofEstimator::homography9DofEstimator(const parameters& par)
    : homographyEstimatorBase() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  homography9DofEstimator::homography9DofEstimator(const homography9DofEstimator& other)
    : homographyEstimatorBase() {
    copy(other);
  }

  // destructor
  homography9DofEstimator::~homography9DofEstimator() {
  }

  // returns the name of this type
  const char* homography9DofEstimator::getTypeName() const {
    return "homography9DofEstimator";
  }

  // copy member
  homography9DofEstimator&
    homography9DofEstimator::copy(const homography9DofEstimator& other) {
      homographyEstimatorBase::copy(other);

    return (*this);
  }

  // alias for copy member
  homography9DofEstimator&
    homography9DofEstimator::operator=(const homography9DofEstimator& other) {
    return (copy(other));
  }


  // clone member
  functor* homography9DofEstimator::clone() const {
    return new homography9DofEstimator(*this);
  }

  // return parameters
  const homography9DofEstimator::parameters&
    homography9DofEstimator::getParameters() const {
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
  class hom9DofHelper {
  public:
    hom9DofHelper(const bool computeSqError) :
    computeSqError_ ( computeSqError ) {
    };

    ~hom9DofHelper() {
    };

    bool apply(const matrix<tpoint<U> >& src,
               vector<T>& dest) const;

    bool apply(const matrix<tpoint<U> >& src,
               vector<T>& dest, vector<T>& error) const;

    
    bool apply(const matrix<tpoint<U> >& src,
               vector<T>& dest,
               const ivector& indices,
               const int numCorrespondences) const;
  
    bool apply(const matrix<tpoint<U> >& src,
               vector<T>& dest, vector<T>& error,
               const ivector& indices,
               const int numCorrespondences) const;

    bool computeResidual(const matrix<tpoint<U> >& src,
                         const vector<T>& hom,
                         vector<T>& error) const;

    bool denormalize(vector<T>& srcdest,
                     const vector<tpoint<T> >& scale,
                     const vector<tpoint<T> >& shift) const;
    
    bool convert(const vector<T>& src,hMatrix3D<U>& dest) const;
  
    bool convert(const vector<T>& src, matrix<U>& dest) const;

  private:
    bool computeSqError_;
            
    void initialize(const matrix<tpoint<U> >& src, matrix<T>& At_A) const;

    void initialize(const matrix<tpoint<U> >& src, const ivector& indices,
                    const int numCorrespondences, matrix<T>& At_A) const;

    bool compute(matrix<T>& AtA, vector<T>& dest) const;

  };

  //------------------
  //the initialization
  //------------------

  template<class T, class U>
  inline void hom9DofHelper<T,U>::initialize(const matrix<tpoint<U> >& src,
                                             matrix<T>& AtA) const {

    //initialize A_transponed * A
    //AtA is composed of 4 symmetric submatrices
    //        | m11   0   m31 |
    // AtA =  |  0   m11  m32 |
    //        | m31  m32  m33 |

    //---------------------------------------------------
    //references for fast access of the vector's elements
    //---------------------------------------------------
    AtA.resize(9,9, T(0), false, true);
    typename matrix<T>::iterator it ( AtA.begin() );
    //1st row
    T& m11_0 = *it;
    T& m11_1 = *(++it);
    T& m11_2 = *(++it);
    *(++it) = T(0); *(++it) = T(0); *(++it) = T(0);
    T& m31_0 = *(++it);
    T& m31_1 = *(++it);
    T& m31_2 = *(++it);

    //2nd row
    T& a21   = *(++it);       //duplicate due to symmetry
    T& m11_3 = *(++it);
    T& m11_4 = *(++it);
    *(++it) = T(0); *(++it) = T(0); *(++it) = T(0);
    T& a27   = *(++it);       //duplicate due to symmetry
    T& m31_3 = *(++it);
    T& m31_4 = *(++it);

    //3rd row
    T& a31   = *(++it);      //duplicate due to symmetry
    T& a32   = *(++it);      //duplicate due to symmetry
    T& m11_5 = *(++it);
    m11_5   = static_cast<T>(src.columns());
    *(++it) = T(0); *(++it) = T(0); *(++it) = T(0);
    T& a37   = *(++it);      //duplicate due to symmetry
    T& a38   = *(++it);      //duplicate due to symmetry
    T& m31_5 = *(++it);
    
    //4th row
    *(++it) = T(0); *(++it) = T(0); *(++it) = T(0);
    T& a44   = *(++it);      //duplicate due to symmetry
    T& a45   = *(++it);      //duplicate due to symmetry
    T& a46   = *(++it);      //duplicate due to symmetry
    T& m32_0 = *(++it);
    T& m32_1 = *(++it);
    T& m32_2 = *(++it);

    //5th row
    *(++it) = T(0); *(++it) = T(0); *(++it) = T(0);
    T& a54   = *(++it);      //duplicate due to symmetry
    T& a55   = *(++it);      //duplicate due to symmetry
    T& a56   = *(++it);      //duplicate due to symmetry
    T& a57   = *(++it);      //duplicate due to symmetry
    T& m32_3 = *(++it);
    T& m32_4 = *(++it);

    //6th row
    *(++it) = T(0); *(++it) = T(0); *(++it) = T(0);
    T& a64   = *(++it);      //duplicate due to symmetry
    T& a65   = *(++it);      //duplicate due to symmetry
    T& a66   = *(++it);      //duplicate due to symmetry
    T& a67   = *(++it);      //duplicate due to symmetry
    T& a68   = *(++it);      //duplicate due to symmetry
    T& m32_5 = *(++it);

    //7th row   
    T& a71   = *(++it);      //duplicate due to symmetry
    T& a72   = *(++it);      //duplicate due to symmetry
    T& a73   = *(++it);      //duplicate due to symmetry
    T& a74   = *(++it);      //duplicate due to symmetry
    T& a75   = *(++it);      //duplicate due to symmetry
    T& a76   = *(++it);      //duplicate due to symmetry
    T& m33_0 = *(++it);
    T& m33_1 = *(++it);
    T& m33_2 = *(++it);

    //8th row
    T& a81   = *(++it);      //duplicate due to symmetry
    T& a82   = *(++it);      //duplicate due to symmetry
    T& a83   = *(++it);      //duplicate due to symmetry
    T& a84   = *(++it);      //duplicate due to symmetry
    T& a85   = *(++it);      //duplicate due to symmetry
    T& a86   = *(++it);      //duplicate due to symmetry
    T& a87   = *(++it);      //duplicate due to symmetry
    T& m33_3 = *(++it);
    T& m33_4 = *(++it);

    //9th row
    T& a91   = *(++it);      //duplicate due to symmetry
    T& a92   = *(++it);      //duplicate due to symmetry
    T& a93   = *(++it);      //duplicate due to symmetry
    T& a94   = *(++it);      //duplicate due to symmetry
    T& a95   = *(++it);      //duplicate due to symmetry
    T& a96   = *(++it);      //duplicate due to symmetry
    T& a97   = *(++it);      //duplicate due to symmetry
    T& a98   = *(++it);      //duplicate due to symmetry
    T& m33_5 = *(++it);

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

      m11_0 += sqX1;
      m11_1 += x1Y1;
      m11_2 += x1;
      m11_3 += sqY1;
      m11_4 += y1;

      const T x2 ( static_cast<T>((*it2).x) );
      const T y2 ( static_cast<T>((*it2).y) );
  
      m31_0 -= sqX1 * x2;
      m31_1 -= x1Y1 * x2;
      m31_2 -= x1 * x2;
      m31_3 -= sqY1 * x2;
      m31_4 -= y1 * x2;
      m31_5 -= x2;
  
      m32_0 -= sqX1 * y2;
      m32_1 -= x1Y1 * y2;
      m32_2 -= x1 * y2;
      m32_3 -= sqY1 * y2;
      m32_4 -= y1 * y2;
      m32_5 -= y2;

      const T tmp ( ( x2 * x2 ) + ( y2 * y2 ) );
      m33_0 += sqX1 * tmp;
      m33_1 += x1Y1 * tmp;
      m33_2 += x1 * tmp;
      m33_3 += sqY1 * tmp;
      m33_4 += y1 * tmp;
      m33_5 += tmp;
    }

    //-------------------------------------------------
    //initialize symmetric elements of A_transponed * A
    //-------------------------------------------------
    //2nd row
    a21 = m11_1;
    a27 = m31_1;
    //3rd row
    a31 = m11_2; a32 = m11_4;
    a37 = m31_2; a38 = m31_4;

    //4th row
    a44 = m11_0; a45 = m11_1; a46 = m11_2;
    //5th row
    a54 = m11_1; a55 = m11_3; a56 = m11_4;
    a57 = m32_1;
    //6th row
    a64 = m11_2; a65 = m11_4; a66 = m11_5;
    a67 = m32_2; a68 = m32_4;

    //7th row   
    a71 = m31_0; a72 = m31_1; a73 = m31_2;
    a74 = m32_0; a75 = m32_1; a76 = m32_2;
    //8th row 
    a81 = m31_1; a82 = m31_3; a83 = m31_4;
    a84 = m32_1; a85 = m32_3; a86 = m32_4;
    a87 = m33_1;
    //9th row
    a91 = m31_2; a92 = m31_4; a93 = m31_5;
    a94 = m32_2; a95 = m32_4; a96 = m32_5;
    a97 = m33_2; a98 = m33_4;

#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    cout << " At_A " << AtA << endl;
#endif
  }

  template<class T, class U>
  inline void hom9DofHelper<T,U>
  ::initialize(const matrix<tpoint<U> >& src, const ivector& indices,
               const int numCorrespondences, matrix<T>& AtA) const {
    
    //re-implemented due to efficiency

    //initialize A_transponed * A
    //AtA is composed of 4 symmetric submatrices
    //        | m11   0   m31 |
    // AtA =  |  0   m11  m32 |
    //        | m31  m32  m33 |

    //---------------------------------------------------
    //references for fast access of the vector's elements
    //---------------------------------------------------
    AtA.resize(9,9, T(0), false, true);
    typename matrix<T>::iterator it ( AtA.begin() );
    //1st row
    T& m11_0 = *it;
    T& m11_1 = *(++it);
    T& m11_2 = *(++it);
    *(++it) = T(0); *(++it) = T(0); *(++it) = T(0);
    T& m31_0 = *(++it);
    T& m31_1 = *(++it);
    T& m31_2 = *(++it);

    //2nd row
    T& a21   = *(++it);       //duplicate due to symmetry
    T& m11_3 = *(++it);
    T& m11_4 = *(++it);
    *(++it) = T(0); *(++it) = T(0); *(++it) = T(0);
    T& a27   = *(++it);       //duplicate due to symmetry
    T& m31_3 = *(++it);
    T& m31_4 = *(++it);

    //3rd row
    T& a31   = *(++it);      //duplicate due to symmetry
    T& a32   = *(++it);      //duplicate due to symmetry
    T& m11_5 = *(++it);
    m11_5   = static_cast<T>(src.columns());
    *(++it) = T(0); *(++it) = T(0); *(++it) = T(0);
    T& a37   = *(++it);      //duplicate due to symmetry
    T& a38   = *(++it);      //duplicate due to symmetry
    T& m31_5 = *(++it);
    
    //4th row
    *(++it) = T(0); *(++it) = T(0); *(++it) = T(0);
    T& a44   = *(++it);      //duplicate due to symmetry
    T& a45   = *(++it);      //duplicate due to symmetry
    T& a46   = *(++it);      //duplicate due to symmetry
    T& m32_0 = *(++it);
    T& m32_1 = *(++it);
    T& m32_2 = *(++it);

    //5th row
    *(++it) = T(0); *(++it) = T(0); *(++it) = T(0);
    T& a54   = *(++it);      //duplicate due to symmetry
    T& a55   = *(++it);      //duplicate due to symmetry
    T& a56   = *(++it);      //duplicate due to symmetry
    T& a57   = *(++it);      //duplicate due to symmetry
    T& m32_3 = *(++it);
    T& m32_4 = *(++it);

    //6th row
    *(++it) = T(0); *(++it) = T(0); *(++it) = T(0);
    T& a64   = *(++it);      //duplicate due to symmetry
    T& a65   = *(++it);      //duplicate due to symmetry
    T& a66   = *(++it);      //duplicate due to symmetry
    T& a67   = *(++it);      //duplicate due to symmetry
    T& a68   = *(++it);      //duplicate due to symmetry
    T& m32_5 = *(++it);

    //7th row   
    T& a71   = *(++it);      //duplicate due to symmetry
    T& a72   = *(++it);      //duplicate due to symmetry
    T& a73   = *(++it);      //duplicate due to symmetry
    T& a74   = *(++it);      //duplicate due to symmetry
    T& a75   = *(++it);      //duplicate due to symmetry
    T& a76   = *(++it);      //duplicate due to symmetry
    T& m33_0 = *(++it);
    T& m33_1 = *(++it);
    T& m33_2 = *(++it);

    //8th row
    T& a81   = *(++it);      //duplicate due to symmetry
    T& a82   = *(++it);      //duplicate due to symmetry
    T& a83   = *(++it);      //duplicate due to symmetry
    T& a84   = *(++it);      //duplicate due to symmetry
    T& a85   = *(++it);      //duplicate due to symmetry
    T& a86   = *(++it);      //duplicate due to symmetry
    T& a87   = *(++it);      //duplicate due to symmetry
    T& m33_3 = *(++it);
    T& m33_4 = *(++it);

    //9th row
    T& a91   = *(++it);      //duplicate due to symmetry
    T& a92   = *(++it);      //duplicate due to symmetry
    T& a93   = *(++it);      //duplicate due to symmetry
    T& a94   = *(++it);      //duplicate due to symmetry
    T& a95   = *(++it);      //duplicate due to symmetry
    T& a96   = *(++it);      //duplicate due to symmetry
    T& a97   = *(++it);      //duplicate due to symmetry
    T& a98   = *(++it);      //duplicate due to symmetry
    T& m33_5 = *(++it);

    //--------------------
    //init all submatrices
    //--------------------

    const vector<tpoint<U> >& pts1 = src.getRow(0);
    const vector<tpoint<U> >& pts2 = src.getRow(1);
    typename ivector::const_iterator iit  ( indices.begin() );
    typename ivector::const_iterator iend ( iit + numCorrespondences );
    for (; iit!=iend; ++iit) {
      
      const tpoint<U>& pt1 = pts1.at(*iit);
      const T x1 ( static_cast<T>(pt1.x) );
      const T y1 ( static_cast<T>(pt1.y) );
      const T sqX1 ( x1*x1 );
      const T sqY1 ( y1*y1 );
      const T x1Y1 ( x1*y1 );

      m11_0 += sqX1;
      m11_1 += x1Y1;
      m11_2 += x1;
      m11_3 += sqY1;
      m11_4 += y1;

      const tpoint<U>& pt2 = pts2.at(*iit);
      const T x2 ( static_cast<T>(pt2.x) );
      const T y2 ( static_cast<T>(pt2.y) );
  
      m31_0 -= sqX1 * x2;
      m31_1 -= x1Y1 * x2;
      m31_2 -= x1 * x2;
      m31_3 -= sqY1 * x2;
      m31_4 -= y1 * x2;
      m31_5 -= x2;
  
      m32_0 -= sqX1 * y2;
      m32_1 -= x1Y1 * y2;
      m32_2 -= x1 * y2;
      m32_3 -= sqY1 * y2;
      m32_4 -= y1 * y2;
      m32_5 -= y2;

      const T tmp ( ( x2 * x2 ) + ( y2 * y2 ) );
      m33_0 += sqX1 * tmp;
      m33_1 += x1Y1 * tmp;
      m33_2 += x1 * tmp;
      m33_3 += sqY1 * tmp;
      m33_4 += y1 * tmp;
      m33_5 += tmp;
    }

    //-------------------------------------------------
    //initialize symmetric elements of A_transponed * A
    //-------------------------------------------------
    //2nd row
    a21 = m11_1;
    a27 = m31_1;
    //3rd row
    a31 = m11_2; a32 = m11_4;
    a37 = m31_2; a38 = m31_4;

    //4th row
    a44 = m11_0; a45 = m11_1; a46 = m11_2;
    //5th row
    a54 = m11_1; a55 = m11_3; a56 = m11_4;
    a57 = m32_1;
    //6th row
    a64 = m11_2; a65 = m11_4; a66 = m11_5;
    a67 = m32_2; a68 = m32_4;

    //7th row   
    a71 = m31_0; a72 = m31_1; a73 = m31_2;
    a74 = m32_0; a75 = m32_1; a76 = m32_2;
    //8th row 
    a81 = m31_1; a82 = m31_3; a83 = m31_4;
    a84 = m32_1; a85 = m32_3; a86 = m32_4;
    a87 = m33_1;
    //9th row
    a91 = m31_2; a92 = m31_4; a93 = m31_5;
    a94 = m32_2; a95 = m32_4; a96 = m32_5;
    a97 = m33_2; a98 = m33_4;

#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    cout << " At_A " << AtA << endl;
#endif
  }

  //solve by singular value decomposition
  template<class T, class U>
  inline bool hom9DofHelper<T,U>::compute(matrix<T>& AtA,
                                          vector<T>& dest) const {

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


    return true;
  }

  //--------
  // applies
  //--------

  template<class T, class U>
  inline bool hom9DofHelper<T,U>::apply(const matrix<tpoint<U> >& src,
                                        vector<T>& dest) const {

    matrix<T> AtA;
    initialize(src, AtA);

    return compute(AtA, dest);
  }

  template<class T, class U>
  inline bool hom9DofHelper<T,U>::apply(const matrix<tpoint<U> >& src,
                                        vector<T>& dest,
                                        vector<T>& error) const {

    if ( !apply(src,dest) ) {
      return false;
    }
   
    return computeResidual(src, dest, error);
  }


  template<class T, class U>
  inline bool hom9DofHelper<T,U>::apply(const matrix<tpoint<U> >& src,
                                        vector<T>& dest,
                                        const ivector& indices,
                                        const int numCorrespondences) const {

    //use the 1st numCorrespondences indices
    matrix<T> AtA;
    initialize(src, indices, numCorrespondences, AtA);

    return compute(AtA, dest);
  }

  template<class T, class U>
  inline bool hom9DofHelper<T,U>::apply(const matrix<tpoint<U> >& src,
                                        vector<T>& dest,
                                        vector<T>& error,
                                        const ivector& indices,
                                        const int numCorrespondences) const {
  
    if ( !apply(src,dest,indices,numCorrespondences) ) {
      return false;
    }
    return computeResidual(src, dest, error);
  }

  template<class T, class U>
  inline bool hom9DofHelper<T,U>
  ::computeResidual(const matrix<tpoint<U> >& src,
                    const vector<T>& hom,
                    vector<T>& error) const { 

    if ( hom.size() != 9 ) {
      return false;
    }

    const int numElem ( src.columns() );
    error.resize(numElem, T(), false, false);
    typename vector<T>::iterator dit  ( error.begin() );
    typename vector<T>::iterator dend ( error.end() );
    typename matrix<tpoint<U> >::const_iterator it1 ( src.begin() );
    typename matrix<tpoint<U> >::const_iterator it2 ( it1 + numElem );
    T elemX, elemY;
    for ( ; dit!=dend; ++it1, ++it2, ++dit ) {

      const T x1 ( static_cast<T>((*it1).x) );
      const T y1 ( static_cast<T>((*it1).y) );
      const T x2 ( static_cast<T>((*it2).x) );
      const T y2 ( static_cast<T>((*it2).y) );
//       typename vector<T>::const_iterator it ( hom.begin() );
//       T elemX ( *it * x1 + *(++it) * y1 + *(++it) );
//       T elemY ( *(++it) * x1 + *(++it) * y1 + *(++it) );
//       const T denom ( *(++it) * x1 + *(++it) * y1 + *(++it) );
      elemX = hom.at(0) * x1 + hom.at(1) * y1 + hom.at(2);
      elemY = hom.at(3) * x1 + hom.at(4) * y1 + hom.at(5);
      const T denom ( hom.at(6) * x1 + hom.at(7) * y1 + hom.at(8) );
      elemX /= denom;
      elemY /= denom;
      elemX -= x2;
      elemY -= y2;
      *dit = elemX * elemX;
      *dit += elemY * elemY;
    }
    
    if ( !computeSqError_ ) {
      for ( dit = error.begin(); dit!=dend; ++dit ) {
        *dit = sqrt(*dit);
      }
    }
    
    return true;
  }

  template<class T, class U>  
  bool hom9DofHelper<T,U>::denormalize(vector<T>& srcdest,
                                       const vector<tpoint<T> >& scale,
                                       const vector<tpoint<T> >& shift) const {

    if ( srcdest.size() != 9 ) {
      return false;
    }
    
    // -- undo the normalization of the points to be transformed
    const tpoint<T>& otherScale = scale.at(0);
    const tpoint<T>& otherShift = shift.at(0);
    typename vector<T>::iterator hit ( srcdest.begin() );
    srcdest[2] += otherShift.x * (*hit) + otherShift.y * srcdest[1];
    *hit *= otherScale.x;         //0
    *(++hit) *= otherScale.y;     //1
    ++hit; //2
    ++hit; //3
    srcdest[5] += otherShift.x * (*hit) + otherShift.y * srcdest[4];
    *hit *= otherScale.x;      //3
    *(++hit) *= otherScale.y;  //4
    ++hit; //5
    ++hit; //6
    srcdest[8] += otherShift.x * (*hit) + otherShift.y * srcdest[7];
    *hit *= otherScale.x;
    *(++hit) *= otherScale.y;

    // -- undo normalization of the reference points 
    const tpoint<T>& refScale = scale.at(1);
    const tpoint<T>& refShift = shift.at(1);
    hit = srcdest.begin();               //0
    *hit -= srcdest[6] * refShift.x;
    *hit /= refScale.x;
    *(++hit) -= srcdest[7] * refShift.x; //1
    *hit    /= refScale.x;
    *(++hit) -= srcdest[8] * refShift.x; //2
    *hit    /= refScale.x;
    *(++hit) -= srcdest[6] * refShift.y; //3
    *hit    /= refScale.y;
    *(++hit) -= srcdest[7] * refShift.y; //4
    *hit    /= refScale.y;
    *(++hit) -= srcdest[8] * refShift.y; //5
    *hit /= refScale.y;
          
    return true;
  }

  template<class T, class U>
  inline bool  hom9DofHelper<T,U>::convert(const vector<T>& src,
                                           hMatrix3D<U>& dest) const {
    
    if ( src.size() != 9 ) {
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
    dest.at(3,3) = static_cast<U>(*(++it)); //todo: is this the right place?
    
    return true;
  }
  
  template<class T, class U>
  inline bool  hom9DofHelper<T,U>::convert(const vector<T>& src, 
                                           matrix<U>& dest) const {
    
    if ( src.size() != 9 ) {
      return false;
    }

    typename vector<T>::const_iterator it ( src.begin() );
    dest.resize(3,3,U(),false,false);
    typename matrix<U>::iterator dit ( dest.begin() );
    //affine x
    *dit  = static_cast<U>(*(it));      //0
    *(++dit) = static_cast<U>(*(++it)); //1
    *(++dit) = static_cast<U>(*(++it)); //3
  
    //affine y
    *(++dit) = static_cast<U>(*(++it)); //0
    *(++dit) = static_cast<U>(*(++it)); //1
    *(++dit) = static_cast<U>(*(++it)); //3

    //the perspective denominator
    *(++dit) = static_cast<U>(*(++it)); //0
    *(++dit) = static_cast<U>(*(++it)); //1
    *(++dit) = static_cast<U>(*(++it)); //3
    
    return true;
  }

  // -------------------------------------------------------------------
  // Required information
  // -------------------------------------------------------------------
  int homography9DofEstimator::minNumberCorrespondences() const {
    return 5;
  }

  int homography9DofEstimator::minCorrespondenceDimension() const {
    return 2;
  }

  int homography9DofEstimator::maxCorrespondenceDimension() const {
    return 2;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool homography9DofEstimator::apply(const matrix<ipoint>& src,
                                      fvector& dest) const {
    
    const parameters& par = getParameters();
    hom9DofHelper<float,int>  help ( par.computeSqError );
    return help.apply(src,dest);
  }
  
  bool homography9DofEstimator::apply(const matrix<ipoint>& src,
                                      fvector& dest, fvector& error) const {

    const parameters& par = getParameters();
    hom9DofHelper<float,int> help ( par.computeSqError );
    return help.apply(src,dest,error);
  }
  
  
  bool homography9DofEstimator::apply(const matrix<ipoint>& src,
                                      fvector& dest,
                                      const ivector& indices,
                                      int numCorrespondences) const {
    
    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    hom9DofHelper<float,int> help ( par.computeSqError );
    return help.apply(src,dest,indices,numCorrespondences);
  }
  
  bool homography9DofEstimator::apply(const matrix<ipoint>& src,
                                      fvector& dest, fvector& error,
                                      const ivector& indices,
                                      int numCorrespondences) const {

    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    hom9DofHelper<float,int> help ( par.computeSqError );
    return help.apply(src,dest,error,indices,numCorrespondences);
  }
 
  bool homography9DofEstimator::apply(const matrix<fpoint>& src,
                                      fvector& dest) const {

    const parameters& par = getParameters();
    hom9DofHelper<float,float>  help ( par.computeSqError );
    return help.apply(src,dest);
  }
  
  bool homography9DofEstimator::apply(const matrix<fpoint>& src,
                                      fvector& dest, fvector& error) const {

    const parameters& par = getParameters();
    hom9DofHelper<float,float> help ( par.computeSqError );
    return help.apply(src,dest,error);
  }
  
  
  bool homography9DofEstimator::apply(const matrix<fpoint>& src,
                                      fvector& dest,
                                      const ivector& indices,
                                      int numCorrespondences) const {
    
    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    hom9DofHelper<float,float> help ( par.computeSqError );
    return help.apply(src,dest,indices,numCorrespondences);
  }
  
  bool homography9DofEstimator::apply(const matrix<fpoint>& src,
                                      fvector& dest, fvector& error,
                                      const ivector& indices,
                                      int numCorrespondences) const {

    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    hom9DofHelper<float,float> help ( par.computeSqError );
    return help.apply(src,dest,error,indices,numCorrespondences);
  }
  
  bool homography9DofEstimator::apply(const matrix<dpoint>& src,
                                      dvector& dest) const {
    
    const parameters& par = getParameters();
    hom9DofHelper<double,double>  help ( par.computeSqError );
    return help.apply(src,dest);
  }
  
  bool homography9DofEstimator::apply(const matrix<dpoint>& src,
                                      dvector& dest, dvector& error) const {
    
    const parameters& par = getParameters();
    hom9DofHelper<double,double>  help ( par.computeSqError );
    return help.apply(src,dest,error);
  }
  
  
  bool homography9DofEstimator::apply(const matrix<dpoint>& src,
                                      dvector& dest,
                                      const ivector& indices,
                                      int numCorrespondences) const {

    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    hom9DofHelper<double,double>  help ( par.computeSqError );
    return help.apply(src,dest,indices,numCorrespondences);
  }
  
  bool homography9DofEstimator::apply(const matrix<dpoint>& src,
                                      dvector& dest, dvector& error,
                                      const ivector& indices,
                                      int numCorrespondences) const {
    
    if ( numCorrespondences < minNumberCorrespondences() ||
         indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    hom9DofHelper<double,double>  help ( par.computeSqError );
    return help.apply(src,dest,error,indices,numCorrespondences);
  }
  
  // -------------------------------------------------------------------
  // The getResidual-methods!
  // -------------------------------------------------------------------

  bool homography9DofEstimator
  ::computeResidual(const matrix<fpoint >& src,
                const fvector& transform, fvector& dest) const {
 
    const parameters& par = getParameters();
    hom9DofHelper<float,float>  help ( par.computeSqError );
    if ( !help.computeResidual(src,transform,dest) ) {
      setStatusString("Transform must have 9 elements.");
    }
    return true;
  }

  bool homography9DofEstimator
  ::computeResidual(const matrix<dpoint >& src,
                    const dvector& transform, dvector& dest) const {

    const parameters& par = getParameters();
    hom9DofHelper<double,double>  help ( par.computeSqError );
    if ( !help.computeResidual(src,transform,dest) ) {
      setStatusString("Transform must have 9 elements.");
    }
    return true;
  }

  // -------------------------------------------------------------------
  // The denormalize-methods!
  // -------------------------------------------------------------------

  bool homography9DofEstimator::denormalize(fvector& srcdest,
                                            const vector<fpoint>& scale,
                                            const vector<fpoint>& shift) const{
 
    const parameters& par = getParameters();
    hom9DofHelper<float,float>  help ( par.computeSqError );
    if ( !help.denormalize(srcdest,scale,shift) ) {
      setStatusString("Transform must have 9 elements.");
    }
    return true;
  }

  bool homography9DofEstimator::denormalize(dvector& srcdest,
                                            const vector<dpoint>& scale,
                                            const vector<dpoint>& shift) const{
  
    const parameters& par = getParameters();
    hom9DofHelper<double,double>  help ( par.computeSqError );
    if ( !help.denormalize(srcdest,scale,shift) ) {
      setStatusString("Transform must have 9 elements.");
    }
    return true;
  }

  // -------------------------------------------------------------------
  // The convert-methods!
  // -------------------------------------------------------------------

  bool homography9DofEstimator::convert(const fvector& src,
                                        hMatrix3D<float>& dest) const {
    
    const parameters& par = getParameters();
    hom9DofHelper<float,float>  help ( par.computeSqError );
    if ( !help.convert(src,dest) ) {
      setStatusString("src must contain 9 elements");
      return false;
    }
    return true;
  }

  bool homography9DofEstimator::convert(const dvector& src,
                                        hMatrix3D<float>& dest) const {
    
    const parameters& par = getParameters();
    hom9DofHelper<double,float>  help ( par.computeSqError );
    if ( !help.convert(src,dest) ) {
      setStatusString("src must contain 9 elements");
      return false;
    }
    return true;
  }

  bool homography9DofEstimator::convert(const fvector& src,
                                        fmatrix& dest) const {
    
    const parameters& par = getParameters();
    hom9DofHelper<float,float>  help ( par.computeSqError );
    if ( !help.convert(src,dest) ) {
      setStatusString("src must contain 9 elements");
      return false;
    }
    return true;
  }

  bool homography9DofEstimator::convert(const dvector& src, 
                                        dmatrix& dest) const {
    
    const parameters& par = getParameters();
    hom9DofHelper<double,double>  help ( par.computeSqError );
    if ( !help.convert(src,dest) ) {
      setStatusString("src must contain 9 elements");
      return false;
    }
    return true;
  }
  
}

#include "ltiUndebug.h"
