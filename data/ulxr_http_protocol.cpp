/***************************************************************************
               ulxr_http_protocol.cpp  -  http prootocol
                             -------------------
    begin                : Mon May 3 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_http_protocol.cpp 1164 2010-01-06 10:03:51Z ewald-arnold $

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

// #define ULXR_SHOW_TRACE
// #define ULXR_DEBUG_OUTPUT
// #define ULXR_SHOW_HTTP
// #define ULXR_SHOW_READ
// #define ULXR_SHOW_WRITE
// #define ULXR_SHOW_XML


#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h> // always first

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <sys/stat.h>

#include <cstring>

#if defined(__BORLANDC__) || defined (_MSC_VER)
#include <utility>
#endif

#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_tcpip_connection.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_response.h>
#include <ulxmlrpcpp/ulxr_call.h>

#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
#include <ulxmlrpcpp/ulxr_mutex.h>
#endif


namespace ulxr
{

struct HttpProtocol::PImpl
{
   CppString    proxy_user;
   CppString    proxy_pass;
   CppString    useragent;
   CppString    header_firstline;
   CppString    header_buffer;
   CppString    hostname;
   unsigned     hostport;

   bool         useconnect;
   bool         connected;

   ConnectorWrapperBase  *connector;

   bool            bChunkedEncoding;
   int             chunk_size;
   bool            chunk_terminated;
   bool            chunk_in_header;
   Cpp8BitString   chunk_data;
   unsigned        chunked_block;
   unsigned        chunk_body_skip;

   bool                              bAcceptcookies;
   std::map<CppString, CppString>    cookies;
   CppString                         serverCookie;
   CppString                         clientCookie;
   std::vector<CppString>            userTempFields;
   header_property                   headerprops;
};


ULXR_API_IMPL0 HttpProtocol::HttpProtocol(const HttpProtocol &prot)
  : Protocol(prot)
  , pimpl(new PImpl)
{
  *pimpl = *prot.pimpl;
}


ULXR_API_IMPL0
  HttpProtocol::HttpProtocol(Connection *conn, const CppString &hn, unsigned hp)
  : Protocol (conn)
  , pimpl(new PImpl)
{
    pimpl->hostname = hn;
    pimpl->hostport = hp;
    ULXR_TRACE(ULXR_PCHAR("HttpProtocol(conn, name, port)"));
    init();
}


ULXR_API_IMPL0
  HttpProtocol::HttpProtocol(TcpIpConnection *conn)
  : Protocol (conn)
  , pimpl(new PImpl)
{
    pimpl->hostname = conn->getPeerName();
    pimpl->hostport = conn->getPort();
    ULXR_TRACE(ULXR_PCHAR("HttpProtocol(conn)"));
    init();
}


ULXR_API_IMPL0 HttpProtocol::~HttpProtocol()
{
    ULXR_TRACE(ULXR_PCHAR("~HttpProtocol"));
    delete pimpl->connector;
    delete pimpl;
    pimpl = 0;
}


ULXR_API_IMPL(HttpProtocol *) HttpProtocol::clone() const
{
    ULXR_TRACE(ULXR_PCHAR("HttpProtocol::clone()"));
    return new HttpProtocol(*this);
}


ULXR_API_IMPL(Protocol *) HttpProtocol::detach()
{
    ULXR_TRACE(ULXR_PCHAR("HttpProtocol::detach()"));
    HttpProtocol *cloneprot = this->clone();
    cloneprot->setConnection(getConnection()->detach());
    cloneprot->pimpl->connector = new ConnectorWrapper<HttpProtocol>(cloneprot, &HttpProtocol::doConnect);
    return cloneprot;  // return previous and running connection
}


ULXR_API_IMPL(void) HttpProtocol::init()
{
    ULXR_TRACE(ULXR_PCHAR("init"));
    pimpl->connector = new ConnectorWrapper<HttpProtocol>(this, &HttpProtocol::doConnect);
    getConnection()->setConnector(pimpl->connector);
    pimpl->useconnect = false;
    pimpl->connected = false;
    ULXR_TRACE(ULXR_PCHAR("init"));
    pimpl->headerprops.clear();
    pimpl->useragent = ULXR_GET_STRING(ULXR_PACKAGE) + ULXR_PCHAR("/") + ULXR_GET_STRING(ULXR_VERSION);
    pimpl->userTempFields.clear();
    pimpl->bAcceptcookies = false;
    pimpl->bChunkedEncoding = false;
    pimpl->chunk_data.clear();
    pimpl->chunk_size = 0;
    pimpl->chunk_body_skip = 0;
    setChunkedTransfer(false);
}


ULXR_API_IMPL(void) HttpProtocol::clearHttpInfo()
{
    ULXR_TRACE(ULXR_PCHAR("clearHttpInfo"));
    pimpl->header_firstline = ULXR_PCHAR("");
    pimpl->header_buffer = ULXR_PCHAR("");
    pimpl->headerprops.clear();
    pimpl->cookies.clear();
    pimpl->bChunkedEncoding = false;
    pimpl->chunk_data.clear();
    pimpl->chunk_size = 0;
    pimpl->chunk_body_skip = 0;
    pimpl->chunk_terminated = false;
    pimpl->chunk_in_header = true;
}


ULXR_API_IMPL(void) HttpProtocol::resetConnection()
{
    ULXR_TRACE(ULXR_PCHAR("resetConnection"));
    Protocol::resetConnection();
    clearHttpInfo();
    //loadCookie(peername);
}


ULXR_API_IMPL(void) HttpProtocol::close()
{
    ULXR_TRACE(ULXR_PCHAR("close"));
    Protocol::close();
    pimpl->connected = false;
//    storeCookie(peername);
}


ULXR_API_IMPL(void) HttpProtocol::shutdown(int mode)
{
    ULXR_TRACE(ULXR_PCHAR("shutdown"));
    if (getConnection() != 0)
      getConnection()->shutdown(mode);
}


ULXR_API_IMPL(CppString) HttpProtocol::getHttpProperty(const CppString &in_name) const
{
    ULXR_TRACE(ULXR_PCHAR("getHttpProperty ") << in_name);
    CppString name = in_name;
    makeLower(name);
    header_property::const_iterator it;

    if ((it = pimpl->headerprops.find(name)) == pimpl->headerprops.end() )
        throw ConnectionException(NotConformingError,
                                  ulxr_i18n(ULXR_PCHAR("Http property field not available: "))+name, 400);

    return (*it).second;
}


ULXR_API_IMPL(bool) HttpProtocol::hasHttpProperty(const CppString &in_name) const
{
    CppString name = in_name;
    makeLower(name);
    bool b = pimpl->headerprops.find(name) != pimpl->headerprops.end();
    ULXR_TRACE(ULXR_PCHAR("hasHttpProperty: ") << in_name << ULXR_PCHAR(" ") << b);
    return b;
}


ULXR_API_IMPL(void) HttpProtocol::parseHeaderLine()
{
    ULXR_TRACE(ULXR_PCHAR("parseHeaderLine"));

    if (pimpl->header_firstline.length() == 0)
    {
        pimpl->header_firstline = pimpl->header_buffer;
        ULXR_DOUT_HTTP(ULXR_PCHAR("firstline: <") << pimpl->header_firstline << ULXR_PCHAR(">"));
    }
    else
    {
        CppString nm, cont;
        std::size_t pos = pimpl->header_buffer.find(':');
        if (pos == CppString::npos)
        {
            nm = pimpl->header_buffer;
            cont = ULXR_PCHAR("");
        }
        else
        {
            nm = pimpl->header_buffer.substr(0, pos);
            cont = pimpl->header_buffer.substr(pos+1);
        }

        makeLower(nm);
        cont = stripWS(cont);
        nm = stripWS(nm);
        pimpl->headerprops.insert(std::make_pair(nm, cont));

        if (pimpl->bAcceptcookies && (nm == ULXR_PCHAR("set-cookie")))  // distinguish between cookie / set-cookie?
          setCookie(cont);

        else if (pimpl->bAcceptcookies && (nm == ULXR_PCHAR("cookie")))
          setCookie(cont);

        ULXR_DOUT_HTTP(ULXR_PCHAR("headerprop: <") << nm
                       << ULXR_PCHAR("> + <") << cont << ULXR_PCHAR("> "));
    }
    pimpl->header_buffer = ULXR_PCHAR("");
}


bool HttpProtocol::hasClosingProperty()
{
  bool do_close = false;
  if (hasHttpProperty(ULXR_PCHAR("connection")))
  {
      CppString sConnect = getHttpProperty(ULXR_PCHAR("connection"));
      makeLower(sConnect);
      if (sConnect == CppString(ULXR_PCHAR("close")))
          do_close = true;
  }

  if (hasHttpProperty(ULXR_PCHAR("proxy-connection")))
  {
      CppString sConnect = getHttpProperty(ULXR_PCHAR("proxy-connection"));
      makeLower(sConnect);
      if (sConnect == CppString(ULXR_PCHAR("close")))
          do_close = true;
  }
  return do_close;
}


ULXR_API_IMPL(bool) HttpProtocol::checkContinue()
{
    ULXR_TRACE(ULXR_PCHAR("checkContinue"));
    CppString head_version;
    unsigned head_status = 500;
    CppString head_phrase;
    splitHeaderLine(head_version, head_status, head_phrase);
    if (head_status == 100)
    {
      ULXR_TRACE(ULXR_PCHAR("Ignoring header 100-Continue"));
      setConnectionState(ConnStart);
      return true;
    }
    else
      return false;
}


ULXR_API_IMPL(Protocol::State)
HttpProtocol::connectionMachine(char * &buffer, long &len)
{
    /*
      Each invokation of this state machine tries to parse one single
      http header line of the buffer. If the content of the buffer is too small
      (no linefeed found) the content is cached in an internal string
      and used the next time.
      buffer points to the beginning of the next line at return if
      a linefeed has been found. In the message body nothing is done.
    */
    ULXR_TRACE(ULXR_PCHAR("connectionMachine with ") << len << ULXR_PCHAR(" bytes"));
    if (len == 0 || buffer == 0)
        return getConnectionState();

    char *chunk_cursor = buffer;
    char *chunk_start = buffer;

    while (len > 0)
    {
        const unsigned state = getConnectionState();
        switch (state)
        {
        case ConnStart:
            setConnectionState(ConnHeaderLine);
            clearHttpInfo();
        break;

        case ConnPendingCR:
            if (*buffer == '\n') // CR+LF
            {
                --len;
                ++buffer;
            }

            if (pimpl->header_buffer.length() == 0)
                setConnectionState(ConnSwitchToBody);
            else
                setConnectionState(ConnPendingHeaderLine);
        break;

        case ConnPendingHeaderLine:
            if (pimpl->header_buffer.length() == 0)
                setConnectionState(ConnSwitchToBody);

            else if (*buffer != ' ')  // continuation line of current header field?
            {
                parseHeaderLine();
                setConnectionState(ConnHeaderLine);
            }
            else
              setConnectionState(ConnHeaderLine);
        break;

        case ConnHeaderLine:
//            ULXR_TRACE(ULXR_PCHAR("ConnHeaderLine:"));

            if (*buffer == '\r')
                setConnectionState(ConnPendingCR);

            else if (*buffer == '\n')
            {
                if (pimpl->header_buffer.length() == 0)
                    setConnectionState(ConnSwitchToBody);
                else
                    setConnectionState(ConnPendingHeaderLine);
            }

            else
                pimpl->header_buffer += *buffer;

            ++buffer;
            --len;
        break;

        case ConnSwitchToBody:
          machine_switchToBody(buffer, len, chunk_start, chunk_cursor);
        break;

        case ConnChunkHeader:
        {
          ULXR_TRACE(ULXR_PCHAR("ConnChunkHeader:"));

          char c = *buffer;
          if (c != '\n' && c != '\r')
            pimpl->chunk_data += c;

          ++buffer;
          --len;

          if (c == '\n')
          {
            char* pStop;
            pimpl->chunk_size = std::strtol(pimpl->chunk_data.c_str(), &pStop, 16);
            ULXR_TRACE(ULXR_PCHAR("chunk with ")
                       << pimpl->chunk_size
                       << ULXR_PCHAR(" bytes announced"));

            if (   *pStop != ' '
                && *pStop != 0
                && *pStop != ';')
            {
                setConnectionState(ConnError);
                throw ConnectionException(SystemError, ulxr_i18n(ULXR_PCHAR("chunk size is followed by a bad character: ")) + *pStop, 500);
            }

            if (getContentLength() > 0)
              setRemainingContentLength(getRemainingContentLength() - pimpl->chunk_size);
            else if(getContentLength() == 0)
              setRemainingContentLength(pimpl->chunk_size);

            if (pimpl->chunk_size < 1)  // chunk size == 0 terminates data block
            {
              pimpl->chunk_in_header = false;
              setConnectionState(State(ConnHeaderLine));
            }
            else
              setConnectionState(State(ConnChunkBody));
          }

          if (len <= 0)
          {
            len = chunk_cursor - chunk_start;
            buffer = chunk_start;
            if (len != 0)
              return ConnBody;  // fake regular body
            else
              return State(ConnChunkHeader);
          }
        }
        break;

        case ConnChunkBodySkip:
          buffer++;
          len--;
          if (--pimpl->chunk_body_skip <= 0)
          {
            pimpl->chunk_data.clear();
            setConnectionState(State(ConnChunkHeader));
          }
        break;

        case ConnChunkBody:
          ULXR_TRACE(ULXR_PCHAR("ConnChunkBody:"));
          while (pimpl->chunk_size > 0 && len > 0)
          {
            *chunk_cursor++ = *buffer++;
            --pimpl->chunk_size;
            --len;
          }

          if (pimpl->chunk_size <= 0)
          {
            pimpl->chunk_body_skip = 2;
            setConnectionState(State(ConnChunkBodySkip));
          }
        break;

        case ConnChunkTerminated:
            ULXR_TRACE(ULXR_PCHAR("ConnChunkTerminated:"));
            return State(ConnChunkTerminated);
        /*break; */

        case ConnBody:
            ULXR_TRACE(ULXR_PCHAR("ConnBody:"));
            return ConnBody;
        /*break; */

        case ConnError:
            ULXR_TRACE(ULXR_PCHAR("ConnError:"));
            return ConnError;
        /*break; */

        default:
            setConnectionState(ConnError);
            throw ConnectionException(SystemError, ulxr_i18n(ULXR_PCHAR("connectionMachine(): unknown state")), 500);
        }
    }

    if (getConnectionState() == ConnSwitchToBody)
      machine_switchToBody(buffer, len, chunk_start, chunk_cursor);

    ULXR_TRACE(ULXR_PCHAR("/connectionMachine"));

    if (pimpl->bChunkedEncoding)
    {
      len = chunk_cursor - chunk_start;
      buffer = chunk_start;
      if (len != 0)
        return ConnBody;
    }

    return getConnectionState();
}


