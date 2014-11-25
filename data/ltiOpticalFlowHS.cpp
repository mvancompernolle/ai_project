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
 * file .......: ltiOpticalFlowHS.cpp
 * authors ....: Bernd Mussmann, Suat Akyol
 * organization: LTI, RWTH Aachen
 * creation ...: 15.8.2000
 * revisions ..: $Id: ltiOpticalFlowHS.cpp,v 1.7 2006/02/08 11:34:43 ltilib Exp $
 */

#include "ltiOpticalFlowHS.h"

namespace lti {
  // --------------------------------------------------
  // opticalFlowHS::parameters
  // --------------------------------------------------

  // default constructor
  opticalFlowHS::parameters::parameters()
    : transform::parameters() {

    kernelSize = int(3);
	  iterations = int(100);
	  lambda	   = float(10);
    multiScale = true;
    initialize = false;
  }

  // copy constructor
  opticalFlowHS::parameters::parameters(const parameters& other)
    : transform::parameters()  {
    copy(other);
  }

  // destructor
  opticalFlowHS::parameters::~parameters() {
  }

  // get type name
  const char* opticalFlowHS::parameters::getTypeName() const {
    return "opticalFlowHS::parameters";
  }

  // copy member

  opticalFlowHS::parameters&
    opticalFlowHS::parameters::copy(const parameters& other) {
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
	  iterations = other.iterations;
	  lambda	   = other.lambda;
	  multiScale = other.multiScale;
    initialize = other.initialize;

    return *this;
  }

  // clone member
  functor::parameters* opticalFlowHS::parameters::clone() const {
    return new parameters(*this);
  }

  // --------------------------------------------------
  // opticalFlowHS
  // --------------------------------------------------

  // default constructor
  opticalFlowHS::opticalFlowHS()
    : transform(){
	  parameters param;
	  setParameters(param);

  }

  // copy constructor
  opticalFlowHS::opticalFlowHS(const opticalFlowHS& other)
    : transform()  {
    copy(other);
  }

  // destructor
  opticalFlowHS::~opticalFlowHS() {
  }

  // returns the name of this type
  const char* opticalFlowHS::getTypeName() const {
    return "opticalFlowHS";
  }

  // copy member
  opticalFlowHS& opticalFlowHS::copy(const opticalFlowHS& other) {
    transform::copy(other);
    return (*this);
  }

  // clone member
  functor* opticalFlowHS::clone() const {
    return new opticalFlowHS(*this);
  }

