//  Copyright (C) 2000--2009  Petter Urkedal
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
#include <more/io/iomanip.h>
#include <stdexcept>
#include <iostream>
#include <cwchar>
#ifdef MORE_CONF_HAVE_CURSES_H
#  include <curses.h>
#endif
#ifdef MORE_CONF_HAVE_TERM_H
#  include <term.h>
#endif
#include "ios_storage.h"

namespace more {
namespace io {

  std::ostream&
  operator<<(std::ostream& os, setwp const& x)
  {
      return os << std::setw(x.width)
		<< std::setprecision(x.precision);
  }

  std::ostream&
  operator<<(std::ostream& os, osfill const& x)
  {
      for(int i = 0; i < x.width; ++i) os.put(x.ch);
      return os;
  }

  std::ostream&
  operator<<(std::ostream& os, oscenter const& x)
  {
      const char ch = x.ch;
      const int n = x.str.size();
      const int w = x.width;
      int bpad = (w - n)/2;
      int epad = w - n - bpad;
      while(bpad--) os.put(ch);
      os << x.str;
      while(epad--) os.put(ch);
      return os;
  }

  char const*
  terminfo_str(char const* cap)
  {
#ifdef MORE_CONF_HAVE_TIGETSTR
      // I assume the tigetstr function does not modify 'cap' even if
      // the prototype lacks 'const'.  Otherwise, we'll have to copy.
      char const* str = tigetstr(const_cast<char*>(cap));
      if (str == (char const*)-1)
	  std::cerr << "more::io::terminfo_str: " << cap
		    << " is not a string capability." << std::endl;
// 	  throw std::runtime_error("more::io::terminfo_str: "
// 				   +std::string(cap)
// 				   +" is not a string capability.");
      return str;
#else
      return 0;
#endif
  }

  char const* ti_bold = 0;
  char const* ti_sgr0 = 0;
  char const* ti_smul = 0;
  char const* ti_rmul = 0;
  char const* ti_sitm = 0;
  char const* ti_ritm = 0;
  char const* ti_smso = 0;
  char const* ti_rmso = 0;

  void
  set_tslang(std::ios_base& iosb, tslang_t tslang)
  {
      bits::storage(iosb)->tslang = tslang;
      switch (tslang) {
#ifdef MORE_CONF_HAVE_TIGETSTR
      case tslang_terminfo:
	  setupterm((char*)0, 2, (int*)0);
	  ti_bold = terminfo_str("bold");
	  ti_sgr0 = terminfo_str("sgr0");
	  ti_smul = terminfo_str("smul");
	  ti_rmul = terminfo_str("rmul");
	  ti_sitm = terminfo_str("sitm");
	  ti_ritm = terminfo_str("ritm");
	  ti_smso = terminfo_str("smso");
	  ti_rmso = terminfo_str("rmso");
	  break;
#endif
      default:
	  break;
      }
  }

  tslang_t
  tslang(std::ios_base& iosb)
  {
      return bits::storage(iosb)->tslang;
  }

  bool has_bold(std::ios_base& iosb)
  {
      bits::ios_storage* ioss = bits::storage(iosb);
      switch (ioss->tslang) {
      case tslang_terminfo:
	  return ti_bold != 0;
      case tslang_html:
	  return true;
      default:
	  return false;
      }
  }

  bool has_italic(std::ios_base& iosb)
  {
      bits::ios_storage* ioss = bits::storage(iosb);
      switch (ioss->tslang) {
      case tslang_terminfo:
	  return ti_sitm != 0;
      case tslang_html:
	  return true;
      default:
	  return false;
      }
  }

  bool has_underline(std::ios_base& iosb)
  {
      bits::ios_storage* ioss = bits::storage(iosb);
      switch (ioss->tslang) {
      case tslang_terminfo:
	  return ti_smul != 0;
      case tslang_html:
	  return true;
      default:
	  return false;
      }
  }

  bool has_standout(std::ios_base& iosb)
  {
      bits::ios_storage* ioss = bits::storage(iosb);
      switch (ioss->tslang) {
      case tslang_terminfo:
	  return ti_smso != 0;
      default:
	  return false;
      }
  }

  bool has_scripts(std::ios_base& iosb)
  {
      bits::ios_storage* ioss = bits::storage(iosb);
      switch (ioss->tslang) {
      case tslang_html:
      case tslang_latex_math:
	  return true;
      default:
	  return false;
      }
  }

