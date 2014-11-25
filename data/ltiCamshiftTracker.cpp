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
 * file .......: ltiCamshiftTracker.cpp
 * authors ....: Suat Akyol
 * organization: LTI, RWTH Aachen
 * creation ...: 8.6.2001
 * revisions ..: $Id: ltiCamshiftTracker.cpp,v 1.10 2006/09/05 10:04:22 ltilib Exp $
 */

//include files
#include "ltiCamshiftTracker.h"

namespace lti {
  const float epsilon = 0.1f;

  // --------------------------------------------------
  // camshiftTracker::parameters
  // --------------------------------------------------

  // default constructor
  camshiftTracker::parameters::parameters()
    : modifier::parameters() {
    //Initialize parameter values
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    kernelType = eKernelType(Constant);
    sizeRatio = 0.0;
    threshold = 0.0;
  }

  // copy constructor
  camshiftTracker::parameters::parameters(const parameters& other)
    : modifier::parameters()  {
    copy(other);
  }

  // destructor
  camshiftTracker::parameters::~parameters() {
  }

  // get type name
  const char* camshiftTracker::parameters::getTypeName() const {
    return "camshiftTracker::parameters";
  }

  // copy member

  camshiftTracker::parameters&
    camshiftTracker::parameters::copy(const parameters& other) {
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
      sizeRatio = other.sizeRatio;
      threshold = other.threshold;

    return *this;
  }

  // alias for copy member
  camshiftTracker::parameters&
    camshiftTracker::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* camshiftTracker::parameters::clone() const {
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
  bool camshiftTracker::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool camshiftTracker::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"sizeRatio",sizeRatio);
      lti::write(handler,"threshold",threshold);

      switch(kernelType) {
          case Constant:
            lti::write(handler,"kernelType","Constant");
            break;
          case Gaussian:
            lti::write(handler,"kernelType","Periodic");
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
  bool camshiftTracker::parameters::write(ioHandler& handler,
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
  bool camshiftTracker::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool camshiftTracker::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"sizeRatio",sizeRatio);
      lti::read(handler,"threshold",threshold);

      std::string str;
      lti::read(handler,"kernelType",str);

      if (str == "Constant") {
        kernelType = Constant;
      } else if (str == "Gaussian") {
        kernelType = Gaussian;
      } else {
        kernelType = Constant;
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
  bool camshiftTracker::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // camshiftTracker
  // --------------------------------------------------

  // default constructor
  camshiftTracker::camshiftTracker()
    : modifier(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

    // Initialize private members
    initialized = false;
    valid = false;
  }

  // copy constructor
  camshiftTracker::camshiftTracker(const camshiftTracker& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  camshiftTracker::~camshiftTracker() {
  }

  // returns the name of this type
  const char* camshiftTracker::getTypeName() const {
    return "camshiftTracker";
  }

  // copy member
  camshiftTracker& camshiftTracker::copy(const camshiftTracker& other) {
    modifier::copy(other);

    td = other.td;
    initialized = other.initialized;
    valid = other.valid;

    return (*this);
  }

  // clone member
  functor* camshiftTracker::clone() const {
    return new camshiftTracker(*this);
  }

  // return parameters
  const camshiftTracker::parameters&
    camshiftTracker::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // shifts and clips rect to fit into given canvas
  bool camshiftTracker::correctRect(trectangle<int>& rect,
                                    const trectangle<int>& canvas) const {
    int diff = rect.ul.x - canvas.ul.x;
    // relocate box inside canvas
    if (diff < 0.0) {
      rect.ul.x -= diff;
      rect.br.x -= diff;
    }
    diff = rect.ul.y - canvas.ul.y;
    if (diff < 0.0) {
      rect.ul.y -= diff;
      rect.br.y -= diff;
    }
    diff = canvas.br.x - rect.br.x;
    if (diff < 0.0) {
      rect.ul.x += diff;
      rect.br.x += diff;
    }
    diff = canvas.br.y - rect.br.y;
    if (diff < 0.0) {
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
    return true;
  };

  // calculates moments in given src
  bool camshiftTracker::getTrackerState(const channel& src){

    // Clear tracker data and get parameters
    td.M00=td.M10=td.M01=td.M11=td.M20=td.M02=0.0;
    const parameters& param = getParameters();
    double maxM00=0.0;

    if (param.kernelType == parameters::Gaussian) {
      point center(src.columns()/2,src.rows()/2);
      //gaussKernel1D<float> xKrnl(src.columns(),src.columns());
      gaussKernel1D<float> xKrnl(src.columns());
      xKrnl.multiply(1/xKrnl.at(0));
      //gaussKernel1D<float> yKrnl(src.rows(),src.rows());
      gaussKernel1D<float> yKrnl(src.rows());
      yKrnl.multiply(1/yKrnl.at(0));
      // Weigthed Moments of p,q-th order
      int rows = src.rows();
      int columns = src.columns();
      float value;
      for (int y=0; y<rows;y++) {
        for (int x=0; x<columns;x++) {
          value = src.at(y,x)*yKrnl.at((y-center.y))*xKrnl.at((x-center.x));
          maxM00 += yKrnl.at((y-center.y))*xKrnl.at((x-center.x));
          td.M00 += value;
          td.M10 += x*value;
          td.M01 += y*value;
          td.M11 += x*y*value;
          td.M20 += x*x*value;
          td.M02 += y*y*value;
        }
      }
    }
    else if (param.kernelType == parameters::Constant) {
      // Moments of p,q-th order
      int rows = src.rows();
      int columns = src.columns();
      float value;
      for (int y=0; y<rows;y++) {
        for (int x=0; x<columns;x++) {
          value = src.at(y,x);
          maxM00 += 1.0;
          td.M00 += value;
          td.M10 += x*value;
          td.M01 += y*value;
          td.M11 += x*y*value;
          td.M20 += x*x*value;
          td.M02 += y*y*value;
        }
      }
    }

    // data within window not sufficient for given threshold -> return false
    if (td.M00 < param.threshold*maxM00) {
      return false;
    }

    // Centers (if M00!=0)
    if (td.M00>0.0) {
      td.xc = td.M10/td.M00;
      td.yc = td.M01/td.M00;
      if (param.kernelType == parameters::Gaussian) {
        td.s = 2*sqrt(td.M00);
      } else if (param.kernelType == parameters::Constant) {
        td.s = sqrt(td.M00);
      }
    }
    else {
      return false;
    }

    return true;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------


  // On copy apply for type channel!
  bool camshiftTracker::apply(const channel& src,rectangle& rect) {
    // get parameters
    const parameters& param = getParameters();

    const tpoint<float> winCenter(static_cast<float>(absdiff(rect.br.x,rect.ul.x))/2,
                                  static_cast<float>(absdiff(rect.br.y,rect.ul.y))/2);
    const point winSize = rect.getDimensions();
    const rectangle canvas(0,0,src.lastColumn(),src.lastRow());

    valid = false;

    tpoint<float> diff(0,0);
    float distanceSquare, lastDistSq;
    distanceSquare = static_cast<float>(winSize.x*winSize.x + winSize.y*winSize.y);

    do {
        // Shift window
        rect.ul.x += iround(diff.x);
        rect.br.x += iround(diff.x);
        rect.ul.y += iround(diff.y);
        rect.br.y += iround(diff.y);
        correctRect(rect, canvas);

        // build subchannel (doesn't own data) for calculation
        const channel subchnl(false, *const_cast<channel*>(&src), // very ugly, don't imitate!
                        rect.ul.y, rect.br.y, rect.ul.x, rect.br.x);
        // get data from rect-region in src
        if (!getTrackerState(subchnl)) {
          // if not valid, then leave
          td.absCenter.x = static_cast<float>(rect.ul.x + td.xc);
          td.absCenter.y = static_cast<float>(rect.ul.y + td.yc);
          return true;
        }
        diff.x = static_cast<float>(td.xc - winCenter.x);
        diff.y = static_cast<float>(td.yc - winCenter.y);
        lastDistSq = distanceSquare;
        distanceSquare = (diff.x*diff.x + diff.y*diff.y);
    } while ( (distanceSquare > 1) && (distanceSquare < lastDistSq-epsilon) );
    td.absCenter.x = static_cast<float>(rect.ul.x + td.xc);
    td.absCenter.y = static_cast<float>(rect.ul.y + td.yc);
    valid = true;

    // adapt window size
    if (param.sizeRatio >1.0) {
      if (td.s > 0.0) {
        float temp = static_cast<float>((rect.ul.x + rect.br.x + 1)/2);
        float temp2 = static_cast<float>(td.s/2);
        rect.ul.x = static_cast<int>( floor(temp - temp2));
        rect.br.x = static_cast<int>( floor(temp + temp2) + 1);
        temp = static_cast<float>((rect.ul.y + rect.br.y + 1)/2);
        rect.ul.y = static_cast<int>( floor(temp - param.sizeRatio*temp2));
        rect.br.y = static_cast<int>( floor(temp + param.sizeRatio*temp2) + 1);
      }
    }
    else if (param.sizeRatio >0.0) {
      if (td.s > 0.0) {
        float temp = static_cast<float>((rect.ul.y + rect.br.y + 1)/2);
        float temp2 = static_cast<float>(td.s/2);
        rect.ul.y = static_cast<int>( floor(temp - temp2));
        rect.br.y = static_cast<int>( floor(temp + temp2) + 1);
        temp = static_cast<float>((rect.ul.x + rect.br.x + 1)/2);
        rect.ul.x = static_cast<int>( floor(temp - temp2/param.sizeRatio));
        rect.br.x = static_cast<int>( floor(temp + temp2/param.sizeRatio) + 1);
      }
    }

    // indicate initialized
    if (!initialized) {
      initialized=true;
    }
    // result
    correctRect(rect, canvas);
    return true;
  };

  // On copy apply for type channel8!
  bool camshiftTracker::apply(const channel8& src,rectangle& rect) {

    lti::channel chnl;
    chnl.castFrom(src);
    return apply(chnl,rect);
  };

  // Resets the internal state but not the parameters
  bool camshiftTracker::reset() {
    initialized = false;
    td.clear();
    return true;
  }

  // Resets the internal state but not the parameters
  bool camshiftTracker::isInitialized() {
    return initialized;
  }

  // Tells, if last tracking attempt delivered valid data
  bool camshiftTracker::isValid() {
    return valid;
  }

  // Returns Orientation (-pi to pi) of current internal distribution state
  double camshiftTracker::getOrientation(){
    // Make sure not to divide by zero.
    if (td.M00==0.0) {
      return 0.0;
    }
    double a = td.M20/td.M00-td.xc*td.xc;
    double b = 2*(td.M11/td.M00-td.xc*td.yc);
    double c = td.M02/td.M00-td.yc*td.yc;
    return 0.5*atan2(b,a-c);
  }

  // Returns length of current internal distribution state
  double camshiftTracker::getLength(){
    if (td.M00==0.0) {
      return 0.0;
    }
    double a = td.M20/td.M00-td.xc*td.xc;
    double b = 2*(td.M11/td.M00-td.xc*td.yc);
    double c = td.M02/td.M00-td.yc*td.yc;
    return sqrt( 0.5*((a+c) + sqrt(b*b + (a-c)*(a-c))) );
  }

  // Returns width of current internal distribution state
  double camshiftTracker::getWidth(){
    if (td.M00==0.0) {
      return 0.0;
    }
    double a = td.M20/td.M00-td.xc*td.xc;
    double b = 2*(td.M11/td.M00-td.xc*td.yc);
    double c = td.M02/td.M00-td.yc*td.yc;

    // prevent from invalid values
    double temp = 0.5*((a+c) - sqrt(b*b + (a-c)*(a-c)));
    if (temp>0.0) {
      return sqrt( temp );
    }
    else {
      return 0.0;
    }
  }

  // Returns center of current internal distribution state
  tpoint<float> camshiftTracker::getCenter(){
    return td.absCenter;
  }

  // The tracker state methods
  camshiftTracker::trackerState::trackerState() {
    M00=M10=M01=M11=M20=M02=0.0;
    xc=yc=s=0.0;
  }

  camshiftTracker::trackerState::~trackerState() {
  }

  void camshiftTracker::trackerState::clear() {
    trackerState();
  }

  camshiftTracker::trackerState&
    camshiftTracker::trackerState::copy(const camshiftTracker::trackerState& other) {
    M00=other.M00;
    M10=other.M10;
    M01=other.M01;
    M11=other.M11;
    M20=other.M02;
    xc=other.xc;
    yc=other.yc;
    yc=other.yc;
    s=other.s;
    absCenter=other.absCenter;
    return *this;
  }

  camshiftTracker::trackerState&
    camshiftTracker::trackerState::operator=(const camshiftTracker::trackerState& other) {
      return copy(other);
  }

}
