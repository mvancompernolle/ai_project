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



#include <more/io/filesys.h>
#include <more/sys/time.h>
#include <more/sys/date.h>
#include <more/gen/functional.h>
#include <more/io/directory.h>
#include <new>
#include <algorithm>
#include <sstream>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <string>

#include <iostream>
#include <iomanip>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <glob.h>
#include <utime.h>


namespace more {
namespace io {

  std::string
  link_redirection_name(std::string const& path)
  {
      for (int n_buf = 256; n_buf < 10000; n_buf *= 2) {
	  char *p_buf = new char[n_buf];
	  int n = readlink(path.c_str(), p_buf, n_buf);
	  if (n == -1) {
	      delete[] p_buf;
	      return "";
	  }
	  if (n < n_buf) {
	      std::string res(p_buf, n);
	      delete[] p_buf;
	      return res;
	  }
	  delete[] p_buf;
      }
      return "";
  }

  bool
  copy_node(std::string const& oldpath, std::string const& newpath)
  {
      // Open files
      struct stat st;
      if (stat(oldpath.c_str(), &st) != 0)
	  return false;
      if (S_ISREG(st.st_mode)) {
	  int in = open(oldpath.c_str(), O_RDONLY);
	  if (in == -1)
	      return false;
	  int out = creat(newpath.c_str(), st.st_mode);
	  if (out == -1) {
	      close(in);
	      return false;
	  }

	  // Copy
	  char buf[0x2000];
	  ssize_t n;
	  while ((n = read(in, buf, 0x2000)) == 0x2000)
	      if (write(out, buf, 0x2000) != 0x2000) {
		  close(in);
		  close(out);
		  return false;
	      }
	  if (n == -1 || (n > 0 && write(out, buf, n) != n)) {
	      close(in);
	      close(out);
	      return false;
	  }

	  // Close
	  close(in);
	  close(out);
      }
      else if (S_ISDIR(st.st_mode)) {
	  if (::mkdir(newpath.c_str(), st.st_mode) != 0)
	      return false;
      }
      else if (S_ISFIFO(st.st_mode)) {
	  if (::mkfifo(newpath.c_str(), st.st_mode) != 0)
	      return false;
      }
      else if (S_ISLNK(st.st_mode)) {
	  std::string ln = link_redirection_name(oldpath);
	  if (ln.empty())
	      return false;
	  ::symlink(ln.c_str(), newpath.c_str());
      }
      else {
	  // if CHR, BLK, SOCK?
	  if (::mknod(newpath.c_str(), st.st_mode, st.st_dev) != 0)
	      return false;
      }

      // Fix file properties
      struct utimbuf ut;
      ut.actime = st.st_atime;
      ut.modtime = st.st_mtime;
      utime(newpath.c_str(), &ut);
      chown(newpath.c_str(), st.st_uid, st.st_gid);
      return true;
  }

  bool
  rename_node(std::string const& oldpath, std::string const& newpath)
  {
      if (::rename(oldpath.c_str(), newpath.c_str()) == -1) {
	  if (errno == EXDEV)
	      if (copy_node(oldpath, newpath))
		  return remove_node(oldpath);
	  return false;
      }
      return true;
  }

  bool
  remove_node(std::string const& path)
  {
      return ::remove(path.c_str()) == 0;
  }

  bool
  remove_node_rec(std::string const& path)
  {
      file_status st(path);
      if (!st.good())
	  return false;
      if (st.is_directory() && !st.is_symbolic_link()) {
	  directory d(path);
	  for (directory::iterator it = d.begin(); it != d.end(); ++it)
	      // NOTE!
	      // It's EXTREMELY important to exclude ".."!!!
	      if (*it != "." && *it != "..") {
		  // What if a character gets accidentally typed into
		  // the above ".." string?  Or if operator!= is buggy?
		  // Be paranoid:
		  if ((*it).size() == 2
		      && (*it)[0] == '.' && (*it)[1] == '.')
		      throw std::logic_error(
			  "more::remove_node_rec: internal error");
		  // TODO. Add a some more paranoia here.
		  if (!remove_node_rec(absolute_file_name(*it, path)))
		      return false;
	      }
      }
      return remove_node(path);
  }

