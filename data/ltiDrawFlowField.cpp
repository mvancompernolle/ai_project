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
 * file .......: ltiDrawFlowField.cpp
 * authors ....: Bernd Mussmann
 * organization: LTI, RWTH Aachen
 * creation ...: 16.8.2000
 * revisions ..: $Id: ltiDrawFlowField.cpp,v 1.10 2006/09/05 10:09:53 ltilib Exp $
 */

// include files
#include "ltiDrawFlowField.h"

namespace lti {
  // --------------------------------------------------
  // drawFlowField::parameters
  // --------------------------------------------------

  // default constructor
  drawFlowField::parameters::parameters()
    : functor::parameters() {
    // Initialize your parameter values!
    zoomfactor = int(1);
    samplerate = int(5);
    threshold  = float(0.1);
    overlayMode= NoOverlay;
    vectorMode = Arrows;
    scalingMode= GridWidth;
    linecolor  = float(1.0);
    tipcolor   = float(1.0);
  }

  // copy constructor
  drawFlowField::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  drawFlowField::parameters::~parameters() {
  }

  // get type name
  const char* drawFlowField::parameters::getTypeName() const {
    return "drawFlowField::parameters";
  }

  // copy member

  drawFlowField::parameters&
    drawFlowField::parameters::copy(const parameters& other) {
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

    zoomfactor  = other.zoomfactor;
    samplerate  = other.samplerate;
	  threshold   = other.threshold;
	  overlayMode = other.overlayMode;
	  vectorMode  = other.vectorMode ;
    scalingMode = other.scalingMode;
	  linecolor   = other.linecolor;
	  tipcolor    = other.tipcolor;

    return *this;
  }

  // clone member
  functor::parameters* drawFlowField::parameters::clone() const {
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
  bool drawFlowField::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool drawFlowField::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"zoomfactor",zoomfactor);
      lti::write(handler,"samplerate",samplerate);
      lti::write(handler,"threshold",threshold);

      switch (overlayMode) {
          case NoOverlay:
            lti::write(handler,"overlayMode","NoOverlay");
            break;
          case OnFlowfield:
            lti::write(handler,"overlayMode","OnFlowfield");
            break;
          case OnValue:
            lti::write(handler,"overlayMode","OnValue");
            break;
          case OnPhase:
            lti::write(handler,"overlayMode","OnPhase");
            break;
          default:
            lti::write(handler,"overlayMode","NoOverlay");
      }

      switch (vectorMode) {
          case Arrows:
            lti::write(handler,"vectorMode","Arrows");
            break;
          case Lines:
            lti::write(handler,"vectorMode","Lines");
            break;
          default:
            lti::write(handler,"vectorMode","Arrows");
      }

      switch (scalingMode) {
          case GridWidth:
            lti::write(handler,"scalingMode","GridWidth");
            break;
          case NoScaling:
            lti::write(handler,"scalingMode","NoScaling");
            break;
          default:
            lti::write(handler,"scalingMode","GridWidth");
      }


      lti::write(handler,"linecolor",linecolor);
      lti::write(handler,"tipcolor",tipcolor);
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
  bool drawFlowField::parameters::write(ioHandler& handler,
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
  bool drawFlowField::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool drawFlowField::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"zoomfactor",zoomfactor);
      lti::read(handler,"samplerate",samplerate);
      lti::read(handler,"threshold",threshold);

      std::string str;
      lti::read(handler,"overlayMode",str);
      if (str == "OnFlowfield") {
        overlayMode = OnFlowfield;
      } else if (str == "OnValue") {
        overlayMode = OnValue;
      } else if (str == "OnPhase") {
        overlayMode = OnPhase;
      } else {
        overlayMode = NoOverlay;
      }

      lti::read(handler,"vectorMode",str);
      if (str == "Lines") {
        vectorMode = Lines;
      } else {
        vectorMode = Arrows;
      }

      lti::read(handler,"scalingMode",str);
      if (str == "NoScaling") {
        scalingMode = NoScaling;
      } else {
        scalingMode = GridWidth;
      }

