//  Copyright (C) 1998--2009  Petter Urkedal
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


#include <more/bits/conf.h>

#include <more/sys/time.h>
#ifdef MORE_CONF_HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif
#ifdef MORE_CONF_HAVE_SYS_RESOURCE_H
#  include <sys/resource.h>
#endif
#ifdef MORE_CONF_HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <time.h> // some C++ libraries does not place some things in std::

#include <iostream>
#include <stdexcept>
#include <locale>
#include <cctype>



namespace more {
namespace sys {

  void
  print_time_nicely(std::ostream& out, double t)
  {
      if(t > 120) {
	  t /= 60;
	  if(t > 120) {
	      t /= 60;
	      if(t > 48) {
		  t /= 24;
		  out << (int)t << " days " << (int)(t*24-(int)t*24) << " h";
	      } else
		  out << (int)t << " h " << (int)(t*60-(int)t*60) << " min";
	  } else
	      out << (int)t << " min " << (int)(t*60-(int)t*60) << " s";
      } else
	  out << t << " s";
  }

  double
  world_time()
  {
#ifdef MORE_CONF_HAVE_GETTIMEOFDAY
      ::timeval tv;
      ::gettimeofday(&tv, NULL);
      return tv.tv_sec + 1e-6*tv.tv_usec;
#else
      return ::time(NULL);
#endif
  }

  double
  world_time(double const& tref)
  {
#ifdef MORE_CONF_HAVE_GETTIMEOFDAY
      long tref_s = (long)tref;
      long tref_us = (long)(tref*1e6);
      ::timeval tv;
      ::gettimeofday(&tv, NULL);
      return (tv.tv_sec - tref_s) + 1e-6*(tv.tv_usec - tref_us);
#else
      return ::time(NULL) - tref;
#endif
  }

  double
  process_time()
  {
#if defined(MORE_CONF_HAVE_GETRUSAGE) && defined(MORE_CONF_HAVE_SYS_RESOURCE_H)
      ::rusage use;
      ::getrusage(RUSAGE_SELF, &use);
      return use.ru_utime.tv_sec + use.ru_stime.tv_sec
	  + 1e-6*(use.ru_utime.tv_usec + use.ru_stime.tv_usec);
#else
      return ::clock()/(double)CLOCKS_PER_SEC;
#endif
  }

  namespace {
    double birth_world_time = world_time();
  }

  double
  process_world_age()
  {
// #ifndef NDEBUG
//       if (birth_world_time == 0.0)
// 	  throw std::logic_error("more::sys::process_world_age: "
// 				 "Initialization of libmore was not done "
// 				 "at startup.");
// #endif
      return world_time(birth_world_time);
  }

  double
  process_birth_world_time()
  {
      return birth_world_time;
  }



}} // more::sys
