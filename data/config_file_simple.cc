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


#include <more/io/config_file.h>
#include <more/io/sourceref.h>
#include <iostream>
#include <more/io/iomanip.h>
#include <fstream>
#include <assert.h>

namespace more {
namespace io {

  namespace {
    char
    scan_to_space(std::istream& is, std::string& str_out)
    {
	for (;;) {
	    if (!is.good())
		return 0;
	    char ch = is.get();
	    if (std::isspace(ch))
		return ch;
	    str_out += ch;
	}
    }

    void
    scan_to_eol(std::istream& is, std::string& str_out)
    {
	for (;;) {
	    if (!is.good())
		break;
	    char ch = is.get();
	    if (ch == '#') {
		while (is.good() && is.get() != '\n')
		    (void)0;
		break;
	    }
	    if (ch == '\n')
		break;
	    str_out += ch;
	}
	std::string::size_type i = str_out.size();
	while (i > 0 && std::isspace(str_out[--i]))
	    (void)0;
	str_out.erase(++i);
    }
  } // NN

  void
  read_config_ksvn(config_tree& t, std::string const& file_name)
  {
      io::sourceref srf(file_name);
      std::ifstream is(file_name.c_str());
      if (!is.good())
	  return;
      for (;;) {
	  char ch;
	  while (std::isspace(ch = is.get()))
	      if (ch == '\n')
		  srf.newline();
	  if (!is.good())
	      break;
	  if (ch == '#')
	      while (is.good() && is.get() != '\n')
		  (void)0;
	  else {
	      is.putback(ch);
	      std::string key;
	      char ch = scan_to_space(is, key);
	      while (is.good() && std::isspace(ch) && ch != '\n')
		  ch = is.get();
	      std::string value(1, ch);
	      if (ch != '\n')
		  scan_to_eol(is, value);
	      t.branches().push_back(key);
	      t.branches().back().branches().push_back(value);
	  }
	  srf.newline();
      }
      assert(is_valid_config_tree(t));
  }

  void
  read_config_kevn(config_tree& t, std::string const& file_name)
  {
      io::sourceref srf(file_name);
      std::ifstream is(file_name.c_str());
      if (!is.good())
	  return;
      for (;;) {
	  char ch;
	  while (std::isspace(ch = is.get()))
	      if (ch == '\n')
		  srf.newline();
	  if (!is.good())
	      break;
	  if (ch == '#') {
	      while (is.good() && is.get() != '\n')
		  (void)0;
	  }
	  else {
	      std::string key;
	      while (is.good() && !std::isspace(ch) && ch != '=') {
		  key += ch;
		  ch = is.get();
	      }
	      while (std::isspace(ch) && ch != '\n')
		  ch = is.get();
	      if (ch != '=') {
		  std::cerr << srf << ": Missing '=' before value.\n";
		  srf.newline();
		  continue;
	      }
	      ch = is.get();
	      while (is.good() && std::isspace(ch) && ch != '\n')
		  ch = is.get();
	      std::string value(1, ch);
	      if (ch != '\n')
		  scan_to_eol(is, value);
	      t.branches().push_back(key);
	      t.branches().back().branches().push_back(value);
	  }
	  srf.newline();
      }
      assert(is_valid_config_tree(t));
  }


}} // more::io
