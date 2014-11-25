/***************************************************************************
            ulxr_responseparse_base.cpp  -  parse rpc method response
                             -------------------
    begin                : Fri Jan 09 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_responseparse_base.cpp 940 2006-12-30 18:22:05Z ewald-arnold $

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

#include <ulxmlrpcpp/ulxr_responseparse_base.h>


namespace ulxr {


ULXR_API_IMPL0 MethodResponseParserBase::~MethodResponseParserBase()
{
}


ULXR_API_IMPL(MethodResponse) MethodResponseParserBase::getMethodResponse()
{
  if (method_value.isStruct() )
  {
     Struct st = method_value;
     if (   st.size() == 2
         && st.hasMember(ULXR_PCHAR("faultCode"))
         && st.hasMember(ULXR_PCHAR("faultString")) )
     {
       Integer iv = st.getMember(ULXR_PCHAR("faultCode"));
       RpcString sv = st.getMember(ULXR_PCHAR("faultString"));
       return MethodResponse(iv.getInteger(), sv.getString());
     }
    else
      return MethodResponse(method_value);
  }
  else
    return MethodResponse(method_value);
}


}  // namespace ulxr
