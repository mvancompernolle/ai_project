/**************************************************************************
               ulxr_valueparse_base.cpp  -  parse rpc primitive values
                             -------------------
    begin                : Fri Jan 09 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_valueparse_base.cpp 1154 2009-08-16 09:24:53Z ewald-arnold $

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

#include <ulxmlrpcpp/ulxr_valueparse_base.h>


namespace ulxr {


ULXR_API_IMPL0 ValueParserBase::~ValueParserBase()
{
}


ULXR_API_IMPL(Value) ValueParserBase::getValue() const
{
  ULXR_TRACE(ULXR_PCHAR("ValueParserBase::getValue()"));

//  if (getTopState()->getState() != eNone)
//    cerr << ulxr_i18n("########## abnormal program behaviour: getTopState()->getState() != eNone: ")
//         << getTopState()->getState()
//         << endl;

  Value *v = getTopValueState()->getValue();
  if (v != 0)
    return *v;
  else
    return Value(); // return ulxr::Void()
}


//////////////////////////////////////////////////////////////////////////////
//


ULXR_API_IMPL0 ValueParserBase::ValueState::ValueState (unsigned st)
 : XmlParserBase::ParserState(st)
 , value (0)
 , candel(true)
{
}


ULXR_API_IMPL(void) ValueParserBase::ValueState::takeValue(Value *val, bool del)
{
  ULXR_TRACE(ULXR_PCHAR("ValueParserBase::ValueState::takeValue(Value *)"));
  value = val;
  candel = del;
}


ULXR_API_IMPL(bool) ValueParserBase::ValueState::canDelete() const
{
  return candel;
}


ULXR_API_IMPL(CppString) ValueParserBase::ValueState::getStateName() const
{
  switch (getParserState())
  {
    case XmlParserBase::eNone     : return ULXR_PCHAR("eNone");
    case ValueParserBase::eValue  : return ULXR_PCHAR("eValue");
    case ValueParserBase::eArray  : return ULXR_PCHAR("eArray");
    case ValueParserBase::eData   : return ULXR_PCHAR("eData");
    case ValueParserBase::eStruct : return ULXR_PCHAR("eStruct");
    case ValueParserBase::eMember : return ULXR_PCHAR("eMember");
    case ValueParserBase::eName   : return ULXR_PCHAR("eName");
    case ValueParserBase::eBoolean: return ULXR_PCHAR("eBoolean");
    case ValueParserBase::eInt    : return ULXR_PCHAR("eInt");
    case ValueParserBase::eI4     : return ULXR_PCHAR("eI4");
    case ValueParserBase::eDouble : return ULXR_PCHAR("eDouble");
    case ValueParserBase::eString : return ULXR_PCHAR("eString");
    case ValueParserBase::eBase64 : return ULXR_PCHAR("eBase64");
    case ValueParserBase::eDate   : return ULXR_PCHAR("eDate");

    case ValueParserBase::eValueParserLast: return ULXR_PCHAR("eValueParserLast");
  }

  return ULXR_PCHAR("eUnknown");
}


ULXR_API_IMPL(void) ValueParserBase::ValueState::takeName(const CppString & /* name */ )
{
}


ULXR_API_IMPL(CppString) ValueParserBase::ValueState::getName() const
{
  return name;
}


ULXR_API_IMPL(void) ValueParserBase::ValueState::setName(const CppString &nm)
{
  name = nm;
}


ULXR_API_IMPL(Value*) ValueParserBase::ValueState::getValue() const
{
  return value;
}


//////////////////////////////////////////////////////////////////////////////
//


ULXR_API_IMPL0 ValueParserBase::MemberState::MemberState(unsigned st, Value *val)
  : ValueState(st),
    mem_val(0)
{
  value = val;
}


ULXR_API_IMPL(void) ValueParserBase::MemberState::takeValue(Value *v, bool del)
{
  candel = del;
  ULXR_TRACE(ULXR_PCHAR("ValueParserBase::MemberState::takeValue(Value *)"));
  if (getName().length() != 0) // wait for name, maybe second tag
  {
    value->getStruct()->addMember(getName(), *v);
    delete v;
  }
  else
    mem_val = v;
}


ULXR_API_IMPL(void) ValueParserBase::MemberState::takeName(const CppString &n)
{
  if (mem_val != 0)
  {
    value->getStruct()->addMember(n, *mem_val);
    delete mem_val;
    mem_val = 0;
  }
  else
    setName(n);
}


//////////////////////////////////////////////////////////////////////////////
//

ULXR_API_IMPL0 ValueParserBase::ArrayState::ArrayState(unsigned st)
  : ValueState(st)
{
}


ULXR_API_IMPL(void) ValueParserBase::ArrayState::takeValue(Value *v, bool del)
{
  candel = del;
  ULXR_TRACE(ULXR_PCHAR("ValueParserBase::ArrayState::takeValue(Value *)"));
  if (value == 0)
    value = new Value(Array());

  value->getArray()->addItem(*v);
  delete v;
}


}  // namespace ulxr

