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
 * file .......: ltiCombinedCodec.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 5.11.2002
 * revisions ..: $Id: ltiCombinedCodec.cpp,v 1.3 2006/02/08 12:01:48 ltilib Exp $
 */

//TODO: include files
#include "ltiCombinedCodec.h"
#include <cctype>
#include <cstdio>

namespace lti {

  // --------------------------------------------------
  // combinedCodec
  // --------------------------------------------------

  // default constructor
  combinedCodec::combinedCodec(const dataCodec& c1, const dataCodec& c2)
    : dataCodec() {

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

    codec1=dynamic_cast<dataCodec*>(c1.clone());
    codec2=dynamic_cast<dataCodec*>(c2.clone());
  }

  // default constructor
  combinedCodec::combinedCodec(const parameters& par)
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
  combinedCodec::combinedCodec(const combinedCodec& other) : dataCodec() {
    copy(other);
  }

  // destructor
  combinedCodec::~combinedCodec() {
    delete codec1;
    delete codec2;
  }

  // returns the name of this type
  const char* combinedCodec::getTypeName() const {
    return "combinedCodec";
  }

  // copy member
  combinedCodec&
    combinedCodec::copy(const combinedCodec& other) {
      dataCodec::copy(other);

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  combinedCodec&
    combinedCodec::operator=(const combinedCodec& other) {
    return (copy(other));
  }


  // clone member
  functor* combinedCodec::clone() const {
    return new combinedCodec(*this);
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool combinedCodec::encodeImplementation(const buffer& src, buffer& dest,
                                           int nsrc, int& ndest) const {

    assert(src.size() >= nsrc && dest.size() >= ndest);

    int ntmp=codec1->estimateEncodedSize(nsrc);
    buffer tmp(ntmp);
    bool result;

    result=codec1->encode(&src.at(0),nsrc,&tmp.at(0),ntmp);
    assert(ntmp != notEnoughSpace);

    result=result &&
      codec2->encode(&tmp.at(0),ntmp,&dest.at(0),ndest);
    return result;
  }

  bool combinedCodec::decodeImplementation(const buffer& src, buffer& dest,
                                           int nsrc, int& ndest) const {

    assert(src.size() >= nsrc && dest.size() >= ndest);

    int ntmp=codec2->estimateDecodedSize(nsrc);
    buffer tmp(ntmp);
    bool result;

    result=codec2->decode(&src.at(0),nsrc,&tmp.at(0),ntmp);
    assert(ntmp != notEnoughSpace);

    result=result && codec1->decode(&tmp.at(0),ntmp,&dest.at(0),ndest);
    return result;
  }



  int combinedCodec::estimateEncodedSize(int old) const {
    int i1=codec1->estimateEncodedSize(old);
    return codec2->estimateEncodedSize(i1);
  }

  int combinedCodec::estimateDecodedSize(int old) const {
    int i1=codec2->estimateDecodedSize(old);
    return codec1->estimateDecodedSize(i1);
  }


}
