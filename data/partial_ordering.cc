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


#include <more/gen/partial_ordering.h>
#include <limits>
#include <iterator>
#include <assert.h>
#include <cstdlib>              // for abort()

namespace more {
namespace gen {
///\if bits
namespace bits_po {
///\endif

  using more::diag::fatal;

  /** Link \arg x to \c m_min and \c m_max.
   * \pre x is a newly created node.
   * \post x in disowned.
   */
  void
  partial_ordering_base::insert_impl(node_base* x)
  {
      assert(x != m_max && x != m_min);
      assert(x != 0);
      assert(m_min && m_max);
      if (*m_min->m_sups.begin() == m_max) {
	  m_min->m_sups.erase(m_max);
	  m_max->m_infs.erase(m_min);
      }
      x->m_sups.insert(m_max);
      m_max->m_infs.insert(x);
      x->m_infs.insert(m_min);
      m_min->m_sups.insert(x);
  }

  /** Unlinks the node, preserving all relations not involving the node.
   * \post x is disowned
   */
  void
  partial_ordering_base::unlink_impl(node_base* x)
  {
      assert(x != m_min && x != m_max);
      for (node_base::inf_iterator it_inf = x->m_infs.begin();
	   it_inf != x->m_infs.end(); ++it_inf)
	  (*it_inf)->m_sups.erase(x);
      for (node_base::sup_iterator it_sup = x->m_sups.begin();
	   it_sup != x->m_sups.end(); ++it_sup)
	  (*it_sup)->m_infs.erase(x);
      for (node_base::inf_iterator it_inf = x->m_infs.begin();
	   it_inf != x->m_infs.end(); ++it_inf) {
	  node_base* x_inf = *it_inf;
	  for (node_base::sup_iterator it_sup = x->m_sups.begin();
	       it_sup != x->m_sups.end(); ++it_sup) {
	      node_base* x_sup = *it_sup;
	      if (!preceq_impl(x_inf, x_sup)) {
		  x_inf->m_sups.insert(x_sup);
		  x_sup->m_infs.insert(x_inf);
	      }
	  }
      }
  }

  /** Helper for preceq_impl. */
  bool
  partial_ordering_base::preceq_impl_x(node_base* x, node_base* y)
  {
      if (x == y)
	  return true;
      else if (x->is_cached())
	  return false;
      for (node_base::sup_iterator it = x->m_sups.begin();
	   it != x->m_sups.end(); ++it)
	  if (preceq_impl_x(*it, y))
	      return true;
      x->set_cached();
      return false;
  }

  /** Determine if \f$x\sqsubseteq y\f$. */
  bool
  partial_ordering_base::preceq_impl(node_base* x, node_base* y)
  {
      assert(x->context == y->context);
      x->context->clear_cache();
      return preceq_impl_x(x, y);
  }

  /** links \c x as a sub of \c y and \c y as a super of \c x.
   *
   * \pre neigher \f$x\sqsubseteq y\f$ nor \f$y\sqsubseteq x\f$ must
   * be true, or loops may be formed.
   */
  void
  partial_ordering_base::constrain_prec_impl(node_base* x, node_base* y)
  {
      assert(x != m_max);
      assert(y != m_min);

      std::list<node_base*> S_below;
      copy_range_impl(m_min, x, std::back_inserter(S_below));
      S_below.push_back(x);

      std::set<node_base*> S_above;
      copy_range_impl(y, m_max, std::inserter(S_above, S_above.begin()));
      S_above.insert(m_max);

      // remove all links from x and below to y and above.
      for (std::list<node_base*>::iterator it_below = S_below.begin();
	   it_below != S_below.end(); ++it_below) {
	  for (node_base::sup_iterator it_upward = (*it_below)->m_sups.begin();
	       it_upward != (*it_below)->m_sups.end();) {
	      std::set<node_base*>::iterator
		  it_above = S_above.find(*it_upward);
	      if (it_above != S_above.end()) {
		  node_base::sup_iterator it_tmp = it_upward;
		  ++it_upward;
		  (*it_tmp)->m_infs.erase(*it_below);
		  (*it_below)->m_sups.erase(it_tmp);
	      }
	      else
		  ++it_upward;
	  }
      }

      // then create the link from x to y
      x->m_sups.insert(y);
      y->m_infs.insert(x);
  }