  bool
  copy_node_rec(std::string const& srcpath, std::string const& dstpath)
  {
      file_status st(srcpath);
      if (!st.good())
	  return false;
      if (!copy_node(srcpath, dstpath))
	  return false;
      if (st.is_directory() && !st.is_symbolic_link()) {
	  directory dir(srcpath);
	  for (directory::iterator it = dir.begin(); it != dir.end(); ++it)
	      if (*it != "." && *it != "..")
		  if (!copy_node_rec(absolute_file_name(*it, srcpath),
				     absolute_file_name(*it, dstpath)))
		      return false;
      }
      return true;
  }

  namespace {
    struct file_remover_type
	: std::list<std::string>
    {
	~file_remover_type()
	{
	    for (iterator it = begin(); it != end(); ++it)
		remove_node(*it);
	}
    } file_remover;

    struct file_rec_remover_type
	: std::list<std::string>
    {
	~file_rec_remover_type()
	{
	    for (iterator it = begin(); it != end(); ++it)
		remove_node_rec(*it);
	}
    } file_rec_remover;
  }

  void
  remove_node_at_exit(std::string const& path)
  {
      file_remover.push_front(path);
  }

  void
  remove_node_rec_at_exit(std::string const& path)
  {
      file_rec_remover.push_front(path);
  }

  bool
  grep_string(std::string const& path, std::string const& str)
  {
      std::ifstream is(path.c_str());
      while (is.good()) {
	  std::string ln;
	  getline(is, ln);
	  if (ln.find(str) != std::string::npos)
	      return true;
      }
      return false;
  }

//   file_status::file_status()
//       : st(0), m_err(-1) {}

  file_status::file_status(std::string file_name)
      : m_stat(new struct stat)
  {
      file_name = directory_as_file_name(file_name);
      std::memset(m_stat, 0, sizeof(struct stat));
      // NOTE! Its important that this is lstat for the sake of
      // remove_node_rec.  May however include an option to select
      // which stat function to call.
      if (lstat(file_name.c_str(), static_cast<struct stat*>(m_stat)) != 0)
	  m_err = errno;
      else
	  m_err = 0;
  }

  file_status::~file_status() { delete static_cast<struct stat*>(m_stat); }

  int
  file_status::user_permissions() const
  {
      struct stat const* st = static_cast<struct stat const*>(m_stat);
      if (getuid() == st->st_uid)
	  return (st->st_mode >> 16) & 7;
      else if (getgid() == st->st_gid)
	  return (st->st_mode >> 8) & 7;
      else
	  return st->st_mode & 7;
  }

  int
  file_status::permissions() const
  {
      return static_cast<struct stat*>(m_stat)->st_mode & 07777;
  }

  unsigned int
  file_status::uid() const
  {
      return static_cast<struct stat*>(m_stat)->st_uid;
  }

  unsigned int
  file_status::gid() const
  {
      return static_cast<struct stat*>(m_stat)->st_gid;
  }

  bool
  file_status::is_regular_file() const
  {
      return static_cast<struct stat*>(m_stat)->st_mode & S_IFREG;
  }

  bool
  file_status::is_directory() const
  {
      return static_cast<struct stat*>(m_stat)->st_mode & S_IFDIR;
  }

  bool
  file_status::is_symbolic_link() const
  {
      return static_cast<struct stat*>(m_stat)->st_mode & S_IFLNK;
  }

  double
  file_status::atime() const
  {
      return static_cast<struct stat*>(m_stat)->st_atime;
  }

  double
  file_status::mtime() const
  {
      return static_cast<struct stat*>(m_stat)->st_mtime;
  }


  // scsh style file-name manipulation

  bool
  is_file_name_directory(std::string fname)
  {
      return fname.size() == 0 || fname[fname.size()-1] == '/';
  }

  bool
  is_file_name_non_directory(std::string fname)
  {
      return fname.size() == 0 || fname[fname.size()-1] != '/';
  }

  std::string
  file_name_as_directory(std::string fname)
  {
      if (fname.size() == 0)
	  return "/";
      if (fname == ".")
	  return "";
      else if (*--fname.end() == '/')
	  return fname;
      else
	  return fname+'/';
  }

