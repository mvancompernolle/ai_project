/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiGeometricTransform.cpp
 * authors ....: Ruediger Weiler
 * organization: LTI, RWTH Aachen
 * creation ...: 5.2.2001
 * revisions ..: $Id: ltiGeometricTransform.cpp,v 1.17 2006/09/05 10:14:07 ltilib Exp $
 */

#include "ltiMacroSymbols.h"
#include "ltiGeometricTransform.h"
#include "ltiMath.h"

namespace lti {
  // --------------------------------------------------
  // geometricTransform::parameters
  // --------------------------------------------------

  // default constructor
  geometricTransform::parameters::parameters()
    : modifier::parameters() {
    
    transMatrix.unit();
    keepDimensions = true;
    boundaryType = lti::Zero;
    interpolator = BilinearInterpolator;
  }

  // copy constructor
  geometricTransform::parameters::parameters(const parameters& other)
    : modifier::parameters()  {
    copy(other);
  }

  // destructor
  geometricTransform::parameters::~parameters() {
  }

  // get type name
  const char* geometricTransform::parameters::getTypeName() const {
    return "geometricTransform::parameters";
  }

  // copy member

  geometricTransform::parameters&
  geometricTransform::parameters::copy(const parameters& other) {
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

    transMatrix.copy(other.transMatrix);
    keepDimensions = other.keepDimensions;
    interpolator = other.interpolator;

    return *this;
  }

  // alias for copy member
  geometricTransform::parameters&
    geometricTransform::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* geometricTransform::parameters::clone() const {
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
  bool geometricTransform::parameters::write(ioHandler& handler,
                                             const bool complete) const
# else
  bool geometricTransform::parameters::writeMS(ioHandler& handler,
                                               const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"transMatrix",transMatrix);
      lti::write(handler,"keepDimensions",keepDimensions);
      lti::write(handler,"interpolator",interpolator);
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
  bool geometricTransform::parameters::write(ioHandler& handler,
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
  bool geometricTransform::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool geometricTransform::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"transMatrix",transMatrix);
      lti::read(handler,"keepDimensions",keepDimensions);
      lti::read(handler,"interpolator",interpolator);
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
  bool geometricTransform::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  void
  geometricTransform::parameters::shift(const tpoint<double>& thefPoint) {
    transMatrix.at(0,3) += static_cast<float>(thefPoint.x);
    transMatrix.at(1,3) += static_cast<float>(thefPoint.y);
  }

  void
  geometricTransform::parameters::shift(const tpoint3D<double>& thefPoint) {
    transMatrix.at(0,3) += static_cast<float>(thefPoint.x);
    transMatrix.at(1,3) += static_cast<float>(thefPoint.y);
    transMatrix.at(2,3) += static_cast<float>(thefPoint.z);
  }

  void
  geometricTransform::parameters::scale(const tpoint<double>& thefPoint) {
    hMatrix3D<float> scaleMatrix;
    scaleMatrix.at(0,0) = static_cast<float>(thefPoint.x);
    scaleMatrix.at(1,1) = static_cast<float>(thefPoint.y);
    scaleMatrix.at(2,2) = 1.0f;
    scaleMatrix.at(3,3) = 1.0f;

    scaleMatrix*=transMatrix;
    transMatrix.copy(scaleMatrix);
  }

  void
  geometricTransform::parameters::scale(const tpoint3D<double>& thefPoint) {
    hMatrix3D<float> scaleMatrix;
    scaleMatrix.at(0,0) = static_cast<float>(thefPoint.x);
    scaleMatrix.at(1,1) = static_cast<float>(thefPoint.y);
    scaleMatrix.at(2,2) = static_cast<float>(thefPoint.z);
    scaleMatrix.at(3,3) = 1.0f;

    scaleMatrix*=transMatrix;
    transMatrix.copy(scaleMatrix);
  }

  void geometricTransform::parameters::scale(const double& x) {
    scale(tpoint3D<double>(x,x,x));
  }

  void geometricTransform::parameters::invert() {
    transMatrix.invert();
  }

  void geometricTransform::parameters::rotate(const double& angle) {
    hPoint3D<float> center(0,0,0),axis(0,0,1.0);
    const float a = static_cast<float>(angle);
    transMatrix.rotate(a,axis,center);
  }

  void geometricTransform::parameters::rotate(const tpoint3D<double>& centre,
                                              const tpoint3D<double>& axis,
                                              const double& angle) {
    hPoint3D<float> cent,ax;
    cent.castFrom(centre);
    ax.castFrom(axis);
    const float a = static_cast<float>(angle);
    transMatrix.rotate(a,ax,cent);
  }

  void geometricTransform::parameters::rotate(const vector<double>& centre,
                                              const vector<double>& axis,
                                              const double& angle) {
    // rotates around an axis in the R3 axis is given by point center
    // and vector axis
    hPoint3D<float> center,ax;
    const float a = static_cast<float>(angle);

    int i;
    int el = min(3,centre.size());
    for (i = 0;i<el;++i) {
      center[i] = static_cast<float>(centre.at(i));
    }

    el = min(3,axis.size());
    for (i = 0;i<el;++i) {
      ax[i] = static_cast<float>(axis.at(i));
    }

    transMatrix.rotate(a,ax,center);
  }

  void geometricTransform::parameters::clear(void){
    //erases and inits the elements
    transMatrix.unit();
  }

  // --------------------------------------------------
  // geometricTransform
  // --------------------------------------------------

  // default constructor
  geometricTransform::geometricTransform()
    : modifier(){
    parameters param;
    setParameters(param);
  }

  // default constructor
  geometricTransform::geometricTransform(const parameters& par)
    : modifier(){
    setParameters(par);
  }

  // copy constructor
  geometricTransform::geometricTransform(const geometricTransform& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  geometricTransform::~geometricTransform() {
  }

  // returns the name of this type
  const char* geometricTransform::getTypeName() const {
    return "geometricTransform";
  }

  // copy member
  geometricTransform&
  geometricTransform::copy(const geometricTransform& other) {
      modifier::copy(other);
    return (*this);
  }

  // clone member
  functor* geometricTransform::clone() const {
    return new geometricTransform(*this);
  }

  // return parameters
  const geometricTransform::parameters&
  geometricTransform::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  geometricTransform::fastMatrix::fastMatrix(const hMatrix3D<float>& m) 
    : m00(m.at(0,0)), m01(m.at(0,1)), m02(m.at(0,2)), m03(m.at(0,3)),
      m10(m.at(1,0)), m11(m.at(1,1)), m12(m.at(1,2)), m13(m.at(1,3)),
      m30(m.at(3,0)), m31(m.at(3,1)), m32(m.at(3,2)), m33(m.at(3,3)) {
  }

  geometricTransform::fastMatrix::fastMatrix(const hMatrix2D<float>& m) 
    : m00(m.at(0,0)), m01(m.at(0,1)), m02(0.0f), m03(m.at(0,2)),
      m10(m.at(1,0)), m11(m.at(1,1)), m12(0.0f), m13(m.at(1,2)),
      m30(m.at(2,0)), m31(m.at(2,1)), m32(0.0f), m33(m.at(2,2)) {
  }

  bool geometricTransform::updateParameters() {
    const parameters& param = getParameters();
    
    // set the collection parameters
    return (dcollect.setParameters(param) &&
            fcollect.setParameters(param) &&
            icollect.setParameters(param) &&
            bcollect.setParameters(param) &&
            ccollect.setParameters(param));
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------





}
