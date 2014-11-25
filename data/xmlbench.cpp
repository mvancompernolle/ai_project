/***************************************************************************
                  xmlbench - simple test suite for speed

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: xmlbench.cpp 990 2007-07-14 15:00:39Z ewald-arnold $

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

#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <iostream>
#include <ctime>
#include <cstring>

#include <ulxmlrpcpp/ulxr_value.h>


int main(int argc, char ** argv)
{
  ulxr::intializeLog4J(argv[0]);

  int success = 0;
  try
  {
    ulxr::Array arr;
    arr.addItem(ulxr::Integer());
    arr.addItem(ulxr::Boolean());
    arr.addItem(ulxr::RpcString());
    arr.addItem(ulxr::Double());
    arr.addItem(ulxr::DateTime());
    arr.addItem(ulxr::Base64());
    arr.addItem(ulxr::Integer());
    arr.addItem(ulxr::Boolean());
    arr.addItem(ulxr::RpcString());
    arr.addItem(ulxr::Double());
    arr.addItem(ulxr::DateTime());
    arr.addItem(ulxr::Base64());
    arr.addItem(ulxr::Integer());
    arr.addItem(ulxr::Boolean());
    arr.addItem(ulxr::RpcString());
    arr.addItem(ulxr::Double());
    arr.addItem(ulxr::DateTime());
    arr.addItem(ulxr::Base64());
    arr.addItem(ulxr::Integer());
    arr.addItem(ulxr::Boolean());
    arr.addItem(ulxr::RpcString());
    arr.addItem(ulxr::Double());
    arr.addItem(ulxr::DateTime());
    arr.addItem(ulxr::Base64());

    const unsigned count = 5000;

////////////////////////////////////////////////////////////////

    ULXR_COUT << ULXR_PCHAR("Starting time measuring for generating xml data\n");

    ulxr::CppString us;
    ulxr_time_t starttime = ulxr_time(0);
    for (unsigned i1 = 0; i1 < count; ++i1)
    {
       us = arr.getXml();
    }
    ULXR_COUT << ULXR_PCHAR("Size of xml string in bytes: ") << us.length() << std::endl;

    ulxr_time_t endtime = ulxr_time(0);
    ulxr_time_t mins = (endtime - starttime) / 60;
    ulxr_time_t secs = (endtime - starttime) % 60;
    ULXR_COUT << ULXR_PCHAR("Time needed for xml (ref: 95us): ") << mins << ULXR_PCHAR(":")
              << secs << std::endl;

    std::string ss;
    starttime = ulxr_time(0);
    for (unsigned i2 = 0; i2 < count; ++i2)
    {
       ss = arr.getWbXml();
    }
    ULXR_COUT << ULXR_PCHAR("Size of wbxml string in bytes: ") << ss.length() << std::endl;

    endtime = ulxr_time(0);
    mins = (endtime - starttime) / 60;
    secs = (endtime - starttime) % 60;
    ULXR_COUT << ULXR_PCHAR("Time needed for wbxml (ref: 100us): ") << mins << ULXR_PCHAR(":")
              << secs << std::endl;

////////////////////////////////////////////////////////////////

  /* Ratio val1_server/client:
     4000 runs, debug output in file:
         wbxml:  30s,
         xml:   130s
   */
  }
  catch(...)
  {
    success = 1;
  }
  return success;
}
