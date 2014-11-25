/***************************************************************************
               ulxr_tcpip_connection.cpp  -  tcpip connection
                             -------------------
    begin                : Mon Apr 29 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_tcpip_connection.cpp 1151 2009-08-12 15:12:01Z ewald-arnold $

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
// #define ULXR_SHOW_READ
// #define ULXR_SHOW_WRITE

#define ULXR_REUSE_SOCKET

#ifndef ULXR_OMIT_TCP_STUFF

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <cstring>
#include <cerrno>

#ifdef __SUN__
#include <sys/systeminfo.h>
#endif

#ifdef __unix__
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#if defined(ULXR_USE_EPOLL) && defined(HAVE_SYS_EPOLL_H)
#include <sys/epoll.h>
#endif

#endif

//#include <iostream>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h>
#include <ulxmlrpcpp/ulxr_htmlform_handler.h>
#include <ulxmlrpcpp/ulxr_except.h>

#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
#include <ulxmlrpcpp/ulxr_mutex.h>
#endif


namespace ulxr {


ULXR_API_IMPL0 TcpIpConnection::ServerSocketData::ServerSocketData(int s_no)
  : count(1)
  , socket_no(s_no)
{
}


ULXR_API_IMPL0 TcpIpConnection::ServerSocketData::~ServerSocketData()
{
  close();
}


ULXR_API_IMPL(int) TcpIpConnection::ServerSocketData::getSocket() const
{
  return socket_no;
}


ULXR_API_IMPL(void) TcpIpConnection::ServerSocketData::incRef()
{
    ++count;
}


ULXR_API_IMPL(int) TcpIpConnection::ServerSocketData::decRef()
{
  if (count > 0)
    --count;
  return count;
}


ULXR_API_IMPL(bool) TcpIpConnection::ServerSocketData::isOpen()
{
  return socket_no != -1;
}


ULXR_API_IMPL(void) TcpIpConnection::ServerSocketData::close()
{
  ULXR_TRACE(ULXR_PCHAR("close"));
#ifndef __unix__
  ::closesocket(socket_no);
#else
  int ret;
  do
    ret = ::close(socket_no);
  while(ret < 0 && (errno == EINTR || errno == EAGAIN));

  if(ret < 0)
    throw ConnectionException(TransportError,
                              ULXR_PCHAR("close() failed for TcpIpConnection::ServerSocketData"), 500);
#endif
  socket_no = -1;
}


ULXR_API_IMPL(void) TcpIpConnection::ServerSocketData::shutdown(int in_mode)
{
  ULXR_TRACE(ULXR_PCHAR("shutdown") << in_mode);
  int ret;
  do
    ret = ::shutdown(socket_no, in_mode);
  while(ret < 0 && (errno == EINTR || errno == EAGAIN));

  if(ret < 0)
    throw ConnectionException(TransportError,
                              ULXR_PCHAR("shutdown() failed for TcpIpConnection::ServerSocketData"), 500);
}


//////////////////////////////////////////////////////////////////////////


struct TcpIpConnection::PImpl
{
   CppString           serverdomain;
   unsigned            port;
   ServerSocketData   *server_data;

   CppString           host_name;
   struct sockaddr_in  hostdata;
   socklen_t           hostdata_len;

   CppString           remote_name;
   struct sockaddr_in  remotedata;
   socklen_t           remotedata_len;
};


ULXR_API_IMPL0 TcpIpConnection::TcpIpConnection(const TcpIpConnection &conn)
  : Connection(conn)
  , pimpl(new PImpl)
{
  *pimpl = *conn.pimpl;
}


ULXR_API_IMPL0 TcpIpConnection::TcpIpConnection(bool I_am_server, long adr, unsigned prt)
  : Connection()
  , pimpl(new PImpl)
{
  ULXR_TRACE(ULXR_PCHAR("TcpIpConnection(bool, long, uint)") << adr << ULXR_PCHAR(" ") << pimpl->port);
  init(prt);

  pimpl->hostdata.sin_addr.s_addr = htonl(adr);

  if (I_am_server)
  {
    pimpl->server_data = new ServerSocketData(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (getServerHandle() < 0)
      throw ConnectionException(SystemError,
                                ulxr_i18n(ULXR_PCHAR("Could not create socket: "))
                                     + ULXR_GET_STRING(getErrorString(getLastError())), 500);

#ifdef ULXR_REUSE_SOCKET
    int sockOpt = 1;
    if (::setsockopt(getServerHandle(), SOL_SOCKET, SO_REUSEADDR,
                     (const char*)&sockOpt, sizeof(sockOpt)) < 0)
      throw ConnectionException(SystemError,
                                      ulxr_i18n(ULXR_PCHAR("Could not set reuse flag for socket: "))
                                     + ULXR_GET_STRING(getErrorString(getLastError())), 500);
#endif

    int iOptVal = getTimeout() * 1000;
    int iOptLen = sizeof(int);
    ::setsockopt(getServerHandle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&iOptVal, iOptLen);
    ::setsockopt(getServerHandle(), SOL_SOCKET, SO_SNDTIMEO, (char*)&iOptVal, iOptLen);

    if((::bind(getServerHandle(), (sockaddr*) &pimpl->hostdata, sizeof(pimpl->hostdata))) < 0)
      throw ConnectionException(SystemError,
                                ulxr_i18n(ULXR_PCHAR("Could not bind adress: "))
                                     + ULXR_GET_STRING(getErrorString(getLastError())), 500);

    ::listen(getServerHandle(), 5);
  }
}


ULXR_API_IMPL0
  TcpIpConnection::TcpIpConnection(bool I_am_server, const CppString &dom, unsigned prt)
  : Connection()
  , pimpl(new PImpl)
{
  ULXR_TRACE(ULXR_PCHAR("TcpIpConnection(bool, string, uint)") << dom << ULXR_PCHAR(" ") << pimpl->port);
  init(prt);

  pimpl->remote_name = dom;

  struct hostent *hp = getHostAdress(dom);
  if (hp == 0)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Host adress not found: ")) + pimpl->serverdomain, 500);
  memcpy(&(pimpl->hostdata.sin_addr), hp->h_addr_list[0], hp->h_length);

  if (I_am_server)
  {
    pimpl->server_data = new ServerSocketData(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (getServerHandle() < 0)
      throw ConnectionException(SystemError,
                                ulxr_i18n(ULXR_PCHAR("Could not create socket: "))
                                     + ULXR_GET_STRING(getErrorString(getLastError())), 500);
#ifdef ULXR_REUSE_SOCKET
    int sockOpt = 1;
    if (::setsockopt(getServerHandle(), SOL_SOCKET, SO_REUSEADDR,
                     (const char*)&sockOpt, sizeof(sockOpt)) < 0)
      throw ConnectionException(SystemError,
                                ulxr_i18n(ULXR_PCHAR("Could not set reuse flag for socket: "))
                                     + ULXR_GET_STRING(getErrorString(getLastError())), 500);
#endif

    int iOptVal = getTimeout() * 1000;
    int iOptLen = sizeof(int);
    ::setsockopt(getServerHandle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&iOptVal, iOptLen);
    ::setsockopt(getServerHandle(), SOL_SOCKET, SO_SNDTIMEO, (char*)&iOptVal, iOptLen);

    if((::bind(getServerHandle(), (sockaddr*) &pimpl->hostdata, sizeof(pimpl->hostdata))) < 0)
      throw ConnectionException(SystemError,
                                ulxr_i18n(ULXR_PCHAR("Could not bind adress: "))
                                     + ULXR_GET_STRING(getErrorString(getLastError())), 500);

    listen(getServerHandle(), 5);
  }
}


ULXR_API_IMPL(TcpIpConnection *) TcpIpConnection::makeClone()
{
  return new TcpIpConnection(*this); // shallow copy !!
}


ULXR_API_IMPL(Connection *) TcpIpConnection::detach()
{
  ULXR_TRACE(ULXR_PCHAR("detach"));
  TcpIpConnection *clone = makeClone();;
  clone->pimpl->server_data = 0;

  clone->setServerData(getServerData());
  if (getServerData() != 0)
    getServerData()->incRef();

  ULXR_TRACE(ULXR_PCHAR("detach getHandle() ") << getHandle());
  ULXR_TRACE(ULXR_PCHAR("detach clone->getHandle() ") << clone->getHandle());
  cut();
  ULXR_TRACE(ULXR_PCHAR("detach getHandle() ") << getHandle());
  ULXR_TRACE(ULXR_PCHAR("detach clone->getHandle() ") << clone->getHandle());

  ULXR_TRACE(ULXR_PCHAR("/detach"));
  return clone;
}


ULXR_API_IMPL(void) TcpIpConnection::setProxy(long adr, unsigned port)
{
  ULXR_TRACE(ULXR_PCHAR("setProxy ") << adr << ULXR_PCHAR(" ") << port);
  pimpl->hostdata.sin_addr.s_addr = htonl(adr);
  pimpl->hostdata.sin_port = htons(port);
}


ULXR_API_IMPL(void) TcpIpConnection::setProxy(const CppString &dom, unsigned port)
{
  ULXR_TRACE(ULXR_PCHAR("setProxy ") << dom << ULXR_PCHAR(" ") << port);
  struct hostent *hp = getHostAdress(dom);
  if (hp == 0)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Host adress for proxy not found: ")) + dom, 500);
  memcpy(&(pimpl->hostdata.sin_addr), hp->h_addr_list[0], hp->h_length);
  pimpl->hostdata.sin_port = htons(port);
}


ULXR_API_IMPL(void) TcpIpConnection::init(unsigned prt)
{
  ULXR_TRACE(ULXR_PCHAR("init"));
#if defined(__WIN32__)  && !defined (ULXR_NO_WSA_STARTUP)
  WORD wVersionRequested;
  WSADATA wsaData;
  wVersionRequested = MAKEWORD( 2, 0 );

  if (WSAStartup( wVersionRequested, &wsaData) != 0)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Could not initialize Windows sockets: "))
                                + ULXR_GET_STRING(getErrorString(getLastError())), 500);
#endif

  pimpl->server_data = 0;
  setTcpNoDelay(false);
  pimpl->serverdomain = ULXR_PCHAR("");
  pimpl->remote_name = ULXR_PCHAR("");
  setTimeout(10);
  pimpl->port = prt;
  pimpl->hostdata_len = sizeof(pimpl->hostdata);
  pimpl->remotedata_len = sizeof(pimpl->remotedata);
  memset(&pimpl->hostdata, 0, sizeof(pimpl->hostdata));
  memset(&pimpl->remotedata, 0, sizeof(pimpl->remotedata));
  pimpl->hostdata.sin_port = htons(pimpl->port);
  pimpl->hostdata.sin_family = AF_INET;

  char buffer [1000];
  memset(buffer, 0, sizeof(buffer));
  int ret = gethostname(buffer, sizeof(buffer)-1);
  if (ret != 0)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Could not get host name: "))
                                   + ULXR_GET_STRING(getErrorString(getLastError())), 500);

  pimpl->host_name = ULXR_GET_STRING(buffer);

#if defined(__SUN__)

  long status = sysinfo(SI_SRPC_DOMAIN ,buffer, sizeof(buffer)-1);
  if (status == -1)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Could not get domain name: "))
                                   + ULXR_GET_STRING(getErrorString(getLastError())), 500);

  if (buffer[0] != 0)
  {
    pimpl->host_name += ULXR_PCHAR(".");
    pimpl->host_name += ULXR_GET_STRING(buffer);
  }

#elif defined(__unix__) || defined(__CYGWIN__)

  ret = getdomainname(buffer, sizeof(buffer)-1);
  if (ret != 0)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Could not get domain name: "))
                                   + ULXR_GET_STRING(getErrorString(getLastError())), 500);

  if (buffer[0] != 0)
  {
    pimpl->host_name += ULXR_PCHAR(".");
    pimpl->host_name += ULXR_GET_STRING(buffer);
  }

#elif _WIN32

#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
  Mutex::Locker lock(gethostbynameMutex);
#endif

  struct hostent *hostEntPtr = gethostbyname(getLatin1(pimpl->host_name).c_str());
  if (!hostEntPtr)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Could not get host+domain name: "))
                                   + ULXR_GET_STRING(getErrorString(getLastError())), 500);
  pimpl->host_name = ULXR_GET_STRING(hostEntPtr->h_name);

#else
# pragma message ("don't know how to determine the domain name")
#endif
}


ULXR_API_IMPL(void) TcpIpConnection::asciiToInAddr(const char *address, struct in_addr &saddr)
{
  memset (&saddr, 0, sizeof(in_addr));
  struct hostent *host;

  /* First try it as aaa.bbb.ccc.ddd. */
  saddr.s_addr = inet_addr(address);
  if ((int)saddr.s_addr == -1)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Could not perform inet_addr(): "))
                                   + ULXR_GET_STRING(getErrorString(getLastError())), 500);

