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


#include <more/lang/cx_locator.h>
#include <more/lang/package.h>
#include <more/gen/iterator.h>
#include <bdwgc/gc_cpp.h>


namespace more {
namespace lang {

  cx_locator::cx_locator(cx_locator* cxt_locr,
			 ct_type const* cxt_type,
			 ct_type const* res_type,
			 size_type offset_count)
      : cx_expr(make_proto(cxt_locr? cxt_locr->arg0_type() : cxt_type,
			   res_type, (ct_type*)0)),
	m_cxt_locr(cxt_locr),
	m_cxt_type(cxt_type),
	m_offset_count(offset_count),
	m_offset_arr(new(UseGC) size_type[offset_count]) {}

  cx_locator::cx_locator(ct_type const* arg0_type,
			 ct_type const* res_type)
      : cx_expr(make_proto(arg0_type, res_type, (void*)0)),
        m_cxt_locr(0),
	m_cxt_type(0),
	m_offset_count(0),
	m_offset_arr(0) {}

  void
  cx_locator::debug_dump(std::ostream& os)
  {
      os << '(';
      std::copy(offset_begin(), offset_end(),
		more::gen::ostream_iterator<unsigned int>(os, ", "));
      os << ')';
  }

  void*
  cx_locator::apply(void* ptr)
  {
      if (m_offset_count) {
	  ptr = static_cast<char*>(ptr) + m_offset_arr[0]; 
	  for (size_type i = 1; i < m_offset_count; ++i) {
	      ptr = *static_cast<void**>(ptr);
	      ptr = static_cast<char*>(ptr) + m_offset_arr[i];
	  }
      }
      return ptr;
  }

  void cx_locator::append_ctors_to(cx_block*) {}
  void cx_locator::append_dtors_to(cx_block*) {}


  // cx_member_locator

  cx_member_locator::
  cx_member_locator(ct_struct const* st,
		    ct_struct::member_const_iterator it_memb)
      : cx_locator(0, st, it_memb->type_of(), 1),
	m_it_memb(it_memb)
  {
      m_offset_arr[0] = it_memb->offset();
  }

  cx_member_locator::
  cx_member_locator(cx_locator* locr,
		     ct_struct::member_const_iterator it_memb)
      : cx_locator(locr, locr->result_type(), it_memb->type_of(),
		   locr->offset_count()),
	m_it_memb(it_memb)
  {
#ifndef MORE_LANG_NDEBUG
      if (dynamic_cast<ct_struct const*>(m_cxt_type) == 0)
	  throw std::logic_error("more::lang::cx_member_locator::"
				 "cx_member_locator: "
				 "Taking member of non-structure.");
#endif
      std::copy(locr->offset_begin(), locr->offset_end(), m_offset_arr);
      m_offset_arr[m_offset_count - 1] += it_memb->offset();
  }

  cx_member_locator::
  cx_member_locator(cx_locator* loc0, cx_member_locator const* loc1)
      : cx_locator(loc0->arg0_type(), loc1->result_type())
  {
      if (loc1->context_locator()) {
	  if (cx_deref_locator const* loc1cxt =
	      dynamic_cast<cx_deref_locator const*>
	      (loc1->context_locator()))
	      m_cxt_locr = new(UseGC) cx_deref_locator(loc0, loc1cxt);
	  else if (cx_member_locator const* loc1cxt =
		   dynamic_cast<cx_member_locator const*>
		   (loc1->context_locator()))
	      m_cxt_locr = new(UseGC) cx_member_locator(loc0, loc1cxt);
	  else
	      throw std::logic_error("more::lang::cx_member_locator::"
				     "cx_member_locator: "
				     "Internal error. Invalid context.");
	  m_cxt_type = loc1->m_cxt_type;
      }
      else {
	  m_cxt_locr = loc0;
	  m_cxt_type = loc0->result_type();
      }
      m_offset_count = m_cxt_locr->offset_count();
      m_offset_arr = new(UseGC) size_type[offset_count()];
      std::copy(m_cxt_locr->offset_begin(), m_cxt_locr->offset_end(),
		m_offset_arr);
      m_it_memb = loc1->m_it_memb;
      m_offset_arr[m_offset_count - 1] += m_it_memb->offset();
  }

  ct_type const*
  cx_member_locator::result_type() const
  {
      return m_it_memb->type_of();
  }

