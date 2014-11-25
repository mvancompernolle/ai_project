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


#include <iostream>
#include "skyrme.h"
#include <more/io/iomanip.h>
#include <more/io/filesys.h>
#include <more/io/directory.h>


namespace more {
namespace phys {

  template<typename T>
    void
    skyrme_interaction<T>::read(std::istream& is)
    {
	getline(is, m_name);
	is >> m_force
	   >> t0 >> x0 >> t1 >> x1 >> t2 >> x2
	   >> t3 >> x3 >> alpha >> w;
	if (isospin_dependent_LS_p())
	    is >> w2;
    }

  template<typename T>
    void
    skyrme_interaction<T>::write(std::ostream& os) const
    {
	os << std::setw(5) << m_force << '\n';
	os << std::setiosflags(std::ios_base::scientific)
	   << io::setwp(13, 6) << t0
	   << io::setwp(13, 6) << x0
	   << io::setwp(13, 6) << t1
	   << io::setwp(13, 6) << x1
	   << io::setwp(13, 6) << t2
	   << io::setwp(13, 6) << x2 << '\n'
	   << io::setwp(13, 6) << t3
	   << io::setwp(13, 6) << x3
	   << io::setwp(13, 6) << alpha
	   << io::setwp(13, 6) << w;
	if(isospin_dependent_LS_p()) os << io::setwp(13, 6) << w2;
	os << std::resetiosflags(std::ios_base::scientific) << std::endl;
    }

  template<typename T>
    void
    skyrme_interaction<T>::sync(io::syncstream& sio)
    {
	int version = 0;
	sio | version | m_force
	    | t0 | x0 | t1 | x1 | t2 | x2 | t3 | x3 | alpha | w;
	if (isospin_dependent_LS_p())  sio | w2;
    }

  template<typename T>
    void
    skyrme_interaction<T>::load(std::string name)
    {
	m_fname = skyrme_name_list::name_to_path_comp(name);
	std::ifstream is;
	name = io::resolve_file_name(m_fname, skyrme_datadir);
	if (io::file_name_extension(name) != ".skyrme")
	    name.append(".skyrme");
	is.open(name.c_str());
	if (is.good())
	    is >> *this;
	if (is.fail())
	    throw std::runtime_error("more::phys::skyrme_interaction: "
				     "Could not read "+name+".");
    }

  template class skyrme_interaction<double>;


  skyrme_name_list::skyrme_name_list()
  {
      if (!s_done_init) {
	  io::directory dirlst(skyrme_datadir);
	  for (io::directory::iterator it = dirlst.begin();
	       it != dirlst.end(); ++it) {
	      int n = it->size();
	      if (n > 7 && it->substr(n - 7) == ".skyrme") {
		  std::ifstream is((skyrme_datadir + "/" + *it).c_str());
		  std::string name;
		  std::getline(is, name);
		  if (!is.good())
		      name = it->substr(0, n-7);
		  s_names.push_back(name);
	      }
	  }
	  s_done_init = true;
      }
  }

  std::string
  skyrme_name_list::name_to_path_comp(std::string name)
  {
      if (name == "SkM⋆" || name == "SkM*")
	  name = "skmx";
      else
	  for (std::size_t i = 0; i < name.size(); ++i)
	      name[i] = std::tolower(name[i]);
      return name;
  }

  bool skyrme_name_list::s_done_init;
  skyrme_name_list::subcontainer skyrme_name_list::s_names;

  std::string skyrme_datadir = MORE_CONF_DATADIR"/more/skyrme";

}
}
