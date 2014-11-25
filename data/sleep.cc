//  Copyright (C) 2001--2009  Petter Urkedal
//
//  This file is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  As a special exception, you may use this file as part of a free
//  software library without restriction.  Specifically, if other files
//  instantiate templates or use macros or inline functions from this
//  file, or you compile this file and link it with other files to
//  produce an executable, this file does not by itself cause the
//  resulting executable to be covered by the GNU General Public
//  License.  This exception does not however invalidate any other
//  reasons why the executable file might be covered by the GNU General
//  Public License.


#include <more/sys/time.h>
#include <more/diag/errno.h>
#include <cmath>
#include <csignal>
#include <more/bits/conf.h>

#ifdef MORE_CONF_HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif
#ifdef MORE_CONF_HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef MORE_CONF_HAVE_UNISTD_H
#  include <unistd.h>
#endif


namespace more {
namespace sys {

#ifdef MORE_CONF_HAVE_NANOSLEEP

  void
  sleep(time_t t)
  {
      struct timespec req;
      double sec;
      double subsec;
      subsec = std::modf(t, &sec);
      req.tv_sec = (int)sec;
      req.tv_nsec = (int)(subsec*1e9);
      nanosleep(&req, 0);
  }

#elif defined(MORE_CONF_HAVE_SETITIMER) && defined(MORE_CONF_HAVE_PAUSE)

  void
  sleep_handler(int sig)
  {
      // nop
  }

  void
  sleep(time_t t)
  {
      itimerval it;
      it.it_interval.tv_sec = 0;
      it.it_interval.tv_usec = 0;
      double sec;
      double sub = std::modf(t, &sec)*1e6;
      it.it_value.tv_sec = (int)sec;
      it.it_value.tv_usec = (int)sub;
      void (*oldh)(int) = std::signal(SIGALRM, sleep_handler);
      if (oldh != SIG_ERR) {
	  if (setitimer(ITIMER_REAL, &it, 0) == -1)
	      throw diag::errno_exception();
	  ::pause();
	  std::signal(SIGALRM, oldh);
      }
      else
	  throw std::runtime_error("more::sys::sleep: "
				   "Could not set a signal handler.");
  }

#else

  void
  sleep(time_t t)
  {
      ::sleep((int)t);
  }

#endif

}} // more::sys
