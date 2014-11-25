/***************************************************************************
             UlxrIdlParser.cpp  -  parse Ulxr idl files
                             -------------------
    begin                : Sun May 27 2007
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: UlxrIdlClass.cpp 1162 2009-12-16 15:02:27Z ewald-arnold $

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


#include "UlxrIdlClass.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <iostream>


struct MethodComp
{
  bool operator()(const Method *lhs, const Method *rhs)
  {
    return lhs->getName() < rhs->getName();
  }
};


Type::Type()
{
}


void Type::setProperty(const std::string &in_left, const std::string &in_right)
{
  left = in_left;
  right = in_right;
}


void Type::setName(const std::string &in_name)
{
  name = in_name;
  simplename = in_name;

  unsigned pos;

  const std::string spaces = "  ";
  while ((pos = simplename.find(spaces)) != std::string::npos)
    simplename.erase(pos, 1);

  const std::string unsigneds = "unsigned";
  while ((pos = simplename.find(unsigneds)) != std::string::npos)
    simplename.erase(pos, unsigneds.length());

  const std::string signeds = "signed";
  while ((pos = simplename.find(signeds)) != std::string::npos)
    simplename.erase(pos, signeds.length());

  const std::string consts = "const";
  while ((pos = simplename.find(consts)) != std::string::npos)
    simplename.erase(pos, consts.length());
}


std::string Type::getRpcName() const
{
  if (   simplename == "char"
      && isPointer())
    return "ulxr::RpcString";

  else if (   simplename == "wchar_t"
           && isPointer())
    return "ulxr::RpcString";

  else if (   simplename == "int"
           || simplename == "long int"
           || simplename == "short int"
           || simplename == "char"
           || simplename == "wchar_t"
           || simplename == "long")
    return "ulxr::Integer";

  else if (simplename == "std::string"
           || simplename == "std::wstring"
           || simplename == "std::basic_string<char>"
           || simplename == "std::basic_string<wchar_t>")
    return "ulxr::RpcString";

  else if (   simplename == "float"
           || simplename == "double")
    return "ulxr::Double";

  else if (simplename == "bool")
    return "ulxr::Boolean";

  else if (simplename == "void")
    return "ulxr::Void";

  return std::string("??") + simplename + "/" + name + "??";
}


std::string Type::getRpcAccessor() const
{
  if (   simplename == "char"
      && isPointer())
    return "getString";

  else if (   simplename == "wchar_t"
           && isPointer())
    return "getString";

  else if (   simplename == "int"
      || simplename == "long int"
      || simplename == "short int"
      || simplename == "char"
      || simplename == "wchar_t"
      || simplename == "long")
    return "getInteger";

  else if (simplename == "std::string"
           || simplename == "std::wstring"
           || simplename == "std::basic_string<char>"
           || simplename == "std::basic_string<wchar_t>")
    return "getString";

  else if (   simplename == "float"
           || simplename == "double")
    return "getDouble";

  else if (simplename == "bool")
    return "getBoolean";

  return std::string("??") + simplename + "/" + name + "??";
}


std::string Type::getProxyType() const
{
  if (   simplename == "char"
      && isPointer())
    return "std::string";

  else if (   simplename == "wchar_t"
           && isPointer())
    return "std::wstring";

  else if (   simplename == "int"
           || simplename == "long int"
           || simplename == "short int"
           || simplename == "char"
           || simplename == "wchar_t"
           || simplename == "long")
    return "long int";

  else if (simplename == "std::basic_string<char>"
           || simplename == "std::string")
    return "std::string";

  else if (   simplename == "std::wstring"
           || simplename == "std::basic_string<wchar_t>")
    return "std::wstring";

  else if (   simplename == "float"
           || simplename == "double")
    return "double";

  else if (   simplename == "bool"
           /* || simplename == "double" */)
    return "bool";

  return std::string("??") + simplename + "/" + name + "??";
}


std::string Type::getTypeDereference() const
{
  if (   simplename == "char"
      && isPointer())
    return "";

  else if (   simplename == "wchar_t"
           && isPointer())
    return "";

  if (isPointer())
    return "*";

  else
    return "";
}


std::string Type::getTypeAccessor() const
{
  if (   simplename == "char"
      && isPointer())
    return ".c_str()";

  else if (   simplename == "wchar_t"
           && isPointer())
    return ".c_str()";

  return "";
}


