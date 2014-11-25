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


#include "ens_conv.h"

#include <more/phys/ens.h>
#include <more/io/fstream.h>
#include <more/io/sourceref.h>
#include <more/diag/debug.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <memory>
#include <string>
#include <algorithm>
#include <assert.h>


#if 0
#  define PARSE_ERROR(msg) throw parse_error((msg))
#else
#  define PARSE_ERROR(msg) \
	(*nucleus::s_clog << m_buf.where() << ": " << (msg) << '\n' \
			  << m_buf.line_str(0, 80) << std::endl)
#endif

#if 0  // Verbose debugging
namespace {
  struct _tracer {
      _tracer(std::string name)
	  : m_name(name)
      {
	  std::cerr << "debug: " << indent//std::string(' ', indent)
		    << "--> " << name << std::endl;
	  indent += 4;
      }

      ~_tracer()
      {
	  indent -= 4;
	  std::cerr << "debug: " << indent//std::string(' ', indent)
		    << "<-- " << m_name << std::endl;
      }

      std::string m_name;
      static int indent;
  };
  int _tracer::indent = 0;
}
#  define D_TRACE(name) _tracer _traceme((name));
#else
#  define D_TRACE(name) ((void)0)
#endif


namespace more {
namespace phys {
namespace ens {

#ifdef MORE_CHECK_ALLOCATIONS
  checkmem _checkmem_parser;
#endif

  symbol_t
  str_to_symbol(std::string const& sym)
  {
      switch (sym.size()) {
      case 1:
	  switch (sym[0]) {
	  case 'E':
	      return symbol_E;
	  case 'J':
	      return symbol_J;
	  case 'T':
	      return symbol_T;
	  case 'L':
	      return symbol_L;
	  case 'S':
	      return symbol_S;
	  default:
	      break;
	  }
	  break;

      case 2:
	  switch (sym[0]) {
	  case 'I':
	      switch (sym[1]) {
	      case 'B':
		  return symbol_IB;
	      case 'E':
		  return symbol_IE;
	      case 'A':
		  return symbol_IA;
	      default:
		  break;
	      }
	      break;
	  case 'U':
	      switch (sym[1]) {
	      case 'N':
		  return symbol_UN;
	      default:
		  break;
	      }
	  case 'T':
	      switch (sym[1]) {
	      case 'I':
		  return symbol_TI;
	      default:
		  break;
	      }
	  case 'H':
	      switch (sym[1]) {
	      case 'F':
		  return symbol_HF;
	      default:
		  break;
	      }
	  default:
	      break;
	  }

      case 5:
	  if (sym == "LOGFT")
	      return symbol_LOGFT;
	  break;

      default:
	  break;
      }
      return symbol_none;
  }


  //
  // record_buffer
  //
  struct record_buffer
  {
      static size_t const n_buf_add = 10;
      static int const n_rows_init = 4;

      explicit record_buffer(std::istream&, char const*);
      ~record_buffer();

      bool next();
      bool eof() { return m_is_eof; }
      std::string line_str(int, int);
      std::string multiline_str(int, int);
      char at(int n) { return m_buf[n]; }
      void rewind() { m_is.seekg(0); m_where.rewind(); m_is_eof = false; }
      more::io::sourceref const& where() { return m_where; }

    private:
      std::istream& m_is;
      int m_nrows, m_nrows_capacity;
      more::io::sourceref m_where;
      bool m_is_eof;
      char *m_buf;
      friend class ensdf_parser;
  };

  record_buffer::~record_buffer()
  {
      delete m_buf;
  }

  record_buffer::record_buffer(std::istream& is, char const* file_name)
      : m_is(is),
	m_where(file_name),
	m_is_eof(false)
  {
      int bufsize;
      bufsize = 80*n_rows_init + n_buf_add;
      m_buf = new char[2*bufsize];
      m_nrows = 0;
      m_nrows_capacity = n_rows_init;
      if (!m_is.getline(m_buf, 90).good()) {
	  delete [] m_buf;
	  m_buf = 0;
	  throw std::runtime_error("record_buffer::record_buffer: "
				   "File error at beginning of file "
				   + std::string(file_name) + ".");
      }
      if (!next())
	  throw std::runtime_error("more::phys::ens::record_buffer"
				   "::record_buffer: File error.");
  }

  bool
  record_buffer::next()
  {
      m_where.newline(m_nrows);
      if (m_nrows > 0)
	  std::memcpy(m_buf, m_buf+m_nrows*80, (size_t)80);
      m_nrows = 0;
      do {
	  if (m_is.eof()) {
	      if (m_nrows > 0)
		  return true;
	      m_is_eof = true;
	      return false;
	  }
	  ++m_nrows;
	  if (m_nrows+1 >= m_nrows_capacity) {
	      int bufsize = m_nrows_capacity*160 + n_buf_add;
	      char *newbuf = new char[2*bufsize];
	      std::memcpy(newbuf, m_buf, (size_t)80*m_nrows_capacity);
	      delete m_buf;
	      m_buf = newbuf;
	      m_nrows_capacity *= 2;
	  }
	  m_is.getline(m_buf + m_nrows*80, 90);
	  if (m_is.eof())
	      continue;
	  if (m_is.fail()) {
	      if (m_is.bad())
		  nucleus::clog() << m_where << ": error: Line too long.\n";
	      else {
		  m_is.clear();
		  nucleus::clog() << m_where << ": error: Read error.\n";
		  if (m_is.ignore(0x8000, '\n').fail())
		      m_is_eof = true;
	      }
	      return false;
	  }
	  size_t ln_len = std::strlen(m_buf+m_nrows*80);

	  // Accept the modified format from http://ie.lbl.gov/:
	  if (ln_len && m_buf[m_nrows*80 + ln_len - 1] == '\r') {
	      --ln_len;
	      m_buf[m_nrows*80 + ln_len] = 0;
	  }
	  if (ln_len < 81) {
	      std::memset(m_buf + m_nrows*80 + ln_len, ' ',
			  80 - ln_len);
	      *(m_buf + (m_nrows + 1)*80) = 0;
	      assert(std::strlen(m_buf+m_nrows*80) == 80);
	  }

      } while (m_buf[m_nrows*80+5] != ' ' &&
	       m_buf[m_nrows*80+5] != '1');
      return true;
  }

