/*
 * Copyright (C) 2003, 2004, 2005, 2006
 * Vlad Popovici, EPFL STI-ITS, Switzerland
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
 * file .......: ltiRotation.cpp
 * authors ....: Vlad Popovici
 * organization: EPFL STI-ITS/LTS1
 * creation ...: 18.6.2003
 * revisions ..: $Id: ltiRotation.cpp,v 1.12 2006/09/05 10:29:44 ltilib Exp $
 */

#include "ltiRotation.h"
#include "ltiMath.h"
#include <limits>

#undef _LTI_DEBUG
// #define _LTI_DEBUG
#include "ltiDebug.h"

namespace lti {

  // ------------------------------------------------------------------------
  // rotation helper
  // ------------------------------------------------------------------------
  //
  // This is a very private helper class, which does the real work.  It is
  // not done as internal class of rotation, because of Visual C++.
  //

  /**
   * Rotation helper class
   */
  template<class T>
  class rotationHelper {
  public:
    /**
     * Construct a helper
     */
    rotationHelper();


    /**
     * Skews horizontally the matrix<T> src and save the result in dst.
     * The argument weight is an interpolation factor between 0 and 1
     */ 
    bool horizontalSkew(const matrix<T>& src, 
		        matrix<T>& dst, 
			const int rowIdx,
			const int colOffset,
			const double weight) const;

    /**
     * Skews vertically the matrix<T> src and save the result in dst.
     * The argument weight is an interpolation factor between 0 and 1
     */
    bool verticalSkew(const matrix<T>& src,
		      matrix<T>& dst,
		      const int colIdx,
		      const int rowOffset,
		      const double weight) const;

    /**
     * rotate the matrix<T>
     */
    bool rotate(const matrix<T>& src, matrix<T>& dst,
                const double& angle) const; 

    /**
     * rotate the matrix 90 degrees
     */
    bool rotate90(const matrix<T>& src, matrix<T>& dst) const;

    /**
     * rotate the matrix 180 degrees
     */
    bool rotate180(const matrix<T>& src, matrix<T>& dst) const;

    /**
     * rotate the matrix 270 degrees
     */
    bool rotate270(const matrix<T>& src, matrix<T>& dst) const;

    /**
     *  do the job and rotate the image!
     */
    bool apply(const matrix<T>& src,matrix<T>& dest,const double& angle) const;

  };

  // Helper functions.

  template <class T>
  rotationHelper<T>::rotationHelper() {
  }

  // Skews horizontally the matrix<T> src and save the result in dst.
  template <class T>
  bool rotationHelper<T>::horizontalSkew(const matrix<T>& src, 
                                               matrix<T>& dst, 
                                         const int rowIdx,
                                         const int colOffset, 
                                         const double weight) const {

    // ASSUMTION: dst has the correct size, which is always >= src.size()

    T p0,p1;

    if (colOffset>=0) {
      // col offset is positive: black + data + black

      T oldpx = T(0);

      // some pointer arithmetic to accelerate the shift
      T* dptr = &dst.at(rowIdx,0);
      T* eptr = dptr+colOffset;

      // clear the beginning
      while (dptr<eptr) {
        *dptr++=T(0);
      }    
      
      // copy the shifted elements
      const T* sptr = &src.at(rowIdx,0);

      if (colOffset+src.columns() < dst.columns()) {
        eptr = (&dst.at(rowIdx,0)) + colOffset + src.columns();
      } else {
        eptr = (&dst.at(rowIdx,0)) + dst.columns();
      }
    
      while (dptr<eptr) {        
        p0      = (*sptr);
        p1      = static_cast<T>(p0 * weight);
        p0     -= (p1 - oldpx);
        (*dptr) = p0;
        oldpx   = p1;

        ++dptr;
        ++sptr;
      }

      eptr = (&dst.at(rowIdx,0)) + dst.columns();

      if (dptr<eptr) {
        *dptr++=oldpx;
      }

      while (dptr < eptr) {
        *dptr++=T(0);
      }

    } else {
      // colOffset<0
      T oldpx = static_cast<T>(src.at(rowIdx,-1-colOffset) * weight);

      // some pointer arithmetic to accelerate the shift
      T* dptr = &dst.at(rowIdx,0);
      const T* sptr = (&src.at(rowIdx,0))-colOffset; // colOffset is negative!
      const T *const eptr = (&src.at(rowIdx,0))+src.columns();
      
      while (sptr<eptr) {
        p0      = (*sptr);
        p1      = static_cast<T>(p0 * weight);
        p0     -= (p1 - oldpx);
        (*dptr) = p0;
        oldpx   = p1;

        ++dptr;
        ++sptr;
      }

      // try to get the last interpolation with zero
      T *const edptr = (&dst.at(rowIdx,0)) + dst.columns();
      if (dptr<edptr) {
        *dptr++=oldpx;
      }

      // fill the rest with zero
      while (dptr < edptr) {
        *dptr++=T(0);
      }

    }

    return true;
  }


