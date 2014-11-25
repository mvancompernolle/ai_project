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
 * file .......: ltiDataCodec.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 5.11.2002
 * revisions ..: $Id: ltiDataCodec.cpp,v 1.8 2006/09/05 10:34:53 ltilib Exp $
 */

//TODO: include files
#include "ltiDataCodec.h"

namespace lti {
  // --------------------------------------------------
  // dataCodec::parameters
  // --------------------------------------------------

  // default constructor
  dataCodec::parameters::parameters()
    : functor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    direction=encode;

  }

  // copy constructor
  dataCodec::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  dataCodec::parameters::~parameters() {
  }

  // get type name
  const char* dataCodec::parameters::getTypeName() const {
    return "dataCodec::parameters";
  }

  // copy member

  dataCodec::parameters&
    dataCodec::parameters::copy(const parameters& other) {
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

    direction=other.direction;

    return *this;
  }

  // alias for copy member
  dataCodec::parameters&
    dataCodec::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* dataCodec::parameters::clone() const {
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
  bool dataCodec::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool dataCodec::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      std::string tmp;
      switch (direction) {
        case encode: tmp="encode"; break;
        case decode: tmp="decode"; break;
        default:
          tmp="unknown"; break;
      }

      b=b && lti::write(handler, "direction", tmp);

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
  bool dataCodec::parameters::write(ioHandler& handler,
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
  bool dataCodec::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool dataCodec::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      std::string dir;
      b=b && lti::read(handler, "direction", dir);
      if (dir == "encode") {
        direction=encode;
      } else if (dir == "decode") {
        direction=decode;
      }

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
  bool dataCodec::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // dataCodec
  // --------------------------------------------------

  // default constructor
  dataCodec::dataCodec()
    : dataTransformer(){

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
  dataCodec::dataCodec(const parameters& par)
    : dataTransformer() {

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  dataCodec::dataCodec(const dataCodec& other)
    : dataTransformer()  {
    copy(other);
  }

  // destructor
  dataCodec::~dataCodec() {
  }

  // returns the name of this type
  const char* dataCodec::getTypeName() const {
    return "dataCodec";
  }

  // copy member
  dataCodec& dataCodec::copy(const dataCodec& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  dataCodec& dataCodec::operator=(const dataCodec& other) {
    return (copy(other));
  }

  // return parameters
  const dataCodec::parameters&
    dataCodec::getParameters() const {
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

  /**
   * Encodes data in place.
   */
  bool dataCodec::encode(buffer& srcdest) const {
    bool result=false;
    int n=srcdest.size();
    int dsize=estimateEncodedSize(n);
    buffer dest(dsize);
    int tmp;
    do {
      tmp=dsize;
      result=encodeImplementation(srcdest,dest,n,tmp);
      if (tmp == notEnoughSpace) {
        dsize=getNewEncodingBufferSize(dsize);
        dest.resize(dsize);
      }
    } while (tmp == notEnoughSpace);
    srcdest.fill(dest,0,tmp);
    srcdest.resize(tmp);
    return result;
  }


  /**
   * Encodes data on copy.
   */
  bool dataCodec::encode(const buffer& src, buffer& dest) const {
    int dsize=dest.size();
    bool result=false;
    int tmp;
    do {
      tmp=dsize;
      result=encodeImplementation(src,dest,src.size(),tmp);
      if (tmp == notEnoughSpace) {
        dsize=getNewEncodingBufferSize(dest.size());
        dest.resize(dsize);
      }
    } while (tmp == notEnoughSpace);
    dest.resize(tmp);
    return result;
  }


  /*
   * Decodes data in place.
   */
  bool dataCodec::decode(buffer& srcdest) const {
    bool result=false;
    int n=srcdest.size();
    int dsize=estimateDecodedSize(n);
    buffer dest(dsize);
    int tmp;
    do {
      tmp=dsize;
      result=decodeImplementation(srcdest,dest,n,tmp);
      if (tmp == notEnoughSpace) {
        dsize=getNewDecodingBufferSize(dsize);
        dest.resize(dsize);
      }
    } while (tmp == notEnoughSpace);
    srcdest.fill(dest,0,tmp);
    srcdest.resize(tmp);
    return result;
  }


  /*
   * Decodes data on copy.
   */
  bool dataCodec::decode(const buffer& src, buffer& dest) const {
    int dsize=dest.size();
    bool result=false;
    int tmp;
    do {
      tmp=dsize;
      result=decodeImplementation(src,dest,src.size(),tmp);
      if (tmp == notEnoughSpace) {
        dsize=getNewDecodingBufferSize(dest.size());
        dest.resize(dsize);
      }
    } while (tmp == notEnoughSpace);
    dest.resize(tmp);
    return result;
  }


  /*
   * Encodes data in place.
   */
  bool dataCodec::encode(bufferElement* srcdest, int nsrc, int& ndest) const {
    buffer tmp(ndest);
    buffer src(nsrc,srcdest,true);
    bool result=false;
    result=encodeImplementation(src,tmp,nsrc,ndest);
    if (ndest == notEnoughSpace) {
      result=false;
    } else {
      memcpy(srcdest,&tmp.at(0),ndest);
    }
    return result;
  }


  /**
   * Encodes data on copy.
   */
  bool dataCodec::encode(const bufferElement* src, int nsrc,
                         bufferElement* dest, int& ndest) const {
    // the next line is ugly, but it is the only way to create a
    // wrapper for the const buffer
    const buffer tsrc(nsrc,const_cast<bufferElement*>(src),true);
    buffer tdest(ndest,dest,true);
    return encodeImplementation(tsrc,tdest,nsrc,ndest);
  }


  /**
   * Decodes data in place.
   */
  bool dataCodec::decode(bufferElement* srcdest, int nsrc, int& ndest) const {
    buffer tmp(ndest);
    buffer src(nsrc,srcdest,true);
    bool result=false;
    result=decodeImplementation(src,tmp,nsrc,ndest);
    if (ndest == notEnoughSpace) {
      result=false;
    } else {
      memcpy(srcdest,&tmp.at(0),ndest);
    }
    return result;
  }



  /**
   * Decodes data on copy.
   */
  bool dataCodec::decode(const bufferElement* src, int nsrc,
                         bufferElement* dest, int& ndest) const {
     // the next line is ugly, but it is the only way to create a
    // wrapper for the const buffer
   const buffer tsrc(nsrc,const_cast<bufferElement*>(src),true);
    buffer tdest(ndest,dest,true);
    return decodeImplementation(tsrc,tdest,nsrc,ndest);
  }


  int dataCodec::estimateEncodedSize(int originalSize) const {
    return originalSize;
  }

  /*
   * Computes the default buffer size for decoded data. The
   * source of the encoded data buffer is given as argument.
   * The default is to return the given value.
   */
  int dataCodec::estimateDecodedSize(int encodedSize) const {
    return encodedSize;
  }

    /*
     * Computes a new reasonable buffer size for encoded data
     * if the given buffer size is too small. The default is
     * 2*orginialSize;
     */
  int dataCodec::getNewEncodingBufferSize(int originalSize) const {
    return 2*originalSize;
  }

    /*
     * Computes a new reasonable buffer size for encoded data
     * if the given buffer size is too small. The default is
     * 2*orginialSize;
     */
  int dataCodec::getNewDecodingBufferSize(int encodedSize) const {
    return 2*encodedSize;
  }

  /*
   * Operates on the given arguments. If the data is encoded or
   * decoded depends on the coding direction parameter.
   */
  bool dataCodec::apply(bufferElement* srcdest, int nsrc, int& ndest) const {
    return
      getParameters().direction == parameters::encode ?
      encode(srcdest, nsrc, ndest) :
      decode(srcdest, nsrc, ndest);
  }

  /*
   * Operates on the given argument. If the data is encoded or
   * decoded depends on the coding direction parameter.
   * @param srcdest vector with the input data which will
   *        also receive the transformed data.
   * @return true if apply successful or false otherwise.
   */
  bool dataCodec::apply(buffer& srcdest) const {
    return
      getParameters().direction == parameters::encode ?
      encode(srcdest) :
      decode(srcdest);
  }

  /*
   * Operates on a copy of the given arguments.
   */
  bool dataCodec::apply(const bufferElement* src, int nsrc, bufferElement* dest, int& ndest) const {
    return
      getParameters().direction == parameters::encode ?
      encode(src,nsrc,dest,ndest) :
      decode(src,nsrc,dest,ndest);
  }

  /**
   * Operates on the given argument.
   * @param srcdest vector with the input data which will
   *        also receive the transformed data.
   * @return true if apply successful or false otherwise.
   */
  bool dataCodec::apply(const buffer& src, buffer& dest) const {
    return
      getParameters().direction == parameters::encode ?
      encode(src,dest) :
      decode(src,dest);
  }


}
