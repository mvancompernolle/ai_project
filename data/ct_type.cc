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


#include <more/lang/ct_type.h>
#include <more/io/sourceref.h>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <gc_cpp.h>


#ifdef MORE_LANG_USE_GC
#  define MORE_LANG_NEW new(UseGC)
#else
#  define MORE_LANG_NEW new
#endif


namespace more {
namespace lang {

#ifdef MORE_LANG_PROVIDE_FFI
  template<typename T> struct ffi_type_of_helper {};
#define D(T, ffi)							\
  template<>								\
    struct ffi_type_of_helper<T>					\
    {									\
	inline static ffi_type* get() { return &ffi_type_##ffi; }	\
    }

  D(char,		sint8);
  D(signed char,	sint8);
  D(unsigned char,	uint8);

#if MORE_CONF_SIZEOF_SHORT == 2
  D(short,		sint16);
  D(unsigned short,	uint16);
#elif MORE_CONF_SIZEOF_SHORT == 4
  D(short,		sint32);
  D(unsigned short,	uint32);
#else
#  error Could not deduce ffi_type for short.
#  error Please, mail me the the config.h file in the top-level
#  error build directory and tell me your architecture.
#endif

#if MORE_CONF_SIZEOF_INT == 4
  D(int,		sint32);
  D(unsigned int,	uint32);
#elif MORE_CONF_SIZEOF_INT == 8
  D(int,		sint64);
  D(unsigned int,	uint64);
#else
#  error Could not deduce ffi_type for int.
#  error Please, mail me the the config.h file in the top-level
#  error build directory and tell me your architecture.
#endif

#if MORE_CONF_SIZEOF_LONG == 4
  D(long,		sint32);
  D(unsigned long,	uint32);
#elif MORE_CONF_SIZEOF_LONG == 8
  D(long,		sint64);
  D(unsigned long,	uint64);
#else
#  error Could not deduce ffi_type for long.
#  error Please, mail me the the config.h file in the top-level
#  error build directory and tell me your architecture.
#endif

#ifdef MORE_CONF_CXX_HAVE_LONG_LONG
#if MORE_CONF_SIZEOF_LONG_LONG == 8
  D(long long,		sint64);
  D(unsigned long long,	uint64);
#else
#  error Could not deduce ffi_type for long long.
#  error Please, mail me the the config.h file in the top-level
#  error build directory and tell me your architecture.
#endif
#endif

  D(float,		float);
  D(double,		double);
  D(long double,	longdouble);

  template<typename T>
    ffi_type* ffi_type_of() { return ffi_type_of_helper<T>::get(); }
#undef D
#endif // MORE_LANG_PROVIDE_FFI



  static bool const opt_check_allocations = false;

  template <typename T>
    size_t alignment_of()
    {
	struct test {
	    char pad;
	    T x;
	};
	return offsetof(test, x);
    }

  //
  //  ct_type
  //

  ct_type::ct_type()
#ifndef MORE_LANG_USE_FFI
      : m_size(size_type(-1)),
	m_is_const(0),
#else
      : m_is_const(0),
#endif
	m_is_volatile(0),
	m_cv_array(MORE_LANG_NEW ct_type*[4])
  {
      m_cv_array[0] = this;
      m_cv_array[1] = 0;
      m_cv_array[2] = 0;
      m_cv_array[3] = 0;
  }

#ifdef MORE_LANG_USE_FFI
  ct_type::ct_type(bool const_p, bool volatile_p)
      : m_is_const(const_p),
	m_is_volatile(volatile_p),
	m_cv_array(0)
  {}

#else
  ct_type::ct_type(size_type size, alignment_type alignment)
      : m_size(size),
	m_alignment(alignment),
	m_is_const(0),
	m_is_volatile(0),
	m_cv_array(MORE_LANG_NEW ct_type*[4])
  {
      m_cv_array[0] = this;
      m_cv_array[1] = 0;
      m_cv_array[2] = 0;
      m_cv_array[3] = 0;
  }

