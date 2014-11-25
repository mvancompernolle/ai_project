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


#include <more/lang/ct_struct.h>
#include <more/lang/stdint.h>
#include <more/gen/string.h>
#include <more/gen/utility.h>

namespace more {
namespace lang {


  //--------------------------------------------------------------------
  //			    ct_incomplete
  //--------------------------------------------------------------------

  ct_incomplete*
  ct_incomplete::clone() const
  {
      return new(UseGC) ct_incomplete(*this);
  }

#ifdef MORE_LANG_PROVIDE_FFI
  ffi_type*
  ct_incomplete::ffi_type_of() const
  {
      return &ffi_type_void;
  }
#endif

  void
  ct_incomplete::construct_copy(void*, void*) const
  {
      throw std::runtime_error("more::lang::ct_incomplete::construct_copy: "
			       "Can not construct a copy of an object with "
			       "an incomplete type.");
  }

  void
  ct_incomplete::destruct(void*) const
  {
      throw std::runtime_error("more::lang::ct_incomplete::destruct: "
			       "Can not destruct an object of an incomplete "
			       "type.");
  }

  bool
  ct_incomplete::equal(void*, void*) const
  {
      throw std::runtime_error("more::lang::ct_incomplete::equal: "
			       "Can not compare objects of an incomplete "
			       "type.");
  }

  void
  ct_incomplete::print_declaration_pre(std::ostream& os,
				       printopt_type opt) const
  {
      if (opt & printopt_space_before_name)
	  os << ' ';
      os << m_c_name;
      if (opt & printopt_space_after_name)
	  os << ' ';
  }

  void
  ct_incomplete::print_declaration_post(std::ostream& os,
					printopt_type opt) const
  {
  }

  void
  ct_incomplete::hash(hash_type& h) const
  {
      h.insert(m_c_name + 0, m_c_name + std::strlen(m_c_name));
//       throw std::runtime_error("more::lang::ct_incomplete::hash: "
// 			       "Can not hash an incomplete type.");
  }

  bool
  ct_incomplete::equal_to(ct_type const* rhs) const
  {
      // XXX Might want to cross-compare with ct_struct.
      if (ct_incomplete const* t = dynamic_cast<ct_incomplete const*>(rhs))
	  return std::strcmp(m_c_name, t->m_c_name) == 0;
      else
	  return false;
  }



  //--------------------------------------------------------------------
  //			      ct_struct
  //--------------------------------------------------------------------

  ct_struct::ct_struct(char const* c_name)
      : m_c_name(c_name),
	m_base_link(0),
	m_is_frozen(0),
	m_do_optimize(!c_name) {}

  ct_struct::~ct_struct()
  {
//GC      delete[] m_arr;
//GC      delete[] m_ffi_type.elements;
  }

  void
  ct_struct::construct_copy(void* dst, void* src) const
  {
      for (member_const_iterator it = member_begin();
	   it != member_end(); ++it) {
	  ct_type const* rtt = it->type_of();
	  assert(rtt);
	  std::size_t offset = it->offset();
	  rtt->construct_copy((char*)dst + offset,
			      (char*)src + offset);
      }
  }


  ct_struct*
  ct_struct::clone() const { return new(UseGC) ct_struct(*this); }