  bool has_rec_scripts(std::ios_base& iosb)
  {
      bits::ios_storage* ioss = bits::storage(iosb);
      switch (ioss->tslang) {
      case tslang_html:
      case tslang_latex_math:
	  return true;
      default:
	  return false;
      }
  }


  template<typename Char, typename Traits>
    std::basic_ostream<Char, Traits>&
    set_bold(std::basic_ostream<Char, Traits>& os)
    {
	using namespace bits;
	ios_storage* ioss = storage(os);
	switch (ioss->tslang) {
	case tslang_none:
	    break;
	case tslang_terminfo:
	    if (ti_bold)
		os << ti_bold;
	    ioss->style |= ios_storage::style_bold;
	    break;
	case tslang_html:
	    os << "<b>";
	    break;
	default:
	    break;
	}
	return os;
    }

  template<typename Char, typename Traits>
    std::basic_ostream<Char, Traits>&
    clr_bold(std::basic_ostream<Char, Traits>& os)
    {
	using namespace bits;
	ios_storage* ioss = storage(os);
	switch (ioss->tslang) {
	case tslang_none:
	    break;
	case tslang_terminfo:
	    if (ti_sgr0)
		os << ti_sgr0;
	    // Restore other styles
	    ioss->style &= ~ios_storage::style_bold;
	    break;
	case tslang_html:
	    os << "</b>";
	    break;
	default:
	    break;
	}
	return os;
    }

  template<typename Char, typename Traits>
    std::basic_ostream<Char, Traits>&
    set_underline(std::basic_ostream<Char, Traits>& os)
    {
	using namespace bits;
	ios_storage* ioss = storage(os);
	switch (ioss->tslang) {
	case tslang_none:
	    break;
	case tslang_terminfo:
	    {
		if (ti_smul)
		    os << ti_smul;
		ioss->style |= ios_storage::style_underline;
	    }
	    break;
	case tslang_html:
	    os << "<u>";
	    break;
	default:
	    break;
	}
	return os;
    }

  template<typename Char, typename Traits>
    std::basic_ostream<Char, Traits>&
    clr_underline(std::basic_ostream<Char, Traits>& os)
    {
	using namespace bits;
	ios_storage* ioss = storage(os);
	switch (ioss->tslang) {
	case tslang_none:
	    break;
	case tslang_terminfo:
	    {
		if (ti_rmul)
		    os << ti_rmul;
		ioss->style &= ~ios_storage::style_underline;
	    }
	    break;
	case tslang_html:
	    os << "</u>";
	    break;
	default:
	    break;
	}
	return os;
    }

  template<typename Char, typename Traits>
    std::basic_ostream<Char, Traits>&
    set_italic(std::basic_ostream<Char, Traits>& os)
    {
	using namespace bits;
	ios_storage* ioss = storage(os);
	switch (ioss->tslang) {
	case tslang_none:
	    break;
	case tslang_terminfo:
	    {
		if (ti_sitm)
		    os << ti_sitm;
		ioss->style |= ios_storage::style_italic;
	    }
	    break;
	case tslang_html:
	    os << "<i>";
	    break;
	default:
	    break;
	}
	return os;
    }

  template<typename Char, typename Traits>
    std::basic_ostream<Char, Traits>&
    clr_italic(std::basic_ostream<Char, Traits>& os)
    {
	using namespace bits;
	ios_storage* ioss = storage(os);
	switch (ioss->tslang) {
	case tslang_none:
	    break;
	case tslang_terminfo:
	    {
		if (ti_ritm)
		    os << ti_ritm;
		ioss->style &= ~ios_storage::style_italic;
	    }
	    break;
	case tslang_html:
	    os << "</i>";
	    break;
	default:
	    break;
	}
	return os;
    }

  template<typename Char, typename Traits>
    std::basic_ostream<Char, Traits>&
    set_standout(std::basic_ostream<Char, Traits>& os)
    {
	using namespace bits;
	ios_storage* ioss = storage(os);
	switch (ioss->tslang) {
	case tslang_none:
	    break;
	case tslang_terminfo:
	    {
		if (ti_sitm)
		    os << ti_smso;
		ioss->style |= ios_storage::style_standout;
	    }
	    break;
	default:
	    break;
	}
	return os;
    }

  template<typename Char, typename Traits>
    std::basic_ostream<Char, Traits>&
    clr_standout(std::basic_ostream<Char, Traits>& os)
    {
	using namespace bits;
	ios_storage* ioss = storage(os);
	switch (ioss->tslang) {
	case tslang_none:
	    break;
	case tslang_terminfo:
	    {
		if (ti_ritm)
		    os << ti_rmso;
		ioss->style &= ~ios_storage::style_standout;
	    }
	    break;
	default:
	    break;
	}
	return os;
    }

  template<typename Char, typename Traits>
    std::basic_ostream<Char, Traits>&
    beg_super(std::basic_ostream<Char, Traits>& os)
    {
	using namespace bits;
	ios_storage* ioss = storage(os);
	switch (ioss->tslang) {
	case tslang_html:
	    os << "<sup>";
	    break;
	case tslang_latex_math:
	    os << "^{";
	    break;
	default:
	    break;
	}
	return os;
    }

  template<typename Char, typename Traits>
    std::basic_ostream<Char, Traits>&
    end_super(std::basic_ostream<Char, Traits>& os)
    {
	using namespace bits;
	ios_storage* ioss = storage(os);
	switch (ioss->tslang) {
	case tslang_html:
	    os << "</sup>";
	    break;
	case tslang_latex_math:
	    os << "}";
	    break;
	default:
	    break;
	}
	return os;
    }

  template<typename Char, typename Traits>
    std::basic_ostream<Char, Traits>&
    beg_sub(std::basic_ostream<Char, Traits>& os)
    {
	using namespace bits;
	ios_storage* ioss = storage(os);
	switch (ioss->tslang) {
	case tslang_html:
	    os << "<sub>";
	    break;
	case tslang_latex_math:
	    os << "_{";
	    break;
	default:
	    break;
	}
	return os;
    }

  template<typename Char, typename Traits>
    std::basic_ostream<Char, Traits>&
    end_sub(std::basic_ostream<Char, Traits>& os)
    {
	using namespace bits;
	ios_storage* ioss = storage(os);
	switch (ioss->tslang) {
	case tslang_html:
	    os << "</sub>";
	    break;
	case tslang_latex_math:
	    os << "}";
	    break;
	default:
	    break;
	}
	return os;
    }

  template
    std::basic_ostream< char, std::char_traits<char> >&
    set_bold(std::basic_ostream< char, std::char_traits<char> >&);
  template
    std::basic_ostream< char, std::char_traits<char> >&
    clr_bold(std::basic_ostream< char, std::char_traits<char> >&);
  template
    std::basic_ostream< char, std::char_traits<char> >&
    set_underline(std::basic_ostream< char, std::char_traits<char> >&);
  template
    std::basic_ostream< char, std::char_traits<char> >&
    clr_underline(std::basic_ostream< char, std::char_traits<char> >&);
  template
    std::basic_ostream< char, std::char_traits<char> >&
    set_italic(std::basic_ostream< char, std::char_traits<char> >&);
  template
    std::basic_ostream< char, std::char_traits<char> >&
    clr_italic(std::basic_ostream< char, std::char_traits<char> >&);
  template
    std::basic_ostream< char, std::char_traits<char> >&
    set_standout(std::basic_ostream< char, std::char_traits<char> >&);
  template
    std::basic_ostream< char, std::char_traits<char> >&
    clr_standout(std::basic_ostream< char, std::char_traits<char> >&);
  template
    std::basic_ostream< char, std::char_traits<char> >&
    beg_super(std::basic_ostream< char, std::char_traits<char> >&);
  template
    std::basic_ostream< char, std::char_traits<char> >&
    end_super(std::basic_ostream< char, std::char_traits<char> >&);
  template
    std::basic_ostream< char, std::char_traits<char> >&
    beg_sub(std::basic_ostream< char, std::char_traits<char> >&);
  template
    std::basic_ostream< char, std::char_traits<char> >&
    end_sub(std::basic_ostream< char, std::char_traits<char> >&);

  template
    std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&
    set_bold(std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&);
  template
    std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&
    clr_bold(std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&);
  template
    std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&
    set_underline(std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&);
  template
    std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&
    clr_underline(std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&);
  template
    std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&
    set_italic(std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&);
  template
    std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&
    clr_italic(std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&);
  template
    std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&
    set_standout(std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&);
  template
    std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&
    clr_standout(std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&);
  template
    std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&
    beg_super(std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&);
  template
    std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&
    end_super(std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&);
  template
    std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&
    beg_sub(std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&);
  template
    std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&
    end_sub(std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&);

}}
