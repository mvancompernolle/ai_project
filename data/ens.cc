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


#include <more/gen/iterator.h>
#include <more/phys/ens.h>
#include <more/io/fstream.h>
#include <more/io/cmdline.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <memory>
#include <string>
#include <more/diag/debug.h>

#include "ens_conv.h"
#include "ens_db.h"


namespace { const bool file_debug = false; }


namespace more {
namespace phys {
namespace ens {


#ifdef MORE_CHECK_ALLOCATIONS
  checkmem _checkmem;
#endif
  void parse(std::istream& is, char const* name, nucleus&);

  void
  print_energy_ref(std::ostream& os, energy_index_t eref)
  {
      switch (eref) {
      case energy_index_sn:
	  os << " + S↓n";
	  break;
      case energy_index_sp:
	  os << " + S↓p";
	  break;
      case energy_index_sa:
	  os << " + S↓α";
	  break;
      case energy_index_none:
	  break;
      default:
	  if (eref < 0)
	      throw std::logic_error("more::phys::ens::NN::"
				     "print_energy_ref: "
				     "Invalid energy_index_t.");
	  os << " + E↓" << eref;
	  break;
      }
  }

  void
  print_rstatus_parent(std::ostream& os, rstatus_t st)
  {
      switch (st) {
      case rstatus_uncertain:
	  os << " (uncertain)";
	  break;
      case rstatus_predicted:
	  os << " (predicted)";
	  break;
      default:
	  break;
      }
  }

  comment_refs_t::comment_refs_t(char const* beg, char const* end)
      : m_arr(beg, end)
  {
      std::sort(m_arr.begin(), m_arr.end());
  }

  void
  comment_refs_t::sync(io::syncstream& sio)
  {
      io::sync_sequence(sio, m_arr);
  }

  void
  ang_mom_xfer_elt::sync(io::syncstream& sio)
  {
      sio | m_L | m_flags;
  }

  void
  ang_mom_xfer_set::sync(io::syncstream& sio)
  {
      io::sync_associative_container(sio, *this);
  }

  void
  ang_mom_to_strength_map::sync(io::syncstream& sio)
  {
      io::sync_pair_associative_container(sio, *this);
  }

  void
  spin_parity_expr_t::sync(io::syncstream& sio)
  {
      sio | m_str;
  }

  void
  publication_info_t::sync(io::syncstream& sio)
  {
      sio | m_str;
  }


  //
  // Records and Dataset
  //
  template<typename Iterator>
    void
    dump_range(Iterator first, Iterator last, std::ostream& os)
    {
	while (first != last) {
	    first->dump(os);
	    ++first;
	}
    }

  rec_commentable_base::~rec_commentable_base()
  {
      while (!m_comments.empty())
	  MORE_DELETE(rec_comment, &*m_comments.begin());
  }

  void
  rec_commentable_base::dump_base(std::ostream& os) const
  {
      for (comment_iterator it = comment_begin(); it != comment_end(); ++it)
	  it->dump(os);
  }

  void
  rec_commentable_base::sync_base(io::syncstream& sio)
  {
      if (sio.is_output()) {
	  size_t size = std::distance(m_comments.begin(), m_comments.end());
	  sio | size;
	  io::sync_range(sio, m_comments.begin(), m_comments.end());
      }
      else {
	  size_t size;
	  sio | size;
	  while (size) {
	      rec_comment* com = new rec_comment;
	      sio | *com;
	      m_comments.join_back(com);
	      --size;
	  }
      }
  }

  void
  rec_ident::dump(std::ostream& os) const
  {
      os << "\n" << m_dsid << "\n";
      if (!m_dsref.empty()) {
	  os << "\tdsref = ‘";
	  std::copy(m_dsref.begin(), m_dsref.end(),
		    gen::ostream_iterator<citation_t>(os, "’, ‘"));
	  os << "’\n";
      }
      os << "\tpub = " << m_pub.as_string() << '\n'
	 << "\tdate = " << m_date << '\n';
  }
  void
  rec_ident::sync(more::io::syncstream& sio)
  {
      sio | m_nucid | m_dsid;
      more::io::sync_sequence(sio, m_dsref);
      sio | m_pub | m_date;
  }

