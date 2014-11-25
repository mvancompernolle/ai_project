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
 * file .......: ltiLkmColorQuantization.cpp
 * authors ....: Axel Berner
 * organization: LTI, RWTH Aachen
 * creation ...: 15.5.2001
 * revisions ..: $Id: ltiLkmColorQuantization.cpp,v 1.9 2006/09/05 10:50:39 ltilib Exp $
 */

#include "ltiLkmColorQuantization.h"

#ifdef _DEBUG
//#define _LTI_DEBUG
#endif

#ifdef _LTI_DEBUG

#include <iostream>
using std::cout;
using std::endl;

#endif

namespace lti {
  // --------------------------------------------------
  // lkmColorQuantization::parameters
  // --------------------------------------------------

  // default constructor
  lkmColorQuantization::parameters::parameters()
    : colorQuantization::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    shrinkLearnRate = 0.5;
    learnRate       = 0.6;
    maxIterations   =  10;
    neighbour       = 0.1;
  }

  // copy constructor
  lkmColorQuantization::parameters::parameters(const parameters& other)
    : colorQuantization::parameters()  {
    copy(other);
  }

  // destructor
  lkmColorQuantization::parameters::~parameters() {
  }

  // get type name
  const char* lkmColorQuantization::parameters::getTypeName() const {
    return "lkmColorQuantization::parameters";
  }

  // copy member

  lkmColorQuantization::parameters&
    lkmColorQuantization::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    colorQuantization::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    colorQuantization::parameters& (colorQuantization::parameters::* p_copy)
      (const colorQuantization::parameters&) =
      colorQuantization::parameters::copy;
    (this->*p_copy)(other);
# endif


      shrinkLearnRate = other.shrinkLearnRate;
      learnRate       = other.learnRate;
      maxIterations   = other.maxIterations;
      neighbour       = other.neighbour;

    return *this;
  }

  // alias for copy member
  lkmColorQuantization::parameters&
    lkmColorQuantization::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* lkmColorQuantization::parameters::clone() const {
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
  bool lkmColorQuantization::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool lkmColorQuantization::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"shrinkLearnRate",shrinkLearnRate);
      lti::write(handler,"learnRate",learnRate);
      lti::write(handler,"maxIterations",maxIterations);
      lti::write(handler,"neighbour",neighbour);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && colorQuantization::parameters::write(handler,false);
# else
    bool (colorQuantization::parameters::* p_writeMS)(ioHandler&,const bool) const =
      colorQuantization::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool lkmColorQuantization::parameters::write(ioHandler& handler,
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
  bool lkmColorQuantization::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool lkmColorQuantization::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"shrinkLearnRate",shrinkLearnRate);
      lti::read(handler,"learnRate",learnRate);
      lti::read(handler,"maxIterations",maxIterations);
      lti::read(handler,"neighbour",neighbour);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && colorQuantization::parameters::read(handler,false);
# else
    bool (colorQuantization::parameters::* p_readMS)(ioHandler&,const bool) =
      colorQuantization::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool lkmColorQuantization::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // lkmColorQuantization
  // --------------------------------------------------

  const int* lkmColorQuantization::sqrLUT = 0;

  // default constructor
  lkmColorQuantization::lkmColorQuantization()
    : colorQuantization(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

    // create LUT for sqroot just once!
    if (isNull(sqrLUT)) {
      int* sqrlut = new int[511];
      int i;
      for(i = -255; i < 256; ++i) {
        sqrlut[i+255] = (i*i);
      }
      sqrLUT = &sqrlut[255];
    }
  }

  // copy constructor
  lkmColorQuantization::lkmColorQuantization(const lkmColorQuantization& other)
    : colorQuantization()  {
    copy(other);
  }

  // destructor
  lkmColorQuantization::~lkmColorQuantization() {
  }

  // returns the name of this type
  const char* lkmColorQuantization::getTypeName() const {
    return "lkmColorQuantization";
  }

  // copy member
  lkmColorQuantization&
    lkmColorQuantization::copy(const lkmColorQuantization& other) {
      colorQuantization::copy(other);
    return (*this);
  }

  // clone member
  functor* lkmColorQuantization::clone() const {
    return new lkmColorQuantization(*this);
  }

  // return parameters
  const lkmColorQuantization::parameters&
    lkmColorQuantization::getParameters() const {
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

  // On copy apply for type image!
  bool lkmColorQuantization::apply(const image& src,
				   channel8& dest,
				   lti::palette& lkmCPalette) const {
    // defInit
    if (src.empty()) {
      dest.clear();
      lkmCPalette.clear();
      setStatusString("Empty source image");
      return false;
    }

    const parameters& param = getParameters();

    if(( param.numberOfColors > 256) || (param.numberOfColors < 1) ) {
      dest.clear();
      lkmCPalette.clear();
      setStatusString("Palette can have min. 1 entry and max. 256 entries");
      return false;
    }

    lkmCPalette.resize(param.numberOfColors,rgbPixel(),false,false);
    int palSize = lkmCPalette.size();

    // init destinationImage
    if (dest.getMode() == lti::channel8::Line) {
      dest.clear();
    }

    dest.resize(src.size(),0,false,false);

    // Init lkmCPalette
    ubyte v;
    int i;
    for (i=0; i<palSize; ++i) {
      v = ubyte(255*i/(palSize-1));
      lkmCPalette[i].set(v,v,v,0);
    }

    //Local K-means algorithm
    int changes = -1;
    double alpha = param.learnRate;
    int colorDiff;
    int pxlSize = src.columns()*src.rows(); // # Pixel of the image
    int palMinPos(0); //PalettePosition

    static const int stepsize[25] = {   2,   3,   5,   7,  11,
                                       13,  19,  37,  59,  79,
                                      109, 139, 149, 151, 157,
                                      163, 197, 239, 307, 499,
                                      757,1009,1423,1801,2309};

    int iter = 0;
    while (iter<param.maxIterations && changes!=0) {
      changes = 0;

      int pxlPos = 0;

      for(int ss=4;ss<20;ss++) // indices for the stepsize array(pseudorandom)

	for(pxlPos = pxlPos%pxlSize;
            pxlPos < pxlSize;
            pxlPos += stepsize[ss]) {

          const rgbPixel& spx = src.at(pxlPos);  // the analysed source pixel

	  int minDiff = 195075; //3*255*255;
	  for(int palPos = 0; palPos < palSize; ++palPos) {
            rgbPixel& ppx = lkmCPalette[palPos];
	    colorDiff =  sqrLUT[static_cast<int>(spx.getRed()) -
                                static_cast<int>(ppx.getRed())]
                        +sqrLUT[static_cast<int>(spx.getGreen()) -
                                static_cast<int>(ppx.getGreen())]
                        +sqrLUT[static_cast<int>(spx.getBlue()) -
                                static_cast<int>(ppx.getBlue())];
	    if (colorDiff<minDiff) {
	      minDiff = colorDiff;
	      palMinPos = palPos;
	    }
	  }

          rgbPixel& px = lkmCPalette.at(palMinPos);

	  px.setRed(ubyte((1.0-alpha)*px.getRed()
                         +(    alpha)*spx.getRed() +0.5));
	  px.setGreen(ubyte( (1.0-alpha)*px.getGreen()
                            +(    alpha)*spx.getGreen() +0.5));
          px.setBlue(ubyte( (1.0-alpha)*px.getBlue()
                           +(    alpha)*spx.getBlue() +0.5));

	  if (palMinPos > 0) {
            rgbPixel& pxn = lkmCPalette.at(palMinPos-1);
	    pxn.setRed(ubyte( (1.0-alpha*param.neighbour)*pxn.getRed()
                             +(    alpha*param.neighbour)*spx.getRed() +0.5));
            pxn.setGreen(ubyte( (1.0-alpha*param.neighbour)*pxn.getGreen()
                               +( alpha*param.neighbour)*spx.getGreen()+0.5));
	    pxn.setBlue (ubyte( (1.0-alpha*param.neighbour)*pxn.getBlue()
                               +( alpha*param.neighbour)*spx.getBlue() +0.5));
	  }
	  if (palMinPos < palSize-1) {
            rgbPixel& pxn = lkmCPalette.at(palMinPos+1);
	    pxn.setRed  (ubyte( (1.0-alpha*param.neighbour)*pxn.getRed()
                               +( alpha*param.neighbour)*spx.getRed() +0.5));
	    pxn.setGreen(ubyte( (1.0-alpha*param.neighbour)*pxn.getGreen()
                               +( alpha*param.neighbour)*spx.getGreen() +0.5));
	    pxn.setBlue (ubyte( (1.0-alpha*param.neighbour)*pxn.getBlue()
                               +( alpha*param.neighbour)*spx.getBlue() +0.5));
	  }

	  if (dest.at(pxlPos) != palMinPos) {
	    dest.at(pxlPos) = palMinPos;
	    changes++;
	  }
	}

#ifdef _LTI_DEBUG
	cout << "Iteration " << iter << " ready; " << changes << "Changes \n";
#endif

      alpha *= param.shrinkLearnRate; // in every step alpha is smaller
      iter++; // next iteration!
    }

    // set palette index for all points
    for(int pxlPos = 0; pxlPos < pxlSize; pxlPos++) {

      const rgbPixel& spx = src.at(pxlPos);  // the analysed source pixel

      int minDiff = 195075; //3*255*255;
      for(int palPos = 0; palPos < palSize; ++palPos) {
        rgbPixel& ppx = lkmCPalette[palPos];
        colorDiff =  sqrLUT[static_cast<int>(spx.getRed())   -
                            static_cast<int>(ppx.getRed()  )]
                    +sqrLUT[static_cast<int>(spx.getGreen()) -
                            static_cast<int>(ppx.getGreen())]
                    +sqrLUT[static_cast<int>(spx.getBlue())  -
                            static_cast<int>(ppx.getBlue() )];
        if(colorDiff < minDiff) {
          minDiff = colorDiff;
          palMinPos = palPos;
        }
      }
      dest.at(pxlPos) = palMinPos;
    }

    return true;
  };//end apply

  bool lkmColorQuantization::apply(const image& src,
                                         image& dest) const {
    return colorQuantization::apply(src,dest);
  };

  bool lkmColorQuantization::apply(image& srcdest) const {
    return colorQuantization::apply(srcdest);
  };
}
