/**************************************************************************
               ulxr_callparse_base.cpp  -  parse rpc method call
                             -------------------
    begin                : Fri Jan 09 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_callparse_base.cpp 940 2006-12-30 18:22:05Z ewald-arnold $

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

#include <ulxmlrpcpp/ulxr_callparse_base.h>


namespace ulxr {


ULXR_API_IMPL0 MethodCallParserBase::~MethodCallParserBase()
{
}


ULXR_API_IMPL(unsigned) MethodCallParserBase::numParams() const
{
  return methodcall.numParams();
}


ULXR_API_IMPL(Value) MethodCallParserBase::getParam(unsigned ind) const
{
  return methodcall.getParam(ind);
}


ULXR_API_IMPL(CppString) MethodCallParserBase::getMethodName() const
{
  return methodcall.getMethodName();
}


ULXR_API_IMPL(MethodCall) MethodCallParserBase::getMethodCall() const
{
  return methodcall;
}


/*
string MethodCallParserBase::ValueState::getStateName() const
{
  switch (state)
  {
    case eMethodCall      : return "eMethodCall";
    case eMethodName      : return "eMethodName";
    case eParams          : return "eParams";
    case eParam           : return "eParam";
    case eCallParserLast  : return "eCallParserLast";
  }

  return ValueParser::getStateName();
}
*/

}  // namespace ulxr

