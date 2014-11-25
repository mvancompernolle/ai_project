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


#include <more/io/sourceref.h>

#ifndef stdin
namespace more {
namespace io {
  sourceref::stdin_tag const sourceref::stdin = sourceref::stdin_tag();
}}
#endif

#include <iostream>
#include <stdexcept>

namespace more {
namespace io {

  void
  sourceref::newline(int n)
  {
      if (is_unknown())
	  throw std::logic_error("more::sourceref::newline: Unknown location.");
      m_line += n;
      m_column = 0;
  }

  template<typename Char, typename Traits>
    std::basic_ostream<Char, Traits>&
    operator<<(std::basic_ostream<Char, Traits>& os,
	       sourceref const& loc)
    {
	if (loc.is_unknown())
	    os << "[unknown location]";
	else
	    os << loc.file_name() << ':' << loc.line() << ':' << loc.column();
	return os;
    }

  template
    std::basic_ostream<char, std::char_traits<char> >&
    operator<<(std::basic_ostream<char, std::char_traits<char> >&,
	       sourceref const&);

#if 0
  template
    std::basic_ostream<wchar_t, std::char_traits<wchar_t> >&
    operator<<(std::basic_ostream<wchar_t, std::char_traits<wchar_t> >&,
	       sourceref const&);
#endif
}} // more::io
