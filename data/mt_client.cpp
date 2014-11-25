/***************************************************************************
       mt_client.cpp  --  test file for a mulithreaded client

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: mt_client.cpp 1151 2009-08-12 15:12:01Z ewald-arnold $

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

#include <cstring>
#include <iostream>
#include <algorithm>
#include <memory>

#include "util.c"

#ifdef ULXR_MULTITHREADED

#include <ulxmlrpcpp/ulxr_tcpip_connection.h>  // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_mutex.h>
#include <ulxmlrpcpp/ulxr_except.h>


#ifdef __WIN32__
const unsigned max_requests = 20;         // os-dependend: number of parallel threads
#else
const unsigned max_requests = 200;
#endif

#define SLEEP_FACTOR   1  // 0 or 1
#define RUN_SLEEP(x)   do { x; } while (false)

unsigned multi_cnt = 0;
ulxr::Mutex multi_mutex;

std::vector<unsigned> meth_nums;
std::vector<unsigned> func_nums;

unsigned global_errors = 0;

void lister_func (const ulxr::MethodResponse &resp)
{
  try
  {
    if (!resp.isOK())
    {
      ++global_errors;
      ulxr::Struct st = resp.getResult();
      int ec = ulxr::Integer(st.getMember(ULXR_PCHAR("faultCode"))).getInteger();
      ulxr::CppString es = ulxr::RpcString(st.getMember(ULXR_PCHAR("faultString"))).getString();
      ULXR_COUT << ULXR_PCHAR("A problem has happened.")
                << ULXR_PCHAR(" Error code: ") << ec
                << ULXR_PCHAR(" Error string: ") << es << ULXR_PCHAR("\n");
      return;
    }

    ULXR_COUT << ULXR_PCHAR("response to multi func: ") << ulxr::Integer(resp.getResult()).getInteger() << std::endl;
//  ULXR_COUT << resp.getXml() << std::endl;
    for (unsigned i = 0; i < 100; ++i)
#ifdef __unix__
      RUN_SLEEP(usleep(1000 * SLEEP_FACTOR));
#elif defined(__WIN32__)
      RUN_SLEEP(Sleep(1 * SLEEP_FACTOR));
#else
#error platform not supported
#endif
    {  // mutual protection area start
      ulxr::Mutex::Locker locker(multi_mutex);
      unsigned num = ulxr::Integer(resp.getResult()).getInteger();
      func_nums.push_back(num);
      multi_cnt++;
    }
  }

  catch(...)
  {
    ++global_errors;
    ULXR_COUT << ULXR_PCHAR("A problem occured while processing the rpc response\n");
    return;
  }
}


class lister_class
{
 public:
  void lister(const ulxr::MethodResponse &resp)
  {
    try
    {
      if (!resp.isOK())
      {
        ++global_errors;
        ulxr::Struct st = resp.getResult();
        int ec = ulxr::Integer(st.getMember(ULXR_PCHAR("faultCode"))).getInteger();
        ulxr::CppString es = ulxr::RpcString(st.getMember(ULXR_PCHAR("faultString"))).getString();
        ULXR_COUT << ULXR_PCHAR("A problem has happened.")
                  << ULXR_PCHAR(" Error code: ") << ec
                  << ULXR_PCHAR(" Error string: ") << es << ULXR_PCHAR("\n");
        return;
      }

      ULXR_COUT << ULXR_PCHAR("response to multi method: ") << ulxr::Integer(resp.getResult()).getInteger() << std::endl;
//    ULXR_COUT << resp.getXml() << std::endl;
      for (unsigned i = 0; i < 100; ++i)
#ifdef __unix__
        RUN_SLEEP(usleep(1000 * SLEEP_FACTOR));
#elif defined(__WIN32__)
        RUN_SLEEP(Sleep(1 * SLEEP_FACTOR));
#else
#error platform not supported
#endif

      {  // mutual protection area start
        ulxr::Mutex::Locker locker(multi_mutex);
        unsigned num = ulxr::Integer(resp.getResult()).getInteger();
        meth_nums.push_back(num);
        multi_cnt++;
      }
    }

    catch(...)
    {
      ++global_errors;
      ULXR_COUT << ULXR_PCHAR("A problem occured while processing the rpc response\n");
      return;
    }
  }
};

#endif// ULXR_MULTITHREADED

int main(int argc, char **argv)
{
  int ret = 0;

#ifdef ULXR_MULTITHREADED

#ifdef __unix__
#ifdef PTHREAD_THREADS_MAX
  ULXR_COUT << ULXR_PCHAR("PTHREAD_THREADS_MAX ") << PTHREAD_THREADS_MAX << ULXR_PCHAR("\n");
#endif
#endif

  try
  {
    ulxr::intializeLog4J(argv[0]);
    ulxr::getLogger4J()->send(ULXR_PCHAR("DEBUG"),
                              ULXR_PCHAR("mt_client started"),
                              ULXR_GET_STRING(__FILE__),
                              __LINE__);

    ulxr::MethodCall testcall_in_class_dynamic (ULXR_PCHAR("testcall_in_class_dynamic"));

    ulxr::CppString host = ULXR_PCHAR("localhost");
    if (argc > 1)
      host = ULXR_GET_STRING(argv[1]);

    unsigned port = 32005;
    if (argc > 2)
      port = ulxr_atoi(argv[2]);

    unsigned num_requests = 1000;
    if (argc > 3)
       num_requests = ulxr_atoi(argv[3]);
    num_requests *= 2;   // ensure EVEN number
                         // AND more calls than server threads

    bool wbxml = haveOption(argc, argv, "wbxml");
    bool secure = haveOption(argc, argv, "ssl");
    bool chunked = haveOption(argc, argv, "chunked");
    bool persistent = haveOption(argc, argv, "persistent");

    ulxr::CppString sec = ULXR_PCHAR("unsecured");
    if (secure)
      sec = ULXR_PCHAR("secured");

    ULXR_COUT << ULXR_PCHAR("Sending ") << sec << ULXR_PCHAR(" rpc requests at ") << host << ULXR_PCHAR(":") << port << std::endl;
    ULXR_COUT << ULXR_PCHAR("WBXML: ") << wbxml << std::endl
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
    if (persistent)
      conn->setTcpNoDelay(true);

    if (prot.isPersistent())
      ULXR_COUT << ULXR_PCHAR("Using persistent connections\n") ;
    else
      ULXR_COUT << ULXR_PCHAR("Using non-persistent connections\n") ;

    ulxr::Requester client(&prot, wbxml);
    lister_class lc;

    for (unsigned i0 = 0; i0 < num_requests / 2; )
    {
      if (client.numPendingRequests() < (max_requests / 2))
      {
        ULXR_COUT << ULXR_PCHAR("sending index: ") << i0 << std::endl;
        testcall_in_class_dynamic.setParam(ulxr::Integer(i0));
        client.call(testcall_in_class_dynamic, ULXR_PCHAR("/RPC2"), ulxr::make_receiver(lc, &lister_class::lister));
        client.call(testcall_in_class_dynamic, ULXR_PCHAR("/RPC2"), ulxr::make_receiver(lister_func));
#ifdef __unix__
        RUN_SLEEP(usleep(1000 * SLEEP_FACTOR));
#elif defined(__WIN32__)
        RUN_SLEEP(Sleep(1 * SLEEP_FACTOR));
#else
#error platform not supported
#endif
        ++i0;
      }
      else
      {
        ULXR_COUT << ULXR_PCHAR("currently too many running threads, waiting a bit: ") << client.numPendingRequests() << std::endl;
#ifdef __unix__
        usleep(500000);
#elif defined(__WIN32__)
        Sleep(500);
#else
#error platform not supported
#endif
      }
    }
/*
    while(true)
      usleep(1000 * 10);
*/
    ULXR_COUT << ULXR_PCHAR("All calls sent, waiting for processing to complete\n");

    while (multi_cnt < num_requests)
    {
      ULXR_COUT << ULXR_PCHAR("Waiting for all methods to complete, remaining ")
                << num_requests - multi_cnt
                << ULXR_PCHAR("\n");
#ifdef __unix__
      usleep(1000);
#elif defined(__WIN32__)
      Sleep(1);
#else
#error platform not supportedRuntimeException
#endif
    }

    ULXR_COUT << ULXR_PCHAR("Checking if all indices were received\n");
    bool problem = false;
    for (unsigned i1 = 0; i1 < num_requests / 2; ++i1)
      if (meth_nums.end() == std::find(meth_nums.begin(), meth_nums.end(), i1))
      {
        ULXR_COUT << ULXR_PCHAR("Index via meth_nums not received: ") << i1 << std::endl;
        problem = true;
      }

    for (unsigned i2 = 0; i2 < num_requests / 2; ++i2)
      if (func_nums.end() == std::find(func_nums.begin(), func_nums.end(), i2))
      {
        ULXR_COUT << ULXR_PCHAR("Index via func_nums not received: ") << i2 << std::endl;
        problem = true;
      }

    if (!problem)
      ULXR_COUT << ULXR_PCHAR("All indices were received\n");


    ulxr::MethodCall number (ULXR_PCHAR("testcall_numthreads"));
    ulxr::MethodResponse resp = client.call(number, ULXR_PCHAR("/RPC2"));
    ulxr::Integer num = resp.getResult();
    ULXR_COUT << ULXR_PCHAR("Shutting ") << num.getInteger() << ULXR_PCHAR(" servers down..\n");
    ulxr::MethodCall shutter (ULXR_PCHAR("testcall_shutdown"));

    for (unsigned i = 0; i < (unsigned) num.getInteger(); ++i)
    {
      try
      {
         client.call(shutter, ULXR_PCHAR("/RPC2"));  // don't expect an answer
      }

      catch(ulxr::Exception &ex)
      {
        ULXR_COUT << ULXR_PCHAR("Unexpected problem while transmit()ing shutdown request " << i << ".\n")
                  << ex.why() << std::endl;
        ret = 1;
        break;
      }

      catch(...)
      {
        ULXR_COUT << ULXR_PCHAR("Unexpected problem while transmit()ing shutdown request " << i << ".\n");
        ret = 1;
        break;
      }
    }

    ULXR_COUT << ULXR_PCHAR("Well done.\n");
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

  if (global_errors != 0)
  {
     ULXR_COUT << ULXR_PCHAR("Some side effects occured: ") << global_errors << std::endl;
     return 1;
  }
  else
     ULXR_COUT << ULXR_PCHAR("No side effects occured.") << std::endl;

  ULXR_COUT << ULXR_PCHAR("Ready.") << std::endl;

#else // ULXR_MULTITHREADED

  ULXR_COUT << ULXR_PCHAR("Multithreaded support disabled.");

#endif

  return ret;
}
