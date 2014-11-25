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


#include <more/phys/confidence_interval.h>
#include <more/math/math.h>
#include <more/io/iomanip.h>
#include "../io/ios_storage.h"
#include <iostream>
#include <sstream>
#include <assert.h>


namespace more {
namespace phys {

  namespace {
    bool opt_utf8 = true;
    bool opt_utf8_exponent = false;
    char opt_dev_begin = '(';  // '[' or '('
    char opt_dev_end = ')';    // ']' or ')'
  }

  // Format:
  //   22.9(11)       centered at 22.9 with deviation 1.1
  //   20.31(11)e3    centered at 20.31e3 with deviation .11e3=110
  //   1.03(-11,+17)  centered at 1.03, with deviation .11 below and .17 above

  char const*
  tosuper(char ch)
  {
      // These superscripts sucks, but sometimes, esp for terminal
      // output, there is no markup available.
      switch (ch) {
	  // Standard superscripts
      case '0': return "⁰";
      case '1': return "¹";
      case '2': return "²";
      case '3': return "³";
      case '4': return "⁴";
      case '5': return "⁵";
      case '6': return "⁶";
      case '7': return "⁷";
      case '8': return "⁸";
      case '9': return "⁹";
      case '+': return "⁺";
      case '-': return "⁻";
      case '=': return "⁼";
      case '(': return "⁽";
      case ')': return "⁾";
      case 'n': return "ⁿ";
	  // Modifier letters
      case 'l': return "ˡ";
      case 's': return "ˢ";
      case 'x': return "ˣ";
      case 'h': return "ʰ";
      case 'j': return "ʲ";
      case 'r': return "ʳ";
      case 'w': return "ʷ";
      case 'y': return "ʸ";
      default:
	  return 0;
      }
  }

  template<typename T>
    typename confidence_interval<T>::unknown_tag
    confidence_interval<T>::unknown;

  template<typename T>
    typename confidence_interval<T>::infinite_tag
    confidence_interval<T>::infinite;

  template<typename T>
    void
    confidence_interval<T>::
    add_deviation(confidence_interval const& rhs)
    {
	if (!rhs.is_known()) {
	    m_pf_cent = m_pf_s_mi = m_pf_s_pl = pf_unknown;
	    return;
	}
	m_moment = std::max(m_moment, rhs.m_moment);
	if (m_moment == 1) {
	    if (!rhs.dev_below_is_finite())
		m_pf_s_mi = pf_unknown;
	    else if (rhs.dev_below_is_inf())
		m_pf_s_mi = pf_infinite;
	    else
		m_s_mi += rhs.m_s_mi;

	    if (!rhs.dev_above_is_known())
		m_pf_s_pl = pf_unknown;
	    else if (rhs.dev_above_is_inf())
		m_pf_s_pl = pf_infinite;
	    else
		m_s_pl += rhs.m_s_pl;
	}
	else {
	    if (!rhs.dev_below_is_finite())
		m_pf_s_mi = pf_unknown;
	    else if (rhs.dev_below_is_inf())
		m_pf_s_mi = pf_infinite;
	    else
		m_s_mi = pow(pow(m_s_mi, m_moment) +
			     pow(rhs.m_s_mi, m_moment),
			     T(1)/m_moment);

	    if (!rhs.dev_above_is_known())
		m_pf_s_pl = pf_unknown;
	    else if (rhs.dev_above_is_inf())
		m_pf_s_pl = pf_infinite;
	    else
		m_s_pl = pow(pow(m_s_pl, m_moment) +
			     pow(rhs.m_s_pl, m_moment),
			     T(1)/m_moment);
	}
	m_origin |= rhs.m_origin;
	m_aux_flags |= rhs.m_aux_flags;
    }

