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
#include <cstring>

#include <c++-gtk-utils/reassembler.h>
#include <c++-gtk-utils/convert.h>
#include <c++-gtk-utils/shared_handle.h>

using namespace Cgu;

extern "C" {
static void test_reassembler() {

  Utf8::Reassembler r;

  // ßëø[m with hook]
  wchar_t wtext[] = {0xdf, 0xeb, 0xd8, 0x1dac, 0};
  std::wstring wide = std::wstring(wtext);
  std::string u8 = Utf8::uniwide_to_utf8(wide);
  // string u8 holds 9 bytes of text.  The first three unicode
  // characters occupy 2 bytes each, and the last (m with hook) 3
  // bytes.

  std::string::size_type beg = 0;
  std::string::size_type end = 1;
  std::string sub = u8.substr(beg, end - beg);
  SharedHandle<char*> out = r(sub.data(), sub.length());
  g_assert_cmpuint(std::strlen(out), ==, 0);  // empty
  g_assert_cmpuint(r.get_stored(), ==, 1);    // first byte of ß

  beg = 1;
  end = 3;
  sub = u8.substr(beg, end - beg);
  out = r(sub.data(), sub.length());
  g_assert_cmpuint(std::strlen(out), ==, 2);  // ß
  g_assert_cmpuint(r.get_stored(), ==, 1);    // first byte of ë
  
  beg = 3;
  end = 8;
  sub = u8.substr(beg, end - beg);
  out = r(sub.data(), sub.length());
  g_assert_cmpuint(std::strlen(out), ==, 4);  // ëø
  g_assert_cmpuint(r.get_stored(), ==, 2);    // first two bytes of [m with hook]

  beg = 8;
  sub = u8.substr(beg);
  out = r(sub.data(), sub.length());
  g_assert_cmpuint(std::strlen(out), ==, 3);  // [m with hook]
  g_assert_cmpuint(r.get_stored(), ==, 0);
}
} // extern "C"


int main (int argc, char* argv[]) {
  setlocale(LC_ALL, "");
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/reassembler/reassembler", test_reassembler); 

  return g_test_run();
}
