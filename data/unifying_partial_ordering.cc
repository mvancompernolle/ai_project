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


#include <more/gen/unifying_partial_ordering.h>
#include <limits>
#include <iterator>
#include <assert.h>

namespace more {
namespace gen {
///\if bits
namespace bits_upo {
///\endif

  using more::diag::fatal;

  /** Link \arg x to \c m_min and \c m_max.
   * \pre x is a newly created node.
   * \post x in disowned.
   */
  void
  unifying_partial_ordering_base::insert_impl(node_base* x)
  {
      assert(x != m_max && x != m_min);
      assert(x != 0);
      assert(m_min != 0 && m_max != 0);
      x->m_sups.insert(m_max);
      m_max->m_infs.insert(x);
      x->m_infs.insert(m_min);
      m_min->m_sups.insert(x);
  }

  /** Helper for preceq_impl. */
  bool
  unifying_partial_ordering_base::preceq_impl_x(node_base* x, node_base* y)
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
  unifying_partial_ordering_base::preceq_impl(node_base* x, node_base* y)
  {
      assert(x->context == y->context);
      x->context->clear_cache();
      return preceq_impl_x(x, y);
  }

#if 0
  bool
  unifying_partial_ordering_base::constrain_preceq_impl(node_base* x, node_base* y) {
      assert(x && y);
      assert(m_max && m_min);
      if (preceq_impl(x, y))
	  return true;
      else if (preceq_impl(y, x))
	  return constrain_eq_impl(x, y);
      else {
	  std::set<node_base*> sups;
	  copy_range_impl(y, m_max, std::inserter(sups, sups.begin()));
	  sups.insert(m_max);

	  std::list<node_base*> infs;
	  copy_range_impl(m_min, x, std::back_inserter(infs));
	  infs.push_back(x);

	  for (std::list<node_base*>::iterator it_sub = infs.begin();
	       it_sub != infs.end(); ++it_sub) {
	      for (node_base::sup_iterator it0 = (*it_sub)->m_sups.begin();
		   it0 != (*it_sub)->m_sups.end();) {
		  std::set<node_base*>::iterator it_super = sups.find(*it0);
		  if (it_super != sups.end()) {
		      node_base::sup_iterator it1 = it0;
		      ++it0;
		      (*it1)->m_infs.erase(*it_sub);
		      (*it_sub)->m_sups.erase(it1);
		  }
		  else
		      ++it0;
	      }
	  }
	  x->m_sups.insert(y);
	  y->m_infs.insert(x);
	  return true;
      }
  }
#endif
  /** links \c x as a sub of \c y and \c y as a super of \c x.
   *
   * \pre neigher \f$x\sqsubseteq y\f$ nor \f$y\sqsubseteq x\f$ must
   * be true, or loops may be formed.
   */
  void unifying_partial_ordering_base::
  constrain_preceq_impl(node_base* x, node_base* y)
  {
      // remove all links between subs of x and m_sups of y.
      std::set<node_base*> sups;
      copy_range_impl(y, m_max, std::inserter(sups, sups.begin()));
      sups.insert(m_max);

      std::list<node_base*> infs;
      copy_range_impl(m_min, x, std::back_inserter(infs));
      infs.push_back(x);

      for (std::list<node_base*>::iterator it_sub = infs.begin();
	   it_sub != infs.end(); ++it_sub) {
	  for (node_base::sup_iterator it0 = (*it_sub)->m_sups.begin();
	       it0 != (*it_sub)->m_sups.end();) {
	      std::set<node_base*>::iterator it_super = sups.find(*it0);
	      if (it_super != sups.end()) {
		  node_base::sup_iterator it1 = it0;
		  ++it0;
		  (*it1)->m_infs.erase(*it_sub);
		  (*it_sub)->m_sups.erase(it1);
	      }
	      else
		  ++it0;
	  }
      }

      // the create a link between x and y
      x->m_sups.insert(y);
      y->m_infs.insert(x);
  }

  /** check the integrity, starting from m_max and working
   * downwards. Helper for check_integrity. */
  void unifying_partial_ordering_base::
  check_integrity_from_bottom(node_base* x)
  {
      if (x == m_max) return;
      if (x->is_cached()) {
	  if (x->cache().as_bool == true)
	      fatal("unifying_partial_ordering_base::check_integrity: loop detected.");
      }
      x->set_cached();
      x->cache().as_bool = true;
      for (node_base::sup_iterator it = x->m_sups.begin();
	   it != x->m_sups.end(); ++it) {
	  node_base* y = *it;
	  if (y->m_infs.find(x) == y->m_infs.end()) {
	      std::cerr << "** unifying_partial_ordering_base::check_integrity: "
			<< "node @" << y << " lacks a sub link." << std::endl;
	      dump_structure();
	      abort(); //throw 0;
	  }
	  check_integrity_from_bottom(y);
      }
      x->cache().as_bool = false;
  }

  /** check the integrity, starting from m_min and working
   * upwards. Helper for check_integrity. */
  void unifying_partial_ordering_base::check_integrity_from_top(node_base* x) {
      if (x == m_min) return;
      if (x->is_cached()) {
	  if (x->cache().as_bool == true)
	      fatal("unifying_partial_ordering_base::check_integrity: loop detected.");
      }
      x->set_cached();
      x->cache().as_bool = true;
      for (node_base::inf_iterator it = x->m_infs.begin();
	   it != x->m_infs.end(); ++it) {
	  node_base* y = *it;
	  if (y->m_sups.find(x) == y->m_sups.end()) {
	      std::cerr << "** unifying_partial_ordering_base::check_integrity: "
			<< "node @" << y << "lacks a super link." << std::endl;
	      dump_structure();
	      abort();
	  }
	  check_integrity_from_top(y);
      }
      x->cache().as_bool = false;
  }