#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
  Mutex::Locker lock(gethostbynameMutex);
#endif

  host = gethostbyname(address);
  if (host == 0)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Could not perform gethostbyname(): "))
                                   + ULXR_GET_STRING(getErrorString(getLastError())), 500);

  memmove((void*)&saddr, host->h_addr_list, sizeof(in_addr));
}


ULXR_API_IMPL(struct hostent *)
  TcpIpConnection::getHostAdress(const CppString &dom)
{
  unsigned start = 0;
  if (dom.substr(start, 5) == ULXR_PCHAR("http:"))
    start += 5;

  if (dom.substr(start, 2) == ULXR_PCHAR("//"))
    start += 2;

  std::size_t slash = dom.find (ULXR_PCHAR("/"), start);
  if (slash != CppString::npos)
    pimpl->serverdomain = dom.substr(start, slash-1);
  else
    pimpl->serverdomain = dom;

#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
  Mutex::Locker lock(gethostbynameMutex);
#endif

  return gethostbyname(getLatin1(pimpl->serverdomain).c_str() );
}


ULXR_API_IMPL0 TcpIpConnection::~TcpIpConnection()
{
  ULXR_TRACE(ULXR_PCHAR("~TcpIpConnection"));
  try
  {
    decrementServerRef();
  }
  catch(...)
  {
    // forget exception?
  }
  delete pimpl;
  pimpl = 0;
}


