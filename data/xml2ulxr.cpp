/***************************************************************************
             xml2ulxr.cpp -- generate rpc methods from idl files
                             -------------------
    begin                : Sun May 20 2007
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: xml2ulxr.cpp 1162 2009-12-16 15:02:27Z ewald-arnold $

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


#include "UlxrIdlParser.h"
#include "GenServerMethod.h"
#include "GenClientMethod.h"
#include "GenServerFunction.h"
#include "GenClientFunction.h"
#include "xmlexcept.h"

#include <iostream>
#include <cstdio>

#ifdef feof  // remove legacy stuff
#undef feof
#endif

bool gen_client = false;
bool gen_server = false;
bool gen_funcs = false;
std::string funcsname;
std::string sourcename;
std::string destdir;


void gen_functions(const std::vector<Method> &theMethods)
{
    if (gen_server && gen_funcs)
    {
      GenerateServerFunctions converter(theMethods);
      converter.generate(destdir, funcsname);
    }

    if (gen_client && gen_funcs)
    {
      GenerateClientFunctions converter(theMethods);
      converter.generate(destdir, funcsname);
    }

    if (!gen_client && !gen_server)
    {
      std::cerr << "No conversion method given\n";
      exit(1);
    }
}


void gen_file(const UlxrIdlClass &theClass)
{
    if (gen_server)
    {
      GenerateServerMethods converter(theClass);
      converter.generate(destdir, theClass.getBaseName());
    }

    if (gen_client)
    {
      GenerateClientMethods converter(theClass);
      converter.generate(destdir, theClass.getBaseName());
    }

    if (!gen_client && !gen_server)
    {
      std::cerr << "No conversion method given\n";
      exit(1);
    }
}


int main(int argc, char**argv)
{
  int ret = 1;


  if (argc < 2)
  {
    std::cerr
      << "No parameters given\n"
      << "Usage:\n"
      << "xml2ulxr [--gen-server]\n"
      << "         [--gen-client]\n"
      << "         [--gen-class=class-name]\n"
      << "         [--gen-funcs=prefix]\n"
      << "         [--file-name=filename.ext]\n"
      << "         [--dest-dir=dest-path]\n"
      << "         [--disable-timestamp\n"
      << "\n";
    return 1;
  }

  std::vector<std::string> classes;

  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];
    if (arg == "--gen-server")
      gen_server = true;

    else if (arg == "--gen-client")
      gen_client = true;

    else if (arg.substr(0, 12) == "--gen-class=")
      classes.push_back(arg.substr(12));

    else if (arg.substr(0, 12) == "--gen-funcs=")
    {
      funcsname = arg.substr(12);
      gen_funcs = true;
    }

    else if (arg.substr(0, 12) == "--file-name=")
      sourcename = arg.substr(12);

    else if (arg.substr(0, 11) == "--dest-dir=")
      destdir = arg.substr(11);

    else if (arg == "--disable-timestamp")
      GenerateMethod::setDateOutput(false);

    else
    {
      std::cerr << "Unknown option: " << arg << std::endl;
      return 1;
    }
  }

  if (sourcename.length() == 0)
  {
    std::cerr << "No source file given\n";
    return 1;
  }

  std::FILE *fs;
  fs = std::fopen (sourcename.c_str(), "r");

  if (fs == 0)
  {
    std::cerr << "File not readable: " << sourcename << std::endl;
    return 2;
  }

  char buffer[1000];

  UlxrIdlParser parser;
  try
  {
    while (!feof(fs))
    {
      unsigned readed = std::fread(buffer, 1, sizeof(buffer), fs);
      if (!parser.parse(buffer, readed, false))
      {
        throw XmlException(parser.mapToFaultCode(parser.getErrorCode()),
                          "Problem while parsing idl file",
                          parser.getCurrentLineNumber(),
                          parser.getErrorString(parser.getErrorCode()));
      }
    }
    ret = 0;
  }

  catch(std::exception &ex)
  {
    std::cerr << ex.what() << std::endl;
    ret = 1;
  }

  catch(...)
  {
    std::cerr << "Unexpected problem while parsing idl file\n";
    ret = 1;
  }

  fclose(fs);

  if (ret != 0)
    return ret;

  try
  {
    for (unsigned ii = 0; ii < parser.numClasses(); ++ii)
    {
//    std::cout << "Class available: " << parser.getClass(ii).getName() << std::endl;

      if (classes.size() == 0)
        gen_file(parser.getClass(ii));

      else
      {
        for (unsigned ic = 0; ic < classes.size(); ++ic)
        {
          if (classes[ic] == parser.getClass(ii).getName())
            gen_file(parser.getClass(ii));
          else
            std::cout << "Skipping class: " << parser.getClass(ii).getName() << std::endl;
        }
      }
    }

    gen_functions(parser.getFunctions());

    ret = 0;
  }

  catch(std::exception &ex)
  {
    std::cerr << ex.what() << std::endl;
    ret = 2;
  }

  catch(...)
  {
    std::cerr << "Unexpected problem while generating mock files\n";
    ret = 2;
  }

  return ret;
}


