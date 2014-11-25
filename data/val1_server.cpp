/***************************************************************************
        val1_server.cpp  --  server for userland validator tests

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    These validator functions are taken from the specs
    on http://validator.xmlrpc.com/ and should also run from
    their online validator website.

    $Id: val1_server.cpp 1151 2009-08-12 15:12:01Z ewald-arnold $

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

#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <cstdlib>
#include <iostream>
#include <cstring>
#include <memory>

#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_signature.h>
#include <ulxmlrpcpp/ulxr_dispatcher.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_tcpip_connection.h>
#include <ulxmlrpcpp/ulxr_ssl_connection.h>

#include "util.c"

using namespace ulxr;

bool terminate_server = false;

MethodResponse arrayOfStructs (const MethodCall &calldata)
{
  if (calldata.numParams() != 1)
    throw ParameterException(InvalidMethodParameterError,
                             ULXR_PCHAR("Exactly 1 parameter allowed for \"")
                             +calldata.getMethodName() + ULXR_PCHAR("\""));

  Array arr = calldata.getParam(0);
  unsigned long sum = 0;
  for (unsigned i = 0; i < arr.size(); ++i)
  {
     Struct str = arr.getItem(i);
     Integer num = str.getMember(ULXR_PCHAR("curly"));
     sum += num.getInteger();
  }

  return MethodResponse (Integer(sum));;
}


MethodResponse countTheEntities (const MethodCall &calldata)
{
  if (calldata.numParams() != 1)
    throw ParameterException(InvalidMethodParameterError,
                             ULXR_PCHAR("Exactly 1 parameter allowed for \"")
                             +calldata.getMethodName() + ULXR_PCHAR("\""));

  CppString s = RpcString(calldata.getParam(0)).getString();
  int leftangs = 0;
  int rightangs = 0;
  int ampers = 0;
  int apos = 0;
  int quotes = 0;
  for (unsigned i = 0; i < s.length(); ++i)
  {
     switch(s[i])
     {
       case ULXR_CHAR('<'): ++leftangs;
       break;

       case ULXR_CHAR('>'): ++rightangs;
       break;

       case ULXR_CHAR('&'): ++ampers;
       break;

       case ULXR_CHAR('\"'): ++quotes;
       break;

       case ULXR_CHAR('\''): ++apos;
       break;
     }
  }

  Struct str;
  str.addMember(ULXR_PCHAR("ctLeftAngleBrackets"), Integer(leftangs));
  str.addMember(ULXR_PCHAR("ctRightAngleBrackets"), Integer(rightangs));
  str.addMember(ULXR_PCHAR("ctAmpersands"), Integer(ampers));
  str.addMember(ULXR_PCHAR("ctApostrophes"), Integer(apos));
  str.addMember(ULXR_PCHAR("ctQuotes"), Integer(quotes));

  return MethodResponse (str);
}


MethodResponse easyStructTest (const MethodCall &calldata)
{
  if (calldata.numParams() != 1)
    throw ParameterException(InvalidMethodParameterError,
                             ULXR_PCHAR("Exactly 1 parameter allowed for \"")
                             +calldata.getMethodName() + ULXR_CHAR("\""));

  Struct str = Struct(calldata.getParam(0));

  int sum = 0;

  Integer num = str.getMember(ULXR_CHAR("moe"));
  sum += num.getInteger();

  num = str.getMember(ULXR_CHAR("larry"));
  sum += num.getInteger();

  num = str.getMember(ULXR_CHAR("curly"));
  sum += num.getInteger();

  return MethodResponse (Integer(sum));
}


MethodResponse echoStructTest (const MethodCall &calldata)
{
  if (calldata.numParams() != 1)
    throw ParameterException(InvalidMethodParameterError,
                             ULXR_PCHAR("Exactly 1 parameter allowed for \"")
                             +calldata.getMethodName() + ULXR_PCHAR("\""));

  Struct str = Struct(calldata.getParam(0));

//   if (str.hasMember(ULXR_CHAR("mickey")) )
//   {
//     ULXR_COUT << ULXR_PCHAR("mickey ") << Base64(str.getMember(ULXR_PCHAR("mickey"))).getString() << std::endl;
//     ULXR_COUT << ULXR_PCHAR("mickey ") << Base64(str.getMember(ULXR_PCHAR("mickey"))).getBase64() << std::endl;
//   }

  return MethodResponse (str);
}


MethodResponse manyTypesTest(const MethodCall &calldata)
{
  if (calldata.numParams() != 6)
    throw ParameterException(InvalidMethodParameterError,
                             ULXR_PCHAR("Exactly 6 parameter allowed for \"")
                             +calldata.getMethodName() + ULXR_PCHAR("\""));

  Array arr;

  arr.addItem(Integer(calldata.getParam(0)));
  arr.addItem(Boolean(calldata.getParam(1)));
  arr.addItem(RpcString(calldata.getParam(2)));
  arr.addItem(Double(calldata.getParam(3)));
  arr.addItem(DateTime(calldata.getParam(4)));
  arr.addItem(Base64(calldata.getParam(5)));

  return MethodResponse (arr);
}


MethodResponse moderateSizeArrayCheck(const MethodCall &calldata)
{
  if (calldata.numParams() != 1)
    throw ParameterException(InvalidMethodParameterError,
                             ULXR_PCHAR("Exactly 1 parameter allowed for \"")
                             +calldata.getMethodName() + ULXR_PCHAR("\""));

  Array arr = calldata.getParam(0);

  if (arr.size() < 2)
    throw ParameterException(InvalidMethodParameterError,
                             ULXR_PCHAR("Need at least 2 elements in the array parameter for \"")
                             +calldata.getMethodName() + ULXR_PCHAR("\""));

 RpcString first =RpcString(arr.getItem(0)).getString();
 RpcString last =RpcString(arr.getItem(arr.size()-1)).getString();

  return MethodResponse (RpcString(first.getString() + last.getString()));
}


MethodResponse nestedStructTest(const MethodCall &calldata)
{
  if (calldata.numParams() != 1)
    throw ParameterException(InvalidMethodParameterError,
                             ULXR_PCHAR("Exactly 1 parameter allowed for \"")
                             +calldata.getMethodName() + ULXR_PCHAR("\""));

  Struct years = Struct(calldata.getParam(0));
  Struct months = years.getMember(ULXR_CHAR("2000"));
  Struct days = months.getMember(ULXR_CHAR("04"));
  Struct data = days.getMember(ULXR_CHAR("01"));

  int moe = Integer(data.getMember(ULXR_CHAR("moe"))).getInteger();
  int larry = Integer(data.getMember(ULXR_CHAR("larry"))).getInteger();
  int curly = Integer(data.getMember(ULXR_CHAR("curly"))).getInteger();

  return MethodResponse (Integer(moe+larry+curly));
}


MethodResponse simpleStructReturnTest(const MethodCall &calldata)

{

  if (calldata.numParams() != 1)
    throw ParameterException(InvalidMethodParameterError,
                             ULXR_PCHAR("Exactly 1 parameter allowed for \"")
                             +calldata.getMethodName() + ULXR_PCHAR("\""));

  int i = Integer(calldata.getParam(0)).getInteger();

  Struct str;
  str.addMember(ULXR_CHAR("times10"), Integer(i * 10));
  str.addMember(ULXR_CHAR("times100"), Integer(i * 100));
  str.addMember(ULXR_CHAR("times1000"), Integer(i * 1000));

  return MethodResponse (str);
}


ulxr::MethodResponse shutdown (const ulxr::MethodCall &/*calldata*/)
{
  ULXR_COUT << ULXR_PCHAR("got signal to shut down\n");
  terminate_server = true;
  return ulxr::MethodResponse(ulxr::Boolean(true));
}


int main(int argc, char **argv)
{
  ulxr::intializeLog4J(argv[0]);

  ulxr::CppString host = ULXR_PCHAR("localhost");
  if (argc > 1)
    host = ULXR_GET_STRING(argv[1]);

  unsigned port = 32002;
  if (argc > 2)
    port = ulxr_atoi(argv[2]);

  bool persistent = haveOption(argc, argv, "persistent");
  bool chunked = haveOption(argc, argv, "chunked");
  bool wbxml = haveOption(argc, argv, "wbxml");
  bool secure = haveOption(argc, argv, "ssl");

  ulxr::CppString sec = ULXR_PCHAR("unsecured");
  if (secure)
    sec = ULXR_PCHAR("secured");

  ULXR_COUT << ULXR_PCHAR("Serving ") << sec << ULXR_PCHAR(" rpc requests at ") << host << ULXR_PCHAR(":") << port << std::endl;
  ULXR_COUT << ULXR_PCHAR("Chunked transfer: ") << chunked << std::endl;
  ULXR_COUT << ULXR_PCHAR("WBXML: ") << wbxml << std::endl;

  std::auto_ptr<ulxr::TcpIpConnection> conn;
#ifdef ULXR_INCLUDE_SSL_STUFF
  if (secure)
  {
    ulxr::SSLConnection *ssl = new ulxr::SSLConnection (true, host, port);
    ssl->setCryptographyData("password", "foo-cert.pem", "foo-cert.pem");
    conn.reset(ssl);
  }
  else
#endif
#ifdef _MSC_VER
  {
    std::auto_ptr<ulxr::TcpIpConnection> temp(new ulxr::TcpIpConnection (true, host, port));
    conn = temp;
  }
#else
    conn.reset(new ulxr::TcpIpConnection (true, host, port));
#endif

  ulxr::HttpProtocol prot(conn.get());
  ulxr::Dispatcher server(&prot, wbxml);
  int res = 0;

  prot.setPersistent(persistent);
  if (persistent)
    conn->setTcpNoDelay(true);

  if (prot.isPersistent())
     ULXR_COUT << ULXR_PCHAR("Using persistent connections\n") ;
  else
     ULXR_COUT << ULXR_PCHAR("Using non-persistent connections\n") ;

  prot.setChunkedTransfer(chunked);

  try
  {
    server.addMethod(&arrayOfStructs,
                     Integer::getValueName(),
                     ULXR_PCHAR("validator1.arrayOfStructs"),
                     Array::getValueName(),
                     ULXR_PCHAR("Validator function to tot up elements in a Struct"));

    server.addMethod(&countTheEntities,
                     Struct::getValueName(),
                     ULXR_PCHAR("validator1.countTheEntities"),
                     RpcString::getValueName(),
                     ULXR_PCHAR("Validator function to count some characters in a string"));

    server.addMethod(&easyStructTest,
                     Integer::getValueName(),
                     ULXR_PCHAR("validator1.easyStructTest"),
                     Struct::getValueName(),
                     ULXR_PCHAR("Validator function to tot up some Struct members"));

    server.addMethod(&echoStructTest,
                     Struct::getValueName(),
                     ULXR_PCHAR("validator1.echoStructTest"),
                     Struct::getValueName(),
                     ULXR_PCHAR("Validator function echoing a Struct completely back"));

    server.addMethod(&manyTypesTest,
                     Array::getValueName(),
                     ULXR_PCHAR("validator1.manyTypesTest"),
                     Integer::getValueName()
                     +ULXR_PCHAR(",") + Boolean::getValueName()
                     +ULXR_PCHAR(",") + RpcString::getValueName()
                     +ULXR_PCHAR(",") + Double::getValueName()
                     +ULXR_PCHAR(",") + DateTime::getValueName()
                     +ULXR_PCHAR(",") + Base64::getValueName(),
                     ULXR_PCHAR("Validator function return all input parameters back as array"));

    server.addMethod(&moderateSizeArrayCheck,
                     RpcString::getValueName(),
                     ULXR_PCHAR("validator1.moderateSizeArrayCheck"),
                     Array::getValueName(),
                     ULXR_PCHAR("Validator function returns the first + the last elements of a string array"));

    server.addMethod(&nestedStructTest,
                     Integer::getValueName(),
                     ULXR_PCHAR("validator1.nestedStructTest"),
                     Struct::getValueName(),
                     ULXR_PCHAR("Validator function returns sum of nested elements"));

    server.addMethod(&simpleStructReturnTest,
                     Struct::getValueName(),
                     ULXR_PCHAR("validator1.simpleStructReturnTest"),
                     Integer::getValueName(),
                     ULXR_PCHAR("Validator function returns calculation with some nested elements"));

    server.addMethod(&shutdown,
                     ulxr::Signature(),
                     ULXR_PCHAR("shutdown"),
                     ulxr::Signature(),
                     ULXR_PCHAR("Shut down Worker"));

    for (unsigned i = 0; terminate_server == false /* i < 100 */; ++i)
    {
      ULXR_COUT << ULXR_PCHAR("Run ") << i << std::endl;
      MethodCall call = server.waitForCall();
      MethodResponse resp = server.dispatchCall(call);
      if (!prot.isTransmitOnly())
        server.sendResponse(resp);
      if (!prot.isPersistent())
        prot.close();
    }

    if (prot.isPersistent())
      ULXR_COUT << ULXR_PCHAR("Used persistent connections\n") ;
    else
      ULXR_COUT << ULXR_PCHAR("Used non-persistent connections\n") ;
  }

  catch(XmlException& xmlex)
  {
     res = 1;
     ULXR_COUT << ULXR_PCHAR("Xml Error occured: ") << ULXR_GET_STRING(xmlex.why()) << std::endl;
     ULXR_COUT << ULXR_PCHAR("  in line ") << xmlex.getErrorLine()
               << ULXR_PCHAR(". Reason: ") << ULXR_GET_STRING(xmlex.getErrorString()) << std::endl;

     if (prot.isOpen())
     {
       try{
         MethodResponse resp(1, xmlex.why() );
         if (!prot.isTransmitOnly())
           server.sendResponse(resp);
       }
       catch(...)
       {
         ULXR_COUT << ULXR_PCHAR("error within exception occured\n");
       }
       prot.close();
     }
  }

  catch(Exception& ex)
  {
     res = 1;
     ULXR_COUT << ULXR_PCHAR("Error occured: ") << ULXR_GET_STRING(ex.why()) << std::endl;
     if (prot.isOpen())
     {
       try{
         MethodResponse resp(1, ex.why() );
         if (!prot.isTransmitOnly())
           server.sendResponse(resp);
       }
       catch(...)
       {
         ULXR_COUT << ULXR_PCHAR("error within exception occured\n");
       }
       prot.close();
     }
  }

  ULXR_COUT << ULXR_PCHAR("Terminating.\n");
  return res;
}