void HttpProtocol::machine_switchToBody(char * &buffer,
                                        long   &len,
                                        char * &chunk_start,
                                        char * &chunk_cursor)
{
  ULXR_TRACE(ULXR_PCHAR("ConnSwitchToBody:"));
  if (pimpl->chunk_in_header)
  {
    if (!checkContinue())
    {
      if (hasHttpProperty(ULXR_PCHAR("transfer-encoding")))
      {
          CppString sEncoding = getHttpProperty(ULXR_PCHAR("transfer-encoding"));
          if (sEncoding == ULXR_PCHAR("chunked"))
          {
              setRemainingContentLength(-1);
              setContentLength(-1);
              ULXR_TRACE(ULXR_PCHAR("have chunked transfer encoding"));
              pimpl->bChunkedEncoding = true;
              pimpl->chunk_size = 0;
              pimpl->chunk_data.clear();
          }
      }

      if (!pimpl->bChunkedEncoding)
      {
        if (hasHttpProperty(ULXR_PCHAR("content-length")))
        {
            determineContentLength();

            ULXR_TRACE(ULXR_PCHAR("content_length: ") << getContentLength());
            ULXR_TRACE(ULXR_PCHAR("len: ") << len);

            if (getContentLength() >= 0)
                setRemainingContentLength(getContentLength() - len);
        }
        setConnectionState(ConnBody);
      }
      else
        setConnectionState(State(ConnChunkHeader));
    }
  }
  else
  {
    len = chunk_cursor - chunk_start;
    buffer = chunk_start;
    setConnectionState(State(ConnChunkTerminated));
    pimpl->chunk_terminated = true;
  }

  if (hasClosingProperty())
    setPersistent(false);
  else
    setPersistent(true);
}


ULXR_API_IMPL(bool) HttpProtocol::hasBytesToRead() const
{
  bool b = false;
  if (pimpl->bChunkedEncoding)
    b = !pimpl->chunk_terminated;
  else
    b = getRemainingContentLength() != 0;

  ULXR_TRACE(ULXR_PCHAR("hasBytesToRead: " << b));
  return b;
}


ULXR_API_IMPL(void) HttpProtocol::determineContentLength()
{
    ULXR_TRACE(ULXR_PCHAR("determineContentLength"));

    header_property::iterator it;
    if ((it = pimpl->headerprops.find(ULXR_PCHAR("content-length"))) != pimpl->headerprops.end() )
    {
        ULXR_TRACE(ULXR_PCHAR(" content-length: ") << (*it).second);
        setContentLength(ulxr_atoi(getLatin1((*it).second).c_str()));
        ULXR_TRACE(ULXR_PCHAR(" length: ") << getContentLength());
    }
    else
    {
      if (pimpl->bChunkedEncoding)
        setContentLength(0); // set with next chunk header
      else
        throw ConnectionException(NotConformingError,
                                  ulxr_i18n(ULXR_PCHAR("Content-Length of message not available")), 411);

    }

    setRemainingContentLength(getContentLength());
    ULXR_TRACE(ULXR_PCHAR(" content_length: ") << getContentLength());
}


