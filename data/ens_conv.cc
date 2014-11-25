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


#include <more/phys/ens.h>
#include <more/gen/iterator.h>
#include <string>
#include <sstream>
#include <stack>
#include "ens_conv.h"
#include <assert.h>

#if 0
#  define PARSE_ERROR(msg) throw parse_error((msg))
#else
#  define PARSE_ERROR(msg) \
	(nucleus::clog() << "error: " << msg << std::endl)
#endif
#  define PARSE_WARNING(msg) \
	(nucleus::clog() << "warning: " << msg << std::endl)

namespace more {
namespace phys {
namespace ens {

  const int opt_warning_level = 2;

  std::string
  decode_text(std::string const& s_txt, markup_language_t ml)
  {
      char const* s = s_txt.c_str();
      std::string r;
      std::stack<char> markup_stk;
      while (*s) {
	  if (isspace(*s)) {
	      do ++s; while (isspace(*s));
	      r += ' ';
	  }
	  switch (*s) {
	  case '|':
	      switch(*++s) {
	      case ' ': r += " "; break; // NO-BREAK SPACE, undocumented
	      case '\n': break;		 // Line continuation, undocumented
	      case '!': r += "©"; break;
//	      case '"': break; // XXX Obscured in the manual
	      case '#': r += "§"; break;
	      case '$': r += "e"; break; // XXX Right? But for what purpose?
	      case '%': r += "√"; break;
	      case '&': r += "≡"; break;
	      case '\\': r += "|\\"; break; // undocumented
	      case '\'': r += "°"; break;
	      case '(': r += "←"; break;
	      case ')': r += "→"; break;
	      case '*': r += "×"; break;
	      case '+': r += "±"; break;
	      case ',': r += "½"; break;
	      case '-': r += "∓"; break;
	      case '.': r += "∝"; break;
	      case '`': r += "′"; break;
	      case '/': r += "÷"; break;
	      case '0': r += "("; break;
	      case '1': r += ")"; break;
	      case '2': r += "["; break;
	      case '3': r += "]"; break;
	      case '4': r += "〈"; break;
	      case '5': r += "〉"; break;
	      case '6': r += "√"; break;
	      case '7': r += "∫"; break;
	      case '8': r += "∏"; break;
	      case '9': r += "∑"; break;
	      case ':': r += "†"; break;
	      case ';': r += "‡"; break;
	      case '<': r += "≤"; break;
	      case '=': r += "≠"; break;
	      case '>': r += "≥"; break;
	      case '?': r += "≈"; break;
	      case '@': r += "∞"; break;
	      case 'A': r += "Α"; break;
	      case 'B': r += "Β"; break;
	      case 'C': r += "Η"; break;
	      case 'D': r += "Δ"; break;
	      case 'E': r += "Ε"; break;
	      case 'F': r += "Φ"; break;
	      case 'G': r += "Γ"; break;
	      case 'H': r += "Χ"; break;
	      case 'I': r += "Ι"; break;
	      case 'J': r += "∼"; break;
	      case 'K': r += "Κ"; break;
	      case 'L': r += "Λ"; break;
	      case 'M': r += "Μ"; break;
	      case 'N': r += "Ν"; break;
	      case 'O': r += "Ο"; break;
	      case 'P': r += "Π"; break;
	      case 'Q': r += "Θ"; break;
	      case 'R': r += "Ρ"; break;
	      case 'S': r += "Σ"; break;
	      case 'T': r += "Τ"; break;
	      case 'U': r += "Υ"; break;
	      case 'V': r += "∇"; break;
	      case 'W': r += "Ω"; break;
	      case 'X': r += "Ξ"; break;
	      case 'Y': r += "Ψ"; break;
	      case 'Z': r += "Ζ"; break;
	      case '[': r += "{"; break;
	      case ']': r += "}"; break;
	      case '^': r += "↑"; break;
	      case '_': r += "↓"; break;
	      case 'a': r += "α"; break;
	      case 'b': r += "β"; break;
	      case 'c': r += "η"; break;
	      case 'd': r += "δ"; break;
	      case 'e': r += "ε"; break;
	      case 'f': r += "ϕ"; break;
	      case 'g': r += "γ"; break;
	      case 'h': r += "χ"; break;
	      case 'i': r += "ι"; break;
	      case 'j': r += "∈"; break; // \in, XXX or is it \varepsilon?
	      case 'k': r += "κ"; break;
	      case 'l': r += "λ"; break;
	      case 'm': r += "μ"; break;
	      case 'n': r += "ν"; break;
	      case 'o': r += "ο"; break; // \omicron
	      case 'p': r += "π"; break;
	      case 'q': r += "θ"; break;
	      case 'r': r += "ρ"; break;
	      case 's': r += "σ"; break;
	      case 't': r += "τ"; break;
	      case 'u': r += "υ"; break;
	      case 'w': r += "ω"; break;
	      case 'x': r += "ξ"; break;
	      case 'y': r += "ψ"; break;
	      case 'z': r += "ζ"; break;
	      case 0:
		  if (opt_warning_level >= 1)
		      PARSE_WARNING("String termination at entity.");
		  return r;
	      default:
		  if (opt_warning_level >= 2)
		      PARSE_WARNING("Unknown entity ‘|" << *s << "’\n");
		  break;
	      }
	      ++s;
	      break;
	  case '~':
	      switch (*++s) {
	      case '#': r += "⊗"; break;
	      case '&': r += "&"; break;
	      case '\'': r += "Å"; break;
	      case '*': r += "⋅"; break;
	      case '@': r += "∗"; break;
	      case 'A': r += "Ä"; break;
	      case 'E': r += "Ë"; break;
	      case 'O': r += "Ö"; break;
	      case 'Q': r += "Õ"; break;
	      case 'U': r += "Ü"; break;
	      case 'a': r += "ä"; break;
	      case 'e': r += "é"; break;
	      case 'h': r += "ℏ"; break;
	      case 'l': r += "ƛ"; break;
	      case 'o': r += "ö"; break;
	      case 'q': r += "õ"; break;
	      case 'u': r += "ü"; break;
	      default:
		  r += *s;
		  break;
	      }
	      ++s;
	      break;
	  case '<':
	      if (ml == markup_language_html)
		  r += "&lt;";
	      else
		  r += "<";
	      ++s;
	      break;
	  case '>':
	      if (ml == markup_language_html)
		  r += "&gt;";
	      else
		  r += ">";
	      ++s;
	      break;
	  case '&':
	      if (ml == markup_language_html)
		  r += "&amp;";
	      else
		  r += "&";
	      ++s;
	      break;
	  case '{':
	      if (ml == markup_language_html) {
		  switch (*++s) {
		  case '+': r += "<sup>"; break;
		  case '-': r += "<sub>"; break;
		  case 'I': r += "<i>"; break;
		  case 'B': r += "<b>"; break;
		  case 'U': r += "<u>"; break;
		  default:
		      r += "{";
		      goto no_esc;
		  }
		  markup_stk.push(*s);
	      }
	      else
		  r += "{";
	      ++s;
	  no_esc:
	      break;
	  case '}':
	      if (markup_stk.empty()) {
		  r += *s++;
		  break;
	      }
	      switch (markup_stk.top()) {
	      case '+': r += "</sup>"; break;
	      case '-': r += "</sub>"; break;
	      case 'I': r += "</i>"; break;
	      case 'B': r += "</b>"; break;
	      case 'U': r += "</u>"; break;
	      }
	      ++s;
	      break;
	  default:
	      r += *s++;
	      break;
	  }
      }
      return r;
  }


