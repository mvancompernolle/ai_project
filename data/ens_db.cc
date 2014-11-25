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


#include "ens_db.h"

#include <more/bits/conf.h>
#include <more/io/filesys.h>
#include <more/io/fstream.h>
#include <more/sys/system.h>
#include <more/phys/si.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <memory>
#include <stack>
#include <list>

#include <stdio.h>  // DON'T REMOVE -- some version of libcurl lacks it.
#ifdef MORE_CONF_HAVE_LIBCURL
#  include <curl/types.h>
#  include <curl/curl.h>
#  include <curl/easy.h>
#endif

#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/types.h>
#include <unistd.h>


namespace more {
namespace phys {
namespace ens {

  static const int opt_warning_level = 1;

  std::string
  tolower(std::string s)
  {
      for (std::string::iterator it = s.begin(); it != s.end(); ++it)
	  *it = std::tolower(*it);
      return s;
  }

  bool
  might_have_ensdf_data(nucleus const& nucl)
  {
      if (nucl.n_prot() <= 4 && nucl.n_neut() <= 6)
	  return true;
      double B = semiempirical_binding(nucl);
      double S_neut = B - semiempirical_binding(neighbor(nucl, -1, 0));
      double S_prot = B - semiempirical_binding(neighbor(nucl, 0, -1));
      if (nucl.n_part() < 50)
	  return S_neut >= -1.0*SI::MeV && S_prot >= -3.0*SI::MeV;
      else if (nucl.n_part() < 100)
	  return S_neut >= 0.0 && S_prot >= -2.0*SI::MeV;
      else if (nucl.n_part() < 150)
	  return S_neut >= 1.0*SI::MeV && S_prot >= -1.5*SI::MeV;
      else
	  return S_neut >= 2.0*SI::MeV && S_prot >= -1.0*SI::MeV;
  }

  ensdb::ensdb()
      : download_url("http://ie.lbl.gov/databases/data"),
	m_fetching_enabled(true),
	m_restricted(false),
	m_do_compress(true),
	m_enable_fetch_bounds(true)
  {
      char const *ensdf_path = getenv("MORE_ENSDF_PATH");
      if (ensdf_path) {
	  local_dir = ensdf_path;
	  if (m_restricted &&
	      !io::file_status(local_dir + "/fetch_suid.path").exists())
	      throw std::runtime_error("Bad value of MORE_ENSDF_PATH.");
      }
      else
	  local_dir = MORE_CONF_ENSDF_CACHE_DIR;
  }

  std::string
  ensdb::local_nucleus_dir(more::phys::nucleus const& nuc) const
  {
      std::ostringstream oss;
      oss << std::setfill('0') << std::setw(3) << nuc.n_part();
      std::string nucl_dir = io::absolute_file_name(oss.str(), local_dir);
      return nucl_dir;
  }

  std::string
  ensdb::nucleus_url(more::phys::nucleus const& nuc) const
  {
      std::ostringstream oss;
      oss << std::setfill('0') << std::setw(3) << nuc.n_part() << "/"
	  << nuc.name_AAASy() << "/" << nuc.name_AAASy() << ".ens";
      return download_url + "/" + oss.str();
  }

