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
 * file .......: ltiIdentityCodec.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 5.11.2002
 * revisions ..: $Id: ltiIdentityCodec.cpp,v 1.3 2006/02/08 12:04:35 ltilib Exp $
 */

//TODO: include files
#include "ltiIdentityCodec.h"
#include <cctype>
#include <cstdio>

namespace lti {

  // --------------------------------------------------
  // identityCodec
  // --------------------------------------------------

  // default constructor
  identityCodec::identityCodec()
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
  identityCodec::identityCodec(const parameters& par)
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
  identityCodec::identityCodec(const identityCodec& other)
    : dataCodec() {
    copy(other);
  }

  // destructor
  identityCodec::~identityCodec() {
  }

  // returns the name of this type
  const char* identityCodec::getTypeName() const {
    return "identityCodec";
  }

  // copy member
  identityCodec&
    identityCodec::copy(const identityCodec& other) {
      dataCodec::copy(other);

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  identityCodec&
    identityCodec::operator=(const identityCodec& other) {
    return (copy(other));
  }


  // clone member
  functor* identityCodec::clone() const {
    return new identityCodec(*this);
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool identityCodec::encodeImplementation(const buffer& src, buffer& dest,
                                           int nsrc, int& ndest) const {

    assert(src.size() >= nsrc && dest.size() >= ndest);

    if (nsrc > ndest) {
      ndest=notEnoughSpace;
      setStatusString(notEnoughSpaceMsg);
      return false;
    }
    // Ok, we have enough space
    ndest=0;
    buffer::const_iterator si=src.begin();
    buffer::iterator di=dest.begin();

    for (int i=0; i<nsrc; i++) {
      *di++=*si++;
      ndest++;
    }
    return true;
  }


  bool identityCodec::decodeImplementation(const buffer& src, buffer& dest,
                                           int nsrc, int& ndest) const {

    assert(src.size() >= nsrc && dest.size() >= ndest);

    if (nsrc > ndest) {
      ndest=notEnoughSpace;
      setStatusString(notEnoughSpaceMsg);
      return false;
    }
    
    // Ok, we have enough space
    ndest=0;
    buffer::const_iterator si=src.begin();
    buffer::iterator di=dest.begin();
    for (int i=0; i<nsrc; i++) {
      *di++=*si++;
      ndest++;
    }
    return true;
  }

}