ULXR_API_IMPL(void)
HttpProtocol::sendResponseHeader(int code,
                                 const CppString &phrase,
                                 const CppString &type,
                                 unsigned long len,
                                 bool wbxml_mode)
{
    // doConnect(); must already be pimpl->connected

    ULXR_TRACE(ULXR_PCHAR("sendResponseHeader"));
    char stat[40];
    ulxr_sprintf(stat, "%d", code );

    char contlen[40];
    ulxr_sprintf(contlen, "%ld", len );

    CppString ps = phrase;

    std::size_t pos = 0;
    while ((pos = ps.find('\n', pos)) != CppString::npos)
    {
        ps.replace(pos, 1, ULXR_PCHAR(" "));
        pos += 1;
    }

    pos = 0;
    while ((pos = ps.find(ULXR_CHAR('\r'), pos)) != CppString::npos)
    {
        ps.replace(pos, 1, ULXR_PCHAR(" "));
        pos += 1;
    }

    CppString http_str = (CppString) ULXR_PCHAR("HTTP/1.1 ")
                         + ULXR_GET_STRING(stat) + ULXR_PCHAR(" ") + ps
                         + ULXR_PCHAR("\r\n");

    if (!isPersistent())
        http_str += ULXR_PCHAR("Connection: Close\r\n");
    else
        http_str += ULXR_PCHAR("Proxy-Connection: Keep-Alive\r\n");

    if (len != 0 && type.length() != 0)
        http_str  += ULXR_PCHAR("Content-Type: ") + type + ULXR_PCHAR("\r\n");

    for (unsigned i = 0; i < pimpl->userTempFields.size(); ++i)
        http_str += pimpl->userTempFields[i] + ULXR_CHAR("\r\n");
    pimpl->userTempFields.clear();

    if (hasServerCookie())
      http_str += ULXR_PCHAR("Set-Cookie: ") + getServerCookie() + ULXR_PCHAR("\r\n");

    if (isChunkedTransfer())
    {
      http_str += ULXR_PCHAR("Transfer-Encoding: chunked\r\n");
#ifdef ULXR_DEBUG_OUTPUT
      http_str += ULXR_PCHAR("X-Content-Length: ") + ULXR_GET_STRING(contlen) + ULXR_PCHAR("\r\n");
#endif
    }
    else
      http_str += ULXR_PCHAR("Content-Length: ") + ULXR_GET_STRING(contlen) + ULXR_PCHAR("\r\n");

    if (!wbxml_mode)
    {
        http_str += ULXR_PCHAR("X-Powered-By: ") + getUserAgent() + ULXR_PCHAR("\r\n")
                    + ULXR_PCHAR("Server: ") + pimpl->hostname + ULXR_PCHAR("\r\n")
                    + ULXR_PCHAR("Date: ") + getDateStr() + ULXR_PCHAR("\r\n");
    }

    http_str += ULXR_PCHAR("\r\n");    // empty line at end of header

    ULXR_DOUT_HTTP(ULXR_PCHAR("resp: \n") << http_str.c_str());

#ifdef ULXR_UNICODE
    Cpp8BitString utf = unicodeToUtf8(http_str);
    writeRaw(utf.data(), utf.length());
#else
    writeRaw(http_str.data(), http_str.length());
#endif
}


ULXR_API_IMPL(void) HttpProtocol::enableConnect(bool enable)
{
  ULXR_TRACE(ULXR_PCHAR("enableConnect ") << enable);
  pimpl->useconnect = enable;
}


ULXR_API_IMPL(bool) HttpProtocol::isConnectEnabled() const
{
  ULXR_TRACE(ULXR_PCHAR("isConnectEnabled ") << pimpl->useconnect);
  return pimpl->useconnect;
}


ULXR_API_IMPL(bool) HttpProtocol::isConnected() const
{
  ULXR_TRACE(ULXR_PCHAR("ispimpl->connected ") << pimpl->connected);
  return pimpl->connected;
}


ULXR_API_IMPL(void) HttpProtocol::awaitConnect()
{
    ULXR_TRACE(ULXR_PCHAR("awaitConnect"));

    char buffer[ULXR_RECV_BUFFER_SIZE];
    char *buff_ptr;
    bool done = false;
    long readed;
    while (!done && hasBytesToRead()
                && ((readed = readRaw(buffer, sizeof(buffer))) > 0) )
    {
      buff_ptr = buffer;
      ULXR_TRACE(ULXR_PCHAR("loop"));

      if (readed > 0)
      {
        State state = connectionMachine(buff_ptr, readed);
        if (state == ConnError)
        {
          ULXR_TRACE(ULXR_PCHAR("ConnError"));
          done = true;
          throw ConnectionException(TransportError, ulxr_i18n(ULXR_PCHAR("network problem occured")), 400);
        }

        else if (state == ConnSwitchToBody)
        {
          ULXR_TRACE(ULXR_PCHAR("ConnSwitchToBody"));
          done = true;
        }

        else if (state == ConnBody)
        {
          ULXR_TRACE(ULXR_PCHAR("ConnBody"));
          done = true;
        }
      }
    }

    CppString head_version;
    unsigned head_status = 500;
    CppString head_phrase = ULXR_PCHAR("Internal error");
    splitHeaderLine(head_version, head_status, head_phrase);

    if (head_status != 200)
      throw ConnectionException(TransportError, head_phrase, head_status);

    pimpl->connected = true;
    ULXR_TRACE(ULXR_PCHAR("awaitConnect: pimpl->connected"));
}


