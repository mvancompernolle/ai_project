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
 * file .......: ltiCrossCorrelationCoefficient.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 26.1.2004
 * revisions ..: $Id: ltiCrossCorrelationCoefficient.cpp,v 1.8 2006/09/05 10:08:11 ltilib Exp $
 */

#include "ltiCrossCorrelationCoefficient.h"
#include "ltiHomographyEstimatorFactory.h"

#include "limits"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1

#include "ltiDebug.h"


namespace lti {
  // --------------------------------------------------
  // crossCorrelationCoefficient::parameters
  // --------------------------------------------------

  // default constructor
  crossCorrelationCoefficient::parameters::parameters()
    : featureExtractor::parameters() {
 
    window = int(10);
    transform = 0;
    _lti_debug("CrossCorrelationCoefficient::parameters: default constrcutor"
	       <<"\n");
  }

  // copy constructor
  crossCorrelationCoefficient::parameters::parameters(const parameters& other)
    : featureExtractor::parameters() {
    transform = 0;
    copy(other);
    _lti_debug("CrossCorrelationCoefficient::parameters: default constrcutor"
	       <<"\n");
  }

  // destructor
  crossCorrelationCoefficient::parameters::~parameters() {
    if ( notNull(transform) ) {
      delete transform;
      transform = 0;
      _lti_debug("destroyed transform"<<"\n");
    }
  }

  // get type name
  const char* crossCorrelationCoefficient::parameters::getTypeName() const {
    return "crossCorrelationCoefficient::parameters";
  }

  void crossCorrelationCoefficient::parameters
  ::setTransform(eTransformEstimatorType name) {
    
    if ( notNull(transform)) {
      delete transform;
      transform = 0;
    }
    homographyEstimatorFactory fact;
    transform = fact.newInstance(name);
  }

  void crossCorrelationCoefficient::parameters
  ::setTransform(const homographyEstimatorBase& method) {
    if ( notNull(transform) ) {
      delete transform;
      transform = 0;
    }
    transform = dynamic_cast<homographyEstimatorBase*>(method.clone());
  }

  bool crossCorrelationCoefficient::parameters::existsTransform() const {
   return notNull(transform);
  }

  
  const homographyEstimatorBase& 
  crossCorrelationCoefficient::parameters::getTransform() const {
    
    if ( isNull(transform) ) {
      throw exception("Null pointer to homographyEstimatorBase");
    }
    return *transform;
  }

  // copy member
  crossCorrelationCoefficient::parameters&
    crossCorrelationCoefficient::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    featureExtractor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    featureExtractor::parameters& (featureExtractor::parameters::* p_copy)
      (const featureExtractor::parameters&) =
      featureExtractor::parameters::copy;
    (this->*p_copy)(other);
# endif

    window = other.window;

    if ( notNull(transform)) {
      delete transform;
      transform = 0;
      _lti_debug("destroyed transform"<<"\n");
    }  
    
    if ( notNull(other.transform) ) {
      transform = dynamic_cast<homographyEstimatorBase*>
	(other.transform->clone());
      _lti_debug("cloned transform"<<"\n");
    }
    
