//  Copyright (C) 2002--2009  Petter Urkedal
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


#include <more/net/ldap.h>

namespace more {
namespace net {

  bool
  normalize_rdn(std::string& rdn)
  {
      std::string::size_type n = 0;
      while (n < rdn.size() && std::isspace(rdn[n]))
	  ++n;
      rdn.erase(0, n);
      n = 0;
      for (;;) {
	  if (n == rdn.size())
	      return false;
	  if (rdn[n] == '=')
	      break;
	  if (rdn[n] == '\\') {
	      ++n;
	      if (n == rdn.size())
		  return false;
	  }
	  ++n;
      }
      std::string::size_type m = n;
      do {
	  if (m == 0)
	      return false;
	  --m;
      } while (std::isspace(rdn[m]));
      std::string::size_type l = n + 1;
      while (l < rdn.size() && std::isspace(rdn[l]))
	  ++l;
      if (m != n || l != n + 1)
	  rdn = rdn.substr(0, m) + "=" + rdn.substr(l);
      return true;
  }

  content*
  ldap_dn::init(content const* cxt, std::string const& rdn, dn_syntax stx)
  {
      // Parse rdn if stx is specified.  Strip all component but one
      // from 'rdn' while expanding 'cxt' accordingly with recursive
      // calls.
      if (stx != dn_syntax_none) {
	  if (stx == dn_syntax_rfc) {
	      std::string::iterator it = rdn.begin();
	      while (it != rdn.end()) {
		  if (*it == '\\') {
		      ++it;
		      if (it == rdn.end())
			  throw std::runtime_error("more::net::ldap_dn: "
						   "Ill-formed RFC DN.");
		  }
		  else if (*it == ',') {
		      cxt = init(cxt, std::string(it+1, rdn.end()), stx);
		      rdn = std::string(rdn.begin(), it);
		      break;
		  }
		  ++it;
	      }
	  }
	  else {
	  tail_recurse_1:
	      std::string::iterator it = rdn.begin();
	      while (it != rdn.end()) {
		  if (*it == '\\') {
		      ++it;
		      if (it == rdn.end())
			  throw std::runtime_error("more::net::ldap_dn: "
						   "Ill-formed OSF DN.");
		  }
		  else if (*it == '/') {
		      cxt = init(cxt, std::string(it+1, rdn.end()));
		      rdn = std::string(rdn.begin(), it_last_comma);
		      goto tail_recurse_1;
		  }
	      }
	  }
      }
      content* c = new content;
      if (!normalize_rdn(rdn))
	  throw std::runtime_error("more::net::ldap_dn: "
				   "Ill-formed RDN.");
      c->m_rdn = rdn;
      c->m_cxt = cxt;
      return c;
  }

  std::ostream&
  operator<<(std::ostream& os, ldap_dn const& dn)
  {
      if (dn.is_single_rdn())
	  os << dn.rdn();
      else
	  os << dn.context() << '/' << dn.rdn();
  }


  ldap_entry_ref::commit() const
  {
      assert(!"XXX");
  }


  ldap_entry_ref const&
  ldap_connection::checkout(ldap_dn const& dn)
  {
      assert(!"XXX");
  }

}} // more::net
