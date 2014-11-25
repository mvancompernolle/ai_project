/***************************************************************************
      ulxr_cached_resource.cpp - resources accessed via ulxmlrpcpp
                             -------------------
    begin                : Sun May 1 2005
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_cached_resource.cpp 940 2006-12-30 18:22:05Z ewald-arnold $

 ***************************************************************************/

/**************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***************************************************************************/

//#define ULXR_SHOW_TRACE
//#define ULXR_DEBUG_OUTPUT


#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <ulxmlrpcpp/ulxr_cached_resource.h>


namespace ulxr {


ULXR_API_IMPL0 CachedResource::CachedResource(const CppString &in_name)
 : name(in_name)
{
  reset();
}


ULXR_API_IMPL(CppString) CachedResource::getResourceName() const
{
  ULXR_TRACE(ULXR_PCHAR("CachedResource::getResourceName"));
  return name;
}


ULXR_API_IMPL0 CachedResource::~CachedResource()
{
  ULXR_TRACE(ULXR_PCHAR("~CachedResource ") << name);
}


ULXR_API_IMPL(std::string) CachedResource::data() const
{
  ULXR_TRACE(ULXR_PCHAR("CachedResource::data"));
  return cache;
}


ULXR_API_IMPL(std::string) CachedResource::read()
{
  ULXR_TRACE(ULXR_PCHAR("CachedResource::read"));
  std::string s = cache.substr(read_pointer);
  read_pointer = cache.length();
  return s;
}


ULXR_API_IMPL(std::string) CachedResource::read(unsigned len)
{
  ULXR_TRACE(ULXR_PCHAR("CachedResource::read(int) ") << len);
  std::string s = cache.substr(read_pointer, len);
  read_pointer += len;
  if (read_pointer > cache.length())
    read_pointer = cache.length();

  return s;
}


ULXR_API_IMPL(void) CachedResource::write (const std::string &s)
{
  ULXR_TRACE(ULXR_PCHAR("CachedResource::write ") << s.length());
  cache += s;
}


ULXR_API_IMPL(void) CachedResource::write (const char *str, unsigned len)
{
  ULXR_TRACE(ULXR_PCHAR("CachedResource::write(str, len) ") << len);
  cache.append(str, len);
}


ULXR_API_IMPL(void) CachedResource::clear()
{
  cache = "";
  read_pointer = 0;
}


ULXR_API_IMPL(void) CachedResource::reset()
{
  read_pointer = 0;
}


};  // namespace ulxr