  void
  ct_struct::finish()
  {
      m_is_frozen = 1;
      if (m_do_optimize) {
	  // XXX

	  // Distribute bitfields into substructures wherever there is
	  // declared bitspace.  Merge the other bitfields into one or
	  // more suitable unsigned integer types.  Use as small types
	  // as possible (two uint8 is better than one uint16) to make
	  // them easier to align and pack into substructures.

	  // For each substructure which leaves an has open space,
	  // replace it with a duplicate, and put in the largest
	  // members which fulfills the alignment and fits in.  (This
	  // is not always the most optimal, but it's simple.)

	  // Sort with descending alignment
      }

#ifdef MORE_LANG_PROVIDE_FFI
      m_ffi_type.size = 0;
      m_ffi_type.type = FFI_TYPE_STRUCT;
      m_ffi_type.elements = new(UseGC) ffi_type*[member_count() + 1];
      int i = 0;
      size_type o = 0;
      size_type a = 1;
      for (member_iterator it = member_begin(); it != member_end(); ++it) {
	  assert(it->type_of());
	  ffi_type* t = it->type_of()->ffi_type_of();
	  assert(t);
	  o = (o + t->alignment - 1)/t->alignment*t->alignment;
	  it->m_offset = o;
	  o += t->size;
	  m_ffi_type.elements[i++] = t;
	  if (t->alignment > a)
	      a = t->alignment;
      }
      m_ffi_type.elements[i] = 0;
      if (0) {
	  // This should fill in the alignment and size of m_ffi_type?
	  ffi_cif cif;
	  ffi_type* atypes[] = { &m_ffi_type };
	  ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &ffi_type_void, atypes);
      }
      else {
	  // Do it ourselves, but this may be less portable
	  o = (o + a - 1)/a*a;
	  m_ffi_type.size = o;
	  m_ffi_type.alignment = a;
      }
#else
      size_type o = 0;
      alignment_type a = 0;
      for (member_iterator it = member_begin(); it != member_end(); ++it) {
	  ct_type const* t = it->type_of();
	  size_type size0 = t->size();
	  alignment_type a0 = t->alignment();
	  o = (o + a0 - 1)/a0*a0;
	  it->m_offset = o;
	  o += size0;
	  if (a0 > a)
	      a = a0;
      }
      o = (o + a - 1)/a*a;
#endif
#ifndef MORE_LANG_USE_FFI
      set_size_and_alignment(o, a);
#endif

      if (m_c_name == 0) {
	  hash_type h(0);
	  hash(h);
	  char name[hash_type::c_name_strlen + 1];
	  h.strncpy_to_c_name(name);
	  name[hash_type::c_name_strlen] = 0;
	  m_c_name = name;
      }
  }


  ct_struct::member_iterator
  ct_struct::append(ct_type const* t, char const* c_name)
  {
      assert(t);
      req_not_frozen();
      m_lst.push_back(member(t, c_name));
      return --m_lst.end();
  }

  ct_struct::member_iterator
  ct_struct::append_inherited(ct_type const* t)
  {
      assert(t);
      req_not_frozen();
      m_lst.push_back(member(t, gen::struniq_dense(member_count()),
				m_base_link));
      ct_struct::member_iterator it = m_lst.end();
      --it;
      m_base_link = &*it;
      return it;
  }

  std::pair<ct_struct const*, ct_struct::member const*>
  ct_struct::find_inherited_rec(ct_type const* t) const
  {
      // We can not report an exact match, so require that this is
      // already checked.
      if (t == this)
	  throw std::logic_error("more::lang::ct_struct::find_inherited_rec: "
				 "Type to find is the same as type to search.");

      for (member const* md = m_base_link; md; md = md->m_base_link)
	  if (md->m_ct_type == t)
	      return std::make_pair(this, md);

      ct_struct const* s_cand = 0;
      member const* md_cand = 0;
      for (member* md = m_base_link; md; md = md->m_base_link) {
	  if (ct_struct const* s
	      = dynamic_cast<ct_struct const*>(md->m_ct_type)) {
	      ct_struct const* s0;
	      member const* md0;
	      gen::tie(s0, md0) = s->find_inherited_rec(t);
	      if (s0) {
		  if (s_cand) // ambiguous
		      return std::make_pair(this, (member const*)0);
		  else {
		      s_cand = s0;
		      md_cand = md0;
		  }
	      }
	  }
      }
      // valid or none
      return std::make_pair(s_cand, md_cand);
  }

  void*
  ct_struct::proj(void* ptr, ct_type const* t) const
  {
      if (!ptr)
	  return 0;
      if (t == this)
	  return ptr;
      ct_struct const* subsct;
      member const* memb;
      gen::tie(subsct, memb) = find_inherited_rec(t);
      if (memb)
	  return memb->apply(ptr);
      else if (subsct)
	  throw std::runtime_error("more::lang::resolver::"
				   "resolve(ct_type const*): "
				   "Ambiguous cast.");
      else
	  return 0;
  }

#ifdef MORE_LANG_PROVIDE_FFI
  ffi_type*
  ct_struct::ffi_type_of() const
  {
      req_frozen();
      return &m_ffi_type;
  }
#endif

//   void*
//   ct_struct::member(void* p, member_const_iterator it) const
//   {
//       req_frozen();
//       return (char*)p + it->m_offset;
//   }