  void
  rec_history::dump(std::ostream& os) const
  {
      os << "    History\n";
      if (!m_typ.empty())
	  os << "\ttyp = ‘" << m_typ << "’\n";
      if (!m_aut.empty())
	  os << "\taut = ‘" << m_aut << "’\n";
      if (m_dat.is_defined())
	  os << "\tdat = " << m_dat << '\n';
      if (m_cut.is_defined())
	  os << "\tcut = " << m_cut << '\n';
      if (!m_cit.empty())
	  os << "\tcit = ‘" << m_cit << "’\n";
      if (!m_com.empty())
	  os << "\tcom = ‘" << m_com << "’\n";
      if (!m_keyless.empty()) {
	  os << "\t‘";
	  std::copy(m_keyless.begin(), m_keyless.end(),
		    gen::ostream_iterator<std::string>(os, "’\n\t‘"));
	  os << "’\n";
      }
  }

  void
  rec_history::sync(more::io::syncstream& sio)
  {
      sio | m_nucid | m_typ | m_aut | m_dat | m_cut | m_cit | m_com;
      io::sync_sequence(sio, m_keyless);
//       io::sync_pair_associative_container(sio, m_entries);
  }

  void
  rec_qvalue::dump(std::ostream& os) const
  {
      os << "    Q-Value\n"
	 << "\tQ↑- = " << m_qmi/SI::keV << " keV\n"
	 << "\tS↓n = " << m_sn/SI::keV  << " keV\n"
	 << "\tS↓p = " << m_sp/SI::keV  << " keV\n"
	 << "\tQ↓α = " << m_qa/SI::keV  << " keV\n";
      if (!m_qref.empty()) {
	  os << "\tqref = { ‘";
	  std::copy(m_qref.begin(), m_qref.end(),
		    gen::ostream_iterator<citation_t>(os, "’, ‘"));
	  os << "’ }\n";
      }
  }

  void
  rec_qvalue::sync(more::io::syncstream& sio)
  {
      static_cast<rec_commentable_base*>(this)->sync_base(sio);
      sio | m_qmi | m_sn | m_sp | m_qa;
      io::sync_sequence(sio, m_qref);
  }

  void
  rec_xref::dump(std::ostream& os) const
  {
      os << "\t["
	 << m_dssym << "] " << m_dsid << "\n";
  }

  void
  rec_xref::sync(more::io::syncstream& ss)
  {
      ss | m_dssym | m_dsid;
  }

  void
  rec_comment::dump(std::ostream& os) const
  {
      os << "    Comment";
      if (m_rtype != record_kind_none)
	  os << " on " << record_kind_to_str(m_rtype);
      if (m_psym != particle_kind_none)
	  os << " for particle " << particle_kind_to_str(m_psym);
      os << "\n";
      if (!m_symflags.empty()) {
	  os << "\tApplies to { ";
	  for (symflags_iterator it = m_symflags.begin();
	       it != m_symflags.end(); ++it) {
	      if (it != m_symflags.begin())
		  os << ", ";
	      os << it->first;
	      if (!it->second.empty()) {
		  os << "(";
		  std::copy(it->second.begin(), it->second.end(),
			    gen::ostream_iterator<char>(os, ","));
		  os << ")";
	      }
	  }
	  os << " }.\n";
      }
      if (m_fill_p) {
	  std::string word;
	  std::istringstream iss(m_ctext);
	  iss >> word;
	  os << "\t" << word;
	  int col = word.size();
	  while (iss >> word, !iss.fail()) {
	      col += word.size() + 1;
	      if (col >= 64) {
		  os << "\n\t";
		  col = 0;
		  os << word;
	      }
	      else
		  os << ' ' << word;
	  }
	  os << "\n";
      }
      else
	  os << "\tcom = <<EOS\n" << m_ctext << "\nEOS\n";
  }
  void
  rec_comment::sync(more::io::syncstream& sio)
  {
      int rtype = m_rtype;
      int psym = m_psym;
      int fill_p = m_fill_p;
      int doc_p = m_doc_p;
      sio | rtype | psym;
      sync_pair_associative_container(sio, m_symflags);
      sio | m_ctext | fill_p | doc_p;
      m_rtype = (record_kind_t)rtype;
      m_psym = (particle_kind_t)psym;
      m_fill_p = fill_p;
      m_doc_p = doc_p;
  }

