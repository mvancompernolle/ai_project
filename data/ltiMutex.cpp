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
 * file .......: ltiMutex.cpp
 * authors ....: Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 04.11.99
 * revisions ..: $Id: ltiMutex.cpp,v 1.4 2006/02/08 12:53:39 ltilib Exp $
 */

#include "ltiMutex.h"
#include "ltiAssert.h"
#include <errno.h>

namespace lti {

  // --------------------------------------------------------
  //                        UNIX/LINUX
  // --------------------------------------------------------

  // returns the name of this type
  const char* mutex::getTypeName() const {
    return "mutex";
  }


# ifndef _LTI_WIN32

  // default constructor
  mutex::mutex() {
    pthread_mutex_init(&theMutex,0);
  }

  // destructor
  mutex::~mutex() {
    destroy();
  }

  // wait until lock for mutex becomes available
  void mutex::lock() {
    pthread_mutex_lock(&theMutex);
  }

  // try to lock mutex, but do not block
  bool mutex::tryLock() {
    return (pthread_mutex_trylock(&theMutex)==0);
  }

  // unlock mutex
  void mutex::unlock() {
    pthread_mutex_unlock(&theMutex);
  }

  // destroy mutex
  void mutex::destroy() {
    int status;
    while((status=pthread_mutex_destroy(&theMutex))!=0) {
      assert( status==EBUSY );
      unlock();
    }
    pthread_mutex_destroy(&theMutex);
  }


# else

  // --------------------------------------------------------
  //                        WINDOWS
  // --------------------------------------------------------

  // default constructor
  mutex::mutex() {
    theMutex = CreateMutex(NULL,FALSE,NULL);
  }

  // destructor
  mutex::~mutex() {
    destroy();
    CloseHandle(theMutex);
  }

  // wait until lock for mutex becomes available
  void mutex::lock() {
    WaitForSingleObject(theMutex,INFINITE);
  }

  // try to lock mutex, but do not block
  bool mutex::tryLock() {
    return (WaitForSingleObject(theMutex,0) == WAIT_OBJECT_0);
  }

  // unlock mutex
  void mutex::unlock() {
    ReleaseMutex(theMutex);
  }

  // destroy mutex
  void mutex::destroy() {
    unlock();
  }

# endif
}
