/* Copyright (C) 2005, 2009, 2011 and 2014 Chris Vine

The library comprised in this file or of which this file is part is
distributed by Chris Vine under the GNU Lesser General Public
License as follows:

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License, version 2.1, for more details.

   You should have received a copy of the GNU Lesser General Public
   License, version 2.1, along with this library (see the file LGPL.TXT
   which came with this source code package in the src/utils sub-directory);
   if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include <c++-gtk-utils/lib_defs.h>

#include <c++-gtk-utils/convert.h>
#include <c++-gtk-utils/shared_handle.h>
#include <c++-gtk-utils/gerror_handle.h>

namespace Cgu {

namespace Utf8 {

std::wstring uniwide_from_utf8(const std::string& input) {
  GError* error_p = 0;
  glong written = 0;
#if SIZEOF_WCHAR_T == 2
  if (sizeof(gunichar2) != 2) { // let the optimiser elide this block if it cannot be reached
    throw ConversionError{"wchar_t and gunichar2 are of different sizes "
	                  "(char type used with UTF-8 may not be an octet)"};
  }
  ScopedHandle<gunichar2*, GFree> result_h{g_utf8_to_utf16(input.data(), input.size(), 0,
							   &written, &error_p)};
#elif SIZEOF_WCHAR_T == 4
  if (sizeof(gunichar) != 4) { // let the optimiser elide this block if it cannot be reached
    throw ConversionError{"wchar_t and gunichar are of different sizes "
	                  "(char type used with UTF-8 may not be an octet)"};
  }
  ScopedHandle<gunichar*, GFree> result_h{g_utf8_to_ucs4(input.data(), input.size(), 0,
							 &written, &error_p)};
#else
  throw ConversionError{"char type used with UTF-8 is not an octet"};
  ScopedHandle<gunichar*, GFree> result_h; // dummy value to prevent compile error
#endif
  if (error_p) {
    GerrorScopedHandle handle_h{error_p};
    throw ConversionError{error_p};
  }
  // This might appear to break the strict aliasing rule in §3.10/10
  // of the C++11 standard because wchar_t is a distinct type from its
  // underlying integral representation (§3.9.1/5).  However, where as
  // here the aliased object (the wide character string) is returned
  // by a function in another compilation unit (g_utf8_to_utf16() or
  // g_utf8_to_ucs4()) and has the same value representation as an
  // array of wchar_t, and its pointer is not modified or dereferenced
  // in this function but only cast to wchar_t*, the compiler cannot
  // assume the dynamic type is not wchar_t and so optimize against it
  // when std::wstring's constructor dereferences it, even though that
  // constructor is in a template class and so inline and visible.
  return std::wstring(reinterpret_cast<wchar_t*>(result_h.get()), written);
}

std::string uniwide_to_utf8(const std::wstring& input) {
  GError* error_p = 0;
  glong written = 0;
#if SIZEOF_WCHAR_T == 2
  if (sizeof(gunichar2) != 2) { // let the optimiser elide this block if it cannot be reached
    throw ConversionError{"wchar_t and gunichar2 are of different sizes "
	                  "(char type used with UTF-8 may not be an octet)"};
  }
  GcharScopedHandle result_h{g_utf16_to_utf8(reinterpret_cast<const gunichar2*>(input.data()),
					     input.size(), 0,
					     &written, &error_p)};
#elif SIZEOF_WCHAR_T == 4
  if (sizeof(gunichar) != 4) { // let the optimiser elide this block if it cannot be reached
    throw ConversionError{"wchar_t and gunichar are of different sizes "
	                  "(char type used with UTF-8 may not be an octet)"};
  }
  GcharScopedHandle result_h{g_ucs4_to_utf8(reinterpret_cast<const gunichar*>(input.data()),
					    input.size(), 0,
					    &written, &error_p)};
#else
  throw ConversionError{"char type used with UTF-8 is not an octet"};
  GcharScopedHandle result_h; // dummy value to prevent compile error
#endif
  if (error_p) {
    GerrorScopedHandle handle_h{error_p};
    throw ConversionError{error_p};
  }
  return std::string(result_h.get(), written);
}

std::u32string utf32_from_utf8(const std::string& input) {

  if (sizeof(gunichar) != sizeof(char32_t)) { // let the optimiser elide this block if it cannot be reached
    throw ConversionError{"char32_t and gunichar are of different sizes!"};
  }

  GError* error_p = 0;
  glong written = 0;
  ScopedHandle<gunichar*, GFree> result_h{g_utf8_to_ucs4(input.data(), input.size(), 0,
							 &written, &error_p)};
  if (error_p) {
    GerrorScopedHandle handle_h{error_p};
    throw ConversionError{error_p};
  }
  // This might appear to break the strict aliasing rule in §3.10/10
  // of the C++11 standard because char32_t is a distinct type from
  // its underlying integral representation (§3.9.1/5).  However,
  // where as here the aliased object (the wide character string) is
  // returned by a function in another compilation unit
  // (g_utf8_to_ucs4()) and has the same value representation as an
  // array of char32_t, and its pointer is not modified or
  // dereferenced in this function but only cast to char32_t*, the
  // compiler cannot assume the dynamic type is not char32_t and so
  // optimize against it when std::u32string's constructor
  // dereferences it, even though that constructor is in a template
  // class and so inline and visible.
  return std::u32string(reinterpret_cast<char32_t*>(result_h.get()), written);
}

std::string utf32_to_utf8(const std::u32string& input) {

  if (sizeof(gunichar) != sizeof(char32_t)) { // let the optimiser elide this block if it cannot be reached
    throw ConversionError{"char32_t and gunichar are of different sizes!"};
  }

  GError* error_p = 0;
  glong written = 0;
  GcharScopedHandle result_h{g_ucs4_to_utf8(reinterpret_cast<const gunichar*>(input.data()),
					    input.size(), 0,
					    &written, &error_p)};
  if (error_p) {
    GerrorScopedHandle handle_h{error_p};
    throw ConversionError{error_p};
  }
  return std::string(result_h.get(), written);
}

std::u16string utf16_from_utf8(const std::string& input) {

  if (sizeof(gunichar2) != sizeof(char16_t)) { // let the optimiser elide this block if it cannot be reached
    throw ConversionError{"char16_t and gunichar2 are of different sizes!"};
  }

  GError* error_p = 0;
  glong written = 0;
  ScopedHandle<gunichar2*, GFree> result_h{g_utf8_to_utf16(input.data(), input.size(), 0,
							   &written, &error_p)};
  if (error_p) {
    GerrorScopedHandle handle_h{error_p};
    throw ConversionError{error_p};
  }
  // This might appear to break the strict aliasing rule in §3.10/10
  // of the C++11 standard because char16_t is a distinct type from
  // its underlying integral representation (§3.9.1/5).  However,
  // where as here the aliased object (the wide character string) is
  // returned by a function in another compilation unit
  // (g_utf8_to_utf16()) and has the same value representation as an
  // array of char16_t, and its pointer is not modified or
  // dereferenced in this function but only cast to char16_t*, the
  // compiler cannot assume the dynamic type is not char16_t and so
  // optimize against it when std::u16string's constructor
  // dereferences it, even though that constructor is in a template
  // class and so inline and visible.
  return std::u16string(reinterpret_cast<char16_t*>(result_h.get()), written);
}

std::string utf16_to_utf8(const std::u16string& input) {

  if (sizeof(gunichar2) != sizeof(char16_t)) { // let the optimiser elide this block if it cannot be reached
    throw ConversionError{"char16_t and gunichar2 are of different sizes!"};
  }

  GError* error_p = 0;
  glong written = 0;
  GcharScopedHandle result_h{g_utf16_to_utf8(reinterpret_cast<const gunichar2*>(input.data()),
					     input.size(), 0,
					     &written, &error_p)};
  if (error_p) {
    GerrorScopedHandle handle_h{error_p};
    throw ConversionError{error_p};
  }
  return std::string(result_h.get(), written);
}

std::wstring wide_from_utf8(const std::string& input) {
  GError* error_p = 0;
  gsize written = 0;
  GcharScopedHandle result_h{g_convert(input.data(), input.size(),
				       "WCHAR_T", "UTF-8",
				       0, &written, &error_p)};
  if (error_p) {
    GerrorScopedHandle handle_h{error_p};
    throw ConversionError{error_p};
  }
  // This might appear to break the strict aliasing rule in §3.10/10
  // of the C++11 standard.  However, where as here the aliased object
  // (the wide character string) is returned by char* by a function in
  // another compilation unit (g_convert()) and has the value
  // representation of an array of wchar_t, and its pointer is not
  // modified or dereferenced in this function but only cast to
  // wchar_t*, the compiler cannot assume the dynamic type is not
  // wchar_t and so optimize against it when std::wstring's
  // constructor dereferences it, even though that constructor is in a
  // template class and so inline and visible.
  return std::wstring(reinterpret_cast<wchar_t*>(result_h.get()), written/sizeof(wchar_t));
}

std::string wide_to_utf8(const std::wstring& input) {
  GError* error_p = 0;
  gsize written = 0;
  GcharScopedHandle result_h{g_convert(reinterpret_cast<const gchar*>(input.data()),
				       input.size() * sizeof(wchar_t),
				       "UTF-8", "WCHAR_T",
				       0, &written, &error_p)};
  if (error_p) {
    GerrorScopedHandle handle_h{error_p};
    throw ConversionError{error_p};
  }
  return std::string(result_h.get(), written);
}

std::string filename_from_utf8(const std::string& input) {
  GError* error_p = 0;
  gsize written = 0;
  GcharScopedHandle result_h{g_filename_from_utf8(input.data(), input.size(), 0,
						  &written, &error_p)};
    
  if (error_p) {
    GerrorScopedHandle handle_h{error_p};
    throw ConversionError{error_p};
  }
  return std::string(result_h.get(), written);
}

std::string filename_to_utf8(const std::string& input) {
  GError* error_p = 0;
  gsize written = 0;
  GcharScopedHandle result_h{g_filename_to_utf8(input.data(), input.size(), 0,
						&written, &error_p)};
    
  if (error_p) {
    GerrorScopedHandle handle_h{error_p};
    throw ConversionError{error_p};
  }
  return std::string(result_h.get(), written);
}

std::string locale_from_utf8(const std::string& input) {
  GError* error_p = 0;
  gsize written = 0;
  GcharScopedHandle result_h{g_locale_from_utf8(input.data(), input.size(), 0,
						&written, &error_p)};
    
  if (error_p) {
    GerrorScopedHandle handle_h{error_p};
    throw ConversionError{error_p};
  }
  return std::string(result_h.get(), written);
}

std::string locale_to_utf8(const std::string& input) {
  GError* error_p = 0;
  gsize written = 0;
  GcharScopedHandle result_h{g_locale_to_utf8(input.data(), input.size(), 0,
					      &written, &error_p)};
    
  if (error_p) {
    GerrorScopedHandle handle_h{error_p};
    throw ConversionError{error_p};
  }
  return std::string(result_h.get(), written);
}

} // namespace Utf8

} // namespace Cgu