  void
  rec_parent::dump(std::ostream& os) const
  {
      os << "\tnucid = " << m_nucid << '\n'
	 << "\tE = " << m_E/SI::keV << " keV";
      print_energy_ref(os, m_E_ref);
      os << "\n"
	 << "\tJπ = " << m_J.as_string() << '\n'
	 << "\tT = " << m_T/SI::keV << (m_T_as_width? " keV" : " s")
	 << '\n'
	 << "\tQP = " << m_QP/SI::keV << " keV\n";
      static_cast<rec_commentable_base const*>(this)->dump_base(os);
  }

  void
  rec_parent::sync(more::io::syncstream& sio)
  {
      int Taw = m_T_as_width;
      int E_ref = m_E_ref;
      sio | m_nucid | m_E | E_ref | m_J | m_T | m_QP | Taw;
      m_T_as_width = Taw;
      m_E_ref = E_ref;
      static_cast<rec_commentable_base*>(this)->sync_base(sio);
  }

  void
  rec_norm::dump(std::ostream& os) const
  {
      os << "    Normalization Record\n"
	 << "\tnr = " << m_nr << '\n'
	 << "\tnt = " << m_nt << '\n'
	 << "\tbr = " << m_br << '\n'
	 << "\tnb = " << m_nb << '\n'
	 << "\tnp = " << m_np << '\n';
      static_cast<rec_commentable_base const*>(this)->dump_base(os);
  }
  void
  rec_norm::sync(more::io::syncstream& sio)
  {
      sio | m_nr | m_nt | m_br | m_nb | m_np;
      static_cast<rec_commentable_base*>(this)->sync_base(sio);
  }

  void
  rec_pnorm::dump(std::ostream& os) const
  {
      os << "    Production Normalization Record\n"
	 << "\tnr_br = " << m_nr_br << '\n'
	 << "\tnt_br = " << m_nt_br << '\n'
	 << "\tnb_br = " << m_nb_br << '\n'
	 << "\tnp = " << m_np << '\n';
      static_cast<rec_commentable_base const*>(this)->dump_base(os);
  }
  void
  rec_pnorm::sync(more::io::syncstream& sio)
  {
      sio | m_nr_br | m_nt_br | m_nb_br | m_np;
      static_cast<rec_commentable_base*>(this)->sync_base(sio);
  }

  rec_level::~rec_level()
  {
      while (!m_radiations.empty())
	  rec_radiation::do_delete(&*m_radiations.begin());
  }