  template<typename Char, typename Traits, typename T>
    std::basic_ostream<Char, Traits>&
    operator<<(std::basic_ostream<Char, Traits>& os,
	       confidence_interval<T> const& I)
    {
	using math::ifloor;
	using math::iround;

	if (!I.is_known()) {
	    os << "unknown";
	    return os;
	}
	double const devmax = 25.0;
	std::ostringstream oss;
	io::tslang_t tslang = io::tslang(os);
	io::tslang_t tslang_save = tslang;
	if (tslang == io::tslang_latex_text) {
	    tslang = io::tslang_latex_math;
	    os << "$";
	}
	io::set_tslang(oss, tslang);
	io::bits::ios_storage* iosstor = io::bits::storage(os);
	assert(iosstor != NULL);
	if (I.is_uncertain())
	    oss << "uncertain ";

	char const* str_pm = "±";
	char const* str_mp = "∓";
	char const* str_inf = "∞";
	char const* str_approx = "≅";
	if (tslang == io::tslang_latex_math) {
	    str_pm = "\\pm";
	    str_mp = "\\mp";
	    str_inf = "\\infty";
	    str_approx = "\\approx";
	}

	// Extract and print sign
	T cent = I.cent();
	if (I.is_signless()) {
	    if (cent < 0) {
		oss << "∓";
		cent = -cent;
	    }
	    else
		oss << "±";
	}
	else if (cent < 0.0) {
	    oss << '-';
	    cent = -cent;
	}

	if (I.is_inf()) {
	    oss << str_inf;
	    if (!I.dev_below_is_known())
		oss << opt_dev_begin << str_approx << opt_dev_end;
	}
	else {
	    // Determine the exponent and the number of decimals
	    int xpt0 = cent == 0.0? 0 : ifloor(log10(cent));
	    int xpt = xpt0;
	    T x_dev;
	    if (I.dev_below_is_finite()) {
		if (I.dev_above_is_finite())
		    x_dev = I.dev();
		else
		    x_dev = I.dev_below();
	    }
	    else if (I.dev_above_is_finite())
		x_dev = I.dev_above();
	    else
		x_dev = 0; //cent*.1;

	    int decimals;
	    if (x_dev == 0)
		decimals = os.precision();
	    else
		decimals
		    = ifloor(log10(cent)) - ifloor(log10(x_dev/devmax)) - 1;
	    if (-4 < xpt && xpt <= decimals) {
		decimals -= xpt;
		xpt = 0;
	    }

	    // Print value
	    oss.setf(std::ios_base::fixed);
	    oss.precision(decimals);
	    oss << cent*pow(.1, xpt);

	    // Print deviation
	    if (!I.is_exact()) {
		oss << opt_dev_begin;
		double scale = pow(10.0, decimals - xpt);
		if (I.is_symmetric()) {
		    if (I.dev_below_is_known())
			oss << iround(I.dev_below()*scale);
		    else
			oss << str_approx;
		}
		else {
		    if (I.dev_below_is_finite())
			oss << '-' << iround(I.dev_below()*scale) << ',';
		    else if (I.dev_below_is_inf())
			oss << '-' << str_inf << ',';
		    else
			oss << "-unknown,";
		    if (I.dev_above_is_finite())
			oss << '+' << iround(I.dev_above()*scale);
		    else if (I.dev_above_is_inf())
			oss << '+' << str_inf;
		    else
			oss << "+unknown";
		}
		oss << opt_dev_end;
	    }

	    // Print exponent
	    if (xpt != 0) {
		if (tslang == io::tslang_latex_math)
		    oss << "\\cdot10^{" << xpt << "}";
		else if (io::has_scripts(os)) {
		    oss << "⋅10"
			<< io::beg_super << xpt << io::end_super;
		}
		else if (opt_utf8_exponent) {
		    std::ostringstream osx;
		    osx << xpt;
		    std::string strx = osx.str();
		    oss << "⋅10";
		    for (std::string::size_type i = 0; i < strx.size(); ++i)
			oss << tosuper(strx[i]);
		}
		else
		    oss << 'e' << xpt;
	    }
	}
	if (I.is_known()) {
	    char const* extra_pfx = "";
	    char const* extra_sfx = "";
	    if (tslang == io::tslang_latex_math) {
		extra_pfx = "\\mbox{";
		extra_sfx = "}";
	    }
	    if (I.origin() == origin_unknown) {
		if (iosstor->origin_unknown_indicator)
		    oss << extra_pfx << iosstor->origin_indicator_prefix
			<< iosstor->origin_unknown_indicator
			<< iosstor->origin_indicator_suffix << extra_sfx;
	    }
	    else if (I.origin() != origin_theory) {
		bool cont = false;
		if ((I.origin() & origin_experiment)
		    && iosstor->origin_experiment_indicator) {
		    oss << extra_pfx << iosstor->origin_indicator_prefix;
		    oss << iosstor->origin_experiment_indicator;
		    cont = true;
		}
		if ((I.origin() & origin_calculation)
		    && iosstor->origin_calculation_indicator) {
		    if (cont)
			oss << iosstor->origin_indicator_separator;
		    else
			oss << extra_pfx << iosstor->origin_indicator_prefix;
		    oss << iosstor->origin_calculation_indicator;
		    cont = true;
		}
		if ((I.origin() & origin_systematics)
		    && iosstor->origin_systematics_indicator) {
		    if (cont)
			oss << iosstor->origin_indicator_separator;
		    else
			oss << extra_pfx << iosstor->origin_indicator_prefix;
		    oss << iosstor->origin_systematics_indicator;
		    cont = true;
		}
		if ((I.origin() & origin_other)
		    && iosstor->origin_other_indicator) {
		    if (cont)
			oss << iosstor->origin_indicator_separator;
		    else
			oss << extra_pfx << iosstor->origin_indicator_prefix;
		    oss << iosstor->origin_other_indicator;
		    cont = true;
		}
		if (cont)
		    oss << iosstor->origin_indicator_suffix << extra_sfx;
	    }
	}
	os << oss.str();
	if (tslang_save == io::tslang_latex_text)
	    os << "$";
	return os;
    }

  template<typename T>
    void
    confidence_interval<T>::sync(io::syncstream& sio)
    {
	if (sio._version() < 1) {
	    assert(!"Conversion from V0.0.0 format is not implemented.");
	    static bool warned = false;
	    if (!warned) {
		std::clog << "waring: Converting from old format for "
			  << "more::lang::confidence_interval.\n";
		warned = true;
	    }
	    assert(sio.is_input());
	    typedef unsigned short flags_type;
	    static flags_type const unknown_lower = 1;
	    static flags_type const unknown_upper = 2;
	    static flags_type const calculation = 4;
	    static flags_type const systematic = 8;
	    static flags_type const flag_infinite = 16;
	    flags_type flags;
	    sio | flags;
	    m_pf_cent = m_pf_s_mi = m_pf_s_pl = pf_finite;
	    if (flags & flag_infinite)
		m_pf_cent = m_pf_s_mi = m_pf_s_pl = pf_infinite;
	    if (flags & unknown_lower) {
		if (flags & unknown_upper)
		    m_pf_s_mi = m_pf_s_pl = pf_unknown;
		else
		    m_pf_s_mi = pf_infinite;
	    }
	    else if (flags & unknown_upper)
		m_pf_s_pl = pf_infinite;
	    m_aux_flags = 0;

	    if (flags & calculation)
		m_origin = origin_calculation;
	    else if (flags & systematic)
		m_origin = origin_systematics;
	    else
		m_origin = origin_experiment;
	    sio | m_cent;
	    sio | m_s_mi | m_s_pl;
	    m_moment = 2;
	    std::cout << "debug: read " << *this << std::endl;
	    return;
	}

	if (sio.is_output()) {
	    unsigned int precf
		= m_pf_cent
		| (m_pf_s_mi << 2)
		| (m_pf_s_pl << 4)
		| (m_aux_flags << 6);
	    sio | precf;
	    if (!is_known())
		return;
	    unsigned int o = m_origin;
	    sio | o;
	    if (!is_finite())
		return;

	    // 10 bits ≅ 3 decimal digit precision for approximate values
	    // 7 bits ≅ 2 decimal digit precision for the deviation
	    int digsave = sio.digits();
	    int dig = 10;
	    if (dev_below_is_finite() || dev_above_is_finite()) {
		double dev;
		if (dev_above_is_known()) {
		    if (dev_below_is_known())
			dev = std::min(m_s_mi, m_s_pl);
		    else
			dev = m_s_mi;
		}
		else
		    dev = m_s_pl;
		dig = math::iceil(std::log(std::fabs(m_cent)/dev)/std::log(2.0)) + 7;
	    }
	    sio.set_digits(dig);
	    sio | m_cent;
	    sio | m_moment;
	    sio.set_digits(7);
	    if (dev_below_is_finite())
		sio | m_s_mi;
	    if (dev_above_is_finite())
		sio | m_s_pl;
	    sio.set_digits(digsave);
	}
	else {
	    unsigned int precf;
	    sio | precf;
	    m_pf_cent = (precf >> 0) & 3;
	    m_pf_s_mi = (precf >> 2) & 3;
	    m_pf_s_pl = (precf >> 4) & 3;
	    m_aux_flags = (precf >> 6) & 15;
	    if (!is_known()) {
		m_origin = origin_unknown;
		return;
	    }
	    unsigned int o;
	    sio | o;
	    m_origin = o;
	    if (!is_finite())
		return;
	    sio | m_cent;
	    sio | m_moment;
	    if (dev_below_is_finite())
		sio | m_s_mi;
	    if (dev_above_is_finite())
		sio | m_s_pl;
	}
    }

