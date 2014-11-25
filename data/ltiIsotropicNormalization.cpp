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
 * file .......: ltiIsotropicNormalization.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 29.3.2004
 * revisions ..: $Id: ltiIsotropicNormalization.cpp,v 1.7 2006/09/05 10:17:35 ltilib Exp $
 */

#include "ltiIsotropicNormalization.h"
#include "ltiHTypes.h"
#include "ltiMath.h"
#include "ltiL2Distance.h"

namespace lti {
  // --------------------------------------------------
  // isotropicNormalization::parameters
  // --------------------------------------------------

  // default constructor
  isotropicNormalization::parameters::parameters()
    : pointSetNormalization::parameters() {
 
    averageDistance = float(sqrt(2.));
  }

  // copy constructor
  isotropicNormalization::parameters::parameters(const parameters& other)
    : pointSetNormalization::parameters() {
    copy(other);
  }

  // destructor
  isotropicNormalization::parameters::~parameters() {
  }

  // get type name
  const char* isotropicNormalization::parameters::getTypeName() const {
    return "isotropicNormalization::parameters";
  }

  // copy member

  isotropicNormalization::parameters&
    isotropicNormalization::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    pointSetNormalization::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    pointSetNormalization::parameters& (pointSetNormalization::parameters::* p_copy)
      (const pointSetNormalization::parameters&) =
      pointSetNormalization::parameters::copy;
    (this->*p_copy)(other);
# endif

    
      averageDistance = other.averageDistance;

    return *this;
  }

  // alias for copy member
  isotropicNormalization::parameters&
    isotropicNormalization::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* isotropicNormalization::parameters::clone() const {
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
  bool isotropicNormalization::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool isotropicNormalization::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"averageDistance",averageDistance);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && pointSetNormalization::parameters::write(handler,false);
# else
    bool (pointSetNormalization::parameters::* p_writeMS)(ioHandler&,const bool) const =
      pointSetNormalization::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool isotropicNormalization::parameters::write(ioHandler& handler,
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
  bool isotropicNormalization::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool isotropicNormalization::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"averageDistance",averageDistance);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && pointSetNormalization::parameters::read(handler,false);
# else
    bool (pointSetNormalization::parameters::* p_readMS)
      (ioHandler&,const bool) =
      pointSetNormalization::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool isotropicNormalization::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // hidden template
  // --------------------------------------------------
  template <class T>
  class isotropicNormHelper {
  public:
    isotropicNormHelper(const float averageDist):
    averageDist_ ( static_cast<T>(averageDist) ) {};
    
    ~isotropicNormHelper() {};
    
    bool apply(const vector<tpoint<T> >& src,
	       vector<tpoint<T> >& dest) const;

    bool apply(const vector<tpoint<T> >& src,
	       vector<tpoint<T> >& dest,
	       tpoint<T>& scale, tpoint<T>& shift) const;

    bool apply(vector<tpoint<T> >& srcdest) const;

    bool apply(vector<tpoint<T> >& srcdest,
	       tpoint<T>& scale, tpoint<T>& shift) const;
  private:
    T averageDist_;
  };

  template <class T>
  inline bool isotropicNormHelper<T>::apply(vector<tpoint<T> >& srcdest) const{

    vector<tpoint<T> > tmp;
    tpoint<T> scale;
    tpoint<T> shift;
    if ( !apply(srcdest, tmp, scale, shift) ) {
      return false;
    }
    tmp.detach(srcdest);
    return true;
  }
  
  template <class T>
  inline bool isotropicNormHelper<T>::apply(vector<tpoint<T> >& srcdest,
					    tpoint<T>& scale,
					    tpoint<T>& shift) const{

    vector<tpoint<T> > tmp;
    if ( !apply(srcdest, tmp, scale, shift) ) {
      return false;
    }
    tmp.detach(srcdest);
    return true;
  }

  template <class T>
  inline bool isotropicNormHelper<T>::apply(const vector<tpoint<T> >& src,
					    vector<tpoint<T> >& dest) const{

    vector<tpoint<T> > tmp;
    tpoint<T> scale;
    tpoint<T> shift;
    return apply(src, dest, scale, shift);
  }

  template <class T>
  inline bool isotropicNormHelper<T>::apply(const vector<tpoint<T> >& src, 
					    vector<tpoint<T> >& dest,
					    tpoint<T>& scale,
					    tpoint<T>& shift) const {
    
    //build the normalization matrix
    typename vector<tpoint<T> >::const_iterator it  ( src.begin() );
    typename vector<tpoint<T> >::const_iterator eit ( src.end()   );

    // compute mean value
    shift = tpoint<T>( 0, 0 );
    
    for ( ; it!=eit; ++it) {
      shift.add(*it);
    }
    shift.divide(src.size());

    // compute scale factor as the mean distance to the mean point
    scale.x = static_cast<T>(0);
    for (it=src.begin(),eit=src.end();it!=eit;++it) {
      scale.x+=sqrt(shift.distanceSqr(*it));
    }
    scale.x/=src.size();
    scale.x = static_cast<T>(1)/scale.x;
    scale.x *= averageDist_;
    scale.y = scale.x;

    //scale the shift
    shift *= -scale.x;
 
    // create normMat
    // ** if ltilib wold be speeded up by assembler 
    // ** then hMatrix2D-multiply were faster
    hMatrix2D<T> normMat;
    normMat.unit(); // set a unit matrix first.
    
    normMat.at(0,0) = scale.x;
    normMat.at(1,1) = scale.x;
    normMat.at(0,2) = shift.x;
    normMat.at(1,2) = shift.y;

    //normalize the data
    dest.resize(src.size(),tpoint<T>(),false,false);
    typename vector<tpoint<T> >::iterator dit ( dest.begin() );
    for (it=src.begin(); it<eit; ++it,++dit) {
      normMat.multiply(*it,*dit);
    }

    return true;
  }
  
  // --------------------------------------------------
  // isotropicNormalization
  // --------------------------------------------------

  // default constructor
  isotropicNormalization::isotropicNormalization()
    : pointSetNormalization(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  isotropicNormalization::isotropicNormalization(const parameters& par)
    : pointSetNormalization() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  isotropicNormalization
  ::isotropicNormalization(const isotropicNormalization& other)
    : pointSetNormalization() {
    copy(other);
  }

  // destructor
  isotropicNormalization::~isotropicNormalization() {
  }

  // returns the name of this type
  const char* isotropicNormalization::getTypeName() const {
    return "isotropicNormalization";
  }

  // copy member
  isotropicNormalization&
    isotropicNormalization::copy(const isotropicNormalization& other) {
      pointSetNormalization::copy(other);

    return (*this);
  }

  // alias for copy member
  isotropicNormalization&
    isotropicNormalization::operator=(const isotropicNormalization& other) {
    return (copy(other));
  }


  // clone member
  functor* isotropicNormalization::clone() const {
    return new isotropicNormalization(*this);
  }

  // return parameters
  const isotropicNormalization::parameters&
    isotropicNormalization::getParameters() const {
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

  
  // On place apply for type vector<fpoint>!
  bool isotropicNormalization::apply(vector<fpoint>& srcdest) const {
    const parameters& par = getParameters();
    isotropicNormHelper<float>  help(par.averageDistance);
    return help.apply(srcdest);
  };

  bool isotropicNormalization::apply(vector<fpoint>& srcdest,
				     fpoint& scale, fpoint& shift) const {
    const parameters& par = getParameters();
    isotropicNormHelper<float>  help(par.averageDistance);
    return help.apply(srcdest,scale,shift);
  };

  // On place apply for type vector<dpoint>!
  bool isotropicNormalization::apply(vector<dpoint>& srcdest) const {
    const parameters& par = getParameters();
    isotropicNormHelper<double>  help(par.averageDistance);
    return help.apply(srcdest);
  };

  bool isotropicNormalization::apply(vector<dpoint>& srcdest,
				     dpoint& scale, dpoint& shift) const {
    const parameters& par = getParameters();
    isotropicNormHelper<double>  help(par.averageDistance);
    return help.apply(srcdest,scale,shift);
  };

  // On copy apply for type vector<fpoint>!
  bool isotropicNormalization::apply(const vector<fpoint>& src,
				     vector<fpoint>& dest) const {
    const parameters& par = getParameters();
    isotropicNormHelper<float>  help(par.averageDistance);
    return help.apply(src,dest);
  };

  bool isotropicNormalization::apply(const vector<fpoint>& src,
				     vector<fpoint>& dest,
				     fpoint& scale, fpoint& shift) const {
    const parameters& par = getParameters();
    isotropicNormHelper<float>  help(par.averageDistance);
    return help.apply(src,dest,scale,shift);
  };
  
  // On copy apply for type vector<dpoint>!
  bool isotropicNormalization::apply(const vector<dpoint>& src,
				     vector<dpoint>& dest) const {
    const parameters& par = getParameters();
    isotropicNormHelper<double>  help(par.averageDistance);
    return help.apply(src,dest);
  };
  
  bool isotropicNormalization::apply(const vector<dpoint>& src,
				     vector<dpoint>& dest,
				     dpoint& scale, dpoint& shift) const {
    const parameters& par = getParameters();
    isotropicNormHelper<double>  help(par.averageDistance);
    return help.apply(src,dest,scale,shift);
  };
}