  ct_type::ct_type(size_type size, alignment_type alignment,
		   bool const_p, bool volatile_p)
      : m_size(size),
	m_alignment(alignment),
	m_is_const(const_p),
	m_is_volatile(volatile_p),
	m_cv_array(0)
  {}

#endif

  ct_type::~ct_type()
  {
#ifndef MORE_LANG_USE_GC
      int i = (const_p? 1 : 0) + (volatile_p? 2 : 0);
      m_cv_array[i] = 0;
      for (int j = 0; j < 4; ++j)
	  if (m_cv_array[j] != 0)
	      return;
      delete[] m_cv_array;
#endif
  }

  void
  ct_type::print_object(std::ostream& os, void* p) const
  {
      os << "#[";
      print_declaration(os, 0);
      os << " @" << p << "]";
//       throw std::logic_error("ct_type::print_object: "
//                         "Can not create source.");
  }

//XXX    gc_string
//    ct_type::print_as_expr(void* data, coder& cr) const
//    {
//        std::ostringstream oss;
//        print_object(oss, data);
//        return oss.str().c_str();
//    }

  void
  ct_type::print_declaration(std::ostream& os, printopt_type opt,
			     char const* var) const
  {
      if (var) {
	  print_declaration_pre(os, opt | printopt_space_after_name);
	  os << var;
      }
      else
	  print_declaration_pre(os, opt);
      print_declaration_post(os, opt);
  }

  void
  ct_type::print_declaration_cv(std::ostream& os, printopt_type opt) const
  {
      if (is_const() || is_volatile()) {
	  if (opt & printopt_space_before_name)
	      os << ' ';
	  if (is_const()) {
	      if (is_volatile())
		  os << "const volatile";
	      else
		  os << "const";
	  }
	  else
	      os << "volatile";
	  if (opt & printopt_space_after_name)
	      os << ' ';
      }
  }

  char const*		ct_type::s_private_pfx = "_p_";

  bool	ct_type::is_builtin() const { return false; }
  bool	ct_type::is_number() const { return false; }

  // Properties according to numeric_limits
  // XXX rt_value* ct_type::min() const
  // XXX rt_value* ct_type::max() const
  int	ct_type::digits() const { return 0; }
  int	ct_type::digits10() const { return 0; }
  bool	ct_type::is_integer() const { return false; }
  bool	ct_type::is_exact() const { return false; }
  int	ct_type::radix() const { return false; }
  // rt_value* ct_type::epsilon() const
  // rt_value* ct_type::round_error() const
  int	ct_type::min_exponent() const { return 0; }
  int	ct_type::min_exponent10() const { return 0; }
  int	ct_type::max_exponent() const { return 0; }
  int	ct_type::max_exponent10() const { return 0; }
  bool	ct_type::has_infinity() const { return false; }
  bool	ct_type::has_quiet_NaN() const { return false; }
  bool	ct_type::has_signaling_NaN() const { return false; }
  std::float_denorm_style
    ct_type::has_denorm() const { return std::denorm_absent; }
  bool	ct_type::has_denorm_loss() const { return false; }
  // T ct_type::infinity() const
  // T ct_type::quiet_NaN() const
  // T ct_type::signaling_NaN() const
  // T ct_type::denorm_min() const
  bool	ct_type::is_iec559() const { return false; }
  bool	ct_type::is_bounded() const { return false; }
  bool	ct_type::is_modulo() const { return false; }
  bool	ct_type::traps() const { return false; }
  bool	ct_type::tinyness_before() const { return false; }
  std::float_round_style
    ct_type::round_style() const { return std::round_toward_zero; }

  ct_type const* ct_type::promoted() const { return 0; }

