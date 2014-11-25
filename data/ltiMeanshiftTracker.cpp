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
 * file .......: ltiMeanshiftTracker.cpp
 * authors ....: Torsten Kämper
 * organization: LTI, RWTH Aachen
 * creation ...: 24.9.2001
 * revisions ..: $Id: ltiMeanshiftTracker.cpp,v 1.10 2006/09/05 10:21:59 ltilib Exp $
 */

// include files
#include "ltiMeanshiftTracker.h"
#include "ltiHistogram.h"
#include <cstdio>
#include <iostream>

namespace lti {
  const int DIM =3;
  const int CELLS_PER_DIM =32;

  // --------------------------------------------------
  // meanshiftTracker::parameters
  // --------------------------------------------------

  // default constructor
  meanshiftTracker::parameters::parameters()
    : modifier::parameters() {
    // Initialize parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters
    kernelType = eKernelType(Normal);
    precision = 2.0f;
    threshold = 0.8f;
    sizeAdaptRatio = 0.1f;
  }

  // copy constructor
  meanshiftTracker::parameters::parameters(const parameters& other)
    : modifier::parameters()  {
    copy(other);
  }

  // destructor
  meanshiftTracker::parameters::~parameters() {
  }

  // get type name
  const char* meanshiftTracker::parameters::getTypeName() const {
    return "meanshiftTracker::parameters";
  }

  // copy member

  meanshiftTracker::parameters&
    meanshiftTracker::parameters::copy(const parameters& other) {
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

    kernelType = other.kernelType;
    precision = other.precision;
    threshold = other.threshold;
    sizeAdaptRatio = other.sizeAdaptRatio;
    return *this;
  }

