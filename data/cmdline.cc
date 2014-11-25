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



#include <more/io/cmdline.h>
#include <more/io/iomanip.h>
#include <ios>
#include <iostream>
#include <sstream>
#include <utility>
#include <string>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <more/bits/conf.h>
#ifdef MORE_CONF_HAVE_UNISTD_H
#include <unistd.h>
#endif

namespace more {
namespace io {

  void
  cmdline_arg_parser<std::string>::operator()(std::istream& is)
  {
      if (is.peek() == cmdline::argv_separator())
	  is.get();
      while (is.good() && is.peek() != cmdline::argv_separator())
	  m_x += is.get();
      if (!is.fail())
	  m_notify = true;
  }

  struct cmdline_tslang
      : std::unary_function<std::istream&, void>
  {
      explicit cmdline_tslang(unsigned int mask, unsigned int tty_mask = 14)
	  : m_mask(mask), m_tty_mask(tty_mask) {}

      void
      operator()(std::istream& is)
      {
	  unsigned int mask = m_mask;
	  std::string str;
	  while (std::isalpha(is.peek()))
	      str += (char)is.get();
	  if (is.fail())
	      return;
	  tslang_t tslang = tslang_none;
	  if (str == "none")
	      tslang = tslang_none;
	  else if (str == "terminfo") {
	      tslang = tslang_terminfo;
	      mask &= m_tty_mask;
	  }
	  else if (str == "html")
	      tslang = tslang_html;
	  else {
	      is.setstate(std::ios_base::failbit);
	      return;
	  }
	  is.clear();
	  if (mask & 1)
	      set_tslang(std::cin, tslang);
	  if (mask & 2)
	      set_tslang(std::cout, tslang);
	  if (mask & 4)
	      set_tslang(std::cerr, tslang);
	  if (mask & 8)
	      set_tslang(std::clog, tslang);
      }
    private:
      unsigned int m_mask;
      unsigned int m_tty_mask;
  };

  cmdline::cmdline(flag_type fl)
      : m_req_count(-1),
	m_flags(fl)
  {
      insert_reference("--verbosity", "Set level of logging.", s_opt_verb);
      insert_reference("--debuglevel", "Set level of debug information",
		       s_opt_debug);
      insert_functional("--markup",
			"ml#Set the markup capability "
			"of stdout and stderr to ml ∈ "
			"{ none, html, terminfo }.",
			cmdline_tslang(14));
  }