ULXR_API_IMPL(void) HttpProtocol::tryConnect()
{
    ULXR_TRACE(ULXR_PCHAR("performConnect"));

    char ports[40];
    ulxr_sprintf(ports, ":%d", pimpl->hostport);
    CppString resource = pimpl->hostname + ULXR_GET_STRING(ports);
    CppString http_str = ULXR_PCHAR("CONNECT ") + resource + ULXR_PCHAR(" HTTP/1.1\r\n");

    http_str += ULXR_PCHAR("User-Agent: ") + getUserAgent() + ULXR_PCHAR("\r\n");
    http_str += ULXR_PCHAR("Proxy-Connection: Keep-Alive\r\n");
    http_str += ULXR_PCHAR("Host: ") + pimpl->hostname + ULXR_PCHAR("\r\n");

    if (pimpl->proxy_user.length() + pimpl->proxy_pass.length() != 0)
       http_str += ULXR_PCHAR("Proxy-Authorization: Basic ")
                   + encodeBase64(pimpl->proxy_user + ULXR_PCHAR(":") + pimpl->proxy_pass);

    http_str += ULXR_PCHAR("\r\n");    // empty line at end of header

    ULXR_DOUT_HTTP(ULXR_PCHAR("connect: \n") << http_str.c_str());

#ifdef ULXR_UNICODE
    Cpp8BitString utf = unicodeToUtf8(http_str);
    writeRaw(utf.data(), utf.length());
#else
    writeRaw(http_str.data(), http_str.length());
#endif
}


ULXR_API_IMPL(void) HttpProtocol::doConnect()
{
    if (isConnectEnabled() && !isConnected())
    {
      resetConnection();
      tryConnect();
      awaitConnect();
      resetConnection();
    }
}


ULXR_API_IMPL(void)
HttpProtocol::sendRequestHeader(const CppString &method,
                                const CppString &in_resource,
                                const CppString &type,
                                unsigned long len,
                                bool wbxml_mode)
{
    doConnect();
    pimpl->bChunkedEncoding = false;
    ULXR_TRACE(ULXR_PCHAR("sendRequestHeader"));
    char contlen[40];
    ulxr_sprintf(contlen, "%ld", len );

    char ports[40];
    ulxr_sprintf(ports, "%d", pimpl->hostport);
    CppString resource = ULXR_PCHAR("http://") + pimpl->hostname + ULXR_PCHAR(":") + ULXR_GET_STRING(ports) + in_resource;
    CppString http_str = method + ULXR_PCHAR(" ") + resource + ULXR_PCHAR(" HTTP/1.1\r\n");
    http_str += ULXR_PCHAR("Host: ") + pimpl->hostname + ULXR_PCHAR("\r\n");

    if(!wbxml_mode)
        http_str += ULXR_PCHAR("User-Agent: ") + getUserAgent() + ULXR_PCHAR("\r\n");

    if (pimpl->proxy_user.length() + pimpl->proxy_pass.length() != 0)
       http_str += ULXR_PCHAR("Proxy-Authorization: Basic ")
                   + encodeBase64(pimpl->proxy_user + ULXR_PCHAR(":") + pimpl->proxy_pass);

    if (!isPersistent())
        http_str += ULXR_PCHAR("Connection: Close\r\n");
    else
        http_str += ULXR_PCHAR("Proxy-Connection: Keep-Alive\r\n");

    if (len != 0 && type.length() != 0)
        http_str += ULXR_PCHAR("Content-Type: ") + type + ULXR_PCHAR("\r\n");

    for (unsigned i = 0; i < pimpl->userTempFields.size(); ++i)
        http_str += pimpl->userTempFields[i] + ULXR_CHAR("\r\n");
    pimpl->userTempFields.clear();

    if(!wbxml_mode)
      http_str += ULXR_PCHAR("Date: ") + getDateStr() + ULXR_PCHAR("\r\n");

    if (isChunkedTransfer())
    {
      http_str += ULXR_PCHAR("Transfer-Encoding: chunked\r\n");
#ifdef ULXR_DEBUG_OUTPUT
      http_str += ULXR_PCHAR("X-Content-Length: ") + ULXR_GET_STRING(contlen) + ULXR_PCHAR("\r\n");
#endif
    }
    else
      http_str += ULXR_PCHAR("Content-Length: ") + ULXR_GET_STRING(contlen) + ULXR_PCHAR("\r\n");

    if (hasClientCookie())
      http_str += ULXR_PCHAR("Cookie: ") + getClientCookie() + ULXR_PCHAR("\r\n");

    http_str += ULXR_PCHAR("\r\n");    // empty line at end of header

    ULXR_DOUT_HTTP(ULXR_PCHAR("req: \n") << http_str.c_str());

#ifdef ULXR_UNICODE
    Cpp8BitString utf = unicodeToUtf8(http_str);
    writeRaw(utf.data(), utf.length());
#else
    writeRaw(http_str.data(), http_str.length());
#endif
};


ULXR_API_IMPL(CppString) HttpProtocol::getDateStr()
{
    ULXR_TRACE(ULXR_PCHAR("getDateStr"));
    time_t tm;
    time(&tm);

#ifndef HAVE_CTIME_R

#ifndef ULXR_OMIT_REENTRANT_PROTECTOR   // todo: optionally replace with ctime_r
    Mutex::Locker lock(ctimeMutex);
#endif

    char * ct = ulxr_ctime(&tm);
    CppString s = ULXR_GET_STRING(ct);  // "\n" already included!

#else

    char buff[40];
    char * ct = ::ctime_r(&tm, buff);
    CppString s = ULXR_GET_STRING(ct);  // "\n" already included!

#endif

    s.erase(s.length()-1);              // remove it
    return s;
}


ULXR_API_IMPL(void)
HttpProtocol::sendNegativeResponse(int status,
                                   const CppString &phrase,
                                   const CppString &info)
{
    ULXR_TRACE(ULXR_PCHAR("sendNegativeResponse"));

    // doConnect(); must already be pimpl->connected

    char stat[40];
    ulxr_sprintf(stat, "%d", status );

    CppString msg = ulxr_i18n(ULXR_PCHAR("<html>")
                         ULXR_PCHAR("<head><title>Error occured</title></head>")
                         ULXR_PCHAR("<body>")
                         ULXR_PCHAR("<b>Sorry, error occured: ")) + ULXR_GET_STRING(stat)
                    + ULXR_PCHAR(", ") + phrase;

    if (info.length() != 0)
        msg += ULXR_PCHAR("<br />") + info;

    msg += ulxr_i18n(ULXR_PCHAR("</b>")
                ULXR_PCHAR("<hr /><p>")
                ULXR_PCHAR("This cute little server is powered by")
                ULXR_PCHAR(" <a href=\"http://ulxmlrpcpp.sourceforge.net\">"));

    msg += ULXR_GET_STRING(ULXR_PACKAGE)
           + ULXR_PCHAR("/v") + ULXR_GET_STRING(ULXR_VERSION)
           + ULXR_PCHAR("</a>")
           + ULXR_PCHAR("</p>")
           ULXR_PCHAR("</body>")
           ULXR_PCHAR("</html>");

    ULXR_DOUT_RESP(ULXR_PCHAR("msg:\n") << msg);

#ifdef ULXR_UNICODE
    Cpp8BitString utf = unicodeToUtf8(msg);
    sendResponseHeader(status, phrase, ULXR_PCHAR("text/html"), utf.length());
    writeRaw(utf.data(), utf.length());
#else
    sendResponseHeader(status, phrase, ULXR_PCHAR("text/html"), msg.length());
    writeRaw(msg.data(), msg.length());
#endif
}


ULXR_API_IMPL(void) HttpProtocol::sendRpcResponse(const MethodResponse &resp, bool wbxml_mode)
{
    ULXR_TRACE(ULXR_PCHAR("sendRpcResponse"));

    // doConnect(); must already be pimpl->connected

    if (wbxml_mode)
    {
        std::string xml = resp.getWbXml();
        ULXR_DOUT_XML(binaryDebugOutput(xml));
        sendResponseHeader(200, ULXR_PCHAR("OK"), ULXR_PCHAR("application/x-wbxml-ulxr"), xml.length(), wbxml_mode);
        writeBody(xml.data(), xml.length());
    }
    else
    {
        CppString xml = resp.getXml(0)+ULXR_PCHAR("\n");
        ULXR_DOUT_XML(xml);

#ifdef ULXR_UNICODE
        Cpp8BitString utf = unicodeToUtf8(xml);
        sendResponseHeader(200, ULXR_PCHAR("OK"), ULXR_PCHAR("text/xml"), utf.length(), wbxml_mode);
        writeBody(utf.data(), utf.length());
#else
        sendResponseHeader(200, ULXR_PCHAR("OK"), ULXR_PCHAR("text/xml"), xml.length(), wbxml_mode);
        writeBody(xml.data(), xml.length());
#endif

    }
}

ULXR_API_IMPL(void) HttpProtocol::writeChunk(const char *data, unsigned long len)
{
  if (!isChunkedTransfer())
    throw ConnectionException(NotConformingError,
                              ulxr_i18n(ULXR_PCHAR("Protocol is not prepared for chunked encoding: ")), 400);

  if (len != 0)
  {
    char stat[40];
    ulxr_sprintf(stat, "%lx", len);
    ULXR_TRACE(ULXR_PCHAR("HttpProtocol::writeChunk() chunk with 0x")
               << ULXR_GET_STRING(stat)
               << ULXR_PCHAR(" bytes"));
    writeRaw(stat, strlen(stat));
    writeRaw("\r\n", 2);
    writeRaw(data, len);
    writeRaw("\r\n", 2);
  }
  else
  {
    ULXR_TRACE(ULXR_PCHAR("HttpProtocol::writeChunk() last chunk with 0 bytes"));
    writeRaw("0\r\n\r\n", 5);  // terminator
  }
}


ULXR_API_IMPL(void) HttpProtocol::writeBody(const char *data, unsigned long len)
{
  if (!isChunkedTransfer())
    writeRaw(data, len);
  else
  {
    writeChunk(data, len);
    writeChunk(data, 0);
  }
}


ULXR_API_IMPL(void) HttpProtocol::sendRpcCall(const MethodCall &call,
                                           const CppString &resource,
                                           bool wbxml_mode)
{
    ULXR_TRACE(ULXR_PCHAR("sendRpcCall"));
    doConnect();

    if (wbxml_mode)
    {
        std::string xml = call.getWbXml();
        ULXR_DOUT_XML(binaryDebugOutput(xml));
        sendRequestHeader(ULXR_PCHAR("POST"), resource, ULXR_PCHAR("application/x-wbxml-ulxr"), xml.length(), wbxml_mode);
        writeBody(xml.data(), xml.length());
    }
    else
    {
        CppString xml = call.getXml(0)+ULXR_PCHAR("\n");
        ULXR_DOUT_XML(xml);

#ifdef ULXR_UNICODE
        Cpp8BitString utf = unicodeToUtf8(xml);
        sendRequestHeader(ULXR_PCHAR("POST"), resource, ULXR_PCHAR("text/xml"), utf.length(), wbxml_mode);
        writeBody(utf.data(), utf.length());
#else
        sendRequestHeader(ULXR_PCHAR("POST"), resource, ULXR_PCHAR("text/xml"), xml.length(), wbxml_mode);
        writeBody(xml.data(), xml.length());
#endif

    }
};


ULXR_API_IMPL(bool) HttpProtocol::responseStatus(CppString &phrase) const
{
    ULXR_TRACE(ULXR_PCHAR("responseStatus"));

    CppString s = stripWS(getFirstHeaderLine());
    if (s.length() == 0)
    {
        s = ulxr_i18n(ULXR_PCHAR("No connection status available"));
        return false;
    }

    std::size_t pos = s.find(' ');
    if (pos != CppString::npos)  // skip version
        s.erase(0, pos+1);
    else
        s = ULXR_PCHAR("");

    CppString stat;
    s = stripWS(s);
    pos = s.find(ULXR_CHAR(' '));
    if (pos != CppString::npos)
    {
        stat = s.substr(0, pos);
        s.erase(0, pos+1);
    }
    else
    {
        stat = s;
        s = ULXR_PCHAR("");
    }

    phrase = stripWS(s);

    return stat == ULXR_PCHAR("200");
}


ULXR_API_IMPL(bool) HttpProtocol::determineClosing(const CppString &http_ver)
{
    ULXR_TRACE(ULXR_PCHAR("determineClosing"));
    if (   http_ver == ULXR_PCHAR("0.9")
            || http_ver == ULXR_PCHAR("1.0"))
    {
        if (hasHttpProperty(ULXR_PCHAR("connection")))
        {
            CppString s = getHttpProperty(ULXR_PCHAR("connection"));
            makeLower(s);
            return !(s == ULXR_PCHAR("keep-alive"));
        }
        ULXR_TRACE(ULXR_PCHAR("determineClosing: true"));
        return true;  // close by default
    }
    else  // 1.1 and later
    {
        return hasClosingProperty();
/*
        ULXR_TRACE(ULXR_PCHAR("determineClosing: false"));
        return false;  // keep open by default
*/
    }
    /*return true; avoids warning */
}


ULXR_API_IMPL(bool) HttpProtocol::getUserPass(CppString &user,
        CppString &pass) const
{
    ULXR_TRACE(ULXR_PCHAR("getUserPass"));
    user = ULXR_PCHAR("");
    pass = ULXR_PCHAR("");

    if (hasHttpProperty(ULXR_PCHAR("authorization")) )
    {
        CppString auth = getHttpProperty(ULXR_PCHAR("authorization"));

        ULXR_TRACE(ULXR_PCHAR("getUserPass: ") + auth);
        ULXR_TRACE(ULXR_PCHAR("getUserPass: basic?"));

        // only know basic auth
        CppString auth_id = auth.substr(0, 6);
        makeLower(auth_id);
        if (auth_id != ULXR_PCHAR("basic "))
            return false;

        auth.erase(0, 6);
        auth = decodeBase64(auth);
        ULXR_TRACE(ULXR_PCHAR("getUserPass: ':'? ")  + auth);
        std::size_t pos = auth.find(':');
        if (pos != CppString::npos)
        {
            user = stripWS(auth.substr(0, pos));
            pass = stripWS(auth.substr(pos+1));
            ULXR_TRACE(ULXR_PCHAR("getUserPass: user=") +user + ULXR_PCHAR(", pass=")+pass);
            return true;
        }
    }

    return false;
}


ULXR_API_IMPL(void) HttpProtocol::rejectAuthentication(const CppString &realm)
{
    ULXR_TRACE(ULXR_PCHAR("rejectAuthentication: ") + realm);
    addOneTimeHttpField(ULXR_PCHAR("WWW-Authenticate"),
                        ULXR_PCHAR("Basic realm=\"") + realm +ULXR_PCHAR("\""));
    sendNegativeResponse(401, ULXR_PCHAR("Authentication required for realm \"")+ realm + ULXR_PCHAR("\""));
}


ULXR_API_IMPL(void) HttpProtocol::addOneTimeHttpField(const CppString &name, const CppString &value)
{
    ULXR_TRACE(ULXR_PCHAR("addOneTimeHttpField: ") + name + ULXR_PCHAR(": ") + value);
    pimpl->userTempFields.push_back(stripWS(name) + ULXR_PCHAR(": ") + stripWS(value));
}


ULXR_API_IMPL(void) HttpProtocol::setMessageAuthentication(const CppString &user,
                                                        const CppString &pass)
{
    ULXR_TRACE(ULXR_PCHAR("setMessageAuthentication"));
    CppString s = ULXR_PCHAR("Basic ");
    s += encodeBase64(user + ULXR_PCHAR(":") + pass);
    addOneTimeHttpField(ULXR_PCHAR("Authorization"), s);
}


ULXR_API_IMPL(void) HttpProtocol::setProxyAuthentication(const CppString &user,
                                                      const CppString &pass)
{
    ULXR_TRACE(ULXR_PCHAR("setProxyAuthentication"));
    pimpl->proxy_user = user;
    pimpl->proxy_pass = pass;
}


ULXR_API_IMPL(void) HttpProtocol::setTransmitOnly()
{
    ULXR_TRACE(ULXR_PCHAR("setTransmitOnly"));
    addOneTimeHttpField(ULXR_PCHAR("X-TransmitOnly"), ULXR_PCHAR("true"));
}


ULXR_API_IMPL(bool) HttpProtocol::isTransmitOnly()
{
    ULXR_TRACE(ULXR_PCHAR("isTransmitOnly"));
    return hasHttpProperty(ULXR_PCHAR("X-TransmitOnly"))
           && (getHttpProperty(ULXR_PCHAR("X-TransmitOnly")) == ULXR_PCHAR("true"));
}


ULXR_API_IMPL(CppString) HttpProtocol::getProtocolName()
{
    return ULXR_PCHAR("http");
}


bool HttpProtocol::hasCookie() const
{
  bool b = !pimpl->cookies.empty();
  ULXR_TRACE(ULXR_PCHAR("hasCookie: ") << b);
  return b;
}


void HttpProtocol::setCookie(const CppString &in_cont)
{
  ULXR_TRACE(ULXR_PCHAR("setCookie: ") << in_cont);
  CppString cont = in_cont;
  std::size_t uEnd = cont.find(';');
  while (uEnd != CppString::npos)
  {
    CppString sKV = cont.substr(0, uEnd);
    cont.erase(0, uEnd+1);
    std::size_t uEq = sKV.find('=');
    if (uEq != CppString::npos)
    {
        CppString sKey = stripWS(sKV.substr(0, uEq));
        CppString sVal = stripWS(sKV.substr(uEq+1));
        ULXR_TRACE(ULXR_PCHAR("setCookie: ") << sKey << ULXR_PCHAR(" ") << sVal);
        pimpl->cookies[sKey] = sVal;
    }
    uEnd = cont.find(';');
  }

  std::size_t uEq = cont.find('=');
  if (uEq != CppString::npos)
  {
      CppString sKey = stripWS(cont.substr(0, uEq));
      CppString sVal = stripWS(cont.substr(uEq+1));
      ULXR_TRACE(ULXR_PCHAR("setCookie: ") << sKey << ULXR_PCHAR(" ") << sVal);
      pimpl->cookies[sKey] = sVal;
  }
}


CppString HttpProtocol::getCookie() const
{
  CppString ret;
  for (std::map<CppString, CppString>::const_iterator iCookie = pimpl->cookies.begin();
        iCookie != pimpl->cookies.end();
        ++iCookie)
  {
      if (iCookie != pimpl->cookies.begin())
          ret += ULXR_PCHAR("; ");
      ret += (*iCookie).first + ULXR_PCHAR("=") + (*iCookie).second;
  }
  ULXR_TRACE(ULXR_PCHAR("getCookie: ") << ret);
  return ret;
}


ULXR_API_IMPL(void) HttpProtocol::setAcceptCookies(bool bAccept)
{
  ULXR_TRACE(ULXR_PCHAR("setAcceptCookies: ") << bAccept);
  pimpl->bAcceptcookies = bAccept;
}


ULXR_API_IMPL(bool) HttpProtocol::isAcceptCookies() const
{
  ULXR_TRACE(ULXR_PCHAR("isAcceptCookies: ") << pimpl->bAcceptcookies);
  return pimpl->bAcceptcookies;
}


ULXR_API_IMPL(void) HttpProtocol::setServerCookie(const CppString &cookie)
{
  ULXR_TRACE(ULXR_PCHAR("setServerCookie: ") << cookie);
  pimpl->serverCookie = cookie;
}


ULXR_API_IMPL(CppString) HttpProtocol::getServerCookie() const
{
  ULXR_TRACE(ULXR_PCHAR("getServerCookie: ") << pimpl->serverCookie);
  return pimpl->serverCookie;
}


ULXR_API_IMPL(bool) HttpProtocol::hasServerCookie() const
{
  bool b = pimpl->serverCookie.length() != 0;
  ULXR_TRACE(ULXR_PCHAR("hasServerCookie: ") << b);
  return b;
}


ULXR_API_IMPL(void) HttpProtocol::setClientCookie(const CppString &cookie)
{
  ULXR_TRACE(ULXR_PCHAR("setClientCookie: ") << cookie);
  pimpl->clientCookie = cookie;
}


ULXR_API_IMPL(CppString) HttpProtocol::getClientCookie() const
{
  ULXR_TRACE(ULXR_PCHAR("getClientCookie: ") << pimpl->clientCookie);
  return pimpl->clientCookie;
}


ULXR_API_IMPL(bool) HttpProtocol::hasClientCookie() const
{
  bool b = pimpl->clientCookie.length() != 0;
  ULXR_TRACE(ULXR_PCHAR("hasClientCookie: ") << b);
  return b;
}


ULXR_API_IMPL(void) HttpProtocol::setUserAgent(const CppString &ua)
{
  pimpl->useragent = ua;
}


ULXR_API_IMPL(CppString) HttpProtocol::getUserAgent() const
{
  return pimpl->useragent;
}


ULXR_API_IMPL(CppString) HttpProtocol::getFirstHeaderLine() const
{
  return pimpl->header_firstline;
}


ULXR_API_IMPL(void)
  HttpProtocol::splitHeaderLine(CppString &head_version, unsigned  &head_status, CppString &head_phrase)
{
   head_version = ULXR_PCHAR("");
   head_status = 500;
   head_phrase = ULXR_PCHAR("Internal error");

   CppString s = stripWS(getFirstHeaderLine());
   std::size_t pos = s.find(' ');
   if (pos != CppString::npos)
   {
     head_version = s.substr(0, pos);
     s.erase(0, pos+1);
   }
   else
   {
     head_version = s;
     s = ULXR_PCHAR("");
   }
   pos = head_version.find('/');
   if (pos != CppString::npos)
     head_version.erase(0, pos+1);

   CppString stat;
   s = stripWS(s);
   pos = s.find(' ');
   if (pos != CppString::npos)
   {
     stat = s.substr(0, pos);
     s.erase(0, pos+1);
   }
   else
   {
     stat = s;
     s = ULXR_PCHAR("");
   }
   head_status = ulxr_atoi(getLatin1(stat).c_str());

   s = stripWS(s);
   head_phrase = s;
}


ULXR_API_IMPL(void) HttpProtocol::setChunkedTransfer(bool chunked)
{
  ULXR_TRACE(ULXR_PCHAR("setChunkedTransfer() ") << chunked);
  pimpl->chunked_block = chunked;
}


ULXR_API_IMPL(bool) HttpProtocol::isChunkedTransfer() const
{
  ULXR_TRACE(ULXR_PCHAR("isChunkedTransfer() ") << pimpl->chunked_block);
  return pimpl->chunked_block;
}


}  // namespace ulxr