  ct_type const*
  to_cv(ct_type const* rtt, bool const_p, bool volatile_p)
  {
      int i = (const_p? 1 : 0) + (volatile_p? 2 : 0);
      ct_type** cv_arr = rtt->m_cv_array;
      if (!cv_arr) {
	  if (rtt->is_const() == const_p &&
	      rtt->is_volatile() == volatile_p)
	      return rtt;
	  throw std::logic_error("more::lang::to_cv: "
				 "Can to change CV qualifiers for this type.");
      }
      if (cv_arr[i] == 0) {
	  ct_type* rtt_new = rtt->clone();
	  rtt_new->m_cv_array = cv_arr;
	  cv_arr[i] = rtt_new;
	  rtt_new->m_is_const = const_p;
	  rtt_new->m_is_volatile = volatile_p;
	  return rtt_new;
      }
      else
	  return cv_arr[i];
  }


  //
  //  ct_pointer
  //

#ifdef MORE_LANG_USE_FFI
  ct_pointer::ct_pointer(ct_type const* t)
      : m_deref_type(t) {}
#else
  ct_pointer::ct_pointer(ct_type const* t)
      : ct_type(sizeof(void*),
		alignment_of<void*>()),
	m_deref_type(t) {}
#endif

  ct_pointer::~ct_pointer() {}

  ct_pointer*
  ct_pointer::clone() const
  {
      return MORE_LANG_NEW ct_pointer(*this);
  }

#ifdef MORE_LANG_PROVIDE_FFI
  ffi_type*
  ct_pointer::ffi_type_of() const { return &ffi_type_pointer; }
#endif

  void
  ct_pointer::construct_copy(void* p0, void* p1) const
  {
      *(void**)p0 = *(void**)p1;
  }

  void
  ct_pointer::destruct(void*) const {}

  bool
  ct_pointer::equal(void* p0, void* p1) const
  {
      return *(void**)p0 == *(void**)p1;
  }

  void
  ct_pointer::print_object(std::ostream& os, void* ptr) const
  {
      std::ios_base::fmtflags base = os.flags();
      os.setf(std::ios_base::hex, std::ios_base::basefield);
      os << '@' << *(void**)ptr;
      os.setf(base, std::ios_base::basefield);
  }

  void
  ct_pointer::print_declaration_pre(std::ostream& os, printopt_type opt) const
  {
      opt |= printopt_low_precedence;
      opt |= printopt_space_after_name;
      deref_type()->print_declaration_pre(os, opt);
      os << "*";
      print_declaration_cv(os, opt & ~printopt_space_before_name);
  }

  void
  ct_pointer::print_declaration_post(std::ostream& os, printopt_type opt) const
  {
      opt |= printopt_low_precedence;
      deref_type()->print_declaration_post(os, opt);
  }

  void
  ct_pointer::hash(hash_type& h) const
  {
      h.insert(math::hash_id32('l', 'r', 't', 'p', 't', 'r'));
      deref_type()->hash(h);
  }

  bool
  ct_pointer::equal_to(ct_type const* rhs0) const
  {
      if (ct_pointer const* rhs = dynamic_cast<ct_pointer const*>(rhs0))
	  return is_const() == rhs->is_const()
	      && is_volatile() == rhs->is_volatile()
	      && deref_type()->equal_to(rhs->deref_type());
      else
	  return false;
  }


  //
  //  ct_array
  //

#ifdef MORE_LANG_USE_FFI
  ct_array::ct_array(ct_type const* t, std::size_t count)
      : m_element_type(t),
	m_count(count)
#else
  ct_array::ct_array(ct_type const* t, std::size_t count)
      : ct_type(t->size()*count, t->alignment()*count),
	m_element_type(t),
	m_count(count)
#endif
  {
      // The ffi_type created here may not be usable for a function
      // call, as arrays are not defined by libffi, but ct_struct can
      // still use it to calculate member offsets.
#ifdef MORE_LANG_PROVIDE_FFI
      ffi_type* elt_ffi = t->ffi_type_of();
      m_ffi_type.size = elt_ffi->size*m_count;
      m_ffi_type.alignment = elt_ffi->alignment;
      m_ffi_type.type = 0; // XXX There is no id for arrays
      m_ffi_type.elements = 0;
#endif
  }

