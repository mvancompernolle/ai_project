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


#include <more/lang/compilation.h>
#include <more/io/filesys.h>
#include <more/io/syncstream.h>
#include <more/io/redirection.h>
#include <more/sys/system.h>
#include <more/sys/cfg.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>

namespace more {
namespace lang {

  namespace {
    bool const opt_use_libtool = true;
  }

  using io::resolve_file_name;

  compilation::compilation(std::string const& srcdir,
			   std::string const& builddir)
      : m_srcdir(srcdir),
	m_builddir(builddir),
	m_target_type(target_none),
	m_so_version_scheme(s_so_version_scheme_libtool),
	m_so_version_current(0),
	m_so_version_revision(-1),
	m_so_version_age(0),
	m_interface_added(0),
	m_interface_changed(0)
  {
      if (m_srcdir.empty())
	  throw std::runtime_error("more::lang::compilation::compilation: "
				   "Invalid srcdir.");
      if (m_builddir.empty())
	  throw std::runtime_error("more::lang::compilation::compilation: "
				   "Invalid builddir.");
  }

  compilation::compilation(std::string const& srcdir,
			   std::string const& builddir,
			   std::string const& includedir,
			   std::string const& libdir)
      : m_srcdir(srcdir),
	m_builddir(builddir),
	m_includedir(includedir),
	m_libdir(libdir),
	m_target_type(target_none),
	m_so_version_scheme(s_so_version_scheme_libtool),
	m_so_version_current(0),
	m_so_version_revision(0),
	m_so_version_age(0),
	m_interface_added(0),
	m_interface_changed(0)
  {
      if (m_srcdir.empty())
	  throw std::runtime_error("more::lang::compilation::compilation: "
				   "Invalid srcdir.");
      if (m_builddir.empty())
	  throw std::runtime_error("more::lang::compilation::compilation: "
				   "Invalid builddir.");
  }

  std::string
  compilation::insert_header(std::string const& bname, lang_t lang)
  {
      m_headers.push_back(bname);
      return header_file_name(bname, lang, m_srcdir);
  }

  std::string
  compilation::insert_source(std::string const& bname, lang_t lang,
			     version_type version)
  {
      source_entry_iterator it = m_sources.find(bname);
      if (it == m_sources.end())
	  m_sources.insert(std::make_pair(bname,
					  source_entry(lang, version, true)));
      else if (version > it->second.m_version) {
	  it->second.m_version = version;
	  it->second.m_rebuild_p = true;
      }
      else
	  return "";
      return source_file_name(bname, lang, m_srcdir);
  }

  std::string
  compilation::insert_source(std::string const& bname, lang_t lang)
  {
      m_sources[bname] = source_entry(lang, 0, true);
      return source_file_name(bname, lang, m_srcdir);
  }

  void
  compilation::append_cppflags(std::string const& cppflags)
  {
      m_cppflags += " ";
      m_cppflags += cppflags;
  }

  void
  compilation::append_cflags(std::string const& cflags)
  {
      m_cflags += " ";
      m_cflags += cflags;
  }

  void
  compilation::append_cxxflags(std::string const& cxxflags)
  {
      m_cxxflags += " ";
      m_cxxflags += cxxflags;
  }

  void
  compilation::append_ldflags(std::string const& ldflags)
  {
      m_ldflags += " ";
      m_ldflags += ldflags;
  }

  void
  compilation::set_target_program(std::string const& prg)
  {
      m_target_bname = prg;
      m_target_type = target_program;
      m_target = lang::program_file_name(prg);
      check_cache();
//      return resolve_file_name(m_target, m_builddir);
  }

  void
  compilation::set_target_library(std::string const& bname, bool module_p)
  {
      m_target_bname = bname;
      if (module_p)
	  m_target_type = target_library_dlopen;
      else
	  m_target_type = target_library;
      if (opt_use_libtool)
	  m_target = lang::library_file_name(bname);
      else
	  m_target = bname + ".so";
      check_cache();
//      return resolve_file_name(m_target, m_builddir);
  }

