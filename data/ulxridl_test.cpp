/***************************************************************************
                 ulxridl_test.cpp  -  test generated files
                             -------------------
    begin                : Sun Jul 23 2007
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxridl_test.cpp 1034 2007-07-29 14:19:13Z ewald-arnold $

 ***************************************************************************/


//#define ULXR_UNICODE_ONLY_HELPERS
#include <ulxmlrpcpp/ulxmlrpcpp.h> // always first

#include <iostream>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_mtrpc_server.h>

#include "../ulxr2xml/ulxridl_test.h"
#include "new-meth/UlxrIdlTest_ulxr_client.h"
#include "new-meth/UlxrIdlTest_ulxr_server.h"
#include "new-meth/UlxrIdlTestFuncs_ulxr_client.h"
#include "new-meth/UlxrIdlTestFuncs_ulxr_server.h"


ulxr::MultiThreadRpcServer *my_handler;

namespace myname {

UlxrIdlTest::UlxrIdlTest()
  : IdlInterface("name")
{}

UlxrIdlTest::UlxrIdlTest(int i, const std::string &s)
  : IdlInterface("name")
{}

UlxrIdlTest::UlxrIdlTest(int i, const std::string &s, long last)
  : IdlInterface("name")
{}

UlxrIdlTest::~UlxrIdlTest()
{}

void UlxrIdlTest::first_url4()
{
  ULXR_COUT << ULXR_PCHAR("got signal to shut down\n");
  my_handler->terminateAllThreads();
}

std::string * UlxrIdlTest::first_url4(int i)
{ return 0; }

std::string * UlxrIdlTest::first_url4(long li)
{ return 0; }

const char * UlxrIdlTest::first_url4(float li)
{ return 0; }

wchar_t * const UlxrIdlTest::first_url4(double li)
{ return 0; }

std::string UlxrIdlTest::constTest() const
{
  throw ulxr::RuntimeException(123, ULXR_PCHAR("my exception"));
}

std::string & UlxrIdlTest::constRefTest() const
{  static std::string s; return s; }

long* UlxrIdlTest::not_firstViewConst() const
{ return 0; }

float* *UlxrIdlTest::not_firstView()
{ return 0; }

double* UlxrIdlTest::not_nextView()
{ return 0; }

bool* UlxrIdlTest::not_nextView2()
{ return 0; }

long* UlxrIdlTest::firstViewConst() const
{ return 0; }

std::string* UlxrIdlTest::firstView()
{ return 0; }

std::wstring* UlxrIdlTest::nextView1(int i, long * l, std::string &s, const bool * b, char c) const
{ return 0; }

std::basic_string<char>* UlxrIdlTest::nextView2(int i, long l, std::string &s, bool * const b, char c) const
{ return 0; }

void UlxrIdlTest::getNumObjects(std::string s)
{}

const std::basic_string<wchar_t>* UlxrIdlTest::getObject(const std::string &s)
{
  static std::wstring rs;
  rs = L"getObject-";
  rs += ulxr::getUnicode(s);
  return &rs;
}

int UlxrIdlTest::getNumPages(const std::wstring &s) throw(int, long)
{ return 0; }

const std::string * UlxrIdlTest::last_url4()
{ return 0; }

std::string * UlxrIdlTest::last_url4(int i)
{ return 0; }

std::string * UlxrIdlTest::last_url4(long li) throw(int, float)
{ return 0; }

}

//-------------------------------------

// namespace funcs {
//
// void free_function1(int i)
// {
// }
//
// long free_function1(long i, float f)
// {
//   return -2l + (long) i + (long) f;
// }
//
// }

// -----------------------------------------------------------------------------

int main(int argc, char**argv)
{
  ulxr::intializeLog4J(argv[0]);

  ulxr::CppString host = ULXR_PCHAR("localhost");
  unsigned port = 32010;

  // --

  ulxr::TcpIpConnection client_conn (false, host, port);
  ulxr::HttpProtocol client_prot(&client_conn);
  ulxr::Requester requester(&client_prot);

  ulxr::CppString user = ULXR_CHAR("ali-baba");
  ulxr::CppString pass = ULXR_CHAR("open-sesame");
  ulxr::CppString realm = ULXR_CHAR("/RPC2");

  UlxrIdlTestClient client(requester, realm, user, pass);
  UlxrIdlTestFuncs_setClientCredentials(requester, realm, user, pass);

  // --

  myname::UlxrIdlTest worker;

  ulxr::TcpIpConnection server_conn (true, host, port);
  ulxr::HttpProtocol server_prot(&server_conn);

  ulxr::MultiThreadRpcServer handler(&server_prot, 2, false);
  my_handler = &handler;
  UlxrIdlTestServer server(handler, worker);
  UlxrIdlTestFuncs_setupServerMethods(handler);

  handler.dispatchAsync();
  sleep(1);

  std::wstring ws = *client.getObject("client-test");
  std::cout << "return from remote call: " << ulxr::getLatin1(ws) << std::endl;

  try
  {
    client.constTest();
  }
  catch(std::exception &ex)
  {
    std::cout << "exception occured: " << ulxr::getLatin1(ULXR_GET_STRING(ex.what())) << std::endl;
  }
  catch(...)
  {
    std::cout << "unknown excption occured\n";
  }

  ulxr::CppString ss = ULXR_PCHAR("afasfasfd");
  std::cout << "sizeof(CppString[0]) = " << sizeof(ss[0]) << "\n";

  client.first_url4();
  client.first_url4();

  handler.waitAsync(false, true);
  UlxrIdlTestFuncs_removeServerMethods(handler);

  return 0;
}


