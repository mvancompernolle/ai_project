/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiIoObject.cpp
 * authors ....: Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 14.12.01
 * revisions ..: $Id: ltiIoObject.cpp,v 1.3 2006/02/08 12:09:37 ltilib Exp $
 */

#include "ltiIoObject.h"

namespace lti {

  // default constructor
  ioObject::ioObject() : object() {
  }

  // copy constructor
  ioObject::ioObject(const ioObject& other)  : object() {
    copy(other);
  }

  // destructor
  ioObject::~ioObject() {
  }

  // copy data from other ioObject
  ioObject& ioObject::copy(const ioObject& other) {
    return (*this);
  }

  // assigment operator (alias for copy(other)).
  ioObject& ioObject::operator=(const ioObject& other) {
    return copy(other);
  }

  // get type name
  const char* ioObject::getTypeName() const {
    return "ioObject";
  }


  // read ioObject
  bool read(ioHandler& handler, ioObject& p, const bool complete) {
    return p.read(handler,complete);
  }

  // write ioObject
  bool write(ioHandler& handler, const ioObject& p, const bool complete) {
    return p.write(handler,complete);
  }

/*  bool operator==(const ioObject& one, const ioObject& other) {
    return true;
  }*/
}