    return *this;
  }

  // alias for copy member
  crossCorrelationCoefficient::parameters&
    crossCorrelationCoefficient::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* crossCorrelationCoefficient::parameters::clone() const {
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
  bool crossCorrelationCoefficient::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool crossCorrelationCoefficient::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"window",window);
      if ( notNull(transform) ) {
	lti::write(handler,"transform", transform->getTypeName());
	transform->getParameters().write(handler,complete);
      } else {
	lti::write(handler,"transform","NoTransformEstimator");
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && featureExtractor::parameters::write(handler,false);
# else
    bool (featureExtractor::parameters::* p_writeMS)
      (ioHandler&,const bool) const =
      featureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool crossCorrelationCoefficient::parameters::write(ioHandler& handler,
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
  bool crossCorrelationCoefficient::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool crossCorrelationCoefficient::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"window",window);

      if (notNull(transform)) {
	delete transform;
	transform = 0;
      }
      
      std::string str;
      lti::read(handler,"transform",str);
      //factory returns null-pointer if the class name is unknown
      homographyEstimatorFactory fact;
      transform = fact.newInstance(std::string("lti::")+str);
      if ( notNull(transform) ) {
	transform->read(handler,complete);
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && featureExtractor::parameters::read(handler,false);
# else
    bool (featureExtractor::parameters::* p_readMS)(ioHandler&,const bool) =
      featureExtractor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool crossCorrelationCoefficient::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // helper class
  // --------------------------------------------------
  // we need to insert a "very private" class  here, in order to cope with
  // a MS-VC++ bug
  //
  // this is a template class, which does the real job, but even if we would
  // like to have all methods as template methods of the real functor, visual
  // doesn't like it.
  template<class T, class U>
  class crossCorrHelper {
  public:
 
    crossCorrHelper(const crossCorrelationCoefficient::parameters& par)
      : m_params(par),
	m_statusStrg("") {
    };

    bool apply(const matrix<T>& src1, const matrix<T>& src2,
	       const pointList& controlPoints,
	       const vector<U>& transf,
	       U& dest);

    bool apply(const matrix<T>& src1, const matrix<T>& src2,
	       const pointList& pts1, const pointList& pts2,
	       U& dest);
    
    bool apply(const matrix<T>& src1, const matrix<T>& src2,
	       const ipoint& pt1, const ipoint& pt2,
	       U& dest);
    
    bool apply(const matrix<T>& src1, const matrix<T>& src2,
	       U& dest);
    
    bool apply(const vector<T>& src1, const vector<T>& src2,
	       U& dest);  

    const std::string& getStatusString() const;

  protected:
    void apply(const matrix<T>& src1, const matrix<T>& src2,
	       const ipoint& pt1, const ipoint& pt2,
	       const int& halfWindow, const U& scale,
	       std::vector<T>& values1, std::vector<T>& values2,
	       U& mean1, U& mean2) const;
    
    void apply(const matrix<T>& src1, const matrix<T>& src2,
	       const ipoint& pt, const vector<U>& transf,
	       const int& halfWindow, const U& scale,
	       std::vector<T>& values1, std::vector<T>& values2,
	       U& mean1, U& mean2) const;

    bool apply(const std::vector<T>& src1, const std::vector<T>& src2,
	       U mean1, U mean2, U& dest);
    
    // parameters
    const crossCorrelationCoefficient::parameters& m_params;
    std::string m_statusStrg;
  };

  template<class T, class U>
  inline const std::string& crossCorrHelper<T,U>::getStatusString() const {
    return m_statusStrg;
  }

  template<class T, class U>
  inline bool crossCorrHelper<T,U>
  ::apply(const matrix<T>& src1, const matrix<T>& src2,
	  const pointList& controlPoints,
	  const vector<U>& transf,
	  U& dest) {

    dest = static_cast<U>(0);
    U mean1 ( 0 );
    U mean2 ( 0 );
    std::vector<T> values1;
    std::vector<T> values2;

    if ( controlPoints.size() == 0 ) {
      m_statusStrg += std::string(" no control points ");
      return false;
    }
    const U scale ( static_cast<U>(1) / static_cast<U>(controlPoints.size()) );
    const int halfWindow ( m_params.window / 2 );
    pointList::const_iterator end ( controlPoints.end() );
    pointList::const_iterator it  ( controlPoints.begin() );
    
    if ( !m_params.existsTransform() ) {

      //special fast code without transform 
 
      //do not transform the control points
      for ( ; it!=end; ++it ) {
	apply(src1,src2,*it,*it,halfWindow,scale,
	      values1,values2,mean1,mean2);
      }

    } else {
      //do transform the control points
      for ( ; it!=end; ++it ) {
	apply(src1,src2,*it,transf,halfWindow,scale,
	      values1,values2,mean1,mean2);
      }
    }
    //true size not known apriori (transform + border condition )
    const U finalScale ( (static_cast<U>(values1.size()) * scale) );
    if ( finalScale < std::numeric_limits<U>::epsilon() ) {
      return false;
    }
    mean1 /= finalScale;
    mean2 /= finalScale;

    return apply(values1, values2, mean1, mean2, dest);
  };

  //does not consider transform
  template<class T, class U>
  inline bool crossCorrHelper<T,U>
  ::apply(const matrix<T>& src1, const matrix<T>& src2,
	  const pointList& pts1, const pointList& pts2,
	  U& dest) {

    dest = static_cast<U>(0);
    if ( pts1.size() != pts2.size() ) {
      m_statusStrg += std::string(" pointlist sizes do not match ");
      return false;
    }

    U mean1 ( 0 );
    U mean2 ( 0 );
    std::vector<T> values1;
    std::vector<T> values2;
    //to avoid overflow
    const U scale ( static_cast<U>(1) / static_cast<U>(pts1.size()) ); 
    const int halfWindow ( m_params.window / 2 );

    pointList::const_iterator end1 ( pts1.end() );
    pointList::const_iterator it1  ( pts1.begin() );
    pointList::const_iterator it2  ( pts2.begin() );
    for ( ; it1!=end1; ++it1, ++it2 ) {
      apply(src1,src2,*it1,*it2,halfWindow,scale,
	    values1,values2,mean1,mean2);
    }
    //true size not known apriori (transform + border condition )
    const U finalScale ( (static_cast<U>(values1.size()) * scale) );
    if ( finalScale < std::numeric_limits<U>::epsilon() ) {
      return false;
    }
    mean1 /= finalScale;
    mean2 /= finalScale;

    return apply(values1, values2, mean1, mean2, dest);
  }

  template<class T, class U>
  inline bool crossCorrHelper<T,U>::apply(const matrix<T>& src1,
					  const matrix<T>& src2,
					  const ipoint& pt1,
					  const ipoint& pt2,
					  U& dest) {

    dest = static_cast<U>(0);
    U mean1 ( 0 );
    U mean2 ( 0 );
    std::vector<T> values1;
    std::vector<T> values2;
    
    //compute scale from halfWindow due to possible truncation
    const int halfWindow ( m_params.window / 2 );
    const U scale ( static_cast<U>(1) / static_cast<U>( (halfWindow*2+1) * 
							(halfWindow*2+1) ) );
    
    apply(src1,src2,pt1,pt2,halfWindow,scale,values1,values2,mean1,mean2);
    //true size not known apriori (transform + border condition )
    const U finalScale ( (static_cast<U>(values1.size()) * scale) );
    if ( finalScale < std::numeric_limits<U>::epsilon() ) {
      return false;
    }
    mean1 /= finalScale;
    mean2 /= finalScale;

    return apply(values1, values2, mean1, mean2, dest);
  }

  template<class T, class U>
  inline bool crossCorrHelper<T,U>::apply(const matrix<T>& src1,
					  const matrix<T>& src2,
					  U& dest) {
    dest = static_cast<U>(0);
    if ( src1.rows() != src2.rows() ||
	 src1.columns() != src2.columns() ) {
      m_statusStrg += std::string(" src sizes do not match ");
      return false;
    }
    const U scale ( static_cast<U>(src1.rows() * src1.columns() ) );

    //compute the means
    std::vector<T> values1;
    std::vector<T> values2;
    U mean1 ( 0 ); 
    typename matrix<T>::const_iterator end ( src1.end() );
    typename matrix<T>::const_iterator it  ( src1.begin() );
    for ( ; it!=end; ++it ) {
      mean1 += static_cast<U>(*it) / scale;
      values1.push_back(*it);  //todo: inefficient, but easier interface
    }
    U mean2 ( 0 );
    end = src2.end();
    it = src2.begin();
    for ( ; it!=end; ++it ) {
      mean2 += static_cast<U>(*it) / scale;
      values2.push_back(*it);  //todo: inefficient, but easier interface
    }
    
    return apply(values1,values2,mean1,mean2,dest);
  };

  template<class T, class U>
  inline bool crossCorrHelper<T,U>::apply(const vector<T>& src1, 
					  const vector<T>& src2,
					  U& dest) {

    dest = static_cast<U>(0);
    if ( src1.size() != src2.size() ) {
      m_statusStrg += (" src sizes do not match ");
      return false;
    }
    const U scale ( static_cast<U>(src1.size()) );
    
    //compute the means
    std::vector<T> values1;
    std::vector<T> values2;
    U mean1 ( 0 );
    typename vector<T>::const_iterator end ( src1.end() );
    typename vector<T>::const_iterator it  ( src1.begin() );
    for ( ; it!=end; ++it ) {
      mean1 += static_cast<U>(*it) / scale;
      values1.push_back(*it);  //todo: inefficient, but easier interface
    }
    U mean2 ( 0 );
    end = src2.end();
    it = src2.begin();
    for ( ; it!=end; ++it ) {
      mean2 += static_cast<U>(*it) / scale;
      values2.push_back(*it);
    }
  
    return apply(values1,values2,mean1,mean2,dest);
  }; 

  //apply without transformation
  //values and mean are incremental
  template<class T, class U>
  inline void crossCorrHelper<T,U>::apply(const matrix<T>& src1,
					  const matrix<T>& src2,
					  const ipoint& pt1,
					  const ipoint& pt2,
					  const int& halfWindow,
					  const U& scale,
					  std::vector<T>& values1,
					  std::vector<T>& values2,
					  U& mean1, U& mean2) const {

    const ipoint& imgSize1 = src1.size();  //todo efficiency: here, fct stack ?
    const ipoint& imgSize2 = src2.size();  //     not: member <- thread safety

    //window and push_back all values inside
    int i ( -halfWindow );
    for ( ; i <= halfWindow; i++ ) {
      int j ( -halfWindow );
      for ( ; j <= halfWindow; j++ ) {
	
	const int x1 ( pt1.x + j );
	const int y1 ( pt1.y + i );
	const int x2 ( pt2.x + j );
	const int y2 ( pt2.y + i );
	
	//only points lying in both images are considered
	if ( x1 < 0 || y1 < 0 || x1 >= imgSize1.x || y1 >= imgSize1.y ||
	     x2 < 0 || y2 < 0 || x2 >= imgSize2.x || y2 >= imgSize2.y ) {
	  continue;
	}
	
	const T val1 ( src1.at(y1,x1) );  //todo efficiency: iterator
	const T val2 ( src2.at(y2,x2) );  //todo efficiency: iterator
	mean1 += static_cast<U>(val1) * scale;
	mean2 += static_cast<U>(val2) * scale;
	values1.push_back(val1);
	values2.push_back(val2);
      }
    }
  }

  //apply with transformation
  //values and mean are incremental
  template<class T, class U>
  inline void crossCorrHelper<T,U>::apply(const matrix<T>& src1,
					  const matrix<T>& src2,
					  const ipoint& pt,
					  const vector<U>& transf,
					  const int& halfWindow,
					  const U& scale,
					  std::vector<T>& values1,
					  std::vector<T>& values2,
					  U& mean1, U& mean2) const {

    const homographyEstimatorBase& transform = m_params.getTransform();  
    const ipoint& imgSize1 = src1.size();  //todo efficiency: here, fct stack ?
    const ipoint& imgSize2 = src2.size();  //     not: member <- thread safety
    
    int i ( -halfWindow );
    for ( ; i <= halfWindow; i++ ) {
      int j ( -halfWindow );
      for ( ; j <= halfWindow; j++ ) {
	
	//distort point pt1 into src2
	ipoint pt1 ( pt.x + j, pt.y + i );
	ipoint pt2 ( transform.transform(pt1, transf) );
	//only points lying in both images are considered
	if ( pt1.x < 0 || pt1.y < 0 ||
	     pt1.x >= imgSize1.x || pt1.y >= imgSize1.y ||
	     pt2.x < 0 || pt2.y < 0 ||
	     pt2.x >= imgSize2.x || pt2.y >= imgSize2.y ) {
	  continue;
	}
	const T val1 ( src1.at(pt1) ); //todo: iterator
	const T val2 ( src2.at(pt2) ); //iterator not useful
	mean1 += static_cast<U>(val1) * scale;
	mean2 += static_cast<U>(val2) * scale;
	values1.push_back(val1);
	values2.push_back(val2);
      }
    }
  }

  //finishes the computation
  template<class T, class U>
  inline bool crossCorrHelper<T,U>::apply(const std::vector<T>& src1, 
					  const std::vector<T>& src2,
					  U mean1, U mean2,
					  U& dest) {
    
    dest = static_cast<U>(0);
    if ( src1.size() != src2.size() || src1.size() == 0 ) {
      m_statusStrg += (" src sizes do not match ");
      return false;
    }
 
    typename std::vector<T>::const_iterator vend ( src1.end()   );
    typename std::vector<T>::const_iterator vit1 ( src1.begin() );
    typename std::vector<T>::const_iterator vit2 ( src2.begin() );
    U denom1 ( 0 );
    U denom2 ( 0 );
    for ( ; vit1!=vend; ++vit1, ++vit2) {
      const U tmp1 ( static_cast<U>(*vit1) - mean1 );
      const U tmp2 ( static_cast<U>(*vit2) - mean2 );
      dest += tmp1 * tmp2;
      denom1 += tmp1 * tmp1;
      denom2 += tmp2 * tmp2;
    }
    if ( denom1 > -std::numeric_limits<U>::epsilon() &&
	 denom1 < std::numeric_limits<U>::epsilon() ) {
      //src1 is constant
      if ( denom2 > -std::numeric_limits<U>::epsilon() &&
	   denom2 <  std::numeric_limits<U>::epsilon() &&
	   mean1 == mean2 ) {
	//src2 is constant too
	dest = static_cast<U>(1);
      } else {
	dest = static_cast<U>(0);
      }
    } else {
      denom1 *= denom2;
      dest *= dest / denom1;
      dest = sqrt(dest);
    }
    _lti_debug(" __ccH:finish " << dest << std::endl);
    return true;
  };
  
  // --------------------------------------------------
  // crossCorrelationCoefficient
  // --------------------------------------------------

  // default constructor
  crossCorrelationCoefficient::crossCorrelationCoefficient()
    : featureExtractor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  crossCorrelationCoefficient
  ::crossCorrelationCoefficient(const parameters& par)
    : featureExtractor() {
    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  crossCorrelationCoefficient
  ::crossCorrelationCoefficient(const crossCorrelationCoefficient& other)
    : featureExtractor() {
    copy(other);
  }

  // destructor
  crossCorrelationCoefficient::~crossCorrelationCoefficient() {
  }

  // returns the name of this type
  const char* crossCorrelationCoefficient::getTypeName() const {
    return "crossCorrelationCoefficient";
  }

  // copy member
  crossCorrelationCoefficient& crossCorrelationCoefficient
  ::copy(const crossCorrelationCoefficient& other) {
      featureExtractor::copy(other);

    return (*this);
  }

  // alias for copy member
  crossCorrelationCoefficient&
    crossCorrelationCoefficient
  ::operator=(const crossCorrelationCoefficient& other) {
    return (copy(other));
  }


  // clone member
  functor* crossCorrelationCoefficient::clone() const {
    return new crossCorrelationCoefficient(*this);
  }

  // return parameters
  const crossCorrelationCoefficient::parameters&
    crossCorrelationCoefficient::getParameters() const {
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

  bool crossCorrelationCoefficient::apply(const matrix<ubyte>& src1,	
					  const matrix<ubyte>& src2,
					  const pointList& controlPoints,
					  const fvector& transf,
					  float& dest) const {
 
    const parameters& par = getParameters();
    crossCorrHelper<ubyte,float> helper(par);
    if ( !helper.apply(src1,src2,controlPoints,transf,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  };
  
  bool crossCorrelationCoefficient::apply(const matrix<float>& src1,
					  const matrix<float>& src2,
					  const pointList& controlPoints,
					  const fvector& transf,
					  float& dest) const {
 
    const parameters& par = getParameters();
    crossCorrHelper<float,float> helper(par);
    if ( !helper.apply(src1,src2,controlPoints,transf,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  };
 
  bool crossCorrelationCoefficient::apply(const matrix<ubyte>& src1,	
					  const matrix<ubyte>& src2,
					  const pointList& controlPoints,
					  const dvector& transf,
					  double& dest) const {
 
    const parameters& par = getParameters();
    crossCorrHelper<ubyte,double> helper(par);
    if ( !helper.apply(src1,src2,controlPoints,transf,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  };
  
  bool crossCorrelationCoefficient::apply(const matrix<float>& src1,
					  const matrix<float>& src2,
					  const pointList& controlPoints,
					  const dvector& transf,
					  double& dest) const {
 
    const parameters& par = getParameters();
    crossCorrHelper<float,double> helper(par);
    if ( !helper.apply(src1,src2,controlPoints,transf,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  };
 

  bool crossCorrelationCoefficient::apply(const matrix<ubyte>& src1,
					  const matrix<ubyte>& src2,
					  const pointList& pts1,
					  const pointList& pts2,
					  float& dest) const {
    const parameters& par = getParameters();
    crossCorrHelper<ubyte,float> helper(par);
    if ( !helper.apply(src1,src2,pts1,pts2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  }

  bool crossCorrelationCoefficient::apply(const matrix<float>& src1,
					  const matrix<float>& src2,
					  const pointList& pts1,
					  const pointList& pts2,
					  float& dest) const {
    const parameters& par = getParameters();
    crossCorrHelper<float,float> helper(par);
    if ( !helper.apply(src1,src2,pts1,pts2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  }

  bool crossCorrelationCoefficient::apply(const matrix<ubyte>& src1,
					  const matrix<ubyte>& src2,
					  const pointList& pts1,
					  const pointList& pts2,
					  double& dest) const {

    const parameters& par = getParameters();
    crossCorrHelper<ubyte,double> helper(par);
    if (  !helper.apply(src1,src2,pts1,pts2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  }

  bool crossCorrelationCoefficient::apply(const matrix<float>& src1,
					  const matrix<float>& src2,
					  const pointList& pts1,
					  const pointList& pts2,
					  double& dest) const {
    const parameters& par = getParameters();
    crossCorrHelper<float,double> helper(par);
    if ( !helper.apply(src1,src2,pts1,pts2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  }

  bool crossCorrelationCoefficient::apply(const matrix<ubyte>& src1,
					  const matrix<ubyte>& src2,
					  const ipoint& pts1,
					  const ipoint& pts2,
					  float& dest) const {
    const parameters& par = getParameters();
    crossCorrHelper<ubyte,float> helper(par);
    if ( !helper.apply(src1,src2,pts1,pts2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  }
  
  bool crossCorrelationCoefficient::apply(const matrix<float>& src1,
					  const matrix<float>& src2,
					  const ipoint& pts1,
					  const ipoint& pts2,
					  float& dest) const {
    const parameters& par = getParameters();
    crossCorrHelper<float,float> helper(par);
    if ( !helper.apply(src1,src2,pts1,pts2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  }

  bool crossCorrelationCoefficient::apply(const matrix<ubyte>& src1,
					  const matrix<ubyte>& src2,
					  const ipoint& pts1,
					  const ipoint& pts2,
					  double& dest) const {

    const parameters& par = getParameters();
    crossCorrHelper<ubyte,double> helper(par);
    if ( !helper.apply(src1,src2,pts1,pts2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  }

  bool crossCorrelationCoefficient::apply(const matrix<float>& src1,
					  const matrix<float>& src2,
					  const ipoint& pts1,
					  const ipoint& pts2,
					  double& dest) const {
    const parameters& par = getParameters();
    crossCorrHelper<float,double> helper(par);
    if ( !helper.apply(src1,src2,pts1,pts2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  }

  bool crossCorrelationCoefficient::apply(const matrix<ubyte>& src1,
					  const matrix<ubyte>& src2,
					  float& dest) const {
    const parameters& par = getParameters();
    crossCorrHelper<ubyte,float> helper(par);
    if ( !helper.apply(src1,src2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  };

  bool crossCorrelationCoefficient::apply(const matrix<float>& src1,
					  const matrix<float>& src2,
					  float& dest) const {
    const parameters& par = getParameters();
    crossCorrHelper<float,float> helper(par);
    if ( !helper.apply(src1,src2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  };
  
  bool crossCorrelationCoefficient::apply(const matrix<ubyte>& src1,
					  const matrix<ubyte>& src2,
					  double& dest) const {
    const parameters& par = getParameters();
    crossCorrHelper<ubyte,double> helper(par);
    if ( !helper.apply(src1,src2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  };

  bool crossCorrelationCoefficient::apply(const matrix<float>& src1,
					  const matrix<float>& src2,
					  double& dest) const {
    const parameters& par = getParameters();
    crossCorrHelper<float,double> helper(par);
    if ( !helper.apply(src1,src2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  };

  bool crossCorrelationCoefficient::apply(const vector<ubyte>& src1,
					  const vector<ubyte>& src2,
					  float& dest) const {
    const parameters& par = getParameters();
    crossCorrHelper<ubyte,float> helper(par);
    if ( !helper.apply(src1,src2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  };
  
  bool crossCorrelationCoefficient::apply(const vector<float>& src1,
					  const vector<float>& src2,
					  float& dest) const {
    const parameters& par = getParameters();
    crossCorrHelper<float,float> helper(par);
    if ( !helper.apply(src1,src2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  }; 

  bool crossCorrelationCoefficient::apply(const vector<ubyte>& src1,
					  const vector<ubyte>& src2,
					  double& dest) const {
    const parameters& par = getParameters();
    crossCorrHelper<ubyte,double> helper(par);
    if ( !helper.apply(src1,src2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  };
  
  bool crossCorrelationCoefficient::apply(const vector<float>& src1,
					  const vector<float>& src2,
					  double& dest) const {
    const parameters& par = getParameters();
    crossCorrHelper<float,double> helper(par);
    if ( !helper.apply(src1,src2,dest) ) {
      setStatusString(helper.getStatusString().c_str());
      return false;
    }
    return true;
  }; 

}

#include "ltiUndebug.h"
