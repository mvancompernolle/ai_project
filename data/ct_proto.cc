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


#include <more/lang/ct_proto.h>
#include <more/lang/cx_expr.h>
#ifndef MORE_LANG_USE_FFI
#  include <more/lang/package.h>
#endif

namespace more {
namespace lang {


  //--------------------------------------------------------------------
  //			   ct_proto_handler
  //--------------------------------------------------------------------

#ifndef MORE_LANG_USE_FFI
  struct ct_proto_handler
      : cx_package_variable
  {
      ct_proto_handler(ct_proto* proto)
	  : cx_package_variable(ifproto()),
	    m_proto(proto)
      {
	  set_version(0);
      }

      virtual cx_expr* make_definiens();

    private:
      static ct_proto* ifproto();

      ct_proto*		m_proto;
      static ct_proto*	s_proto;
  };

  cx_expr*
  ct_proto_handler::make_definiens()
  {
      cx_lambda* fn = new(UseGC) cx_lambda(ifproto());

      std::size_t n = m_proto->arg_count();
      cx_expr** args = new(UseGC) cx_expr*[n];
      for (std::size_t i = 0; i < n; ++i)
	  args[i]
	      = make_deref_expr(
		  make_cast(new(UseGC) ct_pointer(m_proto->arg_type(i)),
			    make_element_expr(fn->arg(2),
					      make_literal(i))));
      cx_call* call
	  = new(UseGC) cx_call(make_cast(new(UseGC) ct_pointer(m_proto),
					 fn->arg(0)), args);
      if (m_proto->result_type() == ct_type_of<void>())
	  fn->body()->append_stmt(call);
      else
	  fn->body()->append_stmt(
	      make_assignment(
		  make_deref_expr(
		      make_cast(new(UseGC) ct_pointer(m_proto->result_type()),
				fn->arg(1))),
		  call));
      return fn;
  }

  ct_proto*
  ct_proto_handler::ifproto()
  {
      if (!s_proto) {
	  s_proto = new(UseGC) ct_proto(3, ct_type_of<void>());
	  s_proto->set_arg_type(0, ct_type_of<fn_ptr_t>());
	  s_proto->set_arg_type(1, ct_type_of<void*>());  // result
	  s_proto->set_arg_type(2, ct_type_of<void**>()); // args
	  s_proto->freeze();
      }
      return s_proto;
  }

  ct_proto*
  ct_proto_handler::s_proto = 0;
#endif


  //--------------------------------------------------------------------
  //			       ct_proto
  //--------------------------------------------------------------------

  ct_proto::ct_proto(size_type n_args, ct_type const* t_res)
#ifdef MORE_LANG_USE_FFI
      : ct_type(true, false),
#else
      : ct_type(0, 0, true, false),
#endif
	m_arg_arr(new(UseGC) arg_type_type[n_args]),
	m_arg_count(n_args),
	m_result_type(t_res),
#ifndef MORE_LANG_USE_FFI
	m_adaptor(0),
#endif
	m_frozen_p(0)
  {
      std::fill(m_arg_arr, m_arg_arr + n_args, (ct_type*)0);
  }

  ct_proto::~ct_proto()
  {
  }

  void
  ct_proto::construct_copy(void* dst, void* src) const
  {
      throw std::logic_error("more::lang::ct_proto: "
			     "Can not construct copy of a function.");
  }

  ct_proto*
  ct_proto::clone() const
  {
      // Since CV qualifiers do not apply to functions (only to the
      // pointer to functions), there is no reason to copy this.  The
      // problem is that location used when !MORE_LANG_USE_FFI is
      // non-copyable.
      throw std::logic_error("more::lang::ct_proto::clone(): "
			     "Prototypes can not be cloned.");
//       return new(UseGC) ct_proto(*this);
  }

  void
  ct_proto::finish()
  {
      if (!m_result_type)
	  throw std::logic_error("more::lang::ct_proto: "
				 "Missing result type.");
      m_frozen_p = 1;
#ifdef MORE_LANG_USE_FFI
      m_ffi_arg_types = new(UseGC) ffi_type*[m_arg_count];
      int i = 0;
      for (arg_type_mutable_iterator it = arg_type_mutable_begin();
	   it != arg_type_mutable_end(); ++it) {
#  ifndef MORE_LANG_NDEBUG
	  if (!*it)
	      throw std::runtime_error("more::lang::ct_proto: "
				       "Missing type for argument.");
#  endif
	  m_ffi_arg_types[i++] = (*it)->ffi_type_of();
      }
      if (ffi_prep_cif(&m_cif, FFI_DEFAULT_ABI, m_arg_count,
		       m_result_type->ffi_type_of(), m_ffi_arg_types) != FFI_OK)
	  throw std::runtime_error("more::lang::ct_proto: "
				   "Cound not create ffi_cif");
#else
#  ifndef MORE_LANG_NDEBUG
      for (arg_type_iterator it = arg_type_begin();
	   it != arg_type_end(); ++it)
	  if (!*it)
	      throw std::runtime_error("more::lang::ct_proto: "
				       "Missing type for argument.");
#  endif
      math::hash<192> hc(0);
      hash(hc);
      char name[math::hash<192>::c_name_strlen + 1];
      hc.strncpy_to_c_name(name);
      name[math::hash<192>::c_name_strlen] = 0;
      if (!s_adaptor_pkg)
	  s_adaptor_pkg
	      = new(UseGC) package(bits_package(), "adapt",
				   package::flag_split, package::flag_split);
      m_adaptor.set_resolver(new(UseGC) ct_proto_handler(this));
      s_adaptor_pkg->define(name, m_adaptor, true);
#endif
  }

