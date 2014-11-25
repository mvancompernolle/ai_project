/***************************************************************************
             rpc_client.cpp  --  test file for an rpc client

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: rpc_client.cpp 1151 2009-08-12 15:12:01Z ewald-arnold $

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
#include <ctime>
#include <memory>
#include <cstring>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h>  // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_log4j.h>

#include "util.c"

int main(int argc, char **argv)
{
  try
  {
    ulxr::intializeLog4J(argv[0]);
    ulxr::getLogger4J()->send(ULXR_PCHAR("DEBUG"),
                              ULXR_PCHAR("client started"),
                              ULXR_GET_STRING(__FILE__),
                              __LINE__);

    ulxr::CppString host = ULXR_PCHAR("localhost");
    if (argc > 1)
      host = ULXR_GET_STRING(argv[1]);

    unsigned port = 32000;
    if (argc > 2)
      port = ulxr_atoi(argv[2]);

    bool big = haveOption(argc, argv, "big");
    bool secure = haveOption(argc, argv, "ssl");
    bool chunked = haveOption(argc, argv, "chunked");
    bool persistent = haveOption(argc, argv, "persistent");

    ulxr::CppString sec = ULXR_PCHAR("unsecured");
    if (secure)
      sec = ULXR_PCHAR("secured");

    ULXR_COUT << ULXR_PCHAR("Requesting ") << sec << ULXR_PCHAR(" rpc calls at ")
              << host << ULXR_PCHAR(":") << port << std::endl
              << ULXR_PCHAR("Chunked transfer: ") << chunked << std::endl;

    std::auto_ptr<ulxr::TcpIpConnection> conn;
#ifdef ULXR_INCLUDE_SSL_STUFF
    if (secure)
      conn.reset(new ulxr::SSLConnection (false, host, port));
    else
#endif
#ifdef _MSC_VER
    {
   	  std::auto_ptr<ulxr::TcpIpConnection> temp(new ulxr::TcpIpConnection (false, host, port));
	  conn = temp;
	}
#else
      conn.reset(new ulxr::TcpIpConnection (false, host, port));
#endif

    ulxr::HttpProtocol prot(conn.get());
    prot.setChunkedTransfer(chunked);
    prot.setPersistent(persistent);
    ulxr::Requester client(&prot);

    if (prot.isPersistent())
      ULXR_COUT << ULXR_PCHAR("Using persistent connections\n") ;
    else
      ULXR_COUT << ULXR_PCHAR("Using non-persistent connections\n") ;

    ulxr_time_t starttime = ulxr_time(0);

    ulxr::MethodCall testcall_shutdown (ULXR_PCHAR("testcall_shutdown"));

    ulxr::MethodCall testcall_function (ULXR_PCHAR("testcall_function"));
    testcall_function.addParam(ulxr::Integer(456))
                     .addParam(ulxr::Integer(123));

    ulxr::MethodCall stringcall (ULXR_PCHAR("stringcall"));
//    ulxr::CppString big_str (5 * 1000 * 1000, ULXR_CHAR('x'));
    ulxr::CppString big_str (10 * 1000 * 100, ULXR_CHAR('<'));
    stringcall.addParam(ulxr::RpcString(big_str));

    ulxr::MethodCall testcall_static (ULXR_PCHAR("testcall_in_class_static"));

    ulxr::MethodCall testcall_dynamic (ULXR_PCHAR("testcall_in_class_dynamic"));
    testcall_dynamic.addParam(ulxr::Integer(456));

    /////////////////////////////

    ulxr::MethodCall list_methods (ULXR_PCHAR("system.listMethods"));

    ulxr::MethodCall method_help (ULXR_PCHAR("system.methodHelp"));
    method_help.addParam(ulxr::RpcString(ULXR_PCHAR("system.methodHelp")));

    ulxr::MethodCall method_sig (ULXR_PCHAR("system.methodSignature"));
    method_sig.addParam(ulxr::RpcString(ULXR_PCHAR("system.methodSignature")));

    ulxr::MethodCall method_sig2 (ULXR_PCHAR("system.methodSignature"));
    method_sig2.addParam(ulxr::RpcString(ULXR_PCHAR("testcall_in_class_dynamic")));

    ulxr::MethodResponse resp;

    /////////////////////////////

    ULXR_COUT << ULXR_PCHAR("call list_methods: \n");
    prot.setAcceptCookies(true);
    prot.setClientCookie(ULXR_PCHAR(" cookie-test=123 "));
    resp = client.call(list_methods, ULXR_PCHAR("/RPC2"));

#ifdef ULXR_DEBUG_OUTPUT
    ULXR_COUT << ULXR_PCHAR("call result: \n");
    ULXR_COUT << resp.getXml(0) << std::endl;
#endif

    if (!prot.hasCookie() || prot.getCookie() != ULXR_PCHAR("cookie-test=123; received=true"))
    {
      ULXR_COUT << ULXR_PCHAR("bad received cookie: ") << prot.getCookie() << std::endl;
      return 1;
    }

    ULXR_COUT << ULXR_PCHAR("call method_help: \n");
    prot.setClientCookie(ULXR_PCHAR("  cookie-test-1=123;   cookie-test-2=456;   cookie-test-3=789  "));
    resp = client.call(method_help, ULXR_PCHAR("/RPC2"));

#ifdef ULXR_DEBUG_OUTPUT
    ULXR_COUT << ULXR_PCHAR("call result: \n");
    ULXR_COUT << resp.getXml(0) << std::endl;
#endif

    if (!prot.hasCookie() || prot.getCookie() != ULXR_PCHAR("cookie-test-1=123; cookie-test-2=456; cookie-test-3=789; received=true"))
    {
      ULXR_COUT << ULXR_PCHAR("bad received cookie: ") << prot.getCookie() << std::endl;
      return 1;
    }

    ULXR_COUT << ULXR_PCHAR("call method_sig: \n");
    prot.setClientCookie(ULXR_PCHAR(""));
    resp = client.call(method_sig, ULXR_PCHAR("/RPC2"));

    ULXR_COUT << ULXR_PCHAR("hostname: ") << conn->getHostName() << std::endl;
    ULXR_COUT << ULXR_PCHAR("peername: ") << conn->getPeerName() << std::endl;

#ifdef ULXR_DEBUG_OUTPUT
    ULXR_COUT << ULXR_PCHAR("call result: \n");
    ULXR_COUT << resp.getXml(0) << std::endl;
#endif

    if (!prot.hasCookie() || prot.getCookie() != ULXR_PCHAR("newcookie=mycookie"))
    {
      ULXR_COUT << ULXR_PCHAR("bad received cookie: ") << prot.getCookie() << std::endl;
      return 1;
    }

    ULXR_COUT << ULXR_PCHAR("call method_sig2: \n");
    resp = client.call(method_sig2, ULXR_PCHAR("/RPC2"));

#ifdef ULXR_DEBUG_OUTPUT
    ULXR_COUT << ULXR_PCHAR("call result: \n");
    ULXR_COUT << resp.getXml(0) << std::endl;
#endif

    ULXR_COUT << ULXR_PCHAR("call testcall_function: \n");
    resp = client.call(testcall_function, ULXR_PCHAR("/RPC2"));

#ifdef ULXR_DEBUG_OUTPUT
    ULXR_COUT << ULXR_PCHAR("call result: \n");
    ULXR_COUT << resp.getXml(0) << std::endl;
#endif

    ULXR_COUT << ULXR_PCHAR("call testcall_static: \n");
    resp = client.call(testcall_static, ULXR_PCHAR("/RPC2"));

#ifdef ULXR_DEBUG_OUTPUT
    ULXR_COUT << ULXR_PCHAR("call result: \n");
    ULXR_COUT << resp.getXml(0) << std::endl;
#endif

    ULXR_COUT << ULXR_PCHAR("call testcall_dynamic: \n");
    resp = client.call(testcall_dynamic, ULXR_PCHAR("/RPC2"));

#ifdef ULXR_DEBUG_OUTPUT
    ULXR_COUT << ULXR_PCHAR("call result: \n");
    ULXR_COUT << resp.getXml(0) << std::endl;
#endif

    ulxr::CppString ret_str;
    if (big)
    {
      ULXR_COUT << ULXR_PCHAR("call stringcall: \n");
      resp = client.call(stringcall, ULXR_PCHAR("/RPC2"));

      ret_str = resp.getXml(0);

#ifdef ULXR_DEBUG_OUTPUT
      ULXR_COUT << ULXR_PCHAR("call result: \n");

      ULXR_COUT << ULXR_PCHAR("call result: [No need to display ")
                << ret_str.length()
                << ULXR_PCHAR(" bytes.]\n");
  //    ULXR_COUT << resp.getXml(0) << std::endl;
#endif
    }

    ULXR_COUT << ULXR_PCHAR("call testcall_shutdown: \n");
    resp = client.call(testcall_shutdown, ULXR_PCHAR("/RPC2"));

#ifdef ULXR_DEBUG_OUTPUT
    ULXR_COUT << ULXR_PCHAR("call result: \n");
    ULXR_COUT << resp.getXml(0) << std::endl;
#endif

    if (prot.isPersistent())
      ULXR_COUT << ULXR_PCHAR("Used persistent connections\n") ;
    else
      ULXR_COUT << ULXR_PCHAR("Used non-persistent connections\n") ;

    ulxr_time_t endtime = ulxr_time(0);
    ulxr_time_t totalsecs = endtime - starttime;
    ulxr_time_t mins = totalsecs / 60;
    ulxr_time_t secs = totalsecs % 60;
    ULXR_COUT << ULXR_PCHAR("\nOverall time needed: ")
               << mins << ULXR_PCHAR(":") << secs << std::endl;
    if (big)
      ULXR_COUT << ULXR_PCHAR("Two-way transmission of ")
                << ret_str.length() << ULXR_PCHAR(" bytes")
                << ULXR_PCHAR("\nKBytes/s: ") << (ret_str.length() / totalsecs / 1024 * 2) << std::endl;
  }

  catch(ulxr::Exception &ex)
  {
     ULXR_COUT << ULXR_PCHAR("Error occured: ") << ULXR_GET_STRING(ex.why()) << std::endl;
     return 1;
  }

  catch(...)
  {
     ULXR_COUT << ULXR_PCHAR("unknown Error occured.\n");
     return 1;
  }

  ULXR_COUT << ULXR_PCHAR("Well done, Ready.\n");
  return 0;
}