  void
  rec_level::dump(std::ostream& os) const
  {
      os << "    Level Record";
      switch (m_rstatus) {
      case rstatus_uncertain:
	  os << " (uncertain)";
	  break;
      case rstatus_predicted:
	  os << " (predicted)";
	  break;
      default:
	  break;
      }
      os << '\n';
      if (m_E.is_known()) {
	  os << "\tE = " << m_E/SI::keV << " keV";
	  print_energy_ref(os, m_E_ref);
	  os << '\n';
      }
      if (m_J.is_known())
	  os << "\tJπ ∈ {" << m_J.as_string() << "}\n";
      if (m_T.is_known()) {
	  os << "\tT = ";
	  if (m_T_as_width)
	      os << m_T/SI::keV << " keV";
	  else
	      os << m_T/SI::s << " s";
	  os << '\n';
      }
      for (ang_mom_to_strength_map::const_iterator it = m_LS.begin();
	   it != m_LS.end(); ++it) {
	  if (it == m_LS.begin())
	      os << "\t";
	  else
	      os << ", ";
	  if (it->first.is_unknown())
	      os << "(L = unknown";
	  else {
	      if (it->first.is_lower_limit())
		  os << "(L ≤ ";
	      else if (it->first.is_upper_limit())
		  os << "(L ≥ ";
	      else
		  os << "(L = ";
	      if (it->first.is_uncertain())
		  os << "uncertain ";
	      else if (it->first.is_assumed())
		  os << "assumed ";
	      if (it->first.is_magnetic())
		  os << 'M';
	      if (it->first.is_electric())
		  os << 'E';
	      os << it->first.L();
	  }
	  os << ", S = " << it->second << ")";
      }
      os << "\n";
#if 0
      os << "\n\t(L, S) ∈ {";
      for (ang_mom_to_strength_map::const_iterator it = m_LS.begin();
	   it != m_LS.end(); ++it) {
	  if (it != m_LS.begin())
	      os << ", ";
	  os << "(" << it->first.L() << ", " << it->second << ")";
	  if (it->first.is_uncertain())
	      os << "?";
      }
      os << "}\n";
#endif
      if (!m_C.empty()) {
	  os << "\tcomment_refs = ‘";
	  std::copy(m_C.begin(), m_C.end(),
		    gen::ostream_iterator<char>(os, ","));
	  os << "’\n";
      }
      os << "\tMS = ";
      switch (m_MS) {
      case 0:
	  os << "none";
	  break;
      case 1:
	  os << "M";
	  break;
      default:
	  os << "M" << m_MS - 1;
	  break;
      }
      os << '\n';
      static_cast<rec_commentable_base const*>(this)->dump_base(os);
      for (gen::link<rec_radiation>::const_iterator it = m_radiations.begin();
	   it != m_radiations.end(); ++it)
	  it->dump(os);
  }
  void
  rec_level::sync(more::io::syncstream& sio)
  {
      sio | m_E;
      energy_index_t E_ref = m_E_ref;
      sio | E_ref;
      m_E_ref = E_ref;
      sio | m_J | m_T | m_LS | m_C;
      unsigned int bits = m_MS
	  | (m_rstatus << 4)
	  | (m_T_as_width << 6);
      sio | bits;
      m_MS = bits & ((1 << 4) - 1);
      m_rstatus = (bits >> 4) & 3;
      m_T_as_width = (bits >> 6) & 1;
      static_cast<rec_commentable_base*>(this)->sync_base(sio);
      // XXX
      // m_radiations
  }

  void
  rec_radiation::do_delete(rec_radiation* r)
  {
      switch (r->m_sel) {
	case sel_beta_mi:
	  MORE_DELETE(rec_beta_mi, static_cast<rec_beta_mi*>(r));
	  break;
	case sel_beta_pl:
	  MORE_DELETE(rec_beta_pl, static_cast<rec_beta_pl*>(r));
	  break;
	case sel_alpha:
	  MORE_DELETE(rec_alpha, static_cast<rec_alpha*>(r));
	  break;
	case sel_particle:
	  MORE_DELETE(rec_particle, static_cast<rec_particle*>(r));
	  break;
	case sel_gamma:
	  MORE_DELETE(rec_gamma, static_cast<rec_gamma*>(r));
	  break;
	default:
	  assert(0);
      }
  }

