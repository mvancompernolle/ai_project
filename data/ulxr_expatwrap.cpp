/***************************************************************************
                   ulxr_expatwrap.h  -  c++ wrapper for expat
                             -------------------
    begin                : Thu Apr 30 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_expatwrap.cpp 1074 2007-08-27 16:46:55Z ewald-arnold $

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


#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <ulxmlrpcpp/ulxr_expatwrap.h>


namespace ulxr {


ULXR_API_IMPL0 ExpatWrapper::ExpatWrapper(bool createParser)
  : XmlParserBase()
{
  if (createParser)
  {
    expatParser = ::XML_ParserCreate(0);
    setHandler();
  }
}


ULXR_API_IMPL0 ExpatWrapper::~ExpatWrapper()
{
  if (expatParser)  // allows subclasses to avoid finishing parsing
    ::XML_ParserFree(expatParser);
}


ULXR_API_IMPL(void) ExpatWrapper::setHandler()
{
  ::XML_SetUserData(expatParser, this);
  ::XML_SetElementHandler(expatParser, startElementCallback, endElementCallback);
  ::XML_SetCharacterDataHandler(expatParser, charDataCallback);
}


ULXR_API_IMPL(void) ExpatWrapper::resetParser()
{
  ::XML_ParserReset(expatParser, 0);
  setHandler();
}


ULXR_API_IMPL(void) ExpatWrapper::startElement(const XML_Char*, const XML_Char**)
{
}


ULXR_API_IMPL(void) ExpatWrapper::endElement(const XML_Char*)
{
}


ULXR_API_IMPL(void) ExpatWrapper::charData(const XML_Char*, int )
{
}


ULXR_API_IMPL(int) ExpatWrapper::mapToFaultCode(int xpatcode) const
{
   int fc = NotWellformedError;
   // try to map error codes
   switch (xpatcode)
   {
     case XML_ERROR_UNKNOWN_ENCODING:
       fc = UnsupportedEncodingError;
     break;

     case XML_ERROR_BAD_CHAR_REF:
     case XML_ERROR_PARTIAL_CHAR:
     case XML_ERROR_INCORRECT_ENCODING: // fallthrough
       fc = InvalidCharacterError;
     break;

     default:
       fc = NotWellformedError;
   }
   return fc;
}


ULXR_API_IMPL(void)
  ExpatWrapper::startElementCallback(void *userData,
                                     const XML_Char* name,
                                     const XML_Char** atts)
{
  ((ExpatWrapper*)userData)->startElement(name, atts);
}


ULXR_API_IMPL(void)
  ExpatWrapper::endElementCallback(void *userData, const XML_Char* name)
{
   ((ExpatWrapper*)userData)->endElement(name);
}


ULXR_API_IMPL(void)
  ExpatWrapper::charDataCallback(void *userData, const XML_Char* s, int len)
{
   ((ExpatWrapper*)userData)->charData(s, len);
}


ULXR_API_IMPL(int)  ExpatWrapper::parse(const char* buffer, int len, int isFinal)
{
  return ::XML_Parse(expatParser, buffer, len, isFinal);
}


ULXR_API_IMPL(unsigned)  ExpatWrapper::getErrorCode() const
{
  return ::XML_GetErrorCode(expatParser);
}


ULXR_API_IMPL(CppString)  ExpatWrapper::getErrorString(unsigned code) const
{
#ifdef ULXR_UNICODE
  return ULXR_GET_STRING(XML_ErrorString((XML_Error) code));
#else
  return XML_ErrorString((XML_Error) code);
#endif
}


ULXR_API_IMPL(int)  ExpatWrapper::getCurrentLineNumber() const
{
  return ::XML_GetCurrentLineNumber(expatParser);
}


}  // namespace ulxr
