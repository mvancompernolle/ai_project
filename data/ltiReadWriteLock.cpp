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
 * file .......: ltiReadWriteLock.cpp
 * authors ....: Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 10.11.99
 * revisions ..: $Id: ltiReadWriteLock.cpp,v 1.5 2006/02/08 12:54:13 ltilib Exp $
 */
#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
#include "ltiReadWriteLock.h"

namespace lti {
  // default constructor
  readWriteLock::readWriteLock() {
    pthread_rwlock_init(&theLock,0);
  }

  // destructor
  readWriteLock::~readWriteLock() {
    destroy();
  }

  // wait until lock for readWriteLock becomes available; lock for reading
  void readWriteLock::lockRead() const {
    pthread_rwlock_rdlock(&theLock);
  }

  // try to lock readWriteLock for read access, but do not block
  bool readWriteLock::tryLockRead() const {
    return (pthread_rwlock_tryrdlock(&theLock)==0);
  }

  // wait until lock for readWriteLock becomes available; lock for writing
  void readWriteLock::lockWrite() const {
    pthread_rwlock_wrlock(&theLock);
  }

  // try to lock readWriteLock for write access, but do not block
  bool readWriteLock::tryLockWrite() const {
    return (pthread_rwlock_trywrlock(&theLock)==0);
  }

  // unlock readWriteLock
  void readWriteLock::unlock() const {
    pthread_rwlock_unlock(&theLock);
  }

  // destroy readWriteLock
  void readWriteLock::destroy() {
    while(pthread_rwlock_destroy(&theLock)!=0) {
  unlock();
    }
    pthread_rwlock_destroy(&theLock);
  }

  // returns the name of this type
  const char* readWriteLock::getTypeName() const {
    return "readWriteLock";
  }
}
#endif
