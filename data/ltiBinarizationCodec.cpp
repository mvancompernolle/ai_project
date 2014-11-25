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
 * file .......: ltiBinarizationCodec.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 5.11.2002
 * revisions ..: $Id: ltiBinarizationCodec.cpp,v 1.8 2008/10/02 15:34:26 alvarado Exp $
 */

//TODO: include files
#include "ltiBinarizationCodec.h"
#include <cctype>
#include <cstdio>


namespace lti {

  // --------------------------------------------------
  // binarizationCodec::parameters
  // --------------------------------------------------

  // default constructor
  binarizationCodec::parameters::parameters()
    : dataCodec::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    maxValue=std::numeric_limits<bufferElement>::max();
    threshold=maxValue/2+1;
  }

  // copy constructor
  binarizationCodec::parameters::parameters(const parameters& other)
    : dataCodec::parameters()  {
    copy(other);
  }

  // destructor
  binarizationCodec::parameters::~parameters() {
  }

  // get type name
  const char* binarizationCodec::parameters::getTypeName() const {
    return "binarizationCodec::parameters";
  }

  // copy member

  binarizationCodec::parameters&
  binarizationCodec::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    dataCodec::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    dataCodec::parameters& (dataCodec::parameters::* p_copy)
      (const dataCodec::parameters&) =
      dataCodec::parameters::copy;
    (this->*p_copy)(other);
# endif

    threshold=other.threshold;
    maxValue=other.maxValue;

    return *this;
  }

  // alias for copy member
  binarizationCodec::parameters&
  binarizationCodec::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* binarizationCodec::parameters::clone() const {
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
  bool binarizationCodec::parameters::write(ioHandler& handler,
                                            const bool complete) const
# else
    bool binarizationCodec::parameters::writeMS(ioHandler& handler,
                                                const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      b=b && lti::write(handler, "threshold", threshold);
      b=b && lti::write(handler, "maxValue", maxValue);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && dataCodec::parameters::write(handler,false);
# else
    bool (dataCodec::parameters::* p_writeMS)(ioHandler&,const bool) const =
      dataCodec::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool binarizationCodec::parameters::write(ioHandler& handler,
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
  bool binarizationCodec::parameters::read(ioHandler& handler,
                                           const bool complete)
# else
    bool binarizationCodec::parameters::readMS(ioHandler& handler,
                                               const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      b=b && lti::read(handler, "threshold", threshold);
      b=b && lti::read(handler, "maxValue", maxValue);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && dataCodec::parameters::read(handler,false);
# else
    bool (dataCodec::parameters::* p_readMS)(ioHandler&,const bool) =
      dataCodec::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool binarizationCodec::parameters::read(ioHandler& handler,
                                           const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // binarizationCodec
  // --------------------------------------------------

  // default constructor
  binarizationCodec::binarizationCodec()
    : dataCodec(){

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
  binarizationCodec::binarizationCodec(const parameters& par)
    : dataCodec() {

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  binarizationCodec::binarizationCodec(const binarizationCodec& other)
    : dataCodec() {
    copy(other);
  }

  // destructor
  binarizationCodec::~binarizationCodec() {
  }

  // returns the name of this type
  const char* binarizationCodec::getTypeName() const {
    return "binarizationCodec";
  }

  // copy member
  binarizationCodec&
  binarizationCodec::copy(const binarizationCodec& other) {
    dataCodec::copy(other);

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  binarizationCodec&
  binarizationCodec::operator=(const binarizationCodec& other) {
    return (copy(other));
  }


  // clone member
  functor* binarizationCodec::clone() const {
    return new binarizationCodec(*this);
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool binarizationCodec::encodeImplementation(const buffer& src, buffer& dest,
                                               int nsrc, int& ndest) const {

    assert((src.size() >= nsrc) && (dest.size() >= ndest));

    int count=0;


    buffer::const_iterator si=src.begin();
    buffer::iterator di=dest.begin();

    int i=0;

    bufferElement threshold=getParameters().threshold;
    *di=0;
    while ((i < nsrc) && (count < ndest)) {
      *di=(*di << 1) | (*si++ >= threshold ? 1 : 0);
      i++;
      if (i%(sizeof(bufferElement)*8) == 0) {
        di++;
        count++;
        *di=0;
      }
    }
    if ((count >= ndest) && (i < nsrc)) {
      ndest=-1;
      setStatusString(notEnoughSpaceMsg);
      return false;
    }
    ndest=count;
    return true;
  }

  bool binarizationCodec::decodeImplementation(const buffer& src, buffer& dest,
                                               int nsrc, int& ndest) const {

    assert((src.size() >= nsrc) && (dest.size() >= ndest));

    int count=0;

		
    static int* threshs;
    threshs=new int[sizeof(bufferElement)*8];

    buffer::const_iterator si=src.begin();
    buffer::iterator di=dest.begin();

    // init threshold array
    for (unsigned int b=0; b<sizeof(bufferElement)*8; b++) {
      threshs[b]=1<<(sizeof(bufferElement)*8-b-1);
    }
    bufferElement maxValue=getParameters().maxValue;
    int i=0;

    while ((i < nsrc) && (count < ndest)) {
      int tmp=*si;
      for (unsigned int j=0; j<sizeof(bufferElement)*8; i++) {
        if ((tmp & threshs[j]) != 0) {
          *di++=maxValue;
        } else {
          *di++=0;
        }
        count++;
      }
      si++;
      i++;
    }
    delete[] threshs;

    if ((count >= ndest) && (i < nsrc)) {
      ndest=-1;
      setStatusString(notEnoughSpaceMsg);
      return false;
    }
    ndest=count;
    return true;
  }


  int binarizationCodec::estimateEncodedSize(int old) const {
    // this is the absolute worst case
    return old/8+1;
  }


  int binarizationCodec::estimateDecodedSize(int old) const {
    return old*8;
  }

  // return parameters
  const binarizationCodec::parameters&
  binarizationCodec::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&dataCodec::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }



}
