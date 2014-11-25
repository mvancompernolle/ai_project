/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiOpticalFlowLK.cpp
 * authors ....: Bernd Mussmann, Suat Akyol
 * organization: LTI, RWTH Aachen
 * creation ...: 5.9.2000
 * revisions ..: $Id: ltiOpticalFlowLK.cpp,v 1.7 2006/02/08 11:35:06 ltilib Exp $
 */

// include files
#include "ltiOpticalFlowLK.h"

namespace lti {
  // --------------------------------------------------
  // opticalFlowLK::parameters
  // --------------------------------------------------

  // default constructor
  opticalFlowLK::parameters::parameters()
    : transform::parameters() {

     kernelSize = int(3);
     windowEdgeSize = int(7);
  }

  // copy constructor
  opticalFlowLK::parameters::parameters(const parameters& other)
    : transform::parameters()  {
    copy(other);
  }

  // destructor
  opticalFlowLK::parameters::~parameters() {
  }

  // get type name
  const char* opticalFlowLK::parameters::getTypeName() const {
    return "opticalFlowLK::parameters";
  }

  // copy member

  opticalFlowLK::parameters&
  opticalFlowLK::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    transform::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    transform::parameters& (transform::parameters::* p_copy)
      (const transform::parameters&) =
      transform::parameters::copy;
    (this->*p_copy)(other);
# endif

    kernelSize = other.kernelSize;
    windowEdgeSize = other.windowEdgeSize;