  std::string
  directory_as_file_name(std::string fname)
  {
      if (fname.size() == 0)
	  return ".";
      else if (fname[fname.size()-1] == '/') {
	  if (fname.size() == 1)
	      return fname;
	  else
	      return fname.substr(0, fname.size()-1);
      }
      else
	  return fname;
  }

  bool
  is_file_name_absolute(std::string fname)
  {
      return fname.size() == 0
	  || fname[0] == '/'
	  || fname[0] == '~';
  }

  std::string
  file_name_directory(std::string fname)
  {
      fname = directory_as_file_name(fname);
      std::string::size_type i = fname.rfind('/');
      if (i == std::string::npos)
	  return "";
      else
	  return fname.substr(0, i+1);
  }

  std::string
  file_name_nondirectory(std::string fname)
  {
      fname = directory_as_file_name(fname);
      std::string::size_type i = fname.rfind('/');
      if (i == std::string::npos)
	  return fname;
      else
	  return fname.substr(i+1);
  }

  std::pair<std::string, std::string>
  split_file_name(std::string fname)
  {
      std::string::size_type i = fname.rfind('/');
      if (i == std::string::npos)
	  return std::make_pair(std::string(), fname);
      else
	  return std::make_pair(fname.substr(0, i+1),
				fname.substr(i+1));
  }

  std::string
  path_list_to_file_name(const_list_of_string_iterator first,
			 const_list_of_string_iterator past_end,
			 std::string dir)
  {
      std::string fname = file_name_as_directory(dir);
      while (first != past_end) {
	  fname.append(1, '/');
	  fname.append(*first);
	  ++first;
      }
      return fname;
  }

  std::string
  file_name_extension(std::string fname)
  {
      std::string::size_type i = fname.rfind('.');
      if (i == std::string::npos || i == 0 || fname[i - 1] == '/'
	  || fname.find('/', i) != std::string::npos)
	  return "";
      else
	  return fname.substr(i);
  }

  std::string
  file_name_sans_extension(std::string fname)
  {
      std::string::size_type i = fname.rfind('.');
      if (i == std::string::npos || i == 0 || fname[i - 1] == '/'
	  || fname.find('/', i) != std::string::npos)
	  return fname;
      else
	  return fname.substr(0, i);
  }

  //parse_file_name(std::string fname, XXX)

  std::string
  replace_extension(std::string fname, std::string ext)
  {
      return file_name_sans_extension(fname) + ext;
  }

  std::string
  resolve_file_name(std::string fname, std::string dir)
  {
      if (fname.size() == 0 || fname[0] == '/')
	  return fname;
      else if (fname[0] == '~')
	  return home_dir() + fname.substr(1);
      else
	  return file_name_as_directory(dir) + fname;
  }

  std::string
  absolute_file_name(std::string fname, std::string dir)
  {
      if (fname.size() == 0 || fname[0] == '/') // is absolute
	  return fname;
      else if (!dir.empty() && dir != ".")
	  return file_name_as_directory(dir) + fname;
      else
	  return file_name_as_directory(current_directory()) + fname;
  }

  std::string
  relative_file_name(std::string fpath, std::string dir)
  {
      std::size_t j = 0;
      for (std::size_t i = 0;; ++i) {
	  if (i == dir.size()) {
	      if (i == fpath.size())
		  return ".";
	      else if (fpath[i] == '/') {
		  if (++i == fpath.size())
		      return ".";
		  else
		      return fpath.substr(i);
	      }
	      else
		  break;
	  }
	  else if (i == fpath.size()) {
	      if (dir[i] == '/') {
		  j = i + 1;
		  if (j == dir.size())
		      return ".";
	      }
	      break;
	  }
	  else if (fpath[i] != dir[i])
	      break;
	  else if (fpath[i] == '/')
	      j = i + 1;
      }
      // j is the index of the start of the first differing component.
      std::string newpath;
      if (j != dir.size())
	  newpath = "../";
      for (std::size_t i = j; i < dir.size() - 1; ++i)
	  if (dir[i] == '/')
	      newpath += "../";
      if (j >= fpath.size())
	  return newpath;
      else
	  return newpath + fpath.substr(j);
  }