  //
  // Type parsers
  //
  nucleus
  str_to_nucid(std::string const& nucid)
  {
      std::istringstream iss(nucid);
      nucleus nucl;
      iss >> nucl;
      if (iss.fail())
	  throw parse_error("more::phys::ens: Bad NUCID.: " + nucid);
      return nucl;
  }

  record_kind_t
  chars_to_rtype(char c0, char c1 = ' ')
  {
      if (c1 == ' ')
	  switch (c0) {
	  case 'H': return record_kind_history;
	  case 'N': return record_kind_norm;
	  case 'P': return record_kind_parent;
	  case 'Q': return record_kind_qvalue;
	  case 'L': return record_kind_level;
	  case 'G': return record_kind_gamma;
	  case 'B': return record_kind_beta_mi;
	  case 'E': return record_kind_beta_pl;
	  case 'A': return record_kind_alpha;
	  case 'R': return record_kind_reference;
	  case 'X': return record_kind_xref;
	  case ' ': return record_kind_none;
	  default:  break;
	  }
      else if (c0 == 'N' && c1 == 'P')
	  return record_kind_pnorm;
      // XXX The Particle and Delayed Particle specifications are not
      // fully clear.  (p. 41).
      else if (c0 == 'D')
	  return record_kind_dparticle;
      else if (c0 == ' ')
	  return record_kind_particle;

      PARSE_WARNING("Don’t understand RTYPE ‘" << c0 << c1 << "’.");
      return record_kind_invalid;
  }

  std::string
  record_kind_to_str(record_kind_t rt)
  {
      switch (rt) {
      case record_kind_invalid: return "!!INVALID R-TYPE!!";
      case record_kind_none: return "none";
      case record_kind_history: return "History";
      case record_kind_norm: return "Normalization";
      case record_kind_parent: return "Parent";
      case record_kind_qvalue: return "Q-Value";
      case record_kind_level: return "Level";
      case record_kind_gamma: return "γ-Decay";
      case record_kind_beta_mi: return "β⁻-Decay";
      case record_kind_beta_pl: return "β⁺-Decay";
      case record_kind_alpha: return "α-Decay";
      case record_kind_reference: return "Reference";
      case record_kind_xref: return "Cross Reference";
      case record_kind_particle: return "Particle";
      case record_kind_dparticle: return "Delayed Particle";
      default:
	  throw std::invalid_argument("more::phys::ens::record_kind_to_str: "
				      "No such rtype.");
      }
  }

  particle_kind_t
  char_to_psym(char c)
  {
      switch (c) {
      case 'A': return particle_kind_alpha;
      case 'P': return particle_kind_proton;
      case 'N': return particle_kind_neutron;
      case ' ': return particle_kind_none;
      default:  break;
      }
      return particle_kind_invalid;
  }

  std::string
  particle_kind_to_str(particle_kind_t ps)
  {
      switch (ps) {
      case particle_kind_alpha: return "α";
      case particle_kind_proton: return "p";
      case particle_kind_neutron: return "n";
      case particle_kind_none: return "#[no particle]";
      default:
	  throw std::invalid_argument("more::phys::ens::particle_kind_to_str: "
				      "Invalid particle.");
      }
  }


  bool
  citation_t::is_valid() const
  {
      return m_year < 10000
	  && std::isalpha(m_rest[0])
	  && std::isalpha(m_rest[1])
	  && std::isalnum(m_rest[2])
	  && std::isalnum(m_rest[3]);
  }

  void
  citation_t::sync(io::syncstream& sio)
  {
      sio | m_year;
      sio.sync_raw_data(m_rest, 4);
  }

  std::istream&
  operator>>(std::istream& is, citation_t& kn)
  {
      int year = 0xffff;
      char s[4];
      is >> year;
      if (is.fail())
	  return is;
      is >> s[0] >> s[1] >> s[2] >> s[3];
      if (is.fail())
	  return is;
      citation_t kn0(year, s);
      if (kn0.is_valid()) {
	  kn = kn0;
	  return is;
      }
      else {
	  is.setstate(std::ios_base::failbit);
	  return is;
      }
  }

  std::ostream&
  operator<<(std::ostream& os, citation_t const& kn)
  {
      if (kn.is_valid()) {
	  char const* s = kn.rest_begin();
	  char save = os.fill();
	  os.fill('0');
	  os << std::setw(4) << kn.year() << s[0] << s[1] << s[2] << s[3];
	  os.fill(save);
      }
      else
	  os << "#[invalid keynum]";
      return os;
  }

  std::istream&
  operator>>(std::istream& is, citation_list& knl)
  {
      if (std::isspace(is.peek()) || is.eof())
	  return is;
      knl.clear();
      for (;;) {
	  citation_t kn;
	  is >> kn;
	  if (is.fail())
	      return is;
	  knl.push_back(kn);
	  if (std::isspace(is.peek()) || is.eof())
	      return is;

	  char ch;
	  is >> ch;
	  if (ch != ',') {
	      is.putback(ch);
	      is.clear();
	      return is;
	  }
      }
  }

  std::ostream&
  operator<<(std::ostream& os, citation_list const& knl)
  {
      std::copy(knl.begin(), knl.end(),
		gen::ostream_iterator<citation_t>(os, ","));
      return os;
  }

  bool
  str_to_keynum_list(std::string const& str, citation_list& knl)
  {
      std::istringstream iss(str);
      iss >> knl;
      return iss.eof() || !iss.fail();
//       std::string::size_type i = 0, j = 0, n = str.size();
//       while (i < n) {
// 	  j = str.find(',', i);
// 	  if (j == std::string::npos)
// 	      j = n;
// 	  while (i < j && str[i] == ' ')
// 	      ++i;
// 	  knl.push_back(str.substr(i, j - i));
// 	  i = j + 1;
//       }
  }

  bool
  str_to_date(std::string const& str, sys::date& d)
  {
      static int current_year = 0;
      if (current_year == 0)
	  current_year = sys::date(sys::current_time()).year();

      if (str.size() != 6)
	  return false;
      for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	  if (!std::isdigit(*it))
	      return false;
      int year;
      int month;

      std::istringstream iss_year(str.substr(0, 4));
      iss_year >> year;
      if (iss_year.fail())
	  return false;

      std::istringstream iss_month(str.substr(4));
      iss_month >> month;
      if (iss_month.fail())
	  return false;

      // Nuclear physics started around 1900.  There is no way
      // anything could be dated before that, but let's say 1800.
      if (1 <= month && month <= 12
	  && 1800 <= year && year < current_year + 5) {
	  d = sys::date(year, month);
	  return true;
      }
      else if (month == 99) {
	  // UNDOCUMENTED. month == 99 seems to mean "unspecified".
	  d = sys::date(year);
	  return true;
      }
      // There are some ill-formed dates of the form YYMMDD, better
      // just leave them since we are close to a 19XX vs 20XX
      // ambiguity.
#if 0
      else if (1 <= month && month <= 31
	       && 1 <= (year % 100) && (year % 100) <= 12) {
	  return false;
      }
#endif
      else
	  return false;
  }

  int
  str_to_month(std::string str)
  {
      str += ' ';
      switch (str[0]) {
      case 'j':
	  switch (str[1]) {
	  case 'a':
	      switch (str[2]) {
	      case 'n':
		  return 1;
	      default:
		  return -1;
	      }
	  case 'u':
	      switch (str[2]) {
	      case 'n':
		  return 6;
	      case 'l':
		  return 7;
	      default:
		  return -1;
	      }
	  default:
	      return -1;
	  }
      case 'f':
	  switch (str[1]) {
	  case 'e':
	      switch (str[2]) {
	      case 'b':
		  return 2;
	      default:
		  return -1;
	      }
	  default:
	      return -1;
	  }
      case 'm':
	  switch (str[1]) {
	  case 'a':
	      switch (str[2]) {
	      case 'r':
		  return 3;
	      case 'y':
		  return 5;
	      default:
		  return -1;
	      }
	  default:
	      return -1;
	  }
      case 'a':
	  switch (str[1]) {
	  case 'p':
	      switch (str[2]) {
	      case 'r':
		  return 4;
	      default:
		  return -1;
	      }
	  case 'u':
	      switch (str[2]) {
	      case 'g':
		  return 8;
	      default:
		  return -1;
	      }
	  default:
	      return -1;
	  }
      case 's':
	  switch (str[1]) {
	  case 'e':
	      switch (str[2]) {
	      case 'p':
		  return 9;
	      default:
		  return -1;
	      }
	  default:
	      return -1;
	  }
      case 'o':
	  switch (str[1]) {
	  case 'c':
	      switch (str[2]) {
	      case 't':
		  return 10;
	      default:
		  return -1;
	      }
	  default:
	      return -1;
	  }
      case 'n':
	  switch (str[1]) {
	  case 'o':
	      switch (str[2]) {
	      case 'v':
		  return 11;
	      default:
		  return -1;
	      }
	  default:
	      return -1;
	  }
      case 'd':
	  switch (str[1]) {
	  case 'e':
	      switch (str[2]) {
	      case 'c':
		  return 12;
	      default:
		  return -1;
	      }
	  default:
	      return -1;
	  }
      default:
	  return -1;
      }
      assert(!"Not reached.");
      return -1;
  }


  bool
  str_vulgar_to_date(std::string const& str, sys::date& d)
  {
      std::string monthname;
      int day, month, year;
      char ch;
      std::istringstream iss(str);
      iss >> day >> ch;
      if (iss.fail() || ch != '-')
	  return false;
      iss >> ch;
      while (std::isalpha(ch)) {
	  monthname += std::tolower(ch);
	  ch = iss.get();
      }
      if (ch != '-')
	  return false;
      month = str_to_month(monthname);
      if (month == -1)
	  return false;
      iss >> year;
      if (iss.fail() || (iss >> ch, !iss.eof()))
	  return false;
      d = sys::date(year, month, day);
      return true;
  }


  void
  str_to_jpi_expr(std::string const& str, spin_parity_expr_t& jpi)
  {
      // XXX spin_parity_expr_t will be redefined
      jpi = spin_parity_expr_t(str);
  }



  //
  //  Related to Numeric Quantities
  //

  double
  time_unit_scale(std::string unit)
  {
      if (unit.size() == 1)
	  switch (unit[0]) {
	    case 'Y': return SI::year;
	    case 'D': return SI::day;
	    case 'H': return SI::hour;
	    case 'M': return SI::min;
	    case 'S': return SI::s;
	    default: return 0;
	  }
      else if (unit.size() == 2 && unit[1] == 'S')
	  switch (unit[0]) {
	    case 'M': return SI::ms;
	    case 'U': return SI::us;
	    case 'N': return SI::ns;
	    case 'P': return SI::ps;
	    case 'F': return SI::fs;
	    case 'A': return SI::as;
	    default: return 0;
	  }
      else
	  return 0;
  }

  double
  energy_unit_scale(std::string unit)
  {
      if (unit == "EV") return SI::eV;
      if (unit == "KEV") return SI::keV;
      if (unit == "MEV") return SI::MeV;
      std::cerr << "** Bad energy unit " << unit << std::endl;
      return 0;
  }


  std::string
  trim_spaces(std::string const& str)
  {
      int n = str.size();
      int i, j;
      for (i = 0; i < n; ++i)
	  if (!std::isspace(str[i]))
	      break;
      for (j = n; i < j; --j)
	  if (!std::isspace(str[j - 1]))
	      break;
      return str.substr(i, j - i);
  }

  // V13
  bool
  str_to_number(std::string s, double& x, bool& par_p)
  {
      char c;
      std::istringstream iss(s);
      iss >> c;
      if (c == '(')
	  par_p = true;
      else {
	  iss.putback(c);
	  par_p = false;
      }
      iss >> x;
      if (c == '(')
	  iss >> c;
      else
	  c = ')';
      if (iss.fail() || c != ')' || (iss >> c, !iss.eof()))
	  return false;
      return true;
  }


  bool
  str_to_energy(std::string s_val, std::string s_dev,
		confiv_t& x, energy_index_t& ref)
  {
      if (s_val.empty() && s_dev.empty()) {
	  x = confiv_t();
	  ref = energy_index_none;
	  return true;
      }
      s_val = trim_spaces(s_val);
      std::string s_ref;
      if (std::isalpha(s_val[0])) {
	  std::string::size_type i = s_val.find_first_of("+-");
	  if (i == std::string::npos) {
	      s_ref = s_val;
	      s_val.clear();
	  }
	  else {
	      s_ref = s_val.substr(0, i);
	      s_val = s_val.substr(i);  // include the sign in case it is -
	  }
      }
      else if (std::isalpha(s_val[s_val.size() - 1])) {
	  std::string::size_type i = s_val.rfind('+');
	  if (i == std::string::npos) {
	      PARSE_ERROR("Malformed energy value " << s_val << ".");
	      return false;
	  }
	  else {
	      s_ref = s_val.substr(i + 1);
	      s_val = s_val.substr(0, i);
	  }
      }

      if (s_ref.empty())
	  ref = energy_index_none;
      else if (s_ref.size() == 1) {
	  if (!std::isalpha(s_ref[0]))
	      return false;
	  ref = std::toupper(s_ref[0]) - 'A' + 1;
      }
      else if (s_ref == "SN")
	  ref = energy_index_sn;
      else if (s_ref == "SP")
	  ref = energy_index_sp;
      else if (s_ref == "SA") // undocumented
	  ref = energy_index_sa;
      else
	  std::cerr << "** Bad reference point ‘" << s_ref << "’.\n";

      if (!s_val.empty())
	  if (!str_to_confiv(s_val, s_dev, x, true))
	      return false;
      x *= SI::keV;

      return true;
  }

  bool
  str_to_energy(std::string s_val, std::string s_dev,
		confiv_t& x)
  {
      if (s_val.empty() && s_dev.empty()) {
	  x = confiv_t();
	  return true;
      }
      if (!str_to_confiv(s_val, s_dev, x, true))
	  return false;
      x *= SI::keV;
      return true;
  }

  bool
  str_to_halflife(std::string s_val, std::string s_dev,
		  confiv_t& x, bool& as_width_p)
  {
      bool uncertain_p = false;
      if (s_val.empty() && s_dev.empty()) {
	  x = confiv_t();
	  as_width_p = false;
	  return true;
      }
      if (s_val[s_val.size() - 1] == '?') {
	  uncertain_p = true;
	  s_val.erase(s_val.size() - 1);
      }
      else
	  uncertain_p = false;
      if (s_val == "STABLE") {
	  x = confiv_t::infinite;
	  as_width_p = false;
	  x.set_origin(origin_experiment);
	  if (s_dev == "CA")
	      x.set_origin(origin_calculation);
	  else if (s_dev == "SY")
	      x.set_origin(origin_systematics);
 	  else if (s_dev == "AP")
	      x = confiv_t(confiv_t::infinite, confiv_t::unknown);
	  else if (!s_dev.empty()) {
	      PARSE_ERROR("Don't understand deviation " + s_dev +
			  " on STABLE halflife.");
	      return false;
	  }
      }
      else {
	  // Determine scale from unit.
	  std::string::size_type i = s_val.find(' ');
	  double sc;
	  if (i == std::string::npos) {
	      // In e.g. Gd-157 the half-life field is relabeled, so
	      // it would be better to check for such relabelling and
	      // ignore the values in those cases.
	      return false;
	  }
	  else {
	      std::string s_unit = trim_spaces(s_val.substr(i + 1));
	      s_val = s_val.substr(0, i);
	      sc = time_unit_scale(s_unit);
	      if (sc == 0.0) {
		  as_width_p = true;
		  sc = energy_unit_scale(s_unit);
		  if (sc == 0)
		      return false;
	      }
	      else
		  as_width_p = false;
	  }

	  // Determine value.
	  if (!str_to_confiv(s_val, s_dev, x))
	      return false;
	  x *= sc;
      }
      if (uncertain_p)
	  x.be_uncertain();
      return true;
  }

  // Pre: s_val is a unitless value
  bool
  str_to_confiv(std::string s_val, std::string s_dev, confiv_t& x_ci,
		bool allow_par)
  {
      bool infinite_below = false;
      bool infinite_above = false;
      bool approximate = false;
      origin_t origin = origin_experiment;

      if (allow_par) {
	  if (!s_val.empty()
	      && s_val[0] == '(' && s_val[s_val.size()-1] == ')') {
	      s_val = s_val.substr(1, s_val.size() - 2);
	      // The manual says that the parentheses means that the value
	      // is derived or taken from another experiment.
	      origin = origin_other;
	  }
      }

      // Parse the value
      double x_val = 0.0;
      if (s_val.empty()) {
	  if (!trim_spaces(s_dev).empty())
	      PARSE_ERROR("Deviation given, but no value.");
	  return false;
      }
      std::istringstream iss(s_val);
      iss >> x_val;
      if (iss.fail() || (iss.get(), !iss.eof())) {
	  PARSE_ERROR("Ill-formed float ‘" + s_val + "’.");
	  return false;
      }

      // Parse the deviation
      int i_dev = 0;
      double x_dev[2] = { 0.0, 0.0 };
      if (!s_dev.empty()) {
	  // Also allowed: LT, GT, LE, GE, AP, CA, SY
	  if (s_dev == "LT" || s_dev == "LE")
	      infinite_below = true;
	  else if (s_dev == "GT" || s_dev == "GE")
	      infinite_above = true;
	  else if (s_dev == "AP")
	      approximate = true;
	  else if (s_dev == "CA")
	      origin = origin_calculation;
	  else if (s_dev == "SY")
	      origin = origin_systematics;
	  else {
	      std::string s_mkdev;
	      if (s_val[0] == '-' || s_val[0] == '+')
		  s_mkdev = s_val.substr(1);
	      else if (std::isdigit(s_val[0]) || s_val[0] == '.')
		  s_mkdev = s_val;
	      else {
		  PARSE_ERROR("Does not understand prefix on value " + s_val);
		  return false;
	      }
	      std::string::size_type i;
	      char const* s1[3] = { 0, 0, 0 };
	      std::string::size_type strlen_s1[2];
	      char const* s = s_dev.c_str();
	      if (s[0] == '+') {
		  s1[0] = s + 1;
		  char const* s2 = std::strchr(s, '-');
		  strlen_s1[0] = s2 - s - 1;
		  if (!s2) {
		      PARSE_ERROR("Can't parse deviation on " + s_dev
				  + "(" + s + ")\n");
		      return false;
		  }
		  s1[1] = ++s2;
		  strlen_s1[1] = std::strlen(s2);
	      }
	      else if (s[0] == '-') {
		  s1[1] = s + 1;
		  char const* s2 = std::strchr(s, '+');
		  strlen_s1[1] = s2 - s - 1;
		  if (!s2) {
		      PARSE_ERROR("Can't parse deviation on " + s_val
				  + "(" + s + ")\n");
		      return false;
		  }
		  s1[0] = ++s2;
		  strlen_s1[0] = std::strlen(s2);
	      }
	      else {
		  s1[0] = s;
		  strlen_s1[0] = std::strlen(s);
	      }
	      for (int k = 0; s1[k]; ++k) {
//dbg 		  std::cerr << "Deviation ‘" << s1[k] << "’ on " << s_val;
		  for (i = 0; i < s_mkdev.size(); ++i) {
		      if (isdigit(s_mkdev[i]))
			  s_mkdev[i] = '0';
		      else if (s_mkdev[i] == 'E' || s_mkdev[i] == 'e')
			  break;
		      else
			  switch (s_mkdev[i]) {
			    case '-': case '+': case ' ': case '.':
			      break;
			    default:
			      if (opt_warning_level >= 1)
				  PARSE_ERROR("Badly formatted number"
					      + s_val + ".\n");
			      break;
			  }
		  }
		  char const* s = s1[k];
		  std::string::size_type j = strlen_s1[k];
		  while (j > 0) {
		      --j;
		      --i;
		      if (s_mkdev[i] == '.') --i;
		      if (i >= 0 && s_mkdev[i] == '0')
			  s_mkdev[i] = s[j];
		      else {
			  ++i;
			  s_mkdev = s_mkdev.substr(0, i) + s[j]
			      + s_mkdev.substr(i);
		      }
		  }
		  std::istringstream iss(s_mkdev);
		  iss >> x_dev[i_dev++];
		  if (opt_warning_level >= 1
		      && (iss.fail() || !(iss.get(), iss.eof())))
		      PARSE_ERROR("Ill-formed number " + s_mkdev + ".\n");
//dbg 		  std::cerr << " parsed as " << x_dev[i_dev-1] << std::endl;
	      }
	  }
      }
      if (i_dev < 2)
	  x_dev[1] = x_dev[0];

      if (approximate)
	  x_ci = confiv_t(x_val, confiv_t::unknown);
      else if (infinite_above)
	  x_ci = confiv_t(x_val, x_dev[0], confiv_t::infinite);
      else if (infinite_below)
	  x_ci = confiv_t(x_val, confiv_t::infinite, x_dev[1]);
      else
	  x_ci = confiv_t(x_val, x_dev[0], x_dev[1]);
      x_ci.set_origin(origin);
      return true;
  }



}}} // more::phys::ens

// Local Variables:
// coding: utf-8
// indent-tabs-mode: t
// End:
