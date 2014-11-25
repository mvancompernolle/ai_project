/***************************************************************************
       mt_server.cpp  --  test file for a mulithreaded server

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: mt_server.cpp 1151 2009-08-12 15:12:01Z ewald-arnold $

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

#include <cstring>
#include <cstdlib>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h>  // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>

#ifdef __WIN32__
#include <process.h>
#endif

#include "util.c"

#ifdef ULXR_MULTITHREADED

#ifdef __unix__
#include <pthread.h>
#endif

#endif

#include <iostream>
#include <cstring>
#include <memory>

#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_signature.h>
#include <ulxmlrpcpp/ulxr_mtrpc_server.h>


#ifdef __WIN32__
const unsigned num_threads = 10;
#else
const unsigned num_threads = 100;
#endif

////////////////////////////////////////////////////////////////////////

#ifdef ULXR_MULTITHREADED

ulxr::MultiThreadRpcServer *mtServer = 0;

class TestWorker
{
 public:

   TestWorker ()
   {
   }

   ulxr::MethodResponse numThreads (const ulxr::MethodCall &/*calldata*/)
   {
     ulxr::MethodResponse resp;
     resp.setResult(ulxr::Integer(mtServer->numThreads()));
     return resp;
   }

   ulxr::MethodResponse shutdown (const ulxr::MethodCall &/*calldata*/)
   {
     ULXR_COUT << ULXR_PCHAR("TestWorker got signal to shut down\n");
     ulxr::MethodResponse resp;
     resp.setResult(ulxr::Boolean(true));
     ULXR_COUT << ULXR_PCHAR(" Terminating..\n");
     mtServer->terminateAllThreads();
     ULXR_COUT << ULXR_PCHAR(" Returning..\n");
     return resp;
   }

   ulxr::MethodResponse testcall (const ulxr::MethodCall &calldata)
   {
     ulxr::Integer I = calldata.getParam(0);
     int p1 = I.getInteger();
     ULXR_COUT << ULXR_PCHAR("TestWorker got call(")  << p1 << ULXR_PCHAR(") \n");
     ulxr::MethodResponse resp;
     resp.setResult(I);
     return resp;
   }
};

#endif // ULXR_MULTITHREADED

////////////////////////////////////////////////////////////////////////


int main(int argc, char **argv)
{
#ifdef ULXR_MULTITHREADED

  try
  {
    ulxr::intializeLog4J(argv[0]);
    ulxr::getLogger4J()->send(ULXR_PCHAR("DEBUG"),
                              ULXR_PCHAR("mt_server started"),
                              ULXR_GET_STRING(__FILE__),
                              __LINE__);

    ulxr::CppString host = ULXR_PCHAR("localhost");
    if (argc > 1)
      host = ULXR_GET_STRING(argv[1]);

    unsigned port = 32005;
    if (argc > 2)
      port = ulxr_atoi(argv[2]);

    bool wbxml = haveOption(argc, argv, "wbxml");
    bool secure = haveOption(argc, argv, "ssl");
    bool chunked = haveOption(argc, argv, "chunked");
    bool shutme = haveOption(argc, argv, "shutdown");
    bool persistent = haveOption(argc, argv, "persistent");

    ulxr::CppString sec = ULXR_PCHAR("unsecured");
    if (secure)
      sec = ULXR_PCHAR("secured");

    ULXR_COUT << ULXR_PCHAR("Serving ") << sec << ULXR_PCHAR(" rpc requests at ") << host << ULXR_PCHAR(":") << port << std::endl;
    ULXR_COUT << ULXR_PCHAR("WBXML: ") << wbxml << std::endl
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

    std::auto_ptr<ulxr::HttpProtocol> prot(new ulxr::HttpProtocol(conn.get()));
    prot->setChunkedTransfer(chunked);
    prot->setPersistent(persistent);
    if (persistent)
      conn->setTcpNoDelay(true);

    if (prot->isPersistent())
      ULXR_COUT << ULXR_PCHAR("Using persistent connections\n") ;
    else
      ULXR_COUT << ULXR_PCHAR("Using non-persistent connections\n") ;

    ulxr::MultiThreadRpcServer server(prot.get(), num_threads, wbxml);
    mtServer = &server;

    TestWorker worker;

    server.addMethod(ulxr::make_method(worker, &TestWorker::testcall),
                     ulxr::Signature(ulxr::Struct()),
                     ULXR_PCHAR("testcall_in_class_dynamic"),
                     ulxr::Signature(ulxr::Integer()),
                     ULXR_PCHAR("Testcase with a dynamic method in a class"));

    server.addMethod(ulxr::make_method(worker, &TestWorker::shutdown),
                     ulxr::Signature(ulxr::Boolean()),
                     ULXR_PCHAR("testcall_shutdown"),
                     ulxr::Signature(),
                     ULXR_PCHAR("Testcase with a dynamic method in a class, shut down server, return old state"));

    server.addMethod(ulxr::make_method(worker, &TestWorker::numThreads),
                     ulxr::Signature(ulxr::Integer()),
                     ULXR_PCHAR("testcall_numthreads"),
                     ulxr::Signature(),
                     ULXR_PCHAR("Returns number of installed threads at startup"));

    unsigned started = server.dispatchAsync();
    ULXR_COUT << ULXR_PCHAR("Started ") << started << ULXR_PCHAR(" threads for dispatching rpc requests\n");

    if (shutme)
    {
      ULXR_COUT << ULXR_CHAR("sleep before shutting down\n");
#ifdef __unix__
      usleep(5 * 1000 * 1000);
#elif defined(__WIN32__)
      Sleep(5 * 1000);
#else
#error platform not supported
#endif
      ULXR_COUT << ULXR_CHAR("shutdownAllThreads\n");
      server.shutdownAllThreads();
    }

    server.waitAsync(false, true);
    ULXR_COUT << ULXR_PCHAR("Done.\n");
  }

  catch(ulxr::Exception& ex)
  {
    ULXR_COUT << ULXR_PCHAR("Error occured: ") << ULXR_GET_STRING(ex.why()) << std::endl;
    return 1;
  }

  catch(...)
  {
    ULXR_COUT << ULXR_PCHAR("Unknown error occured\n");
    return 1;
  }

  ULXR_COUT << ULXR_PCHAR("Ready.") << std::endl;

#else // ULXR_MULTITHREADED

  ULXR_COUT << ULXR_PCHAR("Multithreaded support disabled.");

#endif

  return 0;
}