      lti::read(handler,"linecolor",linecolor);
      lti::read(handler,"tipcolor",tipcolor);
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
  bool drawFlowField::parameters::read(ioHandler& handler,
                                    const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // drawFlowField
  // --------------------------------------------------

  // default constructor
  drawFlowField::drawFlowField()
    : functor(){
	  parameters params;
	  setParameters(params);
  }

  // copy constructor
  drawFlowField::drawFlowField(const drawFlowField& other)
    : functor()  {
    copy(other);
  }

  // destructor
  drawFlowField::~drawFlowField() {
  }

  // returns the name of this type
  const char* drawFlowField::getTypeName() const {
    return "drawFlowField";
  }

  // copy member
  drawFlowField& drawFlowField::copy(const drawFlowField& other) {
    functor::copy(other);
    return (*this);
  }

  // clone member
  functor* drawFlowField::clone() const {
    return new drawFlowField(*this);
  }

  // return parameters
  const drawFlowField::parameters&
    drawFlowField::getParameters() const {
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


  // On place apply for type channel! Result is given back in first parameter!
  bool drawFlowField::apply( channel& x_component,
                                 channel& y_component) const {

    lti::channel temp;

    if (apply(x_component,y_component,temp)) {
      temp.detach(x_component);
      return true;
    }
    return false;
  }


  // On copy apply for type channel!
  bool drawFlowField::apply(const channel& x_component,
                            const channel& y_component,
                                  channel& flowfield) const  {
    // x-comonent and y-component must have same size!
    if (x_component.size()!=y_component.size()) {
      setStatusString("Input components have different size");
      return false;
    }

    int inputrows = x_component.rows();
    int inputcolumns = x_component.columns();

    // Get Parameters
    const parameters& params = getParameters();

    // Kernel for channel downsampling
    lti::gaussKernel1D<lti::channel::value_type> tmp(params.samplerate);
    lti::sepKernel<lti::channel::value_type> kernel(tmp);
    // Parameters for channel downsampling
    lti::downsampling::parameters ds_params;
    ds_params.factor = lti::point(params.samplerate,params.samplerate);
    ds_params.boundaryType = lti::Constant;
    ds_params.setKernel(kernel);
    // the downsampler
    lti::downsampling downsampler;
    downsampler.setParameters(ds_params);
    /*
      // Set Kernel for Downsampling of input channels
      int size = params.samplerate-1;
      lti::kernel1D<lti::channel::value_type> tmp(-size,0,1.0/(size+1));
      lti::sepKernel<lti::channel::value_type> kernel(tmp);
      // Set Parameters for Downsampling of input channels
      lti::downsampling::parameters ds_params;
      ds_params.factor = lti::point(size+1,size+1);
      ds_params.setKernel(kernel);
      lti::downsampling downsampler;
      downsampler.setParameters(ds_params);
    */
    // Downsample input channels
    lti::channel phase, value, x_downsampled, y_downsampled;
    x_downsampled = x_component;
    downsampler.apply(x_downsampled);
    y_downsampled = y_component;
    downsampler.apply(y_downsampled);

    // Calculation of value and phase and determination of maxvalue
    value.resize(x_downsampled.size(),float(),false,false);
    phase.resize(x_downsampled.size(),float(),false,false);
    float maxvalue = sqrt(x_downsampled.at(0,0)*x_downsampled.at(0,0)+
                          y_downsampled.at(0,0)*y_downsampled.at(0,0));
    lti::matrix<lti::channel::value_type>::const_iterator it_x,it_y,e;
    lti::matrix<lti::channel::value_type>::iterator it_value,it_phase;
    it_value = value.begin();
    it_phase = phase.begin();

    for ( it_x = x_downsampled.begin(),it_y = y_downsampled.begin(),
            e=x_downsampled.end();
          it_x!=e;
          it_x++,it_y++)	{
      (*it_value) = sqrt( (*it_x)*(*it_x) + (*it_y)*(*it_y) );
      if( (*it_value) > maxvalue ) maxvalue = (*it_value);

      (*it_phase)	= atan2( (*it_y) , (*it_x) );
      it_value++;
      it_phase++;
    }

    // Prepare flowfield for overlayMode
    switch (params.overlayMode)	{
      case parameters::NoOverlay: { // for flow field on black background
        // Create clear channel
        flowfield.resize( params.zoomfactor*inputrows,
                          params.zoomfactor*inputcolumns,
                          float(0),
                          false,
                          true);
      }	break;
      case parameters::OnFlowfield: { // for overlaying flow field on channel,
        // which is given as destination for the flowfield
        lti::point dimensions = flowfield.size();
        if (dimensions != lti::point(0,0))	{
          // prepare upsampler
          lti::filledUpsampling::parameters us_params;
          lti::filledUpsampling upsampler;
          us_params.factor = lti::point(params.zoomfactor,params.zoomfactor);
          upsampler.setParameters(us_params);
          // Upsample overlay image given in flowfield
          upsampler.apply(flowfield);
        }
        // Adjust size (in case, the given image had incorrect size)
        flowfield.resize( params.zoomfactor*inputrows,
                          params.zoomfactor*inputcolumns,
                          float(0),
                          true,
                          true);
      }	break;
      case parameters::OnValue: { // for overlaying flow field on value
        // Set size for Upsampling of value image
        int size = params.zoomfactor*params.samplerate;
        // prepare upsampler
        lti::filledUpsampling::parameters us_params;
        lti::filledUpsampling upsampler;
        us_params.factor = lti::point(size,size);
        upsampler.setParameters(us_params);
				// Upsample overlay image given in flowfield
        flowfield = value;
        upsampler.apply(flowfield);
      }	break;
      case parameters::OnPhase: { // for overlaying flow field on phase
        // Set size for Upsampling of value image
        int size = params.zoomfactor*params.samplerate;
        // prepare upsampler
        lti::filledUpsampling::parameters us_params;
        lti::filledUpsampling upsampler;
        us_params.factor = lti::point(size,size);
        upsampler.setParameters(us_params);
				// Upsample overlay image given in flowfield
        flowfield = phase;
        upsampler.apply(flowfield);
      }	break;
      default: break;
    }

    // Prepare flowfield and drawers for drawing
    lti::draw<lti::channel::value_type> flowlinedrawer, tipdrawer;
    flowlinedrawer.use(flowfield); // for drawing flow lines
    flowlinedrawer.setColor(params.linecolor);
    tipdrawer.use(flowfield);	   // for drawing the tips,
                                   // which indicate the direction
    tipdrawer.setColor(params.tipcolor);

    // factor for scaling flow lines (maxvalue corresponds to gridwidth
    // of flowfield in pixels)
    float vectorlength=0.0f;
    switch (params.scalingMode) {
      case parameters::GridWidth:{
        vectorlength =
          static_cast<float>((params.samplerate*params.zoomfactor)/maxvalue);
        break;
      }
      case parameters::NoScaling:{
        vectorlength = static_cast<float>(params.zoomfactor);
        break;
      }
      default:break;
    }

    float tiplength = static_cast<float>(params.samplerate*params.zoomfactor)
                      /5;
    // Variables for temporary storage of flowvector tip-positions
    float tip_x, tip_y;

    // Draw flowfield
    if (maxvalue == 0.0) {
      // ready! (image empty!)
      return true;
    }

    float threshold = params.threshold*maxvalue;
    int a = params.samplerate*params.zoomfactor;

    for(int i = 0;i < value.rows();i++) {
      for(int j=0;j < value.columns();j++) {
        if (value.at(i,j)>=threshold)	{

          tip_x = a*j+vectorlength*x_downsampled.at(i,j);
          tip_y = a*i+vectorlength*y_downsampled.at(i,j);
          switch (params.vectorMode) {
            case parameters::Arrows: {
              flowlinedrawer.arrow(a*j,a*i,
                                   static_cast<int>(tip_x),
                                   static_cast<int>(tip_y),tiplength);
              break;
            }
            case parameters::Lines: {
              flowlinedrawer.line(a*j,a*i,
                                  static_cast<int>(tip_x),
                                  static_cast<int>(tip_y));
              break;
            }
            default: {
              break;
            }
          }
          tipdrawer.set(static_cast<int>(tip_x),
                        static_cast<int>(tip_y));
        }
      }
    }

    return true;
  }

}
