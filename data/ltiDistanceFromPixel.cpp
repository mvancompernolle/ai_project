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
 * file .......: ltiDistanceFromPixel.cpp
 * authors ....: Peter Hosten, Florian Bley
 * organization: LTI, RWTH Aachen
 * creation ...: 30.9.2004
 * revisions ..: $Id: ltiDistanceFromPixel.cpp,v 1.5 2006/09/05 10:39:22 ltilib Exp $
 */

#include "ltiDistanceFromPixel.h"


namespace lti {
  // --------------------------------------------------
  // DistanceFromPixel::parameters
  // --------------------------------------------------

  // default constructor
  DistanceFromPixel::parameters::parameters()
    : functor::parameters() {
   fHeight=1.0f;     // Height of camera over the ground in cm
   fD=1.0f;          // Focal length in cm
   fKx=1.0f;         // Camera const in  x-direction cm/Pixel
   fKy=1.0f;         // Camera const in  y-direction cm/Pixel
   iPictureSizeX=640;// Size of the picture in x Pixel     
   iPictureSizeY=480;// Size of the picture in y Pixel     
      
      
  }

  // copy constructor
  DistanceFromPixel::parameters::parameters(const parameters& other)
    : functor::parameters() {
    copy(other);
    
  }

  // destructor
  DistanceFromPixel::parameters::~parameters() {
  }

  // get type name
  const char* DistanceFromPixel::parameters::getTypeName() const {
    return "DistanceFromPixel::parameters";
  }

  // copy member

  DistanceFromPixel::parameters&
    DistanceFromPixel::parameters::copy(const parameters& other) {
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

    fHeight=other.fHeight;
    fD=other.fD;    
    fKx=other.fKx;
    fKy=other.fKy;    
    iPictureSizeX=other.iPictureSizeX;
    iPictureSizeY=other.iPictureSizeY;
    

    return *this;
  }

  // alias for copy member
  DistanceFromPixel::parameters&
    DistanceFromPixel::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* DistanceFromPixel::parameters::clone() const {
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
  bool DistanceFromPixel::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool DistanceFromPixel::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
        lti::write(handler,"fHeight",fHeight);
        lti::write(handler,"fD",fD);
        lti::write(handler,"fKx",fKx);
        lti::write(handler,"fKy",fKy);
        lti::write(handler,"iPictureSizeX",iPictureSizeX);
        lti::write(handler,"iPictureSizeY",iPictureSizeY);
         
  
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
  bool DistanceFromPixel::parameters::write(ioHandler& handler,
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
  bool DistanceFromPixel::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool DistanceFromPixel::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
        lti::read(handler,"fHeight",fHeight);
        lti::read(handler,"fD",fD);
        lti::read(handler,"fKx",fKx);
        lti::read(handler,"fKy",fKy);
        lti::read(handler,"iPictureSizeX",iPictureSizeX);
        lti::read(handler,"iPictureSizeY",iPictureSizeY);
      
    
    
    
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
  bool DistanceFromPixel::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


     
  // --------------------------------------------------
  // DistanceFromPixel
  // --------------------------------------------------

  // default constructor
  DistanceFromPixel::DistanceFromPixel()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    defaultParameters.fD=1.0f;
    defaultParameters.fKx=1.0f;
    defaultParameters.fKy=1.0f;
    defaultParameters.fHeight=1.0f;
    defaultParameters.iPictureSizeX=640;  // x
    defaultParameters.iPictureSizeY=480;  // y
     
    
    setParameters(defaultParameters);
    
  }

  // default constructor
  DistanceFromPixel::DistanceFromPixel(const parameters& par)
    : functor() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  DistanceFromPixel::DistanceFromPixel(const DistanceFromPixel& other)
    : functor() {
    copy(other);
  }

  // destructor
  DistanceFromPixel::~DistanceFromPixel() {
  }

  // returns the name of this type
  const char* DistanceFromPixel::getTypeName() const {
    return "DistanceFromPixel";
  }

  // copy member
  DistanceFromPixel&
    DistanceFromPixel::copy(const DistanceFromPixel& other) {
      functor::copy(other);

    return (*this);
  }

  // alias for copy member
  DistanceFromPixel&
    DistanceFromPixel::operator=(const DistanceFromPixel& other) {
    return (copy(other));
  }


  // clone member
  functor* DistanceFromPixel::clone() const {
    return new DistanceFromPixel(*this);
  }

  // return parameters
  const DistanceFromPixel::parameters&
    DistanceFromPixel::getParameters() const {
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

  
  // This method calculates the corresponding 3D space point from an 2D image point 
  // The requirements needed for this to work 
  // are described above in the detailed description of this class.  
  // Input:  lti::vector<int> src(2,0)
  // Output: lti::vector<int> dest(3,0)
  bool DistanceFromPixel::apply(const vector<int>& src ,vector<float>& dest) const {
    
    const parameters& par = getParameters();  // copy of data
        
    float fDeltaX=DeltaX(par.iPictureSizeX,src[0],par.fKx);
    float fDeltaY=DeltaY(par.iPictureSizeY,src[1],par.fKy);
    float fZDistance=CalcZDistance(fDeltaY,par.fHeight,par.fD);
    float fXDistance=CalcXDistance(fDeltaX,fZDistance,par.fD);
    
    dest.fill(fXDistance,0,0);
    dest.fill(-par.fHeight,1,1);
    dest.fill(fZDistance,2,2);
    
    return true;
  };

  
  // This method calculates the euclidian distance between the 3D space position 
  // of an image point and the camera. The requirements needed for this to work 
  // are described above in the detailed description of this class.  
  // Input:  lti::vector<int> src(2,0)
  // Output: float dest
  bool DistanceFromPixel::apply(const vector<int>& src,float& dest) const{

    const parameters& par = getParameters();  // copy of data
    
    float fDeltaX=DeltaX(par.iPictureSizeX,src[0],par.fKx);
    float fDeltaY=DeltaY(par.iPictureSizeY,src[1],par.fKy);
    float fZDistance=CalcZDistance(fDeltaY,par.fHeight,par.fD);
    float fXDistance=CalcXDistance(fDeltaX,fZDistance,par.fD);
    
    dest=EuclidDistance(fXDistance,par.fHeight,fZDistance);
  
    return true;
  };

  
    
  
  float DistanceFromPixel::DeltaY(const int SizeY,const int PointY,const float Ky) const{
    return ((SizeY/2)-PointY)*Ky;        //in pixel->cm
  }
  
  float DistanceFromPixel::DeltaX(const int SizeX,const int PointX,const float Kx) const{
    return (PointX-(SizeX/2))*Kx;        //in pixel->cm
  }
  
  
  float DistanceFromPixel::CalcZDistance(const float relPointY,const float Height, const float D) const{
    return ((-Height*D)/relPointY)-(D/relPointY);
  }
   
  float DistanceFromPixel::CalcXDistance(const float relPointX,const float ZDistance, const float D) const{
    return (((relPointX*ZDistance)/D)+relPointX);
  }
  
  float DistanceFromPixel::EuclidDistance(const float XDistance,const float Height,const float ZDistance) const {
    return sqrt((XDistance*XDistance)+(Height*Height)+(ZDistance*ZDistance));
  
  }

  

}
