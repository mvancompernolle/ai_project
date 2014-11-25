/***************************************************************************
            ulxr_responseparse.cpp  -  parse xml-rpc method response
                             -------------------
    begin                : Wed Mar 13 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_responseparse.cpp 1152 2009-08-12 15:48:25Z ewald-arnold $

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

#include <cstring>
#include <memory>

#include <ulxmlrpcpp/ulxr_responseparse.h>
#include <ulxmlrpcpp/ulxr_except.h>


namespace ulxr {


ULXR_API_IMPL(void)
  MethodResponseParser::startElement(const XML_Char* name, const XML_Char** atts)
{
  if (!testStartElement(name, atts))
    ValueParser::testStartElement(name, atts);
}


ULXR_API_IMPL(bool)
  MethodResponseParser::testStartElement(const XML_Char* name, const XML_Char** /*atts*/)
{
  ULXR_TRACE(ULXR_PCHAR("MethodResponseParser::testStartElement(const XML_Char*, const char**)")
             << ULXR_PCHAR("\n  name: ")
             << ULXR_GET_STRING(name)
            );

  switch(states.top()->getParserState() )
  {
    case eNone:
      if(strcmp(name, "methodResponse") == 0)
      {
        setComplete (false);
        states.push(new ValueState(eMethodResponse));
      }
      else
        return false;
    break;

    case eMethodResponse:
      if(strcmp(name, "fault") == 0)
        states.push(new ValueState(eFault));

      else if(strcmp(name, "params") == 0)
        states.push(new ValueState(eParams));

      else
        return false;
    break;

    case eFault:
      if(strcmp(name, "value") == 0)
        states.push(new ValueState(eValue));
      else
        return false;
    break;

    case eParams:
      if(strcmp(name, "param") == 0)
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


ULXR_API_IMPL(void) MethodResponseParser::endElement(const XML_Char *name)
{
  if (!testEndElement(name))
    ValueParser::testEndElement(name);
}


ULXR_API_IMPL(bool) MethodResponseParser::testEndElement(const XML_Char *name)
{
  ULXR_TRACE(ULXR_PCHAR("MethodResponseParser::testEndElement(const XML_Char*)"));

  if (states.size() <= 1)
    throw RuntimeException(ApplicationError, ulxr_i18n(ULXR_PCHAR("abnormal program behaviour: MethodResponseParser::testEndElement() had no states left")));

  std::auto_ptr<ValueState> curr(getTopValueState());
  states.pop();
  ValueState *on_top = getTopValueState();

  switch(curr->getParserState() )
  {
    case eMethodResponse:
      setComplete(true);
      assertEndElement(name, "methodResponse");
      on_top->takeValue (curr->getValue());
      if (on_top->getValue() != 0)
        method_value = *on_top->getValue();
    break;

    case eFault:
      assertEndElement(name, "fault");
      on_top->takeValue (curr->getValue());
    break;

    case eParams:
      assertEndElement(name, "params");
      on_top->takeValue (curr->getValue());
    break;

    case eParam:
      assertEndElement(name, "param");
      on_top->takeValue (curr->getValue());
    break;

    default:
      states.push(curr.release());   // put back, someone else will process
      return false;
  }

  return true;
}


}  // namespace ulxr
