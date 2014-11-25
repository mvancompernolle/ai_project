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
 * file .......: ltiSemaphore.cpp
 * authors ....: Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 04.11.99
 * revisions ..: $Id: ltiSemaphore.cpp,v 1.8 2009/08/23 01:18:51 alvarado Exp $
 */

#include "ltiSemaphore.h"
#include "ltiAssert.h"
#include "ltiException.h"
#include <errno.h>

namespace lti {


  // returns the name of this type
  const char* semaphore::getTypeName() const {
    return "semaphore";
  }

#if defined(_LTI_MACOSX)

  // --------------------------------------------------------
  //                        MACOSX
  // --------------------------------------------------------

  static int curNum = 0;

  // default constructor
  semaphore::semaphore(const int initialValue) :
    
    initValue(initialValue) {
    assert( initValue<SEM_VALUE_MAX );
    osxSemNumber=++curNum;
    sprintf(myName,"ltisem_%u", osxSemNumber);
    
    osxSemaphore = sem_open(myName,O_CREAT,0644,initValue);

  }

  // destructor
  semaphore::~semaphore() {
    destroy();
  }

  // wait on semaphore, i.e. decrease the value or wait if counter <= 0
  bool semaphore::wait() {
    return(sem_wait(osxSemaphore) == 0);
  }

  // try to wait on semaphore, but do not block
  bool semaphore::tryWait() {
    return (sem_trywait(osxSemaphore) == 0);
  }

  // post semaphore, i.e. increase the value
  bool semaphore::post() {
  	return (sem_post(osxSemaphore) == 0);
  }

  // get current value // FIXME sem_getvalue() broken?
  int semaphore::getValue() {
    int value=0;
    sem_getvalue(osxSemaphore,&value);
    return value;
  }

  // reset value to initialValue
  void semaphore::reset() {
    destroy();
    osxSemaphore = sem_open(myName,O_CREAT,0644,initValue);
  }

  // destroy semaphore
  void semaphore::destroy() {
    sem_close(osxSemaphore);
  }

#elif defined(_LTI_WIN32)
  // --------------------------------------------------------
  //                        WINDOWS
  // --------------------------------------------------------

  // default constructor
  semaphore::semaphore(const int initialValue)
    : initValue(initialValue),counter(initialValue) {
    theSemaphore =
       CreateSemaphore(NULL,initialValue,SEM_VALUE_MAX,NULL);
    if (theSemaphore == NULL) {
      unsigned int error;
      error = GetLastError();
      throw exception("Semaphore cannot be created!");
    }
  }

  // destructor
  semaphore::~semaphore() {
    destroy();
  }

  // wait until lock for semaphore becomes available
  bool semaphore::wait() {
    counter--;
    return (WaitForSingleObject(theSemaphore,INFINITE) != WAIT_FAILED);
  }

  // try to lock semaphore, but do not block
  bool semaphore::tryWait() {
    if (WaitForSingleObject(theSemaphore,0) == WAIT_OBJECT_0) {
      counter--;
      return true;
    } else {
      return false;
    };
  }

  // unlock semaphore
  bool semaphore::post() {
    // TODO: This value might be invalid:
    counter++;
    return (ReleaseSemaphore(theSemaphore,1,NULL) != 0);
  }

  // destroy semaphore
  void semaphore::destroy() {

    while(!tryWait() && (counter < initValue) ) {
      post();
    }

    post();

    CloseHandle(theSemaphore);
  }
  // get current value
  int semaphore::getValue() {
    return counter;
  }

  // reset value to initialValue
  void semaphore::reset() {
    destroy();
    theSemaphore =
       CreateSemaphore(NULL,initValue,SEM_VALUE_MAX,NULL);

  }


#else 

  // --------------------------------------------------------
  //                        UNIX/LINUX
  // --------------------------------------------------------

# ifdef _LTI_GNUC_4_3
  // in gcc 4.3.1 there is no SEM_VALUE_MAX defined, so we will import
  // the place where is now defined
#   include <climits>
# endif

  // default constructor
  semaphore::semaphore(const int initialValue) :
    initValue(initialValue) {
    assert( initValue<SEM_VALUE_MAX );
    sem_init(&theSemaphore,0,initValue);
  }

  // destructor
  semaphore::~semaphore() {
    destroy();
  }

  // wait on semaphore, i.e. decrease the value or wait if counter <= 0
  bool semaphore::wait() {
    return (sem_wait(&theSemaphore) == 0);
  }

  // try to wait on semaphore, but do not block
  bool semaphore::tryWait() {
    return (sem_trywait(&theSemaphore)==0);
  }

  // post semaphore, i.e. increase the value
  bool semaphore::post() {
    return (sem_post(&theSemaphore) == 0);
  }

  // get current value
  int semaphore::getValue() {
    int value;
    sem_getvalue(&theSemaphore,&value);
    return value;
  }

  // reset value to initialValue
  void semaphore::reset() {
    destroy();
    sem_init(&theSemaphore,0,initValue);
  }

  // destroy semaphore
  void semaphore::destroy() {
    while(sem_destroy(&theSemaphore)!=0) {
      assert( errno==EBUSY );
      while(getValue()<=0) {
        post();
      }
    }
    sem_destroy(&theSemaphore);
  }

#endif
}
