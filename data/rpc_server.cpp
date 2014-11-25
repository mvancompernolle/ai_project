/***************************************************************************
               rpc_server.cpp  --  test file for an rpc server

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: rpc_server.cpp 1151 2009-08-12 15:12:01Z ewald-arnold $

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
//      << ("second" << i2)
#define ULXR_STRUCT_MEMBER_FROM_NAME_VALUE

#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h>  // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_signature.h>
#include <ulxmlrpcpp/ulxr_dispatcher.h>
#include <ulxmlrpcpp/ulxr_log4j.h>

#include "util.c"

class TestClass
{
 public:
   static ulxr::MethodResponse testcall  (const ulxr::MethodCall &calldata);
   static ulxr::MethodResponse getInput  (const ulxr::MethodCall &calldata);
   static ulxr::MethodResponse getAllInputs  (const ulxr::MethodCall &calldata);
   static ulxr::MethodResponse setOutput (const ulxr::MethodCall &calldata);
};


ulxr::MethodResponse TestClass::testcall (const ulxr::MethodCall &/*calldata*/)
{
  ulxr::MethodResponse resp(ulxr::Integer(654321));
  resp.setResult(ulxr::Integer(123456));
  return resp;
}


ulxr::MethodResponse TestClass::getAllInputs (const ulxr::MethodCall &calldata)
{
  ulxr::CppString s;
  for (unsigned i = 0; i < 6; ++i)
  {
    bool b = 1 & ulxr::getRand();
    if (b)
      s += '1';
    else
      s += '0';
  }
  return ulxr::MethodResponse (ulxr::RpcString(s));
}


ulxr::MethodResponse TestClass::getInput (const ulxr::MethodCall &calldata)
{
  ulxr::Integer nr = calldata.getParam(0);
  bool b = 1 & ulxr::getRand();
  return ulxr::MethodResponse (ulxr::Boolean(b));
}


ulxr::MethodResponse TestClass::setOutput (const ulxr::MethodCall &calldata)
{
  ulxr::Integer nr = calldata.getParam(0);
  ulxr::Boolean state = calldata.getParam(1);
  ULXR_COUT << ULXR_PCHAR("out ") << nr.getInteger() << ULXR_PCHAR(" ") << state.getBoolean() << ULXR_PCHAR("\n");
  ulxr::MethodResponse resp;
  return resp;
}


ulxr::MethodResponse stringcall (const ulxr::MethodCall &calldata)
{
  ulxr::RpcString rpcs = calldata.getParam(0);
  ulxr::CppString s = rpcs.getString();
  ulxr::MethodResponse resp;
  resp.setResult(ulxr::RpcString(s));
  return resp;
}


////////////////////////////////////////////////////////////////////////


class TestWorker
{
 public:

   TestWorker () : running(true)
   {}


   ulxr::MethodResponse shutdown (const ulxr::MethodCall &/*calldata*/)
   {
     ULXR_COUT << ULXR_PCHAR("got signal to shut down\n");
     ulxr::MethodResponse resp;
     resp.setResult(ulxr::Boolean(running));
     running = false;
     return resp;
   }

   ulxr::MethodResponse testcall1 (const ulxr::MethodCall &calldata)
   {
     ulxr::MethodResponse resp;
     ulxr::Struct st;
     st.addMember(ULXR_PCHAR("param 1"), calldata.getParam(0));
     return resp;
   }

   ulxr::MethodResponse testcall2 (const ulxr::MethodCall &/*calldata*/)
   {
     ulxr::MethodResponse resp;
     resp.setResult(ulxr::Integer(654321));
     return resp;
   }

   bool running;
};

////////////////////////////////////////////////////////////////////////

static ulxr::MethodResponse testcall (const ulxr::MethodCall &calldata)
{
  ulxr::MethodResponse resp;
  ulxr::Integer i = calldata.getParam(0);
  ulxr::Integer i2 = calldata.getParam(1);

  ulxr::Array ar;
  ar << ulxr::Integer(1) << ulxr::Integer(2) << ulxr::RpcString("3");

// upon requst you may also use intrinsic types like bool, int, double, char*
// which expand to their xmlrpc counterparts. Define before #including ulxr_value.h!!
// #define ULXR_USE_INTRINSIC_VALUE_TYPES
//  todo: ar << 11 << 22 << "33";

  ulxr::Struct st;

  st  << ulxr::make_member(ULXR_PCHAR("Hallo"), ar)

#ifdef ULXR_UNICODE
      << ulxr::make_member(ULXR_PCHAR("Hallo wstring"), ulxr::RpcString(L"wstring"))
#endif
      << ulxr::make_member(ULXR_PCHAR("before"), i)

// upon requst you may also use the following construct
//      << ("second" << i2)
// #define ULXR_STRUCT_MEMBER_FROM_NAME_VALUE

      << ulxr::make_member(ULXR_PCHAR("num_param"), ulxr::Integer(123))
      << ulxr::make_member(ULXR_PCHAR("after"), ulxr::Integer(i.getInteger()+1111))
      << ulxr::make_member(ULXR_PCHAR("difference"), ulxr::RpcString("1111"));

  resp.setResult(st);
  return ulxr::MethodResponse (st);
}


////////////////////////////////////////////////////////////////////////