  /** check the integrity of the links. Used for debugging. */
  void unifying_partial_ordering_base::check_integrity() {
      clear_cache();
      check_integrity_from_bottom(m_min);
      check_integrity_from_top(m_max);
  }

#if 0 // more efficient if we don't need to preserve the container when
      // unification fails.
  unifying_partial_ordering_base::node_base*
  unifying_partial_ordering_base::
  constrain_eq_impl_x(node_base* x, node_base* y)
  {
      // invariant:  x as a pointer is unchanged
      // post:       if x [<] y, x is the collapse of [x, y], and
      //                         y is invalidated
      // returns:    x if x [<=] y, 0 otherwise

      if (x == y)
	  return x;

      if (x->is_cached()) {
	  assert(x->cache().as_int == 0);
	  return x->cache().as_int? x : 0;
      }
      else {
	  x->set_cached();
#ifndef NDEBUG
	  x->cache().as_int = 2;
#endif
	  bool connected = false;
	  for (node_base::sup_iterator it_super = x->m_sups.begin();
	       it_super != x->m_sups.end();) {
	      if (constrain_eq_impl_x(*it_super, y)) {
		  y = *it_super;
		  connected = true;
		  node_base::sup_iterator it_erase = it_super;
		  ++it_super;
		  (*it_erase)->m_infs.erase(x);
		  x->m_sups.erase(it_erase);
	      }
	      else
		  ++it_super;
	  }
	  if (connected) {
	      for (node_base::sup_iterator it_super = y->m_sups.begin();
		   it_super != y->m_sups.end(); ++it_super) {
		  (*it_super)->m_infs.erase(y);
		  (*it_super)->m_infs.insert(x);
		  x->m_sups.insert(*it_super);
	      }
	      for (node_base::inf_iterator it_sub = y->m_infs.begin();
		   it_sub != y->m_infs.end(); ++it_sub) {
		  (*it_sub)->m_sups.erase(y);
		  (*it_sub)->m_sups.insert(x);
		  x->m_infs.insert(*it_sub);
	      }

	      for (user_iterator it_user = y->users.begin();
		   it_user != y->users.end(); ++it_user)
		  it_user->ptr = x;
	      x->users.join(y->users);
	      unify_nodes_to_first(x, y); // unlinks y from x->users

	      x->cache().as_int = 1;
	      return x;
	  }
	  else {
	      x->cache().as_int = 0;
	      return 0;
	  }
      }
  }
#endif

  /** replace the nodes in S with \c new_node.  This moves all
   * relations which are external to S to \c new_node, and then
   * update all element_base objects referring to nodes in \c S to
   * refer to \c new_node.
   *
   * \pre S forms a range. \c new_node is a newly allocated
   * unconnected node.
   *
   * \post All nodes in \c S must be explicitely deleted to complete
   * the operation.  \c new_node is disowned.
   */
  void
  unifying_partial_ordering_base::
  constrain_eq_impl(std::set<node_base*> const& S, node_base* new_node)
  {
      for (std::set<node_base*>::iterator it_S = S.begin();
	   it_S != S.end(); ++it_S) {
	  for (node_base::inf_iterator it_sub = (**it_S).m_infs.begin();
	       it_sub != (**it_S).m_infs.end(); ++it_sub) {
	      if (S.find(*it_sub) == S.end()) {
		  (**it_sub).m_sups.insert(new_node);
		  new_node->m_infs.insert(*it_sub);
	      }
	      (**it_sub).m_sups.erase(*it_S);
	  }
	  for (node_base::sup_iterator it_super = (**it_S).m_sups.begin();
	       it_super != (**it_S).m_sups.end(); ++it_super) {
	      if (S.find(*it_super) == S.end()) {
		  (**it_super).m_infs.insert(new_node);
		  new_node->m_sups.insert(*it_super);
	      }
	      (**it_super).m_infs.erase(*it_S);
	  }
	  for (user_iterator it_user = (**it_S).users.begin();
	       it_user != (**it_S).users.end(); ++it_user)
	      it_user->ptr = new_node;
	  new_node->users.join((**it_S).users);
	  (**it_S).users.unlink();
      }
  }

  /** Incrememt the algo_serial, and if it reaches its maximum,
   * reset it and algo_serial of all nodes.
   *
   * \post \c this->algo_serial is different from \c x->algo_serial
   * of all nodes \c x.
   */
  void
  unifying_partial_ordering_base::clear_cache() const
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
  unifying_partial_ordering_base::dump_structure()
  {
      typedef std::list<node_base*> container;
      container l;
      copy_range_impl(m_min, m_max, back_inserter(l));
      l.push_back(m_max);
      std::cerr << "Partial ordering structure dump:" << std::endl;
      for (container::iterator it = l.begin(); it != l.end(); ++it) {
	  std::cerr << "  " << *it;
	  std::list<node_base*> l0;
	  copy_supremums_impl(*it, back_inserter(l0));
	  std::cerr << "\n    LUBs: ";
	  std::copy(l0.begin(), l0.end(),
		    std::ostream_iterator<void*>(std::cerr, " "));
	  l0.clear();
	  copy_infimums_impl(*it, back_inserter(l0));
	  std::cerr << "\n    GLBs: ";
	  std::copy(l0.begin(), l0.end(),
		    std::ostream_iterator<void*>(std::cerr, " "));
	  std::cerr << std::endl;
      }
      std::cerr << std::endl;
  }
///\if bits_upo
}
///\endif
}} // namespace more::gen::unifying_partial_ordering_ns

// #ifdef MORE_CONF_CXX_HAVE_EXPORT
// #  include "unifying_partial_ordering.tcc"
// #endif
