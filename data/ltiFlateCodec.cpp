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
 * file .......: ltiFlateCodec.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 5.11.2002
 * revisions ..: $Id: ltiFlateCodec.cpp,v 1.3 2006/02/08 12:03:43 ltilib Exp $
 */

#include "ltiFlateCodec.h"
#if HAVE_LIBZ
#include <cctype>
#include <cstdio>
#include <zlib.h>


namespace lti {

  // --------------------------------------------------
  // flateCodec
  // --------------------------------------------------

  // default constructor
  flateCodec::flateCodec()
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
  flateCodec::flateCodec(const parameters& par)
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
  flateCodec::flateCodec(const flateCodec& other)
    : dataCodec() {
    copy(other);
  }

  // destructor
  flateCodec::~flateCodec() {
  }

  // returns the name of this type
  const char* flateCodec::getTypeName() const {
    return "flateCodec";
  }

  // copy member
  flateCodec&
    flateCodec::copy(const flateCodec& other) {
      dataCodec::copy(other);

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  flateCodec&
    flateCodec::operator=(const flateCodec& other) {
    return (copy(other));
  }


  // clone member
  functor* flateCodec::clone() const {
    return new flateCodec(*this);
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool flateCodec::encodeImplementation(const buffer& src, buffer& dest,
                                        int nsrc, int& ndest) const {

    assert(src.size() >= nsrc && dest.size() >= ndest);

    if (float(nsrc)*1.01f+12 > ndest) {
      ndest=notEnoughSpace;
      setStatusString(notEnoughSpaceMsg);
    }

    z_stream zs;
    zs.next_in=const_cast<bufferElement*>(&src.at(0));
    zs.avail_in=nsrc;
    zs.next_out=&dest.at(0);
    zs.avail_out=ndest;
    zs.zalloc=Z_NULL;
    zs.zfree=Z_NULL;
    zs.opaque=Z_NULL;

    // must return Z_OK
    int errc=deflateInit(&zs,Z_DEFAULT_COMPRESSION);
    if (errc != Z_OK) {
      setStatusString(notNull(zs.msg) ? zs.msg : "deflateInit failed.");
      return false;
    }
    // must return Z_STREAM_END
    errc=deflate(&zs,Z_FINISH);
    if (errc != Z_STREAM_END) {
      setStatusString(notNull(zs.msg) ? zs.msg : "deflate failed.");
      return false;
    }
    ndest=zs.next_out-&dest.at(0);
    // must return Z_OK
    errc=deflateEnd(&zs);
    if (errc != Z_OK) {
      setStatusString(notNull(zs.msg) ? zs.msg : "deflateEnd failed.");
      return false;
    }

    return true;
  }


  bool flateCodec::decodeImplementation(const buffer& src, buffer& dest,
                                        int nsrc, int& ndest) const {

    assert(src.size() >= nsrc && dest.size() >= ndest);

    z_stream zs;
    zs.next_in=const_cast<bufferElement*>(&src.at(0));
    zs.avail_in=nsrc;
    zs.next_out=&dest.at(0);
    zs.avail_out=ndest;
    zs.zalloc=Z_NULL;
    zs.zfree=Z_NULL;
    zs.opaque=Z_NULL;

    int errc;
    // must return Z_OK
    errc=inflateInit(&zs);
    if (errc != Z_OK) {
      setStatusString(notNull(zs.msg) ? zs.msg : "inflateInit failed.");
      return false;
    }
    // must return Z_STREAM_END
    errc=inflate(&zs,Z_FINISH);
    if (errc != Z_STREAM_END) {
      if (zs.avail_out == 0 && errc == Z_OK) {
        ndest=notEnoughSpace;
        setStatusString(notEnoughSpaceMsg);
      } else {
        setStatusString(notNull(zs.msg) ? zs.msg : "inflate failed.");
      }
      return false;
    }
    ndest=zs.next_out-&dest.at(0);
    // must return Z_OK
    errc=inflateEnd(&zs);
    if (errc != Z_OK) {
      setStatusString(notNull(zs.msg) ? zs.msg : "inflateEnd failed.");
      return false;
    }

    return true;
  }


  int flateCodec::estimateEncodedSize(int old) const {
    return static_cast<int>(float(old)*1.01f+0.5f+12);
  }


  int flateCodec::estimateDecodedSize(int old) const {
    return old*10;
  }

}

#endif