  void
  cmdline::parse(int argc, char** argv)
  {
      bool env_assign = true;
      // Check some environment arguments
      unsigned int tty_mask = 7;
#ifdef MORE_CONF_HAVE_ISATTY
      if (isatty(1))
	  set_tslang(std::cout, tslang_terminfo);
      else
	  tty_mask &= ~2;
      if (isatty(2)) {
	  set_tslang(std::cerr, tslang_terminfo);
	  set_tslang(std::clog, tslang_terminfo);
      }
      else
	  tty_mask &= ~12;
#endif
      if (char const* markup = std::getenv("MORE_IO_MARKUP")) {
	  cmdline_tslang f(14, tty_mask);
	  std::istringstream iss(std::string(markup)+argv_separator());
	  f(iss);
	  if (iss.fail())
	      std::cerr << "more::io::cmdline::parse: "
			<< "warning: Invalid value of $MORE_IO_MARKUP.\n";
      }
      if (char const* markup = std::getenv("MORE_IO_COUT_MARKUP")) {
	  cmdline_tslang f(2, tty_mask);
	  std::istringstream iss(std::string(markup)+argv_separator());
	  f(iss);
	  if (iss.fail())
	      std::cerr << "more::io::cmdline::parse: "
			<< "warning: Invalid value of $MORE_IO_COUT_MARKUP.\n";
      }
      if (char const* markup = std::getenv("MORE_IO_CERR_MARKUP")) {
	  cmdline_tslang f(4, tty_mask);
	  std::istringstream iss(std::string(markup)+argv_separator());
	  f(iss);
	  if (iss.fail())
	      std::cerr << "more::io::cmdline::parse: "
			<< "warning: Invalid value of $MORE_IO_CERR_MARKUP.\n";
      }
      if (char const* markup = std::getenv("MORE_IO_CLOG_MARKUP")) {
	  cmdline_tslang f(8, tty_mask);
	  std::istringstream iss(std::string(markup)+argv_separator());
	  f(iss);
	  if (iss.fail())
	      std::cerr << "more::io::cmdline::parse: "
			<< "warning: Invalid value of $MORE_IO_CLOG_MARKUP.\n";
      }
      if (char const* v = std::getenv("MORE_IO_CMDLINE_ASSIGN")) {
	  if (std::strcmp(v, "false") == 0)
	      env_assign = false;
	  else if (std::strcmp(v, "true") == 0)
	      env_assign = true;
	  else
	      std::cerr << "more::io::cmdline::parse: "
			<< "warning: Invalid value of "
			<< "$MORE_IO_CMDLINE_ASSIGN\n";
      }

      // Parse arguments
      s_prgname = argv[0];
      std::list<closure_type>::iterator arg_it = m_arg_list.begin();
      if (argc == 0)
	  std::logic_error("argc is zero!");
      if (argc == 2 && std::string(argv[1]) == "--help") {
	  print_help(std::cout);
	  throw relax("Help request.");
      }
      std::string is_str;
      for (int i = 1; i < argc; ++i) {
	  if ((m_flags & disable_assignment_style) == 0
	      && argv[i][0] != '-' && std::strchr(argv[i], '='))
	      is_str.append(std::string("--") + argv[i]);
	  else
	      is_str.append(argv[i]);
	  if (i != argc)
	      is_str.append(1, argv_separator());
      }
      std::istringstream is(is_str);
      char c = is.get();
      while (is.good()) {
	  if (c == '-' && is.peek() != argv_separator()
	      && !isdigit(is.peek())) {
	      if (is.peek() == '-') {
		  is.get();
		  std::string opt = "--";
		  while (is.good()) {
		      char c = is.get();
		      if (c == '=' || c == argv_separator())
			  break;
		      opt.append(1, c);
		  }
		  if (opt == "--help") {
		      print_help(std::cout);
		      throw relax("Help request.");
		  }
		  opt_map_type::iterator it = m_opt_map.find(opt);
		  if (it == m_opt_map.end())
		      throw failure("Bad option " + opt);
		  it->second(is);
		  if (is.fail())
		      throw failure("Bad value for option " + opt);
	      }
	      else {
		  std::string opt = "-";
		  if (!isspace(is.peek()))
		      opt.append(1,
				 std::string::traits_type
				 ::to_char_type(is.get()));
		  opt_map_type::iterator it = m_opt_map.find(opt);
		  if (it == m_opt_map.end())
		      throw failure("Bad option " + opt);
		  if (is.peek() == '=')  is.get();
		  it->second(is);
		  if (is.fail())
		      throw failure("Bad argument for option " + opt);
		  if (is.peek() != argv_separator()) {
		      std::string trash;
		      while (is.good() && is.peek() != argv_separator())
			  trash += is.get();
		      throw failure("Trash \"" + trash +
				    "\" after option " + opt + ".");
		  }
	      }
	  }
	  else {
	      is.putback(c);
	      if (arg_it == m_arg_list.end())
		  throw failure(std::string("Too many arguments."));
	      (*arg_it)(is);
	      if (is.fail()) {
		  std::ostringstream oss;
		  oss << "Bad value for argument #"
		      << distance(m_arg_list.begin(), arg_it)+1 << '\0';
		  throw failure(oss.str());
	      }
	      ++arg_it;
	  }
	  while ((c = is.get()) == argv_separator());
      }
      if ((size_type)distance(m_arg_list.begin(), arg_it) <
	  (m_req_count < 0? m_arg_list.size() : m_req_count ))
	  throw failure("Too few arguments.");
  }

  static std::ostream& (*set_argstyle)(std::ostream&) = &set_underline;
  static std::ostream& (*clr_argstyle)(std::ostream&) = &clr_underline;

  static void
  fmt_helper(std::ostream& os, std::istream& is, std::size_t col,
	     std::size_t left_margin, std::size_t right_margin,
	     std::string const& arg_sym)
  {
      if (col >= left_margin) {
	  os << '\n';
	  col = 0;
      }
      while (col < left_margin) {
	  os << ' ';
	  ++col;
      }
      for (;;) {
	  std::list<std::string> words;
	  int connected_length = 0;
	  char ch = is.get();
	  if (is.eof())
	      break;
	  for (;;) {
	      std::string word;
	      if (is.eof() || std::isspace(ch))
		  break;
	      if (std::isalpha(ch)) {
		  do {
		      word += ch;
		      ch = is.get();
		  }
		  while (std::isalnum(ch));
	      }
	      else {
		  do {
		      word += ch;
		      ch = is.get();
		  }
		  while (is.good() && !std::isalpha(ch)
			 && !std::isspace(ch));
	      }
	      words.push_back(word);
	      connected_length += word.size();
	  }
	  if (col != left_margin) {
	      if (col + connected_length > right_margin) {
		  os << '\n';
		  col = 0;
		  while (col + 8 <= left_margin) {
		      os << '\t';
		      col += 8;
		  }
		  while (col < left_margin) {
		      os << ' ';
		      ++col;
		  }
	      }
	      else {
		  ++col;
		  os << ' ';
	      }
	  }
	  for (std::list<std::string>::iterator it = words.begin();
	       it != words.end(); ++it) {
	      if (*it == arg_sym)
		  os << set_argstyle << arg_sym << clr_argstyle;
	      else
		  os << *it;
	  }
	  col += connected_length;
	  if (is.eof())
	      break;
      }
      os << '\n';
  }

  void
  cmdline::print_help(std::ostream& os) const
  {
      const std::size_t mid = 28, right = 76;
      os << "Usage: " << s_prgname;
      if (m_opt_doc.size())
	  os << " " << set_argstyle << "option" << clr_argstyle << "*";
      for (arg_doc_type::const_iterator it = m_arg_doc.begin();
	  it != m_arg_doc.end(); ++it) {
	  std::string::size_type n = it->find('#');
	  bool is_opt = m_req_count >= 0
	      && (int)distance(m_arg_doc.begin(), it) >= m_req_count;
	  os << ' ';
	  if (n >= it->size())
	      os << set_argstyle
		 << "arg" << std::distance(m_arg_doc.begin(), it)
		 << clr_argstyle;
	  else
	      os << set_argstyle << it->substr(0, n) << clr_argstyle;
	  if (is_opt)
	      os << '?';
      }
      os << '\n';
      bool head_printed = false;
      for (arg_doc_type::const_iterator it = m_arg_doc.begin();
	  it != m_arg_doc.end(); ++it) {
	  if (!head_printed) {
	      os << "\nArguments:\n";
	      head_printed = true;
	  }
	  std::string arg_sym;
	  std::string::size_type n = it->find('#');
	  std::size_t i = 4;
	  if (n >= it->size()) {
	      int j = std::distance(m_arg_doc.begin(), it);
	      os << "    " << set_argstyle << "arg" << j << clr_argstyle;
	      i += 4;
	      do {
		  ++i;
		  j /= 10;
	      } while (j);
	  }
	  else {
	      arg_sym = it->substr(0, n);
	      os << "    " << set_argstyle << it->substr(0, n)
		 << clr_argstyle;
	      i += n;
	  }
	  std::istringstream iss(it->substr(n+1));
	  fmt_helper(os, iss, i, mid, right, arg_sym);
      }
      if (!m_opt_doc.empty())
	  os << "\nOptions:\n";
      for (opt_doc_type::const_iterator it = m_opt_doc.begin();
	   it != m_opt_doc.end(); ++it) {
	  os << "    " << it->first;
	  int i = it->first.length() + 4;
	  std::string arg_sym;
	  std::string::size_type n = it->second.find('#');
	  if (n < it->second.size()) {
	      if (n != 0) {
		  arg_sym = it->second.substr(0, n);
		  os << '=' << set_argstyle
		     << arg_sym << clr_argstyle;
		  i += n + 1;
	      }
	      ++n;
	  }
	  else
	      n = 0;

	  std::istringstream iss(it->second.substr(n));
	  fmt_helper(os, iss, i, mid, right, arg_sym);
      }
      if (!m_opt_doc.empty() && (m_flags & disable_assignment_style) == 0)
	  os << "The \"--\" prefix of long options is optional if the "
	     << "equality sign is present.\n";
  }

  void
  cmdline::priv_insert(std::string const& opt, std::string const& doc,
		       closure_type f)
  {
      int j = 0;
      if (opt.size() != 0) {
	  for (std::string::size_type i = 0; i < opt.size(); ++i) {
	      if (opt[i] == '|') {
		  m_opt_map.insert(std::make_pair(opt.substr(j, i), f));
		  j = i+1;
	      }
	  }
	  m_opt_map.insert(std::make_pair(opt.substr(j, opt.size()), f));
	  m_opt_doc.push_back(std::make_pair(opt, doc));
      } else {
	  m_arg_list.push_back(f);
	  m_arg_doc.push_back(doc);
      }
  }

  std::string cmdline::s_prgname;
  bool cmdline::s_null_notify;
  char cmdline::s_argv_separator;

  int cmdline::s_opt_verb;
  int cmdline::s_opt_debug;

}} // more::io

// Local Variables:
// coding: utf-8
// End:
