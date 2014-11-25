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
 * file .......: ltiException.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 07.04.99
 * revisions ..: $Id: ltiException.cpp,v 1.2 2006/02/07 18:08:08 ltilib Exp $
 */

#include "ltiException.h"

#ifdef _DEBUG_EXCEPTION
#include <iostream>
#endif

namespace lti {

  exception::exception(const char* excName)
    : object(),std::exception(),exceptionName(excName) {
#   ifdef _DEBUG_EXCEPTION
    std::cerr << "lti::exception created with message: " << std::endl << "  ";
    std::cerr << excName << std::endl;
#   endif

  };

  exception::exception(const std::string& excName)
    : object(),std::exception(),exceptionName(excName) {
#   ifdef _DEBUG_EXCEPTION
    std::cerr << "lti::exception created with message: " << std::endl << "  ";
    std::cerr << excName << std::endl;
#   endif
  };

  exception::~exception() throw () {
  };


  /*
   * copy constructor
   */
  exception::exception(const exception& other)
    : object(),std::exception() {
    copy(other);
  };

  exception* exception::clone() const {
    return (new exception(*this));
  };

  const char* exception::getTypeName() const {
    return "exception";
  }

  const char* exception::what() const throw () {
    return exceptionName.c_str();
  }

  const char* allocException::getTypeName() const  {
    return "allocException";
  }
}
