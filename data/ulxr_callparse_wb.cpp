/**************************************************************************
               ulxr_callparse_wb.cpp  -  parse wbxml-rpc method call
                             -------------------
    begin                : Fri Jan 09 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_callparse_wb.cpp 1158 2009-08-30 14:34:24Z ewald-arnold $

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

#include <memory>

#include <ulxmlrpcpp/ulxr_callparse_wb.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_htmlform_handler.h>


namespace ulxr {


ULXR_API_IMPL(void) MethodCallParserWb::startElement(unsigned token, const Attributes &attr)
{
  if (!testStartElement(token, attr))
    ValueParserWb::testStartElement(token, attr);
}


ULXR_API_IMPL(bool)
  MethodCallParserWb::testStartElement(unsigned token, const Attributes &)
{
  ULXR_TRACE(ULXR_PCHAR("MethodCallParserWb::testStartElement()")
             << ULXR_PCHAR("\n  token: ")+HtmlFormHandler::makeHexNumber(token)
            );

  switch(states.top()->getParserState() )
  {
    case eNone:
      if (wbToken_MethodCall == token)
      {
        setComplete(false);
        states.push(new ValueState(eMethodCall));
      }
      else
        return false;
    break;

    case eMethodCall:
      if (wbToken_MethodName == token)
        states.push(new ValueState(eMethodName));

      else if (wbToken_Params == token)
        states.push(new ValueState(eParams));

      else
        return false;
    break;

    case eParams:
      if (wbToken_Param == token)
        states.push(new ValueState(eParam));
      else
        return false;
    break;

    case eParam:
      if(wbToken_Value == token)
        states.push(new ValueState(eValue));
      else
        return false;
    break;

    default:
        return false;
  }

  return true;
}


ULXR_API_IMPL(void) MethodCallParserWb::endElement()
{
  if (!testEndElement())
    ValueParserWb::testEndElement();
}


ULXR_API_IMPL(bool) MethodCallParserWb::testEndElement()
{
  ULXR_TRACE(ULXR_PCHAR("MethodCallParserWb::testEndElement()"));
  if (states.size() <= 1)
    throw RuntimeException(ApplicationError, ulxr_i18n(ULXR_PCHAR("abnormal program behaviour: MethodCallParserWb::testEndElement() had no states left")));

  std::auto_ptr<ValueState> curr(getTopValueState());
  states.pop();

  switch(curr->getParserState() )
  {
    case eMethodName:
      methodcall.setMethodName(curr->getCharData());
    break;

    case eMethodCall:
      setComplete(true);
    break;

    case eParams:
    break;

    case eParam:
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