  void
  compilation::sync_state(io::syncstream& sio)
  {
      sio | m_so_version_scheme | m_so_version_current
	  | m_so_version_revision | m_so_version_age;
      if (m_so_version_scheme < 0
	  || m_so_version_scheme > s_so_version_scheme_max)
	  throw std::runtime_error("Unsupported versioning scheme in "
				   "library " + m_target_bname);
      io::sync_pair_associative_container(sio, m_sources);
  }

  bool
  compilation::check_cache()
  {
      io::ifsyncstream sin(io::resolve_file_name(m_target_bname + ".state",
						 m_builddir).c_str());
      if (sin.good()) {
	  sync_state(sin);
	  return true;
      }
      else
	  return false;
  }

  bool
  compilation::libtool(std::string const& cmd, char const* mode)
  {
      // XXX Error messages from gcc are not reported and a non-zero
      // exit status is not reported.
//       std::string cmdl = "cd ";
//       cmdl += m_builddir;
//       cmdl += " && ";
      std::string dir = io::current_directory();
      ::chdir(m_builddir.c_str());
//       cmdl += sys::getcfg(sys::cfg_LIBTOOL);
      std::string cmdl = "libtool";
      cmdl += " --mode=";
      cmdl += mode;
      cmdl += " --tag=CC ";
      cmdl += cmd;
      std::clog << "[more::lang::compilation] " << cmdl << "\n";
      io::redirection redir;
      std::string logfile = mode;
      logfile += ".log";
      redir.set_out_err(logfile);
      redir.activate();
      int st = sys::system(cmdl);
      redir.deactivate();
      if (st == 0) {
	  ::chdir(dir.c_str());
	  return true;
      }
      else {
	  std::clog << "\nError log:\n";
	  std::ifstream is(logfile.c_str());
	  char lastch = '\n';
	  for (;;) {
	      char ch = is.get();
	      if (!is.good())
		  break;
	      if (lastch == '\n')
		  std::clog << "| ";
	      std::clog.put(ch);
	      lastch = ch;
	  }
	  std::clog << "\n";
	  ::chdir(dir.c_str());
	  return false;
      }
  }

