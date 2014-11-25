/***************************************************************************
                  ulxr_call.h  -  create xml-rpc calling data
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_call.cpp 1026 2007-07-25 07:48:09Z ewald-arnold $

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
//#define ULXR_SHOW_READ
//#define ULXR_SHOW_WRITE

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <ulxmlrpcpp/ulxr_call.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_callparse_wb.h>
#include <ulxmlrpcpp/ulxr_wbxmlparse.h>


namespace ulxr {


ULXR_API_IMPL0 MethodCall::MethodCall(const ulxr::Char *name)
{
  methodname = name;
}


ULXR_API_IMPL0 MethodCall::MethodCall()
{
}


ULXR_API_IMPL0 MethodCall::~MethodCall()
{
}


ULXR_API_IMPL0 MethodCall::MethodCall(const CppString &name)
{
  methodname = name;
}


MethodCall&  /*ULXR_API_IMPL0*/ MethodCall::addParam (const Value &val)
{
  params.push_back(val);
  return *this;
}


MethodCall&  /*ULXR_API_IMPL0*/ MethodCall::setParam (const Value &val)
{
  clear();
  return addParam(val);
}


ULXR_API_IMPL(CppString) MethodCall::getSignature(bool name_braces) const
{
  CppString s;
  if (name_braces)
    s += methodname + ULXR_PCHAR("(");

  bool comma = params.size() >= 1;
  for (unsigned i = 0; i < params.size(); ++i) {
    if (comma && i != 0)
      s += ',';
    s += params[i].getSignature();
  }

  if (name_braces)
     s += ULXR_PCHAR(")");
  return s;
}


ULXR_API_IMPL(CppString) MethodCall::getXml(int indent) const
{
  CppString ind = getXmlIndent(indent);
  CppString ind1 = getXmlIndent(indent+1);
  CppString ind2 = getXmlIndent(indent+2);
  CppString s = ULXR_PCHAR("<?xml version=\"1.0\" encoding=\"UTF-8\"?>") + getXmlLinefeed();
  s += ind + ULXR_PCHAR("<methodCall>") + getXmlLinefeed();
  s += ind1 + ULXR_PCHAR("<methodName>")+methodname+ULXR_PCHAR("</methodName>") + getXmlLinefeed();

  s += ind1 + ULXR_PCHAR("<params>") + getXmlLinefeed();

  for (std::vector<Value>::const_iterator
         it = params.begin(); it != params.end(); ++it)
  {
    s += ind2 + ULXR_PCHAR("<param>") + getXmlLinefeed();
    s += (*it).getXml(indent+3) + getXmlLinefeed();
    s += ind2 + ULXR_PCHAR("</param>") + getXmlLinefeed();
  }

  s += ind1 + ULXR_PCHAR("</params>") + getXmlLinefeed();
  s += ind + ULXR_PCHAR("</methodCall>");
  return s;
}


ULXR_API_IMPL(std::string) MethodCall::getWbXml() const
{
  std::string s;
  s.assign(WbXmlParser::wbxml_START_SEQ_STR, WbXmlParser::wbxml_START_SEQ_LEN);
  s += MethodCallParserWb::wbToken_MethodCall;
  s += MethodCallParserWb::wbToken_MethodName;
  s += getWbXmlString(methodname);
  s += WbXmlParser::wbxml_END;

  s += MethodCallParserWb::wbToken_Params;

  for (std::vector<Value>::const_iterator
         it = params.begin(); it != params.end(); ++it)
  {
    s += MethodCallParserWb::wbToken_Param;
    s += (*it).getWbXml();
    s += WbXmlParser::wbxml_END;
  }

  s += WbXmlParser::wbxml_END;
  s += WbXmlParser::wbxml_END;
  return s;
}


ULXR_API_IMPL(Value) MethodCall::getParam(unsigned ind) const
{
  if (ind < params.size() )
    return params[ind];

  throw ParameterException(InvalidMethodParameterError,
                           ulxr_i18n(ULXR_PCHAR("Not enough actual parameters for call to method: ")
                               +getSignature()));
}

ULXR_API_IMPL(unsigned) MethodCall::numParams() const
{
  return params.size();
}


ULXR_API_IMPL(void) MethodCall::clear()
{
  params.clear();
}


ULXR_API_IMPL(CppString) MethodCall::getMethodName() const
{
  return methodname;
}


ULXR_API_IMPL(void) MethodCall::setMethodName(const CppString &nm)
{
  methodname = nm;
}


}  // namespace ulxr