  // Skews vertically the matrix<T> src and save the result in dst.
  template <class T>
  bool rotationHelper<T>::verticalSkew(const matrix<T>& src,
                                             matrix<T>& dst,
                                       const int colIdx, 
                                       const int rowOffset,
                                       const double weight) const {


    // ASSUMTION: dst has the correct size
    T p0,p1;
    int y,dy;

    if (rowOffset>=0) {
      T oldpx = T(0);

      // clear the beginning
      for (y=0;y<rowOffset;++y) {
        dst.at(y,colIdx)=T(0);
      }
     
      // copy the shifted elements
      int end;
      if (rowOffset+src.rows() < dst.rows()) {
        end = rowOffset+src.rows();
      }
      else {
        end = dst.rows();
      }
      
      for (dy=0;y<end;++y,++dy) {
        p0               = src.at(dy,colIdx);
        p1               = static_cast<T>(p0 * weight);
        p0              -= (p1 - oldpx);
        dst.at(y,colIdx) = p0;
        oldpx            = p1;
      }

      if (y < dst.rows()) {
        dst.at(y,colIdx) = oldpx;
        ++y;
      }
     
      for (;y<dst.rows();++y) {
        dst.at(y,colIdx)=T(0);
      }

    } else {
      // rowOffset<0
      T oldpx = static_cast<T>(src.at(-1-rowOffset,colIdx) * weight);

      for (y=-rowOffset,dy=0;y<src.rows();++y,++dy) {
        p0                = src.at(y,colIdx);
        p1                = static_cast<T>(p0 * weight);
        p0               -= (p1 - oldpx);
        dst.at(dy,colIdx) = p0;
        oldpx             = p1;
      }

      if (dy < dst.rows()) {
        dst.at(dy,colIdx) = oldpx;
        ++dy;
      }

      // fill the rest with zero
      for (;dy<dst.rows();++dy) {
        dst.at(dy,colIdx) = T(0);
      }

    }

    return true;
  }

  // Rotates by an angle between -pi/2 and pi/2 using the 3-shear technique.
  template <class T>
  bool rotationHelper<T>::rotate(const matrix<T>& src,
                                       matrix<T>& dst,
                                 const double& angle) const {
    if (angle == 0.0) {
      dst.copy(src);
      return true;
    }

    const double sinA = sin(angle);
    const double cosA = cos(angle);
    const double tanA = tan(angle/2.0);
    double factor = 0.0;

    const int nRows1 = src.rows();
    const int nCols1 = src.columns() + iround(src.rows() * abs(tanA));
 
    matrix<T> tmpImg1(false,nRows1, nCols1);

    // Shear 1: horizontal
    int x,y;
    if (tanA < 0.0) { // clockwise rotation
      for (y = 0; y < nRows1; y++) {
	factor = tanA * (y - nRows1 + 0.5);
        horizontalSkew(src, tmpImg1, y, static_cast<int>(floor(factor)),
                       factor - floor(factor));
      }
    }      
    else { // counter-clockwise rotation
      for (y = 0; y < nRows1; y++) {
        factor = tanA * (y + 0.5);
        horizontalSkew(src, tmpImg1, y, static_cast<int>(floor(factor)),
                       factor - floor(factor));
      }
    }

    // Shear 2: vertical
    const int nCols2 = nCols1;
    const int nRows2 = static_cast<int>(abs(sinA) * src.columns() + 
                                        cosA* src.rows()) + 1;

    matrix<T> tmpImg2(false,nRows2,nCols2);

    if (sinA < 0.0) { // clockwise
      factor = -sinA * (src.columns() - nCols2);
    }
    else {  // counter-clockwise
      factor = sinA * (src.columns() - 1.0);
    }
    for (x = 0; x < nCols2; ++x) {
      verticalSkew(tmpImg1, tmpImg2, x, static_cast<int>(floor(factor)),
		   factor - floor(factor));
      factor -= sinA;
    }

    // free some memory:
    tmpImg1.clear();

    // prepare destination
    const int nRows = nRows2;
    const int nCols = static_cast<int>(abs(sinA) * src.rows() + 
                                       cosA * src.columns()) + 1;

    dst.resize(nRows, nCols, T(0),false,false);

    // Shear 3: horizontal
    if (sinA < 0.0) { // clockwise
      factor = tanA * (-sinA * (src.columns() - 1) + 1.0 - nRows);
    }
    else {   // counter-clockwise
      factor = -tanA * sinA * (src.columns() - 1);
    }
    for (y = 0; y < nRows; ++y) {
      horizontalSkew(tmpImg2, dst, y, static_cast<int>(floor(factor)),
		     factor - floor(factor));
      factor += tanA;
    }

    return true;
  }

