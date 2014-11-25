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


#include <more/io/redirection.h>

#if defined(MORE_CONF_HAVE_SYS_TYPES_H) && defined(MORE_CONF_HAVE_SYS_STAT_H) && defined(MORE_CONF_HAVE_FCNTL_H)
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


namespace more {
namespace io {

  redirection::redirection()
  {
      for (int i = 0; i < 3; ++i)
	  m_set[i] = m_save[i] = -1;
  }

  redirection::redirection(redirection const &redir)
  {
      for (int i = 0; i < 3; ++i) {
	  if (redir.m_set[i] != -1)
	      m_set[i] = dup(redir.m_set[i]);
	  else
	      m_set[i] = -1;
	  m_save[i] = -1;
      }
  }

  redirection::~redirection()
  {
      deactivate();
      for (int i = 0; i < 3; ++i)
	  if (m_set[i] != -1)
	      close(m_set[i]);
  }

  bool
  redirection::set(int i, int fd)
  {
      // Close old redirection if present
      if (m_set[i] != -1)
	  close(m_set[i]);
      m_set[i] = fd;

      if (m_save[i] != -1) {
	  // If old redirection was active, activate the new one.
	  switch (i) {
	  case 0: break;
	  case 1: std::cout.flush(); break;
	  case 2: std::cerr.flush(); break;
	  }
	  return dup2(m_set[i], i) != -1;
      }
      else
	  return true;
  }

  bool
  redirection::set(int ich, std::string name, int perm)
  {
      // Open the file.
      int flags;
      if (ich == 0)
	  flags = O_RDONLY;
      else
	  flags = O_WRONLY | O_CREAT | O_TRUNC;
      int fd = open(name.c_str(), flags, perm);
      if (fd == -1)
	  return false;

      // Set redirection.
      if (ich == 3) {
	  int fd2 = dup(fd);
	  if (fd2 == -1) {
	      close(fd);
	      return false;
	  }
	  return set(1, fd) && set(2, fd2);
      }
      else
	  return set(ich, fd);
  }

  bool
  redirection::activate() const
  {
      //std::cin.flush();
      std::cout.flush();
      std::cerr.flush();
      for (int i = 0; i < 3; ++i) {
	  if (m_set[i] != -1 && m_save[i] == -1) {
	      m_save[i] = dup(i);
	      if (m_save[i] == -1) {
		  deactivate();
		  return false;
	      }
	      if (dup2(m_set[i], i) == -1) {
		  deactivate();
		  return false;
	      }
	  }
      }
      return true;
  }

  bool
  redirection::permanent() const
  {
      std::cout.flush();
      std::cerr.flush();
      for (int i = 0; i < 3; ++i) {
	  if (m_save[i] != -1)
	      close(m_save[i]);
	  else if (m_set[i] != -1) {
	      if (dup2(m_set[i], i) == -1)
		  return false;
	  }
      }
      return true;
  }

  void
  redirection::deactivate() const
  {
      for (int i = 0; i < 3; ++i) {
	  if (m_save[i] != -1) {
	      dup2(m_save[i], i);
	      close(m_save[i]);
	      m_save[i] = -1;
	  }
      }
  }

}}

#else
namespace more {
namespace io {

  redirection::redirection()
  {
      static bool informed_p = false;
      if (!informed_p) {
	  std::cerr << "warning: Redirection of standard streams is not "
		    << "supported on this platform.\n";
	  informed_p = true;
      }
  }

  redirection::~redirection() {}
  bool redirection::set_in(std::string const&) {}
  bool redirection::set_out(std::string const&) {}
  bool redirection::set_err(std::string const&) {}
  bool redirection::set_out_err(std::string const&) {}
  bool redirection::permanent() const {}
  bool redirection::activate() const {}
  void redirection::deactivate() const {}
  bool redirection::set(int, std::string, int) {}
  bool redirection::set(int, int) {}

}} // more::io
#endif