  std::string
  current_directory()
  {
      for (std::size_t size = 128; size < 100000; size *= 2) {
	  char* buf = new char[size];
	  if (getcwd(buf, size)) {
	      std::string res = buf;
	      delete buf;
	      return res;
	  }
	  delete buf;
      }
      throw std::runtime_error("more::io::current_directory: "
			       "Could not get current directory.");
  }


  std::string
  home_dir(/* XXX std::string user = "" */)
  {
      char const* c_str = getenv("HOME");
      if (!c_str)
	  throw std::runtime_error("more::home_dir: Could not get "
				   "path from environment variable HOME.");
      return c_str;
  }

  std::string
  home_file(std::string fname)
  {
      return home_dir() + '/' + fname;
  }

  bool
  create_dir(std::string path)
  {
      return ::mkdir(path.c_str(), 0755) == 0;
  }

  bool
  create_dir_rec(std::string path)
  {
      if (path.empty())
	  return true;
      if (!file_status(path).good()) {
	  std::string dir = file_name_directory(path);
	  if (!create_dir_rec(dir))
	      return false;
	  return ::mkdir(path.c_str(), 0755) == 0;
      }
      return true;
  }

  namespace {
    void
    rand_extension(std::string& str)
    {
	// Extend with a 60 bit random pattern.
	for (int j = 0; j < 10; ++j) {
	    int x = rand()%64;
	    if (x < 10)
		str += '0' + x;
	    else if ((x -= 10) < 26)
		str += 'a' + x;
	    else if ((x -= 26) < 26)
		str += 'A' + x;
	    else if (x == 26)
		str += '_';
	    else if (x == 27)
		str += '-';
	    else {
		assert(0);
	    }
	}
    }
  }

  bool
  reserve_tmp_file(std::string& name, bool remove_p)
  {
      srand(clock());
      // Adding 8 bits to the randomness of the name is more likely to
      // give a unique name than trying 256 different names, so for
      // optimal speed, use more bits in the file name.
      // rand_extension most likely gives a unique name at first try,
      // but since the srand(clock()) and rand() combination is not
      // that good, do a few iterations, as well.
      for (int i = 0; i < 16; ++i) {
	  std::string nm = name;
	  nm += '-';
	  rand_extension(nm);
	  int fd = open(nm.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
	  if (fd != -1) {
	      close(fd);
	      name = nm;
	      if (remove_p)
		  remove_node_at_exit(nm);
	      return true;
	  }
      }
      return false;
  }

  bool
  create_tmp_dir(std::string& name)
  {
      srand(clock());
      for (int i = 0; i < 16; ++i) {
	  std::string nm = name;
	  nm += '-';
	  rand_extension(nm);
	  if (::mkdir(nm.c_str(), 0700) != -1) {
	      name = nm;
	      remove_node_rec_at_exit(nm);
	      return true;
	  }
      }
      return false;
  }


  //
  //  file_glob_list
  //

  static glob_t g_empty_glob;

  file_glob_list::file_glob_list(std::string patt)
      : m_data(new glob_t)
  {
      static const int flags = GLOB_ERR;
      switch (glob(patt.c_str(), flags, 0, (glob_t*)m_data)) {
	case 0:
	  break;
	case GLOB_NOSPACE:
	  throw std::bad_alloc();
	case GLOB_NOMATCH:
	  g_empty_glob.gl_pathv = 0;
	  g_empty_glob.gl_pathc = 0;
	  m_data = &g_empty_glob;
	  break;
	case GLOB_ABORTED:
	default:
	  m_data = 0;
	  break;
      }
  }

  file_glob_list::~file_glob_list()
  {
      if (m_data && m_data != &g_empty_glob) {
	  globfree((glob_t*)m_data);
	  delete (glob_t*)m_data;
      }
  }

  file_glob_list::const_iterator
  file_glob_list::begin() const
  {
      if (!m_data)
	  throw std::logic_error(
	      "more::file_glob_list::begin: Invalid glob list.");
      return ((glob_t*)m_data)->gl_pathv;
  }

  file_glob_list::const_iterator
  file_glob_list::end() const
  {
      if (!m_data)
	  throw std::logic_error(
	      "more::file_glob_list::end: Invalid glob list.");
      return ((glob_t*)m_data)->gl_pathv + ((glob_t*)m_data)->gl_pathc;
  }

}
}
