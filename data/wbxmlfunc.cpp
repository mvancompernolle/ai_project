/***************************************************************************
                     wbxmlfunc.cpp  -  test suite for wbxml-rpc
                             -------------------
    begin                : Fri Jan 09 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: wbxmlfunc.cpp 990 2007-07-14 15:00:39Z ewald-arnold $

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

#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <iostream>
#include <map>
#include <cstdlib>
#include <string.h>
#include <cstdio>

#include <ulxmlrpcpp/ulxr_except.h>

#include <ulxmlrpcpp/ulxr_htmlform_handler.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_call.h>
#include <ulxmlrpcpp/ulxr_response.h>

#include <ulxmlrpcpp/ulxr_valueparse_wb.h>
#include <ulxmlrpcpp/ulxr_callparse_wb.h>
#include <ulxmlrpcpp/ulxr_responseparse_wb.h>

#define MIB_Enum_UTF8      (char) 106
#define MIB_Enum_LATIN1    (char) 4

#ifdef ULXR_UNICODE
#define WB_PROLOG              0x01,  \
                               0x01,  \
                               MIB_Enum_UTF8, \
                               0x00
#else
#define WB_PROLOG              0x01, \
                               0x01, \
                               MIB_Enum_LATIN1, \
                               0x00
#endif

const char emptyRespPattern[]    = { WB_PROLOG,
                                     ulxr::MethodResponseParserWb::wbToken_MethodResponse,
                                      ulxr::MethodResponseParserWb::wbToken_Params,
                                      ulxr::WbXmlParser::wbxml_END,
                                     ulxr::WbXmlParser::wbxml_END
                                   };

const char emptyArrayPattern[]   = { WB_PROLOG,
                                     ulxr::MethodResponseParserWb::wbToken_MethodResponse,
                                      ulxr::MethodResponseParserWb::wbToken_Params,
                                       ulxr::MethodResponseParserWb::wbToken_Param,
                                        ulxr::MethodResponseParserWb::wbToken_Value,
                                         ulxr::MethodResponseParserWb::wbToken_Array,
                                          ulxr::MethodResponseParserWb::wbToken_Data,
                                          ulxr::WbXmlParser::wbxml_END,
                                         ulxr::WbXmlParser::wbxml_END,
                                        ulxr::WbXmlParser::wbxml_END,
                                       ulxr::WbXmlParser::wbxml_END,
                                      ulxr::WbXmlParser::wbxml_END,
                                     ulxr::WbXmlParser::wbxml_END
                                   };

const char emptyStructPattern[]  = { WB_PROLOG,
                                     ulxr::MethodResponseParserWb::wbToken_MethodResponse,
                                      ulxr::MethodResponseParserWb::wbToken_Params,
                                       ulxr::MethodResponseParserWb::wbToken_Param,
                                        ulxr::MethodResponseParserWb::wbToken_Value,
                                         ulxr::MethodResponseParserWb::wbToken_Struct,
                                         ulxr::WbXmlParser::wbxml_END,
                                        ulxr::WbXmlParser::wbxml_END,
                                       ulxr::WbXmlParser::wbxml_END,
                                      ulxr::WbXmlParser::wbxml_END,
                                     ulxr::WbXmlParser::wbxml_END
                                   };

const char int1Pattern[]   = { WB_PROLOG,
                               ulxr::ValueParserWb::wbToken_Value,
                                ulxr::ValueParserWb::wbToken_Int,
                                 ulxr::WbXmlParser::wbxml_STR_I, '1', '2', '3', 0,
                                ulxr::WbXmlParser::wbxml_END,
                               ulxr::WbXmlParser::wbxml_END
                             };

const char int2Pattern[]   = { WB_PROLOG,
                               ulxr::ValueParserWb::wbToken_Value,
                                ulxr::ValueParserWb::wbToken_I4,
                                 ulxr::WbXmlParser::wbxml_STR_I, '1', '2', '3', 0,
                                ulxr::WbXmlParser::wbxml_END,
                               ulxr::WbXmlParser::wbxml_END
                             };

const char boolPattern[]   = { WB_PROLOG,
                               ulxr::ValueParserWb::wbToken_Value,
                                ulxr::ValueParserWb::wbToken_Boolean,
                                 ulxr::WbXmlParser::wbxml_STR_I, 'f', 'a', 'l', 's', 'e', 0,
                                ulxr::WbXmlParser::wbxml_END,
                               ulxr::WbXmlParser::wbxml_END
                             };

const char doublePattern[] = { WB_PROLOG,
                               ulxr::ValueParserWb::wbToken_Value,
                                ulxr::ValueParserWb::wbToken_Double,
                                 ulxr::WbXmlParser::wbxml_STR_I, '1', '2', '3', '.', '4', '5', '6', '0', '0', '0', 0,
                                ulxr::WbXmlParser::wbxml_END,
                               ulxr::WbXmlParser::wbxml_END
                             };

const char stringPattern[] = { WB_PROLOG,
                               ulxr::ValueParserWb::wbToken_Value,
                                ulxr::ValueParserWb::wbToken_String,
                                 ulxr::WbXmlParser::wbxml_STR_I, '&', 'a', 'm', 'p', ';', '\"', '\'',
                                 's', 't', 'r', 'i', 'n', 'g', '&', 'l', 't', ';',
                                 '>', '&', 'a', 'm', 'p', ';', '\"', '\'', 0,
                                ulxr::WbXmlParser::wbxml_END,
                               ulxr::WbXmlParser::wbxml_END
                             };

const char implPattern[]   = { WB_PROLOG,
                               ulxr::ValueParserWb::wbToken_Value,
                                ulxr::WbXmlParser::wbxml_STR_I, 'i', 'm', 'p', 'l', 'i', 'z', 'i', 't',
                                ' ', 's', 't', 'r', 'i', 'n', 'g', 0,
                               ulxr::WbXmlParser::wbxml_END
                             };

const char datePattern[]   = { WB_PROLOG,
                               ulxr::ValueParserWb::wbToken_Value,
                                ulxr::ValueParserWb::wbToken_Date,
                                 ulxr::WbXmlParser::wbxml_STR_I, '2', '0', '0', '2', '0', '3', '1', '0',
                                 'T', '1', '0', ':', '2', '3', ':', '4', '5', 0,
                                ulxr::WbXmlParser::wbxml_END,
                               ulxr::WbXmlParser::wbxml_END
                             };


const char base64Pattern[] = { WB_PROLOG,
                               ulxr::ValueParserWb::wbToken_Value,
                                ulxr::ValueParserWb::wbToken_Base64,
                                 ulxr::WbXmlParser::wbxml_STR_I, 'A', 'B', 'A', 'S', 'r', 't',
                                 '4', '6', '6', 'a', '9', '0', 0,
                                ulxr::WbXmlParser::wbxml_END,
                               ulxr::WbXmlParser::wbxml_END
                             };

const char struct1Pattern[]= { WB_PROLOG,
                               ulxr::ValueParserWb::wbToken_Value,
                                ulxr::ValueParserWb::wbToken_Struct,

                                 ulxr::ValueParserWb::wbToken_Member,
                                  ulxr::ValueParserWb::wbToken_Name,
                                   ulxr::WbXmlParser::wbxml_STR_I, 'f', 'a', 'u', 'l', 't',
                                   'S', 't', 'r', 'i', 'n', 'g', 0,
                                  ulxr::WbXmlParser::wbxml_END,
                                  ulxr::ValueParserWb::wbToken_Value,
                                   ulxr::ValueParserWb::wbToken_String,
                                    ulxr::WbXmlParser::wbxml_STR_I, 'f', 'a', 'u', 'l', 't', 's', 't', 'r', '1', 0,
                                   ulxr::WbXmlParser::wbxml_END,
                                  ulxr::WbXmlParser::wbxml_END,
                                 ulxr::WbXmlParser::wbxml_END,

                                 ulxr::ValueParserWb::wbToken_Member,
                                  ulxr::ValueParserWb::wbToken_Name,
                                   ulxr::WbXmlParser::wbxml_STR_I, 'f', 'a', 'u', 'l', 't', 'c', 'o', 'd', 'e', 0,
                                  ulxr::WbXmlParser::wbxml_END,
                                  ulxr::ValueParserWb::wbToken_Value,
                                   ulxr::ValueParserWb::wbToken_I4,
                                    ulxr::WbXmlParser::wbxml_STR_I, '1', '2', '3', 0,
                                   ulxr::WbXmlParser::wbxml_END,
                                  ulxr::WbXmlParser::wbxml_END,
                                 ulxr::WbXmlParser::wbxml_END,

                                ulxr::WbXmlParser::wbxml_END,
                               ulxr::WbXmlParser::wbxml_END
                             };


const char struct2Pattern[]= { WB_PROLOG,
                               ulxr::ValueParserWb::wbToken_Value,
                                ulxr::ValueParserWb::wbToken_Struct,

                                 ulxr::ValueParserWb::wbToken_Member,
                                  ulxr::ValueParserWb::wbToken_Value,
                                   ulxr::ValueParserWb::wbToken_String,
                                    ulxr::WbXmlParser::wbxml_STR_I, 'f', 'a', 'u', 'l', 't', 's', 't', 'r', '2', 0,
                                   ulxr::WbXmlParser::wbxml_END,
                                  ulxr::WbXmlParser::wbxml_END,
                                  ulxr::ValueParserWb::wbToken_Name,
                                   ulxr::WbXmlParser::wbxml_STR_I, 'f', 'a', 'u', 'l', 't',
                                   'S', 't', 'r', 'i', 'n', 'g', 0,
                                  ulxr::WbXmlParser::wbxml_END,
                                 ulxr::WbXmlParser::wbxml_END,

                                 ulxr::ValueParserWb::wbToken_Member,
                                  ulxr::ValueParserWb::wbToken_Value,
                                   ulxr::ValueParserWb::wbToken_I4,
                                    ulxr::WbXmlParser::wbxml_STR_I, '4', '5', '6', 0,
                                   ulxr::WbXmlParser::wbxml_END,
                                  ulxr::WbXmlParser::wbxml_END,
                                  ulxr::ValueParserWb::wbToken_Name,
                                   ulxr::WbXmlParser::wbxml_STR_I, 'f', 'a', 'u', 'l', 't', 'c', 'o', 'd', 'e', 0,
                                  ulxr::WbXmlParser::wbxml_END,
                                 ulxr::WbXmlParser::wbxml_END,

                                ulxr::WbXmlParser::wbxml_END,
                               ulxr::WbXmlParser::wbxml_END
                             };

const char arrayPattern[]  = { WB_PROLOG,
                               ulxr::ValueParserWb::wbToken_Value,
                                ulxr::ValueParserWb::wbToken_Array,
                                 ulxr::ValueParserWb::wbToken_Data,

                                  ulxr::ValueParserWb::wbToken_Value,
                                   ulxr::ValueParserWb::wbToken_Boolean,
                                    ulxr::WbXmlParser::wbxml_STR_I, 't', 'r', 'u', 'e', 0,
                                   ulxr::WbXmlParser::wbxml_END,
                                  ulxr::WbXmlParser::wbxml_END,

                                  ulxr::ValueParserWb::wbToken_Value,
                                   ulxr::ValueParserWb::wbToken_I4,
                                    ulxr::WbXmlParser::wbxml_STR_I, '1', '2', '3', 0,
                                   ulxr::WbXmlParser::wbxml_END,
                                  ulxr::WbXmlParser::wbxml_END,

                                 ulxr::WbXmlParser::wbxml_END,
                                ulxr::WbXmlParser::wbxml_END,
                               ulxr::WbXmlParser::wbxml_END
                             };

const char respPattern[]   = { WB_PROLOG,
                               ulxr::MethodResponseParserWb::wbToken_MethodResponse,
                                ulxr::MethodResponseParserWb::wbToken_Fault,
                                 ulxr::ValueParserWb::wbToken_Value,
                                  ulxr::ValueParserWb::wbToken_Struct,

                                   ulxr::ValueParserWb::wbToken_Member,
                                    ulxr::ValueParserWb::wbToken_Name,
                                     ulxr::WbXmlParser::wbxml_STR_I, 'f', 'a', 'u', 'l', 't', 'C', 'o', 'd', 'e', 0,
                                    ulxr::WbXmlParser::wbxml_END,
                                    ulxr::ValueParserWb::wbToken_Value,
                                     ulxr::ValueParserWb::wbToken_I4,
                                      ulxr::WbXmlParser::wbxml_STR_I, '1', '2', '3', 0,
                                     ulxr::WbXmlParser::wbxml_END,
                                    ulxr::WbXmlParser::wbxml_END,
                                   ulxr::WbXmlParser::wbxml_END,

                                   ulxr::ValueParserWb::wbToken_Member,
                                    ulxr::ValueParserWb::wbToken_Name,
                                     ulxr::WbXmlParser::wbxml_STR_I, 'f', 'a', 'u', 'l', 't',
                                     'S', 't', 'r', 'i', 'n', 'g', 0,
                                    ulxr::WbXmlParser::wbxml_END,
                                    ulxr::ValueParserWb::wbToken_Value,
                                     ulxr::ValueParserWb::wbToken_String,
                                      ulxr::WbXmlParser::wbxml_STR_I, 'f', 'a', 'u', 'l', 't', 's',
                                      't', 'r', '_', 'r', 0,
                                    ulxr::WbXmlParser::wbxml_END,
                                   ulxr::WbXmlParser::wbxml_END,

                                  ulxr::WbXmlParser::wbxml_END,
                                 ulxr::WbXmlParser::wbxml_END,
                                ulxr::WbXmlParser::wbxml_END,
                               ulxr::WbXmlParser::wbxml_END
                             };


const char callPattern[]   = { WB_PROLOG,
                               ulxr::MethodCallParserWb::wbToken_MethodCall,
                                ulxr::MethodCallParserWb::wbToken_MethodName,
                                 ulxr::WbXmlParser::wbxml_STR_I, 's', 'h', 'e', 'l', 'l', '.', 'c', 'a', 'l', 'l', 0,
                                ulxr::WbXmlParser::wbxml_END,

                                ulxr::MethodCallParserWb::wbToken_Params,

                                 ulxr::MethodCallParserWb::wbToken_Param,
                                  ulxr::ValueParserWb::wbToken_Value,
                                   ulxr::ValueParserWb::wbToken_String,
                                    ulxr::WbXmlParser::wbxml_STR_I, 's', 't', 'r', 'i', 'n', 'g', 0,
                                   ulxr::WbXmlParser::wbxml_END,
                                  ulxr::WbXmlParser::wbxml_END,
                                 ulxr::WbXmlParser::wbxml_END,

                                 ulxr::MethodCallParserWb::wbToken_Param,
                                  ulxr::ValueParserWb::wbToken_Value,
                                   ulxr::ValueParserWb::wbToken_I4,
                                    ulxr::WbXmlParser::wbxml_STR_I, '1', '2', '3', 0,
                                   ulxr::WbXmlParser::wbxml_END,
                                  ulxr::WbXmlParser::wbxml_END,
                                 ulxr::WbXmlParser::wbxml_END,

                                 ulxr::MethodCallParserWb::wbToken_Param,
                                  ulxr::ValueParserWb::wbToken_Value,
                                   ulxr::ValueParserWb::wbToken_Boolean,
                                    ulxr::WbXmlParser::wbxml_STR_I, 't', 'r', 'u', 'e', 0,
                                   ulxr::WbXmlParser::wbxml_END,
                                  ulxr::WbXmlParser::wbxml_END,
                                 ulxr::WbXmlParser::wbxml_END,

                                ulxr::WbXmlParser::wbxml_END,
                               ulxr::WbXmlParser::wbxml_END
                             };

const char emptyCallPattern[]
                           = { WB_PROLOG,
                               ulxr::MethodCallParserWb::wbToken_MethodCall,
                                ulxr::MethodCallParserWb::wbToken_MethodName,
                                 ulxr::WbXmlParser::wbxml_STR_I, 's', 'h', 'e', 'l', 'l', '.', 'c', 'a', 'l', 'l', 0,
                                ulxr::WbXmlParser::wbxml_END,

                                ulxr::MethodCallParserWb::wbToken_Params,

                                ulxr::WbXmlParser::wbxml_END,
                               ulxr::WbXmlParser::wbxml_END
                             };

void testPattern(const char *pattern, unsigned len)
{
  ULXR_COUT << ULXR_PCHAR("====================================================\n");
  unsigned cnt = 0;
  for (unsigned i = 0; i < len; ++i)
  {
    ULXR_COUT << ulxr::HtmlFormHandler::makeHexNumber(pattern[i]) << ULXR_PCHAR(" ");
    if (++cnt > 16)
    {
      ULXR_COUT << std::endl;
      cnt = 0;
    }
  }
  ULXR_COUT << std::endl
            << ULXR_PCHAR("-----------------------------------------------------\n");
  ulxr::ValueParserWb parser;
  bool done = false;
  ULXR_COUT << ULXR_PCHAR("Parser start\n");
  if (!parser.parse(pattern, len, done))
  {
    ULXR_COUT << ULXR_PCHAR("Parser error\n");
    std::cerr << ulxr::getLatin1(parser.getErrorString(parser.getErrorCode())).c_str()
              << " at line "
              << parser.getCurrentLineNumber()
              << std::endl;
  }
  ULXR_COUT << ULXR_PCHAR("Parser finish\n");

  ulxr::Value val = parser.getValue();
  ULXR_COUT << val.getSignature(true) << std::endl;
  ULXR_COUT << val.getXml(0) << std::endl;
  if (val.isString() )
  {
    ulxr::RpcString str= val;
    ULXR_COUT << ULXR_PCHAR("EinRpcString: ") << str.getString() << std::endl;
    return;
  }
  ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");
  ULXR_COUT << ulxr::binaryDebugOutput(val.getWbXml());
  ULXR_COUT << ULXR_PCHAR("====================================================\n");
}


void testCallPattern(const char *pattern, unsigned len)
{
  ULXR_COUT << ULXR_PCHAR("====================================================\n");
  unsigned cnt = 0;
  for (unsigned i2 = 0; i2 < len; ++i2)
  {
    ULXR_COUT << ulxr::HtmlFormHandler::makeHexNumber(pattern[i2]) << ULXR_PCHAR(" ");
    if (++cnt > 16)
    {
      ULXR_COUT << std::endl;
      cnt = 0;
    }
  }
  ULXR_COUT << std::endl
            << ULXR_PCHAR("-----------------------------------------------------\n");

  ulxr::MethodCallParserWb parser;
  bool done= false;
  if (!parser.parse(pattern, len, done))
  {
    ULXR_COUT << ULXR_PCHAR("Parser error\n");
    std::cerr << ulxr::getLatin1(parser.getErrorString(parser.getErrorCode())).c_str()
              << " at line "
              << parser.getCurrentLineNumber()
              << std::endl;
  }

  ULXR_COUT << parser.getMethodCall().getSignature(true) << std::endl;
  ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");
  ULXR_COUT << parser.getMethodCall().getXml(0) << std::endl;
  ULXR_COUT << parser.getMethodName() << std::endl;
  ULXR_COUT << parser.numParams() << std::endl;
  for (unsigned i1 = 0; i1 < parser.numParams(); ++i1)
    ULXR_COUT << parser.getParam(i1).getSignature(true) << std::endl;

  ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");
  ULXR_COUT << ulxr::binaryDebugOutput(parser.getMethodCall().getWbXml());
  ULXR_COUT << ULXR_PCHAR("====================================================\n");
}


void testRespPattern(const char *pattern, unsigned len)
{
  ULXR_COUT << ULXR_PCHAR("====================================================\n");
  unsigned cnt = 0;
  for (unsigned i = 0; i < len; ++i)
  {
    ULXR_COUT << ulxr::HtmlFormHandler::makeHexNumber(pattern[i]) << ULXR_PCHAR(" ");
    if (++cnt > 16)
    {
      ULXR_COUT << std::endl;
      cnt = 0;
    }
  }
  ULXR_COUT << std::endl
            << ULXR_PCHAR("-----------------------------------------------------\n");

  ulxr::MethodResponseParserWb parser;
  bool done= false;
  if (!parser.parse(pattern, len, done))
  {
    ULXR_COUT << ULXR_PCHAR("Parser error\n");
    std::cerr << ulxr::getLatin1(parser.getErrorString(parser.getErrorCode())).c_str()
              << " at line "
              << parser.getCurrentLineNumber()
              << std::endl;
  }

  ulxr::Value val= parser.getValue();
  ULXR_COUT << ULXR_PCHAR("!Value...\n");
  ULXR_COUT << val.getSignature(true) << std::endl;
  ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");
  ULXR_COUT << val.getXml(0) << std::endl;
  ULXR_COUT << ULXR_PCHAR("Response...\n");
  ULXR_COUT << parser.getMethodResponse().getSignature(true) << std::endl;
  ULXR_COUT << parser.getMethodResponse().getXml(0) << std::endl;

  ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");
  ULXR_COUT << ulxr::binaryDebugOutput(parser.getMethodResponse().getWbXml());
  ULXR_COUT << ULXR_PCHAR("====================================================\n");
}

// #define STRESS_IT

int main(int argc, char * argv [])
{
  ulxr::intializeLog4J(argv[0]);

  int success = 0;

#ifdef STRESS_IT
  for (int i= 0; i < 1000; ++i)
  {
#endif
    try
    {
      ULXR_COUT << ULXR_PCHAR("Testing patterns\n");

      ULXR_COUT << ULXR_PCHAR("boolPattern\n") << std::flush;
      testPattern(boolPattern, sizeof(boolPattern));

      ULXR_COUT << ULXR_PCHAR("int1Pattern\n");
      testPattern(int1Pattern, sizeof(int1Pattern));

      ULXR_COUT << ULXR_PCHAR("int2Pattern\n");
      testPattern(int2Pattern, sizeof(int2Pattern));

      ULXR_COUT << ULXR_PCHAR("doublePattern\n");
      testPattern(doublePattern, sizeof(doublePattern));

      ULXR_COUT << ULXR_PCHAR("stringPattern\n");
      testPattern(stringPattern, sizeof(stringPattern));

      ULXR_COUT << ULXR_PCHAR("base64Pattern\n");
      testPattern(base64Pattern, sizeof(base64Pattern));

      ULXR_COUT << ULXR_PCHAR("datePattern\n");
      testPattern(datePattern, sizeof(datePattern));

      ULXR_COUT << ULXR_PCHAR("struct1Pattern\n");
      testPattern(struct1Pattern, sizeof(struct1Pattern));

      ULXR_COUT << ULXR_PCHAR("struct2Pattern\n");
      testPattern(struct2Pattern, sizeof(struct2Pattern));

      ULXR_COUT << ULXR_PCHAR("arrayPattern\n");
      testPattern(arrayPattern, sizeof(arrayPattern));

      ULXR_COUT << ULXR_PCHAR("callPattern\n");
      testCallPattern(callPattern, sizeof(callPattern));

      ULXR_COUT << ULXR_PCHAR("emptyCallPattern\n");
      testCallPattern(emptyCallPattern, sizeof(emptyCallPattern));

      ULXR_COUT << ULXR_PCHAR("respPattern\n");
      testRespPattern(respPattern, sizeof(respPattern));

      ULXR_COUT << ULXR_PCHAR("implPattern\n");
      testPattern(implPattern, sizeof(implPattern));

      ULXR_COUT << ULXR_PCHAR("emptyArrayPattern\n");
      testRespPattern(emptyArrayPattern, sizeof(emptyArrayPattern));

      ULXR_COUT << ULXR_PCHAR("emptyRespPattern\n");
      testRespPattern(emptyRespPattern, sizeof(emptyRespPattern));

      ULXR_COUT << ULXR_PCHAR("emptyStructPattern\n");
      testRespPattern(emptyStructPattern, sizeof(emptyStructPattern));

      ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");

      ULXR_COUT << ULXR_PCHAR("MethodResponse abc\n");

      ulxr::MethodResponse mr1(123, ULXR_PCHAR("faultstr_m"));
      ULXR_COUT << ulxr::binaryDebugOutput(mr1.getWbXml());
      ULXR_COUT << std::endl << mr1.getXml(0) << std::endl;

      ulxr::MethodResponse mr2(ulxr::Integer(1));
      ULXR_COUT << ulxr::binaryDebugOutput(mr2.getWbXml());
      ULXR_COUT << std::endl << mr2.getXml(0) << std::endl;

      ulxr::MethodResponse mr3;
      ULXR_COUT << ulxr::binaryDebugOutput(mr3.getWbXml());
      ULXR_COUT << std::endl << mr3.getXml(0) << std::endl;

      ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");

      ulxr::MethodCall mc (ULXR_PCHAR("test.call"));
      ULXR_COUT << ulxr::binaryDebugOutput(mc.getWbXml());

      ULXR_COUT << std::endl << mc.getXml(0) << std::endl;

      ULXR_COUT << ULXR_PCHAR("====================================================\n");

      ulxr::Boolean b(true);
      ulxr::Integer i(123);
      ulxr::Double d(123.456);
      ulxr::RpcString s("<>&\"\'string<>&\"\'");
      ulxr::DateTime dt(ULXR_PCHAR("20020310T10:23:45"));
      ulxr::Base64 b64(ULXR_PCHAR("ABASrt466a90"));
      ulxr::Struct st;
      ulxr::Array ar;

      ULXR_COUT << ulxr::binaryDebugOutput(b.getWbXml()) << std::endl;
      ULXR_COUT << b.getXml(0) << std::endl;

      ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");
      ULXR_COUT << ulxr::binaryDebugOutput(i.getWbXml()) << std::endl;
      ULXR_COUT << i.getXml(0) << std::endl;

      ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");
      ULXR_COUT << ulxr::binaryDebugOutput(d.getWbXml()) << std::endl;
      ULXR_COUT << d.getXml(0) << std::endl;

      ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");
      ULXR_COUT << ulxr::binaryDebugOutput(s.getWbXml()) << std::endl;
      ULXR_COUT << s.getXml(0) << std::endl;

      ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");
      ULXR_COUT << ulxr::binaryDebugOutput(dt.getWbXml()) << std::endl;
      ULXR_COUT << dt.getXml(0) << std::endl;

      ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");
      ULXR_COUT << ulxr::binaryDebugOutput(b64.getWbXml()) << std::endl;
      ULXR_COUT << b64.getXml(0) << std::endl;

      ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");
      ULXR_COUT << ulxr::binaryDebugOutput(st.getWbXml()) << std::endl;
      ULXR_COUT << st.getXml(0) << std::endl;

      ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");
      ULXR_COUT << ulxr::binaryDebugOutput(ar.getWbXml()) << std::endl;
      ULXR_COUT << ar.getXml(0) << std::endl;

      ULXR_COUT << ULXR_PCHAR("====================================================\n");

      ULXR_COUT << "wbToken_Value   " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::ValueParserWb::wbToken_Value) << std::endl;
      ULXR_COUT << "wbToken_Array   " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::ValueParserWb::wbToken_Array) << std::endl;
      ULXR_COUT << "wbToken_Data    " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::ValueParserWb::wbToken_Data) << std::endl;
      ULXR_COUT << "wbToken_Struct  " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::ValueParserWb::wbToken_Struct) << std::endl;
      ULXR_COUT << "wbToken_Member  " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::ValueParserWb::wbToken_Member) << std::endl;
      ULXR_COUT << "wbToken_Name    " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::ValueParserWb::wbToken_Name) << std::endl;
      ULXR_COUT << "wbToken_Boolean " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::ValueParserWb::wbToken_Boolean) << std::endl;
      ULXR_COUT << "wbToken_Int     " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::ValueParserWb::wbToken_Int) << std::endl;
      ULXR_COUT << "wbToken_I4      " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::ValueParserWb::wbToken_I4) << std::endl;
      ULXR_COUT << "wbToken_Double  " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::ValueParserWb::wbToken_Double) << std::endl;
      ULXR_COUT << "wbToken_String  " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::ValueParserWb::wbToken_String) << std::endl;
      ULXR_COUT << "wbToken_Base64  " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::ValueParserWb::wbToken_Base64) << std::endl;
      ULXR_COUT << "wbToken_Date    " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::ValueParserWb::wbToken_Date) << std::endl;
      ULXR_COUT << std::endl;

      ULXR_COUT << "wbToken_MethodCall " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::MethodCallParserWb::wbToken_MethodCall) << std::endl;
      ULXR_COUT << "wbToken_MethodName " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::MethodCallParserWb::wbToken_MethodName) << std::endl;
      ULXR_COUT << "wbToken_Params     " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::MethodCallParserWb::wbToken_Params) << std::endl;
      ULXR_COUT << "wbToken_Param      " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::MethodCallParserWb::wbToken_Param) << std::endl;
      ULXR_COUT << std::endl;

      ULXR_COUT << "wbToken_MethodResponse " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::MethodResponseParserWb::wbToken_MethodResponse) << std::endl;
      ULXR_COUT << "wbToken_Fault          " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::MethodResponseParserWb::wbToken_Fault) << std::endl;
      ULXR_COUT << "wbToken_Params         " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::MethodResponseParserWb::wbToken_Params) << std::endl;
      ULXR_COUT << "wbToken_Param          " << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::MethodResponseParserWb::wbToken_Param) << std::endl;
      ULXR_COUT << std::endl;

      ULXR_COUT << "wbToken_ValueParserLast "
                << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::ValueParserWb::wbToken_ValueParserLast) << std::endl;

      ULXR_COUT << "wbToken_CallParserLast "
                << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::MethodCallParserWb::wbToken_CallParserLast) << std::endl;

      ULXR_COUT << "wbToken_ResponseParserLast "
                << ulxr::HtmlFormHandler::makeHexNumber((unsigned char)ulxr::MethodResponseParserWb::wbToken_ResponseParserLast) << std::endl;


    }

    catch(ulxr::Exception &ex)
    {
       ULXR_COUT << ULXR_PCHAR("Error occured: ")
                 << ULXR_GET_STRING(ex.why()) << std::endl;
       success= 1;
    }
#ifdef STRESS_IT
  }
#endif

  return success;
}

