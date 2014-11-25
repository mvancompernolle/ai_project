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
 * file .......: ltiTranslationScaleEstimator.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 14.4.2004
 * revisions ..: $Id: ltiTranslationScaleEstimator.cpp,v 1.6 2006/09/05 10:32:57 ltilib Exp $
 */

#include "ltiTranslationScaleEstimator.h"
#include "ltiCholeskyDecomposition.h"
#include "ltiForwardSubstitution.h"
#include "ltiBackSubstitution.h"

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
  // translationScaleEstimator::parameters
  // --------------------------------------------------

  // default constructor
  translationScaleEstimator::parameters::parameters()
    : homographyEstimatorBase::parameters() {
  }

  // copy constructor
  translationScaleEstimator::parameters::parameters(const parameters& other)
    : homographyEstimatorBase::parameters() {
    copy(other);
  }

  // destructor
  translationScaleEstimator::parameters::~parameters() {
  }

  // get type name
  const char* translationScaleEstimator::parameters::getTypeName() const {
    return "translationScaleEstimator::parameters";
  }

  // copy member

  translationScaleEstimator::parameters&
    translationScaleEstimator::parameters::copy(const parameters& other) {
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
  translationScaleEstimator::parameters&
    translationScaleEstimator::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* translationScaleEstimator::parameters::clone() const {
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
  bool translationScaleEstimator::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool translationScaleEstimator::parameters::writeMS(ioHandler& handler,
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
  bool translationScaleEstimator::parameters::write(ioHandler& handler,
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
  bool translationScaleEstimator::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool translationScaleEstimator::parameters::readMS(ioHandler& handler,
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
  bool translationScaleEstimator::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // translationScaleEstimator
  // --------------------------------------------------

  // default constructor
  translationScaleEstimator::translationScaleEstimator()
    : homographyEstimatorBase(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  translationScaleEstimator::translationScaleEstimator(const parameters& par)
    : homographyEstimatorBase() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  translationScaleEstimator::translationScaleEstimator(const translationScaleEstimator& other)
    : homographyEstimatorBase() {
    copy(other);
  }

  // destructor
  translationScaleEstimator::~translationScaleEstimator() {
  }

  // returns the name of this type
  const char* translationScaleEstimator::getTypeName() const {
    return "translationScaleEstimator";
  }

  // copy member
  translationScaleEstimator&
    translationScaleEstimator::copy(const translationScaleEstimator& other) {
      homographyEstimatorBase::copy(other);

    return (*this);
  }

  // alias for copy member
  translationScaleEstimator&
    translationScaleEstimator::operator=(const translationScaleEstimator& other) {
    return (copy(other));
  }


  // clone member
  functor* translationScaleEstimator::clone() const {
    return new translationScaleEstimator(*this);
  }

  // return parameters
  const translationScaleEstimator::parameters&
    translationScaleEstimator::getParameters() const {
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
  class translScaleHelper {
  public:
    translScaleHelper(const bool computeSqError) :
    computeSqError_ ( computeSqError ) {
    };

    ~translScaleHelper() {
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

    bool convert(const vector<T>& src,hMatrix3D<U>& dest) const;
  
    bool convert(const vector<T>& src, matrix<U>& dest) const;

    bool denormalize(vector<T>& srcdest,
		     const vector<tpoint<T> >& scale,
		     const vector<tpoint<T> >& shift) const;
    
  private:
    bool computeSqError_;
    	    
    void initialize(const matrix<tpoint<U> >& src, 
		    matrix<T>& At_A, vector<T>& b) const;

    void initialize(const matrix<tpoint<U> >& src, const ivector& indices,
		    const int numCorrespondences,
		    matrix<T>& At_A, vector<T>& b) const;

    bool compute(matrix<T>& AtA, vector<T>& dest) const;

  };

  //------------------
  //the initialization
  //------------------

  template<class T, class U>
  inline void translScaleHelper<T,U> 
  ::initialize(const matrix<tpoint<U> >& src,
	       matrix<T>& AtA,
	       vector<T>& b) const {

    //initialize A_transponed * A (symmetric)
    //        | a11  a12  a13 |
    // AtA =  | a12   n    0 |
    //        | a13   0    n |

    //---------------------------------------------------
    //references for fast access of the vector's elements
    //---------------------------------------------------
    //compared to homography9DofEstimator the 2nd, 4th and 7th row
    //and column are missing
    AtA.resize(3,3, T(0), false, false);
    typename matrix<T>::iterator it ( AtA.begin() );
    //1st row
    T& a11 = *it;
    T& a12 = *(++it);
    T& a13 = *(++it);

    //2nd row
    T& d21   = *(++it);       //duplicate due to symmetry
    *(++it) = static_cast<T>(src.columns());  //n
    *(++it) = T(0);
 
    //3rd row
    T& d31   = *(++it);       //duplicate due to symmetry
    *(++it) = T(0);
    *(++it) = static_cast<T>(src.columns());  //n

    //b
    b.resize(3, T(0), false, false);
    typename vector<T>::iterator vit ( b.begin() );
    T& b1 = *vit;
    T& b2 = *(++vit);
    T& b3 = *(++vit);

    //--------------------
    //init all submatrices
    //--------------------
    a11 = static_cast<T>(0);
    a12 = static_cast<T>(0);
    a13 = static_cast<T>(0);
    b1 = static_cast<T>(0);
    b2 = static_cast<T>(0);
    b3 = static_cast<T>(0);

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
  
      a11 += sqX1 + sqY1;
      a12 += x1;
      a13 += y1;
 
      const T x2 ( static_cast<T>((*it2).x) );
      const T y2 ( static_cast<T>((*it2).y) );
      
      b1 += x1 * x2 + y1 * y2;
      b2 += x2;
      b3 += y2;
    }

    //-------------------------------------------------
    //initialize symmetric elements of A_transponed * A
    //-------------------------------------------------
    d21 = a12;
    d31 = a13;

#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    cout << " At_A " << AtA << endl
	 << " b " << b << endl << flush;
#endif
  }

  template<class T, class U>
  inline void translScaleHelper<T,U>
  ::initialize(const matrix<tpoint<U> >& src, const ivector& indices,
	       const int numCorrespondences,
	       matrix<T>& AtA, vector<T>& b) const {
 
    //initialize A_transponed * A (symmetric)
    //        | a11  a12  a13 |
    // AtA =  | a12   n    0 |
    //        | a13   0    n |

    //---------------------------------------------------
    //references for fast access of the vector's elements
    //---------------------------------------------------
    //compared to homography9DofEstimator the 2nd, 4th and 7th row
    //and column are missing
    AtA.resize(3,3, T(0), false, false);
    typename matrix<T>::iterator it ( AtA.begin() );
    //1st row
    T& a11 = *it;
    T& a12 = *(++it);
    T& a13 = *(++it);

    //2nd row
    T& d21   = *(++it);       //duplicate due to symmetry
    *(++it) = static_cast<T>(src.columns());  //n
    *(++it) = T(0);
 
    //3rd row
    T& d31   = *(++it);       //duplicate due to symmetry
    *(++it) = T(0);
    *(++it) = static_cast<T>(src.columns());  //n

    //b
    b.resize(3, T(0), false, false);
    typename vector<T>::iterator vit ( b.begin() );
    T& b1 = *vit;
    T& b2 = *(++vit);
    T& b3 = *(++vit);

    //--------------------
    //init all submatrices
    //--------------------
    a11 = static_cast<T>(0);
    a12 = static_cast<T>(0);
    a13 = static_cast<T>(0);
    b1 = static_cast<T>(0);
    b2 = static_cast<T>(0);
    b3 = static_cast<T>(0);

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
 
      a11 += sqX1 + sqY1;
      a12 += x1;
      a13 += y1;
 
      const tpoint<U>& pt2 = pts2.at(*iit);
      const T x2 ( static_cast<T>(pt2.x) );
      const T y2 ( static_cast<T>(pt2.y) );

      b1 += x1 * x2 + y1 * x2;
      b2 += x2;
      b3 += y2;
    }

    //-------------------------------------------------
    //initialize symmetric elements of A_transponed * A
    //-------------------------------------------------
    d21 = a12;
    d31 = a13;

#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    cout << " At_A " << AtA << endl
	 << " b " << b << endl << flush;
#endif
  }

  //solve by singular value decomposition
  template<class T, class U>
  inline bool translScaleHelper<T,U>::compute(matrix<T>& AtA,
						vector<T>& dest) const {

    choleskyDecomposition<T> cholesky;
    if ( !cholesky.apply(AtA) ) {
      return false;
    }
    forwardSubstitution<T> fSubst;
    fSubst.apply(AtA, dest, forwardSubstitution<T>::parameters::Upper);

#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    _lti_debug("cholesky:" << endl << AtA << endl);
    _lti_debug("forward substituion:" << endl << dest <<endl);
#endif

    backSubstitution<T> bSubst;
    bSubst.apply(AtA, dest, backSubstitution<T>::parameters::Upper);

#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 3)
    _lti_debug("backward substituion:" << endl << dest <<endl);
#endif

    return true;
  }

  //--------
  // applies
  //--------

  template<class T, class U>
  inline bool translScaleHelper<T,U>::apply(const matrix<tpoint<U> >& src,
					      vector<T>& dest) const {

    matrix<T> AtA;
    initialize(src, AtA, dest);

    return compute(AtA, dest);
  }

  template<class T, class U>
  inline bool translScaleHelper<T,U>::apply(const matrix<tpoint<U> >& src,
					      vector<T>& dest,
					      vector<T>& error) const {

    if ( !apply(src,dest) ) {
      return false;
    }
   
    return computeResidual(src, dest, error);
  }


  template<class T, class U>
  inline bool translScaleHelper<T,U>
  ::apply(const matrix<tpoint<U> >& src, vector<T>& dest,
	  const ivector& indices, const int numCorrespondences) const {

    //use the 1st numCorrespondences indices
    matrix<T> AtA;
    initialize(src, indices, numCorrespondences, AtA, dest);

    return compute(AtA, dest);
  }

  template<class T, class U>
  inline bool translScaleHelper<T,U>
  ::apply(const matrix<tpoint<U> >& src, vector<T>& dest, vector<T>& error,
	  const ivector& indices, const int numCorrespondences) const {
  
    if ( !apply(src,dest,indices,numCorrespondences) ) {
      return false;
    }
    return computeResidual(src, dest, error);
  }

  template<class T, class U>
  inline bool translScaleHelper<T,U>
  ::computeResidual(const matrix<tpoint<U> >& src,
		    const vector<T>& hom,
		    vector<T>& error) const { 

    if ( hom.size() != 3 ) {
      return false;
    }

    const int numElem ( src.columns() );
    error.resize(numElem, T(), false, false);
    typename vector<T>::iterator dit  ( error.begin() );
    typename vector<T>::iterator dend ( error.end() );
    typename matrix<tpoint<U> >::const_iterator it1 ( src.begin() );
    typename matrix<tpoint<U> >::const_iterator it2 ( it1 + numElem );
    for ( ; dit!=dend; ++it1, ++it2, ++dit ) {

      const T x1 ( static_cast<T>((*it1).x) );
      const T y1 ( static_cast<T>((*it1).y) );
      const T x2 ( static_cast<T>((*it2).x) );
      const T y2 ( static_cast<T>((*it2).y) );
      typename vector<T>::const_iterator it ( hom.begin() );
      T elemX ( *it * x1 );
      T elemY ( *it * y1 );
      elemX += *(++it);
      elemY += *(++it);
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
  bool translScaleHelper<T,U>::denormalize(vector<T>& srcdest,
				       const vector<tpoint<T> >& scale,
				       const vector<tpoint<T> >& shift) const
  {
    if ( srcdest.size() != 3 ) {
      return false;
    }
    
    // -- undo the normalization of the points to be transformed
    const tpoint<T>& otherScale = scale.at(0);
    const tpoint<T>& otherShift = shift.at(0);

    typename vector<T>::iterator hit ( srcdest.begin() );
    srcdest[1] += otherShift.x * (*hit);
    srcdest[2] += otherShift.y * (*hit);
    *hit *= otherScale.x;
    
    // -- undo normalization of the reference points
    const tpoint<T>& refScale = scale.at(1);
    const tpoint<T>& refShift = shift.at(1);
    *hit /= refScale.x;
    *(++hit) -= refShift.x;
    *hit    /= refScale.x;
    *(++hit) -= refShift.y;
    *hit    /= refScale.x;

    return true;
  }

  template<class T, class U>
  inline bool translScaleHelper<T,U>::convert(const vector<T>& src,
						 hMatrix3D<U>& dest) const {
    
    if ( src.size() != 3 ) {
      return false;
    }

    typename vector<T>::const_iterator it ( src.begin() );
    //hMatrix3D<U>::iterator dit ( dest.begin() ); //does not exist
    //affine x
    dest.at(0,0) = static_cast<U>(*(it));   //0
    dest.at(0,1) = static_cast<U>(0);
    dest.at(0,2) = static_cast<U>(0);
    dest.at(0,3) = static_cast<U>(*(++it)); //3
  
    //affine y
    dest.at(1,0) = static_cast<U>(0); //0
    dest.at(1,1) = static_cast<U>(src.at(0)); //1
    dest.at(1,2) = static_cast<U>(0);
    dest.at(1,3) = static_cast<U>(*(++it)); //3

    dest.at(2,0) = static_cast<U>(0);
    dest.at(2,1) = static_cast<U>(0);
    dest.at(2,2) = static_cast<U>(1);
    dest.at(2,3) = static_cast<U>(0);

    dest.at(3,0) = static_cast<U>(0);   //0
    dest.at(3,1) = static_cast<U>(0);   //1
    dest.at(3,2) = static_cast<U>(0);
    dest.at(3,3) = static_cast<U>(1);
    
    return true;
  }
  
  template<class T, class U>
  inline bool  translScaleHelper<T,U>::convert(const vector<T>& src, 
					       matrix<U>& dest) const {
    
    if ( src.size() != 3 ) {
      return false;
    }

    typename vector<T>::const_iterator it ( src.begin() );
    dest.resize(3,3,U(),false,false);
    typename matrix<U>::iterator dit ( dest.begin() );
    //affine x
    *dit  = static_cast<U>(*(it));      //0
    *(++dit) = static_cast<U>(0); //1
    *(++dit) = static_cast<U>(*(++it)); //3
  
    //affine y
    *(++dit) = static_cast<U>(0); //0
    *(++dit) = static_cast<U>(src.at(0)); //1
    *(++dit) = static_cast<U>(*(++it)); //3

    //the perspective denominator
    *(++dit) = static_cast<U>(0); //0
    *(++dit) = static_cast<U>(0); //1
    *(++dit) = static_cast<U>(1); //3
    
    return true;
  }

  // -------------------------------------------------------------------
  // Required information
  // -------------------------------------------------------------------
  int translationScaleEstimator::minNumberCorrespondences() const {
    return 2;
  }

  int translationScaleEstimator::minCorrespondenceDimension() const {
    return 2;
  }

  int translationScaleEstimator::maxCorrespondenceDimension() const {
    return 2;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool translationScaleEstimator::apply(const matrix<ipoint>& src,
				      fvector& dest) const {
    
    const parameters& par = getParameters();
    translScaleHelper<float,int>  help ( par.computeSqError );
    return help.apply(src,dest);
  }
  
  bool translationScaleEstimator::apply(const matrix<ipoint>& src,
				      fvector& dest, fvector& error) const {

    const parameters& par = getParameters();
    translScaleHelper<float,int> help ( par.computeSqError );
    return help.apply(src,dest,error);
  }
  
  
  bool translationScaleEstimator::apply(const matrix<ipoint>& src,
				      fvector& dest,
				      const ivector& indices,
				      int numCorrespondences) const {
    
    if ( numCorrespondences < minNumberCorrespondences() ||
	 indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    translScaleHelper<float,int> help ( par.computeSqError );
    return help.apply(src,dest,indices,numCorrespondences);
  }
  
  bool translationScaleEstimator::apply(const matrix<ipoint>& src,
				      fvector& dest, fvector& error,
				      const ivector& indices,
				      int numCorrespondences) const {

    if ( numCorrespondences < minNumberCorrespondences() ||
	 indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    translScaleHelper<float,int> help ( par.computeSqError );
    return help.apply(src,dest,error,indices,numCorrespondences);
  }
 
  bool translationScaleEstimator::apply(const matrix<fpoint>& src,
				      fvector& dest) const {

    const parameters& par = getParameters();
    translScaleHelper<float,float>  help ( par.computeSqError );
    return help.apply(src,dest);
  }
  
  bool translationScaleEstimator::apply(const matrix<fpoint>& src,
				      fvector& dest, fvector& error) const {

    const parameters& par = getParameters();
    translScaleHelper<float,float> help ( par.computeSqError );
    return help.apply(src,dest,error);
  }
  
  
  bool translationScaleEstimator::apply(const matrix<fpoint>& src,
				      fvector& dest,
				      const ivector& indices,
				      int numCorrespondences) const {
    
    if ( numCorrespondences < minNumberCorrespondences() ||
	 indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    translScaleHelper<float,float> help ( par.computeSqError );
    return help.apply(src,dest,indices,numCorrespondences);
  }
  
  bool translationScaleEstimator::apply(const matrix<fpoint>& src,
				      fvector& dest, fvector& error,
				      const ivector& indices,
				      int numCorrespondences) const {

    if ( numCorrespondences < minNumberCorrespondences() ||
	 indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    translScaleHelper<float,float> help ( par.computeSqError );
    return help.apply(src,dest,error,indices,numCorrespondences);
  }
  
  bool translationScaleEstimator::apply(const matrix<dpoint>& src,
				      dvector& dest) const {
    
    const parameters& par = getParameters();
    translScaleHelper<double,double>  help ( par.computeSqError );
    return help.apply(src,dest);
  }
  
  bool translationScaleEstimator::apply(const matrix<dpoint>& src,
				      dvector& dest, dvector& error) const {
    
    const parameters& par = getParameters();
    translScaleHelper<double,double>  help ( par.computeSqError );
    return help.apply(src,dest,error);
  }
  
  
  bool translationScaleEstimator::apply(const matrix<dpoint>& src,
				      dvector& dest,
				      const ivector& indices,
				      int numCorrespondences) const {

    if ( numCorrespondences < minNumberCorrespondences() ||
	 indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    translScaleHelper<double,double>  help ( par.computeSqError );
    return help.apply(src,dest,indices,numCorrespondences);
  }
  
  bool translationScaleEstimator::apply(const matrix<dpoint>& src,
				      dvector& dest, dvector& error,
				      const ivector& indices,
				      int numCorrespondences) const {
    
    if ( numCorrespondences < minNumberCorrespondences() ||
	 indices.size() < numCorrespondences ) {
      setStatusString("number of correspondences to small or too few indices");
      return false;
    } 
    const parameters& par = getParameters();
    translScaleHelper<double,double>  help ( par.computeSqError );
    return help.apply(src,dest,error,indices,numCorrespondences);
  }
  
  // -------------------------------------------------------------------
  // The getResidual-methods!
  // -------------------------------------------------------------------

  bool translationScaleEstimator
  ::computeResidual(const matrix<fpoint >& src,
		const fvector& transform, fvector& dest) const {
 
    const parameters& par = getParameters();
    translScaleHelper<float,float>  help ( par.computeSqError );
    if ( !help.computeResidual(src,transform,dest) ) {
      setStatusString("Transform must have 4 elements.");
    }
    return true;
  }

  bool translationScaleEstimator
  ::computeResidual(const matrix<dpoint >& src,
		const dvector& transform, dvector& dest) const {
    dvector res;
    const parameters& par = getParameters();
    translScaleHelper<double,double>  help ( par.computeSqError );
   if ( !help.computeResidual(src,transform,dest) ) {
      setStatusString("Transform must have 4 elements.");
    }
    return true;
  }
 
  // -------------------------------------------------------------------
  // The denormalize-methods!
  // -------------------------------------------------------------------
  
  bool translationScaleEstimator::denormalize(fvector& srcdest,
					    const vector<fpoint>& scale,
					    const vector<fpoint>& shift) const
  { 
    const parameters& par = getParameters();
    translScaleHelper<float,float>  help ( par.computeSqError );
    if ( !help.denormalize(srcdest,scale,shift) ) {
      setStatusString("Transform must have 3 elements.");
    }
    return true;
  }

  bool translationScaleEstimator::denormalize(dvector& srcdest,
					    const vector<dpoint>& scale,
					    const vector<dpoint>& shift) const
  {
    const parameters& par = getParameters();
    translScaleHelper<double,double>  help ( par.computeSqError );
    if ( !help.denormalize(srcdest,scale,shift) ) {
      setStatusString("Transform must have 3 elements.");
    }
    return true;
  }

  // -------------------------------------------------------------------
  // The convert-methods!
  // -------------------------------------------------------------------

  bool translationScaleEstimator::convert(const fvector& src,
					     hMatrix3D<float>& dest) const {
    
    const parameters& par = getParameters();
    translScaleHelper<float,float>  help ( par.computeSqError );
    if ( !help.convert(src,dest) ) {
      setStatusString("src must contain 3 elements");
      return false;
    }
    return true;
  }

  bool translationScaleEstimator::convert(const dvector& src,
					     hMatrix3D<float>& dest) const {
    
    const parameters& par = getParameters();
    translScaleHelper<double,float>  help ( par.computeSqError );
    if ( !help.convert(src,dest) ) {
      setStatusString("src must contain 3 elements");
      return false;
    }
    return true;
  }

  bool translationScaleEstimator::convert(const fvector& src,
					     fmatrix& dest) const {
    
    const parameters& par = getParameters();
    translScaleHelper<float,float>  help ( par.computeSqError );
    if ( !help.convert(src,dest) ) {
      setStatusString("src must contain 3 elements");
      return false;
    }
    return true;
  }

  bool translationScaleEstimator::convert(const dvector& src, 
					  dmatrix& dest) const {
    
    const parameters& par = getParameters();
    translScaleHelper<double,double>  help ( par.computeSqError );
    if ( !help.convert(src,dest) ) {
      setStatusString("src must contain 3 elements");
      return false;
    }
    return true;
  }
  
}

#include "ltiUndebug.h"