void TcpIpConnection::decrementServerRef(bool in_shutdown)
{
  ULXR_TRACE(ULXR_PCHAR("decrementServerRef"));

  if (pimpl->server_data != 0 && pimpl->server_data->decRef() <= 0)
  {
    ULXR_TRACE(ULXR_PCHAR("delete serverdata ") << std::hex << (void*)pimpl->server_data << std::dec);

    if(in_shutdown)
    {
      ULXR_TRACE(ULXR_PCHAR("shutdown server_data"));
      if (pimpl->server_data->isOpen())
#ifdef __WIN32__
        pimpl->server_data->shutdown(SD_BOTH);
#else
        pimpl->server_data->shutdown(SHUT_RD);
#endif
    }

    delete pimpl->server_data;
    pimpl->server_data = 0;
  }
}


ULXR_API_IMPL(bool) TcpIpConnection::isServerMode() const
{
  return pimpl->server_data != 0;
}


ULXR_API_IMPL(void) TcpIpConnection::open()
{
  ULXR_TRACE(ULXR_PCHAR("open"));
  if (isOpen() )
    throw RuntimeException(ApplicationError,
                           ulxr_i18n(ULXR_PCHAR("Attempt to open an already open connection")));

  if (pimpl->server_data != 0)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Connection is NOT prepared for client mode")), 500);
//  resetConnection();

  setHandle(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
  if (getHandle() < 0)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Could not create socket: "))
                                   + ULXR_GET_STRING(getErrorString(getLastError())), 500);

  int iOptVal = getTimeout() * 1000;
  int iOptLen = sizeof(int);
  ::setsockopt(getHandle(), SOL_SOCKET, SO_RCVTIMEO, (char*)&iOptVal, iOptLen);
  ::setsockopt(getHandle(), SOL_SOCKET, SO_SNDTIMEO, (char*)&iOptVal, iOptLen);
  doTcpNoDelay();

  if(connect(getHandle(), (struct sockaddr *)&pimpl->hostdata, sizeof(pimpl->hostdata)) < 0)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Could not connect: "))
                                   + ULXR_GET_STRING(getErrorString(getLastError())), 500);

  ULXR_TRACE(ULXR_PCHAR("/open.peername"));
