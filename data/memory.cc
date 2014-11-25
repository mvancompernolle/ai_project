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

#include <more/gen/memory.h>

namespace more {
namespace gen {

  bad_deref::~bad_deref() throw() {}

  char const*
  bad_deref::what() const throw()
  {
      if (m_what)
	  return m_what;
      else
	  return "more::gen::bad_deref: "
		 "Invalid dereference of pointer or iterator.";
  }


#ifdef MORE_CONF_HAVE_BDWGC
  namespace bits {

    //----------------------------------------------------------------
    //			    weak_ptr_base
    //----------------------------------------------------------------

    weak_ptr_base::weak_ptr_base(void* ptr) throw()
	: m_atom(GC_NEW_ATOMIC(atom_type))
    {
	(*m_atom)[0] = (*m_atom)[1] = ptr;
	if (ptr)
	    GC_GENERAL_REGISTER_DISAPPEARING_LINK(&(*m_atom)[0], ptr);
    }

    void
    weak_ptr_base::reset_base(void* ptr) throw()
    {
	m_atom = GC_NEW_ATOMIC(atom_type);
	(*m_atom)[0] = (*m_atom)[1] = ptr;
	if (ptr)
	    GC_GENERAL_REGISTER_DISAPPEARING_LINK(&(*m_atom)[0], ptr);
    }

    void*
    weak_ptr_base::mt_get(void* wpb) throw()
    {
	return static_cast<weak_ptr_base*>(wpb)->m_atom[0];
    }

    void*
    weak_ptr_base::mt_eq(void* args) throw()
    {
	if (static_cast<weak_ptr_base*>(((void**)args)[0])->m_atom[1]
	    == static_cast<weak_ptr_base*>(((void**)args)[1])->m_atom[1])
	    return (void*)1;
	else
	    return 0;
    }

    void*
    weak_ptr_base::mt_lt(void* args) throw()
    {
	if (static_cast<weak_ptr_base*>(((void**)args)[0])->m_atom[1]
	    < static_cast<weak_ptr_base*>(((void**)args)[1])->m_atom[1])
	    return (void*)1;
	else
	    return 0;
    }
  }

#endif

}}
