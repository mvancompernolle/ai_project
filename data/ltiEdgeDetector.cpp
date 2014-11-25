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
 * file .......: ltiEdgeDetector.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 30.10.2002
 * revisions ..: $Id: ltiEdgeDetector.cpp,v 1.10 2006/09/05 10:10:08 ltilib Exp $
 */

#include "ltiEdgeDetector.h"
#include "ltiSplitImageToRGB.h"
#include "ltiMergeRGBToImage.h"

namespace lti {
  // --------------------------------------------------
  // edgeDetector::parameters
  // --------------------------------------------------

  // default constructor
  edgeDetector::parameters::parameters()
    : modifier::parameters() {
    noEdgeValue = ubyte(0);
    edgeValue = ubyte(255);
  }

  // copy constructor
  edgeDetector::parameters::parameters(const parameters& other)
    : modifier::parameters()  {
    copy(other);
  }

  // destructor
  edgeDetector::parameters::~parameters() {
  }

  // get type name
  const char* edgeDetector::parameters::getTypeName() const {
    return "edgeDetector::parameters";
  }

  // copy member

  edgeDetector::parameters&
    edgeDetector::parameters::copy(const parameters& other) {
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


    noEdgeValue = other.noEdgeValue;
    edgeValue = other.edgeValue;

    return *this;
  }

  // alias for copy member
  edgeDetector::parameters&
    edgeDetector::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* edgeDetector::parameters::clone() const {
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
  bool edgeDetector::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool edgeDetector::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"noEdgeValue",noEdgeValue);
      lti::write(handler,"edgeValue",edgeValue);
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
  bool edgeDetector::parameters::write(ioHandler& handler,
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
  bool edgeDetector::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool edgeDetector::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"noEdgeValue",noEdgeValue);
      lti::read(handler,"edgeValue",edgeValue);
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
  bool edgeDetector::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // edgeDetector
  // --------------------------------------------------

  // default constructor
  edgeDetector::edgeDetector()
    : modifier(){
  }

  // copy constructor
  edgeDetector::edgeDetector(const edgeDetector& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  edgeDetector::~edgeDetector() {
  }

  // returns the name of this type
  const char* edgeDetector::getTypeName() const {
    return "edgeDetector";
  }

  // copy member
  edgeDetector& edgeDetector::copy(const edgeDetector& other) {
    modifier::copy(other);

    return (*this);
  }

  // alias for copy member
  edgeDetector&
    edgeDetector::operator=(const edgeDetector& other) {
    return (copy(other));
  }

  // return parameters
  const edgeDetector::parameters&
    edgeDetector::getParameters() const {
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


  // On place apply for type channel8!
  bool edgeDetector::apply(channel8& srcdest) const {

    channel8 tmp;
    if (apply(srcdest,tmp)) {
      // can we just use the result as new image?
      if (srcdest.getMode() == channel8::Connected) {
        // yes: just transfer the data
        tmp.detach(srcdest);
      } else {
        // no: srcdest is part of another bigger image, just copy
        // the data.
        srcdest.fill(tmp);
      }
      return true;
    }


    return false;
  };

  // On place apply for type channel!
  bool edgeDetector::apply(channel& srcdest) const {
    channel tmp;
    if (apply(srcdest,tmp)) {
      // can we just use the result as new image?
      if (srcdest.getMode() == channel::Connected) {
        // yes: just transfer the data
        tmp.detach(srcdest);
      } else {
        // no: srcdest is part of another bigger image, just copy
        // the data.
        srcdest.fill(tmp);
      }
      return true;
    }

    return false;
  };

  // On copy apply for type channel8!
  bool edgeDetector::apply(const channel8& src,channel8& dest) const {
    setStatusString("void implementation of cornerDetector::apply called");

    return false;
  };

  // On copy apply for type channel!
  bool edgeDetector::apply(const channel& src,channel& dest) const {
    setStatusString("void implementation of cornerDetector::apply called");

    return false;
  };

  // On copy apply for type channel!
  bool edgeDetector::apply(const channel& src,channel8& dest) const {
    setStatusString("void implementation of cornerDetector::apply called");

    return false;
  };

  // on place apply for images
  bool edgeDetector::apply(image& srcdest) const {
    image tmp;
    if (apply(srcdest,tmp)) {
      // can we just use the result as new image?
      if (srcdest.getMode() == image::Connected) {
        // yes: just transfer the data
        tmp.detach(srcdest);
      } else {
        // no: srcdest is part of another bigger image, just copy
        // the data.
        srcdest.fill(tmp);
      }
      return true;
    }

    return false;
  };

  /*
   * Compute the edges for the red, green and blue components of the image
   * and leave the result in each channel of the destination image.
   */
  bool edgeDetector::apply(const image& src,image& dest) const {
    splitImageToRGB splitter;
    channel8 r,g,b;
    splitter.apply(src,r,g,b);
    if (apply(r) && apply(g) && apply(b)) {
      mergeRGBToImage merger;
      merger.apply(r,g,b,dest);
      return true;
    }
    return false;
  }

  /*
   * Compute the edges for the red, green and blue components of the image
   * and leave the sum of the results in the given channel.
   */
  bool edgeDetector::apply(const image& src,channel8& dest) const {
    channel8 r,g,b;
    splitImageToRGB splitter;
    splitter.apply(src,r,g,b);
    if (apply(r) && apply(g) && apply(b)) {
      dest.add(r,g);
      dest.add(b);
      return true;
    }
    return false;
  }



}
