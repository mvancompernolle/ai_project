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
 * file .......: ltiRunLengthCodec.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 5.11.2002
 * revisions ..: $Id: ltiRunLengthCodec.cpp,v 1.4 2006/02/08 12:07:28 ltilib Exp $
 */

//TODO: include files
#include "ltiRunLengthCodec.h"
#include <cctype>
#include <cstdio>


namespace lti {

  // --------------------------------------------------
  // runLengthCodec
  // --------------------------------------------------

  // default constructor
  runLengthCodec::runLengthCodec()
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
  runLengthCodec::runLengthCodec(const parameters& par)
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
  runLengthCodec::runLengthCodec(const runLengthCodec& other)
    : dataCodec() {
    copy(other);
  }

  // destructor
  runLengthCodec::~runLengthCodec() {
  }

  // returns the name of this type
  const char* runLengthCodec::getTypeName() const {
    return "runLengthCodec";
  }

  // copy member
  runLengthCodec&
    runLengthCodec::copy(const runLengthCodec& other) {
      dataCodec::copy(other);

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  runLengthCodec&
    runLengthCodec::operator=(const runLengthCodec& other) {
    return (copy(other));
  }


  // clone member
  functor* runLengthCodec::clone() const {
    return new runLengthCodec(*this);
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool runLengthCodec::encodeImplementation(const buffer& src, buffer& dest,
                                        int nsrc, int& ndest) const {

    assert(src.size() >= nsrc && dest.size() >= ndest);

    int count=0;

    buffer::const_iterator si=src.begin();
    buffer::iterator di=dest.begin();

    int i=0;

    bufferElement old;

    while (i < nsrc && count < ndest) {
      old=*si;
      buffer::const_iterator tmpi=si;

      // detect run
      int n=1;
      int tmpj=i+1;
      tmpi++;
      while (tmpj < nsrc && *tmpi == old && n < 128) {
        tmpi++;
        tmpj++;
        n++;
      }

      // check number of identical bytes
      if (n > 1) {
        // have at least two copies; write length byte of n
        *di++=static_cast<bufferElement>(257-n);
        // write data byte
        *di++=old;
        si+=n;
        count+=2;
      } else {
        // we do not have a run; detect number of different elements
        if (tmpj < nsrc) {
          old=*tmpi;
          tmpi++;
          tmpj++;
          while (tmpj < nsrc && *tmpi != old && n < 128) {
            old=*tmpi;
            tmpi++;
            tmpj++;
            n++;
          }
          // we now have n bytes that differ
          *di++=static_cast<bufferElement>(n-1);
          for (int j=0; j<n; j++) {
            *di++=*si++;
          }
          count+=n+1;
        } else {
          // special case: We have a single byte at the end of the
          // stream
          *di++=static_cast<bufferElement>(0);
          *di++=old;
          count+=2;
          si++;
        }
      }
      i+=n;
    }
    if (count >= ndest && i < nsrc) {
      ndest=-1;
      setStatusString(notEnoughSpaceMsg);
      return false;
    }
    ndest=count;
    return true;
  }

  bool runLengthCodec::decodeImplementation(const buffer& src, buffer& dest,
                                        int nsrc, int& ndest) const {

    assert(src.size() >= nsrc && dest.size() >= ndest);

    int count=0;

    buffer::const_iterator si=src.begin();
    buffer::iterator di=dest.begin();

    int i=0;

    while (i < nsrc && count < ndest) {
      bufferElement tmp=*si++;
      if (tmp < 128) {
        // copy next tmp+1 bytes
        tmp++;
        for (int j=0; j<tmp; j++) {
          *di++=*si++;
        }
        i+=tmp+1;
      } else if (tmp > 128) {
        // we have a run
        bufferElement data=*si++;
        tmp=257-tmp;
        for (int j=0; j<tmp; j++) {
          *di++=data;
        }
        i+=2;
      } else {
        // end of data
        setStatusString("End of data occured in the middle of the buffer");
        ndest=count;
        return false;
      }
      count+=tmp;
    }

    if (count >= ndest && i < nsrc) {
      ndest=-1;
      setStatusString(notEnoughSpaceMsg);
      return false;
    }
    ndest=count;
    return true;
  }


  int runLengthCodec::estimateEncodedSize(int old) const {
    // this is the absolute worst case
    return 2*old;
  }


  int runLengthCodec::estimateDecodedSize(int old) const {
    return old*2;
  }

}
