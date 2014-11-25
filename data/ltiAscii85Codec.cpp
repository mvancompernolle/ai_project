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
 * file .......: ltiAscii85Codec.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 5.11.2002
 * revisions ..: $Id: ltiAscii85Codec.cpp,v 1.3 2006/02/08 11:59:58 ltilib Exp $
 */

//TODO: include files
#include "ltiAscii85Codec.h"
#include <cctype>
#include <cstdio>

namespace lti {

  // --------------------------------------------------
  // ascii85Codec
  // --------------------------------------------------

  const dataCodec::bufferElement ascii85Codec::offset=static_cast<bufferElement>('!');

  // default constructor
  ascii85Codec::ascii85Codec()
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
  ascii85Codec::ascii85Codec(const parameters& par)
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
  ascii85Codec::ascii85Codec(const ascii85Codec& other)
    : dataCodec() {
    copy(other);
  }

  // destructor
  ascii85Codec::~ascii85Codec() {
  }

  // returns the name of this type
  const char* ascii85Codec::getTypeName() const {
    return "ascii85Codec";
  }

  // copy member
  ascii85Codec&
    ascii85Codec::copy(const ascii85Codec& other) {
      dataCodec::copy(other);

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  ascii85Codec&
    ascii85Codec::operator=(const ascii85Codec& other) {
    return (copy(other));
  }


  // clone member
  functor* ascii85Codec::clone() const {
    return new ascii85Codec(*this);
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool ascii85Codec::encodeImplementation(const buffer& src, buffer& dest,
                                          int nsrc, int& ndest) const {

    assert(src.size() >= nsrc && dest.size() >= ndest);

    int rem=nsrc%4;
    if (rem == 0) {
      if (nsrc*5 > ndest*4) {
        ndest=notEnoughSpace;
        setStatusString(notEnoughSpaceMsg);
        return false;
      }
    } else {
      if ((nsrc/4)*5+rem+1 > ndest) {
        ndest=notEnoughSpace;
        setStatusString(notEnoughSpaceMsg);
        return false;
      }
    }
    // Ok, we have enough space
    ndest=0;

    buffer::const_iterator si=src.begin();
    buffer::iterator di=dest.begin();

    for (int i=0; i<nsrc-rem; i+=4) {
      uint32 b1=*si++;
      uint32 b2=*si++;
      uint32 b3=*si++;
      uint32 b4=*si++;

      // build 4-byte number
      uint32 tmp=(b1<<24)|(b2<<16)|(b3<<8)|b4;
      // now build a base-85 representation
      bool z=true;

      buffer::iterator tmpi=di;
      tmpi+=5;
      for (int j=0; j<5; j++) {
        uint32 d=tmp%85;
        *--tmpi=static_cast<bufferElement>(d)+offset;
        tmp/=85;
        z=z && (d == 0);
      }
      // handle the special case if all digits are zero
      if (z) {
        *di='z';
        di++;
        ndest++;
      } else {
        di+=5;
        ndest+=5;
      }
    }
    if (rem != 0) {
      // we have some extra bytes
      int n=rem;
      uint32 tmp=0;
	  int i,j;
      for (i=0; i<n; i++) {
        tmp=tmp<<8;
        tmp|=*si++;
      }
      for (i=n; i<4; i++) {
        tmp=tmp<<8;
      }
      for (j=5; j>n+1; j--) {
        tmp/=85;
      }
      buffer::iterator tmpi=di;
      tmpi+=n+1;
      for (j=n+1; j>0; j--) {
        uint32 d=tmp%85;
        *--tmpi=static_cast<bufferElement>(d)+offset;
        tmp/=85;
      }
      ndest+=n+1;
    }

    return true;
  }


  bool ascii85Codec::decodeImplementation(const buffer& src, buffer& dest,
                                          int nsrc, int& ndest) const {

    assert(src.size() >= nsrc && dest.size() >= ndest);
    int rem=nsrc%5;

    if (rem == 0) {
      if (nsrc*4 > ndest*5) {
        ndest=notEnoughSpace;
        setStatusString(notEnoughSpaceMsg);
        return false;
      }
    } else {
      if ((nsrc/5)*4+rem-1 > ndest) {
        ndest=notEnoughSpace;
        setStatusString(notEnoughSpaceMsg);
        return false;
      }
    }

    buffer::const_iterator si=src.begin();
    buffer::iterator di=dest.begin();

    // Ok, we have enough space
    ndest=0;
    for (int i=0; i<nsrc-rem; ndest+=4) {
      int32 c1=*si++;
      if (c1 == 'z') {
        *di++=0;
        *di++=0;
        *di++=0;
        *di++=0;
        si++;
        i++;
      } else {
        c1-=offset;
        int32 c2=(*si++)-offset;
        int32 c3=(*si++)-offset;
        int32 c4=(*si++)-offset;
        int32 c5=(*si++)-offset;
        if (c1 < 0 || c1 >= 85 || c2 < 0 || c2 >= 85 ||
            c3 < 0 || c3 >= 85 || c3 < 0 || c3 >= 85 ||
            c4 < 0 || c4 >= 85) {
          char msg[80];
          sprintf(msg,"Illegal characters in input buffer at position %d",i);
          setStatusString(msg);
          return false;
        }
        uint32 tmp=85*(85*(85*(85*c1+c2)+c3)+c4)+c5;

        *di++=static_cast<bufferElement>(tmp>>24);
        *di++=static_cast<bufferElement>(tmp>>16&0xff);
        *di++=static_cast<bufferElement>(tmp>>8&0xff);
        *di++=static_cast<bufferElement>(tmp&0xff);
        si+=5;
        i+=5;
      }
    }
    if (rem != 0) {
      int n=rem;
      uint32 tmp=0;
	  int i,j;
      for (i=0; i<n; i++) {
        tmp=tmp*85;
        tmp+=*si++-offset;
      }
      for (i=n; i<5; i++) {
        tmp=tmp*85;
      }
      for (j=4; j>n-1; j--) {
        tmp=tmp>>8;
      }
      buffer::iterator tmpi=di;
      tmpi+=n-1;
      for (j=n-1; j>0; j--) {
        *--tmpi=static_cast<bufferElement>(tmp&0xff);
        tmp=tmp>>8;
      }
      // for some reason, the last char is always one too small
      // therefore, simply add one
      tmpi=di;
      tmpi+=n-2;
      (*tmpi)++;
      ndest+=n-1;
    }

    return true;
  }


  int ascii85Codec::estimateEncodedSize(int old) const {
    if (old%4 == 0) {
      return 5*old/4;
    } else {
      return (old/4)*5+old%4+1;
    }
  }


  int ascii85Codec::estimateDecodedSize(int old) const {
    if (old%5 == 0) {
      return 4*old/5;
    } else {
      return (old/5)*4+old%5-1;
    }
  }


}