  bool
  compilation::make_all()
  {
      bool have_new_p = false;
      bool have_cxx_p = false;
      std::string objs;
      std::string rel_srcdir = io::relative_file_name(m_srcdir, m_builddir);

      for (source_entry_iterator it = m_sources.begin();
	   it != m_sources.end(); ++it) {
	  std::string obj = ltobject_file_name(it->first);
	  objs += ' ';
	  objs += obj;
	  if (it->second.m_lang == lang_cxx)
	      have_cxx_p = true;
	  if (!it->second.m_rebuild_p)
	      continue;
	  have_new_p = true;
	  std::string src = source_file_name(it->first, it->second.m_lang,
					     m_srcdir);
	  more::io::file_status st_src(src);
	  if (!st_src.exists())
	      throw std::runtime_error("more::lang::compilation: "
				       "File "+src+" does not exist.");
	  std::string cmd;
	  switch (it->second.m_lang) {
	    case lang_c:
	      cmd = sys::getcfg(sys::cfg_CC);
	      cmd += " -c";
	      cmd += m_cppflags;
	      cmd += m_cflags;
	      break;
	    case lang_cxx:
	      cmd = sys::getcfg(sys::cfg_CXX);
	      cmd += " -c";
	      cmd += m_cppflags;
	      cmd += m_cxxflags;
	      break;
	    default:
	      throw std::out_of_range("more::lang::compilation: "
				      "Can not compile "
				      + language_name(it->second.m_lang));
	  }
	  cmd += " -o ";
	  cmd += obj;
	  cmd += " ";
	  cmd += source_file_name(it->first, it->second.m_lang, rel_srcdir);
	  if (opt_use_libtool) {
	      if (!libtool_compile(cmd))
		  return false;
	  } else {
	      std::clog << "[more::lang::compilation] " << cmd << std::endl;
	      if (sys::system("cd " + m_builddir + " && " + cmd) != 0)
		  return false;
	  }
      }

      if (have_new_p) {
	  if (m_so_version_scheme == s_so_version_scheme_libtool) {
	      if (m_interface_changed) {
		  ++m_so_version_current;
		  m_so_version_age = 0;
		  m_so_version_revision = 0;
	      }
	      else if (m_interface_added) {
		  ++m_so_version_current;
		  ++m_so_version_age;
		  m_so_version_revision = 0;
	      }
	      else
		  ++m_so_version_revision;
	      m_interface_changed = 0;
	      m_interface_added = 0;
	  }

	  std::ostringstream oss_cmd;
	  oss_cmd << sys::getcfg(have_cxx_p? sys::cfg_CXXLD : sys::cfg_CCLD);
	  if (opt_use_libtool) {
	      if (m_target_type == target_library_dlopen)
		  oss_cmd << " -module";
	      if (m_so_version_scheme == s_so_version_scheme_libtool
		  && (m_target_type == target_library
		      || m_target_type == target_library_dlopen))
		  oss_cmd << " -version-info "
			  << m_so_version_current << ":"
			  << m_so_version_revision << ":"
			  << m_so_version_age;
	      oss_cmd << " -rpath ";
	      if (m_libdir.empty()) {
		  oss_cmd << io::absolute_file_name(m_builddir);
		  oss_cmd << " -no-install";
	      }
	      else
		  oss_cmd << m_libdir;
	      oss_cmd << " -o ";
	      oss_cmd << m_target;
	      oss_cmd << objs;
	      if (!libtool_link(oss_cmd.str()))
		  return false;
	  }
	  else { // This is for GCC. Just testing.
	      // XXX versioning
	      if (m_target_type == target_library_dlopen)
		  oss_cmd << " -shared";
	      oss_cmd << " -o ";
	      oss_cmd << m_target;
	      oss_cmd << objs;
	      std::string cmd = oss_cmd.str();
	      std::clog << "[more::lang::compilation] " << cmd << std::endl;
	      if (sys::system("cd " + m_builddir + " && " + cmd) != 0)
		  return false;
	  }

	  // Write out the new state.  This will also set m_rebuild_p
	  // to false for all files.
	  io::ofsyncstream
	      osync(io::resolve_file_name(m_target_bname + ".state",
					  m_builddir).c_str());
	  sync_state(osync);
      }
      return true;
  }

  bool
  compilation::make_install()
  {
      if (!m_target.empty()) {
	  if (m_libdir.empty())
	      throw std::logic_error("more::lang::compilation::make_install: "
				     "No libdir specified.");
	  if (!io::create_dir_rec(m_libdir))
	      return false;
	  std::ostringstream oss_cmd;
	  oss_cmd << sys::getcfg(m_target_type == target_program
				 ? sys::cfg_INSTALL_PROGRAM
				 : sys::cfg_INSTALL);
	  oss_cmd << " ";
	  oss_cmd << m_target;
	  oss_cmd << " ";
	  oss_cmd << resolve_file_name(m_target, m_libdir);
	  if (!libtool(oss_cmd.str(), "install"))
	      return false;
      }
      if (!m_headers.empty()) {
	  if (m_includedir.empty())
	      throw std::logic_error("more::lang::compilation::make_install: "
				     "includedir is not specified.");
	  if (!io::create_dir_rec(m_includedir))
	      return false;
	  for (string_list::iterator it = m_headers.begin();
	       it != m_headers.end(); ++it) {
	      std::string hdr = header_file_name(*it, lang_c); // XXX
	      std::ostringstream oss_cmd;
	      oss_cmd << sys::getcfg(sys::cfg_INSTALL_DATA);
	      oss_cmd << " ";
	      oss_cmd << hdr;
	      oss_cmd << " ";
	      oss_cmd << resolve_file_name(hdr, m_includedir);
	      if (!libtool(oss_cmd.str(), "install"))
		  return false;
	  }
      }
      return true;
  }

