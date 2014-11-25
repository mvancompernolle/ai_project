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
 * file .......: ltiMaskImage.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 6.8.2003
 * revisions ..: $Id: ltiMaskImage.cpp,v 1.9 2006/09/05 10:21:34 ltilib Exp $
 */

#include "ltiMaskImage.h"

namespace lti {
  // --------------------------------------------------
  // maskImage::parameters
  // --------------------------------------------------

  // default constructor
  maskImage::parameters::parameters()
    : functor::parameters() {
    
    assumeLabeledMask = bool(false);
    backgroundLabel = int(0);
    colors.resize(8,rgbPixel(),false,false);
    colors.at(0).set(0,0,0,0);
    colors.at(1).set(255,255,255,255);
    colors.at(2).set(255,128,128,128);
    colors.at(3).set(128,255,128,128);
    colors.at(4).set(128,128,255,128);
    colors.at(5).set(255,255,0,128);
    colors.at(6).set(255,0,255,128);
    colors.at(7).set(0,255,255,128);

    borderColors.clear();
    enhanceRegionBorders = bool(false);

  }

  // copy constructor
  maskImage::parameters::parameters(const parameters& other)
    : functor::parameters() {
    copy(other);
  }

  // destructor
  maskImage::parameters::~parameters() {
  }

  // get type name
  const char* maskImage::parameters::getTypeName() const {
    return "maskImage::parameters";
  }

  // copy member

  maskImage::parameters&
    maskImage::parameters::copy(const parameters& other) {
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
    
    assumeLabeledMask = other.assumeLabeledMask;
    backgroundLabel = other.backgroundLabel;
    colors.copy(other.colors);
    enhanceRegionBorders = other.enhanceRegionBorders;
    borderColors = other.borderColors;
    return *this;
  }

