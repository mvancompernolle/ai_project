/***************************************************************************
        val1_client.cpp  --  client for userland validator tests

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    These validator functions are taken from the specs
    on http://validator.xmlrpc.com/ and should also run from
    their online validator website.

    $Id: val1_client.cpp 1027 2007-07-25 07:51:31Z ewald-arnold $

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
#include <cmath>
#include <memory>
#include <cstring>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h> // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_except.h>

#include "util.c"

using namespace ulxr;


void printResponse (const MethodCall &call, const MethodResponse &resp)
{
    ULXR_COUT << call.getMethodName() + ULXR_PCHAR(" returned error") << std::endl;
    Struct estruct = resp.getResult();
    Integer ecode = Integer(estruct.getMember(ULXR_PCHAR("faultCode")));
    RpcString estr = RpcString(estruct.getMember(ULXR_PCHAR("faultString")));
    ULXR_COUT << ULXR_PCHAR("Code: ") << ecode.getInteger() << std::endl;
    ULXR_COUT << ULXR_PCHAR("String: ") << estr.getString() << std::endl;
}


bool check_arrayOfStructs(Requester &client)
{
  Array arr;
  Struct str;

  int my_sum = 0;

  int val = 1+(int) (10000.0*ulxr::getRand()/(RAND_MAX+1.0));
  my_sum += val;
  str.clear();
  str.addMember(ULXR_PCHAR("curly"), Integer(val));
  str.addMember(ULXR_PCHAR("moe"),   Integer(54321));
  str.addMember(ULXR_PCHAR("larry"), Integer(54321));
  arr.addItem(str);

  val = 1+(int) (10000.0*ulxr::getRand()/(RAND_MAX+1.0));
  str.clear();
  my_sum += val;
  str.addMember(ULXR_PCHAR("curly"), Integer(val));
  str.addMember(ULXR_PCHAR("moe"),   Integer(12));
  str.addMember(ULXR_PCHAR("larry"), Integer(87));
  arr.addItem(str);

  val = 1+(int) (10000.0*ulxr::getRand()/(RAND_MAX+1.0));
  str.clear();
  my_sum += val;
  str.addMember(ULXR_PCHAR("curly"), Integer(val));
  str.addMember(ULXR_PCHAR("moe"),   Integer(278));
  str.addMember(ULXR_PCHAR("larry"), Integer(1));
  arr.addItem(str);

  MethodCall arrayOfStructs (ULXR_PCHAR("validator1.arrayOfStructs"));
  arrayOfStructs.addParam(arr);

  MethodResponse resp = client.call(arrayOfStructs, ULXR_PCHAR("/RPC2"));
  if (!resp.isOK())
  {
    printResponse(arrayOfStructs, resp);
    return false;
  }

  Integer sum = resp.getResult();

  return sum.getInteger() == my_sum;
}


bool check_countTheEntities (Requester &client)
{
  CppString s0 (ULXR_PCHAR("khera>&sd< asd<f>e56&&& \"$")
                ULXR_PCHAR("\"\"\'&<&>sd w45234908 0xd9f0��)/()/")
                ULXR_PCHAR("()N u6we4\"E!\"%&((>YXC>YXASESGYDX>YVBHJE%W&TW$"));

#ifndef ULXR_UNICODE
  CppString s = asciiToUtf8(s0); // we use utf8 internally!
#else
  CppString s = s0;
#endif

  for (unsigned ii = 0; ii < 5; ++ii)
    s += s;

  int rightangs = 0;
  int leftangs = 0;
  int ampers = 0;
  int quotes = 0;
  int apos = 0;
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

  MethodCall countTheEntities (ULXR_PCHAR("validator1.countTheEntities"));
  countTheEntities.addParam(RpcString(s));

  MethodResponse resp = client.call(countTheEntities, ULXR_PCHAR("/RPC2"));

  if (!resp.isOK())
  {
    printResponse(countTheEntities, resp);
    return false;
  }

  Struct str = resp.getResult();

  if (leftangs != Integer(str.getMember(ULXR_PCHAR("ctLeftAngleBrackets"))).getInteger())
    return false;

  if (rightangs != Integer(str.getMember(ULXR_PCHAR("ctRightAngleBrackets"))).getInteger())
    return false;

  if (ampers != Integer(str.getMember(ULXR_PCHAR("ctAmpersands"))).getInteger())
    return false;

  if (apos != Integer(str.getMember(ULXR_PCHAR("ctApostrophes"))).getInteger())
    return false;

  if (quotes != Integer(str.getMember(ULXR_PCHAR("ctQuotes"))).getInteger())
    return false;

  return true;
}


bool check_easyStructTest(Requester &client)
{
  Array arr;
  Struct str;

  int my_sum = 0;

  int val = 1+(int) (10000.0*ulxr::getRand()/(RAND_MAX+1.0));
  my_sum += val;
  str.addMember(ULXR_PCHAR("curly"), Integer(val));

  val = 1+(int) (10000.0*ulxr::getRand()/(RAND_MAX+1.0));
  my_sum += val;
  str.addMember(ULXR_PCHAR("moe"),   Integer(val));

  val = 1+(int) (10000.0*ulxr::getRand()/(RAND_MAX+1.0));
  my_sum += val;
  str.addMember(ULXR_PCHAR("larry"), Integer(val));

  MethodCall easyStructTest(ULXR_PCHAR("validator1.easyStructTest"));
  easyStructTest.addParam(str);

  MethodResponse resp = client.call(easyStructTest, ULXR_PCHAR("/RPC2"));
  if (!resp.isOK())
  {
    printResponse(easyStructTest, resp);
    return false;
  }

  Integer sum = resp.getResult();

  return sum.getInteger() == my_sum;
}


bool scientific = false;


bool check_echoStructTest(Requester &client)
{
  Struct str;

  str.addMember(ULXR_PCHAR("curly"), Integer(12345));
  str.addMember(ULXR_PCHAR("moe"),   Boolean(true));
  str.addMember(ULXR_PCHAR("larry"), Double(3.14159));
  str.addMember(ULXR_PCHAR("huge"), Double(123E300));
  str.addMember(ULXR_PCHAR("minne"), RpcString("hallo string"));
  str.addMember(ULXR_PCHAR("mickey"), Base64(ULXR_PCHAR("asdfasdf8978978")));

  ULXR_COUT << ULXR_PCHAR("mickey ") << Base64(str.getMember(ULXR_PCHAR("mickey"))).getString() << std::endl;
  ULXR_COUT << ULXR_PCHAR("mickey ") << Base64(str.getMember(ULXR_PCHAR("mickey"))).getBase64() << std::endl;

  MethodCall echoStructTest(ULXR_PCHAR("validator1.echoStructTest"));
  echoStructTest.addParam(str);

  MethodResponse resp = client.call(echoStructTest, ULXR_PCHAR("/RPC2"));
  if (!resp.isOK())
  {
    printResponse(echoStructTest, resp);
    return false;
  }

  Struct ret = resp.getResult();

  scientific = !scientific;
  Double::setScientificMode(scientific);
//  ULXR_COUT << ULXR_PCHAR("return string ") << str.getXml() << std::endl;

  ULXR_COUT << ULXR_PCHAR("check sig ") << str.getSignature(true) << std::endl;
  if (str.getSignature(true) != ret.getSignature(true))
    return false;

  ULXR_COUT << ULXR_PCHAR("check int\n");
  if (12345 != Integer(str.getMember(ULXR_PCHAR("curly"))).getInteger())
    return false;

  ULXR_COUT << ULXR_PCHAR("check bool\n");
  if (true  !=  Boolean(str.getMember(ULXR_PCHAR("moe"))).getBoolean())
    return false;

  ULXR_COUT << ULXR_PCHAR("check double\n");
  if (ulxr_fabs(3.14159 - Double(str.getMember(ULXR_PCHAR("larry"))).getDouble()) > 0.0001)
    return false;

  ULXR_COUT << ULXR_PCHAR("check huge double\n");
  if (ulxr_fabs(123E300 - Double(str.getMember(ULXR_PCHAR("huge"))).getDouble()) > 0.0001)
    return false;

  ULXR_COUT << ULXR_PCHAR("check string\n");
  if ((CppString)ULXR_PCHAR("hallo string") != RpcString(str.getMember(ULXR_PCHAR("minne"))).getString())
    return false;

  ULXR_COUT << ULXR_PCHAR("check base64\n");
  if ((CppString) ULXR_PCHAR("asdfasdf8978978") != Base64(str.getMember(ULXR_PCHAR("mickey"))).getString())
    return false;

  return true;
}


bool check_manyTypesTest(Requester &client)
{
  Integer i4(12345);
  Boolean b(false);
  RpcString s("asdfasdf");
  Double d(2.789);
  DateTime dt(ULXR_PCHAR("20000412T123456"));
  Base64 b64(ULXR_PCHAR("sdgb7987987"));

  MethodCall manyTypesTest(ULXR_PCHAR("validator1.manyTypesTest"));
  manyTypesTest.addParam(i4)
               .addParam(b)
               .addParam(s)
               .addParam(d)
               .addParam(dt)
               .addParam(b64);

  MethodResponse resp = client.call(manyTypesTest, ULXR_PCHAR("/RPC2"));
  if (!resp.isOK())
  {
    printResponse(manyTypesTest, resp);
    return false;
  }

  Array arr = resp.getResult();

  if (12345 != Integer(arr.getItem(0)).getInteger() )
  {
    ULXR_COUT << ULXR_PCHAR("validator1.manyTypesTest integer failed.\n");
    return false;
  }

  if (false != Boolean(arr.getItem(1)).getBoolean() )
  {
    ULXR_COUT << ULXR_PCHAR("validator1.manyTypesTest integer failed.\n");
    return false;
  }


  if (ULXR_PCHAR("asdfasdf") != RpcString(arr.getItem(2)).getString())
  {
    ULXR_COUT << ULXR_PCHAR("validator1.manyTypesTest string failed.\n");
    return false;
  }


  if (ulxr_fabs(2.789 - Double(arr.getItem(3)).getDouble()) > 0.0001)
  {
    ULXR_COUT << ULXR_PCHAR("validator1.manyTypesTest double failed.\n");
    return false;
  }


  if (ULXR_PCHAR("20000412T123456") != DateTime(arr.getItem(4)).getDateTime())
  {
    ULXR_COUT << ULXR_PCHAR("validator1.manyTypesTest datetime failed.\n");
    return false;
  }


  if (ULXR_PCHAR("sdgb7987987") != Base64(arr.getItem(5)).getString())
  {
    ULXR_COUT << ULXR_PCHAR("validator1.manyTypesTest base64 failed: ")
              << Base64(arr.getItem(5)).getString() << std::endl;
    return false;
  }


  return true;
}


bool check_moderateSizeArrayCheck(Requester &client)
{
  MethodCall moderateSizeArrayCheck(ULXR_PCHAR("validator1.moderateSizeArrayCheck"));

  Array arr;
  CppString first, last;
  for (unsigned i = 0; i < 250; ++i)
  {
    unsigned len = 1+(int) (40.0*ulxr::getRand()/(RAND_MAX+1.0));
    CppString s;
    for (unsigned j = 0; j < len; ++j)
       s += (ulxr::Char) (0x41+j);
    if (i == 0)
      first = s;
    arr.addItem(RpcString(s));
    last = s;
  }
  moderateSizeArrayCheck.addParam(arr);
  CppString comp = first + last;

  MethodResponse resp = client.call(moderateSizeArrayCheck, ULXR_PCHAR("/RPC2"));
  if (!resp.isOK())
  {
    printResponse(moderateSizeArrayCheck, resp);
    return false;
  }

  RpcString str = resp.getResult();

  if (comp  != str.getString())
    return false;

  return true;
}


bool check_nestedStructTest(Requester &client)
{
  Struct str;

  Struct days;
  Struct days2;
  Struct months;

  Struct months2;
  Struct years;
  Struct data;
  Struct data2;

  int moe_val = 123;
  int larry_val = 815;
  int curly_val = 4711;
  int my_sum = moe_val + larry_val + curly_val;

  data.addMember(ULXR_PCHAR("moe"), Integer(moe_val));
  data.addMember(ULXR_PCHAR("larry"), Integer(larry_val));
  data.addMember(ULXR_PCHAR("curly"), Integer(curly_val));

  data2.addMember(ULXR_PCHAR("2moe2"), Integer(moe_val+4));
  data2.addMember(ULXR_PCHAR("2larry2"), Integer(larry_val+1234));
  data2.addMember(ULXR_PCHAR("2curly2"), Integer(curly_val-1234));

  days.addMember(ULXR_PCHAR("01"), data);  // needed
  days.addMember(ULXR_PCHAR("04"), data2);
  days.addMember(ULXR_PCHAR("15"), data);
  days.addMember(ULXR_PCHAR("23"), data2);

  days2.addMember(ULXR_PCHAR("02"), data2);
  days2.addMember(ULXR_PCHAR("05"), data);
  days2.addMember(ULXR_PCHAR("16"), data2);
  days2.addMember(ULXR_PCHAR("24"), data);

  months.addMember(ULXR_PCHAR("04"), days); // needed
  months.addMember(ULXR_PCHAR("14"), days2);
  months.addMember(ULXR_PCHAR("24"), days);
  months.addMember(ULXR_PCHAR("28"), days2);

  months2.addMember(ULXR_PCHAR("05"), days2);
  months2.addMember(ULXR_PCHAR("15"), days);
  months2.addMember(ULXR_PCHAR("16"), days);
  months2.addMember(ULXR_PCHAR("25"), days2);
  months2.addMember(ULXR_PCHAR("27"), days2);
  months2.addMember(ULXR_PCHAR("28"), days2);
  months2.addMember(ULXR_PCHAR("29"), days);

  years.addMember(ULXR_PCHAR("2000"), months); // needed
  years.addMember(ULXR_PCHAR("2005"), months2);
  years.addMember(ULXR_PCHAR("2011"), months);
  years.addMember(ULXR_PCHAR("2012"), months);
  years.addMember(ULXR_PCHAR("2013"), months2);
  years.addMember(ULXR_PCHAR("2014"), months2);
  years.addMember(ULXR_PCHAR("2015"), months);
  years.addMember(ULXR_PCHAR("2016"), months2);

  MethodCall nestedStructTest(ULXR_PCHAR("validator1.nestedStructTest"))        ;
  nestedStructTest.addParam(years);

  MethodResponse resp = client.call(nestedStructTest, ULXR_PCHAR("/RPC2"));
  if (!resp.isOK())
  {
    printResponse(nestedStructTest, resp);
    return false;
  }

  Integer ret_sum = resp.getResult();

  return ret_sum.getInteger() == my_sum;
}


bool check_simpleStructReturnTest(Requester &client)
{

  MethodCall simpleStructReturnTest(ULXR_PCHAR("validator1.simpleStructReturnTest"));
  int val = 31;
  simpleStructReturnTest.addParam(Integer(val));

  MethodResponse resp = client.call(simpleStructReturnTest, ULXR_PCHAR("/RPC2"));
  if (!resp.isOK())
  {
    printResponse(simpleStructReturnTest, resp);
    return false;
  }

  Struct ret_str = resp.getResult();
  Integer val_10 = ret_str.getMember(ULXR_PCHAR("times10"));
  Integer val_100 = ret_str.getMember(ULXR_PCHAR("times100"));
  Integer val_1000 = ret_str.getMember(ULXR_PCHAR("times1000"));

  if (val_10.getInteger() != val * 10)
    return false;

  if (val_100.getInteger() != val * 100)
    return false;

  if (val_1000.getInteger() != val * 1000)
    return false;

  return true;
}


int main(int argc, char **argv)
{
  unsigned bad = 0;
  try
  {
    ulxr::intializeLog4J(argv[0]);
    ulxr::getLogger4J()->send(ULXR_PCHAR("DEBUG"),
                              ULXR_PCHAR("val1_client started"),
                              ULXR_GET_STRING(__FILE__),
                              __LINE__);

    ulxr::CppString host = ULXR_PCHAR("localhost");
    if (argc > 1)
      host = ULXR_GET_STRING(argv[1]);

    unsigned port = 32002;
    if (argc > 2)
      port = ulxr_atoi(argv[2]);

#ifdef __BORLANDC__  // CodeGuard slows down
    unsigned last = 25;
#else
    unsigned last = 500;
#endif
    if (argc > 3)
       last = ulxr_atoi(argv[3]);

    bool wbxml = haveOption(argc, argv, "wbxml");
    bool secure = haveOption(argc, argv, "ssl");
    bool useproxy = haveOption(argc, argv, "proxy");
    bool useconnect = haveOption(argc, argv, "connect");
    bool chunked = haveOption(argc, argv, "chunked");
    bool persistent = haveOption(argc, argv, "persistent");

    ulxr::CppString sec = ULXR_PCHAR("unsecured");
    if (secure)
      sec = ULXR_PCHAR("secured");

    ULXR_COUT << ULXR_PCHAR("Requesting ") << last << ULXR_PCHAR(" ") << sec <<  ULXR_PCHAR(" rpc calls at ")
              << host << ULXR_PCHAR(":") << port << std::endl
              << ULXR_PCHAR("WBXML: ") << wbxml << std::endl
              << ULXR_PCHAR("  SSL: ") << secure << std::endl
              << ULXR_PCHAR("Proxy: ") << useproxy << std::endl
              << ULXR_PCHAR("Proxy-Connect: ") << useconnect << std::endl
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
    if (useproxy)
      conn->setProxy(ULXR_PCHAR("sonne"), 8080);

    HttpProtocol prot(conn.get());
    prot.setChunkedTransfer(chunked);

    if (useproxy)
      prot.setProxyAuthentication(ULXR_PCHAR("proxyuser"), ULXR_PCHAR("prx!usr"));

    if (useconnect)
      prot.enableConnect();

    prot.setPersistent(persistent);
    if (persistent)
      conn->setTcpNoDelay(true);

    Requester client(&prot, wbxml);

    if (prot.isPersistent())
      ULXR_COUT << ULXR_PCHAR("Using persistent connections\n") ;
    else
      ULXR_COUT << ULXR_PCHAR("Using non-persistent connections\n") ;

    MethodCall pretty_print (ULXR_PCHAR("ulxmlrpcpp.pretty_print"));
    pretty_print.addParam(ulxr::Boolean(true));

    MethodResponse resp = client.call(pretty_print, ULXR_PCHAR("/RPC2"));

    ulxr_time_t starttime = ulxr_time(0);
    unsigned good = 0;
    for (unsigned i = 0; i < last; ++i)
    {
      ULXR_COUT << ULXR_PCHAR("Run ") << i << std::endl;

      if (!check_moderateSizeArrayCheck(client))
      {
        ++bad;
        ULXR_COUT << ULXR_PCHAR("moderateSizeArrayCheck failed\n");
      }
      else
      {
        ++good;
        ULXR_COUT << ULXR_PCHAR("moderateSizeArrayCheck succeeded\n");

      }

      if (!check_nestedStructTest(client))
      {
        ++bad;
        ULXR_COUT << ULXR_PCHAR("nestedStructTest failed\n");
      }
      else
      {
        ++good;
        ULXR_COUT << ULXR_PCHAR("nestedStructTest succeeded\n");
      }

      if (!check_simpleStructReturnTest(client))
      {
        ++bad;
        ULXR_COUT << ULXR_PCHAR("simpleStructReturnTest failed\n");
      }
      else
      {
        ++good;
        ULXR_COUT << ULXR_PCHAR("simpleStructReturnTest succeeded\n");
      }

      if (!check_manyTypesTest(client))
      {
        ++bad;
        ULXR_COUT << ULXR_PCHAR("manyTypesTest failed\n");
      }
      else
      {
        ++good;
        ULXR_COUT << ULXR_PCHAR("manyTypesTest succeeded\n");
      }

      if (!check_echoStructTest(client))
      {
        ++bad;
        ULXR_COUT << ULXR_PCHAR("echoStructTest failed\n");
      }
      else
      {
        ++good;
        ULXR_COUT << ULXR_PCHAR("echoStructTest succeeded\n");
      }

      if (!check_easyStructTest(client))
      {
        ++bad;
        ULXR_COUT << ULXR_PCHAR("easyStructTest failed\n");

      }
      else
      {
        ++good;
        ULXR_COUT << ULXR_PCHAR("easyStructTest succeeded\n");
      }

      if (!check_countTheEntities(client))
      {
        ++bad;
        ULXR_COUT << ULXR_PCHAR("countTheEntities failed\n");
      }
      else
      {
        ++good;
        ULXR_COUT << ULXR_PCHAR("countTheEntities succeeded\n");
      }

      if (!check_arrayOfStructs(client))
      {
        ++bad;
        ULXR_COUT << ULXR_PCHAR("arrayOfStructs failed\n");
      }

      else
      {
        ++good;
        ULXR_COUT << ULXR_PCHAR("arrayOfStructs succeeded\n");
      }

    }

    if (prot.isPersistent())
      ULXR_COUT << ULXR_PCHAR("Used persistent connections\n") ;
    else
      ULXR_COUT << ULXR_PCHAR("Used non-persistent connections\n") ;

    ULXR_COUT << ULXR_PCHAR("Shutting server down.\n");
    ulxr::MethodCall shutdown (ULXR_PCHAR("shutdown"));
    resp = client.call(shutdown, ULXR_PCHAR("/RPC2"));
    if (!resp.isOK())
    {
      printResponse(shutdown, resp);
      return false;
    }

    ulxr_time_t endtime = ulxr_time(0);
    ulxr_time_t  mins = (endtime - starttime) / 60;
    ulxr_time_t  secs = (endtime - starttime) % 60;

    ULXR_COUT << ULXR_PCHAR("Ready.\n");
    ULXR_COUT << ULXR_PCHAR("Good runs  : ") << good << std::endl;
    ULXR_COUT << ULXR_PCHAR("Bad runs   : ") << bad << std::endl;
    ULXR_COUT << ULXR_PCHAR("Time needed: ") << mins << ULXR_PCHAR(":")
              << secs << std::endl;
  }

  catch(Exception &ex)
  {
     ULXR_COUT << ULXR_PCHAR("Error occured: ")
               << ULXR_GET_STRING(ex.why()) << std::endl;
  }

  catch(...)
  {
     ULXR_COUT << ULXR_PCHAR("unknown Error occured.\n");
  }

  ULXR_COUT << ULXR_PCHAR("Terminating.\n");

  return bad == 0;
}
