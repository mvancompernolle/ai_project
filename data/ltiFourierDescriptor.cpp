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
 * file .......: ltiFourierDescriptor.cpp
 * authors ....: Norman Pfeil
 * organization: LTI, RWTH Aachen
 * creation ...: 26.4.2001
 * revisions ..: $Id: ltiFourierDescriptor.cpp,v 1.9 2006/09/05 10:13:14 ltilib Exp $
 */

#include "ltiFourierDescriptor.h"

namespace lti {
  // --------------------------------------------------
  // fourierDescriptor::parameters
  // --------------------------------------------------

  // default constructor
  fourierDescriptor::parameters::parameters()
    : transform::parameters() {
    // Initialize parameter
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    origin = lti::tpoint<float>(0,0);
  }

  // copy constructor
  fourierDescriptor::parameters::parameters(const parameters& other)
    : transform::parameters()  {
    copy(other);
  }

  // destructor
  fourierDescriptor::parameters::~parameters() {
  }

  // get type name
  const char* fourierDescriptor::parameters::getTypeName() const {
    return "fourierDescriptor::parameters";
  }

  // copy member

  fourierDescriptor::parameters&
    fourierDescriptor::parameters::copy(const parameters& other) {
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

    origin = other.origin;

    return *this;
  }

  // alias for copy member
  fourierDescriptor::parameters&
    fourierDescriptor::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* fourierDescriptor::parameters::clone() const {
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
  bool fourierDescriptor::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool fourierDescriptor::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"origin",origin);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::write(handler,false);
# else
    bool (transform::parameters::* p_writeMS)(ioHandler&,const bool) const =
      transform::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool fourierDescriptor::parameters::write(ioHandler& handler,
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
  bool fourierDescriptor::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool fourierDescriptor::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::write(handler,"origin",origin);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::read(handler,false);
# else
    bool (transform::parameters::* p_readMS)(ioHandler&,const bool) =
      transform::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool fourierDescriptor::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // fourierDescriptor
  // --------------------------------------------------

  // default constructor
  fourierDescriptor::fourierDescriptor()
    : transform(){
    parameters param;
    setParameters(param);
  }

  // copy constructor
  fourierDescriptor::fourierDescriptor(const fourierDescriptor& other)
    : transform()  {
    copy(other);
  }

  // destructor
  fourierDescriptor::~fourierDescriptor() {
  }

  // returns the name of this type
  const char* fourierDescriptor::getTypeName() const {
    return "fourierDescriptor";
  }

  // copy member
  fourierDescriptor&
    fourierDescriptor::copy(const fourierDescriptor& other) {
      transform::copy(other);
    return (*this);
  }

  // clone member
  functor* fourierDescriptor::clone() const {
    return new fourierDescriptor(*this);
  }

  // return parameters
  const fourierDescriptor::parameters&
    fourierDescriptor::getParameters() const {
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


  // On copy apply
  bool fourierDescriptor::apply(const borderPoints&  input,
                                vector<float>& xRealOutput,
                                vector<float>& xImagOutput,
                                vector<float>& yRealOutput,
                                vector<float>& yImagOutput) const {

    const parameters& param = getParameters();

    // local variables
    lti::vector<float> xVector,yVector;      // input split to x and y values
    lti::borderPoints::const_iterator bIter; // Iterator of input
    int newSize;                             // new size of xVector and yVector
    int i;                                   // counter

    // Increase xVector and yVector to next 2^n and
    // fill with first point of the input (refering to origin)
    newSize = 1 << static_cast<int>( (log(double(input.size())))/
                                     (log(2.0)) +1.0); // 2^...
    xVector.resize(newSize,(*input.begin()).x-param.origin.x,false);
    yVector.resize(newSize,(*input.begin()).y-param.origin.y,false);

    // Use parameter "origin" as reference
    for (bIter = input.begin(), i=0;
         bIter != input.end(), i<input.size();
         ++bIter, ++i) {
      xVector[i]=(*bIter).x-param.origin.x;
      yVector[i]=(*bIter).y-param.origin.y;
    }

    // Fast fourier transformation of xVector and yVector
    lti::realFFT fft;
    fft.apply(xVector,xRealOutput,xImagOutput);
    fft.apply(yVector,yRealOutput,yImagOutput);

    return true;
  };

  // On copy apply
  bool fourierDescriptor::apply(const vector<float>& xRealInput,
                                const vector<float>& xImagInput,
                                const vector<float>& yRealInput,
                                const vector<float>& yImagInput,
                                borderPoints&  output) const {
    const parameters& param = getParameters();

    // compute inverse fourier transformed
    lti::vector<float> xVectorNew, yVectorNew;

    lti::realInvFFT ifft;
    ifft.apply(xRealInput,xImagInput,xVectorNew);
    ifft.apply(yRealInput,yImagInput,yVectorNew);

    // Create temporary channel with minimum required size (+margin)
    const int margin = 5;

    float fminX,fmaxX,fminY,fmaxY;

    xVectorNew.getExtremes(fminX,fmaxX);
    yVectorNew.getExtremes(fminY,fmaxY);

    const int minX(static_cast<int>(fminX));
    const int maxX(static_cast<int>(fmaxX));
    const int minY(static_cast<int>(fminY));
    const int maxY(static_cast<int>(fmaxY));

    int xoff(minX-margin);
    int yoff(minY-margin);
    int columns(abs(maxX - minX) +2*margin);
    int rows((maxY - minY) +2*margin);

    // Draw resulting points as closed contour ("margin" ensures
    // coordinate validity)

    lti::channel8 chnl(rows,columns,static_cast<ubyte>(0));
    int xLast,yLast,x,y;
    lti::draw<lti::channel8::value_type> drawer;
    drawer.setColor(255);
    drawer.use(chnl);
    // initialize
    x = xLast = static_cast<int>(lti::round(xVectorNew[0]-xoff));
    y = yLast = static_cast<int>(lti::round(yVectorNew[0]-yoff));
    drawer.set(x,y);
    // start drawing
    for (int i=1; i<xVectorNew.size();i++) {
      x = static_cast<int>(lti::round(xVectorNew[i]-xoff));
      y = static_cast<int>(lti::round(yVectorNew[i]-yoff));
      if (abs(x-xLast)>1 || abs(y-yLast)>1) {
        // Close gap with line
        drawer.lineTo(x,y);
      }
      else {
        // Set point
        drawer.set(x,y);
      }
      xLast = x;
      yLast = y;
    }
    // Close border
    drawer.lineTo(static_cast<int>(lti::round(xVectorNew[0]-xoff)),
                  static_cast<int>(lti::round(yVectorNew[0]-yoff)));

    // Get object borderpoints
    lti::objectsFromMask ofm;
    lti::objectsFromMask::parameters ofmParams;
    ofmParams.level=0;
    ofm.setParameters(ofmParams);
    std::list<lti::borderPoints> bpl;
    ofm.apply(chnl,bpl);
    // push points into output list, after considering offset and origin
    output.clear();
    lti::borderPoints::iterator it;
    for (it=(*bpl.begin()).begin(); it!=(*bpl.begin()).end(); it++){
      output.push_back(lti::point(static_cast<int>(lti::round((*it).x+xoff+param.origin.x)) ,
                                  static_cast<int>(lti::round((*it).y+yoff+param.origin.y))) );
    }

    return true;
  };
}