#ifdef ULXR_ENABLE_GET_PEERNAME
  pimpl->remotedata_len = sizeof(pimpl->remotedata);
  if(getpeername(getHandle(),
                 (struct sockaddr *)&pimpl->remotedata,
                 &pimpl->remotedata_len)<0)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Could not get peer data: "))
                                   + ULXR_GET_STRING(getErrorString(getLastError())), 500);
#ifdef __BORLANDC__
  pimpl->remote_name = ULXR_PCHAR("<remote-host>");  // FIXME, not working
  host = 0;
  host;
#else
  else
  {
    ULXR_TRACE(ULXR_PCHAR("/open.hostby ") << ULXR_GET_STRING(inet_ntoa(pimpl->remotedata.sin_addr))
               << ULXR_PCHAR(":") << HtmlFormHandler::makeNumber(ntohs(pimpl->remotedata.sin_port)));

#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
  Mutex::Locker lock(gethostbyaddrMutex);
#endif

    struct hostent *host = 0;
#ifdef ULXR_ENABLE_DNS_LOOKUP
    host = gethostbyaddr((char*)&pimpl->remotedata.sin_addr,
                         sizeof(pimpl->remotedata.sin_addr),
                         AF_INET);
#endif

    if (0 == host)
    {
      pimpl->remote_name = ULXR_GET_STRING(inet_ntoa(pimpl->remotedata.sin_addr))
                           + ULXR_GET_STRING(":") + HtmlFormHandler::makeNumber(ntohs(pimpl->remotedata.sin_port));
      ULXR_TRACE(ULXR_PCHAR("/open.hostby.if") << pimpl->remote_name);
    }
    else
    {
      pimpl->remote_name = ULXR_GET_STRING(host->h_name);
      ULXR_TRACE(ULXR_PCHAR("/open.hostby.else ") << pimpl->remote_name);
    }
  }
