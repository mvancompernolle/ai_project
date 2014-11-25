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
 * file .......: ltiThread.cpp
 * authors ....: Thomas Rusert, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 03.11.99
 * revisions ..: $Id: ltiThread.cpp,v 1.5 2006/02/08 12:54:47 ltilib Exp $
 */

#include "ltiThread.h"
#include <signal.h>
#include "ltiException.h"

#ifdef _LTI_WIN32
#  include <windows.h>
#  include <process.h>
#endif

#undef _LTI_DEBUG
// #define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {

  mutex thread::startMutex;


  // destructor
  thread::~thread() {
    stop();
  }

  // returns the name of this type
  const char* thread::getTypeName() const {
    return "thread";
  }

  // default constructor
  thread::thread() : alive(false), suspendSem(0) {
  }

# ifndef _LTI_WIN32

  // --------------------------------------------------------
  //                        UNIX/LINUX
  // --------------------------------------------------------

  // start thread
  void thread::start() {
    startMutex.lock();
    if(!isAlive()) {
      suspendSem.reset();
      if(pthread_create(&theThread, 0, &(this->execute), this)==0) {
        pthread_detach(theThread);
        alive = true;
      }
    }
    startMutex.unlock();
  }

  // stop thread
  void thread::stop() {
    _lti_debug("thread::stop() called" << endl);
    if (isAlive()) {
      if (representsCalledThread()) {
        _lti_debug("exiting current thread" << endl);
        pthread_exit(0);
      } else {
        _lti_debug("cancel thread" << endl);
        pthread_cancel(theThread); // request cancellation...
        _lti_debug("and join it" << endl);
        join();                    // ...and wait for the thread
        _lti_debug("join done." << endl);
      }
    }
  }

  // wait for thread termination
  void thread::join() {
    startMutex.lock();
    if(isAlive() && !representsCalledThread()) {
      startMutex.unlock();
      suspendSem.wait();
      startMutex.lock();

      while(isAlive()) {
        startMutex.unlock();
        startMutex.lock();
      }
    }
    startMutex.unlock();
  }

  bool thread::representsCalledThread() const {
    if(isAlive()) {
      return(pthread_equal(theThread,pthread_self())!=0);
    } else {
      return false;
    }
  }

  void* thread::execute(void* threadObject) {
    thread* theObject = reinterpret_cast<thread*>(threadObject);
    if (notNull(theObject)) {
      int oldstate;
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&oldstate);
      pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&oldstate);
      pthread_cleanup_push(clean,threadObject);
      theObject->run();
      pthread_cleanup_pop(1);
    } else {
      throw lti::exception("Not a valid threadObject");
    }
    return 0;
  }

  void thread::clean(void* threadObject) {
    _lti_debug("thread::clean" << endl);
    thread* theObject = reinterpret_cast<thread*>(threadObject);
    theObject->startMutex.lock();
    if (theObject->alive) {
      theObject->alive = false;
      theObject->startMutex.unlock();
      theObject->cleanUp();
      theObject->startMutex.lock();
      _lti_debug(" posting thread::suspendSem" << endl);
      theObject->suspendSem.post(); // in case there is someone
                                    // joining this thread! (called
                                    // before cleanUp in case the last
                                    // doesn't return properly
    }
    theObject->startMutex.unlock();

  }

  bool thread::isAlive() const {
    return alive;
  }


  // --------------------------------------------------------
  //                        WINDOWS
  // --------------------------------------------------------

# else

  // start thread
  void thread::start() {
    startMutex.lock();

    if(!isAlive()) {
      unsigned long tmp;

      suspendSem.reset();
      tmp = _beginthread(execute,0,this);
      if ((tmp!=0) && (tmp!=-1)) {
        theThread = (HANDLE)tmp;
        alive = true;
      }
    }
    startMutex.unlock();
  }

  // stop thread
  void thread::stop() {
    if(isAlive()) {
      if (representsCalledThread()) {
        _endthread();
        clean(this);
      } else {
        // stop the thread
        TerminateThread(theThread,0);
        clean(this);
      }
    }
  }

  // wait for thread termination
  void thread::join() {
    startMutex.lock();
    if(isAlive() && !representsCalledThread()) {
      startMutex.unlock();
      suspendSem.wait();
      startMutex.lock();

      while(isAlive()) {
        startMutex.unlock();
        startMutex.lock();
      }
    }
    startMutex.unlock();
  }

  bool thread::representsCalledThread() const {
    if(isAlive()) {
      return(calledThreadId == GetCurrentThreadId());
    } else {
      return false;
    }
  }

  // function to be executed in a new thread!
  void thread::execute(void* threadObject) {
    thread* theObject = reinterpret_cast<thread*>(threadObject);
    if (notNull(theObject)) {
      theObject->calledThreadId = GetCurrentThreadId();
      theObject->run();
      clean(threadObject);
    } else {
      throw lti::exception("Not a valid threadObject");
    }
  }

  void thread::clean(void* threadObject) {
    thread* theObject = reinterpret_cast<thread*>(threadObject);
    theObject->startMutex.lock();

    if (theObject->alive) {
      theObject->cleanUp();
      theObject->alive = false;
      theObject->suspendSem.post(); // in case there is someone
                                    // joining this thread! (called
                                    // before cleanUp in case the last
                                    // doesn't return properly
    }

    theObject->startMutex.unlock();

  }

  bool thread::isAlive() const {
    return alive;
  }

# endif

}