  void
  compilation::source_entry::sync(io::syncstream& sio)
  {
      int lang = m_lang;
      sio | lang;
      m_lang = (lang_t)lang;
      sio | m_version;

      // The state is only written out after all files are compiled, so
      m_rebuild_p = false;
  }


  //
  // File names
  //

  namespace {
    struct lang_info_t
    {
	char const* name;
	char const* ext;
	char const* ext_h;
    } lang_info[lang_end] = {
	{0, 0},
	{"C", ".c"},
	{"C++", ".cc"},
	{"Scheme", ".scm"},
	{"Java", ".java"},
	{"Bourne Shell", ".sh"},
	{"Perl", ".perl"},
	{"FORTRAN 77", ".f77"},
	{"FORTRAN 90", ".f90"}
    };
    char const* c_hdr_ext = ".h";
    char const* cxx_hdr_ext = ".h";
  }

  std::string
  language_name(lang_t lang)
  {
      if (char const* name = lang_info[lang].name)
	  return name;
      else
	  throw std::out_of_range("more::lang::language_name: "
				  "Invalid language enumerator.");
  }

  std::string
  source_file_name(std::string const& bname, lang_t lang,
		   std::string const& dir)
  {
      if (char const* ext = lang_info[lang].ext)
	  return resolve_file_name(bname + ext, dir);
      else
	  throw std::out_of_range("more::lang::source_file_name: "
				  "Invalid language enumerator.");
  }

  std::string
  header_file_name(std::string const& bname, lang_t lang,
		   std::string const& dir)
  {
      switch (lang) {
	case lang_c:
	  return resolve_file_name(bname + c_hdr_ext, dir);
	case lang_cxx:
	  return resolve_file_name(bname + cxx_hdr_ext, dir);
	default:
	  throw std::out_of_range("more::lang::header_file_name: "
				  "Language "+language_name(lang)
				  +" has no header files.");
      }
  }

  std::string
  c_header_file_name(std::string const& bname, std::string const& dir)
  {
      return resolve_file_name(bname + c_hdr_ext, dir);
  }

  std::string
  cxx_header_file_name(std::string const& bname, std::string const& dir)
  {
      return resolve_file_name(bname + cxx_hdr_ext, dir);
  }

  std::string
  c_source_file_name(std::string const& bname, std::string const& dir)
  {
      return resolve_file_name(bname+lang_info[lang_c].ext, dir);
  }

  std::string
  cxx_source_file_name(std::string const& bname, std::string const& dir)
  {
      return resolve_file_name(bname+lang_info[lang_cxx].ext, dir);
  }

  std::string
  ltobject_file_name(std::string const& bname, std::string const& dir)
  {
      return resolve_file_name(bname+".lo", dir);
  }

  std::string
  program_file_name(std::string const& bname, std::string const& dir)
  {
      return resolve_file_name(bname, dir);
  }

  std::string
  library_file_name(std::string const& bname, std::string const& dir)
  {
      return resolve_file_name("lib"+bname+".la", dir);
  }

  std::string
  dlopen_file_name(std::string const& bname, std::string const& dir)
  {
      return resolve_file_name(bname+".so", dir);
  }

}}

// [0] The granulatity of the file modification times are too low for
// fast applications.  Therefore source files are recompiled if they
// are the same age as the object files.  This may cause unnecessary
// recompilation, but the alternative is worse, since a application
// may rewrite the source file quickly after the first compilation and
// ask for a recompilation which will not happen due to the lack of
// time-granularity.
