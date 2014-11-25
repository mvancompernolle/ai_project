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

#include <more/bits/conf.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>              // for abort()

namespace more {
  char const* package = MORE_CONF_PACKAGE_NAME;
  char const* version = MORE_CONF_PACKAGE_VERSION;

  bool
  parse_version(char const* str, int& i0, int& i1, int& i2)
  {
      char ch0 = 0, ch1 = 0;
      std::istringstream iss(str);
      i0 = i1 = i2 = 0;
      iss >> i0 >> ch0 >> i1 >> ch1 >> i2;
      return ch0 == '.';
  }

  char const* version_str() { return version; }

  bool
  version_is_at_least(char const* str)
  {
      int i0, i1, i2;
      int j0, j1, j2;
      if (!parse_version(version, i0, i1, i2)) {
	  std::cout << "Configuration error, can't parse own version string."
		    << std::endl;
	  abort();
      }
      if (!parse_version(str, j0, j1, j2))
	  return false;
      if (i0 > j0)
	  return true;
      if (i0 < j0)
	  return false;
      if (i1 > j1)
	  return true;
      if (i1 < j1)
	  return false;
      return i2 >= j2;
  }
}
