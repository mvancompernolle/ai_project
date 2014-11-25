/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiSelective2DConvolution.cpp
 * authors ....: Henning Luepschen
 * organization: LTI, RWTH Aachen
 * creation ...: 10.9.2003
 * revisions ..: $Id: ltiSelective2DConvolution.cpp,v 1.9 2008/08/17 22:20:12 alvarado Exp $
 */

#include "ltiSelective2DConvolution.h"

namespace lti {
  // --------------------------------------------------
  // selective2DConvolution::parameters
  // --------------------------------------------------

  // default constructor
  selective2DConvolution::parameters::parameters()
    : filter::parameters(), kernel2D_(0) {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters
    
  }

  // copy constructor
  selective2DConvolution::parameters::parameters(const parameters& other)
    : filter::parameters() {
    kernel2D_ = 0;
    copy(other);
  }

  // destructor
  selective2DConvolution::parameters::~parameters() {
    delete kernel2D_;
  }

  // get type name
  const char* selective2DConvolution::parameters::getTypeName() const {
    return "selective2DConvolution::parameters";
  }

  // copy member

  selective2DConvolution::parameters&
    selective2DConvolution::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    filter::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    filter::parameters& (filter::parameters::* p_copy)
      (const filter::parameters&) =
      filter::parameters::copy;
    (this->*p_copy)(other);
# endif

    delete kernel2D_;
    kernel2D_ = 0;
    
    if (notNull(other.kernel2D_)) {
      kernel2D_=dynamic_cast<lti::kernel2D<float>*>(other.kernel2D_->clone());
    }

    return *this;
  }

  // alias for copy member
  selective2DConvolution::parameters&
    selective2DConvolution::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* selective2DConvolution::parameters::clone() const {
    return new parameters(*this);
  }

  const lti::kernel2D<float>& 
  selective2DConvolution::parameters::getKernel2D() const {
    if (isNull(kernel2D_)) {
      throw invalidParametersException(getTypeName());
    }
    return *kernel2D_;
  }

  void selective2DConvolution::parameters::
  setKernel2D(const lti::kernel2D<float>& aKernel) {
    delete kernel2D_;
    kernel2D_ = 0;
    kernel2D_ = dynamic_cast<lti::kernel2D<float>*>(aKernel.clone());
  }


  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool selective2DConvolution::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool selective2DConvolution::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      if (isNull(kernel2D_)) {
        lti::write(handler,"kernel","none");
      } else {
        lti::write(handler,"kernel",getKernel2D());
      }
      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && filter::parameters::write(handler,false);
# else
    bool (filter::parameters::* p_writeMS)(ioHandler&,const bool) const =
      filter::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool selective2DConvolution::parameters::write(ioHandler& handler,
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
  bool selective2DConvolution::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool selective2DConvolution::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::kernel2D<float> kern;
      lti::read(handler,"kernel",kern);
      setKernel2D(kern);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && filter::parameters::read(handler,false);
# else
    bool (filter::parameters::* p_readMS)(ioHandler&,const bool) =
      filter::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool selective2DConvolution::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // selective2DConvolution
  // --------------------------------------------------

  // default constructor
  selective2DConvolution::selective2DConvolution()
    : filter(){

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  selective2DConvolution::selective2DConvolution(const parameters& par)
    : filter() {

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // set the given parameters
    setParameters(par);
  }

  // constructor to set filter kernel directly
  selective2DConvolution::selective2DConvolution(const kernel2D<float>& aKernel)
    : filter() {
    parameters tmpParam;
    tmpParam.setKernel2D(aKernel);
    setParameters(tmpParam);
  }

  // copy constructor
  selective2DConvolution::selective2DConvolution(const selective2DConvolution& other)
    : filter() {
    copy(other);
  }

  // destructor
  selective2DConvolution::~selective2DConvolution() {
  }

  // returns the name of this type
  const char* selective2DConvolution::getTypeName() const {
    return "selective2DConvolution";
  }

  // copy member
  selective2DConvolution&
  selective2DConvolution::copy(const selective2DConvolution& other) {
    filter::copy(other);

    return (*this);
  }

  // alias for copy member
  selective2DConvolution&
    selective2DConvolution::operator=(const selective2DConvolution& other) {
    return (copy(other));
  }


  // clone member
  functor* selective2DConvolution::clone() const {
    return new selective2DConvolution(*this);
  }

  // return parameters
  const selective2DConvolution::parameters&
    selective2DConvolution::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  void selective2DConvolution::setKernel2D(const kernel2D<float>& aKernel) {
    parameters tmpParam(getParameters());
    tmpParam.setKernel2D(aKernel);
    setParameters(tmpParam);
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

    
  // On copy apply for type channel!
  // TODO: boundaryType is NOT checked yet!
  bool selective2DConvolution::apply(const channel& src, channel& dest,
				     const channel& mask) const {

    const parameters& param = getParameters();
    const kernel2D<float>& myKernel = param.getKernel2D();

    // no boundaryType checked, so be careful with size.
    if( src.size() != mask.size() || src.size()<myKernel.size() ) {
      return false;
    }
    
    int iY, iX;
    const int iSrcLastRow = src.lastRow();
    const int iSrcLastColumn = src.lastColumn();
    const int iKernelLastRow = myKernel.lastRow();
    const int iKernelLastColumn = myKernel.lastColumn();
    const int iKernelFirstRow = myKernel.firstRow();
    const int iKernelFirstColumn = myKernel.firstColumn();
    
    int iOffsetY, iOffsetX;

    dest.resize( src.size(), 0.0f, false, true );

    // Check convolution mask
    for( iY = -iKernelFirstRow; iY<=iSrcLastRow-iKernelLastRow; ++iY ) {
      const vector<float>& vMaskRow = mask.getRow(iY);
      vector<float>& vDestRow = dest.getRow(iY);
      
      for( iX = -iKernelFirstColumn; iX<=iSrcLastColumn-iKernelLastColumn; ++iX ) {

	// Convolute point? Mirror kernel.
	if( vMaskRow.at(iX)!=0.0f ) {
	  float fConvResult = 0.0f;
	  for( iOffsetY=iKernelLastRow; iOffsetY>=iKernelFirstRow; --iOffsetY ) {
	    const vector<float>& vSrcRow = src.getRow(iY-iOffsetY);
	    
	    for( iOffsetX=iKernelLastColumn; iOffsetX>=iKernelFirstColumn; --iOffsetX ) {
	      fConvResult += myKernel.at(iOffsetY, iOffsetX)*vSrcRow.at(iX-iOffsetX);
	    }
	  }
	  vDestRow.at(iX) = fConvResult;
	}
      }
    }

    return true;
  };
  
  // On copy apply for type channel!
  // TODO: boundaryType is NOT checked yet!
  bool selective2DConvolution::apply(const channel& src, float& result,
				     const int row, const int column) const {
    
    const parameters& param = getParameters();
    const kernel2D<float>& myKernel = param.getKernel2D();
    const int iKernelFirstRow = myKernel.firstRow();
    const int iKernelFirstColumn = myKernel.firstColumn();

    // We don't check the boundayType yet. So, don't allow border points.
    if( row<-iKernelFirstRow || row>src.lastRow()-myKernel.lastRow() || 
	column<-iKernelFirstColumn || column>src.lastColumn()-myKernel.lastColumn() ) {
      return false;
    }
    
    int iOffsetY, iOffsetX;

    // Convolution starts here (convolution mask is mirrored).
    result = 0.0f;
    for( iOffsetY=myKernel.lastRow(); iOffsetY>=iKernelFirstRow; --iOffsetY ) {
      const vector<float>& vSrcRow = src.getRow(row-iOffsetY);
	    
      for( iOffsetX= myKernel.lastColumn(); iOffsetX>=iKernelFirstColumn; --iOffsetX ) {
	result += myKernel.at(iOffsetY, iOffsetX)*vSrcRow.at(column-iOffsetX);
      }
    }

    return true;
  };

  // On copy apply for type channel!
  bool selective2DConvolution::apply(const channel& src, channel& dest,
				     const int row, const int column) const {

    float fResult = 0.0f;

    dest.resize( src.size(), 0.0f, true, false );
    
    if( !apply(src,fResult,row,column) ) {
      return false;
    }

    dest.at(row, column) = fResult;

    return true;
  };
  
  // On copy apply for type channel!
  bool selective2DConvolution::apply(const channel& src, float& result,
				     const point& thePoint) const {
    return apply(src, result, thePoint.y, thePoint.x);
  };

  // On copy apply for type channel!
  bool selective2DConvolution::apply(const channel& src, channel& dest,
				     const point& thePoint) const {
    return apply(src, dest, thePoint.y, thePoint.x);
  };

  
  
  // On copy apply for type channel!
  bool selective2DConvolution::apply(const channel& src, channel& dest,
				     const pointList& thePointList) const {
    pointList::const_iterator it;

    bool bRet = false;

    // The return value will be TRUE, if at least one point can be convoluted.
    for( it=thePointList.begin(); it!=thePointList.end(); ++it ) {
      if( apply(src, dest, (*it).y, (*it).x) ) {
	bRet = true;
      }
    }
    
    return bRet;
  };
  
}
