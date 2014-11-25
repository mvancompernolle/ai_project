/***************************************************************************
                 resource.cpp  --  test file for resources

    begin                : Sun May 1 2005
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: resource.cpp 940 2006-12-30 18:22:05Z ewald-arnold $

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

#include <ulxmlrpcpp/ulxmlrpcpp.h>

#if defined(_MSC_VER)
#include <io.h>
#endif

#include <iostream>
#include <cstdio>

#include <ulxmlrpcpp/ulxr_virtual_resource.h>
#include <ulxmlrpcpp/ulxr_file_resource.h>
#include <ulxmlrpcpp/ulxr_except.h>

#define ASSERTER(x) \
   if (!(x) ) { \
     std::cout << std::endl \
               << "Error occured: " << #x << std::endl \
               << std::endl \
               << std::flush; \
     return false; \
   }


bool test_virtual()
{
  std::cout << "test_virtual()\n";
  ulxr::VirtualResource vr(ULXR_PCHAR("resource"));
  ASSERTER(ULXR_PCHAR("resource") == vr.getResourceName());

  vr.write ("12");
  ASSERTER("12" == vr.read());

  vr.write ("3456", 2);
  vr.write ("7890");
  ASSERTER("347" == vr.read(3));
  ASSERTER("890" == vr.read());

  vr.reset();
  ASSERTER("12347890" == vr.read());

  vr.clear();
  vr.write("abcd");
  ASSERTER("abcd" == vr.read());

  return true;
}


bool test_file()
{
  std::cout << "test_file()\n";
  ulxr::CppString filename = ULXR_PCHAR("dummy-resource.txt");
  ::unlink(ulxr::getLatin1(filename).c_str());
#ifdef __unix__
  ASSERTER(0 != ulxr_access(ulxr::getLatin1(filename).c_str(), F_OK));
#else
  ASSERTER(0 != ulxr_access(ulxr::getLatin1(filename).c_str(), 0)); 
#endif
  ulxr::FileResource fr(ULXR_PCHAR("resource"), filename);
  ASSERTER(ULXR_PCHAR("resource") == fr.getResourceName());

  fr.write ("12");
  ASSERTER("12" == fr.read());

  fr.write ("3456", 2);
  fr.write ("7890");
  ASSERTER("347" == fr.read(3));
  ASSERTER("890" == fr.read());

  fr.reset();
  ASSERTER("12347890" == fr.read());

  fr.clear();
  fr.open();

  fr.write("abcd");
  ASSERTER("abcd" == fr.read());

  fr.close();
#ifdef __unix__
  ASSERTER(0 == ::access(ulxr::getLatin1(filename).c_str(), F_OK));
#else
  ASSERTER(0 == ::access(ulxr::getLatin1(filename).c_str(), 0));
#endif

  ulxr::FileResource fr2(ULXR_PCHAR("resource"), filename);
  ASSERTER("abcd" == fr2.read());

  fr.clear();
#ifdef __unix__
  ASSERTER(0 != ::access(ulxr::getLatin1(filename).c_str(), F_OK));
#else
  ASSERTER(0 != ::access(ulxr::getLatin1(filename).c_str(), 0));
#endif

  ASSERTER(true == fr.good());
  ASSERTER(true == fr2.good());

  return true;
}


int main()
{
  bool success = true;
  try
  {
     if (!test_virtual())
       success = false;

     if (!test_file())
       success = false;

     ULXR_COUT << ULXR_PCHAR("Ready.\n");
  }
  catch(ulxr::Exception &ex)
  {
     ULXR_COUT << ULXR_PCHAR("Error occured: ")
               << ULXR_GET_STRING(ex.why()) << std::endl;
     success = false;
  }

  ULXR_COUT << ULXR_PCHAR("Terminating.\n");
  return success ? 0 : 1;
}
