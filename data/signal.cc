
//  Copyright (C) 1999--2009  Petter Urkedal
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


#include <more/cf/signal.h>
#include <csignal>
#include <iostream>
#include <stdexcept>
#include <algorithm>

namespace more {
namespace cf {

  namespace bits {
    int pending_signal = 0;
    void (*pointcheckers[MORE_CF_POINTCHECKER_COUNT_MAX])();
    int n_pointcheckers = 0;
  }

  namespace {
    void
    pending_signal_handler(int sig)
    {
	bits::pending_signal = sig;
    }
  }

  void
  handle_signal(int sig)
  {
      signal(sig, pending_signal_handler);
  }

  char const*
  signal_raised::what() const throw()
  {
      switch (m_signo) {

	  // POSIX signals:
      case SIGHUP: return "SIGHUP raised.";
      case SIGINT: return "SIGINT raised.";
      case SIGQUIT: return "SIGQUIT raised.";
      case SIGILL: return "SIGILL raised.";
      case SIGABRT: return "SIGABRT raised.";
      case SIGFPE: return "SIGFPE raised.";
      case SIGKILL: return "SIGKILL raised.";
      case SIGSEGV: return "SIGSEGV raised.";
      case SIGPIPE: return "SIGPIPE raised.";
      case SIGALRM: return "SIGALRM raised.";
      case SIGTERM: return "SIGTERM raised.";
      case SIGUSR1: return "SIGUSR1 raised.";
      case SIGUSR2: return "SIGUSR2 raised.";
      case SIGCHLD: return "SIGCHLD raised.";
      case SIGCONT: return "SIGCONT raised.";
      case SIGSTOP: return "SIGSTOP raised.";
      case SIGTSTP: return "SIGTSTP raised.";
      case SIGTTIN: return "SIGTTIN raised.";
      case SIGTTOU: return "SIGTTOU raised.";

	  // SESv2 signals
#ifdef SIGBUS
      case SIGBUS: return "SIGBUS raised.";
#endif
#ifdef SIGPOLL
      case SIGPOLL: return "SIGPOLL raised.";
#endif
#ifdef SIGPROF
      case SIGPROF: return "SIGPROF raised.";
#endif
#ifdef SIGSYS
      case SIGSYS: return "SIGSYS raised.";
#endif
#ifdef SIGTRAP
      case SIGTRAP: return "SIGTRAP raised.";
#endif
#ifdef SIGURG
      case SIGURG: return "SIGURG raised.";
#endif
#ifdef SIGVTALRM
      case SIGVTALRM: return "SIGVTALRM raised.";
#endif
#ifdef SIGXCPU
      case SIGXCPU: return "SIGXCPU raised.";
#endif
#ifdef SIGXFSZ
      case SIGXFSZ: return "SIGXFSZ raised.";
#endif

	  // Other signals.  If there are duplicate case labels when
	  // compiling thin, please add the appropriate SIGx != SIGy
	  // conjunction if the '#if' and submit a patch or send the
	  // compiler messages and the '$Id: signal.cc,v 1.1 2002/05/30 18:01:37 petter_urkedal Exp $' string on top of
	  // this file.
#ifdef SIGEMT
      case SIGEMT: return "SIGEMT raised.";
#endif
#ifdef SIGSTKFLT
      case SIGSTKFLT: return "SIGSTKFLT raised.";
#endif
#if defined(SIGIO) && SIGIO != SIGPOLL
      case SIGIO: return "SIGIO raised.";
#endif
#ifdef SIGPWR
      case SIGPWR: return "SIGPWR raised.";
#endif
#if defined(SIGLOST) && SIGLOST != SIGABRT
      case SIGLOST: return "SIGLOST raised.";
#endif
#ifdef SIGWINCH
      case SIGWINCH: return "SIGWINCH raised.";
#endif
      default: return "A signal was raised.";
      }
  }

  sigint_raised::sigint_raised()   : signal_raised(SIGINT) {}
  sigfpe_raised::sigfpe_raised()   : signal_raised(SIGFPE) {}
  sigalrm_raised::sigalrm_raised() : signal_raised(SIGALRM) {}
  sigsegv_raised::sigsegv_raised() : signal_raised(SIGSEGV) {}

  namespace bits {
    void
    dispatch_signal()
    {
	int sig = bits::pending_signal;
	bits::pending_signal = 0;
	switch(sig) {
	  case SIGINT:
	    throw sigint_raised();
	  case SIGFPE:
	    throw sigfpe_raised();
	  case SIGSEGV:
	    throw sigsegv_raised();
	  default:
	    throw signal_raised(sig);
	}
    }
  }

  void
  register_pointchecker(void (*f)())
  {
      if (bits::n_pointcheckers == MORE_CF_POINTCHECKER_COUNT_MAX)
	  throw std::logic_error(
	      "more::register_pointchecker():  Too many pointcheckers "
	      "in use.  You\nmay increase MORE_CF_POINTCHECKER_COUNT_MAX in "
	      "more/misc/signal.h and "
	      "rebuild\nlibmore.a.");
      if (std::find(bits::pointcheckers + 0,
		    bits::pointcheckers + bits::n_pointcheckers, f)
	 != bits::pointcheckers+bits::n_pointcheckers)
	  throw std::logic_error(
	      "more::register_pointchecker(): Already registered.");
      bits::pointcheckers[bits::n_pointcheckers++] = f;
  }

  void
  unregister_pointchecker(void (*f)())
  {
      void (**new_end)() =
	  std::remove(bits::pointcheckers+0,
		      bits::pointcheckers+bits::n_pointcheckers, f);
      if(new_end != bits::pointcheckers+bits::n_pointcheckers-1)
	  throw std::logic_error(
	      "more::unregister_pointchecker(): Not registered.");
      --bits::n_pointcheckers;
  }

  //
  //  Delayed interrupt
  //

  void (*old_sigint_handler)(int) = 0;

  void
  _delayed_interrupt(int sig)
  {
      bits::pending_signal = sig;
      std::cerr
	  << "** Delayed interrupt. Press CTRL-C for immediate effect.\n";
      signal(SIGINT, old_sigint_handler);
  }

  void
  enable_delayed_interrupt()
  {
      old_sigint_handler = signal(SIGINT, _delayed_interrupt);
  }

}} // more::cf