int main(int argc, char **argv)
{
  ulxr::intializeLog4J(argv[0]);
  ulxr::getLogger4J()->send(ULXR_PCHAR("DEBUG"),
                            ULXR_PCHAR("server started"),
                            ULXR_GET_STRING(__FILE__),
                            __LINE__);

  ulxr::CppString host = ULXR_PCHAR("localhost");
  if (argc > 1)
    host = ULXR_GET_STRING(argv[1]);

  unsigned port = 32000;
  if (argc > 2)
    port = ulxr_atoi(argv[2]);

  bool secure = haveOption(argc, argv, "ssl");
  bool chunked = haveOption(argc, argv, "chunked");
  bool persistent = haveOption(argc, argv, "persistent");

  ulxr::CppString sec = ULXR_PCHAR("unsecured");
  if (secure)
    sec = ULXR_PCHAR("secured");

  ULXR_COUT << ULXR_PCHAR("Serving ") << sec << ULXR_PCHAR(" rpc requests at ")
            << host << ULXR_PCHAR(":") << port << std::endl
            << ULXR_PCHAR("Chunked transfer: ") << chunked << std::endl;

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
  prot.setChunkedTransfer(chunked);
  prot.setPersistent(persistent);
  ulxr::Dispatcher server(&prot);

  if (prot.isPersistent())
    ULXR_COUT << ULXR_PCHAR("Using persistent connections\n") ;
  else
    ULXR_COUT << ULXR_PCHAR("Using non-persistent connections\n") ;

  try
  {
    server.addMethod(ulxr::make_method(TestClass::testcall),
                     ulxr::Struct::getValueName(),
                     ULXR_PCHAR("testcall_in_class_static"),
                     ULXR_PCHAR(""),
                     ULXR_PCHAR("Testcase with a static method in a class"));

    server.addMethod(ulxr::make_method(testcall),
                     ulxr::Signature() << ulxr::Integer(),
                     ULXR_PCHAR("testcall_function"),
                     ulxr::Signature() << ulxr::Integer()
                                       << ulxr::Integer(),
                     ULXR_PCHAR("Testcase with a c-function"));

    server.addMethod(ulxr::make_method(TestClass::getInput),
                     ulxr::Signature() << ulxr::Boolean(),
                     ULXR_PCHAR("getInput"),
                     ulxr::Signature()<< ulxr::Integer(),
                     ULXR_PCHAR("get input state"));

    server.addMethod(ulxr::make_method(TestClass::getAllInputs),
                     ulxr::Signature() << ulxr::RpcString(),
                     ULXR_PCHAR("getAllInputs"),
                     ulxr::Signature(),
                     ULXR_PCHAR("get all input states"));

    server.addMethod(ulxr::make_method(TestClass::setOutput),
                     ulxr::Signature(),
                     ULXR_PCHAR("setOutput"),
                     ulxr::Signature() << ulxr::Integer()
                                       << ulxr::Boolean(),
                     ULXR_PCHAR("set output state"));

    server.addMethod(ulxr::make_method(stringcall),
                     ulxr::Signature() << ulxr::RpcString(),
                     ULXR_PCHAR("stringcall"),
                     ulxr::Signature() << ulxr::RpcString(),
                     ULXR_PCHAR("Testcase return input string"));

    TestWorker worker;

    server.addMethod(ulxr::make_method(worker, &TestWorker::testcall1),
                     ulxr::Signature(ulxr::Struct()),
                     ULXR_PCHAR("testcall_in_class_dynamic"),
                     ulxr::Signature(ulxr::Integer()),
                     ULXR_PCHAR("Testcase with a dynamic method in a class"));

    server.addMethod(ulxr::make_method(worker, &TestWorker::shutdown),
                     ulxr::Signature(ulxr::Boolean()),
                     ULXR_PCHAR("testcall_shutdown"),
                     ulxr::Signature(),
                     ULXR_PCHAR("Testcase with a dynamic method in a class, shut down server, return old state"));

    prot.setAcceptCookies(true);
    while (worker.running)
    {
      ulxr::MethodCall call = server.waitForCall();

      ULXR_COUT << ULXR_PCHAR("hostname: ") << conn->getHostName() << std::endl;
      ULXR_COUT << ULXR_PCHAR("peername: ") << conn->getPeerName() << std::endl;

      if (prot.hasCookie())
      {
        ulxr::CppString s = prot.getCookie();
        ULXR_COUT << ULXR_PCHAR("received cookie: ") << s << std::endl;
        prot.setServerCookie(s + ULXR_PCHAR("; received=true"));
      }
      else
      {
        prot.setServerCookie(ULXR_PCHAR("newcookie=mycookie"));
        ULXR_COUT << ULXR_PCHAR("no cookie received\n");
      }

      ulxr::MethodResponse resp = server.dispatchCall(call);
      if (!prot.isTransmitOnly())
        server.sendResponse(resp);

      if (!prot.isPersistent())
        prot.close();

      if (prot.isPersistent())
        ULXR_COUT << ULXR_PCHAR("Used persistent connections\n") ;
      else
        ULXR_COUT << ULXR_PCHAR("Used non-persistent connections\n") ;
    }
  }

  catch(ulxr::Exception& ex)
  {
    ULXR_COUT << ULXR_PCHAR("Error occured: ") << ULXR_GET_STRING(ex.why()) << std::endl;
    if (prot.isOpen())
    {
      try
      {
        ulxr::MethodResponse resp(1, ex.why() );
        if (!prot.isTransmitOnly())
          server.sendResponse(resp);
      }
      catch(...)
      {
        ULXR_COUT << ULXR_PCHAR("error within exception occured\n");
      }
      prot.close();
    }
    return 1;
  }
  ULXR_COUT << ULXR_PCHAR("Well done, Ready.\n");
  return 0;
}
