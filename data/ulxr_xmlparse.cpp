/**************************************************************************
               ulxr_xmlparse.cpp  -  parse xml files
                             -------------------
    begin                : Thu Aug 26 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_xmlparse.cpp 1157 2009-08-29 09:02:31Z ewald-arnold $

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
#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <cstring>

#include <ulxmlrpcpp/ulxr_xmlparse.h>
#include <ulxmlrpcpp/ulxr_except.h>


namespace ulxr {


ULXR_API_IMPL0 XmlParser::XmlParser()
  : ExpatWrapper()
{
}


ULXR_API_IMPL(void) XmlParser::charData(const XML_Char *s, int len)
{
  ULXR_TRACE(ULXR_PCHAR("XmlParser::charData(const XML_Char*, int)")
/*             << ULXR_PCHAR("\n  len: ") << len
             << ULXR_PCHAR("\n  s: >>")
             << ULXR_GET_STRING(Cpp8BitString(s, len))
             << ULXR_PCHAR("<<")
*/
            );
  states.top()->appendCharData(s, len);
  ULXR_TRACE(ULXR_PCHAR("XmlParser::charData(const XML_Char*, int) finished"));
}


ULXR_API_IMPL(bool) XmlParser::testStartElement(const XML_Char *name, const XML_Char ** /*atts*/)
{
  throw XmlException(NotWellformedError,
                  ulxr_i18n(ULXR_PCHAR("Problem while parsing xml structure")),
                  getCurrentLineNumber(),
                  ulxr_i18n(ULXR_PCHAR("unexpected opening tag: "))+ULXR_GET_STRING(name) );
}


ULXR_API_IMPL(bool) XmlParser::testEndElement(const XML_Char *name)
{
  throw XmlException(NotWellformedError,
                  ulxr_i18n(ULXR_PCHAR("Problem while parsing xml structure")),
                  getCurrentLineNumber(),
                  ulxr_i18n(ULXR_PCHAR("unexpected closing tag: "))+ULXR_GET_STRING(name) );
}


ULXR_API_IMPL(void) XmlParser::assertEndElement(const char *current, const char *expected)
{
  ULXR_TRACE(ULXR_PCHAR("XmlParser::assertEndElement(const char*, const char*): ")
             << ULXR_PCHAR("\n curr: ")
             << ULXR_GET_STRING(current)
             << ULXR_PCHAR(" exp: ")
             << ULXR_GET_STRING(expected)
             );
  if (0 != strcmp(current, expected))
    throw XmlException(NotWellformedError,
                       (CppString) ulxr_i18n(ULXR_PCHAR("Unexpected xml tag: ")) + ULXR_GET_STRING(current)
                       + ulxr_i18n(ULXR_PCHAR(", wanted: ")) + ULXR_GET_STRING(expected),
                       getCurrentLineNumber(),
                       ulxr_i18n(ULXR_PCHAR("Document not wellformed")));
}


}  // namespace ulxr