  ct_array::~ct_array() {}

  ct_array*
  ct_array::clone() const
  {
      return MORE_LANG_NEW ct_array(*this);
  }

#ifdef MORE_LANG_PROVIDE_FFI
  ffi_type*
  ct_array::ffi_type_of() const { return &m_ffi_type; }
#endif

  void
  ct_array::construct_copy(void* dest, void* src) const
  {
      std::size_t elt_size = element_type()->size();
      for (std::size_t i = 0; i < m_count; ++i) {
	  element_type()->construct_copy(dest, src);
	  dest = (char *)dest + elt_size;
	  src = (char *)src + elt_size;
      }
  }

  void
  ct_array::destruct(void* p) const
  {
      std::size_t elt_size = element_type()->size();
      std::size_t i = m_count;
      p = (char *)p + elt_size;
      while (i) {
	  p = (char *)p - elt_size;
	  element_type()->destruct(p);
	  --i;
      }
  }

  bool
  ct_array::equal(void* p0, void* p1) const
  {
      //XXX
      return p0 == p1;
  }

  void
  ct_array::print_object(std::ostream& os, void* ptr) const
  {
      os << '@' << ptr << '[' << m_count << ']';
  }

  void
  ct_array::print_declaration_pre(std::ostream& os, printopt_type opt) const
  {
      // CV qualifiers should be on the element type
      if (opt & printopt_low_precedence) {
	  opt &= ~printopt_low_precedence;
	  element_type()->print_declaration_pre(os, opt);
	  os << "(";
      }
      else
	  element_type()->print_declaration_pre(os, opt);
  }

  void
  ct_array::print_declaration_post(std::ostream& os, printopt_type opt) const
  {
      if (opt & printopt_low_precedence) {
	  opt &= ~printopt_low_precedence;
	  os << ")";
      }
      os << '[' << m_count << ']';
      element_type()->print_declaration_post(os, opt);
  }

  void
  ct_array::hash(hash_type& h) const
  {
      h.insert(math::hash_id32('l', 'r', 't', 'a', 'r', 'r'));
      h.insert(m_count);
      element_type()->hash(h);
  }

  bool
  ct_array::equal_to(ct_type const* rhs0) const
  {
      if (ct_array const* rhs = dynamic_cast<ct_array const*>(rhs0))
	  return is_const() == rhs->is_const()
	      && is_volatile() == rhs->is_volatile()
	      && m_count == rhs->m_count
	      && m_element_type->equal_to(rhs->m_element_type);
      else
	  return false;
  }



  //--------------------------------------------------------------------
  //			      ct_builtin
  //--------------------------------------------------------------------

  template<typename T> char const* c_value_string(void*);

  template<typename T>
    struct ct_builtin : ct_type
    {
#ifdef MORE_LANG_USE_FFI
	explicit ct_builtin(char const* c_name)
	    : m_c_name(c_name) {}
	ct_builtin(char const* c_name, alignment_type alignment)
	    : m_c_name(c_name) {}
#else
	explicit ct_builtin(char const* c_name)
	    : ct_type((size_type)sizeof(T),
		      (alignment_type)alignment_of<T>()),
	      m_c_name(c_name) {}
	ct_builtin(char const* c_name, alignment_type alignment)
	    : ct_type((size_type)sizeof(T),
		      alignment),
	      m_c_name(c_name) {}
#endif