#endif

#else // ULXR_ENABLE_GET_PEERNAME
  pimpl->remote_name = ULXR_GET_STRING("");
#endif

  abortOnClose(true);
  ULXR_TRACE(ULXR_PCHAR("/open"));
}


ULXR_API_IMPL(bool) TcpIpConnection::accept(int in_timeout)
{
  ULXR_TRACE(ULXR_PCHAR("accept"));
  if (isOpen() )
    throw RuntimeException(ApplicationError,
                           ulxr_i18n(ULXR_PCHAR("Attempt to accept an already open connection")));

//  resetConnection();

  if (pimpl->server_data == 0)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Connection is NOT prepared for server mode")), 500);

  pimpl->remotedata_len = sizeof(pimpl->remotedata);

  if (in_timeout != 0)
  {

#if defined(ULXR_USE_EPOLL) && defined(HAVE_SYS_EPOLL_H)

#ifdef __GNUC__
// #warning ("message using epoll_*()")
#endif

    ULXR_TRACE(ULXR_PCHAR("waiting for epoll_*()"));
    /* Epoll file descriptor */
    int epollfd;
    /* epoll register event structure */
    struct epoll_event ev_accept;
    memset(&ev_accept,0,sizeof(struct epoll_event));

    /* Create a epoll structure */
    if((epollfd = ::epoll_create(1/* Just a tip to the kernel */)) != -1)
    {
      /* Prepare the event structure to a dummy use of epoll */
      ev_accept.events = /*EPOLLET |*/ EPOLLIN;
      ev_accept.data.ptr = NULL;
      /* Check-return var  */
      int ctlresult;
      if((ctlresult = ::epoll_ctl(epollfd, EPOLL_CTL_ADD, getServerData()->getSocket(), &ev_accept)) == 0)
      {
        /* epoll receive event structure */
        struct epoll_event ev[1];
        memset(&ev[0],0,sizeof(struct epoll_event));

        int toval;
        if (in_timeout != 0)
          toval = in_timeout*1000;
        else
          toval = -1;

        int waitresult;
        if((waitresult = epoll_wait(epollfd,ev,1, toval)) == -1)
        {
          ::close(epollfd);
          throw ConnectionException(SystemError, ulxr_i18n(ULXR_PCHAR("Could not wait for the connection (epoll_wait() error):")), 500);
        }

        else
        {
          ::close(epollfd);
          if(waitresult == 0)
          { /* Timeout */
            return false;
          }

          else
          {
            if(waitresult == 1)
            {
              /* It is possible accept */
              /* Just keep running the following code outside the epoll test block... Do nothing here... */
            }

            else
            {
              /* Should not reach here */
              throw ConnectionException(SystemError,ulxr_i18n(ULXR_PCHAR("Problem while attempting to accept by epoll.")), 500);
            }
          }
        }

      }

      else
      {
        ::close(epollfd);
        throw ConnectionException(SystemError,ulxr_i18n(ULXR_PCHAR("Could not perform epoll_ctl() call: ")), 500);
      }
    }

    else
    {
      ::close(epollfd);
      throw ConnectionException(SystemError,ulxr_i18n(ULXR_PCHAR("Could not perform epoll_create() call: ")), 500);
    }

#else

#ifdef __GNUC__
// #warning ("message using select()")
#endif

    ULXR_TRACE(ULXR_PCHAR("waiting for select()"));

    fd_set oReadSockSet;
    FD_ZERO( &oReadSockSet );
    FD_SET( getServerData()->getSocket(), &oReadSockSet );

    struct timeval  tv;
    struct timeval  *ptv = 0;
    tv.tv_sec = in_timeout;
    tv.tv_usec = 0;
    if (in_timeout != 0)
      ptv = &tv;

    int ret = 0;

    if((ret = select(FD_SETSIZE, &oReadSockSet, NULL, NULL, ptv)) < 0)
      throw ConnectionException(SystemError,
          ulxr_i18n(ULXR_PCHAR("Could not wait for the connection (select() error):"))
          + ULXR_GET_STRING(getErrorString(getLastError())), 500);

    // checking whether the timeout occured
    if(!ret)
    {
      ULXR_TRACE(ULXR_PCHAR("accept returns false"));
      return false;
    }

#endif // ULXR_USE_EPOLL

  }

  ULXR_TRACE(ULXR_PCHAR("waiting for connection"));
  do
    setHandle(::accept(getServerHandle(),
                              (sockaddr*) &pimpl->remotedata, &pimpl->remotedata_len ));
  while(getHandle() < 0 && (errno == EINTR || errno == EAGAIN));

  if(getHandle() < 0)
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Could not accept a connection: "))
                                   + ULXR_GET_STRING(getErrorString(getLastError())), 500);

  doTcpNoDelay();