  template struct confidence_interval<float>;
  template struct confidence_interval<double>;
  template struct confidence_interval<long double>;

  template
    std::basic_ostream<char, std::char_traits<char> >&
    operator<<(std::basic_ostream<char, std::char_traits<char> >&,
	       confidence_interval<float> const&);
  template
    std::basic_ostream<char, std::char_traits<char> >&
    operator<<(std::basic_ostream<char, std::char_traits<char> >&,
	       confidence_interval<double> const&);
  template
    std::basic_ostream<char, std::char_traits<char> >&
    operator<<(std::basic_ostream<char, std::char_traits<char> >&,
	       confidence_interval<long double> const&);

  void
  set_origin_indicator(std::ostream& os, origin_t which, char const* ind)
  {
      io::bits::ios_storage* stor = io::bits::storage(os);
      switch (which) {
//       case origin_theory:
// 	  stor->origin_theory_indicator = ind;
// 	  break;
      case origin_experiment:
	  stor->origin_experiment_indicator = ind;
	  break;
      case origin_calculation:
	  stor->origin_calculation_indicator = ind;
	  break;
      case origin_systematics:
	  stor->origin_systematics_indicator = ind;
	  break;
      case origin_other:
	  stor->origin_other_indicator = ind;
	  break;
      case origin_unknown:
	  stor->origin_unknown_indicator = ind;
	  break;
      default:
	  throw std::out_of_range("more::phys::set_origin_indicator: "
				  "Can not set indicator for given origin.");
      }
  }

  void
  set_origin_indicator(std::ostream& os, char const* pfx, char const* sfx,
		       char const* sep)
  {
      io::bits::ios_storage* stor = io::bits::storage(os);
      if (!pfx || !sfx || !sep)
	  throw std::out_of_range("more::phys::set_origin_indicator: "
				  "Zero string pointer.");
      stor->origin_indicator_prefix = pfx;
      stor->origin_indicator_suffix = sfx;
      stor->origin_indicator_separator = sep;
  }

}} // more::phys