  // alias for copy member
  maskImage::parameters&
    maskImage::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* maskImage::parameters::clone() const {
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
  bool maskImage::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool maskImage::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {     
      lti::write(handler,"assumeLabeledMask",assumeLabeledMask);
      lti::write(handler,"backgroundLabel",backgroundLabel);
      lti::write(handler,"colors",colors);
      lti::write(handler,"enhanceRegionBorders",enhanceRegionBorders);
      lti::write(handler,"borderColors",borderColors);
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
  bool maskImage::parameters::write(ioHandler& handler,
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
  bool maskImage::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool maskImage::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"assumeLabeledMask",assumeLabeledMask);
      lti::read(handler,"backgroundLabel",backgroundLabel);
      lti::read(handler,"colors",colors);
      lti::read(handler,"enhanceRegionBorders",enhanceRegionBorders);
      lti::read(handler,"borderColors",borderColors);      
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
  bool maskImage::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // maskImage
  // --------------------------------------------------

  // default constructor
  maskImage::maskImage()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  maskImage::maskImage(const parameters& par)
    : functor() {

    // set the given parameters
    setParameters(par);
  }

  // default constructor
  maskImage::maskImage(const bool assumeLabeledMask,
                       const bool enhanceRegionBorders)
    : functor() {
    
    parameters par;

    par.assumeLabeledMask = assumeLabeledMask;
    par.enhanceRegionBorders = enhanceRegionBorders;

    setParameters(par);
  }

  // copy constructor
  maskImage::maskImage(const maskImage& other)
    : functor() {
    copy(other);
  }

  // destructor
  maskImage::~maskImage() {
  }

  // returns the name of this type
  const char* maskImage::getTypeName() const {
    return "maskImage";
  }

  // copy member
  maskImage&
  maskImage::copy(const maskImage& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  maskImage&
    maskImage::operator=(const maskImage& other) {
    return (copy(other));
  }


  // clone member
  functor* maskImage::clone() const {
    return new maskImage(*this);
  }

  // return parameters
  const maskImage::parameters&
    maskImage::getParameters() const {
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
  
  // On place apply for type image!
  bool maskImage::apply(const image& src,
                        const channel8& mask,
                        image& dest) const {

    const parameters& par = getParameters();
    return apply(src,mask,par.colors,dest);
  };

  // On copy apply for type image!
  bool maskImage::apply(const image& src,
                        const imatrix& mask,
                        image& dest) const {

    const parameters& par = getParameters();
    return apply(src,mask,par.colors,dest);
  };

  inline void maskImage::combine(const rgbPixel& src,
                          const rgbPixel& mask,
                          rgbPixel& dest) const {
    int alpha = mask.getDummy();
    dest.set(static_cast<ubyte>(((int(src.getRed())-mask.getRed())*alpha)
                                /255 + mask.getRed()),
             static_cast<ubyte>(((int(src.getGreen())-mask.getGreen())*alpha)
                                /255 + mask.getGreen()),
             static_cast<ubyte>(((int(src.getBlue())-mask.getBlue())*alpha)
                                /255 + mask.getBlue()));
  }

  inline int maskImage::fixIdx(const int idx,const int palLastIdx) const {
    if (idx>palLastIdx) {
      return ((idx%(palLastIdx))+1);
    }
    return idx;
  }

  bool maskImage::apply(const image& src,
                        const channel8& mask,
                        const palette& colors,
                        image& dest) const {

    const parameters& par = getParameters();
    
    if (par.assumeLabeledMask) {
      return labeled(src,mask,colors,dest);
    } else {
      return unlabeled(src,mask,colors,dest);
    }
    
  };

  // On copy apply for type image!
  bool maskImage::apply(const image& src,
                        const imatrix& mask,
                        const palette& colors,
                        image& dest) const {

    const parameters& par = getParameters();

    if (par.assumeLabeledMask) {
      return labeled(src,mask,colors,dest);
    } else {
      return unlabeled(src,mask,colors,dest);
    }
  };

  bool maskImage::labeled(const image& src,
                          const channel8& mask,
                          const palette& colors,
                          image& dest) const {

    if (src.size() != mask.size()) {
      setStatusString("Image and mask have different sizes.");
      return false;
    }

    const parameters& par = getParameters();
    dest.resize(src.size(),rgbPixel(),false,false);
    int y,x,i;
    for (y=0;y<src.rows();++y) {
      for (x=0;x<src.columns();++x) {
        i=fixIdx(mask.at(y,x),colors.lastIdx());
        combine(src.at(y,x),colors.at(i),dest.at(y,x));
      }
    }
    
    // enhance borders
    if (par.enhanceRegionBorders) {
      if (par.borderColors.empty()) {
        for (y=0;y<src.lastRow();++y) {
          for (x=0;x<src.lastColumn();++x) {
            if (mask.at(y,x) != mask.at(y,x+1)) {
              dest.at(y,x)   = colors.at(fixIdx(mask.at(y,x),
                                                colors.lastIdx()));
              dest.at(y,x+1) = colors.at(fixIdx(mask.at(y,x+1),
                                                colors.lastIdx()));
            }
            if (mask.at(y,x) != mask.at(y+1,x)) {
              dest.at(y,x)   = colors.at(fixIdx(mask.at(y,x),
                                                colors.lastIdx()));
              dest.at(y+1,x) = colors.at(fixIdx(mask.at(y+1,x),
                                                colors.lastIdx()));
            }
          }
        }      
      } else {
        for (y=0;y<src.lastRow();++y) {
          for (x=0;x<src.lastColumn();++x) {
            if (mask.at(y,x) != mask.at(y,x+1)) {
              i=fixIdx(mask.at(y,x),par.borderColors.lastIdx());
              combine(src.at(y,x),par.borderColors.at(i),dest.at(y,x));
              i=fixIdx(mask.at(y,x+1),par.borderColors.lastIdx());
              combine(src.at(y,x+1),par.borderColors.at(i),dest.at(y,x+1));
            }
            if (mask.at(y,x) != mask.at(y+1,x)) {
              i=fixIdx(mask.at(y,x),par.borderColors.lastIdx());
              combine(src.at(y,x),par.borderColors.at(i),dest.at(y,x));

              i=fixIdx(mask.at(y+1,x),par.borderColors.lastIdx());
              combine(src.at(y+1,x),par.borderColors.at(i),dest.at(y+1,x));
            }
          }
        }    
      }
    }

    return true;
  };

  // On copy apply for type image!
  bool maskImage::labeled(const image& src,
                          const imatrix& mask,
                          const palette& colors,
                          image& dest) const {
    
    if (src.size() != mask.size()) {
      setStatusString("Image and mask have different sizes.");
      return false;
    }

    const parameters& par = getParameters();
    dest.resize(src.size(),rgbPixel(),false,false);
    int y,x,i;
    for (y=0;y<src.rows();++y) {
      for (x=0;x<src.columns();++x) {
        i=fixIdx(mask.at(y,x),colors.lastIdx());
        combine(src.at(y,x),colors.at(i),dest.at(y,x));
      }
    }
    
    // enhance borders
    if (par.enhanceRegionBorders) {
      if (par.borderColors.empty()) {
        for (y=0;y<src.lastRow();++y) {
          for (x=0;x<src.lastColumn();++x) {
            if (mask.at(y,x) != mask.at(y,x+1)) {
              dest.at(y,x)  = colors.at(fixIdx(mask.at(y,x),
                                               colors.lastIdx()));
              dest.at(y,x+1)= colors.at(fixIdx(mask.at(y,x+1),
                                               colors.lastIdx()));
            }
            if (mask.at(y,x) != mask.at(y+1,x)) {
              dest.at(y,x)  = colors.at(fixIdx(mask.at(y,x),
                                               colors.lastIdx()));
              dest.at(y+1,x)= colors.at(fixIdx(mask.at(y+1,x),
                                               colors.lastIdx()));
            }
          }  
        }
      } else {
        for (y=0;y<src.lastRow();++y) {
          for (x=0;x<src.lastColumn();++x) {
            if (mask.at(y,x) != mask.at(y,x+1)) {
              i=fixIdx(mask.at(y,x),par.borderColors.lastIdx());
              combine(src.at(y,x),par.borderColors.at(i),dest.at(y,x));
              i=fixIdx(mask.at(y,x+1),par.borderColors.lastIdx());
              combine(src.at(y,x+1),par.borderColors.at(i),dest.at(y,x+1));
            }
            if (mask.at(y,x) != mask.at(y+1,x)) {
              i=fixIdx(mask.at(y,x),par.borderColors.lastIdx());
              combine(src.at(y,x),par.borderColors.at(i),dest.at(y,x));

              i=fixIdx(mask.at(y+1,x),par.borderColors.lastIdx());
              combine(src.at(y+1,x),par.borderColors.at(i),dest.at(y+1,x));
            }
          }
        }  
      }
    }

    return true;
  };


  bool maskImage::unlabeled(const image& src,
                            const channel8& mask,
                            const palette& colors,
                            image& dest) const {

    if (src.size() != mask.size()) {
      setStatusString("Image and mask have different sizes.");
      return false;
    }

    const parameters& par = getParameters();
    dest.resize(src.size(),rgbPixel(),false,false);
    int y,x,i;
    for (y=0;y<src.rows();++y) {
      for (x=0;x<src.columns();++x) {
        i=(mask.at(y,x) != par.backgroundLabel) ? 1 : 0;
        combine(src.at(y,x),colors.at(i),dest.at(y,x));
      }
    }
    
    return true;
  }

  // On copy apply for type image!
  bool maskImage::unlabeled(const image& src,
                            const imatrix& mask,
                            const palette& colors,
                            image& dest) const {

    if (src.size() != mask.size()) {
      setStatusString("Image and mask have different sizes.");
      return false;
    }

    const parameters& par = getParameters();
    dest.resize(src.size(),rgbPixel(),false,false);
    int y,x,i;
    for (y=0;y<src.rows();++y) {
      for (x=0;x<src.columns();++x) {
        i=(mask.at(y,x) != par.backgroundLabel) ? 1 : 0;
        combine(src.at(y,x),colors.at(i),dest.at(y,x));
      }
    }
    

    // enhance borders
    if (par.enhanceRegionBorders) {
      int m1,m2;
      if (par.borderColors.empty()) {
        for (y=0;y<src.lastRow();++y) {
          for (x=0;x<src.lastColumn();++x) {
            m1 = (mask.at(y,x) != par.backgroundLabel) ? 1 : 0;
            m2 = (mask.at(y,x+1) != par.backgroundLabel) ? 1 : 0;
            if (m1 != m2) {
              dest.at(y,x)   = colors.at(fixIdx(m1,colors.lastIdx()));
              dest.at(y,x+1) = colors.at(fixIdx(m2,colors.lastIdx()));
            }
            m2 = (mask.at(y+1,x) != par.backgroundLabel) ? 1 : 0;
            if (mask.at(y,x) != m2) {
              dest.at(y,x)   = colors.at(fixIdx(m1,colors.lastIdx()));
              dest.at(y+1,x) = colors.at(fixIdx(m2,colors.lastIdx()));
            }
          }
        }      
      } else {
        for (y=0;y<src.lastRow();++y) {
          for (x=0;x<src.lastColumn();++x) {
            m1 = (mask.at(y,x) != par.backgroundLabel) ? 1 : 0;
            m2 = (mask.at(y,x+1) != par.backgroundLabel) ? 1 : 0;

            if (m1 != m2) {
              i=fixIdx(m1,par.borderColors.lastIdx());
              combine(src.at(y,x),par.borderColors.at(i),dest.at(y,x));
              i=fixIdx(m2,par.borderColors.lastIdx());
              combine(src.at(y,x+1),par.borderColors.at(i),dest.at(y,x+1));
            }

            m2 = (mask.at(y+1,x) != par.backgroundLabel) ? 1 : 0;
            if (m1 != m2) {
              i=fixIdx(m1,par.borderColors.lastIdx());
              combine(src.at(y,x),par.borderColors.at(i),dest.at(y,x));

              i=fixIdx(m2,par.borderColors.lastIdx());
              combine(src.at(y+1,x),par.borderColors.at(i),dest.at(y+1,x));
            }
          }
        }
      }
    }

    return true;
  };


}