  std::string
  record_buffer::line_str(int beg, int end)
  {
      assert(0 <= beg && beg < end && end <= 80);
//      assert(line < m_nrows);
//      beg += line*80;
      while (m_buf[--end] == ' ' && end > beg)
	  ;
      ++end;
      while (m_buf[beg] == ' ' && beg < end)
	  ++beg;
      m_where.set_column(beg);

      std::string ret = std::string(m_buf + beg, m_buf + end);
      //std::cerr<<"line_str("<<beg<<","<<end<<") = \"" << ret << "\"\n";
      return ret;
  }

  std::string
  record_buffer::multiline_str(int col_beg, int col_end)
  {
      assert(0 <= col_beg && col_beg < col_end && col_end <= 80);
      std::string s;
      char *bp;
      int col_beg_0 = col_beg;
      int col_end_last = col_end;

      if (m_nrows == 1)
	  return line_str(col_beg, col_end);

      bp = m_buf;
      while (bp[col_beg_0] == ' ' && col_beg_0 < col_end)
	  ++col_beg_0;
      bp = m_buf + (m_nrows-1)*80;
      while (bp[--col_end_last] == ' ' && col_end_last > col_beg)
	  ;
      ++col_end_last;

      bp = m_buf;
      s.append(bp + col_beg_0, bp + col_end);
      s += '\n';
      for (int i = 1; i < m_nrows-1; ++i) {
	  bp += 80;
  	  s.append(bp + col_beg, bp + col_end);
  	  s += '\n';
      }
      bp += 80;
      s.append(bp + col_beg, bp + col_end_last);
      m_where.set_column(col_beg);
      return s;
  }



  //
  // parser
  //
  struct parser
  {
      explicit parser(std::istream& is, char const* file_name)
	  : m_buf(is, file_name) {}

      void		parse(nucleus&);
      dataset*		parse_dataset();
      void		parse_comments(gen::link<rec_comment>&);
      rec_history*	parse_history();
      rec_xref*		parse_xref();
      void		parse_radiations(gen::link<rec_radiation>&);
      void		parse_levels(gen::link<rec_level>&);
      void		parse_radiation_base(rec_radiation* r);
      rec_beta_mi*	parse_beta_mi();
      rec_beta_pl*	parse_beta_pl();
      rec_alpha*	parse_alpha();
      rec_particle*	parse_particle();
      rec_gamma*	parse_gamma();

      template<typename OutputIterator>
      void parse_ang_mom_xfer(OutputIterator);

    private:
      std::string line_str(int i, int j) { return m_buf.line_str(i, j); }

      record_buffer m_buf;
      std::string m_nucid;
      dataset* m_ds;
  };

  void
  parser::parse(nucleus& nucl)
  {
      do
	  if (dataset* ds = parse_dataset())
	      nucl.m_datasets->join_back(ds);
      while (!m_buf.eof());
  }