#ifdef ULXR_ENABLE_GET_PEERNAME
  struct hostent *host = 0;

#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
  Mutex::Locker lock(gethostbyaddrMutex);
#endif

#ifdef ULXR_ENABLE_DNS_LOOKUP
  host = gethostbyaddr((char*)&pimpl->remotedata.sin_addr,
                        sizeof(pimpl->remotedata.sin_addr),
                        AF_INET);
#endif

  if (0 == host)
    pimpl->remote_name = ULXR_GET_STRING(inet_ntoa(pimpl->remotedata.sin_addr))
                         + ULXR_GET_STRING(":") + HtmlFormHandler::makeNumber(ntohs(pimpl->remotedata.sin_port));
  else
    pimpl->remote_name = ULXR_GET_STRING(host->h_name);
#else
  pimpl->remote_name = ULXR_PCHAR("");
#endif

  ULXR_TRACE(ULXR_PCHAR("/accept"));

  abortOnClose(true);
  return true;
}


ULXR_API_IMPL(unsigned) TcpIpConnection::getPort()
{
  if(pimpl->port == 0)
  {
    struct sockaddr_in saddr_in;
    socklen_t size = sizeof(struct sockaddr_in);

    int err = getsockname(getServerHandle(), (struct sockaddr*) &saddr_in, &size);
    if(!err)
      pimpl->port = ntohs(saddr_in.sin_port);
  }

  return pimpl->port;
}


ULXR_API_IMPL(CppString) TcpIpConnection::getHostName() const
{
  return pimpl->host_name;
}


ULXR_API_IMPL(CppString) TcpIpConnection::getServerDomain() const
{
  return pimpl->serverdomain;
}


