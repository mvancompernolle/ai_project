/***************************************************************************
                 ulxr_response.cpp  -  create xml-rpc response
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_response.cpp 1026 2007-07-25 07:48:09Z ewald-arnold $

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

#include <ulxmlrpcpp/ulxr_response.h>
#include <ulxmlrpcpp/ulxr_responseparse_wb.h>
#include <ulxmlrpcpp/ulxr_wbxmlparse.h>
#include <ulxmlrpcpp/ulxr_value.h>


namespace ulxr {


ULXR_API_IMPL0 MethodResponse::MethodResponse()
  : wasOk(true)
{
}


ULXR_API_IMPL0 MethodResponse::MethodResponse (const Void & /*val*/)
  : wasOk(true)
{
}


ULXR_API_IMPL0 MethodResponse::MethodResponse(int fval, const CppString &fstr)

{
  setFault(fval, fstr);
}


ULXR_API_IMPL0 MethodResponse::MethodResponse (const Value &val)
{
  setResult (val);
}


ULXR_API_IMPL0 MethodResponse::~MethodResponse()
{
}


ULXR_API_IMPL(bool) MethodResponse::isOK() const
{
  return wasOk;
}


ULXR_API_IMPL(void) MethodResponse::setFault(int fval, const CppString &fstr)
{
  ULXR_TRACE(ULXR_PCHAR("setFault"));
  wasOk = false;
  Struct st;
  st.addMember(ULXR_PCHAR("faultCode"), Integer(fval));
  st.addMember(ULXR_PCHAR("faultString"), RpcString(fstr));
  respval = st;
}


ULXR_API_IMPL(void) MethodResponse::setResult (const Value &val)
{
  ULXR_TRACE(ULXR_PCHAR("setResult"));
  wasOk = true;
  respval = val;
}


ULXR_API_IMPL(const Value&) MethodResponse::getResult() const
{
  return respval;
}


ULXR_API_IMPL(CppString) MethodResponse::getSignature(bool deep) const
{
  return respval.getSignature(deep);
}


ULXR_API_IMPL(CppString) MethodResponse::getXml(int indent) const
{
  CppString ind = getXmlIndent(indent);
  CppString ind1 = getXmlIndent(indent+1);
  CppString ind2 = getXmlIndent(indent+2);
  CppString s = ULXR_PCHAR("<?xml version=\"1.0\" encoding=\"utf-8\"?>") + getXmlLinefeed();
  s += ind + ULXR_PCHAR("<methodResponse>") + getXmlLinefeed();
  if (wasOk)
  {
    s += ind1 + ULXR_PCHAR("<params>") + getXmlLinefeed();
    if (!respval.isVoid())
    {
      s += ind2 + ULXR_PCHAR("<param>") + getXmlLinefeed();
      s += respval.getXml(indent+3) + getXmlLinefeed();
      s += ind2 + ULXR_PCHAR("</param>") + getXmlLinefeed();
    }
    s += ind1 + ULXR_PCHAR("</params>") + getXmlLinefeed();
  }
  else
  {
    s += ind1 + ULXR_PCHAR("<fault>") + getXmlLinefeed();
    s += respval.getXml(indent+2) +  getXmlLinefeed();
    s += ind1 + ULXR_PCHAR("</fault>") + getXmlLinefeed();
  }
  s += ind + ULXR_PCHAR("</methodResponse>");
  return s;
}


ULXR_API_IMPL(std::string) MethodResponse::getWbXml() const
{
  std::string s;
  s.assign(WbXmlParser::wbxml_START_SEQ_STR, WbXmlParser::wbxml_START_SEQ_LEN);
  s += MethodResponseParserWb::wbToken_MethodResponse;
  if (wasOk)
  {
    s += MethodResponseParserWb::wbToken_Params;
    if (!respval.isVoid())
    {
      s += MethodResponseParserWb::wbToken_Param;
      s += respval.getWbXml();
      s += WbXmlParser::wbxml_END;
    }
    s += WbXmlParser::wbxml_END;
  }
  else
  {
    s += MethodResponseParserWb::wbToken_Fault;
    s += respval.getWbXml();
    s += WbXmlParser::wbxml_END;
  }
  s += WbXmlParser::wbxml_END;
  return s;
}


}  // namespace ulxr

