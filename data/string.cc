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


#include <more/gen/string.h>
#include <more/bits/conf.h>
#ifdef MORE_CONF_HAVE_BDWGC
#  include <bdwgc/gc.h>
#endif
#include <vector>
#include <set>
#include <map>
#include <cstring>
#include <cwchar>
#include <cstdlib>

namespace more {
namespace gen {

#ifdef MORE_CONF_CXX_HAVE_WCSCMP
  using std::wcscmp;
#else
  int
  wcscmp(wchar_t const* s0, wchar_t const* s1)
  {
      while (*s0 && *s1) {
	  if (*s0 < *s1)
	      return -1;
	  if (*s1 < *s0)
	      return 1;
	  ++s0;
	  ++s1;
      }
      if (*s0)
	  return -1;
      if (*s1)
	  return 1;
      return 0;
  }
#endif

#ifdef MORE_CONF_CXX_HAVE_WCSLEN
  using std::wcslen;
#else
  std::size_t
  wcslen(wchar_t const* s)
  {
      std::size_t n = 0;
      while (*s) {
	  ++n;
	  ++s;
      }
      return n;
  }
#endif

  namespace {

    struct strless
	: std::binary_function<char const*, char const*, bool>
    {
	bool operator()(char const* s0, char const* s1) const
	{
	    return std::strcmp(s0, s1) < 0;
	}
    };
    typedef std::set< char const*, strless> struniq_container;
    struniq_container c_struniq;

    struct wcsless
	: std::binary_function<wchar_t const*, wchar_t const*, bool>
    {
	bool operator()(wchar_t const* s0, wchar_t const* s1) const
	{
	    return wcscmp(s0, s1) < 0;
	}
    };
    typedef std::set< wchar_t const*, wcsless> wcsuniq_container;
    wcsuniq_container c_wcsuniq;
  }

  char const*
  struniq_static(char const* str)
  {
      return *c_struniq.insert(str).first;
  }

  wchar_t const*
  struniq_static(wchar_t const* wcs)
  {
      return *c_wcsuniq.insert(wcs).first;
  }

  char const*
  struniq_copy(char const* str)
  {
      if (!str)
	  return 0;
      struniq_container::iterator it = c_struniq.find(str);
      if (it == c_struniq.end()) {
	  char* str0 = new char[std::strlen(str) + 1];
	  strcpy(str0, str);
	  return *c_struniq.insert(str0).first;
      }
      else
	  return *it;
  }

  wchar_t const*
  struniq_copy(wchar_t const* wcs)
  {
      if (!wcs)
	  return 0;
      wcsuniq_container::iterator it = c_wcsuniq.find(wcs);
      if (it == c_wcsuniq.end()) {
	  wchar_t* wcs0 = new wchar_t[wcslen(wcs) + 1];
	  wcscpy(wcs0, wcs);
	  return *c_wcsuniq.insert(wcs0).first;
      }
      else
	  return *it;
  }

  const std::size_t int_idfr_max_size = 3 + sizeof(unsigned int)*2;

  struct uint_to_name_chunk
  {
      static const std::size_t chunk_size = 256;
      char str[chunk_size][int_idfr_max_size];
  };

  void
  mk_int_idfr(char* s, unsigned int n)
  {
      *s++ = 'x';
      *s++ = 'x';
      int l = sizeof(unsigned int)*8 - 4;
      while (l >= 0 && (n >> l) == 0)
	  l -= 4;
      while (l >= 0) {
	  int m = (n >> l) & 0x0f;
	  if (m < 10)
	      *s++ = '0' + m;
	  else
	      *s++ = 'a' + m - 10;
	  l -= 4;
      }
      *s++ = 0;
  }

  char const*
  struniq_dense(int index)
  {
      static std::size_t const chunk_size = uint_to_name_chunk::chunk_size;
      static std::vector<uint_to_name_chunk*> v;
      int plus = 1;
      if (index < 0) {
	  plus = 0;
	  index = -index;
      }
      unsigned int I = index/chunk_size;
      unsigned int i = index%chunk_size;
      if (I >= v.size()) {
	  unsigned int J = v.size();
	  v.resize(I + 1);
	  for (; J <= I; ++J) {
	      uint_to_name_chunk* chunk = new uint_to_name_chunk;
	      v[J] = chunk;
	      for (unsigned int j = 0; j < chunk_size; ++j) {
		  char* s = chunk->str[j];
		  unsigned int Jj = J*chunk_size + j;
		  mk_int_idfr(s, Jj);
	      }
	  }
      }
      return v[I]->str[i] + plus;
  }

  char const*
  struniq_sparse(int index)
  {
#if 1  // Faster, but takes more memory...
      static std::map<int, char const*> m;
      std::map<int, char const*>::iterator it = m.find(index);
      int plus = 1;
      if (index < 0) {
	  plus = 0;
	  index = -index;
      }
      if (it == m.end()) {
	  char str0[int_idfr_max_size];
	  mk_int_idfr(str0, index);
	  char const* str = struniq_copy(str0);
	  struniq_static(str + 1);
	  m.insert(std::make_pair(index, str));
	  return str + plus;
      }
      else
	  return it->second + plus;
#else   // ... than this.
      char str[int_idfr_max_size];
      int plus = 1;
      if (index < 0) {
	  plus = 0;
	  index = -index;
      }
      mk_int_idfr(str, index);
      return struniq_copy(str + plus);
#endif
  }

#ifdef MORE_CONF_HAVE_BDWGC
  char*
  wcstombs_gc(wchar_t const* ws)
  {
      int n = std::wcstombs(0, ws, 0) + 1;
      char* s0 = (char*)GC_malloc(sizeof(char)*n);
      wcstombs(s0, ws, n);
      return s0;
  }

  wchar_t*
  mbstowcs_gc(char const* str)
  {
      int n = std::mbstowcs(0, str, 0) + 1;
      wchar_t* wcs = (wchar_t*)GC_malloc(sizeof(wchar_t)*n);
      mbstowcs(wcs, str, n);
      return wcs;
  }
#endif

}}
