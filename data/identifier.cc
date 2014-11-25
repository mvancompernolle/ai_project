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


#include <more/gen/identifier.h>
#include <more/gen/string.h>
#include <more/io/syncstream.h>
#include <string>

namespace more {
namespace gen {

  identifier::identifier(char const* str)
      : m_str(struniq_copy(str)) {}

  void
  identifier::sync(io::syncstream& sio)
  {
      if (sio.is_input()) {
	  bool def_p;
	  sio | def_p;
	  if (def_p) {
	      std::string str;
	      sio | str;
	      m_str = struniq_copy(str.c_str());
	  }
	  else
	      m_str = 0;
      }
      else {
	  if (m_str) {
	      bool def_p = true;
	      std::string str = m_str;
	      sio | def_p | str;
	  }
	  else {
	      bool def_p = false;
	      sio | def_p;
	  }
      }
  }

  std::ostream&
  operator<<(std::ostream& os, identifier const& idr)
  {
//       if (!idr.is_defined())
// 	  throw std::logic_error("more::gen::operator<<"
// 				 "(std::ostream&, identifier const&): "
// 				 "Undefined identifier.");
      if (!idr.is_defined())
	  os << "#[undefined identifier]";
      os << idr.c_str();
      return os;
  }

}}
