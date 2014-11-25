/***************************************************************************
       introspect.cpp  --  lists information about available methods

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: introspect.cpp 1151 2009-08-12 15:12:01Z ewald-arnold $

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
#include <cstdio>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h> // first, don't move: important msvc #include bug
#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_log4j.h>

using namespace ulxr;

#include <iostream>

bool splitUrl(const std::string &url,
              ulxr::CppString &server,
              ulxr::CppString &resource,
              unsigned &port)
{
  server = ULXR_GET_STRING(url);
  std::size_t pos;

  if ((pos = server.find('/') ) != ulxr::CppString::npos)
  {
      resource = server.substr(pos);
      server.erase(pos);
  }

  if ((pos = server.find(':')) != ulxr::CppString::npos)
  {
      ulxr::CppString ps = server.substr(pos+1);
      char *endp;
      std::string s = getLatin1(ps);
      port = strtol(s.c_str(), &endp, 10);
      if (endp != 0 && *endp != '\0')
      {
        ULXR_CERR << ULXR_PCHAR( "Error in port number: ")
                  << ULXR_GET_STRING(endp) << ULXR_PCHAR("\n\n");
        return false;
      }
      server.erase(pos);
  }

  return true;
}


void usage()
{
  ULXR_CERR << ULXR_PCHAR("usage:\n")
            << ULXR_PCHAR(" --host=server:port/resource \n")
            << ULXR_PCHAR("   [--use-proxy=host:port]\n")
            << ULXR_PCHAR("   [--cpp-decl=filename]\n")
            << ULXR_PCHAR("   [--cpp-impl=filename]\n")
            << ULXR_PCHAR(" example: introspect --host=myhost.com:80/rpc/server.php --use-proxy=host:8080\n")
            << ULXR_PCHAR("\n");
}


int main(int argc, char **argv)
{
  try
  {
    ulxr::intializeLog4J(argv[0]);
    ulxr::getLogger4J()->send(ULXR_PCHAR("DEBUG"),
                              ULXR_PCHAR("introspect started"),
                              ULXR_GET_STRING(__FILE__),
                              __LINE__);
    if (argc < 2)
    {
      usage();
      return 1;
    }

    unsigned proxyport = 0;
    ulxr::CppString proxyname;

    unsigned port = 80;
    ulxr::CppString resource = ULXR_PCHAR("/");
    ulxr::CppString server;
    bool have_host = false;

    std::string declfile;
    std::string implfile;

    for (int i = 1; i < argc; ++i)
    {
      std::string arg = argv[i];
      if (arg.substr(0, 12) == "--use-proxy=")
      {
        proxyport = 8080;
        ulxr::CppString rsc;
        if (!splitUrl(arg.substr(12), proxyname, rsc, proxyport))
        {
          usage();
          return 1;
        }
      }

      else if (arg.substr(0, 7) == "--host=")
      {
        have_host = true;
        if (!splitUrl(arg.substr(7), server, resource, port))
        {
          usage();
          return 1;
        }
      }

      else if (arg.substr(0, 11) == "--cpp-decl=")
        declfile = arg.substr(11);

      else if (arg.substr(0, 11) == "--cpp-impl=")
        implfile = arg.substr(11);

      else
      {
        std::cerr << "Unknown option: " << arg << std::endl;
        {
          usage();
          return 1;
        }
      }
    }

    if (!have_host)
    {
      std::cerr << "No target host given\n";
      usage();
      return 1;
    }

    ULXR_COUT << ULXR_PCHAR("Connecting to ")
              << server << ULXR_PCHAR(":") << port << resource
              << ULXR_PCHAR("\n");

    Integer i(1);
    TcpIpConnection conn (false, server, port);

    if (proxyport != 0 && proxyname.length() != 0)
    {
      ULXR_COUT << ULXR_PCHAR("using http proxy ")
                << proxyname << ULXR_PCHAR(":") << proxyport
                << ULXR_PCHAR("\n");
      conn.setProxy(proxyname, proxyport);
    }
    HttpProtocol prot(&conn);
    Requester client(&prot);
    MethodResponse resp;

    MethodCall list_methods (ULXR_PCHAR("system.listMethods"));
    MethodCall method_help (ULXR_PCHAR("system.methodHelp"));
    MethodCall method_sig (ULXR_PCHAR("system.methodSignature"));
    MethodCall get_capabilities (ULXR_PCHAR("system.getCapabilities"));

    resp = client.call(list_methods, resource);
    Array meth = resp.getResult();

    bool have_capabilities = false;
    ULXR_COUT << ULXR_PCHAR("Found ") << meth.size() <<  ULXR_PCHAR(" methods:\n\n");

    std::FILE *decl_fs = 0;
    if (declfile.length() != 0)
      decl_fs = std::fopen(declfile.c_str(), "w");

    std::FILE *impl_fs = 0;
    if (implfile.length() != 0)
      impl_fs = std::fopen(implfile.c_str(), "w");

    for (unsigned m = 0; m < meth.size(); ++m)
    {
      RpcString name = meth.getItem(m);
      if (name.getString() == get_capabilities.getMethodName() )
        have_capabilities = true;

      ULXR_COUT << m+1 << ULXR_PCHAR(") ") << name.getString() << std::endl;
      method_sig.clear();
      method_sig.addParam(name);
      resp = client.call(method_sig, resource);
      ulxr::CppString signature;
      ulxr::CppString purpose;
      if (resp.getResult().getType() == RpcArray)
      {
        Array meths = resp.getResult();
        for (unsigned ms = 0; ms < meths.size(); ++ms)
        {
          Array sign = meths.getItem(ms);
          signature.clear();
          for (unsigned is = 0; is < sign.size(); ++is)
          {
             if (signature.length() != 0)
               signature += ULXR_PCHAR(", ");
             signature +=RpcString(sign.getItem(is)).getString();
          }
          ULXR_COUT << ULXR_PCHAR(" Signature: ") << signature << ULXR_PCHAR("\n");
        }
        ULXR_COUT << ULXR_PCHAR("\n");
      }
      else
      {
        signature = ULXR_PCHAR("not available");
        ULXR_COUT << ULXR_PCHAR(" Signature: none available\n\n");
      }

      method_help.clear();
      method_help.addParam(name);
      resp = client.call(method_help, resource);
      purpose = RpcString(resp.getResult()).getString();
      ulxr::CppString cpurpose = purpose;

      unsigned pos = 0;
      bool multi = false;
      while ((pos = cpurpose.find(L'\n', pos)) != ulxr::CppString::npos)
      {
        multi = true;
         cpurpose.insert(pos+1, ULXR_PCHAR("  // "));
         pos += 1;   // 2 spaces indentation
      }
      if (multi)
        cpurpose.insert(0, ULXR_PCHAR("\n  // "));

      pos = 0;
      while ((pos = purpose.find('\n', pos)) != ulxr::CppString::npos)
      {
         purpose.insert (pos+1, ULXR_PCHAR("  "));
         pos += 3;   // 2 spaces indentation
      }

      ULXR_COUT << ULXR_PCHAR(" Documentation: \n")
               << ULXR_PCHAR("  ") << purpose << ULXR_PCHAR("\n\n");

      std::string s = ulxr::getLatin1(name.getString());
      std::string cname = s;
      while((pos = cname.find(".")) != std::string::npos)
        cname[pos] = '_';

      if (decl_fs != 0)
      {
        if (m == 0)
          std::fprintf(decl_fs, "\n///////////////////////////////////////////////////////\n"
                                "// XMLRPC Methods imported from %s\n\n", ulxr::getLatin1(server+resource).c_str());

        std::fprintf(decl_fs, "  // signature: %s\n", ulxr::getLatin1(signature).c_str() );
        std::fprintf(decl_fs, "  // purpose:   %s\n", ulxr::getLatin1(cpurpose).c_str() );
        std::fprintf(decl_fs, "  ulxr::MethodCall %s;\n\n", cname.c_str());
      }

      if (impl_fs != 0)
        std::fprintf(impl_fs, "  , %s (ULXR_PCHAR(\"%s\"))\n", cname.c_str(), s.c_str() );
    }

    if (decl_fs != 0)
      std::fprintf(decl_fs, "///////////////////////////////////////////////////////\n\n");

    if (impl_fs != 0)
      std::fclose(impl_fs);

    if (decl_fs != 0)
      std::fclose(decl_fs);

    ULXR_COUT << ULXR_PCHAR("Checking for well known system capabilities:\n");
    if (have_capabilities)
    {
      resp = client.call(get_capabilities, resource);
      Struct cap = resp.getResult();
      if (cap.hasMember(ULXR_PCHAR("faults_interop")))
      {
        Struct inter = cap.getMember(ULXR_PCHAR("faults_interop"));
        RpcString url = inter.getMember(ULXR_PCHAR("specUrl"));
        Integer vers = inter.getMember(ULXR_PCHAR("specVersion"));

       ULXR_COUT << ULXR_PCHAR(" Server supports interoperational fault codes as defined at:\n ")
                 << url.getString() << ULXR_PCHAR(" with version ")
                 << vers.getInteger() << std::endl;
      }
    }
    else
     ULXR_COUT << ULXR_PCHAR(" None avaliable.\n");
  }

  catch(XmlException &ex)
  {
    ULXR_COUT << ULXR_PCHAR("Xml Error occured: ") << ex.why()
              << ULXR_PCHAR(" (") << ex.getErrorString() << ULXR_PCHAR(") ")
              << ULXR_PCHAR(" in line ") << ex.getErrorLine() << std::endl;
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