  // alias for copy member
  meanshiftTracker::parameters&
    meanshiftTracker::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* meanshiftTracker::parameters::clone() const {
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
  bool meanshiftTracker::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool meanshiftTracker::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
       lti::write(handler,"precision",precision);
       lti::write(handler,"threshold",threshold);
       lti::write(handler,"sizeAdaptRatio",sizeAdaptRatio);

        switch(kernelType) {
          case Normal:
            lti::write(handler,"kernelType","Normal");
            break;
          case Epanechnikov:
            lti::write(handler,"kernelType","Epanechnikov");
            break;
          default:
            lti::write(handler,"kernelType","unknown");
      }
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
  bool meanshiftTracker::parameters::write(ioHandler& handler,
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
  bool meanshiftTracker::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool meanshiftTracker::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
       lti::read(handler,"precision",precision);
       lti::read(handler,"threshold",threshold);
       lti::read(handler,"sizeAdaptRatio",sizeAdaptRatio);

       std::string str;
       lti::read(handler,"kernelType",str);

       if (str == "Normal") {
        kernelType = Normal;
       } else if (str == "Epanechnikov") {
        kernelType = Epanechnikov;
       } else {
        kernelType = Normal;
       }
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
  bool meanshiftTracker::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // meanshiftTracker
  // --------------------------------------------------

  // default constructor
  meanshiftTracker::meanshiftTracker()
    : modifier(){

    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

    initialized = false;
    valid=false;
  }

  // copy constructor
  meanshiftTracker::meanshiftTracker(const meanshiftTracker& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  meanshiftTracker::~meanshiftTracker() {
  }

  // returns the name of this type
  const char* meanshiftTracker::getTypeName() const {
    return "meanshiftTracker";
  }

  // copy member
  meanshiftTracker& meanshiftTracker::copy(const meanshiftTracker& other) {
    modifier::copy(other);
    
    td = other.td;
    initialized = other.initialized;
    valid = other.valid;

    return (*this);
  }

  // alias for copy member
  meanshiftTracker&
    meanshiftTracker::operator=(const meanshiftTracker& other) {
    return (copy(other));
  }


  // clone member
  functor* meanshiftTracker::clone() const {
    return new meanshiftTracker(*this);
  }

  // return parameters
  const meanshiftTracker::parameters&
    meanshiftTracker::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  void meanshiftTracker::calcProb(thistogram<float>& prob,
                                  const trectangle<int>& canvas,
                                  const image& src,
                                  const channel8& mask) const {
    // fills the histogram "prob" with the colors inside the
    // rectangle "canvas" in the image. Each color is weighted
    // with a kernel function AND with the given mask. Note:
    // the histogramm is not
    // normalized to 1, thus use prob.getProbability() to obtain
    // probabilities.
    prob.initialize();

    vector<int> vec(3,0);
    point dim=canvas.getDimensions();
    tpoint<int> radius(dim.x/2,dim.y/2);

    for (int row=canvas.ul.y;row<canvas.br.y; row++) {   //row (y)
      for (int col=canvas.ul.x;col<canvas.br.x; col++) {  //column (x)
        tpoint<int> position(col,row);
        ubyte weight = mask.at(row,col);
        if (weight) {

          rgbPixel pixel=src.at(row,col);

          vec.at(0)=pixel.getRed()*CELLS_PER_DIM/256;
          vec.at(1)=pixel.getGreen()*CELLS_PER_DIM/256;
          vec.at(2)=pixel.getBlue()*CELLS_PER_DIM/256;

          float kern = kernel(canvas.getCenter(),position,radius)*static_cast<float>(weight);

          prob.put(vec,kern);
        }
      }
    }
  }

   void meanshiftTracker::calcProb(thistogram<float>& prob,
                                  const trectangle<int>& canvas,
                                  const image& src,
                                  const channel& mask) const {
    // fills the histogram "prob" with the colors inside the
    // rectangle "canvas" in the image. Each color is weighted
    // with a kernel function AND with the given mask. Note:
    // the histogramm is not
    // normalized to 1, thus use prob.getProbability() to obtain
    // probabilities.
    prob.initialize();

    vector<int> vec(3,0);
    point dim=canvas.getDimensions();
    tpoint<int> radius(dim.x/2,dim.y/2);

    for (int row=canvas.ul.y;row<canvas.br.y; row++) {   //row (y)
      for (int col=canvas.ul.x;col<canvas.br.x; col++) {  //column (x)
        tpoint<int> position(col,row);
        float weight = mask.at(row,col);
        if (weight) {

          rgbPixel pixel=src.at(row,col);

          vec.at(0)=pixel.getRed()*CELLS_PER_DIM/256;
          vec.at(1)=pixel.getGreen()*CELLS_PER_DIM/256;
          vec.at(2)=pixel.getBlue()*CELLS_PER_DIM/256;

          float kern = kernel(canvas.getCenter(),position,radius)*weight;

          prob.put(vec,kern);
        }
      }
    }
  }


  void meanshiftTracker::calcProb(thistogram<float>& prob,
                                  const trectangle<int>& canvas,
                                  const image& src) const {
    // fills the histogram "prob" with the colors inside the
    // rectangle "canvas" in the image. Each color is weighted
    // with a kernel function. Note: the histogramm is not
    // normalized to 1, thus use prob.getProbability() to obtain
    // probabilities.
    prob.initialize();

    vector<int> vec(3,0);
    point dim=canvas.getDimensions();
    tpoint<int> radius(dim.x/2,dim.y/2);

    for (int row=canvas.ul.y;row<canvas.br.y; row++) {   //row (y)
      for (int col=canvas.ul.x;col<canvas.br.x; col++) {  //column (x)

        tpoint<int> position(col,row);
        rgbPixel pixel=src.at(row,col);

        vec.at(0)=pixel.getRed()*CELLS_PER_DIM/256;
        vec.at(1)=pixel.getGreen()*CELLS_PER_DIM/256;
        vec.at(2)=pixel.getBlue()*CELLS_PER_DIM/256;

        float kern = kernel(canvas.getCenter(),position,radius);

        prob.put(vec,kern);
      }
    }
  }

  float meanshiftTracker::kernel(const tpoint<int>& cen,
                                 const tpoint<int>& pos,
                                 const tpoint<int>& h) const {

    const parameters& param = getParameters();

    float temp1 = static_cast<float>(cen.x-pos.x);
    temp1 = temp1/h.x;
    temp1 = temp1*temp1;

    float temp2 = static_cast<float>(cen.y-pos.y);
    temp2 = temp2/h.y;
    temp2 = temp2*temp2;

    float ksi = temp1 + temp2;

    // Normal Kernel
    if (param.kernelType == parameters::Normal){
      //return 1/(2*Pi)*exp(-0.5*ksi);
      return exp(-0.5f*ksi);
    }
    // Epanechnikov Kernel
    else if(param.kernelType == parameters::Epanechnikov){
      if (ksi <=1) {
        //return 1/(2*cd)*4*(1-ksi);
        return (1-ksi);
      }
       else {
         return 0;
       }
    }
    // unknown Kernel
    else return 0;

  }

  float meanshiftTracker::derivateKernel(const tpoint<int>& cen,
                                         const tpoint<int>& pos,
                                         const tpoint<int>& h) const {

    const parameters& param = getParameters();

    float temp1 = static_cast<float>(cen.x-pos.x);
    temp1 = temp1/h.x;
    temp1 = temp1*temp1;

    float temp2 = static_cast<float>(cen.y-pos.y);
    temp2 = temp2/h.y;
    temp2 = temp2*temp2;

    float ksi = temp1 + temp2;

    // Normal Kernel
    if (param.kernelType == parameters::Normal){
      return 0.5f*exp(-0.5f*ksi);
    }
    // Epanechnikov Kernel
    else if(param.kernelType == parameters::Epanechnikov){
      if (ksi <=1) {
        return 1;
      }
       else {
         return 0;
       }
    }
    // unknown Kernel
    else return 0;
  }

  void meanshiftTracker::initialize(const image& src,trectangle<int>& window, const channel8& mask) {
    td.y1=lti::tpoint<float>(
      static_cast<float>(window.getDimensions().x)/2,
      static_cast<float>(window.getDimensions().y)/2);
    td.bhat1=0.0;
    td.distance=0;
    calcProb(td.targetProb,window,src,mask);
    td.candProb.initialize();
    td.hwRatio = static_cast<float>(window.getDimensions().y)/window.getDimensions().x;

    initialized=true;
    valid=false;
  }

   void meanshiftTracker::initialize(const image& src,trectangle<int>& window, const channel& mask) {
    td.y1=lti::tpoint<float>(
      static_cast<float>(window.getDimensions().x)/2,
      static_cast<float>(window.getDimensions().y)/2);
    td.bhat1=0.0;
    td.distance=0;
    calcProb(td.targetProb,window,src,mask);
    td.candProb.initialize();
    td.hwRatio = static_cast<float>(window.getDimensions().y)/window.getDimensions().x;

    initialized=true;
    valid=false;
  }

  void meanshiftTracker::initialize(const image& src,trectangle<int>& window) {
    td.y1=lti::tpoint<float>(
      static_cast<float>(window.getDimensions().x)/2,
      static_cast<float>(window.getDimensions().y)/2);
    td.bhat1=0.0;
    td.distance=0;
    calcProb(td.targetProb,window,src);
    td.candProb.initialize();
    td.hwRatio = static_cast<float>(window.getDimensions().y)/window.getDimensions().x;

    initialized=true;
    valid=false;
  }

  float meanshiftTracker::calcBhatCoef(const thistogram<float>& targetProb,
                                       const thistogram<float>& candProb) const {
    // calculates the bhattacharyya metric for measuring
    // similarity of two color distributions
    thistogram<float>::const_iterator it1,it2;
    float bhat=0.0;
    float entries = targetProb.getNumberOfEntries()*candProb.getNumberOfEntries();

    if (entries <= 0.0) {
      return 0.0;
    }

    it1=targetProb.begin();
    for (it2=candProb.begin();it2!=candProb.end();it2++) {
      bhat += sqrt( (*it1)*(*it2) );
      it1++;
    }
    return bhat/sqrt(entries);
  }

  float meanshiftTracker::calcDistance(const thistogram<float>& targetProb,
                                       const thistogram<float>& candProb) const {
    // calculate the color distribution distance between two histogramms
    float dist;
    dist=sqrt(1.0f-calcBhatCoef(targetProb,candProb));
    return dist;
  }

  void meanshiftTracker::correctRect(trectangle<int>& rect,
                                     const trectangle<int>& canvas) const {
    int diff = rect.ul.x - canvas.ul.x;
    // relocate box inside canvas
    if (diff < 0) {
      rect.ul.x -= diff;
      rect.br.x -= diff;
    }
    diff = rect.ul.y - canvas.ul.y;
    if (diff < 0) {
      rect.ul.y -= diff;
      rect.br.y -= diff;
    }
    diff = canvas.br.x - rect.br.x;
    if (diff < 0) {
      rect.ul.x += diff;
      rect.br.x += diff;
    }
    diff = canvas.br.y - rect.br.y;
    if (diff < 0) {
      rect.ul.y += diff;
      rect.br.y += diff;
    }
    // if still outside, clip length
    if (rect.ul.x < canvas.ul.x) {
      rect.ul.x = canvas.ul.x;
    }
    if (rect.ul.y < canvas.ul.y) {
      rect.ul.y = canvas.ul.y;
    }
    if (rect.br.x > canvas.br.x) {
      rect.br.x = canvas.br.x;
    }
    if (rect.br.y > canvas.br.y) {
      rect.br.y = canvas.br.y;
    }
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------


  // On copy apply for type image!
  bool meanshiftTracker::apply(const image& src, trectangle<int>& window){

    // get parameters
    const parameters& param = getParameters();

    tpoint<int> newCenter;
    trectangle<int> origWindow = window;

    // the image canvas (needed for correctRect())
    const trectangle<int> canvas(0,0,src.lastColumn(),src.lastRow());

    // make sure, that the window is inside the image
    correctRect(window,canvas);

    // initialize the tracker and return
    if (!initialized) {
      initialize(src,window);
      return true;
    }

    thistogram<float> tempHist(DIM,CELLS_PER_DIM);

    //local distance between y0 and y1
    tpoint<float> y0;
    y0.x=static_cast<float>(window.br.x + window.ul.x)/2;
    y0.y=static_cast<float>(window.br.y + window.ul.y)/2;
    float dist = 0;
    float lastDist;
    td.y1 = y0;

    ivector vec(3,0);
    float bhat0;
    // begin iterations of mean-shift algorithm
    do {
      // step 1: initialize the location of the target in the current
      // frame with y0, compute the distribution and evaluate the
      // bhattacharrya metric
      y0=td.y1;

      calcProb(tempHist,window,src);
      bhat0=calcBhatCoef(td.targetProb,tempHist);

      // step 2: derive the weights {wi}
      // is done inside the loop!

      // step 3: based on the mean-shift vector, derive the new location
      // of the target
      // denominator for y1 calculation
      float denominator=0.0;
      // numerator for y1 calculation
      tpoint<float> numerator(0,0);

      // radius of distribution
      tpoint<int> radius, position;
      radius.x=window.getDimensions().x/2;
      radius.y=window.getDimensions().y/2;

      //weight
      float wi;

      for (int row=window.ul.y;row<window.br.y; row++) {    //zeile(y),
        for (int col=window.ul.x;col<window.br.x; col++) {  //spalte(x)
          position.x=col;
          position.y=row;
          rgbPixel pixel=src.at(row,col);
          vec.at(0)=pixel.getRed()*CELLS_PER_DIM/256;
          vec.at(1)=pixel.getGreen()*CELLS_PER_DIM/256;
          vec.at(2)=pixel.getBlue()*CELLS_PER_DIM/256;
          //reads the weights in histogram for the color at position (row,col)
          float temp1 = static_cast<float>(td.targetProb.getProbability(vec));
          float temp2 = static_cast<float>(tempHist.getProbability(vec));
          if (temp2>0.0) {
            wi=sqrt(temp1/temp2);
            float kern = derivateKernel(window.getCenter(),position,radius);
            // numerator
            numerator.x += wi*kern*position.x;
            numerator.y += wi*kern*position.y;
            // denominator
            denominator += wi*kern;
          }
        }
      }
      if(denominator!=0.0){
        td.y1.x=numerator.x/denominator;
        td.y1.y=numerator.y/denominator;
      }
      else{
        return false;
      }

      // update and evaluate at new position y1
      newCenter.x=static_cast<int>(round(td.y1.x));
      newCenter.y=static_cast<int>(round(td.y1.y));
      window.setCenter(newCenter);
      correctRect(window, canvas);
      //calculates the histogram and the bhattacharrya coeff.
      // for new window position y1
      calcProb(td.candProb,window,src);
      td.bhat1=calcBhatCoef(td.targetProb,td.candProb);

      // step 4:
      float lastBhat = 0;
      while ((td.bhat1<bhat0)&&(fabs(lastBhat-td.bhat1)>0.01)) {
        td.y1.x=0.5f*(y0.x+td.y1.x);
        td.y1.y=0.5f*(y0.y+td.y1.y);

        newCenter.x=static_cast<int>(round(td.y1.x));
        newCenter.y=static_cast<int>(round(td.y1.y));
        window.setCenter(newCenter);
        correctRect(window, canvas);
        calcProb(td.candProb,window,src);
        lastBhat = td.bhat1;
        td.bhat1=calcBhatCoef(td.targetProb,td.candProb);
      }

      lastDist=dist;
      dist=td.y1.distanceTo(y0);

    }while((dist>=param.precision) && (fabs(lastDist-dist)>0.1));//back to Step1

    // store distance
    td.distance=sqrt(1-td.bhat1);

    // scale adaptation
    if (param.sizeAdaptRatio>0.0) {
      thistogram<float> tempHist2(DIM,CELLS_PER_DIM);
      // calculates the color distribution distances for original-size,+10% and -10%
      float sDistance,lDistance;
      trectangle<int> sWindow,lWindow;
      point origDim=origWindow.getDimensions();

      //+ param.sizeAdaptRatio*100%
      point largerDim;
      largerDim.x=static_cast<int>(round(origDim.x*(1+param.sizeAdaptRatio)));
      largerDim.y=static_cast<int>(round(largerDim.x*td.hwRatio));
      //calculates the distance for larger window-size
      lWindow.resize(largerDim);
      lWindow.setCenter(newCenter);
      correctRect(lWindow,canvas);
      calcProb(tempHist,lWindow,src);
      lDistance=calcDistance(td.targetProb,tempHist);

      //- param.sizeAdaptRatio*100%
      point smallerDim;
      smallerDim.x=static_cast<int>(round(origDim.x*(1-param.sizeAdaptRatio)));
      smallerDim.y=static_cast<int>(round(smallerDim.x*td.hwRatio));
      //calculates the distance for smaller window-size
      sWindow.resize(smallerDim);
      sWindow.setCenter(newCenter);
      correctRect(sWindow,canvas);
      calcProb(tempHist2,sWindow,src);
      sDistance=calcDistance(td.targetProb,tempHist2);

      // see if larger or smaller window fits better
      if ( (sDistance<td.distance) || (lDistance<td.distance) ) {
        if ( lDistance<=sDistance ) {
          window = lWindow;
          td.distance=lDistance;
          tempHist.detach(td.candProb);
        }
        else if ( sDistance<lDistance ) {
          window = sWindow;
          td.distance=sDistance;
          tempHist2.detach(td.candProb);
        }
      }
    }


    // see if result is valid
    if(td.distance>param.threshold){
      window = origWindow;
      valid = false;
    }
    else {
      valid = true;
    }

    return true;
  };

  bool meanshiftTracker::isInitialized() const {
    return initialized;
  }

  void meanshiftTracker::reset() {
    td.clear();
    initialized=false;
    valid=false;
  }

  bool meanshiftTracker::isValid() const {
    return valid;
  }

  float meanshiftTracker::getDistance() const {
    return td.distance;
  }

  tpoint<float> meanshiftTracker::getCenter() const {
    return td.y1;
  }

  const thistogram<float>& meanshiftTracker::getTargetHist() const {
    return td.targetProb;
  }

  const thistogram<float>& meanshiftTracker::getCandidateHist() const {
    return td.candProb;
  }

  meanshiftTracker::trackerState::trackerState() {
    y1=lti::tpoint<float>(0,0);
    bhat1=0.0;
    distance = 0;
    hwRatio = 1;

    targetProb.resize(DIM,CELLS_PER_DIM);
    candProb.resize(DIM,CELLS_PER_DIM);
  }

  meanshiftTracker::trackerState::~trackerState() {
  }

  void meanshiftTracker::trackerState::clear() {
    y1=lti::tpoint<float>(0,0);
    bhat1=0.0;
    distance = 0;
    hwRatio = 1;

    targetProb.initialize();
    candProb.initialize();
  }

  meanshiftTracker::trackerState&
    meanshiftTracker::trackerState::copy(const meanshiftTracker::trackerState& other) {
    y1=other.y1;
    bhat1=other.bhat1;
    distance = other.distance;
    hwRatio = other.hwRatio;

    targetProb = other.targetProb;
    candProb = other.candProb;
    return *this;
  }

  meanshiftTracker::trackerState&
    meanshiftTracker::trackerState::operator=(const meanshiftTracker::trackerState& other) {
      return copy(other);
  }

}