  // The immediate base classes are lined into the ct_struct as field
  // locations with is_base() true.  The locations inside a field
  // location are relative to the field location, so the location
  // returned by base(...) is composed with intext.  C++ classes are
  // treated the same way, where the offsets are offsets of the base
  // class rather than fields.
//    location*
//    ct_struct::base_location(any* meta, location* cxt)
//    {
//        // Is this needed?
//        if (m_state < state_compiled)
//  	  compiled();

//        location* loc = 0;
//        for (location_iterator* it = location_begin();
//  	   it != location_end(); ++it) {
//  	  if (it->is_base()) {
//  	      if (it->meta() == meta)
//  	  }
//        }
//    }

  void
  ct_struct::destruct(void* p) const
  {
      member_const_iterator it = member_end();
      while (it != member_begin()) {
	  --it;
	  void* pm = it->apply(p);
	  it->type_of()->destruct(pm);
      }
  }

  bool
  ct_struct::equal(void* p0, void* p1) const
  {
      for (member_const_iterator it = member_begin(); it != member_end(); ++it)
	  if (!it->type_of()->equal(it->apply(p0), it->apply(p1)))
	      return false;
      return true;
  }

  void
  ct_struct::req_not_frozen() const
  {
      if (is_frozen())
	  throw std::logic_error("ct_struct::append: "
				 "Object must be frozen.");
  }

  void
  ct_struct::print_forward(std::ostream& os, printopt_type opt) const
  {
      if (m_do_optimize)
	  return;
      if (!is_frozen())
	  throw std::logic_error("ct_struct::print_definition: "
				 "Object must be frozen.");
      if (!(opt & printopt_lang_cxx))
	  os << "typedef struct "
	     << s_private_pfx << m_c_name << ' ' << m_c_name << ";\n";
      else
	  os << "struct " << m_c_name << ";\n";
  }

  void
  ct_struct::print_declaration_pre(std::ostream& os, printopt_type opt) const
  {
      req_frozen();
      if (opt & printopt_space_before_name)
	  os << ' ';
      os << m_c_name;
      if (opt & printopt_space_after_name)
	  os << ' ';
  }

  void
  ct_struct::print_declaration_post(std::ostream& os, printopt_type opt) const
  {
      req_frozen();
  }

  void
  ct_struct::print_definition(std::ostream& os, printopt_type opt) const
  {
      if (m_do_optimize)
	  return;
      if (!is_frozen())
	  throw std::logic_error("ct_struct::print_definition: "
				 "Object must be frozen.");
      if (!(opt & printopt_lang_cxx))
	  os << "struct " << s_private_pfx;
      else
	  os << "struct ";
      os << m_c_name << "\n{\n";
      for (member_const_iterator it = member_begin();
	   it != member_end(); ++it) {
	  os << '\t';
	  it->type_of()->print_declaration(os, opt, it->c_name());
	  os << ";\n";
      }
      os << "};\n";
  }

  void
  ct_struct::hash(hash_type& h) const
  {
      h.insert(math::hash_id32('l', 'r', 't', 's', 't', 'r'));
      for (member_const_iterator it = member_begin();
	   it != member_end(); ++it) {
	  it->type_of()->hash(h);
	  if (it->c_name())
	      h.insert(it->c_name(), it->c_name() + strlen(it->c_name()) + 1);
      }
  }

  bool
  ct_struct::equal_to(ct_type const* rhs0) const
  {
      req_frozen();
      if (ct_struct const* rhs = dynamic_cast<ct_struct const*>(rhs0)) {
	  // print_definition already depends on there being a
	  // c_name(), and to types in C are never identical if they
	  // have separate definitions.
	  return is_const() == rhs->is_const()
	      && is_volatile() == rhs->is_volatile()
	      && c_name() == rhs->c_name();

	  // There may be some cases when pure layout structs could be
	  // useful, and those would compare as:
// 	  for (member_const_iterator
// 		   it = member_begin(),
// 		   it_rhs = rhs->member_begin();; ++it, ++it_rhs) {
// 	      if (it == member_end())
// 		  return it_rhs == rhs->member_end();
// 	      else if (it_rhs == rhs->member_end())
// 		  return false;
// 	      else if (!it->type_of()->equal_to(it_rhs->type_of()))
// 		  return false;
// 	      else if (it->offset() != it_rhs->offset())
// 		  return false;
// 	      else if (it->c_name() != it_rhs->c_name())
// 		  return false;
// 	      else if (it->is_static() != it_rhs->is_static())
// 		  return false;
// 	  }
// 	  return true;
      }
      else
	  return false;
  }




  
}}