  /** check the integrity, starting from m_max and working
   *  downwards. Helper for check_integrity. */
  void
  partial_ordering_base::check_integrity_from_bottom(node_base* x)
  {
      if (x == m_max) return;
      if (x->is_cached()) {
	  if (x->cache().as_bool == true)
	      fatal("partial_ordering_base::check_integrity: loop detected.");
      }
      x->set_cached();
      x->cache().as_bool = true;
      for (node_base::sup_iterator it = x->m_sups.begin();
	   it != x->m_sups.end(); ++it) {
	  node_base* y = *it;
	  if (y->m_infs.find(x) == y->m_infs.end()) {
	      std::cerr << "** partial_ordering_base::check_integrity: "
			<< "node @" << y << " lacks a sub link." << std::endl;
	      dump_structure();
	      abort(); //throw 0;
	  }
	  check_integrity_from_bottom(y);
      }
      x->cache().as_bool = false;
  }

  /** check the integrity, starting from m_min and working
   *  upwards. Helper for check_integrity. */
  void
  partial_ordering_base::check_integrity_from_top(node_base* x)
  {
      if (x == m_min) return;
      if (x->is_cached()) {
	  if (x->cache().as_bool == true)
	      fatal("partial_ordering_base::check_integrity: loop detected.");
      }
      x->set_cached();
      x->cache().as_bool = true;
      for (node_base::inf_iterator it = x->m_infs.begin();
	   it != x->m_infs.end(); ++it) {
	  node_base* y = *it;
	  if (y->m_sups.find(x) == y->m_sups.end()) {
	      std::cerr << "** partial_ordering_base::check_integrity: "
			<< "node @" << y << "lacks a super link." << std::endl;
	      dump_structure();
	      abort();
	  }
	  check_integrity_from_top(y);
      }
      x->cache().as_bool = false;
  }

  /** check the integrity of the links. Used for debugging. */
  void
  partial_ordering_base::check_integrity()
  {
      clear_cache();
      check_integrity_from_bottom(m_min);
      check_integrity_from_top(m_max);
  }

  /** Incrememt the algo_serial, and if it reaches its maximum,
   * reset it and algo_serial of all nodes.
   *
   * \post \c this->algo_serial is different from \c x->algo_serial
   * of all nodes \c x.
   */
  void
  partial_ordering_base::clear_cache() const
  {
      if (algo_serial == std::numeric_limits<unsigned int>::max()) {
	  // XXX
	  std::cerr << "resetting algo_serial" << std::endl;
	  std::set<node_base*> es;
	  copy_range_impl(m_min, m_max, inserter(es, es.begin()));
	  for (std::set<node_base*>::iterator it = es.begin();
	       it != es.end(); ++it)
	      (**it).algo_serial = 0;
	  algo_serial = 1;
      }
      else
	  ++algo_serial;
  }

  /** Dump of all nodes and links, used for debugging. */
  void
  partial_ordering_base::dump_structure()
  {
      typedef std::list<node_base*> container;
      container l;
      copy_range_impl(m_min, m_max, back_inserter(l));
      l.push_back(m_max);
      std::cerr << "Partial ordering structure dump:" << std::endl;
      for (container::iterator it = l.begin(); it != l.end(); ++it) {
	  std::cerr << "  " << *it;
	  if (*it == m_min)
	      std::cerr << " (minimum)";
	  else if (*it == m_max)
	      std::cerr << " (maximum)";
	  std::list<node_base*> l0;
	  copy_supremums_impl(*it, back_inserter(l0));
	  std::cerr << "\n    supremums: ";
	  std::copy(l0.begin(), l0.end(),
		    std::ostream_iterator<void*>(std::cerr, " "));
	  l0.clear();
	  copy_infimums_impl(*it, back_inserter(l0));
	  std::cerr << "\n    infimums:  ";
	  std::copy(l0.begin(), l0.end(),
		    std::ostream_iterator<void*>(std::cerr, " "));
	  std::cerr << std::endl;
      }
      std::cerr << std::endl;
  }
///\if bits
}
///\endif
}} // namespace more::partial_ordering_ns

// #ifdef MORE_CONF_CXX_HAVE_EXPORT
// #  include "partial_ordering.tcc"
// #endif
