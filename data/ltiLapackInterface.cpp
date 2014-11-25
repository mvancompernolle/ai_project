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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiLapackInterface.cpp
 * authors ....:
 * organization: LTI, RWTH Aachen
 * creation ...: 13.11.2002
 * revisions ..: $Id: ltiLapackInterface.cpp,v 1.4 2006/02/07 18:05:57 ltilib Exp $
 */


#include "ltiLapackInterface.h"

#ifdef HAVE_LAPACK

extern "C" int MAIN__() {
  // dummy method for f2c lib
  return 0;
}

namespace lti {

  // initialize lock-lapack mutex pointer with 0
  mutex lapackInterface::lola;

  lapackInterface::lapackInterface() {
    static mutex protectPointer;

    // prevent this dummy function from being thrown out by the
    // linker
    MAIN__();

    //protectPointer.lock();
    //if (isNull(lola)) {
    //  lola = new mutex;
    //}
    //protectPointer.unlock();
  }


  lapackInterface::~lapackInterface() {
  }

}

#endif
