//  Copyright (C) 1999--2009  Petter Urkedal
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


#define MORE_BACKWARD 0

#include <more/math/qtypes.h>
#include <more/math/complex.h>
#include <more/io/syncstream.h>
#include <ios>
#include <iostream>
#include <iomanip>
#include <cctype>


namespace more {
namespace math {

  onei_tag onei;
  one_tag one;
  zero_tag zero;
  half_tag half;

  //--------------------------------------------------------------------
  //			       pm_half
  //--------------------------------------------------------------------

  void
  pm_half::sync(io::syncstream& sio)
  {
      sio | e;
  }

  std::ostream&
  operator<<(std::ostream& os, pm_half x)
  {
      if(x == -half) os << "-1/2";
      else os << "1/2";
      return os;
  }

  std::istream&
  operator>>(std::istream& is, pm_half& x)
  {
      char c;
      is >> c;
      if(c == '-') { x = -half; is >> c; }
      else if(c == '+') { x = half; is >> c; }
      else x = half;
      if(c != '1' || is.get() != '/' || is.get() != '2')
	  is.setstate(std::ios_base::failbit);
      return is;
  }


  //--------------------------------------------------------------------
  //			       halfint
  //--------------------------------------------------------------------

  void
  halfint::sync(io::syncstream& sio)
  {
      sio | twice;
  }

  std::ostream&
  operator<<(std::ostream& os, halfint x)
  {
      if(x.twice&1) {
	  if(os.width() > 2) os.width(os.width()-2);
	  os << x.twice << "/2";
      }
      else os << x.twice/2;
      return os;
  }

  std::istream&
  operator>>(std::istream& is, halfint& x)
  {
      is >> x.twice;
      char c = is.peek();
      if(c == '/') {
	  is.get();
	  c = is.get();
	  if(c != '2') is.setstate(std::ios_base::failbit);
	  c = is.peek();
	  if(isdigit(c)) is.setstate(std::ios_base::failbit);
      }
      else {
	  x.twice *= 2;
      }
      return is;
  }


  //--------------------------------------------------------------------
  //				pm_one
  //--------------------------------------------------------------------

  void
  pm_one::sync(io::syncstream& sio)
  {
      sio | e;
  }

  std::ostream&
  operator<<(std::ostream& os, pm_one pi)
  {
      os << (pi == one? '+' : '-');
      return os;
  }
  std::istream&
  operator>>(std::istream& is, pm_one& pi)
  {
      char c;
      is >> c;
      switch(c) {
	case '+':  pi = one; break;
	case '-':  pi = -one; break;
	default:   /*is.putback(c);*/ is.setstate(std::ios::failbit); break;
      }
      return is;
  }

  template<typename T>
    std::ostream&
    operator<<(std::ostream& os, range<T> const& x)
    {
	os << x.inf() << ".." << x.sup();
	return os;
    }

  //  !! backward !!
  one_tag positive;
  minus_one_tag negative;
  //  !! backward !!
}}
