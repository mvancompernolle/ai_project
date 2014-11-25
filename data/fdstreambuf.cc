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


#include <iostream>
#include <stdexcept>
#include <more/bits/fdstreambuf.h>

#include <unistd.h>


namespace more {
namespace io {

  namespace bits {

    template<typename CharT, typename Traits>
      basic_fdstreambuf<CharT, Traits>::
      ~basic_fdstreambuf()
      {
	  sync();
	  delete this->pbase();
	  delete this->eback();
      }

    template<typename CharT, typename Traits>
      typename basic_fdstreambuf<CharT, Traits>::int_type
      basic_fdstreambuf<CharT, Traits>::
      overflow(int_type c)
      {
	  /* test for the empty sequence */
	  if (Traits::eq_int_type(c, Traits::eof()) &&
	      (!this->pptr() || (this->pptr() == this->pbase())))
	      return Traits::not_eof(c);

	  /* otherwise require that we are capable of output */
	  if (fd_out < 0)
	      throw std::logic_error("more::bits::fdstreambuf: "
				     "Not capable of output.");

	  /* if no buffer, make one, otherwise empty it */
	  if (!this->pptr()) {
	      CharT* buf = new CharT[nbuf_put];
	      setp(buf, buf + nbuf_put);
	  }
	  else if (this->pbase() != this->pptr()) {
	      int n_wish = this->pptr() - this->pbase();
	      int n = ::write(fd_out, this->pbase(), n_wish*sizeof(CharT));
	      if (n != n_wish)
		  return Traits::eof();
	      setp(this->pbase(), this->epptr());
	  }

	  if (Traits::eq_int_type(c, Traits::eof()))
	      return Traits::not_eof(c);
	  else {
	      *this->pbase() = c;
	      this->pbump(1);
	      return c;
	  }
      }

    template<typename CharT, typename Traits>
      typename basic_fdstreambuf<CharT, Traits>::int_type
      basic_fdstreambuf<CharT, Traits>::
      underflow()
      {
	  if (fd_in < 0)
	      throw std::logic_error("more::bits::fdstreambuf: "
				     "Not capable of input.");
	  if (!this->eback()) {
	      CharT* p = new CharT[nbuf_get];
	      setg(p, p, p);
	  }
	  int nread = ::read(fd_in, this->eback(), nbuf_get*sizeof(CharT));
	  if (nread <= 0)
	      return Traits::eof();
	  else {
	      this->setg(this->eback(), this->eback(), this->eback() + nread);
	      return *this->eback();
	  }
      }

    template<typename CharT, typename Traits>
      int
      basic_fdstreambuf<CharT, Traits>::
      sync()
      {
	  overflow();
	  if (fd_in >= 0) fsync(fd_out);
// 	  if (fd_in >= 0) fsync(fd_in); XXX?
	  return 0;
      }

    template class basic_fdstreambuf< char, std::char_traits<char> >;

  } // namespace bits

}
}
