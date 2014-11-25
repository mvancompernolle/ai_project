/**************************************************************************
                   ulxr_xmlparse_base.cpp  -  parse xml files
                             -------------------
    begin                : Fri Jan 09 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_xmlparse_base.cpp 940 2006-12-30 18:22:05Z ewald-arnold $

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

#include <ulxmlrpcpp/ulxr_xmlparse_base.h>


namespace ulxr {


ULXR_API_IMPL0 XmlParserBase::XmlParserBase()
{
  ULXR_TRACE(ULXR_PCHAR("XmlParserBase::XmlParserBase()"));
  complete = false;
}


ULXR_API_IMPL0 XmlParserBase::~XmlParserBase()
{
}


ULXR_API_IMPL(void) XmlParserBase::clearStates()
{
  while (!states.empty())
  {
    delete states.top();
    states.pop();
  }
}

ULXR_API_IMPL(bool) XmlParserBase::isComplete() const
{
  return complete;
}


ULXR_API_IMPL(void) XmlParserBase::setComplete(bool comp)
{
  complete = comp;
}


//////////////////////////////////////////////////////////////////////////
//

ULXR_API_IMPL0 XmlParserBase::ParserState::ParserState (unsigned st)
  : state(st)
  , prevstate(eUnknown)
{
}


ULXR_API_IMPL0 XmlParserBase::ParserState::~ParserState()
{
}


ULXR_API_IMPL(unsigned) XmlParserBase::ParserState::getParserState() const
{
  return state;
}


ULXR_API_IMPL(unsigned) XmlParserBase::ParserState::getPrevParserState() const
{
  return prevstate;
}


ULXR_API_IMPL(void) XmlParserBase::ParserState::setPrevParserState(unsigned prev)
{
  prevstate = prev;
}


ULXR_API_IMPL(CppString) XmlParserBase::ParserState::getStateName() const
{
  return ULXR_PCHAR("eUnknown");
}


ULXR_API_IMPL(void) XmlParserBase::ParserState::appendCharData(const std::string &/*s*/)
{
  ULXR_TRACE(ULXR_PCHAR("XmlParserBase::ParserState::appendCharData(const std::string &)"));
#ifdef ULXR_UNICODE
#else
#endif
}


ULXR_API_IMPL(void) XmlParserBase::ParserState::appendCharData(const XML_Char *s, int len)
{
  ULXR_TRACE(ULXR_PCHAR("XmlParserBase::ParserState::appendCharData(const XML_Char *, int)"));
#ifdef ULXR_UNICODE
  cdata += utf8ToUnicode(Cpp8BitString(s, len));
#else
  cdata.append(s, len);
#endif
}


ULXR_API_IMPL(CppString) XmlParserBase::ParserState::getCharData() const
{
  ULXR_TRACE(ULXR_PCHAR("XmlParserBase::ParserState::getCharData()"));
  return cdata;
}


}  // namespace ulxr

