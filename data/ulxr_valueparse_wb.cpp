/**************************************************************************
               ulxr_valueparse_wb.cpp  -  parse wbxml-rpc primitive values
                             -------------------
    begin                : Fri Jan 09 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_valueparse_wb.cpp 1158 2009-08-30 14:34:24Z ewald-arnold $

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

//#define ULXR_DEBUG_OUTPUT

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <memory>

#include <ulxmlrpcpp/ulxr_valueparse_wb.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_htmlform_handler.h>

#ifdef DEBUG
#include <iostream>
#endif


namespace ulxr {


ULXR_API_IMPL0 ValueParserWb::ValueParserWb()
  : ValueParserBase()
{
  ULXR_TRACE(ULXR_PCHAR("ValueParser::ValueParser()"));
  states.push(new ValueState(eNone));
}


ULXR_API_IMPL0 ValueParserWb::~ValueParserWb()
{
  ULXR_TRACE(ULXR_PCHAR("ValueParser::~ValueParser()"));

#ifdef DEBUG
  if (states.size() != 1)
    std::cerr << ulxr_i18n("########## abnormal program behaviour: states.size() != 1: ")
              << states.size()
              << std::endl;

  if (getTopValueState()->getParserState() != eNone)
    std::cerr << ulxr_i18n("########## abnormal program behaviour: getTopState()->getState() != eNone: ")
              << getTopValueState()->getParserState()
              << std::endl;
#endif

  while (states.size() != 0)
  {
    if (getTopValueState()->canDelete())
      delete getTopValueState()->getValue();
    delete getTopValueState();
    states.pop();
  }
}


ULXR_API_IMPL(ValueParserBase::ValueState *) ValueParserWb::getTopValueState() const
{
  ULXR_TRACE(ULXR_PCHAR("ValueParserWb::getTopState() size: ") << states.size());
#ifdef DEBUG
  ValueState *vs = dynamic_cast<ValueState*> (states.top());      // be careful about type
  if (vs == 0)
  {
    ULXR_TRACE(ULXR_PCHAR("ValueParserWb::getTopState(), top state: ") << (void*) states.top());
    ULXR_TRACE(ULXR_PCHAR("ValueParserWb::getTopState(): state <> ValueState"));
    ULXR_TRACE(ULXR_PCHAR("ValueParserWb::getTopState(): state == ")
                          << ULXR_GET_STRING(typeid(states.top()).name()));
  }
  return vs;
#else
  return reinterpret_cast<ValueParserBase::ValueState*> (states.top());  // dont care anymore
#endif
}


ULXR_API_IMPL(void)
  ValueParserWb::startElement(unsigned token, const Attributes &attr)
{
  ULXR_TRACE(ULXR_PCHAR("ValueParserWb::startElement()"));
  if (!testStartElement(token, attr))
    WbXmlParser::testStartElement(token, attr);
}


ULXR_API_IMPL(bool)
  ValueParserWb::testStartElement(unsigned token, const Attributes &)
{
  ULXR_TRACE(ULXR_PCHAR("ValueParserWb::testStartElement()")
             << ULXR_PCHAR("\n  token: ")+HtmlFormHandler::makeHexNumber(token)
            );
  switch(getTopValueState()->getParserState() )
  {
    case eNone:
      if (wbToken_Value == token)
        states.push(new ValueState(eValue));
      else
        return false;
    break;

    case eValue:
      if (wbToken_Array == token)
        states.push(new ValueState(eArray));

      else if (wbToken_Struct == token)
        states.push(new ValueState(eStruct));

      else if (wbToken_Boolean == token)
        states.push(new ValueState(eBoolean));

      else if (wbToken_Int == token)
        states.push(new ValueState(eInt));

      else if (wbToken_I4 == token)
        states.push(new IntegerState(eI4));

      else if (wbToken_Double == token)
        states.push(new ValueState(eDouble));

      else if (wbToken_String == token)
        states.push(new ValueState(eString));

      else if (wbToken_Base64 == token)
        states.push(new ValueState(eBase64));

      else if (wbToken_Date == token)
        states.push(new ValueState(eDate));

      else
        return false;
    break;

    case eStruct:
      if (wbToken_Member == token)
      {
        if (getTopValueState()->getValue() == 0)  // first closing member adds struct-item
          getTopValueState()->takeValue(new Value(Struct()), false);
        states.push(new MemberState(eMember, getTopValueState()->getValue()));
      }
      else
        return false;
    break;

    case eMember:
      if (wbToken_Name == token)
        states.push(new ValueState(eName));

      else if (wbToken_Value == token)
        states.push(new ValueState(eValue));

      else
        return false;

    break;

    case eArray:
      if (wbToken_Data == token)
        states.push(new ArrayState(eData));
      else
        return false;
    break;

    case eData:
      if (wbToken_Value == token)  // closing value adds data-items
        states.push(new ValueState(eValue));
      else
        return false;
    break;

    default:
        return false;
  }

  return true;
}


ULXR_API_IMPL(void) ValueParserWb::endElement()
{
  ULXR_TRACE(ULXR_PCHAR("ValueParserWb::endElement()"));
  if (!testEndElement())
    WbXmlParser::testEndElement();
}


ULXR_API_IMPL(bool) ValueParserWb::testEndElement()
{
  ULXR_TRACE(ULXR_PCHAR("ValueParserWb::testEndElement()"));

  if (states.size() <= 1)
    throw RuntimeException(ApplicationError, ulxr_i18n(ULXR_PCHAR("abnormal program behaviour: ValueParserWb::testEndElement() had no states left")));

  std::auto_ptr<ValueState> curr(getTopValueState());
  states.pop();
/*
  ULXR_DOUT (ULXR_PCHAR(" = cur-val: ")
             << std::hex << (void*) curr->getValue()
             << ULXR_PCHAR(" state: ")
             << std::hex << (void*) curr->getParserState()
             << ULXR_PCHAR(" prev state: ")
             << std::hex << (void*) curr->getPrevParserState()
             << std::dec);
*/
  states.top()->setPrevParserState(curr->getParserState());
  switch(curr->getParserState() )
  {
    case eInt:
    case eI4:
      getTopValueState()->takeValue(new Value(Integer(curr->getCharData())) );
    break;

    case eDouble:
      getTopValueState()->takeValue(new Value(Double(curr->getCharData())) );
    break;

    case eBoolean:
      getTopValueState()->takeValue(new Value(Boolean(curr->getCharData())) );
    break;

    case eString:
      getTopValueState()->takeValue(new Value(RpcString(curr->getCharData())) );
    break;

    case eBase64:
    {
      Base64 b64;
      b64.setBase64(curr->getCharData()); // move raw data!
      getTopValueState()->takeValue(new Value(b64));
    }
    break;

    case eDate:
      getTopValueState()->takeValue(new Value(DateTime(curr->getCharData())) );
    break;

    case eName:
      getTopValueState()->takeName((curr->getCharData()) );
    break;

    case eValue:
      if (curr->getValue() == 0)     // special case
      {
        if(curr->getPrevParserState() == eArray)            // not empty Array
           getTopValueState()->takeValue (new Value(Array()));

        else if (curr->getPrevParserState() == eStruct)     // not empty Struct
           getTopValueState()->takeValue (new Value(Struct()));

        else                                                // no type tag defaults to string
          getTopValueState()->takeValue (new Value(RpcString(curr->getCharData())));
      }
      else
        getTopValueState()->takeValue (curr->getValue());
    break;

    case eMember:
    case eStruct:
    case eArray:
    case eData:
      getTopValueState()->takeValue (curr->getValue());
    break;

    default:
      states.push(curr.release());
      return false;
  }

  return true;
}


//////////////////////////////////////////////////////////////////////////////
//

ULXR_API_IMPL0 ValueParserWb::IntegerState::IntegerState(unsigned st)
  : ValueState(st)
{
}


ULXR_API_IMPL(void) ValueParserWb::IntegerState::takeValue(Value *, bool del)
{
  candel = del;
}


}  // namespace ulxr