	virtual ct_builtin* clone() const
	{
	    return MORE_LANG_NEW ct_builtin(*this);
	}

#ifdef MORE_LANG_PROVIDE_FFI
	virtual ffi_type* ffi_type_of() const;
#endif
#if 0
  	virtual void construct(void* p) const { ::new(p) T; }
#endif
	virtual void construct_copy(void* p, void* src) const
	{
	    ::new(p) T(*static_cast<T*>(src));
	}
	virtual void destruct(void* p) const
	{
	    static_cast<T*>(p)->~T();
	}
	virtual bool equal(void* p0, void* p1) const
	{
	    return *static_cast<T*>(p0) == *static_cast<T*>(p1);
	}
	virtual void print_object_source(std::ostream& os, void* p) const
	{
	    os << more::lang::c_value_string<T>(p);
	}

	virtual void
	print_declaration_pre(std::ostream& os, printopt_type opt) const
	{
	    if (opt & printopt_space_before_name)
		os << ' ';
	    os << m_c_name;
	    print_declaration_cv(os,
				 (opt & ~printopt_space_after_name)
				 | printopt_space_before_name);
	    if (opt & printopt_space_after_name)
		os << ' ';
	}

	virtual void
	print_declaration_post(std::ostream& os, printopt_type opt) const
	{
	}

	virtual void hash(hash_type& h) const
	{
	    h.insert(m_c_name, m_c_name + std::strlen(m_c_name) + 1);
	}

	virtual bool equal_to(ct_type const* rhs0) const
	{
	    return this == rhs0;
	}

	virtual bool	is_builtin() const { return true; }
	virtual bool	is_number() const
	{ return std::numeric_limits<T>::is_specialized; }

	//
	// numeric_limits
	//
	// XXX rt_value* min() const
	// XXX rt_value* max() const
	virtual int	digits() const
	{ return std::numeric_limits<T>::digits; }
	virtual int	digits10() const
	{ return std::numeric_limits<T>::digits10; }
	virtual bool	is_integer() const
	{ return std::numeric_limits<T>::is_integer; }
	virtual bool	is_exact() const
	{ return std::numeric_limits<T>::is_exact; }
	virtual int	radix() const
	{ return std::numeric_limits<T>::radix; }
	// rt_value* epsilon() const
	// rt_value* round_error() const
	virtual int	min_exponent() const
	{ return std::numeric_limits<T>::min_exponent; }
	virtual int	min_exponent10() const
	{ return std::numeric_limits<T>::min_exponent10; }
	virtual int	max_exponent() const
	{ return std::numeric_limits<T>::max_exponent; }
	virtual int	max_exponent10() const
	{ return std::numeric_limits<T>::max_exponent10; }
	virtual bool	has_infinity() const
	{ return std::numeric_limits<T>::has_infinity; }
	virtual bool	has_quiet_NaN() const
	{ return std::numeric_limits<T>::has_quiet_NaN; }
	virtual bool	has_signaling_NaN() const
	{ return std::numeric_limits<T>::has_signaling_NaN; }
	virtual std::float_denorm_style has_denorm() const
	{ return std::numeric_limits<T>::has_denorm; }
	virtual bool	has_denorm_loss() const
	{ return std::numeric_limits<T>::has_denorm_loss; }
	// T infinity() const
	// T quiet_NaN() const
	// T signaling_NaN() const
	// T denorm_min() const
	virtual bool	is_iec559() const
	{ return std::numeric_limits<T>::is_iec559; }
	virtual bool	is_bounded() const
	{ return std::numeric_limits<T>::is_bounded; }
	virtual bool	is_modulo() const
	{ return std::numeric_limits<T>::is_modulo; }
	virtual bool	traps() const
	{ return std::numeric_limits<T>::traps; }
	virtual bool	tinyness_before() const
	{ return std::numeric_limits<T>::tinyness_before; }
	virtual std::float_round_style round_style() const
	{ return std::numeric_limits<T>::round_style; }

      private:
	char const* m_c_name;
    };

