/***************************************************************************
                     ulxr_mutex.cpp  -  mutex handling
                             -------------------
    begin                : Thu Dec 03 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_mutex.cpp 940 2006-12-30 18:22:05Z ewald-arnold $

 ***************************************************************************/

/**************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***************************************************************************/

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <cerrno>

#ifdef __unix__
#include <pthread.h>
#endif

#include "ulxr_mutex.h"
#include "ulxr_except.h"


namespace ulxr {


ULXR_API_IMPL0 Mutex::Mutex()
{
#if defined __WIN32__
  if ((handle = CreateMutex(0, false, 0)) < 0)
#else
  if (pthread_mutex_init(&handle, 0) != 0)
#endif
    throw Exception(SystemError, ulxr_i18n(ULXR_PCHAR("Could not create mutex")));
}


ULXR_API_IMPL0 Mutex::~Mutex() throw()
{
#if defined __WIN32__
  CloseHandle(handle);
#elif _THREAD
  pthread_mutex_destroy(&handle);
#endif
}


ULXR_API_IMPL(void) Mutex::lock()
{
#if defined __WIN32__
  if (WaitForSingleObject(handle, INFINITE) == WAIT_TIMEOUT)
#else
  if (pthread_mutex_lock(&handle) != 0)
#endif
    throw Exception(SystemError, ulxr_i18n(ULXR_PCHAR("Could not lock mutex")));
}


ULXR_API_IMPL(void) Mutex::unlock()
{
#if defined __WIN32__
  if (!ReleaseMutex(handle))
#else
  if (pthread_mutex_unlock(&handle)!=0)
#endif
    throw Exception(SystemError, ulxr_i18n(ULXR_PCHAR("Could not unlock mutex")));
}


ULXR_API_IMPL(bool) Mutex::tryLock()
{
#if defined __WIN32__

  DWORD ret = WaitForSingleObject(handle, 1);

  if (ret == WAIT_FAILED)
    return false;

  if (ret == WAIT_TIMEOUT)
    return false;

  if (ret == WAIT_ABANDONED)
    return false;

  if (ret != WAIT_OBJECT_0)
    return false;

#else

  int ret = pthread_mutex_trylock(&handle);
  if (ret == EBUSY)
    return false;

  if (ret != 0)
    return false;

#endif

  return true;
}


///////////////////////////////////////////////////////////
//


ULXR_API_IMPL0 Mutex::Locker::Locker(Mutex &mtx)
  : mutex(&mtx)
{
  mutex->lock();
}


ULXR_API_IMPL0 Mutex::Locker::~Locker() throw()
{
  try
  {
    mutex->unlock();  // might throw, forget it
  }
  catch(...)
  {
  }
}


///////////////////////////////////////////////////////////
//


ULXR_API_IMPL0 Mutex::TryLocker::TryLocker(Mutex &mtx)
  : mutex(&mtx)
{
  locked = mutex->tryLock();
}


ULXR_API_IMPL0 Mutex::TryLocker::~TryLocker() throw()
{
  try
  {
    if (isLocked())
      mutex->unlock();  // might throw, forget it
  }
  catch(...)
  {
  }
}


ULXR_API_IMPL(bool) Mutex::TryLocker::isLocked()
{
  return locked;
}


}  // namespace ulxr

