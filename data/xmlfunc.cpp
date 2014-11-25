/***************************************************************************
                     xmlfunc.cpp  -  test suite for xml-rpc
                             -------------------
    begin                : Son Mar 10 19:33:24 CET 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: xmlfunc.cpp 1165 2010-01-06 10:51:05Z ewald-arnold $

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
// which expand to their xmlrpc counterparts. Define before #including ulxr_value.h!!
#define ULXR_USE_INTRINSIC_VALUE_TYPES

// upon requst you may also use the following construct
//    ("second" << i2)
#define ULXR_STRUCT_MEMBER_FROM_NAME_VALUE

#define DEBUG

#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <cstring>
#include <iostream>
#include <map>
#include <ctime>
#include <cstdlib>
#include <cstdio>

#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_call.h>
#include <ulxmlrpcpp/ulxr_response.h>
#include <ulxmlrpcpp/ulxr_valueparse.h>
#include <ulxmlrpcpp/ulxr_callparse.h>
#include <ulxmlrpcpp/ulxr_responseparse.h>

const char *crashPattern    =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>                                           \
  <methodResponse>                                                                    \
    <params><param><value><array><data><value>                                        \
      <struct>                                                                        \
       <member><name>Welt</name><value><string>Hallo</string></value></member>        \
       <member><name>World</name><value><string>Hello</string></value></member>       \
      </struct>                                                                       \
      </value>                                                                        \
      <value><struct>                                                                 \
       <member><name>Gemeinde</name><value><string>Liebe</string></value></member>    \
       <member><name>Trek</name><value><string>Star</string></value></member>         \
       <member><name>Yoda</name><value><string>Yedi</string></value></member>         \
      </struct></value>                                                               \
      </data></array></value></param></params>                                        \
      </methodResponse>                                                               \
";

const char *emptyRespPattern    =  "<methodResponse>\n"
                                   "  <params>\n"
                                   "  </params>\n"
                                   "</methodResponse>\n";

const char *emptyArrayPattern   =  "<methodResponse>\n"
                                   "  <params>\n"
                                   "   <param>\n"
                                   "    <value>\n"
                                   "     <array><data></data></array>\n"
                                   "    </value>\n"
                                   "   </param>\n"
                                   "  </params>\n"
                                   "</methodResponse>\n";

const char *emptyStructPattern   =  "<methodResponse>\n"
                                   "  <params>\n"
                                   "   <param>\n"
                                   "    <value>\n"
                                   "     <struct>\n"
                                   "     </struct>\n"
                                   "    </value>\n"
                                   "   </param>\n"
                                   "  </params>\n"
                                   "</methodResponse>\n";

const char *int1Pattern   =  "<value><i4>123</i4></value>";
const char *int2Pattern   =  "<value><int>321</int></value>";

const char *boolPattern   =  "<value><boolean>false</boolean></value>";

const char *doublePattern =  "<value><double>123.456000</double></value>";

const char *stringPattern =  "<value><string>&amp;\"'string&lt;>&amp;\"'</string></value>";

const char *implPattern   =  "<value>implizit string</value>";

const char *datePattern   =  "<value><dateTime.iso8601>20020310T10:23:45</dateTime.iso8601></value>";

const char *base64Pattern =  "<value><base64>ABASrt466a90</base64></value>";

const char *struct1Pattern = "<value>\n"
                             " <struct>\n"
                             "  <member>\n"
                             "   <name>faultString</name>\n"
                             "   <value><string>faultstr1</string></value>\n"
                             "  </member>\n"
                             "  <member>\n"
                             "   <name>faultcode</name>\n"
                             "   <value><i4>123</i4></value>\n"
                             "  </member>\n"
                             " </struct>\n"
                             "</value>\n\n";

const char *struct2Pattern =
                             "<value>\n"
                             " <struct>\n"
                             "  <member>\n"
                             "   <value><string>faultstr2</string></value>\n"
                             "   <name>faultString</name>\n"
                             "  </member>\n"
                             "  <member>\n"
                             "   <value><i4>456</i4></value>\n"
                             "   <name>faultcode</name>\n"
                             "  </member>\n"
                             " </struct>\n"
                             "</value>";

const char *arrayPattern  =
                             "<value>\n"
                             " <array>\n"
                             "  <data>\n"
                             "   <value><boolean>true</boolean></value>\n"
                             "   <value><i4>123</i4></value>\n"
                             "  </data>\n"
                             " </array>\n"
                             "</value>";

const char *respPattern  =
                            "<methodResponse>\n"
                            " <fault>\n"
                            "  <value>\n"
                            "   <struct>\n"
                            "    <member>\n"
                            "     <name>faultCode</name>\n"
                            "     <value><i4>123</i4></value>\n"
                            "    </member>\n"
                            "    <member>\n"
                            "     <name>faultString</name>\n"
                            "     <value><string>faultstr_r</string></value>\n"
                            "    </member>\n"
                            "   </struct>\n"
                            "  </value>\n"
                            " </fault>\n"
                            "</methodResponse>\n";

const char *callPattern1  =
"<?xml version = '1.0'?>\n"
"  <methodCall>\n"
"    <methodName>system.getCapabilities</methodName>\n"
"   <params>\n"
"      <param><value><string><![CDATA[]]></string></value></param>\n"
"   </params>\n"
"</methodCall>\n";

const char *callPattern2  =
                            "<methodCall>\n"
                            " <methodName>shell.call</methodName>\n"
                            " <params>\n"
                            "  <param>\n"
                            "   <value><string>string</string></value>\n"
                            "  </param>\n"
                            "  <param>\n"
                            "   <value><i4>123</i4></value>\n"
                            "  </param>\n"
                            "  <param>\n"
                            "   <value><boolean>true</boolean></value>\n"
                            "  </param>\n"
                            " </params>\n"
                            "</methodCall>\n\n";

const char *emptyCallPattern1  =
                            "<methodCall>\n"
                            " <methodName>shell.call</methodName>\n"
                            " <params>\n"
                            " </params>\n"
                            "</methodCall>\n\n";

const char *emptyCallPattern2  =
                            "<methodCall>\n"
                            " <methodName>shell.call</methodName>\n"
                            " <params/>\n"
                            "</methodCall>\n\n";

void testPattern(const char *pattern)
{
  ULXR_COUT << ULXR_GET_STRING(pattern)
            << std::endl
            << ULXR_PCHAR("-----------------------------------------------------\n");
  ulxr::ValueParser parser;
  bool done = false;
  ULXR_COUT << ULXR_PCHAR("Parser start\n");
  if (!parser.parse(pattern, strlen(pattern), done))
  {
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
    ulxr::RpcString str = val;
    ULXR_COUT << ULXR_PCHAR("EinRpcString: ") << str.getString() << std::endl;
    return;
  }
  ULXR_COUT << ULXR_PCHAR("====================================================\n");
}


void testCallPattern(const char *pattern)
{
  ULXR_COUT << pattern << std::endl
            << ULXR_PCHAR("-----------------------------------------------------\n");
  ulxr::MethodCallParser parser;
  bool done = false;
  if (!parser.parse(pattern, strlen(pattern), done))
  {
    std::cerr << ulxr::getLatin1(parser.getErrorString(parser.getErrorCode())).c_str()
              << " at line "
              << parser.getCurrentLineNumber()
              << std::endl;
  }

  ULXR_COUT << parser.getMethodCall().getSignature(true) << std::endl;
  ULXR_COUT << parser.getMethodCall().getXml(0) << std::endl;
  ULXR_COUT << parser.getMethodName() << std::endl;
  ULXR_COUT << parser.numParams() << std::endl;
  for (unsigned i = 0; i < parser.numParams(); ++i)
    ULXR_COUT << parser.getParam(i).getSignature(true) << std::endl;
  ULXR_COUT << ULXR_PCHAR("====================================================\n");
}


void testcrashPattern(const char *pattern)
{
  ulxr::MethodResponseParser parser;
  bool done = false;
  if (!parser.parse(pattern, strlen(pattern), done))
  {
    std::cerr << ulxr::getLatin1(parser.getErrorString(parser.getErrorCode())).c_str()
              << " at line "
              << parser.getCurrentLineNumber()
              << std::endl;
  }

  ulxr::MethodResponse listresp = parser.getMethodResponse();
  std::cout << ulxr::getLatin1(listresp.getXml()) << std::endl;
  ulxr::Array *arrresp = const_cast<ulxr::Array*>(listresp.getResult().getArray());
  for(unsigned i = 0; i < arrresp->size(); i++)
  {
    ulxr::Value item = arrresp->getItem(i);
    const ulxr::Struct *structresp = item.getStruct();
    std::vector<ulxr::CppString> key = structresp->getMemberNames();
    for(unsigned j = 0; j < key.size(); j++)
    {
      std::cout << structresp << " ";
      std::cout << i + 1 << " " << ulxr::getLatin1(key[j]) << " ";
      bool  h = structresp->hasMember(key[j]); std::cout << h << " ";
      ulxr::Value v = structresp->getMember(key[j]); ///< this segfaults
      ulxr::RpcString s = (ulxr::RpcString)v;
      std::cout << std::endl;
    }
  }
}

void testRespPattern(const char *pattern)
{
  ULXR_COUT << pattern << std::endl
       << ULXR_PCHAR("-----------------------------------------------------\n");
  ulxr::MethodResponseParser parser;
  bool done = false;
  if (!parser.parse(pattern, strlen(pattern), done))
  {
    std::cerr << ulxr::getLatin1(parser.getErrorString(parser.getErrorCode())).c_str()
              << " at line "
              << parser.getCurrentLineNumber()
              << std::endl;
  }

  ulxr::Value val = parser.getValue();
  ULXR_COUT << ULXR_PCHAR("!Value...\n");
  ULXR_COUT << val.getSignature(true) << std::endl;
  ULXR_COUT << val.getXml(0) << std::endl;
  ULXR_COUT << ULXR_PCHAR("Response...\n");
  ULXR_COUT << parser.getMethodResponse().getSignature(true) << std::endl;
  ULXR_COUT << parser.getMethodResponse().getXml(0) << std::endl;
  ULXR_COUT << ULXR_PCHAR("====================================================\n");
}

// #define STRESS_IT

int main(int argc, char * argv [])
{
  ulxr::intializeLog4J(argv[0]);
  ulxr::enableXmlPrettyPrint(true);

  int success = 0;

#ifdef STRESS_IT
  for (int often = 0; often < 1000; ++often)
  {
#endif
    try
    {
      ulxr::Boolean b(true);
      ulxr::Integer i(123);
      ulxr::Double d(123.456);
      ulxr::RpcString s("<>&\"'string<>&\"'0xd9f0�)/()/");
      ulxr::DateTime dt(ULXR_PCHAR("20020310T10:23:45"));
      std::time_t t = std::time(0);
      ulxr::DateTime dt2(t);
      ulxr::Base64 b64(ULXR_PCHAR("ABASrt466a90"));

      typedef std::map<ulxr::Cpp8BitString, ulxr::Value> members;
      typedef std::pair<ulxr::Cpp8BitString, ulxr::Value> member_pair;
      members val;
      {
        ulxr::Array arr1;
        unsigned szarr = sizeof(arr1);
        ULXR_COUT << szarr << std::endl;
      }

      ulxr::ValueBase *vb = new ulxr::RpcString("asdfasdf");
      delete vb;

      ulxr::Struct st;
      ulxr::Array ar;

      ulxr::Value rv = i;
      i = rv;
      ULXR_COUT << ULXR_PCHAR("rv(i) = ") << rv.getSignature(true) << std::endl;

      rv = d;
      d = rv;
      ULXR_COUT << d.getDouble() << std::endl;
      ULXR_COUT << ULXR_PCHAR("rv(d) = ") << rv.getSignature(true) << std::endl;

      rv = b;
      b = rv;
      ULXR_COUT << ULXR_PCHAR("rv(b) = ") << rv.getSignature(true) << std::endl;

      rv = s;
      s = rv;
      ULXR_COUT << ULXR_PCHAR("rv(s) = ") << rv.getSignature(true) << std::endl;

      rv = dt;
      dt = rv;
      ULXR_COUT << ULXR_PCHAR("rv(dt) = ") << rv.getSignature(true) << std::endl;

      rv = b64;
      b64 = rv;
      ULXR_COUT << ULXR_PCHAR("rv(b64) = ") << rv.getSignature(true) << std::endl;

      rv = ar;
      ar = rv;
      ULXR_COUT << ULXR_PCHAR("rv(ar) = ") << rv.getSignature(true) << std::endl;

      rv = st;
      st = rv;
      ULXR_COUT << ULXR_PCHAR("rv(st) = ") << rv.getSignature(true) << std::endl << std::endl;

      ar.addItem(b);
      ar.addItem(i);
      ar.addItem(d);
      ar.addItem(s);
      ar.addItem(b64);
      ar.addItem(st);

      st.addMember (ULXR_PCHAR("intmem"), i);
      st.addMember (ULXR_PCHAR("dmem"), d);
      st.addMember (ULXR_PCHAR("arrmem"), ar);

      ULXR_COUT << i.getSignature(true) << std::endl;
      ULXR_COUT << i.getXml(0) << std::endl << std::endl;

      ULXR_COUT << rv.getSignature(true) << std::endl;
      ULXR_COUT << rv.getXml(0) << std::endl << std::endl;

      ULXR_COUT << b.getSignature(true) << std::endl;
      ULXR_COUT << b.getXml(0) << std::endl << std::endl;

      ULXR_COUT << d.getSignature(true) << std::endl;
      ULXR_COUT << d.getXml(0) << std::endl << std::endl;

      ULXR_COUT << s.getSignature(true) << std::endl;
      ULXR_COUT << s.getXml(0) << std::endl << std::endl;

      ULXR_COUT << dt.getSignature(true) << std::endl;
      ULXR_COUT << dt.getXml(0) << std::endl << std::endl;

      ULXR_COUT << dt2.getSignature(true) << std::endl;
      ULXR_COUT << dt2.getXml(0) << std::endl << std::endl;

      ULXR_COUT << b64.getSignature(true) << std::endl;
      ULXR_COUT << b64.getXml(0) << std::endl << std::endl;

      ULXR_COUT << st.getSignature(true) << std::endl;
      ULXR_COUT << st.getXml(0) << std::endl << std::endl;

      ULXR_COUT << ar.getSignature(true) << std::endl;
      ULXR_COUT << ar.getXml(0) << std::endl << std::endl;

      st.clear();
      st.addMember (ULXR_PCHAR("intmem"), i);
      st.addMember (ULXR_PCHAR("dmem"), d);

      ulxr::MethodCall mc (ULXR_PCHAR("test.call"));
      mc.addParam (st);
      mc.addParam (ar);
      mc.addParam (b);

      ULXR_COUT << mc.getXml(0) << std::endl;
      ULXR_COUT << mc.getSignature(true) << std::endl << std::endl;

      ulxr::MethodCall sc (ULXR_PCHAR("shell.call"));
      sc.addParam(s);
      sc.addParam (i);
      sc.addParam (b);

      ULXR_COUT << sc.getXml(0) << std::endl;
      ULXR_COUT << sc.getSignature(true) << std::endl << std::endl;

      ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");

      ULXR_COUT << ULXR_PCHAR("MethodResponse abc\n");

      ulxr::MethodResponse mr1(123, ULXR_PCHAR("faultstr_m"));
      ULXR_COUT << mr1.getXml(0) << std::endl;
      ULXR_COUT << mr1.getSignature(true) << std::endl << std::endl;

      mr1.setResult(st);
      ULXR_COUT << mr1.getXml(0) << std::endl;
      ULXR_COUT << mr1.getSignature(true) << std::endl << std::endl;

      ulxr::MethodResponse mr2(d);
      ULXR_COUT << mr2.getXml(0) << std::endl;
      ULXR_COUT << mr2.getSignature(true) << std::endl << std::endl;

      ulxr::MethodResponse mr3;
      ULXR_COUT << mr3.getXml(0) << std::endl;
      ULXR_COUT << mr3.getSignature(true) << std::endl << std::endl;

      ULXR_COUT << ULXR_PCHAR("----------------------------------------------------\n");

      ULXR_COUT << ULXR_PCHAR("Testing patterns\n");

      ULXR_COUT << ULXR_PCHAR("boolPattern\n") << std::flush;
      testPattern(boolPattern);

      ULXR_COUT << ULXR_PCHAR("int1Pattern\n");
      testPattern(int1Pattern);

      ULXR_COUT << ULXR_PCHAR("int2Pattern\n");
      testPattern(int2Pattern);

      ULXR_COUT << ULXR_PCHAR("doublePattern\n");
      testPattern(doublePattern);

      ULXR_COUT << ULXR_PCHAR("stringPattern\n");
      testPattern(stringPattern);

      ULXR_COUT << ULXR_PCHAR("base64Pattern\n");
      testPattern(base64Pattern);

      ULXR_COUT << ULXR_PCHAR("datePattern\n");
      testPattern(datePattern);

      ULXR_COUT << ULXR_PCHAR("struct1Pattern\n");
      testPattern(struct1Pattern);

      ULXR_COUT << ULXR_PCHAR("struct2Pattern\n");
      testPattern(struct2Pattern);

      ULXR_COUT << ULXR_PCHAR("arrayPattern\n");
      testPattern(arrayPattern);

      ULXR_COUT << ULXR_PCHAR("callPattern1\n");
      testCallPattern(callPattern1);

      ULXR_COUT << ULXR_PCHAR("callPattern2\n");
      testCallPattern(callPattern2);

      ULXR_COUT << ULXR_PCHAR("emptyCallPattern1\n");
      testCallPattern(emptyCallPattern1);

      ULXR_COUT << ULXR_PCHAR("emptyCallPattern2\n");
      testCallPattern(emptyCallPattern2);

      ULXR_COUT << ULXR_PCHAR("respPattern\n");
      testRespPattern(respPattern);

      ULXR_COUT << ULXR_PCHAR("implPattern\n");
      testPattern(implPattern);

      ULXR_COUT << ULXR_PCHAR("emptyArrayPattern\n");
      testRespPattern(emptyArrayPattern);

      ULXR_COUT << ULXR_PCHAR("emptyRespPattern\n");
      testRespPattern(emptyRespPattern);

      ULXR_COUT << ULXR_PCHAR("emptyRespPattern\n");
      testcrashPattern(crashPattern);

      ULXR_COUT << ULXR_PCHAR("emptyStructPattern\n");
      testRespPattern(emptyStructPattern);

      ar.clear();
      ar << ulxr::Integer(1) << ulxr::Integer(2) << ulxr::RpcString("3") << ulxr::Double(5.0);

// upon requst you may also use intrinsic types like bool, int, double, char*
// #define ULXR_USE_INTRINSIC_VALUE_TYPES
      ar << 11 << 22 << "33"
#ifdef ULXR_UNICODE
         << L"44"
#endif
         << 5.5 << true;

      ULXR_COUT << ar.getXml(0) << std::endl;
      ULXR_COUT << ULXR_PCHAR("====================================================\n");

      st.clear();
      st
          << ulxr::make_member(ULXR_PCHAR("before"), i)
          << ulxr::make_member(ULXR_PCHAR("Hallo"), ar)
#ifdef ULXR_UNICODE
          << ulxr::make_member(ULXR_PCHAR("Hallo wstring"), L"wstring")
#endif
          << ulxr::make_member(ULXR_PCHAR("Hallo double"), 1.0)
          << ulxr::make_member(ULXR_PCHAR("Hallo bool"), true)

// upon requst you may also use the following construct
// #define ULXR_STRUCT_MEMBER_FROM_NAME_VALUE
          << (ulxr::CppString(ULXR_PCHAR("second")) << ulxr::Integer(2))

          << ulxr::make_member(ULXR_PCHAR("difference"), ULXR_PCHAR("1111"))
      ;

      ULXR_COUT << st.getXml(0) << std::endl;
      ULXR_COUT << ULXR_PCHAR("====================================================\n");

    }
    catch(ulxr::Exception &ex)
    {
       ULXR_COUT << ULXR_PCHAR("Error occured: ")
                 << ULXR_GET_STRING(ex.why()) << std::endl;
       success = 1;
    }
#ifdef STRESS_IT
  }
#endif

  int major, minor, patch;
  bool debug;
  ulxr::CppString info;
  ulxr::getVersion (major, minor, patch, debug, info);

  ULXR_COUT << ulxr_i18n(ULXR_PCHAR("Version of the ulxmlrpcpp lib in use: "))
            << major << ULXR_PCHAR(".") << minor << ULXR_PCHAR(".") << patch << std::endl
            << ulxr_i18n(ULXR_PCHAR("Debugging was turned ")) << (debug ? ulxr_i18n(ULXR_PCHAR("ON")) : ulxr_i18n(ULXR_PCHAR("OFF"))) << std::endl
            << ulxr_i18n(ULXR_PCHAR("Additional info: ")) << info << std::endl
            << ulxr_i18n(ULXR_PCHAR("Ready.\n"));

  return success;
}




