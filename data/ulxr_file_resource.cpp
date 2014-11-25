/***************************************************************************
                  ulxr_call.h  -  create xml-rpc calling data
                             -------------------
    begin                : Sun May 1 2005
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_file_resource.cpp 1078 2007-09-04 17:18:47Z ewald-arnold $

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

#ifdef _MSC_VER
# include <io.h>
#endif

#include <cstdio>

#include <ulxmlrpcpp/ulxr_file_resource.h>

#ifdef feof  // remove legacy stuff
#undef feof
#endif


namespace ulxr {


ULXR_API_IMPL0 FileResource::FileResource(const CppString &res_name,
                                       const CppString &in_file_name,
                                       bool do_read)
 : CachedResource(res_name)
 , filename(in_file_name)
 , error(false)
 , opened(false)
{
  open(do_read);
}


ULXR_API_IMPL0 FileResource::~FileResource()
{
  close();
}


ULXR_API_IMPL(CppString) FileResource::getFilename() const
{
  return filename;
}


ULXR_API_IMPL(void) FileResource::open()
{
  open(true);
}


ULXR_API_IMPL(void) FileResource::open(bool do_read)
{
  ULXR_TRACE(ULXR_PCHAR("FileResource::open ") << filename);
  if (opened)
    return;

  reset();
  if (do_read)
  {
    ULXR_TRACE(ULXR_PCHAR("FileResource::open do_read"));
#ifdef __unix__
    if (0 == ulxr_access(getLatin1(getFilename()).c_str(), F_OK)) // file exists?
#else
    if (0 == ulxr_access(getLatin1(getFilename()).c_str(), 0))
#endif
    {
      ulxr_FILE *ifs = ulxr_fopen (getLatin1(filename).c_str(), "rb");
      char buffer [2000];
      if (ifs != 0)
      {
        while (!ulxr_feof(ifs) && !error)
        {
          size_t readed = ulxr_fread(buffer, 1, sizeof(buffer), ifs);
          if (ulxr_ferror(ifs))
            error = true;

          write(buffer, readed);
        }
        ulxr_fclose (ifs);
      }
    }
  }

  opened = true;
}


ULXR_API_IMPL(void) FileResource::close()
{
  ULXR_TRACE(ULXR_PCHAR("FileResource::close ") << filename);
  if (!opened)
    return;

//  size_t written;
  const std::string dat = data();
  const unsigned len = dat.length();
  ulxr_FILE *ifs = ulxr_fopen (getLatin1(filename).c_str(), "wb");
  if (ifs != 0)
  {
    if (len != 0)
    {
      /* written = */ ulxr_fwrite(dat.data(), 1, len, ifs);
      if (ulxr_ferror(ifs))
        error = true;
    }
    ulxr_fclose (ifs);
  }
  else
    error = true;

  opened = false;
  CachedResource::clear();
}


ULXR_API_IMPL(void) FileResource::reset()
{
  ULXR_TRACE(ULXR_PCHAR("FileResource::reset ") << filename);
  CachedResource::reset();
  error = false;
}


ULXR_API_IMPL(void) FileResource::clear()
{
  ULXR_TRACE(ULXR_PCHAR("FileResource::clear ") << filename);
  CachedResource::clear();
  opened = false;

#ifdef __unix__
  if (0 == ulxr_access(getLatin1(getFilename()).c_str(), F_OK))
#else
  if (0 == ulxr_access(getLatin1(getFilename()).c_str(), 0))
#endif
    if (0 != ulxr_remove(getLatin1(getFilename()).c_str()))
      error = true;
}


ULXR_API_IMPL(bool) FileResource::good()
{
  ULXR_TRACE(ULXR_PCHAR("FileResource::good ") << error);
  return !error;
}


};  // namespace ulxr