  dataset*
  parser::parse_dataset() try
  {
      D_TRACE("parse_dataset");
      if (m_buf.eof())
	  return 0;
      more::diag::auto_ptr<dataset> ds(m_ds = MORE_NEW(dataset));

      // Parse Identification record.
      while (m_buf.at(6) != ' ' || m_buf.at(7) != ' ' ||
	     m_buf.at(8) != ' ' || m_buf.at(9) == ' ') {
	  PARSE_ERROR("Missing identification record.");
	  // Skip to next dataset if any.
	  do {
	      int i = 0;
	      while (i < 80 && m_buf.at(i) == ' ')
		  ++i;
	      if (i == 80)
		  break;
	  } while (m_buf.next());
	  if (!m_buf.next())
	      return 0;
      }
      m_nucid = m_buf.line_str(0, 5);
      ds->m_ident.m_nucid = str_to_nucid(m_nucid);
      ds->m_ident.m_dsid = m_buf.multiline_str(9, 39);
      if (!str_to_keynum_list(m_buf.multiline_str(39, 65), ds->m_ident.m_dsref))
	  PARSE_ERROR("Ill-formed DSREF (KEYNUM list).");
      ds->m_ident.m_pub = publication_info_t(m_buf.line_str(65, 74));
      if (!str_to_date(m_buf.line_str(74, 80), ds->m_ident.m_date))
	  PARSE_ERROR("Ill-formed date.");
      m_buf.next();

      // Parse History, Cross-Reference, Comments, Q-Value,
      // Production Normalization and Normalization
      for (bool break2_p = false; !break2_p; ) {
	  parse_comments(ds->m_comments);
	  if (m_buf.at(6) != ' ' && m_buf.at(7) != 'N')
	      break;

	  switch (m_buf.at(7)) {
	  case 'H':
	      ds->m_history.join_back(parse_history());
	      if (!m_buf.next())
		  break2_p = true;
	      break;

	  case 'X':
	      if (m_buf.line_str(0, 5) != m_nucid)
		  PARSE_ERROR("NUCID does not match dataset");
	      ds->m_xref.join_back(parse_xref());
	      if (!m_buf.next())
		  break2_p = true;
	      break;

	  case 'Q':
	      if (m_buf.line_str(0, 5) != m_nucid)
		  PARSE_ERROR("NUCID does not match dataset");
	      if (ds->m_qvalue) {
		  PARSE_ERROR("Multiple Q-Value Records in dataset.");
		  m_buf.next();
		  break;
	      }
	      else {
		  // Parse Q-Value and Q-Value Comment Records
		  rec_qvalue* r = ds->m_qvalue = MORE_NEW(rec_qvalue);
		  if (!str_to_energy(line_str(9, 19), line_str(19, 21),
				     r->m_qmi))
		      PARSE_ERROR("Ill-formed Q-.");
		  if (!str_to_energy(line_str(21, 29), line_str(29, 31),
				     r->m_sn))
		      PARSE_ERROR("Ill-formed Sn.");
		  if (!str_to_energy(line_str(31, 39), line_str(39, 41),
				     r->m_sp))
		      PARSE_ERROR("Ill-formed Sp.");
		  if (!str_to_energy(line_str(41, 49), line_str(49, 55),
				     r->m_qa))
		      PARSE_ERROR("Ill-formed Qα.");
		  if (!str_to_keynum_list(line_str(55, 80), r->m_qref))
		      PARSE_ERROR("Ill-formed QREF (KEYNUM list).");
		  if (m_buf.next())
		      parse_comments(r->m_comments);
		  else
		      break2_p = true;
	      }
	      break;

	  case 'P':
	      // Parent Record
	      {
		  unsigned int index;
		  char ch_idx = m_buf.at(8);
		  if (ch_idx == ' ')
		      index = 0;
		  else if (std::isdigit(ch_idx))
		      index = ch_idx - '0';
		  else {
		      PARSE_ERROR("Non-integer index of parent record.");
		      index = 0;
		  }

		  decay_info* pnp = ds->find_or_insert_pnp(index);
		  if (pnp->m_parent) {
		      PARSE_ERROR("Multiple parent records with the same "
				  "index.");
		      m_buf.next();
		      break;
		  }

		  // Parse Parent and Parent Comment Records
		  rec_parent* r = pnp->m_parent = MORE_NEW(rec_parent);
		  r->m_nucid = str_to_nucid(m_buf.line_str(0, 5));
		  energy_index_t E_ref;
		  if (!str_to_energy(line_str(9, 19), line_str(19, 21),
				     r->m_E, E_ref))
		      PARSE_ERROR("Ill-formed E.");
		  r->m_E_ref = E_ref;
		  str_to_jpi_expr(line_str(21, 39), r->m_J);
		  bool as_width_p;
		  if (!str_to_halflife(line_str(39, 49), line_str(49, 55),
				       r->m_T, as_width_p))
		      PARSE_ERROR("Ill-formed time in parent record.");
		  r->m_T_as_width = as_width_p;
		  if (!str_to_energy(line_str(64, 74), line_str(74, 76),
				     r->m_QP))
		      PARSE_ERROR("Ill-formed QP.");
		  std::string ion = line_str(76, 80);
		  if (!ion.empty())
		      nucleus::clog() << "ION = " << ion << std::endl;
		  //XXX m_ion
		  if (m_buf.next())
		      parse_comments(r->m_comments);
	      }
	      break;

	  case 'N':
	      if (m_buf.line_str(0, 5) != m_nucid)
		  PARSE_ERROR("NUCID does not match dataset");

	      // Normalization Records
	      if (m_buf.at(6) == ' ') {
		  decay_info* pnp = ds->find_or_insert_pnp(m_buf.at(7));
		  if (pnp->m_norm) {
		      PARSE_ERROR("Multiple normalization records with "
				  "same index.");
		      m_buf.next();
		      break;
		  }
		  rec_norm* r = pnp->m_norm = MORE_NEW(rec_norm);
		  str_to_confiv(line_str(9, 19), line_str(19, 21), r->m_nr);
		  str_to_confiv(line_str(21, 29), line_str(29, 31), r->m_nt);
		  str_to_confiv(line_str(31, 39), line_str(39, 41), r->m_br);
		  str_to_confiv(line_str(41, 49), line_str(49, 55), r->m_nb);
		  str_to_confiv(line_str(55, 62), line_str(62, 64), r->m_np);
		  r->m_nr /= 100; // Given in per 100 in ENSDF
		  r->m_nt /= 100;
		  r->m_nb /= 100;
		  if (m_buf.next())
		      parse_comments(r->m_comments);
	      }

	      // Production Normalization
	      else if (m_buf.at(6) == 'P') {
		  decay_info* pnp = ds->find_or_insert_pnp(m_buf.at(7));
		  if (pnp->m_pnorm) {
		      PARSE_ERROR("Multiple production normalizations for "
				  "parent.");
		      m_buf.next();
		      break;
		  }
		  rec_pnorm* r = pnp->m_pnorm = MORE_NEW(rec_pnorm);
		  str_to_confiv(line_str(9, 19), line_str(19, 21),
				r->m_nr_br);
		  str_to_confiv(line_str(21, 29), line_str(29, 31),
				r->m_nt_br);
		  str_to_confiv(line_str(41, 49), line_str(49, 55),
				r->m_nb_br);
		  str_to_confiv(line_str(55, 62), line_str(62, 64), r->m_np);
		  r->m_nr_br /= 100; // Given in per 100 in ENSDF
		  r->m_nt_br /= 100;
		  r->m_nb_br /= 100;
		  if (m_buf.next())
		      parse_comments(r->m_comments);
	      }
	      else {
		  PARSE_ERROR("Bad char in col 6.");
		  m_buf.next();
	      }
	      break;

	  default:
	      break2_p = true;
	      break;
	  }
      }

      // Parse Unplaced Radiations
      parse_radiations(ds->m_unpl_rads);

      // Parse Levels
      parse_levels(ds->m_levels);

      if (m_buf.eof())
	  PARSE_ERROR("Missing end record.");
      else {
	  int i = 0;
	  while (i < 80 && m_buf.at(i) == ' ')
	      ++i;
	  if (i != 80)
	      PARSE_ERROR("Missing end record.");
	  else
	      m_buf.next();
      }

      return ds.release();
  }
  catch (parse_error& xc) {
      std::ostringstream oss;
      oss << m_buf.where() << ": " << xc.what();
      oss << " in ‘" << m_buf.line_str(0, 80) << "...’";
      PARSE_ERROR(oss.str());
      return 0;
  }

