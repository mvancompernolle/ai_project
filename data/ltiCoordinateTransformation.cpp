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
 * file .......: ltiCoordinateTransformation.cpp
 * authors ....: Christoph Meyer, Florian Bley
 * organization: LTI, RWTH Aachen
 * creation ...: 1.10.2004
 * revisions ..: $Id: ltiCoordinateTransformation.cpp,v 1.8 2006/09/05 10:38:59 ltilib Exp $
 */

#include "ltiCoordinateTransformation.h"


namespace lti {
  // ------------------------------------------------------------------------------------------
  // coordinateTransformation::parameters
  // ------------------------------------------------------------------------------------------

  // default constructor
  coordinateTransformation::parameters::parameters()
    : linearAlgebraFunctor::parameters() {
      // initialize the rotation-matrices as 3x3 matrices with all elements=0.
      mRotateAroundXAxis.resize(3,3,0);     // A
      mRotateAroundYAxis.resize(3,3,0);     // B
      mRotateAroundZAxis.resize(3,3,0);     // C
      mRotateAroundXYZAxis.resize(3,3,0);   // D
      // initialize the displacement-vector as a 3D vector with all elements=0.
      vDisplace.resize(3,0);
  }

  // copy constructor
  coordinateTransformation::parameters::parameters(const parameters& other)
    : linearAlgebraFunctor::parameters() {
    copy(other);
  }

  // destructor
  coordinateTransformation::parameters::~parameters() {
  }


  // ------------------------------------------------ methods of parameter  ---------------
  // get type name
  const char* coordinateTransformation::parameters::getTypeName() const {
    return "coordinateTransformation::parameters";
  }

  /**
   * This function has to be called to initialize the parameters
   * ---------------------------------------------------------------
   * @param fAngleXAxisRotation : the rotation of the x-axis in degree
   * @param fAngleYAxisRotation : the rotation of the y-axis in degree
   * @param fAngleZAxisRotation : the rotation of the z-axis in degree
   * @param vDisplacement       : the vector from the origin (0,0,0) of the 
   *                              inner coordinate system to the origin (0,0,0)
   *                              of the outer coordinate system.
   */
  bool coordinateTransformation::parameters::initParameters(float fAngleXAxisRotation,
                                                              float fAngleYAxisRotation,
                                                              float fAngleZAxisRotation,
                                                              vector<float> vDisplacement){
    vDisplace = vDisplacement;                         // vector between (0,0,0)and(0,0,0)
    defineTransformationMatrices(fAngleXAxisRotation,  // calculates A, B and C
                                 fAngleYAxisRotation,
                                 fAngleZAxisRotation);
    calculateTotalRotationMatrix();                    // calculates D
    return true;
  }

  /**
   * Calculates the three rotation-matrices A, B and C.
   * @param falpha : the rotation of the x-axis in degree
   * @param fbeta : the rotation of the y-axis in degree
   * @param fgamma : the rotation of the z-axis in degree
   */
  bool coordinateTransformation::parameters::defineTransformationMatrices(float falpha,
                                                                            float fbeta,
                                                                            float fgamma){
    //conversion: DEG into RAD
    falpha = falpha / 180.0f*static_cast<float>(Pi);
    fbeta  = fbeta / 180.0f*static_cast<float>(Pi);
    fgamma = fgamma / 180.0f*static_cast<float>(Pi);

    float sina, cosa;

    //  Matrix for rotation around the X-Axis    A = { 1,           0,           0,
    //                                                 0,  cos(alpha),  sin(alpha),
    //                                                 0, -sin(alpha),  cos(alpha) };
    mRotateAroundXAxis[0][0]= 1;
    sincos(falpha, sina, cosa);
    mRotateAroundXAxis[1][1]= cosa; mRotateAroundXAxis[1][2]= sina;
    mRotateAroundXAxis[2][1]=-sina; mRotateAroundXAxis[2][2]= cosa;

    //  Matrix for rotation around the Y-Axis    B = { cos(beta),  0, -sin(beta),
    //                                                          0,  1,           0,
    //                                                +sin(beta),  0,  cos(beta) };
    sincos(fbeta, sina, cosa);
    mRotateAroundYAxis[0][0]= cosa; mRotateAroundYAxis[0][2]=-sina;
    mRotateAroundYAxis[1][1]= 1;
    mRotateAroundYAxis[2][0]= sina; mRotateAroundYAxis[2][2]= cosa;

    //  Matrix for rotation around the Z-Axis    C = {  cos(gamma), sin(gamma,  0,
    //                                                 -sin(gamma), cos(gamma), 0,
    //                                                           0,          0, 1 };
    sincos(fgamma, sina, cosa);
    mRotateAroundZAxis[0][0]= cosa; mRotateAroundZAxis[0][1]= sina;
    mRotateAroundZAxis[1][0]=-sina; mRotateAroundZAxis[1][1]= cosa;
    mRotateAroundZAxis[2][2]= 1;
    return true;
  }

  /**
   * calculates the total-rotation-matrix by multiplying the three 
   * rotation-matrices: D=C*B*A
   */
  bool coordinateTransformation::parameters::calculateTotalRotationMatrix(void) {
    mRotateAroundXYZAxis.multiply(mRotateAroundZAxis,mRotateAroundYAxis);// D=C*B
    mRotateAroundXYZAxis.multiply(mRotateAroundXAxis);                   // D=D*A = C*B*A
    return true;
  }


  // copy member
  coordinateTransformation::parameters&
    coordinateTransformation::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    linearAlgebraFunctor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    linearAlgebraFunctor::parameters& (linearAlgebraFunctor::parameters::* p_copy)
      (const linearAlgebraFunctor::parameters&) =
      linearAlgebraFunctor::parameters::copy;
    (this->*p_copy)(other);
# endif

        mRotateAroundXAxis = other.mRotateAroundXAxis;
        mRotateAroundYAxis = other.mRotateAroundYAxis;
        mRotateAroundZAxis = other.mRotateAroundZAxis;
        mRotateAroundXYZAxis = other.mRotateAroundXYZAxis;
        vDisplace = other.vDisplace;

    return *this;
  }

  // alias for copy member
  coordinateTransformation::parameters&
    coordinateTransformation::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* coordinateTransformation::parameters::clone() const {
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
  bool coordinateTransformation::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool coordinateTransformation::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
    
        lti::write(handler,"mRotateAroundXAxis",mRotateAroundXAxis);
        lti::write(handler,"mRotateAroundYAxis",mRotateAroundYAxis);
        lti::write(handler,"mRotateAroundZAxis",mRotateAroundZAxis);
        lti::write(handler,"mRotateAroundXYZAxis",mRotateAroundXYZAxis);
        lti::write(handler,"vDisplace",vDisplace);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && linearAlgebraFunctor::parameters::write(handler,false);
# else
    bool (linearAlgebraFunctor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      linearAlgebraFunctor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool coordinateTransformation::parameters::write(ioHandler& handler,
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
  bool coordinateTransformation::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool coordinateTransformation::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

        lti::read(handler,"mRotateAroundXAxis",mRotateAroundXAxis);
        lti::read(handler,"mRotateAroundYAxis",mRotateAroundYAxis);
        lti::read(handler,"mRotateAroundZAxis",mRotateAroundZAxis);
        lti::read(handler,"mRotateAroundXYZAxis",mRotateAroundXYZAxis);
        lti::read(handler,"vDisplace",vDisplace);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && linearAlgebraFunctor::parameters::read(handler,false);
# else
    bool (linearAlgebraFunctor::parameters::* p_readMS)(ioHandler&,const bool) =
      linearAlgebraFunctor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool coordinateTransformation::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------------------------------------------------
  // coordinateTransformation
  // -------------------------------------------------------------------------------------------

  // default constructor
  coordinateTransformation::coordinateTransformation()
    : linearAlgebraFunctor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    // initialize the rotation-matrices as 3x3 matrices with all elementes=0.
    defaultParameters.mRotateAroundXAxis.resize(3,3,0);     // A
    defaultParameters.mRotateAroundYAxis.resize(3,3,0);     // B
    defaultParameters.mRotateAroundZAxis.resize(3,3,0);     // C
    defaultParameters.mRotateAroundXYZAxis.resize(3,3,0);   // D
    // initialize the displacement-vector as a 3D vector with all elements=0.
    defaultParameters.vDisplace.resize(3,0);
    setParameters(defaultParameters);

  }

  // default constructor
  coordinateTransformation::coordinateTransformation(const parameters& par)
    : linearAlgebraFunctor() {

    // set the given parameters
    setParameters(par);
  }

  // copy constructor
  coordinateTransformation::coordinateTransformation(const coordinateTransformation& other)
    : linearAlgebraFunctor() {
    copy(other);
  }

  // destructor
  coordinateTransformation::~coordinateTransformation() {
  }

  // returns the name of this type
  const char* coordinateTransformation::getTypeName() const {
    return "coordinateTransformation";
  }

  // copy member
  coordinateTransformation&
    coordinateTransformation::copy(const coordinateTransformation& other) {
      linearAlgebraFunctor::copy(other);

    return (*this);
  }

  // alias for copy member
  coordinateTransformation&
    coordinateTransformation::operator=(const coordinateTransformation& other) {
    return (copy(other));
  }

  // clone member
  functor* coordinateTransformation::clone() const {
    return new coordinateTransformation(*this);
  }

  // return parameters
  const coordinateTransformation::parameters&
    coordinateTransformation::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  
  // -------------------------------------------------------------------
  // The apply-methods
  // -------------------------------------------------------------------
  /**
   * Transforms a given point ( as vector \e srcdest ) in the inner 
   * coordinate system (e.g. camera) to a point (also \e srcdest) in the outer
   * coordinate system (e.g. robot).
   */
  bool coordinateTransformation::apply(vector<float>& srcdest) const {
    const parameters& par = getParameters();  // make copy of parameters
    
    // multiplies the vector srcdest with the whole TransformationMatrix,
    // and leaves the result in srcdest.
    par.mRotateAroundXYZAxis.multiply(srcdest);
    // adds the displacement (translation) which exists between the two
    // coordinate systems.
    srcdest.add((par.vDisplace));
    return true;
  };

  /**
   * Transforms a given point ( as vector \e src ) in the inner 
   * coordinate system (e.g. camera) to a point (also \e dest) in the outer
   * coordinate system (e.g. robot).
   * @param src vector with the source data.
   * @param dest vector where the result will be left.
   */
  bool coordinateTransformation::apply(const vector<float>& src,
                                               vector<float>& dest) const {
    dest=src;
    apply(dest);
    return false;
  };

}
