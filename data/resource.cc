//  Copyright (C) 2002--2009  Petter Urkedal
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

#include <more/sys/resource.h>
#include <more/bits/conf.h>
#ifdef MORE_CONF_HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif
#ifdef MORE_CONF_HAVE_SYS_RESOURCE_H
#  include <sys/resource.h>
#endif
#ifdef MORE_CONF_HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <assert.h>

namespace more {
namespace sys {

  double
  current_temporal_energy()
  {
#ifdef MORE_CONF_HAVE_GETLOADAVG
      double loadavg[1];
      if (getloadavg(loadavg, 1) != -1) {
	  // The return value should be (N_AP - 1)/N_CPU.  We don't know
	  // how much of the calling process is included in loadavg, so
	  // let N_AP ≅ loadavg[0] + 0.5.
# ifdef _SC_NPROCESSORS_CONF
	  return 2*loadavg[0]/sysconf(_SC_NPROCESSORS_CONF);
# else
	  return 2*loadavg[0];
# endif
      }
#endif
      return 1.8; // ... whatever
  }

  double
  current_spatial_energy(std::ptrdiff_t size)
  {
#if defined(_SC_AVPHYS_PAGES) && defined(_SC_PAGESIZE)
      static long sc_pagesize = sysconf(_SC_PAGESIZE);
      double free_size = sysconf(_SC_AVPHYS_PAGES);
      free_size *= sc_pagesize;
      //std::cout << "free_size = " << free_size << std::endl;

      // XXX Out of the air.
      return size*(1.0 + 1e8/(1e6 + free_size));
#else
      return size*1.8;
#endif

#if 0
      double sig = XXX;
      double sig2 = sig*sig;
      double c0 = .5/sig2;
      double c1 = sqrt(c0);
      double two_sig2 = 2.0*sig2;

      double Mmax_mi_M0 = Mmax - M0;
      double q0
	  = (two_sig2*exp(-c0*math::pow2(Mmax_mi_M0))
	     - Mmax_mi_M0*math::erfc(c1*Mmax_mi_M0))
	  / math::erfc(c1*(-M0));

      double Mmax_mi_M1 = Mmax - M1;
      double q1
	  = (two_sig2*exp(-c0*math::pow2(Mmax_mi_M1))
	     - Mmax_mi_M1*math::erfc(c1*Mmax_mi_M1))
	  / math::erfc(c1*(-M1));
#endif
  }

}} // more::sys