  void
  ct_proto::call(fn_ptr_t f, void* ret, void** args) const
  {
      req_frozen();
#ifdef MORE_LANG_USE_FFI
      ffi_call(&m_cif, (void(*)())f, ret, args);
#else
      ((void (*)(fn_ptr_t, void*, void**))m_adaptor.get())(f, ret, args);
#endif
  }

#ifdef MORE_LANG_PROVIDE_FFI
  ffi_type*
  ct_proto::ffi_type_of() const
  {
      req_frozen();
      // XXX The type of a function pointer is not necessarily
      // compatible with a data pointer.
      return &ffi_type_pointer;
  }
#endif

  void ct_proto::destruct(void* p) const { req_frozen(); }

  bool
  ct_proto::equal(void* p0, void* p1) const
  {
      req_frozen();
      return *static_cast<void (**)(...)>(p0)
	  == *static_cast<void (**)(...)>(p1);
  }

  void
  ct_proto::print_declaration_pre(std::ostream& os, printopt_type opt) const
  {
      result_type()->print_declaration_pre(os,
					   (opt & printopt_lang_cxx)
					   | printopt_space_after_name);
      if (opt & printopt_low_precedence)
	  os << '(';
  }

  void
  ct_proto::print_declaration_post(std::ostream& os, printopt_type opt,
				   cx_identifier const* const* args) const
  {
      if (opt & printopt_low_precedence)
	  os << ')';
      os << "(";
      if ((opt & printopt_function_args) || args) {
	  std::size_t i = 0;
	  for (arg_type_iterator it = arg_type_begin();
	       it != arg_type_end(); ++it) {
	      if (i)
		  os << ',';
	      (*it)->print_declaration_pre(os,
					   (opt & printopt_lang_cxx)
					   | printopt_space_after_name);
	      if (args)
		  args[i]->print_as_expr(os);
	      else
		  os << "arg" << i;
	      (*it)->print_declaration_post(os, opt & printopt_lang_cxx);
	      ++i;
	  }
	  if (i == 0 && !(opt & printopt_lang_cxx))
	      os << "void";
      }
      else {
	  arg_type_iterator it = arg_type_begin();
	  if (it != arg_type_end()) {
	      (*it)->print_declaration(os, opt & printopt_lang_cxx);
	      while (++it != arg_type_end()) {
		  os << ", ";
		  (*it)->print_declaration(os, opt & printopt_lang_cxx);
	      }
	  }
	  else if (!(opt & printopt_lang_cxx))
	      os << "void";
      }
      os << ')';
      result_type()->print_declaration_post(os, opt & printopt_lang_cxx);
  }

  void
  ct_proto::print_declaration_post(std::ostream& os, printopt_type opt) const
  {
      print_declaration_post(os, opt, 0);
  }

  void
  ct_proto::hash(hash_type& h) const
  {
      h.insert(math::hash_id32('l', 'r', 't', 'p', 'r', 'o'));
      for (arg_type_iterator it = arg_type_begin();
	   it != arg_type_end(); ++it)
	  (*it)->hash(h);
  }

  bool
  ct_proto::equal_to(ct_type const* rhs0) const
  {
      req_frozen();
      if (ct_proto const* rhs = dynamic_cast<ct_proto const*>(rhs0)) {
	  if (arg_count() != rhs->arg_count())
	      return false;
	  for (size_type i = 0; i < arg_count(); ++i)
	      if (!arg_type(i)->equal_to(rhs->arg_type(i)))
		  return false;
	  return result_type()->equal_to(rhs->result_type());
      }
      else
	  return false;
  }

#ifndef MORE_LANG_USE_FFI
  package*
  ct_proto::s_adaptor_pkg = 0;
#endif

  ct_proto*
  make_proto(ct_type const* arg, ...)
  {
      va_list va;
      va_start(va, arg);
      std::size_t n_args = 0;
      while (va_arg(va, ct_type const*))
	  ++n_args;
      va_end(va);
      va_start(va, arg);
      ct_proto* proto = new(UseGC) ct_proto(n_args);
      ct_type const* arg_prev = arg;
      ct_proto::arg_type_mutable_iterator it_arg
	  = proto->arg_type_mutable_begin();
      while (ct_type const* arg = va_arg(va, ct_type const*)) {
	  *it_arg++ = arg_prev;
	  arg_prev = arg;
      }
      proto->set_result_type(arg_prev);
      proto->freeze();
      return proto;
  }

}}