std::string Type::getTypeAdapter() const
{
  if (   simplename == "char"
      && isPointer())
    return "ulxr::getLatin1";

  else if (   simplename == "wchar_t"
           && isPointer())
    return "ulxr::getUnicode";

  else if (   simplename == "std::basic_string<char>"
           || simplename == "std::string")
    return "ulxr::getLatin1";

  else if (   simplename == "std::wstring"
           || simplename == "std::basic_string<wchar_t>")
    return "ulxr::getUnicode";

  return "";
}


std::string Type::getInversTypeAdapter() const
{
  if (   simplename == "char"
      && isPointer())
    return "ULXR_GET_STRING";

  else if (   simplename == "wchar_t"
           && isPointer())
    return "ULXR_GET_STRING";

  else if (   simplename == "std::basic_string<char>"
           || simplename == "std::string")
    return "ULXR_GET_STRING";

  else if (   simplename == "std::wstring"
           || simplename == "std::basic_string<wchar_t>")
    return "ULXR_GET_STRING";

  return "";
}


std::string Type::getLeft() const
{
  return left;
}


std::string Type::getRight() const
{
  return right;
}


bool Type::isReference() const
{
  return  getRight().find("&") != std::string::npos;
}


bool Type::isPointer() const
{
  return  getRight().find("*") != std::string::npos;
}


std::string Type::getName() const
{
  return name;
}


void Type::clear()
{
  name.clear();
  left.clear();
  right.clear();
}


std::string Type::getCppString() const
{
  std::string ret;

  if (left.length() != 0)
    ret += left +  " ";

  ret += name;

  if (right.length() != 0)
    ret += " " + right;

  return ret;
}


//////////////////////////////////////////////////////////


Argument::Argument()
{
}


void Argument::setName(const std::string &in_name)
{
  name = in_name;
}


std::string Argument::getConstCastedName() const
{
  return Argument::getConstCastedName(name, getType());
}

namespace
{
  static std::string sconst = "const";
  static unsigned lconst = sconst.length();
}

std::string Argument::getConstCastedName(const std::string &name,
                                         const Type &type,
                                         const std::string &prefix,
                                         const std::string &suffix)
{
  bool consttype = false;

  std::string sleft = type.getLeft();
  unsigned pos;
  while ((pos = sleft.find(sconst)) != std::string::npos)
  {
    consttype = true;
    sleft.erase(pos, lconst);
  }

  std::string sright = type.getRight();
  while ((pos = sright.find(sconst)) != std::string::npos)
  {
    consttype = true;
    sright.erase(pos, lconst);
  }

  if (!consttype)
    return name;

  while ((pos = sright.find("&")) != std::string::npos)
    sright.erase(pos, 1);

//  return std::string("const_cast<") + prefix + sleft + " " + type.getName() + " " + sright + suffix + ">(" + name + ")";
  return std::string("(") + prefix + sleft + " " + type.getName() + " " + sright + suffix + ")(" + name + ")";
}


std::string Argument::getName() const
{
  return name;
}


void Argument::setType(const Type &in_type)
{
  type = in_type;
}


Type Argument::getType() const
{
  return type;
}


std::string Argument::getCppString() const
{
  return type.getCppString() + " " + name;
}


void Argument::clear()
{
  type.clear();
  name.clear();
}


//////////////////////////////////////////////////////////


Method::Method()
  : isfunction(false)
{
}


void Method::setProperty(bool constructor,
                         bool in_virtual,
                         bool in_const)
{
  isvirtual = in_virtual;
  isconst = in_const;
  isconstructor = constructor;
}


void Method::setName(const std::string &in_classname, const std::string &in_name)
{
  name = in_name;
  setOverloadName(in_name);
  classname = in_classname;
}


void Method::setName(const std::string &in_name)
{
  name = in_name;
  setOverloadName(in_name);
}


std::string Method::extractNamespace()
{
  std::string ns;
  unsigned pos;
  while ((pos = name.find("::")) != std::string::npos)
  {
    if (ns.length() != 0)
      ns += "::";
    ns += name.substr(0, pos);
    name.erase(0, pos+2);
  }

  while ((pos = classname.find("::")) != std::string::npos)
    classname.erase(0, pos+2);

//  setOverloadName(name);
  return ns;
}