  void
  rec_radiation::dump(std::ostream& os) const
  {
      switch (m_sel) {
      case sel_beta_mi:
	  static_cast<rec_beta_mi const*>(this)->dump(os);
	  break;
      case sel_beta_pl:
	  static_cast<rec_beta_pl const*>(this)->dump(os);
	  break;
      case sel_alpha:
	  static_cast<rec_alpha const*>(this)->dump(os);
	  break;
      case sel_particle:
	  static_cast<rec_particle const*>(this)->dump(os);
	  break;
      case sel_gamma:
	  static_cast<rec_gamma const*>(this)->dump(os);
	  break;
      default:
	  assert(!"Not reached.");
      }
  }
  void
  rec_radiation::dump_base(std::ostream& os) const
  {
      // XXX print uncertain and expected in subclass
      if (m_E.is_known()) {
	  os << "\tE = " << m_E/SI::keV << " keV";
	  print_energy_ref(os, m_E_ref);
	  os << '\n';
      }
      // Skip IX
      // XXX Not sure about the COIN (col 77) flag.
      if (!m_C.empty()) {
	  os << "\tcomment_refs = ‘";
	  std::copy(m_C.begin(), m_C.end(),
		    gen::ostream_iterator<char>(os, ","));
	  os << "’\n";
      }
  }

  void
  rec_radiation::sync_base(more::io::syncstream& sio)
  {
      int E_ref = m_E_ref;
      unsigned int bits = m_sel | (m_rstatus << 3);
      sio | bits | m_E | E_ref | m_IX | m_C;
      m_sel = (selector)(bits & 7);
      m_E_ref = E_ref;
      m_rstatus = bits >> 3;
      static_cast<rec_commentable_base*>(this)->sync_base(sio);
  }

  void
  rec_beta_mi::dump(std::ostream& os) const
  {
      os << "    β- Decay and Electron Capture";
      print_rstatus_parent(os, status());
      os << "\n\tforbiddenness = ";
      if (m_un_unique)
	  os << "U";
      os << m_un_forbiddenness << "\n"
	 << "\tlog(ft) = " << m_log_ft << '\n';
      dump_base(os);
      static_cast<rec_commentable_base const*>(this)->dump_base(os);
  }
  void
  rec_beta_mi::sync(more::io::syncstream& ss)
  {
      // XXX
  }

  void
  rec_beta_pl::dump(std::ostream& os) const
  {
      os << "    β+ Decay";
      print_rstatus_parent(os, status());
      os << "\n\tforbiddenness = ";
      if (m_un_unique)
	  os << "U";
      os << m_un_forbiddenness
	 << "\n\tlog(ft) = " << m_log_ft
	 << "\n\tI_tot_rel = " << I_tot_rel()
	 << "\n\tI_EC_rel = " << I_EC_rel()
	 << "\n\tI_β+_rel = " << I_beta_pl_rel() << '\n';
      dump_base(os);
      static_cast<rec_commentable_base const*>(this)->dump_base(os);
  }
  void
  rec_beta_pl::sync(more::io::syncstream& ss)
  {
      // XXX
  }

  void
  rec_alpha::dump(std::ostream& os) const
  {
      os << "    α Decay";
      print_rstatus_parent(os, status());
      dump_base(os);
      os << "\thindrance_factor = " << m_HF;
      static_cast<rec_commentable_base const*>(this)->dump_base(os);
  }
  void
  rec_alpha::sync(more::io::syncstream& ss)
  {
      // XXX
  }

  void
  rec_particle::dump(std::ostream& os) const
  {
      if (m_delayed_p)
	  os << "    Delayed ";
      else
	  os << "    ";
      if (m_particle != particle_kind_none)
	  os << particle_kind_to_str(m_particle);
      os << " Particle Decay";
      print_rstatus_parent(os, status());
      os << "\n";
      dump_base(os);
      if (m_EI_given) {
	  os << "\tE_imedi = " << m_EI/SI::keV;
	  if (m_EI_given == 1)
	      os << " (uncertain)";
	  os << "\n";
      }
      os << "\tT = " << m_T/SI::keV << " keV";
      for (ang_mom_xfer_set::const_iterator it = m_L.begin();
	   it != m_L.end(); ++it) {
	  if (it == m_L.begin())
	      os << "\n\t";
	  else
	      os << ", ";
	  if (it->is_lower_limit())
	      os << "L ≤ ";
	  else if (it->is_upper_limit())
	      os << "L ≥ ";
	  else
	      os << "L = ";
	  if (it->is_magnetic())
	      os << 'M';
	  if (it->is_electric())
	      os << 'E';
	  if (it->is_uncertain())
	      os << it->L() << " (uncertain)";
	  else if (it->is_assumed())
	      os << it->L() << " (assumed)";
	  else
	      os << it->L();
      }
      os << "\n\t";
      switch (m_coin) {
      case fuzzy_false:
	  os << "No coincidence\n";
	  break;
      case fuzzy_probable:
	  os << "Probable coincidence\n";
	  break;
      case fuzzy_true:
	  os << "With coincidence\n";
	  break;
      }
      static_cast<rec_commentable_base const*>(this)->dump_base(os);
  }
  void
  rec_particle::sync(more::io::syncstream& ss)
  {
      // XXX
  }

  void
  rec_gamma::dump(std::ostream& os) const
  {
      os << "    γ Decay\n"
	 << "\tmultipolarity = " << m_M.as_string() << '\n';
      if (m_MR.is_known()) {
	  os << "\tmixing_ratio = ";
	  os << m_MR << '\n';
      }
      if (m_CC.is_known())
	  os << "\tconversion_coefficient = " << m_CC << '\n';
      if (m_TI.is_known())
	  os << "\tI_rel_tot = " << m_TI << '\n';
      os << "\t";
      switch (m_coin) {
      case fuzzy_false:
	  os << "No coincidence\n";
	  break;
      case fuzzy_probable:
	  os << "Probable coincidence\n";
	  break;
      case fuzzy_true:
	  os << "With coincidence\n";
	  break;
      }
      dump_base(os);
      static_cast<rec_commentable_base const*>(this)->dump_base(os);
  }
  void
  rec_gamma::sync(more::io::syncstream& ss)
  {
      // XXX
  }

  void
  rec_ref::dump(std::ostream& os) const
  {
      os << "    Referece: " << m_keynum << " - " << m_reference
	 << '\n';
  }
  void
  rec_ref::sync(more::io::syncstream& ss)
  {
      // XXX
  }

  decay_info::~decay_info()
  {
      MORE_DELETE(rec_parent, m_parent);
      MORE_DELETE(rec_norm, m_norm);
      MORE_DELETE(rec_pnorm, m_pnorm);
  }

  dataset::~dataset()
  {
      while (!m_history.empty())
	  MORE_DELETE(rec_history, &*m_history.begin());
      while (!m_xref.empty())
	  MORE_DELETE(rec_xref, &*m_xref.begin());
      while (!m_comments.empty())
	  MORE_DELETE(rec_comment, &*m_comments.begin());
      MORE_DELETE(rec_qvalue, m_qvalue);
      while (!m_pnp.empty())
	  MORE_DELETE(decay_info, &*m_pnp.begin());
      while (!m_unpl_rads.empty())
	  rec_radiation::do_delete(&*m_unpl_rads.begin());
      while (!m_levels.empty())
	  MORE_DELETE(rec_level, &*m_levels.begin());
  }

  void
  dataset::dump(std::ostream& os) const
  {
      m_ident.dump(os);

      dump_range(m_history.begin(), m_history.end(), os);
      if (!m_xref.empty()) {
	  os << "    Cross References (ID, dataset):\n";
	  dump_range(m_xref.begin(), m_xref.end(), os);
      }
      dump_range(m_comments.begin(), m_comments.end(), os);

      if (m_qvalue)
	  m_qvalue->dump(os);
      for (gen::link<decay_info>::const_iterator
	       it = m_pnp.begin(); it != m_pnp.end(); ++it) {
	  if (it->parent()) {
	      os << "\n    Parent\n";
	      it->parent()->dump(os);
	  }
	  if (it->norm()) {
	      os << "\n    Normalization\n";
	      it->norm()->dump(os);
	  }
	  if (it->pnorm()) {
	      os << "\n    Production normalization\n";
	      it->pnorm()->dump(os);
	  }
      }

      os << "    Unplaced Radiations\n";
      dump_range(m_unpl_rads.begin(),
		  m_unpl_rads.end(), os);
      os << "    Levels\n";
      dump_range(m_levels.begin(), m_levels.end(), os);
  }