  // return parameters
  const opticalFlowHS::parameters&
    opticalFlowHS::getParameters() const {
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
  void opticalFlowHS::apply(channel& ch_t0_u,channel& ch_t1_v) const {
    // Call "On copy apply"
    lti::channel temp_u;
    lti::channel temp_v;
    opticalFlowHS::apply( ch_t0_u, ch_t1_v, temp_u, temp_v);
    temp_u.detach(ch_t0_u);
    temp_v.detach(ch_t1_v);
  }

  // On copy apply for type channel!
  void opticalFlowHS::apply(const channel& ch_t0,
                            const channel& ch_t1,
                            channel& u,
                            channel& v) const {

    // Input channels must be equal sized
    if (ch_t0.size()!=ch_t1.size()) {
      return; // Do nothing!
    }

    // Get parameters
    const parameters& params = getParameters();

    if (params.multiScale==true) {
      computeMultiScaled(ch_t0,ch_t1,u,v);
    }
    else {
      computeSingleScaled(ch_t0,ch_t1,u,v);
    }
  }


  // Private Function
  void opticalFlowHS::computeOpticalFlow(const channel& ch_t0,
                                         const channel& ch_t1,
                                         channel& u,
                                         channel& v,
                                         int iterations) const{

    // Get parameters
    const parameters& params = getParameters();

    if(iterations <=0) return; // Do nothing


    // Some local variables
    lti::channel gt,gx,gy;
    int rows	= ch_t0.rows();
    int columns = ch_t0.columns();

    // Compute spatial gradients
    lti::convolution filter;
    lti::convolution::parameters filter_params;
    lti::gradientKernelX<float> gradient_x(params.kernelSize);
    lti::gradientKernelY<float> gradient_y(params.kernelSize);
    filter_params.boundaryType = lti::Constant;
    filter.setParameters(filter_params);
    // x-direction
    filter.setKernel(gradient_x);
    filter.apply(ch_t0,gx);
    gx.multiply(-2.0f);    // Algorithm requires polarised negative gradients!
    // y-direction
    filter.setKernel(gradient_y);
    filter.apply(ch_t0,gy);
    gy.multiply(-2.0f);    // Algorithm requires polarised negative gradients!
    // Compute temporal gradient
    gt = ch_t0;
    gt.subtract(ch_t1);

    // Compute optical flow according to Horn-Schunck algorithm
    // lti::channel temp_u, temp_v;
    float median_u, median_v, alpha;
    int totalIterations = 1;
    int i, j;

    while (totalIterations <= iterations)  {
      // first row------------------------------------------------------------
      i=0;
      // First Pixel of Row
      j=0;
      median_u = (u.at(i,j)+u.at(i+1,j)+u.at(i,j)+u.at(i,j+1))/4;
      median_v = (v.at(i,j)+v.at(i+1,j)+v.at(i,j)+v.at(i,j+1))/4;
      alpha = params.lambda*( gx.at(i,j)*median_u+gy.at(i,j)*median_v+
                              gt.at(i,j) )
        / ( 1+params.lambda*( gx.at(i,j)*gx.at(i,j)+gy.at(i,j)*gy.at(i,j) ) );
      u.at(i,j) = median_u-alpha*gx.at(i,j);
      v.at(i,j) = median_v-alpha*gy.at(i,j);
      // Middle Region
      for (j = 1;j<columns-1;j++) {
        median_u = (u.at(i,j)+u.at(i+1,j)+u.at(i,j-1)+u.at(i,j+1))/4;
        median_v = (v.at(i,j)+v.at(i+1,j)+v.at(i,j-1)+v.at(i,j+1))/4;
        alpha = params.lambda*( gx.at(i,j)*median_u+gy.at(i,j)*median_v+
                                gt.at(i,j) )
          / ( 1+params.lambda*( gx.at(i,j)*gx.at(i,j)+gy.at(i,j)*gy.at(i,j)));
        u.at(i,j) = median_u-alpha*gx.at(i,j);
        v.at(i,j) = median_v-alpha*gy.at(i,j);
      }
      // Last Pixel of Row
      median_u = (u.at(i,j)+u.at(i+1,j)+u.at(i,j-1)+u.at(i,j))/4;
      median_v = (v.at(i,j)+v.at(i+1,j)+v.at(i,j-1)+v.at(i,j))/4;
      alpha = params.lambda*( gx.at(i,j)*median_u+gy.at(i,j)*median_v+
                              gt.at(i,j) )
        / ( 1+params.lambda*( gx.at(i,j)*gx.at(i,j)+gy.at(i,j)*gy.at(i,j) ) );
      u.at(i,j) = median_u-alpha*gx.at(i,j);
      v.at(i,j) = median_v-alpha*gy.at(i,j);


      // Middle rows----------------------------------------------------------
      for (i = 1;i<rows-1;i++) {
        // First Pixel of Row
        j=0;
        median_u = (u.at(i-1,j)+u.at(i+1,j)+u.at(i,j)+u.at(i,j+1))/4;
        median_v = (v.at(i-1,j)+v.at(i+1,j)+v.at(i,j)+v.at(i,j+1))/4;
        alpha = params.lambda*( gx.at(i,j)*median_u+gy.at(i,j)*median_v+
                                gt.at(i,j) )
          / ( 1+params.lambda*( gx.at(i,j)*gx.at(i,j)+gy.at(i,j)*gy.at(i,j)));
        u.at(i,j) = median_u-alpha*gx.at(i,j);
        v.at(i,j) = median_v-alpha*gy.at(i,j);
        // Middle Region
        for (j = 1;j<columns-1;j++) {
          median_u = (u.at(i-1,j)+u.at(i+1,j)+u.at(i,j-1)+u.at(i,j+1))/4;
          median_v = (v.at(i-1,j)+v.at(i+1,j)+v.at(i,j-1)+v.at(i,j+1))/4;
          alpha = params.lambda*( gx.at(i,j)*median_u+gy.at(i,j)*median_v+
                                  gt.at(i,j) )
            / (1+params.lambda*( gx.at(i,j)*gx.at(i,j)+gy.at(i,j)*gy.at(i,j)));
          u.at(i,j) = median_u-alpha*gx.at(i,j);
          v.at(i,j) = median_v-alpha*gy.at(i,j);
        }
        // Last Pixel of Row
        median_u = (u.at(i-1,j)+u.at(i+1,j)+u.at(i,j-1)+u.at(i,j))/4;
        median_v = (v.at(i-1,j)+v.at(i+1,j)+v.at(i,j-1)+v.at(i,j))/4;
        alpha = params.lambda*( gx.at(i,j)*median_u+gy.at(i,j)*median_v+
                                gt.at(i,j) )
          / ( 1+params.lambda*( gx.at(i,j)*gx.at(i,j)+gy.at(i,j)*gy.at(i,j)));
        u.at(i,j) = median_u-alpha*gx.at(i,j);
        v.at(i,j) = median_v-alpha*gy.at(i,j);
      }

      // last row------------------------------------------------------------
      // First Pixel of Row
      j=0;
      median_u = (u.at(i-1,j)+u.at(i,j)+u.at(i,j)+u.at(i,j+1))/4;
      median_v = (v.at(i-1,j)+v.at(i,j)+v.at(i,j)+v.at(i,j+1))/4;
      alpha = params.lambda*( gx.at(i,j)*median_u+gy.at(i,j)*median_v+
                              gt.at(i,j) )
        / ( 1+params.lambda*( gx.at(i,j)*gx.at(i,j)+gy.at(i,j)*gy.at(i,j) ) );
      u.at(i,j) = median_u-alpha*gx.at(i,j);
      v.at(i,j) = median_v-alpha*gy.at(i,j);

      // Middle Region
      for (j = 1;j<columns-1;j++) {
        median_u = (u.at(i-1,j)+u.at(i,j)+u.at(i,j-1)+u.at(i,j+1))/4;
        median_v = (v.at(i-1,j)+v.at(i,j)+v.at(i,j-1)+v.at(i,j+1))/4;
        alpha = params.lambda*( gx.at(i,j)*median_u+gy.at(i,j)*median_v+
                                gt.at(i,j) )
          / ( 1+params.lambda*( gx.at(i,j)*gx.at(i,j)+gy.at(i,j)*gy.at(i,j)));
        u.at(i,j) = median_u-alpha*gx.at(i,j);
        v.at(i,j) = median_v-alpha*gy.at(i,j);
      }
      // Last Pixel of Row
      median_u = (u.at(i-1,j)+u.at(i,j)+u.at(i,j-1)+u.at(i,j))/4;
      median_v = (v.at(i-1,j)+v.at(i,j)+v.at(i,j-1)+v.at(i,j))/4;
      alpha = params.lambda*( gx.at(i,j)*median_u+gy.at(i,j)*median_v+
                              gt.at(i,j) )
        / ( 1+params.lambda*( gx.at(i,j)*gx.at(i,j)+gy.at(i,j)*gy.at(i,j) ) );
      u.at(i,j) = median_u-alpha*gx.at(i,j);
      v.at(i,j) = median_v-alpha*gy.at(i,j);


      // --------------------------------------------------------------------

      totalIterations++;
    }
  }


  // Private Function
  void opticalFlowHS::computeMultiScaled(const channel& ch_t0,
							                           const channel& ch_t1,
							                           channel& u,
							                           channel& v) const{

    // Get parameters
    const parameters& params = getParameters();
    int i;

    // prepare sequences of downsampled input channels.
    // first Element of sequence is original channel.
    // the sequences
    lti::sequence<channel> seq_ch_t0;
    lti::sequence<channel> seq_ch_t1;

    // Kernel for channel downsampling
    lti::gaussKernel1D<lti::channel::value_type> tmp(3);
    lti::sepKernel<lti::channel::value_type> kernel(tmp);
    // Parameters for channel downsampling
    lti::downsampling::parameters ds_params;
    ds_params.factor = lti::point(2,2);
    ds_params.boundaryType = lti::Constant;
    ds_params.setKernel(kernel);
    // the downsampler
    lti::downsampling downsampler;
    downsampler.setParameters(ds_params);

    // sample channels down by factor 2 while rows
    // and columns >4 * kernelSize of Gradient Filter
    // AND number of resolution levels < params.iterations.
    // Append all downsampled channels to a sequence.
    const int sizeLimit=4*params.kernelSize;
    seq_ch_t0.append(ch_t0);
    seq_ch_t1.append(ch_t1);
    while ( (seq_ch_t0[seq_ch_t0.size()-1].rows()>=sizeLimit)
            && (seq_ch_t0[seq_ch_t0.size()-1].columns()>=sizeLimit)
            && (seq_ch_t0.size()<params.iterations) ) {
      seq_ch_t0.append(lti::channel());
      seq_ch_t1.append(lti::channel());
      downsampler.apply(seq_ch_t0[seq_ch_t0.size()-2],
                        seq_ch_t0[seq_ch_t0.size()-1]);
      downsampler.apply(seq_ch_t1[seq_ch_t1.size()-2],
                        seq_ch_t1[seq_ch_t1.size()-1]);
    }
    // actual size of sequence.
    const int iSeqLength = seq_ch_t0.size();

    // downsample output channels, if initialization is desired
    // and given channels have correct size
    // resize and initialize output channels, if necessary
    if ( (params.initialize==true) && (u.size()==ch_t0.size()) ){
      for (i=0;i<iSeqLength-1;i++){
        downsampler.apply(u);
      }
    }
    else{
      u.resize(ch_t0.size(),float(0),false,true);
    }
    if ( (params.initialize==true) && (v.size()==ch_t0.size()) ){
      for (i=0;i<iSeqLength-1;i++){
        downsampler.apply(v);
      }
    }
    else{
      v.resize(ch_t0.size(),float(0),false,true);
    }

    // prepare upsampler
    lti::filledUpsampling::parameters us_params;
    lti::filledUpsampling upsampler;
    us_params.factor = lti::point(2,2);
    upsampler.setParameters(us_params);
    // Some variables
    const float iterationsPerLevel = ((float)params.iterations)/iSeqLength;
    int iterationsForThisLevel;
    int levelIndex;

    // Compute flow for each resolution level and upsample result
    for (levelIndex=iSeqLength-1;levelIndex>0;levelIndex--) {
      iterationsForThisLevel = (int)(iterationsPerLevel/(1<<levelIndex));
      if (iterationsForThisLevel<1) {
        iterationsForThisLevel = 1;
      }
      computeOpticalFlow(seq_ch_t0[levelIndex],
                         seq_ch_t1[levelIndex],
                         u,
                         v,
                         iterationsForThisLevel);
      upsampler.apply(u);
      upsampler.apply(v);
      // Do correct line/row difference in size after upsampling
      if (u.size()!=seq_ch_t0[levelIndex-1].size()) {
        u.resize(seq_ch_t0[levelIndex-1].size(),0.0f);
        v.resize(seq_ch_t0[levelIndex-1].size(),0.0f);
      }
    }
    // Compute flow on highest resolution level
    computeOpticalFlow(seq_ch_t0[levelIndex],
                       seq_ch_t1[levelIndex],
                       u,
                       v,
                       params.iterations-
                       (int)((iSeqLength-1)*iterationsPerLevel));
  }

  // Private Function
  void opticalFlowHS::computeSingleScaled(const channel& ch_t0,
                                          const channel& ch_t1,
                                          channel& u,
                                          channel& v) const{
    // Get parameters
    const parameters& params = getParameters();

    // resize and initialize output channels, if necessary
    if ( (u.size()!=ch_t0.size()) || (params.initialize==false) ) {
      u.resize(ch_t0.size(),float(0),false,true);
    }
    if ( (v.size()!=ch_t0.size()) || (params.initialize==false) ) {
      v.resize(ch_t0.size(),float(0),false,true);
    }

    computeOpticalFlow(ch_t0,ch_t1,u,v,params.iterations);
  }


}
