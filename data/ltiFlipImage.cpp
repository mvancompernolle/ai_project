/*
 * Copyright (C) 2003, 2004, 2005, 2006
 * Vlad Popovici, EPFL STI-ITS, Switzerland
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
 * file .......: ltiFlipImage.cpp
 * authors ....: Vlad Popovici
 * organization: EPFL STI-ITS/LTS1
 * creation ...: 20.6.2003
 * revisions ..: $Id: ltiFlipImage.cpp,v 1.8 2006/09/05 10:13:04 ltilib Exp $
 */

#include "ltiFlipImage.h"

namespace lti {
  // --------------------------------------------------
  // flipImage::parameters
  // --------------------------------------------------

  // default constructor
  flipImage::parameters::parameters()
    : modifier::parameters() {
    direction = Horizontal;
  }
  
  // copy constructor
  flipImage::parameters::parameters(const parameters& other)
    : modifier::parameters() {
    copy(other);
  }
  
  // destructor
  flipImage::parameters::~parameters() {
  }
  
  // get type name
  const char* flipImage::parameters::getTypeName() const {
    return "flipImage::parameters";
  }

  // copy member

  flipImage::parameters&
    flipImage::parameters::copy(const parameters& other) {
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
    
    direction = other.direction;
    
    return *this;
  }

  // alias for copy member
  flipImage::parameters&
    flipImage::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* flipImage::parameters::clone() const {
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
  bool flipImage::parameters::write(ioHandler& handler,
                                    const bool complete) const
# else
  bool flipImage::parameters::writeMS(ioHandler& handler,
                                      const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      switch(direction) {
        case Horizontal:
          lti::write(handler,"direction","Horizontal");
          break;
        case Vertical:
          lti::write(handler,"direction","Vertical");
          break;
        case Both:
          lti::write(handler,"direction","Both");
          break;
        default:
          lti::write(handler,"direction","Horizontal");
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
  bool flipImage::parameters::write(ioHandler& handler,
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
  bool flipImage::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool flipImage::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string str;
      lti::read(handler,"direction",str);
      if (str == "Horizontal") {
        direction = Horizontal;
      } else if (str == "Vertical") {
        direction = Vertical;
      } else if (str == "Both") {
        direction = Both;
      } else {
        direction = Horizontal;
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
  bool flipImage::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // flipImage
  // --------------------------------------------------

  // default constructor
  flipImage::flipImage()
    : modifier(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  flipImage::flipImage(const parameters& par)
    : modifier() {
    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  flipImage::flipImage(const flipImage& other)
    : modifier() {
    copy(other);
  }

  // destructor
  flipImage::~flipImage() {
  }

  // returns the name of this type
  const char* flipImage::getTypeName() const {
    return "flipImage";
  }

  // copy member
  flipImage& flipImage::copy(const flipImage& other) {
      modifier::copy(other);
    return (*this);
  }

  // alias for copy member
  flipImage& flipImage::operator=(const flipImage& other) {
    return (copy(other));
  }


  // clone member
  functor* flipImage::clone() const {
    return new flipImage(*this);
  }

  // return parameters
  const flipImage::parameters& flipImage::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // Small private template class to achieve the turn independently of T
  // -------------------------------------------------------------------

  template<class T>
  class flipHelper {
  public:
    static bool apply(matrix<T>& srcdest,
                      const flipImage::parameters::eFlipDirection direction);

    static bool apply(const matrix<T>& src, matrix<T>& dest,
                      const flipImage::parameters::eFlipDirection direction);
  };

  
  template<class T>
  bool flipHelper<T>::apply(matrix<T>& srcdest,
                        const flipImage::parameters::eFlipDirection direction){
    matrix<T> tmpImg;
    if (apply(srcdest, tmpImg, direction)) {
      tmpImg.detach(srcdest);      
      return true;
    }
    return false;
  }

  // On copy apply for type image!
  template<class T>
  bool flipHelper<T>::apply(const matrix<T>& src, 
                         matrix<T>& dest,
                        const flipImage::parameters::eFlipDirection direction){

    dest.resize(src.size(),T(),false,false);
    int x,y;

    switch (direction) {
      case flipImage::parameters::None:
        dest.copy(src);
        break;
      case flipImage::parameters::Horizontal:
        for (y = 0; y < src.rows(); y++) {
          dest.getRow(y).copy(src.getRow(dest.lastRow()-y));
        }
        break;
      case flipImage::parameters::Vertical:
        for (y = 0; y < src.rows(); y++) {
          const vector<T>& srcRow = src.getRow(y);
                vector<T>& destRow = dest.getRow(y);
          for (x = 0; x < src.columns(); x++) {
            destRow.at(dest.lastColumn()-x) = srcRow.at(x);
          }
        }
        break;
      case flipImage::parameters::Both:
        for (y = 0; y < src.rows(); y++) {
          const vector<T>& srcRow = src.getRow(y);
                vector<T>& destRow = dest.getRow(dest.lastRow()-y);
          for (x = 0; x < src.columns(); x++) {
            destRow.at(dest.lastColumn()-x) = srcRow.at(x);
          }
        }
        break;
      default:
        throw lti::exception("flipImage::parameters::direction value unknown");
    }  

    return true;
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------
  
  // On place apply for type image!
  bool flipImage::apply(matrix<rgbPixel>& srcdest) const {
    return flipHelper<rgbPixel>::apply(srcdest,getParameters().direction);
  }

  // On copy apply for type image!
  bool flipImage::apply(const matrix<rgbPixel>& src,
                        matrix<rgbPixel>& dest) const {
    return flipHelper<rgbPixel>::apply(src,dest,getParameters().direction);
  }

  // On place apply for type image!
  bool flipImage::apply(matrix<float>& srcdest) const {
    return flipHelper<float>::apply(srcdest,getParameters().direction);
  }

  // On copy apply for type image!
  bool flipImage::apply(const matrix<float>& src,
                        matrix<float>& dest) const {
    return flipHelper<float>::apply(src,dest,getParameters().direction);
  }

  // On place apply for type image!
  bool flipImage::apply(matrix<ubyte>& srcdest) const {
    return flipHelper<ubyte>::apply(srcdest,getParameters().direction);
  }

  // On copy apply for type image!
  bool flipImage::apply(const matrix<ubyte>& src,
                        matrix<ubyte>& dest) const {
    return flipHelper<ubyte>::apply(src,dest,getParameters().direction);
  }


}
