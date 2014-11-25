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


#include <more/io/lock_file.h>
#include <more/io/filesys.h>
#include <more/sys/date.h>
#include <more/diag/errno.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <errno.h>
#include <cstring>              // for strerror()

namespace more {
namespace io {

  infinite_tag const infinite = infinite_tag();

  void
  lock_file::set_file_name(std::string path)
  {
      if (m_path != path) {
	  if (access() != access_none)
	      release();
	  m_path = path;
      }
  }

  bool
  lock_file::adjust(access_t acc)
  {
      if (acc == m_access)
	  return true;

      switch (acc) {
      case access_w:
	  acc = access_rw;
      case access_r:
      case access_rw:
      case access_none:
	  break;
      default:
	  throw std::out_of_range("more::io::lock_file::adjust_access: "
				  "Invalid access for lock file.");
      }

      // Set up the file names
      if (m_path.empty())
	  throw std::logic_error("more::io::lock_file::adjust_access: "
				 "Missing file name.");
      long pid = getpid();
      struct utsname un;  // "struct" was needed on Alpha.
      if (uname(&un) == -1)
	  throw std::runtime_error("more::io::lock_file::adjust_access: "
				   "Could not obtain node name.");
      std::string unique_name;
      {
	  std::ostringstream oss;
	  oss << m_path << '@' << un.nodename << '!' << pid;
	  unique_name = oss.str();
      }
      std::string excl_name = m_path + "~";
      std::string ro_name = m_path;

      if (m_access == access_rw) {
	  // rw --> ro, none
	  if (acc == access_r) {
	      std::ofstream os(ro_name.c_str());
	      os << 1 << '\n';
	      if (os.fail())
		  return false;
	  }
	  else {
	      assert(acc == access_none);
	      remove_node(unique_name);
	  }
	  remove_node(excl_name);
	  m_access = acc;
	  return true;
      }

      // Create the unique file and put some useful info in it
      if (m_access == access_none) {
	  std::ostringstream oss;
	  oss << "# Lock file\ncreation_time='"
	      << more::sys::date(more::sys::current_time()) << "'\n";
	  oss << "nodename=" << un.nodename << '\n';
	  oss << "pid=" << pid << '\n';
	  // Using O_EXCL as extra safety if we failed to create a
	  // unique name.
	  int fd = open(unique_name.c_str(),
			O_WRONLY | O_CREAT | O_EXCL, 0644);
	  if (fd == -1) {
	      // May also happend if the same program is accessing the
	      // same lock file.  Just deny access.
	      return false;
	  }
	  write(fd, oss.str().c_str(), oss.str().size());
	  close(fd);
      }

      // Acquire a RW lock.
      if (link(unique_name.c_str(), excl_name.c_str()) != 0) {
	  if (errno != EEXIST) {
	      std::cerr << "more::io::lock_file::adjust_access: warning: "
			<< "Failed to create " << m_path << ": "
			<< strerror(errno) << std::endl;
	  }
	  if (m_access == access_none)
	      remove_node(unique_name);
	  return false;
      }

      // Check for RO clients
      int rd_cnt = 0;
      {
	  std::ifstream is(ro_name.c_str());
	  if (is.good()) {
	      is >> rd_cnt;
	      if (is.fail())
		  throw std::runtime_error(
		      "more::io::lock_file::adjust_access: "
		      "Bad content of lock file " + ro_name + ".");
	  }
      }

      if (acc == access_none) {
	  // ro --> none
	  if (--rd_cnt == 0)
	      remove_node(ro_name);
	  else {
	      std::ofstream os(ro_name.c_str());
	      os << rd_cnt;
	  }
	  remove_node(unique_name);
	  remove_node(excl_name);
	  m_access = access_none;
	  return true;
      }
      else if (acc == access_r) {
	  // none --> ro
	  bool ok = true;
	  {
	      std::ofstream os(ro_name.c_str());
	      os << ++rd_cnt << '\n';
	      if (os.fail())
		  ok = false;
	  }
	  if (ok) {
	      remove_node(excl_name);
	      m_access = access_r;
	      return true;
	  }
	  else {
	      m_access = access_none;
	      remove_node(unique_name);
	      remove_node(excl_name);
	      return false;
	  }
      }
      else {
	  // none, ro --> rw
	  assert(acc == access_rw);
	  if (m_access == access_r)
	      --rd_cnt;
	  if (rd_cnt == 0) {
	      if (m_access == access_r)
		  remove_node(ro_name);
	      m_access = access_rw;
	      return true;
	  }
	  else if (rd_cnt > 0) {
	      if (m_access == access_none)
		  remove_node(unique_name);
	      remove_node(excl_name);
	      return false;
	  }
	  else
	      throw std::runtime_error("more::io::lock_file::adjust_access: "
				       "Lock reader count is of " + ro_name
				       + " is out of sync.");
      }
  }

  bool
  lock_file::aquire(access_t acc, infinite_tag, double dt)
  {
      while (!adjust(acc))
	  sys::sleep(dt);
      return true;
  }

  bool
  lock_file::aquire(access_t acc, double t, double dt)
  {
      while (t > 0.0) {
	  if (adjust(acc))
	      return true;
	  if (t <= dt) {
	      sys::sleep(t);
	      break;
	  }
	  sys::sleep(dt);
	  t -= dt;
      }
      return adjust(acc);
  }

  void
  lock_file::lock(std::string path, bool blocking)
  {
      set_file_name(path);
      if (blocking)
	  aquire(access_rw, infinite);
      else
	  adjust(access_rw);
  }

  void
  lock_file::release(access_t acc)
  {
      for (int i = 0; i < 20; ++i) {
	  if (adjust(m_access & ~acc))
	      return;
	  sys::sleep(0.2);
      }
      std::cerr << "** Can not release lock " << m_path
		<< "-- still trying.\n"
		<< "** You may have to terminate this program and/or fix "
		<< "the lock."
		<< std::endl;
      for (;;) {
	  if (adjust(m_access & ~acc))
	      return;
	  sys::sleep(1);
      }
  }

}}
