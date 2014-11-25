/***************************************************************************
               ulxr_log4j.h  -  logging based on log4j
                             -------------------
    begin                : Fri Jul 13 2007
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_log4j.cpp 1164 2010-01-06 10:03:51Z ewald-arnold $

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

#ifndef ULXR_OMIT_TCP_STUFF

#include <ulxmlrpcpp/ulxr_tcpip_connection.h>
#include <ulxmlrpcpp/ulxr_log4j.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_htmlform_handler.h>

#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
#include <ulxmlrpcpp/ulxr_mutex.h>
#endif

#include <sys/timeb.h>

#ifdef __BORLANDC__
#include <strstream>
#else
#include <sstream>
#endif

#include <ctime>

#ifdef __unix__
#include <sys/time.h>
#endif


namespace ulxr {


ULXR_API_IMPL0 Log4JSender::Log4JSender(const CppString &in_appname,
                                        TcpIpConnection &in_conn)
  : appname(in_appname)
  , conn(in_conn)
  , network_error(false)
  , threadname(ULXR_PCHAR("ulxr"))
  , seqnum(0)
  , disable_send(false)
  , first_log4j(false)

{
  char buffer [1000];
  ::gethostname(buffer, sizeof(buffer)-1);
  hostname = buffer;
  open();
}

ULXR_API_IMPL0 Log4JSender::~Log4JSender() throw()
{
}


void ULXR_API_IMPL0 Log4JSender::close()
{
  disable_send = true;
  try
  {
    if (conn.isOpen())
      conn.close();

    network_error = false;
  }
  catch(...)
  {
    network_error = true;
  }
}


void ULXR_API_IMPL0 Log4JSender::open()
{
  try
  {
    if (!conn.isOpen())
    {
      conn.setTimeout(2); 
      conn.open();
    }

    first_log4j = false;
    network_error = false;
    disable_send = false;
 }
  catch(...)
  {
    network_error = true;
  }
}


ULXR_INT64 ULXR_API_IMPL0 Log4JSender::currentTimeMillis()
{
#if defined(HAVE_GETTIMEOFDAY)
    timeval tp;
    ::gettimeofday(&tp, 0);

    return ((ULXR_INT64)tp.tv_sec * 1000) + (ULXR_INT64)(tp.tv_usec / 1000);
#elif defined(HAVE_FTIME) || defined(HAVE_STD_FTIME)
    struct timeb tp;
    ::ftime(&tp);

    return ((ULXR_INT64)tp.time * 1000) + (ULXR_INT64)tp.millitm;
#else
    return (ULXR_INT64)std::time(0) * 1000;
#endif
}


void ULXR_API_IMPL0 Log4JSender::send(const CppString &level,
                                      const CppString &message,
                                      const CppString &in_filename,
                                      unsigned line)
{
  if (network_error || disable_send)
    return;

  ++seqnum;

  Cpp8BitString s;

  if (!first_log4j)
  {
    first_log4j = true;
//    s += "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
  }

  s += "<log4j:event logger=\"";
#ifdef ULXR_UNICODE
  s += unicodeToUtf8(appname);
#else
  s += asciiToUtf8(appname);
#endif

  s += "\" timestamp=\"";
#ifndef __BORLANDC__
  std::stringstream ts;
#else
  std::strstream ts;
#endif
  ts << currentTimeMillis();
  s += ts.str();

  s += "\" sequenceNumber=\"";
  s += getLatin1(HtmlFormHandler::makeNumber(seqnum));

  s += "\" level=\"";
#ifdef ULXR_UNICODE
  s += unicodeToUtf8(level);
#else
  s += asciiToUtf8(level);
#endif
  s += "\" ";

  s += "thread=\"";
#ifdef ULXR_UNICODE
  s += unicodeToUtf8(threadname);
#else
  s += asciiToUtf8(threadname);
#endif
  s += "\"";

  s += ">\n";

  s += "  <log4j:message>";
#ifdef ULXR_UNICODE
  s += unicodeToUtf8(xmlEscape(message, true));
#else
  s += asciiToUtf8(xmlEscape(message, true));
#endif
  s += "</log4j:message>\n";

  classname = ULXR_PCHAR("");
  methodname = ULXR_PCHAR("");

  CppString filename = in_filename;
  // strip path from both unix and win
  if (filename.length() != 0)
  {
    unsigned pos;
    while ((pos = filename.find(ULXR_PCHAR("/"))) != CppString::npos)
      filename.erase(0, pos+1);

    while ((pos = filename.find(ULXR_PCHAR("\\"))) != CppString::npos)
      filename.erase(0, pos+1);
  }

  s += "  <log4j:locationInfo";
  s += " file=\"";
#ifdef ULXR_UNICODE
  s += unicodeToUtf8(filename);
#else
  s += asciiToUtf8(filename);
#endif
  s += "\" line=\"";
  if (line != 0)
    s += getLatin1(HtmlFormHandler::makeNumber(line));
  s += "\" class=\"";
  s += getLatin1(classname);
  s += "\" method=\"";
  s += getLatin1(methodname);
  s += "\" />\n";

  s += "  <log4j:properties>\n";
  s +="    <log4j:data name=\"application\" value=\"";
#ifdef ULXR_UNICODE
  s += unicodeToUtf8(appname);
#else
  s += asciiToUtf8(appname);
#endif
  s += "\" />\n";

  s += "    <log4j:data name=\"hostname\" value=\"";
  s += hostname;
  s += "\" />\n";

//  s += "    <log4j:data name=\"log4jid\" value=\"27\" />\n";
  s += "  </log4j:properties>\n";

  s += "</log4j:event>\n";

  try
  {
    disable_send = true;
    conn.write(s.data(), s.length());
    disable_send = false;
  }
  catch(...)
  {
    network_error = true;
  }
}

#endif // ULXR_OMIT_TCP_STUFF

}  // namespace ulxr

