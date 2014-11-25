/***************************************************************************
        GenClientMethod.cpp  -  generate class with client methods
                             -------------------
    begin                : Sun May 28 2007
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: GenClientMethod.cpp 1016 2007-07-22 15:03:44Z ewald-arnold $

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

#include "GenClientMethod.h"

#include <fstream>
#include <iostream>

#include <sys/stat.h>


GenerateClientMethods::GenerateClientMethods(const UlxrIdlClass &in_class)
  : theClass(in_class)
{
  theClass.resolveOverloaded();
}


void GenerateClientMethods::generate_H(const std::string &destdir, const std::string &name)
{
  const std::string h_name = destdir + theClass.getBaseName() + "_ulxr_client.h";
  std::ofstream h_file(h_name.c_str());
  std::cout << "Header file will be created: " << h_name << std::endl;

  generateHeaderHead(h_file, name + "UlxrClient");
  generateHeaderClassHead(h_file, name);
  generateHeaderMethods(h_file);
  generateHeaderVariables(h_file);
  generateHeaderTail(h_file, name + "UlxrClient");
}


void GenerateClientMethods::generate_CPP(const std::string &destdir, const std::string &name)
{
  const std::string h_name = theClass.getBaseName() + "_ulxr_client.h";
  const std::string cpp_name = destdir + theClass.getBaseName() + "_ulxr_client.cpp";
  std::ofstream cpp_file(cpp_name.c_str());
  std::cout << "Source file will be created: " << cpp_name << std::endl;

  generateSourceHead(cpp_file, h_name);
  generateSourceCtors(cpp_file, name);
  generateSourceMethods(cpp_file);
}


void GenerateClientMethods::generate_CPP_USER(const std::string &destdir, const std::string &name)
{
  const std::string h_name = theClass.getBaseName() + "_ulxr_client.h";
  std::string cpp_name = destdir + theClass.getBaseName() + "_ulxr_client_user.cpp";

  struct stat statbuf;
  if (stat(cpp_name.c_str(), &statbuf) >= 0)
  {
    std::cout << "User file already exists: " << cpp_name << std::endl;
    cpp_name += ".new";
    std::cout << "New template will be created: " << cpp_name << std::endl;
  }

  std::ofstream cpp_file(cpp_name.c_str());
  std::cout << "User file will be created: " << cpp_name << std::endl;

  generateUserSourceHead(cpp_file, h_name);

  cpp_file << "#include <ulxmlrpcpp/ulxr_response.h>\n";
  cpp_file << "#include <ulxmlrpcpp/ulxr_requester.h>\n";
  cpp_file << "#include <ulxmlrpcpp/ulxr_except.h>\n\n";

  cpp_file << "#include \"" << name + "_ulxr_names.h" << "\"\n\n";

  cpp_file << name << "Client::" << name << "Client\n"
           << "  (ulxr::Requester &in_requester, const ulxr::CppString &realm,\n"
              "   const ulxr::CppString &user, const ulxr::CppString &pass)\n"
              "   : requester(in_requester)\n"
              "   , rpc_realm(realm)\n"
              "   , rpc_user(user)\n"
              "   , rpc_pass(pass)\n";

  for (unsigned i = 0; i < theClass.numMethods(); ++i)
  {
    cpp_file << "   , ";

    Method method = theClass.getMethod(i);
    method.extractNamespace();

    cpp_file << "callTo_" << method.getOverloadName()
             << "(ULXR_CALLTO_" << method.getOverloadName(true, "", "_")
             << ")  // mapped to: " << method.getCppString(0, true, "");

    if (method.getName() != method.getOverloadName())
       cpp_file << " (there are overloaded methods)";

    cpp_file << "\n";
  }

  cpp_file << "{\n"
              "}\n\n\n";

  cpp_file << "ulxr::MethodResponse\n  "
           << name << "Client::" << "sendCall(const ulxr::MethodCall &calldata) const\n"
           << "{\n"
           << "  ulxr::MethodResponse resp = requester.call(calldata, rpc_realm, rpc_user, rpc_pass);\n"
           << "\n"
           << "  if (!resp.isOK())\n"
           << "  {\n"
           << "    ulxr::Struct st = resp.getResult();\n"
           << "    int ec = ulxr::Integer(st.getMember(ULXR_PCHAR(\"faultCode\"))).getInteger();\n"
           << "    ulxr::CppString es = ulxr::RpcString(st.getMember(ULXR_PCHAR(\"faultString\"))).getString();\n"
           << "    throw ulxr::RuntimeException(ec, es);\n"
           << "  }\n"
           << "\n"
           << "  return resp;\n"
           << "}\n\n\n";

  cpp_file<< name << "Client::~" << name << "Client()\n"
           << "{\n"
           << "}\n\n\n";
}


void GenerateClientMethods::generate(const std::string &destdir, const std::string &name)
{
  generate_NameDefines(destdir, theClass.getBaseName() , theClass.getAllMethods());
  generate_H(destdir, name);
  generate_CPP(destdir, name);
  generate_CPP_USER(destdir, name);
}


void GenerateClientMethods::generateHeaderMethods(std::ostream & h_file)
{
  for (unsigned i = 0; i < theClass.numMethods(); ++i)
  {
    Method method = theClass.getMethod(i);
    method.extractNamespace();

    h_file << "    // mapped to " << method.getOverloadName();
    if (method.getName() != method.getOverloadName())
      h_file << " (there are overloaded methods)";

    h_file << "\n    " << method.getCppString(0, false, "") << "\n\n";
  }

  h_file << "    virtual ~" << theClass.getBaseName() << "Client();\n\n";

  h_file << "    virtual ulxr::MethodResponse sendCall(const ulxr::MethodCall &calldata) const;\n\n";
}


void GenerateClientMethods::generateSourceCtors(std::ostream & cpp_file,
                                                const std::string &name)
{
  cpp_file << "#include <ulxmlrpcpp/ulxr_requester.h>\n\n";

//  cpp_file << "#include \"" << theClass.getSource() << "\"\n\n\n";
}


void GenerateClientMethods::generateSourceMethods(std::ostream & cpp_file)
{
  for (unsigned i = 0; i < theClass.numMethods(); ++i)
  {
    Method method = theClass.getMethod(i);
    method.extractNamespace();

    cpp_file << "// mapped to " << method.getOverloadName(true);
    if (method.getName() != method.getOverloadName())
      cpp_file << " (there are overloaded methods)";

    cpp_file << "\n"
             << method.getCppString(0, true, "Client") << "\n"
             << "{\n";

    cpp_file << "  callTo_" << method.getOverloadName() << ".clear();\n";

    for (unsigned iarg = 0; iarg < method.numArgs(); ++iarg)
    {
      std::string adap = method.getArg(iarg).getType().getInversTypeAdapter();
      std::string adap2;
      if (adap.length() != 0)
      {
        adap += "(";
        adap2 = ")";
      }

      cpp_file << "  callTo_" << method.getOverloadName() << ".addParam("
               << method.getArg(iarg).getType().getRpcName() << "(" << adap
               << method.getArg(iarg).getType().getTypeDereference() << method.getArg(iarg).getName()
               << adap2
               << "));\n";

    }

    bool have_retval = false;
    if (method.getType().getName() != "void" || method.getType().getLeft() != "" || method.getType().getRight() != "")
      have_retval = true;

    if (have_retval)
      cpp_file << "  ulxr::MethodResponse resp = sendCall(callTo_" << method.getOverloadName() << ");\n";
    else
      cpp_file << "  sendCall(callTo_" << method.getOverloadName() << ");\n";

    if (have_retval)
    {
      std::string adap = method.getType().getTypeAdapter();
      std::string adap2;
      if (adap.length() != 0)
      {
        adap += "(";
        adap2 = ")";
      }

      const bool reftype = method.getType().isReference();
      const bool ptrtype = method.getType().isPointer();
      if (reftype || ptrtype)
        cpp_file << "  ulxr_refFor_"
                << method.getOverloadName() << " = ";
      else
        cpp_file  << "  return ";

      cpp_file << "(" << method.getType().getProxyType() << ") "
               << adap + method.getType().getRpcName() << "(resp.getResult())." << method.getType().getRpcAccessor()
               << "()" << adap2 << ";\n";

      if (reftype)
        cpp_file << "  return ulxr_refFor_"
                << method.getOverloadName() << ";\n";

      else if (ptrtype)
      {
        std::string acc = "&";
        std::string acc2 = method.getType().getTypeAccessor();
        if (acc2.length() != 0)
         acc = "";

        cpp_file << "  return "
                 << "(" << method.getType().getCppString() << ") "
                 << acc << "ulxr_refFor_"
                 << method.getOverloadName() << acc2 << ";\n";
      }
    }

    cpp_file << "}\n\n\n";
  }
}


void GenerateClientMethods::generateHeaderClassHead(std::ostream & h_file,
                                                    const std::string &name)
{
  h_file<< "#include <ulxmlrpcpp/ulxr_call.h>\n\n"
         << "namespace ulxr\n"
         << "{\n"
         << "  class Requester;\n"
         << "  class MethodResponse;\n"
         << "  class MethodCall;\n"
         << "}\n\n"
         << "class " << name << "Client\n"
         << "{\n"
         << "  public:\n\n"
         << "    " << name << "Client\n"
         << "      (ulxr::Requester &requester, const ulxr::CppString &realm = ULXR_PCHAR(\"/RPC2\"),\n"
         << "       const ulxr::CppString &user = ULXR_PCHAR(\"\"), const ulxr::CppString &pass = ULXR_PCHAR(\"\"));\n\n";
}


void GenerateClientMethods::generateHeaderVariables(std::ostream & h_file)
{
  h_file << "  private:\n\n";

  h_file<< "    ulxr::Requester &requester;\n";
  h_file<< "    ulxr::CppString rpc_realm;\n";
  h_file<< "    ulxr::CppString rpc_user;\n";
  h_file<< "    ulxr::CppString rpc_pass;\n\n";

  for (unsigned i = 0; i < theClass.numMethods(); ++i)
  {
    Method method = theClass.getMethod(i);
    method.extractNamespace();

    const bool reftype = method.getType().isReference();
    const bool ptrtype = method.getType().isPointer();
    if (reftype || ptrtype)
      h_file << "    mutable " << method.getType().getProxyType() << " ulxr_refFor_" << method.getOverloadName() << ";\n";
  }

  for (unsigned i = 0; i < theClass.numMethods(); ++i)
  {
    if (i == 0)
      h_file << "\n";

    Method method = theClass.getMethod(i);
    method.extractNamespace();

    h_file << "    mutable ulxr::MethodCall callTo_" << method.getOverloadName()
             << ";  // mapped to: " << method.getCppString(0, true, "")
             << "\n";
  }
}