std::string Method::getName(bool fully) const
{
  if (fully)
    return classname + "::" + name;

  else
    return name;
}


void Method::setOverloadName(const std::string &in_name)
{
  overloadname = in_name;
  unsigned pos = 0;
  while ((pos = overloadname.find(":")) != std::string::npos)
    overloadname[pos] = '_';
}


std::string Method::getOverloadName(bool fully,
                                    const std::string &suffix,
                                    const std::string &binder) const
{
  if (fully && !isfunction)
    return classname + suffix + binder + overloadname;

  else
    return overloadname;
}


void Method::setType(const Type &in_type)
{
  type = in_type;
}


Type Method::getType() const
{
  return type;
}


void Method::addArgument(const Argument &arg)
{
  args.push_back(arg);
}


std::string Method::getTemplateSignature() const
{
  std::string s;
  for (unsigned i = 0; i < args.size(); ++i)
  {
    if (i != 0)
      s += ", ";

    bool reftype = args[i].getType().getRight().find("&") != std::string::npos;
    std::string left = args[i].getType().getLeft();

    unsigned pos;
    while ((pos = left.find(sconst)) != std::string::npos)
      left.erase(pos, lconst);

    if (left.length() != 0)
      s += left +  " ";

    s += args[i].getType().getName();

    std::string right = args[i].getType().getRight();

    while ((pos = right.find(sconst)) != std::string::npos)
      right.erase(pos, lconst);

    if (!reftype && right.length() != 0)
      s += " " + right;
  }

  //

  std::string left = getType().getLeft();
  std::string right = getType().getRight();
  bool reftype = right.find("&") != std::string::npos;

  std::string ret;

  unsigned pos;
  if ((pos = left.find(sconst)) != std::string::npos)
    left.erase(pos, lconst);

  if (left.length() != 0)
    ret += left +  " ";

  ret += getType().getName();

  while ((pos = right.find(sconst)) != std::string::npos)
    right.erase(pos, lconst);

  if (!reftype && right.length() != 0)
    ret += " " + right;

  if (s.length() != 0)
    ret += std::string(", ") + s;

  return ret;
}


std::string Method::getSignature() const
{
  std::string ret;

  for (unsigned i = 0; i < args.size(); ++i)
  {
    if (i != 0)
      ret += ", ";

    ret += args[i].getType().getCppString();
  }

  return ret;
}


void Method::setFunction(bool isfunc)
{
  isfunction = isfunc;
}

std::string Method::getCppString(unsigned indent,
                                 bool impl,
                                 const std::string &suffix) const
{
  return getCppString(name, indent, impl, suffix);
}


std::string Method::getCppString(const std::string &my_name,
                                 unsigned indent,
                                 bool impl,
                                 const std::string &suffix) const
{
  std::string ret;

//   if (isvirtual && !impl)
//     ret += "virtual ";

  if (!isconstructor)
    ret += type.getCppString() + " ";

  else
    ret += "/*ctor*/ ";

  if (impl && !isfunction)
    ret += classname + suffix + "::" + my_name;
  else
    ret += my_name;

  ret += "(";

  for (unsigned i = 0; i < args.size(); ++i)
  {
    if (i != 0)
      ret += ", ";

    ret += args[i].getCppString();
  }

  ret += ")";

  if (isconst)
    ret += " const";

  if (!impl)
    ret += ";";

  return ret;
}


bool Method::isConstructor() const
{
  return isconstructor;
}


bool Method::isVirtual() const
{
  return isvirtual;
}


unsigned Method::numArgs() const
{
  return args.size();
}


Argument Method::getArg(unsigned i) const
{
  return args[i];
}


bool Method::operator<(const Method &rhs) const
{
  return name < rhs.name;
}


void Method::clear()
{
  args.clear();
  name.clear();
  overloadname.clear();
  classname.clear();
// sourcefile
}


void Method::OverloadRename(Method *method, unsigned ctr)
{
   std::stringstream ss;
   ss << method->getName() << "_ovr" << ctr;
   std::cout << "Function overload remapped: "
             << method->getCppString(0, true, "") << " --> "
             << ss.str() << std::endl;
   method->setOverloadName(ss.str());
}


