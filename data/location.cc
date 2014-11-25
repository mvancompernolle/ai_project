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


#include <more/lang/location.h>
#include <more/lang/ct_struct.h>
#include <more/lang/ct_proto.h>
#include <more/gen/utility.h>

namespace more {
namespace lang {

  //--------------------------------------------------------------------
  //			       resolver
  //--------------------------------------------------------------------

//   resolver::resolver(ct_type const* rtt)
//       : cx_expr(rtt),
// 	m_client(0) {}

  resolver::resolver()
      : m_client(0) {}

  resolver::~resolver() {}

#if 0
  void*
  resolver::resolve(ct_type const* t)
  {
      if (!m_client)
	  throw std::logic_error("more::lang::resolver::"
				 "resolve(ct_type const*): "
				 "Missing client location.");
      if (resolve_type_of() != 0 && t->equal_to(resolve_type_of()))
	  return m_client->get();
      else if (ct_struct const*
	       s = dynamic_cast<ct_struct const*>(resolve_type_of())) {
	  void* ptr = m_client->get();
	  return s->proj(ptr, t);
      }
      return 0;
  }
#endif

  void
  resolver::resolve_call(void* res, void** args)
  {
      if (void* ptr = resolve())
	  static_cast<ct_proto const*>(resolve_type_of())
	      ->call((fn_ptr_t)ptr, res, args);
      else
	  throw std::logic_error("more::lang::resolver::call: "
				 "Could not resolve function pointer.");
  }

//   bool
//   resolver::equal_to(resolver const* rhs) const
//   {
//       return m_client->get() == rhs.m_client->get();
//   }

  //--------------------------------------------------------------------
  //			       location
  //--------------------------------------------------------------------

  location::location(resolver* r, void* ptr)
      : m_ptr(ptr),
	m_resolver(r)
  {
      if (r)
	  r->m_client = this;
  }

  location::~location()
  {
      if (m_resolver)
	  m_resolver->m_client = 0;
  }

  void
  location::call(void* ret, void** args)
  {
#ifndef MORE_LANG_NDEBUG
      if (dynamic_cast<ct_proto const*>(type_of()) == 0)
	  throw std::logic_error("more::lang::resolver"
				 "(void*, void**): "
				 "Attempted to call non-function.");
#endif
      if (m_ptr)
	  static_cast<ct_proto const*>(type_of())
	      ->call((fn_ptr_t)m_ptr, ret, args);
      else
	  m_resolver->resolve_call(ret, args);
  }



}}
