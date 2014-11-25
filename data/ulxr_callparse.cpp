/**************************************************************************
               ulxr_callparse.cpp  -  parse xml-rpc method call
                             -------------------
    begin                : Wed Mar 13 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_callparse.cpp 1158 2009-08-30 14:34:24Z ewald-arnold $

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
//#define ULXR_SHOW_XML

#include <cstring>
#include <memory>

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <ulxmlrpcpp/ulxr_callparse.h>
#include <ulxmlrpcpp/ulxr_except.h>


namespace ulxr {


ULXR_API_IMPL(void) MethodCallParser::startElement(const XML_Char* name,
                                                const XML_Char** atts)
{
  if (!testStartElement(name, atts))
    ValueParser::testStartElement(name, atts);
}


ULXR_API_IMPL(bool)
  MethodCallParser::testStartElement(const XML_Char* name,
                                     const XML_Char** /*atts*/)
{
  ULXR_TRACE(ULXR_PCHAR("MethodCallParser::testStartElement(const XML_Char*, const char**)")
             << ULXR_PCHAR("\n  name: ")
             << ULXR_GET_STRING(name)
            );

  switch(states.top()->getParserState() )
  {
    case eNone:
      if (strcmp(name, "methodCall") == 0)
      {
        setComplete(false);
        states.push(new ValueState(eMethodCall));
      }
      else
        return false;
    break;

    case eMethodCall:
      if (strcmp(name, "methodName") == 0)
        states.push(new ValueState(eMethodName));

      else if (strcmp(name, "params") == 0)
        states.push(new ValueState(eParams));

      else
        return false;
    break;

    case eParams:
      if (strcmp(name, "param") == 0)
        states.push(new ValueState(eParam));
      else
        return false;
    break;

    case eParam:
      if(strcmp(name, "value") == 0)
        states.push(new ValueState(eValue));
      else
        return false;
    break;

    default:
        return false;
  }

  return true;
}


ULXR_API_IMPL(void) MethodCallParser::endElement(const XML_Char *name)
{
  if (!testEndElement(name))
    ValueParser::testEndElement(name);
}


ULXR_API_IMPL(bool) MethodCallParser::testEndElement(const XML_Char *name)
{
  ULXR_TRACE(ULXR_PCHAR("MethodCallParser::testEndElement(const XML_Char*)"));

  if (states.size() <= 1)
    throw RuntimeException(ApplicationError, ulxr_i18n(ULXR_PCHAR("abnormal program behaviour: MethodCallParser::testEndElement() had no states left")));

  std::auto_ptr<ValueState> curr(getTopValueState());
  states.pop();

//   ULXR_TRACE(ULXR_PCHAR("\n  current data: <")
//              << curr->getCharData() << ULXR_PCHAR(">"));

  switch(curr->getParserState() )
  {
    case eMethodName:
      assertEndElement(name, "methodName");
      methodcall.setMethodName(curr->getCharData());
    break;

    case eMethodCall:
      assertEndElement(name, "methodCall");
      setComplete(true);
    break;

    case eParams:
      assertEndElement(name, "params");
    break;

    case eParam:
      assertEndElement(name, "param");
      if (curr->getValue() != 0)
        methodcall.addParam(*curr->getValue());
      delete curr->getValue();
    break;

    default:
      states.push(curr.release());  // put back, someone else will process
      return false;
  }

  return true;
}


}  // namespace ulxr

