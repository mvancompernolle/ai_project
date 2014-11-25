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
 * file .......: ltiHexCodec.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 5.11.2002
 * revisions ..: $Id: ltiAsciiHexCodec.cpp,v 1.4 2006/02/08 12:00:37 ltilib Exp $
 */

//TODO: include files
#include "ltiAsciiHexCodec.h"
#include <cctype>
#include <cstdio>

namespace lti {

  // --------------------------------------------------
  // asciiHexCodec
  // --------------------------------------------------

  const ubyte asciiHexCodec::digits[16]= {
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'a',
    'b',
    'c',
    'd',
    'e',
    'f'
  };


  // default constructor
  asciiHexCodec::asciiHexCodec()
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
  asciiHexCodec::asciiHexCodec(const parameters& par)
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
  asciiHexCodec::asciiHexCodec(const asciiHexCodec& other)
    : dataCodec() {
    copy(other);
  }

  // destructor
  asciiHexCodec::~asciiHexCodec() {
  }

  // returns the name of this type
  const char* asciiHexCodec::getTypeName() const {
    return "asciiHexCodec";
  }

  // copy member
  asciiHexCodec&
    asciiHexCodec::copy(const asciiHexCodec& other) {
      dataCodec::copy(other);

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  asciiHexCodec&
    asciiHexCodec::operator=(const asciiHexCodec& other) {
    return (copy(other));
  }


  // clone member
  functor* asciiHexCodec::clone() const {
    return new asciiHexCodec(*this);
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool asciiHexCodec::encodeImplementation(const buffer& src, buffer& dest,
                                           int nsrc, int& ndest) const {

    assert(src.size() >= nsrc && dest.size() >= ndest);

    if (nsrc*2 > ndest) {
      ndest=notEnoughSpace;
      setStatusString(notEnoughSpaceMsg);
      return false;
    }
    // Ok, we have enough space
    ndest=0;
    buffer::const_iterator si=src.begin();
    buffer::iterator di=dest.begin();

    for (int i=0; i<nsrc; i++,si++,ndest+=2) {
      ubyte highnibble=*si>>4;
      ubyte lownibble=*si&0x0f;

      *di++=digits[highnibble];
      *di++=digits[lownibble];
    }
    return true;
  }


  bool asciiHexCodec::decodeImplementation(const buffer& src, buffer& dest,
                                           int nsrc, int& ndest) const {

    assert(src.size() >= nsrc && dest.size() >= ndest);

    if (nsrc > ndest*2) {
      ndest=notEnoughSpace;
      setStatusString(notEnoughSpaceMsg);
      return false;
    }
    if (nsrc%2 != 0) {
      setStatusString("Input data must have an even number of bytes");
      return false;
    }
    // Ok, we have enough space
    ndest=0;
    buffer::const_iterator si=src.begin();
    buffer::iterator di=dest.begin();
    for (int i=0; i<nsrc; i++,di++,ndest++) {
      ubyte highnibble=tolower(*si++);
      ubyte lownibble=tolower(*si++);

      if (highnibble > 'f' || highnibble < '0' || lownibble > 'f' || lownibble < '0'
          || (highnibble > '9' && highnibble < 'a') || (lownibble > '9' && lownibble < 'a')) {
        char msg[80];
        sprintf(msg,"Illegal characters in input buffer at position %d",i);
        setStatusString(msg);
        return false;
      }

      highnibble-=(highnibble >= 'a') ? ('a'-10) : '0';
      lownibble-=(lownibble >= 'a') ? ('a'-10) : '0';
      *di=(highnibble<<4)|lownibble;
    }
    return true;
  }


  int asciiHexCodec::estimateEncodedSize(int old) const {
    return old*2;
  }


  int asciiHexCodec::estimateDecodedSize(int old) const {
    return old/2+old%2;
  }


}