void Method::resolveOverloaded(std::vector<Method> &methods)
{
   // Workaround BCB5: intermediate vector with pointer

   std::vector<Method*> overp;
   for (unsigned i = 0; i < methods.size(); ++i)
     overp.push_back(&methods[i]);

   if (overp.size() != 0)
   {
     std::sort(overp.begin(), overp.end(), MethodComp());

     for (unsigned i = 0; i < overp.size(); ++i)
     {
       std::string s = overp[i]->getName();
       unsigned ctr = 0;
       if (i < overp.size()-1 && s == overp[i+1]->getName())
       {
         OverloadRename(overp[i], ctr);
         ++ctr;
         ++i;

         while (i < overp.size() && s == overp[i]->getName())
         {
           OverloadRename(overp[i], ctr);
           ++ctr;
           ++i;
         }
       }
     }
   }
}


std::string Method::getSource() const
{
  return sourcefile;
}


void Method::setSource(const std::string &in_name)
{
  sourcefile = in_name;
}

//////////////////////////////////////////////////////////


UlxrIdlClass::UlxrIdlClass()
{
}


void UlxrIdlClass::setName(const std::string &in_name)
{
  name = in_name;
}


std::string UlxrIdlClass::getName() const
{
  return name;
}


std::string UlxrIdlClass::getBaseName() const
{
  std::string rem = name;
  unsigned pos;
  while ((pos = rem.find("::")) != std::string::npos)
    rem.erase(0, pos+2);

  return rem;
}


std::string UlxrIdlClass::getNamespace() const
{
  std::string ns;
  std::string s = name;
  unsigned pos;
  while ((pos = s.find("::")) != std::string::npos)
  {
    if (ns.length() != 0)
      ns += "::";
    ns += name.substr(0, pos);
    s.erase(0, pos+2);
  }

  return ns;
}


void UlxrIdlClass::addMethod(const Method &method)
{
  if (method.isConstructor())
    ctors.push_back(method);

  else
    methods.push_back(method);

  unsigned cnt = method.numArgs();
  if (std::find(argsCnt.begin(), argsCnt.end(), cnt) == argsCnt.end())
    argsCnt.push_back(cnt);
}


std::vector<unsigned> UlxrIdlClass::getArgsCount() const
{
  return argsCnt;
}


void UlxrIdlClass::clear()
{
  methods.clear();
  ctors.clear();
  argsCnt.clear();
}


unsigned UlxrIdlClass::numMethods() const
{
  return methods.size();
}


Method UlxrIdlClass::getMethod(unsigned i) const
{
  return methods[i];
}


const std::vector<Method> &
  UlxrIdlClass::getAllMethods() const
{
  return methods;
}


unsigned UlxrIdlClass::numCtors() const
{
  return ctors.size();
}


Method UlxrIdlClass::getCtor(unsigned i) const
{
  return ctors[i];
}


void UlxrIdlClass::OverloadRename(Method *method, unsigned ctr)
{
   std::stringstream ss;
   ss << method->getName() << "_ovr" << ctr;
   std::cout << "Method overload remapped: "
             << method->getCppString(0, true, "") << " --> "
             << ss.str() << std::endl;
   method->setOverloadName(ss.str());
}


void UlxrIdlClass::resolveOverloaded()
{
   // Workaround BCB5: intermediate vector with pointer

   std::vector<Method*> overp;
   for (unsigned i = 0; i < methods.size(); ++i)
     overp.push_back(&methods[i]);

   if (overp.size() != 0)
   {
     std::sort(overp.begin(), overp.end(), MethodComp());

     for (unsigned i = 0; i < overp.size(); ++i)
     {
       std::string s = overp[i]->getName();
       unsigned ctr = 0;
       if (i < overp.size()-1 && s == overp[i+1]->getName())
       {
         OverloadRename(overp[i], ctr);
         ++ctr;
         ++i;

         while (i < overp.size() && s == overp[i]->getName())
         {
           OverloadRename(overp[i], ctr);
           ++ctr;
           ++i;
         }
       }
     }
   }
}


std::string UlxrIdlClass::getSource() const
{
  return sourcefile;
}


void UlxrIdlClass::setSource(const std::string &in_name)
{
  sourcefile = in_name;
}

