/* Copyright (C) 2012 Chris Vine

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
   which came with this source code package in the c++-gtk-utils
   sub-directory); if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <glib.h>
#include <locale.h>
#include <string>

#include <c++-gtk-utils/convert.h>

using namespace Cgu;


extern "C" {
static void test_conversions() {
  // ßëø[m with hook]
  wchar_t wtext[] = {0xdf, 0xeb, 0xd8, 0x1dac, 0};
  std::wstring wide_in = std::wstring(wtext);
  std::string u8_in = Utf8::uniwide_to_utf8(wide_in);
  g_assert_cmpuint(((unsigned int)Utf8::validate(u8_in)), !=, 0);
  std::wstring wide_out = Utf8::uniwide_from_utf8(u8_in);
  g_assert(wide_in == wide_out);
  
  std::string tmp = Utf8::locale_from_utf8(u8_in);
  std::string u8_out =  Utf8::locale_to_utf8(tmp);
  g_assert(u8_in == u8_out);

  tmp = Utf8::filename_from_utf8(u8_in);
  u8_out = Utf8::filename_to_utf8(tmp);
  g_assert(u8_in == u8_out);

  char16_t u16text[] = {0xdf, 0xeb, 0xd8, 0x1dac, 0};
  std::u16string u16_in = std::u16string(u16text);
  tmp = Utf8::utf16_to_utf8(u16_in);
  std::u16string u16_out = Utf8::utf16_from_utf8(tmp);
  g_assert(u16_in == u16_out);

  char32_t u32text[] = {0xdf, 0xeb, 0xd8, 0x1dac, 0};
  std::u32string u32_in = std::u32string(u32text);
  tmp = Utf8::utf32_to_utf8(u32_in);
  std::u32string u32_out = Utf8::utf32_from_utf8(tmp);
  g_assert(u32_in == u32_out);
}

static void test_iterators() {
  wchar_t wtext[] = {0xdf, 0xeb, 0xd8, 0x1dac, 0};
  std::string narrow = Utf8::uniwide_to_utf8(std::wstring(wtext));

  Utf8::Iterator iter;
  int count = 0;
  for (iter = narrow.begin();
       iter != narrow.end();
       ++count, ++iter) {
    // since all wide characters tested here are in the BMP and <
    // 0xf000, we are OK with this cast even if wchar_t is 16 bits
    // wide and signed, C++11 §4.7/3: 'If the destination type is
    // signed, the value is unchanged if it can be represented in the
    // destination type (and bit-field width)'
    g_assert(static_cast<wchar_t>(*(iter++)) == wtext[count]);
    iter--;
    ++iter;
    g_assert(static_cast<wchar_t>(*(--iter)) == wtext[count]);
  }
  
  Utf8::ReverseIterator r_iter;
  count = 3;
  for (r_iter = narrow.rbegin();
       r_iter != narrow.rend();
       --count, ++r_iter) {
    // see above as regards the cast
    g_assert(static_cast<wchar_t>(*(r_iter++)) == wtext[count]);
    r_iter--;
    ++r_iter;
    g_assert(static_cast<wchar_t>(*(--r_iter)) == wtext[count]);
  }
}

} // extern "C"


int main (int argc, char* argv[]) {
  setlocale(LC_ALL, "");
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/convert/conversions", test_conversions); 
  g_test_add_func("/convert/iterators", test_iterators); 

  return g_test_run();
}