  void
  cx_member_locator::print_operation_pre(std::ostream& os) const
  {
      if (context_type()->is_optimized()) {
	  if (m_offset_count) {
	      os << "(*(";
	      result_type()->print_declaration(os, 0, "*");
	      os << ')';
	      size_type i = m_offset_count;
	      while (--i > 0)
		  os << "*((char*)";
	      os << "((char*)&(";
	  }
      }
      else {
	  if (context_locator())
	      context_locator()->print_operation_pre(os);
      }
  }

  void
  cx_member_locator::print_operation_post(std::ostream& os) const
  {
      if (context_type()->is_optimized()) {
	  if (m_offset_count) {
	      os << ")+" << m_offset_arr[0] << ')';
	      for (size_type i = 1; i < m_offset_count; ++i)
		  os << '+' << m_offset_arr[i] << ')';
	      os << ')';
	  }
      }
      else {
	  if (context_locator())
	      context_locator()->print_operation_post(os);
	  os << "." << m_it_memb->c_name();
      }
  }


  // cx_deref_locator

  cx_deref_locator::
  cx_deref_locator(ct_pointer const* p)
      : cx_locator(0, p, p->deref_type(), 1)
  {
      m_offset_arr[0] = 0;
  }

  cx_deref_locator::
  cx_deref_locator(cx_locator* cxt_locr)
      : cx_locator(cxt_locr,
		   cxt_locr->result_type(),
#ifndef MORE_LANG_NDEBUG
		   (dynamic_cast<ct_pointer const*>(cxt_locr->result_type())?
		    static_cast<ct_pointer const*>(cxt_locr->result_type())
		    ->deref_type() : 0),
#else
		   (static_cast<ct_pointer const*>(cxt_locr->result_type())
		    ->deref_type()),
#endif
		   cxt_locr->m_offset_count + 1)
  {
#ifndef MORE_LANG_NDEBUG
      if (dynamic_cast<ct_pointer const*>(m_cxt_type) == 0)
	  throw std::logic_error(
	      "more::lang::cx_deref_locator::cx_deref_locator: "
	      "Dereference of non-pointer.");
#endif
      std::copy(cxt_locr->offset_begin(), cxt_locr->offset_end(), m_offset_arr);
      m_offset_arr[m_offset_count - 1] = 0;
  }

  cx_deref_locator::
  cx_deref_locator(cx_locator* loc0, cx_deref_locator const* loc1)
      : cx_locator(loc0->arg0_type(), loc1->result_type())
  {
      if (loc1->context_locator()) {
	  if (cx_deref_locator const* loc1cxt =
	      dynamic_cast<cx_deref_locator const*>
	      (loc1->context_locator()))
	      m_cxt_locr = new(UseGC) cx_deref_locator(loc0, loc1cxt);
	  else if (cx_member_locator const* loc1cxt =
		   dynamic_cast<cx_member_locator const*>
		   (loc1->context_locator()))
	      m_cxt_locr = new(UseGC) cx_member_locator(loc0, loc1cxt);
	  else
	      throw std::logic_error("more::lang::cx_deref_locator::"
				     "cx_deref_locator: "
				     "Internal error. Invalid context.");
	  m_cxt_type = loc1->m_cxt_type;
      }
      else {
	  m_cxt_locr = loc0;
	  m_cxt_type = loc0->result_type();
      }
      m_offset_count = m_cxt_locr->offset_count() + 1;
      m_offset_arr = new(UseGC) size_type[m_offset_count];
      std::copy(m_cxt_locr->offset_begin(), m_cxt_locr->offset_end(),
		m_offset_arr);
      m_offset_arr[m_offset_count - 1] = 0;
  }

  ct_type const*
  cx_deref_locator::result_type() const
  {
      return context_type()->deref_type();
  }

  void
  cx_deref_locator::print_operation_pre(std::ostream& os) const
  {
      os << "(*";
      if (context_locator())
	  context_locator()->print_operation_pre(os);
  }

  void
  cx_deref_locator::print_operation_post(std::ostream& os) const
  {
      if (context_locator())
	  context_locator()->print_operation_post(os);
      os << ')';
  }


  //--------------------------------------------------------------------
  //			 cx_locator_resolver
  //--------------------------------------------------------------------

  ct_type const*
  cx_locator_resolver::resolve_type_of() const
  {
      return m_locr->result_type();
  }

  void*
  cx_locator_resolver::resolve()
  {
      return m_locr->apply(m_locn.get());
  }

  bool
  cx_locator_resolver::equal_to(resolver* rhs)
  {
      if (cx_locator_resolver* h = dynamic_cast<cx_locator_resolver*>(rhs))
	  return equal(&m_locn, &h->m_locn)
	      && equal(m_locr, h->m_locr);
      else
	  return false;
  }

}}