  void
  parser::parse_comments(gen::link<rec_comment>& lst)
  {
      D_TRACE("parse_comments");
      bool decode_p, fill_p, doc_p = false, unknown_p = false;
      for (;;) {
	  switch (m_buf.at(6)) {
	      // NOTE. 'd' is undocumented, but used (e.g. ap044ca.ens:167).
	  case 'D': decode_p = false; fill_p = false; doc_p = true; break;
	  case 'd': decode_p = true;  fill_p = false; doc_p = true; break;
	  case 'C': decode_p = false; fill_p = true;  break;
	  case 'c': decode_p = true;  fill_p = true;  break;
	  case 'T': decode_p = false; fill_p = false; break;
	  case 't': decode_p = true;  fill_p = false; break;
	  case 'P': case 'p':
	      // UNDOCUMENTED. Used for publication ref in Mo-99 set.
	      if (std::tolower(m_buf.at(7)) != 'n') {
		  decode_p = false; fill_p = false;
		  break;
	      }
	  case ' ':
	      return;

	      // Other characters seems to be used, as well. These are
	      // undocumented:
	  case 'h': case 'u':
	      decode_p = true;  fill_p = false;
	      unknown_p = true;
	      break;
	  default:
	      PARSE_ERROR(std::string("Unexpected character in col 6."));
	      unknown_p = true;
	      m_buf.next();
	      continue;
	  }

	  more::diag::auto_ptr<rec_comment> com(MORE_NEW(rec_comment));
	  com->m_rtype = chars_to_rtype(m_buf.at(7), m_buf.at(8));
	  com->m_psym = char_to_psym(m_buf.at(8));
	  if (com->m_psym == particle_kind_invalid && !unknown_p)
	      PARSE_ERROR(std::string("Invalid PSYM ‘") + m_buf.at(8) + "’.");
	  std::string str = m_buf.multiline_str(9, 80);

	  size_t i = str.find('$');
	  if (i == 0)
	      str = str.substr(1);
	  else if (i != std::string::npos) {
	      std::string sfl = str.substr(0, i);
	      str = str.substr(i + 1);
	      if (str.compare(0, 6, "LABEL=") == 0) {
		  symbol_t sym = str_to_symbol(sfl);
		  if (sym == symbol_none)
		      PARSE_ERROR("Unknown symbol "+sfl+" to relabel.");
		  else {
		      m_ds->m_relabel_intset |= (1 << sym);
		      m_ds->m_relabel_map.insert(std::make_pair(sym, str));
		  }
	      }
	      else {
		  i = 0;
		  while (i < sfl.size()) {
		      size_t j = i;
		      while (i < sfl.size() && sfl[i] != ',' && sfl[i] != '(')
			  ++i;
		      if (i == j)
			  PARSE_ERROR("Missing ‘SYM’ in ‘SYM(FLAG)’ list.");
		      gen::identifier sym = sfl.substr(j, i-j).c_str();
		      comment_refs_t flags;
		      if (sfl[i] == '(') {
			  ++i;
			  while (i < sfl.size() && sfl[i] != ')') {
			      flags.insert(sfl[i]);
			      if (sfl[++i] == ',')
				  ++i;
			  }
			  if (i >= sfl.size())
			      PARSE_ERROR("Missing ‘)’ in ‘SYM(FLAG)’ form.");
		      }
// 		  else if (sfl[i] == ',') {
// 		      ++i;
// 		      if (!sfl[i])
// 			  PARSE_ERROR("Missing flag after comma in "
// 				      "‘SYM(FLAG0,FLAG1,...)’ form.");
// 		  }
		      com->m_symflags.insert(std::make_pair(sym, flags));
		      while (i < sfl.size() && sfl[i] == ',')
			  ++i;
		  }
	      }
	  }
	  if (decode_p)
	      str = decode_text(str, markup_language_html);

	  com->m_ctext = str;
	  com->m_fill_p = fill_p;
	  com->m_doc_p = doc_p;

	  lst.join_back(com.release());
	  if (!m_buf.next())
	      break;
      }
  }

  rec_history*
  parser::parse_history()
  {
      D_TRACE("parse_history");
      more::diag::auto_ptr<rec_history> his(MORE_NEW(rec_history));
      his->m_nucid = str_to_nucid(m_buf.line_str(0, 5));
      std::string s = m_buf.multiline_str(9, 80);
      std::string::size_type i = 0, j = 0;
      for (;;) {
	  while (i < s.size() && std::isspace(s[i]))
	      ++i;
	  j = s.find('$', i);
	  if (j == std::string::npos)
	      j = s.size();
	  if (i == j)
	      break;
	  std::string::size_type k = s.find('=', i);
	  if (k > j) {
	      k = i;  // undocumented
//  	      assert(0 <= i && i <= j && j <= s.size());
//  	      nucleus::clog() << m_buf.where() << ": warning: "
//  			<< "Missing ‘=’ in history item ‘"
//  			<< s.substr(i, j - i) << "’.\n";
//  	      return his.release();
	  }
	  std::string::size_type l = k;
	  while (i < k && std::isspace(s[k-1]))
	      --k;
	  if (s[l] == '=')
	      ++l;
	  while (l < j && std::isspace(s[l]))
	      ++l;
	  assert(i <= k && k <= s.size());
	  assert(k < j && j <= s.size());
	  std::string key = s.substr(i, k - i);
	  std::string val = s.substr(l, j - l);
	  std::transform(key.begin(), key.end(), key.begin(),
			 std::ptr_fun((int (*)(int))std::tolower));
	  if (key == "typ")
	      his->m_typ = val;
	  else if (key == "aut")
	      his->m_aut = val;
	  else if (key == "dat" || key == "date") { // UNDOCUMENTED: "date"
	      if (!str_vulgar_to_date(val, his->m_dat))
		  PARSE_ERROR("Invalid DAT date.");
	  }
	  else if (key == "cut") {
	      if (!str_vulgar_to_date(val, his->m_cut))
		  PARSE_ERROR("Invalid CUT date.");
	  }
	  else if (key == "cit")
	      his->m_cit = val;
	  else if (key == "com")
	      his->m_com = val;
	  else if (key.empty()) // UNDOCUMENTED.
	      his->m_keyless.push_back(val);
	  else {
	      nucleus::clog() << m_buf.where()
			      << ": warning: Got unknown history element ‘"
			      << key << "’ ↦ ‘" << val << "’\n";
// 	      his->m_entries.insert(std::make_pair(key, val));
	  }
	  if (j == s.size())
	      break;
	  i = j + 1;
      }
      return his.release();
  }

  rec_xref*
  parser::parse_xref()
  {
      D_TRACE("parse_xref");
      more::diag::auto_ptr<rec_xref> xr(MORE_NEW(rec_xref));
      xr->m_dssym = m_buf.at(8);
      xr->m_dsid = m_buf.multiline_str(9, 39);
      return xr.release();
  }

  void
  parser::parse_radiations(gen::link<rec_radiation>& lst)
  {
      do {
	  switch (m_buf.at(6)) {
	  case ' ':
	      switch (m_buf.at(7)) {
	      case 'B':
		  lst.join_back(parse_beta_mi());
		  break;
	      case 'E':
		  lst.join_back(parse_beta_pl());
		  break;
	      case 'A':
		  lst.join_back(parse_alpha());
		  break;
	      case ' ':
		  if (m_buf.at(8) == ' ')
		      return;
		  // fall-through
	      case 'D':
		  lst.join_back(parse_particle());
		  break;
	      case 'G':
		  lst.join_back(parse_gamma());
		  break;
	      default:
		  return;
	      }
	      break;

	  default:
	      PARSE_ERROR("Invalid char in col 6 while parsing radiations.");
	      m_buf.next();
	      break;
	  }
      } while (!m_buf.eof());
  }

  void
  parser::parse_levels(gen::link<rec_level>& lst)
  {
      D_TRACE("parse levels");
      while (!m_buf.eof() && m_buf.at(7) == 'L') {
	  if (m_buf.line_str(0, 5) != m_nucid)
	      PARSE_ERROR("NUCID does not match dataset.");
	  if (m_buf.at(6) != ' ') {
	      nucleus::clog()
		  << m_buf.where()
		  << ": warning: Did not expect '" << m_buf.at(6)
		  << "' in col 6 here.\n";
	      break;
	  }
	  more::diag::auto_ptr<rec_level> lev(MORE_NEW(rec_level));
	  energy_index_t E_ref;
	  if (!str_to_energy(line_str(9, 19), line_str(19, 21),
			     lev->m_E, E_ref))
	      PARSE_ERROR("Ill-formed E.");
	  lev->m_E_ref = E_ref;
	  str_to_jpi_expr(m_buf.line_str(21, 39), lev->m_J);
	  bool as_width;
	  if (!str_to_halflife(m_buf.line_str(39, 49), m_buf.line_str(49, 55),
			       lev->m_T, as_width))
	      PARSE_ERROR("Ill-formed time in level record.");
	  lev->m_T_as_width = as_width;

	  {
	      // XXX It is not completely clear how to relate L and S
	      // values in all cases.  E.g. ‘ar076kr’ contains
	      // ‘(2,3,4)’ for L and ‘1.7’ for S, which probably means
	      // that all entries in L are related to the same value,
	      // though the ENSDF manual states that S and DS may be
	      // lists, with each entry related to a separate L value.
	      std::istringstream iss_S(m_buf.line_str(64, 74));
	      std::istringstream iss_DS(m_buf.line_str(74, 76));
	      std::list<ang_mom_xfer_elt> L;
	      parse_ang_mom_xfer(std::back_inserter(L));
	      std::list<ang_mom_xfer_elt>::iterator it_L = L.begin();
	      confiv_t S_default;
	      bool uncert_p = false;
	      for (;;) {
		  char ch;
		  std::string s_val;
		  std::string s_dev;
		  if (iss_S.peek() == '(') {
		      uncert_p = true;
		      iss_S.get();
		  }
		  for (;;) {
		      ch = iss_S.get();
		      if (iss_S.eof())
			  break;
		      // A '+' sign prefixes a value for the S field
		      // in the last 2000-11 dataset of K-46.  It is
		      // also a separator.
		      if ((ch == '+' && !s_val.empty())
			  || ch == ',' || ch == ')')
			  break;
		      s_val += ch;
		  }
		  for (;;) {
		      ch = iss_DS.get();
		      if (iss_DS.eof())
			  break;
		      if (ch == '+' || ch == ',' || ch == ')')
			  break;
		      s_dev += ch;
		  }
		  if (s_val.empty()) {
		      if (!s_dev.empty()) {
			  PARSE_ERROR("Got strength deviation (‘DS’) with no "
				      "corresponding value (‘S’).");
			  return;
		      }
		      break;
		  }
		  confiv_t S;
		  str_to_confiv(s_val, s_dev, S);
		  if (uncert_p)
		      S.be_uncertain();
		  ang_mom_xfer_elt l;
		  if (it_L != L.end()) {
		      l = *it_L;
		      ++it_L;
		  }
		  lev->m_LS.insert(std::make_pair(l, S));
		  if (ch == ')')
		      uncert_p = false;
	      }
	      if (lev->m_LS.size() == 1)
		  S_default = lev->m_LS.begin()->second;
	      while (it_L != L.end()) {
		  lev->m_LS.insert(std::make_pair(*it_L, S_default));
		  ++it_L;
	      }
	  }


#if 0 // XXX
	  // (L, S DS) List
	  {
	      std::string str_L = m_buf.line_str(55, 64);  // V22
	      std::string str_S = m_buf.line_str(64, 74);  // V21
	      std::string str_DS = m_buf.line_str(74, 76); // V11
	      std::string::size_type i_L = 0, i_S = 0;
	      for (;;) {
		  if (i_L == str_L.size() || i_S == str_L.size()) {
		      if (i_L != str_L.size() || i_S != str_L.size())
			  PARSE_ERROR("L and S fields do not match.");
		      break;
		  }
		  real_type L;
		  bool par_p;
		  std::string::size_type j = str_L.find_first_of("+,", i_L);
		  if (j == std::string::npos)
		      j = str_L.size();
		  if (!str_to_number(str_L.substr(i_L, j - i_L), L, par_p)) {
		      PARSE_ERROR("Invalid L value "
				  + str_L.substr(i_L, j - i_L)
				  + " in " + str_L);
		      break;
		  }
		  i_L = j;
		  confiv_t x;
		  j = str_S.find_first_of("+,", i_S);
		  if (j == std::string::npos)
		      j = str_S.size();
		  if (!str_to_confiv(str_S.substr(i_S, j - i_S), str_DS, x)) {
		      PARSE_ERROR("Invalid S value "
				  + str_S.substr(i_S, j - i_S)
				  + " in " + str_S);
		      break;
		  }
		  i_S = j;
		  // XXX check deviations
		  lev->m_LS.push_back(
		      std::make_pair(L_type(more::iround(L), par_p), x));
	      }
	  }
#endif
	  if (m_buf.at(76) != ' ')
	      lev->m_C.insert(m_buf.at(76));

	  // Metastable specification
	  bool inval_p = false;
	  lev->m_MS = 0;
	  if (m_buf.at(77) == 'M') {
	      if (m_buf.at(78) == ' ')
		  lev->m_MS = 15;
	      else if (std::isdigit(m_buf.at(78)))
		  lev->m_MS = m_buf.at(78) - '0' + 1;
	      else
		  inval_p = true;
	  }
	  else if (m_buf.at(77) != ' ' || m_buf.at(78) != ' ')
	      inval_p = true;
	  if (inval_p)
	      nucleus::clog()
		  << m_buf.where() << ": warning: "
		  << "Invalid metastable spec ‘"
		  << m_buf.at(77) << m_buf.at(78) << "’\n";

	  if (!m_buf.next())
	      break;
	  parse_comments(lev->m_comments);
	  parse_radiations(lev->m_radiations);
	  lst.join_back(lev.release());
      }
  }

  void
  parser::parse_radiation_base(rec_radiation* r)
  {
      if (m_buf.line_str(0, 5) != m_nucid)
	  PARSE_ERROR("NUCID does not match dataset");
      energy_index_t E_ref;
      if (!str_to_energy(line_str(9, 19), line_str(19, 21), r->m_E, E_ref))
	  PARSE_ERROR("Ill-formed E.");
      r->m_E_ref = E_ref;
      str_to_confiv(line_str(21, 29), line_str(29, 31), r->m_IX, true);
      if (m_buf.at(76) != ' ')
	  r->m_C.insert(m_buf.at(76));
      switch (m_buf.at(79)) {
	case ' ': r->m_rstatus = rstatus_observed; break;
	case '?': r->m_rstatus = rstatus_uncertain; break;
	case 'S': r->m_rstatus = rstatus_predicted; break;
	default:
	  PARSE_ERROR(std::string("Bad Q flag ‘") + m_buf.at(79) + "’.");
	  break;
      }
  }

  rec_beta_mi*
  parser::parse_beta_mi()
  {
      D_TRACE("parse_beta_mi");

      more::diag::auto_ptr<rec_beta_mi> r(MORE_NEW(rec_beta_mi));
      parse_radiation_base(r.get());

      str_to_confiv(line_str(41, 49), line_str(49, 55), r->m_log_ft);

      char ch = m_buf.at(77);
      if (std::isdigit(ch))
	  r->m_un_forbiddenness = ch - '0';
      else {
	  r->m_un_forbiddenness = 0;
	  if (ch != ' ')
	      PARSE_ERROR("Unexpected character for UN.");
      }
      if ((ch = m_buf.at(78)) == 'U')
	  r->m_un_unique = 1;
      else {
	  r->m_un_unique = 0;
	  if (ch != ' ')
	      PARSE_ERROR("Unexpected character for UN.");
      }

      if (m_buf.next())
	  parse_comments(r->m_comments);
      return r.release();
  }

  rec_beta_pl*
  parser::parse_beta_pl()
  {
      D_TRACE("parse_beta_pl");
      more::diag::auto_ptr<rec_beta_pl> r(MORE_NEW(rec_beta_pl));
      parse_radiation_base(r.get());

      str_to_confiv(line_str(41, 49), line_str(49, 55), r->m_log_ft);
      str_to_confiv(line_str(31, 39), line_str(39, 41), r->m_IE, true);
      str_to_confiv(line_str(64, 74), line_str(74, 76), r->m_TI, true);

      char ch = m_buf.at(77);
      if (std::isdigit(ch))
	  r->m_un_forbiddenness = ch - '0';
      else {
	  r->m_un_forbiddenness = 0;
	  if (ch != ' ')
	      PARSE_ERROR("Unexpected character for UN.");
      }
      if ((ch = m_buf.at(78)) == 'U')
	  r->m_un_unique = 1;
      else {
	  r->m_un_unique = 0;
	  if (ch != ' ')
	      PARSE_ERROR("Unexpected character for UN.");
      }

      if (m_buf.next())
	  parse_comments(r->m_comments);
      return r.release();
  }

  rec_alpha*
  parser::parse_alpha()
  {
      D_TRACE("parse_alpha");
      more::diag::auto_ptr<rec_alpha> r(MORE_NEW(rec_alpha));
      parse_radiation_base(r.get());
      r->m_IX /= 100; // Given in percent in the ENSDF.
      str_to_confiv(line_str(31, 39), line_str(39, 41), r->m_HF);

      if (m_buf.next())
	  parse_comments(r->m_comments);
      return r.release();
  }

  rec_particle*
  parser::parse_particle()
  {
      D_TRACE("parse_particle");
      more::diag::auto_ptr<rec_particle> r(MORE_NEW(rec_particle));
      parse_radiation_base(r.get());
      r->m_IX /= 100; // Given in percent in the ENSDF.
      r->m_delayed_p = m_buf.at(7) == 'D'? 1 : 0;
      r->m_particle = char_to_psym(m_buf.at(8));
      if (r->m_particle == particle_kind_invalid)
	  PARSE_ERROR(std::string("Invalid PSYM ‘") + m_buf.at(8) + "’.");

      switch (m_buf.at(77)) {
      case ' ': r->m_coin = fuzzy_false; break;
      case '?': r->m_coin = fuzzy_probable; break;
      case 'C': r->m_coin = fuzzy_true; break;
      default:
	  PARSE_ERROR(std::string("Bad COIN flag ‘") + m_buf.at(77) + "’.");
	  r->m_coin = fuzzy_invalid;
	  break;
      }

      // UNDOCUMENTED.  The EI field is seen to contain 'SN+...'.
      std::string s = line_str(31, 39);
      if (s.empty())
	  r->m_EI_given = 0;
      else {
	  bool par_p;
	  if (!str_to_number(s, r->m_EI, par_p))
	      PARSE_ERROR("Ill-formed number " + s);
	  if (par_p)
	      r->m_EI_given = 1;
	  else
	      r->m_EI_given = 2;
      }

      str_to_confiv(line_str(39, 49), line_str(49, 55), r->m_T);
      r->m_T *= SI::keV;

      parse_ang_mom_xfer(std::inserter(r->m_L, r->m_L.begin()));

      if (m_buf.next())
	  parse_comments(r->m_comments);
      return r.release();
  }

  rec_gamma*
  parser::parse_gamma()
  {
      D_TRACE("parse_gamma");
      more::diag::auto_ptr<rec_gamma> r(MORE_NEW(rec_gamma));
      parse_radiation_base(r.get());
      r->m_M = multipolarity_t(line_str(31, 41));
      std::string s41 = line_str(41, 49);
      str_to_confiv(s41, line_str(49, 55), r->m_MR);
      if (s41.empty() || (s41[0] != '+' && s41[0] != '-'))
	  r->m_MR.be_signless();
      str_to_confiv(line_str(55, 62), line_str(62, 64), r->m_CC);
      str_to_confiv(line_str(64, 74), line_str(74, 76), r->m_TI, true);
      switch (m_buf.at(77)) {
      case ' ': r->m_coin = fuzzy_false; break;
      case '?': r->m_coin = fuzzy_probable; break;
      case 'C': r->m_coin = fuzzy_true; break;
      default:
	  PARSE_ERROR("Unexpected character for COIN.");
	  r->m_coin = fuzzy_invalid;
	  break;
      }

      if (m_buf.next())
	  parse_comments(r->m_comments);
      return r.release();
  }

  void
  parse(std::istream& is, char const* name, nucleus& nucl)
  {
      parser p(is, name);
      p.parse(nucl);
  }