  // Rotates by 90 degrees.
  template <class T>
  bool rotationHelper<T>::rotate90(const matrix<T>& src, 
                                         matrix<T>& dst) const {
    dst.resize(src.columns(),src.rows(),T(0),false,false);

    const int rows = src.rows();
    const int columns = src.columns()-1;

    for (int y = 0; y < rows; y++) {
      for (int x = 0; x < columns; x++) {
    	  dst.at(columns-x,y) = src.at(y,x);
      }
    }
    
    return true;
  }


  // Rotates by 180 degrees.
  template <class T>
  bool rotationHelper<T>::rotate180(const matrix<T>& src,
                                          matrix<T>& dst) const {
    dst.resize(src.size(),T(0),false,false);

    typename matrix<T>::const_iterator  iterImg     = src.begin();
    typename matrix<T>::const_iterator  iterImgEnd  = src.end();
    typename matrix<T>::iterator        iterDestImg = dst.end();
    iterDestImg--;

    while(iterImg!=iterImgEnd)
    {
      *iterDestImg = *iterImg;
      iterImg++;
      iterDestImg--;
    }

    return true;
  }


  // Rotates by 270 degrees.
  template <class T>
  bool rotationHelper<T>::rotate270(const matrix<T>& src,
                                          matrix<T>& dst) const {
    dst.resize(src.columns(),src.rows(),T(0),false,false);

    const int rows = src.rows()-1;
    const int columns = src.columns();

    for (int y = 0; y <= rows; y++) {
      for (int x = 0; x < columns; x++) {
	      dst.at(x,rows-y) = src.at(y,x);
      }
    }

    return true;
  }

  // On copy apply for type matrix<T>!
  template <class T>
  bool rotationHelper<T>::apply(const matrix<T>& src,matrix<T>& dest,
                             const double& angle) const {
    matrix<T> tmpImg;

    // make sure angle is in [0.0 , 2*Pi)
    double a = abs(angle);
    const double s = signum(angle);
    const double k = a / (2.0*Pi);

    if (abs(a) <= std::numeric_limits<double>::epsilon()) { // nothing to do!
      dest.copy(src);
      return true;
    }

    a = s * (a - 2.0*Pi*floor(k));
    if (a < 0.0) {
      a += 2.0*Pi;
    }

    if (a > (Pi*0.25) && a <= (Pi*0.75)) {
      _lti_debug("Call rotate90 a="<<a<<"\n");
      rotate90(src, tmpImg);            // start with a 90 degree rotation
      a -= 0.5*Pi;   // it's between -45 and 45
    }
    else if (a > (Pi*0.75) && a <= (Pi*1.25)) {
      rotate180(src, tmpImg);        // start with a 180 degree rotation
      a -= Pi;                       // it's between -45 and 45
    }
    else if (a > (Pi*1.25) && a <= (Pi*1.75)) {
      rotate270(src, tmpImg);        // start with a 270 degree rotation
      a -= 1.5*Pi;                   // it's between -45 and 45
    }
    else {
      tmpImg.copy(src);
    }
    
    if (abs(a) > std::numeric_limits<double>::epsilon()) { // final rotation:
      rotate(tmpImg, dest, a);
    }
    else {
      // only one of the special rotate funcs or copy has been called
      tmpImg.detach(dest);
    }

    return true;
  }


  // --------------------------------------------------
  // rotation::parameters
  // --------------------------------------------------

  // default constructor
  rotation::parameters::parameters()
    : modifier::parameters() {
    angle = 0.0;
  }

  // copy constructor
  rotation::parameters::parameters(const parameters& other)
    : modifier::parameters() {
    copy(other);
  }

  // destructor
  rotation::parameters::~parameters() {
  }

  // get type name
  const char* rotation::parameters::getTypeName() const {
    return "rotation::parameters";
  }

  // copy member

  rotation::parameters&
    rotation::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    modifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    modifier::parameters& (modifier::parameters::* p_copy)
      (const modifier::parameters&) =
      modifier::parameters::copy;
    (this->*p_copy)(other);
# endif
    
    angle = other.angle;

    return *this;
  }

  // alias for copy member
  rotation::parameters&
    rotation::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* rotation::parameters::clone() const {
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
  bool rotation::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool rotation::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"angle",angle);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::write(handler,false);
# else
    bool (modifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      modifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool rotation::parameters::write(ioHandler& handler,
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
  bool rotation::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool rotation::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"angle",angle);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::read(handler,false);
# else
    bool (modifier::parameters::* p_readMS)(ioHandler&,const bool) =
      modifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool rotation::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // rotation
  // --------------------------------------------------

  // default constructor
  rotation::rotation()
    : modifier(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  rotation::rotation(const parameters& par)
    : modifier() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  rotation::rotation(const rotation& other)
    : modifier() {
    copy(other);
  }

  // destructor
  rotation::~rotation() {
  }

  // returns the name of this type
  const char* rotation::getTypeName() const {
    return "rotation";
  }

  // copy member
  rotation& rotation::copy(const rotation& other) {
    modifier::copy(other);

    return (*this);
  }

  // alias for copy member
  rotation& rotation::operator=(const rotation& other) {
    return (copy(other));
  }


  // clone member
  functor* rotation::clone() const {
    return new rotation(*this);
  }

  // return parameters
  const rotation::parameters& rotation::getParameters() const {
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

  
  // On place apply for type image!
  bool rotation::apply(image& srcdest) const {
    image tmpImg;
    if (apply(srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }


  // On copy apply for type image!
  bool rotation::apply(const image& src,image& dest) const {
    
    
    image tmpImg;
    rotationHelper<rgbPixel> helper;
    return helper.apply(src,dest,-getParameters().angle);

  }

  // On place apply for type matrix<ubyte>!
  bool rotation::apply(matrix<ubyte>& srcdest) const {
    matrix<ubyte> tmpImg;
    if (apply(srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }


  // On copy apply for type matrix<ubyte>!
  bool rotation::apply(const matrix<ubyte>& src,matrix<ubyte>& dest) const {
    
    
    matrix<ubyte> tmpImg;
    rotationHelper<ubyte> helper;
    return helper.apply(src,dest,-getParameters().angle);

  }

  // On place apply for type image!
  bool rotation::apply(matrix<float>& srcdest) const {
    matrix<float> tmpImg;
    if (apply(srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }


  // On copy apply for type matrix<float>!
  bool rotation::apply(const matrix<float>& src,matrix<float>& dest) const {
    
    
    matrix<float> tmpImg;
    rotationHelper<float> helper;
    return helper.apply(src,dest,-getParameters().angle);

  }

  // --------------------
  // shortcuts with angle
  // --------------------

  // On place apply for type image!
  bool rotation::apply(const double& angle, image& srcdest) const {
    image tmpImg;
    if (apply(angle,srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }


  // On copy apply for type image!
  bool rotation::apply(const double& angle,
                       const image& src,image& dest) const {
    
    rotationHelper<rgbPixel> helper;
    return helper.apply(src,dest,-angle);
  }

  // On place apply for type matrix<ubyte>!
  bool rotation::apply(const double& angle,matrix<ubyte>& srcdest) const {
    matrix<ubyte> tmpImg;
    if (apply(angle,srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }


  // On copy apply for type matrix<ubyte>!
  bool rotation::apply(const double& angle,
                       const matrix<ubyte>& src,matrix<ubyte>& dest) const {
    
    
    matrix<ubyte> tmpImg;
    rotationHelper<ubyte> helper;
    return helper.apply(src,dest,-angle);

  }

  // On place apply for type image!
  bool rotation::apply(const double& angle,
                       matrix<float>& srcdest) const {
    matrix<float> tmpImg;
    if (apply(angle,srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }


  // On copy apply for type matrix<float>!
  bool rotation::apply(const double& angle,
                       const matrix<float>& src,matrix<float>& dest) const {
    
    
    matrix<float> tmpImg;
    rotationHelper<float> helper;
    return helper.apply(src,dest,-angle);

  }



}