  // A builtin type which may be promoted.  (Arithmetic type.)
  template <typename T>
    struct ct_builtin_wp
	: ct_builtin<T>
    {
	explicit ct_builtin_wp(char const* c_name)
	    : ct_builtin<T>(c_name) {}

	virtual ct_builtin_wp* clone() const
	{
	    return MORE_LANG_NEW ct_builtin_wp(*this);
	}

	virtual ct_type const* promoted() const
	{
	    T x = 0;
	    return ct_type_of(x + x);
	}
    };

#define DEF_t(T)					\
    ct_type*						\
    _ct_type<T>::get()					\
    {							\
	static ct_builtin_wp<T>* c = 0;			\
	if (c == 0) {					\
	    if (opt_check_allocations)			\
		c = MORE_LANG_NEW ct_builtin_wp<T>(#T);	\
	    else					\
		c = new ct_builtin_wp<T>(#T);		\
	}						\
	return c;					\
    }

#ifdef MORE_LANG_PROVIDE_FFI
#define DEF_tf(T)				\
    DEF_t(T)					\
    template<>					\
      ffi_type*					\
      ct_builtin<T>::ffi_type_of() const	\
      {						\
	  return lang::ffi_type_of<T>();	\
      }
#else
#define DEF_tf(T) \
    DEF_t(T)
#endif


#define DEF_v(T)						\
    template<>							\
      char const*						\
      c_value_string<T>(void* x)				\
      {								\
	  std::ostringstream oss;				\
	  oss << *static_cast<T*>(x);				\
	  char* c_str = static_cast<char*>(			\
	      GC_malloc(sizeof(char)*(oss.str().size() + 1)));	\
	  std::string str = oss.str();				\
	  std::copy(str.begin(), str.end(), c_str);		\
	  return c_str;						\
      }

#define DEF_tfv(T) DEF_tf(T) DEF_v(T)

#define DEF_tf_nov(T)							\
    DEF_tf(T)								\
    template<>								\
      char const*							\
      c_value_string<T>(void* x)					\
      {									\
	  throw std::logic_error("more::lang::c_value_string: Not "	\
				 "implemented for this ct_type.");	\
	  return 0;							\
      }

  //
  // ct_builtin_wp: simple C types
  //

//XXX  DEF_tfv(bool);
  DEF_tfv(char)
  DEF_tfv(signed char)
  DEF_tfv(unsigned char)
  DEF_tfv(short)
  DEF_tfv(unsigned short)
  DEF_tfv(int)
  DEF_tfv(unsigned int)
  DEF_tfv(long)
  DEF_tfv(unsigned long)
#ifdef MORE_CONF_CXX_HAVE_LONG_LONG
  DEF_tfv(long long)
  DEF_tfv(unsigned long long)
#endif
  DEF_tfv(float)
  DEF_tfv(double)
  DEF_tfv(long double)


  //
  // ct_builtin_wp: gc_wstring and gc_string
  //

#ifdef MORE_LANG_USE_GC
// alignment_of<double> is a guess, a good one I believe, of the
// worst-case alignment, used for these non-PODs to avoid invalid
// offsetof usages.
#define D(T)								\
  ct_type*								\
  _ct_type<T>::get()							\
  {									\
      static ct_type* t = 0;						\
      if (!t)								\
	  t = MORE_LANG_NEW ct_builtin<T>(#T, alignment_of<double>());	\
      return t;								\
  }
  D(gc_string)
  D(gc_wstring)
  D(identifier)
  D(io::sourceref)
#undef D

#ifdef MORE_LANG_PROVIDE_FFI
  template<>
    ffi_type*
    ct_builtin<gc_string>::ffi_type_of() const { return 0; } // XXX
  template<>
    ffi_type*
    ct_builtin<gc_wstring>::ffi_type_of() const { return 0; } // XXX
  template<>
    ffi_type*
    ct_builtin<identifier>::ffi_type_of() const { return 0; } // XXX
  template<>
    ffi_type*
    ct_builtin<io::sourceref>::ffi_type_of() const { return 0; } // XXX
#endif

//   template<>
//     char const*
//     c_value_string<bool>(void* p)
//     {
// 	if (*(bool*)p)
// 	    return "1";
// 	else
// 	    return "0";
//     }
  template<>
    char const*
    c_value_string<gc_string>(void* p)
    {
	gc_string str("more::gen::gc_string(\"");
	str += *static_cast<gc_string*>(p);
	str += "\")";
	return str.c_str();
    }
  template<>
    char const*
    c_value_string<gc_wstring>(void* p)
    {
	gc_string str("more::gen::gc_wstring(L\"");
	str += gen::wcstombs_gc(static_cast<gc_wstring*>(p)->c_str());
	str += "\")";
	return str.c_str();
    }
  template<>
    char const*
    c_value_string<identifier>(void* p)
    {
	gc_string str("more::gen::identifier(\"");
	str += static_cast<gc_string*>(p)->c_str();
	str += "\")";
	return str.c_str();
    }
  template<>
    char const*
    c_value_string<io::sourceref>(void* p)
    {
	gc_string str("more::io::sourceref(");
	io::sourceref& ref = *static_cast<io::sourceref*>(p);
	if (!ref.is_unknown()) {
	    if (ref.is_stdin())
		str += "more::io::sourceref::stdin";
	    else
		str += ref.file_name().c_str();
	    str += ", ";
	    str += ref.line();
	    str += ", ";
	    str += ref.column();
	}
	str += ")";
	return str.c_str();
    }
#endif

  //
  // ct_type_void
  //
  struct ct_type_void : ct_type
  {
#ifdef MORE_LANG_USE_FFI
      ct_type_void() {}
#else
      ct_type_void()
	  : ct_type((size_type)0, (alignment_type)0) {}
#endif
      ct_type_void* clone() const { return MORE_LANG_NEW ct_type_void(*this); }

      virtual void construct_copy(void*, void*) const {}
      virtual void destruct(void*) const {}
      virtual bool equal(void*, void*) const { return true; }

      virtual void
      print_declaration_pre(std::ostream& os, printopt_type opt) const
      {
	  os << "void";
	  if (opt & printopt_space_after_name)
	      os << ' ';
      }
      virtual void
      print_declaration_post(std::ostream& os, printopt_type opt) const
      {
      }

      virtual void hash(hash_type& h) const
      {
	  h.insert(0x729f0e13);
      }
      virtual bool equal_to(ct_type const* rhs) const
      {
	  return dynamic_cast<ct_type_void const*>(rhs) != 0;
      }

#ifdef MORE_LANG_PROVIDE_FFI
      virtual ffi_type*
      ffi_type_of() const
      {
	  return &ffi_type_void;
      }
#endif

      virtual void
      print_object_source(std::ostream& os, void* p)
      {
	  os << "((void)0)";
      }

  };

  ct_type*
  _ct_type<void>::get()
  {
      static ct_type_void* c = 0;
      if (c == 0) {
	  if (opt_check_allocations)
	      c = MORE_LANG_NEW ct_type_void;
	  else
	      c = new ct_type_void;
      }
      return c;
  }

  template<>
    char const*
    c_value_string<void>(void*)
    {
	return "((void)0)";
    }


  //--------------------------------------------------------------------
  //			      Algorithms
  //--------------------------------------------------------------------


  ct_type const*
  arithmetic_result_type(ct_type const* x, ct_type const* y)
  {
      if (!x->is_builtin())
	  return 0;
      if (!y->is_builtin())
	  return 0;
      x = x->promoted();
      y = y->promoted();
      if (x == y)
	  return x;
      if (x->is_integer()) {
	  if (!y->is_integer())
	      return y;
      }
      else if (y->is_integer())
	  return x;
      if (x->digits() > y->digits())
	  return x;
      else
	  return y;
  }

}}

