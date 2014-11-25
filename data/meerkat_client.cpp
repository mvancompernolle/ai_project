/***************************************************************************
          meerkat_client.cpp  --  Requester for Meerkat service

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

   For an explanation of Meerkat services see:
    - http://www.oreillynet.com/pub/a/rss/2000/11/14/meerkat_xmlrpc.html
    - http://www.oreillynet.com/pub/a/rss/2000/05/09/meerkat_api.html

   For explanation about MySql expressions see
    http://www.mysql.com/documentation/mysql/bychapter/manual_Regexp.html

    $Id: meerkat_client.cpp 1151 2009-08-12 15:12:01Z ewald-arnold $

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

#include <ulxmlrpcpp/ulxr_tcpip_connection.h> // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_except.h>

#include <cstdlib>
#include <iomanip>
#include <iostream>

using namespace ulxr;


void usage(int ret)
{
 ULXR_COUT
      << ULXR_PCHAR("Usage: meerkat_client  [options]\n")
         ULXR_PCHAR(" --channels, -c     lists all available channels.\n")
         ULXR_PCHAR(" --categories, -e   lists all available categories.\n")
         ULXR_PCHAR(" --search, -s expr  searches expr in database.\n")
         ULXR_PCHAR("     where \'expr\' is either a string or an SQL regular expression.\n")
         ULXR_PCHAR("\n");

  exit(ret);
}


int handleFault(MethodResponse &resp)
{
   Struct str = resp.getResult();
   if (str.hasMember(ULXR_PCHAR("faultCode")))
   {
     ULXR_COUT << ULXR_PCHAR("Server reports error: ") << RpcString(str.getMember(ULXR_PCHAR("faultString"))).getString()
               << ULXR_PCHAR(" (") << Integer(str.getMember(ULXR_PCHAR("faultCode"))).getInteger() << ULXR_PCHAR(")\n");
     return 1;
   }
   else
   {
     ULXR_COUT << ULXR_PCHAR("Unexpected return structure: \n")
               << str.getXml(0)
               << std::endl;
     return 1;
   }
   /* return 0; */
}


int showCategories(Requester &client)
{
  MethodCall get_Categories (ULXR_PCHAR("meerkat.getCategories"));
  MethodResponse resp = client.call(get_Categories, ULXR_PCHAR("/meerkat/xml-rpc/server.php"));

  if (!resp.isOK())
    return handleFault(resp);

  Array list = resp.getResult();

  ULXR_COUT << ULXR_PCHAR("Found ") << list.size() << ULXR_PCHAR(" categories:\n\n");
  for (unsigned i = 0; i < list.size(); ++i)
  {
    Struct entry = list.getItem(i);
    Integer id = entry.getMember(ULXR_PCHAR("id"));
    RpcString title = entry.getMember(ULXR_PCHAR("title"));
    ULXR_COUT << ULXR_PCHAR(" ") << std::setw(5) << id.getInteger() << ULXR_PCHAR("  ") << title.getString() << std::endl;
  }

  return 0;
}


int showChannels(Requester &client)
{
  MethodCall get_Channels (ULXR_PCHAR("meerkat.getChannels"));
  MethodResponse resp = client.call(get_Channels, ULXR_PCHAR("/meerkat/xml-rpc/server.php"));

  if (!resp.isOK())
    return handleFault(resp);

  Array list = resp.getResult();

  ULXR_COUT << ULXR_PCHAR("Found ") << list.size() << ULXR_PCHAR(" channels:\n\n");
  for (unsigned i = 0; i < list.size(); ++i)
  {
    Struct entry = list.getItem(i);
    Integer id = entry.getMember(ULXR_PCHAR("id"));
   RpcString title = entry.getMember(ULXR_PCHAR("title"));
    ULXR_COUT << ULXR_PCHAR(" ") << std::setw(5)
              << id.getInteger() << ULXR_PCHAR("  ")
              << title.getString() << std::endl;
  }

  return 0;
}


int searchdb(Requester &client, const CppString &patt)
{
  CppString pattern = patt;

  if (pattern[0] != ULXR_CHAR('/'))
    pattern = ULXR_CHAR('/')+ pattern + ULXR_CHAR('/';)

  Struct query;
  query.addMember(ULXR_PCHAR("search"), RpcString(pattern));
  query.addMember(ULXR_PCHAR("time_period"), RpcString(ULXR_PCHAR("48HOUR")));
  query.addMember(ULXR_PCHAR("descriptions"), Integer(76));

  MethodCall get_Items (ULXR_PCHAR("meerkat.getItems"));
  get_Items.addParam(query);
  MethodResponse resp = client.call(get_Items, ULXR_PCHAR("/meerkat/xml-rpc/server.php"));

  if (!resp.isOK())
    return handleFault(resp);

  Value val = resp.getResult();

  if (!val.isArray())
  {

    ULXR_COUT << ULXR_PCHAR("No items found for your query string.\n");
    return 0;
  }

  Array list = val;

  ULXR_COUT << ULXR_PCHAR("Found ") << list.size() << ULXR_PCHAR(" items:\n\n");
  for (unsigned i = 0; i < list.size(); ++i)
  {
    Struct entry = list.getItem(i);
    RpcString title = entry.getMember(ULXR_PCHAR("title"));
    RpcString link = entry.getMember(ULXR_PCHAR("link"));
    RpcString description = entry.getMember(ULXR_PCHAR("description"));

    ULXR_COUT << ULXR_PCHAR(" Title: ") << title.getString() << std::endl
              << ULXR_PCHAR(" Link: ") << link.getString() << std::endl;

    CppString s = description.getString();
    if (s.length() != 0)
      ULXR_COUT << ULXR_PCHAR(" Description: ") <<  s << ULXR_PCHAR("\n");

    ULXR_COUT << ULXR_PCHAR("\n");
  }
  ULXR_COUT << ULXR_PCHAR("\n");
  return 0;
}


int main(int argc, char **argv)
{
  try {
    TcpIpConnection conn (false, ULXR_PCHAR("www.oreillynet.com"), 80);
    HttpProtocol prot(&conn);
    Requester client(&prot);
    MethodResponse resp;

    if (argc < 2)
      usage(1);

    CppString cmd = ULXR_GET_STRING(argv[1]);
    if (argc == 2)
    {
      if (cmd == ULXR_PCHAR("-c") || cmd == ULXR_PCHAR("--channels"))
        return showChannels(client);

      else if (cmd == ULXR_PCHAR("-e") || cmd == ULXR_PCHAR("--categories"))
        return showChannels(client);

      else if (cmd == ULXR_PCHAR("--help"))
        usage(0);

      else
        usage(1);
    }

    else if (argc == 3)
    {
      if (cmd == ULXR_PCHAR("-s") || cmd == ULXR_PCHAR("--search"))
        return searchdb(client, ULXR_GET_STRING(argv[2]));

      else
        usage(1);
    }

    else
      usage(1);

  }
  catch(Exception &ex)
  {
     ULXR_COUT << ULXR_PCHAR("Error occured: ") << ex.why() << std::endl;
     return 1;
  }
  catch(std::exception &ex)
  {
     ULXR_COUT << ULXR_PCHAR("Error occured: ") << ULXR_GET_STRING(ex.what()) << std::endl;
     return 1;
  }
  catch(...)
  {
     ULXR_COUT << ULXR_PCHAR("unknown Error occured.\n");
     return 1;
  }

  return 0;
}

/*
    MethodCall get_ChannelsByCategory ("meerkat.getChannelsByCategory");
    get_ChannelsByCategory.addParam(Integer(10));

    MethodCall get_CategoriesBySubstring ("meerkat.getCategoriesBySubstring");
    get_CategoriesBySubstring.addParam(String ("linux"));

    MethodCall get_ChannelsBySubstring ("meerkat.getChannelsBySubstring");
    get_ChannelsBySubstring.addParam(String ("linux"));

    resp = client.call(get_ChannelsBySubstring, "/meerkat/xml-rpc/server.php");
    ULXR_COUT << "call result: \n";
    ULXR_COUT << resp.getXml(0);

    resp = client.call(get_CategoriesBySubstring, "/meerkat/xml-rpc/server.php");
    ULXR_COUT << "call result: \n";
    ULXR_COUT << resp.getXml(0);

    resp = client.call(get_ChannelsByCategory, "/meerkat/xml-rpc/server.php");
    ULXR_COUT << "call result: \n";
    ULXR_COUT << resp.getXml(0);

    resp = client.call(list_methods, "/meerkat/xml-rpc/server.php");
    ULXR_COUT << "call result: \n";
    ULXR_COUT << resp.getXml(0);
*/
