/*
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006
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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiModifier.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 16.11.99
 * revisions ..: $Id: ltiModifier.cpp,v 1.13 2006/09/05 10:22:32 ltilib Exp $
 */

#include "ltiModifier.h"
#include "ltiSplitImageToRGB.h"
#include "ltiMergeRGBToImage.h"

namespace lti {

  // static deprecated members
  const eBoundaryType modifier::parameters::Black      = lti::Zero;
  const eBoundaryType modifier::parameters::Mirror     = lti::Mirror;
  const eBoundaryType modifier::parameters::Periodic   = lti::Periodic;
  const eBoundaryType modifier::parameters::Constant   = lti::Constant;
  const eBoundaryType modifier::parameters::NoBoundary = lti::NoBoundary;
  // end of deprecated static parameters

  const char* modifier::parameters::getTypeName() const {
    return "modifier::parameters";
  }

  functor::parameters* modifier::parameters::clone() const {
    return (new parameters(*this));
  }

  // modifier parameters
  modifier::parameters& modifier::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters& (functor::parameters::* p_copy)
      (const functor::parameters&) = functor::parameters::copy;
    (this->*p_copy)(other);
#  endif

    boundaryType = other.boundaryType;

    return (*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool modifier::parameters::write(ioHandler& handler,
                                   const bool complete) const
# else
  bool modifier::parameters::writeMS(ioHandler& handler,
                                     const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    b = b && lti::write(handler, "boundaryType", boundaryType);

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
  bool modifier::parameters::write(ioHandler& handler,
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
  bool modifier::parameters::read(ioHandler& handler,
                                  const bool complete)
# else
  bool modifier::parameters::readMS(ioHandler& handler,
                                    const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    b = b && lti::read(handler, "boundaryType", boundaryType);

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
  bool modifier::parameters::read(ioHandler& handler,
                                  const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // returns the current parameters
  const modifier::parameters& modifier::getParameters() const  {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());

    if(params==0) {
      throw invalidParametersException(getTypeName());
    }

    return *params;
  }

  // returns the current parameters (protected non const member!)
  modifier::parameters& modifier::getParameters() {
    parameters* par =
      dynamic_cast<parameters*>(&functor::getParameters());

    if(par==0) {
      throw invalidParametersException(getTypeName());
    }

    return *par;
  }

  // destructor
  modifier::~modifier() {
  }

  // type name
  const char* modifier::getTypeName() const {
    return "modifier";
  }
  
  // apply for float vector
  bool modifier::apply(fvector& srcdest) const {
    throw lti::functor::invalidMethodException();
    return false;
  }

  // apply for integer vector
  bool modifier::apply(ivector& srcdest) const {
    throw lti::functor::invalidMethodException();
    return false;
  }

  // apply for ubyte vector
  bool modifier::apply(vector<ubyte>& srcdest) const {
    throw lti::functor::invalidMethodException();
    return false;
  }

  // apply for channel
  bool modifier::apply(matrix<channel::value_type>& srcdest) const {
    throw lti::functor::invalidMethodException();
    return false;
  }

  // apply for channel
  bool modifier::apply(channel& srcdest) const {
    matrix<channel::value_type>& tmp = srcdest;
    return apply(tmp);
  }

  // apply for matrix of integers
  bool modifier::apply(imatrix& srcdest) const {
    throw lti::functor::invalidMethodException();
    return false;
  }

  // apply for channel of unsigned bytes
  bool modifier::apply(matrix<channel8::value_type>& srcdest) const {
    throw lti::functor::invalidMethodException();
    return false;
  }

  // apply for channel of unsigned bytes
  bool modifier::apply(channel8& srcdest) const {
    matrix<channel8::value_type>& tmp = srcdest;
    return apply(tmp);
  }

  // apply for an image
  bool modifier::apply(image& srcdest) const {
    return apply(srcdest,srcdest);
  }

  // apply (on copy) for a vector of floats
  bool modifier::apply(const fvector& src, fvector& dest) const {
    throw lti::functor::invalidMethodException();
    return false;
  }

  // apply (on copy) for a vector of floats
  bool modifier::apply(const ivector& src, ivector& dest) const {
    throw lti::functor::invalidMethodException();
    return false;
  }

  // apply (on copy) for a vector of floats
  bool modifier::apply(const vector<ubyte>& src,
                                       vector<ubyte>& dest) const {
    throw lti::functor::invalidMethodException();
    return false;
  }

  // apply (on copy) for a vector of floats
  bool modifier::apply(const matrix<channel::value_type>& src, 
                       matrix<channel::value_type>& dest) const {
    throw lti::functor::invalidMethodException();
    return false;
  }

  // apply (on copy) for a vector of floats
  bool modifier::apply(const channel& src, 
                             channel& dest) const {
    const matrix<channel::value_type>& tmp = src;
    return apply(tmp,dest);
  }

  // apply (on copy) for a vector of floats
  bool modifier::apply(const imatrix& src, imatrix& dest) const {
    throw lti::functor::invalidMethodException();
    return false;
  }

  // apply (on copy) for a vector of floats
  bool modifier::apply(const matrix<channel8::value_type>& src, 
                             matrix<channel8::value_type>& dest) const {
    throw lti::functor::invalidMethodException();
    return false;
  }

  // apply (on copy) for a vector of floats
  bool modifier::apply(const channel8& src, 
                             channel8& dest) const {
    const matrix<channel8::value_type>& tmp = src;
    return apply(tmp,dest);
  }

  // apply (on copy) for a vector of floats
  bool modifier::apply(const image& src, image& dest) const {
    splitImageToRGB splitter;
    mergeRGBToImage merger;

    bool result = true;

    channel red,green,blue;
    channel rred,rgreen,rblue;

    if (splitter.apply(src,red,green,blue)) {
      result = this->apply(red,rred);
      result = result && this->apply(green,rgreen);
      result = result && this->apply(blue,rblue);
          
      if (result && merger.apply(rred,rgreen,rblue,dest)) {
        return true;
      } else if (result) {
        setStatusString("Error merging image: ");
        appendStatusString(merger.getStatusString());
      } 
    } else {
      setStatusString("Error splitting image: ");
      appendStatusString(splitter.getStatusString());
    }

    return false;
  }

}
