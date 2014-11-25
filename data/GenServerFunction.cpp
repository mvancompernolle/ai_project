/***************************************************************************
        GenServerMethod.cpp  -  generate class with server methods
                             -------------------
    begin                : Thu Juk 19 2007
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : Ulxr@ewald-arnold.de

    $Id: GenServerFunction.cpp 1164 2010-01-06 10:03:51Z ewald-arnold $

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

#include "GenServerFunction.h"

#include <fstream>
#include <iostream>

#include <sys/stat.h>


GenerateServerFunctions::GenerateServerFunctions(const std::vector<Method> &methods)
  : theMethods(methods)
{
  Method::resolveOverloaded(theMethods);
}


void GenerateServerFunctions::generate_H(const std::string &destdir, const std::string &name)
{
  const std::string h_name = destdir + name + "_ulxr_server.h";
  std::ofstream h_file(h_name.c_str());
  std::cout << "Header file will be created: " << h_name << std::endl;

  generateHeaderHead(h_file, name + "UlxrServer");
  generateHeaderMethods(h_file, name);
  h_file << "#endif // " << name + "UlxrServer" + "_H\n\n";
}


void GenerateServerFunctions::generate_CPP(const std::string &destdir, const std::string &name)
{
  const std::string h_name = name + "_ulxr_server.h";
  const std::string cpp_name = destdir + name + "_ulxr_server.cpp";
  std::ofstream cpp_file(cpp_name.c_str());
  std::cout << "Source file will be created: " << cpp_name << std::endl;

  generateSourceHead(cpp_file, h_name);
  generateSourceMethods(cpp_file, name);
}


void GenerateServerFunctions::generate_CPP_USER(const std::string &destdir,
                                                const std::string &name)
{
  const std::string h_name = name + "_ulxr_server.h";
  std::string cpp_name = destdir + name + "_ulxr_server_user.cpp";

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

//   if (theMethods.size() != 0)
//     cpp_file << "#include \"" << theMethods[0].getSource() << "\"\n";

  cpp_file << "#include \"" << name + "_ulxr_names.h" << "\"\n\n";

  for (unsigned i = 0; i < theMethods.size(); ++i)
    cpp_file << "\nulxr::MethodResponse\n  "
             << name << "_" << theMethods[i].getOverloadName(false) << " (const ulxr::MethodCall &calldata);\n";

  cpp_file <<
    "\n\nvoid " << name << "_setupServerMethods(ulxr::MethodAdder &method_adder)\n"
    "{\n";

  for (unsigned i = 0; i < theMethods.size(); ++i)
  {
    if (i != 0)
      cpp_file << "\n";

    const Method &method = theMethods[i];

    cpp_file << "// mapped to: " << method.getCppString(0, false, "");

    if (method.getName() != method.getOverloadName())
       cpp_file << "   (there are overloaded methods)";

    cpp_file <<
      "\n"
      "  method_adder.addMethod(ulxr::make_method(&" << name << "_" << method.getOverloadName(false) << "),\n"
      "                         " << method.getType().getRpcName() << "::getValueName(),\n"
      "                         ULXR_CALLTO_" << name << "_" << method.getOverloadName(false) << ",\n"
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


void GenerateServerFunctions::generate(const std::string &destdir, const std::string &name)
{
  GenerateMethod::generate_NameDefines(destdir, name, theMethods, name + "_");
  generate_H(destdir, name);
  generate_CPP(destdir, name);
  generate_CPP_USER(destdir, name);
}


void GenerateServerFunctions::generateHeaderMethods(std::ostream & h_file,
                                                    const std::string &name)
{
  h_file << "namespace ulxr\n"
         << "{\n"
         << "  class MethodAdder;\n"
         << "}\n\n";

  h_file << "void " << name << "_setupServerMethods(ulxr::MethodAdder &method_adder);\n\n";
  h_file << "void " << name << "_removeServerMethods(ulxr::MethodAdder &method_adder);\n\n";
}


void GenerateServerFunctions::generateSourceMethods(std::ostream & cpp_file,
                                                    const std::string &name)
{
  cpp_file << "#include <ulxmlrpcpp/ulxr_response.h>\n";
  cpp_file << "#include <ulxmlrpcpp/ulxr_requester.h>\n";
  cpp_file << "#include <ulxmlrpcpp/ulxr_except.h>\n";
  cpp_file << "#include <ulxmlrpcpp/ulxr_method_adder.h>\n\n";

  cpp_file << "#include \"" << name + "_ulxr_names.h" << "\"\n";
//  if (theMethods.size() != 0)
    cpp_file << "#include \"" << theMethods[0].getSource() << "\"\n";

  cpp_file << "\n";

  for (unsigned i = 0; i < theMethods.size(); ++i)
  {
    const Method &method = theMethods[i];

    cpp_file << "// mapped to: " << method.getCppString(0, false, "");

    if (method.getName() != method.getOverloadName())
       cpp_file << "   (there are overloaded methods)";

    cpp_file << "\nulxr::MethodResponse\n  "
             << name << "_" << method.getOverloadName(false) << " (const ulxr::MethodCall &calldata)\n"
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

    cpp_file << method.getName() << "(";

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
    "\nvoid " << name << "_removeServerMethods(ulxr::MethodAdder &method_adder)\n"
    "{\n";

  for (unsigned i = 0; i < theMethods.size(); ++i)
  {
    const Method &method = theMethods[i];
    cpp_file <<
      "  method_adder.removeMethod(ULXR_CALLTO_" << name << "_" << method.getOverloadName(false, "", "_") << ");\n";
  }

  cpp_file << "}\n\n";
}
