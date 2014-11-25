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


#include <new>
#include <stdexcept>
#include <algorithm>
#include <more/sys/system.h>
#include <more/cf/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

// checkme: could not figure out if environ is a POSIX feature.
//extern char** environ;

namespace more {
namespace sys {

  int
  system(std::string command)
  {
      int pid = fork();
      if(pid == -1)
	  switch(errno) {
	  case EAGAIN:
	      throw std::bad_alloc();
//		  "fork: insufficient memory to duplicate process.");
	  case ENOMEM:
	      throw std::bad_alloc();
//		  "fork: insufficient memory for kernel structures.");
	  }
      char* cmd = new char[command.size()+1];
      std::copy(command.begin(), command.end(), cmd);
      cmd[command.size()] = 0;
      if(pid == 0) {
	  char *argv[4];
	  argv[0] = const_cast<char *>("sh");
	  argv[1] = const_cast<char *>("-c");
	  argv[2] = cmd; //const_cast<char*>(command.c_str());
	  argv[3] = 0;
	  execv("/bin/sh", argv);
	  exit(127);
      } else {
	  try {
	      for (;;) {
		  int status;
		  switch (::waitpid(pid, &status, WNOHANG)) {
		  case -1:
		      delete[] cmd;
		      return -1;
		  case 0:
		      break;
		  default:
		      delete[] cmd;
		      return status;
		  }
		  cf::checkpoint();
		  sleep(1);
	      }
	  }
	  catch (...) {
	      delete[] cmd;
	      int status;
	      kill(pid, SIGINT);
	      int wtime = 1;  // Give the process 1 s to clean up.
	      while (::waitpid(pid, &status, WNOHANG) == 0) {
		  if (--wtime < 0) {
		      kill(pid, SIGKILL);
		      ::waitpid(pid, &status, 0);
		      break;
		  }
		  sleep(1);
	      }
	      throw;
	  }
      }
      return 0; // never reached
  }

  int
  system(char const* const* argv, more::io::redirection const& redir)
  {
      int pid = fork();
      if (pid == 0) {
	  if (!redir.permanent())
	      exit(127);
	  execvp(argv[0], const_cast<char* const*>(argv));
	  exit(127);
      }

      if (pid == -1)
	  return -1;

      for (;;) {
	  int st;
	  if (::waitpid(pid, &st, 0) == -1) {
	      if (errno != EINTR)
		  return -1;
	  }
	  else
	      return st;
      }
  }

}} // more::sys
