/***************************************************************************
        GenServerMethod.cpp  -  generate class with server methods
                             -------------------
    begin                : Sun May 28 2007
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : Ulxr@ewald-arnold.de

    $Id: GenServerMethod.cpp 1016 2007-07-22 15:03:44Z ewald-arnold $

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

#include "GenServerMethod.h"

#include <fstream>
#include <iostream>

#include <sys/stat.h>


GenerateServerMethods::GenerateServerMethods(const UlxrIdlClass &in_class)
  : theClass(in_class)
{
  theClass.resolveOverloaded();
}


void GenerateServerMethods::generate_H(const std::string &destdir, const std::string &name)
{
  const std::string h_name = destdir + theClass.getBaseName() + "_ulxr_server.h";
  std::ofstream h_file(h_name.c_str());
  std::cout << "Header file will be created: " << h_name << std::endl;

  generateHeaderHead(h_file, name + "UlxrServer");
  generateHeaderClassHead(h_file, name);
  generateHeaderMethods(h_file);
  generateHeaderVariables(h_file);
  generateHeaderTail(h_file, name + "UlxrServer");
}


void GenerateServerMethods::generate_CPP(const std::string &destdir, const std::string &name)
{
  const std::string h_name = theClass.getBaseName() + "_ulxr_server.h";
  const std::string cpp_name = destdir + theClass.getBaseName() + "_ulxr_server.cpp";
  std::ofstream cpp_file(cpp_name.c_str());
  std::cout << "Source file will be created: " << cpp_name << std::endl;

  generateSourceHead(cpp_file, h_name);
  generateSourceCtors(cpp_file, name);
  generateSourceMethods(cpp_file);
}


void GenerateServerMethods::generate_CPP_USER(const std::string &destdir, const std::string &name)
{
  const std::string h_name = theClass.getBaseName() + "_ulxr_server.h";
  std::string cpp_name = destdir + theClass.getBaseName() + "_ulxr_server_user.cpp";

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
  cpp_file << "#include <ulxmlrpcpp/ulxr_method_adder.h>\n";
  cpp_file << "#include <ulxmlrpcpp/ulxr_signature.h>\n\n";

  cpp_file << "#include \"" << theClass.getSource() << "\"\n";
  cpp_file << "#include \"" << name + "_ulxr_names.h" << "\"\n\n";

  cpp_file <<
    "\nvoid " << name << "Server::setupServerMethods()\n"
    "{\n";

  for (unsigned i = 0; i < theClass.numMethods(); ++i)
  {
    if (i != 0)
      cpp_file << "\n";

    Method method = theClass.getMethod(i);
    method.extractNamespace();

    cpp_file << "  // mapped to: " << method.getCppString(0, false, "");

    if (method.getName() != method.getOverloadName())
       cpp_file << "   (there are overloaded methods)";

    cpp_file <<
      "\n"
      "  method_adder.addMethod(ulxr::make_method(*this, &" << method.getOverloadName(true, "Server") << "),\n"
      "                         " << method.getType().getRpcName() << "::getValueName(),\n"
      "                         ULXR_CALLTO_" << method.getOverloadName(true, "", "_") << ",\n"
      "                         ulxr::Signature()";

    for (unsigned p = 0; p < method.numArgs(); ++p)
      cpp_file << "\n                           << " << method.getArg(p).getType().getRpcName() << "::getValueName()";

    cpp_file <<
      ",\n"
      "                         ulxr_i18n(ULXR_PCHAR(\"Some descriptive comment about '" << method.getCppString(0, true, "") << "'.\"))); // TODO adjust comment\n";
  }

  cpp_file <<
    "}\n\n";
}


void GenerateServerMethods::generate(const std::string &destdir, const std::string &name)
{
  generate_NameDefines(destdir, theClass.getBaseName(), theClass.getAllMethods());
  generate_H(destdir, name);
  generate_CPP(destdir, name);
  generate_CPP_USER(destdir, name);
}


void GenerateServerMethods::generateHeaderMethods(std::ostream & h_file)
{
  for (unsigned i = 0; i < theClass.numMethods(); ++i)
  {
    Method method = theClass.getMethod(i);
    method.extractNamespace();

    h_file << "    // mapped to: " << method.getCppString(0, false, "");

    if (method.getName() != method.getOverloadName())
       h_file << "   (there are overloaded methods)";

    h_file << "\n"
           << "    ulxr::MethodResponse " << method.getOverloadName() << " (const ulxr::MethodCall &calldata);\n\n";
  }

  h_file << " private:\n\n";
  h_file << "    void setupServerMethods();\n";
  h_file << "    void removeServerMethods();\n\n";
}


void GenerateServerMethods::generateSourceCtors(std::ostream & cpp_file,
                                                const std::string &name)
{
  cpp_file << "#include <ulxmlrpcpp/ulxr_response.h>\n";
  cpp_file << "#include <ulxmlrpcpp/ulxr_call.h>\n";
  cpp_file << "#include <ulxmlrpcpp/ulxr_dispatcher.h>\n";
  cpp_file << "#include <ulxmlrpcpp/ulxr_value.h>\n\n";

  cpp_file << "#include \"" << theClass.getSource() << "\"\n\n\n";
  cpp_file << "#include \"" << name + "_ulxr_names.h" << "\"\n\n";

  cpp_file << name << "Server::" << name << "Server"
           << "(ulxr::MethodAdder &in_method_adder, " << theClass.getName() << " &in_server)\n"
           << "   : server(in_server)\n"
              "   , method_adder(in_method_adder)\n";

  cpp_file << "{\n"
              "  setupServerMethods();\n"
              "}\n\n\n";

  cpp_file << name << "Server::~" << name << "Server()\n"
           << "{\n"
              "  removeServerMethods();\n"
              "}\n\n\n";
}


void GenerateServerMethods::generateSourceMethods(std::ostream & cpp_file)
{
  for (unsigned i = 0; i < theClass.numMethods(); ++i)
  {
    Method method = theClass.getMethod(i);
    method.extractNamespace();

    cpp_file << "// mapped to: " << method.getCppString(0, false, "");

    if (method.getName() != method.getOverloadName())
       cpp_file << "   (there are overloaded methods)";

    cpp_file << "\nulxr::MethodResponse\n  "
             << method.getOverloadName(true, "Server") << " (const ulxr::MethodCall &calldata)\n"
             << "{\n"
             << "  try\n"
             << "  {\n";

    for (unsigned iarg = 0; iarg < method.numArgs(); ++iarg)
    {
      std::string adap = method.getArg(iarg).getType().getTypeAdapter();
      std::string adap2;
      if (adap.length() != 0)
      {
        adap += "(";
        adap2 = ")";
      }

      cpp_file << "    " << method.getArg(iarg).getType().getName() << " p" << iarg << " = "
               << "(" << method.getArg(iarg).getType().getName() << ") "
               << adap + method.getArg(iarg).getType().getRpcName() << "(calldata.getParam(" << iarg << "))." << method.getArg(iarg).getType().getRpcAccessor()
               << "()" << adap2 << ";\n";
    }

    bool have_retval = false;
    if (method.getType().getName() != "void" || method.getType().getLeft() != "" || method.getType().getRight() != "")
      have_retval = true;

    if (have_retval)
    {
      cpp_file << "    " << method.getType().getProxyType() << " retval = "
               << method.getType().getTypeDereference();
    }
    else
      cpp_file << "    ";

    cpp_file << "server." << method.getName() << "(";

    for (unsigned iarg = 0; iarg < method.numArgs(); ++iarg)
    {
      if (iarg != 0)
        cpp_file << ", ";

//      cpp_file << "(" << method.getArg(iarg).getType().getCppString() << ")";

      if(method.getArg(iarg).getType().isPointer())
        cpp_file << "&";

      cpp_file  << "p" << iarg;
    }

    cpp_file << ");\n";

    std::string adap = method.getType().getInversTypeAdapter();
    std::string adap2;
    if (adap.length() != 0)
    {
      adap += "(";
      adap2 = ")";
    }

    if (have_retval)
      cpp_file << "    return ulxr::MethodResponse ("<< method.getType().getRpcName() << " (" <<  adap << "retval" << adap2 << "));\n";
    else
      cpp_file << "    return ulxr::MethodResponse (ulxr::Void());\n";

    cpp_file << "  }\n"
             << "  catch(std::exception &ex)\n"
             << "  {\n"
             << "    ulxr::CppString s = ULXR_PCHAR(\"C++ exception caught when invoking '" << method.getCppString(0, false, "") << "'\\n  \");\n"
             << "    s += ULXR_GET_STRING(ex.what());\n"
             << "    return ulxr::MethodResponse(ulxr::ApplicationError, s);\n"
             << "  }\n"
             << "  catch(...)\n"
             << "  {\n"
             << "    ulxr::CppString s = ULXR_PCHAR(\"Unknown exception caught when invoking '" << method.getCppString(0, false, "") << "'\");\n"
             << "    return ulxr::MethodResponse(ulxr::ApplicationError, s);\n"
             << "  }\n";

    cpp_file << "}\n\n\n";
  }

  // ------------------------------------

  cpp_file <<
    "\nvoid " << theClass.getBaseName() << "Server::removeServerMethods()\n"
    "{\n";

  for (unsigned i = 0; i < theClass.numMethods(); ++i)
  {
    Method method = theClass.getMethod(i);
    method.extractNamespace();

    cpp_file <<
      "  method_adder.removeMethod(ULXR_CALLTO_" << method.getOverloadName(true, "", "_") << ");";

    cpp_file << "  // mapped to: " << method.getCppString(0, false, "");

    if (method.getName() != method.getOverloadName())
       cpp_file << "   (there are overloaded methods)";

    cpp_file << "\n";
  }

  cpp_file << "\n}\n\n\n";
}


void GenerateServerMethods::generateHeaderClassHead(std::ostream & h_file,
                                                    const std::string &name)
{
  std::string ns = theClass.getNamespace();
  if (ns.length() == 0)
    h_file << "\n"
           << "class " << name << ";\n\n";
  else
    h_file << "namespace " << ns << "\n{\n"
           << "  class " << theClass.getBaseName() << ";\n"
           << "}\n\n";

  h_file << "\nnamespace ulxr\n"
         << "{\n"
         << "  class MethodAdder;\n"
         << "  class MethodCall;\n"
         << "  class MethodResponse;\n"
         << "}\n\n"
         << "class " << name << "Server\n"
         << "{\n"
         << "  public:\n\n"
         << "    " << name << "Server(ulxr::MethodAdder &method_adder, " << theClass.getName() << " &server);\n\n"
         << "    ~" << name << "Server();\n\n";
}


void GenerateServerMethods::generateHeaderVariables(std::ostream & h_file)
{
  h_file << "  private:\n\n";
  for (unsigned i = 0; i < theClass.numMethods(); ++i)
  {
    Method method = theClass.getMethod(i);
    method.extractNamespace();

    const bool reftype = method.getType().isReference();
    if (reftype)
      h_file << "    mutable " << method.getType().getName() << " ulxr_refFor_" << method.getOverloadName() << ";\n";
  }

  h_file << "    " << theClass.getName() << " &server;\n"
         << "    ulxr::MethodAdder &method_adder;\n";
}