  template<typename OutputIterator>
  void
  parser::parse_ang_mom_xfer(OutputIterator it_out)
  {
      std::istringstream iss(line_str(55, 64));
      bool uncert_p = false;
      bool assumed_p = false;
      for (;;) {
	  bool ge_p = false;
	  bool le_p = false;
	  bool gt_p = false;
	  bool lt_p = false;
	  bool is_M = false;
	  bool is_E = false;
	  bool is_D = false;
	  bool is_Q = false;
	  char ch;
	  iss >> ch;
	  if (iss.eof())
	      break;
	  if (ch == '(') {
	      uncert_p = true;
	      iss >> ch;
	  }
	  else if (ch == '[') {
	      assumed_p = true;
	      iss >> ch;
	  }
	  if (ch == '<') { // UNDOCUMENTED
	      iss >> ch;
	      if (ch == '=') {
		  le_p = true;
		  iss >> ch;
	      }
	      else
		  lt_p = true;
	  }
	  else if (ch == '>') { // UNDOCUMENTED
	      iss >> ch;
	      if (ch == '=') {
		  ge_p = true;
		  iss >> ch;
	      }
	      else
		  gt_p = true;
	  }
	  if (std::isalpha(ch)) {
	      if (std::tolower(ch) == 'l') {
		  ch = iss.get();
		  if (std::tolower(ch) == 'e')
		      le_p = true;
		  else if (std::tolower(ch) == 't')
		      lt_p = true;  // UNDOCUMENTED
		  else {
		      PARSE_ERROR("Expected ‘e’ after ‘l’ in ‘L’ form.");
		      return;
		  }
	      }
	      else if (std::tolower(ch) == 'g') {
		  ch = iss.get();
		  if (std::tolower(ch) == 'e')
		      ge_p = true;
		  else if (std::tolower(ch) == 't')
		      gt_p = true;  // UNDOCUMENTED
		  else {
		      PARSE_ERROR("Expected ‘e’ after ‘g’ in ‘L’ form.");
		      return;
		  }
	      }
	      else if (std::toupper(ch) == 'M')
		  is_M = true;
	      else if (std::toupper(ch) == 'E')
		  is_E = true;
	      else if (std::toupper(ch) == 'D')
		  is_D = true;
	      else if (std::toupper(ch) == 'Q')
		  is_Q = true;
	      else {
		  PARSE_ERROR("Unexpected character in ‘L’ form.");
		  return;
	      }
	      iss >> ch;
	  }
	  if (ch == '(')
	      uncert_p = true;
	  else if (ch == '[')
	      assumed_p = true;
	  else
	      iss.putback(ch);
	  int L;
	  if (is_D) {
	      // XXX I assume ‘D’ means magnetic dipole.  If there is
	      // a distinction, ang_mom_xfer_elt must be refined.
	      L = 1;
	      is_M = true;
	  }
	  else if (is_Q) {
	      // XXX I assume ‘Q’ means electric quadrupole.  If there
	      // is a distinction, ang_mom_xfer_elt must be refined.
	      L = 2;
	      is_E = true;
	  }
	  else {
	      // XXX One of the Co-59 1993-11 datasets has
	      // non-integral (big) L.  This is not handled here.
	      iss >> L;
	      if (iss.fail()) {
		  PARSE_ERROR("Missing number in ‘L’ form.");
		  return;
	      }
	  }
	  if (gt_p) {
	      ++L;
	      ge_p = true;
	  }
	  else if (lt_p) {
	      --L;
	      le_p = true;
	  }
	  *it_out =
	      ang_mom_xfer_elt(
		  L,
		  (ge_p? ang_mom_xfer_elt::flag_upper_limit : 0) |
		  (le_p? ang_mom_xfer_elt::flag_lower_limit : 0) |
		  (uncert_p? ang_mom_xfer_elt::flag_uncertain : 0) |
		  (assumed_p? ang_mom_xfer_elt::flag_assumed : 0) |
		  (is_M? ang_mom_xfer_elt::flag_magnetic : 0) |
		  (is_E? ang_mom_xfer_elt::flag_electric : 0));
	  ++it_out;
	  iss >> ch;
	  switch (ch) {
	  case ')':
	      if (!uncert_p) {
		  PARSE_ERROR("Unbalanced ‘)’ in ‘L’ form.");
		  return;
	      }
	      uncert_p = false;
	      iss >> ch;
	      break;
	  case ']':
	      if (!assumed_p) {
		  PARSE_ERROR("Unbalanced ‘]’ in ‘L’ form.");
		  return;
	      }
	      assumed_p = false;
	      iss >> ch;
	      break;
	  case '(':
	      if (uncert_p) {
		  PARSE_ERROR("Double ‘(’ in ‘L’ form.");
		  return;
	      }
	      uncert_p = true;
	      iss >> ch;
	      break;
	  case '[':
	      if (assumed_p) {
		  PARSE_ERROR("Double ‘[’ in ‘L’ form.");
		  return;
	      }
	      assumed_p = true;
	      iss >> ch;
	      break;
	  default:
	      break;
	  }
	  if (iss.eof())
	      break;
	  if (ch == '-') { // UNDOCUMENTED
	      int L_max;
	      iss >> L_max;
	      if (iss.fail() || ge_p || le_p || is_M || is_E || L_max <= L) {
		  PARSE_ERROR("Don't know how to deal with undocumented '-' "
			      "in ‘L’ form.");
		  return;
	      }
	      while (L <= L_max) {
		  *it_out =
		      ang_mom_xfer_elt(
			  L,
			  (uncert_p? ang_mom_xfer_elt::flag_uncertain : 0) |
			  (assumed_p? ang_mom_xfer_elt::flag_assumed : 0));
		  ++it_out;
		  ++L;
	      }
	  }
	  else if (ch != ',' && ch != '+') {
	      PARSE_ERROR("Expected ‘,’ or ‘+’ separator in ‘L’ form.");
	      return;
	  }
      }
      if (uncert_p)
	  PARSE_ERROR("Missing ‘)’ in ‘L’ form.");
      if (assumed_p)
	  PARSE_ERROR("Missing ‘]’ in ‘L’ form.");
  }


}}} // more::phys::ens

// Local Variables:
// coding: utf-8
// indent-tabs-mode: t
// End:
