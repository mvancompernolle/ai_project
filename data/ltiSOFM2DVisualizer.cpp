/*
 * Copyright (C) 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiSOFM2DVisualizer.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 31.10.2002
 * revisions ..: $Id: ltiSOFM2DVisualizer.cpp,v 1.8 2006/09/05 10:01:22 ltilib Exp $
 */

#include "ltiSOFM2DVisualizer.h"

namespace lti {
  // --------------------------------------------------
  // SOFM2DVisualizer::parameters
  // --------------------------------------------------
    rgbPixel a[]={Red,Green,Blue,
                  Yellow,Cyan,Magenta,
                  rgbPixel(255,127,0),rgbPixel(127,255,0),rgbPixel(255,0,127),
                  rgbPixel(127,0,255),rgbPixel(0,127,255),rgbPixel(0,255,127)
    };
  // default colors for visualization
    const vector<rgbPixel> SOFM2DVisualizer::parameters::defaultColors(12,a);
  // default constructor
  SOFM2DVisualizer::parameters::parameters()
    : functor::parameters() {

    colorMap=defaultColors;
    symbolSize=20;
//      sammonsMapping::parameters mp;

//    smP=mp;
    smP.steps=200;
    smP.alpha=0.4;
    smP.initType=sammonsMapping::parameters::PCA;
    smP.searchType=sammonsMapping::parameters::Steepest;


  }

  // copy constructor
  SOFM2DVisualizer::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  SOFM2DVisualizer::parameters::~parameters() {
  }

  // get type name
  const char* SOFM2DVisualizer::parameters::getTypeName() const {
    return "SOFM2DVisualizer::parameters";
  }

  // copy member

  SOFM2DVisualizer::parameters&
    SOFM2DVisualizer::parameters::copy(const parameters& other) {
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


      colorMap = other.colorMap;
      symbolSize = other.symbolSize;
      smP = other.smP;
    return *this;
  }

  // alias for copy member
  SOFM2DVisualizer::parameters&
    SOFM2DVisualizer::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* SOFM2DVisualizer::parameters::clone() const {
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
  bool SOFM2DVisualizer::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool SOFM2DVisualizer::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"colorMap",colorMap);
      lti::write(handler,"symbolSize",symbolSize);
      lti::write(handler,"smP",smP);
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
  bool SOFM2DVisualizer::parameters::write(ioHandler& handler,
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
  bool SOFM2DVisualizer::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool SOFM2DVisualizer::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"colorMap",colorMap);
      lti::read(handler,"symbolSize",symbolSize);
      lti::read(handler,"smP",smP);
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
  bool SOFM2DVisualizer::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // SOFM2DVisualizer
  // --------------------------------------------------

  // default constructor
  SOFM2DVisualizer::SOFM2DVisualizer()
    : functor(){

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
  SOFM2DVisualizer::SOFM2DVisualizer(const parameters& par)
    : functor() {

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  SOFM2DVisualizer::SOFM2DVisualizer(const SOFM2DVisualizer& other)
    : functor()  {
    copy(other);
  }

  // destructor
  SOFM2DVisualizer::~SOFM2DVisualizer() {
  }

  // returns the name of this type
  const char* SOFM2DVisualizer::getTypeName() const {
    return "SOFM2DVisualizer";
  }

  // copy member
  SOFM2DVisualizer& SOFM2DVisualizer::copy(const SOFM2DVisualizer& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  SOFM2DVisualizer&
    SOFM2DVisualizer::operator=(const SOFM2DVisualizer& other) {
    return (copy(other));
  }


  // clone member
  functor* SOFM2DVisualizer::clone() const {
    return new SOFM2DVisualizer(*this);
  }

  // return parameters
  const SOFM2DVisualizer::parameters&
    SOFM2DVisualizer::getParameters() const {
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

}