  void
  dataset::sync(more::io::syncstream& ss)
  {
      ss | m_ident;
      // XXX
  }

  decay_info*
  dataset::find_pnp(unsigned int index)
  {
      gen::link<decay_info>::iterator
	  it = m_pnp.begin();
      while (it != m_pnp.end()) {
	  if (it->m_index == index)
	      break;
	  ++it;
      }
      if (it == m_pnp.end())
	  return 0;
      else
	  return &*it;
  }

  decay_info*
  dataset::find_or_insert_pnp(unsigned int index)
  {
      decay_info* pnp = find_pnp(index);
      if (pnp == 0)
	  pnp = MORE_NEW(decay_info);
      pnp->m_index = index;
      m_pnp.join_back(pnp);
      return pnp;
  }



  //
  // nucleus
  //
  nucleus::~nucleus()
  {
      // TODO.  The un-caching strategy may be improved.
      if (m_datasets && --m_datasets->m_ic == 0) {
	  --s_n_in_use;
	  ++s_n_unused;
	  if (s_n_unused > 20 && s_n_unused > s_n_in_use)
	      clear_cache();
      }
  }

  void
  nucleus::clear_cache()
  {
      if (file_debug)
	  std::cerr << "debug: clear_cache()" << std::endl;
      for (int i = 0; i < s_N_max*s_Z_max; ++i)
	  if (dataset_container* dsc = s_cache[i].get())
	      if (!dsc->m_ic) {
		  if (file_debug && !dsc->empty())
			  std::cerr << "debug: Clearing "
				    << dsc->front().ident()->nucl() << '\n';
		  while (!dsc->empty())
		      MORE_DELETE(dataset, &*dsc->begin());
		  s_cache[i].reset();
	      }
  }

  void
  nucleus::init()
  {
      if (n_neut() >= s_N_max || n_prot() >= s_Z_max)
	  throw std::logic_error("more::phys::ens::nucleus: "
				 "Nucleus out of range.");
      m_datasets = s_cache[n_neut()*s_Z_max + n_prot()].get();
      if (!m_datasets) {
	  m_datasets = MORE_NEW(dataset_container);
	  s_cache[n_neut()*s_Z_max + n_prot()].reset(m_datasets);
	  std::string name = ensdb::default_db()->fetch_file(*this);
	  if (more::io::cmdline::verbose()) {
	      if (name.empty())
		  std::clog << "info: No data for " << *this << std::endl;
	      else
		  std::clog << "info: Found data for " << *this << std::endl;
	  }
	  if (name.empty())
	      return;
	  more::io::ifstream is(name.c_str());
	  if (!is.good())
	      throw std::runtime_error("more::phys::ens: "
				       "Could not open "+name);
	  parse(is, name.c_str(), *this);
      }
      if (m_datasets->m_ic++ == 0)
	  ++s_n_in_use;
  }

  void
  nucleus::dump(std::ostream& os) const
  {
      dump_range(dataset_begin(), dataset_end(), os);
  }

  void
  nucleus::set_fetching_enabled(bool opt)
  {
      ensdb::default_db()->set_fetching_enabled(opt);
  }

  more::diag::auto_ptr<nucleus::dataset_container>
			nucleus::s_cache[s_N_max*s_Z_max];
  int			nucleus::s_n_in_use;
  int			nucleus::s_n_unused;
  std::ostream*		nucleus::s_clog = &std::clog;


  rec_qvalue const*
  nucleus::qvalue() const
  {
      for (dataset_iterator it = dataset_begin(); it != dataset_end(); ++it)
	  if (it->qvalue())
	      return it->qvalue();
      return 0;
  }

}}} // more::phys::ens

// Local Variables:
// coding: utf-8
// End:
