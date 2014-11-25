/***************************************************************************
                  ulxr_value.h  -  values passed by xml-rpc
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_value.cpp 1157 2009-08-29 09:02:31Z ewald-arnold $

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

// upon requst you may also use intrinsic types like bool, int, double, char*
// which will automatically generate their xmlrpc counterparts.
//
// always compile, but #include just upon demand!
//
#define ULXR_USE_INTRINSIC_VALUE_TYPES

// upon requst you may also use the following construct
//    ("second" << i2)
//
// always compile, but #include just upon demand!
//
#define ULXR_STRUCT_MEMBER_FROM_NAME_VALUE


#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <cstdio>
#include <ctime>
#include <vector>

#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_valueparse_wb.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_htmlform_handler.h>

#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
#include <ulxmlrpcpp/ulxr_mutex.h>
#endif


#define ULXR_ASSERT_RPCTYPE(x) \
  if (x != getType() )    \
  { \
    throw ParameterException(ApplicationError, (CppString) \
                           ULXR_PCHAR("Value type mismatch.\n") \
                           ULXR_PCHAR("Expected: ") + ULXR_GET_STRING(#x) + ULXR_PCHAR(".\n") \
                           ULXR_PCHAR("Actually have: ") +getTypeName() + ULXR_PCHAR(".")); \
  }


namespace ulxr {



//////////////////////////////////////////////////////


ULXR_API_IMPL(bool) Value::isVoid() const
{
  return baseVal->isVoid();
}


ULXR_API_IMPL(bool) Value::isBoolean() const
{
  return baseVal->isBoolean();
}


ULXR_API_IMPL(bool) Value::isInteger() const
{
  return baseVal->isInteger();
}


ULXR_API_IMPL(bool) Value::isDouble()   const
{
  return baseVal->isDouble();
}


ULXR_API_IMPL(bool) Value::isArray() const
{
  return baseVal->isArray();
}


ULXR_API_IMPL(bool) Value::isStruct() const
{
  return baseVal->isStruct();
}


ULXR_API_IMPL(bool) Value::isString() const
{
  return baseVal->isString();
}


ULXR_API_IMPL(bool) Value::isBase64() const
{
  return baseVal->isBase64();
}


ULXR_API_IMPL(bool) Value::isDateTime() const
{
  return baseVal->isDateTime();
}


ULXR_API_IMPL(ValueType) Value::getType() const
{
  return baseVal->getType();
}


ULXR_API_IMPL(CppString) Value::getTypeName() const
{
  return baseVal->getTypeName();
}


ULXR_API_IMPL(CppString) Value::getSignature(bool deep) const
{
  CppString s;
  if (baseVal != 0)
    s = baseVal->getSignature(deep);
  return s;
}


ULXR_API_IMPL(CppString) Value::getXml(int indent) const
{
  CppString ret;
  if (baseVal != 0)
    ret = baseVal->getXml(indent);
  return ret;
}


ULXR_API_IMPL(std::string) Value::getWbXml() const
{
  std::string ret;
  if (baseVal != 0)
    ret = baseVal->getWbXml();
  return ret;
}


ULXR_API_IMPL(Struct*) Value::getStruct()
{
  ULXR_ASSERT_RPCTYPE(RpcStruct);
  return structVal;
};


ULXR_API_IMPL(const Struct*) Value::getStruct() const
{
  ULXR_ASSERT_RPCTYPE(RpcStruct);
  return structVal;
};


ULXR_API_IMPL(Array*) Value::getArray()
{
  ULXR_ASSERT_RPCTYPE(RpcArray);
  return arrayVal;
};


ULXR_API_IMPL(const Array*) Value::getArray() const
{
  ULXR_ASSERT_RPCTYPE(RpcArray);
  return arrayVal;
};


ULXR_API_IMPL0 Value::operator Boolean& ()
{
  ULXR_ASSERT_RPCTYPE(RpcBoolean);
  return *boolVal;
}


ULXR_API_IMPL0 Value::operator const Boolean& () const
{
  ULXR_ASSERT_RPCTYPE(RpcBoolean);
  return *boolVal;
}


ULXR_API_IMPL0 Value::operator Integer& ()
{
  ULXR_ASSERT_RPCTYPE(RpcInteger);
  return *intVal;
}


ULXR_API_IMPL0 Value::operator const Integer& () const
{
  ULXR_ASSERT_RPCTYPE(RpcInteger);
  return *intVal;
}


ULXR_API_IMPL0 Value::operator Double& ()
{
  ULXR_ASSERT_RPCTYPE(RpcDouble);
  return *doubleVal;
}


ULXR_API_IMPL0 Value::operator const Double& () const
{
  ULXR_ASSERT_RPCTYPE(RpcDouble);
  return *doubleVal;
}


ULXR_API_IMPL0 Value::operator Array& ()
{
  ULXR_ASSERT_RPCTYPE(RpcArray);
  return *arrayVal;
}


ULXR_API_IMPL0 Value::operator const Array& () const
{
  ULXR_ASSERT_RPCTYPE(RpcArray);
  return *arrayVal;
}


ULXR_API_IMPL0 Value::operator Struct& ()
{
  ULXR_ASSERT_RPCTYPE(RpcStruct);
  return *structVal;
}


ULXR_API_IMPL0 Value::operator const Struct& () const
{
  ULXR_ASSERT_RPCTYPE(RpcStruct);
  return *structVal;
}


ULXR_API_IMPL0 Value::operator RpcString& ()
{
  ULXR_ASSERT_RPCTYPE(RpcStrType);
  return *stringVal;
}


ULXR_API_IMPL0 Value::operator const RpcString& () const
{
  ULXR_ASSERT_RPCTYPE(RpcStrType);
  return *stringVal;
}


ULXR_API_IMPL0 Value::operator Base64& ()
{
  ULXR_ASSERT_RPCTYPE(RpcBase64);
  return *base64Val;
}


ULXR_API_IMPL0 Value::operator const Base64& () const
{
  ULXR_ASSERT_RPCTYPE(RpcBase64);
  return *base64Val;
}


ULXR_API_IMPL0 Value::operator DateTime& ()
{
  ULXR_ASSERT_RPCTYPE(RpcDateTime);
  return *dateVal;
}


ULXR_API_IMPL0 Value::operator const DateTime& () const
{
  ULXR_ASSERT_RPCTYPE(RpcDateTime);
  return *dateVal;
}


ULXR_API_IMPL0 Value::Value ()
{
   voidVal = new Void();
}


ULXR_API_IMPL0 Value::Value (const Value &val)
{
   baseVal = val.baseVal->cloneValue();
}


ULXR_API_IMPL0 Value::Value (const Boolean &val)
{
   boolVal = new Boolean(val);
}


ULXR_API_IMPL0 Value::Value (const Integer &val)
{
   intVal = new Integer(val);
}


ULXR_API_IMPL0 Value::Value (const Double &val)
{
   doubleVal = new Double(val);
}


ULXR_API_IMPL0 Value::Value (const Array &val)
{
   arrayVal = new Array(val);
}


ULXR_API_IMPL0 Value::Value (const Struct &val)
{
   structVal = new Struct(val);
}


ULXR_API_IMPL0 Value::Value (const RpcString &val)
{
   stringVal = new RpcString(val);
}


ULXR_API_IMPL0 Value::Value (const Base64 &val)
{
   base64Val = new Base64(val);
}



ULXR_API_IMPL0 Value::Value (const DateTime &val)
{
   dateVal = new DateTime(val);
}


#ifdef ULXR_USE_INTRINSIC_VALUE_TYPES


ULXR_API_IMPL0 Value::Value (const char *val)
{
   stringVal = new RpcString(val);
}


#ifdef ULXR_UNICODE

ULXR_API_IMPL0 Value::Value (const wchar_t *val)
{
  Cpp16BitString wstr = val;
  stringVal = new RpcString(unicodeToUtf8(wstr));
}

#endif


ULXR_API_IMPL0 Value::Value (const double &val)
{
   doubleVal = new Double(val);
}


ULXR_API_IMPL0 Value::Value (const int &val)
{
   intVal = new Integer(val);
}


ULXR_API_IMPL0 Value::Value (const bool &val)
{
   boolVal = new Boolean(val);
}


#endif


ULXR_API_IMPL(Value &) Value::operator= (const Value &val)
{
   ValueBase *temp = val.baseVal->cloneValue();
   delete baseVal;
   baseVal = temp;
   return *this;
}


ULXR_API_IMPL0 Value::~Value ()

{
  delete baseVal;
  baseVal = 0;
}


//////////////////////////////////////////////////////


ULXR_API_IMPL0 ValueBase::ValueBase (ValueType t)
     : type(t)
{
}


ULXR_API_IMPL(ValueType) ValueBase::getType() const
{
  return type;
}


ULXR_API_IMPL0 ValueBase::~ValueBase ()
{
}


ULXR_API_IMPL(bool) ValueBase::isVoid() const
{
  return type == RpcVoid;
}


ULXR_API_IMPL(bool) ValueBase::isBoolean() const
{
  return type == RpcBoolean;
}


ULXR_API_IMPL(bool) ValueBase::isInteger() const
{
  return type == RpcInteger;
}


ULXR_API_IMPL(bool) ValueBase::isDouble() const
{
  return type == RpcDouble;
}


ULXR_API_IMPL(bool) ValueBase::isArray() const
{
  return type == RpcArray;
}


ULXR_API_IMPL(bool) ValueBase::isStruct() const
{
  return type == RpcStruct;
}


ULXR_API_IMPL(bool) ValueBase::isString() const
{
  return type == RpcStrType;
}


ULXR_API_IMPL(bool) ValueBase::isBase64() const
{
  return type == RpcBase64;
}


ULXR_API_IMPL(bool) ValueBase::isDateTime() const
{
  return type == RpcDateTime;
}


ULXR_API_IMPL(CppString) ValueBase::getTypeName() const
{
  CppString ret = ULXR_PCHAR("RpcValue");
  switch(getType() )
  {
     case RpcVoid:
       ret = ULXR_PCHAR("RpcVoid");
     break;

     case RpcInteger:
       ret = ULXR_PCHAR("RpcInteger");
     break;

     case RpcDouble:
       ret = ULXR_PCHAR("RpcDouble");
     break;

     case RpcBoolean:
       ret = ULXR_PCHAR("RpcBoolean");
     break;

     case RpcStrType:
       ret = ULXR_PCHAR("RpcString");
     break;

     case RpcDateTime:
       ret = ULXR_PCHAR("RpcDateTime");
     break;

     case RpcBase64:
       ret = ULXR_PCHAR("RpcBase64");
     break;

     case RpcArray:
       ret = ULXR_PCHAR("RpcArray");
     break;

     case RpcStruct:
       ret = ULXR_PCHAR("RpcStruct");
     break;
  }
  return ret;
}


//////////////////////////////////////////////////////


ULXR_API_IMPL0 Void::Void ()
 : ValueBase(RpcVoid)
{
}


ULXR_API_IMPL0 Void::~Void ()
{
}


ULXR_API_IMPL(ValueBase *) Void::cloneValue() const
{
  ULXR_ASSERT_RPCTYPE(RpcVoid);
  return new Void(*this);
}


ULXR_API_IMPL(CppString) Void::getSignature(bool /*deep*/) const
{
  ULXR_ASSERT_RPCTYPE(RpcVoid);
  return getValueName();
}


ULXR_API_IMPL(CppString) Void::getValueName()
{
  return ULXR_PCHAR("void");
}


ULXR_API_IMPL(std::string) Void::getWbXml() const
{
  ULXR_ASSERT_RPCTYPE(RpcVoid);
  return "";
}


ULXR_API_IMPL(CppString) Void::getXml(int /*indent*/) const
{
  ULXR_ASSERT_RPCTYPE(RpcVoid);
  return ULXR_PCHAR("");
}


//////////////////////////////////////////////////////


ULXR_API_IMPL0 Boolean::Boolean ()
 : ValueBase(RpcBoolean)
{
}


ULXR_API_IMPL0 Boolean::Boolean (bool b)
 : ValueBase(RpcBoolean), val(b)
{
}


ULXR_API_IMPL0 Boolean::~Boolean ()
{
}


ULXR_API_IMPL0 Boolean::Boolean (const CppString &s)
  : ValueBase(RpcBoolean)
{
  if (   s == ULXR_PCHAR("true") || s == ULXR_PCHAR("TRUE")
      || s == ULXR_PCHAR("1") || s == ULXR_PCHAR("on") || s == ULXR_PCHAR("ON"))
    val = true;
  else
    val = false;
}


ULXR_API_IMPL(ValueBase *) Boolean::cloneValue() const
{
  ULXR_ASSERT_RPCTYPE(RpcBoolean);
  return new Boolean(*this);
}


ULXR_API_IMPL(CppString) Boolean::getSignature(bool /*deep*/) const
{
  ULXR_ASSERT_RPCTYPE(RpcBoolean);
  return getValueName();
}


ULXR_API_IMPL(CppString) Boolean::getValueName()
{
  return ULXR_PCHAR("bool");
}


ULXR_API_IMPL(std::string) Boolean::getWbXml() const
{
  ULXR_ASSERT_RPCTYPE(RpcBoolean);
  std::string s;

  s = ValueParserWb::wbToken_Value;
  s += ValueParserWb::wbToken_Boolean;

  if (val)
    s += getWbXmlString(ULXR_PCHAR("1"));
  else
    s += getWbXmlString(ULXR_PCHAR("0"));

  s += WbXmlParser::wbxml_END;
  s += WbXmlParser::wbxml_END;
  return s;
}


ULXR_API_IMPL(CppString) Boolean::getXml(int indent) const
{
  ULXR_ASSERT_RPCTYPE(RpcBoolean);
  CppString s = getXmlIndent(indent);
  s += ULXR_PCHAR("<value><boolean>");

  if (val)
    s += ULXR_PCHAR("1"); // "true"
  else
    s += ULXR_PCHAR("0"); // "false"

  s += ULXR_PCHAR("</boolean></value>");
  return s;
}


ULXR_API_IMPL(bool) Boolean::getBoolean () const
{
  ULXR_ASSERT_RPCTYPE(RpcBoolean);
  return val;
}


ULXR_API_IMPL(void) Boolean::setBoolean(const bool newval)
{
  ULXR_ASSERT_RPCTYPE(RpcBoolean);
  val = newval;
}


//////////////////////////////////////////////////////


ULXR_API_IMPL0 Integer::Integer ()
  : ValueBase(RpcInteger)
{
}


ULXR_API_IMPL0 Integer::Integer (int i)
 : ValueBase(RpcInteger), val(i)
{
}


ULXR_API_IMPL0 Integer::~Integer ()
{
}


ULXR_API_IMPL0 Integer::Integer (const CppString &s)
  : ValueBase(RpcInteger)
{
//  val = ulxr_atoi(s.c_str() );
  char *endp;
  val = strtol(getLatin1(s).c_str(), &endp, 10 );
//  if (*endp != 0)
// FIXME
}


ULXR_API_IMPL(ValueBase *) Integer::cloneValue() const
{
  ULXR_ASSERT_RPCTYPE(RpcInteger);
  return new Integer(*this);
}



ULXR_API_IMPL(CppString) Integer::getSignature(bool /*deep*/) const
{
  ULXR_ASSERT_RPCTYPE(RpcInteger);
  return getValueName();
}


ULXR_API_IMPL(CppString) Integer::getValueName()
{
  return ULXR_PCHAR("int");
}


ULXR_API_IMPL(std::string) Integer::getWbXml() const
{
  ULXR_ASSERT_RPCTYPE(RpcInteger);
  std::string s;
  s = (char)ValueParserWb::wbToken_Value;
  s += (char)ValueParserWb::wbToken_I4;

  char buff[100];
  if (ulxr_snprintf(buff, sizeof(buff), "%d", val) >= (int) sizeof(buff))
    throw RuntimeException(ApplicationError,
                           ulxr_i18n(ULXR_PCHAR("Buffer for conversion too small in Integer::getWbXml() ")));

  s += getWbXmlString(ULXR_GET_STRING(buff));
  s += (char)WbXmlParser::wbxml_END;
  s += (char)WbXmlParser::wbxml_END;
  return s;
}


ULXR_API_IMPL(CppString) Integer::getXml(int indent) const
{
  ULXR_ASSERT_RPCTYPE(RpcInteger);
  CppString s = getXmlIndent(indent);
  s += ULXR_PCHAR("<value><i4>");

  char buff[100];
  if (ulxr_snprintf(buff, sizeof(buff), "%d", val) >= (int) sizeof(buff))
    throw RuntimeException(ApplicationError,
                           ulxr_i18n(ULXR_PCHAR("Buffer for conversion too small in Integer::getXml() ")));

  s += ULXR_GET_STRING(buff);
  s += ULXR_PCHAR("</i4></value>");
  return s;
}


ULXR_API_IMPL(int ) Integer::getInteger () const
{
  ULXR_ASSERT_RPCTYPE(RpcInteger);
  return val;
}


ULXR_API_IMPL(void) Integer::setInteger(const int newval)
{
  ULXR_ASSERT_RPCTYPE(RpcInteger);
  val = newval;
}


//////////////////////////////////////////////////////


bool Double::scientific = false;


ULXR_API_IMPL0 Double::Double ()
  : ValueBase(RpcDouble)
{
}


ULXR_API_IMPL0 Double::Double (double d)
  : ValueBase(RpcDouble)
  , val(d)
{
}


ULXR_API_IMPL0 Double::~Double ()
{
}


ULXR_API_IMPL0 Double::Double (const CppString &s)
  : ValueBase(RpcDouble)
{
  val = atof(getLatin1(s).c_str() );
}


ULXR_API_IMPL(ValueBase *) Double::cloneValue() const
{
  ULXR_ASSERT_RPCTYPE(RpcDouble);
  return new Double(*this);
}


ULXR_API_IMPL(CppString) Double::getSignature(bool /*deep*/) const
{
  ULXR_ASSERT_RPCTYPE(RpcDouble);
  return getValueName();
}


ULXR_API_IMPL(CppString) Double::getValueName()
{
  return ULXR_PCHAR("double");
}


ULXR_API_IMPL(bool) Double::setScientificMode(bool in_scientific)
{
  bool prev = scientific;
  scientific = in_scientific;
  return prev;
}


ULXR_API_IMPL(std::string) Double::getWbXml() const
{
  ULXR_ASSERT_RPCTYPE(RpcDouble);
  std::string s;
  char buff[1000];

  unsigned used;
  if (scientific)
    used = ulxr_snprintf(buff, sizeof(buff), "%g", val);
  else
    used = ulxr_snprintf(buff, sizeof(buff), "%f", val);

  if (used >= sizeof(buff))
      throw RuntimeException(ApplicationError,
                           ulxr_i18n(ULXR_PCHAR("Buffer for conversion too small in Double::getWbXml ")));

  s = ValueParserWb::wbToken_Value;
  s += ValueParserWb::wbToken_Double;
  s += getWbXmlString(ULXR_GET_STRING(buff));
  s += (char)WbXmlParser::wbxml_END;
  s += (char)WbXmlParser::wbxml_END;
  return s;
}


ULXR_API_IMPL(CppString) Double::getXml(int indent) const
{
  ULXR_ASSERT_RPCTYPE(RpcDouble);
  CppString s = getXmlIndent(indent);
  s += ULXR_PCHAR("<value><double>");
  char buff[1000];

  unsigned used;
  if (scientific)
    used = ulxr_snprintf(buff, sizeof(buff), "%g", val);
  else
    used = ulxr_snprintf(buff, sizeof(buff), "%f", val);

  if (used >= sizeof(buff))
    throw RuntimeException(ApplicationError,
                           ulxr_i18n(ULXR_PCHAR("Buffer for conversion too small in Double::getXml ")));

  s += ULXR_GET_STRING(buff);
  s += ULXR_PCHAR("</double></value>");
  return s;
}


ULXR_API_IMPL(double) Double::getDouble () const
{
  ULXR_ASSERT_RPCTYPE(RpcDouble);
  return val;
}


ULXR_API_IMPL(void) Double::setDouble(const double newval)
{
  ULXR_ASSERT_RPCTYPE(RpcDouble);
  val = newval;
}


//////////////////////////////////////////////////////


ULXR_API_IMPL0 RpcString::RpcString ()
  : ValueBase(RpcStrType)
{
}


ULXR_API_IMPL0 RpcString::RpcString (const Cpp8BitString &s)
 : ValueBase(RpcStrType)
{
  setString(s);
}


ULXR_API_IMPL0 RpcString::~RpcString ()
{
}


#ifdef ULXR_UNICODE

ULXR_API_IMPL0 RpcString::RpcString (const Cpp16BitString &s)
  : ValueBase(RpcStrType)
{
  setString(s);
}

#endif

ULXR_API_IMPL(ValueBase *) RpcString::cloneValue() const
{
  ULXR_ASSERT_RPCTYPE(RpcStrType);
  return new RpcString(*this);
}


ULXR_API_IMPL(CppString) RpcString::getSignature(bool /*deep*/) const
{
  ULXR_ASSERT_RPCTYPE(RpcStrType);
  return getValueName();
}


ULXR_API_IMPL(CppString) RpcString::getValueName()
{
  return ULXR_PCHAR("string");
}


ULXR_API_IMPL(std::string) RpcString::getWbXml() const
{
  ULXR_ASSERT_RPCTYPE(RpcStrType);
  std::string s;
  s = ValueParserWb::wbToken_Value;
  s += ValueParserWb::wbToken_String;
  s += getWbXmlString(val);
  s += (char)WbXmlParser::wbxml_END;
  s += (char)WbXmlParser::wbxml_END;
  return s;
}


ULXR_API_IMPL(CppString) RpcString::getXml(int indent) const
{
  ULXR_ASSERT_RPCTYPE(RpcStrType);
  CppString s = getXmlIndent(indent);
  s += ULXR_PCHAR("<value><string>");
  s += xmlEscape(val);
  s += ULXR_PCHAR("</string></value>");
  return s;
}


ULXR_API_IMPL(CppString) RpcString::getString () const
{
  ULXR_ASSERT_RPCTYPE(RpcStrType);
  return val;
}


#ifdef ULXR_UNICODE

ULXR_API_IMPL(Cpp16BitString) RpcString::getUnicodeString () const
{
  ULXR_ASSERT_RPCTYPE(RpcStrType);
#ifdef ULXR_UNICODE
  return val;
#else
  return utf8ToUnicode(val);
#endif
}

#endif

#if (defined(HAVE_ICONV_H) || defined(HAVE_ICONV)) && !defined (ULXR_UNICODE)

ULXR_API_IMPL(CppString) RpcString::getString (const char *encoding) const
{
  return utf8ToEncoding(val, encoding);
}


ULXR_API_IMPL(void) RpcString::setString(const Cpp8BitString &newval, const char *encoding)
{
  val = encodingToUtf8(newval, encoding);
}

#endif


ULXR_API_IMPL(void) RpcString::setString(const Cpp8BitString &newval)
{
  ULXR_ASSERT_RPCTYPE(RpcStrType);
#ifdef ULXR_UNICODE
  val = getUnicode(newval);
#else
  val = newval;
#endif
}


#ifdef ULXR_UNICODE

ULXR_API_IMPL(void) RpcString::setString(const Cpp16BitString& newval)
{
  ULXR_ASSERT_RPCTYPE(RpcStrType);
  val = newval;
}

#endif

//////////////////////////////////////////////////////


ULXR_API_IMPL0 Base64::Base64 ()
  : ValueBase(RpcBase64)
{
}


ULXR_API_IMPL0 Base64::Base64 (const CppString &s)
  : ValueBase(RpcBase64)
{
  setString(s);
}


ULXR_API_IMPL0 Base64::~Base64 ()
{
}


ULXR_API_IMPL(CppString) Base64::getBase64() const
{
  return val;
}


ULXR_API_IMPL(void) Base64::setBase64(const CppString s)
{
  val = s;
}


ULXR_API_IMPL(ValueBase *) Base64::cloneValue() const
{
  ULXR_ASSERT_RPCTYPE(RpcBase64);
  return new Base64(*this);
}


ULXR_API_IMPL(CppString) Base64::getSignature(bool /*deep*/) const
{
  ULXR_ASSERT_RPCTYPE(RpcBase64);
  return getValueName();
}


ULXR_API_IMPL(CppString) Base64::getValueName()
{
  return ULXR_PCHAR("base64");
}


ULXR_API_IMPL(CppString) Base64::getXml(int indent) const
{
  ULXR_ASSERT_RPCTYPE(RpcBase64);
  CppString s = getXmlIndent(indent);
  s += ULXR_PCHAR("<value><base64>");
  s += val;
  s += ULXR_PCHAR("</base64></value>");
  return s;
}


ULXR_API_IMPL(std::string) Base64::getWbXml() const
{
  ULXR_ASSERT_RPCTYPE(RpcBase64);
  std::string s;
  s += ValueParserWb::wbToken_Value;
  s += ValueParserWb::wbToken_Base64;
  s += getWbXmlString(val);
  s += (char)WbXmlParser::wbxml_END;
  s += (char)WbXmlParser::wbxml_END;
  return s;
}


ULXR_API_IMPL(CppString) Base64::getString () const
{
  ULXR_ASSERT_RPCTYPE(RpcBase64);
  return decodeBase64(val);
}


ULXR_API_IMPL(void) Base64::setString(const CppString &newval)
{
  ULXR_ASSERT_RPCTYPE(RpcBase64);
  val = encodeBase64(newval);
}


//////////////////////////////////////////////////////


ULXR_API_IMPL0 DateTime::DateTime ()
  : ValueBase(RpcDateTime)
{
}


ULXR_API_IMPL0 DateTime::DateTime (const CppString &s)
 : ValueBase(RpcDateTime)
 , val(s)
{
}


ULXR_API_IMPL0 DateTime::DateTime (const ulxr_time_t &tm)
  : ValueBase(RpcDateTime)
{
  setDateTime(tm);
}


ULXR_API_IMPL0 DateTime::~DateTime ()
{
}


ULXR_API_IMPL(ValueBase *) DateTime::cloneValue() const
{
  ULXR_ASSERT_RPCTYPE(RpcDateTime);
  return new DateTime(*this);
}


ULXR_API_IMPL(CppString) DateTime::getSignature(bool /*deep*/) const
{
  ULXR_ASSERT_RPCTYPE(RpcDateTime);
  return getValueName();
}


ULXR_API_IMPL(CppString) DateTime::getValueName()
{
  return ULXR_PCHAR("dateTime.iso8601");
}


ULXR_API_IMPL(std::string) DateTime::getWbXml() const
{
  ULXR_ASSERT_RPCTYPE(RpcDateTime);
  std::string s;
  s += ValueParserWb::wbToken_Value;
  s += ValueParserWb::wbToken_Date;
  s += getWbXmlString(val);
  s += (char)WbXmlParser::wbxml_END;
  s += (char)WbXmlParser::wbxml_END;
  return s;
}


ULXR_API_IMPL(CppString) DateTime::getXml(int indent) const
{
  ULXR_ASSERT_RPCTYPE(RpcDateTime);
  CppString s = getXmlIndent(indent);
  s += ULXR_PCHAR("<value><dateTime.iso8601>");
  s += val;
  s += ULXR_PCHAR("</dateTime.iso8601></value>");
  return s;
}


ULXR_API_IMPL(CppString) DateTime::getDateTime () const
{
  ULXR_ASSERT_RPCTYPE(RpcDateTime);
  return val;
}


namespace {

static CppString padded(unsigned num)
{
  CppString s;
  if (num < 10)
    s += L'0';
  s += HtmlFormHandler::makeNumber(num);
  return s;
}

}


ULXR_API_IMPL(void) DateTime::setDateTime (const ulxr_time_t &tmt,
                                        bool add_dash /* =false */,
                                        bool add_colon /* =true */)
{
#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
  Mutex::Locker lock(localtimeMutex);
#endif

  CppString s;
  ulxr_tm * ltm = ulxr_localtime(&tmt);
  s += padded(ltm->tm_year+1900);
  if (add_dash)
    s += '-';
  s += padded(ltm->tm_mon+1);
  if (add_dash)
    s += '-';
  s += padded(ltm->tm_mday);
  s += ULXR_PCHAR("T");
  s += padded(ltm->tm_hour);
  if (add_colon)
    s += ':';
  s += padded(ltm->tm_min);
  if (add_colon)
    s += ':';
  s += padded(ltm->tm_sec);
  setDateTime(s);
}


ULXR_API_IMPL(void) DateTime::setDateTime(const CppString &newval)
{
  ULXR_ASSERT_RPCTYPE(RpcDateTime);
  val = newval;
}


//////////////////////////////////////////////////////


ULXR_API_IMPL0 Array::Array ()
  : ValueBase(RpcArray)
{
}


ULXR_API_IMPL0 Array::~Array ()
{
}


ULXR_API_IMPL(unsigned) Array::size() const
{
  return values.size();
}


ULXR_API_IMPL(void) Array::clear()
{
  values.clear();
}


ULXR_API_IMPL(ValueBase *) Array::cloneValue() const
{
  ULXR_ASSERT_RPCTYPE(RpcArray);
  return new Array(*this);
}


ULXR_API_IMPL(CppString) Array::getSignature(bool deep) const
{
  if (!deep)
    return getValueName();


  ULXR_ASSERT_RPCTYPE(RpcArray);
  CppString s;
  if (values.size() == 0)
    return ULXR_PCHAR("[]");

  s += ULXR_CHAR('[');
  bool comma = values.size() >= 1;
  for (unsigned i = 0; i < values.size(); ++i)
  {
    if (comma && i != 0)
      s += ULXR_CHAR(',');
    s += values[i].getSignature();
  }
  s += ULXR_CHAR(']');

  return s;
}


ULXR_API_IMPL(CppString) Array::getValueName()
{
  return ULXR_PCHAR("array");
}


ULXR_API_IMPL(std::string) Array::getWbXml() const
{
  ULXR_ASSERT_RPCTYPE(RpcArray);
  std::string s;
  s = ValueParserWb::wbToken_Value;
  s += ValueParserWb::wbToken_Array;
  s += ValueParserWb::wbToken_Data;

  for (std::vector<Value>::const_iterator
         it = values.begin(); it != values.end(); ++it)
  {
    s += (*it).getWbXml();
  }

  s += WbXmlParser::wbxml_END;
  s += WbXmlParser::wbxml_END;
  s += WbXmlParser::wbxml_END;
  return s;
}


ULXR_API_IMPL(CppString) Array::getXml(int indent) const
{
  ULXR_ASSERT_RPCTYPE(RpcArray);
  CppString ind = getXmlIndent(indent);
  CppString ind1 = getXmlIndent(indent+1);
  CppString ind2 = getXmlIndent(indent+2);
  CppString s = ind + ULXR_PCHAR("<value>") + getXmlLinefeed();
  s += ind1 + ULXR_PCHAR("<array>") + getXmlLinefeed();
  s += ind2 + ULXR_PCHAR("<data>") + getXmlLinefeed();

  for (std::vector<Value>::const_iterator
         it = values.begin(); it != values.end(); ++it)
  {
    s += (*it).getXml(indent+3) + getXmlLinefeed();
  }

  s += ind2 + ULXR_PCHAR("</data>") + getXmlLinefeed();
  s += ind1 + ULXR_PCHAR("</array>")+ getXmlLinefeed();
  s += ind + ULXR_PCHAR("</value>");
  return s;
}


ULXR_API_IMPL(void) Array::addItem(const Value &item)
{
  values.push_back(item);
}


ULXR_API_IMPL(void) Array::setItem(unsigned idx, const Value &item)
{
  if (idx < values.size() )
    values[idx] = item;
}


ULXR_API_IMPL(Value) Array::getItem(unsigned idx)
{
  if (idx < values.size() )
    return values[idx];
  else
   return Value(RpcString());   // FIXME: exception
}


//////////////////////////////////////////////////////


ULXR_API_IMPL0 Struct::Member::Member(const CppString &str_, const Value &t_)
 : str(str_)
 , t(t_)
{
}


const CppString & Struct::Member::getName() const
{
  return str;
}


const Value & Struct::Member::getValue() const
{
  return t;
}


//////////////////////////////////////////////////////


ULXR_API_IMPL0 Struct::Struct ()
 : ValueBase(RpcStruct)
{
}


ULXR_API_IMPL0 Struct::~Struct ()
{
}


ULXR_API_IMPL(ValueBase *) Struct::cloneValue() const
{
  ULXR_ASSERT_RPCTYPE(RpcStruct);
  return new Struct(*this);
}


ULXR_API_IMPL(void) Struct::clear()
{
  val.clear();
}


ULXR_API_IMPL(unsigned) Struct::size() const
{
  return val.size() ;
}


ULXR_API_IMPL(const Struct::Members&) Struct::getAllMembers() const
{
  return val;
}


ULXR_API_IMPL(CppString) Struct::getSignature(bool deep) const
{
  if (!deep)
    return getValueName();

  ULXR_ASSERT_RPCTYPE(RpcStruct);
  CppString s;
  if (val.size() == 0)
    return ULXR_PCHAR("{}");

  if (val.size() > 1)
    s += ULXR_CHAR('{');

  for (Members::const_iterator it = val.begin(); it != val.end(); ++it)
  {
    s += ULXR_CHAR('{');
    s += (*it).first;
    s += ULXR_CHAR(',');
    s += (*it).second.getSignature();
    s += ULXR_CHAR('}');
  }

  if (val.size() > 1)
    s += ULXR_CHAR('}');

  return s;
}


ULXR_API_IMPL(CppString) Struct::getValueName()
{
  return ULXR_PCHAR("struct");
}


ULXR_API_IMPL(CppString) Struct::getXml(int indent) const
{
  ULXR_ASSERT_RPCTYPE(RpcStruct);
  CppString ind = getXmlIndent(indent);
  CppString ind1 = getXmlIndent(indent+1);
  CppString ind2 = getXmlIndent(indent+2);
  CppString ind3 = getXmlIndent(indent+3);
  CppString s = ind + ULXR_PCHAR("<value>")+ getXmlLinefeed();
  s += ind1 + ULXR_PCHAR("<struct>") + getXmlLinefeed();

  for (Members::const_iterator it = val.begin(); it != val.end(); ++it)
  {
    s += ind2 + ULXR_PCHAR("<member>") + getXmlLinefeed();
    s += ind3 + ULXR_PCHAR("<name>") + (*it).first + ULXR_PCHAR("</name>") + getXmlLinefeed();
    s += (*it).second.getXml(indent+3) + getXmlLinefeed();
    s += ind2 + ULXR_PCHAR("</member>") + getXmlLinefeed();
  }

  s += ind1 + ULXR_PCHAR("</struct>") + getXmlLinefeed();
  s += ind + ULXR_PCHAR("</value>");
  return s;
}


ULXR_API_IMPL(std::string) Struct::getWbXml() const
{
  ULXR_ASSERT_RPCTYPE(RpcStruct);
  std::string s;
  s = ValueParserWb::wbToken_Value;
  s += ValueParserWb::wbToken_Struct;

  for (Members::const_iterator it = val.begin(); it != val.end(); ++it)
  {
    s += ValueParserWb::wbToken_Member;
    s += ValueParserWb::wbToken_Name;
    s += getWbXmlString((*it).first);
    s += (char)WbXmlParser::wbxml_END;
    s += (*it).second.getWbXml();
    s += (char)WbXmlParser::wbxml_END;
  }

  s += WbXmlParser::wbxml_END;
  s += WbXmlParser::wbxml_END;
  return s;
}


ULXR_API_IMPL(void) Struct::addMember(const CppString &name, const Value &item)
{
  ULXR_TRACE("Struct::addMember(string, Value)");
  ULXR_ASSERT_RPCTYPE(RpcStruct);
  val.insert(Member_pair(name, item));
}


ULXR_API_IMPL(bool) Struct::hasMember(const CppString &name) const
{
  ULXR_ASSERT_RPCTYPE(RpcStruct);
  return val.find(name) != val.end();
}


ULXR_API_IMPL(Value) Struct::getMember(const CppString &name) const
{
  ULXR_ASSERT_RPCTYPE(RpcStruct);
  Members::const_iterator it = val.find(name);
  if (it  == val.end())
    throw RuntimeException(ApplicationError,
                           ulxr_i18n(ULXR_PCHAR("Attempt to get unknown Struct member: ")) +name);

  return (*it).second;
}


ULXR_API_IMPL(std::vector<CppString>) Struct::getMemberNames() const
{
  std::vector<CppString> names;
  for (Members::const_iterator it = val.begin(); it != val.end(); ++it)
     names.push_back((*it).first);
  return names;
}


#ifdef ULXR_STRUCT_MEMBER_FROM_NAME_VALUE

ULXR_API_IMPL(Struct::Member) operator<< (const CppString &str, const Value &t)
{
  ULXR_TRACE("Struct::Member operator<< (string, T)");
  return Struct::Member(str, t);
}

#endif

ULXR_API_IMPL(Struct::Member) make_member (const CppString &str, const Value &t)
{
  ULXR_TRACE(ULXR_PCHAR("Member operator<< (string, T)"));
  return Struct::Member(str, t);
}


ULXR_API_IMPL(Struct) & operator<< (Struct &st, const Struct::Member &k)
{
  ULXR_TRACE(ULXR_PCHAR("operator<<(Struct, Struct::Member)"));
  st.addMember(k.getName(), k.getValue());
  return st;
}


};  // namespace ulxr