    return *this;
  }

  // clone member
  functor::parameters* opticalFlowLK::parameters::clone() const {
    return new parameters(*this);
  }

  // --------------------------------------------------
  // opticalFlowLK
  // --------------------------------------------------

  // default constructor
  opticalFlowLK::opticalFlowLK()
    : transform(){
    parameters param;
	  setParameters(param);

  }

  // copy constructor
  opticalFlowLK::opticalFlowLK(const opticalFlowLK& other)
    : transform()  {
    copy(other);
  }

  // destructor
  opticalFlowLK::~opticalFlowLK() {
  }

  // returns the name of this type
  const char* opticalFlowLK::getTypeName() const {
    return "opticalFlowLK";
  }

  // copy member
  opticalFlowLK&
    opticalFlowLK::copy(const opticalFlowLK& other) {
      transform::copy(other);
    return (*this);
  }

  // clone member
  functor* opticalFlowLK::clone() const {
    return new opticalFlowLK(*this);
  }

  // return parameters
  const opticalFlowLK::parameters&
    opticalFlowLK::getParameters() const {
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

  // On place apply for type channel!
  void opticalFlowLK::apply(channel& at_t,channel& at_delta_t) const {
  	lti:: channel uTemp, vTemp;
  	apply(at_t,at_delta_t, uTemp,vTemp);
  	uTemp.detach(at_t);
  	vTemp.detach(at_delta_t);
  }

  // On copy apply for type channel!
  void opticalFlowLK::apply(const channel& at_t,
                            const channel& at_delta_t,
                            channel& u,
                            channel& v) const {

    if (at_t.size()!=at_delta_t.size()) {return;}

    const parameters& params = getParameters();

    // compute spatial derivatives
    convolution               filter;
    convolution::parameters   filterParams;
    gradientKernelX<float>    gradient_x(params.kernelSize);
    gradientKernelY<float>    gradient_y(params.kernelSize);
    gaussKernel1D<float>      gaussTemp(params.windowEdgeSize);
    sepKernel<float>          gauss(gaussTemp);

    channel gt,gx,gy;
    int rows = at_t.rows();
    int cols = at_delta_t.columns();

    // compute time derivative
//    gt = at_t;
//    gt.subtract(at_delta_t);

    gt = at_delta_t;
    gt.subtract(at_t);

    // compute x derivative
    filterParams.boundaryType = lti::Constant;
    filterParams.setKernel(gradient_x);
    filter.setParameters(filterParams);
    filter.apply(at_t,gx);

    // compute y derivative
    filterParams.boundaryType = lti::Constant;
    filterParams.setKernel(gradient_y);
    filter.setParameters(filterParams);
    filter.apply(at_t,gy);

    // gaussian filter
    filterParams.boundaryType = lti::Constant;
    filterParams.setKernel(gauss);
    filter.setParameters(filterParams);

    // variables for optical flow
    channel uTemp(rows,cols,float(0));
    channel vTemp(rows,cols,float(0));

    // Solutionmatrix
    // (A1B2   B1)   (u)   (C1)
    // (         ) * ( ) = (  )
    // (A2   A1B2)   (v)   (C2)

    // computing coefficients of matrix, which is to solve
    // (A1B2, A2, B1, C1, C2)
    lti::channel WXX;
    lti::channel WXY;
    lti::channel WYY;
    lti::channel WXT;
    lti::channel WYT;

    WXX.emultiply(gx,gx);
    WXY.emultiply(gx,gy);
    WYY.emultiply(gy,gy);
    WXT.emultiply(gx,gt);
    WYT.emultiply(gy,gt);
/*
    // an additional low-pass filter not used in the description of the alg.
    int smoothIterations=4;
    while(smoothIterations>0) {
      smoothIterations--;
      filter.apply(WXX);
      filter.apply(WYY);
      filter.apply(WXY);
      filter.apply(WXT);
      filter.apply(WYT);
    }
*/
    // apply gaussian filter
    int offset =(int)(params.windowEdgeSize/2);
    int tempX, tempY;
    for (int counter_row=0; counter_row<rows; counter_row++) {
      for (int counter_col=0; counter_col<cols; counter_col++) {

        float A1B2 = 0;
        float A2   = 0;
        float B1   = 0;
        float C1   = 0;
        float C2   = 0;

        int totalsize = params.windowEdgeSize*params.windowEdgeSize;
        for (int winsize=0;winsize<totalsize;winsize++) {
          tempX = winsize%params.windowEdgeSize+counter_col-offset;
          tempY = (int)winsize/params.windowEdgeSize+counter_row-offset;
          if (tempX < 0) {
            tempX = abs(tempX);
          }
          if (tempX >= cols) {
            tempX = cols - (tempX - cols) - 1;
          }
          if (tempY < 0) {
            tempY = abs(tempY);
          }
          if (tempY >= rows) {
            tempY = rows - (tempY - rows) - 1;
          }

          A1B2 += WXY.at(tempY,tempX);
          A2   += WXX.at(tempY,tempX);
          B1   += WYY.at(tempY,tempX);
          C1   += WYT.at(tempY,tempX);
          C2   += WXT.at(tempY,tempX);
        }

        // solve matrix
         float delta = ( A1B2*A1B2 - A2 * B1 );
         if ( delta ) {
            // system is not singular - solving by Kramer method
            float deltaX;
            float deltaY;
            float Idelta = 1 / delta;

            deltaX =  - (C1 * A1B2 - C2 * B1);
            deltaY =  - (A1B2 * C2 - A2 * C1);

            uTemp.at(counter_row,counter_col) =  deltaX * Idelta;
            vTemp.at(counter_row,counter_col) =  deltaY * Idelta;
          }
          else {
            // singular system - find optical flow in gradient direction
            float Norm = (A1B2+A2)*(A1B2+A2) + (B1+A1B2)*(B1+A1B2);
            if (Norm) {
               float IGradNorm = 1 / Norm;
               float temp = - (C1+C2) * IGradNorm;

               uTemp.at(counter_row,counter_col) = (A1B2+A2) * temp;
               vTemp.at(counter_row,counter_col) = (B1+A1B2) * temp;
            }
            else {
              uTemp.at(counter_row,counter_col) =  0;
              vTemp.at(counter_row,counter_col) =  0;
            }
          }

        }
      }

      uTemp.detach(u);
      vTemp.detach(v);
  }
}
