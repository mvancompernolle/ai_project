/* Copyright (C) 2005 Chris Vine

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

#include <c++-gtk-utils/lib_defs.h>

#include <cstring>

#include <glib.h>

#include <c++-gtk-utils/reassembler.h>

namespace Cgu {

namespace Utf8 {

char* Reassembler::join_buffer(const char* str, size_t size) {

  char* out = new char[stored + size + 1];
  std::memcpy(out, buffer, stored);
  std::memcpy(out + stored, str, size);
  *(out + stored + size) = 0;
  stored = 0;

  return out ;
}

SharedHandle<char*> Reassembler::operator() (const char* input_p, size_t size) {

  // we can add a little more efficiency (and robustness) by checking
  // if the input string is valid in itself - if it is then  nothing
  // should be stored in the buffer (or if it is, it must be from a
  // different writer on a multi-writer pipe which isn't properly
  // protected by mutexes for atomic writes, so checking this now helps)
  if (g_utf8_validate(input_p, size, 0)) {
    char* out = new char[size + 1];
    std::memcpy(out, input_p, size);
    *(out + size) = 0;
    return SharedHandle<char*>(out);
  }

  // we may have the previous read of a partial UTF-8 character to deal with
  SharedHandle<char*> return_val;

  size_t new_size = stored + size;
  char* assembled = join_buffer(input_p, size);

  char* end_p;
  if (g_utf8_validate(assembled, new_size, const_cast<const char**>(&end_p))) {
    // we do not need to reset the value of stored: join_buffer() has done that
    return_val = SharedHandle<char*>(assembled);
  }
  else {
    size_t valid_size = end_p - assembled;
    stored = new_size - valid_size;
    if (stored <= buff_size) {
      std::memcpy(buffer, end_p, stored);
      *end_p = 0;  // terminate the string at the end of the valid part
      return_val = SharedHandle<char*>(assembled);
    }
    else {
      stored = 0; // the input string was not valid UTF-8
      delete[] assembled;
    }
  }
  return return_val;
}

} // namespace Utf8

} // namespace Cgu