ULXR_API_IMPL(CppString) TcpIpConnection::getPeerName() const
{
  return pimpl->remote_name;
}


ULXR_API_IMPL(void) TcpIpConnection::setServerData (ServerSocketData *in_server_data)
{
  ULXR_TRACE(ULXR_PCHAR("setServerData ") << std::hex << (void*)in_server_data
                                                      << ULXR_PCHAR(" ") << (void*)pimpl->server_data << std::dec);
  // first closing the previous server, but no shutdown (a forked process can
  // be using it)!
  if(pimpl->server_data != 0)
    decrementServerRef();

  pimpl->server_data = in_server_data;
}


ULXR_API_IMPL(int) TcpIpConnection::getServerHandle ()
{
  if (pimpl->server_data != 0)
    return pimpl->server_data->getSocket();

  return -1;
}


ULXR_API_IMPL(TcpIpConnection::ServerSocketData *) TcpIpConnection::getServerData () const
{
  ULXR_TRACE(ULXR_PCHAR("getServerData ") << std::hex << (void*)pimpl->server_data << std::dec);
  return pimpl->server_data;
}


ULXR_API_IMPL(void) TcpIpConnection::shutdown(int in_mode)
{
    int handle = getServerHandle ();
    if (handle < 0)
      handle = getHandle();

    ULXR_TRACE(ULXR_PCHAR("shutdown for ") << handle);

    int ret;
    do
      ret = ::shutdown(handle, in_mode);
    while(ret < 0 && (errno == EINTR || errno == EAGAIN));  // @todo remove errne check?

    if(ret < 0)
      throw ConnectionException(TransportError,
                                ULXR_PCHAR("Shutdown failed: ")+getErrorString(getLastError()), 500);

    ULXR_TRACE(ULXR_PCHAR("shutdown succeeded"));
}


ULXR_API_IMPL(void) TcpIpConnection::close()
{
  ULXR_TRACE(ULXR_PCHAR("close"));
#ifdef ULXR_ENABLE_GET_PEERNAME
//  pimpl->remote_name = ULXR_PCHAR("");
#endif
  Connection::close();
}


ULXR_API_IMPL(int) TcpIpConnection::abortOnClose(int bOn)
{
  linger sock_linger_struct = {1, 0};
  sock_linger_struct.l_onoff = bOn;
  sock_linger_struct.l_linger = getTimeout();

  int handle = getHandle();
  if (pimpl->server_data != 0)
    handle = pimpl->server_data->getSocket();

#ifdef __WIN32__
  return setsockopt(handle, SOL_SOCKET, SO_LINGER,
                    (const char*)&sock_linger_struct, sizeof(linger));
#else
  return setsockopt(handle, SOL_SOCKET, SO_LINGER,
                    &sock_linger_struct, sizeof(linger));
#endif
}


ULXR_API_IMPL(void) TcpIpConnection::setTcpNoDelay(bool bOn)
{
  noDelayOpt = 0;
  if (bOn)
    noDelayOpt = 1;
  doTcpNoDelay();
}


ULXR_API_IMPL(int) TcpIpConnection::doTcpNoDelay()
{
 int sock;
 if (getServerData() != 0)
   sock = getServerData()->getSocket();
 else
   sock = getHandle();

 int ret = -1;
 if (sock > 0)
#ifdef __WIN32__
   ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                    (const char*)&noDelayOpt, sizeof(noDelayOpt));
#else
   ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                    &noDelayOpt, sizeof(noDelayOpt));
#endif

  ULXR_TRACE(ULXR_PCHAR("/doTcpNoDelay:") << noDelayOpt
            << ULXR_PCHAR(" ret: ") << ret
            << ULXR_PCHAR(" sock: ") << sock);
  return ret;
}


ULXR_API_IMPL(CppString) TcpIpConnection::getInterfaceName()
{
  return ULXR_PCHAR("tcpip");
}


ULXR_API_IMPL(int) TcpIpConnection::getLastError()
{
#ifdef __WIN32__
    return(WSAGetLastError());
#else
    return(errno);
#endif
}


}  // namespace ulxr


#endif // ULXR_OMIT_TCP_STUFF
