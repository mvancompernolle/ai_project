/**************************************************************************
                  ulxr_wbxmlparse.cpp  -  parse wbxml files
                             -------------------
    begin                : Fri Jan 09 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_wbxmlparse.cpp 940 2006-12-30 18:22:05Z ewald-arnold $

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

//#define ULXR_DEBUG_OUTPUT

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <ulxmlrpcpp/ulxr_wbxmlparse.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_htmlform_handler.h>


namespace ulxr {


#define MIB_Enum_UTF8      (char) 106
#define MIB_Enum_LATIN1    (char) 4

//#define MIB_Enum_UTF16     ULXR_MB_INT_14bit(1015)
//#define MIB_Enum_UTF16LE   ULXR_MB_INT_14bit(1014)
//#define MIB_Enum_UTF16BE   ULXR_MB_INT_14bit(1013)


ULXR_API_IMPL(const char)
  WbXmlParser::wbxml_START_SEQ_STR[]      = {
                                              0x01,  // WBXML 1.1
                                              0x01,  // unknown PubId
#ifdef ULXR_UNICODE
                                              MIB_Enum_UTF8,
#else
                                              MIB_Enum_LATIN1,
#endif
                                              0x00   // empty string table
                                            };

const unsigned WbXmlParser::wbxml_START_SEQ_LEN = 4;

enum WbStates
{
  stNone,              // at start
  stVersion,           // reading version
  stPubId,             // reading public identifier
  stCharset,           // reading charset
  stStrTableLen,       // reading string table length byte
  stBody,              // reading wbxml body

  stStr_I              // inline string
};


///////////////////////////////////////////////////////////////


ULXR_API_IMPL0 WbXmlParser::WbXmlState::WbXmlState(unsigned in_state)
{
  current_str = "";
  state = in_state;
}


///////////////////////////////////////////////////////////////


ULXR_API_IMPL0 WbXmlParser::WbXmlParser()
  : XmlParserBase()
{
  init();
}


ULXR_API_IMPL(void) WbXmlParser::init()
{
  lastErrorCode = 0;
  tag_count = 0;
  wbxmlstates.push(stNone);
}


ULXR_API_IMPL0 WbXmlParser::~WbXmlParser()
{
}


ULXR_API_IMPL(unsigned) WbXmlParser::getErrorCode() const
{
  return lastErrorCode;
}



ULXR_API_IMPL(int) WbXmlParser::parse(const char* buffer, int len, int isFinal)
{
  while(len > 0)
  {
    unsigned c = *buffer;
    ++buffer;
    --len;
    ULXR_TRACE((ULXR_PCHAR("wb parse: "))+HtmlFormHandler::makeHexNumber(c));
    ULXR_TRACE((ULXR_PCHAR("state: "))+HtmlFormHandler::makeNumber(wbxmlstates.top().state));

    switch(wbxmlstates.top().state)
    {
      case stNone:
        wb_version = c;
        ULXR_TRACE((ULXR_PCHAR("version: "))+HtmlFormHandler::makeHexNumber(c));
        wbxmlstates.top().state = stPubId;
      break;

      case stPubId:
        wb_pubId = c; // always unknown pub id
        ULXR_TRACE((ULXR_PCHAR("pub id: "))+HtmlFormHandler::makeHexNumber(c));
        wbxmlstates.top().state = stCharset;
      break;

      case stCharset:
        wb_charset = c; // always single byte mib enum
        ULXR_TRACE(ULXR_PCHAR("char set: ")+HtmlFormHandler::makeHexNumber(c));
        wbxmlstates.top().state = stStrTableLen;
      break;

      case stStrTableLen: // len always 0
        ULXR_TRACE(ULXR_PCHAR("str table len: "));
        wbxmlstates.top().state = stBody;
      break;

      case stStr_I:
        if (c == 0)
        {
          charData(wbxmlstates.top().current_str);
          wbxmlstates.pop();
        }
        else
          wbxmlstates.top().current_str += (char) c;
      break;

      case stBody:
        switch (c)
        {
          case wbxml_STR_I:
            ULXR_TRACE(ULXR_PCHAR("start inline string"));
            wbxmlstates.push(stStr_I);
          break;

          case wbxml_END:
            ULXR_TRACE(ULXR_PCHAR("end element"));
            endElement();
            tag_count--;
          break;

          default:
          {
            ULXR_TRACE(ULXR_PCHAR("start element: ")+HtmlFormHandler::makeHexNumber(c));
            tag_count++;
            Attributes atts;
            startElement(c, atts);
          }
        }
      break;
    }
  }

  if (isFinal && tag_count != 0)
    throw XmlException(NotWellformedError,
                      ulxr_i18n(ULXR_PCHAR("Problem while parsing wbxml structure")),
                      getCurrentLineNumber(),
                      ulxr_i18n(ULXR_PCHAR("Opening and closing tags are not equal")));

  return true;
}


ULXR_API_IMPL(CppString) WbXmlParser::getErrorString(unsigned /*code*/) const
{
  return ULXR_PCHAR("");  // TODO:
}


ULXR_API_IMPL(int) WbXmlParser::getCurrentLineNumber() const
{
  return tag_count;
}


ULXR_API_IMPL(int) WbXmlParser::mapToFaultCode(int xpatcode) const
{
  return xpatcode;
}


ULXR_API_IMPL(void) WbXmlParser::charData(const std::string &s)
{
  ULXR_TRACE(ULXR_PCHAR("WbXmlParser::charData()")
             << ULXR_PCHAR("\n  s: >>")
             << ULXR_GET_STRING(s)
             << ULXR_PCHAR("<<")
            );
  states.top()->appendCharData(s.data(), s.length());
  ULXR_TRACE(ULXR_PCHAR("WbXmlParser::charData() finished"));
}


ULXR_API_IMPL(void)
  WbXmlParser::startElement(unsigned token, const Attributes &attr)
{
  if (!testStartElement(token, attr))
    WbXmlParser::testStartElement(token, attr);
}


ULXR_API_IMPL(bool) WbXmlParser::testStartElement(unsigned token, const Attributes &/*attr*/)
{
  throw XmlException(NotWellformedError,
                     ulxr_i18n(ULXR_PCHAR("Problem while parsing wbxml structure")),
                     getCurrentLineNumber(),
                     ulxr_i18n(ULXR_PCHAR("unexpected opening token: "))+HtmlFormHandler::makeHexNumber(token));
}


ULXR_API_IMPL(bool) WbXmlParser::testEndElement()
{
  throw XmlException(NotWellformedError,
                     ulxr_i18n(ULXR_PCHAR("Problem while parsing wbxml structure")),
                     getCurrentLineNumber(),
                     ulxr_i18n(ULXR_PCHAR("unexpected closing token")));
}


}  // namespace ulxr