  std::string
  ensdb::fetch_file(more::phys::nucleus const& nuc, bool force)
  {
      if (m_enable_fetch_bounds && !might_have_ensdf_data(nuc))
	  return "";
      std::string nuc_name = nuc.name_AAASy();
      std::string ens_name = nuc_name + ".ens";
      std::string nuc_dir = local_nucleus_dir(nuc);
      std::string local_fpath = io::absolute_file_name(ens_name, nuc_dir);
      std::string local_fpath_bz2 = local_fpath + ".bz2";
      std::string logfile = io::replace_extension(local_fpath, "-log.html");

      if (!force) {
	  if (io::file_status(local_fpath).exists())
	      return local_fpath;
	  if (io::file_status(local_fpath_bz2).exists())
	      return local_fpath_bz2;

	  // Look for old names.
	  std::ostringstream oss;
	  oss << "ar" << std::setfill('0') << std::setw(3) << nuc.n_part()
	      << tolower(nuc.chemical_name()) << ".ens";
	  std::string old_fpath = io::absolute_file_name(oss.str(), local_dir);
	  if (io::file_status(old_fpath).exists())
	      return old_fpath;
	  old_fpath += ".bz2";
	  if (io::file_status(old_fpath).exists())
	      return old_fpath;
      }

      if (m_fetching_enabled) {
	  bool found_fetcher = false;
	  bool found_suid_fetcher = false;

	  std::string fetcher;

	  if (!m_restricted) {
	      // 1.  Use custom fetcher script from environment.
	      char const *fetcher_env;
	      fetcher_env = std::getenv("MORE_ENSDF_FETCHER");
	      if (!fetcher_env)
		  fetcher_env = std::getenv("MORE_PHYS_FETCHER"); // BW compat
	      if (fetcher_env) {
		  found_fetcher = true;
		  fetcher = fetcher_env;
	      }

	      // 2.  Use custom fetcher script in data dir.
	      if (!found_fetcher) {
		  fetcher = io::absolute_file_name("fetch", local_dir);
		  found_fetcher = io::file_status(fetcher).is_executable();
	      }

	      // 3.  Use SUID fetcher if needed and present.
#ifdef MORE_CONF_ENABLE_ENSDF_SUID
	      if (!found_fetcher) {
		  io::file_status st_dir(local_dir);
		  if (st_dir.uid() != geteuid()) {
		      std::string fetcher_ref =
			  io::absolute_file_name("fetch_suid.path", local_dir);
		      std::ifstream ifs(fetcher_ref.c_str());
		      ifs >> fetcher;
		      if (!ifs.good())
			  throw std::runtime_error(
			      "more::phys::ens::ensdb::fetch_file: "
			      "Could not read from " + fetcher_ref);
		      found_fetcher = found_suid_fetcher = true;
		  }
	      }
#endif
	  }

	  if (found_fetcher) {
	      std::list<std::string> args;
	      if (force)
		  args.push_back("-f");
	      std::ostringstream oss;
	      oss << nuc;
	      args.push_back(oss.str());

	      if (!found_suid_fetcher) {
		  if (m_do_compress)
		      args.push_back(local_fpath_bz2);
		  else
		      args.push_back(local_fpath);
		  io::create_dir_rec(nuc_dir);
	      }
	      std::clog << "Using fetcher " << fetcher << std::endl;
	      int ret = sys::system(fetcher, args.begin(), args.end());
	      std::clog << "Fetcher returned " << ret << std::endl;
	      if (ret != 0)
		  throw std::runtime_error(
		      "more::phys::ens::ensdb::fetch_file: "
		      "Running " + fetcher + " failed.");
	  }
	  else {
#ifdef MORE_CONF_HAVE_LIBCURL
	      std::string nuc_url = nucleus_url(nuc);
	      io::create_dir_rec(nuc_dir);
	      FILE *fp = fopen(local_fpath.c_str(), "w");
	      if (!fp)
		  throw std::runtime_error("Could not open output file.");
	      CURL *curl = curl_easy_init();
	      int st = 0;
	      std::clog << "Fetching " << nuc_url << "." << std::endl;
	      st |= curl_easy_setopt(curl, CURLOPT_URL, nuc_url.c_str());
	      st |= curl_easy_setopt(curl, CURLOPT_FILE, fp);
	      if (st)
		  throw std::logic_error("more::phys::ensdb::fetch_file: "
					 "curl_easy_setopt failed (IE).");
	      if (curl_easy_perform(curl) != 0)
		  throw std::runtime_error("more::phys::ensdb::fetch_file: "
					   "curl_easy_perform failed.");
	      curl_easy_cleanup(curl);
	      fclose(fp);
	      if (m_do_compress) {
		  std::list<std::string> args;
		  args.push_back("-f");
		  args.push_back(local_fpath);
		  more::sys::system("bzip2", args.begin(), args.end());
		  if (io::file_status(local_fpath_bz2).exists())
		      return local_fpath_bz2;
	      }
	      else
		  return local_fpath;
#else
	      throw std::runtime_error(
		  "more::ens::ensdb::fetch_file: Can not fetch ENSDF file "
		  "because libcurl was not detected by configure. As an "
		  "alternative you may set $MORE_ENSDF_FETCHER to a "
		  "custom script ");
#endif
	  }
      }
      if (io::file_status(local_fpath).exists())
	  return local_fpath;
      if (io::file_status(local_fpath_bz2).exists())
	  return local_fpath_bz2;
      return "";
  }

  ensdb* ensdb::s_default = 0;

}}} // more::phys::ens

// Local Variables:
// coding: utf-8
// indent-tabs-mode: t
// End:
