//  Copyright (C) 2000--2009  Petter Urkedal
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


//  Status: experimental.

//  BINARY FORMAT
//
//  The intent of this binary format is to have a compact platform
//  independent representation of any object.  To do so, we encode
//  enough type information that we can distinguish integers from
//  raw data (bit patterns, strings), but that's about it.
//
//  Integers are represented as
//
//	integer ::= non_negative_integer | negative_integer
//
//      non_negative_integer ::= 00xxxxxx | 10xxxxxx (1xxxxxxx)* 0xxxxxxx
//      negative_integer     ::= 01xxxxxx | 11xxxxxx (1xxxxxxx)* 0xxxxxxx
//
//  where the concatenation of the `x's is the binary representation of
//  the absolute value of the number.
//
//  This leaves one some redundant values, in particular we use the
//  following to represent special objects
//
//      marker  ::= 01000000
//
//  A string or a sequence of raw bytes is represented by a marker
//  followed by a non-negative integer representing the size in bytes,
//  followed by the data itself.
//
//      raw_data  ::= marker non_negative_integer byte*
//
//  Floating point numbers are represented as two integers, and
//  some special markers for the singular values
//
//      float   ::= integer integer
//      -Inf    ::= marker `-3'
//      NaN     ::= marker `-2'
//      Inf     ::= marker `-1'
//
//  Notice that the following is also unoccupied:
//
//      reserved ::= 1x000000 00xxxxxx


#include <more/io/binary_io.h>
#include <more/meta.h>
#include <more/math/math.h>
#include <iomanip>
#include <assert.h>


namespace more {
namespace io {

  template<typename T>
    T shift_right(T x, unsigned short y)
    {
	while (y > 8) { x >>= 8; y -= 8; }
	return x >> y;
    }


  //  --- Integer ---

#ifdef MORE_MORE_CONF_HAVE_LONG_LONG
  typedef unsigned long long big_uint;
  typedef long long big_int;
#else
  typedef unsigned long big_uint;
  typedef long big_int;
#endif

  namespace {
    template<typename Char>
      struct fmt_traits {
	  typedef typename to_unsigned_<Char>::eval value_type;
	  typedef unsigned int bitno_type;
	  static const bitno_type cnt_bit = sizeof(value_type)*8-1;
	  static const bitno_type sign_bit = sizeof(value_type)*8-2;
	  static const value_type cnt_flag = 1 << cnt_bit;
	  static const value_type sign_flag = 1 << sign_bit;
	  static const value_type first_mask = (1<<sign_bit) - 1;
	  static const bitno_type first_bitcount = sizeof(value_type)*8-2;
	  static const value_type other_mask = (1<<cnt_bit) - 1;
	  static const bitno_type other_bitcount = sizeof(value_type)*8-1;
	  static const value_type marker = sign_flag;
      };

    template<typename Char>
      void
      binary_put_unsigned(std::basic_ostream<Char>& os,
			  big_uint x, bool neg)
      {
	  typedef fmt_traits<Char> fmt;
	  typedef typename fmt::value_type value_type;
	  typedef typename fmt::bitno_type bitno_type;

	  bitno_type bitno = fmt::first_bitcount;
	  while (shift_right(x, bitno)) {
	      assert(bitno < (sizeof(big_uint) + 1)*8);
	      bitno += fmt::other_bitcount;
	  }
	  bitno -= fmt::first_bitcount;

	  value_type byte = shift_right(x, bitno) & fmt::first_mask;
	  if (neg) byte |= fmt::sign_flag;
	  while (os.good() && bitno) {
	      os.put(byte | fmt::cnt_flag);
	      bitno -= fmt::other_bitcount;
	      byte = shift_right(x, bitno) & fmt::other_mask;
	  }
	  os.put(byte);
      }

    template<typename Char>
      big_uint
      binary_get_unsigned(std::basic_istream<Char>& is, bool& neg)
      {
	  typedef fmt_traits<Char> fmt;
	  typedef typename fmt::value_type value_type;
	  typedef typename fmt::bitno_type bitno_type;

	  value_type byte = is.get();
	  big_uint x = byte & fmt::first_mask;
	  neg = byte & fmt::sign_flag;
	  while (is.good() && (byte & fmt::cnt_flag)) {
	      byte = is.get();
	      x <<= fmt::other_bitcount;
	      x |= byte & fmt::other_mask;
	  }
	  return x;
      }

    template<typename Char>
      inline void
      binary_put_unsigned(std::basic_ostream<Char>& os, big_uint x)
      {
	  binary_put_unsigned(os, x, false);
      }

    template<typename Char>
      inline big_uint
      binary_get_unsigned(std::basic_istream<Char>& is)
      {
	  big_uint y;
	  bool neg;
	  y = binary_get_unsigned(is, neg);
	  if(neg) is.setstate(std::ios_base::failbit);
	  return y;
      }

    template<typename Char>
      inline void
      binary_put_signed(std::basic_ostream<Char>& os, big_int x)
      {
	  if(x < 0) binary_put_unsigned(os, -x, true);
	  else      binary_put_unsigned(os, x, false);
      }

    template<typename Char>
      inline big_int
      binary_get_signed(std::basic_istream<Char>& is)
      {
	  big_uint y;
	  bool neg;
	  y = binary_get_unsigned(is, neg);
	  return neg? -y : y;
      }

    template<typename Char>
      inline void
      binary_put_marker(std::basic_ostream<Char>& os) {
	  os.put((Char)fmt_traits<Char>::marker);
      }
    template<typename Char>
      inline void
      binary_skip_marker(std::basic_istream<Char>& is) {
	  typedef typename fmt_traits<Char>::value_type value_type;
	  if ((value_type)is.get() != fmt_traits<Char>::marker)
	      is.setstate(std::ios_base::failbit);
      }
    template<typename Char>
      inline bool
      binary_peek_marker(std::basic_istream<Char>& is) {
	  typedef typename fmt_traits<Char>::value_type value_type;
	  return (value_type)is.peek() == fmt_traits<Char>::marker;
      }
  }

  void binary_write(std::ostream& os, char x)
    { binary_put_signed(os, x); }
  void binary_read(std::istream& is, char& x)
    { x = binary_get_signed(is); }

  void binary_write(std::ostream& os, signed char x)
    { binary_put_signed(os, x); }
  void binary_write(std::ostream& os, unsigned char x)
    { binary_put_unsigned(os, x); }
  void binary_read(std::istream& is, signed char& x)
    { x = binary_get_signed(is); }
  void binary_read(std::istream& is, unsigned char& x)
    { x = binary_get_unsigned(is); }

  void binary_write(std::ostream& os, short x)
    { binary_put_signed(os, x); }
  void binary_write(std::ostream& os, unsigned short x)
    { binary_put_unsigned(os, x); }
  void binary_read(std::istream& is, short& x)
    { x = binary_get_signed(is); }
  void binary_read(std::istream& is, unsigned short& x)
    { x = binary_get_unsigned(is); }

  void binary_write(std::ostream& os, int x)
    { binary_put_signed(os, x); }
  void binary_write(std::ostream& os, unsigned int x)
    { binary_put_unsigned(os, x); }
  void binary_read(std::istream& is, int& x)
    { x = binary_get_signed(is); }
  void binary_read(std::istream& is, unsigned int& x)
    { x = binary_get_unsigned(is); }

  void binary_write(std::ostream& os, long x)
    { binary_put_signed(os, x); }
  void binary_write(std::ostream& os, unsigned long x)
    { binary_put_unsigned(os, x); }
  void binary_read(std::istream& is, long& x)
    { x = binary_get_signed(is); }
  void binary_read(std::istream& is, unsigned long& x)
    { x = binary_get_unsigned(is); }

#ifdef MORE_MORE_CONF_HAVE_LONG_LONG
  void binary_write(std::ostream& os, long long x)
    { binary_put_signed(os, x); }
  void binary_write(std::ostream& os, unsigned long long x)
    { binary_put_unsigned(os, x); }
  void binary_read(std::istream& is, long long& x)
    { x = binary_get_signed(is); }
  void binary_read(std::istream& is, unsigned long long& x)
    { x = binary_get_unsigned(is); }
#endif


  //  --- Float ---

  namespace {
    template<typename Char, typename T>
      inline void
      binary_write_normalized(std::basic_ostream<Char>& os,
			      T x_norm, int prec) {
	  typedef fmt_traits<Char> fmt;
	  typedef typename fmt::value_type value_type;
	  typedef typename fmt::bitno_type bitno_type;

	  value_type byte;
	  if (x_norm < 0.0) {
	      byte = fmt::sign_flag;
	      x_norm = -x_norm;
	  }
	  else
	      byte = 0;
	  T x_int;
	  x_norm = std::modf(x_norm*(1<<fmt::first_bitcount), &x_int);
	  byte |= (unsigned char)x_int;
	  while (os.good() && prec > 0 && x_norm != 0.0) {
	      os.put(byte | fmt::cnt_flag);
	      x_norm = std::modf(x_norm*(1<<fmt::other_bitcount), &x_int);
	      byte = (int)x_int;
	      prec -= fmt::other_bitcount;
	  }
	  os.put(byte);
      }

    template<typename Char, typename T>
      inline void
      binary_read_normalized(std::basic_istream<Char>& is,
			     T& x_norm, int n_exp = 0) {
	  typedef fmt_traits<Char> fmt;
	  typedef typename fmt::value_type value_type;
	  typedef typename fmt::bitno_type bitno_type;

	  bool minus = false;
	  value_type byte = is.get();
	  if (byte & fmt::sign_flag) minus = true;
	  x_norm = (byte & fmt::first_mask);
	  n_exp -= fmt::first_bitcount;
	  while (is.good() && (byte & fmt::cnt_flag)) {
	      byte = is.get();
	      x_norm *= (1<<fmt::other_bitcount);
	      x_norm += (byte & fmt::other_mask);
	      n_exp -= fmt::other_bitcount;
	  }
	  x_norm = ldexp(x_norm, n_exp);
	  if (minus) x_norm = -x_norm;
      }

    template<typename Char, typename T>
      inline void
      binary_write_fp(std::basic_ostream<Char>& os, T const& x, int prec) {
	  if (!math::is_finite(x)) {
	      binary_put_marker(os);
	      binary_put_signed(os, math::is_inf(x)-2);
	      return;
	  }
	  int n_exp;
	  T x_norm = std::frexp(x, &n_exp);
	  binary_put_signed(os, (big_int)n_exp);
	  binary_write_normalized(os, x_norm, prec);
      }

    template<typename Char, typename T>
      inline void
      binary_read_fp(std::basic_istream<Char>& is, T& x) {
	  if (binary_peek_marker(is)) {
	      binary_skip_marker(is);
	      switch (binary_get_signed(is)+2) {
		case -1: x = -std::numeric_limits<T>::infinity(); return;
		case 0:  x = std::numeric_limits<T>::quiet_NaN(); return;
		case 1:  x = std::numeric_limits<T>::infinity();  return;
	      }
	      // Or simply `return std::numeric_limits<T>::infinity()*
	      // (signed char)is.get()'?!  Or that may raise a signal.
	  }
	  int n_exp = binary_get_signed(is);
	  binary_read_normalized(is, x, n_exp);
      }
  }

  void binary_write(std::ostream& os, float const& x, int prec)
    { binary_write_fp(os, x, prec); }
  void binary_write(std::ostream& os, double const& x, int prec)
    { binary_write_fp(os, x, prec); }
  void binary_write(std::ostream& os, long double const& x, int prec)
    { binary_write_fp(os, x, prec); }

  void binary_read(std::istream& is, float& x)
    { binary_read_fp(is, x); }
  void binary_read(std::istream& is, double& x)
    { binary_read_fp(is, x); }
  void binary_read(std::istream& is, long double& x)
    { binary_read_fp(is, x); }


  //  --- String ---

  void binary_write(std::ostream& os, std::string s) {
      binary_put_marker(os);
      binary_put_unsigned(os, s.size());
      for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
	  if (os.fail()) break;
	  os.put(*it);
      }
  }
  void binary_read(std::istream& is, std::string& s) {
      binary_skip_marker(is);
      s.resize(binary_get_unsigned(is));
      for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
	  if (is.fail()) break;
	  *it = is.get();
      }
  }

  void binary_write_raw(std::ostream& os, const void* p, size_t sz) {
      binary_put_marker(os);
      binary_put_unsigned(os, sz);
      while (sz--) {
	  if (os.fail()) break;
	  os.put(*(char*)p);
	  p = (char *)p + 1;
      }
  }

  void binary_read_raw(std::istream& is, void* p, size_t sz) {
      binary_skip_marker(is);
      if (binary_get_unsigned(is) != sz) {
	  is.setstate(std::ios_base::failbit);
	  return;
      }
      while (sz--) {
	  if (is.fail()) break;
	  *(char*)p = is.get();
	  p = (char *)p + 1;
      }
  }

  size_t binary_read_raw_size(std::istream& is) {
      binary_skip_marker(is);
      return binary_get_unsigned(is);
  }

  void binary_read_raw_data(std::istream& is, void* p, size_t sz) {
      while (sz--) {
	  if (is.fail()) break;
	  *(char*)p = is.get();
	  p = (char *)p + 1;
      }
  }
   

  //  --- Dump ---

  void binary_dump(std::istream& is, std::ostream& os, bool dov) {
      bool const dot = !dov;
      char const* sep = " ";
      while (is.good()) {
	  if (binary_peek_marker(is)) {
	      int i;
	      binary_skip_marker(is);
	      binary_read(is, i);
	      if (i > 0) {
		  if (dot) {
		      os << 'd';
		      os << i;
		  }
		  if (dov) {
		      os << "\""
			 << std::oct << std::setfill('0');
		      while (is.good() && i--) {
			  char c = is.get();
			  if (std::isprint(c))
			      os << c;
			  else
			      os << '\\' << std::setw(3) << (int)c;
		      }
		      os << std::dec << "\"" << sep;
		  }
	      }
	      else if (-3 <= i && i <= -1) {
		  if (dot)
		      os << 'f';
		  if (dov) {
		      os << (i == -3? "-Inf" : i == -2? "NaN" : "Inf")
			 << sep;
		  }
	      }
	      else os << '!';
	  }
	  else {
	      big_int i;
	      binary_read(is, i);
	      if (is.eof()) break;
	      if (dot)
		  os << 'i';
	      if (dov)
		  os << i << sep;
	  }
      }
      os << std::endl;
  }
  //  arrays?

}} // namespace more::io
