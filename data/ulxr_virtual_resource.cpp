/***************************************************************************
   ulxr_virtual_resource.cpp - virtual resources accessed via ulxmlrpcpp
                             -------------------
    begin                : Sun May 1 2005
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_virtual_resource.cpp 940 2006-12-30 18:22:05Z ewald-arnold $

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

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <ulxmlrpcpp/ulxr_virtual_resource.h>


namespace ulxr {


ULXR_API_IMPL0 VirtualResource::VirtualResource(const CppString &res_name)
 : CachedResource(res_name)
{
  open();
}


ULXR_API_IMPL0 VirtualResource::~VirtualResource()
{
  close();
}


ULXR_API_IMPL(void) VirtualResource::open()
{
  reset();
}


ULXR_API_IMPL(void) VirtualResource::close()
{
}


ULXR_API_IMPL(bool) VirtualResource::good()
{
  return true;
}


};  // namespace ulxr
